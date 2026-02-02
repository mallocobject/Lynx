#ifndef HTTP_APP_H
#define HTTP_APP_H

#include "http_base.hpp"

namespace lynx
{

class HttpApp : public HttpBase
{
  public:
	// 直接继承构造函数
	using HttpBase::HttpBase;
};

} // namespace lynx

#endif // HTTP_APP_H