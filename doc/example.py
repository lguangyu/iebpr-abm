import numpy
import matplotlib.pyplot

import iebpr
from iebpr import Simulation, EnvState, SbrPhase, SbrStage, RandType, \
	AgentSubtype, RandConfig, StateRandConfig, TraitRandConfig

################################################################################
# create simulation object
################################################################################
simulation = Simulation(seed=0, pcontinuous=True, timestep=1e-5)

################################################################################
# setup init environment states
################################################################################
simulation.init_env = EnvState(
	volume=40,  # in L
	vfa_conc=0,  # in mgCOD/L
	op_conc=0,  # in mgP/L
)

################################################################################
# setup SBR control
################################################################################
# create phases first
inflow_phase = SbrPhase(
	time_len=30 / 60 / 24,  # 30min->day, the duration
	inflow_rate=5 / (30 / 60 / 24),  # input 5L/30min -> L/day
	inflow_vfa_conc=200,  # in mgCOD/L
	inflow_op_conc=25,  # in mgP/L
	aeration=False,
)

anaerobic_phase = SbrPhase(
	time_len=90 / 60 / 24,
	aeration=False,
)

aerobic_phase = SbrPhase(
	time_len=180 / 60 / 24,
	aeration=True,
)

withdraw_phase = SbrPhase(
	time_len=30 / 60 / 24,
	# withdraw discards biomass based on outflow volume
	# vfa, op, and biomass conc. remain the same during withdraw
	withdraw_rate=1 / (30 / 60 / 24),
	aeration=True,
)

outflow_phase = SbrPhase(
	time_len=30 / 60 / 24,
	# outflow reduces reactor living volume without reducing biomass
	# this will result in biomass conc. increase
	outflow_rate=4 / (30 / 60 / 24),
	aeration=True,
)

# create stage from phases configs
stage = SbrStage(
	n_cycle=100,
	cycle_phases=[
		inflow_phase,
		anaerobic_phase,
		aerobic_phase,
		withdraw_phase,
		outflow_phase,
	]
)

# check if flow is balanced
print("stage flow balanced:", stage.is_flow_balanced())

# add to SBR control
simulation.append_sbr_stage(stage)

################################################################################
# setup agents
################################################################################
# pao
simulation.add_agent_subtype(
	AgentSubtype.pao,
	n_agent=100,
	state_cfg=StateRandConfig(
		biomass=RandConfig(RandType.normal, mean=100, stddev=10),
		glycogen=RandConfig(RandType.normal, mean=10, stddev=2),
		pha=RandConfig(RandType.normal, mean=20, stddev=2),
		polyp=RandConfig(RandType.normal, mean=15, stddev=2),
	),
	trait_cfg=iebpr.agent_template.randconfig_from_template_json(
		"example.pao_trait.json"
	),
)

# gao
simulation.add_agent_subtype(
	AgentSubtype.gao,
	n_agent=100,
	state_cfg=StateRandConfig(
		biomass=RandConfig(RandType.normal, mean=100, stddev=10),
		glycogen=RandConfig(RandType.normal, mean=10, stddev=2),
		pha=RandConfig(RandType.normal, mean=20, stddev=2),
	),
	trait_cfg=iebpr.agent_template.randconfig_from_template_json(
		"example.gao_trait.json"
	),
)

# oho
simulation.add_agent_subtype(
	AgentSubtype.oho,
	n_agent=50,
	state_cfg=StateRandConfig(
		biomass=RandConfig(RandType.normal, mean=100, stddev=10),
	),
	trait_cfg=iebpr.agent_template.randconfig_from_template_json(
		"example.oho_trait.json"
	),
)

# list of subtype name and number of agents, for plot use
subtypes = simulation.n_agent_by_subtype

################################################################################
# setup recording timpoints
################################################################################
last_cycle_start_time = simulation.total_time_len - stage.cycle_time_len

# state_rec_timepoints
# record the bulk-level environment and agent states during the last cycle
timepoints = numpy.linspace(
	last_cycle_start_time,  # start of the last cycle
	simulation.total_time_len,  # end of the last cycle
	1000,  # number of recording timepoints
)
simulation.set_state_rec_timepoints(timepoints)

# snapshot_rec_timepoints
timepoints = [
	# start of anaerobic phase
	last_cycle_start_time + inflow_phase.time_len,
	# end of anaerobic phase / start of aerobic phase
	last_cycle_start_time + inflow_phase.time_len + anaerobic_phase.time_len,
	# end of aerobic phase
	last_cycle_start_time + inflow_phase.time_len + anaerobic_phase.time_len
		+ anaerobic_phase.time_len,
]
simulation.set_snapshot_rec_timepoints(timepoints)

################################################################################
# run simulation
################################################################################
simulation.run()
print("last run duration: %.3fsec" % (simulation.last_run_duration / 1000))


################################################################################
# retrieve recorded data
################################################################################
# retrieve recording data from the simulation by
env_state_rec = simulation.retrieve_env_state_rec()
# plot env state record data
matplotlib.pyplot.plot(
    simulation.get_state_rec_timepoints(), env_state_rec["op_conc"])
matplotlib.pyplot.ylim(0, None)
matplotlib.pyplot.xlabel("day")
matplotlib.pyplot.ylabel("op (mgP/L)")
matplotlib.pyplot.savefig("example.output.op_conc.png", dpi=300)
matplotlib.pyplot.close()

# similarly, retrieve recording data by
agent_state_rec = simulation.retrieve_agent_state_rec()
# plot env state record data
plot_subtype = 0
matplotlib.pyplot.plot(simulation.get_state_rec_timepoints(),
	agent_state_rec[:, plot_subtype]["polyp"])
matplotlib.pyplot.ylim(0, None)
matplotlib.pyplot.xlabel("day")
matplotlib.pyplot.ylabel("polyp (mgP/L)")
matplotlib.pyplot.title(subtypes[plot_subtype][0])
matplotlib.pyplot.savefig("example.output.pao_polyp_conc.png", dpi=300)
matplotlib.pyplot.close()

# similarly, retrieve recording data by
snapshot_rec = simulation.retrieve_snapshot_rec()
snapshot_idx = 2  # the 3rd snapshot recording (end of aerobic phase)
# use this utility function to esimate the distribution
field = "polyp"
pop_cumu, norm_content = iebpr.util.estimate_subtype_distrib_from_snapshot(
	snapshot_rec[plot_subtype][snapshot_idx], field=field,
)
matplotlib.pyplot.plot(pop_cumu, norm_content)
matplotlib.pyplot.xlim(0, 1)
matplotlib.pyplot.ylim(0, None)
matplotlib.pyplot.xlabel("population fraction")
matplotlib.pyplot.ylabel("%s content (mgP/mgCOD biomass)" % field)
matplotlib.pyplot.title("%s in %s at day %.2f" % (field, subtypes[plot_subtype][0],
	simulation.get_snapshot_rec_timepoints()[snapshot_idx])
)
matplotlib.pyplot.savefig("example.output.pao_polyp_snapshot.png", dpi=300)
matplotlib.pyplot.close()
