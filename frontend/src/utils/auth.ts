// Token 管理工具：将 JWT Token 存储在 localStorage 中

const TOKEN_KEY = 'changtan_token'

// 获取 Token
export function getToken(): string | null {
  return localStorage.getItem(TOKEN_KEY)
}

// 保存 Token（登录成功后调用）
export function setToken(token: string): void {
  localStorage.setItem(TOKEN_KEY, token)
}

// 删除 Token（登出或 Token 过期时调用）
export function removeToken(): void {
  localStorage.removeItem(TOKEN_KEY)
}

// 判断是否已登录（通过 Token 是否存在来判断）
export function isLoggedIn(): boolean {
  return !!getToken()
}
