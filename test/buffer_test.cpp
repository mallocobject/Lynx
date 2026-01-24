#include "lynx/include/buffer.h"
#include <cassert>
#include <iostream>
#include <string>

// 辅助宏，模拟简单的单元测试框架
#define EXPECT_EQ(val1, val2)                                                  \
	do                                                                         \
	{                                                                          \
		if ((val1) != (val2))                                                  \
		{                                                                      \
			std::cerr << "FAILED: " << #val1 << " == " << #val2                \
					  << " | Line: " << __LINE__ << " | Values: " << (val1)    \
					  << " vs " << (val2) << std::endl;                        \
		}                                                                      \
		else                                                                   \
		{                                                                      \
			std::cout << "PASSED: " << #val1 << " == " << #val2 << std::endl;  \
		}                                                                      \
	} while (0)

void testBufferBasic()
{
	std::cout << "\n--- Testing Basic Append & Retrieve ---" << std::endl;
	lynx::Buffer buf;

	EXPECT_EQ(buf.readableBytes(), 0);
	EXPECT_EQ(buf.writableBytes(), 1024 - 8); // DATA_SIZE - PREPEND_SIZE

	const std::string str = "Hello, Lynx!";
	buf.append(str.data(), str.size());

	EXPECT_EQ(buf.readableBytes(), str.size());

	std::string retrieved = buf.retrieveString(str.size());
	EXPECT_EQ(retrieved, str);
	EXPECT_EQ(buf.readableBytes(), 0);
	EXPECT_EQ(buf.prependableBytes(), 8 + str.size());
}

void testBufferExpansion()
{
	std::cout << "\n--- Testing Automatic Expansion ---" << std::endl;
	lynx::Buffer buf;

	// 写入一个超过初始 1024 字节的数据
	std::string large_str(2000, 'x');
	buf.append(large_str.data(), large_str.size());

	EXPECT_EQ(buf.readableBytes(), 2000);
	EXPECT_EQ(buf.retrieveString(2000), large_str);
	EXPECT_EQ(buf.readableBytes(), 0);
}

void testBufferMove()
{
	std::cout << "\n--- Testing Internal Data Moving (makeSpace) ---"
			  << std::endl;
	lynx::Buffer buf;

	// 1. 填充数据，占用大部分空间
	std::string part1(800, 'a');
	buf.append(part1.data(), part1.size());
	EXPECT_EQ(buf.readableBytes(), 800);

	// 2. 读取一部分数据，让 read_index 后移
	buf.retrieve(500);
	EXPECT_EQ(buf.readableBytes(), 300);
	EXPECT_EQ(buf.prependableBytes(), 8 + 500);

	// 3. 此时 writableBytes() 应该只有约 200 多字节
	//    尝试写入一个比剩余空间大、但比总空闲空间小的数据
	//    这将触发 makeSpace 里的数据搬移逻辑，而不是 resize
	std::string part2(400, 'b');
	buf.append(part2.data(), part2.size());

	EXPECT_EQ(buf.readableBytes(), 300 + 400);
	EXPECT_EQ(buf.retrieveString(300), std::string(300, 'a'));
	EXPECT_EQ(buf.retrieveString(400), part2);
}

void testBufferPrepend()
{
	std::cout << "\n--- Testing Prepend Logic ---" << std::endl;
	lynx::Buffer buf;

	std::string body = "World";
	buf.append(body.data(), body.size());

	// 在前面塞入 "Hello "
	std::string header = "Hello ";
	buf.prepend(header.data(), header.size());

	EXPECT_EQ(buf.readableBytes(), 11); // "Hello World"
	EXPECT_EQ(buf.retrieveString(11), "Hello World");
}

// 如果你添加了之前建议的 AppendInt32/RetrieveInt32，可以使用以下测试

void testBufferIntSerialization()
{
	std::cout << "\n--- Testing Integer Serialization ---" << std::endl;
	lynx::Buffer buf;

	buf.appendInt32(123456);
	EXPECT_EQ(buf.readableBytes(), 4);
	EXPECT_EQ(buf.retrieveInt32(), 123456);
}

int main()
{
	try
	{
		testBufferBasic();
		testBufferExpansion();
		testBufferMove();
		testBufferPrepend();
		testBufferIntSerialization();

		std::cout << "\nALL TESTS COMPLETED!" << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cerr << "Exception caught: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}