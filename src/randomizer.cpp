#include <algorithm>
#include <cmath>
#include <cstring>
#include "iebpr/randomizer.hpp"

namespace iebpr
{
	bool Randomizer::is_valid_randtype_enum(rand_t type) noexcept
	{
		switch (type)
		{
		case constant:
		case normal:
		case uniform:
		case bernoulli:
		case obsvalues:
			return true;
		case none:
		case invalid:
		default:
			return false;
		}
	}

	const char *Randomizer::randtype_enum_to_name(rand_t type) noexcept
	{
		switch (type)
		{
		case none:
			return "none";
		case constant:
			return "constant";
		case normal:
			return "normal";
		case uniform:
			return "uniform";
		case bernoulli:
			return "bernoulli";
		case obsvalues:
			return "obsvalues";
		case invalid:
		default:
			return "invalid";
		}
	}

	void Randomizer::RandConfig::set_value_list(const decltype(value_list) &values)
	{
		value_list = values;
		std::sort(value_list.begin(), value_list.end());
		return;
	}

	error_enum Randomizer::RandConfig::validate(Randomizer::rand_t force_type) const noexcept
	{
		// type check
		if ((force_type != invalid) && (type != force_type))
			return unexpected_rand_type;
		// as normal, unlikely to draw a non-negative value
		if (non_neg && (type == normal) && (mean + stddev * 2 < 0))
			return normal_nonneg_lowprob;
		// as unifor, low <= high
		if ((type == uniform) && (low > high))
			return uniform_low_high_swap;
		// as uniform, impossible to draw a non-negative value
		if (non_neg && (type == uniform))
			if (((low == high) && (high < 0)) || // as constant, but < 0
				(high / (high - low) < 0.05))	 // both < 0 or low prob to gen > 0
				return uniform_nonneg_lowprob;
		// as bernoulli, 0 <= mean <= 1
		if ((type == bernoulli) && ((mean < 0) || (mean > 1)))
			return bernoulli_error_mean;
		return error_enum::none;
	}

	void Randomizer::seed(seed_t seed) noexcept
	{
		engine.seed(seed);
		return;
	}

	stvalue_t Randomizer::gen_value(const RandConfig &cfg)
	{
		stvalue_t ret;
		do
		{
			switch (cfg.type)
			{
			case constant:
				ret = cfg.mean * cfg._scale;
				break;
			case normal:
				ret = (normal_gen(engine) * cfg.stddev + cfg.mean) * cfg._scale;
				break;
			case uniform:
				ret = ((cfg.high - cfg.low) * uniform_gen(engine) + cfg.low) * cfg._scale;
				break;
			case bernoulli:
			{
				auto res = (bivalue_t)(uniform_gen(engine) <= cfg.mean);
				std::memcpy(&ret, &res, sizeof(ret));
				break;
			}
			case obsvalues:
				ret = _obsvalues_gen_handler(cfg) * cfg._scale;
				break;
			case none:
			default:
				std::memset(&ret, 0x0, sizeof(ret));
				break;
			}
		} while ((ret < 0) && cfg.non_neg &&
				 ((cfg.type == normal) || (cfg.type == uniform)));
		return ret;
	}

	stvalue_t Randomizer::_obsvalues_gen_handler(const RandConfig &cfg)
	{
		const auto &vals = cfg.value_list;

		if (vals.size() < 2)
			return cfg.mean;

		stvalue_t denorm = 0;
		for (size_t i = 0; i < vals.size(); i++)
			denorm += (i) ? (vals[i] + vals[i + 1]) / 2 : vals[i];
		denorm /= vals.size();

		// assumes that value_list is sorted in ascending order
		stvalue_t pos = uniform_gen(engine) * vals.size();
		if (pos <= 1)
		{
			return vals[0] / denorm * cfg.mean;
		}
		else
		{
			int i = (int)std::floor(pos);
			return ((vals[i] - vals[i - 1]) * (pos - i) + vals[i - 1]) / denorm * cfg.mean;
		}
	}

} // namespace iebpr