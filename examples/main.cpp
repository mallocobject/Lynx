#include "handlers.h"
#include "http_app.hpp"
#include "lynx/http/http_router.h"
#include "lynx/net/event_loop.h"
#include "lynx/net/tcp_connection.h"
#include "lynx/net/tcp_server.h"
#include <cstddef>
#include <cstdint>
#include <string>
#include <unistd.h>

struct Config
{
	std::string ip = "0.0.0.0";
	uint16_t port = 8080;
	std::string name = "Lynx-WebServer";
	size_t worker_threads = std::thread::hardware_concurrency();
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
		printf("  -t : Number of worker threads (Default: CPU cores)\n");
		printf("  -n : Server name (Default: Lynx-WebServer)\n");
		return 1;
	}

	lynx::EventLoop loop;
	HttpApp http_app(&loop, conf.ip, conf.port, conf.name, conf.worker_threads);

	lynx::LOG_INFO << "Server [" << conf.name << "] starting...";
	lynx::LOG_INFO << "Listen on " << conf.ip << ":" << conf.port;
	lynx::LOG_INFO << "Threads: 1 (Main) + " << conf.worker_threads
				   << " (Workers)";

	http_app.addRoute("GET", "/",
					  [](const auto& req, auto* res, const auto& conn)
					  {
						  lynx::HttpRouter::serveFile(conn, res,
													  LYNX_WEB_SRC_DIR
													  "/templates/index.html");
					  });

	// 注册 CSS 路由
	http_app.addRoute("GET", "/static/css/style.css",
					  [](const auto& req, auto* res, const auto& conn)
					  {
						  lynx::HttpRouter::serveFile(conn, res,
													  LYNX_WEB_SRC_DIR
													  "/static/css/style.css");
					  });

	// 注册 JS 路由
	http_app.addRoute("GET", "/static/js/script.js",
					  [](const auto& req, auto* res, const auto& conn)
					  {
						  lynx::HttpRouter::serveFile(conn, res,
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

						  conn->send(res->toString());
					  });

	// 注册大文件下载路由
	http_app.addRoute(
		"GET", "/download",
		[](const auto& req, auto* res, const auto& conn)
		{
			lynx::LOG_INFO << "Request large file download...";
			lynx::HttpRouter::serveFile(
				conn, res, LYNX_WEB_SRC_DIR "/templates/large_test_file.dat");
		});

	http_app.startup();
	loop.run();
	lynx::LOG_INFO << "Server started at " << conf.ip << ":" << conf.port;

	return 0;
}