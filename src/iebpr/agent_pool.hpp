#ifndef __IEBPR_AGENT_POOL_HPP__
#define __IEBPR_AGENT_POOL_HPP__

#include <memory>
#include <vector>
#include "error_def.hpp"
#include "randomizer.hpp"
#include "agent_subtype_base.hpp"
#include "agent_subtype_gao.hpp"
#include "agent_subtype_oho.hpp"
#include "agent_subtype_pao.hpp"

namespace iebpr
{
	class AgentPool
	{
	private:
		Randomizer &_rand;

	public:
		std::vector<AgentData> agent_data;
		std::vector<std::unique_ptr<AgentSubtypeBase>> agent_subtype;

	public:
		explicit AgentPool(Randomizer &rand) noexcept
			: _rand(rand), agent_data(0), agent_subtype(0) {}

		//======================================================================
		// EXTERNAL API
		//======================================================================

		// total number of agents
		size_t n_agent(void) const noexcept;
		// total number of subtypes
		size_t n_subtype(void) const noexcept;
		// clear all agent data pool and agent subtype data
		void clear_agent_subtype(void) noexcept;
		// add blanc agent subtype, duplicated subtypes are allowed
		// the added agent can be accessed by agent_subtype.back() for further
		// configurations
		void add_agent_subtype(AgentSubtypeBase::subtype_enum subtype, size_t n_agent);
		// add agent subtype with filling randomizer configurations
		// the added agent can be accessed by agent_subtype.back(), but at this
		// point it's not necessary because all settings should have been done
		void add_agent_subtype(AgentSubtypeBase::subtype_enum subtype, size_t n_agent,
							   const AgentSubtypeBase::StateRandConfig &state_cfg,
							   const AgentSubtypeBase::TraitRandConfig &trait_cfg);
		// self validate before init
		error_enum preinit_validate(void) const noexcept;
		// initialize data before simulation run
		void prerun_init(stvalue_t timestep);
		// self validate after init, before simulation
		error_enum prerun_validate(void) const noexcept;

	private:
		// set a contiguous range of agent data instances for a subtype
		void _set_agent_data(AgentSubtypeBase &subtype,
							 const decltype(agent_data)::iterator &begin);
	};

} // namespace iebpr

#endif