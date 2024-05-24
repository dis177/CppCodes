#ifndef __VAR_CONFIG_H__
#define __VAR_CONFIG_H__

#include <string>

// c++ fundamental types
#define __VAR_FUNDAMENTAL_TYPES__ \
	bool, char, short, int, long, long long, \
	unsigned char, unsigned short, unsigned int, unsigned long, unsigned long long, \
	float, double, long double

// operator types for binary operations
#define __VAR_OPBIN_TYPES__ \
	std::plus<>, std::minus<>, std::multiplies<>, std::divides<>, std::modulus<>, \
	std::equal_to<>, std::not_equal_to<>, std::greater<>, std::less<>, std::greater_equal<>, std::less_equal<>, \
	std::logical_and<>, std::logical_or<>, \
	std::bit_and<>, std::bit_or<>, std::bit_xor<>

namespace registry {
	typedef unsigned long long size_t;
	enum OPBINS : size_t {
		OP_PLUS, OP_MINUS, OP_MUL, OP_DIV, OP_MOD,
		OP_EQ, OP_NEQ, OP_GREATER, OP_LESS, OP_GEQ, OP_LEQ,
		OP_L_AND, OP_L_OR,
		OP_B_AND, OP_B_OR, OP_B_XOR
	};
}

namespace Var {
	class var;
	class list;
	extern const std::string type(const var& val);
}

using var = Var::var;
using list = Var::list;
using Var::type;

#endif // define __VAR_CONFIG_H__