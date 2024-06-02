#ifndef __REGISTRY_OPS_H__
#define __REGISTRY_OPS_H__

#include <type_traits>
#include <utility>

#define create_has_opbins(NAME, OP) \
    template <class Tf, class Tl>\
    struct has_operator_##NAME {\
    private:\
      template <class Uf, class Ul>\
      static auto check(Uf* ptrf, Ul* ptrl) -> decltype((*ptrf) OP (*ptrl), std::true_type{});\
      template <class Uf, class Ul>\
      static std::false_type check(...);\
    public:\
      static constexpr bool value = decltype(check<Tf, Tl>(nullptr, nullptr))::value;\
    }

#define create_has_opunas(NAME, op) \
		template <class T>\
		struct has_operator_##NAME {\
		private:\
			template <class U>\
			static auto check(U* ptru) -> decltype(op (*ptru), std::true_type{});\
			template <class U>\
			static std::false_type check(...);\
		public:\
			static constexpr bool value = decltype(check<T>(nullptr))::value;\
		}

namespace registry {
	namespace ops {
		create_has_opbins(Plus, +);
		create_has_opbins(Minus, -);
		create_has_opbins(Multiplies, *);
		create_has_opbins(Divides, /);
		create_has_opbins(Modulus, %);
		create_has_opbins(Equal_to, ==);
		create_has_opbins(Not_equal_to, !=);
		create_has_opbins(Greater, >);
		create_has_opbins(Less, <);
		create_has_opbins(Greater_equal, >=);
		create_has_opbins(Less_equal, <=);
		create_has_opbins(Logical_and, &&);
		create_has_opbins(Logical_or, ||);
		create_has_opbins(Bit_and, &);
		create_has_opbins(Bit_or, |);
		create_has_opbins(Bit_xor, ^);
		create_has_opbins(Left_shift, <<);
		create_has_opbins(Right_shift, >>);
		

		create_has_opunas(Logical_not, !);
		create_has_opunas(Bit_not, ~);
		create_has_opunas(Pre_increment, ++);
		create_has_opunas(Pre_decrement, --);
		create_has_opunas(Unary_plus, +);
		create_has_opunas(Unary_minus, -);
		create_has_opunas(Dereference, *);

		template <class T>
		struct has_operator_Post_increment {
		private:
			template <class U>
			static auto check(U* ptru) -> decltype((*ptru)++, std::true_type{});
			template <class U>
			static std::false_type check(...);
		public:
			static constexpr bool value = decltype(check<T>(nullptr))::value;
		};

		template <class T>
		struct has_operator_Post_decrement {
		private:
			template <class U>
			static auto check(U* ptru) -> decltype((*ptru)--, std::true_type{});
			template <class U>
			static std::false_type check(...);
		public:
			static constexpr bool value = decltype(check<T>(nullptr))::value;
		};


		template <class _Call, class... _Param>
		struct has_operator_Parenthesis {
		private:
			template <class _CALL, class... _PARAM>
			static auto check(int) -> decltype((*(_CALL*)(nullptr))((*(_PARAM)(nullptr))...), std::true_type{});
			template <class _CALL, class... _PARAM>
			static std::false_type check(...);
		public:
			static constexpr bool value = decltype(check<_Call, _Param...>(0))::value;
		};

		template <class _Call, class _Param>
		struct has_operator_Bracket {
		private:
			template <class _CALL, class _PARAM>
			static auto check(int) -> decltype(_Call{}[_PARAM{}], std::true_type{});
			template <class _CALL, class _PARAM>
			static std::false_type check(...);
		public:
			static constexpr bool value = decltype(check<_Call, _Param>(0))::value;
		};

		struct Plus {
		public:
			template <class _Left, class _Right>
			constexpr decltype(auto) operator()(_Left&& lhs, _Right&& rhs) const 
			requires (has_operator_Plus<std::remove_reference_t<_Left>, std::remove_reference_t<_Right>>::value)
			{
				return lhs + rhs;
			}
		};

		struct Minus {
		public:
			template <class _Left, class _Right>
			constexpr decltype(auto) operator()(_Left&& lhs, _Right&& rhs) const
				requires (has_operator_Minus<std::remove_reference_t<_Left>, std::remove_reference_t<_Right>>::value)
			{
				return lhs - rhs;
			}
		};

		struct Multiplies {
		public:
			template <class _Left, class _Right>
			constexpr decltype(auto) operator()(_Left&& lhs, _Right&& rhs) const
				requires (has_operator_Multiplies<std::remove_reference_t<_Left>, std::remove_reference_t<_Right>>::value)
			{
				return lhs * rhs;
			}
		};

		struct Divides {
		public:
			template <class _Left, class _Right>
			constexpr decltype(auto) operator()(_Left&& lhs, _Right&& rhs) const
				requires (has_operator_Divides<std::remove_reference_t<_Left>, std::remove_reference_t<_Right>>::value)
			{
				return lhs / rhs;
			}
		};

		struct Modulus {
		public:
			template <class _Left, class _Right>
			constexpr decltype(auto) operator()(_Left&& lhs, _Right&& rhs) const
				requires (has_operator_Modulus<std::remove_reference_t<_Left>, std::remove_reference_t<_Right>>::value)
			{
				return lhs % rhs;
			}
		};

		struct Equal_to {
		public:
			template <class _Left, class _Right>
			constexpr decltype(auto) operator()(_Left&& lhs, _Right&& rhs) const
				requires (has_operator_Equal_to<std::remove_reference_t<_Left>, std::remove_reference_t<_Right>>::value)
			{
				return lhs == rhs;
			}
		};

		struct Not_equal_to {
		public:
			template <class _Left, class _Right>
			constexpr decltype(auto) operator()(_Left&& lhs, _Right&& rhs) const
				requires (has_operator_Not_equal_to<std::remove_reference_t<_Left>, std::remove_reference_t<_Right>>::value)
			{
				return lhs != rhs;
			}
		};

		struct Greater {
		public:
			template <class _Left, class _Right>
			constexpr decltype(auto) operator()(_Left&& lhs, _Right&& rhs) const
				requires (has_operator_Greater<std::remove_reference_t<_Left>, std::remove_reference_t<_Right>>::value)
			{
				return lhs > rhs;
			}
		};

		struct Less {
		public:
			template <class _Left, class _Right>
			constexpr decltype(auto) operator()(_Left&& lhs, _Right&& rhs) const
				requires (has_operator_Less<std::remove_reference_t<_Left>, std::remove_reference_t<_Right>>::value)
			{
				return lhs < rhs;
			}
		};

		struct Greater_equal {
		public:
			template <class _Left, class _Right>
			constexpr decltype(auto) operator()(_Left&& lhs, _Right&& rhs) const
				requires (has_operator_Greater_equal<std::remove_reference_t<_Left>, std::remove_reference_t<_Right>>::value)
			{
				return lhs >= rhs;
			}
		};

		struct Less_equal {
		public:
			template <class _Left, class _Right>
			constexpr decltype(auto) operator()(_Left&& lhs, _Right&& rhs) const
				requires (has_operator_Less_equal<std::remove_reference_t<_Left>, std::remove_reference_t<_Right>>::value)
			{
				return lhs <= rhs;
			}
		};

		struct Logical_and {
		public:
			template <class _Left, class _Right>
			constexpr decltype(auto) operator()(_Left&& lhs, _Right&& rhs) const
				requires (has_operator_Logical_and<std::remove_reference_t<_Left>, std::remove_reference_t<_Right>>::value)
			{
				return lhs && rhs;
			}
		};

		struct Logical_or {
		public:
			template <class _Left, class _Right>
			constexpr decltype(auto) operator()(_Left&& lhs, _Right&& rhs) const
				requires (has_operator_Logical_or<std::remove_reference_t<_Left>, std::remove_reference_t<_Right>>::value)
			{
				return lhs || rhs;
			}
		};

		struct Bit_and {
		public:
			template <class _Left, class _Right>
			constexpr decltype(auto) operator()(_Left&& lhs, _Right&& rhs) const
				requires (has_operator_Bit_and<std::remove_reference_t<_Left>, std::remove_reference_t<_Right>>::value)
			{
				return lhs & rhs;
			}
		};

		struct Bit_or {
		public:
			template <class _Left, class _Right>
			constexpr decltype(auto) operator()(_Left&& lhs, _Right&& rhs) const
				requires (has_operator_Bit_or<std::remove_reference_t<_Left>, std::remove_reference_t<_Right>>::value)
			{
				return lhs | rhs;
			}
		};

		struct Bit_xor {
		public:
			template <class _Left, class _Right>
			constexpr decltype(auto) operator()(_Left&& lhs, _Right&& rhs) const
				requires (has_operator_Bit_xor<std::remove_reference_t<_Left>, std::remove_reference_t<_Right>>::value)
			{
				return lhs > rhs;
			}
		};

		struct Left_shift {
		public:
			template <class _Left, class _Right>
			constexpr decltype(auto) operator()(_Left&& lhs, _Right&& rhs) const
				requires (has_operator_Left_shift<std::remove_reference_t<_Left>, std::remove_reference_t<_Right>>::value)
			{
				return lhs << rhs;
			}
		};

		struct Right_shift {
		public:
			template <class _Left, class _Right>
			constexpr decltype(auto) operator()(_Left&& lhs, _Right&& rhs) const
				requires (has_operator_Right_shift<std::remove_reference_t<_Left>, std::remove_reference_t<_Right>>::value)
			{
				return lhs >> rhs;
			}
		};

		struct Logical_not {
		public:
			template <class _Arg>
			constexpr decltype(auto) operator()(_Arg&& arg) const
			requires (has_operator_Logical_not<std::remove_reference_t<_Arg>>::value)
			{
				return !arg;
			}
		};

		struct Bit_not {
		public:
			template <class _Arg>
			constexpr decltype(auto) operator()(_Arg&& arg) const
				requires (has_operator_Bit_not<std::remove_reference_t<_Arg>>::value)
			{
				return ~arg;
			}
		};

		struct Pre_increment {
		public:
			template <class _Arg>
			constexpr decltype(auto) operator()(_Arg&& arg) const
				requires (has_operator_Pre_increment<std::remove_reference_t<_Arg>>::value)
			{
				return ++arg;
			}
		};

		struct Post_increment {
		public:
			template <class _Arg>
			constexpr decltype(auto) operator()(_Arg&& arg) const
				requires (has_operator_Post_increment<std::remove_reference_t<_Arg>>::value)
			{
				return arg++;
			}
		};

		struct Pre_decrement {
		public:
			template <class _Arg>
			constexpr decltype(auto) operator()(_Arg&& arg) const
				requires (has_operator_Pre_decrement<std::remove_reference_t<_Arg>>::value)
			{
				return --arg;
			}
		};

		struct Post_decrement {
		public:
			template <class _Arg>
			constexpr decltype(auto) operator()(_Arg&& arg) const
				requires (has_operator_Post_decrement<std::remove_reference_t<_Arg>>::value)
			{
				return arg--;
			}
		};

		struct Unary_plus {
		public:
			template <class _Arg>
			constexpr decltype(auto) operator()(_Arg&& arg) const
				requires (has_operator_Unary_plus<std::remove_reference_t<_Arg>>::value)
			{
				return +arg;
			}
		};

		struct Unary_minus {
		public:
			template <class _Arg>
			constexpr decltype(auto) operator()(_Arg&& arg) const
				requires (has_operator_Unary_minus<std::remove_reference_t<_Arg>>::value)
			{
				return -arg;
			}
		};

		struct Dereference {
		public:
			template <class _Arg>
			constexpr decltype(auto) operator()(_Arg&& arg) const
				requires (has_operator_Dereference<std::remove_reference_t<_Arg>>::value)
			{
				return *arg;
			}
		};

		struct Parenthesis {
		public:
			template <class _Call, class... _Param>
			constexpr decltype(auto) operator()(_Call&& _call, _Param&&... _params) const
			requires (has_operator_Parenthesis<std::remove_reference_t<_Call>, std::remove_reference_t<_Param>...>::value)
			{
				return _call(std::forward<_Param>(_params)...);
			}
		};

		struct Bracket {
		public:
			template <class _Call, class _Param>
			constexpr decltype(auto) operator()(_Call&& _call, _Param&& _params) const
				requires (has_operator_Bracket <std::remove_reference_t<_Call>, std::remove_reference_t<_Param>>::value)
			{
				return _call[_params];
			}
		};

	}
}

#ifdef create_has_opbins
#undef create_has_opbins
#endif

#ifdef create_has_opunas
#undef create_has_opunas
#endif


#endif