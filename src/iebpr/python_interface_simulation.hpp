#ifndef NO_PYTHON_INTERFACE

#ifndef __IEBPR_PYTHON_INTERFACE_SIMULATION_HPP__
#define __IEBPR_PYTHON_INTERFACE_SIMULATION_HPP__

#include <Python.h>
#include <structmember.h>
#include "simulation.hpp"
#include "python_interface_util.hpp"
#include "python_interface_datastruct.hpp"
#include "python_interface_agent_configs.hpp"

namespace iebpr
{
	namespace python_interface
	{
		struct SimulationPyObject
		{
			PyObject_HEAD;
			Simulation cdata;
			static const PyTypeObject *const type;
		};

		int module_bind_simulation(PyObject *module);

	} // namespace python_interface

} // namespace iebpr::python_interface

#endif

#endif