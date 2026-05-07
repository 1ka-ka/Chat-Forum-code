// 全局类型定义：定义前后端交互的数据结构

// 用户信息（对应后端 User 模型）
export interface User {
  id: number
  username?: string
  nickname: string
  avatar_url: string
  created_at?: string
  post_count?: number  // 发帖数量
}

// 帖子信息（对应后端 Post 模型，包含作者信息用于列表展示）
export interface Post {
  id: number
  user_id: number
  title: string
  content: string
  image_urls: string[]
  like_count: number
  comment_count: number
  is_liked: boolean       // 当前用户是否已点赞
  nickname: string        // 作者昵称（冗余字段，方便展示）
  avatar_url: string      // 作者头像（冗余字段，方便展示）
  created_at: string
}

// 评论信息（支持两级结构：顶级评论 + 回复）
export interface Comment {
  id: number
  post_id: number
  user_id: number
  parent_comment_id: number | null  // 为 null 表示顶级评论，否则是对某评论的回复
  content: string
  nickname: string
  avatar_url: string
  created_at: string
  replies?: Comment[]  // 该评论下的回复列表（前端构建评论树时填充）
}

// 聊天消息
export interface Message {
  id: number
  sender_id: number
  receiver_id: number
  content: string
  is_read: number
  created_at: string
}

// 会话信息（对应后端返回的聊天会话摘要）
export interface Conversation {
  user_id: number
  nickname: string
  avatar_url: string
  last_message: string
  last_time: string
  unread_count: number
}

// 点赞用户信息（查看点赞列表时使用）
export interface LikeUser {
  id: number
  nickname: string
  avatar_url: string
}

// 通知信息（对应后端 Notification 模型）
export interface Notification {
  id: number
  type: string           // 通知类型：like/comment/reply/chat
  post_id: number | null
  comment_id: number | null
  message: string
  is_read: number
  created_at: string
  actor_id: number          // 触发通知的用户 ID
  actor_nickname: string
  actor_avatar_url: string
}

// 后端统一响应格式
export interface ApiResponse<T = any> {
  code: number
  message: string
  data: T
}

// 分页查询结果
export interface PageResult<T> {
  items: T[]
  total: number
  page: number
  page_size: number
  total_pages: number
}

// 登录/注册成功后返回的数据
export interface LoginData {
  id: number
  username: string
  nickname: string
  avatar_url: string
  token: string
}
