#include "videosink.h"

VideoSink::VideoSink(const char *pipeline, gint w, gint h)
    : _loop(NULL),
      _want(0) {
    GstBus *bus;
    GError *error = NULL;

    /* init GStreamer */
    gst_init(0, NULL);

    /* setup pipeline */
    _pipeline = gst_parse_launch(pipeline, &error);
    if (!_pipeline) {
        g_print("Parse error: %s\n", error->message);
        throw;
    }

    _appsrc = gst_bin_get_by_name(GST_BIN(_pipeline), "src");
    // FIXME: add checks for null and throw

    bus = gst_pipeline_get_bus(GST_PIPELINE(_pipeline));
    _bus_watch_id = gst_bus_add_watch(bus, cb_bus, this);
    gst_object_unref(bus);

    /* setup */
    g_object_set(G_OBJECT(_appsrc), "caps",
                 gst_caps_new_simple("video/x-raw",
                                     "format", G_TYPE_STRING, "I420",
                                     "width", G_TYPE_INT, w,
                                     "height", G_TYPE_INT, h,
                                     "framerate", GST_TYPE_FRACTION, 25, 1,
                                     NULL),
                 NULL);

    /* setup appsrc */
    g_object_set(G_OBJECT(_appsrc),
                 "stream-type", 0,  // GST_APP_STREAM_TYPE_STREAM
                 "format", GST_FORMAT_TIME,
                 "is-live", TRUE,
                 NULL);

    g_signal_connect(_appsrc, "need-data", G_CALLBACK(cb_need_data), this);
}

VideoSink::~VideoSink() {
    gst_element_set_state(_pipeline, GST_STATE_NULL);
    gst_object_unref(GST_OBJECT(_pipeline));
    g_source_remove(_bus_watch_id);
}

void VideoSink::start() {
    if (_loop) {
        g_print("already started \n");
        return;
    }
    /* play */
    gst_element_set_state(_pipeline, GST_STATE_PLAYING);

    thread = std::thread([&]() {
        _loop = g_main_loop_new(NULL, FALSE);

        g_main_loop_run(_loop);

        g_main_loop_unref(_loop);
        _loop = NULL;
    });
}

void VideoSink::stop() {
    gst_element_send_event(_pipeline, gst_event_new_eos());
    g_main_loop_quit(_loop);

    thread.join();
}

void VideoSink::push(void *data, guint sz) {
    static GstClockTime timestamp = 0;
    GstBuffer *buffer;
    GstFlowReturn ret;

    if (!_want)
        return;
    _want = 0;

    buffer = gst_buffer_new_wrapped_full(GST_MEMORY_FLAG_READONLY, (gpointer)(data), sz, 0, sz, NULL, NULL);

    GST_BUFFER_PTS(buffer) = timestamp;
    GST_BUFFER_DURATION(buffer) = gst_util_uint64_scale_int(1, GST_SECOND, 25);

    timestamp += GST_BUFFER_DURATION(buffer);

    ret = gst_app_src_push_buffer(GST_APP_SRC(_appsrc), buffer);

    if (ret != GST_FLOW_OK) {
        g_main_loop_quit(_loop);
    }
}

gboolean
VideoSink::cb_bus(GstBus *bus, GstMessage *message, gpointer data) {
    GError *err;
    gchar *debug;

    VideoSink *ctx = static_cast<VideoSink *>(data);

    switch (GST_MESSAGE_TYPE(message)) {
        case GST_MESSAGE_ERROR: {
            gst_message_parse_error(message, &err, &debug);
            g_print("Error: %s\n", err->message);
            g_error_free(err);
            g_free(debug);

            g_main_loop_quit(ctx->_loop);
            break;
        }
        case GST_MESSAGE_WARNING: {
            gst_message_parse_warning(message, &err, &debug);
            g_print("Warning: %s\n", err->message);
            g_error_free(err);
            g_free(debug);

            break;
        }
        case GST_MESSAGE_EOS:
            g_print("Got EOS");
            g_main_loop_quit(ctx->_loop);
            break;
        default:
            g_print("Got message of type: %s\n", GST_MESSAGE_TYPE_NAME(message));
            break;
    }

    return TRUE;
}

void VideoSink::cb_need_data(GstElement *appsrc, guint unused_size, gpointer data) {
    VideoSink *ctx = static_cast<VideoSink *>(data);
    ctx->_want = 1;
}
