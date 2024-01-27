#ifndef __IEBPR_DEF_HPP__
#define __IEBPR_DEF_HPP__

#include <cassert>
#ifndef NDEBUG
#include <iostream>
#endif

#include <cstddef>
#include <cstdint>
#include <limits>

namespace iebpr
{
	// type check
	static_assert(sizeof(bool) == 1, "");
	static_assert(sizeof(int) == 4, "");
	static_assert(sizeof(long long) == 8, "");
	static_assert(sizeof(unsigned long long) == 8, "");
	static_assert(sizeof(double) == 8, "");
	static_assert(std::numeric_limits<double>::has_infinity == true, "");
	static_assert(std::numeric_limits<double>::has_quiet_NaN == true, "");

	// data type of float-point trait and state
	using stvalue_t = double;
	// data type of integer and bool trait and state
	using bivalue_t = long long;
	// ensure aligned
	static_assert(sizeof(stvalue_t) == sizeof(bivalue_t), "");
	// fixed-size enum
	using enum_base_t = uint32_t;

	constexpr size_t agent_field_size = sizeof(stvalue_t);
	constexpr stvalue_t stvalue_inf = std::numeric_limits<stvalue_t>::infinity();
	constexpr stvalue_t stvalue_nan = std::numeric_limits<stvalue_t>::quiet_NaN();

// this macro allow standard-layout struct with the same type of data (or types
// with the same size) to be accessed as an array
// S::arr_size() returns the size of the as-array
// S::as_arr() returns the pointer to the first element of the as-array
#ifndef with_access_as_arr
#define with_access_as_arr(S, T)                                                             \
	static constexpr size_t arr_size(void) noexcept                                          \
	{                                                                                        \
		static_assert(sizeof(S) % sizeof(T) == 0, "S doesn't have a size in multiple of T"); \
		return sizeof(S) / sizeof(T);                                                        \
	};                                                                                       \
	T *as_arr(void) noexcept { return reinterpret_cast<T *>(this); };                        \
	const T *as_arr(void) const noexcept { return reinterpret_cast<const T *>(this); };
#endif

	/*
	template <typename struct_t, typename element_t>
	struct ArrAccessWrap
	{
		using struct_t::struct_t;
		using struct_t::~struct_t;

		static size_t arr_size(void) noexcept
		{
			static_assert(sizeof(struct_t) % sizeof(element_t) == 0, "struct_t doesn't have a size in multiple of element_t");
			return sizeof(struct_t) / sizeof(element_t);
		};

		element_t *as_arr(void) noexcept { return reinterpret_cast<element_t *>(this); };
		const element_t *as_arr(void) const noexcept { return reinterpret_cast<const element_t *>(this); };
	};
	*/

} // namespace iebpr

#endif