#include "lynx/http/http_router.hpp"
#include "lynx/http/http_request.hpp"
#include "lynx/http/http_response.hpp"
#include "lynx/logger/logger.hpp"
#include "lynx/tcp/tcp_connection.hpp"
#include <fcntl.h>
#include <sys/stat.h>

using namespace lynx;

HttpRouter::HttpRouter()
{
	tries_.emplace("GET", Trie());
	tries_.emplace("POST", Trie());
}

HttpRouter::~HttpRouter()
{
}

void HttpRouter::dispatch(const HttpRequest& req, HttpResponse* res,
						  const std::shared_ptr<TcpConnection>& conn)
{
	http_handler f;
	std::string method = req.method;
	std::string path = req.path;

	if ((f = tries_[method].search(path)))
	{
		f(req, res, conn);
	}
	else
	{
		res->setStatusCode(404);
		res->setContentType("text/html");
		res->setBody("<h1>404 Not Found</h1>");

		conn->send(res->toFormattedString());
	}
}

void HttpRouter::sendFile(const std::shared_ptr<lynx::TcpConnection>& conn,
						  lynx::HttpResponse* res, const std::string& file_path)
{
	std::string path = file_path;

	struct stat st;
	if (::stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode))
	{
		res->setStatusCode(200);
		res->setContentType(getMineType(path));
		res->setHeader("Content-Length", std::to_string(st.st_size));

		conn->send(res->toFormattedString());
		conn->sendFile(path);
	}
	else
	{
		LOG_ERROR << "HttpRouter::serveFile: " << file_path
				  << " Error: " << strerror(errno);
		res->setStatusCode(404);
		res->setContentType("text/html");
		res->setBody("<h1>404 Not Found</h1>");

		conn->send(res->toFormattedString());
	}
}

void HttpRouter::Trie::insert(const std::string& path, http_handler handler)
{
	if (path == "/")
	{
		root->f = std::move(handler);
		return;
	}

	size_t start = (path[0] == '/') ? 1 : 0;
	TrieNode* cur = root.get();

	while (start != std::string::npos)
	{
		std::string s = subPath(path, &start);
		if (s.empty() && start == std::string::npos)
		{
			break; // error
		}

		if (cur->next_node.find(s) == cur->next_node.end())
		{
			cur->next_node[s] = std::make_unique<TrieNode>();
		}

		cur = cur->next_node[s].get();
	}

	cur->f = std::move(handler);
}

HttpRouter::http_handler HttpRouter::Trie::search(const std::string& path)
{
	if (path == "/")
	{
		return root->f;
	}

	size_t start = (path[0] == '/') ? 1 : 0;
	TrieNode* cur = root.get();

	while (start != std::string::npos)
	{
		std::string s = subPath(path, &start);
		if (s.empty() && start == std::string::npos)
		{
			break; // error
		}

		if (cur->next_node.find(s) == cur->next_node.end())
		{
			return nullptr; // router not exist
		}

		cur = cur->next_node[s].get();
	}

	return cur->f;
}

std::string HttpRouter::Trie::subPath(const std::string& path, size_t* start)
{
	// because *start maybe arive 1
	if (*start == std::string::npos || *start >= path.size())
	{
		*start = std::string::npos;
		return "";
	}

	size_t begin = *start;
	size_t end = path.find('/', begin);

	if (end != std::string::npos)
	{
		*start = end + 1;
		return path.substr(begin, end - begin);
	}

	*start = std::string::npos;
	return path.substr(begin);
}