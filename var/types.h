#ifndef __REGISTRY_TYPES_H__
#define __REGISTRY_TYPES_H__

#include <iostream>
#include <type_traits>
#include "var/ops.h"

namespace Var {
	template <class _Tp>
	struct var_value;
	template <class _Tp>
	struct var_ref;

	struct none_t {
		constexpr none_t() = default;
		constexpr none_t(const none_t&) = delete;
		constexpr none_t(none_t&&) = delete;
		constexpr none_t& operator=(const none_t&) = delete;
		constexpr none_t& operator=(none_t&&) = delete;
		constexpr bool operator==(const none_t&) const {
			return true;
		}
		template <class _Tp>
		constexpr bool operator==(const _Tp&) const {
			return false;
		}
		constexpr bool operator!=(const none_t&) const {
			return true;
		}
		template <class _Tp>
		constexpr bool operator!=(const _Tp&) const {
			return false;
		}
		template <class _Tp>
			requires (!std::is_same_v<std::remove_reference_t<std::remove_cv_t<_Tp>>, none_t> )
		friend bool operator == (const _Tp&, const none_t&) {
			return false;
		}
		friend std::ostream& operator<<(std::ostream& os, const none_t&) {
			return os << "None";
		}
	};
	static constexpr none_t None{};
}

namespace registry {
	template <class _Tp>
	using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<_Tp>>;

	template <class... Args>
	static constexpr size_t types_count = sizeof...(Args);


	template <size_t N, class _First, class... _Rest>
	struct types_visitor {
		using type = types_visitor<N - 1, _Rest...>::type;
	};

	template <class _First, class... _Rest>
	struct types_visitor<0, _First, _Rest...> {
		using type = _First;
	};

	template <size_t N, class... Args>
	using types_visitor_t = typename types_visitor<N, Args...>::type;

	/*template <class _Tp>
	struct is_convertible_to_holder {
	private:

		template <class U>
		static constexpr auto check(int) -> decltype(std::declval<decltype(Var::var_holder(std::declval<U>()))>(), std::true_type{});
		template <class U>
		static constexpr auto check(...) -> std::false_type;
	public:
		static constexpr bool value = decltype(check<_Tp>(0))::value;
	};*/

	template <class _Tp>
	using is_convertible_to_value = std::conditional_t<!std::is_array_v<_Tp> && !std::is_copy_constructible_v<_Tp>, std::false_type, std::true_type>;
};


#endif