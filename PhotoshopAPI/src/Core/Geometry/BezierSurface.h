#pragma once


#include "Macros.h"
#include "Util/Logger.h"

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
            :m_GridWidth(gridWidth), m_GridHeight(gridHeight) 
        {
            if (controlPoints.size() != gridWidth * gridHeight) 
            {
                PSAPI_LOG_ERROR("BezierSurface", "Control points size must match grid dimensions.");
            }
            if (gridWidth < 4 || gridHeight < 4)
            {
                PSAPI_LOG_ERROR("BezierSurface", "Unable to create bezier surface as it is not at least cubic. Expected at the very least 4x4 divisions");
            }

            // Ensure grid dimensions allow for cubic patches
            if (gridWidth > 4)
            {
                if ((gridWidth - 4) % 3 != 0)
                {
                    PSAPI_LOG_ERROR("BezierSurface", "Grid dimensions must allow for cubic 4x4 patches.");
                }
                m_NumPatchesX = 1 + (gridWidth - 4) / 3;

            }
            if (gridHeight > 4)
            {
                if ((gridHeight - 4) % 3 != 0)
                {
                    PSAPI_LOG_ERROR("BezierSurface", "Grid dimensions must allow for cubic 4x4 patches.");
                }
                m_NumPatchesY = 1 + (gridHeight - 4) / 3;
            }

            // Initialize patches
            m_Patches.reserve(m_NumPatchesX * m_NumPatchesY);

            for (size_t py = 0; py < m_NumPatchesY; ++py) 
            {
                for (size_t px = 0; px < m_NumPatchesX; ++px) 
                {
                    std::array<Point2D<double>, 16> patch;

                    // Fill the 4x4 patch in scanline order with overlap
                    for (size_t y = 0; y < 4; ++y) 
                    {
                        for (size_t x = 0; x < 4; ++x) 
                        {
                            size_t controlPointIndex = (py * 3 + y) * gridWidth + (px * 3 + x);
                            patch[y * 4 + x] = controlPoints[controlPointIndex];
                        }
                    }
                    m_Patches.push_back(patch);
                }
            }
        }

        // Evaluate any patch at (u, v) based on subdivisions across x and y
        Point2D<double> evaluate(double u, double v) const
        {
            PSAPI_PROFILE_FUNCTION();
            double patch_size_u = 1.0 / m_NumPatchesX;
            double patch_size_v = 1.0 / m_NumPatchesY;

            // The std::min here is to ensure we don't try to access an out of bounds patch
            // if u or v is exactly 1
            size_t patch_index_x = std::min(static_cast<size_t>(std::floor(u / patch_size_u)), m_NumPatchesX - 1 );
            size_t patch_index_y = std::min(static_cast<size_t>(std::floor(v / patch_size_v)), m_NumPatchesY - 1 );

            // Calculate base UV of the patch
            double patch_base_u = patch_index_x * patch_size_u;
            double patch_base_v = patch_index_y * patch_size_v;

            // Calculate local UV within the patch
            double local_u = (u - patch_base_u) / patch_size_u;
            double local_v = (v - patch_base_v) / patch_size_v;

            local_u = std::clamp(local_u, static_cast<double>(0.0f), static_cast<double>(1.0f));
            local_v = std::clamp(local_v, static_cast<double>(0.0f), static_cast<double>(1.0f));

            // Retrieve control points for this patch and evaluate
            auto patch = get_patch_ctrl_points(patch_index_x, patch_index_y);
            return evaluate_bezier_patch(patch, local_u, local_v);
        }

        // Convert the bezier patch into a mesh by repeatedly calling evaluate for x and y intervals across the surface
        // This function will create a quadrilateral mesh which can be more easily queried for UV coordinates
        // at a given point than the bezier surface itself lending itself to e.g. mesh warps.
        Mesh<double> mesh(size_t divisions_x, size_t divisions_y, std::array<Geometry::Point2D<double>, 4> non_affine_transform) const
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
            return Mesh<double>(points, non_affine_transform, divisions_x, divisions_y);
        }

        /// Get the patches associated with the Bezier surface, these are sorted in scanline order
        /// and represent 4x4 cubic bezier patches
        std::vector<std::array<Point2D<double>, 16>> patches() const noexcept
        {
            return m_Patches;
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
        std::vector<std::array<Point2D<double>, 16>> m_Patches;
        size_t m_GridWidth = 0;
        size_t m_GridHeight = 0;

        size_t m_NumPatchesX = 1;
        size_t m_NumPatchesY = 1;

        // Retrieve the 4x4 grid of control points for a patch defined by patchX, patchY
        // where patchX is the x position in the grid and patchY is the y position in the grid
        std::array<Point2D<double>, 16> get_patch_ctrl_points(size_t patchX, size_t patchY) const
        {
            return m_Patches[patchY * m_NumPatchesX + patchX];
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