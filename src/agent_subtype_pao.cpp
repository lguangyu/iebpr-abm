#include "iebpr/agent_subtype_pao.hpp"

namespace iebpr
{

	AgentSubtypeBase::subtype_enum AgentSubtypePao::subtype(void) const noexcept
	{
		return pao;
	}

	void AgentSubtypePao::agent_action_aerobic(const EnvState &env, EnvState &d_env, AgentData &agent)
	{
		// agent state change
		auto d_state = AgentState();

		// prepare data
		const auto x_glycogen = agent.x_glycogen();
		const auto monod_glycogen = agent.monod_glycogen();
		const auto i_glycogen = agent.i_glycogen();
		const auto inhib_glycogen = agent.inhib_glycogen();
		const auto x_pha = agent.x_pha();
		const auto monod_pha = agent.monod_pha();
		const auto x_polyp = agent.x_polyp();
		const auto monod_polyp = agent.monod_polyp();
		const auto i_polyp = agent.i_polyp();
		const auto inhib_polyp = agent.inhib_polyp();
		assert(monod_glycogen >= 0);
		assert(monod_glycogen <= 1);
		assert(monod_pha >= 0);
		assert(monod_pha <= 1);
		assert(monod_polyp >= 0);
		assert(monod_polyp <= 1);
		assert(inhib_polyp >= 0);
		assert(inhib_polyp <= 1);

		// glycogen synthesis
		if ((i_glycogen > 0) && (x_pha > 0))
		{
			const auto delta = agent.trait.s.rate.q_glycogen * inhib_glycogen *
							   monod_pha * agent.state.s.biomass;
			d_state.s.glycogen += delta;
			d_state.s.pha -= delta / agent.trait.s.reg.y_glycogen_pha;
		}
		// polyp synthesis
		if ((env.op_conc > 0) && (i_polyp > 0) && (x_pha > 0))
		{
			const auto monod_op_polyp = env.op_conc / (env.op_conc + agent.trait.s.reg.k_op_polyp);
			const auto delta = agent.trait.s.rate.q_polyp * monod_op_polyp *
							   monod_pha * inhib_polyp * agent.state.s.biomass;
			d_state.s.polyp += delta;
			d_state.s.pha -= delta / agent.trait.s.reg.y_polyp_pha;
			d_env.op_conc -= delta;
		}
		// biomass growth on pha, pao uses internal polyp as p source
		if ((x_pha > 0) && (x_polyp > 0))
		{
			const auto delta = agent.trait.s.rate.mu * monod_pha * monod_polyp * agent.state.s.biomass;
			d_state.s.biomass += delta;
			d_state.s.pha -= delta / agent.trait.s.reg.y_h;
			d_state.s.polyp -= delta * agent.trait.s.reg.i_bmp;
		}
		// maintenance (not bound with decay)
		{
			// maintenance
			// portion resolved by calculation order
			const auto p_pha = monod_pha;
			const auto p_glycogen = std::min(1 - p_pha, monod_glycogen);
			const auto p_polyp = std::min(1 - p_pha - p_glycogen, monod_polyp);
			assert(p_glycogen >= 0);
			assert(p_polyp >= 0);
			// belows are order-free
			// glycogen
			{
				const auto delta = p_glycogen * agent.trait.s.rate.m_aerobic *
								   agent_subtype_consts::GLYC_PER_ATP_AER *
								   agent.state.s.biomass;
				d_state.s.glycogen -= delta;
			}
			// pha
			{
				const auto delta = p_pha * agent.trait.s.rate.m_aerobic *
								   agent_subtype_consts::PHA_PER_ATP_AER *
								   agent.state.s.biomass;
				d_state.s.pha -= delta;
			}
			// polyp
			{
				const auto delta = p_polyp * agent.trait.s.rate.m_aerobic *
								   agent_subtype_consts::POLYP_PER_ATP *
								   agent.state.s.biomass;
				d_state.s.polyp -= delta;
				d_env.op_conc += delta;
			}
		}
		// biomass decay
		{
			const auto delta = agent.trait.s.rate.b_aerobic * agent.state.s.biomass;
			d_state.s.biomass -= delta;
			d_env.vfa_conc += delta * agent_subtype_consts::VFA_PER_DECAYED_BIOMASS;
			d_env.op_conc += delta * agent.trait.s.reg.i_bmp;
		}
		// glycogen intrinsic decay, release as vfa
		if (x_glycogen > 0)
		{
			const auto delta = agent.trait.s.rate.b_glycogen * x_glycogen * agent.state.s.biomass;
			d_state.s.glycogen -= delta;
			d_env.vfa_conc += delta;
		}
		// pha intrinsic decay, release as vfa
		if (x_pha > 0)
		{
			const auto delta = agent.trait.s.rate.b_pha * x_pha * agent.state.s.biomass;
			d_state.s.pha -= delta;
			d_env.vfa_conc += delta;
		}
		// polyp intrinsic decay, release as op
		if (x_polyp > 0)
		{
			const auto delta = agent.trait.s.rate.b_polyp * x_polyp * agent.state.s.biomass;
			d_state.s.polyp -= delta;
			d_env.op_conc += delta;
		}
		// update to agent
		assert(d_state.s.split_biomass == 0);
		agent.state.merge_with(d_state);
		return;
	}

	void AgentSubtypePao::agent_action_anaerobic(const EnvState &env, EnvState &d_env, AgentData &agent)
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
		const auto x_polyp = agent.x_polyp();
		const auto monod_polyp = agent.monod_polyp();

		// acetate uptake / pha synthesis (glycolysis)
		if ((env.vfa_conc > 0) && (x_glycogen > 0) && (i_pha > 0) && (x_polyp > 0))
		{
			// delta = vfa for easier calculation
			const auto delta = agent.trait.s.rate.q_pha * monod_vfa * monod_glycogen *
							   inhib_pha * monod_polyp * agent.state.s.biomass;
			d_env.vfa_conc -= delta;
			d_env.op_conc += delta * agent.trait.s.reg.y_prel;
			// d_glycogen + d_vfa = d_pha for conservation of mass
			d_state.s.glycogen -= delta * (agent.trait.s.reg.y_pha_hac - 1);
			d_state.s.pha += delta * agent.trait.s.reg.y_pha_hac;
			d_state.s.polyp -= delta * agent.trait.s.reg.y_prel;
		}
		// acetate uptake / pha synthesis (tca, if enable_tca = true)
		if ((env.vfa_conc > 0) && (i_pha > 0) && (x_polyp > 0) && agent.trait.s.b.enable_tca)
		{
			// delta = vfa for easier calculation
			const auto delta = agent.trait.s.rate.q_pha * monod_vfa * (1 - monod_glycogen) *
							   inhib_pha * monod_polyp * agent.state.s.biomass;
			d_env.vfa_conc -= delta;
			d_env.op_conc += delta * agent.trait.s.reg.y_prel;
			d_state.s.pha += delta; // conservation of mass
			d_state.s.polyp -= delta * agent.trait.s.reg.y_prel;
		}
		// maintenance-bound biomass decay
		{
			// maintenance
			// portion resolved by calculation order
			const auto p_glycogen = agent.trait.s.b.maint_polyp_first ? std::min(1 - monod_polyp, monod_glycogen) : monod_glycogen;
			const auto p_polyp = agent.trait.s.b.maint_polyp_first ? std::min(1 - monod_glycogen, monod_polyp) : monod_polyp;
			// belows are order-free
			// glycogen
			{
				const auto delta = p_glycogen * agent.trait.s.rate.m_anaerobic *
								   agent_subtype_consts::GLYC_PER_ATP_ANA *
								   agent.state.s.biomass;
				d_state.s.glycogen -= delta;
				d_state.s.pha += delta * agent_subtype_consts::PHA_PER_GLYC_ANA_ATP;
			}
			// polyp
			{
				const auto delta = p_polyp * agent.trait.s.rate.m_anaerobic *
								   agent_subtype_consts::POLYP_PER_ATP *
								   agent.state.s.biomass;
				d_state.s.polyp -= delta;
				d_env.op_conc += delta;
			}
			// biomass decay
			{
				const auto delta = (1 - p_glycogen - p_polyp) * agent.trait.s.rate.b_anaerobic * agent.state.s.biomass;
				d_state.s.biomass -= delta;
				d_env.vfa_conc += delta * agent_subtype_consts::VFA_PER_DECAYED_BIOMASS;
				d_env.op_conc += delta * agent.trait.s.reg.i_bmp;
			}
		}
		// glycogen intrinsic decay, release as vfa
		if (x_glycogen > 0)
		{
			const auto delta = agent.trait.s.rate.b_glycogen * x_glycogen * agent.state.s.biomass;
			d_state.s.glycogen -= delta;
			d_env.vfa_conc += delta;
		}
		// pha intrinsic decay, release as vfa
		if (x_pha > 0)
		{
			const auto delta = agent.trait.s.rate.b_pha * x_pha * agent.state.s.biomass;
			d_state.s.pha -= delta;
			d_env.vfa_conc += delta;
		}
		// polyp intrinsic decay, release as op
		if (x_polyp > 0)
		{
			const auto delta = agent.trait.s.rate.b_polyp * x_polyp * agent.state.s.biomass;
			d_state.s.polyp -= delta;
			d_env.op_conc += delta;
		}
		// update to agent
		assert(d_state.s.split_biomass == 0);
		agent.state.merge_with(d_state);
		return;
	}

} // namespace iebpr