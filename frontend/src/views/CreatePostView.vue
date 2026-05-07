<script setup lang="ts">
import { ref } from 'vue'
import { useRouter } from 'vue-router'
import { ElForm, ElFormItem, ElInput, ElButton, ElMessage } from 'element-plus'
import { postApi } from '@/api/post'
import ImageUpload from '@/components/ImageUpload.vue'

const router = useRouter()

const title = ref('')
const content = ref('')
const images = ref<File[]>([])
const loading = ref(false)

async function handleSubmit() {
  if (!title.value.trim()) {
    ElMessage.warning('请输入标题')
    return
  }
  if (!content.value.trim()) {
    ElMessage.warning('请输入内容')
    return
  }

  loading.value = true
  try {
    // 发帖使用 FormData 格式，因为可能包含图片文件
    const res = await postApi.createPost({
      title: title.value,
      content: content.value,
      images: images.value
    })
    ElMessage.success('发帖成功')
    router.push('/')
  } catch {
    // 错误已由请求拦截器统一处理
  } finally {
    loading.value = false
  }
}

function onImagesUpdate(files: File[]) {
  images.value = files
}
</script>

<template>
  <div class="create-post-view">
    <ElButton text @click="router.push('/')">← 返回主页</ElButton>
    <div class="form-container">
      <h1>发布帖子</h1>

      <ElForm @submit.prevent="handleSubmit">
        <ElFormItem label="标题">
          <ElInput
            v-model="title"
            placeholder="请输入帖子标题"
            maxlength="200"
            show-word-limit
          />
        </ElFormItem>

        <ElFormItem label="内容">
          <ElInput
            v-model="content"
            type="textarea"
            :rows="10"
            placeholder="说点什么..."
          />
        </ElFormItem>

        <ElFormItem label="图片（可选，最多9张）">
          <ImageUpload
            :limit="9"
            :max-size="5"
            @update:files="onImagesUpdate"
          />
        </ElFormItem>

        <ElFormItem>
          <ElButton @click="router.back()">取消</ElButton>
          <ElButton type="primary" :loading="loading" native-type="submit">
            发布
          </ElButton>
        </ElFormItem>
      </ElForm>
    </div>
  </div>
</template>

<style scoped>
.create-post-view {
  max-width: 800px;
  margin: 0 auto;
}

.form-container {
  background: white;
  border-radius: 12px;
  padding: 30px;
}

.form-container h1 {
  margin-bottom: 24px;
  color: #333;
}
</style>
