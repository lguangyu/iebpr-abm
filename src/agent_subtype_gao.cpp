#include "iebpr/agent_subtype_gao.hpp"

namespace iebpr
{

	AgentSubtypeBase::subtype_enum AgentSubtypeGao::subtype(void) const noexcept
	{
		return gao;
	}

	void AgentSubtypeGao::agent_action_aerobic(const EnvState &env, EnvState &d_env, AgentData &agent)
	{
		// agent state change
		auto d_state = AgentState();

		// prepare data
		const auto monod_op = agent.monod_op(env);
		const auto x_glycogen = agent.x_glycogen();
		const auto monod_glycogen = agent.monod_glycogen();
		const auto i_glycogen = agent.i_glycogen();
		const auto inhib_glycogen = agent.inhib_glycogen();
		const auto x_pha = agent.x_pha();
		const auto monod_pha = agent.monod_pha();

		// glycogen synthesis
		if ((i_glycogen > 0) && (x_pha > 0))
		{
			const auto delta = agent.trait.rate.q_glycogen * monod_pha * inhib_glycogen * agent.state.biomass;
			d_state.glycogen += delta;
			d_state.pha -= delta / agent.trait.reg.y_glycogen_pha;
		}
		// biomass growth on pha
		if ((env.op_conc > 0) && (x_pha > 0))
		{
			const auto delta = agent.trait.rate.mu * monod_pha * monod_op * agent.state.biomass;
			d_state.biomass += delta;
			d_state.pha -= delta / agent.trait.reg.y_h;
			d_env.op_conc -= delta * agent.trait.reg.i_bmp;
		}
		// maintenance (not bound with decay)
		{
			// portion resolved by calculation order
			// pha first, then glycogen
			const auto p_pha = monod_pha;
			const auto p_glycogen = std::min(1 - p_pha, monod_glycogen);
			// belows are order-free
			// glycogen
			{
				const auto delta = p_glycogen * agent.trait.rate.m_aerobic *
								   agent_subtype_consts::GLYC_PER_ATP_AER *
								   agent.state.biomass;
				d_state.glycogen -= delta;
			}
			// pha
			{
				const auto delta = p_pha * agent.trait.rate.m_aerobic *
								   agent_subtype_consts::PHA_PER_ATP_AER *
								   agent.state.biomass;
				d_state.pha -= delta;
			}
		}
		// biomass decay
		{
			const auto delta = agent.trait.rate.b_aerobic * agent.state.biomass;
			d_state.biomass -= delta;
			d_env.vfa_conc += delta * agent_subtype_consts::VFA_PER_DECAYED_BIOMASS;
			d_env.op_conc += delta * agent.trait.reg.i_bmp;
		}
		// glycogen intrinsic decay, release as vfa
		if (x_glycogen > 0)
		{
			const auto delta = agent.trait.rate.b_glycogen * x_glycogen * agent.state.biomass;
			d_state.glycogen -= delta;
			d_env.vfa_conc += delta;
		}
		// pha intrinsic decay, release as vfa
		if (x_pha > 0)
		{
			const auto delta = agent.trait.rate.b_pha * x_pha * agent.state.biomass;
			d_state.pha -= delta;
			d_env.vfa_conc += delta;
		}
		// update to agent
		assert(d_state.rela_count == 0);
		assert(d_state.split_biomass == 0);
		agent.state.merge_with(d_state);
		assert((agent.state.rela_count != stvalue_inf) &&
			   (agent.state.rela_count != -stvalue_inf) &&
			   (agent.state.rela_count != stvalue_nan));
		return;
	}

	void AgentSubtypeGao::agent_action_anaerobic(const EnvState &env, EnvState &d_env, AgentData &agent)
	{
		// agent state change
		auto d_state = AgentState();

		// prepare data
		const auto monod_vfa = agent.monod_vfa(env);
		const auto x_glycogen = agent.x_glycogen();
		const auto monod_glycogen = agent.monod_glycogen();
		const auto x_pha = agent.x_pha();
		const auto i_pha = agent.i_pha();
		const auto inhib_pha = agent.inhib_pha();

		// acetate uptake / pha synthesis
		if ((env.vfa_conc > 0) && (x_glycogen > 0) && (i_pha > 0))
		{
			// delta = vfa for easier calculation
			const auto delta = agent.trait.rate.q_pha * monod_vfa * monod_glycogen *
							   inhib_pha * agent.state.biomass;
			d_env.vfa_conc += delta;
			// d_glycogen + d_vfa = d_pha for conservation of mass
			d_state.glycogen -= delta * (agent.trait.reg.y_pha_hac - 1);
			d_state.pha += delta * agent.trait.reg.y_pha_hac;
		}
		// maintenance-bound biomass decay
		{
			const auto p_glycogen = monod_glycogen;
			// maintenance
			{
				// glycogen
				const auto delta = p_glycogen * agent.trait.rate.m_anaerobic *
								   agent_subtype_consts::GLYC_PER_ATP_ANA *
								   agent.state.biomass;
				d_state.glycogen -= delta;
				d_state.pha += delta * agent_subtype_consts::PHA_PER_GLYC_ANA_ATP;
			}
			// biomass decay
			{
				const auto delta = (1 - p_glycogen) * agent.trait.rate.b_anaerobic * agent.state.biomass;
				d_state.biomass -= delta;
				d_env.vfa_conc += delta * agent_subtype_consts::VFA_PER_DECAYED_BIOMASS;
				d_env.op_conc += delta * agent.trait.reg.i_bmp;
			}
		}
		// glycogen intrinsic decay, release as vfa
		if (x_glycogen > 0)
		{
			const auto delta = agent.trait.rate.b_glycogen * x_glycogen * agent.state.biomass;
			d_state.glycogen -= delta;
			d_env.vfa_conc += delta;
		}
		// pha intrinsic decay, release as vfa
		if (x_pha > 0)
		{
			const auto delta = agent.trait.rate.b_pha * x_pha * agent.state.biomass;
			d_state.pha -= delta;
			d_env.vfa_conc += delta;
		}
		// update to agent
		assert(d_state.rela_count == 0);
		assert(d_state.split_biomass == 0);
		agent.state.merge_with(d_state);
		assert((agent.state.rela_count != stvalue_inf) &&
			   (agent.state.rela_count != -stvalue_inf) &&
			   (agent.state.rela_count != stvalue_nan));
		return;
	}

} // namespace iebpr