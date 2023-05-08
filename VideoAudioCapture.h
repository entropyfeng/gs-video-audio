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
public:
    VideoAudioOption mOptions;
    std::string mLaunchStr;

    void Close();

    bool Open();
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

    std::shared_ptr<std::vector<uint8_t>> Capture(imageFormat format, uint64_t timeout, int* status);
    std::unique_ptr<gstBufferManager> mBufferManager;

public:
    VideoAudioCapture(std::unique_ptr<VideoAudioOption> option) {
        mBufferManager=std::make_unique<gstBufferManager>(std::move(option));
    }

    bool buildLaunchStr();

    bool initPipeline();

private:
    bool mEOS= false;
    bool mStreaming= false;

};