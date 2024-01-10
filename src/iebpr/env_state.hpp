#ifndef __IEBPR_ENV_STATE_HPP__
#define __IEBPR_ENV_STATE_HPP__

#include "def.hpp"

namespace iebpr
{
	struct EnvState
	{
	public:
		stvalue_t volume;
		stvalue_t vfa_conc;
		stvalue_t op_conc;
		bivalue_t is_aerobic;

		explicit EnvState(void) noexcept
			: volume(0), vfa_conc(0), op_conc(0), is_aerobic(0) {}

		inline void update_change(const EnvState &env_change) noexcept
		{
			volume += env_change.volume;
			vfa_conc += env_change.vfa_conc;
			op_conc += env_change.op_conc;
			is_aerobic ^= env_change.is_aerobic;
			return;
		}
	};

	static_assert(sizeof(EnvState) == sizeof(stvalue_t) * 4, "");

} // namespace iebpr

#endif