<script setup lang="ts">
import { computed } from 'vue'
import { useRouter } from 'vue-router'
import type { Post } from '@/types'
import { ElCard, ElAvatar, ElButton } from 'element-plus'
import LikeButton from './LikeButton.vue'

const props = defineProps<{
  post: Post
}>()

const router = useRouter()

// 点击卡片跳转到帖子详情
function goToPost() {
  router.push(`/post/${props.post.id}`)
}

// 点击作者头像/昵称跳转到用户主页（阻止事件冒泡，避免触发卡片点击）
function goToUser(e: Event) {
  e.stopPropagation()
  router.push(`/user/${props.post.user_id}`)
}
</script>

<template>
  <ElCard class="post-card" shadow="hover" @click="goToPost">
    <div class="post-header">
      <div class="post-author" @click="goToUser">
        <ElAvatar :size="40" :src="post.avatar_url || '/default-avatar.png'" />
        <div class="author-info">
          <span class="nickname">{{ post.nickname }}</span>
          <span class="post-time">{{ post.created_at }}</span>
        </div>
      </div>
    </div>

    <div class="post-content">
      <h3 class="post-title">{{ post.title }}</h3>
      <!-- 内容摘要，最多显示3行 -->
      <p class="post-excerpt">{{ post.content }}</p>
      <!-- 图片预览，列表中最多显示3张 -->
      <div v-if="post.image_urls && post.image_urls.length > 0" class="post-images">
        <img
          v-for="(url, index) in post.image_urls.slice(0, 3)"
          :key="index"
          :src="url"
          alt="post image"
        />
      </div>
    </div>

    <div class="post-footer">
      <div class="post-stats">
        <span>👍 {{ post.like_count }}</span>
        <span>💬 {{ post.comment_count }}</span>
      </div>
      <LikeButton :post-id="post.id" :initial-liked="post.is_liked" :initial-count="post.like_count" @update="post.like_count = $event" />
    </div>
  </ElCard>
</template>

<style scoped>
.post-card {
  cursor: pointer;
  transition: transform 0.2s;
}

.post-card:hover {
  transform: translateY(-2px);
}

.post-header {
  display: flex;
  align-items: center;
  margin-bottom: 12px;
}

.post-author {
  display: flex;
  align-items: center;
  gap: 10px;
}

.author-info {
  display: flex;
  flex-direction: column;
}

.nickname {
  font-weight: 500;
  color: #333;
}

.post-time {
  font-size: 12px;
  color: #999;
}

.post-content {
  margin-bottom: 12px;
}

.post-title {
  font-size: 18px;
  font-weight: 600;
  margin-bottom: 8px;
  color: #333;
}

.post-excerpt {
  color: #666;
  line-height: 1.6;
  display: -webkit-box;
  -webkit-line-clamp: 3;
  -webkit-box-orient: vertical;
  overflow: hidden;
}

.post-images {
  display: flex;
  gap: 8px;
  margin-top: 12px;
}

.post-images img {
  width: 120px;
  height: 80px;
  object-fit: cover;
  border-radius: 4px;
}

.post-footer {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding-top: 12px;
  border-top: 1px solid #f0f0f0;
}

.post-stats {
  display: flex;
  gap: 20px;
  color: #666;
  font-size: 14px;
}
</style>
