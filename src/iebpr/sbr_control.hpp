#ifndef __IEBPR_SBR_CONTROL_HPP__
#define __IEBPR_SBR_CONTROL_HPP__

#include <vector>
#include <random>
#include "error_def.hpp"
#include "randomizer.hpp"
#include "env_state.hpp"
#include "agent_pool.hpp"

namespace iebpr
{
	class SbrControl
	{
	public:
		using simutype_enum = enum : enum_base_t {
			discrete,
			pcontinuous,
		};

		struct Phase
		{
		public:
			stvalue_t time_len;
			stvalue_t inflow_rate;
			stvalue_t inflow_vfa_conc;
			stvalue_t inflow_op_conc;
			stvalue_t withdraw_rate;
			stvalue_t outflow_rate;
			bivalue_t aeration;
			stvalue_t volume_reset;

			explicit Phase(decltype(time_len) time_len = 0,
						   decltype(inflow_rate) inflow_rate = 0,
						   decltype(inflow_vfa_conc) inflow_vfa_conc = 0,
						   decltype(inflow_op_conc) inflow_op_conc = 0,
						   decltype(withdraw_rate) withdraw_rate = 0,
						   decltype(outflow_rate) outflow_rate = 0,
						   decltype(aeration) aeration = 0,
						   decltype(volume_reset) volume_reset = stvalue_inf) noexcept
				: time_len(time_len),
				  inflow_rate(inflow_rate),
				  inflow_vfa_conc(inflow_vfa_conc),
				  inflow_op_conc(inflow_op_conc),
				  withdraw_rate(withdraw_rate),
				  outflow_rate(outflow_rate),
				  aeration(aeration),
				  volume_reset(volume_reset)
			{
			}

			Phase adjust_rate_by_timestep(stvalue_t timestep) const noexcept
			{
				return Phase(time_len,
							 inflow_rate * timestep,
							 inflow_vfa_conc,
							 inflow_op_conc,
							 withdraw_rate * timestep,
							 outflow_rate * timestep,
							 aeration,
							 volume_reset);
			}
		};

		struct Stage
		{
		public:
			size_t n_cycle;
			size_t elapsed_cycle;
			std::vector<Phase> cycle_phases;
			decltype(cycle_phases)::iterator _curr_phase_itr;

		public:
			explicit Stage(void) noexcept
				: n_cycle(0), elapsed_cycle(0), cycle_phases(0),
				  _curr_phase_itr(cycle_phases.begin()) {}

			// num of phases currently set
			inline size_t n_phase(void) const noexcept { return cycle_phases.size(); };
			// append a phase config
			inline void append_phase(const Phase &phase)
			{
				cycle_phases.push_back(phase);
				return;
			};
			// clear all phase config
			void clear_phase(void) noexcept;
			// time length per cycle in this stage
			stvalue_t cycle_time_len(void) const noexcept;
			// total time length of this stage
			stvalue_t total_time_len(void) const noexcept;
			// return true if inflow volume and outflow (outflow + widthdraw)
			// volume are balanced through one cycle
			bool is_flow_balanced(void) const noexcept;
			// start the stage, return 0 on success, 1 on fail
			int start_stage(void);
			// update stage to the next phase
			// return 0 on success, 1 on fail (usually stage has already finished)
			int transit_to_next_phase(void);
			// true when curr_phase_itr reaches the end of phases config
			inline bool finishd_last_cycle(void) const noexcept { return (elapsed_cycle >= n_cycle); };
			// get the current phase
			inline Phase &get_curr_phase(void) { return *_curr_phase_itr; };
			// reset current phase itr
			inline void reset_curr_phase(void) noexcept
			{
				_curr_phase_itr = cycle_phases.begin();
				return;
			};
			// reset stage to prerun state
			inline void reset_stage_progress(void) noexcept
			{
				elapsed_cycle = 0;
				reset_curr_phase();
				return;
			};
		};

		// initial env state; set this by directly access this member variable;
		// no setter/getter
		EnvState init_env;
		EnvState env;
		std::vector<Stage> stages;
		simutype_enum simutype;
		// current phase with rates adjusted by timestep
		// for optimization purpose, to reduce repeated calculations
		Phase rate_adjusted_phase;

	private:
		stvalue_t _curr_time;
		stvalue_t _timestep;
		stvalue_t _phase_trans_time;
		decltype(stages)::iterator _curr_stage_itr;
		Randomizer &_rand;
		std::uniform_int_distribution<size_t> _rand_agent;

	public:
		constexpr static decltype(_timestep) default_timestep = 1e-5;

	public:
		explicit SbrControl(Randomizer &rand, simutype_enum simutype = discrete,
							decltype(_timestep) timestep = default_timestep) noexcept
			: init_env(), env(), stages(0), simutype(simutype), rate_adjusted_phase(),
			  _curr_time(0), _timestep(timestep), _phase_trans_time(0),
			  _curr_stage_itr(stages.begin()), _rand(rand), _rand_agent()
		{
		}

		//======================================================================
		// INTERNAL API
		//======================================================================

		// get timestep
		inline stvalue_t get_timestep(void) const { return _timestep; };
		// set timestep
		inline void set_timestep(stvalue_t timestep) noexcept
		{
			_timestep = timestep;
			return;
		};
		// get current elapsed simulation time
		inline stvalue_t get_curr_time(void) const noexcept { return _curr_time; };
		// num of stages currently set
		inline size_t n_stage(void) const noexcept { return stages.size(); };
		// append a stage config
		inline void append_stage(const Stage &stage)
		{
			stages.push_back(stage);
			return;
		}
		// clear all stage config
		void clear_stage(void) noexcept;
		// total simulation time length of all stages
		stvalue_t total_time_len(void) const noexcept;
		// return true if all stages have balanced inflow volume and outflow
		// (outflow + withdraw) volume
		bool is_flow_balanced(void) const noexcept;
		// get the current stage
		inline Stage &get_curr_stage(void) { return *_curr_stage_itr; };
		// reset current stage itr
		inline void reset_curr_stage(void)
		{
			_curr_stage_itr = stages.begin();
			return;
		};
		// true when curr_stage_itr reaches the end of stage config
		inline bool finished_last_stage(void) const noexcept { return (_curr_stage_itr >= stages.end()); };
		// called to update env and agent state at each time step
		void timestep_update(AgentPool &pool);
		// try transit phase
		void transit_phase(void);
		// self validate before init
		error_enum preinit_validate(void) const noexcept;
		// initialize data before simulation run
		void prerun_init(AgentPool &pool) noexcept;
		// self validate after init, before simulation
		error_enum prerun_validate(void) const noexcept;

	private:
		// set the sbr status to the first state/phase with non-zero time length
		void _prerun_init_stage_phase_status(void) noexcept;
		// force finish sbr run
		void _force_set_finish(void) noexcept;
		// agent action for discrete-time simulation type
		void _timestep_update_agents_discrete(AgentPool &pool);
		// agent action for pseudo-continuous simulation type
		void _timestep_update_agents_pcontinuous(AgentPool &pool);
		// physical process update (inflow/outflow)
		void _timestep_update_env(AgentPool &pool);
		// find next phase, may across stages
		void _transit_next_phase_recursive(void) noexcept;
	};

} // namespace iebpr

#endif