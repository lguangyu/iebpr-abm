#ifndef __IEBPR_AGENT_SUBTYPE_BASE_TRAIT_CFG_HPP__
#define __IEBPR_AGENT_SUBTYPE_BASE_TRAIT_CFG_HPP__

#include "randomizer.hpp"
#include "agent_data.hpp"
#include "agent_subtype_consts.hpp"

namespace iebpr
{
	struct TraitRandConfig
	{
		Randomizer::RandConfig mu;
		Randomizer::RandConfig q_glycogen;
		Randomizer::RandConfig q_pha;
		Randomizer::RandConfig q_polyp;
		Randomizer::RandConfig m_aerobic;
		Randomizer::RandConfig m_anaerobic;
		Randomizer::RandConfig b_aerobic;
		Randomizer::RandConfig b_anaerobic;
		Randomizer::RandConfig b_glycogen;
		Randomizer::RandConfig b_pha;
		Randomizer::RandConfig b_polyp;
		Randomizer::RandConfig x_glycogen_min;
		Randomizer::RandConfig x_glycogen_max;
		Randomizer::RandConfig x_pha_min;
		Randomizer::RandConfig x_pha_max;
		Randomizer::RandConfig x_polyp_min;
		Randomizer::RandConfig x_polyp_max;
		Randomizer::RandConfig k_hac;
		Randomizer::RandConfig k_op;
		Randomizer::RandConfig k_op_polyp;
		Randomizer::RandConfig k_glycogen;
		Randomizer::RandConfig k_pha;
		Randomizer::RandConfig k_polyp;
		Randomizer::RandConfig ki_glycogen;
		Randomizer::RandConfig ki_pha;
		Randomizer::RandConfig ki_polyp;
		Randomizer::RandConfig y_h;
		Randomizer::RandConfig y_glycogen_pha;
		Randomizer::RandConfig y_polyp_pha;
		Randomizer::RandConfig y_pha_hac;
		Randomizer::RandConfig y_prel;
		Randomizer::RandConfig i_bmp;
		Randomizer::RandConfig enable_tca;
		Randomizer::RandConfig maint_polyp_first;
		// array-like access
		with_access_as_arr(TraitRandConfig, Randomizer::RandConfig);

		explicit TraitRandConfig(void) noexcept {}; // use member default ctor

		// apply adjust to n_agent
		void adjust_to_timestep(stvalue_t timestep) noexcept;
		// generate set of values
		void randomize(Randomizer &rand, AgentTrait &trait);
	};

// ensure TraitRandConfig is aligned with AgentTrait
#define _align_static_assert(trc_attr, t_attr)                                            \
	static_assert(offsetof(TraitRandConfig, trc_attr) / sizeof(Randomizer::RandConfig) == \
					  offsetof(AgentTrait, t_attr) / agent_field_size,                    \
				  #trc_attr)
	_align_static_assert(mu, rate.mu);
	_align_static_assert(q_glycogen, rate.q_glycogen);
	_align_static_assert(q_pha, rate.q_pha);
	_align_static_assert(q_polyp, rate.q_polyp);
	_align_static_assert(m_aerobic, rate.m_aerobic);
	_align_static_assert(m_anaerobic, rate.m_anaerobic);
	_align_static_assert(b_aerobic, rate.b_aerobic);
	_align_static_assert(b_anaerobic, rate.b_anaerobic);
	_align_static_assert(b_glycogen, rate.b_glycogen);
	_align_static_assert(b_pha, rate.b_pha);
	_align_static_assert(b_polyp, rate.b_polyp);
	_align_static_assert(x_glycogen_min, reg.x_glycogen_min);
	_align_static_assert(x_glycogen_max, reg.x_glycogen_max);
	_align_static_assert(x_pha_min, reg.x_pha_min);
	_align_static_assert(x_pha_max, reg.x_pha_max);
	_align_static_assert(x_polyp_min, reg.x_polyp_min);
	_align_static_assert(x_polyp_max, reg.x_polyp_max);
	_align_static_assert(k_hac, reg.k_hac);
	_align_static_assert(k_op, reg.k_op);
	_align_static_assert(k_op_polyp, reg.k_op_polyp);
	_align_static_assert(k_glycogen, reg.k_glycogen);
	_align_static_assert(k_pha, reg.k_pha);
	_align_static_assert(k_polyp, reg.k_polyp);
	_align_static_assert(ki_glycogen, reg.ki_glycogen);
	_align_static_assert(ki_pha, reg.ki_pha);
	_align_static_assert(ki_polyp, reg.ki_polyp);
	_align_static_assert(y_h, reg.y_h);
	_align_static_assert(y_glycogen_pha, reg.y_glycogen_pha);
	_align_static_assert(y_polyp_pha, reg.y_polyp_pha);
	_align_static_assert(y_pha_hac, reg.y_pha_hac);
	_align_static_assert(y_prel, reg.y_prel);
	_align_static_assert(i_bmp, reg.i_bmp);
	_align_static_assert(enable_tca, bt.enable_tca);
	_align_static_assert(maint_polyp_first, bt.maint_polyp_first);
#undef _align_static_assert

} // namespace iebpr

#endif