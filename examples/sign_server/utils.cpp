#include "utils.hpp"
#include <format>
#include <fstream>
#include <random>
#include <sys/random.h>

std::string generateVerificationCode()
{
	thread_local static std::random_device rd;
	thread_local static std::mt19937 gen(rd());
	thread_local static std::uniform_int_distribution<int> dist(0, 999999);

	int code = dist(gen);
	return std::format("{:06}", code);
}

bool fill_random_bytes(std::span<uint8_t> buffer)
{
	size_t total = buffer.size();
	uint8_t* ptr = buffer.data();
	while (total > 0)
	{
		// 使用 getrandom()，flags=0 表示阻塞直到熵池初始化完成
		ssize_t ret = getrandom(ptr, total, 0);
		if (ret <= 0)
		{
			if (errno == EINTR)
			{
				continue; // 被信号中断，重试
			}
			return false;
		}
		ptr += ret;
		total -= ret;
	}
	return true;
}

bool fill_random_bytes_fallback(std::span<uint8_t> buffer)
{
	std::ifstream urandom("/dev/urandom", std::ios::binary);
	if (!urandom)
	{
		return false;
	}
	urandom.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
	return urandom.gcount() == static_cast<std::streamsize>(buffer.size());
}

std::string base64_url_encode(std::span<const uint8_t> input)
{
	std::string out;
	out.reserve(((input.size() + 2) / 3) * 4);

	for (size_t i = 0; i < input.size(); i += 3)
	{
		uint32_t octet_a = i < input.size() ? input[i] : 0;
		uint32_t octet_b = i + 1 < input.size() ? input[i + 1] : 0;
		uint32_t octet_c = i + 2 < input.size() ? input[i + 2] : 0;

		uint32_t triple = (octet_a << 16) | (octet_b << 8) | octet_c;

		out.push_back(base64_url_alphabet[(triple >> 18) & 0x3F]);
		out.push_back(base64_url_alphabet[(triple >> 12) & 0x3F]);
		out.push_back(i + 1 < input.size()
						  ? base64_url_alphabet[(triple >> 6) & 0x3F]
						  : '=');
		out.push_back(i + 2 < input.size() ? base64_url_alphabet[triple & 0x3F]
										   : '=');
	}
	return out;
}