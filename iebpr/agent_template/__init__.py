#!/usr/bin/env python3

import importlib.resources
import json
import numbers
import typing

from iebpr.util import RandType
from iebpr import RandConfig, StateRandConfig, TraitRandConfig


_template_data = {
	# state
	"pao_state": {
		"resource_file": "pao_state_template.json",
		"cls": StateRandConfig,
	},
	"gao_state": {
		"resource_file": "gao_state_template.json",
		"cls": StateRandConfig,
	},
	"oho_state": {
		"resource_file": "oho_state_template.json",
		"cls": StateRandConfig,
	},
	# trait
	"pao_trait": {
		"resource_file": "pao_trait_template.json",
		"cls": TraitRandConfig,
	},
	"gao_trait": {
		"resource_file": "gao_trait_template.json",
		"cls": TraitRandConfig,
	},
	"oho_trait": {
		"resource_file": "oho_trait_template.json",
		"cls": TraitRandConfig,
	},
}

_fields_required_by_type = {
	"constant": ["mean"],
	"normal": ["mean", "stddev"],
	"uniform": ["low", "high"],
	"bernoulli": ["mean"],
	"obsvalues": ["mean", "value_list"],
}


def _load_resource_json(resource_file, **kw):
	r = importlib.resources.open_text("iebpr.agent_template", resource_file)
	return json.load(r, **kw)


def get_template(name: str) -> dict:
	"""
	get a minimally configured agent subtype parameter set for building
	StateRandConfig and TraitRandConfig; these templates only indicate which
	parameters are needed by the related subtype; not their configure forms nor
	values indicate recommended settings or the "default" values;
	"""
	if name not in _template_data:
		raise ValueError("template '%s' not found\nchoose from: %s" % (
			name, (", ").join(sorted(_template_data.keys()))
		))
	return _load_resource_json(_template_data[name]["resource_file"])


def _sanitize_template_value_dict(v: dict) -> None:
	if "type" not in v:
		raise ValueError("key 'type' is missing")
	t = v["type"]
	if t not in vars(RandType):
		raise ValueError("distribution type '%s' is invalid")
	# check if required fields are set
	for field in _fields_required_by_type[t]:
		if field not in v:
			raise ValueError("distribution type '%s' requires field '%s'"
				% (t, field))
	return


def _sanitize_template_value(v: typing.Union[float, dict]) -> None:
	if isinstance(v, dict):
		return _sanitize_template_value_dict(v)
	elif isinstance(v, numbers.Real):
		return
	elif isinstance(v, str):
		float(v)  # if it fails, then it fails
		return
	else:
		raise TypeError("value must be real, real-like str, or dict, not %s"
			% type(v).__name__)
	return


def sanitize_template(t: dict) -> None:
	"""
	check if the values from a template have correct types and required fields
	are all configured; this check does not verify if the value themselved are
	meaningful;

	check passes if no TypeError or ValueError is raised.
	"""
	if not isinstance(t, dict):
		raise TypeError("must be dict, not %s" % type(t).__name__)
	for v in t.values():
		_sanitize_template_value(v)
	return


def _randconfig_from_template(cfg_cls, t: dict):
	ret = cfg_cls()
	for k, v in t.items():
		if not hasattr(ret, k):
			return "key '%s' is incompatible with %s" % (k, cfg_cls.__name__)
		# add value as dict
		if isinstance(v, dict):
			vc = v.copy()
			vc["type"] = RandType[v["type"]]
			setattr(ret, k, RandConfig(**vc))
		# add value as float, or a string that can be interpreted as float
		else:
			try:
				fv = float(v)
			except ValueError:
				raise TypeError("template values must be real, real-like str or"
					" dict, not %s" % type(v).__name__)
			setattr(ret, k, RandConfig(RandType.constant, mean=fv))
	return ret


def randconfig_from_template(t: dict
		) -> typing.Union[StateRandConfig, TraitRandConfig]:
	"""
	create a StateRandConfig or TraitRandConfig from template-like dict;
	returns type depends on whichever has the matching value keys
	"""
	found_err = list()
	# try first
	ret = _randconfig_from_template(StateRandConfig, t)
	if isinstance(ret, StateRandConfig):
		return ret
	found_err.append(ret)
	# try second
	ret = _randconfig_from_template(TraitRandConfig, t)
	if isinstance(ret, TraitRandConfig):
		return ret
	found_err.append(ret)
	# raise error after failed all tries
	raise ValueError("found mixed or invalid keys with both %s and %s:\n%s\n%s"
		% (StateRandConfig.__name__, TraitRandConfig.__name__, *found_err))
	return


def randconfig_from_template_json(fname: str, *, sanitize=True, **kw
		) -> typing.Union[StateRandConfig, TraitRandConfig]:
	"""
	create a StateRandConfig or TraitRandConfig from template json file;
	returns type depends on whichever has the matching value keys
	"""
	t = json.load(open(fname, "r"), **kw)
	if sanitize:
		sanitize_template(t)
	return randconfig_from_template(t)
