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

    // VideoSink sink("appsrc name=src ! queue ! videoconvert ! nvh265enc ! queue ! h265parse ! queue ! mpegtsmux ! filesink location=test.ts", 1920, 1080);
    VideoSink sink("appsrc name=src ! videoconvert ! autovideosink", 1920, 1080);

    // starts recorder (in a thread)
    sink.start();

    // prepare test frames
    for (int frame = 0; frame < 25; frame++) {
        for (int i = 0; i < sizeof(1920 * 1080); i++) {
            testbuf[frame][i*3+0] = (0xff * frame) / 25;
            testbuf[frame][i*3+1] = (0xff * frame) / 25;
            testbuf[frame][i*3+2] = 0xff;
        }
    }

    int frame = 0;
    while (!quit) {
        sink.push(testbuf[frame], 1920 * 1080 * 3);

        frame = frame++ % 25;
        std::this_thread::sleep_for(std::chrono::milliseconds(40));
    }

    sink.stop();

    return 0;
}
