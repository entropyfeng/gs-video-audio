#pragma once

#include <iostream>
#include "gst/gst.h"
#include "gobject/gobject.h"
#include "gst/app/gstappsink.h"
#include "spdlog/spdlog.h"
#include "VideoAudioOption.h"
#include "imageFormat.h"
#include "gstBufferManager.h"
class VideoAudioCapture {
    VideoAudioOption mOptions;
    std::string mLaunchStr;
    VideoAudioCapture() {

    }

    void Close();

    void Open();

    void checkMsgBus();

    void checkBuffer();

    bool buildLaunchStr();

    bool init();

    bool initPipeline();

    static GstFlowReturn onPreroll(_GstAppSink *sink, void *user_data);

    static GstFlowReturn onBuffer(_GstAppSink *sink, void *user_data);
    static void onEOS(_GstAppSink* sink, void* user_data);

    GstBus*      mBus;
    GstElement*  mPipeline;
    _GstAppSink* mAppSink;
    bool Capture(void** output, imageFormat format, uint64_t timeout, int* status );

    gstBufferManager *mBufferManager;

};