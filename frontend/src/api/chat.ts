// 聊天相关 API：获取会话列表、消息记录、发送消息、标记已读
import request from './request'
import type { ApiResponse, Conversation, Message, PageResult } from '@/types'

export const chatApi = {
  // 获取当前用户的所有聊天会话
  getConversations() {
    return request.get<ApiResponse<Conversation[]>>('/chat/conversations')
  },

  // 获取与某个用户的消息记录（分页，用于加载历史消息）
  getMessages(userId: number, page: number = 1, pageSize: number = 20) {
    return request.get<ApiResponse<PageResult<Message>>>(`/chat/messages/${userId}`, {
      params: { page, page_size: pageSize }
    })
  },

  // 标记与某个用户的消息为已读
  markAsRead(userId: number) {
    return request.put<ApiResponse>(`/chat/read/${userId}`)
  },

  // 发送消息（通过 HTTP 接口发送，WebSocket 用于实时接收）
  sendMessage(receiverId: number, content: string) {
    return request.post<ApiResponse<Message>>('/chat/send', {
      receiver_id: receiverId,
      content
    })
  }
}
