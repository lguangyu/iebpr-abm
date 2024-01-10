#include "iebpr/agent_pool.hpp"

namespace iebpr
{
	size_t AgentPool::n_agent(void) const noexcept
	{
		size_t sum = 0;
		for (auto &v : agent_subtype)
			sum += v->n_agent;
		return sum;
	}

	size_t AgentPool::n_subtype(void) const noexcept
	{
		return agent_subtype.size();
	}

	void AgentPool::clear_agent_subtype(void) noexcept
	{
		agent_data.clear();
		agent_subtype.clear();
		return;
	}

	void AgentPool::add_agent_subtype(AgentSubtypeBase::subtype_enum subtype, size_t num)
	{
		// allocate space for new agents
		switch (subtype)
		{
		case AgentSubtypeBase::subtype_enum::pao:
		{
			agent_subtype.push_back(std::unique_ptr<AgentSubtypeBase>(
				new AgentSubtypePao(_rand, num)));
			break;
		}
		case AgentSubtypeBase::subtype_enum::gao:
		{
			agent_subtype.push_back(std::unique_ptr<AgentSubtypeBase>(
				new AgentSubtypeGao(_rand, num)));
			break;
		}
		case AgentSubtypeBase::subtype_enum::oho:
		{
			agent_subtype.push_back(std::unique_ptr<AgentSubtypeBase>(
				new AgentSubtypeOho(_rand, num)));
			break;
		}
		default:
		{
			agent_subtype.push_back(std::unique_ptr<AgentSubtypeBase>(
				new AgentSubtypeBase(_rand, num)));
			break;
		}
		}
		return;
	}

	void AgentPool::add_agent_subtype(AgentSubtypeBase::subtype_enum subtype, size_t n_agent,
									  const AgentSubtypeBase::StateRandConfig &state_cfg,
									  const AgentSubtypeBase::TraitRandConfig &trait_cfg)
	{
		add_agent_subtype(subtype, n_agent);
		agent_subtype.back()->state_cfg = state_cfg;
		agent_subtype.back()->trait_cfg = trait_cfg;
		return;
	}

	error_enum AgentPool::preinit_validate(void) const noexcept
	{
		// check rand configs
		for (auto &v : agent_subtype)
		{
			// state config
			for (auto &cfg : v->state_cfg.arr)
				if (auto ec = cfg.validate())
					return ec;
			// rate trait config
			for (auto i = AgentTrait::rate_arr_begin; i < AgentTrait::rate_arr_end; i++)
				if (auto ec = v->trait_cfg.arr[i].validate())
					return ec;
			// regular trait config
			for (auto i = AgentTrait::reg_arr_begin; i < AgentTrait::reg_arr_end; i++)
				if (auto ec = v->trait_cfg.arr[i].validate())
					return ec;
			// bool trait config
			for (auto i = AgentTrait::b_arr_begin; i < AgentTrait::b_arr_end; i++)
			{
				const auto &cfg = v->trait_cfg.arr[i];
				if ((cfg.type != Randomizer::rand_t::none) && (cfg.type != Randomizer::rand_t::bernoulli))
					return bool_trait_wrong_rand_type;
				if (auto ec = cfg.validate())
					return ec;
			}
		}
		return none;
	}

	void AgentPool::prerun_init(stvalue_t timestep)
	{
		// allocate spaces for agents
		agent_data.resize(n_agent());
		auto curr_pool_begin = agent_data.begin();

		// update pool and create agent instances
		for (auto &v : agent_subtype)
		{
			// set pool
			_set_agent_data(*v, curr_pool_begin);
			curr_pool_begin += v->n_agent;
			// instantiate agents
			v->trait_cfg_apply_rate_adjust(timestep);
			v->instantiate_agents();
		}
		return;
	}

	error_enum AgentPool::prerun_validate(void) const noexcept
	{
		// total num of agent match sum from all subtypes
		if (agent_data.size() != n_agent())
			return total_agent_mismatch_subtype_sum;
		// if agent pool overlap between different subtypes
		if (n_subtype() >= 2)
		{
			for (auto i = agent_subtype.begin(); i < agent_subtype.end(); i++)
				for (auto j = i + 1; j < agent_subtype.end(); j++)
					if ((*i)->has_data_overlap_with(**j))
						return agent_subtype_pool_overlap;
		}
		return none;
	}

	void AgentPool::_set_agent_data(AgentSubtypeBase &subtype,
									const decltype(agent_data)::iterator &begin)
	{
		subtype._pool_begin = begin;
		subtype._pool_end = begin + subtype.n_agent;
		return;
	}

} // namespace iebpr