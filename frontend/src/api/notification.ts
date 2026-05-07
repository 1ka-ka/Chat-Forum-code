// 通知相关 API：获取通知列表、未读数、标记已读
import request from './request'
import type { ApiResponse, Notification, PageResult } from '@/types'

export const notificationApi = {
  // 获取通知列表（分页）
  getNotifications(page: number = 1, pageSize: number = 20) {
    return request.get<ApiResponse<PageResult<Notification>>>('/notifications', {
      params: { page, page_size: pageSize }
    })
  },

  // 获取未读通知数量（用于导航栏红点显示）
  getUnreadCount() {
    return request.get<ApiResponse<{ count: number }>>('/notifications/unread_count')
  },

  // 标记单条通知为已读
  markRead(notificationId: number) {
    return request.put<ApiResponse>(`/notifications/read/${notificationId}`)
  },

  // 标记所有通知为已读
  markAllRead() {
    return request.put<ApiResponse>('/notifications/read_all')
  }
}
