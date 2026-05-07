// Axios 请求封装：统一处理请求头、Token 注入、错误提示
import axios from 'axios'
import type { AxiosInstance, AxiosRequestConfig, AxiosError } from 'axios'
import { ElMessage } from 'element-plus'
import router from '@/router'
import { getToken, removeToken } from '@/utils/auth'

// 扩展 Axios 实例类型，让返回值直接是 data 而非 response
interface ApiInstance extends AxiosInstance {
  get<T = any>(url: string, config?: AxiosRequestConfig): Promise<T>;
  post<T = any>(url: string, data?: any, config?: AxiosRequestConfig): Promise<T>;
  put<T = any>(url: string, data?: any, config?: AxiosRequestConfig): Promise<T>;
  delete<T = any>(url: string, config?: AxiosRequestConfig): Promise<T>;
}

// 创建 Axios 实例，所有请求默认以 /api 开头
const request = axios.create({
  baseURL: '/api',
  timeout: 30000,
  headers: {
    'Content-Type': 'application/json'
  }
}) as ApiInstance

// 请求拦截器：在每个请求发出前自动附加 Token
request.interceptors.request.use(
  (config) => {
    const token = getToken()
    if (token) {
      config.headers.Authorization = `Bearer ${token}`
    }
    return config
  },
  (error) => {
    return Promise.reject(error)
  }
)

// 响应拦截器：统一处理后端返回的错误码
request.interceptors.response.use(
  (response) => {
    const res = response.data
    // 后端返回 code !== 200 表示业务逻辑错误
    if (res.code !== 200) {
      if (res.code === 401 && getToken()) {
        // Token 过期或无效：清除本地 Token 并跳转登录页
        removeToken()
        ElMessage.error('登录已过期，请重新登录')
        router.push({ path: '/login', query: { redirect: router.currentRoute.value.fullPath } })
      } else if (res.code !== 401) {
        ElMessage.error(res.message || '请求失败')
      }
      return Promise.reject(new Error(res.message || '请求失败'))
    }
    return res
  },
  (error: AxiosError) => {
    // HTTP 状态码级别的错误处理
    if (error.response?.status === 401 && getToken()) {
      removeToken()
      ElMessage.error('登录已过期，请重新登录')
      router.push({ path: '/login', query: { redirect: router.currentRoute.value.fullPath } })
    } else if (error.response?.status !== 401) {
      ElMessage.error((error.response?.data as any)?.message || '网络错误')
    }
    return Promise.reject(error)
  }
)

export default request
