#include "email.hpp"
#include "lynx/logger/logger.hpp"
#include "lynx/time/time_stamp.hpp"
#include <cstdint>
#include <cstdlib>
#include <format>
#include <lynx/lynx.hpp>
#include <memory>
#include <string>

using namespace lynx;
using entry_type = base::Entry<tcp::Connection>;
using cc_type = base::CircularBuffer<entry_type>;

std::string password;

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		exit(EXIT_FAILURE);
	}

	password = std::string(argv[1]);

	// 初始化日志
	logger::Logger::initAsyncLogging(LYNX_WEB_SRC_DIR "/logs/", "sql");

	tcp::EventLoop loop;

	cc_type conn_buckets_;
	loop.runEvery(1.0, [&conn_buckets_]()
				  { conn_buckets_.push_back(cc_type::Bucket()); });

	// 创建 HTTP 服务器
	tcp::Server server(&loop, "0.0.0.0", 8080, "Lynx-sql", 8);

	// 启动 MySql 连接池，并确保表存在
	auto& connection_pool = [&loop]() -> lynx::sql::ConnectionPool&
	{
		static lynx::sql::ConnectionPool cp(tcp::InetAddr(3306, true), "lynx",
											"lynx7.cc", 8, 16);
		auto sess = cp.connection();
		lynx::sql::Schema db = sess->schema("lynx");
		// 创建 users 表
		db.createTable("users", true)
			.addColumn("uid", lynx::sql::Type::BIGINT, 0, true)
			.addColumn("username", lynx::sql::Type::STRING, 16)
			.addColumn("password", lynx::sql::Type::STRING, 20)
			.addColumn("email", lynx::sql::Type::STRING, 255)
			.primaryKey("uid")
			.execute();

		// 创建 verification_codes 表
		db.createTable("verification_codes", true)
			.addColumn("uid", lynx::sql::Type::BIGINT, 0, true)
			.addColumn("email", lynx::sql::Type::STRING, 255)
			.addColumn("code", lynx::sql::Type::STRING, 6)
			.addColumn("expires_at", lynx::sql::Type::BIGINT, 0)
			.addColumn("created_at", lynx::sql::Type::BIGINT, 0)
			.addColumn("used", lynx::sql::Type::BOOL, 0)
			.primaryKey("uid")
			.execute();

		return cp;
	}();

	// 每天定时删除已使用和过期验证码
	loop.runEvery(
		24 * 60 * 60,
		[&connection_pool]
		{
			auto sess = connection_pool.connection();
			lynx::sql::Schema db = sess->schema("lynx");
			lynx::sql::Table verification_codes_table =
				db.table("verification_codes");
			verification_codes_table.remove()
				.where(std::format("used = 1 OR expires_at < {}",
								   time::TimeStamp::now().toFormattedString()))
				.execute();
		});

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
		"POST", "/verify",
		[&connection_pool](const auto& req, auto* res, const auto& conn)
		{
			try
			{
				json::Tokenizer tokenizer(req.body);
				json::Ref root = json::Parser(&tokenizer).parse();

				std::string target_email = root["email"].asStr();

				auto sess = connection_pool.connection();
				lynx::sql::Schema db = sess->schema("lynx");

				lynx::sql::Table verification_codes_table =
					db.table("verification_codes");
				std::string verify_code = generateVerificationCode();

				CURLcode mail_ok = mail(target_email, verify_code);

				if (mail_ok != CURLE_OK)
				{
					LOG_ERROR << "curl_easy_perform() failed: "
							  << curl_easy_strerror(mail_ok);
					res->setStatusCode(503);
					res->setBody(
						R"({"status":"fail","message":"验证码发送失败"})");
				}
				else
				{
					auto now_time_stamp = time::TimeStamp::now();

					verification_codes_table
						.insert("email", "code", "expires_at", "created_at",
								"used")
						.values(target_email, verify_code,
								time::TimeStamp::addTime(now_time_stamp, 5 * 60)
									.microseconds(),
								now_time_stamp.microseconds(), 0)
						.execute();

					res->setStatusCode(200);
					res->setBody(
						R"({"status":"success","message":"验证码已发送"})");
				}

				res->setContentType("application/json");
				conn->send(res->toFormattedString());
			}
			catch (const ::sql::SQLException& e)
			{
				res->setStatusCode(500);
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

	router.addRoute(
		"POST", "/post",
		[&connection_pool](const auto& req, auto* res, const auto& conn)
		{
			try
			{
				json::Tokenizer tokenizer(req.body);
				json::Ref root = json::Parser(&tokenizer).parse();

				std::string action = root["action"].asStr();

				auto sess = connection_pool.connection();
				lynx::sql::Schema db = sess->schema("lynx");
				lynx::sql::Table users_table = db.table("users");

				if (action == "register")
				{
					std::string username = root["username"].asStr();
					std::string password = root["password"].asStr();
					std::string email = root["email"].asStr();
					std::string code = root["code"].asStr();

					lynx::sql::RowResult result =
						users_table.select("uid")
							.where(
								std::format("username = '{}' OR email = '{}'",
											username, email))
							.execute();

					if (result.size() > 0)
					{
						res->setStatusCode(409);
						res->setBody(
							R"({"status":"fail","message":"用户已存在"})");
					}
					else
					{
						lynx::sql::Table verification_codes_table =
							db.table("verification_codes");

						lynx::sql::RowResult result_code =
							verification_codes_table.select("expires_at")
								.where(
									std::format("email = '{}' AND code = '{}'",
												email, code))
								.execute();

						if (result_code.size() == 1 &&
							result_code.get()->next())
						{
							int64_t stored_expires_at =
								result_code.get()->getInt64("expires_at");

							verification_codes_table.update()
								.set("used", 1)
								.where(
									std::format("email = '{}' AND code = '{}'",
												email, code))
								.execute();

							if (time::TimeStamp::now().microseconds() <=
								stored_expires_at)
							{
								users_table
									.insert("username", "password", "email")
									.values(username, password, email)
									.execute();

								res->setStatusCode(200);
								res->setBody(
									R"({"status":"success","message":"注册成功"})");
							}
							else
							{
								res->setStatusCode(200);
								res->setBody(
									R"({"status":"fail","message":"验证码已过期"})");
							}
						}
						else
						{
							res->setStatusCode(401);
							res->setBody(
								R"({"status":"fail","message":"验证码错误"})");
						}
					}
				}
				else if (action == "login")
				{
					std::string username = root["username"].asStr();
					std::string password = root["password"].asStr();

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
				else if (action == "reset")
				{
					std::string new_password = root["new_password"].asStr();
					std::string email = root["email"].asStr();
					std::string code = root["code"].asStr();

					lynx::sql::RowResult result =
						users_table.select("email")
							.where(std::format("email = '{}'", email))
							.execute();

					if (result.size() == 1 && result.get()->next())
					{
						lynx::sql::Table verification_codes_table =
							db.table("verification_codes");

						lynx::sql::RowResult result_code =
							verification_codes_table.select("expires_at")
								.where(std::format(
									"email = '{}' AND code = '{}' AND used = 0",
									email, code))
								.execute();

						if (result_code.size() == 1 &&
							result_code.get()->next())
						{
							int64_t stored_expires_at =
								result_code.get()->getInt64("expires_at");

							verification_codes_table.update()
								.set("used", 1)
								.where(
									std::format("email = '{}' AND code = '{}'",
												email, code))
								.execute();

							if (time::TimeStamp::now().microseconds() <=
								stored_expires_at)
							{
								users_table.update()
									.set("password", new_password)
									.where(std::format("email = '{}'", email))
									.execute();

								res->setStatusCode(200);
								res->setBody(
									R"({"status":"success","message":"密码修改成功"})");
							}
							else
							{
								res->setStatusCode(200);
								res->setBody(
									R"({"status":"fail","message":"验证码已过期"})");
							}
						}
						else
						{
							res->setStatusCode(401);
							res->setBody(
								R"({"status":"fail","message":"验证码错误"})");
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