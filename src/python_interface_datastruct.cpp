#ifndef NO_PYTHON_INTERFACE

#include "iebpr/python_interface_datastruct.hpp"

namespace iebpr
{
	namespace python_interface
	{
		//======================================================================
		// BINDING OF EnvState
		static PyMethodDef EnvStatePyObjectType_methods[] = {
			{nullptr, nullptr, 0, nullptr},
		};

		static PyMemberDef EnvStatePyObjectType_members[] = {
			{"volume", T_DOUBLE, offsetof(EnvStatePyObject, cdata.volume), 0,
			 "SBR living volume (L) <-> float"},
			{"vfa_conc", T_DOUBLE, offsetof(EnvStatePyObject, cdata.vfa_conc), 0,
			 "bulk-phase vfa concentration (mgCOD/L) <-> float"},
			{"op_conc", T_DOUBLE, offsetof(EnvStatePyObject, cdata.op_conc), 0,
			 "bulk-phase op concentration (mgP/L) <-> float"},
			{"is_aerobic", T_BOOL, offsetof(EnvStatePyObject, cdata.is_aerobic), 0,
			 "aerobic state <-> bool"},
			{nullptr, 0, 0, 0, nullptr},
		};

		static void EnvStatePyObjectType_tp_dealloc(PyObject *self)
		{
			((EnvStatePyObject *)self)->cdata.~EnvState();
			Py_TYPE(self)->tp_free(self);
			return;
		}

		static int EnvStatePyObjectType_tp_init(PyObject *self, PyObject *args, PyObject *kwargs)
		{
			auto _self = (EnvStatePyObject *)self;
			static char *kwlist[] = {
				(char *)"volume",
				(char *)"vfa_conc",
				(char *)"op_conc",
				(char *)"is_aerobic",
				nullptr,
			};
			if (PyTuple_Size(args))
			{
				PyErr_Format(PyExc_TypeError, "%s() expected 0 positional arguments, got %u",
							 Py_TYPE(self)->tp_name, PyTuple_Size(args));
				return -1;
			}
			if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|dddp", kwlist,
											 &(_self->cdata.volume),
											 &(_self->cdata.vfa_conc),
											 &(_self->cdata.op_conc),
											 &(_self->cdata.is_aerobic)))
				return -1;
			return 0;
		}

		static PyObject *EnvStatePyObjectType_tp_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
		{
			auto o = PyType_GenericNew(type, args, kwargs);
			if (o)
				// initialize c++ object
				new (&((EnvStatePyObject *)o)->cdata) EnvState();
			return o;
		}

		static PyTypeObject EnvStatePyObjectType = {
			// head
			PyObject_HEAD_INIT(&PyType_Type)
			// class def
			"_iebpr.EnvState",		  // class name
			sizeof(EnvStatePyObject), // tp_basicsize
			0,						  // tp_itemsize
			// basic methods
			EnvStatePyObjectType_tp_dealloc, // (destructor) tp_dealloc, release member PyObject
			0,								 // tp_print, deprecated in python 3.x
			nullptr,						 // tp_getattr, deprecated
			nullptr,						 // tp_setattr, deprecated
			nullptr,						 // tp_as_async (PyAsyncMethods*)
			nullptr,						 // tp_repr (reprfunc)
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
			"EnvState(**kw)\n--\n"					  // tp_doc (char *), docstring
			"environment state variables\n"
			"keywords:\n"
			"        volume: float = 0\n"
			"      vfa_conc: float = 0\n"
			"       op_conc: float = 0\n"
			"    is_aerobic: bool = False\n"
			"\nsee \'data descriptor\' section below for details\n",
			nullptr,					  // tp_traverse (traverseproc), traverse through members
			nullptr,					  // tp_clear (inquiry), delete members
			nullptr,					  // tp_richcompare (richcmpfunc), rich-comparison
			0,							  // tp_weaklistoffset (Py_ssize_t), weak ref enabler
			nullptr,					  // tp_iter, i.e. self.__iter__()
			nullptr,					  // tp_iternext, i.e. self.__next__()
			EnvStatePyObjectType_methods, // tp_methods (PyMethodDef *), methods def struct
			EnvStatePyObjectType_members, // tp_members (PyMemberDef *), members def struct
			nullptr,					  // tp_getset (PyGetSetDef *), attribute-like access
			nullptr,					  // tp_base (struct _typeobject *), base type
			nullptr,					  // tp_dict, i.e. self.__dict__()
			nullptr,					  // tp_descr_get (descrgetfunc)
			nullptr,					  // tp_descr_set (descrgetfunc)
			0,							  // tp_dictoffset
			EnvStatePyObjectType_tp_init, // tp_init (newfunc), i.e. self.__init___()
			PyType_GenericAlloc,		  // tp_alloc (allocfunc)
			EnvStatePyObjectType_tp_new,  // tp_new (newfunc), i.e. cls.__new__()
			nullptr,					  // tp_free (freefunc)
			nullptr,					  // tp_is_gc (inquiry), gc-related
			nullptr,					  // tp_bases
			nullptr,					  // tp_mro, i.e. self.mro(), method resolution order
			nullptr,					  // tp_cache
			nullptr,					  // tp_subclasses
			nullptr,					  // tp_weaklist
			nullptr,					  // tp_del, i.e. self.__del__()
			0,							  // tp_version_tag, unsigned int
			nullptr,					  // tp_finalize (destructor)
		};

		//======================================================================
		// BINDING OF SbrControl::Phase
		static PyMethodDef SbrPhasePyObjectType_methods[] = {
			{nullptr, nullptr, 0, nullptr},
		};

		static PyMemberDef SbrPhasePyObjectType_members[] = {
			{"time_len", T_DOUBLE, offsetof(SbrPhasePyObject, cdata.time_len), 0,
			 "phase length (day) <-> float"},
			{"inflow_rate", T_DOUBLE, offsetof(SbrPhasePyObject, cdata.inflow_rate), 0,
			 "influent flow rate (L/d) <-> float"},
			{"inflow_vfa_conc", T_DOUBLE, offsetof(SbrPhasePyObject, cdata.inflow_vfa_conc), 0,
			 "vfa concentration in influent (mgCOD/L) <-> float"},
			{"inflow_op_conc", T_DOUBLE, offsetof(SbrPhasePyObject, cdata.inflow_op_conc), 0,
			 "op concentration in influent (mgP/L) <-> float"},
			{"withdraw_rate", T_DOUBLE, offsetof(SbrPhasePyObject, cdata.withdraw_rate), 0,
			 "sludge withdraw flow rate (L/d) <-> float\nsludge withdraw will decrease SBR living volume as well as agents' biomass"},
			{"outflow_rate", T_DOUBLE, offsetof(SbrPhasePyObject, cdata.outflow_rate), 0,
			 "supernatant effluent flow rate (L/d) <-> float\nsupernatant effluent will decrease SBR living volume but not decrease agents' biomass"},
			{"aeration", T_BOOL, offsetof(SbrPhasePyObject, cdata.aeration), 0,
			 "is aerated <-> bool\nwhen aeration is on, agents will perform aerobical actions, and anaerobic actions otherwise"},
			{"volume_reset", T_DOUBLE, offsetof(SbrPhasePyObject, cdata.volume_reset), 0,
			 "(currently not implemented) force reset reactor living volume at the end of this phase, default INF means no reset (L) <-> float\n"
			 "this is used to counter the precision problem with 32-bit floating point numbers"},
			{nullptr, 0, 0, 0, nullptr},
		};

		static void SbrPhasePyObjectType_tp_dealloc(PyObject *self)
		{
			((SbrPhasePyObject *)self)->cdata.~Phase();
			Py_TYPE(self)->tp_free(self);
			return;
		}

		static int SbrPhasePyObjectType_tp_init(PyObject *self, PyObject *args, PyObject *kwargs)
		{
			auto _self = (SbrPhasePyObject *)self;
			static char *kwlist[] = {
				(char *)"time_len",
				(char *)"inflow_rate",
				(char *)"inflow_vfa_conc",
				(char *)"inflow_op_conc",
				(char *)"withdraw_rate",
				(char *)"outflow_rate",
				(char *)"aeration",
				(char *)"volume_reset",
				nullptr,
			};
			if (PyTuple_Size(args))
			{
				PyErr_Format(PyExc_TypeError, "%s() expected 0 positional arguments, got %u",
							 Py_TYPE(self)->tp_name, PyTuple_Size(args));
				return -1;
			}
			if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|ddddddpd", kwlist,
											 &(_self->cdata.time_len),
											 &(_self->cdata.inflow_rate),
											 &(_self->cdata.inflow_vfa_conc),
											 &(_self->cdata.inflow_op_conc),
											 &(_self->cdata.withdraw_rate),
											 &(_self->cdata.outflow_rate),
											 &(_self->cdata.aeration),
											 &(_self->cdata.volume_reset)))
				return -1;
			return 0;
		}

		static PyObject *SbrPhasePyObjectType_tp_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
		{
			auto o = PyType_GenericNew(type, args, kwargs);
			if (o)
				// initialize c++ object
				new (&((SbrPhasePyObject *)o)->cdata) SbrControl::Phase();
			return o;
		}

		static PyTypeObject SbrPhasePyObjectType = {
			// head
			PyObject_HEAD_INIT(&PyType_Type)
			// class def
			"_iebpr.SbrPhase",		  // class name
			sizeof(SbrPhasePyObject), // tp_basicsize
			0,						  // tp_itemsize
			// basic methods
			SbrPhasePyObjectType_tp_dealloc, // (destructor) tp_dealloc, release member PyObject
			0,								 // tp_print, deprecated in python 3.x
			nullptr,						 // tp_getattr, deprecated
			nullptr,						 // tp_setattr, deprecated
			nullptr,						 // tp_as_async (PyAsyncMethods*)
			nullptr,						 // tp_repr (reprfunc)
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
			"SbrPhase(**kw)\n--\n"					  // tp_doc (char *), docstring
			"SBR phase configuration\n"
			"keywords:\n"
			"           time_len: float = 0\n"
			"        inflow_rate: float = 0\n"
			"    inflow_vfa_conc: float = 0\n"
			"     inflow_op_conc: float = 0\n"
			"      withdraw_rate: float = 0\n"
			"       outflow_rate: float = 0\n"
			"           aeration: bool = False\n"
			"       volume_reset: float = 0\n"
			"\nsee \'data descriptor\' section below for details\n",
			nullptr,					  // tp_traverse (traverseproc), traverse through members
			nullptr,					  // tp_clear (inquiry), delete members
			nullptr,					  // tp_richcompare (richcmpfunc), rich-comparison
			0,							  // tp_weaklistoffset (Py_ssize_t), weak ref enabler
			nullptr,					  // tp_iter, i.e. self.__iter__()
			nullptr,					  // tp_iternext, i.e. self.__next__()
			SbrPhasePyObjectType_methods, // tp_methods (PyMethodDef *), methods def struct
			SbrPhasePyObjectType_members, // tp_members (PyMemberDef *), members def struct
			nullptr,					  // tp_getset (PyGetSetDef *), attribute-like access
			nullptr,					  // tp_base (struct _typeobject *), base type
			nullptr,					  // tp_dict, i.e. self.__dict__()
			nullptr,					  // tp_descr_get (descrgetfunc)
			nullptr,					  // tp_descr_set (descrgetfunc)
			0,							  // tp_dictoffset
			SbrPhasePyObjectType_tp_init, // tp_init (newfunc), i.e. self.__init___()
			PyType_GenericAlloc,		  // tp_alloc (allocfunc)
			SbrPhasePyObjectType_tp_new,  // tp_new (newfunc), i.e. cls.__new__()
			nullptr,					  // tp_free (freefunc)
			nullptr,					  // tp_is_gc (inquiry), gc-related
			nullptr,					  // tp_bases
			nullptr,					  // tp_mro, i.e. self.mro(), method resolution order
			nullptr,					  // tp_cache
			nullptr,					  // tp_subclasses
			nullptr,					  // tp_weaklist
			nullptr,					  // tp_del, i.e. self.__del__()
			0,							  // tp_version_tag, unsigned int
			nullptr,					  // tp_finalize (destructor)
		};

		//======================================================================
		// BINDING OF SbrControl::Stage
		static PyObject *_SbrStagePyObjectType_method_append_phase_handler(PyObject *self, PyObject *phase)
		{
			Py_INCREF(phase);
			// check type
			if (!PyObject_IsInstance(phase, (PyObject *)&SbrPhasePyObjectType))
			{
				PyErr_Format(PyExc_TypeError, "must be SbrPhase, not %s",
							 Py_TYPE(phase)->tp_name);
				goto fail_decref;
			}
			// add phase
			((SbrStagePyObject *)self)->cdata.append_phase(((SbrPhasePyObject *)phase)->cdata);
			Py_DECREF(phase);
			Py_RETURN_NONE;
		fail_decref:
			Py_DECREF(phase);
			return nullptr;
		}

		static PyObject *SbrStagePyObjectType_method_append_phase(PyObject *self, PyObject *args)
		{
			return _SbrStagePyObjectType_method_append_phase_handler(self, args);
		}

		static PyObject *SbrStagePyObjectType_method_clear_phase(PyObject *self, PyObject *args)
		{
			((SbrStagePyObject *)self)->cdata.clear_phase();
			Py_RETURN_NONE;
		}

		static PyObject *SbrStagePyObjectType_method_is_flow_balanced(PyObject *self, PyObject *args)
		{
			if (((SbrStagePyObject *)self)->cdata.is_flow_balanced())
				Py_RETURN_TRUE;
			else
				Py_RETURN_FALSE;
		}

		static PyMethodDef SbrStagePyObjectType_methods[] = {
			{"append_phase", SbrStagePyObjectType_method_append_phase, METH_O,
			 "append_phase(self, SbrPhase) -> None\n--\nappend a phase config to the stage"},
			{"clear_phase", SbrStagePyObjectType_method_clear_phase, METH_NOARGS,
			 "clear_phase(self, /) -> None\n--\nclear current phase configs"},
			{"is_flow_balanced", SbrStagePyObjectType_method_is_flow_balanced, METH_NOARGS,
			 "is_flow_balanced(self, /) -> bool\n--\nreturn True if volumes of inflow and outflow + withdraw are balanced"},
			{nullptr, nullptr, 0, nullptr},
		};

		static PyMemberDef SbrStagePyObjectType_members[] = {
			{"n_cycle", T_PYSSIZET, offsetof(SbrStagePyObject, cdata.n_cycle), 0,
			 "number of phase cycles to complete before end of this stage <-> int\n"
			 "a cycle is a run-through of all phases once"},
			{nullptr, 0, 0, 0, nullptr},
		};

		static PyObject *SbrStagePyObjectType_get_cycle_phases(PyObject *self, void *closure)
		{
			const auto &vec = ((SbrStagePyObject *)self)->cdata.cycle_phases;
			auto ret = PyTuple_New(vec.size());
			if (!ret)
				goto tuple_new_fail;
			// fill in values
			for (size_t i = 0; i < vec.size(); i++)
			{
				auto t = SbrPhasePyObjectType.tp_new(&SbrPhasePyObjectType, nullptr, nullptr);
				if (!t)
					goto tuple_build_fail;
				((SbrPhasePyObject *)t)->cdata = vec[i];
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

		static int SbrStagePyObjectType_set_cycle_phases(PyObject *self, PyObject *value, void *closue)
		{
			Py_INCREF(value);
			// allow none, this will clear all settings
			if (Py_IsNone(value))
			{
				((SbrStagePyObject *)self)->cdata.clear_phase();
				goto success_decref;
			}
			// check if is sequence-like
			if (!PySequence_Check(value))
			{
				PyErr_Format(PyExc_TypeError, "must be None or sequence-like, not %s",
							 Py_TYPE(value)->tp_name);
				goto fail_decref;
			}
			// parse the sequence object
			{
				auto r = decltype(((SbrStagePyObject *)self)->cdata.cycle_phases)(0);
				for (auto i = 0; i < PySequence_Size(value); i++)
				{
					PyObject *o = PySequence_GetItem(value, i);
					assert(o); // shouldn't have error here
					if (PyErr_Occurred())
					{
						Py_DECREF(o);
						goto fail_decref;
					}
					if (!PyObject_IsInstance(o, (PyObject *)&SbrPhasePyObjectType))
					{
						PyErr_Format(PyExc_TypeError, "must be SbrPhase, not '%s'",
									 Py_TYPE(o)->tp_name);
						Py_DECREF(o);
						goto fail_decref;
					}
					Py_DECREF(o);
					r.push_back(((SbrPhasePyObject *)o)->cdata);
				}
				((SbrStagePyObject *)self)->cdata.cycle_phases = r;
			}
		success_decref:
			Py_DECREF(value);
			return 0;
		fail_decref:
			Py_DECREF(value);
			return -1;
		}

		static PyObject *SbrStagePyObjectType_get_n_phase(PyObject *self, void *closure)
		{
			return Py_BuildValue("n", ((SbrStagePyObject *)self)->cdata.n_phase());
		}

		static PyObject *SbrStagePyObjectType_get_cycle_time_len(PyObject *self, void *closure)
		{
			return Py_BuildValue("d", ((SbrStagePyObject *)self)->cdata.cycle_time_len());
		}

		static PyObject *SbrStagePyObjectType_get_total_time_len(PyObject *self, void *closure)
		{
			return Py_BuildValue("d", ((SbrStagePyObject *)self)->cdata.total_time_len());
		}

		static PyGetSetDef SbrStagePyObjectType_getsets[] = {
			{"cycle_phases", SbrStagePyObjectType_get_cycle_phases,
			 SbrStagePyObjectType_set_cycle_phases, "SBR phase configs <-> tuple[SbrPhase]", nullptr},
			{"n_phase", SbrStagePyObjectType_get_n_phase,
			 nullptr, "number of added phases in this stage -> int", nullptr},
			{"cycle_time_len", SbrStagePyObjectType_get_cycle_time_len,
			 nullptr, "time length per cycle in this stage -> float", nullptr},
			{"total_time_len", SbrStagePyObjectType_get_total_time_len,
			 nullptr, "total time length of this stage -> float", nullptr},
			{nullptr, nullptr, nullptr, nullptr, nullptr},
		};

		static void SbrStagePyObjectType_tp_dealloc(PyObject *self)
		{
			((SbrStagePyObject *)self)->cdata.~Stage();
			Py_TYPE(self)->tp_free(self);
			return;
		}

		static int SbrStagePyObjectType_tp_init(PyObject *self, PyObject *args, PyObject *kwargs)
		{
			auto _self = (SbrStagePyObject *)self;
			PyObject *cycle_phases = Py_None;
			static char *kwlist[] = {
				(char *)"n_cycle",
				(char *)"cycle_phases",
				nullptr,
			};
			if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|nO", kwlist,
											 &(_self->cdata.n_cycle),
											 &cycle_phases))
				return -1;
			if (SbrStagePyObjectType_set_cycle_phases(self, cycle_phases, nullptr))
				return -1;
			return 0;
		}

		static PyObject *SbrStagePyObjectType_tp_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
		{
			auto o = PyType_GenericNew(type, args, kwargs);
			if (o)
				// initiate c++ object
				new (&((SbrStagePyObject *)o)->cdata) SbrControl::Stage();
			return o;
		}

		static PyTypeObject SbrStagePyObjectType = {
			// head
			PyObject_HEAD_INIT(&PyType_Type)
			// class def
			"_iebpr.SbrStage",		  // class name
			sizeof(SbrStagePyObject), // tp_basicsize
			0,						  // tp_itemsize
			// basic methods
			SbrStagePyObjectType_tp_dealloc, // (destructor) tp_dealloc, release member PyObject
			0,								 // tp_print, deprecated in python 3.x
			nullptr,						 // tp_getattr, deprecated
			nullptr,						 // tp_setattr, deprecated
			nullptr,						 // tp_as_async (PyAsyncMethods*)
			nullptr,						 // tp_repr (reprfunc)
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
			"SbrStage(*ka, **kw)\n--\n"				  // tp_doc (char *), docstring
			"SBR stage configuration\n"
			"arguments:\n"
			"        n_cycle: int = 0\n"
			"    cycle_phase: list[SbrPhase] = None\n"
			"\nsee \'data descriptor\' section below for details\n",
			nullptr,					  // tp_traverse (traverseproc), traverse through members
			nullptr,					  // tp_clear (inquiry), delete members
			nullptr,					  // tp_richcompare (richcmpfunc), rich-comparison
			0,							  // tp_weaklistoffset (Py_ssize_t), weak ref enabler
			nullptr,					  // tp_iter, i.e. self.__iter__()
			nullptr,					  // tp_iternext, i.e. self.__next__()
			SbrStagePyObjectType_methods, // tp_methods (PyMethodDef *), methods def struct
			SbrStagePyObjectType_members, // tp_members (PyMemberDef *), members def struct
			SbrStagePyObjectType_getsets, // tp_getset (PyGetSetDef *), attribute-like access
			nullptr,					  // tp_base (struct _typeobject *), base type
			nullptr,					  // tp_dict, i.e. self.__dict__()
			nullptr,					  // tp_descr_get (descrgetfunc)
			nullptr,					  // tp_descr_set (descrgetfunc)
			0,							  // tp_dictoffset
			SbrStagePyObjectType_tp_init, // tp_init (newfunc), i.e. self.__init___()
			PyType_GenericAlloc,		  // tp_alloc (allocfunc)
			SbrStagePyObjectType_tp_new,  // tp_new (newfunc), i.e. cls.__new__()
			nullptr,					  // tp_free (freefunc)
			nullptr,					  // tp_is_gc (inquiry), gc-related
			nullptr,					  // tp_bases
			nullptr,					  // tp_mro, i.e. self.mro(), method resolution order
			nullptr,					  // tp_cache
			nullptr,					  // tp_subclasses
			nullptr,					  // tp_weaklist
			nullptr,					  // tp_del, i.e. self.__del__()
			0,							  // tp_version_tag, unsigned int
			nullptr,					  // tp_finalize (destructor)
		};

		//======================================================================
		// EXPORT TYPE OBJECT
		const PyTypeObject *const EnvStatePyObject::type = &EnvStatePyObjectType;
		const PyTypeObject *const SbrPhasePyObject::type = &SbrPhasePyObjectType;
		const PyTypeObject *const SbrStagePyObject::type = &SbrStagePyObjectType;

		//======================================================================
		// ADD TO MODULE / TYPE INIT FUNC
		int module_bind_datastructs(PyObject *m)
		{
			if (PyModule_AddType(m, &EnvStatePyObjectType) ||
				PyModule_AddType(m, &SbrPhasePyObjectType) ||
				PyModule_AddType(m, &SbrStagePyObjectType))
				return -1;
			return 0;
		}

	} // namespace python_interface

} // namespace iebpr

#endif