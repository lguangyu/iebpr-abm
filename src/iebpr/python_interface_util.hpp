#ifndef NO_PYTHON_INTERFACE

#ifndef __IEBPR_PYTHON_INTERFACE_UTIL_HPP__
#define __IEBPR_PYTHON_INTERFACE_UTIL_HPP__

#include <Python.h>

namespace iebpr
{
	namespace python_interface
	{
		// exception types
		extern PyObject *PyExc_IebprError;
		extern PyObject *PyExc_IebprPrerunValidateError;

		// misc types
		extern PyObject *EnvStateRecDescr;
		extern PyObject *AgentStateRecDescr;

	} // namespace python_interface

} // namespace iebpr

#endif

#endif