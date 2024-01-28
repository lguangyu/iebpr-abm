#include "iebpr/agent_data.hpp"

namespace iebpr
{
	void AgentState::clear_state_content(void) noexcept
	{
		std::memset(this, 0, sizeof(AgentState));
		return;
	};

	void AgentState::scale_state_content(stvalue_t factor) noexcept
	{
		if (factor <= 0)
		{
			clear_state_content();
			return;
		}

		stvalue_t *const val_ptr = this->as_arr();
		for (size_t i = 0; i < arr_size(); i++)
			*(val_ptr + i) *= factor;
		return;
	}

	void AgentState::merge_with(const AgentState &other, bool no_check) noexcept
	{
		biomass += other.biomass;
		if (!is_active())
		{
			clear_state_content();
			// no need to do the rest
			return;
		}
		rela_count += other.rela_count;
		split_biomass += other.split_biomass;
		glycogen += other.glycogen;
		pha += other.pha;
		polyp += other.polyp;
		if (no_check)
			return;
		if (rela_count < 0)
			rela_count = 0;
		if (split_biomass < 0)
			split_biomass = 0;
		if (glycogen < 0)
			glycogen = 0;
		if (pha < 0)
			pha = 0;
		if (polyp < 0)
			polyp = 0;
		return;
	}

	void AgentTrait::merge_with(const AgentTrait &other, stvalue_t coef_self) noexcept
	{
		auto coef_other = 1.0 - coef_self;

		stvalue_t *const val_ptr = this->as_arr();
		const stvalue_t *const oth_ptr = other.as_arr();

		// merge rate traits
		for (auto i = rate_begin(); i < rate_end(); i++)
			val_ptr[i] = val_ptr[i] * coef_self + oth_ptr[i] * coef_other;

		// merge regular traits
		for (auto i = reg_begin(); i < reg_end(); i++)
			val_ptr[i] = val_ptr[i] * coef_self + oth_ptr[i] * coef_other;

		// merge bool traits
		if (coef_self < 0.5)
			bt = other.bt;
		return;
	}

	void AgentData::merge_with(const AgentData &other) noexcept
	{
		// stvalue_t coef_self;
		stvalue_t coef_self = ((!is_active()) && (!other.is_active())) ? 0.5 : state.biomass / (state.biomass + other.state.biomass);
		state.merge_with(other.state);
		trait.merge_with(other.trait, coef_self);
		return;
	}

} // namespace iebpr