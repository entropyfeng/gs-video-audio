
#include <sstream>
#include "VideoAudioCapture.h"
#include "gstBufferManager.h"

bool gstreamerInit() {

    if (gstreamer_initialized) {
        return true;
    }

    int argc = 0;
    //char* argv[] = { "none" };

    if (!gst_init_check(&argc, nullptr, nullptr)) {
        spdlog::error("failed to initialize gstreamer library with gst_init()");
        return false;
    }

    gstreamer_initialized = true;

    uint32_t ver[] = {0, 0, 0, 0};
    gst_version(&ver[0], &ver[1], &ver[2], &ver[3]);

    spdlog::info("initialized gstreamer, version {}.{}.{}.{}", ver[0], ver[1], ver[2], ver[3]);

    // debugging
    gst_debug_remove_log_function(gst_debug_log_default);

    return true;
}

static const char *gst_stream_status_string(GstStreamStatusType status) {
    switch (status) {
        case GST_STREAM_STATUS_TYPE_CREATE:
            return "CREATE";
        case GST_STREAM_STATUS_TYPE_ENTER:
            return "ENTER";
        case GST_STREAM_STATUS_TYPE_LEAVE:
            return "LEAVE";
        case GST_STREAM_STATUS_TYPE_DESTROY:
            return "DESTROY";
        case GST_STREAM_STATUS_TYPE_START:
            return "START";
        case GST_STREAM_STATUS_TYPE_PAUSE:
            return "PAUSE";
        case GST_STREAM_STATUS_TYPE_STOP:
            return "STOP";
        default:
            return "UNKNOWN";
    }
}

// gst_message_print
gboolean gst_message_print(GstBus *bus, GstMessage *message, gpointer user_data) {
    switch (GST_MESSAGE_TYPE (message)) {
        case GST_MESSAGE_ERROR: {
            GError *err = nullptr;
            gchar *dbg_info = nullptr;

            gst_message_parse_error(message, &err, &dbg_info);
            spdlog::info("gstreamer {} ERROR {}", GST_OBJECT_NAME (message->src), err->message);
            spdlog::info("gstreamer Debugging info: {}", (dbg_info) ? dbg_info : "none");

            g_error_free(err);
            g_free(dbg_info);
            //g_main_loop_quit (app->loop);
            break;
        }
        case GST_MESSAGE_EOS: {
            spdlog::info("gstreamer {} recieved EOS signal...", GST_OBJECT_NAME(message->src));
            //g_main_loop_quit (app->loop);		// TODO trigger plugin Close() upon error
            break;
        }
        case GST_MESSAGE_STATE_CHANGED: {
            GstState old_state, new_state;

            gst_message_parse_state_changed(message, &old_state, &new_state, nullptr);

            spdlog::info("gstreamer {} changed state from {} to {}", GST_OBJECT_NAME(message->src),
                         gst_element_state_get_name(old_state),
                         gst_element_state_get_name(new_state));
            break;
        }
        case GST_MESSAGE_STREAM_STATUS: {
            GstStreamStatusType streamStatus;
            gst_message_parse_stream_status(message, &streamStatus, nullptr);

            spdlog::info("gstreamer {} stream status {} ==> {}", GST_OBJECT_NAME(message->src),
                         gst_stream_status_string(streamStatus),
                         GST_OBJECT_NAME(message->src));
            break;
        }
        case GST_MESSAGE_TAG: {
            GstTagList *tags = nullptr;
            gst_message_parse_tag(message, &tags);
            gchar *txt = gst_tag_list_to_string(tags);

            if (txt != nullptr) {
                spdlog::error("gstreamer {} tag {}", GST_OBJECT_NAME(message->src), txt);
                g_free(txt);
            }

            //gst_tag_list_foreach(tags, gst_print_one_tag, NULL);

            if (tags != nullptr)
                gst_tag_list_free(tags);

            break;
        }
        default: {
            spdlog::error("gstreamer {} message {} ==> {}", GST_OBJECT_NAME(message->src),
                          gst_message_type_get_name(GST_MESSAGE_TYPE(message)), GST_OBJECT_NAME(message->src));
            break;
        }
    }

    return TRUE;
}

bool VideoAudioCapture::initPipeline() {

    if (!gstreamerInit()) {
        spdlog::error("gst init error");
        return false;
    }
    GError *err = nullptr;
    buildLaunchStr();
    mPipeline = gst_parse_launch(mLaunchStr.c_str(), &err);

    if (err != nullptr) {
        spdlog::error("gstDecoder -- failed to create pipeline");
        spdlog::error("   ({})", err->message);
        g_error_free(err);
        return false;
    }

    GstPipeline *pipeline = GST_PIPELINE(mPipeline);

    if (!pipeline) {
        spdlog::error("gstDecoder -- failed to cast GstElement into GstPipeline");
        return false;
    }

    // retrieve pipeline bus
    mBus = gst_pipeline_get_bus(pipeline);

    if (!mBus) {
        spdlog::error("gstDecoder -- failed to retrieve GstBus from pipeline");
        return false;
    }

    // add watch for messages (disabled when we poll the bus ourselves, instead of gmainloop)
    //gst_bus_add_watch(mBus, (GstBusFunc)gst_message_print, NULL);

    // get the appsrc
    GstElement *appsinkElement = gst_bin_get_by_name(GST_BIN(pipeline), "mysink");
    GstAppSink *appsink = GST_APP_SINK(appsinkElement);

    if (!appsinkElement || !appsink) {
        spdlog::error("gstDecoder -- failed to retrieve AppSink element from pipeline");
        return false;
    }

    mAppSink = appsink;

    // setup callbacks
    GstAppSinkCallbacks cb;
    memset(&cb, 0, sizeof(GstAppSinkCallbacks));

    cb.eos = onEOS;
    cb.new_preroll = onPreroll;    // disabled b/c preroll sometimes occurs during Close() and crashes
    cb.new_sample = onBuffer;


    gst_app_sink_set_callbacks(mAppSink, &cb, (void *) this, nullptr);
    return true;
}

bool VideoAudioCapture::buildLaunchStr() {
    std::ostringstream ss;
    ss<< "filesrc location=/home/topeet/gst-workspace/samples/test.mp4 ! qtdemux name=demux demux.video_0 ! queue ! decodebin ! videoconvert ! video/x-raw ,format=(string)RGBA !   appsink name=mysink sync=false";
    mLaunchStr = ss.str();
    return true;
}


#define release_return { gst_sample_unref(gstSample); return; }


void VideoAudioCapture::checkBuffer() {

    if (!mAppSink) {
        return;
    }
    // block waiting for the sample
    GstSample *gstSample = gst_app_sink_pull_sample(mAppSink);

    if (!gstSample) {
        spdlog::error("gstDecoder -- app_sink_pull_sample() returned NULL...");
        return;
    }

    // retrieve sample caps
    GstCaps *gstCaps = gst_sample_get_caps(gstSample);

    if (!gstCaps) {
        spdlog::error("gstDecoder -- gst_sample had NULL caps...");
        release_return;
    }

    // retrieve the buffer from the sample
    GstBuffer *gstBuffer = gst_sample_get_buffer(gstSample);

    if (!gstBuffer) {
        spdlog::error("gstDecoder -- gst_sample had NULL buffer...");
        release_return;
    }


    const uint32_t width = mOptions.width;
    const uint32_t height = mOptions.height;

    // enqueue the buffer for color conversion
    if( !mBufferManager->Enqueue(gstBuffer, gstCaps) )
    {
        spdlog::error("gstDecoder -- failed to enqueue buffer for color conversion");
        release_return;
    }

    if ((width > 0 && width != mOptions.width) || (height > 0 && height != mOptions.height)) {
        spdlog::warn("gstDecoder -- resolution changing from ({0}x{1}) to ({2}x{3})", width, height, mOptions.width,
                     mOptions.height);

        // nvbufsurface: NvBufSurfaceCopy: buffer param mismatch
        GstElement *vidconv = gst_bin_get_by_name(GST_BIN(mPipeline), "vidconv");

        if (vidconv != nullptr) {
            gst_element_set_state(vidconv, GST_STATE_NULL);
            gst_element_set_state(vidconv, GST_STATE_PLAYING);
            gst_object_unref(vidconv);
        }
    }

    //mOptions.frameCount++;
    release_return;
}


void VideoAudioCapture::checkMsgBus() {
    while (true) {
        GstMessage *msg = gst_bus_pop(mBus);

        if (!msg) {
            break;
        }
        gst_message_print(mBus, msg, this);
        gst_message_unref(msg);
    }
}

GstFlowReturn VideoAudioCapture::onBuffer(_GstAppSink *sink, void *user_data) {
    //printf(LOG_GSTREAMER "gstDecoder -- onBuffer()\n");

    if (!user_data){
        return GST_FLOW_OK;
    }


    auto *dec = (VideoAudioCapture *) user_data;

    dec->checkBuffer();
    dec->checkMsgBus();

    return GST_FLOW_OK;
}

GstFlowReturn VideoAudioCapture::onPreroll(_GstAppSink *sink, void *user_data) {
    spdlog::debug("gstDecoder -- onPreroll()\n");

    if (!user_data)
        return GST_FLOW_OK;

    auto *dec = (VideoAudioCapture *) user_data;

    // onPreroll gets called sometimes, just pull and free the buffer
    // otherwise the pipeline may hang during shutdown
    GstSample *gstSample = gst_app_sink_pull_preroll(dec->mAppSink);

    if (!gstSample) {
        spdlog::error("gstDecoder -- app_sink_pull_preroll() returned NULL...");
        return GST_FLOW_OK;
    }

    gst_sample_unref(gstSample);

    dec->checkMsgBus();
    return GST_FLOW_OK;
}

bool VideoAudioCapture::isOpen() {
    return mStreaming;
}

void VideoAudioCapture::onEOS(_GstAppSink *sink, void *user_data) {

    spdlog::info("VideoAudioCapture -- onEOS()\n");
}

std::shared_ptr<std::vector<uint8_t>> VideoAudioCapture::Capture(imageFormat format, uint64_t timeout, int *status) {

    // confirm the stream is open
    if (!mStreaming || mEOS) {
        spdlog::error("the stream is not open or EOS has been reached");
        return nullptr;
    }
    // wait until a new frame is recieved
    auto result = mBufferManager->Dequeue(format, timeout);
    if (result == nullptr) {
        spdlog::error("gstDecoder -- failed to dequeue buffer for capture");

    }
    return result;
}

bool VideoAudioCapture::Open() {


    if (mStreaming) {
        return true;
    }
    spdlog::info("opening gs_push for streaming, transitioning pipeline to GST_STATE_PLAYING");
    const GstStateChangeReturn result = gst_element_set_state(mPipeline, GST_STATE_PLAYING);
    if (result == GST_STATE_CHANGE_ASYNC) {
        spdlog::debug("GST_STATE_CHANGE_ASYNC");
    } else if (result != GST_STATE_CHANGE_SUCCESS) {
        spdlog::error("gstCamera failed to set pipeline state to PLAYING ");
        return false;
    }
    checkMsgBus();
    usleep(100 * 1000);
    checkMsgBus();

    mStreaming = true;
    mEOS= false;
    return true;


}

#if 0
static void queryPipelineState( GstElement* pipeline )
{
    GstState state = GST_STATE_VOID_PENDING;
    GstState pending = GST_STATE_VOID_PENDING;

    GstStateChangeReturn result = gst_element_get_state (pipeline,
                          &state, &pending,  GST_CLOCK_TIME_NONE);

    if( result == GST_STATE_CHANGE_FAILURE )
        printf("GST_STATE_CHANGE_FAILURE\n");

    printf("state - %s\n", gst_element_state_get_name(state));
    printf("pending - %s\n", gst_element_state_get_name(pending));
}
#endif
