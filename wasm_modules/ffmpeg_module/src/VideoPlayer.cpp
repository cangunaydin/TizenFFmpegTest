#include <emscripten.h>
#include <VideoDecoder.h>
#include <fstream>
#include <vector>
#include <iostream>
#include <thread>

void *start_play(void *arg)
{
    VideoDecoder reader;
    Frame frame;
    reader.open("/bun33s.mp4", &frame); // change the file path to test.
    FrameDetails frameDetails;
    reader.getFrameDetails(&frameDetails);
    std::cout << "width - height" << frameDetails.width << " - " << frameDetails.height << ", timebase_num:" << frameDetails.timebase_num << " - timebase_den: " << frameDetails.timebase_den << "\n";
    reader.freeFrame(&frame);
    reader.close();
    return NULL;
}

extern "C"
{
    // void EMSCRIPTEN_KEEPALIVE test_buffer()
    // {
    //     std::ifstream file("/bun33s.mp4", std::ios::binary);
    //     std::vector<char> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    //     std::cout << "Buffer size: " << buffer.size() << std::endl;
    // }
    void play()
    {

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, start_play, NULL);

        // while (true)
        // {
        //     Frame frame;
        //     reader.open("/bun33s.mp4", &frame); // change the file path to test.
        //     FrameDetails frameDetails;
        //     reader.getFrameDetails(&frameDetails);
        //     std::cout << "width - height" << frameDetails.width << " - " << frameDetails.height << ", timebase_num:" << frameDetails.timebase_num << " - timebase_den: " << frameDetails.timebase_den << "\n";
        //     double start;
        //     auto last_pts = 0;
        //     bool first_frame = true;
        //     while (true)
        //     {
        //         if (first_frame)
        //         {
        //             first_frame = false;
        //             start = emscripten_get_now();
        //         }
        //         // Get the current time
        //         auto now = emscripten_get_now();
        //         // Calculate the elapsed time in milliseconds
        //         auto elapsed_ms = now - start;

        //         if (!reader.readFrame(&frame))
        //         {
        //             printf("Couldn't load video frame\n");
        //             return;
        //         }
        //         if (frame.pts_seconds == last_pts)
        //         {
        //             break;
        //         }
        //         last_pts = frame.pts_seconds;
        //         auto pts_ms = last_pts * 1000;
        //         if (pts_ms > elapsed_ms)
        //         {
        //             // Calculate the delay duration in milliseconds
        //             auto delay_ms = static_cast<int>(pts_ms - elapsed_ms);

        //             // Sleep for the delay duration
        //             emscripten_sleep(delay_ms);
        //         }

        //         EM_ASM_({
        //             var yData = new Uint8Array(Module.HEAPU8.subarray($0, $0 + $1));
        //             var uData = new Uint8Array(Module.HEAPU8.subarray($2, $2 + $3));
        //             var vData = new Uint8Array(Module.HEAPU8.subarray($4, $4 + $5));
        //             var width = $6;
        //             var height = $7;
        //             drawFrame(yData, uData,vData,width,height); }, frame.buffer[0], frame.size[0], frame.buffer[1], frame.size[1], frame.buffer[2], frame.size[2], frameDetails.width, frameDetails.height);
        //     }
        //     reader.freeFrame(&frame);
        //     reader.close();
        // }
    }
}
