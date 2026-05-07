<script setup lang="ts">
import { computed } from 'vue'
import type { Message } from '@/types'
import { useUserStore } from '@/stores/user'

const props = defineProps<{
  message: Message
}>()

const userStore = useUserStore()

// 判断消息是否为自己发送的（用于区分左右显示）
const isSelf = computed(() => {
  return props.message.sender_id === userStore.user?.id
})
</script>

<template>
  <!-- 自己的消息靠右显示，对方的消息靠左显示 -->
  <div class="chat-message" :class="{ self: isSelf }">
    <div class="message-bubble">
      <p class="message-content">{{ message.content }}</p>
      <span class="message-time">{{ message.created_at }}</span>
    </div>
  </div>
</template>

<style scoped>
.chat-message {
  display: flex;
  margin-bottom: 12px;
}

.chat-message.self {
  justify-content: flex-end;
}

.message-bubble {
  max-width: 70%;
  padding: 10px 14px;
  border-radius: 12px;
  background: white;
  box-shadow: 0 1px 4px rgba(0, 0, 0, 0.1);
}

.chat-message.self .message-bubble {
  background: #409eff;
  color: white;
}

.message-content {
  line-height: 1.5;
  word-break: break-word;
}

.message-time {
  display: block;
  font-size: 10px;
  color: #999;
  margin-top: 4px;
  text-align: right;
}

.chat-message.self .message-time {
  color: rgba(255, 255, 255, 0.7);
}
</style>
