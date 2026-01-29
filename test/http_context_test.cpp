#include "lynx/http/http_context.h"
#include "lynx/http/http_request.hpp"
#include "lynx/http/http_response.h"
#include "lynx/http/http_router.h"
#include "lynx/include/buffer.h"
#include "lynx/include/event_loop.h"
#include "lynx/include/tcp_connection.h"
#include "lynx/include/tcp_server.h"
#include <any>
#include <memory>
int main()
{
	lynx::EventLoop loop;
	lynx::TcpServer server(&loop, "0.0.0.0", 8080, "Lynx-Http_server");

	lynx::HttpRouter router;
	router.addRoute("GET", "/",
					[](const lynx::HttpRequest& req, lynx::HttpResponse* res)
					{
						res->setStatusCode(200);
						res->setContentType("text/html");
						res->setBody("<h1>Hello from Lynx!</h1><p>Path: /</p>");
					});

	router.addRoute("GET", "/json",
					[](const lynx::HttpRequest& req, lynx::HttpResponse* res)
					{
						res->setStatusCode(200);
						res->setContentType("application/json");
						res->setBody(
							"{\"status\":\"ok\", \"message\":\"hello world\"}");
					});

	server.setConnectionCallback(
		[](const std::shared_ptr<lynx::TcpConnection>& conn)
		{
			if (conn->connected())
			{
				// 每个连接绑定一个 HttpContext 实例
				// lynx::TcpConnection 应该提供 setContext / getContext (基于
				// std::any)
				conn->setContext(std::make_shared<lynx::HttpContext>());
				lynx::LOG_INFO << "New Connection: " << conn->name();
			}
			else
			{
				lynx::LOG_INFO << "Connection closed: " << conn->name();
			}
		});

	server.setMessageCallback(
		[&router](const std::shared_ptr<lynx::TcpConnection>& conn,
				  std::shared_ptr<lynx::Buffer> buf)
		{
			auto context = std::any_cast<std::shared_ptr<lynx::HttpContext>>(
				conn->context());

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

				router.dispatch(req, &res);

				conn->send(res.toString());

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
		});

	server.startup();
	loop.run();
}