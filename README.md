# Signal & Meaning

A C project that uses GStreamer to split a video file into separate audio and video (with no audio) files.

Cursor created this entirely from the following prompt and a little nudging: 
> 🎯 **Developer Request:** write a working GStreamer application in C which splits input video into two separate files: MP3 and MP4 (without video).

It did require some nudging to complete. The first several attempts failed, so it prompted itself to build a `simple` version based on the CLI. From that it generated the `working` version. I pointed out an error in its understanding of the requirements at the end. It corrected that and built the `final` version. I would say that this was strictly vibe-coded--in that I didn't approach this particular challenging in a way that made it easy for cursor, which has severe limitations and I do not recommend. (More on alternatives later.) A complete transcript is available in [transcript.md](transcript.md).

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

## Acknowledgements
Thanks to Vlad Dovgalecs, who had tried this prompt elsewhere.