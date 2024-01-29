#ifndef NO_PYTHON_INTERFACE

#include <cstring>
#include <cstdarg>
// numpy stuff
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#define PY_ARRAY_UNIQUE_SYMBOL _IEBPR_NPY_API
#define NO_IMPORT_ARRAY
#include <numpy/ndarrayobject.h>
#include "iebpr/python_interface_simulation.hpp"

namespace iebpr
{
	namespace python_interface
	{
		// dtype of ndarray, in generating record return values
		// will be filled by module_bind_simulation() call
		PyObject *EnvStateRecDescr = nullptr;
		PyObject *AgentStateRecDescr = nullptr;

		// a wrapper of Py_BuildValue and PyArray_DescrConverter
		// create a PyArray_Descr from format tuple
		static int conv_descr(PyObject *const *export_ptr_const, size_t size_check,
							  const char *format, ...)
		{
			// build the format tuple
			std::va_list args;
			va_start(args, format);
			auto o = Py_VaBuildValue(format, args);
			va_end(args);
			if (!o)
				goto fail;
			// check size is aligned
			if (PySequence_Size(o) != (Py_ssize_t)size_check)
			{
				PyErr_Format(PyExc_ValueError, "bad number of dtype fields "
											   "(%li), excepted %lu; this is "
											   "likely an internal error",
							 PySequence_Size(o), size_check);
				goto fail_decref;
			}
			assert(Py_REFCNT(o) == 1);
			if (!PyArray_DescrConverter(o, (PyArray_Descr **)export_ptr_const))
				goto fail_decref;
			// looks like numpy holds a ref here
			// no need to Py_DECREF() the PyArray_Descr object
			assert(Py_REFCNT(*export_ptr_const) == 1);
			Py_DECREF(o);
			return 0;
		fail_decref:
			Py_DECREF(o);
		fail:
			return -1;
		}

		using simutype_enum = decltype(SbrControl::simutype);

		//======================================================================
		// BINDING OF Simulation
		static PyObject *SimulationPyObjectType_method_append_sbr_stage(PyObject *self, PyObject *args)
		{
			Py_INCREF(args);
			if (!PyObject_IsInstance(args, (PyObject *)SbrStagePyObject::type))
			{
				PyErr_Format(PyExc_TypeError, "must be SbrStage, not %s",
							 Py_TYPE(args)->tp_name);
				goto fail_decref;
			}
			((SimulationPyObject *)self)->cdata.append_sbr_stage(((SbrStagePyObject *)args)->cdata);
			Py_DECREF(args);
			Py_RETURN_NONE;
		fail_decref:
			Py_DECREF(args);
			return nullptr;
		}

		static PyObject *SimulationPyObjectType_method_is_flow_balanced(PyObject *self, PyObject *args)
		{
			if (((SimulationPyObject *)self)->cdata.is_flow_balanced())
				Py_RETURN_TRUE;
			else
				Py_RETURN_FALSE;
		}

		static PyObject *SimulationPyObjectType_method_clear_sbr_stage(PyObject *self, PyObject *args)
		{
			((SimulationPyObject *)self)->cdata.clear_sbr_stage();
			Py_RETURN_NONE;
		}

		static PyObject *SimulationPyObjectType_method_add_agent_subtype(PyObject *self, PyObject *args, PyObject *kwargs)
		{
			AgentSubtypeBase::subtype_enum subtype;
			size_t n_agent;
			PyObject *state_cfg = nullptr, *trait_cfg = nullptr;
			static char *kwlist[] = {
				(char *)"subtype",
				(char *)"n_agent",
				(char *)"state_cfg",
				(char *)"trait_cfg",
				nullptr,
			};
			if (PyTuple_Size(args) > 1)
			{
				PyErr_Format(PyExc_TypeError, "add_agent_subtype() takes at most"
											  " 2 positional argument, got %u",
							 PyTuple_Size(args));
				goto fail;
			}
			if (!PyArg_ParseTupleAndKeywords(args, kwargs, "InOO|", kwlist,
											 &subtype, &n_agent, &state_cfg, &trait_cfg))
				goto fail;
			Py_INCREF(state_cfg);
			Py_INCREF(trait_cfg);
			// type check first
			if (!PyObject_IsInstance(state_cfg, (PyObject *)StateRandConfigPyObject::type))
			{
				PyErr_Format(PyExc_TypeError, "state_cfg must be StateRandConfig, not %s",
							 Py_TYPE(state_cfg)->tp_name);
				goto fail_decref;
			}
			if (!PyObject_IsInstance(trait_cfg, (PyObject *)TraitRandConfigPyObject::type))
			{
				PyErr_Format(PyExc_TypeError, "trait_cfg must be TraitRandConfig, not %s",
							 Py_TYPE(trait_cfg)->tp_name);
				goto fail_decref;
			}
			// parse subtype enum from name and check
			if (!Simulation::is_valid_subtype_enum(subtype))
			{
				PyErr_Format(PyExc_ValueError, "invalid subtype: %s (%u)",
							 Simulation::subtype_enum_to_name(subtype), subtype);
				goto fail_decref;
			}
			((SimulationPyObject *)self)->cdata.add_agent_subtype(subtype, n_agent, ((StateRandConfigPyObject *)state_cfg)->cdata, ((TraitRandConfigPyObject *)trait_cfg)->cdata);
			Py_DECREF(state_cfg);
			Py_DECREF(trait_cfg);
			Py_RETURN_NONE;
		fail_decref:
			Py_DECREF(state_cfg);
			Py_DECREF(trait_cfg);
		fail:
			return nullptr;
		}

		static PyObject *SimulationPyObjectType_method_clear_agent_subtype(PyObject *self, PyObject *args)
		{
			((SimulationPyObject *)self)->cdata.clear_agent_subtype();
			Py_RETURN_NONE;
		}

		static PyObject *SimulationPyObjectType_method_get_state_rec_timepoints(PyObject *self, PyObject *args)
		{
			const auto &vec = ((SimulationPyObject *)self)->cdata.get_state_rec_timepoints();
			auto ret = PyTuple_New(vec.size());
			if (!ret)
				goto tuple_new_fail;
			// fill in values
			for (size_t i = 0; i < vec.size(); i++)
			{
				auto t = Py_BuildValue("d", vec[i]);
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

		static PyObject *SimulationPyObjectType_method_set_state_rec_timepoints(PyObject *self, PyObject *args)
		{
			auto vals = std::vector<stvalue_t>(0);
			Py_INCREF(args);
			if (Py_IsNone(args))
			{
				((SimulationPyObject *)self)->cdata.clear_state_rec_timepoints();
				goto success_decref;
			}
			// check if is sequence-like
			if (!PySequence_Check(args))
			{
				PyErr_Format(PyExc_TypeError, "must be None or sequence-like, not %s",
							 Py_TYPE(args)->tp_name);
				goto fail_decref;
			}
			// parse the sequence object
			((SimulationPyObject *)self)->cdata.clear_state_rec_timepoints();
			for (Py_ssize_t i = 0; i < PySequence_Size(args); i++)
			{
				PyObject *o = PySequence_GetItem(args, i);
				assert(o != nullptr); // shouldn't have error here
				vals.push_back(PyFloat_AsDouble(o));
				Py_DECREF(o);
				if (PyErr_Occurred())
					goto fail_decref;
			}
			((SimulationPyObject *)self)->cdata.set_state_rec_timepoints(std::move(vals));
		success_decref:
			Py_DECREF(args);
			Py_RETURN_NONE;
		fail_decref:
			Py_DECREF(args);
			return nullptr;
		}

		static PyObject *SimulationPyObjectType_method_clear_state_rec_timepoints(PyObject *self, PyObject *args)
		{
			((SimulationPyObject *)self)->cdata.clear_state_rec_timepoints();
			Py_RETURN_NONE;
		}

		static PyObject *SimulationPyObjectType_method_get_snapshot_rec_timepoints(PyObject *self, PyObject *args)
		{
			const auto &vec = ((SimulationPyObject *)self)->cdata.get_snapshot_rec_timepoints();
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

		static PyObject *SimulationPyObjectType_method_set_snapshot_rec_timepoints(PyObject *self, PyObject *args)
		{
			auto vals = std::vector<stvalue_t>(0);
			Py_INCREF(args);
			if (Py_IsNone(args))
			{
				((SimulationPyObject *)self)->cdata.clear_snapshot_rec_timepoints();
				goto success_decref;
			}
			// check if is sequence-like
			if (!PySequence_Check(args))
			{
				PyErr_Format(PyExc_TypeError, "must be None or sequence-like, not %s",
							 Py_TYPE(args)->tp_name);
				goto fail_decref;
			}
			// parse the sequence object
			((SimulationPyObject *)self)->cdata.clear_snapshot_rec_timepoints();
			for (Py_ssize_t i = 0; i < PySequence_Size(args); i++)
			{
				PyObject *o = PySequence_GetItem(args, i);
				assert(o != nullptr); // shouldn't have error here
				vals.push_back(PyFloat_AsDouble(o));
				Py_DECREF(o);
				if (PyErr_Occurred())
					goto fail_decref;
			}
			((SimulationPyObject *)self)->cdata.set_snapshot_rec_timepoints(std::move(vals));
		success_decref:
			Py_DECREF(args);
			Py_RETURN_NONE;
		fail_decref:
			Py_DECREF(args);
			return nullptr;
		}

		static PyObject *SimulationPyObjectType_method_clear_snapshot_rec_timepoints(PyObject *self, PyObject *args)
		{

			((SimulationPyObject *)self)->cdata.clear_snapshot_rec_timepoints();
			Py_RETURN_NONE;
		}

		static PyObject *SimulationPyObjectType_method_run(PyObject *self, PyObject *args)
		{
			auto ec = ((SimulationPyObject *)self)->cdata.run();
			// post run error number interpretation
			switch (ec)
			{
			case none:
				goto success;
			case unexpected_rand_type:
				PyErr_Format(PyExc_IebprPrerunValidateError, "(ERROR 0x%x) unexpected random type", ec);
				break;
			case normal_nonneg_lowprob:
				PyErr_Format(PyExc_IebprPrerunValidateError, "(ERROR 0x%x) type=normal, nonneg=True: probablity to draw non-negative value is too low", ec);
				break;
			case uniform_low_high_swap:
				PyErr_Format(PyExc_IebprPrerunValidateError, "(ERROR 0x%x) type=uniform: low > high", ec);
				break;
			case uniform_nonneg_lowprob:
				PyErr_Format(PyExc_IebprPrerunValidateError, "(ERROR 0x%x) type=uniform, nonneg=True: probablity to draw non-negative value is too low", ec);
				break;
			case bernoulli_error_mean:
				PyErr_Format(PyExc_IebprPrerunValidateError, "(ERROR 0x%x) type=bernoulli: mean < 0 or mean > 1", ec);
				break;
			case invalid_timestep:
				PyErr_Format(PyExc_IebprPrerunValidateError, "(ERROR 0x%x) timestep <= 0", ec);
				break;
			case invalid_init_volume:
				PyErr_Format(PyExc_IebprPrerunValidateError, "(ERROR 0x%x) init sbr living volume <= 0", ec);
				break;
			case total_agent_mismatch_subtype_sum:
				PyErr_Format(PyExc_IebprPrerunValidateError, "(ERROR 0x%x) total agent allocated mismatch sum from subtypes\n"
															 "may caused by a bug, data corruption or tampering",
							 ec);
				break;
			case agent_subtype_pool_overlap:
				PyErr_Format(PyExc_IebprPrerunValidateError, "(ERROR 0x%x) agent instance overlap found between subtypes\n"
															 "may caused by a bug, data corruption or tampering",
							 ec);
				break;
			case bool_trait_wrong_rand_type:
				PyErr_Format(PyExc_IebprPrerunValidateError, "(ERROR 0x%x) agent bool trait random type is not bernoulli/none", ec);
				break;
			case rec_time_exceed_simulation:
				PyErr_Format(PyExc_IebprPrerunValidateError, "(ERROR 0x%x) recording time exceeds simulation time range (0-%.3f)",
							 ec, ((SimulationPyObject *)self)->cdata.total_time_len());
				break;
			case rec_step_smaller_than_timestep:
				PyErr_Format(PyExc_IebprPrerunValidateError, "(ERROR 0x%x) recording step smaller than timestep", ec);
				break;
			case sigint:
				PyErr_SetInterrupt();
				PyErr_CheckSignals();
				break;
			default:
				PyErr_Format(PyExc_IebprPrerunValidateError, "(ERROR 0x%x) uncategorized error");
				break;
			}
			// fail:
			return nullptr;
		success:
			Py_RETURN_NONE;
		}

		static PyObject *SimulationPyObjectType_method_get_run_duration(PyObject *self, PyObject *args)
		{
			return Py_BuildValue("K", (long long)(((SimulationPyObject *)self)->cdata.get_run_duration().count()));
		}

		static PyObject *SimulationPyObjectType_method_retrieve_env_state_rec(PyObject *self, PyObject *args)
		{
			const auto &vec = ((SimulationPyObject *)self)->cdata.retrieve_env_state_rec();
			// find matrix dimensions
			const Py_intptr_t dims[1] = {
				(Py_intptr_t)(vec.size())};
			EnvStateRecEntry *d_ptr = nullptr;
			// create numpy object
			PyObject *ret = PyArray_Zeros(1, dims, (PyArray_Descr *)EnvStateRecDescr, 0);
			if (!ret)
				goto fail;
			// fill in values
			d_ptr = (EnvStateRecEntry *)PyArray_DATA((PyArrayObject *)ret);
			for (Py_intptr_t i = 0; i < dims[0]; i++)
			{
				*d_ptr = vec[i];
				d_ptr++;
			}
			assert(Py_REFCNT(ret) == 1);
			return ret;
		fail:
			PyErr_SetString(PyExc_SystemError, "failed to create return value");
			return nullptr;
		}

		static PyObject *SimulationPyObjectType_method_retrieve_agent_state_rec(PyObject *self, PyObject *args)
		{
			const auto &vec = ((SimulationPyObject *)self)->cdata.retrieve_agent_state_rec();
			// find matrix dimensions
			const Py_intptr_t dims[2] = {
				(Py_intptr_t)(vec.size()),
				(Py_intptr_t)(vec.size() ? vec[0].size() : 0)};
			AgentStateRecEntry *d_ptr;
			// create numpy object
			PyObject *ret = PyArray_Zeros(2, dims, (PyArray_Descr *)AgentStateRecDescr, 0);
			if (!ret)
				goto fail;
			// fill in values
			d_ptr = (AgentStateRecEntry *)PyArray_DATA((PyArrayObject *)ret);
			for (Py_intptr_t i = 0; i < dims[0]; i++)
				for (Py_intptr_t j = 0; j < dims[1]; j++)
				{
					*(d_ptr) = vec[i][j];
					d_ptr++;
				}
			assert(Py_REFCNT(ret) == 1);
			return ret;
		fail:
			PyErr_SetString(PyExc_SystemError, "failed to create return value");
			return nullptr;
		}

		static PyObject *SimulationPyObjectType_method_retrieve_snapshot_rec(PyObject *self, PyObject *args)
		{
			const auto &vec = ((SimulationPyObject *)self)->cdata.retrieve_snapshot_rec();
			// return a list of numpy.ndarray
			PyObject *ret = PyTuple_New(vec.size());
			if (!ret)
				goto tuple_new_fail;
			// fill in each subtype as an ndarray
			for (size_t i = 0; i < vec.size(); i++)
			{
				// find matrix dimensions
				assert(!vec.empty()); // vec shouldn't be empty inside this loop
				const Py_intptr_t dims[2] = {
					(Py_intptr_t)(vec[i].size()),
					(Py_intptr_t)(vec[i].size() ? vec[i][0].size() : 0)};
				AgentStateRecEntry *d_ptr = nullptr;
				// create numpy object
				PyObject *t = PyArray_Zeros(2, dims, (PyArray_Descr *)AgentStateRecDescr, 0);
				if (!t)
					goto tuple_build_fail;
				// fill in values
				d_ptr = (AgentStateRecEntry *)PyArray_DATA((PyArrayObject *)t);
				for (Py_intptr_t j = 0; j < dims[0]; j++)
					for (Py_intptr_t k = 0; k < dims[1]; k++)
					{
						*(d_ptr) = vec[i][j][k];
						d_ptr++;
					}
				PyTuple_SetItem(ret, i, t);
				assert(Py_REFCNT(t) == 1);
			}
			assert(Py_REFCNT(ret) == 1);
			return ret;
		tuple_build_fail:
			for (size_t i = 0; i < vec.size(); i++)
				PyTuple_SetItem(ret, i, nullptr);
			assert(Py_REFCNT(ret) == 1);
			Py_DECREF(ret);
		tuple_new_fail:
			PyErr_SetString(PyExc_SystemError, "failed to create return value");
			return nullptr;
		}

		static PyMethodDef SimulationPyObjectType_methods[] = {
			// Randomizer
			// SbrControl
			{"append_sbr_stage", SimulationPyObjectType_method_append_sbr_stage, METH_O,
			 "append_sbr_stage(self, SbrStage, /) -> None\n--\nappend a stage config to simulation SBR control"},
			{"clear_sbr_stage", SimulationPyObjectType_method_clear_sbr_stage, METH_NOARGS,
			 "clear_sbr_stage(self, /) -> None\n--\nclear all stage configs from simulation SBR control"},
			{"is_flow_balanced", SimulationPyObjectType_method_is_flow_balanced, METH_NOARGS,
			 "is_flow_balanced(self, /) -> bool\n--\nreturn true if inflow and outflow (outflow + withdraw) are balanced"},
			// AgentPool
			{"add_agent_subtype", (PyCFunction)SimulationPyObjectType_method_add_agent_subtype, METH_VARARGS | METH_KEYWORDS,
			 "add_agent_subtype(self, subtype: int, *, n_agent: int, state_cfg: StateRandConfig, trait_cfg: TraitRandConfig) -> None"
			 "\n--\nadd agent subtype to simulation with state/trait configs\n"
			 "subtype: int\n"
			 "    AgentSubtype.pao: phosphate accumulating organism (pao) subtype\n"
			 "    AgentSubtype.gao: glycogen accumulating organism (gao) subtype\n"
			 "    AgentSubtype.oho: ordinary heterotrophic organism (oho) subtype\n"
			 "n_agent: int\n"
			 "    numbe of agents in added subtype\n"
			 "state_cfg: StateRandConfig\n"
			 "    randomizer config set to generate agent state variables\n"
			 "trait_cfg: TraitRandConfig\n"
			 "    randomizer config set to generate agent trait variables\n"},
			{"clear_agent_subtype", SimulationPyObjectType_method_clear_agent_subtype, METH_NOARGS,
			 "clear_agent_subtype(self, /) -> None\n--\nclear all agent subtypes from simulation"},
			// Recorder
			{"get_state_rec_timepoints", SimulationPyObjectType_method_get_state_rec_timepoints, METH_NOARGS,
			 "get_state_rec_timepoints(self, /) -> list[float]\n--\nlist timepoints for state record"},
			{"set_state_rec_timepoints", SimulationPyObjectType_method_set_state_rec_timepoints, METH_O,
			 "set_state_rec_timepoints(self, list[float], /) -> None\n--\nset timepoints for state record, not necessarily sorted"},
			{"clear_state_rec_timepoints", SimulationPyObjectType_method_clear_state_rec_timepoints, METH_NOARGS,
			 "clear_state_rec_timepoints(self, /) -> None\n--\nclear timepoints for state record"},
			{"get_snapshot_rec_timepoints", SimulationPyObjectType_method_get_snapshot_rec_timepoints, METH_NOARGS,
			 "get_snapshot_rec_timepoints(self, /) -> list[float]\n--\nlist timespoints for snapshot record"},
			{"set_snapshot_rec_timepoints", SimulationPyObjectType_method_set_snapshot_rec_timepoints, METH_O,
			 "set_snapshot_rec_timepoints(self, list[float], /) -> None\n--\nset timespoints for snapshot record, not necessarily sorted"},
			{"clear_snapshot_rec_timepoints", SimulationPyObjectType_method_clear_snapshot_rec_timepoints, METH_NOARGS,
			 "clear_snapshot_rec_timepoints(self, /) -> None\n--\nclear timepoints for snapshot record"},
			// Simulation
			{"run", SimulationPyObjectType_method_run, METH_NOARGS,
			 "run(self, /) -> None\n--\nrun simulation, raise an exception if error occurred"},
			{"get_run_duration", SimulationPyObjectType_method_get_run_duration, METH_NOARGS,
			 "get_run_duration(self, /) -> int\n--\nshow the duration of last successful run, in microseconds"},
			{"retrieve_env_state_rec", SimulationPyObjectType_method_retrieve_env_state_rec, METH_NOARGS,
			 "retrieve_env_state_rec(self, /) -> numpy.ndarray\n--\nretrieve environemt state recordings from simulation\n"
			 "return a 1-dimensional numpy.ndarray of index: [timepoints]"},
			{"retrieve_agent_state_rec", SimulationPyObjectType_method_retrieve_agent_state_rec, METH_NOARGS,
			 "retrieve_agent_state_rec(self, /) -> numpy.ndarray\n--\nretrieve agent state recordings from simulation\n"
			 "return a 2-dimensional numpy.ndarray of index order: [timepoints, subtype]"},
			{"retrieve_snapshot_rec", SimulationPyObjectType_method_retrieve_snapshot_rec, METH_NOARGS,
			 "retrieve_snapshot_rec(self, /) -> tuple[numpy.ndarray]\n--\nretrieve agent state snapshot recordings from simulation\n"
			 "return a tuple of 2-dimensional numpy.ndarrays in index order: (tuple)[subtype] -> (numpy.ndarray)[timepoints, agent]"},
			{nullptr, nullptr, 0, nullptr},
		};

		static PyMemberDef SimulationPyObjectType_members[] = {
			{nullptr, 0, 0, 0, nullptr},
		};

		static int SimulationPyObjectType_set_seed(PyObject *self, PyObject *value, void *closure)
		{
			decltype(Randomizer::engine)::result_type seed = PyLong_AsUnsignedLongLong(value);
			if (PyErr_Occurred())
				return -1;
			((SimulationPyObject *)self)->cdata.set_seed((decltype(Randomizer::engine)::result_type)seed);
			return 0;
		}

		static PyObject *SimulationPyObjectType_get_pcontinuous(PyObject *self, void *closure)
		{
			if (((SimulationPyObject *)self)->cdata.get_simutype() == simutype_enum::pcontinuous)
				Py_RETURN_TRUE;
			else
				Py_RETURN_FALSE;
		}

		static int SimulationPyObjectType_set_pcontinuous(PyObject *self, PyObject *value, void *closure)
		{
			auto is_pcontinuous = PyObject_IsTrue(value);
			if (PyErr_Occurred())
				return -1;
			((SimulationPyObject *)self)->cdata.set_simutype(is_pcontinuous ? SbrControl::simutype_enum::pcontinuous : SbrControl::simutype_enum::discrete);
			return 0;
		}

		static int SimulationPyObjectType_set_init_env(PyObject *self, PyObject *value, void *closure)
		{
			if (!PyObject_IsInstance(value, (PyObject *)EnvStatePyObject::type))
			{
				PyErr_Format(PyExc_TypeError, "must be EnvState, not %s",
							 Py_TYPE(value)->tp_name);
				return -1;
			}
			((SimulationPyObject *)self)->cdata.set_init_env(((EnvStatePyObject *)value)->cdata);
			return 0;
		}

		static PyObject *SimulationPyObjectType_get_timestep(PyObject *self, void *closure)
		{
			return Py_BuildValue("d", ((SimulationPyObject *)self)->cdata.get_timestep());
		}

		static int SimulationPyObjectType_set_timestep(PyObject *self, PyObject *value, void *closure)
		{
			auto timestep = PyFloat_AsDouble(value);
			if (PyErr_Occurred())
				return -1;
			if (timestep <= 0)
			{
				PyErr_SetString(PyExc_ValueError, "timestep must be positive");
				return -1;
			}
			((SimulationPyObject *)self)->cdata.set_timestep(timestep);
			return 0;
		}

		static PyObject *SimulationPyObjectType_get_total_time_len(PyObject *self, void *closure)
		{
			return Py_BuildValue("d", ((SimulationPyObject *)self)->cdata.total_time_len());
		}

		static PyObject *SimulationPyObjectType_get_n_agent_subtype(PyObject *self, void *closure)
		{
			return Py_BuildValue("n", ((SimulationPyObject *)self)->cdata.n_agent_subtype());
		}

		static PyObject *SimulationPyObjectType_get_total_n_agent(PyObject *self, void *closure)
		{
			return Py_BuildValue("n", ((SimulationPyObject *)self)->cdata.total_n_agent());
		}

		static PyObject *SimulationPyObjectType_get_n_agent_by_subtype(PyObject *self, void *closure)
		{
			const auto vec = ((SimulationPyObject *)self)->cdata.n_agent_by_subtype();
			auto ret = PyTuple_New(vec.size());
			if (!ret)
				goto tuple_new_fail;
			// fill in values
			for (size_t i = 0; i < vec.size(); i++)
			{
				const char *subtype_name = AgentSubtypeBase::subtype_enum_to_name(vec[i].subtype);
				auto t = Py_BuildValue("sn", subtype_name, vec[i].n_agent);
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

		static PyObject *SimulationPyObjectType_get_n_state_rec_timepoints(PyObject *self, void *args)
		{
			return Py_BuildValue("n", ((SimulationPyObject *)self)->cdata.n_state_rec_timepoints());
		}

		static PyObject *SimulationPyObjectType_get_n_snapshot_rec_timepoints(PyObject *self, void *args)
		{
			return Py_BuildValue("n", ((SimulationPyObject *)self)->cdata.n_snapshot_rec_timepoints());
		}

		static PyGetSetDef SimulationPyObjectType_getsets[] = {
			// Randomizer
			{"seed", nullptr, SimulationPyObjectType_set_seed, "random seed <- int", nullptr},
			// SbrControll
			{"pcontinuous", SimulationPyObjectType_get_pcontinuous,
			 SimulationPyObjectType_set_pcontinuous,
			 "use pseudo-continuous simulation (True) or discrete-time (False) <-> bool",
			 nullptr},
			{"init_env", nullptr, SimulationPyObjectType_set_init_env,
			 "initial environment states <- EnvState", nullptr},
			{"timestep", SimulationPyObjectType_get_timestep,
			 SimulationPyObjectType_set_timestep, "simulation timestep, must be positive <-> float\n"
												  "timestep should take balance between slow simulation (when too small) "
												  "and losing precision (when too large)",
			 nullptr},
			{"total_time_len", SimulationPyObjectType_get_total_time_len, nullptr,
			 "total time length of the simulation -> float", nullptr},
			// AgentPool
			{"n_agent_subtype", SimulationPyObjectType_get_n_agent_subtype, nullptr,
			 "total number of agent subtypes added to simulation -> int", nullptr},
			{"total_n_agent", SimulationPyObjectType_get_total_n_agent, nullptr,
			 "total number of agents in all subtypes -> int", nullptr},
			{"n_agent_by_subtype", SimulationPyObjectType_get_n_agent_by_subtype, nullptr,
			 "list number of agents for each added subtype -> tuple[tuple[str, int]]", nullptr},
			// Recorder
			{"n_state_rec_timepoints", SimulationPyObjectType_get_n_state_rec_timepoints, nullptr,
			 "number of timepoints set for state record -> int", nullptr},
			{"n_snapshot_rec_timepoints", SimulationPyObjectType_get_n_snapshot_rec_timepoints, nullptr,
			 "number of timepoints set for snapshot record -> int", nullptr},
			{nullptr, nullptr, nullptr, nullptr, nullptr},
		};

		static void SimulationPyObjectType_tp_dealloc(PyObject *self)
		{
			((SimulationPyObject *)self)->cdata.~Simulation();
			Py_TYPE(self)->tp_free(self);
			return;
		}

		static int SimulationPyObjectType_tp_init(PyObject *self, PyObject *args, PyObject *kwargs)
		{
			PyObject *seed = nullptr,
					 *pcontinuous = nullptr,
					 *timestep = nullptr,
					 *init_env = nullptr;
			static char *kwlist[] = {
				(char *)"seed",
				(char *)"pcontinuous",
				(char *)"timestep",
				(char *)"init_env",
				nullptr,
			};
			if (PyTuple_Size(args))
			{
				PyErr_Format(PyExc_TypeError, "%s() expected 0 positional arguments, got %u",
							 Py_TYPE(self)->tp_name, PyTuple_Size(args));
				return -1;
			}
			if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|OOOO", kwlist,
											 &seed, &pcontinuous, &timestep, &init_env))
				return -1;
			if (seed && SimulationPyObjectType_set_seed(self, seed, nullptr))
				return -1;
			if (pcontinuous && SimulationPyObjectType_set_pcontinuous(self, pcontinuous, nullptr))
				return -1;
			if (timestep && SimulationPyObjectType_set_timestep(self, timestep, nullptr))
				return -1;
			if (init_env && SimulationPyObjectType_set_init_env(self, init_env, nullptr))
				return -1;
			return 0;
		}

		static PyObject *SimulationPyObjectType_tp_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
		{
			auto o = PyType_GenericNew(type, args, kwargs);
			if (o)
				// initialize c++ object
				new (&(((SimulationPyObject *)o)->cdata)) Simulation();
			return o;
		}

		static PyTypeObject SimulationPyObjectType = {
			// head
			PyVarObject_HEAD_INIT(&PyType_Type, 0)
			// class def
			"iebpr._iebpr.Simulation",	// tp_name (char *), class name
			sizeof(SimulationPyObject), // tp_basicsize
			0,							// tp_itemsize
			// basic methods
			SimulationPyObjectType_tp_dealloc,		  // (destructor) tp_dealloc, release member PyObject
			0,										  // tp_vectorcall_offset
			nullptr,								  // tp_getattr, deprecated
			nullptr,								  // tp_setattr, deprecated
			nullptr,								  // tp_as_async (PyAsyncMethods*)
			nullptr,								  // tp_repr (reprfunc)
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
			PyDoc_STR("Simulation(**kw)\n--\n"		  // tp_doc (char *), docstring
					  "iEBPR simulation manager\n"
					  "arguments:\n"
					  "           seed: int = 0\n"
					  "    pcontinuous: bool = False\n"
					  "       timestep: float = 1e-5\n"
					  "       init_env: EnvState = EnvState()\n"
					  "\nsee \'data descriptor\' section below for details\n"),
			nullptr,						// tp_traverse (traverseproc), traverse through members
			nullptr,						// tp_clear (inquiry), delete members
			nullptr,						// tp_richcompare (richcmpfunc), rich-comparison
			0,								// tp_weaklistoffset (Py_ssize_t), weak ref enabler
			nullptr,						// tp_iter, i.e. self.__iter__()
			nullptr,						// tp_iternext, i.e. self.__next__()
			SimulationPyObjectType_methods, // tp_methods (PyMethodDef *), methods def struct
			SimulationPyObjectType_members, // tp_members (PyMemberDef *), members def struct
			SimulationPyObjectType_getsets, // tp_getset (PyGetSetDef *), attribute-like access
			nullptr,						// tp_base (struct _typeobject *), base type
			nullptr,						// tp_dict, i.e. self.__dict__()
			nullptr,						// tp_descr_get (descrgetfunc)
			nullptr,						// tp_descr_set (descrgetfunc)
			0,								// tp_dictoffset
			SimulationPyObjectType_tp_init, // tp_init (newfunc), i.e. self.__init___()
			PyType_GenericAlloc,			// tp_alloc (allocfunc)
			SimulationPyObjectType_tp_new,	// tp_new (newfunc), i.e. cls.__new__()
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
			nullptr,						// tp_vectorcall (vectorcallfunc)
		};

		//======================================================================
		// EXPORT TYPE OBJECT
		const PyTypeObject *const SimulationPyObject::type = &SimulationPyObjectType;

		//======================================================================
		// ADD TO MODULE / TYPE INIT FUNC
		int module_bind_simulation(PyObject *m)
		{
			// add pyarray descriptors
			if (conv_descr(&EnvStateRecDescr, EnvStateRecEntry::arr_size(),
						   "[(s, s), (s, s), (s, s), (s, s)]",
						   "volume", "f8",
						   "vfa_conc", "f8",
						   "op_conc", "f8",
						   "is_aerobic", "i8") ||
				PyModule_AddObjectRef(m, "EnvStateRecDescr", EnvStateRecDescr))
				return -1;

			if (conv_descr(&AgentStateRecDescr, AgentStateRecEntry::arr_size(),
						   "[(s, s), (s, s), (s, s), (s, s), (s, s)]",
						   "biomass", "f8",
						   "rela_count", "f8",
						   "glycogen", "f8",
						   "pha", "f8",
						   "polyp", "f8") ||
				PyModule_AddObjectRef(m, "AgentStateRecDescr", AgentStateRecDescr))
				return -1;

			if (PyModule_AddType(m, &SimulationPyObjectType))
				return -1;
			return 0;
		}

	} // namespace python_interface

} // namespace iebpr

#endif
