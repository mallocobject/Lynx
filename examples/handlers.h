#ifndef HANDLERS_HPP
#define HANDLERS_HPP

#include "lynx/http/http_request.hpp"
#include "lynx/http/http_response.h"
#include "lynx/tcp/tcp_connection.h"
#include <memory>

namespace handler
{

void handleCalculate(const lynx::HttpRequest& req, lynx::HttpResponse* res,
					 const std::shared_ptr<lynx::TcpConnection>& conn);

} // namespace handler

#endif