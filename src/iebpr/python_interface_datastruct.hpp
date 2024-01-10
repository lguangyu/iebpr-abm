#ifndef NO_PYTHON_INTERFACE

#ifndef __IEBPR_PYTHON_INTERFACE_DATASTRUCT_HPP__
#define __IEBPR_PYTHON_INTERFACE_DATASTRUCT_HPP__

#include <Python.h>
#include <structmember.h>
#include "env_state.hpp"
#include "randomizer.hpp"
#include "sbr_control.hpp"
#include "python_interface_util.hpp"

namespace iebpr
{
	namespace python_interface
	{

		struct EnvStatePyObject
		{
			PyObject_HEAD;
			EnvState cdata;
			static const PyTypeObject *const type;
		};

		struct SbrPhasePyObject
		{
			PyObject_HEAD;
			SbrControl::Phase cdata;
			static const PyTypeObject *const type;
		};

		struct SbrStagePyObject
		{
			PyObject_HEAD;
			SbrControl::Stage cdata;
			static const PyTypeObject *const type;
		};

		int module_bind_datastructs(PyObject *module);

	} // namespace python_interface

} // namespace iebpr::python_interface

#endif

#endif