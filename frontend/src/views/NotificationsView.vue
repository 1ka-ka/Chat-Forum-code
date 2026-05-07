<script setup lang="ts">
import { ref, onMounted, onUnmounted } from 'vue'
import { useRouter } from 'vue-router'
import { ElButton, ElAvatar, ElEmpty, ElMessage } from 'element-plus'
import { notificationApi } from '@/api/notification'
import { useNotificationStore } from '@/stores/notification'
import { wsManager } from '@/utils/websocket'
import type { Notification } from '@/types'

const router = useRouter()
const notificationStore = useNotificationStore()

const notifications = ref<Notification[]>([])
const loading = ref(false)

async function fetchNotifications() {
  loading.value = true
  try {
    const res = await notificationApi.getNotifications(1, 50)
    notifications.value = res.data.items
  } catch {
  } finally {
    loading.value = false
  }
}

async function markAllRead() {
  try {
    await notificationApi.markAllRead()
    notifications.value.forEach(n => n.is_read = 1)
    notificationStore.clearAll()
    ElMessage.success('全部已读')
  } catch {
  }
}

// 点击通知：标记已读并跳转到相关页面
async function handleClick(n: Notification) {
  if (!n.is_read) {
    try {
      await notificationApi.markRead(n.id)
      n.is_read = 1
      notificationStore.decrement()
    } catch {
    }
  }

  // 根据通知类型跳转到对应页面
  if (n.type === 'like' || n.type === 'comment') {
    if (n.post_id) {
      router.push(`/post/${n.post_id}`)
    }
  } else if (n.type === 'reply') {
    if (n.post_id) {
      router.push(`/post/${n.post_id}`)
    }
  } else if (n.type === 'chat') {
    router.push(`/chat/${n.actor_id}`)
  }
}

// 监听 WebSocket 实时通知：在本页面时收到新通知自动刷新列表
function handleWsMessage(data: any) {
  if (data.type === 'notification') {
    fetchNotifications()
  }
}

let removeWsHandler: (() => void) | null = null

onMounted(() => {
  fetchNotifications()
  // 注册 WebSocket 消息监听，实现通知列表实时刷新
  removeWsHandler = wsManager.onMessage(handleWsMessage)
})

onUnmounted(() => {
  // 页面离开时移除监听，防止内存泄漏
  if (removeWsHandler) {
    removeWsHandler()
    removeWsHandler = null
  }
})

function goBack() {
  router.push('/')
}

function getTypeIcon(type: string) {
  if (type === 'like') return '👍'
  if (type === 'comment') return '💬'
  if (type === 'reply') return '↩️'
  if (type === 'chat') return '✉️'
  return '🔔'
}
</script>

<template>
  <div class="notifications-view">
    <div class="notifications-header">
      <ElButton text @click="goBack">← 返回主页</ElButton>
      <h2>消息通知</h2>
      <ElButton v-if="notifications.length > 0" text type="primary" @click="markAllRead">
        全部已读
      </ElButton>
    </div>

    <div v-loading="loading" class="notifications-list">
      <div
        v-for="n in notifications"
        :key="n.id"
        class="notification-item"
        :class="{ unread: !n.is_read }"
        @click="handleClick(n)"
      >
        <ElAvatar :size="40" :src="n.actor_avatar_url || '/default-avatar.png'" />
        <div class="notification-content">
          <div class="notification-message">
            <span class="type-icon">{{ getTypeIcon(n.type) }}</span>
            {{ n.message }}
          </div>
          <span class="notification-time">{{ n.created_at }}</span>
        </div>
        <div v-if="!n.is_read" class="unread-dot"></div>
      </div>
      <ElEmpty v-if="notifications.length === 0 && !loading" description="暂无通知" />
    </div>
  </div>
</template>

<style scoped>
.notifications-view {
  max-width: 700px;
  margin: 0 auto;
}

.notifications-header {
  display: flex;
  align-items: center;
  gap: 16px;
  margin-bottom: 20px;
}

.notifications-header h2 {
  flex: 1;
  font-size: 20px;
  color: #333;
}

.notifications-list {
  background: white;
  border-radius: 12px;
  overflow: hidden;
  min-height: 200px;
}

.notification-item {
  display: flex;
  align-items: center;
  gap: 12px;
  padding: 16px 20px;
  border-bottom: 1px solid #f0f0f0;
  cursor: pointer;
  transition: background 0.2s;
}

.notification-item:hover {
  background: #f5f7fa;
}

.notification-item.unread {
  background: #ecf5ff;
}

.notification-content {
  flex: 1;
  min-width: 0;
}

.notification-message {
  font-size: 14px;
  color: #333;
  line-height: 1.5;
}

.type-icon {
  margin-right: 4px;
}

.notification-time {
  font-size: 12px;
  color: #999;
}

.unread-dot {
  width: 8px;
  height: 8px;
  border-radius: 50%;
  background: #409eff;
  flex-shrink: 0;
}
</style>
