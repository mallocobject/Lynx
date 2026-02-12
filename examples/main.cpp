#include "handlers.h"
#include "http_app.h"
#include "lynx/http/http_router.h"
#include "lynx/logger/logger.h"
#include "lynx/tcp/event_loop.h"
#include "lynx/tcp/tcp_connection.h"
#include "lynx/tcp/tcp_server.h"
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>
#include <unistd.h>

struct Config
{
	std::string ip = "0.0.0.0";
	uint16_t port = 8080;
	std::string name = "Lynx-WebServer";
	size_t worker_threads = std::thread::hardware_concurrency() - 2;
	bool valid = false;
};

Config parseArgs(int argc, char* argv[])
{
	Config conf;
	int opt;
	while ((opt = ::getopt(argc, argv, "p:i:n:t:h")) != -1)
	{
		switch (opt)
		{
		case 'p':
			conf.port = static_cast<uint16_t>(std::stoi(optarg));
			conf.valid = true;
			break;
		case 'i':
			conf.ip = optarg;
			break;
		case 't':
			conf.worker_threads = std::stoi(optarg);
			break;
		case 'n':
			conf.name = optarg;
			break;
		case 'h':
		default:
			conf.valid = false;
			break;
		}
	}
	return conf;
}

using namespace lynx;

int main(int argc, char* argv[])
{
	Config conf = parseArgs(argc, argv);
	if (!conf.valid)
	{
		printf("Usage: %s -p <port> [-i <ip>] [-t <threads>] [-n <name>]\n",
			   argv[0]);
		printf("Options:\n");
		printf("  -p : Port number (Mandatory)\n");
		printf("  -i : IP address (Default: 0.0.0.0)\n");
		printf("  -t : Number of worker threads (Default: CPU cores - 2)\n");
		printf("  -n : Server name (Default: Lynx-WebServer)\n");
		return 1;
	}

	Logger::initAsyncLogging(LYNX_WEB_SRC_DIR "/logs", argv[0]);

	EventLoop loop;
	HttpApp http_app(&loop, conf.ip, conf.port, conf.name, conf.worker_threads);

	LOG_INFO << "Server [" << conf.name << "] starting...";
	LOG_INFO << "Listen on " << conf.ip << ":" << conf.port;
	LOG_INFO << "Threads: 1 (Main) + " << conf.worker_threads << " (Workers)"
			 << " + 1 (Logger)";

	http_app.addRoute("GET", "/",
					  [](const auto& req, auto* res, const auto& conn) {
						  HttpRouter::sendFile(conn, res,
											   LYNX_WEB_SRC_DIR
											   "/templates/index.html");
					  });

	// 注册 CSS 路由
	http_app.addRoute("GET", "/static/css/style.css",
					  [](const auto& req, auto* res, const auto& conn) {
						  HttpRouter::sendFile(conn, res,
											   LYNX_WEB_SRC_DIR
											   "/static/css/style.css");
					  });

	// 注册 JS 路由
	http_app.addRoute("GET", "/static/js/script.js",
					  [](const auto& req, auto* res, const auto& conn) {
						  HttpRouter::sendFile(conn, res,
											   LYNX_WEB_SRC_DIR
											   "/static/js/script.js");
					  });

	http_app.addRoute("POST", "/calculate", handler::handleCalculate);

	http_app.addRoute("GET", "/json",
					  [](const auto& req, auto* res, const auto& conn)
					  {
						  res->setStatusCode(200);
						  res->setContentType("application/json");
						  res->setBody("{\"message\":\"ok\"}");

						  conn->send(res->toFormattedString());
					  });

	http_app.addRoute("GET", "/hello",
					  [](const auto& req, auto* res, const auto& conn)
					  {
						  res->setStatusCode(200);
						  res->setContentType("text/plain");
						  res->setHeader("Server", "Lynx");
						  res->setBody("hello, world!\n");

						  conn->send(res->toFormattedString());
					  });
	// // 注册大文件下载路由
	// http_app.addRoute(
	// 	"GET", "/download",
	// 	[](const auto& req, auto* res, const auto& conn)
	// 	{
	// 		LOG_INFO << "Request large file download...";
	// 		HttpRouter::serveFile(
	// 			conn, res, LYNX_WEB_SRC_DIR "/templates/large_test_file.dat");
	// 	});

	// loop.runEvery(2, [&http_app]()
	// 			  { std::cout << http_app.connectionNum() << std::endl; });

	http_app.run();
	std::cout << "Server started at " << conf.ip << ":" << conf.port
			  << std::endl;
	loop.run();

	Logger::shutdownAsyncLogging();

	return 0;
}