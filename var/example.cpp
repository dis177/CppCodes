struct myChar;

// c++ fundamental types are already registered
// use REGISTRY macros to register more types into type table
// registered types can be used as operator operand
// c++ 内置类型已经注册
// 使用 REGISTRY宏 注册更多类型到类型表
// 注册后的类型可以作为运算符操作数使用
#define REGISTRY Var::list, std::string, myChar

#include "var.h"

struct myChar {
	char _data;
	friend std::ostream& operator << (std::ostream& os, const myChar& Char) {
		return os << Char._data;
	}
	template <class _Tp>
	auto operator + (const _Tp& val) const
		requires (std::is_invocable_v<std::plus<>, char, _Tp>)
	{
		return _data + val;
	}
	friend std::string operator + (const std::string& str, const myChar& Char) {
		return str + Char._data;
	}
};

int main() {
	// var can be initialized or assigned with 'any' type, and can be used in operations
	// var 可以被'任何'类型初始化或赋值, 并且可以进行运算
	var formerVar, latterVar;
	formerVar = 1.123, latterVar = 10;
	std::cout << latterVar - formerVar << std::endl;

	// myChar is a user defined type
	// myChar 是一个用户自定义类型
	formerVar = "1234", latterVar = myChar{ '1' };

	std::cout << formerVar + latterVar << std::endl;

	// list type
	formerVar = { "12", '1' }, latterVar = { {3.11, '2'}, 4 };
	std::cout << type(formerVar) << ' ' << formerVar + latterVar << std::endl;
	latterVar = 2;
	std::cout << formerVar * latterVar << std::endl;
}