#include "lynx/http/http_context.h"
#include "lynx/http/http_request.hpp"
#include "lynx/http/http_response.h"
#include "lynx/http/http_router.h"
#include "lynx/net/buffer.h"
#include "lynx/net/event_loop.h"
#include "lynx/net/tcp_connection.h"
#include "lynx/net/tcp_server.h"
#include <any>
#include <format>
#include <memory>
#include <stdexcept>
#include <string>
#include <sys/stat.h>

int main()
{
	lynx::EventLoop loop;
	lynx::TcpServer server(&loop, "0.0.0.0", 8080, "Lynx-WebServer");

	lynx::HttpRouter router;
	router.addRoute(
		"GET", "/",
		[](const lynx::HttpRequest& req, lynx::HttpResponse* res,
		   const std::shared_ptr<lynx::TcpConnection>& conn)
		{ lynx::HttpRouter::serveFile(conn, res, "../templates/index.html"); });

	// 注册 CSS 路由
	router.addRoute(
		"GET", "/static/css/style.css",
		[](const auto& req, auto* res, const auto& conn)
		{ lynx::HttpRouter::serveFile(conn, res, "../static/css/style.css"); });

	// 注册 JS 路由
	router.addRoute(
		"GET", "/static/js/script.js",
		[](const auto& req, auto* res, const auto& conn)
		{ lynx::HttpRouter::serveFile(conn, res, "../static/js/script.js"); });

	router.addRoute(
		"GET", "/hello",
		[](const lynx::HttpRequest& req, lynx::HttpResponse* res,
		   const std::shared_ptr<lynx::TcpConnection>& conn)
		{ lynx::HttpRouter::serveFile(conn, res, "../templates/hello.html"); });

	router.addRoute("GET", "/json",
					[](const lynx::HttpRequest& req, lynx::HttpResponse* res,
					   const std::shared_ptr<lynx::TcpConnection>& conn)
					{
						res->setStatusCode(200);
						res->setContentType("application/json");
						res->setBody(
							"{\"status\":\"ok\", \"message\":\"hello world\"}");

						conn->send(res->toFormattedString());
					});

	router.addRoute(
		"POST", "/calculate",
		[](const lynx::HttpRequest& req, lynx::HttpResponse* res,
		   const std::shared_ptr<lynx::TcpConnection>& conn)
		{
			double a = 0.0;
			double b = 0.0;

			try
			{
				auto a_pos = req.body.find("\"a\"");
				auto b_pos = req.body.find("\"b\"");
				if (a_pos != std::string::npos && b_pos != std::string::npos)
				{
					a = std::stod(req.body.substr(a_pos + 4));
					b = std::stod(req.body.substr(b_pos + 4));

					double sum = a + b;

					res->setStatusCode(200);
					res->setContentType("application/json");
					res->setBody(std::format("{{\"sum\": {0}}}", sum));

					conn->send(res->toFormattedString());
				}
				else
				{
					throw std::logic_error("invalid data");
				}
			}
			catch (...)
			{
				res->setStatusCode(400);
				res->setContentType("application/json");
				res->setBody("{\"error\": \"invalid data\"}");

				conn->send(res->toFormattedString());
			}
		});

	server.setConnectionCallback(
		[](const std::shared_ptr<lynx::TcpConnection>& conn)
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

				router.dispatch(req, &res, conn);

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