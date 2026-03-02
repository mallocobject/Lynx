#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstdint>
#include <format>
#include <iterator>
#include <span>
#include <stdexcept>
#include <string>
std::string generateVerificationCode();
bool fill_random_bytes(std::span<uint8_t> buffer);
bool fill_random_bytes_fallback(std::span<uint8_t> buffer);

template <size_t N = 32> std::string generate_hex_token()
{
	std::array<uint8_t, N> buffer;
	if (!fill_random_bytes(buffer))
	{
		// 若 getrandom 失败，尝试回退
		if (!fill_random_bytes_fallback(buffer))
		{
			throw std::runtime_error("Failed to generate random bytes");
		}
	}

	std::string result;
	auto out = std::back_inserter(result);
	for (auto b : buffer)
	{
		std::format_to(out, "{:02x}", b); // 小写十六进制，两位，补零
	}

	return result;
}

constexpr char base64_url_alphabet[] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

std::string base64_url_encode(std::span<const uint8_t> input);

template <size_t N = 32> std::string generate_base64_token()
{
	std::array<uint8_t, N> buffer;
	if (!fill_random_bytes(buffer) && !fill_random_bytes_fallback(buffer))
	{
		throw std::runtime_error("Failed to generate random bytes");
	}
	return base64_url_encode(buffer);
}
#endif