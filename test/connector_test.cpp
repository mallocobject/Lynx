#include "lynx/include/connector.h"
#include "lynx/include/event_loop.h"
#include "lynx/include/logger.hpp"
#include <memory>

int main()
{
	lynx::EventLoop loop;
	auto connector =
		std::make_shared<lynx::Connector>(&loop, "127.0.0.1", 8234);
	connector->setNewConnectionCallback(
		[&loop](int fd)
		{
			lynx::LOG_INFO << "connected.";
			loop.quit();
		});

	connector->startup();

	loop.run();
}