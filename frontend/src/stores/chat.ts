// 聊天状态管理：管理会话列表、消息记录、在线状态
import { defineStore } from 'pinia'
import { ref } from 'vue'
import type { Message, Conversation } from '@/types'
import { chatApi } from '@/api/chat'
import { wsManager } from '@/utils/websocket'

export const useChatStore = defineStore('chat', () => {
  // 会话列表
  const conversations = ref<Conversation[]>([])
  // 当前聊天窗口的消息记录
  const currentMessages = ref<Message[]>([])
  // 当前正在聊天的用户 ID
  const currentChatUserId = ref<number | null>(null)
  // 在线用户 ID 集合
  const onlineUsers = ref<Set<number>>(new Set())

  // 获取所有聊天会话
  async function fetchConversations() {
    const res = await chatApi.getConversations()
    conversations.value = res.data || []
  }

  // 获取与某用户的消息记录
  async function fetchMessages(userId: number, page: number = 1) {
    const res = await chatApi.getMessages(userId, page)
    if (page === 1) {
      // 第一页：直接替换消息列表（后端返回的按时间倒序，需反转）
      currentMessages.value = res.data.items.reverse()
    } else {
      // 加载更多历史消息：插入到消息列表前面
      currentMessages.value = [...res.data.items.reverse(), ...currentMessages.value]
    }
    return res.data
  }

  // 标记与某用户的消息为已读，并更新会话列表中的未读计数
  async function markAsRead(userId: number) {
    await chatApi.markAsRead(userId)
    const conv = conversations.value.find((c) => c.user_id === userId)
    if (conv) {
      conv.unread_count = 0
    }
  }

  // 设置当前聊天对象，同时标记该会话消息为已读
  function setCurrentChat(userId: number | null) {
    currentChatUserId.value = userId
    if (userId) {
      markAsRead(userId)
    }
  }

  // 收到新消息时：添加到消息列表，并更新会话列表的最新消息
  function addMessage(message: Message) {
    currentMessages.value.push(message)
    // 根据消息的发送方/接收方找到对应的会话
    const otherUserId = message.sender_id === currentChatUserId.value
      ? message.receiver_id
      : message.sender_id
    const conv = conversations.value.find((c) => c.user_id === otherUserId)
    if (conv) {
      conv.last_message = message.content
      conv.last_time = message.created_at
    }
  }

  /**
   * 收到非当前聊天窗口的消息时，更新会话列表的未读计数
   */
  function incrementUnread(senderId: number) {
    const conv = conversations.value.find((c) => c.user_id === senderId)
    if (conv) {
      conv.unread_count = (conv.unread_count || 0) + 1
      conv.last_message = '' // 内容已通过 notification 推送
    }
  }

  // 更新用户在线/离线状态
  function handleOnlineStatus(userId: number, online: boolean) {
    if (online) {
      onlineUsers.value.add(userId)
    } else {
      onlineUsers.value.delete(userId)
    }
  }

  // 判断某用户是否在线
  function isOnline(userId: number): boolean {
    return onlineUsers.value.has(userId)
  }

  return {
    conversations,
    currentMessages,
    currentChatUserId,
    onlineUsers,
    fetchConversations,
    fetchMessages,
    markAsRead,
    setCurrentChat,
    addMessage,
    incrementUnread,
    handleOnlineStatus,
    isOnline
  }
})
