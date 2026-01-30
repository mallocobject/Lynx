#include "lynx/http/http_router.h"
#include "lynx/http/http_request.hpp"
#include "lynx/http/http_response.h"
#include "lynx/net/tcp_connection.h"
#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <sys/stat.h>
#include <vector>

namespace lynx
{
HttpRouter::HttpRouter()
{
	tries_["GET"] = Trie();
	tries_["POST"] = Trie();
}

HttpRouter::~HttpRouter()
{
}

void HttpRouter::dispatch(const HttpRequest& req, HttpResponse* res,
						  const std::shared_ptr<TcpConnection>& conn)
{
	http_handler f;
	std::string method = req.method_raw;
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

		conn->send(res->toString());
	}
}

void HttpRouter::serveFile(const std::shared_ptr<lynx::TcpConnection>& conn,
						   lynx::HttpResponse* res, const std::string& filepath)
{
	std::string path = filepath;

	// remove the prefix '/'
	if (filepath.starts_with('/'))
	{
		path = filepath.substr(1);
	}

	struct stat st;
	if (::stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode))
	{
		res->setStatusCode(200);
		res->setContentType(getMineType(path));
		res->setHeader("Content-Length", std::to_string(st.st_size));

		conn->send(res->toString());
		conn->sendFile(path);
	}
	else
	{
		res->setStatusCode(404);
		res->setContentType("text/html");
		res->setBody("<h1>404 Not Found</h1>");

		conn->send(res->toString());
	}
}

void HttpRouter::Trie::insert(const std::string& path, http_handler handler)
{
	std::vector<std::string_view> segments = split_path(path);

	if (segments.empty())
	{
		root->f = std::move(handler);
		return;
	}

	TrieNode* cur = root.get();
	std::string s;

	for (std::string_view segment : segments)
	{
		s = segment; // string_view -> string
		if (!cur->next_node.count(s))
		{
			cur->next_node[s] = std::make_unique<TrieNode>();
		}
		cur = cur->next_node[s].get();
	}
	cur->f = std::move(handler);
}

HttpRouter::http_handler HttpRouter::Trie::search(const std::string& path)
{
	std::vector<std::string_view> segments = split_path(path);

	if (segments.empty())
	{
		return root->f;
	}

	TrieNode* cur = root.get();
	std::string s;

	for (std::string_view segment : segments)
	{
		s = segment; // string_view -> string
		if (!cur->next_node.count(s))
		{
			return nullptr;
		}
		cur = cur->next_node[s].get();
	}

	return cur->f;
}

// 是否有前缀 / 均可解析
std::vector<std::string_view> HttpRouter::Trie::split_path(
	std::string_view path)
{
	std::vector<std::string_view> segments;
	size_t start = 0;
	size_t end = path.find('/');

	while (end != std::string::npos)
	{
		std::string_view segment = path.substr(start, end - start);
		if (!segment.empty())
		{
			segments.push_back(segment);
		}
		start = end + 1;
		end = path.find('/', start);
	}

	std::string_view last_segment = path.substr(start);
	if (!last_segment.empty())
	{
		segments.push_back(last_segment);
	}

	return segments;
}
} // namespace lynx