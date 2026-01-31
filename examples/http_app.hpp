#ifndef HTTP_APP_H
#define HTTP_APP_H

#include "lynx/base/logger.hpp"
#include "lynx/http/http_context.h"
#include "lynx/http/http_request.hpp"
#include "lynx/http/http_response.h"
#include "lynx/http/http_router.h"
#include "lynx/net/buffer.h"
#include "lynx/net/event_loop.h"
#include "lynx/net/tcp_connection.h"
#include "lynx/net/tcp_server.h"
#include <any>
#include <cstdint>
#include <functional>
#include <string>
class HttpApp
{
  private:
	lynx::TcpServer server_;
	lynx::HttpRouter router_;

  public:
	HttpApp(lynx::EventLoop* loop, const std::string& ip, uint16_t port,
			const std::string& name,
			size_t sub_reactor_num = std::thread::hardware_concurrency())
		: server_(loop, ip.c_str(), port, name, sub_reactor_num)
	{
		server_.setConnectionCallback(
			std::bind(&HttpApp::onConnection, this, std::placeholders::_1));
		server_.setMessageCallback(std::bind(&HttpApp::onMessage, this,
											 std::placeholders::_1,
											 std::placeholders::_2));
	}

	void addRoute(const std::string& method, const std::string& path,
				  const lynx::HttpRouter::http_handler& handler)
	{
		router_.addRoute(method, path, handler);
	}

	void startup()
	{
		server_.startup();
	}

  private:
	void onConnection(const std::shared_ptr<lynx::TcpConnection>& conn)
	{
		if (conn->connected())
		{
			// 每个连接绑定一个 HttpContext 实例 (基于 std::any)
			conn->setContext(std::make_shared<lynx::HttpContext>());
			lynx::LOG_DEBUG << "New Connection: " << conn->name();
		}
		else
		{
			lynx::LOG_DEBUG << "Connection closed: " << conn->name();
		}
	}

	void onMessage(const std::shared_ptr<lynx::TcpConnection>& conn,
				   std::shared_ptr<lynx::Buffer> buf)
	{
		auto context =
			std::any_cast<std::shared_ptr<lynx::HttpContext>>(conn->context());

		if (!context->parserBuffer(buf.get()))
		{
			conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
			conn->shutdown();
			return;
		}

		if (context->completed())
		{
			const lynx::HttpRequest& req = context->request();
			lynx::HttpResponse res;

			router_.dispatch(req, &res, conn);

			std::string conn_header = req.header("connection");
			if (conn_header == "close" ||
				(req.version == "HTTP/1.0" && conn_header != "keep-alive"))
			{
				conn->shutdown();
			}
			else
			{
				context->lynx::HttpContext::reset();
			}
		}
	}
};

#endif