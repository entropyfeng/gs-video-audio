#pragma once
#include "VideoAudioCapture.h"
#include "imageFormat.h"
#include "RingBuffer.h"
#include "Event.h"
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
    int Dequeue( void** output, imageFormat format, uint64_t timeout=UINT64_MAX );

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
    RingBuffer    mBufferYUV;  /**< Ringbuffer of CPU-based YUV frames (non-NVMM) that come from appsink */
    RingBuffer    mTimestamps; /**< Ringbuffer of timestamps that come from appsink */
    RingBuffer    mBufferRGB;  /**< Ringbuffer of frames that have been converted to RGB colorspace */
    uint64_t      mLastTimestamp;  /**< Timestamp of the latest dequeued frame */
    Event	      mWaitEvent;  /**< Event that gets triggered when a new frame is recieved */

    VideoAudioOption* mOptions;    /**< Options of the gstDecoder / gstCamera object */
    uint64_t	  mFrameCount; /**< Total number of frames that have been recieved */
    bool 	      mNvmmUsed;   /**< Is NVMM memory actually used by the stream? */

};