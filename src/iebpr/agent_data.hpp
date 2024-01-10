#ifndef __IEBPR_AGENT_DATA_HPP__
#define __IEBPR_AGENT_DATA_HPP__

#include <array>
#include "def.hpp"
#include "env_state.hpp"

namespace iebpr
{
	struct AgentStateStruct
	{
	public:
		stvalue_t biomass;
		stvalue_t rela_count;	 // relative cell count
		stvalue_t split_biomass; // biomass to trigger split
		stvalue_t glycogen;		 // store as absolute value, not content
		stvalue_t pha;			 // store as absolute value, not content
		stvalue_t polyp;		 // store as absolute value, not content

		explicit AgentStateStruct(void) noexcept
			: biomass(0), split_biomass(0), glycogen(0), pha(0), polyp(0)
		{
		}
	};

	union AgentState
	{
		AgentStateStruct s;
		constexpr static size_t arr_size = stvalue_arr_size<decltype(s)>();
		std::array<stvalue_t, arr_size> arr;
		// ensure aligned
		static_assert(sizeof(decltype(s)) == sizeof(decltype(arr)), "");

		explicit AgentState(void) noexcept
			: s(){};
		~AgentState(void){};

		//======================================================================
		// INTERNAL API
		//======================================================================

		// return true if agent is active
		inline bool is_active(void) const noexcept { return s.biomass > 0; };
		// true if biomass >= split_biomass
		inline bool can_split(void) const noexcept { return is_active() && (s.biomass >= s.split_biomass); }
		// clear content
		void clear_state_content(void) noexcept;
		// scale content
		void scale_state_content(stvalue_t factor) noexcept;
		// merge another agent with this one
		void merge_with(const AgentState &other, bool no_check = false) noexcept;
	};

	struct AgentRateTraitStruct
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
	};

	struct AgentRegularTraitStruct
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
	};

	struct AgentBoolTraitStruct
	{
	public:
		bivalue_t enable_tca;
		bivalue_t maint_polyp_first;
	};

	struct AgentTraitStruct
	{
		AgentRateTraitStruct rate;
		AgentRegularTraitStruct reg;
		AgentBoolTraitStruct b;
	};

	static_assert(sizeof(AgentTraitStruct) == sizeof(AgentRateTraitStruct) + sizeof(AgentRegularTraitStruct) + sizeof(AgentBoolTraitStruct), "");

	union AgentTrait
	{
		AgentTraitStruct s;
		constexpr static auto arr_size = stvalue_arr_size<decltype(s)>();
		std::array<stvalue_t, arr_size> arr;
		// array access to the sub structs
		constexpr static auto rate_struct_size = stvalue_arr_size<decltype(s.rate)>();
		constexpr static auto reg_struct_size = stvalue_arr_size<decltype(s.reg)>();
		constexpr static auto b_struct_size = stvalue_arr_size<decltype(s.b)>();
		constexpr static auto rate_arr_begin = offsetof(decltype(s), rate) / agent_field_size;
		constexpr static auto rate_arr_end = rate_arr_begin + rate_struct_size;
		constexpr static auto reg_arr_begin = offsetof(decltype(s), reg) / agent_field_size;
		constexpr static auto reg_arr_end = reg_arr_begin + reg_struct_size;
		constexpr static auto b_arr_begin = offsetof(decltype(s), b) / agent_field_size;
		constexpr static auto b_arr_end = b_arr_begin + b_struct_size;
		// ensure aligned
		static_assert(sizeof(decltype(s)) == sizeof(decltype(arr)), "");

		explicit AgentTrait(void) noexcept
		{
			for (auto &v : arr)
				v = 0;
		}
		~AgentTrait(void){};

		//======================================================================
		// INTERNAL API
		//======================================================================

		// merge another agent with this one, the coefficients are combined
		// as the arithmetic mean with weight (coef_self) and (1 - coef_self)
		void merge_with(const AgentTrait &other, stvalue_t coef_self) noexcept;
	};

	static_assert(sizeof(AgentTraitStruct) == sizeof(AgentTrait), "");

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

		inline stvalue_t monod_vfa(const EnvState &env) const noexcept { return env.vfa_conc / (env.vfa_conc + trait.s.reg.k_hac); };
		inline stvalue_t monod_op(const EnvState &env) const noexcept { return env.op_conc / (env.op_conc + trait.s.reg.k_op); };
		inline stvalue_t x_glycogen(void) const noexcept { return is_active() ? state.s.glycogen / state.s.biomass - trait.s.reg.x_glycogen_min : 0; };
		inline stvalue_t x_pha(void) const noexcept { return is_active() ? state.s.pha / state.s.biomass - trait.s.reg.x_pha_min : 0; };
		inline stvalue_t x_polyp(void) const noexcept { return is_active() ? state.s.polyp / state.s.biomass - trait.s.reg.x_polyp_min : 0; };
		inline stvalue_t monod_glycogen(void) const noexcept { return x_glycogen() / (x_glycogen() + trait.s.reg.k_glycogen); };
		inline stvalue_t monod_pha(void) const noexcept { return x_pha() / (x_pha() + trait.s.reg.k_pha); };
		inline stvalue_t monod_polyp(void) const noexcept { return x_polyp() / (x_polyp() + trait.s.reg.k_polyp); };
		inline stvalue_t i_glycogen(void) const noexcept { return is_active() ? trait.s.reg.x_glycogen_max - state.s.glycogen / state.s.biomass : 0; };
		inline stvalue_t i_pha(void) const noexcept { return is_active() ? trait.s.reg.x_pha_max - state.s.pha / state.s.biomass : 0; };
		inline stvalue_t i_polyp(void) const noexcept { return is_active() ? trait.s.reg.x_polyp_max - state.s.polyp / state.s.biomass : 0; };
		inline stvalue_t inhib_glycogen(void) const noexcept { return i_glycogen() / (i_glycogen() + trait.s.reg.ki_glycogen); };
		inline stvalue_t inhib_pha(void) const noexcept { return i_pha() / (i_pha() + trait.s.reg.ki_pha); };
		inline stvalue_t inhib_polyp(void) const noexcept { return i_polyp() / (i_polyp() + trait.s.reg.ki_polyp); };
	};
} // namespace iebpr

#endif