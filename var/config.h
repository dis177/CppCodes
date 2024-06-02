#ifndef __VAR_CONFIG_H__
#define __VAR_CONFIG_H__

#include "var/ops.h"
#include "var/types.h"
#include <string>

// c++ fundamental types
#define __VAR_FUNDAMENTAL_TYPES__ \
	bool, char, short, int, long, long long, \
	unsigned char, unsigned short, unsigned int, unsigned long, unsigned long long, \
	float, double, long double

// operator types for binary operations
#define __VAR_OPBIN_TYPES__ \
	registry::ops::Plus, registry::ops::Minus, registry::ops::Multiplies, \
	registry::ops::Divides, registry::ops::Modulus, registry::ops::Equal_to, \
	registry::ops::Not_equal_to, registry::ops::Greater, registry::ops::Less, \
	registry::ops::Greater_equal, registry::ops::Less_equal, registry::ops::Logical_and, \
	registry::ops::Logical_or, registry::ops::Bit_and, registry::ops::Bit_or, \
	registry::ops::Bit_xor, registry::ops::Left_shift, registry::ops::Right_shift, \
	registry::ops::Bracket

#define __VAR_OPUNA_TYPES__ \
	registry::ops::Logical_not, registry::ops::Bit_not, registry::ops::Pre_increment, \
	registry::ops::Post_increment, registry::ops::Pre_decrement, registry::ops::Post_decrement, \
	registry::ops::Unary_plus, registry::ops::Unary_minus, registry::ops::Dereference

namespace registry {
	typedef unsigned long long size_t;
	enum OPBINS : size_t {
		OP_PLUS, OP_MINUS, OP_MUL, OP_DIV, OP_MOD,
		OP_EQ, OP_NEQ, OP_GREATER, OP_LESS, OP_GEQ, OP_LEQ,
		OP_LAND, OP_LOR, OP_BAND, OP_BOR, OP_BXOR, OP_LSHIFT, OP_RSHIFT,
		OP_BRACKET
	};
	enum OPUNAS : size_t {
		OP_LNOT, OP_BNOT, OP_PREINCREMENT, OP_POSTINCREMENT,
		OP_PREDECREMENT, OP_POSTDECREMENT, OP_UPLUS, OP_UMINUS,
		OP_DEFERENCE
	};
}



namespace Var {
	class var;
	class list;
	struct NONE_TYPE;
	typedef const NONE_TYPE* None_t;
	extern const std::string type(const var& val);
}

using var = Var::var;
using list = Var::list;
using Var::type;

#endif // define __VAR_CONFIG_H__