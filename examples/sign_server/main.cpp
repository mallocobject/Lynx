#include <format>
#include <lynx/lynx.hpp>
#include <string>

using namespace lynx;

int main(int argc, char* argv[])
{
	// 初始化日志
	logger::Logger::initAsyncLogging(LYNX_WEB_SRC_DIR "/logs/", "sql");

	// 创建 HTTP 服务器
	tcp::EventLoop loop;
	tcp::Server server(&loop, "0.0.0.0", 8080, "Lynx-sql", 8);

	// 启动 MySql 连接池，并确保表存在
	auto& connection_pool = []() -> lynx::sql::ConnectionPool&
	{
		static lynx::sql::ConnectionPool cp(tcp::InetAddr(3306, true), "liudan",
											"liudan", 8, 16);
		auto sess = cp.connection();
		lynx::sql::Schema db = sess->schema("mysql");
		// 如果表不存在则创建
		db.createTable("users", true)
			.addColumn("uid", lynx::sql::Type::INT, 0, true)
			.addColumn("username", lynx::sql::Type::STRING, 50)
			.addColumn("password", lynx::sql::Type::STRING, 255)
			.addColumn("email", lynx::sql::Type::STRING, 100)
			.primaryKey("uid")
			.execute();
		return cp;
	}();

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
		"POST", "/posts",
		[&connection_pool](const auto& req, auto* res, const auto& conn)
		{
			try
			{
				json::Tokenizer tokenizer(req.body);
				json::Ref root = json::Parser(&tokenizer).parse();

				std::string action = root["action"].asStr();
				std::string username = root["username"].asStr();
				std::string password = root["password"].asStr();

				auto sess = connection_pool.connection();
				lynx::sql::Schema db = sess->schema("mysql");
				lynx::sql::Table users_table = db.table("users");

				if (action == "register")
				{
					lynx::sql::RowResult result =
						users_table.select("password")
							.where(std::format("username = '{}'", username))
							.execute();

					if (result.size() > 0)
					{
						res->setStatusCode(409);
						res->setBody(
							R"({"status":"fail","message":"用户已存在"})");
					}
					else
					{
						std::string email = root["email"].asStr();

						users_table.insert("username", "password", "email")
							.values(username, password, email)
							.execute();

						res->setStatusCode(200);
						res->setBody(
							R"({"status":"success","message":"注册成功"})");
					}
				}
				else if (action == "login")
				{
					lynx::sql::RowResult result =
						users_table.select("password")
							.where(std::format("username = '{}'", username))
							.execute();

					if (result.size() == 1 && result.get()->next())
					{
						std::string stored_pwd =
							result.get()->getString("password");
						if (stored_pwd == password)
						{
							res->setStatusCode(200);
							res->setBody(
								R"({"status":"success","message":"登录成功"})");
						}
						else
						{
							res->setStatusCode(401);
							res->setBody(
								R"({"status":"fail","message":"密码错误"})");
						}
					}
					else
					{
						res->setStatusCode(401);
						res->setBody(
							R"({"status":"fail","message":"用户不存在"})");
					}
				}
				else
				{
					res->setStatusCode(400);
					res->setBody(R"({"error":"未知操作"})");
				}

				res->setContentType("application/json");
				conn->send(res->toFormattedString());
			}
			catch (const ::sql::SQLException& e)
			{
				// 处理 SQL 特定异常
				res->setStatusCode(500); // 内部服务器错误
				res->setContentType("application/json");
				res->setBody(std::format(
					"{{\"error\": \"Database error: {}\"}}", e.what()));

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