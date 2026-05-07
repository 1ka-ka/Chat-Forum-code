import { defineConfig } from 'vite'
import vue from '@vitejs/plugin-vue'
import { resolve } from 'path'

export default defineConfig({
  plugins: [vue()],
  resolve: {
    alias: {
      // 路径别名：用 @ 代替 src 目录，方便导入
      '@': resolve(__dirname, 'src')
    }
  },
  server: {
    port: 5173,
    proxy: {
      // 将前端 /api 请求代理到后端服务器，解决开发环境跨域问题
      '/api': {
        target: 'http://localhost:8080',
        changeOrigin: true
      },
      // WebSocket 代理，支持实时通信
      '/ws': {
        target: 'ws://localhost:8080',
        ws: true
      },
      // 图片等静态资源代理
      '/uploads': {
        target: 'http://localhost:8080',
        changeOrigin: true
      }
    }
  }
})
