#include "iebpr/recorder.hpp"

namespace iebpr
{
	error_enum Recorder::preinit_validate(void) const noexcept
	{
		return none;
	}

	void Recorder::prerun_init(const AgentPool &pool)
	{
		// sort timepoints
		std::sort(state_rec_timepoints.begin(), state_rec_timepoints.end());
		std::sort(snapshot_rec_timepoints.begin(), snapshot_rec_timepoints.end());

		// clear only record and reserve space for new record
		env_state_rec.clear();
		env_state_rec.reserve(state_rec_timepoints.size());
		agent_state_rec.clear();
		agent_state_rec.reserve(state_rec_timepoints.size());
		_next_state_rec_time_itr = state_rec_timepoints.begin();

		// clear only record and reserve space for new record
		snapshot_rec.clear();
		snapshot_rec.resize(pool.n_subtype());
		for (auto &v : snapshot_rec)
			v.reserve(snapshot_rec_timepoints.size());
		_next_snapshot_rec_time_itr = snapshot_rec_timepoints.begin();
		return;
	}

	static error_enum _validate_timepoins(const std::vector<stvalue_t> &timepoints,
										  const SbrControl &sbr)
	{
		// no need to check if no timepoints set
		if (timepoints.empty())
			return none;

		// check range, should be within simulation time range
		if ((timepoints.front() < 0) || (timepoints.back() > sbr.total_time_len()))
			return rec_time_exceed_simulation;

		// check steps between timepoints
		// no need to check if there is only one timepoint
		if (timepoints.size() == 1)
			return none;

		for (auto itr = timepoints.begin(); itr < timepoints.end() - 1; itr++)
			if ((*(itr + 1) - *itr) <= sbr.get_timestep())
				return rec_step_smaller_than_timestep;

		return none;
	}

	error_enum Recorder::prerun_validate(const SbrControl &sbr) const noexcept
	{
		error_enum ret = none;
		ret = _validate_timepoins(state_rec_timepoints, sbr);
		if (ret != none)
			return ret;
		ret = _validate_timepoins(snapshot_rec_timepoints, sbr);
		if (ret != none)
			return ret;
		return none;
	}

	void Recorder::record(const SbrControl &sbr, const AgentPool &pool)
	{
		_state_record(sbr, pool);
		_snapshot_record(sbr, pool);
		return;
	}

	void Recorder::_state_record(const SbrControl &sbr, const AgentPool &pool)
	{
		if ((_next_state_rec_time_itr == state_rec_timepoints.end()) ||
			(sbr.get_curr_time() < *_next_state_rec_time_itr))
			return;
		// env state record
		env_state_rec.push_back(sbr.env);
		// agent state record
		auto rec = std::vector<AgentStateRecEntry>(0);
		for (auto &v : pool.agent_subtype)
			rec.push_back(v->summarize_agent_state());
		agent_state_rec.push_back(rec);
		//
		_next_state_rec_time_itr++;
		return;
	}

	void Recorder::_snapshot_record(const SbrControl &sbr, const AgentPool &pool)
	{
		if ((_next_snapshot_rec_time_itr == snapshot_rec_timepoints.end()) ||
			(sbr.get_curr_time() < *_next_snapshot_rec_time_itr))
			return;
		// take snapshot by subtype
		for (size_t i = 0; i < pool.n_subtype(); i++)
		{
			auto _n = pool.agent_subtype[i]->n_agent;
			auto snapshot = std::vector<AgentStateRecEntry>(_n);
			std::transform(pool.agent_subtype[i]->pool_begin(),
						   pool.agent_subtype[i]->pool_end(),
						   snapshot.begin(),
						   [](const AgentData &agent)
						   { return AgentStateRecEntry(agent.state); });
			snapshot_rec[i].push_back(std::move(snapshot));
		}
		//
		_next_snapshot_rec_time_itr++;
		return;
	}

} // namespace iebpr