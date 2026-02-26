#ifndef LYNX_HTTP_RESPONSE_HPP
#define LYNX_HTTP_RESPONSE_HPP

#include "lynx/base/noncopyable.hpp"
#include <map>
#include <string>
namespace lynx
{
namespace http
{
class Response : public base::noncopyable
{
  private:
	int status_code_;
	std::string status_msg_;
	std::string version_;
	std::map<std::string, std::string> headers_;
	std::string body_;

  public:
	Response();
	~Response();

	void setStatusCode(int code)
	{
		status_code_ = code;
		status_msg_ = code2msg(code);
	}

	void setHeader(const std::string& key, const std::string& value)
	{
		headers_[key] = value;
	}

	void setBody(const std::string& body)
	{
		body_ = body;
		setHeader("Content-Length", std::to_string(body_.size()));
	}

	void setContentType(const std::string& type)
	{
		setHeader("Content-Type", type);
	}

	void setKeepAlive(bool on)
	{
		if (on)
		{
			setHeader("Connection", "keep-alive");
		}
		else
		{
			setHeader("Connection", "close");
		}
	}

	std::string toFormattedString() const;

  private:
	static std::string_view code2msg(int code)
	{
		static const std::map<int, std::string_view> status_msgs = {
			{200, "OK"},		  {301, "Moved Permanently"},
			{400, "Bad Request"}, {403, "Forbidden"},
			{404, "Not Found"},	  {500, "Internal Server Error"},
		};

		auto it = status_msgs.find(code);
		return (it != status_msgs.end()) ? it->second : "Unknown";
	}
};
} // namespace http
} // namespace lynx

#endif