#include "handlers.h"
#include <cctype>
#include <format>
#include <stdexcept>
#include <string>

void handler::handleCalculate(const lynx::HttpRequest& req,
							  lynx::HttpResponse* res,
							  const std::shared_ptr<lynx::TcpConnection>& conn)
{
	double a = 0.0;
	double b = 0.0;

	try
	{
		const std::string& body = req.body;

		auto a_pos = req.body.find("\"a\"");
		auto b_pos = req.body.find("\"b\"");

		if (a_pos == std::string::npos || b_pos == std::string::npos)
		{
			throw std::runtime_error("Invalid data");
		}

		a = std::stod(req.body.substr(a_pos + 4));
		b = std::stod(req.body.substr(b_pos + 4));

		double sum = a + b;

		res->setStatusCode(200);
		res->setContentType("application/json");
		res->setBody(std::format("{{\"sum\": {0}}}", sum));

		conn->send(res->toFormattedString());
	}
	catch (const std::exception& e)
	{
		res->setStatusCode(400);
		res->setContentType("application/json");
		res->setBody(std::format("{{\"error\": \"{}\"}}", e.what()));

		conn->send(res->toFormattedString());
	}
}
