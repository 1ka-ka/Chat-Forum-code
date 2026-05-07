// Vue 应用入口文件：创建应用实例并安装插件
import { createApp } from 'vue'
import { createPinia } from 'pinia'
import ElementPlus from 'element-plus'
import 'element-plus/dist/index.css'
import App from './App.vue'
import router from './router'
import './styles/global.css'

// 创建 Vue 应用实例
const app = createApp(App)

// 安装 Pinia 状态管理（用于全局共享数据，如用户信息、通知等）
app.use(createPinia())
// 安装路由（控制页面跳转）
app.use(router)
// 安装 Element Plus 组件库（提供按钮、表单、弹窗等 UI 组件）
app.use(ElementPlus)

// 将应用挂载到 index.html 中 id 为 app 的元素上
app.mount('#app')
