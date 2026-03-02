#ifndef EMAIL_HPP
#define EMAIL_HPP

#include <string>

#include <curl/curl.h>

extern std::string password;

CURLcode mail(const std::string& target_email, const std::string& verify_code);

#endif