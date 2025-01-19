/*
Basic rendering library for defining a renderable buffer as well as rendering targets into that buffer.
Support for this is still experimental and primarily for debug purposes
*/


#pragma once

#include "Macros.h"

#include <vector>
#include <array>
#include <memory>
#include <cmath>
#include <algorithm>
#include <tuple>

#include "ImageBuffer.h"

#include "Core/Geometry/Point.h"
#include "Core/Geometry/Mesh.h"
#include "Core/Geometry/BezierSurface.h"

// If we compile with C++<20 we replace the stdlib implementation with the compatibility
// library
#if (__cplusplus < 202002L)
#include "tcb_span.hpp"
#else
#include <span>
#endif

#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>

PSAPI_NAMESPACE_BEGIN

namespace Render
{


    /// Rendering style enum
    enum class Style
    {
        outline,
        filled
    };


    /// Render a line to the image buffer from pt1 to pt2 with the specified pixel value.
    ///
    /// This function draws a line between two points, `pt1` and `pt2`, in the provided `ChannelBuffer`.
    /// The pixel values along the line are set to the specified `value`. This implementation utilizes
    /// Wu's algorithm to achieve anti-aliasing for smoother line rendering.
    ///
    /// \tparam T The type of pixel value stored in the `ChannelBuffer`.
    /// \tparam U The type of the coordinate values used in `Geometry::Point2D`.
    ///
    /// \param buffer The image buffer where the line will be rendered.
    /// \param pt1 The starting point of the line as a `Geometry::Point2D` object.
    /// \param pt2 The ending point of the line as a `Geometry::Point2D` object.
    /// \param value The pixel value to use for rendering the line. This value will be used to set 
    ///              the color of the pixels along the line.
    template <typename T, typename U>
    void render_line(ChannelBuffer<T> buffer, Geometry::Point2D<U> pt1, Geometry::Point2D<U> pt2, T value)
    {
        auto fpart = [](float x) -> float {return x - std::floor(x); };
        auto rfpart = [=](float x) -> float {return 1 - fpart(x); };

        auto set_pixel = [&](int x, int y, float alpha)
            {
                if (x < buffer.width && y < buffer.height)
                {
                    T& pixel = buffer.buffer[y * buffer.width + x];
                    pixel = static_cast<T>(pixel * (1.0f - alpha) + value * alpha);
                }
            };

        // Wu's algorithm adapted from here:
        // https://rosettacode.org/wiki/Xiaolin_Wu%27s_line_algorithm
        auto plot_line_wu = [&](int x0, int y0, int x1, int y1)
            {
                bool steep = std::abs(y1 - y0) > std::abs(x1 - x0);

                if (steep)
                {
                    std::swap(x0, y0);
                    std::swap(x1, y1);
                }
                if (x0 > x1)
                {
                    std::swap(x0, x1);
                    std::swap(y0, y1);
                }

                float dx = x1 - x0;
                float dy = y1 - y0;
                float gradient = (dx == 0) ? 1 : dy / dx;

                int xpx11 = 0;
                float intery = 0.0f;
                {
                    const float xend = std::round(x0);
                    const float yend = y0 + gradient * (xend - x0);
                    const float xgap = rfpart(x0 + 0.5);
                    xpx11 = static_cast<int>(xend);
                    const int ypx11 = static_cast<int>(std::floor(yend));
                    if (steep) 
                    {
                        set_pixel(ypx11, xpx11, rfpart(yend) * xgap);
                        set_pixel(ypx11 + 1, xpx11, fpart(yend) * xgap);
                    }
                    else 
                    {
                        set_pixel(xpx11, ypx11, rfpart(yend) * xgap);
                        set_pixel(xpx11, ypx11 + 1, fpart(yend) * xgap);
                    }
                    intery = yend + gradient;
                }

                int xpx12 = 0;
                {
                    const float xend = std::round(x1);
                    const float yend = y1 + gradient * (xend - x1);
                    const float xgap = rfpart(x1 + 0.5);
                    xpx12 = static_cast<int>(xend);
                    const int ypx12 = static_cast<int>(std::floor(yend));
                    if (steep) 
                    {
                        set_pixel(ypx12, xpx12, rfpart(yend) * xgap);
                        set_pixel(ypx12 + 1, xpx12, fpart(yend) * xgap);
                    }
                    else 
                    {
                        set_pixel(xpx12, ypx12, rfpart(yend) * xgap);
                        set_pixel(xpx12, ypx12 + 1, fpart(yend) * xgap);
                    }
                }

                if (steep) 
                {
                    for (int x = xpx11 + 1; x < xpx12; x++) 
                    {
                        set_pixel(static_cast<int>(std::floor(intery)), x, rfpart(intery));
                        set_pixel(static_cast<int>(std::floor(intery)) + 1, x, fpart(intery));
                        intery += gradient;
                    }
                }
                else 
                {
                    for (int x = xpx11 + 1; x < xpx12; x++) 
                    {
                        set_pixel(x, static_cast<int>(std::floor(intery)), rfpart(intery));
                        set_pixel(x, static_cast<int>(std::floor(intery)) + 1, fpart(intery));
                        intery += gradient;
                    }
                }
            };

        plot_line_wu(pt1.x_checked<int>(), pt1.y_checked<int>(), pt2.x_checked<int>(), pt2.y_checked<int>());
    }


    /// Render a circle at the specified center point into the given image buffer.
    ///
    /// This function draws a circle with a specified radius centered at the provided `Geometry::Point2D`.
    /// The circle can be either filled or outlined based on the specified `Style`. The value used 
    /// to color the circle is passed as a template parameter.
    ///
    /// \tparam T The type of pixel value stored in the `ChannelBuffer`.
    /// \tparam U The type of the coordinate values used in `Geometry::Point2D`.
    ///
    /// \param buffer The image buffer where the circle will be rendered.
    /// \param center The center point of the circle as a `Geometry::Point2D` object.
    /// \param radius The radius of the circle to be rendered.
    /// \param value The pixel value to use for rendering the circle.
    /// \param style The style of the circle rendering, which can be either filled or outlined.
    template <typename T, typename U>
    void render_circle(ChannelBuffer<T>& buffer, Geometry::Point2D<U> center, size_t radius, T value, Style style)
    {
        auto set_pixel = [&](size_t x, size_t y)
            {
                if (x < buffer.width && y < buffer.height)
                {
                    buffer.buffer[y * buffer.width + x] = value;
                }
            };

        if (style == Style::filled)
        {
            // Filled circle - draw all pixels within radius
            for (size_t y = center.y - radius; y <= center.y + radius; ++y)
            {
                for (size_t x = center.x - radius; x <= center.x + radius; ++x)
                {
                    size_t dx = x - center.x;
                    size_t dy = y - center.y;
                    if (dx * dx + dy * dy <= radius * radius)
                    {
                        set_pixel(x, y);
                    }
                }
            }
        }
        else
        {
            // Outline circle - Bresenham’s Circle Algorithm
            size_t x = radius;
            size_t y = 0;
            size_t decisionOver2 = 1 - x;

            while (y <= x)
            {
                set_pixel(center.x + x, center.y + y);
                set_pixel(center.x + y, center.y + x);
                set_pixel(center.x - x, center.y + y);
                set_pixel(center.x - y, center.y + x);
                set_pixel(center.x - x, center.y - y);
                set_pixel(center.x - y, center.y - x);
                set_pixel(center.x + x, center.y - y);
                set_pixel(center.x + y, center.y - x);
                ++y;
                if (decisionOver2 <= 0)
                {
                    decisionOver2 += 2 * y + 1;
                }
                else
                {
                    --x;
                    decisionOver2 += 2 * (y - x) + 1;
                }
            }
        }
    }


    /// Render a rectangle at the specified coordinates into the given image buffer.
    ///
    /// This function draws a rectangle defined by two corner points (top-left and bottom-right)
    /// in the provided `ChannelBuffer`. The rectangle can be either filled or outlined based on 
    /// the specified `Style`. The value used to color the rectangle is passed as a template parameter.
    ///
    /// \tparam T The type of pixel value stored in the `ChannelBuffer`.
    /// \tparam U The type of the coordinate values used in `Geometry::Point2D`. Internally converts to int
    ///
    /// \param buffer The image buffer where the rectangle will be rendered.
    /// \param top_left The top-left corner of the rectangle as a `Geometry::Point2D` object.
    /// \param bottom_right The bottom-right corner of the rectangle as a `Geometry::Point2D` object.
    /// \param value The pixel value to use for rendering the rectangle. This value will be converted 
    ///              to float for rendering purposes.
    /// \param style The style of the rectangle rendering, which can be either filled or outlined.
    template <typename T, typename U>
    void render_rectangle(ChannelBuffer<T>& buffer, Geometry::Point2D<U> top_left, Geometry::Point2D<U> bottom_right, T value, Style style)
    {
        float value_float = static_cast<float>(value);
        bool fill = style == Style::filled ? true : false;

        auto buff = buffer.to_oiio();
        OIIO::ImageBufAlgo::render_box(
            buff,
            top_left.x_checked<int>(),
            top_left.y_checked<int>(),
            bottom_right.x_checked<int>(),
            bottom_right.y_checked<int>(),
            { value_float, value_float, value_float, value_float },
            fill
        );
    }


    /// Render text into the image buffer at the specified position.
    ///
    /// This function draws the given text at the specified `position` in the provided `ChannelBuffer`.
    /// The text is rendered using the specified font and size, with each pixel's color set to 
    /// the specified `pixel_value`. The function utilizes OpenImageIO's text rendering capabilities.
    ///
    /// \tparam T The type of pixel value stored in the `ChannelBuffer`.
    /// \tparam U The type of the coordinate values used in `Geometry::Point2D`.
    ///
    /// \param buffer The image buffer where the text will be rendered.
    /// \param position The position where the text will be drawn, as a `Geometry::Point2D` object.
    /// \param text The text string to be rendered.
    /// \param font_name The name of the font to use for rendering the text.
    ///                  If this isn't available we instead fall back to some sensible default.
    ///                  For more information please visit the OpenImageIO documentation
    /// \param pixel_value The pixel value to use for rendering the text. This value will be used
    ///                    to set the color of the text pixels.
    /// \param font_size The size of the font to use when rendering the text.
    template <typename T, typename U>
    void render_text(ChannelBuffer<T>& buffer, Geometry::Point2D<U> position, std::string_view text, std::string_view font_name, T pixel_value, size_t font_size)
    {
        // TODO: make this not rely on OIIO as the implementation seems to be pretty slow
        float value_float = static_cast<float>(pixel_value);

        auto buff = buffer.to_oiio();
        OIIO::ImageBufAlgo::render_text(
            buff,
            position.x_checked<int>(),
            position.y_checked<int>(),
            text,
            font_size,
            font_name,
            { value_float, value_float, value_float, value_float }
        );
    }


    /// Render a mesh into the image buffer at the specified position.
    ///
    /// This function draws the given mesh at the specified `position` in the provided `ChannelBuffer`.
    /// The mesh is rendered as a polygon grid for each of the segments optionally rendering point numbers
    /// on each of the points
    ///
    /// \tparam T The type of pixel value stored in the `ChannelBuffer`.
    /// \tparam U The type of the coordinate values used in `Geometry::QuadMesh`.
    ///
    /// \param buffer The image buffer where the mesh will be rendered.
    /// \param mesh The mesh which will be drawn, as a `Geometry::Mesh` object.
    /// \param value The intensity of the pixel value
    /// \param font_name The name of the font to use for rendering the text.
    ///                  If this isn't available we instead fall back to some sensible default.
    ///                  For more information please visit the OpenImageIO documentation.
    ///                  If render_pt_num is false this parameter has no effect
    /// \param render_pt_num Whether to render a point number
    template <typename T, typename U>
    void render_mesh(ChannelBuffer<T>& buffer, const Geometry::QuadMesh<U>& mesh, T value, std::string_view font_name = "", bool render_pt_num = false)
    {
        std::set<std::tuple<size_t, size_t>> rendered_edges;
        std::set<size_t> rendered_point_nums;

        for (const Geometry::Face<U, 4>& face : mesh.faces())
        {
            auto vertex_indices = face.vertex_indices();

            auto v0_idx = vertex_indices[0];    // top-left vertex
            auto v1_idx = vertex_indices[1];    // top-right vertex
            auto v2_idx = vertex_indices[2];    // bot-left vertex
            auto v3_idx = vertex_indices[3];    // bot-right vertex
            
            
            std::array<std::tuple<size_t, size_t>, 4> edges{
                std::make_tuple(v0_idx, v1_idx),
                std::make_tuple(v0_idx, v2_idx),
                std::make_tuple(v3_idx, v1_idx),
                std::make_tuple(v3_idx, v2_idx)
            };

            for (const auto& edge : edges)
            {
                // Render the point numbers (if requested)
                if (render_pt_num && !rendered_point_nums.contains(std::get<0>(edge)))
                {
                    auto idx = std::get<0>(edge);
                    auto point = mesh.vertex(idx).point();

                    auto str = std::to_string(idx);
                    render_text<T, U>(buffer, point, str, font_name, value, 50);
                    rendered_point_nums.insert(idx);
                }
                if (render_pt_num && !rendered_point_nums.contains(std::get<1>(edge)))
                {
                    auto idx = std::get<1>(edge);
                    auto point = mesh.vertex(idx).point();

                    auto str = std::to_string(idx);
                    render_text<T, U>(buffer, point, str, font_name, value, 50);
                    rendered_point_nums.insert(idx);
                }

                // Render the edges if it is not already rendered
                if (!rendered_edges.contains(edge))
                {
                    render_line<T, U>(
                        buffer,
                        mesh.vertex(std::get<0>(edge)).point(),
                        mesh.vertex(std::get<1>(edge)).point(),
                        value);

                    rendered_edges.insert(edge);
                }
            }
        }
    }


    /// Render a bezier surface with the given u and v intervals
    ///
    /// This function draws bezier surface with the intervals specified by u_intervals and v_intervals.
    /// Note that these lines that are drawn are not polygons but rather bezier curves. To get the actual mesh
    /// from a bezier you have to run `Geometry::BezierSurface::mesh()`
    ///
    /// \tparam T The type of pixel value stored in the `ChannelBuffer`.
    ///
    /// \param buffer The image buffer where the surface will be rendered.
    /// \param surface The surface which will be drawn, as a `Geometry::BezierSurface` object.
    /// \param value The intensity of the pixel value
    /// \param u_intervals Horizontal divisions
    /// \param v_intervals Vertical divisions
    template <typename T>
    void render_bezier_surface(ChannelBuffer<T>& buffer, const Geometry::BezierSurface& surface, T value, size_t u_intervals, size_t v_intervals)
    {
        // Calculate the step sizes for u and v based on the number of intervals
        float u_step = 1.0f / static_cast<float>(u_intervals);
        float v_step = 1.0f / static_cast<float>(v_intervals);

        // Iterate over the specified number of v intervals
        for (size_t v_index = 0; v_index <= v_intervals; ++v_index)
        {
            float v = v_index * v_step;
            for (size_t x = 0; x < buffer.width; ++x)
            {
                float u = static_cast<float>(x) / (buffer.width - 1);

                // Evaluate point on the surface
                auto biased_uv = surface.bias_uv(u, v);
                auto point = surface.evaluate(biased_uv.x, biased_uv.y);

                size_t px = static_cast<size_t>(std::round(point.x));
                size_t py = static_cast<size_t>(std::round(point.y));

                // Check if the point is within the pixel buffer bounds
                if (px < buffer.width && py < buffer.height)
                {
                    buffer.buffer[py * buffer.width + px] = value; // Set pixel for the horizontal line
                }
            }
        }

        // Iterate over the specified number of u intervals
        for (size_t u_index = 0; u_index <= u_intervals; ++u_index)
        {
            float u = u_index * u_step; 
            for (size_t y = 0; y < buffer.height; ++y)
            {
                float v = static_cast<float>(y) / (buffer.height - 1);

                // Evaluate point on the surface
                auto biased_uv = surface.bias_uv(u, v);
                auto point = surface.evaluate(biased_uv.x, biased_uv.y);

                size_t px = static_cast<size_t>(std::round(point.x));
                size_t py = static_cast<size_t>(std::round(point.y));

                // Check if the point is within the pixel buffer bounds
                if (px < buffer.width && py < buffer.height)
                {
                    buffer.buffer[py * buffer.width + px] = value; // Set pixel for the vertical line
                }
            }
        }
    }
}


PSAPI_NAMESPACE_END