#include "http_app.hpp"
#include "lynx/http/http_router.h"
#include "lynx/net/event_loop.h"
#include "lynx/net/tcp_connection.h"
#include "lynx/net/tcp_server.h"
#include <cstddef>
#include <cstdint>
#include <format>
#include <stdexcept>
#include <string>
#include <unistd.h>

struct Config
{
	std::string ip = "0.0.0.0";
	uint16_t port = 0;
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
		// printf("Options:\n");
		// printf("  -p : Port number (Mandatory)\n");
		// printf("  -i : IP address (Default: 0.0.0.0)\n");
		// printf("  -t : Number of worker threads (Default: CPU cores)\n");
		// printf("  -n : Server name (Default: Lynx-WebServer)\n");
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

	http_app.addRoute(
		"POST", "/calculate",
		[](const auto& req, auto* res, const auto& conn)
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

					conn->send(res->toString());
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

				conn->send(res->toString());
			}
		});

	http_app.startup();
	loop.run();
	lynx::LOG_INFO << "Server started at " << conf.ip << ":" << conf.port;

	return 0;
}