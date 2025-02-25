Geometric Operations
----------------------------

.. doxygenfunction:: Geometry::create_normalized_quad
.. doxygenfunction:: Geometry::create_quad

.. doxygenfunction:: Geometry::Operations::move(std::vector<Point2D<T>> &points, Point2D<T> offset)
.. doxygenfunction:: Geometry::Operations::move(std::vector<Vertex<T>> &vertices, Point2D<T> offset)

.. doxygenfunction:: Geometry::Operations::rotate(std::vector<Point2D<T>> &points, double angle, Point2D<T> center)
.. doxygenfunction:: Geometry::Operations::rotate(std::vector<Vertex<T>> &vertices, double angle, Point2D<T> center)

.. doxygenfunction:: Geometry::Operations::scale(std::vector<Point2D<T>> &points, Point2D<T> scalar, Point2D<T> center)
.. doxygenfunction:: Geometry::Operations::scale(std::vector<Point2D<T>> &points, double factor, Point2D<T> center)
.. doxygenfunction:: Geometry::Operations::scale(std::vector<Vertex<T>> &vertices, Point2D<T> scalar, Point2D<T> center)
.. doxygenfunction:: Geometry::Operations::scale(std::vector<Vertex<T>> &vertices, double factor, Point2D<T> center)

.. doxygenfunction:: Geometry::Operations::transform(std::vector<Point2D<T>> &points, const Eigen::Matrix<T, 3, 3> &matrix)
.. doxygenfunction:: Geometry::Operations::transform(std::vector<Vertex<T>> &vertices, const Eigen::Matrix<T, 3, 3> &matrix)

.. doxygenfunction:: Geometry::Operations::create_transformation_matrix(Point2D<T> move, T angle, Point2D<T> scale, Point2D<T> pivot)
.. doxygenfunction:: Geometry::Operations::create_transformation_matrix(T move_x, T move_y, T angle, T scale_x, T scale_y, Point2D<T> pivot)

.. doxygenfunction:: Geometry::Operations::create_homography_matrix