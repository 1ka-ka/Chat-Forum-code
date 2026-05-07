// 评论相关 API：发表评论、获取评论列表
import request from './request'
import type { ApiResponse, Comment, PageResult } from '@/types'

export const commentApi = {
  // 发表评论，parentCommentId 不为空时表示回复某条评论
  createComment(postId: number, content: string, parentCommentId: number | null = null) {
    return request.post<ApiResponse<Comment>>(`/posts/${postId}/comments`, {
      content,
      parent_comment_id: parentCommentId
    })
  },

  // 获取某篇帖子的评论列表（分页）
  getComments(postId: number, page: number = 1, pageSize: number = 20) {
    return request.get<ApiResponse<PageResult<Comment>>>(`/posts/${postId}/comments`, {
      params: { page, page_size: pageSize }
    })
  }
}
