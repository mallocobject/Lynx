# Lynx 🐱

**Lynx** 是一个轻量级 C++ 基础库，提供了包括 HTTP、JSON、日志、SQL、TCP、时间等常用模块。本库基于 **C++20 标准**开发，旨在为服务器端应用提供便捷的工具集合。

## 🚀 快速开始

### 环境要求
- **Compiler**: GCC 13+ (推荐 13.3.0)
- **Build System**: CMake 3.25+
- **OS**: Ubuntu 24.04 LTS 或其他 Linux 发行版
- **可选依赖**：若使用 SQL 模块，需安装 MySQL Connector/C++ 开发库：
```bash
sudo apt update
sudo apt install libmysqlcppconn-dev
```

### 编译库

#### 1. 克隆仓库
```bash
git clone https://github.com/mallocobject/Lynx.git
cd Lynx
```

#### 2. 配置并编译（Debug 版本）
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```
> 注意：如果使用 SQL 模块，请确保已安装 libmysqlcppconn-dev，否则编译会报错。

#### 3. 安装库文件和头文件
```bash
sudo cmake --install build
```

**安装位置：**
- 静态库: `/usr/local/lib/liblynx_lib_d.a`
- 公开头文件: `/usr/local/include/lynx/`

#### 4. 验证安装
```bash
ls /usr/local/lib/liblynx_lib_d.a
ls /usr/local/include/lynx/
```

---

## 🔧 日志系统配置

Lynx 提供高性能的异步日志系统，支持两层配置：

### 1️⃣ 编译期级别过滤（CMakeLists.txt）

编译库时设置最小日志级别，低于此级别的日志代码会被编译器优化掉：

```cmake
# 设置日志过滤级别
# 可选值: TRACE, DEBUG, INFO, WARN, ERROR, FATAL

set(LOG_LEVEL "INFO" CACHE STRING "Logger level")

# 默认示例（生产推荐）
set(LOG_LEVEL "INFO")

# 开发调试
# set(LOG_LEVEL "DEBUG")

# 性能测试（极致性能）
# set(LOG_LEVEL "OFF")
```

**编译命令：**
```bash
# 默认 INFO 级别
cmake -B build

# 或指定 DEBUG 级别
cmake -B build -DLOG_LEVEL=DEBUG

# 性能测试
cmake -B build -DLOG_LEVEL=OFF
```

### 2️⃣ 运行时日志初始化（main.cpp）

在应用程序启动时初始化异步日志系统：

```cpp
#include "lynx/logger.hpp"

int main() {
    // 1. 初始化异步日志系统
    lynx::Logger::initAsyncLogging(
        "logs/",             // 日志文件存放目录（需提前创建）
        "my_server",         // 日志文件名前缀
        100 * 1024 * 1024,   // 单个日志文件滚动大小 (100MB)
        3                    // 后端定期刷盘间隔 (3秒)
    );

    LOG_INFO("Lynx Server Started");

    // 业务逻辑代码
    // ...

    // 2. 程序退出前关闭日志系统
    lynx::Logger::shutdownAsyncLogging();
    
    return 0;
}
```

## 📚 使用示例

### 示例 1: Echo 服务器

简单的 Echo 服务器，接收客户端消息并原样返回。

**文件：** `examples/echo_server/main.cpp`

```cpp
#include <lynx/lynx.hpp>

using namespace lynx;

int main(int argc, char* argv[])
{
	// 初始化日志
	logger::Logger::initAsyncLogging(LYNX_WEB_SRC_DIR "/logs/", argv[0]);

	// 创建 TCP 服务器
	tcp::EventLoop loop;

	tcp::Server server(&loop, "0.0.0.0", 9999, "EchoServer", 8);

	// 设置连接回调
	server.setConnectionCallback(
		[](const std::shared_ptr<tcp::Connection>& conn)
		{
			if (conn->connected())
			{
				LOG_INFO << "Client connected: "
						 << conn->addr().toFormattedString();
			}
			if (conn->disconnected())
			{
				LOG_INFO << "Client disconnected: "
						 << conn->addr().toFormattedString();
			}
		});

	// 设置消息回调（Echo 逻辑）
	server.setMessageCallback(
		[](const std::shared_ptr<tcp::Connection>& conn, tcp::Buffer* buf)
		{
			std::string msg = buf->retrieveString(buf->readableBytes());
			conn->send(msg); // 原样返回
			LOG_INFO << "Echo: " << msg;
		});

	LOG_INFO << "Echo Server listening on 0.0.0.0:9999";
	server.run();
	loop.run();

	logger::Logger::shutdownAsyncLogging();
	return 0;
}
```

**编译和运行：**
```bash
cd examples/echo_server
cmake -B build
cmake --build build
./build/Lynx_EchoServer
```

**测试：**
```bash
# 另一个终端
nc 127.0.0.1 9999
# 输入任意内容，会被原样返回
```

---

### 示例 2: HTTP Web 服务器

完整的 HTTP 服务器，支持路由、静态文件和动态内容。

**文件：** `examples/http_server/main.cpp`

```cpp
#include <lynx/lynx.hpp>

using namespace lynx;

int main(int argc, char* argv[])
{
	// 初始化日志
	logger::Logger::initAsyncLogging(LYNX_WEB_SRC_DIR "/logs/", "http_server");

	// 创建 HTTP 服务器
	tcp::EventLoop loop;
	tcp::Server server(&loop, "0.0.0.0", 8080, "Lynx-WebServer", 8);

	// 创建路由器
	auto router = http::Router();

	// 注册路由
	router.addRoute("GET", "/",
					[](const auto& req, auto* res, const auto& conn)
					{
						http::Router::sendFile(conn, res,
											   LYNX_WEB_SRC_DIR
											   "/templates/index.html");
					});

	// 注册 CSS 路由
	router.addRoute("GET", "/static/css/style.css",
					[](const auto& req, auto* res, const auto& conn)
					{
						http::Router::sendFile(conn, res,
											   LYNX_WEB_SRC_DIR
											   "/static/css/style.css");
					});

	// 注册 JS 路由
	router.addRoute("GET", "/static/js/script.js",
					[](const auto& req, auto* res, const auto& conn)
					{
						http::Router::sendFile(
							conn, res, LYNX_WEB_SRC_DIR "/static/js/script.js");
					});

	router.addRoute(
		"POST", "/calculate",
		[](const auto& req, auto* res, const auto& conn)
		{
			try
			{
				json::Tokenizer tokenizer(req.body);
				json::Ref root = json::Parser(&tokenizer).parse();

				double sum = 0.0;

				auto a = root["a"];
				auto b = root["b"];

				if (a.getShared()->isValue() &&
					a.getShared()->asValue()->isFloat())
				{
					sum += a.asFloat();
				}
				else if (a.getShared()->isValue() &&
						 a.getShared()->asValue()->isInt())
				{
					sum += a.asInt();
				}

				if (b.getShared()->isValue() &&
					b.getShared()->asValue()->isFloat())
				{
					sum += b.asFloat();
				}
				else if (b.getShared()->isValue() &&
						 b.getShared()->asValue()->isInt())
				{
					sum += b.asInt();
				}

				json::Ref result =
					json::make_object({{"sum", json::make_value(sum)}});

				res->setStatusCode(200);
				res->setContentType("application/json");

				res->setBody(result.serialize());

				conn->send(res->toFormattedString());
			}

			catch (const std::exception& e)
			{
				res->setStatusCode(400);
				res->setContentType("application/json");
				res->setBody(std::format("{{\"error\": \"{}\"}}", e.what()));

				conn->send(res->toFormattedString());
			}
		});

	// 设置连接回调
	server.setConnectionCallback(
		[](const std::shared_ptr<tcp::Connection>& conn)
		{
			if (conn->connected())
			{
				LOG_INFO << "Client connected: "
						 << conn->addr().toFormattedString();
				// 每个连接绑定一个 HttpContext 实例 (基于 std::any)
				conn->setContext(std::make_shared<http::Context>());
			}
			else if (conn->disconnected())
			{
				LOG_INFO << "Client disconnected: "
						 << conn->addr().toFormattedString();
			}
		});

	// 设置 HTTP 处理回调
	server.setMessageCallback(
		[&router](const std::shared_ptr<tcp::Connection>& conn,
				  tcp::Buffer* buf)
		{
			auto context =
				std::any_cast<std::shared_ptr<http::Context>>(conn->context());

			if (!context->parser(buf))
			{
				conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
				conn->shutdown();
				return;
			}

			if (context->completed())
			{
				const http::Request& req = context->req();
				http::Response res;

				router.dispatch(req, &res, conn);

				std::string conn_header = req.header("connection");
				if (conn_header == "close" ||
					(req.version == "HTTP/1.0" && conn_header != "keep-alive"))
				{
					conn->shutdown();
				}
				else
				{
					context->clear();
				}
			}
		});

	LOG_INFO << "HTTP Server listening on 0.0.0.0:8080";
	server.run();
	loop.run();

	logger::Logger::shutdownAsyncLogging();
	return 0;
}
```

**编译和运行：**
```bash
cd examples/http_server
cmake -B build
cmake --build build
./build/Lynx_WebServer
```

**测试：**

```
http://127.0.0.1:8080/
```

直接在浏览器地址栏输入上述 URL，即可访问服务器主页

---

## 🔧 高级配置

### 编译 Release 版本（生产环境）

```bash
cmake -B build_release \
    -DCMAKE_BUILD_TYPE=Release \
    -DLOG_LEVEL=INFO
cmake --build build_release -j 16
sudo cmake --install build_release --config=Release
```

### 设置自定义日志目录

```bash
mkdir -p /var/log/lynx
sudo chown $USER:$USER /var/log/lynx
```

然后在 `main.cpp` 中：
```cpp
Logger::initAsyncLogging("/var/log/lynx/", "my_app");
```

---

## ✨ *Powered by Lynx — Swift as the Lynx, Steady as the Rock* ✨

───────────────────────────────────────────────

> 本项目旨在深入学习 **C++ 服务端开发**，实现高性能网络编程最佳实践