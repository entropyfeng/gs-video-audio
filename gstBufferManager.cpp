#include "gstBufferManager.h"
#include "gstVideoAudioUtils.hpp"
#include "timespec.h"
#include "memory_resources.h"
int gstBufferManager::Dequeue(void **output, imageFormat format, uint64_t timeout)
{
    // wait until a new frame is recieved
    if( !mWaitEvent.Wait(timeout) )
        return 0;

    void* latestYUV = nullptr;


    // handle the CPU path (non-NVMM)
    if( !mNvmmUsed )
        latestYUV = mBufferYUV.Next(RingBuffer::ReadLatestOnce);

    if( !latestYUV )
        return -1;

    // handle timestamp (both paths)
    void* pLastTimestamp = nullptr;
    pLastTimestamp = mTimestamps.Next(RingBuffer::ReadLatestOnce);

    if( !pLastTimestamp )
    {
        spdlog::warn("gstBufferManager -- failed to retrieve timestamp buffer (default to 0)");
        mLastTimestamp = 0;
    }
    else
    {
        mLastTimestamp = *((uint64_t*)pLastTimestamp);
    }

    // output raw image if conversion format is unknown
    if ( format == imageFormat::IMAGE_UNKNOWN )
    {
        *output = latestYUV;
        return 1;
    }

    // allocate ringbuffer for colorspace conversion
    const size_t rgbBufferSize = imageFormatSize(format, mOptions->width, mOptions->height);

    if( !mBufferRGB.Alloc(mOptions->numBuffers, rgbBufferSize, mOptions->zeroCopy ? RingBuffer::ZeroCopy : 0) )
    {
        spdlog::error("gstBufferManager -- failed to allocate {} buffers ({} bytes each)", mOptions->numBuffers, rgbBufferSize);
        return -1;
    }

    // perform colorspace conversion
    void* nextRGB = mBufferRGB.Next(RingBuffer::Write);

    if( CUDA_FAILED(cudaConvertColor(latestYUV, mFormatYUV, nextRGB, format, mOptions->width, mOptions->height)) )
    {
        LogError(LOG_GSTREAMER "gstBufferManager -- unsupported image format (%s)\n", imageFormatToStr(format));
        LogError(LOG_GSTREAMER "                    supported formats are:\n");
        LogError(LOG_GSTREAMER "                       * rgb8\n");
        LogError(LOG_GSTREAMER "                       * rgba8\n");
        LogError(LOG_GSTREAMER "                       * rgb32f\n");
        LogError(LOG_GSTREAMER "                       * rgba32f\n");

        return -1;
    }

    *output = nextRGB;
    return 1;
}

bool gstBufferManager::Enqueue(GstBuffer *gstBuffer, GstCaps *gstCaps){


    if(!gstBuffer || !gstCaps ){
        return false;
    }

    uint64_t timestamp = apptime_nano();

    // map the buffer memory for read access
    GstMapInfo map;

    if( !gst_buffer_map(gstBuffer, &map, GST_MAP_READ) )
    {
        spdlog::error("gstBufferManager -- failed to map gstreamer buffer memory");
        return false;
    }

    const void* gstData = map.data;
    const gsize gstSize = map.maxsize; //map.size;

    if( !gstData )
    {
        spdlog::error("gstBufferManager -- gst_buffer_map had NULL data pointer...");
        return false;
    }

    if( map.maxsize > map.size && mFrameCount == 0 )
    {
        spdlog::warn("gstBufferManager -- map buffer size was less than max size ({} vs {})", map.size, map.maxsize);
    }

    // on the first frame, print out the recieve caps
    if( mFrameCount == 0 ){
        spdlog::info("gstBufferManager recieve caps:  {}", gst_caps_to_string(gstCaps));

    }


    // retrieve caps structure
    GstStructure* gstCapsStruct = gst_caps_get_structure(gstCaps, 0);

    if( !gstCapsStruct )
    {
        spdlog::error("gstBufferManager -- gst_caps had NULL structure...");
        return false;
    }

    // retrieve the width and height of the buffer
    int width  = 0;
    int height = 0;

    if( !gst_structure_get_int(gstCapsStruct, "width", &width) ||
        !gst_structure_get_int(gstCapsStruct, "height", &height) )
    {
        spdlog::error("gstBufferManager -- gst_caps missing width/height...");
        return false;
    }

    if( width < 1 || height < 1 )
        return false;

    mOptions->width = width;
    mOptions->height = height;

    // verify format
    if( mFrameCount == 0 )
    {
        mFormatYUV = gst_parse_format(gstCapsStruct);

        if( mFormatYUV == imageFormat::IMAGE_UNKNOWN )
        {
            spdlog::error("gstBufferManager -- stream {} does not have a compatible decoded format", mOptions->resource);
            return false;
        }
        spdlog::info("gstBufferManager -- recieved first frame, codec={} format={} width={} height={} size={}",VideoAudioOption::CodecToStr(mOptions->codec), imageFormatToStr(mFormatYUV), mOptions->width, mOptions->height, gstSize);

    }

    //LogDebug(LOG_GSTREAMER "gstBufferManager -- recieved %ix%i frame (%zu bytes)\n", width, height, gstSize);


    // handle CPU path (non-NVMM)
    if( !mNvmmUsed )
    {
        // allocate image ringbuffer
        if( !mBufferYUV.Alloc(mOptions->numBuffers, gstSize, RingBuffer::ZeroCopy) )
        {
            LogError(LOG_GSTREAMER "gstBufferManager -- failed to allocate %u image buffers (%zu bytes each)\n", mOptions->numBuffers, gstSize);
            return false;
        }

        // copy to next image ringbuffer
        void* nextBuffer = mBufferYUV.Peek(RingBuffer::Write);

        if( !nextBuffer )
        {
            LogError(LOG_GSTREAMER "gstBufferManager -- failed to retrieve next image ringbuffer for writing\n");
            return false;
        }

        memcpy(nextBuffer, gstData, gstSize);
        mBufferYUV.Next(RingBuffer::Write);
    }

    // handle timestamps in either case (CPU or NVMM path)
    size_t timestamp_size = sizeof(uint64_t);

    // allocate timestamp ringbuffer (GPU only if not ZeroCopy)
    if( !mTimestamps.Alloc(mOptions->numBuffers, timestamp_size, RingBuffer::ZeroCopy) )
    {
        LogError(LOG_GSTREAMER "gstBufferManager -- failed to allocate %u timestamp buffers (%zu bytes each)\n", mOptions->numBuffers, timestamp_size);
        return false;
    }

    // copy to next timestamp ringbuffer
    void* nextTimestamp = mTimestamps.Peek(RingBuffer::Write);

    if( !nextTimestamp )
    {
        spdlog::error("gstBufferManager -- failed to retrieve next timestamp ringbuffer for writing");
        return false;
    }

    if (GST_BUFFER_DTS_IS_VALID(gstBuffer) || GST_BUFFER_PTS_IS_VALID(gstBuffer))
    {
        timestamp = GST_BUFFER_DTS_OR_PTS(gstBuffer);
    }

    memcpy(nextTimestamp, (void*)&timestamp, timestamp_size);
    mTimestamps.Next(RingBuffer::Write);

    mWaitEvent.Wake();
    mFrameCount++;

    gst_buffer_unmap(gstBuffer, &map);


    return true;

}
