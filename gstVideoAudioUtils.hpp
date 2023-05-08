#pragma once
#include "imageFormat.h"
#include <strings.h>
#include <type_traits>

using uchar3 = struct uchar3
{
    unsigned char x, y, z;
};
using uchar4=struct uchar4
{
    unsigned char x, y, z, w;
};
using float3=struct float3
{
    unsigned char x, y, z;
};
using float4=struct float4
{
    unsigned char x, y, z, w;
};
// imageFormatDepth
inline size_t imageFormatDepth( imageFormat format )
{
    switch(format)
    {
        case imageFormat::IMAGE_RGB8:
        case imageFormat::IMAGE_BGR8:		return sizeof(uchar3) * 8;
        case imageFormat::IMAGE_RGBA8:
        case imageFormat::IMAGE_BGRA8:		return sizeof(uchar4) * 8;
        case imageFormat::IMAGE_RGB32F:
        case imageFormat::IMAGE_BGR32F:		return sizeof(float3) * 8;
        case imageFormat::IMAGE_RGBA32F:
        case imageFormat::IMAGE_BGRA32F:		return sizeof(float4) * 8;
        case imageFormat::IMAGE_GRAY8:		return sizeof(unsigned char) * 8;
        case imageFormat::IMAGE_GRAY32F:		return sizeof(float) * 8;
        case imageFormat::IMAGE_I420:
        case imageFormat::IMAGE_YV12:
        case imageFormat::IMAGE_NV12:		return 12;
        case imageFormat::IMAGE_UYVY:
        case imageFormat::IMAGE_YUYV:
        case imageFormat::IMAGE_YVYU:		return 16;
        case imageFormat::IMAGE_BAYER_BGGR:
        case imageFormat::IMAGE_BAYER_GBRG:
        case imageFormat::IMAGE_BAYER_GRBG:
        case imageFormat::IMAGE_BAYER_RGGB:	return sizeof(unsigned char) * 8;
    }

    return 0;
}

// imageFormatSize
size_t imageFormatSize( imageFormat format, size_t width, size_t height )
{
    return (width * height * imageFormatDepth(format)) / 8;
}

imageFormat gst_parse_format( GstStructure* caps )
{
    const char* format = gst_structure_get_string(caps, "format");

    if( !format )
        return imageFormat::IMAGE_UNKNOWN;

    if( strcasecmp(format, "rgb") == 0 )
        return imageFormat::IMAGE_RGB8;
    else if( strcasecmp(format, "yuy2") == 0 )
        return imageFormat::IMAGE_YUY2;
    else if( strcasecmp(format, "i420") == 0 )
        return imageFormat::IMAGE_I420;
    else if( strcasecmp(format, "nv12") == 0 )
        return imageFormat::IMAGE_NV12;
    else if( strcasecmp(format, "yv12") == 0 )
        return imageFormat::IMAGE_YV12;
    else if( strcasecmp(format, "yuyv") == 0 )
        return imageFormat::IMAGE_YUYV;
    else if( strcasecmp(format, "yvyu") == 0 )
        return imageFormat::IMAGE_YVYU;
    else if( strcasecmp(format, "uyvy") == 0 )
        return imageFormat::IMAGE_UYVY;
    else if( strcasecmp(format, "bggr") == 0 )
        return imageFormat::IMAGE_BAYER_BGGR;
    else if( strcasecmp(format, "gbrg") == 0 )
        return imageFormat::IMAGE_BAYER_GBRG;
    else if( strcasecmp(format, "grgb") == 0 )
        return imageFormat::IMAGE_BAYER_GRBG;
    else if( strcasecmp(format, "rggb") == 0 )
        return imageFormat::IMAGE_BAYER_RGGB;
    else if (strcasecmp(format,"rgba")==0){
        return imageFormat::IMAGE_RGBA8;
    }

    return imageFormat::IMAGE_UNKNOWN;
}

// imageFormatToStr
inline const char* imageFormatToStr( imageFormat format )
{
    switch(format)
    {
        case imageFormat::IMAGE_RGB8:	 	return "rgb8";
        case imageFormat::IMAGE_RGBA8:	 	return "rgba8";
        case imageFormat::IMAGE_RGB32F:	 	return "rgb32f";
        case imageFormat::IMAGE_RGBA32F:	 	return "rgba32f";
        case imageFormat::IMAGE_BGR8:	 	return "bgr8";
        case imageFormat::IMAGE_BGRA8:	 	return "bgra8";
        case imageFormat::IMAGE_BGR32F:	 	return "bgr32f";
        case imageFormat::IMAGE_BGRA32F:	 	return "bgra32f";
        case imageFormat::IMAGE_I420:	 	return "i420";
        case imageFormat::IMAGE_YV12:	 	return "yv12";
        case imageFormat::IMAGE_NV12:	 	return "nv12";
        case imageFormat::IMAGE_UYVY:	 	return "uyvy";
        case imageFormat::IMAGE_YUYV:	 	return "yuyv";
        case imageFormat::IMAGE_YVYU:		return "yvyu";
        case imageFormat::IMAGE_BAYER_BGGR:	return "bayer-bggr";
        case imageFormat::IMAGE_BAYER_GBRG:	return "bayer-gbrg";
        case imageFormat::IMAGE_BAYER_GRBG:	return "bayer-grbg";
        case imageFormat::IMAGE_BAYER_RGGB:	return "bayer-rggb";
        case imageFormat::IMAGE_GRAY8:	 	return "gray8";
        case imageFormat::IMAGE_GRAY32F:  	return "gray32f";
        case imageFormat::IMAGE_UNKNOWN: 	return "unknown";
    };

    return "unknown";
}
