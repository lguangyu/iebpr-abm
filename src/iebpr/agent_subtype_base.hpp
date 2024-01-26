#ifndef __IEBPR_AGENT_SUBTYPE_BASE_HPP__
#define __IEBPR_AGENT_SUBTYPE_BASE_HPP__

#include <vector>
#include "env_state.hpp"
#include "agent_subtype_base_state_cfg.hpp"
#include "agent_subtype_base_trait_cfg.hpp"
#include "agent_subtype_consts.hpp"

namespace iebpr
{
	class AgentPool;

	//==========================================================================
	// agent state randomizer config full set
	class AgentSubtypeBase
	{
	public:
		using agent_itr_t = std::vector<AgentData>::iterator;
		// enum type for agent subtypes
		// currently only pao, gao, and oho are implemented
		using subtype_enum = enum : enum_base_t {
			// none is used internally for test-only purposes
			none = 0,
			// normal types
			pao,
			gao,
			oho,
			// invalid flag
			invalid = 0xffffffff,
		};

		// used as a convenient type to report how many agent a subtype has
		using SubtypeSizeReport = struct
		{
			subtype_enum subtype;
			size_t n_agent;
		};

		//======================================================================
		// EXTERNAL API / STATIC AGENT SUBTYPE CONVERSION
		//======================================================================
		// these functions need to be updated if more agent subtypes are added

		// check if subtype enum value is recognized
		static bool is_valid_subtype_enum(subtype_enum subtype, bool allow_none = false);
		// interpret subtype enum value to string
		static const char *subtype_enum_to_name(subtype_enum subtype) noexcept;

	public:
		StateRandConfig state_cfg;
		TraitRandConfig trait_cfg;
		const size_t n_agent;

	private:
		Randomizer &_rand;
		agent_itr_t _pool_begin;
		agent_itr_t _pool_end;

	public:
		explicit AgentSubtypeBase(Randomizer &rand, size_t n_agent)
			: state_cfg(), trait_cfg(), n_agent(n_agent), _rand(rand),
			  _pool_begin(), _pool_end()
		{
		}
		virtual ~AgentSubtypeBase(void) noexcept;

		//======================================================================
		// INTERNAL API
		//======================================================================

		friend class AgentPool;

		// subtype indicator
		virtual subtype_enum subtype(void) const noexcept;
		// begin of pool range
		const decltype(_pool_begin) pool_begin(void) const noexcept { return _pool_begin; }
		// end of pool range
		const decltype(_pool_end) pool_end(void) const noexcept { return _pool_end; }
		// check if the agent data range overlaps with another subtype claim
		bool has_data_overlap_with(const AgentSubtypeBase &other) const noexcept;
		// report subtype and number of agents
		SubtypeSizeReport report_subtype_and_size(void) const noexcept;
		// apply timestep to rate rand configs for optimization before simulation run
		void trait_cfg_apply_rate_adjust(stvalue_t timestep);
		// fill agent state and trait, use generated random values
		void instantiate_agents(void);
		// update env and agent state, check agent split in the end
		void agent_action(const EnvState &env, EnvState &d_env, agent_itr_t agent_itr)
		{
			if (!agent_itr->is_active())
				return;

			// agent action (cell process)
			// calls subtype-dependent implementations
			env.is_aerobic ? this->agent_action_aerobic(env, d_env, *agent_itr)
						   : this->agent_action_anaerobic(env, d_env, *agent_itr);

			if (agent_itr->can_split())
				agent_split(agent_itr);

			return;
		}
		// agent action under anaerobic conditions
		// called internally by agent_action; subtype-dependent implementation
		virtual void agent_action_aerobic(const EnvState &env, EnvState &d_env, AgentData &agent);
		// agent action under anaerobic conditions
		// called internally by agent_action; subtype-dependent implementation
		virtual void agent_action_anaerobic(const EnvState &env, EnvState &d_env, AgentData &agent);
		// called when agent biomass >= split_biomass
		void agent_split(agent_itr_t agent_itr);
		// summarize current state of agents
		AgentState summarize_agent_state_cont(void) const noexcept;

	private:
		void _randomize_agent_state(AgentState &state);
		void _randomize_agent_trait(AgentTrait &trait);
	};

} // namespace iebpr

#endif