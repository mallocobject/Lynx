#include <functional>
#include <lynx/lynx.hpp>
#include <memory>

using namespace lynx;
using entry_type = base::Entry<tcp::Connection>;
using cc_type = base::CircularBuffer<entry_type>;

int main(int argc, char* argv[])
{
	// 初始化日志
	logger::Logger::initAsyncLogging(LYNX_WEB_SRC_DIR "/logs/", "http_server");

	tcp::EventLoop loop;

	// 初始化 time wheel
	cc_type conn_buckets_;
	loop.runEvery(1.0, [&conn_buckets_]()
				  { conn_buckets_.push_back(cc_type::Bucket()); });

	// 创建 HTTP 服务器
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
		[&conn_buckets_](const std::shared_ptr<tcp::Connection>& conn)
		{
			if (conn->connected())
			{
				LOG_INFO << "Client connected: "
						 << conn->addr().toFormattedString();

				std::shared_ptr<entry_type> entry =
					std::make_shared<entry_type>(
						conn,
						std::bind(&tcp::Connection::shutdown, conn.get()));
				conn_buckets_.push_back(entry);

				tcp::Context ctx;
				ctx.session_ = std::make_shared<http::Session>();
				ctx.entry_ = entry;

				conn->setContext(ctx);
			}
			else if (conn->disconnected())
			{
				LOG_INFO << "Client disconnected: "
						 << conn->addr().toFormattedString();
			}
		});

	// 设置 HTTP 处理回调
	server.setMessageCallback(
		[&router, &conn_buckets_](const std::shared_ptr<tcp::Connection>& conn,
								  tcp::Buffer* buf)
		{
			auto session = conn->context<tcp::Context>().session_;
			auto entry = conn->context<tcp::Context>().entry_.lock();

			if (entry)
			{
				conn_buckets_.push_back(entry);
			}

			if (!session->parser(buf))
			{
				conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
				conn->shutdown();
				return;
			}

			if (session->completed())
			{
				const http::Request& req = session->req();
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
					session->clear();
				}
			}
		});

	LOG_INFO << "HTTP Server listening on 0.0.0.0:8080";
	server.run();
	loop.run();

	logger::Logger::shutdownAsyncLogging();
	return 0;
}