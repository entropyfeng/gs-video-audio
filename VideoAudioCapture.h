#pragma once

#include <iostream>
#include "gst/gst.h"
#include "gobject/gobject.h"
#include "gst/app/gstappsink.h"
#include "spdlog/spdlog.h"
#include "VideoAudioOption.h"
#include "imageFormat.h"
#include "gstBufferManager.h"
#include "blockingconcurrentqueue.h"
class VideoAudioCapture {
    VideoAudioOption mOptions;
    std::string mLaunchStr;

    void Close();

    void Open();
    bool isOpen();
    void checkMsgBus();

    void checkBuffer();

    bool init();

    static GstFlowReturn onPreroll(_GstAppSink *sink, void *user_data);

    static GstFlowReturn onBuffer(_GstAppSink *sink, void *user_data);
    static void onEOS(_GstAppSink* sink, void* user_data);

    GstBus*      mBus;
    GstElement*  mPipeline;
    _GstAppSink* mAppSink;
    bool Capture(void** output, imageFormat format, uint64_t timeout, int* status );

    std::unique_ptr<gstBufferManager> mBufferManager;

public:
    VideoAudioCapture() {

    }

    bool buildLaunchStr();

    bool initPipeline();

private:
    bool mEOS;
    bool mStreaming;

};