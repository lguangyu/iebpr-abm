#include "iebpr/sbr_control.hpp"

namespace iebpr
{

	void SbrControl::Stage::clear_phase(void) noexcept
	{
		cycle_phases.clear();
		reset_curr_phase();
		return;
	}

	stvalue_t SbrControl::Stage::cycle_time_len(void) const noexcept
	{
		stvalue_t ret = 0;
		for (auto &v : cycle_phases)
			ret += v.time_len;
		return ret;
	}

	stvalue_t SbrControl::Stage::total_time_len(void) const noexcept
	{
		return cycle_time_len() * n_cycle;
	}

	bool SbrControl::Stage::is_flow_balanced(void) const noexcept
	{
		stvalue_t vol_in = 0, vol_out = 0;
		for (auto &v : cycle_phases)
		{
			vol_in += v.inflow_rate * v.time_len;
			vol_out += (v.withdraw_rate + v.outflow_rate) * v.time_len;
		}
		return std::abs(vol_in - vol_out) < 1e-8;
	}

	int SbrControl::Stage::start_stage(void)
	{
		elapsed_cycle = 0;
		reset_curr_phase();
		if (cycle_phases.empty())
		{
			elapsed_cycle = n_cycle;
			return 1;
		}
		return 0;
	}

	int SbrControl::Stage::transit_to_next_phase(void)
	{
		if (cycle_phases.empty())
			elapsed_cycle = n_cycle;
		if (finishd_last_cycle())
			return 1;
		_curr_phase_itr++;
		if (_curr_phase_itr >= cycle_phases.end())
		{
			// start a new cycle, add one finished
			_curr_phase_itr = cycle_phases.begin();
			elapsed_cycle++;
		}
		return finishd_last_cycle() ? 1 : 0;
	}

	void SbrControl::clear_stage(void) noexcept
	{
		stages.clear();
		reset_curr_stage();
		return;
	}

	stvalue_t SbrControl::total_time_len(void) const noexcept
	{
		stvalue_t ret = 0;
		for (auto &v : stages)
			ret += v.total_time_len();
		return ret;
	}

	bool SbrControl::is_flow_balanced(void) const noexcept
	{
		auto ret = true;
		for (auto &v : stages)
			ret &= v.is_flow_balanced();
		return ret;
	}

	error_enum SbrControl::preinit_validate(void) const noexcept
	{
		// check timestep
		if (get_timestep() <= 0)
			return invalid_timestep;

		// check init volume
		if (init_env.volume <= 0)
			return invalid_init_volume;

		return none;
	}

	void SbrControl::prerun_init(AgentPool &pool) noexcept
	{
		env = init_env;
		_curr_time = 0;
		// reset stages
		for (auto &stage : stages)
			stage.reset_stage_progress();
		_phase_trans_time = 0;
		_prerun_init_stage_phase_status();
		// note the -1 @ the second parameter of _rand_agent
		_rand_agent.param(std::uniform_int_distribution<size_t>::param_type(0, pool.n_agent() - 1));
		return;
	}

	error_enum SbrControl::prerun_validate(void) const noexcept
	{
		return none;
	}

	void SbrControl::_prerun_init_stage_phase_status(void) noexcept
	{
		for (auto si = stages.begin(); si < stages.end(); si++)
			if (!si->start_stage())
			{
				_curr_stage_itr = si;
				const Phase &phase = si->get_curr_phase();
				_phase_trans_time += phase.time_len;
				rate_adjusted_phase = phase.adjust_rate_by_timestep(get_timestep());
				return;
			}
		// reaching here means no stages or all stages are empty
		_force_set_finish();
		return;
	}

	void SbrControl::_force_set_finish(void) noexcept
	{
		for (auto &v : stages)
			v.elapsed_cycle = v.n_cycle;
		_curr_stage_itr = stages.end();
		return;
	}

	void SbrControl::_transit_next_phase_recursive(void) noexcept
	{
		if (finished_last_stage())
			return;
		if (!get_curr_stage().transit_to_next_phase())
			return;
		// transit to next stage
		_curr_stage_itr++;
		if (!finished_last_stage())
			get_curr_stage().start_stage();
		return _transit_next_phase_recursive();
	}

	void SbrControl::transit_phase(void)
	{
		if (_curr_time < _phase_trans_time)
			return;

		// transition to a new phase
		_transit_next_phase_recursive();
		if (finished_last_stage())
		{
			rate_adjusted_phase = Phase(); // clear value
		}
		else
		{
			const Phase &phase = get_curr_stage().get_curr_phase();
			rate_adjusted_phase = phase.adjust_rate_by_timestep(get_timestep());
			_phase_trans_time += rate_adjusted_phase.time_len;
		}
		return;
	}

	void SbrControl::timestep_update(AgentPool &pool)
	{
		if (!finished_last_stage())
		{
			// update biomass
			(simutype == pcontinuous) ? _timestep_update_agents_pcontinuous(pool)
									  : _timestep_update_agents_discrete(pool);
			// update env
			_timestep_update_env(pool);
		}

		_curr_time += _timestep;

		transit_phase();

		return;
	}

	void SbrControl::_timestep_update_agents_discrete(AgentPool &pool)
	{
		// update env only at the end of a complete timestep
		auto d_env = EnvState();
		for (auto &v : pool.agent_subtype)
			for (auto itr = v->pool_begin(); itr < v->pool_end(); itr++)
				v->agent_action(env, d_env, itr);
		// update env
		assert(d_env.is_aerobic == 0);
		env.update_change(d_env); // shouldn't change
		return;
	}

	void SbrControl::_timestep_update_agents_pcontinuous(AgentPool &pool)
	{
		for (size_t i = 0; i < pool.n_agent(); i++)
		{

			size_t agent_id = _rand_agent(_rand.engine);
			assert(agent_id < pool.n_agent());
			// update env for every agent action calculation
			auto d_env = EnvState();
			for (auto &v : pool.agent_subtype)
			{
				if (agent_id < v->n_agent)
				{
					v->agent_action(env, d_env, v->pool_begin() + agent_id);
					break;
				}
				agent_id -= v->n_agent;
			}
			// update env
			assert(d_env.is_aerobic == 0); // shouldn't change
			env.update_change(d_env);
		}
		return;
	}

	void SbrControl::_timestep_update_env(AgentPool &pool)
	{
		const Phase &phase = rate_adjusted_phase;
		assert(phase.inflow_rate == get_curr_stage().get_curr_phase().inflow_rate * get_timestep());
		assert(phase.withdraw_rate == get_curr_stage().get_curr_phase().withdraw_rate * get_timestep());
		assert(phase.outflow_rate == get_curr_stage().get_curr_phase().outflow_rate * get_timestep());
		const auto old_volume = env.volume;
		// calculate env and agent state changes
		const auto dvi = phase.inflow_rate;
		const auto dvw = phase.withdraw_rate;
		const auto dvo = phase.outflow_rate;
		// update volume first
		env.volume += dvi - dvw - dvo;
		if (env.volume <= 0) // no need to update the concentration other wise
		{
			env.volume = 0;
			env.vfa_conc = 0;
			env.op_conc = 0;
			// and clear all content due to total outwash
			for (auto &v : pool.agent_data)
				v.state.clear_state_content();
		}
		else
		{
			// scale conc. by new_c = ((new_v - dvi) * c + dvi * ci) / new_v
			env.vfa_conc = ((env.volume - dvi) * env.vfa_conc + dvi * phase.inflow_vfa_conc) / env.volume;
			env.op_conc = ((env.volume - dvi) * env.op_conc + dvi * phase.inflow_op_conc) / env.volume;
			env.is_aerobic = phase.aeration; // overwrite the old value
			// scale biomass by (old_v - dvw) / new_v
			auto factor = (old_volume - dvw) / env.volume;
			for (auto &v : pool.agent_data)
				// clear all content due to total outwash
				v.state.scale_state_content(factor);
		}
		return;
	}

} // namespace iebpr