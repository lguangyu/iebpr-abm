#ifndef __IEBPR_AGENT_SUBTYPE_BASE_HPP__
#define __IEBPR_AGENT_SUBTYPE_BASE_HPP__

#include <array>
#include <vector>
#include "randomizer.hpp"
#include "env_state.hpp"
#include "agent_data.hpp"
#include "agent_subtype_consts.hpp"

namespace iebpr
{
	class AgentPool;

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

		// fixed-size RandConfig array allowing access as both an array and an
		// struct; the struct name fields shall keep consistent with AgentState
		union StateRandConfig
		{
			constexpr static size_t array_size = AgentState::arr_size;

			std::array<Randomizer::RandConfig, array_size> arr;
			struct
			{
				Randomizer::RandConfig biomass;
				Randomizer::RandConfig rela_count;
				Randomizer::RandConfig split_biomass;
				Randomizer::RandConfig glycogen;
				Randomizer::RandConfig pha;
				Randomizer::RandConfig polyp;
			} s;

			// ensure aligned
			static_assert(sizeof(decltype(s)) == sizeof(decltype(arr)), "");

			explicit StateRandConfig(void)
				: arr(){};
			StateRandConfig(const StateRandConfig &other) noexcept
				: arr(other.arr){};
			~StateRandConfig(void){};
			StateRandConfig &operator=(const StateRandConfig &other) noexcept
			{
				arr = other.arr;
				return *this;
			};
		};

// ensure StateRandConfig is aligned with AgentTrait
#define _align_static_assert(attr)                                                                        \
	static_assert(offsetof(AgentSubtypeBase::StateRandConfig, s.attr) / sizeof(Randomizer::RandConfig) == \
					  offsetof(AgentState, s.attr) / agent_field_size,                                    \
				  #attr)

		_align_static_assert(biomass);
		_align_static_assert(rela_count);
		_align_static_assert(split_biomass);
		_align_static_assert(glycogen);
		_align_static_assert(pha);
		_align_static_assert(polyp);

#undef _align_static_assert

		// fixed-size RandConfig array allowing access as both an array and an
		// struct; the struct name fields shall keep consistent with AgentTrait
		union TraitRandConfig
		{
			constexpr static size_t array_size = AgentTrait::arr_size;

			std::array<Randomizer::RandConfig, array_size> arr;
			struct
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
			} s;

			// ensure aligned
			static_assert(sizeof(decltype(s)) == sizeof(decltype(arr)), "");

			explicit TraitRandConfig(void)
				: arr(){};
			TraitRandConfig(const TraitRandConfig &other) noexcept
				: arr(other.arr){};
			~TraitRandConfig(void){};
			inline TraitRandConfig &operator=(const TraitRandConfig &other) noexcept
			{
				arr = other.arr;
				return *this;
			};
		};

	private:
		Randomizer &_rand;

	public:
		StateRandConfig state_cfg;
		TraitRandConfig trait_cfg;
		const size_t n_agent;

	private:
		agent_itr_t _pool_begin;
		agent_itr_t _pool_end;

	public:
		explicit AgentSubtypeBase(Randomizer &rand, size_t n_agent)
			: _rand(rand), state_cfg(), trait_cfg(), n_agent(n_agent),
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

// ensure TraitRandConfig is aligned with AgentTrait
#define _align_static_assert(trc_attr, t_attr)                                                                \
	static_assert(offsetof(AgentSubtypeBase::TraitRandConfig, s.trc_attr) / sizeof(Randomizer::RandConfig) == \
					  offsetof(AgentTrait, s.t_attr) / agent_field_size,                                      \
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
		_align_static_assert(enable_tca, b.enable_tca);
		_align_static_assert(maint_polyp_first, b.maint_polyp_first);

#undef _align_static_assert
	};

} // namespace iebpr

#endif