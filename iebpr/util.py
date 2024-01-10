#!/usr/bin/env python3

import numpy
from . import _iebpr

# useful constants organized under namespaces


class Namespace(object):
	def __init__(self, **kw):
		for k, v in kw.items():
			setattr(self, k, v)
		return

	def __getitem__(self, key: str):
		return vars(self)[key]


RandType = Namespace(
	constant=_iebpr.constant,
	normal=_iebpr.normal,
	uniform=_iebpr.uniform,
	bernoulli=_iebpr.bernoulli,
	obsvalues=_iebpr.obsvalues,
)

AgentSubtype = Namespace(
	pao=_iebpr.pao,
	gao=_iebpr.gao,
	oho=_iebpr.oho,
)

def estimate_subtype_distrib_from_snapshot(snapshot: numpy.ndarray, field: str,
		*, use_rela_count=True) -> (numpy.ndarray, numpy.ndarray):
	if snapshot.ndim != 1:
		raise TypeError("snapshot must be 1-d array, not %u" % snapshot.ndim)
	if snapshot.dtype != _iebpr.AgentStateRecDescr:
		raise TypeError("snapshot dtype must be %s, not %s"
			% (_iebpr.AgentStateRecDescr, snapshot.dtype))
	# normalize field and sort (descend)
	normalized = snapshot[field] / snapshot["biomass"]
	sort_idx = numpy.argsort(normalized)[::-1]
	# calculate cumulated population frac
	if use_rela_count:
		pop_frac = snapshot["rela_count"][sort_idx]
		pop_cumu = numpy.cumsum(pop_frac)
		pop_cumu /= pop_cumu[-1]
	else:
		pop_cumu = numpy.linspace(1 / len(sort_idx), 1, len(sort_idx))
	return pop_cumu, normalized[sort_idx]

