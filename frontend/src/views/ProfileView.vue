<script setup lang="ts">
import { ref, onMounted } from 'vue'
import { useRouter } from 'vue-router'
import { ElAvatar, ElButton, ElInput, ElMessage } from 'element-plus'
import { useUserStore } from '@/stores/user'

const router = useRouter()
const userStore = useUserStore()

const nickname = ref('')
const avatarFile = ref<File | null>(null)
const avatarPreview = ref('')
const loading = ref(false)

// 选择头像文件后生成本地预览 URL
function handleAvatarChange(e: Event) {
  const target = e.target as HTMLInputElement
  if (target.files && target.files[0]) {
    avatarFile.value = target.files[0]
    avatarPreview.value = URL.createObjectURL(target.files[0])
  }
}

async function handleSave() {
  if (!nickname.value.trim()) {
    ElMessage.warning('请输入用户名')
    return
  }

  loading.value = true
  try {
    // 调用 store 方法更新资料（同时更新后端和本地状态）
    await userStore.updateProfile({
      nickname: nickname.value,
      avatar: avatarFile.value || undefined
    })
    ElMessage.success('更新成功')
  } catch {
  } finally {
    loading.value = false
  }
}

onMounted(async () => {
  // 如果 store 中没有用户信息，先从后端拉取
  if (!userStore.user) {
    await userStore.fetchProfile()
  }
  // 用当前用户信息填充表单
  if (userStore.user) {
    nickname.value = userStore.user.nickname
    avatarPreview.value = userStore.user.avatar_url
  }
})
</script>

<template>
  <div class="profile-view">
    <ElButton text @click="router.back()">← 返回</ElButton>
    <div class="profile-card">
      <h1>个人中心</h1>

      <div v-loading="loading" class="profile-content">
        <div class="avatar-section">
          <ElAvatar :size="100" :src="avatarPreview || '/default-avatar.png'" />
          <div class="avatar-upload">
            <input type="file" accept="image/*" @change="handleAvatarChange" />
            <span>点击更换头像</span>
          </div>
        </div>

        <div class="form-section">
          <div class="form-item">
            <label>账号</label>
            <span class="readonly-value">{{ userStore.user?.username }}</span>
            <span class="hint">账号不可修改</span>
          </div>

          <div class="form-item">
            <label>用户名</label>
            <ElInput v-model="nickname" placeholder="请输入用户名" />
            <span class="hint">用户名唯一，可修改</span>
          </div>

          <div class="form-item">
            <label>注册时间</label>
            <span class="readonly-value">{{ userStore.user?.created_at || '-' }}</span>
          </div>
        </div>

        <div class="actions">
          <ElButton type="primary" :loading="loading" @click="handleSave">保存修改</ElButton>
        </div>
      </div>
    </div>
  </div>
</template>

<style scoped>
.profile-view { max-width: 600px; margin: 0 auto; }
.profile-card { background: white; border-radius: 12px; padding: 30px; margin-top: 10px; }
.profile-card h1 { text-align: center; margin-bottom: 30px; color: #333; }
.profile-content { min-height: 200px; }
.avatar-section { display: flex; flex-direction: column; align-items: center; gap: 16px; margin-bottom: 30px; }
.avatar-upload { display: flex; flex-direction: column; align-items: center; gap: 4px; cursor: pointer; }
.avatar-upload input { width: 80px; opacity: 0; cursor: pointer; }
.avatar-upload span { font-size: 12px; color: #409eff; }
.form-section { display: flex; flex-direction: column; gap: 20px; margin-bottom: 30px; }
.form-item { display: flex; align-items: center; gap: 12px; }
.form-item label { width: 80px; color: #666; flex-shrink: 0; }
.form-item .el-input { flex: 1; }
.readonly-value { color: #333; flex: 1; }
.hint { font-size: 12px; color: #999; flex-shrink: 0; }
.actions { display: flex; justify-content: center; }
</style>
