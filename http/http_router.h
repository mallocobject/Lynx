#ifndef LYNX_HTTP_ROUTER_H
#define LYNX_HTTP_ROUTER_H

#include "lynx/include/common.h"
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
namespace lynx
{
class HttpRequest;
class HttpResponse;
class TcpConnection;
class HttpRouter
{
	using http_handler =
		std::function<void(const HttpRequest&, HttpResponse*,
						   const std::shared_ptr<TcpConnection>&)>;

  private:
	// std::unordered_map<std::string, http_handler> routes_;

	struct TrieNode
	{
		http_handler f;
		std::unordered_map<std::string, std::unique_ptr<TrieNode>> next_node;
	};

	struct Trie
	{
		std::unique_ptr<TrieNode> root;
		Trie() : root(std::make_unique<TrieNode>())
		{
		}

		void insert(const std::string& path, http_handler handler);
		http_handler search(const std::string& path);

	  private:
		std::vector<std::string_view> split_path(std::string_view path);
	};

	std::unordered_map<std::string, Trie> tries_;

  public:
	DISABLE_COPY(HttpRouter)

	HttpRouter();
	~HttpRouter();

	void addRoute(const std::string& method, const std::string& path,
				  const http_handler& handler)
	{
		tries_[method].insert(path, handler);
	}

	void dispatch(const HttpRequest& req, HttpResponse* res,
				  const std::shared_ptr<TcpConnection>& conn);

	static void serveFile(const std::shared_ptr<lynx::TcpConnection>& conn,
						  lynx::HttpResponse* res, const std::string& path);

  private:
	static std::string getMineType(const std::string& path)
	{
		if (path.ends_with(".html"))
		{
			return "text/html; charset=utf-8";
		}
		else if (path.ends_with(".css"))
		{
			return "text/css";
		}
		else if (path.ends_with(".js"))
		{
			return "application/javascript";
		}
		else if (path.ends_with(".png"))
		{
			return "image/png";
		}
		else if (path.ends_with(".jpg") || path.ends_with(".jpeg"))
		{
			return "image/jpeg";
		}

		return "application/octet-stream";
	}
};
} // namespace lynx

#endif