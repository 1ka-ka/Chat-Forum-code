<script setup lang="ts">
import { onMounted, onUnmounted, watch } from 'vue'
import { RouterView } from 'vue-router'
import NavBar from './components/NavBar.vue'
import { useUserStore } from '@/stores/user'
import { useNotificationStore } from '@/stores/notification'
import { isLoggedIn } from '@/utils/auth'
import { wsManager } from '@/utils/websocket'

const userStore = useUserStore()
const notificationStore = useNotificationStore()

// 处理 WebSocket 推送的消息，目前只处理通知类型
function handleWsMessage(data: any) {
  if (data.type === 'notification') {
    notificationStore.handleIncomingNotification(data.notification)
  }
}

let removeWsHandler: (() => void) | null = null

// 应用挂载时：恢复登录状态、建立 WebSocket 连接、获取未读通知数
onMounted(async () => {
  // 如果已登录但还没获取用户信息，先拉取用户资料
  if (isLoggedIn() && !userStore.user) {
    await userStore.fetchProfile()
  }
  // 已登录则建立 WebSocket 实时连接，并注册消息处理函数
  if (isLoggedIn()) {
    wsManager.connect()
    removeWsHandler = wsManager.onMessage(handleWsMessage)
    notificationStore.fetchUnreadCount()
  }
})

// 监听用户状态变化：登录时连接 WebSocket，登出时断开
watch(() => userStore.user, (newUser) => {
  if (newUser) {
    wsManager.connect()
    if (!removeWsHandler) {
      removeWsHandler = wsManager.onMessage(handleWsMessage)
    }
    notificationStore.fetchUnreadCount()
  } else {
    wsManager.disconnect()
    if (removeWsHandler) {
      removeWsHandler()
      removeWsHandler = null
    }
  }
})

// 组件卸载时移除 WebSocket 消息监听，防止内存泄漏
onUnmounted(() => {
  if (removeWsHandler) {
    removeWsHandler()
    removeWsHandler = null
  }
})
</script>

<template>
  <div class="app-container">
    <NavBar />
    <main class="main-content">
      <!-- RouterView 根据当前路由渲染对应的页面组件 -->
      <RouterView />
    </main>
  </div>
</template>

<style scoped>
.app-container {
  min-height: 100vh;
  background-color: #f5f5f5;
}

.main-content {
  max-width: 1200px;
  margin: 0 auto;
  padding: 20px;
}
</style>
