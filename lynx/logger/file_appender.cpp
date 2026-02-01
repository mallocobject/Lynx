#include "lynx/logger/file_appender.h"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <string>
#include <unistd.h>

namespace lynx
{
thread_local char t_errnobuf[512];

char* getErrorInfo(int err)
{
	auto p = ::strerror_r(err, t_errnobuf, sizeof(t_errnobuf));
	return t_errnobuf;
}

FileAppender::FileAppender(const char* filename)
{
	init(filename);
}

FileAppender::~FileAppender()
{
	if (file_ != nullptr)
	{
		::fflush(file_);
		::fclose(file_);
	}
}

void FileAppender::append(const char* line, size_t len)
{
	size_t written = 0;

	while (written != len)
	{
		size_t remain = len - written;
		size_t n = write(line + written, remain);
		if (n != remain)
		{
			int err = ferror(file_);
			if (err)
			{
				fprintf(stderr, "AppendFile::append() failed %s\n",
						getErrorInfo(err));
				break;
			}
			if (n == 0)
			{
				throw std::runtime_error("write 出错，FILE*为空");
			}
		}
		written += n;
	}

	written_bytes_ += written;
}

void FileAppender::flush()
{
	if (file_)
	{
		::fflush(file_);
	}
}

size_t FileAppender::write(const char* line, size_t len)
{
	size_t sz = 0;
	if (file_)
	{
		sz = ::fwrite_unlocked(line, 1, len, file_);
	}
	return sz;
}

void FileAppender::init(const char* filename)
{
	auto filepos = ::strrchr(filename, '/');
	if (!filepos)
	{
		//      LB_TRACE_("无效的文件路径 {}", filename);
		throw std::runtime_error(std::string("invalid filepath ") +
								 std::string(filename));
	}
	const_cast<char*>(filepos)[0] = '\0';

	int ret = ::access(filename, F_OK);

	if (ret == -1)
	{
		throw std::runtime_error(std::string("file directory not exist: ") +
								 filename);
	}
	const_cast<char*>(filepos)[0] = '/';

	assert(const_cast<char*>(filepos)[0] != '\0');

	file_ = ::fopen(filename, "ae");
	if (file_ == nullptr)
	{
		int err = ferror(file_);
		auto p = ::strerror_r(err, t_errnobuf, sizeof(t_errnobuf));
		auto* errorInfo = t_errnobuf;
		fprintf(stderr, "FileAppender error in open file:%s erron:%s \r\n",
				filename, errorInfo);
		throw std::runtime_error("panic:FILE* is null");
	}
	std::setvbuf(file_, buffer_, _IOFBF, sizeof(buffer_));
}

} // namespace lynx
