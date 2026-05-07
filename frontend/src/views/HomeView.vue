<script setup lang="ts">
import { ref, onMounted, computed, watch } from 'vue'
import { useRouter } from 'vue-router'
import { ElButton, ElPagination, ElEmpty, ElInput, ElAlert } from 'element-plus'
import { postApi } from '@/api/post'
import { useUserStore } from '@/stores/user'
import { isLoggedIn } from '@/utils/auth'
import type { Post } from '@/types'
import PostCard from '@/components/PostCard.vue'

const router = useRouter()
const userStore = useUserStore()

const posts = ref<Post[]>([])
const loading = ref(false)
const currentPage = ref(1)
const pageSize = ref(20)
const total = ref(0)
const totalPages = ref(0)
const searchKeyword = ref('')
// 搜索防抖定时器
let searchTimer: ReturnType<typeof setTimeout> | null = null

// 判断是否为游客（未登录）
const isGuest = computed(() => !userStore.user || !isLoggedIn())

async function fetchPosts(page: number = 1) {
  loading.value = true
  try {
    const res = await postApi.getPostList(page, pageSize.value, searchKeyword.value)
    posts.value = res.data.items
    total.value = res.data.total
    totalPages.value = res.data.total_pages
    currentPage.value = page
  } catch {
  } finally {
    loading.value = false
  }
}

// 搜索防抖：用户输入后等 300ms 才发起请求，避免每次按键都请求后端
function handleSearch() {
  if (searchTimer) clearTimeout(searchTimer)
  searchTimer = setTimeout(() => {
    fetchPosts(1)
  }, 300)
}

function clearSearch() {
  searchKeyword.value = ''
  fetchPosts(1)
}

function handlePageChange(page: number) {
  fetchPosts(page)
}

function goToCreate() {
  router.push('/create')
}

function goToLogin() {
  router.push('/login')
}

onMounted(() => {
  fetchPosts()
})
</script>

<template>
  <div class="home-view">
    <div class="home-header">
      <h1>最新帖子</h1>
      <div class="header-actions">
        <ElButton v-if="!isGuest" type="primary" @click="goToCreate">发帖</ElButton>
        <ElButton v-else type="primary" @click="goToLogin">登录后发帖</ElButton>
      </div>
    </div>

    <ElAlert
      v-if="isGuest"
      title="您正在以游客身份浏览，登录后可使用点赞、评论、私聊等功能"
      type="info"
      show-icon
      :closable="false"
      style="margin-bottom: 16px"
    />

    <div class="search-bar">
      <ElInput
        v-model="searchKeyword"
        placeholder="搜索帖子标题..."
        clearable
        @input="handleSearch"
        @clear="clearSearch"
      >
        <template #prefix>
          <span>🔍</span>
        </template>
      </ElInput>
    </div>

    <div v-loading="loading">
      <div v-if="posts.length > 0" class="post-list">
        <PostCard v-for="post in posts" :key="post.id" :post="post" />
      </div>
      <ElEmpty v-else-if="!loading" :description="searchKeyword ? '未找到相关帖子' : '暂无帖子，来说点什么吧！'" />
    </div>

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
</template>

<style scoped>
.home-view { max-width: 800px; margin: 0 auto; }
.home-header { display: flex; justify-content: space-between; align-items: center; margin-bottom: 16px; }
.home-header h1 { font-size: 24px; color: #333; }
.header-actions { display: flex; gap: 8px; }
.search-bar { margin-bottom: 20px; }
.post-list { display: flex; flex-direction: column; gap: 16px; }
.pagination { display: flex; justify-content: center; margin-top: 30px; }
</style>
