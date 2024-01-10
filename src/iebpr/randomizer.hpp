#ifndef __IEBPR_RANDOMIZER_HPP__
#define __IEBPR_RANDOMIZER_HPP__

#include <random>
#include <vector>
#include "def.hpp"
#include "error_def.hpp"

namespace iebpr
{
	class Randomizer
	{
	public:
		using rand_t = enum : enum_base_t {
			none = 0,  // always return 0
			constant,  // always return mean@RandConfig
			normal,	   // normal distrib, use mean,stddev@RandConfig
			uniform,   // uniform distrib, use low,high@RandConfig
			bernoulli, // 0 or 1, expected value is mean@RandConfig
			obsvalues, // sample from observed value list and normalize by mean
			// invalid flag
			invalid = 0xffffffff,
		};

		//======================================================================
		// EXTERNAL API / STATIC AGENT SUBTYPE CONVERSION
		//======================================================================
		// these functions need to be updated if more random types are added

		// check if random type enum value is recognized
		static bool is_valid_randtype_enum(rand_t type) noexcept;
		// interpret random type enum value to string
		static const char *randtype_enum_to_name(rand_t type) noexcept;
		// translate str to random type enum, returns rand_t::invalid on failure

		struct RandConfig
		{
		public:
			rand_t type;					   // type of distribution
			stvalue_t mean;					   // constant, normal, bernoulli, obsvalues
			stvalue_t stddev;				   // normal
			stvalue_t low;					   // uniform
			stvalue_t high;					   // uniform
			std::vector<stvalue_t> value_list; // obsvalues, must be sorted in asending order
			uint32_t non_neg;				   // bool; non-zero to ensure non-negativity; only works on type normal and uniform;
			stvalue_t _scale;				   // scale the result value, should only be used internally; 1.0 = noscaling
											   // scale will only be applied to constant/normal/uniform/obsvalues results

			explicit RandConfig(void) noexcept
				: type(none), mean(0), low(0), high(0), value_list(0), non_neg(1), _scale(1){};

			// set value_list, ensure sorted
			void set_value_list(const decltype(value_list) &values);
			// validate randomizer settings
			error_enum validate(rand_t force_type = invalid) const noexcept;
		};

		std::default_random_engine engine;
		std::uniform_real_distribution<stvalue_t> uniform_gen;
		std::normal_distribution<stvalue_t> normal_gen;

		using seed_t = decltype(engine)::result_type;

		explicit Randomizer(seed_t seed = 0) noexcept
			: engine(seed), uniform_gen(0., 1.), normal_gen(0., 1.){};

		//======================================================================
		// INTERNAL API
		//======================================================================

		// set engine state
		void seed(seed_t seed) noexcept;
		// generate a random value using config
		stvalue_t gen_value(const RandConfig &cfg);

	private:
		stvalue_t _obsvalues_gen_handler(const RandConfig &cfg);
	};

} // namespace iebpr

#endif