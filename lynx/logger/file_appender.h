#ifndef LYNX_FILE_APPENDER_H
#define LYNX_FILE_APPENDER_H

#include "lynx/base/common.hpp"
#include <cstddef>
#include <cstdio>
namespace lynx
{
class FileAppender
{

  private:
	char buffer_[64 * 1024];
	FILE* file_;
	size_t written_bytes_;

  public:
	DISABLE_COPY(FileAppender)

	explicit FileAppender(const char* filename);
	~FileAppender();

	void append(const char* line, size_t len);

	void flush();

	size_t writtenBytes() const
	{
		return written_bytes_;
	}

	void resetWritten()
	{
		written_bytes_ = 0;
	}

  private:
	size_t write(const char* line, size_t len);
	void init(const char* filename);
};
} // namespace lynx

#endif