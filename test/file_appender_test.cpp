#include "lynx/logger/file_appender.h"
#include <iostream>

int main()
{

	lynx::FileAppender fa("log/stress.log");
	char line[] = "12345678901234567890\n";

	for (int i = 0; i < 10'000'000; ++i)
	{
		fa.append(line, sizeof(line) - 1);
		if (i % 1'000'000 == 0)
			std::cout << "Written: " << i << "\n";
	}

	fa.flush();
}