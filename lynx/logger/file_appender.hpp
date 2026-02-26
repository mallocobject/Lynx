#ifndef LYNX_LOGGER_FILE_APPENDER_HPP
#define LYNX_LOGGER_FILE_APPENDER_HPP

#include "lynx/base/noncopyable.hpp"
#include <cstddef>
#include <cstdio>
#include <filesystem>
namespace lynx
{
namespace logger
{
class FileAppender : base::noncopyable
{
  private:
	std::filesystem::path filepath_;
	char* buf_;
	FILE* file_{nullptr};
	size_t written_bytes_{0};

  public:
	explicit FileAppender(const std::filesystem::path& filepath);
	~FileAppender();

	void append(const char* data, size_t len);
	void flush();

	size_t writtenBytes() const
	{
		return written_bytes_;
	}

	void resetWrittenBytes()
	{
		written_bytes_ = 0;
	}

	const std::filesystem::path& filepath() const
	{
		return filepath_;
	}

  private:
	void ensureDirectoryExists() const;
};
} // namespace logger
} // namespace lynx

#endif