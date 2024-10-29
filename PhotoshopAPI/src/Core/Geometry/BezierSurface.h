#pragma once


#include "Macros.h"
#include "Logger.h"

#include <vector>

#include "Point.h"
#include "Mesh.h"


PSAPI_NAMESPACE_BEGIN

namespace Geometry
{

    /// A generic Bezier surface struct with 
    struct BezierSurface 
    {

        /// Constructor that initializes control points, these must be in scanline order. So the points should go as follows:
        /// 1 2 3 4 5
        /// 6 7 8 9 10 ...
        BezierSurface(const std::vector<Point2D<double>>& controlPoints, size_t gridWidth, size_t gridHeight)
            : m_ControlPoints(controlPoints), m_GridWidth(gridWidth), m_GridHeight(gridHeight) 
        {
            if (controlPoints.size() != gridWidth * gridHeight) 
            {
                PSAPI_LOG_ERROR("BezierSurface", "Control points size must match grid dimensions.");
            }
            if (gridWidth < 4 || gridHeight < 4)
            {
                PSAPI_LOG_ERROR("BezierSurface", "Unable to create bezier surface as it is not at least cubic. Expected at the very least 4x4 divisions");
            }
        }

        // Evaluate any patch at (u, v) based on subdivisions across x and y
        Point2D<double> evaluate(double u, double v) const
        {
            // Scale u and v to locate the correct patch
            double u_scaled = u * (m_GridWidth - 3);
            double v_scaled = v * (m_GridHeight - 3);

            // Get integer patch indices and local u, v for the patch
            size_t patchX = static_cast<size_t>(u_scaled);
            size_t patchY = static_cast<size_t>(v_scaled);

            // Ensure we don't go out of bounds
            patchX = std::min(patchX, m_GridWidth - 4);
            patchY = std::min(patchX, m_GridHeight - 4);

            double localU = u_scaled - patchX;
            double localV = v_scaled - patchY;

            // Retrieve control points for this patch and evaluate
            std::array<Point2D<double>, 16> patch = get_patch_ctrl_points(patchX, patchY);
            return evaluate_bezier_patch(patch, localU, localV);
        }

        // Convert the bezier patch into a mesh by repeatedly calling evaluate for x and y intervals across the surface
        // This function will create a quadrilateral mesh which can be more easily queried for UV coordinates
        // at a given point than the bezier surface itself lending itself to e.g. mesh warps.
        Mesh<double> mesh(size_t divisions_x, size_t divisions_y) const
        {
            PSAPI_PROFILE_FUNCTION();

            std::vector<Point2D<double>> points;
            points.reserve(divisions_x * divisions_y);
            for (size_t y = 0; y < divisions_y; ++y)
            {
                float v = static_cast<float>(y) / (divisions_y - 1);
                for (size_t x = 0; x < divisions_x; ++x)
                {
                    float u = static_cast<float>(x) / (divisions_x - 1);
                    points.emplace_back(evaluate(u, v));
                }
            }
            return Mesh<double>(points, divisions_x, divisions_y);
        }

        /// Get the control points associated with the Bezier surface, these are sorted in scanline order
        std::vector<Point2D<double>> control_points() const noexcept
        {
            return m_ControlPoints;
        }

        /// Get the number of divisions across the x plane
        size_t grid_width() const noexcept
        {
            return m_GridWidth;
        }

        /// Get the number of divisions across the y plane
        size_t grid_height() const noexcept
        {
            return m_GridWidth;
        }

    private:
        std::vector<Point2D<double>> m_ControlPoints;
        size_t m_GridWidth = 0;
        size_t m_GridHeight = 0;

        // Cache for memoization of get_patch_ctrl_points so we dont have to 
        // create the patch for what is likely to be a lot of repeated calls
        mutable std::unordered_map<Point2D<size_t>, std::array<Point2D<double>, 16>> m_Cache;

        // Retrieve the 4x4 grid of control points for a patch defined by patchX, patchY
        // where patchX is the x position in the grid and patchY is the y position in the grid
        std::array<Point2D<double>, 16> get_patch_ctrl_points(size_t patchX, size_t patchY) const
        {
            auto cacheKey = Point2D<size_t>(patchX, patchY);
            auto it = m_Cache.find(cacheKey);
            if (it != m_Cache.end()) 
            {
                return it->second; // Return cached result
            }

            std::array<Point2D<double>, 16> patch;
            for (size_t i = 0; i < 4; ++i)
            {
                for (size_t j = 0; j < 4; ++j)
                {
                    size_t index = (patchY + i) * m_GridWidth + (patchX + j);
                    patch[i * 4 + j] = m_ControlPoints[index];
                }
            }

            m_Cache[cacheKey] = patch; // Store in cache
            return patch;
        }

        /// Evaluate the cubic bezier patch at the given u and v coordinate returning the position in world space
        /// of the intersection. Uses De Casteljau's algorithm internally
        Point2D<double> evaluate_bezier_patch(const std::array<Point2D<double>, 16> patch, double u, double v) const
        {
            std::array<Point2D<double>, 4> curves;

            curves[0] = evaluate_bezier_curve({ patch[0], patch[1], patch[2], patch[3]       }, u);
            curves[1] = evaluate_bezier_curve({ patch[4], patch[5], patch[6], patch[7]       }, u);
            curves[2] = evaluate_bezier_curve({ patch[8], patch[9], patch[10], patch[11]     }, u);
            curves[3] = evaluate_bezier_curve({ patch[12], patch[13], patch[14], patch[15]   }, u);

            return evaluate_bezier_curve(curves, v);
        }

        /// Evaluate a cubic Bézier curve at parameter t using De Casteljau's algorithm.
        Point2D<double> evaluate_bezier_curve(const std::array<Point2D<double>, 4> points, double t) const
        {
            auto a = Point2D<double>::lerp(points[0], points[1], t);
            auto b = Point2D<double>::lerp(points[1], points[2], t);
            auto c = Point2D<double>::lerp(points[2], points[3], t);

            auto d = Point2D<double>::lerp(a, b, t);
            auto e = Point2D<double>::lerp(b, c, t);

            return Point2D<double>::lerp(d, e, t);
        }
    };

}

PSAPI_NAMESPACE_END