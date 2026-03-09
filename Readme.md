# Lynx 🐱

<p align="center"> <img src="https://img.shields.io/badge/C%2B%2B-20-blue.svg" alt="C++20"> <img src="https://img.shields.io/badge/platform-Linux-red.svg" alt="Platform"> <img src="https://img.shields.io/badge/build-CMake-blueviolet.svg" alt="CMake"> <img src="https://img.shields.io/badge/version-1.0.0-orange.svg" alt="Version"> </p><p align="center"> <b>Swift as the Lynx, Steady as the Rock</b><br> 一个基于 C++20 的高性能轻量级基础库，专为服务端应用打造。 </p>

## ✨ 特性一览
- ⚡ **高性能网络模块**：基于 epoll 的多线程 TCP 服务器，支持非阻塞 I/O 和零拷贝（sendfile）。

- 🌐 **HTTP 服务器**：内置轻量级 HTTP 解析器与路由器，轻松构建 REST API 或静态文件服务。

- 📝 **异步日志系统**：两级日志过滤（编译期 + 运行时），高效异步写入，支持滚动文件。

- 🔢 **JSON 解析器**：简洁的 DOM 风格 JSON 库，快速解析与生成。

- 🗄️ **SQL 模块**：基于 MySQL Connector/C++ 的封装（可选），简化数据库操作。

- ⏱️ **时间与工具**：日期时间、内存池、缓冲区等常用组件开箱即用。

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

```bash
# 1. 克隆仓库
git clone https://github.com/mallocobject/Lynx.git
cd Lynx

# 2. 配置 Debug 版本
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DLOG_LEVEL=INFO

# 3. 编译
cmake --build build

# 4. 安装（默认安装到 /usr/local）
sudo cmake --install build
```

**安装位置：**
- 静态库: `/usr/local/lib/liblynx_lib_d.a`
- 头文件: `/usr/local/include/lynx/`

---

## 🔧 日志系统配置

Lynx 提供高性能的异步日志系统，支持两层配置：

### 1️⃣ 编译期级别过滤（CMake）

在项目根目录执行 CMake 配置时，通过 `-DLOG_LEVEL=<级别>` 指定

```bash
# 默认 INFO 级别（生产环境）
cmake -B build

# 开发调试使用 DEBUG
cmake -B build -DLOG_LEVEL=DEBUG

# 性能压测关闭日志
cmake -B build -DLOG_LEVEL=OFF
```

也可以在 `CMakeLists.txt` 中直接修改默认值（不推荐，除非项目有固定需求）：

```cmake
set(LOG_LEVEL "INFO" CACHE STRING "Logger level" FORCE)
```

### 2️⃣ 运行时日志初始化（main.cpp）

在应用程序启动时初始化异步日志系统：

```cpp
#include "lynx/logger.hpp"

int main() {
    // 初始化异步日志
    lynx::Logger::initAsyncLogging(
        "logs/",             // 日志目录
        "my_server",         // 文件名前缀
        100 * 1024 * 1024,   // 单个文件滚动大小（100MB）
        3                    // 刷盘间隔（秒）
    );

    LOG_INFO("Lynx server started");

    // ... 业务代码 ...

    lynx::Logger::shutdownAsyncLogging();
    return 0;
}
```

## 📚 使用示例

### 示例 1: Echo 服务器

一个简单的 TCP Echo 服务器，接收客户端消息并原样返回。

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

**编译运行**：
```bash
cd examples/echo_server
cmake -B build
cmake --build build
./build/Lynx_EchoServer
```

测试：nc 127.0.0.1 9999

---

### 示例 2: HTTP Web 服务器

一个完整的 HTTP 服务器，支持路由、静态文件和 JSON API

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

访问 http://127.0.0.1:8080/ 即可看到主页。

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

然后在代码中：
```cpp
Logger::initAsyncLogging("/var/log/lynx/", "my_app");
```

## 🌟 致谢
- [muduo](https://github.com/chenshuo/muduo)