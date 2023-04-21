#pragma once
enum class imageFormat
{
    // RGB
    IMAGE_RGB8=0,					/**< uchar3 RGB8    (`'rgb8'`) */
    IMAGE_RGBA8,					/**< uchar4 RGBA8   (`'rgba8'`) */
    IMAGE_RGB32F,					/**< float3 RGB32F  (`'rgb32f'`) */
    IMAGE_RGBA32F,					/**< float4 RGBA32F (`'rgba32f'`) */

    // BGR
    IMAGE_BGR8,					/**< uchar3 BGR8    (`'bgr8'`) */
    IMAGE_BGRA8,					/**< uchar4 BGRA8   (`'bgra8'`) */
    IMAGE_BGR32F,					/**< float3 BGR32F  (`'bgr32f'`) */
    IMAGE_BGRA32F,					/**< float4 BGRA32F (`'bgra32f'`) */

    // YUV
    IMAGE_YUYV,					/**< YUV YUYV 4:2:2 packed (`'yuyv'`) */
    IMAGE_YUY2=IMAGE_YUYV,			/**< Duplicate of YUYV     (`'yuy2'`) */
    IMAGE_YVYU,					/**< YUV YVYU 4:2:2 packed (`'yvyu'`) */
    IMAGE_UYVY,					/**< YUV UYVY 4:2:2 packed (`'uyvy'`) */
    IMAGE_I420,					/**< YUV I420 4:2:0 planar (`'i420'`) */
    IMAGE_YV12,					/**< YUV YV12 4:2:0 planar (`'yv12'`) */
    IMAGE_NV12,					/**< YUV NV12 4:2:0 planar (`'nv12'`) */

    // Bayer
    IMAGE_BAYER_BGGR,				/**< 8-bit Bayer BGGR (`'bayer-bggr'`) */
    IMAGE_BAYER_GBRG,				/**< 8-bit Bayer GBRG (`'bayer-gbrg'`) */
    IMAGE_BAYER_GRBG,				/**< 8-bit Bayer GRBG (`'bayer-grbg'`) */
    IMAGE_BAYER_RGGB,				/**< 8-bit Bayer RGGB (`'bayer-rggb'`) */

    // grayscale
    IMAGE_GRAY8,					/**< uint8 grayscale  (`'gray8'`)   */
    IMAGE_GRAY32F,					/**< float grayscale  (`'gray32f'`) */

    // extras
    IMAGE_COUNT,					/**< The number of image formats */
    IMAGE_UNKNOWN=999,				/**< Unknown/undefined format */
    IMAGE_DEFAULT=IMAGE_RGBA32F		/**< Default format (IMAGE_RGBA32F) */
};
