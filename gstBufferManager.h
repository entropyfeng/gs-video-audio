#pragma once
#include "VideoAudioOption.h"
#include "gst/gst.h"
#include "imageFormat.h"
#include "pmr.hpp"

class gstBufferManager
{
public:
    /**
     * Constructor
     */
    gstBufferManager( VideoAudioOption* options );

    /**
     * Destructor
     */
    ~gstBufferManager();

    /**
     * Enqueue a GstBuffer from GStreamer.
     */
    bool Enqueue( GstBuffer* buffer, GstCaps* caps );

    /**
     * Dequeue the next frame.  Returns 1 on success, 0 on timeout, -1 on error.
     */
    std::shared_ptr<std::vector<uint8_t>> Dequeue(imageFormat format, uint64_t timeout);

    /**
     * Get timestamp of the latest dequeued frame.
     */
    uint64_t GetLastTimestamp() const { return mLastTimestamp; }

    /**
     * Get raw image format.
       */
    inline imageFormat GetRawFormat() const { return mFormatYUV; }

    /**
     * Get the total number of frames that have been recieved.
     */
    inline uint64_t GetFrameCount() const	{ return mFrameCount; }

protected:


    imageFormat   mFormatYUV;  /**< The YUV colorspace format coming from appsink (typically NV12 or YUY2) */
    PMRQueue    mBufferYUV;  /**< Ringbuffer of CPU-based YUV frames (non-NVMM) that come from appsink */
    PMRQueue    mTimestamps; /**< Ringbuffer of timestamps that come from appsink */
    PMRQueue    mBufferRGB;  /**< Ringbuffer of frames that have been converted to RGB colorspace */
    uint64_t      mLastTimestamp;  /**< Timestamp of the latest dequeued frame */
    VideoAudioOption* mOptions;    /**< Options of the gstDecoder / gstCamera object */
    uint64_t	  mFrameCount; /**< Total number of frames that have been recieved */

};