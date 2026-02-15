#ifndef HANDLERS_HPP
#define HANDLERS_HPP

#include "lynx/http_request.hpp"
#include "lynx/http_response.hpp"
#include "lynx/tcp_connection.hpp"
#include <memory>

namespace handler
{

void handleCalculate(const lynx::HttpRequest& req, lynx::HttpResponse* res,
					 const std::shared_ptr<lynx::TcpConnection>& conn);

} // namespace handler

#endif