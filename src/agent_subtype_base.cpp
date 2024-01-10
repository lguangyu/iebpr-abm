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

	void _sort_by_biomass(agent_itr_t itrs[2]) noexcept
	{
		if (itrs[0]->state.s.biomass <= itrs[1]->state.s.biomass)
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

	void AgentSubtypeBase::trait_cfg_apply_rate_adjust(stvalue_t timestep)
	{
		// refresh all rate scale to 1
		for (auto &i : trait_cfg.arr)
			i._scale = 1;
		// apply timestep as scale to these specific rate configs
		for (auto i = AgentTrait::rate_arr_begin; i < AgentTrait::rate_arr_end; i++)
			trait_cfg.arr[i]._scale = timestep;
		return;
	}

	void AgentSubtypeBase::instantiate_agents(void)
	{
		for (agent_itr_t itr = pool_begin(); itr < pool_end(); itr++)
		{
			_randomize_agent_state(itr->state);
			_randomize_agent_trait(itr->trait);
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

		// update the splitting agent state, except split_biomass
		auto t = agent_itr->state.s.split_biomass;
		agent_itr->state.scale_state_content(0.5);
		agent_itr->state.s.split_biomass = t;

		// copy the state, will be the splitted agent state
		AgentState split_state = agent_itr->state;

		// find the two agents with lowest biomass and merge
		// when reaching here we have at least two agents
		agent_itr_t merge_itrs[2] = {pool_begin(), pool_begin() + 1};
		_sort_by_biomass(merge_itrs);
		for (agent_itr_t itr = pool_begin() + 2; itr < pool_end(); itr++)
			if (itr->state.s.biomass < merge_itrs[1]->state.s.biomass)
			{
				merge_itrs[1] = itr;
				_sort_by_biomass(merge_itrs);
			}
		// merge the two, the merged one is used for the new split agent
		merge_itrs[1]->merge_with(*merge_itrs[0]);
		merge_itrs[0]->state = split_state;
		// trait of the new split will be randomized (approximate mutation (?))
		_randomize_agent_trait(merge_itrs[0]->trait);
		return;
	}

	void AgentSubtypeBase::_randomize_agent_state(AgentState &state)
	{
		for (size_t i = 0; i < state_cfg.arr.size(); i++)
			state.arr[i] = _rand.gen_value(state_cfg.arr[i]);
		assert(n_agent != 0);
		state.scale_state_content(1.0 / n_agent);
		// ajust specific states
		state.s.rela_count = agent_subtype_consts::INIT_RELA_COUNT;
		state.s.split_biomass = std::max(state.s.split_biomass,
										 state.s.biomass * agent_subtype_consts::MIN_RELA_SPLIT_BIOMASS);
		return;
	}

	void AgentSubtypeBase::_randomize_agent_trait(AgentTrait &trait)
	{
		// generate rate traits
		for (auto i = AgentTrait::rate_arr_begin; i < AgentTrait::rate_arr_end; i++)
			trait.arr[i] = _rand.gen_value(trait_cfg.arr[i]);

		// generate regula traits
		for (auto i = AgentTrait::reg_arr_begin; i < AgentTrait::reg_arr_end; i++)
			trait.arr[i] = _rand.gen_value(trait_cfg.arr[i]);

		// generate bool traits
		for (auto i = AgentTrait::b_arr_begin; i < AgentTrait::b_arr_end; i++)
			*(bivalue_t *)&(trait.arr[i]) = (bool)_rand.gen_value(trait_cfg.arr[i]);
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