#ifndef NO_PYTHON_INTERFACE

#include "iebpr/python_interface_agent_configs.hpp"

namespace iebpr
{
	namespace python_interface
	{
		//======================================================================
		// BINDING OF RandConfig
		static PyMethodDef RandConfigPyObjectType_methods[] = {
			{nullptr, nullptr, 0, nullptr},
		};

		static PyMemberDef RandConfigPyObjectType_members[] = {
			{"mean", T_DOUBLE, offsetof(RandConfigPyObject, cdata.mean), 0,
			 "mean of distribution <-> float\n"
			 "used by constant, normal, uniform, bernoulli, and obsvalues"},
			{"stddev", T_DOUBLE, offsetof(RandConfigPyObject, cdata.stddev), 0,
			 "standard deviation <-> float\n"
			 "used by normal"},
			{"low", T_DOUBLE, offsetof(RandConfigPyObject, cdata.low), 0,
			 "lower boundry <-> float\n"
			 "used by uniform"},
			{"high", T_DOUBLE, offsetof(RandConfigPyObject, cdata.high), 0,
			 "higher boundry <-> float\n"
			 "used by uniform"},
			{"non_neg", T_BOOL, offsetof(RandConfigPyObject, cdata.non_neg), 0,
			 "if True, discard negative values and regenerate until a non-negative is acquired <-> bool\n"
			 "this option only works on normal and uniform distributions"},
			{nullptr, 0, 0, 0, nullptr},
		};

		static PyObject *RandConfigPyObjectType_get_type(PyObject *self, void *closure)
		{
			auto type = ((RandConfigPyObject *)self)->cdata.type;
			return Py_BuildValue("s", Randomizer::randtype_enum_to_name(type));
		}

		static int RandConfigPyObjectType_set_type(PyObject *self, PyObject *value, void *closure)
		{
			assert(value);
			assert(Py_REFCNT(value));
			auto type = (Randomizer::rand_t)PyLong_AsUnsignedLongLong(value);
			if (PyErr_Occurred())
				return -1;
			((RandConfigPyObject *)self)->cdata.type = type;
			return 0;
		}

		static PyObject *RandConfigPyObjectType_get_value_list(PyObject *self, void *closure)
		{
			const auto &vec = ((RandConfigPyObject *)self)->cdata.value_list;
			auto ret = PyTuple_New(vec.size());
			if (!ret)
				goto tuple_new_fail;
			// fill in values
			for (size_t i = 0; i < vec.size(); i++)
			{
				PyObject *t = Py_BuildValue("d", vec[i]);
				if (!t)
					goto tuple_build_fail;
				PyTuple_SetItem(ret, i, t);
				assert(Py_REFCNT(t) == 1);
			}
			return ret;
		tuple_build_fail:
			for (size_t i = 0; i < vec.size(); i++)
				PyTuple_SetItem(ret, i, nullptr);
			Py_DECREF(ret);
		tuple_new_fail:
			PyErr_SetString(PyExc_SystemError, "failed to create return value");
			return nullptr;
		}

		static int RandConfigPyObjectType_set_value_list(PyObject *self, PyObject *value, void *closure)
		{
			Py_INCREF(value);
			// allow none, this will clear all settings
			if (Py_IsNone(value))
			{
				((RandConfigPyObject *)self)->cdata.value_list.clear();
				goto success_decref;
			}
			// check if is sequence-like
			if (!PySequence_Check(value))
			{
				PyErr_Format(PyExc_TypeError, "value_list must be None or sequence-like, not %s",
							 Py_TYPE(value)->tp_name);
				goto fail_decref;
			}
			// parse the sequence object
			{
				auto r = decltype(((RandConfigPyObject *)self)->cdata.value_list)(0);
				for (auto i = 0; i < PySequence_Size(value); i++)
				{
					PyObject *o = PySequence_GetItem(value, i);
					assert(o != nullptr); // shouldn't have error here
					auto v = PyFloat_AsDouble(o);
					Py_DECREF(o);
					if (PyErr_Occurred())
						goto fail_decref;
					r.push_back(v);
				}
				((RandConfigPyObject *)self)->cdata.set_value_list(r);
			}
		success_decref:
			Py_DECREF(value);
			return 0;
		fail_decref:
			Py_DECREF(value);
			return -1;
		}

		static PyGetSetDef RandConfigPyObjectType_getsets[] = {
			{"type", RandConfigPyObjectType_get_type, RandConfigPyObjectType_set_type,
			 "type of distribution <-> int\n"
			 "accepted values:\n"
			 "     RandType.constant - always assigns mean\n"
			 "       RandType.normal - normal distribution N(mean, stddev)\n"
			 "      RandType.uniform - uniform distribution drawn from [low, high)\n"
			 "    RandType.bernoulli - draw from {0, 1} with P(1) = mean\n"
			 "    RandType.obsvalues - draw based on observed value list\n",
			 nullptr},
			{"value_list", RandConfigPyObjectType_get_value_list,
			 RandConfigPyObjectType_set_value_list,
			 "list of observed values from a population <-> list[float]\n"
			 "used by obsvalues",
			 nullptr},
			{nullptr, nullptr, nullptr, nullptr, nullptr},
		};

		static void RandConfigPyObjectType_tp_dealloc(PyObject *self)
		{
			((RandConfigPyObject *)self)->cdata.~RandConfig();
			Py_TYPE(self)->tp_free(self);
			return;
		}

		static int RandConfigPyObjectType_tp_init(PyObject *self, PyObject *args, PyObject *kwargs)
		{
			auto _self = (RandConfigPyObject *)self;
			PyObject *type = nullptr, *value_list = nullptr;
			static char *kwlist[] = {
				(char *)"type",
				(char *)"mean",
				(char *)"stddev",
				(char *)"low",
				(char *)"high",
				(char *)"value_list",
				(char *)"non_neg",
				nullptr,
			};
			if (PyTuple_Size(args) > 1)
			{
				PyErr_Format(PyExc_TypeError, "%s() expected at most 1 positional arguments, got %u",
							 Py_TYPE(self)->tp_name, PyTuple_Size(args));
				return -1;
			}
			if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|ddddOp", kwlist,
											 &type,
											 &(_self->cdata.mean),
											 &(_self->cdata.stddev),
											 &(_self->cdata.low),
											 &(_self->cdata.high),
											 &value_list,
											 &(_self->cdata.non_neg)))
				return -1;
			if (type && RandConfigPyObjectType_set_type(self, type, nullptr))
				return -1;
			if (value_list && RandConfigPyObjectType_set_value_list(self, value_list, nullptr))
				return -1;
			return 0;
		}

		static PyObject *RandConfigPyObjectType_tp_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
		{
			auto o = PyType_GenericNew(type, args, kwargs);
			if (o)
				// initialize c++ object
				new (&((RandConfigPyObject *)o)->cdata) Randomizer::RandConfig();
			return o;
		}

		static PyTypeObject RandConfigPyObjectType = {
			// head
			PyObject_HEAD_INIT(&PyType_Type)
			// class def
			"_iebpr.RandConfig",		// class name
			sizeof(RandConfigPyObject), // tp_basicsize
			0,							// tp_itemsize
			// basic methods
			RandConfigPyObjectType_tp_dealloc, // (destructor) tp_dealloc, release member PyObject
			0,								   // tp_print, deprecated in python 3.x
			nullptr,						   // tp_getattr, deprecated
			nullptr,						   // tp_setattr, deprecated
			nullptr,						   // tp_as_async (PyAsyncMethods*)
			nullptr,						   // tp_repr (reprfunc)
			// standard magic methods
			nullptr,								  // tp_as_number (PyNumberMethods *)
			nullptr,								  // tp_as_sequence (PySequenceMethods *)
			nullptr,								  // tp_as_mapping (PyMappingMethods *)
			nullptr,								  // tp_hash, i.e. self.__hash__()
			nullptr,								  // tp_call, i.e. self.__call__()
			nullptr,								  // tp_str (reprfunc), i.e. self.__str__()
			PyObject_GenericGetAttr,				  // tp_getattro (getattrofunc), i.e. self.__getattr__()
			PyObject_GenericSetAttr,				  // tp_setattro (setattrofunc), i.e. self.__setattr__()
			nullptr,								  // tp_as_buffer (PyBufferProcs *)
			Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, // tp_flags, unsigned long
			"RandConfig(type: int, **kw)\n--\n"		  // tp_doc (char *), docstring
			"config to generate randomized state/trait values\n"
			"keywords:\n"
			"      mean: float = 0\n"
			"    stddev: float = 0\n"
			"       low: float = 0\n"
			"      high: float = 0\n"
			"value_list: list[float] = None\n"
			"   non_neg: bool = True\n"
			"needed fields are based on distribution type\n"
			"\nsee \'data descriptor\' section below for details\n",
			nullptr,						// tp_traverse (traverseproc), traverse through members
			nullptr,						// tp_clear (inquiry), delete members
			nullptr,						// tp_richcompare (richcmpfunc), rich-comparison
			0,								// tp_weaklistoffset (Py_ssize_t), weak ref enabler
			nullptr,						// tp_iter, i.e. self.__iter__()
			nullptr,						// tp_iternext, i.e. self.__next__()
			RandConfigPyObjectType_methods, // tp_methods (PyMethodDef *), methods def struct
			RandConfigPyObjectType_members, // tp_members (PyMemberDef *), members def struct
			RandConfigPyObjectType_getsets, // tp_getset (PyGetSetDef *), attribute-like access
			nullptr,						// tp_base (struct _typeobject *), base type
			nullptr,						// tp_dict, i.e. self.__dict__()
			nullptr,						// tp_descr_get (descrgetfunc)
			nullptr,						// tp_descr_set (descrgetfunc)
			0,								// tp_dictoffset
			RandConfigPyObjectType_tp_init, // tp_init (newfunc), i.e. self.__init___()
			PyType_GenericAlloc,			// tp_alloc (allocfunc)
			RandConfigPyObjectType_tp_new,	// tp_new (newfunc), i.e. cls.__new__()
			nullptr,						// tp_free (freefunc)
			nullptr,						// tp_is_gc (inquiry), gc-related
			nullptr,						// tp_bases
			nullptr,						// tp_mro, i.e. self.mro(), method resolution order
			nullptr,						// tp_cache
			nullptr,						// tp_subclasses
			nullptr,						// tp_weaklist
			nullptr,						// tp_del, i.e. self.__del__()
			0,								// tp_version_tag, unsigned int
			nullptr,						// tp_finalize (destructor)
		};

		//======================================================================
		// BINDING OF StateRandConfig
		static PyMethodDef StateRandConfigPyObjectType_methods[] = {
			{nullptr, nullptr, 0, nullptr},
		};

		static PyMemberDef StateRandConfigPyObjectType_members[] = {
			{nullptr, 0, 0, 0, nullptr},
		};

#define _statecfg_getset(attr, docstr)                                                                                           \
	{                                                                                                                            \
		#attr,                                                                                                                   \
			[](PyObject *self, void *closure) /* getter */                                                                       \
		{                                                                                                                        \
			auto ret = RandConfigPyObjectType.tp_new(&RandConfigPyObjectType, nullptr, nullptr);                                 \
			if (!ret)                                                                                                            \
				return (PyObject *)nullptr;                                                                                      \
			((RandConfigPyObject *)ret)->cdata = ((StateRandConfigPyObject *)self)->cdata.s.attr;                                \
			assert(offsetof(AgentSubtypeBase::StateRandConfig, s.attr) / sizeof(Randomizer::RandConfig) < AgentState::arr_size); \
			return ret;                                                                                                          \
		},                                                                                                                       \
			[](PyObject *self, PyObject *value, void *closure) /* setter */                                                      \
		{                                                                                                                        \
			if (!PyObject_IsInstance(value, (PyObject *)&RandConfigPyObjectType))                                                \
				return -1;                                                                                                       \
			((StateRandConfigPyObject *)self)->cdata.s.attr = ((RandConfigPyObject *)value)->cdata;                              \
			assert(offsetof(AgentSubtypeBase::StateRandConfig, s.attr) / sizeof(Randomizer::RandConfig) < AgentState::arr_size); \
			return 0;                                                                                                            \
		},                                                                                                                       \
			PyDoc_STR(docstr),                                                                                                   \
			nullptr,                                                                                                             \
	}

		static PyGetSetDef StateRandConfigPyObjectType_getsets[] = {
			_statecfg_getset(biomass, "randomizer config for biomass <-> RandConfig\n"
									  "    overll biomass conc. of agent subtype (mgCOD/L)"),
			_statecfg_getset(split_biomass, "randomizer config for split_biomass <-> RandConfig\n"
											"    biomass threshold to trigger agent split (mgCOD/L)"),
			_statecfg_getset(glycogen, "randomizer config for glycogen <-> RandConfig\n"
									   "    overll glycogen conc. of agent subtype (mgCOD/L)"),
			_statecfg_getset(pha, "randomizer config for pha <-> RandConfig\n"
								  "    overll pha conc. of agent subtype (mgCOD/L)"),
			_statecfg_getset(polyp, "randomizer config for polyp <-> RandConfig\n"
									"    overll polyp conc. of agent subtype (mgP/L)"),
			{nullptr, nullptr, nullptr, nullptr, nullptr},
		};

#undef _statecfg_getset

		static void StateRandConfigPyObjectType_tp_dealloc(PyObject *self)
		{
			((StateRandConfigPyObject *)self)->cdata.~StateRandConfig();
			Py_TYPE(self)->tp_free(self);
			return;
		}

		static int StateRandConfigPyObjectType_tp_init(PyObject *self, PyObject *args, PyObject *kwargs)
		{
			// check args
			if (PyTuple_Size(args))
			{
				PyErr_Format(PyExc_TypeError, "%s() expected 0 positional arguments, got %u",
							 Py_TYPE(self)->tp_name, PyTuple_Size(args));
				return -1;
			}
			if (!kwargs)
				return 0;
			// iterate over kwargs
			assert(Py_REFCNT(kwargs));
			auto keys = PyDict_Keys(kwargs);
			assert(Py_REFCNT(keys) == 1);
			for (auto i = 0; i < PyList_Size(keys); i++)
			{
				auto key = PyList_GetItem(keys, i);
				// keywords are enforced to be strings
				auto value = PyDict_GetItem(kwargs, key); // borrowed ref, value
				if (PyObject_SetAttr(self, key, value))
					goto add_cfg_fail;
			}
			Py_DECREF(keys);
			return 0;
		add_cfg_fail:
			Py_DECREF(keys);
			return -1;
		}

		static PyObject *StateRandConfigPyObjectType_tp_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
		{
			auto o = PyType_GenericNew(type, args, kwargs);
			if (o)
				// initialize c++ object
				new (&((StateRandConfigPyObject *)o)->cdata) AgentSubtypeBase::StateRandConfig();
			return o;
		}

		static PyTypeObject StateRandConfigPyObjectType = {
			// head
			PyObject_HEAD_INIT(&PyType_Type)
			// class def
			"_iebpr.StateRandConfig",		 // class name
			sizeof(StateRandConfigPyObject), // tp_basicsize
			0,								 // tp_itemsize
			// basic methods
			StateRandConfigPyObjectType_tp_dealloc, // (destructor) tp_dealloc, release member PyObject
			0,										// tp_print, deprecated in python 3.x
			nullptr,								// tp_getattr, deprecated
			nullptr,								// tp_setattr, deprecated
			nullptr,								// tp_as_async (PyAsyncMethods*)
			nullptr,								// tp_repr (reprfunc)
			// standard magic methods
			nullptr,								  // tp_as_number (PyNumberMethods *)
			nullptr,								  // tp_as_sequence (PySequenceMethods *)
			nullptr,								  // tp_as_mapping (PyMappingMethods *)
			nullptr,								  // tp_hash, i.e. self.__hash__()
			nullptr,								  // tp_call, i.e. self.__call__()
			nullptr,								  // tp_str (reprfunc), i.e. self.__str__()
			PyObject_GenericGetAttr,				  // tp_getattro (getattrofunc), i.e. self.__getattr__()
			PyObject_GenericSetAttr,				  // tp_setattro (setattrofunc), i.e. self.__setattr__()
			nullptr,								  // tp_as_buffer (PyBufferProcs *)
			Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, // tp_flags, unsigned long
			"StateRandConfig(**kw)\n--\n"			  // tp_doc (char *), docstring
			"randomizer config for agent states\n"
			"keywords:\n"
			"          biomass: RandConfig\n"
			"    split_biomass: RandConfig\n"
			"         glycogen: RandConfig\n"
			"              pha: RandConfig\n"
			"            polyp: RandConfig\n"
			"\nsee \'data descriptor\' section below for details",
			nullptr,							 // tp_traverse (traverseproc), traverse through members
			nullptr,							 // tp_clear (inquiry), delete members
			nullptr,							 // tp_richcompare (richcmpfunc), rich-comparison
			0,									 // tp_weaklistoffset (Py_ssize_t), weak ref enabler
			nullptr,							 // tp_iter, i.e. self.__iter__()
			nullptr,							 // tp_iternext, i.e. self.__next__()
			StateRandConfigPyObjectType_methods, // tp_methods (PyMethodDef *), methods def struct
			StateRandConfigPyObjectType_members, // tp_members (PyMemberDef *), members def struct
			StateRandConfigPyObjectType_getsets, // tp_getset (PyGetSetDef *), attribute-like access
			nullptr,							 // tp_base (struct _typeobject *), base type
			nullptr,							 // tp_dict, i.e. self.__dict__()
			nullptr,							 // tp_descr_get (descrgetfunc)
			nullptr,							 // tp_descr_set (descrgetfunc)
			0,									 // tp_dictoffset
			StateRandConfigPyObjectType_tp_init, // tp_init (newfunc), i.e. self.__init___()
			PyType_GenericAlloc,				 // tp_alloc (allocfunc)
			StateRandConfigPyObjectType_tp_new,	 // tp_new (newfunc), i.e. cls.__new__()
			nullptr,							 // tp_free (freefunc)
			nullptr,							 // tp_is_gc (inquiry), gc-related
			nullptr,							 // tp_bases
			nullptr,							 // tp_mro, i.e. self.mro(), method resolution order
			nullptr,							 // tp_cache
			nullptr,							 // tp_subclasses
			nullptr,							 // tp_weaklist
			nullptr,							 // tp_del, i.e. self.__del__()
			0,									 // tp_version_tag, unsigned int
			nullptr,							 // tp_finalize (destructor)
		};

		//======================================================================
		// BINDING OF TraitRandConfig
		static PyMethodDef TraitRandConfigPyObjectType_methods[] = {
			{nullptr, nullptr, 0, nullptr},
		};

		static PyMemberDef TraitRandConfigPyObjectType_members[] = {
			{nullptr, 0, 0, 0, nullptr},
		};

#define _traitcfg_getset(attr, docstr)                                                                                           \
	{                                                                                                                            \
		#attr,                                                                                                                   \
			[](PyObject *self, void *closure) /* getter */                                                                       \
		{                                                                                                                        \
			auto ret = RandConfigPyObjectType.tp_new(&RandConfigPyObjectType, nullptr, nullptr);                                 \
			if (!ret)                                                                                                            \
				return (PyObject *)nullptr;                                                                                      \
			((RandConfigPyObject *)ret)->cdata = ((TraitRandConfigPyObject *)self)->cdata.s.attr;                                \
			assert(offsetof(AgentSubtypeBase::TraitRandConfig, s.attr) / sizeof(Randomizer::RandConfig) < AgentTrait::arr_size); \
			return ret;                                                                                                          \
		},                                                                                                                       \
			[](PyObject *self, PyObject *value, void *closure) /* setter */                                                      \
		{                                                                                                                        \
			if (!PyObject_IsInstance(value, (PyObject *)&RandConfigPyObjectType))                                                \
				return -1;                                                                                                       \
			((TraitRandConfigPyObject *)self)->cdata.s.attr = ((RandConfigPyObject *)value)->cdata;                              \
			assert(offsetof(AgentSubtypeBase::TraitRandConfig, s.attr) / sizeof(Randomizer::RandConfig) < AgentTrait::arr_size); \
			return 0;                                                                                                            \
		},                                                                                                                       \
			PyDoc_STR(docstr),                                                                                                   \
			nullptr,                                                                                                             \
	}

		static PyGetSetDef TraitRandCOnfigPyObjectType_getsets[] = {
			_traitcfg_getset(mu, "randomizer config for trait 'mu' <-> RandConfig\n"
								 "    specific growth rate (day^-1)"),
			_traitcfg_getset(q_glycogen, "randomizer config for trait 'q_glycogen' <-> RandConfig\n"
										 "    specific glycogen synthesis rate (mgCOD/mgCOD biomass/day)"),
			_traitcfg_getset(q_pha, "randomizer config for trait 'q_pha' <-> RandConfig\n"
									"    specific pha synthesis rate (mgCOD/mgCOD biomass/day)"),
			_traitcfg_getset(q_polyp, "randomizer config for trait 'q_polyp' <-> RandConfig\n"
									  "    specific polyp synthesis rate (mgP/mgCOD biomass/day)"),
			_traitcfg_getset(m_aerobic, "randomizer config for trait 'm_aerobic' <-> RandConfig\n"
										"    aerobic maintenance rate (mmol-ATP/mmol-C biomass/day)"),
			_traitcfg_getset(m_anaerobic, "randomizer config for trait 'm_anaerobic' <-> RandConfig\n"
										  "    anaerobic maintenance rate (mmol-ATP/mmol-C biomass/day)"),
			_traitcfg_getset(b_aerobic, "randomizer config for trait 'b_aerobic' <-> RandConfig\n"
										"    aerobic decay rate (day^-1)"),
			_traitcfg_getset(b_anaerobic, "randomizer config for trait 'b_anaerobic' <-> RandConfig\n"
										  "    anaerobic decay rate (day^-1)"),
			_traitcfg_getset(b_glycogen, "randomizer config for trait 'b_glycogen' <-> RandConfig\n"
										 "    glycogen decay rate (day^-1)"),
			_traitcfg_getset(b_pha, "randomizer config for trait 'b_pha' <-> RandConfig\n"
									"    pha decay rate (day^-1)"),
			_traitcfg_getset(b_polyp, "randomizer config for trait 'b_polyp' <-> RandConfig\n"
									  "    polyp decay rate (day^-1)"),
			_traitcfg_getset(x_glycogen_min, "randomizer config for trait 'x_glycogen_min' <-> RandConfig\n"
											 "    normalized glycogen min quota (mgCOD/mgCOD biomass)"),
			_traitcfg_getset(x_glycogen_max, "randomizer config for trait 'x_glycogen_max' <-> RandConfig\n"
											 "    normalized glycogen max quota (mgCOD/mgCOD biomass)"),
			_traitcfg_getset(x_pha_min, "randomizer config for trait 'x_pha_min' <-> RandConfig\n"
										"    normalized pha min quota (mgCOD/mgCOD biomass)"),
			_traitcfg_getset(x_pha_max, "randomizer config for trait 'x_pha_max' <-> RandConfig\n"
										"    normalized pha max quota (mgCOD/mgCOD biomass)"),
			_traitcfg_getset(x_polyp_min, "randomizer config for trait 'x_polyp_min' <-> RandConfig\n"
										  "    normalized polyp min quota (mgP/mgCOD biomass)"),
			_traitcfg_getset(x_polyp_max, "randomizer config for trait 'x_polyp_max' <-> RandConfig\n"
										  "    normalized polyp max quota (mgP/mgCOD biomass)"),
			_traitcfg_getset(k_hac, "randomizer config for trait 'k_hac' <-> RandConfig\n"
									"    bulk-phase substrate (acetate) utilize (mgCOD/L)"),
			_traitcfg_getset(k_op, "randomizer config for trait 'k_op' <-> RandConfig\n"
								   "    bulk-phase op utilize for biomass growth (mgP/L)"),
			_traitcfg_getset(k_op_polyp, "randomizer config for trait 'k_op_polyp' <-> RandConfig\n"
										 "    bulk-phase op utilize for storage as polyp (mgP/L)"),
			_traitcfg_getset(k_glycogen, "randomizer config for trait 'k_glycogen' <-> RandConfig\n"
										 "    intracellular glycogen utilize (mgCOD/mgCOD biomass)"),
			_traitcfg_getset(k_pha, "randomizer config for trait 'k_pha' <-> RandConfig\n"
									"    intracellular pha utilize (mgCOD/mgCOD biomass)"),
			_traitcfg_getset(k_polyp, "randomizer config for trait 'k_polyp' <-> RandConfig\n"
									  "    intracellular polyp utilize (mgP/mgCOD biomass)"),
			_traitcfg_getset(ki_glycogen, "randomizer config for trait 'ki_glycogen' <-> RandConfig\n"
										  "    glycogen synthesis (mgCOD/mgCOD biomass)"),
			_traitcfg_getset(ki_pha, "randomizer config for trait 'ki_pha' <-> RandConfig\n"
									 "    pha synthesis (mgCOD/mgCOD biomass)"),
			_traitcfg_getset(ki_polyp, "randomizer config for trait 'ki_polyp' <-> RandConfig\n"
									   "    polyp synthesis (mgP/mgCOD biomass)"),
			_traitcfg_getset(y_h, "randomizer config for trait 'y_h' <-> RandConfig\n"
								  "    heterotrophic growth from substrate (acetate) (mgCOD biomass/mgCOD)"),
			_traitcfg_getset(y_glycogen_pha, "randomizer config for trait 'y_glycogen_pha' <-> RandConfig\n"
											 "    glycogen synthesis from pha (mgCOD glycogen/mgCOD pha)"),
			_traitcfg_getset(y_polyp_pha, "randomizer config for trait 'y_polyp_pha' <-> RandConfig\n"
										  "    polyp synthesis from pha (mgP polyp/mgCOD pha)"),
			_traitcfg_getset(y_pha_hac, "randomizer config for trait 'y_pha_hac' <-> RandConfig\n"
										"    pha synthesis from substrate (acetate) (mgCOD pha/mgCOD hac)"),
			_traitcfg_getset(y_prel, "randomizer config for trait 'y_prel' <-> RandConfig\n"
									 "    op release per vfa (acetate) update (mgP/mgCOD)"),
			_traitcfg_getset(i_bmp, "randomizer config for trait 'i_bmp' <-> RandConfig\n"
									"    biomass p fraction (mgP/mgCOD biomass)"),
			_traitcfg_getset(enable_tca, "randomizer config for trait 'enable_tca' <-> RandConfig\n"
										 "    can enable tca in pha synthses (used by pao subtype only) (bool)"),
			_traitcfg_getset(maint_polyp_first, "randomizer config for trait 'maint_polyp_first' <-> RandConfig\n"
												"    pao uses polyp first then glycogen in anaerobic maintenance, or otherwise (bool)"),
			{nullptr, nullptr, nullptr, nullptr, nullptr},
		};

#undef _traitcfg_getset

		static void TraitRandConfigPyObjectType_tp_dealloc(PyObject *self)
		{
			((TraitRandConfigPyObject *)self)->cdata.~TraitRandConfig();
			Py_TYPE(self)->tp_free(self);
			return;
		}

		static int TraitRandConfigPyObjectType_tp_init(PyObject *self, PyObject *args, PyObject *kwargs)
		{
			// check args
			if (PyTuple_Size(args))
			{
				PyErr_Format(PyExc_TypeError, "%s() expected 0 positional arguments, got %u",
							 Py_TYPE(self)->tp_name, PyTuple_Size(args));
				return -1;
			}
			if (!kwargs)
				return 0;
			// iterate over kwargs
			auto keys = PyDict_Keys(kwargs); // new ref, keys
			for (auto i = 0; i < PyList_Size(keys); i++)
			{
				auto key = PyList_GetItem(keys, i);
				// keywords are enforced to be strings
				auto value = PyDict_GetItem(kwargs, key); // borrowed ref, value
				Py_DECREF(key);
				if (PyObject_SetAttr(self, key, value))
					goto add_cfg_fail;
			}
			Py_DECREF(keys);
			return 0;
		add_cfg_fail:
			Py_DECREF(keys);
			return -1;
		}

		static PyObject *TraitRandConfigPyObjectType_tp_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
		{
			auto o = PyType_GenericNew(type, args, kwargs);
			if (o)
				// initialize c++ object
				new (&((TraitRandConfigPyObject *)o)->cdata) AgentSubtypeBase::TraitRandConfig();
			return o;
		}

		static PyTypeObject TraitRandConfigPyObjectType = {
			// head
			PyObject_HEAD_INIT(&PyType_Type)
			// class def
			"_iebpr.TraitRandConfig",		 // class name
			sizeof(TraitRandConfigPyObject), // tp_basicsize
			0,								 // tp_itemsize
			// basic methods
			TraitRandConfigPyObjectType_tp_dealloc, // (destructor) tp_dealloc, release member PyObject
			0,										// tp_print, deprecated in python 3.x
			nullptr,								// tp_getattr, deprecated
			nullptr,								// tp_setattr, deprecated
			nullptr,								// tp_as_async (PyAsyncMethods*)
			nullptr,								// tp_repr (reprfunc)
			// standard magic methods
			nullptr,								  // tp_as_number (PyNumberMethods *)
			nullptr,								  // tp_as_sequence (PySequenceMethods *)
			nullptr,								  // tp_as_mapping (PyMappingMethods *)
			nullptr,								  // tp_hash, i.e. self.__hash__()
			nullptr,								  // tp_call, i.e. self.__call__()
			nullptr,								  // tp_str (reprfunc), i.e. self.__str__()
			PyObject_GenericGetAttr,				  // tp_getattro (getattrofunc), i.e. self.__getattr__()
			PyObject_GenericSetAttr,				  // tp_setattro (setattrofunc), i.e. self.__setattr__()
			nullptr,								  // tp_as_buffer (PyBufferProcs *)
			Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, // tp_flags, unsigned long
			"TraitRandConfig(**kw)\n--\n"			  // tp_doc (char *), docstring
			"randomizer config for agent trait\n"
			"keywords:"
			"\n[synthesis rates]\n"
			"               mu: RandConfig\n"
			"       q_glycogen: RandConfig\n"
			"            q_pha: RandConfig\n"
			"          q_polyp: RandConfig\n"
			"\n[maintenance rates]\n"
			"        m_aerobic: RandConfig\n"
			"      m_anaerobic: RandConfig\n"
			"\n[decay rates]\n"
			"        b_aerobic: RandConfig\n"
			"      b_anaerobic: RandConfig\n"
			"       b_glycogen: RandConfig\n"
			"            b_pha: RandConfig\n"
			"          b_polyp: RandConfig\n"
			"\n[min/max quotas]\n"
			"   x_glycogen_min: RandConfig\n"
			"   x_glycogen_max: RandConfig\n"
			"        x_pha_min: RandConfig\n"
			"        x_pha_max: RandConfig\n"
			"      x_polyp_min: RandConfig\n"
			"      x_polyp_max: RandConfig\n"
			"\n[half-saturation constants]\n"
			"            k_hac: RandConfig\n"
			"             k_op: RandConfig\n"
			"       k_op_polyp: RandConfig\n"
			"       k_glycogen: RandConfig\n"
			"            k_pha: RandConfig\n"
			"          k_polyp: RandConfig\n"
			"\n[inhibitory half-saturation constants]\n"
			"      ki_glycogen: RandConfig\n"
			"           ki_pha: RandConfig\n"
			"         ki_polyp: RandConfig\n"
			"\n[yield ratios]\n"
			"              y_h: RandConfig\n"
			"   y_glycogen_pha: RandConfig\n"
			"      y_polyp_pha: RandConfig\n"
			"        y_pha_hac: RandConfig\n"
			"           y_prel: RandConfig\n"
			"            i_bmp: RandConfig\n"
			"\n[misc]\n"
			"       enable_tca: RandConfig\n"
			"maint_polyp_first: RandConfig\n"
			"\nsee \'data descriptor\' section below for details",
			nullptr,							 // tp_traverse (traverseproc), traverse through members
			nullptr,							 // tp_clear (inquiry), delete members
			nullptr,							 // tp_richcompare (richcmpfunc), rich-comparison
			0,									 // tp_weaklistoffset (Py_ssize_t), weak ref enabler
			nullptr,							 // tp_iter, i.e. self.__iter__()
			nullptr,							 // tp_iternext, i.e. self.__next__()
			TraitRandConfigPyObjectType_methods, // tp_methods (PyMethodDef *), methods def struct
			TraitRandConfigPyObjectType_members, // tp_members (PyMemberDef *), members def struct
			TraitRandCOnfigPyObjectType_getsets, // tp_getset (PyGetSetDef *), attribute-like access
			nullptr,							 // tp_base (struct _typeobject *), base type
			nullptr,							 // tp_dict, i.e. self.__dict__()
			nullptr,							 // tp_descr_get (descrgetfunc)
			nullptr,							 // tp_descr_set (descrgetfunc)
			0,									 // tp_dictoffset
			TraitRandConfigPyObjectType_tp_init, // tp_init (newfunc), i.e. self.__init___()
			PyType_GenericAlloc,				 // tp_alloc (allocfunc)
			TraitRandConfigPyObjectType_tp_new,	 // tp_new (newfunc), i.e. cls.__new__()
			nullptr,							 // tp_free (freefunc)
			nullptr,							 // tp_is_gc (inquiry), gc-related
			nullptr,							 // tp_bases
			nullptr,							 // tp_mro, i.e. self.mro(), method resolution order
			nullptr,							 // tp_cache
			nullptr,							 // tp_subclasses
			nullptr,							 // tp_weaklist
			nullptr,							 // tp_del, i.e. self.__del__()
			0,									 // tp_version_tag, unsigned int
			nullptr,							 // tp_finalize (destructor)
		};

		//======================================================================
		// EXPORT TYPE OBJECT
		const PyTypeObject *const RandConfigPyObject::type = &RandConfigPyObjectType;
		const PyTypeObject *const StateRandConfigPyObject::type = &StateRandConfigPyObjectType;
		const PyTypeObject *const TraitRandConfigPyObject::type = &TraitRandConfigPyObjectType;

		//======================================================================
		// ADD TO MODULE / TYPE INIT FUNC
		int module_bind_agent_configs(PyObject *m)
		{
			if (PyModule_AddType(m, &RandConfigPyObjectType) ||
				PyModule_AddType(m, &StateRandConfigPyObjectType) ||
				PyModule_AddType(m, &TraitRandConfigPyObjectType))
				return -1;
			return 0;
		}

	} // namespace python_interface

} // namespace iebpr

#endif