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
	std::string json_str = R"JSON({
		"metadata": {
			"version": "2.1.0",
			"timestamp": "2026-02-25T10:30:00Z",
			"source": "Lynx JSON Test Suite",
			"tags": ["stress", "edge_cases", "ascii"],
			"flags": {
			"enable_logging": true,
			"max_retries": 5,
			"timeout": 30.5,
			"features": [null, "compression", "encryption"]
			}
		},
		"user": {
			"id": "usr_0a1b2c3d4e5f",
			"profile": {
			"username": "john_doe_123",
			"full_name": "John Doe",
			"age": 30,
			"gender": "male",
			"birthdate": "1996-04-23",
			"height_cm": 185.2,
			"weight_kg": 78.9,
			"is_active": true,
			"is_verified": false,
			"email": "john.doe@example.com",
			"phone_numbers": [
				{
				"type": "home",
				"number": "+1-555-1234",
				"primary": true
				},
				{
				"type": "work",
				"number": "+1-555-5678",
				"primary": false,
				"extension": 123
				},
				null
			],
			"addresses": {
				"home": {
				"street": "123 Main St",
				"city": "Springfield",
				"state": "IL",
				"zip": "62701",
				"country": "USA",
				"coordinates": {
					"latitude": 39.7817,
					"longitude": -89.6501,
					"accuracy": 5.5
				}
				},
				"work": null
			}
			},
			"preferences": {
			"theme": "dark",
			"language": "en-US",
			"notifications": {
				"email": true,
				"sms": false,
				"push": {
				"enabled": true,
				"sound": "chime.mp3",
				"vibrate": [200, 100, 200]
				}
			},
			"privacy": {
				"share_location": false,
				"show_online_status": true
			}
			}
		},
		"activity": {
			"last_login": "2026-02-25T08:15:22Z",
			"login_count": 128,
			"sessions": [
			{
				"id": "sess_001",
				"device": "iPhone 15 Pro",
				"os": "iOS 18.2",
				"ip": "192.168.1.100",
				"duration_sec": 1250,
				"pages_visited": ["/home", "/profile", "/settings"],
				"purchases": []
			},
			{
				"id": "sess_002",
				"device": "MacBook Pro",
				"os": "macOS 15.1",
				"ip": "10.0.0.5",
				"duration_sec": 3600,
				"pages_visited": ["/dashboard", "/reports"],
				"purchases": [
				{
					"item": "Laptop Stand",
					"price": 45.99,
					"currency": "USD",
					"date": "2026-02-24"
				}
				]
			}
			]
		},
		"skills": [
			{
			"name": "C++",
			"level": "expert",
			"years": 8,
			"keywords": ["templates", "STL", "multithreading"]
			},
			{
			"name": "Python",
			"level": "intermediate",
			"years": 4,
			"keywords": ["Django", "pandas", "asyncio"]
			},
			{
			"name": "JavaScript",
			"level": "advanced",
			"years": 5,
			"keywords": ["React", "Node.js", "TypeScript"]
			}
		],
		"projects": [
			{
			"title": "Lynx JSON Parser",
			"description": "A high-performance JSON parser in C++",
			"contributors": ["john_doe", "jane_smith"],
			"repo": "https://github.com/lynx/json-parser",
			"stars": 1234,
			"forks": 89,
			"issues": {
				"open": 12,
				"closed": 345,
				"labels": ["bug", "enhancement", "help wanted"]
			},
			"releases": [
				{
				"tag": "v1.0.0",
				"date": "2025-01-15",
				"assets": [
					{"name": "source.zip", "size": 102400},
					{"name": "binaries.tar.gz", "size": 512000}
				]
				},
				{
				"tag": "v2.0.0-beta",
				"date": "2026-02-01",
				"assets": []
				}
			]
			}
		],
		"comments": [
			{
			"id": 1,
			"author": "alice",
			"content": "Great project! Keep it up. :)",
			"likes": 25,
			"replies": [
				{
				"id": 2,
				"author": "bob",
				"content": "Thanks Alice! :)",
				"likes": 5,
				"replies": []
				}
			]
			},
			{
			"id": 3,
			"author": "charlie",
			"content": "Testing escaped characters: \" \\ \/ \b \f \n \r \t",
			"likes": 8,
			"replies": null
			}
		],
		"edge_cases": {
			"empty_object": {},
			"empty_array": [],
			"zero": 0,
			"negative_zero": -0,
			"large_number": 1.23456789e+100,
			"small_number": 1.23e-200,
			"max_int": 9223372036854775807,
			"min_int": -9223372036854775808,
			"boolean_true": true,
			"boolean_false": false,
			"null_value": null,
			"ascii_string": "Hello, world!",
			"escaped_string": "\" \\ \/ \b \f \n \r \t",
			"control_characters": "\u0000\u0001\u0002\u001F\u007F",
			"deeply_nested": {
			"level1": {
				"level2": {
				"level3": {
					"level4": {
					"level5": {
						"level6": {
						"level7": {
							"level8": {
							"level9": {
								"level10": "bottom"
							}
							}
						}
						}
					}
					}
				}
				}
			}
			},
			"array_with_mixed_types": [1, "two", false, null, {"key": "value"}, [3, 4]],
			"object_with_special_keys": {
			"": "empty key",
			" ": "space key",
			"with.dot": "dot in key",
			"with\"quote": "quote in key",
			"with\\backslash": "backslash in key",
			"with/ slash": "slash in key",
			"ascii_key": "ascii value"
			}
		},
		"statistics":
{
	"mean" : 42.195, "variance" : 17.3,
		"distribution"
		: [ [ 0.1, 0.2, 0.3 ], [ 0.4, 0.5, 0.6 ], [ 0.7, 0.8, 0.9 ] ],
		  "histogram":
	{
		"bins" : [ 10, 20, 30, 40 ], "counts" : [ 5, 15, 8, 2 ]
	}
}
})JSON";

	Tokenizer tokenizer(json_str);
	Ref root = Parser(&tokenizer).parse();
	// Ref root = make_object(
	// 	{{"name", make_value("John")},
	// 	 {"age", make_value(30)},
	// 	 {"courses", make_array({make_value("C++"), make_value("Python")})},
	// 	 {"address", make_value(nullptr)}});

	std::cout << root << std::endl;

	std::cout << root.serialize() << std::endl;

	delete root.get();
}