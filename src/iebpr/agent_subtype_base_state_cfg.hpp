#ifndef __IEBPR_AGENT_SUBTYPE_BASE_STATE_CFG_HPP__
#define __IEBPR_AGENT_SUBTYPE_BASE_STATE_CFG_HPP__

#include "randomizer.hpp"
#include "agent_data.hpp"
#include "agent_subtype_consts.hpp"

namespace iebpr
{
	struct StateRandConfig
	{
		Randomizer::RandConfig biomass;
		Randomizer::RandConfig rela_count;
		Randomizer::RandConfig split_biomass;
		Randomizer::RandConfig glycogen;
		Randomizer::RandConfig pha;
		Randomizer::RandConfig polyp;
		// array-like access
		with_access_as_arr(StateRandConfig, Randomizer::RandConfig);

		explicit StateRandConfig(void) noexcept
		{
			// the rela_count is special
			rela_count.type = Randomizer::rand_t::constant;
			rela_count.mean = agent_subtype_consts::INIT_RELA_COUNT;
		};

		// apply adjust to n_agent
		void adjust_to_n_agent(size_t n_agent) noexcept;
		// generate set of values
		void randomize(Randomizer &rand, AgentState &state);
	};

// ensure StateRandConfig is aligned with AgentState field-wise
#define _align_static_assert(attr)                                                    \
	static_assert(offsetof(StateRandConfig, attr) / sizeof(Randomizer::RandConfig) == \
					  offsetof(AgentState, attr) / agent_field_size,                  \
				  #attr)
	_align_static_assert(biomass);
	_align_static_assert(rela_count);
	_align_static_assert(split_biomass);
	_align_static_assert(glycogen);
	_align_static_assert(pha);
	_align_static_assert(polyp);
#undef _align_static_assert

} // namespace iebpr

#endif