<script setup lang="ts">
import { ref } from 'vue'
import { useRouter, useRoute } from 'vue-router'
import { ElForm, ElFormItem, ElInput, ElButton, ElMessage } from 'element-plus'
import { userApi } from '@/api/user'
import { useUserStore } from '@/stores/user'

const router = useRouter()
const route = useRoute()
const userStore = useUserStore()

const form = ref({
  username: '',
  password: ''
})
const loading = ref(false)

async function handleLogin() {
  if (!form.value.username || !form.value.password) {
    ElMessage.warning('请输入用户名和密码')
    return
  }

  loading.value = true
  try {
    const res = await userApi.login(form.value.username, form.value.password)
    // 登录成功：将用户信息和 Token 保存到 store
    userStore.login(
      {
        id: res.data.id,
        username: res.data.username,
        nickname: res.data.nickname,
        avatar_url: res.data.avatar_url
      },
      res.data.token
    )
    ElMessage.success('登录成功')
    // 跳转到登录前的页面，如果没有则跳转首页
    const redirect = (route.query.redirect as string) || '/'
    router.push(redirect)
  } catch {
  } finally {
    loading.value = false
  }
}
</script>

<template>
  <div class="login-container">
    <div class="login-card">
      <h2 class="login-title">登录 畅谈</h2>
      <ElForm @submit.prevent="handleLogin">
        <ElFormItem>
          <ElInput
            v-model="form.username"
            placeholder="账号"
            size="large"
          />
        </ElFormItem>
        <ElFormItem>
          <ElInput
            v-model="form.password"
            type="password"
            placeholder="密码"
            size="large"
          />
        </ElFormItem>
        <ElFormItem>
          <ElButton
            type="primary"
            size="large"
            :loading="loading"
            native-type="submit"
            style="width: 100%"
          >
            登录
          </ElButton>
        </ElFormItem>
      </ElForm>
      <div class="login-footer">
        还没有账号？<router-link to="/register">立即注册</router-link>
      </div>
    </div>
  </div>
</template>

<style scoped>
.login-container {
  display: flex;
  justify-content: center;
  align-items: center;
  min-height: calc(100vh - 60px);
}

.login-card {
  width: 400px;
  padding: 40px;
  background: white;
  border-radius: 12px;
  box-shadow: 0 4px 20px rgba(0, 0, 0, 0.1);
}

.login-title {
  text-align: center;
  margin-bottom: 30px;
  color: #333;
}

.login-footer {
  text-align: center;
  margin-top: 20px;
  color: #666;
}

.login-footer a {
  color: #409eff;
}
</style>
