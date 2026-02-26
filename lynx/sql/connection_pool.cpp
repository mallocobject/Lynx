#include "lynx/sql/connection_pool.hpp"
#include "lynx/logger/logger.hpp"
#include "lynx/sql/session.hpp"
#include <atomic>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <exception>
#include <memory>
#include <mutex>

using namespace lynx;
using namespace lynx::sql;

ConnectionPool::ConnectionPool(const std::string& ip, uint16_t port,
							   const std::string& user, const std::string& pass,
							   uint32_t min_conn_num, uint32_t max_conn_num)
	: ip_(ip), port_(port), user_(user), pass_(pass),
	  min_conn_num_(min_conn_num), max_conn_num_(max_conn_num),
	  cur_conn_num_(0), stop_(false)
{
	for (size_t i = 0; i < min_conn_num_; i++)
	{
		Session* sess = createSession();
		if (sess)
		{
			sesses_.push(sess);
		}
	}

	cur_conn_num_ = sesses_.size();
	LOG_INFO << "MySQL Connection Pool initialized with " << sesses_.size()
			 << " connections";
}

ConnectionPool::ConnectionPool(const tcp::InetAddr& addr,
							   const std::string& user, const std::string& pass,
							   uint32_t min_conn_num, uint32_t max_conn_num)
	: ConnectionPool(addr.ip(), addr.port(), user, pass, min_conn_num,
					 max_conn_num)
{
}

ConnectionPool::~ConnectionPool()
{
	close();
}

std::shared_ptr<Session> ConnectionPool::connection()
{
	if (stop_.load(std::memory_order_acquire))
	{
		return nullptr;
	}

	{
		std::unique_lock<std::mutex> lock(mtx_);
		if (!sesses_.empty())
		{
			Session* sess = sesses_.front();
			sesses_.pop();
			lock.unlock();

			if (!sess->nativeConnection()->isValid())
			{
				delete sess;
				sess = createSession();
				if (!sess)
				{
					// 创建失败，回滚计数
					lock.lock();
					cur_conn_num_--;
					return nullptr;
				}
			}
			return std::shared_ptr<Session>(sess, [this](Session* s)
											{ this->returnConnection(s); });
		}

		if (cur_conn_num_ >= max_conn_num_)
		{
			if (cv_.wait_for(lock, std::chrono::milliseconds(500)) ==
				std::cv_status::timeout)
			{
				LOG_WARN << "MySQL Connection Pool busy/timeout";
				return nullptr;
			}

			// 可能被 returnConnection 虚假唤醒
			if (sesses_.empty())
			{
				return nullptr;
			}

			Session* sess = sesses_.front();
			sesses_.pop();
			lock.unlock();

			if (!sess->nativeConnection()->isValid())
			{
				delete sess;
				sess = createSession();
				if (!sess)
				{
					lock.lock();
					cur_conn_num_--;
					return nullptr;
				}
			}

			return std::shared_ptr<Session>(sess, [this](Session* s)
											{ this->returnConnection(s); });
		}
		else
		{
			cur_conn_num_++;
			lock.unlock();
		}
	}
	Session* sess = createSession();
	if (!sess)
	{
		std::lock_guard<std::mutex> lock(mtx_);
		cur_conn_num_--;
		return nullptr;
	}
	return std::shared_ptr<Session>(sess, [this](Session* s)
									{ this->returnConnection(s); });
}

Session* ConnectionPool::createSession()
{
	try
	{
		Session* sess = new Session(ip_, port_, user_, pass_);
		if (sess->nativeConnection()->isValid())
		{
			return sess;
		}
		else
		{
			delete sess;
			return nullptr;
		}
	}
	catch (std::exception& e)
	{
		LOG_ERROR << "Create Session Error: " << e.what();
		return nullptr;
	}
}

void ConnectionPool::returnConnection(Session* sess)
{
	if (stop_.load(std::memory_order_acquire))
	{
		delete sess;
		return;
	}

	std::unique_ptr<Session> defer;
	{
		std::unique_lock<std::mutex> lock(mtx_);
		assert(cur_conn_num_ >= min_conn_num_ &&
			   cur_conn_num_ <= max_conn_num_);

		if (sesses_.size() < min_conn_num_)
		{
			sesses_.push(sess);
		}
		else
		{
			cur_conn_num_--;
			lock.unlock();
			defer.reset(sess);
		}
	}

	cv_.notify_one();
}

void ConnectionPool::close()
{
	stop_.store(true, std::memory_order_release);
	std::unique_lock<std::mutex> lock(mtx_);
	while (!sesses_.empty())
	{
		Session* sess = sesses_.front();
		sesses_.pop();
		delete sess;
	}
}