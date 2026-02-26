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

				LOG_INFO << "root: " << root;

				double sum = 0.0;

				if (root["a"].get()->isValue() &&
					root["a"].get()->asValue()->isFloat())
				{
					sum += root["a"].asFloat();
				}
				else if (root["a"].get()->isValue() &&
						 root["a"].get()->asValue()->isInt())
				{
					sum += root["a"].asInt();
				}

				if (root["b"].get()->isValue() &&
					root["b"].get()->asValue()->isFloat())
				{
					sum += root["b"].asFloat();
				}
				else if (root["b"].get()->isValue() &&
						 root["b"].get()->asValue()->isInt())
				{
					sum += root["b"].asInt();
				}

				json::Ref result =
					json::make_object({{"sum", json::make_value(sum)}});

				res->setStatusCode(200);
				res->setContentType("application/json");

				LOG_INFO << "result: " << result.serialize();
				res->setBody(result.serialize());

				conn->send(res->toFormattedString());

				delete root.get();
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