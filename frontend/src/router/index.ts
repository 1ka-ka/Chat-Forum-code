import { createRouter, createWebHistory } from 'vue-router'
import { isLoggedIn } from '@/utils/auth'

const router = createRouter({
  // 使用 HTML5 History 模式（URL 无 # 号）
  history: createWebHistory(),
  routes: [
    {
      path: '/login',
      name: 'login',
      // 路由懒加载：只有访问该页面时才下载对应组件，减小首屏体积
      component: () => import('@/views/LoginView.vue'),
      // meta.guest 表示仅游客可访问，已登录用户会被重定向到首页
      meta: { guest: true }
    },
    {
      path: '/register',
      name: 'register',
      component: () => import('@/views/RegisterView.vue'),
      meta: { guest: true }
    },
    {
      path: '/',
      name: 'home',
      component: () => import('@/views/HomeView.vue')
    },
    {
      path: '/post/:id',
      name: 'post-detail',
      component: () => import('@/views/PostDetailView.vue')
    },
    {
      path: '/create',
      name: 'create-post',
      component: () => import('@/views/CreatePostView.vue'),
      // meta.requiresAuth 表示需要登录才能访问
      meta: { requiresAuth: true }
    },
    {
      path: '/profile',
      name: 'profile',
      component: () => import('@/views/ProfileView.vue'),
      meta: { requiresAuth: true }
    },
    {
      path: '/user/:id',
      name: 'user-profile',
      component: () => import('@/views/UserView.vue')
    },
    {
      path: '/chat',
      name: 'chat',
      component: () => import('@/views/ChatView.vue'),
      meta: { requiresAuth: true }
    },
    {
      path: '/chat/:userId',
      name: 'chat-user',
      component: () => import('@/views/ChatView.vue'),
      meta: { requiresAuth: true }
    },
    {
      path: '/notifications',
      name: 'notifications',
      component: () => import('@/views/NotificationsView.vue'),
      meta: { requiresAuth: true }
    }
  ]
})

// 全局路由守卫：在每次跳转前检查权限
router.beforeEach((to, from, next) => {
  if (to.meta.requiresAuth && !isLoggedIn()) {
    // 需要登录但未登录 → 跳转登录页，并记录原始目标路径以便登录后回跳
    next({ name: 'login', query: { redirect: to.fullPath } })
  } else if (to.meta.guest && isLoggedIn()) {
    // 游客专属页面但已登录 → 重定向到首页
    next({ name: 'home' })
  } else {
    next()
  }
})

export default router
