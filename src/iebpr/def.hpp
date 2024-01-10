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

	/*
	template <typename T>
	inline stvalue_t *unsafe_as_stvalue_arr(T *obj)
	{
		return (stvalue_t *)obj;
	}

	template <typename T>
	inline stvalue_t *unsafe_as_stvalue_arr(T &obj)
	{
		return (stvalue_t *)(&obj);
	}
	*/

	template <typename T>
	constexpr size_t stvalue_arr_size(void)
	{
		static_assert(sizeof(T) % agent_field_size == 0, "converted type doesn't have a multiple size of stvalue_t/bivalue_t");
		return sizeof(T) / agent_field_size;
	}

} // namespace iebpr

#endif