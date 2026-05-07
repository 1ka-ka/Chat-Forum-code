<script setup lang="ts">
import type { Comment } from '@/types'
import { ElAvatar } from 'element-plus'
import { useRouter } from 'vue-router'

const props = defineProps<{
  comment: Comment
}>()

const router = useRouter()

// 点击头像/昵称跳转到用户主页
function goToUser() {
  router.push(`/user/${props.comment.user_id}`)
}
</script>

<template>
  <div class="comment-item">
    <ElAvatar
      class="comment-avatar"
      :size="36"
      :src="comment.avatar_url || '/default-avatar.png'"
      @click="goToUser"
    />
    <div class="comment-content">
      <div class="comment-header">
        <span class="comment-author" @click="goToUser">{{ comment.nickname }}</span>
        <span class="comment-time">{{ comment.created_at }}</span>
      </div>
      <p class="comment-text">{{ comment.content }}</p>
    </div>
  </div>
</template>

<style scoped>
.comment-item {
  display: flex;
  gap: 12px;
  padding: 12px 0;
  border-bottom: 1px solid #f0f0f0;
}

.comment-item:last-child {
  border-bottom: none;
}

.comment-avatar {
  cursor: pointer;
  flex-shrink: 0;
}

.comment-content {
  flex: 1;
}

.comment-header {
  display: flex;
  align-items: center;
  gap: 10px;
  margin-bottom: 4px;
}

.comment-author {
  font-weight: 500;
  color: #333;
  cursor: pointer;
}

.comment-author:hover {
  color: #409eff;
}

.comment-time {
  font-size: 12px;
  color: #999;
}

.comment-text {
  color: #666;
  line-height: 1.6;
}
</style>
