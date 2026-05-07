<script setup lang="ts">
import { ElAvatar } from 'element-plus'
import { computed } from 'vue'

const props = defineProps<{
  src?: string      // 头像图片 URL
  size?: number     // 头像尺寸
  nickname?: string // 昵称（图片加载失败时显示首字母）
}>()

// 头像地址，无有效地址时使用默认头像
const avatarSrc = computed(() => {
  if (props.src) {
    return props.src.startsWith('http') ? props.src : props.src
  }
  return '/default-avatar.png'
})

// 昵称首字母，作为头像加载失败时的兜底显示
const initials = computed(() => {
  if (props.nickname) {
    return props.nickname.charAt(0).toUpperCase()
  }
  return 'U'
})

const avatarSize = computed(() => {
  return props.size || 40
})
</script>

<template>
  <ElAvatar
    :size="avatarSize"
    :src="avatarSrc"
  >
    <!-- 图片加载失败时显示昵称首字母 -->
    {{ initials }}
  </ElAvatar>
</template>
