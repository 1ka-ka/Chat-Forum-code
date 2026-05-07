// 帖子相关 API：发帖、获取帖子列表、获取帖子详情
import request from './request'
import type { ApiResponse, Post, PageResult } from '@/types'

export const postApi = {
  // 创建帖子（支持上传图片，使用 FormData）
  createPost(data: { title: string; content: string; images?: File[] }) {
    const formData = new FormData()
    formData.append('title', data.title)
    formData.append('content', data.content)
    if (data.images) {
      data.images.forEach((img) => {
        formData.append('images', img)
      })
    }
    return request.post<ApiResponse<Post>>('/posts', formData, {
      headers: { 'Content-Type': 'multipart/form-data' }
    })
  },

  // 获取帖子列表（分页 + 搜索）
  getPostList(page: number = 1, pageSize: number = 20, search: string = '') {
    return request.get<ApiResponse<PageResult<Post>>>('/posts', {
      params: { page, page_size: pageSize, search }
    })
  },

  // 根据 ID 获取帖子详情
  getPostById(postId: number) {
    return request.get<ApiResponse<Post>>(`/posts/${postId}`)
  }
}
