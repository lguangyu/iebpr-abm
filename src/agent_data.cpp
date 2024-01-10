#include "iebpr/agent_data.hpp"

namespace iebpr
{
	void AgentState::clear_state_content(void) noexcept
	{
		arr.fill(0);
		return;
	};

	void AgentState::scale_state_content(stvalue_t factor) noexcept
	{
		if (factor <= 0)
		{
			clear_state_content();
			return;
		}
		for (auto &v : arr)
			v *= factor;
		return;
	}

	void AgentState::merge_with(const AgentState &other, bool no_check) noexcept
	{
		s.biomass += other.s.biomass;
		if (!is_active())
		{
			clear_state_content();
			// no need to do the rest
			return;
		}
		s.rela_count += other.s.rela_count;
		s.split_biomass += other.s.split_biomass;
		s.glycogen += other.s.glycogen;
		s.pha += other.s.pha;
		s.polyp += other.s.polyp;
		if (no_check)
			return;
		if (s.rela_count < 0)
			s.rela_count = 0;
		if (s.split_biomass < 0)
			s.split_biomass = 0;
		if (s.glycogen < 0)
			s.glycogen = 0;
		if (s.pha < 0)
			s.pha = 0;
		if (s.polyp < 0)
			s.polyp = 0;
		return;
	}

	void AgentTrait::merge_with(const AgentTrait &other, stvalue_t coef_self) noexcept
	{
		auto coef_other = 1.0 - coef_self;

		// merge rate traits
		for (auto i = rate_arr_begin; i < rate_arr_end; i++)
			arr[i] = arr[i] * coef_self + other.arr[i] * coef_other;

		// merge regular traits
		for (auto i = reg_arr_begin; i < reg_arr_end; i++)
			arr[i] = arr[i] * coef_self + other.arr[i] * coef_other;

		// merge bool traits
		if (coef_self < 0.5)
			s.b = other.s.b;
		return;
	}

	void AgentData::merge_with(const AgentData &other) noexcept
	{
		// stvalue_t coef_self;
		stvalue_t coef_self = ((!is_active()) && (!other.is_active())) ? 0.5 : state.s.biomass / (state.s.biomass + other.state.s.biomass);
		state.merge_with(other.state);
		trait.merge_with(other.trait, coef_self);
		return;
	}

} // namespace iebpr