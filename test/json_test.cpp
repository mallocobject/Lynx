#include "lynx/json/array.hpp"
#include "lynx/json/element.hpp"
#include "lynx/json/object.hpp"
#include "lynx/json/value.hpp"
#include <cassert>
#include <string>

using namespace lynx;

int main()
{
	Element* Null = new Value;
	Element* Int = new Value(5);
	Element* Float = new Value(1.50);
	Element* String = new Value("Hello World");
	Element* Boolean = new Value(true);

	assert(Null->isValue());

	Array* arr = new Array;
	arr->append(85);
	arr->append(5.4);

	Object obj;
	obj["key"] = Int;

	delete Null;
	// delete Int;
	delete Float;
	delete String;
	delete Boolean;
	delete arr;
}