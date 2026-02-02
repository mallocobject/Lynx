# Lynx 🐱

**Lynx** 是一款基于 **C++20** 开发的高性能, 非阻塞, 事件驱动的 **Tcp 服务器框架**.

## ✨ 核心特性
- 🚀 高性能设计: 基于传统的 **Reactor** 模型, 实现高并发连接处理
- 🌐 HTTP 支持: 内置 **有限状态机 (FSM)**, 高效解析 HTTP/1.1 协议
- ⚡ 零拷贝技术: 支持 **Zero-copy** 静态资源分发, 最大化 I/O 性能
- 🛡️ 现代安全: 充分利用 C++20 特性, 确保代码的现代化与内存安全


## 🚀 快速开始

### 环境要求
- GCC 13+
- CMake 3.25+

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
#   -t,             工作线程数 (默认: 系统核心数)
#   -n,             服务器名称 (默认: Lynx-WebServer)

# 示例：在 8080 端口启动服务器
./build/examples/Lynx_WebServer -p 8080 -n Lynx-WebServer
```

成功启动后，将看到类似输出:

```bash
[INFO]2026-02-02 22:04:48.041130 : Server [Lynx-WebServer] starting...
[INFO]2026-02-02 22:04:48.041182 : Listen on 0.0.0.0:8080
[INFO]2026-02-02 22:04:48.041197 : Threads: 1 (Main) + 12 (Workers) + 1 (Logger)
[INFO]2026-02-02 22:04:48.042570 : Server started at 0.0.0.0:8080
```

## 📊 性能基准测试

使用 wrk 进行压力测试

```bash
# 测试命令
wrk -t12 -c400 -d30s --latency http://127.0.0.1:8080/
```

同步日志记录模式

```text
Running 30s test @ http://127.0.0.1:8080/
  12 threads and 400 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency     1.20ms    1.42ms  49.27ms   86.29%
    Req/Sec    37.91k     2.75k   66.42k    80.51%
  Latency Distribution
     50%  735.00us
     75%    1.48ms
     90%    3.11ms
     99%    5.74ms
  13588706 requests in 30.10s, 15.38GB read
Requests/sec: 451414.97
Transfer/sec:    523.06MB
```

异步日志记录模式

```text
Running 30s test @ http://127.0.0.1:8080/
  12 threads and 400 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency     1.37ms    2.03ms  80.38ms   90.06%
    Req/Sec    36.58k    11.96k  231.66k    96.20%
  Latency Distribution
     50%  771.00us
     75%    1.67ms
     90%    3.39ms
     99%    7.99ms
  12899813 requests in 29.57s, 14.60GB read
  Socket errors: connect 0, read 0, write 0, timeout 396
Requests/sec: 436271.58
Transfer/sec:    505.51MB
```
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
│   └── net/
└── test/
```

## 🔧 日志系统配置

Lynx 提供可选的异步日志系统，可通过以下方式配置:

```cpp
// 在应用程序中初始化日志系统
Logger::initAsyncLogging(
    "./log/webserver",           // 日志文件路径
    100 * 1024 * 1024,           // 滚动大小 (100MB)
    3                            // 刷新间隔 (3秒)
);
```


### ✨ *Powered by Lynx — Swift as the Lynx, Steady as the Rock* ✨

───────────────────────────────────────────────

> 本项目旨在深入学习 **C++ 服务端开发**领域...