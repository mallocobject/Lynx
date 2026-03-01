#include "email.hpp"
#include <cstring>
#include <format>
#include <random>
#include <string>

// 邮件内容回调函数：libcurl 会反复调用此函数来获取要发送的数据
static size_t read_callback(char* ptr, size_t size, size_t nmemb, void* userp)
{
	std::string* data = static_cast<std::string*>(userp);
	size_t room = size * nmemb;

	if (data->empty())
		return 0;

	size_t copy_len = std::min(room, data->size());
	memcpy(ptr, data->c_str(), copy_len);
	data->erase(0, copy_len);

	return copy_len;
}

CURLcode mail(const std::string& target_email, const std::string& verify_code)
{
	CURL* curl;
	CURLcode res = CURLE_OK;

	// 发件人、收件人（可多个）
	const std::string from = "lynxqqcom@qq.com";
	// const std::vector<std::string> recipients = {"lynxqqcom@qq.com"};

	// 邮件内容（符合RFC 2822格式，需要包含必要的头部）
	// "From: Test <lynxqqcom@qq.com>\r\n"
	// "To: <lynxqqcom@qq.com>\r\n"
	// "Subject: Test email from C++ libcurl\r\n"
	// "\r\n" // 空行分隔头部和正文
	// "Hello, this is a test email sent via C++ libcurl using QQ SMTP.\r\n"
	// "--\r\n"
	// "Sent by libcurl\r\n";

	std::string email_payload =
		std::format("From: Lynx <lynxqqcom@qq.com>\r\n"
					"To: <{}>\r\n"
					"Subject: [Lynx] 您的邮箱验证码\r\n"
					"\r\n"
					"亲爱的用户：\r\n"
					"\r\n"
					"您正在 Lynx 平台进行验证操作，这是您的验证码：\r\n"
					"\r\n"
					"      {}      \r\n"
					"\r\n"
					"(验证码 5 分钟内有效)\r\n"
					"\r\n"
					"如果这不是您本人的操作，请忽略此邮件。\r\n"
					"\r\n"
					"------------------\r\n"
					"Lynx Team\r\n",
					target_email, verify_code);

	// SMTP 服务器配置
	const std::string smtp_server =
		"smtp://smtp.qq.com:587"; // 使用587端口，支持STARTTLS
	const std::string username = "lynxqqcom@qq.com"; // 邮箱账号
	// const std::string password = ""; // QQ邮箱授权码

	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();

	if (curl)
	{
		// 基本URL
		curl_easy_setopt(curl, CURLOPT_URL, smtp_server.c_str());
		// curl_easy_setopt(curl, CURLOPT_PORT, 587L);
		// 启用详细输出（调试用）
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);

		// 设置认证信息（使用LOGIN方式）
		curl_easy_setopt(curl, CURLOPT_USERNAME, username.c_str());
		curl_easy_setopt(curl, CURLOPT_PASSWORD, password.c_str());
		curl_easy_setopt(curl, CURLOPT_LOGIN_OPTIONS, "AUTH=LOGIN");

		// 启用STARTTLS
		curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

		// 设置发件人
		curl_easy_setopt(curl, CURLOPT_MAIL_FROM, from.c_str());

		// 设置收件人列表
		struct curl_slist* recipients_list = nullptr;
		recipients_list =
			curl_slist_append(recipients_list, target_email.c_str());

		curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients_list);

		// 设置邮件数据回调
		curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
		curl_easy_setopt(curl, CURLOPT_READDATA, &email_payload);
		curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L); // 表示要上传数据

		// 执行传输
		res = curl_easy_perform(curl);

		// // 检查结果
		// if (res != CURLE_OK)
		// {
		// 	LOG_WARN << "curl_easy_perform() failed: "
		// 			 << curl_easy_strerror(res);
		// }
		// else
		// {
		// 	std::cout << "Email sent successfully!" << std::endl;
		// }

		// 清理
		curl_slist_free_all(recipients_list);
		curl_easy_cleanup(curl);
	}

	curl_global_cleanup();
	return res;
}

std::string generateVerificationCode()
{
	thread_local static std::random_device rd;
	thread_local static std::mt19937 gen(rd());
	thread_local static std::uniform_int_distribution<int> dist(0, 999999);

	int code = dist(gen);
	return std::format("{:06}", code);
}