#pragma once
#include "URI.h"
enum Codec
{
    CODEC_UNKNOWN = 0,		/**< Unknown/unsupported codec */
    CODEC_RAW,			/**< Uncompressed (e.g. RGB) */
    CODEC_H264,			/**< H.264 */
    CODEC_H265,			/**< H.265 */
    CODEC_VP8,			/**< VP8 */
    CODEC_VP9,			/**< VP9 */
    CODEC_MPEG2,			/**< MPEG2 (decode only) */
    CODEC_MPEG4,			/**< MPEG4 (decode only) */
    CODEC_MJPEG			/**< MJPEG */
};
class VideoAudioOption{
public:
    int height;
    int width;



    URI resource;

    /**
     * The framerate of the stream (the default is 30Hz).
     * This option can be set from the command line using `--input-rate=N` or `--output-rate=N`
     * for input and output streams, respectively. The `--framerate=N` option sets it for both.
     */
    float frameRate;

    /**
     * The number of frames that have been captured or output on this interface.
     */
    uint64_t frameCount;

    /**
     * The encoding bitrate for compressed streams (only applies to video codecs like H264/H265).
     * For videoOutput streams, this option can be set from the command line using `--bitrate=N`.
     * @note the default bitrate for encoding output streams is 4Mbps (target VBR).
     */
    uint32_t bitRate;

    /**
     * The number of ring buffers used for threading.
     * This option can be set from the command line using `--num-buffers=N`.
     * @note the default number of ring buffers is 4.
     */
    uint32_t numBuffers;

    /**
     * If true, indicates the buffers are allocated in zeroCopy memory that is mapped to
     * both the CPU and GPU.  Otherwise, the buffers are only accessible from the GPU.
     * @note the default is true (zeroCopy CPU/GPU access enabled).
     */
    bool zeroCopy;


    Codec codec;
    // CodecToStr
    static const char* CodecToStr(Codec codec )
    {
        switch(codec)
        {
            case CODEC_UNKNOWN:	return "unknown";
            case CODEC_RAW:	return "raw";
            case CODEC_H264:	return "H264";
            case CODEC_H265:	return "H265";
            case CODEC_VP8:	return "VP8";
            case CODEC_VP9:	return "VP9";
            case CODEC_MPEG2:	return "MPEG2";
            case CODEC_MPEG4:	return "MPEG4";
            case CODEC_MJPEG:	return "MJPEG";
        }

        return nullptr;
    }

};
