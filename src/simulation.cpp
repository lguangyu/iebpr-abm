#include "iebpr/simulation.hpp"

namespace iebpr
{
	bool Simulation::is_valid_randtype_enum(rand_t type) noexcept
	{
		return Randomizer::is_valid_randtype_enum(type);
	}

	const char *Simulation::randtype_enum_to_name(rand_t type) noexcept
	{
		return Randomizer::randtype_enum_to_name(type);
	}

	void Simulation::set_seed(Randomizer::seed_t seed) noexcept
	{
		_rand.seed(seed);
		return;
	}

	SbrControl::simutype_enum Simulation::get_simutype(void) const noexcept
	{
		return sbr.simutype;
	}

	void Simulation::set_simutype(SbrControl::simutype_enum simutype) noexcept
	{
		sbr.simutype = simutype;
		return;
	}

	void Simulation::set_init_env(const EnvState &env) noexcept
	{
		sbr.init_env = env;
		return;
	}

	stvalue_t Simulation::get_timestep(void) const noexcept
	{
		return sbr.get_timestep();
	}

	void Simulation::set_timestep(stvalue_t timestep) noexcept
	{
		sbr.set_timestep(timestep);
		return;
	}

	void Simulation::append_sbr_stage(const SbrControl::Stage &stage)
	{
		sbr.append_stage(stage);
		return;
	}

	void Simulation::clear_sbr_stage(void) noexcept
	{
		sbr.clear_stage();
		return;
	}

	stvalue_t Simulation::total_time_len(void) const noexcept
	{
		return sbr.total_time_len();
	}

	bool Simulation::is_flow_balanced(void) const noexcept
	{
		return sbr.is_flow_balanced();
	}

	bool Simulation::is_valid_subtype_enum(subtype_enum subtype, bool allow_none) noexcept
	{
		return AgentSubtypeBase::is_valid_subtype_enum(subtype, allow_none);
	}

	const char *Simulation::subtype_enum_to_name(subtype_enum subtype) noexcept
	{
		return AgentSubtypeBase::subtype_enum_to_name(subtype);
	}

	size_t Simulation::n_agent_subtype(void) const noexcept
	{
		return pool.n_subtype();
	}

	size_t Simulation::total_n_agent(void) const noexcept
	{
		return pool.n_agent();
	}

	std::vector<AgentSubtypeBase::SubtypeSizeReport> Simulation::n_agent_by_subtype(void) const noexcept
	{
		auto ret = std::vector<AgentSubtypeBase::SubtypeSizeReport>(0);
		for (auto &v : pool.agent_subtype)
			ret.push_back(v->report_subtype_and_size());
		return ret;
	}

	void Simulation::add_agent_subtype(subtype_enum subtype, size_t n_agent,
									   const AgentSubtypeBase::StateRandConfig &state_cfg,
									   const AgentSubtypeBase::TraitRandConfig &trait_cfg)
	{
		pool.add_agent_subtype(subtype, n_agent, state_cfg, trait_cfg);
		return;
	}

	void Simulation::clear_agent_subtype(void) noexcept
	{
		pool.clear_agent_subtype();
		return;
	}

	size_t Simulation::n_state_rec_timepoints(void) const noexcept
	{
		return recorder.state_rec_timepoints.size();
	}

	std::vector<stvalue_t> Simulation::get_state_rec_timepoints(void) const
	{
		return recorder.state_rec_timepoints;
	}

	void Simulation::set_state_rec_timepoints(std::vector<stvalue_t> &timepoints)
	{
		recorder.state_rec_timepoints = timepoints;
		return;
	}

	void Simulation::set_state_rec_timepoints(std::vector<stvalue_t> &&timepoints)
	{
		recorder.state_rec_timepoints = timepoints;
		return;
	}

	void Simulation::clear_state_rec_timepoints(void) noexcept
	{
		recorder.state_rec_timepoints.clear();
		return;
	}

	size_t Simulation::n_snapshot_rec_timepoints(void) const noexcept
	{
		return recorder.snapshot_rec_timepoints.size();
	}

	std::vector<stvalue_t> Simulation::get_snapshot_rec_timepoints(void) const
	{
		return recorder.snapshot_rec_timepoints;
	}

	void Simulation::set_snapshot_rec_timepoints(std::vector<stvalue_t> &timepoints)
	{
		recorder.snapshot_rec_timepoints = timepoints;
		return;
	}

	void Simulation::set_snapshot_rec_timepoints(std::vector<stvalue_t> &&timepoints)
	{
		recorder.snapshot_rec_timepoints = timepoints;
		return;
	}

	void Simulation::clear_snapshot_rec_timepoints(void) noexcept
	{
		recorder.snapshot_rec_timepoints.clear();
		return;
	}

	error_enum Simulation::run(void)
	{
		// pre initialize check
		if (auto ret = sbr.preinit_validate())
			return ret;
		if (auto ret = pool.preinit_validate())
			return ret;
		if (auto ret = recorder.preinit_validate())
			return ret;

		// initialize
		pool.prerun_init(sbr.get_timestep());
		sbr.prerun_init(pool);
		recorder.prerun_init(pool);

		// pre run check
		if (auto ret = sbr.prerun_validate())
			return ret;
		if (auto ret = pool.prerun_validate())
			return ret;
		if (auto ret = recorder.prerun_validate(sbr))
			return ret;

		// main loop
		sigint_handler.activate();
		_timer.start();
		while (!(sbr.finished_last_stage() || sigint_handler.sig_received()))
		{
			sbr.timestep_update(pool);
			recorder.record(sbr, pool);
		}
		_timer.stop();
		sigint_handler.deactivate();

		return sigint_handler.sig_received() ? error_enum::sigint : error_enum::none;
	}

	std::chrono::milliseconds Simulation::get_run_duration(void) const noexcept
	{
		return _timer.get_duration();
	}

	const decltype(Recorder::env_state_rec) &Simulation::retrieve_env_state_rec(void) const noexcept
	{
		return recorder.env_state_rec;
	}

	const decltype(Recorder::agent_state_rec) &Simulation::retrieve_agent_state_rec(void) const noexcept
	{
		return recorder.agent_state_rec;
	}

	const decltype(Recorder::snapshot_rec) &Simulation::retrieve_snapshot_rec(void) const noexcept
	{
		return recorder.snapshot_rec;
	}

} // namespace iebpr
