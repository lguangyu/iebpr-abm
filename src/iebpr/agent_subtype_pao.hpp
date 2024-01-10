#ifndef __IEBPR_AGENT_SUBTYPE_PAO_HPP__
#define __IEBPR_AGENT_SUBTYPE_PAO_HPP__

#include "agent_subtype_base.hpp"

namespace iebpr
{
	class AgentSubtypePao : public AgentSubtypeBase
	{
	public:
		using AgentSubtypeBase::AgentSubtypeBase;
		subtype_enum subtype(void) const noexcept;
		void agent_action_aerobic(const EnvState &env, EnvState &d_env, AgentData &agent);
		void agent_action_anaerobic(const EnvState &env, EnvState &d_env, AgentData &agent);
	};

} // namespace iebpr

#endif