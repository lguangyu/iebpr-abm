#ifndef __IEBPR_SIMULATION_HPP__
#define __IEBPR_SIMULATION_HPP__

#include <vector>
#include "def.hpp"
#include "error_def.hpp"
#include "randomizer.hpp"
#include "agent_pool.hpp"
#include "sbr_control.hpp"
#include "recorder.hpp"
#include "timer.hpp"
#include "signal.hpp"

namespace iebpr
{
	class Simulation
	{
	public:
		using rand_t = Randomizer::rand_t;
		using subtype_enum = AgentSubtypeBase::subtype_enum;

	private:
		Timer _timer;
		Randomizer _rand;

	public:
		SbrControl sbr;
		AgentPool pool;
		Recorder recorder;
		SignalHandler<SIGINT> sigint_handler;

		explicit Simulation(decltype(_rand.engine)::result_type seed = 0,
							bool pcontinuous = false,
							stvalue_t timestep = SbrControl::default_timestep) noexcept
			: _rand(seed), sbr(_rand, pcontinuous ? SbrControl::simutype_enum::pcontinuous : SbrControl::simutype_enum::discrete,
							   timestep),
			  pool(_rand), recorder() {}

		//======================================================================
		// EXTERNAL API
		//======================================================================
		// Randomizer

		// check if random type enum value is recognized
		static bool is_valid_randtype_enum(rand_t type) noexcept;
		// interpret random type enum value to string
		static const char *randtype_enum_to_name(rand_t type) noexcept;
		// set random seed
		void set_seed(Randomizer::seed_t seed) noexcept;

		//======================================================================
		// SbrControl setup

		// get simulation type
		SbrControl::simutype_enum get_simutype(void) const noexcept;
		// set simulation type
		void set_simutype(SbrControl::simutype_enum simutype) noexcept;
		// set initial env state
		void set_init_env(const EnvState &env) noexcept;
		// get timestep of SbrControl subunit
		stvalue_t get_timestep(void) const noexcept;
		// set timestep of SbrControl subunit
		void set_timestep(stvalue_t timestep) noexcept;
		// add stage config to SbrControl subunit
		void append_sbr_stage(const SbrControl::Stage &stage);
		// clear all stage config
		void clear_sbr_stage(void) noexcept;
		// return total simulation time length
		stvalue_t total_time_len(void) const noexcept;
		// return true if all stages have balanced inflow volume and outflow
		// (outflow + withdraw) volume
		bool is_flow_balanced(void) const noexcept;

		//======================================================================
		// AgentPool

		// check if subtype enum value is recognized
		static bool is_valid_subtype_enum(subtype_enum subtype, bool allow_none = false) noexcept;
		// interpret subtype enum value to string
		static const char *subtype_enum_to_name(subtype_enum subtype) noexcept;
		// query number of agent subtypes
		size_t n_agent_subtype(void) const noexcept;
		// query number of total agents
		size_t total_n_agent(void) const noexcept;
		// list number of agents by subtype
		std::vector<AgentSubtypeBase::SubtypeSizeReport> n_agent_by_subtype(void) const noexcept;
		// add new subtype config to AgentPool subunit
		void add_agent_subtype(subtype_enum subtype, size_t n_agent,
							   const StateRandConfig &state_cfg,
							   const TraitRandConfig &trait_cfg);
		// clear all agent subtype
		void clear_agent_subtype(void) noexcept;

		//======================================================================
		// Recorder

		// get number of timepoints for state record
		size_t n_state_rec_timepoints(void) const noexcept;
		// get state record timepoints, as copy
		std::vector<stvalue_t> get_state_rec_timepoints(void) const;
		// set state record timepoints
		void set_state_rec_timepoints(std::vector<stvalue_t> &timepoints);
		void set_state_rec_timepoints(std::vector<stvalue_t> &&timepoints);
		// clear state record timepoints
		void clear_state_rec_timepoints(void) noexcept;

		// get number of timepoints for snapshot record
		size_t n_snapshot_rec_timepoints(void) const noexcept;
		// get snapshot record timepoints, as copy
		std::vector<stvalue_t> get_snapshot_rec_timepoints(void) const;
		// set snapshot record timepoints
		void set_snapshot_rec_timepoints(std::vector<stvalue_t> &timepoints);
		void set_snapshot_rec_timepoints(std::vector<stvalue_t> &&timepoints);
		// clear snapshot record timepoints
		void clear_snapshot_rec_timepoints(void) noexcept;

		//======================================================================
		// Simulation

		// run simulation, main loop
		error_enum run(void);
		// get simulation run duration
		std::chrono::milliseconds get_run_duration(void) const noexcept;
		// retireve env state record results from simulation
		const decltype(Recorder::env_state_rec) &retrieve_env_state_rec(void) const noexcept;
		// retireve agent state record results from simulation
		const decltype(Recorder::agent_state_rec) &retrieve_agent_state_rec(void) const noexcept;
		// retireve record snapshot results from simulation
		const decltype(Recorder::snapshot_rec) &retrieve_snapshot_rec(void) const noexcept;
	};

} // namespace iebpr

#endif