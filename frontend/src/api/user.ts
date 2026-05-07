// 用户相关 API：注册、登录、获取/修改用户信息
import request from './request'
import type { ApiResponse, LoginData, User } from '@/types'

export const userApi = {
  // 注册新用户
  register(username: string, password: string, nickname: string) {
    return request.post<ApiResponse<LoginData>>('/auth/register', {
      username,
      password,
      nickname
    })
  },

  // 登录，返回用户信息和 Token
  login(username: string, password: string) {
    return request.post<ApiResponse<LoginData>>('/auth/login', {
      username,
      password
    })
  },

  // 获取当前登录用户的资料
  getProfile() {
    return request.get<ApiResponse<User>>('/user/profile')
  },

  // 根据 ID 获取其他用户的公开信息
  getUserById(userId: number) {
    return request.get<ApiResponse<User>>(`/user/${userId}`)
  },

  // 更新用户资料（昵称和头像），使用 FormData 因为头像为文件上传
  updateProfile(data: { nickname?: string; avatar?: File }) {
    const formData = new FormData()
    if (data.nickname) {
      formData.append('nickname', data.nickname)
    }
    if (data.avatar) {
      formData.append('avatar', data.avatar)
    }
    return request.put<ApiResponse<User>>('/user/profile', formData, {
      headers: { 'Content-Type': 'multipart/form-data' }
    })
  }
}
