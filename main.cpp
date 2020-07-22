#include <gst/app/gstappsrc.h>
#include <gst/gst.h>

#include <chrono>
#include <iostream>
#include <thread>

#include "videosink.h"

const int width = 1920;
const int height = 1080;

const int n_test_samples = 50;

static uint8_t testbuf[n_test_samples][width * height * 3];
static bool quit = false;

void ctrl_c(int sig) {  // can be called asynchronously
    quit = true;
}

gint main(gint argc, gchar *argv[]) {
    signal(SIGINT, ctrl_c);

    // prepare test frames
    for (int frame = 0; frame < n_test_samples; frame++) {
        for (int i = 0; i < (width * height); i++) {
            testbuf[frame][i * 3 + 0] = 0xff;
            testbuf[frame][i * 3 + 1] = testbuf[frame][i * 3 + 2] = (0xff * frame) / n_test_samples;
        }
    }

    /* *** Uncomment one line below to test different pipelines. Pipeline must start with "appsrc name=src". *** */
    /* --- */
    // VideoSink sink("appsrc name=src ! videoconvert ! queue ! nvh265enc ! queue ! h265parse ! queue ! mpegtsmux ! filesink location=test.ts", width, height);
    // VideoSink sink("appsrc name=src ! videoconvert ! queue ! nvh265enc ! queue ! h265parse ! queue ! mp4mux ! filesink location=test.mp4", width, height);
    VideoSink sink("appsrc name=src ! videoconvert ! autovideosink sync=false", width, height);

    // starts sink (in a thread)
    sink.start();

    // start feeding sink with some frames
    int frame = 0;
    while (!quit) {
        sink.push(testbuf[frame], width * height * 3);

        frame = ++frame % n_test_samples;
        std::this_thread::sleep_for(std::chrono::milliseconds(40));
    }

    sink.stop();

    return 0;
}
