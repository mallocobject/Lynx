#ifndef HTTP_BASE_HPP
#define HTTP_BASE_HPP

#include "lynx/buffer.hpp"
#include "lynx/event_loop.hpp"
#include "lynx/http_context.hpp"
#include "lynx/http_request.hpp"
#include "lynx/http_response.hpp"
#include "lynx/http_router.hpp"
#include "lynx/logger.hpp"
#include "lynx/tcp_connection.hpp"
#include "lynx/tcp_server.hpp"
#include <any>
#include <cstdint>
#include <functional>
#include <string>
class HttpBase
{
  private:
	lynx::TcpServer server_;
	lynx::HttpRouter router_;

  public:
	HttpBase(lynx::EventLoop* loop, const std::string& ip, uint16_t port,
			 const std::string& name,
			 size_t sub_reactor_num = std::thread::hardware_concurrency())
		: server_(loop, ip, port, name, sub_reactor_num)
	{
		server_.setConnectionCallback(
			std::bind(&HttpBase::onConnection, this, std::placeholders::_1));
		server_.setMessageCallback(std::bind(&HttpBase::onMessage, this,
											 std::placeholders::_1,
											 std::placeholders::_2));
	}

	void addRoute(const std::string& method, const std::string& path,
				  const lynx::HttpRouter::http_handler& handler)
	{
		router_.addRoute(method, path, handler);
	}

	void run()
	{
		server_.run();
	}

	size_t connectionNum() const
	{
		return server_.connectionNum();
	}

  private:
	virtual void onConnection(const std::shared_ptr<lynx::TcpConnection>& conn)
	{
		if (conn->connected())
		{
			LOG_INFO << "New connection from "
					 << conn->addr().toFormattedString();
			// 每个连接绑定一个 HttpContext 实例 (基于 std::any)
			conn->setContext(std::make_shared<lynx::HttpContext>());
		}
		else if (conn->disconnected())
		{
			LOG_INFO << "Connection closed on "
					 << conn->addr().toFormattedString();
		}
	}

	virtual void onMessage(const std::shared_ptr<lynx::TcpConnection>& conn,
						   lynx::Buffer* buf)
	{
		auto context =
			std::any_cast<std::shared_ptr<lynx::HttpContext>>(conn->context());

		if (!context->parser(buf))
		{
			conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
			conn->shutdown();
			return;
		}

		if (context->completed())
		{
			const lynx::HttpRequest& req = context->req();
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
				context->clear();
			}
		}
	}
};

#endif