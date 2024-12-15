#pragma once

#include "Macros.h"

#include <vector>
#include <array>
#include <memory>
#include <cmath>
#include <cassert>
#include <algorithm>

// If we compile with C++<20 we replace the stdlib implementation with the compatibility
// library
#if (__cplusplus < 202002L)
#include "tcb_span.hpp"
#else
#include <span>
#endif

PSAPI_NAMESPACE_BEGIN

namespace Geometry
{

    /// Trivially copyable 2D Point implementation that provides basic arithmetic operators
    /// as well as bounds-checked template access to the arguments
    template <typename T>
    struct Point2D
    {
        T x{};
        T y{};

        Point2D() = default;
        constexpr Point2D(T x_val, T y_val) : x(x_val), y(y_val) {}

        // Equality operators
        bool operator==(Point2D<T> other) const { return this->x == other.x && this->y == other.y; }
        bool operator==(T other) const { return this->x == other && this->y == other; }
        bool operator!=(Point2D<T> other) const { return !(*this == other); }

        static bool equal(Point2D<T> pt1, Point2D<T> pt2, double epsilon = 1e-6)
        {
            if constexpr (!std::is_floating_point_v<T>)
            {
                PSAPI_LOG_ERROR("Point2D", "equal() function not required for non-floating type points, consider using operator==");
            }
            bool x_equal = std::fabs(pt1.x - pt2.x) < epsilon;
            bool y_equal = std::fabs(pt1.y - pt2.y) < epsilon;
            return x_equal && y_equal;
        }

        // Arithmetic operators
        Point2D<T> operator+(Point2D<T> other) const { return { this->x + other.x, this->y + other.y }; }

        Point2D<T> operator-(Point2D<T> other) const { return { this->x - other.x, this->y - other.y }; }
        Point2D<T> operator-(double other) const { return { this->x - other, this->y - other }; }
        Point2D<T> operator-() const { return { -this->x, -this->y }; }

        Point2D<T> operator*(double factor) const { return { this->x * factor, this->y * factor }; }
        Point2D<T> operator*(Point2D<T> other) const { return { this->x * other.x, this->y * other.y }; }

        Point2D<T>& operator*=(Point2D<T> other) { this->x *= other.x; this->y *= other.y; return *this; }

        Point2D<T> operator/(double value) const 
        { 
            assert(value != static_cast<double>(0.0f));
            return Point2D<T>(this->x / value, this->y / value); 
        }
        Point2D<T> operator/(Point2D<T> other) const 
        { 
            assert(other.x != static_cast<double>(0.0f));
            assert(other.y != static_cast<double>(0.0f));
            return Point2D<T>{this->x / other.x, this->y / other.y};
        }

		T distance(const Point2D<T> other) const 
        {
			return std::sqrt((x - other.x) * (x - other.x) + (y - other.y) * (y - other.y));
		}

        /// Static lerp function interpolating between a and b at position t where position t is between 0 and 1
        static Point2D lerp(const Point2D<T> a, const Point2D<T> b, double t)
        {
            return a * (1 - t) + b * t;
        }

        /// Bounds-checked access to the x coordinate of the point, throws error if value would exceed numeric limit
        template <typename U = T>
        U x_checked()
        {
            if constexpr (!std::is_arithmetic_v<U>)
            {
                PSAPI_LOG_ERROR("Point2D", "Unable to perform bounds-checked access to points' y coordinate as U is not arithmetic");
            }

            if (this->x > std::numeric_limits<U>::max())
            {
                PSAPI_LOG_ERROR("Point2D", "Unable to perform bounds-checked access to points' x coordinate as it would exceed the numeric limit of U");
            }
            if (this->x < std::numeric_limits<U>::min())
            {
                PSAPI_LOG_ERROR("Point2D", "Unable to perform bounds-checked access to points' x coordinate as it would exceed the numeric limit of U");
            }
            return static_cast<U>(this->x);
        }

        /// Bounds-checked access to the y coordinate of the point, throws error if value would exceed numeric limit
        template <typename U = T>
        U y_checked()
        {
            if constexpr (!std::is_arithmetic_v<U>)
            {
                PSAPI_LOG_ERROR("Point2D", "Unable to perform bounds-checked access to points' y coordinate as U is not arithmetic");
            }

            if (this->y > std::numeric_limits<U>::max())
            {
                PSAPI_LOG_ERROR("Point2D", "Unable to perform bounds-checked access to points' y coordinate as it would exceed the numeric limit of U");
            }
            if (this->y < std::numeric_limits<U>::min())
            {
                PSAPI_LOG_ERROR("Point2D", "Unable to perform bounds-checked access to points' y coordinate as it would exceed the numeric limit of U");
            }
            return static_cast<U>(this->y);
        }

        /// Hash function for Point2D, allowing Point2D objects to be used in hashed containers like std::unordered_map.
        /// Combines the hash of the x and y coordinates.
        size_t hash() const
        {
            // Use a simple hash combination method
            return std::hash<T>()(x) ^ (std::hash<T>()(y) << 1); // XOR combined with left shift for distribution
        }
    };


    /// Extension of a Point2D to additionally describe the UV coordinate of a given point.
    template <typename T>
    struct Vertex
    {
        Vertex() = default;
        Vertex(Point2D<T> point) : m_Point(point) {}
        Vertex(Point2D<T> point, Point2D<double> uv) : m_Point(point), m_UV(uv) {}

        Point2D<T> point() const
        {
            return m_Point;
        }

        Point2D<T>& point()
        {
            return m_Point;
        }

        Point2D<double> uv() const
        {
            return m_UV;
        }

    private:
        Point2D<T> m_Point = { 0.0f, 0.0f };
        Point2D<double> m_UV = { -1.0, -1.0 };
    };
}

PSAPI_NAMESPACE_END


// Specialization of std::hash for Point2D
namespace std
{
    template <typename T>
    struct hash<NAMESPACE_PSAPI::Geometry::Point2D<T>>
    {
        size_t operator()(const NAMESPACE_PSAPI::Geometry::Point2D<T>& point) const
        {
            return point.hash(); // Use the Point2D's hash method
        }
    };
}