/*
Utility header which defines some common conversion function between py::array_t<T> <---> Eigen::Matrix
*/

#pragma once

#include "Util/Enum.h"
#include "ImageConversion.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <vector>

#include <Eigen/Dense>
#include <fmt/format.h>


namespace Util
{

	/// Convert a 3x3 numpy array into a Eigen double matrix. Requires the array
	/// to be of shape (3, 3) otherwise the conversion will throw a valueerror.
	inline Eigen::Matrix3d matrix_from_py_array(py::array_t<double> _array)
	{
		auto shape = Impl::shape_from_py_array<double>(_array, { 2 }, 9);
		if (shape != {3, 3})
		{
			throw py::value_error(fmt::format("Expected a 3x3 matrix as the transformation matrix, instead got shape ({})",
				fmt::join(shape, ", ")));
		}
		auto vec = vector_from_py_array<double>(std::move(_array), 3, 3);

		Eigen::Matrix3d mat;
		mat << vec;
		return mat;
	}


	/// Convert an Eigen3d matrix into a 3x3 numpy ndarray. 
	inline py::array_t<double> matrix_from_py_array(Eigen::Matrix3d _mat)
	{
		std::vector<double> vec {
			_mat(0, 0), _mat(0, 1), _mat(0, 2),
			_mat(1, 0), _mat(1, 1), _mat(1, 2),
			_mat(2, 0), _mat(2, 1), _mat(2, 2),
 		}

		auto _array = py_array_from_vector(std::move(vec), { 3, 3 });
		return _array;
	}

}