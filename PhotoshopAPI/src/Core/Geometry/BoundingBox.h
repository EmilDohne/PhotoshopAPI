
#pragma once

#include "Macros.h"

#include "Point.h"


PSAPI_NAMESPACE_BEGIN

namespace Geometry
{

    /// Basic bounding box representation with methods for checking intersections of
    /// Point -> BoundingBox and BoundingBox -> BoundingBox
    template <typename T>
    struct BoundingBox
    {
        Point2D<T> minimum;
        Point2D<T> maximum;

        BoundingBox() = default;

        BoundingBox(Point2D<T> _minimum, Point2D<T> _maximum)
            : minimum(_minimum), maximum(_maximum) {}

        bool in_bbox(Point2D<T> pt) const
        {
            return pt.x >= minimum.x && pt.x <= maximum.x &&
                pt.y >= minimum.y && pt.y <= maximum.y;
        }

        // Check if another bounding box partially overlaps with this one
        bool in_bbox(const BoundingBox<T> other) const
        {
            return !(other.maximum.x < minimum.x || other.minimum.x > maximum.x ||
                other.maximum.y < minimum.y || other.minimum.y > maximum.y);
        }

        /// Return the size of the bbox
        Point2D<T> size() const
        {
            return {
                maximum.x - minimum.x,
                maximum.y - minimum.y
            };
        }

        T width() const
        {
            return maximum.x - minimum.x;
        }

		T height() const
		{
			return maximum.y - minimum.y;
		}
        
        Point2D<T> center() const
        {
            return Point2D<T>((minimum.x + maximum.x) * .5f, (minimum.y + maximum.y) * .5f);
        }

        /// Compute the bounding box over the provided points
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
    };

}

PSAPI_NAMESPACE_END