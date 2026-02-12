# Lynx 🐱

**Lynx** 是一款基于 **C++20** 开发的高性能, 非阻塞, 事件驱动的 **Tcp 服务器框架**.

## ✨ 核心特性
- 🚀 **高并发架构**: 采用 **Multi-Reactor** 多线程模型与 **Epoll (LT)** 水平触发模式，非阻塞 I/O 实现高并发连接处理
- ⚡ **极致 I/O 性能**: 支持 **Zero-copy** 静态资源分发, 最大化 I/O 性能
- 🌐 **HTTP 支持**: 内置 **有限状态机 (FSM)**, 高效解析 HTTP/1.1 协议
- 📝 **低延迟日志**: 集成 [Elogger](https://github.com/mallocobject/Elogger) **异步日志系统**, 采用双缓冲技术


## 🚀 快速开始

### 环境要求
- Compiler: GCC 13+ (Specifically 13.3.0)
- Build System: CMake 3.25+
- OS: Ubuntu 24.04 LTS

### 编译安装
```bash
# 1. 克隆仓库
git clone https://github.com/mallocobject/Lynx.git
cd Lynx

# 2. 配置并编译
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## 🔍 启动 Web 服务器示例

```bash
# 必需参数
#   -p,             监听端口 (必需)
# 可选参数  
#   -i,             绑定 IP 地址 (默认: 0.0.0.0)
#   -t,             工作线程数 (默认: CPU 数量 - 2)
#   -n,             服务器名称 (默认: Lynx-WebServer)

# 示例：在 8080 端口启动服务器
./build/examples/Lynx_WebServer -p 8080 -n Lynx-WebServer
```

<!-- ## 📊 性能基准测试

使用 wrk 进行压力测试

```bash
# 测试命令
wrk -t12 -c400 -d30s --latency http://127.0.0.1:8080/
```

无日志输出

```text
Running 30s test @ http://127.0.0.1:8080/
  12 threads and 400 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency     1.08ms    1.13ms  28.09ms   87.97%
    Req/Sec    37.94k     5.47k   67.44k    67.94%
  Latency Distribution
     50%  636.00us
     75%    1.21ms
     90%    2.53ms
     99%    5.28ms
  13617611 requests in 30.08s, 15.41GB read
Requests/sec: 452719.63
Transfer/sec:    524.57MB
```

异步日志输出 (仅记录 INFO 等级以上信息)

```text
Running 30s test @ http://127.0.0.1:8080/
  12 threads and 400 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency     1.07ms    1.09ms  21.99ms   87.66%
    Req/Sec    37.83k     4.94k   97.10k    70.09%
  Latency Distribution
     50%  626.00us
     75%    1.22ms
     90%    2.52ms
     99%    5.24ms
  13570294 requests in 30.10s, 15.36GB read
Requests/sec: 450856.01
Transfer/sec:    522.41MB
``` -->

## 📁 项目结构

```text
Lynx
├── CMakeLists.txt
├── examples
│   ├── CMakeLists.txt
│   ├── handlers.cpp
│   ├── handlers.h
│   ├── http_app.h
│   ├── http_base.hpp
|   ├── logs
│   ├── main.cpp
│   ├── static
│   │   ├── css
│   │   │   └── style.css
│   │   └── js
│   │       └── script.js
│   └── templates
│       ├── hello.html
│       └── index.html
├── logs
├── lynx
│   ├── CMakeLists.txt
│   ├── base/
│   ├── http/
│   ├── logger/
│   └── tcp/
└── test/
...
```

## 🔧 日志系统配置

Lynx 提供高性能的异步日志系统, 支持 **编译期级别** 过滤（零开销）与 **运行时异步刷盘** 配置

### 1. 编译期配置 (CMakeLists.txt)

```CMake
# 设置日志过滤级别
# 可选值: LYNX_TRACE, LYNX_DEBUG, LYNX_INFO, LYNX_WARN, LYNX_ERROR, LYNX_FATAL, LYNX_OFF

# 示例：生产环境推荐 (仅保留 INFO 及以上)
set(LOG_LEVEL "LYNX_INFO")

# 示例：极致性能压测 (完全关闭日志)
# set(LOG_LEVEL "LYNX_OFF")

message(STATUS "Current logger level: ${LOG_LEVEL}")
add_definitions(-DLOGGER_LEVEL_SETTING=${LOG_LEVEL})
```
### 2. 运行时初始化 (main.cpp)

在应用程序启动时初始化异步日志后端, 配置日志文件的存储路径和滚动策略

```cpp
#include "lynx/logger/logger.h"

int main() {
    // 1. 初始化异步日志系统
    lynx::Logger::initAsyncLogging(
        "logs/",             // 日志文件存放目录 (请确保目录存在)
        "lynx_server",       // 日志文件名前缀
        100 * 1024 * 1024,   // 单个日志文件滚动大小 (100MB)
        3                    // 缓冲区后端定期刷盘间隔 (3秒)
    );

    LOG_INFO << "Lynx Async Logging started.";

    /*
     * 业务逻辑 / 服务器启动代码
     * ...
     */

    // 2. 程序退出前关闭日志系统
    // 确保所有缓冲区内的日志都被刷新到磁盘
    lynx::Logger::shutdownAsyncLogging();
    
    return 0;
}
```


### ✨ *Powered by Lynx — Swift as the Lynx, Steady as the Rock* ✨

───────────────────────────────────────────────

> 本项目旨在深入学习 **C++ 服务端开发** ...