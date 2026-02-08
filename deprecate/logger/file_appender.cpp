#include "lynx/logger/file_appender.h"
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <system_error>

namespace lynx
{

namespace
{
void throw_system_error(const char* operation)
{
	throw std::system_error(errno, std::system_category(), operation);
}

void throw_runtime_error(std::string_view message)
{
	throw std::runtime_error(std::string(message));
}
} // namespace

FileAppender::FileAppender(const std::filesystem::path& filepath)
	: filepath_(filepath), buffer_(std::make_unique<char[]>(BUFFER_SIZE))
{
	ensure_directory_exists();

	file_ = ::fopen(filepath_.c_str(), "ab");
	if (!file_)
	{
		throw_system_error("Failed to open log file");
	}

	// 明确生效的缓冲（这是关键）
	if (::setvbuf(file_, buffer_.get(), _IOFBF, BUFFER_SIZE) != 0)
	{
		::fclose(file_);
		file_ = nullptr;
		throw_runtime_error("Failed to set file buffer");
	}
}

FileAppender::~FileAppender()
{
	if (file_)
	{
		::fflush(file_);
		::fclose(file_);
		file_ = nullptr;
	}
}

void FileAppender::append(const char* data, size_t len)
{
	if (!data || len == 0 || !file_)
	{
		return;
	}

	const size_t written = ::fwrite_unlocked(data, 1, len, file_);

	if (written != len)
	{
		if (::ferror(file_))
		{
			throw_system_error("Failed to write log file");
		}
		throw_runtime_error("Partial write to log file");
	}

	written_bytes_ += len;
}

void FileAppender::flush()
{
	if (!file_)
	{
		return;
	}

	if (::fflush(file_) != 0)
	{
		throw_system_error("Failed to flush log file");
	}
}

void FileAppender::ensure_directory_exists() const
{
	const auto parent_path = filepath_.parent_path();

	if (parent_path.empty())
	{
		// 当前目录，总是存在
		return;
	}

	std::error_code ec;
	const bool exists = std::filesystem::exists(parent_path, ec);

	if (ec)
	{
		throw_system_error("Failed to check directory existence");
	}

	if (!exists)
	{
		// 尝试创建目录
		if (!std::filesystem::create_directories(parent_path, ec) && ec)
		{
			throw_system_error("Failed to create log directory");
		}
	}

	// 验证目录是否可写
	if (!std::filesystem::is_directory(parent_path, ec) || ec)
	{
		throw_runtime_error("Log path is not a directory");
	}
}

} // namespace lynx