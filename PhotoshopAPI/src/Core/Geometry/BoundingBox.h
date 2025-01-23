
#pragma once

#include "Macros.h"

#include "Point.h"

#include <array>

PSAPI_NAMESPACE_BEGIN

namespace Geometry
{

    namespace lambdas
    {
        constexpr inline auto in_bbox = []<typename T>(const Point2D<T>&pt, const Point2D<T>&minimum, const Point2D<T>&maximum) noexcept -> bool
        {
            return pt.x >= minimum.x && pt.x <= maximum.x && pt.y >= minimum.y && pt.y <= maximum.y;
        };
    }

    /// Basic bounding box representation with methods for checking intersections of
    /// Point -> BoundingBox and BoundingBox -> BoundingBox
    template <typename T>
    struct BoundingBox
    {
        /// The minimum (bottom-left) corner of the bounding box.
        Point2D<T> minimum;

        /// The maximum (top-right) corner of the bounding box.
        Point2D<T> maximum;

        /// Default constructor.
        /// Initializes the bounding box with default `Point2D` values.
        BoundingBox() = default;

        /// Constructor to initialize the bounding box with specific corners.
        /// \param _minimum The bottom-left corner.
        /// \param _maximum The top-right corner.
        BoundingBox(Point2D<T> _minimum, Point2D<T> _maximum)
            : minimum(_minimum), maximum(_maximum) {}

        /// Check if a point is inside the bounding box.
        /// \param pt The point to check.
        /// \return True if the point is within the bounding box; otherwise, false.
        constexpr bool in_bbox(const Point2D<T>& pt) const
        {
            return pt.x >= minimum.x && pt.x <= maximum.x && pt.y >= minimum.y && pt.y <= maximum.y;
        }

        

        /// Check if another bounding box partially overlaps with this one.
        /// \param other The other bounding box to check.
        /// \return True if the bounding boxes overlap; otherwise, false.
        bool in_bbox(const BoundingBox<T> other) const
        {
            return !(other.maximum.x < minimum.x || other.minimum.x > maximum.x ||
                other.maximum.y < minimum.y || other.minimum.y > maximum.y);
        }

        /// Compute the size of the bounding box.
        /// \return A `Point2D` representing the width and height of the bounding box.
        Point2D<T> size() const
        {
            return {
                maximum.x - minimum.x,
                maximum.y - minimum.y
            };
        }

        /// Compute the width of the bounding box.
        /// \return The width of the bounding box.
        T width() const
        {
            return maximum.x - minimum.x;
        }

        /// Compute the height of the bounding box.
        /// \return The height of the bounding box.
		T height() const
		{
			return maximum.y - minimum.y;
		}
        
        /// Compute the center of the bounding box.
        /// \return A `Point2D` representing the center of the bounding box.
        Point2D<T> center() const
        {
            return Point2D<T>((minimum.x + maximum.x) * .5f, (minimum.y + maximum.y) * .5f);
        }

        /// Offset the bounding box by a given point.
        /// \param _offset The point to add to both the minimum and maximum corners.
        void offset(Point2D<T> _offset)
        {
            this->minimum = this->minimum + _offset;
            this->maximum = this->maximum + _offset;
        }

        /// Pad the bounding box by the given amount which may be negative.
        /// \param _amount The amount to pad by
        void pad(T _amount)
        {
            this->minimum = this->minimum - _amount;
            this->maximum = this->maximum + _amount;
        }

        /// Compute the intersection of two bounding boxes.
        /// \param a The first bounding box.
        /// \param b The second bounding box.
        /// \return An optional bounding box representing the overlap between the two.
        /// If no overlap exists, returns `std::nullopt`.
        template <typename T>
        static std::optional<BoundingBox<T>> intersect(const BoundingBox<T>& a, const BoundingBox<T>& b)
        {
            // Calculate the intersection bounds
            Point2D<T> new_minimum = {
                std::max(a.minimum.x, b.minimum.x),
                std::max(a.minimum.y, b.minimum.y)
            };

            Point2D<T> new_maximum = {
                std::min(a.maximum.x, b.maximum.x),
                std::min(a.maximum.y, b.maximum.y)
            };

            // Check if the intersection is valid (non-empty)
            if (new_minimum.x <= new_maximum.x && new_minimum.y <= new_maximum.y)
            {
                return BoundingBox<T>(new_minimum, new_maximum);
            }
            else
            {
                return std::nullopt; // No intersection
            }
        }

        /// Represent the bounding box as a quadrilateral.
        /// \return An array of four points representing the corners of the bounding box
        /// in the order: top-left, top-right, bottom-left, bottom-right.
        constexpr std::array<Geometry::Point2D<T>, 4> as_quad() const
        {
            return std::array<Geometry::Point2D<T>, 4> {
                minimum, Geometry::Point2D<T>{maximum.x, minimum.y},
                Geometry::Point2D<T>{minimum.x, maximum.y}, maximum
            };
        }

        /// Compute the bounding box over a set of points.
        /// \param points A vector of points to enclose.
        /// \return The bounding box that minimally encloses all the points.
        static BoundingBox compute(const std::vector<Point2D<T>>& points)
        {
            BoundingBox bbox;
            bbox.minimum = Point2D<T>(std::numeric_limits<T>::max(), std::numeric_limits<T>::max());
            bbox.maximum = Point2D<T>(std::numeric_limits<T>::lowest(), std::numeric_limits<T>::lowest());

            for (auto point : points)
            {
                bbox.minimum.x = std::min(bbox.minimum.x, point.x);
                bbox.minimum.y = std::min(bbox.minimum.y, point.y);
                bbox.maximum.x = std::max(bbox.maximum.x, point.x);
                bbox.maximum.y = std::max(bbox.maximum.y, point.y);
            }
            return bbox;
        }


        /// Compute the bounding box over a set of vertices.
        /// \param vertices A vector of vertices to enclose.
        /// \return The bounding box that minimally encloses all the vertices.
        static BoundingBox compute(const std::vector<Vertex<T>>& vertices)
        {
            BoundingBox bbox;
            bbox.minimum = Point2D<T>(std::numeric_limits<T>::max(), std::numeric_limits<T>::max());
            bbox.maximum = Point2D<T>(std::numeric_limits<T>::lowest(), std::numeric_limits<T>::lowest());

            for (const auto& vtx : vertices)
            {
                bbox.minimum.x = std::min(bbox.minimum.x, vtx.point().x);
                bbox.minimum.y = std::min(bbox.minimum.y, vtx.point().y);
                bbox.maximum.x = std::max(bbox.maximum.x, vtx.point().x);
                bbox.maximum.y = std::max(bbox.maximum.y, vtx.point().y);
            }

            return bbox;
        }
    };

}

PSAPI_NAMESPACE_END