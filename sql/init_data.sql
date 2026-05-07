USE chatforum;

-- Clear existing data first
DELETE FROM messages;
DELETE FROM likes;
DELETE FROM comments;
DELETE FROM posts;
DELETE FROM users;

-- 插入测试用户（密码均为 123456 的 bcrypt 哈希）
-- bcrypt hash for "123456": $2a$10$N9qo8uLOickgx2ZMRZoMyeIjZAgcfl7p92ldGxad68LJZdL17lhWy
INSERT INTO users (username, password_hash, nickname, avatar_url) VALUES
  ('alice',   '$2a$10$N9qo8uLOickgx2ZMRZoMyeIjZAgcfl7p92ldGxad68LJZdL17lhWy', 'Alice', ''),
  ('bob',     '$2a$10$N9qo8uLOickgx2ZMRZoMyeIjZAgcfl7p92ldGxad68LJZdL17lhWy', 'Bob',   ''),
  ('charlie', '$2a$10$N9qo8uLOickgx2ZMRZoMyeIjZAgcfl7p92ldGxad68LJZdL17lhWy', 'Charlie',   '');

-- 插入测试帖子
INSERT INTO posts (user_id, title, content, image_urls, like_count, comment_count) VALUES
  (1, 'Welcome to 畅谈!', 'This is the first post. Welcome everyone to communicate and discuss here!', NULL, 3, 2),
  (2, 'C++17 Features Share', 'Recently studying C++17 features including std::optional, std::variant, std::any, structured bindings, if constexpr. Any usage insights?', NULL, 2, 1),
  (1, 'Vue 3 Composition API Practice', 'When using Vue 3 Composition API for projects, found many considerations for ref vs reactive usage. Experience: use ref for primitives, reactive for objects.', NULL, 1, 1),
  (3, 'Drogon Framework Beginner Guide', 'Drogon is a high-performance C++ Web framework supporting HTTP and WebSocket. Its async non-blocking IO model makes handling high concurrency very efficient!', NULL, 2, 0),
  (2, 'MySQL Index Optimization Tips', 'Optimizing DB query performance recently, summarized some index usage tips: 1. Follow leftmost prefix rule 2. Avoid functions on indexed columns 3. Use covering indexes 4. Pay attention to index selectivity', NULL, 1, 1);

-- 插入测试评论
INSERT INTO comments (post_id, user_id, content) VALUES
  (1, 2, 'Great share! This forum looks nice!'),
  (1, 3, 'Looking forward to more features!'),
  (2, 1, 'std::optional is very practical, avoids using pointers for optional values.'),
  (3, 2, 'Thanks for sharing! Learned a lot.'),
  (5, 1, 'Index optimization is indeed important, especially for large datasets.');

-- 插入测试点赞
INSERT INTO likes (post_id, user_id) VALUES
  (1, 2),
  (1, 3),
  (2, 1),
  (2, 3),
  (3, 2),
  (4, 1),
  (4, 2),
  (5, 3);

-- 插入测试私聊消息
INSERT INTO messages (sender_id, receiver_id, content, is_read) VALUES
  (1, 2, 'Hi Bob! What are you busy with recently?', 1),
  (2, 1, 'Hi Alice! Recently studying the Drogon framework.', 1),
  (1, 2, 'Sounds interesting! Can you share some insights?', 0),
  (3, 1, 'Alice, your posts are well written!', 1),
  (1, 3, 'Thanks Charlie!', 0);
