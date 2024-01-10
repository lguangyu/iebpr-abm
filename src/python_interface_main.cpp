#ifndef NO_PYTHON_INTERFACE

#include <Python.h>
// numpy stuff
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#define PY_ARRAY_UNIQUE_SYMBOL _IEBPR_NPY_API
#include <numpy/ndarrayobject.h>
#include "iebpr/python_interface_util.hpp"
#include "iebpr/python_interface_datastruct.hpp"
#include "iebpr/python_interface_agent_configs.hpp"
#include "iebpr/python_interface_simulation.hpp"

namespace iebpr
{
	namespace python_interface
	{
		PyObject *PyExc_IebprError;
		PyObject *PyExc_IebprPrerunValidateError;

		static PyMethodDef _iebpr_methods[] = {
			{nullptr, nullptr, 0, nullptr},
		};

		//======================================================================
		// MODULE DEF
		//======================================================================

		static PyModuleDef _iebpr_def = {
			PyModuleDef_HEAD_INIT,
			.m_name = "_iebpr",								// module name
			.m_doc = "python binding of iEBPR C++ library", // docstr
			.m_size = -1,
			.m_methods = _iebpr_methods, //_iebpr_methods,
		};

		//======================================================================
		// MODULE ADD OBJECT
		//======================================================================

		static int module_new_exception(PyObject *m, const char *name,
										PyObject *base, PyObject *dict,
										PyObject **export_ptr = nullptr)
		{
			// create new exception type
			auto exc = PyErr_NewException(name, base, dict);
			if (!exc)
				goto exc_new_fail;
			if (export_ptr)
				*export_ptr = exc;
			// add to module
			{
				// find the name string after the last separator '.'
				const char *last_sep = name, *c = name;
				while (*c)
				{
					if (*c == '.')
						last_sep = c;
					c++;
				}
				auto failed = PyModule_AddObjectRef(m, last_sep + 1, exc);
				Py_DECREF(exc);
				if (failed)
					goto exc_add_fail;
			}
			return 0;
		exc_add_fail:
			Py_DECREF(exc);
		exc_new_fail:
			return -1;
		}

		static int module_add_subtype_enum(PyObject *m, Simulation::subtype_enum subtype)
		{
			return PyModule_AddIntConstant(m, Simulation::subtype_enum_to_name(subtype),
										   (enum_base_t)subtype);
		}

		static int module_add_randtype_enum(PyObject *m, Simulation::rand_t type)
		{
			return PyModule_AddIntConstant(m, Simulation::randtype_enum_to_name(type),
										   (enum_base_t)type);
		}

	} // namespace python_interface

} // namespace iebpr

extern "C"
{

	PyMODINIT_FUNC PyInit__iebpr(void)
	{
		// initiate numpy array
		import_array();

		// create module
		PyObject *m = PyModule_Create(&iebpr::python_interface::_iebpr_def);
		if (!m)
			goto module_new_fail;

		// add exception types
		if (iebpr::python_interface::module_new_exception(m, "_iebpr.IebprError",
														  nullptr,
														  nullptr,
														  &iebpr::python_interface::PyExc_IebprError))
			goto module_add_member_fail;

		if (iebpr::python_interface::module_new_exception(m, "_iebpr.IebprPrerunValidateError",
														  iebpr::python_interface::PyExc_IebprError,
														  nullptr,
														  &iebpr::python_interface::PyExc_IebprPrerunValidateError))
			goto module_add_member_fail;

		// add wrapped c++ classes to module
		if (iebpr::python_interface::module_bind_datastructs(m) ||
			iebpr::python_interface::module_bind_agent_configs(m) ||
			iebpr::python_interface::module_bind_simulation(m))
			goto module_add_member_fail;

		// add enum values to module
		if (iebpr::python_interface::module_add_subtype_enum(m, iebpr::Simulation::subtype_enum::pao) ||
			iebpr::python_interface::module_add_subtype_enum(m, iebpr::Simulation::subtype_enum::gao) ||
			iebpr::python_interface::module_add_subtype_enum(m, iebpr::Simulation::subtype_enum::oho))
			goto module_add_member_fail;

		if (iebpr::python_interface::module_add_randtype_enum(m, iebpr::Simulation::rand_t::constant) ||
			iebpr::python_interface::module_add_randtype_enum(m, iebpr::Simulation::rand_t::normal) ||
			iebpr::python_interface::module_add_randtype_enum(m, iebpr::Simulation::rand_t::uniform) ||
			iebpr::python_interface::module_add_randtype_enum(m, iebpr::Simulation::rand_t::bernoulli) ||
			iebpr::python_interface::module_add_randtype_enum(m, iebpr::Simulation::rand_t::obsvalues))
			goto module_add_member_fail;

		return m;
	module_add_member_fail:
		Py_DECREF(m);
	module_new_fail:
		return nullptr;
	}
}

#endif