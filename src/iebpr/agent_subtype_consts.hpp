#ifndef __AGENT_SUBTYPE_CONSTS_HPP__
#define __AGENT_SUBTYPE_CONSTS_HPP__

#include "def.hpp"

namespace iebpr
{
	namespace agent_subtype_consts
	{
		// AgentState.split_biomass will not be smaller than this value times
		// the initial AgentState.biomass
		constexpr stvalue_t MIN_RELA_SPLIT_BIOMASS = 1.5;
		// the initial AgentState.rela_count constant
		constexpr stvalue_t INIT_RELA_COUNT = 1.0;

		// mgP polyp equivalence per mmol ATP (mgP polyp/mmol ATP)
		constexpr stvalue_t POLYP_PER_ATP = 0.907;
		// mgCOD glycogen equivalence per mmol ATP produced anaerobically (mgCOD glycogen/mmol ATP)
		constexpr stvalue_t GLYC_PER_ATP_ANA = 1.41;
		// mgCOD glycogen equivalence per mmol ATP produced aerobically (mgCOD glycogen/mmol ATP)
		constexpr stvalue_t GLYC_PER_ATP_AER = 0.215;
		// mgCOD pha equivalence per mmol ATP produced aerobically (mgCOD pha/mmol ATP)
		constexpr stvalue_t PHA_PER_ATP_AER = 0.226;
		// vfa equivalence released per unit biomass decay (mgCOD/mgCOD biomass)
		constexpr stvalue_t VFA_PER_DECAYED_BIOMASS = 1.0;
		// pha production per glycogen for pure atp production
		constexpr stvalue_t PHA_PER_GLYC_ANA_ATP = 0.833;

	} // namespace agent_subtype_consts

} // namespace iebpr

#endif