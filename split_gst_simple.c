#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    printf("Running GStreamer pipeline to split video...\n");
    
    // Execute the working GStreamer command-line pipeline
    const char *command = "gst-launch-1.0 filesrc location=input.mp4 ! decodebin name=d ! tee name=t t. ! queue ! audioconvert ! faac ! mp4mux ! filesink location=output.mp4 t. ! queue ! audioconvert ! faac ! mpegpsmux name=mux ! filesink location=output.mps d. ! queue ! x264enc ! mux.";
    
    int result = system(command);
    
    if (result == 0) {
        printf("GStreamer pipeline completed successfully!\n");
        printf("Created output.mp4 (audio-only) and output.mps (video+audio)\n");
    } else {
        printf("GStreamer pipeline failed with exit code: %d\n", result);
    }
    
    return result;
} 