#include "iebpr/agent_subtype_base_trait_cfg.hpp"

namespace iebpr
{
	void TraitRandConfig::adjust_to_timestep(stvalue_t timestep) noexcept
	{
		// refresh all rate scale to 1
		for (size_t i = 0; i < arr_size(); i++)
			as_arr()[i]._scale = 1;
		// apply timestep as scale to these specific rate configs
		for (auto i = AgentTrait::rate_begin(); i < AgentTrait::rate_end(); i++)
			as_arr()[i]._scale = timestep;
		return;
	}

	void TraitRandConfig::randomize(Randomizer &rand, AgentTrait &trait)
	{
		auto *const val_ptr = trait.as_arr();
		const auto *const cfg_ptr = as_arr();

		// generate rate traits
		for (auto i = AgentTrait::rate_begin(); i < AgentTrait::rate_end(); i++)
			val_ptr[i] = rand.gen_value(cfg_ptr[i]);

		// generate regula traits
		for (auto i = AgentTrait::reg_begin(); i < AgentTrait::reg_end(); i++)
			val_ptr[i] = rand.gen_value(cfg_ptr[i]);

		// generate bool traits
		for (auto i = AgentTrait::bt_begin(); i < AgentTrait::bt_end(); i++)
		{
			auto v = rand.gen_value(cfg_ptr[i]);
			std::memcpy(val_ptr + i, &v, agent_field_size);
		}
		return;
	}

} // namespace iebpr