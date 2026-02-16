#include "lynx/time/entry.hpp"
#include "lynx/tcp/tcp_connection.hpp"

using namespace lynx;

Entry::Entry(const std::weak_ptr<TcpConnection>& weak_conn)
	: weak_conn_(weak_conn)
{
}

Entry::~Entry()
{
	auto conn = weak_conn_.lock();
	if (conn)
	{
		conn->shutdown();
	}
}