#include <utility>
#include <cstring>
#include "iebpr/agent_subtype_base.hpp"

namespace iebpr
{
	AgentSubtypeBase::~AgentSubtypeBase(void) noexcept
	{
		return;
	}

	AgentSubtypeBase::subtype_enum AgentSubtypeBase::subtype(void) const noexcept
	{
		return none;
	}

	bool AgentSubtypeBase::has_data_overlap_with(const AgentSubtypeBase &other) const noexcept
	{
		assert(this != &other);
		return ((this->pool_begin() < other.pool_end()) &&
				(other.pool_begin() < this->pool_end()));
	}

	using agent_itr_t = AgentSubtypeBase::agent_itr_t;

	void _sort_biomass_ascend(agent_itr_t itrs[2]) noexcept
	{
		if (itrs[0]->state.biomass > itrs[1]->state.biomass)
			return;
		std::swap(itrs[0], itrs[1]);
		return;
	}

	AgentSubtypeBase::SubtypeSizeReport
	AgentSubtypeBase::report_subtype_and_size(void) const noexcept
	{
		SubtypeSizeReport ret = {subtype(), n_agent};
		return ret;
	}

	void AgentSubtypeBase::state_cfg_apply_num_adjust(void) noexcept
	{
		state_cfg.adjust_to_n_agent(n_agent);
		return;
	}

	void AgentSubtypeBase::trait_cfg_apply_rate_adjust(stvalue_t timestep) noexcept
	{
		trait_cfg.adjust_to_timestep(timestep);
		return;
	}

	void AgentSubtypeBase::instantiate_agents(void)
	{
		for (agent_itr_t itr = pool_begin(); itr < pool_end(); itr++)
		{
			state_cfg.randomize(_rand, itr->state);
			trait_cfg.randomize(_rand, itr->trait);
		}
		return;
	}

	void AgentSubtypeBase::agent_action_aerobic(const EnvState &env, EnvState &d_env, AgentData &agent)
	{
		return;
	}

	void AgentSubtypeBase::agent_action_anaerobic(const EnvState &env, EnvState &d_env, AgentData &agent)
	{
		return;
	}

	void AgentSubtypeBase::agent_split(agent_itr_t agent_itr)
	{
		if (n_agent <= 1)
			return;

		// update the splitting agent state, except split_biomass and rela_count
		auto sb = agent_itr->state.split_biomass;
		auto rc = agent_itr->state.rela_count;
		agent_itr->state.scale_state_content(0.5);
		agent_itr->state.split_biomass = sb;
		agent_itr->state.rela_count = rc;

		// copy the state, will be the splitted agent state
		AgentState split_state = agent_itr->state;

		// find the two agents with lowest biomass and merge
		// when reaching here we have at least two agents
		agent_itr_t to_merge_itrs[2] = {pool_begin(), pool_begin() + 1};
		_sort_biomass_ascend(to_merge_itrs);
		for (agent_itr_t itr = pool_begin() + 2; itr < pool_end(); itr++)
			if (itr->state.biomass < to_merge_itrs[1]->state.biomass)
			{
				to_merge_itrs[1] = itr;
				_sort_biomass_ascend(to_merge_itrs);
			}
		// merge the two
		to_merge_itrs[1]->merge_with(*to_merge_itrs[0]);
		// the merged one is used for the new split agent
		to_merge_itrs[0]->state = split_state;
		// trait of the new split will be randomized (approximate mutation (?))
		trait_cfg.randomize(_rand, to_merge_itrs[0]->trait);
		return;
	}

	AgentState AgentSubtypeBase::summarize_agent_state_cont(void) const noexcept
	{
		AgentState ret = AgentState();

		for (auto itr = pool_begin(); itr < pool_end(); itr++)
			ret.merge_with(itr->state, true);

		return ret;
	}

	bool AgentSubtypeBase::is_valid_subtype_enum(subtype_enum subtype, bool allow_none)
	{
		switch (subtype)
		{
		case none:
			return allow_none;
		case pao:
			return true;
		case gao:
			return true;
		case oho:
			return true;
		case invalid:
		default:
			return false;
		}
	}

	const char *AgentSubtypeBase::subtype_enum_to_name(subtype_enum subtype) noexcept
	{
		switch (subtype)
		{
		case none:
			return "none";
		case pao:
			return "pao";
		case gao:
			return "gao";
		case oho:
			return "oho";
		default:
			return "invalid";
		}
	}

} // namespace iebpr