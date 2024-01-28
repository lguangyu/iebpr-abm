#ifndef __IEBPR_AGENT_DATA_HPP__
#define __IEBPR_AGENT_DATA_HPP__

#include "def.hpp"
#include "env_state.hpp"
#include <cstring>

namespace iebpr
{
	struct AgentState
	{
		stvalue_t biomass;
		stvalue_t rela_count;	 // relative cell count
		stvalue_t split_biomass; // biomass to trigger split
		stvalue_t glycogen;		 // store as absolute value, not content
		stvalue_t pha;			 // store as absolute value, not content
		stvalue_t polyp;		 // store as absolute value, not content
		// array-like access utility
		with_access_as_arr(AgentState, stvalue_t);

		explicit AgentState(void) noexcept
			: biomass(0), split_biomass(0), glycogen(0), pha(0), polyp(0)
		{
		}

		//======================================================================
		// INTERNAL API
		//======================================================================

		// return true if agent is active
		inline bool is_active(void) const noexcept { return biomass > 0; };
		// true if biomass >= split_biomass
		inline bool can_split(void) const noexcept { return is_active() && (biomass >= split_biomass); }
		// clear content
		void clear_state_content(void) noexcept;
		// scale content
		void scale_state_content(stvalue_t factor) noexcept;
		// merge another agent with this one
		void merge_with(const AgentState &other, bool no_check = false) noexcept;
	};

	struct AgentRateTrait
	{
	public:
		// synthesis rate
		stvalue_t mu;
		stvalue_t q_glycogen;
		stvalue_t q_pha;
		stvalue_t q_polyp;
		// maintenance rate
		stvalue_t m_aerobic;
		stvalue_t m_anaerobic;
		// decay rate
		stvalue_t b_aerobic;
		stvalue_t b_anaerobic;
		stvalue_t b_glycogen;
		stvalue_t b_pha;
		stvalue_t b_polyp;
		// array-like access utility
		with_access_as_arr(AgentRateTrait, stvalue_t);
	};

	struct AgentRegularTrait
	{
	public:
		// mix/max quota
		stvalue_t x_glycogen_min;
		stvalue_t x_glycogen_max;
		stvalue_t x_pha_min;
		stvalue_t x_pha_max;
		stvalue_t x_polyp_min;
		stvalue_t x_polyp_max;
		// half-sat
		stvalue_t k_hac;
		stvalue_t k_op;
		stvalue_t k_op_polyp;
		stvalue_t k_glycogen;
		stvalue_t k_pha;
		stvalue_t k_polyp;
		// half-sat, inhibitory
		stvalue_t ki_glycogen;
		stvalue_t ki_pha;
		stvalue_t ki_polyp;
		// yield coef
		stvalue_t y_h;
		stvalue_t y_glycogen_pha;
		stvalue_t y_polyp_pha;
		stvalue_t y_pha_hac;
		stvalue_t y_prel;
		stvalue_t i_bmp;
		// array-like access utility
		with_access_as_arr(AgentRegularTrait, stvalue_t);
	};

	struct AgentBoolTrait
	{
	public:
		bivalue_t enable_tca;
		bivalue_t maint_polyp_first;
		// array-like access utility
		with_access_as_arr(AgentBoolTrait, stvalue_t);
	};

	struct AgentTrait
	{
		AgentRateTrait rate;
		AgentRegularTrait reg;
		AgentBoolTrait bt;
		// array-like access utility
		with_access_as_arr(AgentTrait, stvalue_t);
		// begin offset
		static constexpr size_t rate_begin() { return offsetof(AgentTrait, rate) / agent_field_size; };
		static constexpr size_t reg_begin() { return offsetof(AgentTrait, reg) / agent_field_size; };
		static constexpr size_t bt_begin() { return offsetof(AgentTrait, bt) / agent_field_size; };
		// end offset
		static constexpr size_t rate_end() { return rate_begin() + AgentRateTrait::arr_size(); };
		static constexpr size_t reg_end() { return reg_begin() + AgentRegularTrait::arr_size(); };
		static constexpr size_t bt_end() { return bt_begin() + AgentBoolTrait::arr_size(); };

		explicit AgentTrait(void) noexcept
		{
			std::memset(this, 0, sizeof(AgentTrait));
		}

		//======================================================================
		// INTERNAL API
		//======================================================================

		// merge another agent with this one, the coefficients are combined
		// as the arithmetic mean with weight (coef_self) and (1 - coef_self)
		void merge_with(const AgentTrait &other, stvalue_t coef_self) noexcept;
	};

	struct AgentData
	{
	public:
		AgentState state;
		AgentTrait trait;

		//======================================================================
		// INTERNAL API
		//======================================================================

		// return ture if agent is active, i.d. to call state.is_active();
		inline bool is_active(void) const noexcept { return state.is_active(); };
		// true if biomass >= split_biomass
		inline bool can_split(void) const noexcept { return state.can_split(); }
		// merge another agent with this one
		void merge_with(const AgentData &other) noexcept;

		//======================================================================
		// INTERNAL API / COMPILE-TIME STRCUT REFLECTION
		//======================================================================

		//======================================================================
		// INTERNAL API / FREQUENTLY USED AGENT CALCULATION MACROS
		//======================================================================

		inline stvalue_t monod_vfa(const EnvState &env) const noexcept { return env.vfa_conc / (env.vfa_conc + trait.reg.k_hac); };
		inline stvalue_t monod_op(const EnvState &env) const noexcept { return env.op_conc / (env.op_conc + trait.reg.k_op); };
		inline stvalue_t x_glycogen(void) const noexcept { return is_active() ? state.glycogen / state.biomass - trait.reg.x_glycogen_min : 0; };
		inline stvalue_t x_pha(void) const noexcept { return is_active() ? state.pha / state.biomass - trait.reg.x_pha_min : 0; };
		inline stvalue_t x_polyp(void) const noexcept { return is_active() ? state.polyp / state.biomass - trait.reg.x_polyp_min : 0; };
		inline stvalue_t monod_glycogen(void) const noexcept { return x_glycogen() / (x_glycogen() + trait.reg.k_glycogen); };
		inline stvalue_t monod_pha(void) const noexcept { return x_pha() / (x_pha() + trait.reg.k_pha); };
		inline stvalue_t monod_polyp(void) const noexcept { return x_polyp() / (x_polyp() + trait.reg.k_polyp); };
		inline stvalue_t i_glycogen(void) const noexcept { return is_active() ? trait.reg.x_glycogen_max - state.glycogen / state.biomass : 0; };
		inline stvalue_t i_pha(void) const noexcept { return is_active() ? trait.reg.x_pha_max - state.pha / state.biomass : 0; };
		inline stvalue_t i_polyp(void) const noexcept { return is_active() ? trait.reg.x_polyp_max - state.polyp / state.biomass : 0; };
		inline stvalue_t inhib_glycogen(void) const noexcept { return i_glycogen() / (i_glycogen() + trait.reg.ki_glycogen); };
		inline stvalue_t inhib_pha(void) const noexcept { return i_pha() / (i_pha() + trait.reg.ki_pha); };
		inline stvalue_t inhib_polyp(void) const noexcept { return i_polyp() / (i_polyp() + trait.reg.ki_polyp); };
	};

} // namespace iebpr

#endif