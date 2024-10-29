#pragma once

#include "Macros.h"

#include <vector>
#include <array>
#include <memory>
#include <cmath>
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
        Point2D(T x_val, T y_val) : x(x_val), y(y_val) {}

        // Equality operators
        bool operator==(const Point2D<T>& other) const { return this->x == other.x && this->y == other.y; }
        bool operator==(const T other) const { return this->x == other && this->y == other; }
        bool operator!=(const Point2D<T>& other) const { return !(*this == other); }

        // Arithmetic operators
        Point2D<T> operator+(const Point2D<T>& other) const { return { this->x + other.x, this->y + other.y }; }
        Point2D<T> operator-(const Point2D<T>& other) const { return { this->x - other.x, this->y - other.y }; }
        Point2D<T> operator*(double factor) const { return { this->x * factor, this->y * factor }; }
        Point2D<T> operator/(double value) const { return Point2D<T>(this->x / value, this->y / value); }

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