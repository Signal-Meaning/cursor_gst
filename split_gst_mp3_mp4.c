#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    printf("Running GStreamer pipeline to split video into MP3 and MP4...\n");
    
    // Execute GStreamer pipeline to create MP3 and MP4 files
    const char *command = "gst-launch-1.0 filesrc location=input.mp4 ! decodebin ! tee name=t t. ! queue ! audioconvert ! lamemp3enc ! filesink location=output.mp3 t. ! queue ! audioconvert ! faac ! mp4mux ! filesink location=output.mp4";
    
    int result = system(command);
    
    if (result == 0) {
        printf("GStreamer pipeline completed successfully!\n");
        printf("Created output.mp3 (audio-only) and output.mp4 (audio-only)\n");
    } else {
        printf("GStreamer pipeline failed with exit code: %d\n", result);
    }
    
    return result;
} 