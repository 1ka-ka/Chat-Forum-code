// 用户状态管理：存储当前登录用户信息，提供登录/登出/更新资料等方法
import { defineStore } from 'pinia'
import { ref } from 'vue'
import type { User } from '@/types'
import { setToken, removeToken, isLoggedIn } from '@/utils/auth'
import { userApi } from '@/api/user'

// 使用 Pinia 组合式 API 风格定义 store
export const useUserStore = defineStore('user', () => {
  // 当前登录用户信息，null 表示未登录
  const user = ref<User | null>(null)
  const token = ref<string | null>(null)

  function setUser(userData: User) {
    user.value = userData
  }

  // 保存 Token 到 store 和 localStorage
  function setUserToken(userToken: string) {
    token.value = userToken
    setToken(userToken)
  }

  // 登录：同时保存用户信息和 Token
  function login(userData: User, userToken: string) {
    user.value = userData
    token.value = userToken
    setToken(userToken)
  }

  // 登出：清除用户信息和 Token
  function logout() {
    user.value = null
    token.value = null
    removeToken()
  }

  // 从后端拉取当前用户资料（页面刷新后恢复登录状态）
  async function fetchProfile() {
    if (!isLoggedIn()) return
    try {
      const res = await userApi.getProfile()
      user.value = res.data
    } catch {
      // Token 无效时自动登出
      logout()
    }
  }

  // 更新用户资料并同步到 store
  async function updateProfile(data: { nickname?: string; avatar?: File }) {
    const res = await userApi.updateProfile(data)
    user.value = res.data
  }

  return {
    user,
    token,
    setUser,
    setUserToken,
    login,
    logout,
    fetchProfile,
    updateProfile
  }
})
