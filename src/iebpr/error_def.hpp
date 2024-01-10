#ifndef __IEBPR_ERROR_DEF_HPP__
#define __IEBPR_ERROR_DEF_HPP__

#include <cstdint>
#include "def.hpp"

namespace iebpr
{
	typedef enum : enum_base_t
	{
		none = 0x0,
		// Randomizer
		unexpected_rand_type = 0x100,
		normal_nonneg_lowprob,
		uniform_low_high_swap,
		uniform_nonneg_lowprob,
		bernoulli_error_mean,

		// SbrControll
		invalid_timestep = 0x200,
		invalid_init_volume,

		// AgentPool
		total_agent_mismatch_subtype_sum = 0x300,
		agent_subtype_pool_overlap,
		bool_trait_wrong_rand_type,

		// Recorder
		rec_time_exceed_simulation = 0x400,
		rec_step_smaller_than_timestep,

		// Simulation
		sigint = 0x500,

	} error_enum;

} // namespace iebpr

#endif