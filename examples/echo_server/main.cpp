#include "echo_app.hpp"
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
	std::string name = "Lynx-EchoServer";
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
		printf("  -n : Server name (Default: Lynx-EchoServer)\n");
		return 1;
	}

	Logger::initAsyncLogging(LYNX_WEB_SRC_DIR "/logs", argv[0]);

	EventLoop loop;
	EchoApp echo_app(&loop, conf.ip, conf.port, conf.name, conf.worker_threads);

	loop.runEvery(1.0,
				  [&echo_app]()
				  {
					  std::cout << "Connection buckets have "
								<< echo_app.connectionNumInBuckets()
								<< " connections" << std::endl;
				  });

	LOG_INFO << "Server [" << conf.name << "] starting...";
	LOG_INFO << "Listen on " << conf.ip << ":" << conf.port;
	LOG_INFO << "Threads: 1 (Main) + " << conf.worker_threads << " (Workers)"
			 << " + 1 (Logger)";

	echo_app.run();
	std::cout << "Server started at " << conf.ip << ":" << conf.port
			  << std::endl;
	loop.run();

	Logger::shutdownAsyncLogging();

	return 0;
}