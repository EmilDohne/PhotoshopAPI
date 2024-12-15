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


        /// Rescale the buffer using neares neighbour interpolation for the given precision.
        /// 
        /// This function calculates the nearest neighbour sampled rescaled result of the buffer for a given width or height
        /// where the width and height do not necessarily have to be the same aspect ratio as the existing width and height.
        /// The function can be instantiated for half, float and double precision where the default is float.
        ///
        /// \tparam T The type of pixel value stored in the `ImageBuffer`.
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
        /// \tparam T The type of pixel value stored in the `ImageBuffer`.
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

                        ImageBuffer<T>::get_matrix<2, 2>(matrix, *this, orig_x_int, orig_y_int);
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
        /// \tparam T The type of pixel value stored in the `ImageBuffer`.
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
                        ImageBuffer<T>::get_matrix<4, 4>(matrix, *this, x_int, orig_y_int);
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


        /// Bicubicly interpolate the pixel value at a given floating-point coordinate.
        /// 
        /// This function samples the pixel value at the specified floating-point coordinates in the 
        /// `ImageBuffer`. If the coordinates are integers, it directly returns the pixel value at 
        /// that location. Otherwise, it performs bicubic interpolation using the 16 surrounding 
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
                ImageBuffer<T, is_const>::get_matrix<4, 4, is_const>(matrix, *this, x_int, y_int);

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
            U x = uv.x * static_cast<U>(width) - .5;
            U y = uv.y * static_cast<U>(height) - .5;
            return sample_bilinear(Geometry::Point2D<U>(x, y));
        }


        /// Bucibicly interpolate the pixel value based on normalized UV coordinates (0-1).
        /// 
        /// This function maps the provided UV coordinates, which range from (0,0) to (1,1), 
        /// to the corresponding pixel coordinates in the `ImageBuffer`. It then calls 
        /// `sample_bicubic` to perform the interpolation based on the converted coordinates.
        /// 
        /// \tparam T The type of pixel value stored in the `ImageBuffer`.
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
                PSAPI_LOG_ERROR("ImageBuffer", "Unable to sample bilinear coordinate with non-floating point UV coordinate");
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
        static std::array<T, _m * _n> get_matrix(const ImageBuffer<T>& buffer, std::int64_t x, std::int64_t y) noexcept
        {
            std::array<T, _m * _n> matrix;
            ImageBuffer<T>::get_matrix<_m, _n>(matrix, buffer, x, y);
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
        static void get_matrix(std::array<T, _m * _n>& matrix, const ImageBuffer<T, _is_const>& buffer, std::int64_t x, std::int64_t y)
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
                    matrix[col * _m + row] = ImageBuffer<T, _is_const>::get_pixel(buffer.buffer, x_offset, y_offset, buffer.width, buffer.height);
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


	template <typename T>
	using ConstImageBuffer = ImageBuffer<T, true>;
}

PSAPI_NAMESPACE_END