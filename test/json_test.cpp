#include "lynx/json/array.hpp"
#include "lynx/json/element.hpp"
#include "lynx/json/object.hpp"
#include "lynx/json/parser.hpp"
#include "lynx/json/ref.hpp"
#include "lynx/json/tokenizer.hpp"
#include "lynx/json/value.hpp"
#include <cassert>
#include <iostream>
#include <string>

using namespace lynx;

int main()
{
	std::string json_str = R"({
		"name": "张三",
		"age": 28,
		"is_student": false,
		"height": 175.5,
		"address": {
			"city": "北京",
			"district": "朝阳区",
			"street": "建国路"
		},
		"hobbies": ["读书", "游泳", "编程"],
		"contact": {
			"email": "zhangsan@example.com",
			"phone": null
		},
		"scores": [
			{"subject": "数学", "score": 95},
			{"subject": "英语", "score": 88}
		]
	})";

	Tokenizer tokenizer(json_str);
	Ref root = make_object(
		{{"name", make_value("John")},
		 {"age", make_value(30)},
		 {"courses", make_array({make_value("C++"), make_value("Python")})},
		 {"address", make_value(nullptr)}});

	std::cout << root << std::endl;

	std::cout << root.serialize() << std::endl;

	delete root.get();
}