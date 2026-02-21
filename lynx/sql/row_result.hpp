#ifndef LYNX_ROW_RESULT_HPP
#define LYNX_ROW_RESULT_HPP

#include "lynx/base/noncopyable.hpp"
#include <cppconn/resultset.h>
#include <cstdint>
#include <iostream>
#include <memory>
#include <ostream>
namespace lynx
{
class RowResult : public noncopyable
{
  private:
	std::unique_ptr<sql::ResultSet> res_;

  public:
	explicit RowResult(sql::ResultSet* res) : res_(res)
	{
	}

	void printAll()
	{
		if (!res_)
		{
			return;
		}

		uint32_t col_count = res_->getMetaData()->getColumnCount();
		while (res_->next())
		{
			std::cout << "Row: [";
			for (uint32_t i = 1; i <= col_count; i++)
			{
				std::cout << res_->getString(i) << (i < col_count ? ", " : "");
			}
			std::cout << ']' << std::endl;
		}
	}

	sql::ResultSet* get()
	{
		return res_.get();
	}
};
} // namespace lynx

#endif