<script setup lang="ts">
import { ref, onMounted, computed } from 'vue'
import { useRoute, useRouter } from 'vue-router'
import { ElButton, ElInput, ElAvatar, ElEmpty, ElMessage, ElDialog, ElAlert } from 'element-plus'
import { postApi } from '@/api/post'
import { commentApi } from '@/api/comment'
import { likeApi } from '@/api/like'
import { useUserStore } from '@/stores/user'
import { isLoggedIn } from '@/utils/auth'
import type { Post, Comment, LikeUser } from '@/types'
import LikeButton from '@/components/LikeButton.vue'
import CollapsibleText from '@/components/CollapsibleText.vue'

const route = useRoute()
const router = useRouter()
const userStore = useUserStore()

const post = ref<Post | null>(null)
const comments = ref<Comment[]>([])
const loading = ref(false)
const commentLoading = ref(false)
const newComment = ref('')
// 当前正在回复的评论，null 表示发表顶级评论
const replyTo = ref<Comment | null>(null)
const commentPage = ref(1)
const commentTotalPages = ref(1)
const likeCount = ref(0)
const isLiked = ref(false)

const likeUsers = ref<LikeUser[]>([])
const showLikeDialog = ref(false)
const likeUsersLoading = ref(false)

const isGuest = computed(() => !userStore.user || !isLoggedIn())

// 将扁平的评论列表构建为树形结构（顶级评论 + 回复）
// 后端返回的评论是扁平的，通过 parent_comment_id 关联父子评论
function buildCommentTree(items: Comment[]): Comment[] {
  const map = new Map<number, Comment>()
  const roots: Comment[] = []
  // 第一遍：将所有评论存入 Map，并初始化 replies 数组
  for (const c of items) {
    c.replies = []
    map.set(c.id, c)
  }
  // 第二遍：将子评论挂载到父评论的 replies 下，没有父评论的作为顶级评论
  for (const c of items) {
    if (c.parent_comment_id && map.has(c.parent_comment_id)) {
      map.get(c.parent_comment_id)!.replies!.push(c)
    } else {
      roots.push(c)
    }
  }
  return roots
}

const commentTree = ref<Comment[]>([])

async function fetchPost() {
  loading.value = true
  try {
    const postId = Number(route.params.id)
    const res = await postApi.getPostById(postId)
    post.value = res.data
    likeCount.value = res.data.like_count
    isLiked.value = res.data.is_liked
  } catch {
    ElMessage.error('帖子不存在')
    router.push('/')
  } finally {
    loading.value = false
  }
}

async function fetchComments(page: number = 1) {
  commentLoading.value = true
  try {
    const postId = Number(route.params.id)
    const res = await commentApi.getComments(postId, page)
    comments.value = res.data.items
    // 将扁平评论列表转为树形结构
    commentTree.value = buildCommentTree(res.data.items)
    commentTotalPages.value = res.data.total_pages
    commentPage.value = page
  } catch {
  } finally {
    commentLoading.value = false
  }
}

async function submitComment() {
  if (!newComment.value.trim()) {
    ElMessage.warning('请输入评论内容')
    return
  }

  try {
    const postId = Number(route.params.id)
    await commentApi.createComment(postId, newComment.value, replyTo.value?.id || null)
    ElMessage.success(replyTo.value ? '回复成功' : '评论成功')
    newComment.value = ''
    replyTo.value = null
    // 评论成功后刷新评论列表和评论计数
    fetchComments(1)
    if (post.value) {
      post.value.comment_count++
    }
  } catch {
  }
}

// 点击"回复"按钮：设置回复目标，并在输入框预填 @昵称
function setReplyTo(comment: Comment) {
  replyTo.value = comment
  newComment.value = `@${comment.nickname} `
}

function cancelReply() {
  replyTo.value = null
  newComment.value = ''
}

async function fetchLikeUsers() {
  if (!post.value) return
  likeUsersLoading.value = true
  try {
    const res = await likeApi.getLikeUsers(post.value.id)
    likeUsers.value = res.data
  } catch {
  } finally {
    likeUsersLoading.value = false
  }
}

function openLikeUsers() {
  showLikeDialog.value = true
  fetchLikeUsers()
}

function goToUser(userId: number) {
  router.push(`/user/${userId}`)
}

function goToChat(userId: number) {
  showLikeDialog.value = false
  router.push(`/chat/${userId}`)
}

function goBack() {
  router.push('/')
}

function requireLogin() {
  ElMessage.warning('请先登录')
  router.push({ path: '/login', query: { redirect: route.fullPath } })
}

onMounted(() => {
  fetchPost()
  fetchComments()
})
</script>

<template>
  <div class="post-detail-view">
    <ElButton text @click="goBack">← 返回主页</ElButton>

    <div v-loading="loading" class="post-content-wrapper">
      <div v-if="post" class="post-content">
        <h1 class="post-title">{{ post.title }}</h1>

        <div class="post-author" @click="goToUser(post.user_id)">
          <ElAvatar :size="48" :src="post.avatar_url || '/default-avatar.png'" />
          <div class="author-info">
            <span class="nickname">{{ post.nickname }}</span>
            <span class="post-time">{{ post.created_at }}</span>
          </div>
          <ElButton
            v-if="!isGuest && post.user_id !== userStore.user?.id"
            size="small"
            type="primary"
            plain
            @click.stop="goToChat(post.user_id)"
          >
            私信
          </ElButton>
        </div>

        <div class="post-body">
          <CollapsibleText :text="post.content" :max-lines="10" />
          <div v-if="post.image_urls && post.image_urls.length > 0" class="post-images">
            <img v-for="(url, index) in post.image_urls" :key="index" :src="url" alt="post image" />
          </div>
        </div>

        <div class="post-actions">
          <template v-if="!isGuest">
            <LikeButton :post-id="post.id" :initial-liked="isLiked" :initial-count="likeCount" @update="likeCount = $event" />
          </template>
          <template v-else>
            <ElButton size="small" @click="requireLogin">👍 点赞（需登录）</ElButton>
          </template>
          <span class="action-item clickable" @click="openLikeUsers">👍 {{ likeCount }} 人点赞</span>
          <span class="action-item">💬 {{ post.comment_count }} 条评论</span>
        </div>

        <div class="comment-section">
          <h3>评论 ({{ post.comment_count }})</h3>

          <template v-if="!isGuest">
            <div v-if="replyTo" class="reply-hint">
              回复 <strong>{{ replyTo.nickname }}</strong>
              <ElButton size="small" text @click="cancelReply">取消</ElButton>
            </div>
            <div class="comment-form">
              <ElInput v-model="newComment" type="textarea" :rows="3" :placeholder="replyTo ? `回复 ${replyTo.nickname}...` : '说点什么...'" />
              <ElButton type="primary" @click="submitComment" :loading="commentLoading">发布</ElButton>
            </div>
          </template>
          <ElAlert v-else title="登录后即可发表评论" type="info" show-icon :closable="false" style="margin-bottom: 16px">
            <template #default>
              <ElButton size="small" type="primary" @click="requireLogin">去登录</ElButton>
            </template>
          </ElAlert>

          <div class="comment-list">
            <div v-for="comment in commentTree" :key="comment.id" class="comment-item">
              <ElAvatar class="comment-avatar" :size="36" :src="comment.avatar_url || '/default-avatar.png'" @click="goToUser(comment.user_id)" />
              <div class="comment-body">
                <div class="comment-header">
                  <span class="comment-author" @click="goToUser(comment.user_id)">{{ comment.nickname }}</span>
                  <span class="comment-time">{{ comment.created_at }}</span>
                  <ElButton v-if="!isGuest" size="small" text type="primary" @click="setReplyTo(comment)">回复</ElButton>
                  <ElButton v-if="!isGuest && comment.user_id !== userStore.user?.id" size="small" text type="primary" @click="goToChat(comment.user_id)">私信</ElButton>
                </div>
                <CollapsibleText :text="comment.content" :max-lines="5" />
                <!-- 渲染该评论下的回复列表 -->
                <div v-if="comment.replies && comment.replies.length > 0" class="replies">
                  <div v-for="reply in comment.replies" :key="reply.id" class="reply-item">
                    <ElAvatar class="reply-avatar" :size="28" :src="reply.avatar_url || '/default-avatar.png'" @click="goToUser(reply.user_id)" />
                    <div class="reply-body">
                      <span class="reply-author" @click="goToUser(reply.user_id)">{{ reply.nickname }}</span>
                      <span class="reply-content">{{ reply.content }}</span>
                      <span class="reply-time">{{ reply.created_at }}</span>
                      <ElButton v-if="!isGuest" size="small" text type="primary" @click="setReplyTo(reply)">回复</ElButton>
                    </div>
                  </div>
                </div>
              </div>
            </div>
            <ElEmpty v-if="commentTree.length === 0 && !commentLoading" description="暂无评论" />
          </div>
        </div>
      </div>
    </div>

    <ElDialog v-model="showLikeDialog" title="点赞用户" width="400px">
      <div v-loading="likeUsersLoading">
        <div v-if="likeUsers.length > 0" class="like-user-list">
          <div v-for="user in likeUsers" :key="user.id" class="like-user-item">
            <ElAvatar :size="36" :src="user.avatar_url || '/default-avatar.png'" />
            <span class="like-user-name" @click="goToUser(user.id)">{{ user.nickname }}</span>
            <ElButton v-if="!isGuest && user.id !== userStore.user?.id" size="small" type="primary" plain @click="goToChat(user.id)">私信</ElButton>
          </div>
        </div>
        <ElEmpty v-else description="暂无点赞" />
      </div>
    </ElDialog>
  </div>
</template>

<style scoped>
.post-detail-view { max-width: 800px; margin: 0 auto; }
.post-content-wrapper { min-height: 200px; }
.post-content { background: white; border-radius: 12px; padding: 30px; margin-top: 20px; }
.post-title { font-size: 28px; color: #333; margin-bottom: 20px; }
.post-author { display: flex; align-items: center; gap: 12px; margin-bottom: 20px; cursor: pointer; }
.author-info { display: flex; flex-direction: column; flex: 1; }
.nickname { font-weight: 500; color: #333; }
.post-time { font-size: 12px; color: #999; }
.post-body { line-height: 1.8; color: #333; margin-bottom: 20px; }
.post-images { display: flex; flex-wrap: wrap; gap: 12px; margin-top: 16px; }
.post-images img { max-width: 300px; border-radius: 8px; }
.post-actions { display: flex; align-items: center; gap: 16px; padding: 16px 0; border-top: 1px solid #f0f0f0; border-bottom: 1px solid #f0f0f0; }
.action-item { color: #666; font-size: 14px; }
.action-item.clickable { cursor: pointer; transition: color 0.2s; }
.action-item.clickable:hover { color: #409eff; }
.comment-section { margin-top: 30px; }
.comment-section h3 { margin-bottom: 16px; color: #333; }
.reply-hint { padding: 8px 12px; background: #f0f9ff; border-radius: 6px; margin-bottom: 12px; font-size: 14px; color: #409eff; }
.comment-form { display: flex; gap: 12px; margin-bottom: 20px; }
.comment-form .el-input { flex: 1; }
.comment-list { background: #fafafa; border-radius: 8px; padding: 16px; }
.comment-item { display: flex; gap: 12px; padding: 12px 0; border-bottom: 1px solid #f0f0f0; }
.comment-item:last-child { border-bottom: none; }
.comment-avatar { cursor: pointer; flex-shrink: 0; }
.comment-body { flex: 1; min-width: 0; }
.comment-header { display: flex; align-items: center; gap: 10px; margin-bottom: 4px; }
.comment-author { font-weight: 500; color: #333; cursor: pointer; }
.comment-author:hover { color: #409eff; }
.comment-time { font-size: 12px; color: #999; }
.replies { margin-top: 8px; padding-left: 12px; border-left: 2px solid #e4e7ed; }
.reply-item { display: flex; gap: 8px; padding: 8px 0; }
.reply-avatar { cursor: pointer; flex-shrink: 0; }
.reply-body { font-size: 13px; line-height: 1.6; }
.reply-author { font-weight: 500; color: #333; cursor: pointer; margin-right: 6px; }
.reply-author:hover { color: #409eff; }
.reply-content { color: #555; }
.reply-time { font-size: 11px; color: #bbb; margin-left: 8px; }
.like-user-list { display: flex; flex-direction: column; gap: 12px; }
.like-user-item { display: flex; align-items: center; gap: 12px; padding: 8px 0; }
.like-user-name { flex: 1; font-weight: 500; color: #333; cursor: pointer; }
.like-user-name:hover { color: #409eff; }
</style>
