<script setup lang="ts">
import { ref, computed } from 'vue'

const props = defineProps<{
  text: string
  maxLines?: number  // 折叠时显示的最大行数
}>()

const expanded = ref(false)

const maxLines = computed(() => props.maxLines || 5)

// 判断文本是否过长需要折叠（按行数或字符数判断）
const isLong = computed(() => {
  const lines = props.text.split('\n')
  return lines.length > maxLines.value || props.text.length > 300
})
</script>

<template>
  <div class="collapsible-text">
    <!-- 折叠状态时通过 max-height 限制显示高度 -->
    <div
      class="text-content"
      :class="{ collapsed: !expanded && isLong }"
      :style="{ '--max-lines': maxLines }"
    >
      <p style="white-space: pre-wrap">{{ text }}</p>
    </div>
    <!-- 长文本显示展开/收起按钮 -->
    <div v-if="isLong" class="expand-btn" @click="expanded = !expanded">
      {{ expanded ? '收起' : '展开全文' }}
      <span class="arrow" :class="{ up: expanded }">▼</span>
    </div>
  </div>
</template>

<style scoped>
.collapsible-text {
  width: 100%;
}

.text-content {
  position: relative;
  transition: max-height 0.3s ease;
}

.text-content.collapsed {
  max-height: calc(var(--max-lines) * 1.8em + 40px);
  overflow: hidden;
}

/* 折叠时底部渐变遮罩，暗示还有更多内容 */
.text-content.collapsed::after {
  content: '';
  position: absolute;
  bottom: 0;
  left: 0;
  right: 0;
  height: 60px;
  background: linear-gradient(transparent, white);
  pointer-events: none;
}

.expand-btn {
  display: inline-block;
  color: #409eff;
  cursor: pointer;
  font-size: 14px;
  margin-top: 4px;
  user-select: none;
}

.expand-btn:hover {
  color: #66b1ff;
}

.arrow {
  display: inline-block;
  font-size: 12px;
  transition: transform 0.3s;
  margin-left: 2px;
}

.arrow.up {
  transform: rotate(180deg);
}
</style>
