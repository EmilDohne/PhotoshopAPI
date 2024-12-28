#pragma once

#include "Macros.h"

#include <vector>
#include <memory>
#include <cmath>

#include "Point.h"
#include "BoundingBox.h"

#include <Eigen/Dense>

PSAPI_NAMESPACE_BEGIN


namespace Geometry
{

    /// Generate a normalized quad from [0 - 1] in the order top-left, top-right, bot-left, bot-right.
    /// This can then be used to e.g. create a homography.
    template <typename T>
    std::array<Geometry::Point2D<T>, 4> create_normalized_quad()
    {
        auto top_left  = Geometry::Point2D<T>(static_cast<T>(0), static_cast<T>(0));
        auto top_right = Geometry::Point2D<T>(static_cast<T>(1), static_cast<T>(0));
        auto bot_left  = Geometry::Point2D<T>(static_cast<T>(0), static_cast<T>(1));
        auto bot_right = Geometry::Point2D<T>(static_cast<T>(1), static_cast<T>(1));

        return { top_left, top_right, bot_left, bot_right };
    }

    /// Generate a quad from [0 - width/height] in the order top-left, top-right, bot-left, bot-right.
    /// This can then be used to e.g. create a homography.
    template <typename T>
    std::array<Geometry::Point2D<T>, 4> create_quad(T width, T height)
    {
        auto top_left  = Geometry::Point2D<T>(static_cast<T>(0), static_cast<T>(0));
        auto top_right = Geometry::Point2D<T>(width, static_cast<T>(0));
        auto bot_left  = Geometry::Point2D<T>(static_cast<T>(0), height);
        auto bot_right = Geometry::Point2D<T>(width, height);

        return { top_left, top_right, bot_left, bot_right };
    }

    namespace Operations
    {

        /// Move the points by the given offset in-place
        ///
        /// \param points The points to move
        /// \param offset The offset to move by
        template<typename T>
        void move(std::vector<Point2D<T>>& points, Point2D<T> offset)
        {
            std::for_each(std::execution::par_unseq, points.begin(), points.end(), [&](Point2D<T>& point)
                {
                    point.x += offset.x;
                    point.y += offset.y;
                });
        }

        /// Move the vertices by the given offset in-place
        ///
        /// \param vertices The vertices to move
        /// \param offset The offset to move by
        template<typename T>
        void move(std::vector<Vertex<T>>& vertices, Point2D<T> offset)
        {
            std::for_each(std::execution::par_unseq, vertices.begin(), vertices.end(), [&](Vertex<T>& vertex)
                {
                    vertex.point().x += offset.x;
                    vertex.point().y += offset.y;
                });
        }

        /// Rotate the points around the center by the provided angle in-place.
        ///
        /// \param points The points to rotate
        /// \param angle The angle to rotate by in radians
        /// \param center The center point to rotate about
        template<typename T>
        void rotate(std::vector<Point2D<T>>& points, double angle, Point2D<T> center)
        {
            double cos_theta = std::cos(angle);
            double sin_theta = std::sin(angle);

            std::for_each(std::execution::par_unseq, points.begin(), points.end(), [&](Point2D<T>& point)
                {
                    T x = point.x - center.x;
                    T y = point.y - center.y;

                    // Apply rotation around center point
                    T rotated_x = x * cos_theta - y * sin_theta + center.x;
                    T rotated_y = x * sin_theta + y * cos_theta + center.y;

                    Point2D<T> rotated_point = { rotated_x, rotated_y };

                    point = rotated_point;
                });
        }

        /// Rotate the vertices around the center by the provided angle in-place.
        ///
        /// \param vertices The vertices to rotate
        /// \param angle The angle to rotate by in radians
        /// \param center The center point to rotate about
        template<typename T>
        void rotate(std::vector<Vertex<T>>& vertices, double angle, Point2D<T> center)
        {
            double cos_theta = std::cos(angle);
            double sin_theta = std::sin(angle);

            std::for_each(std::execution::par_unseq, vertices.begin(), vertices.end(), [&](Vertex<T>& vertex)
                {
                    T x = vertex.point().x - center.x;
                    T y = vertex.point().y - center.y;

                    // Apply rotation around center point
                    T rotated_x = x * cos_theta - y * sin_theta + center.x;
                    T rotated_y = x * sin_theta + y * cos_theta + center.y;

                    Point2D<T> rotated_point = { rotated_x, rotated_y };

                    vertex.point() = rotated_point;
                });
        }

        /// Scale the points around the center by the factor in-place
        ///
        /// \param points The points to scale
        /// \param factor The scalar factor
        /// \param center The center point to scale about
        template<typename T>
        void scale(std::vector<Point2D<T>>& points, double factor, Point2D<T> center)
        {
            std::for_each(std::execution::par_unseq, points.begin(), points.end(), [&](Point2D<T>& point)
                {
                    T x = point.x - center.x;
                    T y = point.y - center.y;

                    Point2D<T> scaled_point = { x * factor + center.x, y * factor + center.y };
                    // Apply scaling relative to the center
                    point = scaled_point;
                });
        }

        /// Scale the vertices around the center by the factor in-place
        ///
        /// \param vertices The vertices to scale
        /// \param factor The scalar factor
        /// \param center The center point to scale about
        template<typename T>
        void scale(std::vector<Vertex<T>>& vertices, double factor, Point2D<T> center)
        {
            std::for_each(std::execution::par_unseq, vertices.begin(), vertices.end(), [&](Vertex<T>& vertex)
                {
                    T x = vertex.point().x - center.x;
                    T y = vertex.point().y - center.y;

                    Point2D<T> scaled_point = { x * factor + center.x, y * factor + center.y };
                    // Apply scaling relative to the center
                    vertex.point() = scaled_point;
                });
        }

        /// Scale the points around the center by the factor in-place
        ///
        /// \param points The points to scale
        /// \param scalar The scalar
        /// \param center The center point to scale about
        template<typename T>
        void scale(std::vector<Point2D<T>>& points, Point2D<T> scalar, Point2D<T> center)
        {
            std::for_each(std::execution::par_unseq, points.begin(), points.end(), [&](Point2D<T>& point)
                {
                    T x = point.x - center.x;
                    T y = point.y - center.y;

                    Point2D<T> scaled_point = { x * scalar.x + center.x, y * scalar.y + center.y };
                    // Apply scaling relative to the center
                    point = scaled_point;
                });
        }

        /// Scale the vertices around the center by the factor in-place
        ///
        /// \param vertices The vertices to scale
        /// \param scalar The scalar
        /// \param center The center point to scale about
        template<typename T>
        void scale(std::vector<Vertex<T>>& vertices, Point2D<T> scalar, Point2D<T> center)
        {
            std::for_each(std::execution::par_unseq, vertices.begin(), vertices.end(), [&](Vertex<T>& vertex)
                {
                    T x = vertex.point().x - center.x;
                    T y = vertex.point().y - center.y;

                    Point2D<T> scaled_point = { x * scalar.x + center.x, y * scalar.y + center.y };
                    // Apply scaling relative to the center
                    vertex.point() = scaled_point;
                });
        }



        /// Apply a 3x3 transformation matrix to the points
        ///
        /// \param points The points to transform
        /// \param matrix The transformation matrix to apply
        template<typename T>
        void transform(std::vector<Point2D<T>>& points, const Eigen::Matrix<T, 3, 3>& matrix)
        {
            bool zero_division = false;

            std::for_each(std::execution::par_unseq, points.begin(), points.end(), [&](Point2D<T>& point)
                {
                    // Convert to homogeneous coordinates
                    Eigen::Matrix<T, 3, 1> homogenous_points;
                    homogenous_points << point.x, point.y, T(1.0);

                    // Perform transformation
                    Eigen::Matrix<T, 3, 1> transformed_point = matrix * homogenous_points;

                    T w = transformed_point(2); // Get the homogeneous coordinate w

                    if (w != static_cast<T>(0.0))
                    {
                        point = Point2D<T>(transformed_point(0) / w, transformed_point(1) / w);
                    }
                    else
                    {
                        zero_division = true;
                    }
                });

            if (zero_division)
            {
                PSAPI_LOG_ERROR("Geometry", "Error: tried to divide by zero");
            }
        }


        /// Apply a 3x3 transformation matrix to the vertices
        ///
        /// \param vertices The vertices to transform
        /// \param matrix The transformation matrix to apply
        template<typename T>
        void transform(std::vector<Vertex<T>>& vertices, const Eigen::Matrix<T, 3, 3>& matrix)
        {
            bool zero_division = false;

            std::for_each(std::execution::par_unseq, vertices.begin(), vertices.end(), [&](Vertex<T>& vertex)
                {
                    Point2D<T> point = vertex.point();

                    // Convert to homogeneous coordinates
                    Eigen::Matrix<T, 3, 1> homogenous_point;
                    homogenous_point << point.x, point.y, T(1.0);

                    // Perform transformation
                    Eigen::Matrix<T, 3, 1> transformed_point = matrix * homogenous_point;

                    T w = transformed_point(2); // Get the homogeneous coordinate w

                    if (w != static_cast<T>(0.0))
                    {
                        vertex.point() = Point2D<T>(transformed_point(0) / w, transformed_point(1) / w);
                    }
                    else
                    {
                        zero_division = true;
                    }
                });

            if (zero_division)
            {
                PSAPI_LOG_ERROR("Geometry", "Error: tried to divide by zero");
            }
        }

        /// Create a transformation matrix from the given translation, rotation and scalar factors.
        ///
        /// \param move_x  The x component to move by
        /// \param move_y  The y component to move by
        /// \param angle   The angle to rotate by, in radians
        /// \param scale_x The x component of the scale
        /// \param scale_y The y component of the scale
        /// \param pivot   The pivot to perform the transformation around
        template<typename T>
        Eigen::Matrix<T, 3, 3> create_transformation_matrix(T move_x, T move_y, T angle, T scale_x, T scale_y, Point2D<T> pivot)
        {
            T cosAngle = std::cos(angle);
            T sinAngle = std::sin(angle);

            // Create the transformation matrix
            Eigen::Matrix<T, 3, 3> transformationMatrix;
            transformationMatrix <<
                scale_x * cosAngle, -scale_y * sinAngle, pivot.x + move_x - pivot.x * scale_x * cosAngle + pivot.y * scale_y * sinAngle,
                scale_x* sinAngle, scale_y* cosAngle, pivot.y + move_y - pivot.x * scale_x * sinAngle - pivot.y * scale_y * cosAngle,
                0, 0, 1;

            return transformationMatrix;
        }

        /// Create a transformation matrix from the given translation, rotation and scalar factors.
        ///
        /// \param move  The translation factor
        /// \param angle The angle to rotate by, in radians
        /// \param scale The scalar factor
        /// \param pivot The pivot to perform the transformation around
        template<typename T>
        Eigen::Matrix<T, 3, 3> create_transformation_matrix(Point2D<T> move, T angle, Point2D<T> scale, Point2D<T> pivot)
        {
            return create_transformation_matrix(move.x, move.y, angle, scale.x, scale.y, pivot);
        }

        /// Compute a 3x3 homography transformation matrix based on the given source and destination quad
        ///
        /// \param source_points      The points from which we want to compute the homography
        /// \param destination points The points to which we want to transform.
        template<typename T>
        Eigen::Matrix3d create_homography_matrix(const std::array<Point2D<T>, 4>& source_points, const std::array<Point2D<T>, 4>& destination_points)
        {
            // Populate the matrix using the src and dest coordinates
            // https://math.stackexchange.com/questions/494238/how-to-compute-homography-matrix-h-from-corresponding-points-2d-2d-planar-homog
            Eigen::MatrixXd A(8, 9);

            for (size_t i = 0; i < source_points.size(); ++i)
            {
                double x = static_cast<double>(source_points[i].x);
                double y = static_cast<double>(source_points[i].y);
                double xw = static_cast<double>(destination_points[i].x);
                double yw = static_cast<double>(destination_points[i].y);

                A(2 * i, 0) = x;
                A(2 * i, 1) = y;
                A(2 * i, 2) = 1;
                A(2 * i, 3) = 0;
                A(2 * i, 4) = 0;
                A(2 * i, 5) = 0;
                A(2 * i, 6) = -xw * x;
                A(2 * i, 7) = -xw * y;
                A(2 * i, 8) = -xw;

                A(2 * i + 1, 0) = 0;
                A(2 * i + 1, 1) = 0;
                A(2 * i + 1, 2) = 0;
                A(2 * i + 1, 3) = x;
                A(2 * i + 1, 4) = y;
                A(2 * i + 1, 5) = 1;
                A(2 * i + 1, 6) = -yw * x;
                A(2 * i + 1, 7) = -yw * y;
                A(2 * i + 1, 8) = -yw;
            }

            // Compute A^T * A
            Eigen::MatrixXd AtA = A.transpose() * A;

            // Compute the eigenvalues and eigenvectors
            Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> solver(AtA);
            Eigen::VectorXd eigenvalues = solver.eigenvalues();
            Eigen::MatrixXd eigenvectors = solver.eigenvectors();

            // Get the index of the smallest eigenvalue
            int min_eig_idx = -1;
            eigenvalues.minCoeff(&min_eig_idx);

            // Extract the corresponding eigenvector
            Eigen::VectorXd smallest_eigen_vec = eigenvectors.col(min_eig_idx);

            // Reshape into 3x3 homography matrix
            Eigen::Matrix3d homography;
            homography << smallest_eigen_vec(0), smallest_eigen_vec(1), smallest_eigen_vec(2),
                smallest_eigen_vec(3), smallest_eigen_vec(4), smallest_eigen_vec(5),
                smallest_eigen_vec(6), smallest_eigen_vec(7), smallest_eigen_vec(8);

            // Normalize the homography
            homography /= homography(2, 2);

            return homography;
        }


    } // namespace Operations

} // namespace Geometry

PSAPI_NAMESPACE_END