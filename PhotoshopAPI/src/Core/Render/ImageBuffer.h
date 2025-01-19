#pragma once

#include "Macros.h"

#include <vector>
#include <cmath>
#include <cstdint>
#include <algorithm>


// If we compile with C++<20 we replace the stdlib implementation with the compatibility
// library
#if (__cplusplus < 202002L)
#include "tcb_span.hpp"
#else
#include <span>
#endif

#include "Core/Geometry/Point.h"
#include "Util/Enum.h"
#include "Interleave.h"

#include <fmt/format.h>
#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>
#include <OpenImageIO/half.h>

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

    /// ChannelBuffer for a single channel with encoded width and height. Can additionally be a const
    /// view if the is_const template parameter is set to true. Can be trivially passed as a copy as 
    /// it is a view over the data itself. This does however also mean that the lifetime of this buffer
    /// is tied to the data it was created from! So e.g. returning an ChannelBuffer from a temporary 
    /// object is unspecified
    template <typename T, bool is_const = false>
    struct ChannelBuffer
    {
        /// Specialize using a const span or regular span depending on the is_const template
        /// parameter
        using BufferType = std::conditional_t<is_const, std::span<const T>, std::span<T>>;

        using _DataType = std::conditional_t<is_const, std::vector<const T>, std::vector<T>>;

        /// The buffer associated with the data, this does not own the buffer
        BufferType buffer;

        /// An offset to apply to the ChannelBuffer, from the Canvas center. This is only used
        /// when we actually have a canvas such as in our compositing code. All other pixel access
        /// code is without this position
        Geometry::Point2D<int> position = { 0, 0 };

        /// The width of the held buffer
        size_t width = 0;

        /// The height of the held buffer
        size_t height = 0;

        ChannelBuffer(BufferType buffer_, size_t width_, size_t height_, int offset_x_ = 0, int offset_y_ = 0) :
            buffer(buffer_), width(width_), height(height_), position(Geometry::Point2D<int>(offset_x_, offset_y_))
        {
            if (this->buffer.size() != this->width * this->height)
            {
                PSAPI_LOG_ERROR("ChannelBuffer", "Unable to construct ChannelBuffer from given width and height as they do not match the buffers size");
            }
        }

        /// Compute the bounding box of the channel based on the document.
        Geometry::BoundingBox<int> bbox(size_t document_width, size_t document_height) const
        {
            // Get the canvas center as Point2D so we can offset the bbox by the positions
            auto center = Geometry::Point2D<int>(
                static_cast<int>(std::round(static_cast<double>(document_width) / 2)),
                static_cast<int>(std::round(static_cast<double>(document_height) / 2))
            );

            auto absolute_position = center;
            absolute_position += this->position;
            auto _bbox = Geometry::BoundingBox<int>(Geometry::Point2D<int>(0, 0), Geometry::Point2D<int>(this->width, this->height));
            _bbox.offset(absolute_position);

            return _bbox;
        }


        /// Convert this image buffer into an image buffer recognized by OpenImageIO. 
        /// Only available if the image buffer was constructed as non-const
        OIIO::ImageBuf to_oiio()
        {
            if constexpr (is_const)
            {
                PSAPI_LOG_ERROR("ChannelBuffer", "Unable to construct OpenImageIO ChannelBuffer from const image buffer");
            }

            if (this->width > std::numeric_limits<int>::max())
            {
                PSAPI_LOG_ERROR("ChannelBuffer", "Unable to construct OpenImageIO ChannelBuffer from ChannelBuffer struct as width would exceed numeric limit of int");
            }
            if (this->height > std::numeric_limits<int>::max())
            {
                PSAPI_LOG_ERROR("ChannelBuffer", "Unable to construct OpenImageIO ChannelBuffer from ChannelBuffer struct as height would exceed numeric limit of int");
            }

            auto spec = OIIO::ImageSpec(static_cast<int>(this->width), static_cast<int>(this->height), 1, get_type_desc<T>());
            auto buff = OIIO::ImageBuf(spec, this->buffer.data());
            return buff;
        }

        /// Access a pixel at a given x and y coordinate, does no bounds checking so the 
        /// coordinate must be within the buffer itself.
        T pixel(size_t x, size_t y)
        {
            return this->buffer[y * this->width + x];
        }

        size_t index(size_t x, size_t y)
        {
            return y * this->width + x;
        }

        /// Access a pixel at a given x and y coordinate, does no bounds checking so the 
        /// coordinate must be within the buffer itself.
        T pixel(Geometry::Point2D<size_t> pos)
        {
            return this->buffer[pos.y * this->width + pos.x];
        }

        size_t index(Geometry::Point2D<size_t> pos)
        {
            return pos.y * this->width + pos.x;
        }

        /// Rescale the buffer using neares neighbour interpolation for the given precision.
        /// 
        /// This function calculates the nearest neighbour sampled rescaled result of the buffer for a given width or height
        /// where the width and height do not necessarily have to be the same aspect ratio as the existing width and height.
        /// The function can be instantiated for half, float and double precision where the default is float.
        ///
        /// \tparam T The type of pixel value stored in the `ChannelBuffer`.
        /// \tparam _Precision The precision of computations to perform
        ///
        /// \param width The new width of the image
        /// \param height The new height of the image
        /// 
        /// \returns The rescaled image in the same bitdepth as the buffer
        template <typename _Precision = float, typename = std::enable_if_t<std::is_floating_point_v<_Precision> || std::is_same_v<_Precision, Imath::half>>>
        std::vector<T> rescale_nearest_neighbour(size_t width, size_t height)
        {
            std::vector<T> out(width * height);

            // Generate vertical iterator for the output resolution
            std::vector<size_t>vertical_iter(height);
            std::iota(vertical_iter.begin(), vertical_iter.end(), 0);

            std::for_each(vertical_iter.begin(), vertical_iter.end(), [&](size_t y)
                {
                    // orig_y and orig_x in this case stand for the coordinates in the 
                    // coordinate space of the original buffer rather than in our rescaled
                    // buffer
                    _Precision v = static_cast<_Precision>(y) / height;
                    size_t orig_y = static_cast<size_t>(std::round(v * this->height));

                    for (size_t x = 0; x < width; ++x)
                    {
                        _Precision u = static_cast<_Precision>(x) / width;
                        size_t orig_x = static_cast<size_t>(std::round(u * this->width));

                        // Interpolate along y-axis and return result
                        out[y * width + x] = this->buffer[orig_y * this->width + orig_x];
                    }
                });

            return out;
        }

        /// Rescale the buffer using bilinear interpolation for the given precision.
        /// 
        /// This function calculates the bilinearly sampled rescaled result of the buffer for a given width or height
        /// where the width and height do not necessarily have to be the same aspect ratio as the existing width and height.
        /// The function can be instantiated for half, float and double precision where the default is float.
        ///
        /// \tparam T The type of pixel value stored in the `ChannelBuffer`.
        /// \tparam _Precision The precision of computations to perform
        ///
        /// \param width The new width of the image
        /// \param height The new height of the image
        /// 
        /// \returns The rescaled image in the same bitdepth as the buffer
        template <typename _Precision = float, typename = std::enable_if_t<std::is_floating_point_v<_Precision> || std::is_same_v<_Precision, Imath::half>>>
        std::vector<T> rescale_bilinear(size_t width, size_t height)
        {
            std::vector<T> out(width * height);

            // Generate vertical iterator for the output resolution
            std::vector<size_t>vertical_iter(height);
            std::iota(vertical_iter.begin(), vertical_iter.end(), 0);

            std::for_each(vertical_iter.begin(), vertical_iter.end(), [&](size_t y)
                {
                    // orig_y and orig_x in this case stand for the coordinates in the 
                    // coordinate space of the original buffer rather than in our rescaled
                    // buffer
                    _Precision v = static_cast<_Precision>(y) / height;
                    _Precision orig_y = (v * this->height) - .5;
                    std::int64_t orig_y_int = static_cast<std::int64_t>(orig_y);
                    _Precision orig_y_fract = orig_y - std::floor(orig_y);

                    std::array<T, 2 * 2> matrix;
                    for (size_t x = 0; x < width; ++x)
                    {
                        _Precision u = static_cast<_Precision>(x) / width;
                        _Precision orig_x = (u * this->width) - .5;
                        std::int64_t orig_x_int = static_cast<std::int64_t>(orig_x);
                        _Precision orig_x_fract = orig_x - std::floor(orig_x);

                        ChannelBuffer<T>::get_matrix<2, 2>(matrix, *this, orig_x_int, orig_y_int);
                        // Interpolate along x-axis
                        _Precision top = matrix[0] + orig_x_fract * (matrix[1] - matrix[0]);
                        _Precision bot = matrix[2] + orig_x_fract * (matrix[3] - matrix[2]);

                        // Interpolate along y-axis
                        out[y * width + x] = static_cast<T>(top + orig_y_fract * (bot - top));
                    }
                });

            return out;
        }

        /// Rescale the buffer using bicubic interpolation for the given precision.
        /// 
        /// This function is adapted from: https://blog.demofox.org/2015/08/15/resizing-images-with-bicubic-interpolation/
        /// and is designed to calculate the bicubic rescaled result of the buffer for a given width or height
        /// where the width and height do not necessarily have to be the same aspect ratio as the existing width and height.
        /// The function can be instantiated for half, float and double precision where the default is float.
        ///
        /// \tparam T The type of pixel value stored in the `ChannelBuffer`.
        /// \tparam _Precision The precision of computations to perform
        ///
        /// \param width The new width of the image
        /// \param height The new height of the image
        /// \param min The min value to clamp the result to
        /// \param max The max value to clamp the result to
        /// 
        /// \returns The rescaled image in the same bitdepth as the buffer
        template <typename _Precision = float, typename = std::enable_if_t<std::is_floating_point_v<_Precision> || std::is_same_v<_Precision, Imath::half>>>
        std::vector<T> rescale_bicubic(size_t width, size_t height, T min, T max) const
        {
            // Generate output vector and vertical iterator for for_each
            std::vector<T> out(width * height);

            // Generate horizontal and vertical iterators for the output resolution
            std::vector<size_t>vertical_iter(height);
            std::vector<size_t>horizontal_iter(width);
            std::iota(vertical_iter.begin(), vertical_iter.end(), 0);
            std::iota(horizontal_iter.begin(), horizontal_iter.end(), 0);

            /// Precompute the horizontal x coordinates as their int and fractional components
            std::vector<std::int64_t> orig_x_int(width);
            std::vector<_Precision> orig_x_fract(width);
            std::for_each(horizontal_iter.begin(), horizontal_iter.end(), [&](size_t x)
                {
                    _Precision u = static_cast<_Precision>(x) / width;
                    _Precision orig_x = (u * this->width) - .5;
                    orig_x_int[x] = static_cast<std::int64_t>(orig_x);
                    orig_x_fract[x] = orig_x - std::floor(orig_x);
                });

            std::for_each(vertical_iter.begin(), vertical_iter.end(), [&](size_t y)
                {
                    // orig_y and orig_x in this case stand for the coordinates in the 
                    // coordinate space of the original buffer rather than in our rescaled
                    // buffer
                    _Precision v = static_cast<_Precision>(y) / height;
                    _Precision orig_y = (v * this->height) - .5;
                    std::int64_t orig_y_int = static_cast<std::int64_t>(orig_y);
                    _Precision orig_y_fract = orig_y - std::floor(orig_y);

                    std::array<T, 4 * 4> matrix;
                    for (size_t x = 0; x < width; ++x)
                    {
                        std::int64_t x_int = orig_x_int[x];
                        _Precision x_fract = orig_x_fract[x];

                        // Get the 4x4 pixel matrix and apply the cubic hermite first across
                        // the x dimension and then across the y dimension along the combined results
                        ChannelBuffer<T>::get_matrix<4, 4>(matrix, *this, x_int, orig_y_int);
                        _Precision col0 = cubic_hermite<_Precision>(matrix[0], matrix[1], matrix[2], matrix[3], x_fract);
                        _Precision col1 = cubic_hermite<_Precision>(matrix[4], matrix[5], matrix[6], matrix[7], x_fract);
                        _Precision col2 = cubic_hermite<_Precision>(matrix[8], matrix[9], matrix[10], matrix[11], x_fract);
                        _Precision col3 = cubic_hermite<_Precision>(matrix[12], matrix[13], matrix[14], matrix[15], x_fract);
                        _Precision value = cubic_hermite<_Precision>(col0, col1, col2, col3, orig_y_fract);

                        out[y * width + x] = std::clamp(static_cast<T>(value), min, max);
                    }
                });

            return out;
        }


        /// Bilinearly interpolate the pixel value at a given floating-point coordinate.
        /// 
        /// This function samples the pixel value at the specified floating-point coordinates in the 
        /// `ChannelBuffer`. If the coordinates are integers, it directly returns the pixel value at 
        /// that location. Otherwise, it performs bilinear interpolation using the four surrounding 
        /// pixel values to compute a smoother result.
        /// 
        /// \tparam T The type of pixel value stored in the `ChannelBuffer`.
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


        /// Bicubicly interpolate the pixel value at a given floating-point coordinate.
        /// 
        /// This function samples the pixel value at the specified floating-point coordinates in the 
        /// `ChannelBuffer`. If the coordinates are integers, it directly returns the pixel value at 
        /// that location. Otherwise, it performs bicubic interpolation using the 16 surrounding 
        /// pixel values to compute a smoother result.
        /// 
        /// \tparam T The type of pixel value stored in the `ChannelBuffer`.
        /// \tparam U The type of the coordinate values used in `Point2D`.
        ///
        /// \param point The `Point2D` object representing the floating-point coordinates for which 
        ///               the pixel value needs to be interpolated. If this is outside of the image buffer
        ///               the behaviour is undefined
        /// 
        /// \return The interpolated pixel value at the specified coordinates, either as a direct 
        ///         pixel value if the coordinates are integers or as a result of bicubic 
        ///         interpolation if the coordinates are floating-point values.
        template <typename U>
        T sample_bicubic(const Geometry::Point2D<U> point) const
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
                std::int64_t x_int = static_cast<std::int64_t>(point.x);
                std::int64_t y_int = static_cast<std::int64_t>(point.y);

                // Calculate fractional weights
                U dx = point.x - static_cast<U>(x_int);
                U dy = point.y - static_cast<U>(y_int);

                // Get the 4x4 pixel matrix and apply the cubic hermite first across
                // the x dimension and then across the y dimension along the combined results
                std::array<T, 4 * 4> matrix;
                ChannelBuffer<T, is_const>::get_matrix<4, 4, is_const>(matrix, *this, x_int, y_int);

                U col0  = cubic_hermite<U>(static_cast<U>(matrix[0]),  static_cast<U>(matrix[1]),  static_cast<U>(matrix[2]),  static_cast<U>(matrix[3]),  dx);
                U col1  = cubic_hermite<U>(static_cast<U>(matrix[4]),  static_cast<U>(matrix[5]),  static_cast<U>(matrix[6]),  static_cast<U>(matrix[7]),  dx);
                U col2  = cubic_hermite<U>(static_cast<U>(matrix[8]),  static_cast<U>(matrix[9]),  static_cast<U>(matrix[10]), static_cast<U>(matrix[11]), dx);
                U col3  = cubic_hermite<U>(static_cast<U>(matrix[12]), static_cast<U>(matrix[13]), static_cast<U>(matrix[14]), static_cast<U>(matrix[15]), dx);
                U value = cubic_hermite<U>(col0, col1, col2, col3, dy);

                value = std::clamp<U>(value, 0, std::numeric_limits<T>::max());
                return static_cast<T>(value);
            }
        }

        /// Bilinearly interpolate the pixel value based on normalized UV coordinates (0-1).
        /// 
        /// This function maps the provided UV coordinates, which range from (0,0) to (1,1), 
        /// to the corresponding pixel coordinates in the `ChannelBuffer`. It then calls 
        /// `sample_bilinear` to perform the interpolation based on the converted coordinates.
        /// 
        /// \tparam T The type of pixel value stored in the `ChannelBuffer`.
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
                PSAPI_LOG_ERROR("ChannelBuffer", "Unable to sample bilinear coordinate with non-floating point UV coordinate");
            }
            // Map UV coordinates to pixel coordinates
            U x = uv.x * static_cast<U>(width) - .5;
            U y = uv.y * static_cast<U>(height) - .5;
            return sample_bilinear(Geometry::Point2D<U>(x, y));
        }


        /// Bucibicly interpolate the pixel value based on normalized UV coordinates (0-1).
        /// 
        /// This function maps the provided UV coordinates, which range from (0,0) to (1,1), 
        /// to the corresponding pixel coordinates in the `ChannelBuffer`. It then calls 
        /// `sample_bicubic` to perform the interpolation based on the converted coordinates.
        /// 
        /// \tparam T The type of pixel value stored in the `ChannelBuffer`.
        /// \tparam U The type of the UV coordinate values used in `Point2D`.
        ///
        /// \param uv The `Point2D` object representing the normalized UV coordinates.
        /// 
        /// \return The interpolated pixel value at the specified UV coordinates.
        template <typename U>
        T sample_bicubic_uv(const Geometry::Point2D<U> uv) const
        {
            if constexpr (!std::is_floating_point_v<U>)
            {
                PSAPI_LOG_ERROR("ChannelBuffer", "Unable to sample bilinear coordinate with non-floating point UV coordinate");
            }
            // Map UV coordinates to pixel coordinates
            U x = uv.x * static_cast<U>(width)  - .5;
            U y = uv.y * static_cast<U>(height) - .5;
            return sample_bicubic(Geometry::Point2D<U>(x, y));
        }

        /// Retrieve a m*n submatrix of a given buffer at coordinate x and y. 
        ///
        /// Intended for e.g. access during a convolution where the surrounding pixels are required.
        /// Boundary conditions are handled by clamping to the edge. so if we try to sample the pixel 
        /// at 0, 0 we get the following matrix (shown by offsets)
        /// 
        /// 0 0 1
        /// 0 0 1
        /// 1 1 1
        /// 
        /// Similarly, the matrix is centered around the middle point but if we have dimensions which do not have
        /// a defined center point it is clamped to the left. E.g. with a 4x4 matrix the start point would be:
        /// 
        /// 0 0 0 0
        /// 0 x 0 0
        /// 0 0 0 0
        /// 0 0 0 0
        /// 
        /// \tparam _m The rows of the matrix to extract
        /// \tparam _n The columns of the matrix to extract
        /// 
        /// \param buffer The buffer to sample the matrix from
        /// \param x The x coordinate to sample at, gets clamped at boundaries
        /// \param y The y coordinate to sample at, gets clamped at boundaries
        template <size_t _m, size_t _n>
        static std::array<T, _m * _n> get_matrix(const ChannelBuffer<T>& buffer, std::int64_t x, std::int64_t y) noexcept
        {
            std::array<T, _m * _n> matrix;
            ChannelBuffer<T>::get_matrix<_m, _n>(matrix, buffer, x, y);
            return matrix;
        }


        /// Retrieve a m*n submatrix of a given buffer at coordinate x and y. 
        ///
        /// Intended for e.g. access during a convolution where the surrounding pixels are required.
        /// Boundary conditions are handled by clamping to the edge. so if we try to sample the pixel 
        /// at 0, 0 we get the following matrix (shown by offsets)
        /// 
        /// 0 0 1
        /// 0 0 1
        /// 1 1 1
        /// 
        /// Similarly, the matrix is centered around the middle point but if we have dimensions which do not have
        /// a defined center point it is clamped to the left. E.g. with a 4x4 matrix the start point would be:
        /// 
        /// 0 0 0 0
        /// 0 x 0 0
        /// 0 0 0 0
        /// 0 0 0 0
        /// 
        /// \tparam _m The rows of the matrix to extract
        /// \tparam _n The columns of the matrix to extract
        /// 
        /// \param matrix The matrix to populate
        /// \param buffer The buffer to sample the matrix from
        /// \param x The x coordinate to sample at, gets clamped at boundaries
        /// \param y The y coordinate to sample at, gets clamped at boundaries
        template <size_t _m, size_t _n, bool _is_const>
        static void get_matrix(std::array<T, _m * _n>& matrix, const ChannelBuffer<T, _is_const>& buffer, std::int64_t x, std::int64_t y)
        {
            static_assert(_m >= 2, "Must access a matrix with at least 2x2 dimensions");
            static_assert(_n >= 2, "Must access a matrix with at least 2x2 dimensions");

            constexpr size_t offset_x = (_m - 1) / 2;
            constexpr size_t offset_y = (_n - 1) / 2;

            for (size_t col = 0; col < _n; ++col)
            {
                for (size_t row = 0; row < _m; ++row)
                {
                    // Calculate the x and y coordinates for the current pixel
                    std::int64_t x_offset = x + row - offset_x;
                    std::int64_t y_offset = y + col - offset_y;

                    // Get the pixel and insert it into the matrix
                    matrix[col * _m + row] = ChannelBuffer<T, _is_const>::get_pixel(buffer.buffer, x_offset, y_offset, buffer.width, buffer.height);
                }
            }
        }

    private:

        /// Sample the cubic hermite defined by points A, B, C, D at position t in 1d space.
        /// These represent a contiguous, evenly spaced curve with the positions sample at t
        /// where t at 0 is point B and t at 1 is point C. For more information look here:
        /// https://dsp.stackexchange.com/questions/18265/bicubic-interpolation/18273#18273
        /// https://en.wikipedia.org/wiki/Hermite_polynomials
        /// 
        /// \tparam _Precision the precision of the function, accepts half, float and double 
        /// 
        /// \param A The leftmost control point, the slope from A to C is equivalent to the derivative of B
        /// \param B The middle-left control point, defines the leftmost bound of the value that is possible
        /// \param C The middle-right control point, defines the rightmost bound of the value that is possible
        /// \param D The rightmost control point, the slope from B to D is equivalent to the derivative of C
        /// \param t the interpolation value between B and C, from 0-1.
        template <typename _Precision = float, typename = std::enable_if_t<std::is_floating_point_v<_Precision> || std::is_same_v<_Precision, Imath::half>>>
        constexpr _Precision cubic_hermite(_Precision A, _Precision B, _Precision C, _Precision D, _Precision t) const
        {
            // Expanded forms of the hermite cubic polynomial,
            // adapted from https://blog.demofox.org/2015/08/08/cubic-hermite-interpolation/
            _Precision a = -A / 2.0f + (3.0f * B) / 2.0f - (3.0f * C) / 2.0f + D / 2.0f;
            _Precision b = A - (5.0f * B) / 2.0f + 2.0f * C - D / 2.0f;
            _Precision c = -A / 2.0f + C / 2.0f;
            _Precision d = B;

            return a*t*t*t + b*t*t + c*t + d;
        }

        /// Get a pixel at the given pixel coordinate in the source image
        /// clamping into the range 0 - (width-1) and 0 - (height-1) respectively
        static T get_pixel(const BufferType buffer, std::int64_t x, std::int64_t y, size_t width, size_t height)
        {
            x = std::clamp(x, static_cast<std::int64_t>(0), static_cast<std::int64_t>(width) - 1);
            y = std::clamp(y, static_cast<std::int64_t>(0), static_cast<std::int64_t>(height) - 1);
            auto idx = y * width + x;
            return buffer[idx];
        };


    };

    /// Utility typedef for a ChannelBuffer<T, true>
	template <typename T>
	using ConstChannelBuffer = ChannelBuffer<T, true>;


    /// Wrapper around ChannelBuffer for whole images with utility functions to e.g. write to disk.
    /// Similarly to ChannelBuffer the ImageBuffer does not manage the memory held by it, it is just a view over 
    /// it. Therefore constructing it with a temporary is undefined.
    template <typename T, bool is_const = false>
    struct ImageBuffer
    {
        using ChannelType = ChannelBuffer<T, is_const>;
        using _DataType = ChannelType::_DataType;
        using BufferType = ChannelType::BufferType;

        struct Metadata
        {
            /// Optional name for debug logging.
            std::string name = "";

            /// Default value of the mask (i.e. what the mask pixel value is outside of the mask bbox). If this is e.g. black then 
            /// anything outside of the masks' bbox can be discarded. If it is white the whole bbox of the channels must be considered.
            /// TODO: fill this value
            std::optional<uint8_t> mask_default_value;

            /// An optional position parameter offsetting the whole image by the given x and y offset.
            /// These offsets are from the canvas center, not from the top left of the canvas!
            /// Each sub-channel may additionally have an offset that is calculated on top of this.
            Geometry::Point2D<int> position = { 0, 0 };

            /// The global opacity multiplier of the imagebuffer. To be used for compositing
            float opacity = 1.0f;
        };

        /// The channels of the ImageBuffer mapped by their respective logical indices like e.g. 0, 1, 2 for R, G, B. 
        /// the -1 index is reserved for alpha channels. Masks are stored on `mask` as it is the only channel which may have a potentially different resolution
        std::unordered_map<int, ChannelType> channels;

        /// Optional mask channel, this is separate as in the context of photoshop this is the only channel that can have a different bounding box than the rest of the channels.
        /// We must account for this so that the two bounding boxes can be combined
        std::optional<ChannelType> mask;

        /// The width of the held buffers, all channels hold the same width.
        size_t width = 0;

        /// The height of the held buffers, all channels hold the same width.
        size_t height = 0;

        Metadata metadata;
        

        ImageBuffer(std::unordered_map<int, ChannelType> channels)
        {
            if (channels.contains(-2))
            {
                self.mask = channels[-2];
                channels.erase(-2);
            }

            this->channels = channels;
            for (auto [_index, _channel] : this->channels)
            {
                if (this->position.x != 0 && this->position.x != _channel.position.x)
                {
                    throw std::invalid_argument(fmt::format("Unable to construct ImageBuffer as channel {} does not match position x of other previous channels, expected {}", _channel.position.x, this->position.x));
                }
                if (this->position.y != 0 && this->position.y != _channel.position.y)
                {
                    throw std::invalid_argument(fmt::format("Unable to construct ImageBuffer as channel {} does not match position y of other previous channels, expected {}", _channel.position.y, this->position.y));
                }
                if (this->width != 0 && this->width != _channel.width)
                {
                    throw std::invalid_argument(fmt::format("Unable to construct ImageBuffer as channel {} does not match width of other previous channels, expected {}", _channel.width, this->width));
                }
                if (this->height != 0 && this->height != _channel.height)
                {
                    throw std::invalid_argument(fmt::format("Unable to construct ImageBuffer as channel {} does not match height of other previous channels, expected {}", _channel.height, this->height));
                }
                this->width = _channel.width;
                this->height = _channel.height;
            }
        }

        /// Does the ImageBuffer contain an alpha channel
        bool has_alpha() const noexcept{ return this->channels.contains(-1); }

        /// Return the alpha channel of the buffer, throws a std::out_of_range if the alpha channel does not exist
        /// One may use `has_alpha` to check if the alpha channel exists.
        ChannelType alpha() { return this->channels.at(-1); }

        /// Compute the alpha for all pixels of the alpha itself. Will throw exception if alpha or mask is not present.
        /// Internally, this will cache the computed result into _cached_alpha. If `has_mask()` returns false this method
        /// returns the 
        template <typename _Precision = float>
        requires std::is_floating_point_v<_Precision> || std::is_same_v<_Precision, Imath::half>;
        BufferType compute_alpha(size_t document_width, size_t document_height)
        {
            if (!this->mask)
            {
                return this->alpha().buffer
            }
            if (!this->_cached_alpha.empty())
            {
                return BufferType(this->_cached_alpha);
            }

            constexpr T max_t = std::numeric_limits<T>::max();
            if constexpr (std::is_same_v<T, float32_t>)
            {
                max_t = static_cast<float32_t>(1.0f);
            }

            // Set the alpha to either the alpha channel or the mask default value with the size of width * height
            std::vector<T> _alpha;
            if (this->has_alpha())
            {
                _alpha = std::vector<T>(this->alpha().buffer);
            }
            else
            {
                _alpha = std::vector<T>(this->width * this->height, this->mask_default_value.value_or())
            }
            
            // Compose the mask on top of the alpha if present.
            if (this->mask)
            {
                const auto _mask = this->mask.value().buffer;
                auto _alpha_bbox = this->bbox(document_width, document_height);
                auto _mask_bbox = this->mask.value().bbox(document_width, document_height);

                // create the intersection bbox and compute the min and max values
                auto roi_bbox = _bbox.intersect(_mask_bbox);
                size_t min_y = std::max<size_t>(static_cast<size_t>(std::round(roi_bbox.minimum.y)), 0);
                size_t max_y = std::min<size_t>(static_cast<size_t>(std::round(roi_bbox.maximum.y)), _alpha_bbox.height - 1);
                size_t min_x = std::max<size_t>(static_cast<size_t>(std::round(roi_bbox.minimum.x)), 0);
                size_t max_x = std::min<size_t>(static_cast<size_t>(std::round(roi_bbox.maximum.x)), _alpha_bbox.width - 1);

                auto vertical_iter = std::views::iota(min_y, max_y);
                auto horizontal_iter = std::views::iota(min_x, max_x);
                const auto alpha_width = this->alpha().width;

                std::for_each(std::execution::par_unseq, vertical_iter.begin(), vertical_iter.end(), [&](size_t y)
                    {
                        for (auto x : horizontal_iter)
                        {
                            Geometry::Point2D<size_t> point(x, y);
                            const Geometry::Point2D<size_t> point_alpha = point - _alpha_bbox.minimum;
                            const Geometry::Point2D<size_t> point_mask  = point - _mask_bbox.minimum;

                            T& alpha_value = _alpha[point_alpha.y * alpha_width + point_alpha.x];
                            const T mask_value = _mask.pixel(point_mask.x, point_mask.y);

                            // Combine alpha and mask values
                            alpha_value = static_cast<T>((static_cast<_Precision>(alpha_value) * mask_value) / max_t);
                        }
                    });
            }

            this->_cached_alpha = std::move(_alpha);
            return BufferType(this->_cached_alpha);
        }

        /// Clear the cached alpha, freeing the memory held by this struct.
        void clear_cached_alpha()
        {
            this->_cached_alpha = {};
        }

        /// Return the alpha channel as a std::optional.
        std::optional<ChannelType> alpha_optional() { return this->has_alpha() ? std::make_optional(this->alpha()) : std::nullopt; }

        constexpr int alpha_index() { return -1; }

        /// Does the ImageBuffer contain a mask channel
        bool has_mask() const noexcept { return this->mask.has_value(); }

        constexpr int mask_index() { return -2; }

        /// Compute the bounding box of the layer taking into account the mask (if present)
        Geometry::BoundingBox<int> bbox(size_t document_width, size_t document_height) const
        {
            // Get the canvas center as Point2D so we can offset the bbox by the positions
            auto center = Geometry::Point2D<int>(
                static_cast<int>(std::round(static_cast<double>(document_width) / 2)),
                static_cast<int>(std::round(static_cast<double>(document_height) / 2))
            );

            uint8_t mask_default = this->mask_default_value.value_or(255);
            std::optional<Geometry::BoundingBox<int>> mask_bbox;

            if (this->mask)
            {
                auto mask_width = this->mask.value().width;
                auto mask_height = this->mask.value().height;

                auto absolute_mask_position = center;
                absolute_mask_position += this->mask.value().position;

                auto _bbox = Geometry::BoundingBox<int>(Geometry::Point2D<int>(0, 0), Geometry::Point2D<int>(mask_width, mask_height));
                _bbox.offset(absolute_mask_position);
                mask_bbox.emplace(std::move(_bbox));
            }

            auto absolute_position = center;
            absolute_position += this->position;
            auto image_bbox = Geometry::BoundingBox<int>(Geometry::Point2D<int>(0, 0), Geometry::Point2D<int>(this->width, this->height));
            image_bbox.offset(absolute_position);

            // If the mask default is 0 (black), we can safely discard anything outside of the intersection of the two bounding boxes
            if (mask_default == 0 && mask_bbox)
            {
                return Geometry::BoundingBox<int>::intersect(image_bbox, mask_bbox.value());
            }

            // Otherwise the bbox is just the image bbox
            return image_bbox;
        }

        /// Total number of channels in the ImageBuffer
        size_t num_channels() { return this->channels.size(); }

        /// Write the ImageBuffer to the given filepath, currently we only support writing RGB/RGBA images where the channels are laid out as follows:
        /// 
        /// RGB: 0, 1, 2
        /// 
        /// RGBA: 0, 1, 2, -1
        /// 
        /// \param filepath The filepath to write the image to, needs to be a valid image file path recognized by OpenImageIO
        void write(std::filesystem::path filepath)
        {
            if (!filepath.has_filename())
            {
                throw std::invalid_argument(fmt::format("Unable to write to filepath {} as it does not have an file component", filepath.string()));
            }
            if (this->channels.size() != 3 && this->channels.size() != 4)
            {
                throw std::runtime_error(fmt::format("Unable to write image channels as we currently expect either 3 or 4 RGB/RGBA channels but instead the ImageBuffer stores {}.", this->channels.size()));
            }
            if (!this->channels.contains(0) || !this->channels.contains(1) || !this->channels.contains(2))
            {
                throw std::runtime_error("Unable to write image channels as we expect the channel indices 0, 1 and 2 to always be present for this operation.");
            }

            auto buffer_r = this->channels.at(0);
            auto buffer_g = this->channels.at(1);
            auto buffer_b = this->channels.at(2);
            auto buffer_a = this->alpha_optional();

            // Small utility function to convert a std::span<T> to a std::span<const T>
            auto to_const_span = [](std::span<T> span) 
                {
                    return std::span<const T>(span.begin(), span.end());
                };

            std::vector<std::span<const T>> channels;
            if constexpr (is_const)
            {
                channels = { buffer_r.buffer, buffer_g.buffer, buffer_b.buffer };
            }
            else
            {
                channels = { to_const_span(buffer_r.buffer), to_const_span(buffer_g.buffer), to_const_span(buffer_b.buffer) };
            }

            // Append the alpha buffer if provided
            if (buffer_a)
            {
                if constexpr (is_const)
                {
                    channels.push_back(buffer_a.value().buffer);
                }
                else
                {
                    channels.push_back(to_const_span(buffer_a.value().buffer));
                }
            }

            // Allocate the data for the interleaved buffer, and interleave the bytes together.
            auto interleaved = Render::interleave_alloc(channels);

            // Create OIIO primitives
            filepath.make_preferred();
            auto filepath_str = filepath.string();
            auto out = OIIO::ImageOutput::create(filepath_str);
            if (!out)
            {
                throw std::runtime_error(fmt::format("Unable to open output file {} for write", filepath_str));
            }
            auto type_desc = get_type_desc<T>();
            auto spec = OIIO::ImageSpec(this->width, this->height, channels.size(), type_desc);

            // Write file to disk
            out->open(filepath.string(), spec);
            out->write_image(type_desc, interleaved.data());
            out->close();
        }

    private:

        _DataType _cached_alpha;
    };


    /// Utility typedef for a ImageBuffer<T, true>
    template <typename T>
    using ConstImageBuffer = ImageBuffer<T, true>;
}

PSAPI_NAMESPACE_END