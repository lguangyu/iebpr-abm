#ifndef __IEBPR_RECORDER_HPP__
#define __IEBPR_RECORDER_HPP__

#include <algorithm>
#include <vector>
#include "error_def.hpp"
#include "env_state.hpp"
#include "agent_pool.hpp"
#include "sbr_control.hpp"

namespace iebpr
{
	struct EnvStateRecData : public EnvState
	{
		// array-like access
		static const size_t arr_size;

		explicit EnvStateRecData(void) noexcept
			: EnvState(){}; // use member ctor
		EnvStateRecData(const EnvState &other) noexcept
			: EnvState(other){};
	};

	struct AgentStateRecData
	{
		stvalue_t biomass;
		stvalue_t rela_count;
		stvalue_t glycogen;
		stvalue_t pha;
		stvalue_t polyp;
		// array-like access
		static const size_t arr_size;

		explicit AgentStateRecData(void) noexcept
			: biomass(0), rela_count(0), glycogen(0), pha(0), polyp(0) {}

		AgentStateRecData(const AgentState &state) noexcept
			: biomass(state.biomass), rela_count(state.rela_count),
			  glycogen(state.glycogen), pha(state.pha), polyp(state.polyp)
		{
		}
	};

	class Recorder
	{
	public:
		std::vector<stvalue_t> state_rec_timepoints;
		std::vector<stvalue_t> snapshot_rec_timepoints;
		std::vector<EnvStateRecData> env_state_rec;
		std::vector<std::vector<AgentStateRecData>> agent_state_rec;
		std::vector<std::vector<std::vector<AgentStateRecData>>> snapshot_rec;

	private:
		decltype(state_rec_timepoints)::iterator _next_state_rec_time_itr;
		decltype(snapshot_rec_timepoints)::iterator _next_snapshot_rec_time_itr;

	public:
		explicit Recorder(void) noexcept
			: state_rec_timepoints(0), snapshot_rec_timepoints(0),
			  env_state_rec(0), agent_state_rec(0), snapshot_rec(0),
			  _next_state_rec_time_itr(state_rec_timepoints.begin()),
			  _next_snapshot_rec_time_itr(snapshot_rec_timepoints.begin())
		{
		}

		//======================================================================
		// INTERNAL API
		//======================================================================

		// self validate before simulation run
		error_enum preinit_validate(void) const noexcept;
		// initialize data before init
		void prerun_init(const AgentPool &pool);
		// take record, called in simulation main loop
		void record(const SbrControl &sbr, const AgentPool &pool);
		// self validate after init, before simulation
		error_enum prerun_validate(const SbrControl &sbr) const noexcept;

	private:
		void _state_record(const SbrControl &sbr, const AgentPool &pool);
		void _snapshot_record(const SbrControl &sbr, const AgentPool &pool);
	};

} // namespace iebpr

#endif