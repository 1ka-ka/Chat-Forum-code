<script setup lang="ts">
import { ref } from 'vue'
import { ElUpload, ElMessage } from 'element-plus'
import type { UploadFile } from 'element-plus'

const props = defineProps<{
  limit?: number    // 最大文件数量
  maxSize?: number  // 单个文件最大大小（MB）
}>()

const emit = defineEmits<{
  (e: 'update:files', files: File[]): void
}>()

const fileList = ref<File[]>([])

// 生成本地预览 URL
function getObjectURL(file: File): string {
  return URL.createObjectURL(file)
}

function handleExceed() {
  ElMessage.warning(`最多只能上传 ${props.limit || 9} 个文件`)
}

// 文件选择变化时的处理：过滤无效文件并通知父组件
function onChange(uploadFile: UploadFile, uploadFiles: any[]) {
  const validFiles = uploadFiles
    .filter((f) => f.status === 'ready')
    .map((f) => f.raw as File)
    .filter((f) => {
      // 检查文件大小是否超过限制
      if (props.maxSize && f.size > props.maxSize * 1024 * 1024) {
        ElMessage.warning(`文件 ${f.name} 大小超过限制`)
        return false
      }
      return true
    })
  fileList.value = validFiles
  emit('update:files', validFiles)
}

// 移除已选文件
function removeFile(index: number) {
  fileList.value.splice(index, 1)
  emit('update:files', fileList.value)
}
</script>

<template>
  <div class="image-upload">
    <!-- auto-upload=false 表示不自动上传，由父组件在提交时统一上传 -->
    <ElUpload
      action="#"
      :auto-upload="false"
      :limit="limit || 9"
      :on-exceed="handleExceed"
      :on-change="onChange"
      :file-list="[]"
      multiple
      accept="image/jpeg,image/png,image/gif"
    >
      <template #trigger>
        <div class="upload-trigger">
          <span class="upload-icon">+</span>
          <span class="upload-text">添加图片</span>
        </div>
      </template>
    </ElUpload>

    <!-- 图片预览列表 -->
    <div v-if="fileList.length > 0" class="preview-list">
      <div v-for="(file, index) in fileList" :key="index" class="preview-item">
        <img :src="getObjectURL(file)" alt="preview" />
        <span class="remove-btn" @click="removeFile(index)">×</span>
      </div>
    </div>
  </div>
</template>

<style scoped>
.image-upload {
  width: 100%;
}

.upload-trigger {
  width: 100px;
  height: 100px;
  border: 2px dashed #ddd;
  border-radius: 8px;
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  cursor: pointer;
  transition: border-color 0.3s;
}

.upload-trigger:hover {
  border-color: #409eff;
}

.upload-icon {
  font-size: 32px;
  color: #999;
}

.upload-text {
  font-size: 12px;
  color: #999;
  margin-top: 4px;
}

.preview-list {
  display: flex;
  flex-wrap: wrap;
  gap: 10px;
  margin-top: 10px;
}

.preview-item {
  position: relative;
  width: 100px;
  height: 100px;
}

.preview-item img {
  width: 100%;
  height: 100%;
  object-fit: cover;
  border-radius: 8px;
}

.remove-btn {
  position: absolute;
  top: -8px;
  right: -8px;
  width: 20px;
  height: 20px;
  background: #f56c6c;
  color: white;
  border-radius: 50%;
  display: flex;
  align-items: center;
  justify-content: center;
  cursor: pointer;
  font-size: 14px;
}
</style>
