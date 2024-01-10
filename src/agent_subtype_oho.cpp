#include "iebpr/agent_subtype_oho.hpp"

namespace iebpr
{

	AgentSubtypeBase::subtype_enum AgentSubtypeOho::subtype(void) const noexcept
	{
		return oho;
	}

	void AgentSubtypeOho::agent_action_aerobic(const EnvState &env, EnvState &d_env, AgentData &agent)
	{
		// agent state change
		auto d_state = AgentState();

		// prepare data
		const auto monod_vfa = agent.monod_vfa(env);
		const auto monod_op = agent.monod_op(env);

		// cell growth
		if ((env.vfa_conc > 0) && (env.op_conc > 0))
		{
			const auto delta = agent.trait.s.rate.mu * monod_vfa * monod_op * agent.state.s.biomass;
			d_state.s.biomass += delta;
			d_env.vfa_conc -= delta / agent.trait.s.reg.y_h;
			d_env.op_conc -= delta * agent.trait.s.reg.i_bmp;
		}
		// biomass decay
		{
			const auto delta = agent.trait.s.rate.b_aerobic * agent.state.s.biomass;
			d_state.s.biomass -= delta;
			d_env.vfa_conc += delta * agent_subtype_consts::VFA_PER_DECAYED_BIOMASS;
			d_env.op_conc += delta * agent.trait.s.reg.i_bmp;
		}
		// update to agent
		assert(d_state.s.split_biomass == 0);
		agent.state.merge_with(d_state);
		return;
	}

	void AgentSubtypeOho::agent_action_anaerobic(const EnvState &env, EnvState &d_env, AgentData &agent)
	{
		// agent state change
		auto d_state = AgentState();

		// prepare data

		// biomass decay
		{
			const auto delta = agent.trait.s.rate.b_anaerobic * agent.state.s.biomass;
			d_state.s.biomass -= delta;
			d_env.vfa_conc += delta * agent_subtype_consts::VFA_PER_DECAYED_BIOMASS;
			d_env.op_conc += delta * agent.trait.s.reg.i_bmp;
		}
		// update to agent
		assert(d_state.s.split_biomass == 0);
		agent.state.merge_with(d_state);
		return;
	}

} // namespace iebpr