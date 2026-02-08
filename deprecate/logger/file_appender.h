#ifndef LYNX_FILE_APPENDER_H
#define LYNX_FILE_APPENDER_H

#include "lynx/base/common.hpp"
#include <cstddef>
#include <filesystem>
#include <memory>

namespace lynx
{

class FileAppender
{
  private:
	std::filesystem::path filepath_;
	std::unique_ptr<char[]> buffer_;
	FILE* file_{nullptr};
	size_t written_bytes_{0};
	static constexpr size_t BUFFER_SIZE = 64 * 1024;

  public:
	explicit FileAppender(const std::filesystem::path& filepath);
	~FileAppender();

	DISABLE_COPY(FileAppender)

	void append(const char* data, size_t len);
	void flush();

	[[nodiscard]] size_t writtenBytes() const noexcept
	{
		return written_bytes_;
	}
	void resetWritten() noexcept
	{
		written_bytes_ = 0;
	}

	[[nodiscard]] const std::filesystem::path& filepath() const noexcept
	{
		return filepath_;
	}

  private:
	void ensure_directory_exists() const;
};

} // namespace lynx

#endif // LYNX_FILE_APPENDER_H