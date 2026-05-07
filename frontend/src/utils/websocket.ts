// WebSocket 管理器：负责建立连接、自动重连、心跳保活、消息分发
import { ref } from 'vue'
import { getToken } from './auth'

type MessageHandler = (data: any) => void

class WebSocketManager {
  private ws: WebSocket | null = null
  private reconnectAttempts = 0
  private maxReconnectAttempts = 10
  private reconnectDelay = 1000
  private pingInterval: number | null = null
  // 消息处理器集合，支持多个组件同时监听 WebSocket 消息
  private messageHandlers: Set<MessageHandler> = new Set()
  private isConnected = ref(false)
  // 标记是否为主动关闭（主动关闭不触发自动重连）
  private intentionalClose = false

  // 建立 WebSocket 连接
  connect() {
    const token = getToken()
    if (!token) return

    // 避免重复连接
    if (this.ws?.readyState === WebSocket.OPEN) return

    this.intentionalClose = false
    // 根据当前页面协议选择 ws 或 wss，Token 通过 URL 参数传递给后端鉴权
    const wsUrl = `${window.location.protocol === 'https:' ? 'wss:' : 'ws:'}//${window.location.host}/ws/chat?token=${token}`
    this.ws = new WebSocket(wsUrl)

    this.ws.onopen = () => {
      this.isConnected.value = true
      this.reconnectAttempts = 0  // 连接成功后重置重连次数
      this.startPing()             // 开始心跳
    }

    this.ws.onmessage = (event) => {
      try {
        const data = JSON.parse(event.data)
        if (data.type === 'pong') return  // 忽略心跳响应
        // 将消息分发给所有注册的处理器
        this.messageHandlers.forEach((handler) => handler(data))
      } catch {
        console.error('Failed to parse WebSocket message')
      }
    }

    this.ws.onclose = () => {
      this.isConnected.value = false
      this.stopPing()
      // 非主动关闭时尝试自动重连
      if (!this.intentionalClose) {
        this.attemptReconnect()
      }
    }

    this.ws.onerror = () => {
      this.isConnected.value = false
    }
  }

  // 自动重连：使用指数退避策略（1s, 2s, 4s, 8s...），避免频繁重连
  private attemptReconnect() {
    if (this.reconnectAttempts < this.maxReconnectAttempts) {
      this.reconnectAttempts++
      const delay = this.reconnectDelay * Math.pow(2, this.reconnectAttempts - 1)
      setTimeout(() => {
        this.connect()
      }, delay)
    }
  }

  // 心跳机制：每 30 秒发送 ping，防止连接因空闲被服务器断开
  private startPing() {
    this.pingInterval = window.setInterval(() => {
      this.send({ type: 'ping' })
    }, 30000)
  }

  private stopPing() {
    if (this.pingInterval) {
      clearInterval(this.pingInterval)
      this.pingInterval = null
    }
  }

  // 发送原始数据到 WebSocket
  send(data: { type: string; receiver_id?: number; content?: string }) {
    if (this.ws?.readyState === WebSocket.OPEN) {
      this.ws.send(JSON.stringify(data))
    }
  }

  // 发送聊天消息的便捷方法
  sendMessage(receiverId: number, content: string) {
    this.send({
      type: 'message',
      receiver_id: receiverId,
      content
    })
  }

  // 注册消息处理器，返回取消注册的函数（方便组件卸载时清理）
  onMessage(handler: MessageHandler) {
    this.messageHandlers.add(handler)
    return () => {
      this.messageHandlers.delete(handler)
    }
  }

  // 主动断开连接（登出时调用）
  disconnect() {
    this.intentionalClose = true
    this.stopPing()
    if (this.ws) {
      this.ws.close()
      this.ws = null
    }
    this.isConnected.value = false
  }

  get connectionState() {
    return this.isConnected
  }
}

// 导出全局单例，整个应用共享同一个 WebSocket 连接
export const wsManager = new WebSocketManager()
