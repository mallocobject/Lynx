#ifndef LYNX_HTTP_ROUTER_H
#define LYNX_HTTP_ROUTER_H

#include "lynx/base/noncopyable.hpp"
#include <cassert>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
namespace lynx
{
class HttpRequest;
class HttpResponse;
class TcpConnection;
class HttpRouter : public noncopyable
{
  public:
	using http_handler =
		std::function<void(const HttpRequest&, HttpResponse*,
						   const std::shared_ptr<TcpConnection>&)>;

  private:
	struct TrieNode
	{
		http_handler f;
		std::unordered_map<std::string, std::unique_ptr<TrieNode>> next_node;
	};

	struct Trie
	{
		std::unique_ptr<TrieNode> root;
		Trie()
		{
			root = std::make_unique<TrieNode>();
		}

		~Trie()
		{
		}

		Trie(Trie&&) = default;

		void insert(const std::string& path, http_handler handler);
		http_handler search(const std::string& path);

	  private:
		std::string subPath(const std::string& path, size_t* start);
	};

	std::unordered_map<std::string, Trie> tries_;

  public:
	HttpRouter();
	~HttpRouter();

	void addRoute(const std::string& method, const std::string& path,
				  const http_handler& handler)
	{
		assert(tries_.size() == 2);
		tries_[method].insert(path, handler);
	}

	void dispatch(const HttpRequest& req, HttpResponse* res,
				  const std::shared_ptr<TcpConnection>& conn);

	static void sendFile(const std::shared_ptr<lynx::TcpConnection>& conn,
						 lynx::HttpResponse* res, const std::string& file_path);

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