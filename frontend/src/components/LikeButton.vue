<script setup lang="ts">
import { ref } from 'vue'
import { likeApi } from '@/api/like'
import { ElMessage } from 'element-plus'
import { isLoggedIn } from '@/utils/auth'
import { useRouter } from 'vue-router'

const props = defineProps<{
  postId: number
  initialLiked: boolean
  initialCount: number
}>()

// 通知父组件点赞数变化
const emit = defineEmits<{
  (e: 'update', count: number): void
}>()

const isLiked = ref(props.initialLiked)
const likeCount = ref(props.initialCount)
const loading = ref(false)
const router = useRouter()

// 切换点赞状态
async function toggleLike() {
  if (!isLoggedIn()) {
    router.push('/login')
    return
  }

  // 防止重复点击
  if (loading.value) return
  loading.value = true

  try {
    const res = await likeApi.toggleLike(props.postId)
    isLiked.value = res.data.is_liked
    likeCount.value = res.data.like_count
    // 将最新点赞数传递给父组件
    emit('update', likeCount.value)
  } catch {
    ElMessage.error('操作失败')
  } finally {
    loading.value = false
  }
}
</script>

<template>
  <button
    class="like-button"
    :class="{ liked: isLiked }"
    @click.stop="toggleLike"
    :disabled="loading"
  >
    <span class="heart">{{ isLiked ? '❤️' : '🤍' }}</span>
    <span class="count">{{ likeCount }}</span>
  </button>
</template>

<style scoped>
.like-button {
  display: flex;
  align-items: center;
  gap: 4px;
  padding: 6px 12px;
  border-radius: 20px;
  background: #f5f5f5;
  border: none;
  cursor: pointer;
  transition: all 0.2s;
}

.like-button:hover {
  background: #ffe6e6;
}

.like-button.liked {
  background: #fff0f0;
}

.heart {
  font-size: 16px;
}

.count {
  font-size: 14px;
  color: #666;
}

.like-button.liked .count {
  color: #f56c6c;
}
</style>
