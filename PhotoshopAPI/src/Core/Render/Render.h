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

	enum class Interpolation
	{
		nearest_neighbour,
		bilinear,
		bicubic,
	};

    /// Get a OpenImageIO TypeDesc based on the given template parameter returning OIIO::TypeDesc::Unknown
    /// if the image coordinate is not part of the valid template specializations for photoshop buffers
    template <typename T>
    constexpr OIIO::TypeDesc get_type_desc()
    {
        if constexpr (std::is_same_v<T, bpp8_t>)
        {
            return OIIO::TypeDesc::UINT8;
        }
        else if constexpr (std::is_same_v<T, bpp16_t>)
        {
            return OIIO::TypeDesc::UINT16;
        }
        if constexpr (std::is_same_v<T, bpp32_t>)
        {
            return OIIO::TypeDesc::FLOAT;
        }
        return OIIO::TypeDesc::UNKNOWN;
    }

    /// ImageBuffer for a single channel with encoded width and height. Can additionally be a const
    /// view if the is_const template parameter is set to true. Can be trivially passed as a copy as 
    /// it is a view over the data itself. This does however also mean that the lifetime of this buffer
    /// is tied to the data it was created from! So e.g. returning an ImageBuffer from a temporary 
    /// object is unspecified
    template <typename T, bool is_const = false>
    struct ImageBuffer
    {
        /// Specialize using a const span or regular span depending on the is_const template
        /// parameter
        using BufferType = std::conditional_t<is_const, std::span<const T>, std::span<T>>;

        /// The buffer associated with the data, this does not own the buffer
        BufferType buffer;

        /// The width of the held buffer
        size_t width = 0;

        /// The height of the held buffer
        size_t height = 0;

        ImageBuffer(BufferType buffer_, size_t width_, size_t height_) :
            buffer(buffer_), width(width_), height(height_)
        {
            if (this->buffer.size() != this->width * this->height)
            {
                PSAPI_LOG_ERROR("ImageBuffer", "Unable to construct ImageBuffer from given width and height as they do not match the buffers size");
            }
        }


        /// Convert this image buffer into an image buffer recognized by OpenImageIO. 
        /// Only available if the image buffer was constructed as non-const
        OIIO::ImageBuf to_oiio()
        {
            if constexpr (is_const)
            {
                PSAPI_LOG_ERROR("ImageBuffer", "Unable to construct OpenImageIO imagebuffer from const image buffer");
            }

            if (this->width > std::numeric_limits<int>::max())
            {
                PSAPI_LOG_ERROR("ImageBuffer", "Unable to construct OpenImageIO imagebuffer from ImageBuffer struct as width would exceed numeric limit of int");
            }
            if (this->height > std::numeric_limits<int>::max())
            {
                PSAPI_LOG_ERROR("ImageBuffer", "Unable to construct OpenImageIO imagebuffer from ImageBuffer struct as height would exceed numeric limit of int");
            }

            auto spec = OIIO::ImageSpec(static_cast<int>(this->width), static_cast<int>(this->height), 1, get_type_desc<T>());
            auto buff = OIIO::ImageBuf(spec, this->buffer.data());
            return buff;
        }


        /// Bilinearly interpolate the pixel value at a given floating-point coordinate.
        /// 
        /// This function samples the pixel value at the specified floating-point coordinates in the 
        /// `ImageBuffer`. If the coordinates are integers, it directly returns the pixel value at 
        /// that location. Otherwise, it performs bilinear interpolation using the four surrounding 
        /// pixel values to compute a smoother result.
        /// 
        /// \tparam T The type of pixel value stored in the `ImageBuffer`.
        /// \tparam U The type of the coordinate values used in `Point2D`.
        ///
        /// \param point The `Point2D` object representing the floating-point coordinates for which 
        ///               the pixel value needs to be interpolated. If this is outside of the image buffer
        ///               the behaviour is undefined
        /// 
        /// \return The interpolated pixel value at the specified coordinates, either as a direct 
        ///         pixel value if the coordinates are integers or as a result of bilinear 
        ///         interpolation if the coordinates are floating-point values.
        template <typename U>
        T sample_bilinear(const Geometry::Point2D<U> point) const
        {
            // If coordinates are integers, return the pixel directly
            if constexpr (!std::is_floating_point_v<U>)
            {
                size_t x = static_cast<size_t>(point.x);
                size_t y = static_cast<size_t>(point.y);
                return buffer[y * this->width + x];
            }
            else
            {
                // Get integer positions around the point
                size_t x0 = static_cast<size_t>(std::fmax(std::floor(point.x), 0.0f));
                size_t y0 = static_cast<size_t>(std::fmax(std::floor(point.y), 0.0f));
                size_t x1 = std::min(x0 + 1, width - 1);
                size_t y1 = std::min(y0 + 1, height - 1);

                // Calculate weights
                U dx = point.x - static_cast<U>(x0);
                U dy = point.y - static_cast<U>(y0);

                // Sample four surrounding pixels
                T v00 = buffer[y0 * width + x0];
                T v01 = buffer[y0 * width + x1];
                T v10 = buffer[y1 * width + x0];
                T v11 = buffer[y1 * width + x1];

                // Interpolate along x-axis
                T top = v00 + dx * (v01 - v00);
                T bottom = v10 + dx * (v11 - v10);

                // Interpolate along y-axis and return result
                return top + dy * (bottom - top);
            }
        }

        /// Bilinearly interpolate the pixel value at a given coordinate with bounds checking.
        /// 
        /// This function first checks that the provided `point` coordinates are within the bounds
        /// of the image buffer. If they are valid, it calls `sample_bilinear` to perform the
        /// interpolation. If the coordinates are out of bounds, it returns a default pixel value
        /// (e.g., black or transparent).
        ///
        /// \tparam T The type of pixel value stored in the `ImageBuffer`.
        /// \tparam U The type of the coordinate values used in `Point2D`.
        ///
        /// \param point The `Point2D` object representing the floating-point coordinates for which 
        ///               the pixel value needs to be interpolated.
        /// 
        /// \return The interpolated pixel value at the specified coordinates or a default pixel 
        ///         value if the coordinates are out of bounds.
        template <typename U>
        T sample_bilinear_checked(const Geometry::Point2D<U> point) const
        {
            // Check bounds
            if (point.x < 0 || static_cast<T>(point.x) >= width || point.y < 0 || static_cast<T>(point.y) >= height)
            {
                return T{}; 
            }

            // Call the main bilinear sample function
            return sample_bilinear(point);
        }

        /// Bilinearly interpolate the pixel value based on normalized UV coordinates (0-1).
        /// 
        /// This function maps the provided UV coordinates, which range from (0,0) to (1,1), 
        /// to the corresponding pixel coordinates in the `ImageBuffer`. It then calls 
        /// `sample_bilinear` to perform the interpolation based on the converted coordinates.
        /// 
        /// \tparam T The type of pixel value stored in the `ImageBuffer`.
        /// \tparam U The type of the UV coordinate values used in `Point2D`.
        ///
        /// \param uv The `Point2D` object representing the normalized UV coordinates.
        /// 
        /// \return The interpolated pixel value at the specified UV coordinates.
        template <typename U>
        T sample_bilinear_uv(const Geometry::Point2D<U> uv) const
        {
            if constexpr (!std::is_floating_point_v<U>)
            {
                PSAPI_LOG_ERROR("ImageBuffer", "Unable to sample bilinear coordinate with non-floating point UV coordinate");
            }
            // Map UV coordinates to pixel coordinates
            U x = uv.x * static_cast<U>(width - 1);
            U y = uv.y * static_cast<U>(height - 1);
            return sample_bilinear(Geometry::Point2D<U>(x, y));
        }

        /// Bilinearly interpolate the pixel value based on normalized UV coordinates with bounds checking.
        /// 
        /// This function first checks that the provided UV coordinates are within the valid range
        /// of (0,0) to (1,1). If they are valid, it calls `sample_bilinear_uv` to perform the
        /// interpolation. If the UV coordinates are out of bounds, it returns a default pixel value
        /// (e.g., black or transparent).
        ///
        /// \tparam T The type of pixel value stored in the `ImageBuffer`.
        /// \tparam U The type of the UV coordinate values used in `Point2D`.
        ///
        /// \param uv The `Point2D` object representing the normalized UV coordinates.
        /// 
        /// \return The interpolated pixel value at the specified UV coordinates or a default pixel 
        ///         value if the coordinates are out of bounds.
        template <typename U>
        T sample_bilinear_uv_checked(const Geometry::Point2D<U> uv) const
        {
            if constexpr (!std::is_floating_point_v<U>)
            {
                PSAPI_LOG_ERROR("ImageBuffer", "Unable to sample bilinear coordinate with non-floating point UV coordinate");
            }

            // Check bounds for UV coordinates
            if (uv.x < static_cast<U>(0.0f) || uv.x > static_cast<U>(1.0f) || uv.y < static_cast<U>(0.0f) || uv.y > static_cast<U>(1.0f))
            {
                return T{}; 
            }

            // Call the main bilinear UV sample function
            return sample_bilinear_uv(uv);
        }


        std::vector<T> rescale_bicubic(size_t width, size_t height, T min, T max)
        {

            auto get_pixel = [&](size_t x, size_t y)
                {
                    x = std::clamp(x, 0, width - 1);
                    y = std::clamp(y, 0, height - 1);
                    auto idx = y * width + x;
                    return this->buffer[idx];
                }


            std::vector<T> out(width * height);

            std::vector<size_t>vertical_iter(height);
            std::iota(vertical_iter.begin(), vertical_iter.end(), 0);

            std::for_each(vertical_iter.begin(), vertical_iter.end(), [&](size_t y)
                {
                    float v = static_cast<float>(y) / height;

                    for (size_t x = 0; x < width; ++x)
                    {
                    }
                })
        }

    private:

		float cubic_hermite(float A, float B, float C, float D, float t)
		{
            // Expanded forms of the hermite cubic polynomial,
            // adapted from 
			float a = -A / 2.0f + (3.0f * B) / 2.0f - (3.0f * C) / 2.0f + D / 2.0f;
			float b = A - (5.0f * B) / 2.0f + 2.0f * C - D / 2.0f;
			float c = -A / 2.0f + C / 2.0f;
			float d = B;

			return a * t * t * t + b * t * t + c * t + d;
		}

    };


    /// Rendering style enum
    enum class Style
    {
        outline,
        filled
    };


    /// Render a line to the image buffer from pt1 to pt2 with the specified pixel value.
    ///
    /// This function draws a line between two points, `pt1` and `pt2`, in the provided `ImageBuffer`.
    /// The pixel values along the line are set to the specified `value`. This implementation utilizes
    /// Wu's algorithm to achieve anti-aliasing for smoother line rendering.
    ///
    /// \tparam T The type of pixel value stored in the `ImageBuffer`.
    /// \tparam U The type of the coordinate values used in `Geometry::Point2D`.
    ///
    /// \param buffer The image buffer where the line will be rendered.
    /// \param pt1 The starting point of the line as a `Geometry::Point2D` object.
    /// \param pt2 The ending point of the line as a `Geometry::Point2D` object.
    /// \param value The pixel value to use for rendering the line. This value will be used to set 
    ///              the color of the pixels along the line.
    template <typename T, typename U>
    void render_line(ImageBuffer<T> buffer, Geometry::Point2D<U> pt1, Geometry::Point2D<U> pt2, T value)
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
    /// \tparam T The type of pixel value stored in the `ImageBuffer`.
    /// \tparam U The type of the coordinate values used in `Geometry::Point2D`.
    ///
    /// \param buffer The image buffer where the circle will be rendered.
    /// \param center The center point of the circle as a `Geometry::Point2D` object.
    /// \param radius The radius of the circle to be rendered.
    /// \param value The pixel value to use for rendering the circle.
    /// \param style The style of the circle rendering, which can be either filled or outlined.
    template <typename T, typename U>
    void render_circle(ImageBuffer<T>& buffer, Geometry::Point2D<U> center, size_t radius, T value, Style style)
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
    /// in the provided `ImageBuffer`. The rectangle can be either filled or outlined based on 
    /// the specified `Style`. The value used to color the rectangle is passed as a template parameter.
    ///
    /// \tparam T The type of pixel value stored in the `ImageBuffer`.
    /// \tparam U The type of the coordinate values used in `Geometry::Point2D`. Internally converts to int
    ///
    /// \param buffer The image buffer where the rectangle will be rendered.
    /// \param top_left The top-left corner of the rectangle as a `Geometry::Point2D` object.
    /// \param bottom_right The bottom-right corner of the rectangle as a `Geometry::Point2D` object.
    /// \param value The pixel value to use for rendering the rectangle. This value will be converted 
    ///              to float for rendering purposes.
    /// \param style The style of the rectangle rendering, which can be either filled or outlined.
    template <typename T, typename U>
    void render_rectangle(ImageBuffer<T>& buffer, Geometry::Point2D<U> top_left, Geometry::Point2D<U> bottom_right, T value, Style style)
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
    /// This function draws the given text at the specified `position` in the provided `ImageBuffer`.
    /// The text is rendered using the specified font and size, with each pixel's color set to 
    /// the specified `pixel_value`. The function utilizes OpenImageIO's text rendering capabilities.
    ///
    /// \tparam T The type of pixel value stored in the `ImageBuffer`.
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
    void render_text(ImageBuffer<T>& buffer, Geometry::Point2D<U> position, std::string_view text, std::string_view font_name, T pixel_value, size_t font_size)
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
    /// This function draws the given mesh at the specified `position` in the provided `ImageBuffer`.
    /// The mesh is rendered as a polygon grid for each of the segments optionally rendering point numbers
    /// on each of the points
    ///
    /// \tparam T The type of pixel value stored in the `ImageBuffer`.
    /// \tparam U The type of the coordinate values used in `Geometry::Mesh`.
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
    void render_mesh(ImageBuffer<T>& buffer, const Geometry::Mesh<U>& mesh, T value, std::string_view font_name = "", bool render_pt_num = false)
    {
        if (!render_pt_num)
        {
            for (const auto& line : mesh.half_edges())
            {
                render_line<T, U>(
                    buffer, 
                    line.vertex<U>(mesh).point(), 
                    line.pointed_at<U>(mesh).point(), 
                    value);
            }
        }
        else
        {
            std::set<size_t> rendered_point_nums;
            for (const auto& line : mesh.half_edges())
            {
                auto point1 = line.vertex<U>(mesh).point();
                auto point1_idx = line.vertex_idx();
                auto point2 = line.pointed_at<U>(mesh).point();
                auto point2_idx = line.pointed_at_idx();

                if (!rendered_point_nums.contains(point1_idx))
                {
                    auto str = std::to_string(point1_idx);
                    render_text<T, U>(buffer, point1, str, font_name, value, 50);
                    rendered_point_nums.insert(point1_idx);
                }

                if (!rendered_point_nums.contains(point2_idx))
                {
                    auto str = std::to_string(point2_idx);
                    render_text<T, U>(buffer, point2, str, font_name, value, 50);
                    rendered_point_nums.insert(point2_idx);
                }

                render_line<T, U>(buffer, point1, point2, value);
            }
        }
    }


    /// Render a bezier surface with the given u and v intervals
    ///
    /// This function draws bezier surface with the intervals specified by u_intervals and v_intervals.
    /// Note that these lines that are drawn are not polygons but rather bezier curves. To get the actual mesh
    /// from a bezier you have to run `Geometry::BezierSurface::mesh()`
    ///
    /// \tparam T The type of pixel value stored in the `ImageBuffer`.
    ///
    /// \param buffer The image buffer where the surface will be rendered.
    /// \param surface The surface which will be drawn, as a `Geometry::BezierSurface` object.
    /// \param value The intensity of the pixel value
    /// \param u_intervals Horizontal divisions
    /// \param v_intervals Vertical divisions
    template <typename T>
    void render_bezier_surface(ImageBuffer<T>& buffer, const Geometry::BezierSurface& surface, T value, size_t u_intervals, size_t v_intervals)
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
                auto point = surface.evaluate(u, v);
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
                auto point = surface.evaluate(u, v);
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