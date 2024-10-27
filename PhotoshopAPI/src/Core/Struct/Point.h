#pragma once

#include "Macros.h"


PSAPI_NAMESPACE_BEGIN


template <typename T>
struct Point
{
	T x{};
	T y{};

	Point() = default;
	Point(T x_val, T y_val) : x(x_val), y(y_val) {}

	// Equality operators
	bool operator==(const Point& other) const { return x == other.x && y == other.y; }
	bool operator!=(const Point& other) const { return !(*this == other); }

	Point operator+(const Point& other) const { return { x + other.x, y + other.y }; }
	Point operator*(double factor) const { return { x * factor, y * factor }; }
	Point& operator+=(const Point& other) { x += other.x; y += other.y; return *this; }
	Point operator/(float value) const { return Point<T>(x / value, y / value); }
};

PSAPI_NAMESPACE_END