#include <gst/app/gstappsrc.h>
#include <gst/gst.h>

#include <thread>

class VideoSink {
   public:
    VideoSink(const char * pipeline, gint w, gint h);
    ~VideoSink();

    void start();
    void push(void *data, guint sz);
    void stop();

   private:
    VideoSink();

    static gboolean cb_bus(GstBus *bus, GstMessage *message, gpointer data);
    static void cb_need_data(GstElement *appsrc, guint unused_size, gpointer data);

    guint _want;
    guint _bus_watch_id;

    GMainLoop *_loop;
    GstElement *_pipeline;
    GstElement *_appsrc;

    std::thread thread;
};
