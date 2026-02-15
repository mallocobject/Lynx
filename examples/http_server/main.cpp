#include <lynx/lynx.hpp>

using namespace lynx;

void handleCalculate(const HttpRequest& req, HttpResponse* res,
					 const std::shared_ptr<TcpConnection>& conn)
{
	double a = 0.0;
	double b = 0.0;

	try
	{
		const std::string& body = req.body;

		auto a_pos = req.body.find("\"a\"");
		auto b_pos = req.body.find("\"b\"");

		if (a_pos == std::string::npos || b_pos == std::string::npos)
		{
			throw std::runtime_error("Invalid data");
		}

		a = std::stod(req.body.substr(a_pos + 4));
		b = std::stod(req.body.substr(b_pos + 4));

		double sum = a + b;

		res->setStatusCode(200);
		res->setContentType("application/json");
		res->setBody(std::format("{{\"sum\": {0}}}", sum));

		conn->send(res->toFormattedString());
	}
	catch (const std::exception& e)
	{
		res->setStatusCode(400);
		res->setContentType("application/json");
		res->setBody(std::format("{{\"error\": \"{}\"}}", e.what()));

		conn->send(res->toFormattedString());
	}
}

int main(int argc, char* argv[])
{
	// 初始化日志
	Logger::initAsyncLogging(LYNX_WEB_SRC_DIR "/logs/", "http_server");

	// 创建 HTTP 服务器
	EventLoop loop;
	TcpServer server(&loop, "0.0.0.0", 8080, "Lynx-WebServer", 4);

	// 创建路由器
	auto router = HttpRouter();

	// 注册路由
	router.addRoute("GET", "/",
					[](const auto& req, auto* res, const auto& conn) {
						HttpRouter::sendFile(conn, res,
											 LYNX_WEB_SRC_DIR
											 "/templates/index.html");
					});

	// 注册 CSS 路由
	router.addRoute("GET", "/static/css/style.css",
					[](const auto& req, auto* res, const auto& conn) {
						HttpRouter::sendFile(conn, res,
											 LYNX_WEB_SRC_DIR
											 "/static/css/style.css");
					});

	// 注册 JS 路由
	router.addRoute("GET", "/static/js/script.js",
					[](const auto& req, auto* res, const auto& conn) {
						HttpRouter::sendFile(
							conn, res, LYNX_WEB_SRC_DIR "/static/js/script.js");
					});

	router.addRoute("POST", "/calculate", &handleCalculate);

	// 设置连接回调
	server.setConnectionCallback(
		[](const std::shared_ptr<TcpConnection>& conn)
		{
			if (conn->connected())
			{
				LOG_INFO << "Client connected: "
						 << conn->addr().toFormattedString();
				// 每个连接绑定一个 HttpContext 实例 (基于 std::any)
				conn->setContext(std::make_shared<HttpContext>());
			}
			else if (conn->disconnected())
			{
				LOG_INFO << "Client disconnected: "
						 << conn->addr().toFormattedString();
			}
		});

	// 设置 HTTP 处理回调
	server.setMessageCallback(
		[&router](const std::shared_ptr<TcpConnection>& conn, Buffer* buf)
		{
			auto context =
				std::any_cast<std::shared_ptr<HttpContext>>(conn->context());

			if (!context->parser(buf))
			{
				conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
				conn->shutdown();
				return;
			}

			if (context->completed())
			{
				const HttpRequest& req = context->req();
				HttpResponse res;

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

	Logger::shutdownAsyncLogging();
	return 0;
}