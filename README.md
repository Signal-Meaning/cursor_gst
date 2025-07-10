# Signal & Meaning

A C project that uses GStreamer for video processing and manipulation.

Cursor created this from the following prompt: `write a working GStreamer application in C which splits input video into two separate files: MP3 and MP4 (without video).` Its first attempt failed, so it prompted itself to build the `simple` version. From that it generated the `working` version. I pointed out an error. It corrected that and built the `final` version. This was strictly vibe coded, which I do not recommend. (More on that later.)

## Files

- `split_gst.c` - Main GStreamer video processing implementation
- `split_gst_simple.c` - Simplified version of the video processing
- `split_gst_working.c` - Working version with additional features
- `split_gst_final.c` - Final optimized version

## Building

To compile the project, you'll need GStreamer development libraries installed:

```bash
# On macOS with Homebrew
brew install gstreamer gst-plugins-base gst-plugins-good

# Compile the programs
gcc -o split_gst split_gst.c $(pkg-config --cflags --libs gstreamer-1.0)
gcc -o split_gst_simple split_gst_simple.c $(pkg-config --cflags --libs gstreamer-1.0)
gcc -o split_gst_working split_gst_working.c $(pkg-config --cflags --libs gstreamer-1.0)
gcc -o split_gst_final split_gst_final.c $(pkg-config --cflags --libs gstreamer-1.0)
```

## Usage

The programs process video files using GStreamer pipelines. Input video files should be placed in the project directory.

## Dependencies

- GStreamer 1.0
- GStreamer plugins (base, good)
- C compiler (GCC or Clang) 