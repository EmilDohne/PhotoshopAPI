
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
    };

}

PSAPI_NAMESPACE_END