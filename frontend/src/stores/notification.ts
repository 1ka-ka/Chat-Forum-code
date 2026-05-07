// 通知状态管理：管理未读通知计数，处理实时通知推送的智能已读逻辑
import { defineStore } from 'pinia'
import { ref } from 'vue'
import { notificationApi } from '@/api/notification'
import { isLoggedIn } from '@/utils/auth'
import router from '@/router'

export const useNotificationStore = defineStore('notification', () => {
  // 未读通知数量（用于导航栏红点显示）
  const unreadCount = ref(0)

  // 从后端获取未读通知数量
  async function fetchUnreadCount() {
    if (!isLoggedIn()) {
      unreadCount.value = 0
      return
    }
    try {
      const res = await notificationApi.getUnreadCount()
      unreadCount.value = res.data.count
    } catch {
    }
  }

  // 未读数减一（标记单条已读时调用）
  function decrement() {
    if (unreadCount.value > 0) {
      unreadCount.value--
    }
  }

  // 清零未读数（全部标记已读时调用）
  function clearAll() {
    unreadCount.value = 0
  }

  /**
   * 处理 WebSocket 推送的实时通知 —— 智能已读逻辑
   * 如果用户已经在查看相关页面（如正在看被点赞的帖子），则自动标记已读，不显示红点
   * 否则增加未读计数，提示用户有新通知
   */
  async function handleIncomingNotification(notification: {
    id: number
    type: string
    post_id: number | null
    comment_id: number | null
    message: string
    actor_id: number
  }) {
    const currentRoute = router.currentRoute.value
    let isAlreadyOnPage = false

    // 判断用户是否正在查看通知相关的页面
    if (notification.type === 'like' || notification.type === 'comment' || notification.type === 'reply') {
      // 点赞/评论/回复通知：检查是否正在查看对应的帖子详情页
      if (notification.post_id && currentRoute.name === 'post-detail') {
        const routePostId = Number(currentRoute.params.id)
        if (routePostId === notification.post_id) {
          isAlreadyOnPage = true
        }
      }
    } else if (notification.type === 'chat') {
      // 聊天通知：检查是否正在和发送者聊天
      if (currentRoute.name === 'chat-user') {
        const routeUserId = Number(currentRoute.params.userId)
        if (routeUserId === notification.actor_id) {
          isAlreadyOnPage = true
        }
      }
    }

    if (isAlreadyOnPage) {
      // 用户已在相关页面，自动标记已读，不增加红点
      try {
        await notificationApi.markRead(notification.id)
      } catch {
      }
    } else {
      // 用户不在相关页面，增加未读计数以显示红点提示
      unreadCount.value++
    }
  }

  return {
    unreadCount,
    fetchUnreadCount,
    decrement,
    clearAll,
    handleIncomingNotification
  }
})
