<script setup lang="ts">
import { ref, onMounted, computed } from 'vue'
import { useRoute, useRouter } from 'vue-router'
import { ElAvatar, ElButton, ElEmpty, ElPagination } from 'element-plus'
import { userApi } from '@/api/user'
import { postApi } from '@/api/post'
import { useUserStore } from '@/stores/user'
import { isLoggedIn } from '@/utils/auth'
import type { User, Post } from '@/types'
import PostCard from '@/components/PostCard.vue'

const route = useRoute()
const router = useRouter()
const userStore = useUserStore()

const userInfo = ref<User | null>(null)
const posts = ref<Post[]>([])
const loading = ref(false)
const currentPage = ref(1)
const pageSize = ref(20)
const total = ref(0)
const totalPages = ref(0)

// 判断查看的是否是自己的主页
const isSelf = computed(() => {
  return userInfo.value?.id === userStore.user?.id
})

const isGuest = computed(() => !userStore.user || !isLoggedIn())

async function fetchUserInfo() {
  loading.value = true
  try {
    const userId = Number(route.params.id)
    const res = await userApi.getUserById(userId)
    userInfo.value = res.data
  } catch {
  } finally {
    loading.value = false
  }
}

async function fetchUserPosts(page: number = 1) {
  const userId = Number(route.params.id)
  try {
    // 获取帖子列表后在前端过滤出该用户的帖子
    const res = await postApi.getPostList(page, pageSize.value)
    const userPosts = res.data.items.filter((p: Post) => p.user_id === userId)
    posts.value = userPosts
    total.value = res.data.total
    totalPages.value = res.data.total_pages
    currentPage.value = page
  } catch {
  }
}

function goToChat() {
  if (userInfo.value) {
    router.push(`/chat/${userInfo.value.id}`)
  }
}

function goBack() {
  router.back()
}

function requireLogin() {
  router.push({ path: '/login', query: { redirect: route.fullPath } })
}

function handlePageChange(page: number) {
  fetchUserPosts(page)
}

onMounted(() => {
  fetchUserInfo()
  fetchUserPosts()
})
</script>

<template>
  <div class="user-view">
    <ElButton text @click="goBack">← 返回</ElButton>

    <div v-loading="loading" class="user-profile-card">
      <div v-if="userInfo" class="user-info">
        <ElAvatar :size="80" :src="userInfo.avatar_url || '/default-avatar.png'" />
        <div class="user-details">
          <h2>{{ userInfo.nickname }}</h2>
          <p class="user-meta">
            <span>发帖数：{{ userInfo.post_count || 0 }}</span>
            <span>注册时间：{{ userInfo.created_at }}</span>
          </p>
        </div>
        <div class="user-actions">
          <ElButton v-if="!isGuest && !isSelf" type="primary" @click="goToChat">发私信</ElButton>
          <ElButton v-if="isGuest" @click="requireLogin">登录后私信</ElButton>
        </div>
      </div>
      <ElEmpty v-else-if="!loading" description="用户不存在" />
    </div>

    <div class="user-posts">
      <h3>TA的帖子</h3>
      <div v-if="posts.length > 0" class="post-list">
        <PostCard v-for="post in posts" :key="post.id" :post="post" />
      </div>
      <ElEmpty v-else description="暂无帖子" />

      <div v-if="totalPages > 1" class="pagination">
        <ElPagination
          v-model:current-page="currentPage"
          :page-size="pageSize"
          :total="total"
          layout="prev, pager, next"
          @current-change="handlePageChange"
        />
      </div>
    </div>
  </div>
</template>

<style scoped>
.user-view {
  max-width: 800px;
  margin: 0 auto;
}

.user-profile-card {
  background: white;
  border-radius: 12px;
  padding: 30px;
  margin-bottom: 20px;
  min-height: 100px;
  margin-top: 10px;
}

.user-info {
  display: flex;
  align-items: center;
  gap: 20px;
}

.user-details {
  flex: 1;
}

.user-details h2 {
  font-size: 22px;
  color: #333;
  margin-bottom: 8px;
}

.user-meta {
  display: flex;
  gap: 20px;
  color: #999;
  font-size: 14px;
}

.user-posts {
  background: white;
  border-radius: 12px;
  padding: 20px;
}

.user-posts h3 {
  margin-bottom: 16px;
  color: #333;
}

.post-list {
  display: flex;
  flex-direction: column;
  gap: 16px;
}

.pagination {
  display: flex;
  justify-content: center;
  margin-top: 20px;
}
</style>
