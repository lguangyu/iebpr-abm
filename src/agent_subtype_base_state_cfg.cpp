#include "iebpr/agent_subtype_base_state_cfg.hpp"

namespace iebpr
{
	void StateRandConfig::adjust_to_n_agent(size_t n_agent) noexcept
	{
		auto scale = (n_agent == 0) ? 1.0
									: stvalue_t(1.0) / n_agent;
		for (size_t i = 0; i < arr_size(); i++)
			// skip the rela_count field
			as_arr()[i]._scale = (i == (offsetof(AgentState, rela_count) / agent_field_size)) ? agent_subtype_consts::INIT_RELA_COUNT
																							  : scale;
		return;
	}

	void StateRandConfig::randomize(Randomizer &rand, AgentState &state)
	{
		for (size_t i = 0; i < arr_size(); i++)
			state.as_arr()[i] = rand.gen_value(as_arr()[i]);
		assert(state.rela_count == agent_subtype_consts::INIT_RELA_COUNT);
		// ensure split_biomass is above a value
		state.split_biomass = std::max(state.split_biomass,
									   state.biomass * agent_subtype_consts::MIN_RELA_SPLIT_BIOMASS);
		return;
	}

} // namespace iebpr