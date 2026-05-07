<script setup lang="ts">
import { ref, onMounted, onUnmounted, computed, watch, nextTick } from 'vue'
import { useRoute, useRouter } from 'vue-router'
import { ElInput, ElButton, ElAvatar, ElEmpty, ElScrollbar } from 'element-plus'
import { useUserStore } from '@/stores/user'
import { useChatStore } from '@/stores/chat'
import { wsManager } from '@/utils/websocket'
import type { Message } from '@/types'

const route = useRoute()
const router = useRouter()
const userStore = useUserStore()
const chatStore = useChatStore()

const messageInput = ref('')
const messagesContainer = ref<HTMLElement | null>(null)

// 从路由参数获取当前聊天对象 ID
const currentUserId = computed(() => {
  const id = route.params.userId
  return id ? Number(id) : null
})

// 从会话列表中找到当前聊天对象的信息
const currentConversation = computed(() => {
  if (!currentUserId.value) return null
  return chatStore.conversations.find((c) => c.user_id === currentUserId.value)
})

// 通过 WebSocket 发送消息
async function sendMessage() {
  if (!messageInput.value.trim() || !currentUserId.value) return

  wsManager.sendMessage(currentUserId.value, messageInput.value)
  messageInput.value = ''
}

// 滚动到消息列表底部（新消息时自动滚动）
function scrollToBottom() {
  nextTick(() => {
    if (messagesContainer.value) {
      messagesContainer.value.scrollTop = messagesContainer.value.scrollHeight
    }
  })
}

// 处理 WebSocket 推送的消息
function handleMessage(data: any) {
  if (data.type === 'message') {
    const message: Message = {
      id: data.message_id,
      sender_id: data.sender_id,
      receiver_id: data.receiver_id,
      content: data.content,
      is_read: 0,
      created_at: data.created_at
    }

    const myId = userStore.user?.id || 0
    // 判断消息的对方用户 ID
    const otherUserId = data.sender_id === myId ? data.receiver_id : data.sender_id

    if (otherUserId === currentUserId.value) {
      // 正在和对方聊天，直接显示消息并标记已读
      chatStore.addMessage(message)
      if (data.sender_id !== myId) {
        chatStore.markAsRead(currentUserId.value!)
      }
    } else if (data.sender_id !== myId) {
      // 收到非当前聊天对象的消息，更新会话未读数
      chatStore.incrementUnread(data.sender_id)
    }

    scrollToBottom()
  } else if (data.type === 'online_status') {
    // 处理用户上线/离线状态变更
    chatStore.handleOnlineStatus(data.user_id, data.online)
  }
}

// 点击会话列表中的某个会话，切换聊天对象
async function selectConversation(userId: number) {
  router.push(`/chat/${userId}`)
}

// 加载与当前聊天对象的消息记录
async function loadChat() {
  if (currentUserId.value) {
    chatStore.setCurrentChat(currentUserId.value)
    await chatStore.fetchMessages(currentUserId.value)
    scrollToBottom()
  }
}

// 格式化消息时间：今天只显示时分，其他日期显示月日+时分
function formatTime(timeStr: string) {
  if (!timeStr) return ''
  const d = new Date(timeStr)
  const now = new Date()
  const isToday = d.toDateString() === now.toDateString()
  if (isToday) {
    return d.toLocaleTimeString('zh-CN', { hour: '2-digit', minute: '2-digit' })
  }
  return d.toLocaleDateString('zh-CN', { month: '2-digit', day: '2-digit' }) + ' ' +
    d.toLocaleTimeString('zh-CN', { hour: '2-digit', minute: '2-digit' })
}

// 路由参数变化时重新加载聊天（切换聊天对象）
watch(currentUserId, async (newId) => {
  if (newId) {
    await loadChat()
  }
})

let removeMessageHandler: (() => void) | null = null

onMounted(async () => {
  await chatStore.fetchConversations()
  // WebSocket 在 App.vue 中已全局连接，这里只需注册消息处理器
  removeMessageHandler = wsManager.onMessage(handleMessage)

  if (currentUserId.value) {
    await loadChat()
  }
})

onUnmounted(() => {
  // 页面离开时移除消息监听，并清除当前聊天对象
  if (removeMessageHandler) {
    removeMessageHandler()
    removeMessageHandler = null
  }
  chatStore.setCurrentChat(null)
})
</script>

<template>
  <div class="chat-page">
    <ElButton text @click="router.push('/')" style="margin-bottom: 8px">← 返回主页</ElButton>
    <div class="chat-view">
      <div class="conversation-panel">
        <div class="panel-header">
          <h3>私信</h3>
        </div>
      <div class="conversation-list">
        <div
          v-for="conv in chatStore.conversations"
          :key="conv.user_id"
          class="conv-item"
          :class="{ active: conv.user_id === currentUserId }"
          @click="selectConversation(conv.user_id)"
        >
          <ElAvatar :size="44" :src="conv.avatar_url || '/default-avatar.png'" />
          <div class="conv-detail">
            <div class="conv-top">
              <span class="conv-name">{{ conv.nickname }}</span>
              <span class="conv-time">{{ formatTime(conv.last_time) }}</span>
            </div>
            <div class="conv-bottom">
              <span class="conv-msg">{{ conv.last_message }}</span>
              <!-- 未读消息数红点 -->
              <span v-if="conv.unread_count > 0" class="unread-badge">{{ conv.unread_count }}</span>
            </div>
          </div>
        </div>
        <ElEmpty v-if="chatStore.conversations.length === 0" description="暂无会话" :image-size="80" />
      </div>
    </div>

    <div class="chat-panel">
      <template v-if="currentUserId">
        <div class="chat-header">
          <ElAvatar :size="32" :src="currentConversation?.avatar_url || '/default-avatar.png'" />
          <span class="chat-target-name">{{ currentConversation?.nickname || '聊天' }}</span>
        </div>

        <div ref="messagesContainer" class="messages-area">
          <div v-if="chatStore.currentMessages.length === 0" class="no-messages">
            还没有消息，开始聊天吧
          </div>
          <div
            v-for="msg in chatStore.currentMessages"
            :key="msg.id"
            class="msg-row"
            :class="{ self: msg.sender_id === userStore.user?.id }"
          >
            <!-- 对方消息：头像在左侧 -->
            <ElAvatar
              v-if="msg.sender_id !== userStore.user?.id"
              :size="36"
              :src="currentConversation?.avatar_url || '/default-avatar.png'"
              class="msg-avatar"
            />
            <div class="msg-body">
              <div class="msg-bubble">
                <p>{{ msg.content }}</p>
              </div>
              <span class="msg-time">{{ formatTime(msg.created_at) }}</span>
            </div>
            <!-- 自己的消息：头像在右侧 -->
            <ElAvatar
              v-if="msg.sender_id === userStore.user?.id"
              :size="36"
              :src="userStore.user?.avatar_url || '/default-avatar.png'"
              class="msg-avatar"
            />
          </div>
        </div>

        <div class="input-area">
          <ElInput
            v-model="messageInput"
            placeholder="输入消息..."
            @keyup.enter="sendMessage"
            size="large"
          />
          <ElButton type="primary" size="large" @click="sendMessage">发送</ElButton>
        </div>
      </template>
      <div v-else class="empty-chat">
        <ElEmpty description="选择一个会话开始聊天" :image-size="120" />
      </div>
      </div>
    </div>
  </div>
</template>

<style scoped>
.chat-view {
  display: flex;
  height: calc(100vh - 100px);
  background: white;
  border-radius: 12px;
  overflow: hidden;
  box-shadow: 0 2px 12px rgba(0, 0, 0, 0.06);
}

.conversation-panel {
  width: 300px;
  border-right: 1px solid #ebeef5;
  display: flex;
  flex-direction: column;
}

.panel-header {
  padding: 20px;
  border-bottom: 1px solid #ebeef5;
}

.panel-header h3 {
  margin: 0;
  font-size: 18px;
  color: #303133;
}

.conversation-list {
  flex: 1;
  overflow-y: auto;
}

.conv-item {
  display: flex;
  align-items: center;
  gap: 12px;
  padding: 14px 20px;
  cursor: pointer;
  transition: background 0.2s;
  border-bottom: 1px solid #f5f5f5;
}

.conv-item:hover {
  background: #f5f7fa;
}

.conv-item.active {
  background: #ecf5ff;
}

.conv-detail {
  flex: 1;
  min-width: 0;
}

.conv-top {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 4px;
}

.conv-name {
  font-weight: 600;
  font-size: 14px;
  color: #303133;
}

.conv-time {
  font-size: 11px;
  color: #c0c4cc;
}

.conv-bottom {
  display: flex;
  align-items: center;
  gap: 8px;
}

.conv-msg {
  font-size: 12px;
  color: #909399;
  overflow: hidden;
  text-overflow: ellipsis;
  white-space: nowrap;
  flex: 1;
}

.unread-badge {
  background: #f56c6c;
  color: white;
  font-size: 10px;
  min-width: 18px;
  height: 18px;
  line-height: 18px;
  text-align: center;
  border-radius: 9px;
  padding: 0 5px;
  flex-shrink: 0;
}

.chat-panel {
  flex: 1;
  display: flex;
  flex-direction: column;
  min-width: 0;
}

.chat-header {
  display: flex;
  align-items: center;
  gap: 10px;
  padding: 16px 24px;
  border-bottom: 1px solid #ebeef5;
  background: #fafafa;
}

.chat-target-name {
  font-weight: 600;
  font-size: 16px;
  color: #303133;
}

.messages-area {
  flex: 1;
  padding: 20px 24px;
  overflow-y: auto;
  background: #f5f7fa;
}

.no-messages {
  text-align: center;
  color: #c0c4cc;
  padding-top: 80px;
  font-size: 14px;
}

.msg-row {
  display: flex;
  align-items: flex-start;
  gap: 10px;
  margin-bottom: 16px;
}

.msg-row.self {
  flex-direction: row-reverse;
}

.msg-avatar {
  flex-shrink: 0;
}

.msg-body {
  max-width: 60%;
  min-width: 0;
}

.msg-bubble {
  padding: 10px 16px;
  border-radius: 12px;
  background: white;
  box-shadow: 0 1px 4px rgba(0, 0, 0, 0.06);
  word-break: break-word;
}

.msg-bubble p {
  margin: 0;
  line-height: 1.6;
  font-size: 14px;
  color: #303133;
}

.msg-row.self .msg-bubble {
  background: #409eff;
}

.msg-row.self .msg-bubble p {
  color: white;
}

.msg-time {
  display: block;
  font-size: 11px;
  color: #c0c4cc;
  margin-top: 4px;
}

.msg-row.self .msg-time {
  text-align: right;
}

.input-area {
  display: flex;
  gap: 12px;
  padding: 16px 24px;
  border-top: 1px solid #ebeef5;
  background: white;
}

.input-area .el-input {
  flex: 1;
}

.empty-chat {
  flex: 1;
  display: flex;
  align-items: center;
  justify-content: center;
}
</style>
