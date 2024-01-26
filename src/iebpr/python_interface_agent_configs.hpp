#ifndef NO_PYTHON_INTERFACE

#ifndef __IEBPR_PYTHON_INTERFACE_AGENT_CONFIGS_HPP__
#define __IEBPR_PYTHON_INTERFACE_AGENT_CONFIGS_HPP__

#include <Python.h>
#include <structmember.h>
#include "agent_subtype_base.hpp"
#include "python_interface_util.hpp"

namespace iebpr
{
	namespace python_interface
	{
		struct RandConfigPyObject
		{
			PyObject_HEAD;
			Randomizer::RandConfig cdata;
			static const PyTypeObject *const type;
		};

		struct StateRandConfigPyObject
		{
			PyObject_HEAD;
			StateRandConfig cdata;
			static const PyTypeObject *const type;
		};

		struct TraitRandConfigPyObject
		{
			PyObject_HEAD;
			TraitRandConfig cdata;
			static const PyTypeObject *const type;
		};

		int module_bind_agent_configs(PyObject *module);

	} // namespace python_interface

} // namespace iebpr::python_interface

#endif

#endif