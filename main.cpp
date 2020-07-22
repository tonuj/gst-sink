#include <gst/app/gstappsrc.h>
#include <gst/gst.h>

#include <chrono>
#include <iostream>
#include <thread>

#include "videosink.h"

static uint8_t testbuf[25][1920 * 1080 * 3];
static bool quit = false;

void ctrl_c(int sig) {  // can be called asynchronously
    quit = true;
}

gint main(gint argc, gchar *argv[]) {
    signal(SIGINT, ctrl_c);

    // prepare test frames
    for (int frame = 0; frame < 25; frame++) {
        for (int i = 0; i < (1920 * 1080); i++) {
            testbuf[frame][i * 3 + 0] = 0xff;
            testbuf[frame][i * 3 + 1] =
                testbuf[frame][i * 3 + 2] = (0xff * frame) / 25;
        }
    }

    // uncomment one line below to test different pipelines
    VideoSink sink("appsrc name=src ! queue ! videoconvert ! nvh265enc ! queue ! h265parse ! queue ! mpegtsmux ! filesink location=test.ts", 1920, 1080);
    // VideoSink sink("appsrc name=src ! videoconvert ! autovideosink sync=false", 1920, 1080);

    // starts sink (in a thread)
    sink.start();

    // start feeding sink with some frames
    int frame = 0;
    while (!quit) {
        sink.push(testbuf[frame], 1920 * 1080 * 3);

        frame = ++frame % 25;
        std::this_thread::sleep_for(std::chrono::milliseconds(40));
    }

    sink.stop();

    return 0;
}
