/**
 * Logger 使用示例
 * 
 * 演示两种日志输出模式：
 * 1. 仅控制台输出（默认）
 * 2. 异步文件输出
 */

#include "lynx/logger/logger.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace lynx;

int main()
{
	std::cout << "=== 日志系统演示 ===" << std::endl << std::endl;

	// ========== 模式一：仅输出到控制台 ==========
	std::cout << "【模式一】仅输出到控制台（无异步日志初始化）" << std::endl;
	std::cout << "----------------------------------------" << std::endl;

	LOG_INFO << "Application started";
	LOG_DEBUG << "Debug information: value=" << 42;
	LOG_WARN << "Warning: low memory";
	LOG_ERROR << "Error occurred: " << "connection timeout";

	std::cout << std::endl;

	// ========== 模式二：输出到文件 + 控制台 ==========
	std::cout << "【模式二】初始化异步日志，输出到文件" << std::endl;
	std::cout << "----------------------------------------" << std::endl;

	// 初始化异步日志
	Logger::initAsyncLogging(
		"./logs/app.log",       // 日志文件路径
		100 * 1024 * 1024,      // 100MB 滚动大小
		3                       // 3秒刷新间隔
	);

	LOG_INFO << "Async logging initialized";
	LOG_DEBUG << "Processing request: id=" << 12345;
	LOG_WARN << "Retry attempt 2/3";
	LOG_ERROR << "Database error: connection refused";

	// 模拟多线程日志
	std::thread t1([]() {
		LOG_INFO << "Thread 1: working...";
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		LOG_INFO << "Thread 1: done";
	});

	std::thread t2([]() {
		LOG_DEBUG << "Thread 2: processing...";
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		LOG_DEBUG << "Thread 2: finished";
	});

	t1.join();
	t2.join();

	std::cout << std::endl;

	// 关闭异步日志
	std::cout << "【关闭异步日志】等待所有日志写入..." << std::endl;
	Logger::shutdownAsyncLogging();

	std::cout << std::endl << "【模式一恢复】异步日志关闭后回到控制台输出" << std::endl;
	std::cout << "----------------------------------------" << std::endl;

	LOG_INFO << "Back to console output";
	LOG_ERROR << "Final error message";

	std::cout << std::endl << "演示完成！" << std::endl;
	std::cout << "日志文件位置：./logs/app.log" << std::endl;

	return 0;
}

/**
 * 输出说明
 * ========
 * 
 * 模式一输出（到控制台）：
 * [INFO] 2024-02-02 10:30:45 : Application started
 * [DEBUG] 2024-02-02 10:30:45 : Debug information: value=42
 * [WARN] 2024-02-02 10:30:45 : Warning: low memory
 * [ERROR] 2024-02-02 10:30:45 : Error occurred: connection timeout
 * 
 * 模式二输出（到文件）：
 * 日志内容会写入 ./logs/app.log，格式为：
 * 2024-02-02 10:30:45.123456 [12345] [INFO] logger.cpp:42 main() Async logging initialized
 * 2024-02-02 10:30:45.124001 [12345] [DEBUG] logger.cpp:50 main() Processing request: id=12345
 * 
 * 关键点：
 * 1. 模式一没有初始化异步日志，所以日志输出到控制台（带颜色和时间戳）
 * 2. 模式二初始化异步日志后，日志输出到文件（由 Formatter 统一格式化）
 * 3. 线程ID会自动缓存，避免重复系统调用
 * 4. 关闭异步日志前会等待所有缓冲的日志写入完成
 */
