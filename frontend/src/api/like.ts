// 点赞相关 API：点赞/取消点赞、获取点赞用户列表
import request from './request'
import type { ApiResponse, LikeUser } from '@/types'

// 点赞操作的结果：当前是否已点赞 + 总点赞数
export interface LikeResult {
  is_liked: boolean
  like_count: number
}

export const likeApi = {
  // 切换点赞状态（已点赞则取消，未点赞则点赞）
  toggleLike(postId: number) {
    return request.post<ApiResponse<LikeResult>>(`/posts/${postId}/like`)
  },

  // 获取某篇帖子的点赞用户列表
  getLikeUsers(postId: number) {
    return request.get<ApiResponse<LikeUser[]>>(`/posts/${postId}/like/users`)
  }
}
