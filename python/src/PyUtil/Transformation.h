/*
Utility header which defines some common conversion function between py::array_t<T> <---> Eigen::Matrix
*/

#pragma once

#include "Util/Enum.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>
#include <pybind11/stl_bind.h>
#include <pybind11/numpy.h>
#include <pybind11/functional.h>
#include <pybind11/iostream.h>

#include <vector>
#include <string>
#include <algorithm>

#include <Eigen/Dense>


namespace Util
{

	inline Eigen::Matrix3d matrix_from_py_array(py::array_t<double> array)
	{

	}
}