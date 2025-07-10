#include <gst/gst.h>

// Forward declaration
static void on_pad_added(GstElement *src, GstPad *new_pad, gpointer user_data);

int main(int argc, char *argv[]) {
    GstElement *pipeline, *source, *demux, *queue1, *queue2, *queue3, *queue4;
    GstElement *video_parse, *audio_parse, *mpegps_mux, *mp4_mux;
    GstElement *mpegps_sink, *mp4_sink;
    GstElement *video_enc, *audio_enc, *audio_enc2;
    GstBus *bus;
    GstMessage *msg;
    GMainLoop *loop;

    gst_init(&argc, &argv);

    // Create elements
    pipeline = gst_pipeline_new("split-pipeline");
    source = gst_element_factory_make("filesrc", "source");
    demux = gst_element_factory_make("decodebin", "demux");
    queue1 = gst_element_factory_make("queue", "queue1");
    queue2 = gst_element_factory_make("queue", "queue2");
    queue3 = gst_element_factory_make("queue", "queue3");
    queue4 = gst_element_factory_make("queue", "queue4");
    video_enc = gst_element_factory_make("x264enc", "video_enc");
    audio_enc = gst_element_factory_make("faac", "audio_enc");
    audio_enc2 = gst_element_factory_make("faac", "audio_enc2");
    mpegps_mux = gst_element_factory_make("mpegpsmux", "mpegps_mux");
    mp4_mux = gst_element_factory_make("mp4mux", "mp4_mux");
    mpegps_sink = gst_element_factory_make("filesink", "mpegps_sink");
    mp4_sink = gst_element_factory_make("filesink", "mp4_sink");

    if (!pipeline || !source || !demux || !queue1 || !queue2 || !queue3 || !queue4 ||
        !video_enc || !audio_enc || !audio_enc2 || !mpegps_mux || !mp4_mux ||
        !mpegps_sink || !mp4_sink) {
        g_printerr("Not all elements could be created.\n");
        return -1;
    }

    // Set properties
    g_object_set(source, "location", "input.mp4", NULL);
    g_object_set(mpegps_sink, "location", "output.mps", NULL);
    g_object_set(mp4_sink, "location", "output.mp4", NULL);

    // Build pipeline
    gst_bin_add_many(GST_BIN(pipeline), source, demux, queue1, queue2, queue3, queue4,
                     video_enc, audio_enc, audio_enc2, mpegps_mux, mp4_mux,
                     mpegps_sink, mp4_sink, NULL);

    if (!gst_element_link(source, demux)) {
        g_printerr("Source and demux could not be linked.\n");
        gst_object_unref(pipeline);
        return -1;
    }

    // Dynamic pad linking for decodebin
    g_signal_connect(demux, "pad-added", G_CALLBACK(on_pad_added), pipeline);

    // Main loop
    loop = g_main_loop_new(NULL, FALSE);
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    g_print("Running...\n");
    g_main_loop_run(loop);

    // Clean up
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    g_main_loop_unref(loop);

    return 0;
}

// Pad-added handler
static void on_pad_added(GstElement *src, GstPad *new_pad, gpointer user_data) {
    GstElement *pipeline = (GstElement *)user_data;
    GstCaps *caps = gst_pad_get_current_caps(new_pad);
    const gchar *name = gst_structure_get_name(gst_caps_get_structure(caps, 0));

    GstElement *queue1 = gst_bin_get_by_name(GST_BIN(pipeline), "queue1");
    GstElement *queue2 = gst_bin_get_by_name(GST_BIN(pipeline), "queue2");
    GstElement *queue3 = gst_bin_get_by_name(GST_BIN(pipeline), "queue3");
    GstElement *queue4 = gst_bin_get_by_name(GST_BIN(pipeline), "queue4");
    GstElement *video_enc = gst_bin_get_by_name(GST_BIN(pipeline), "video_enc");
    GstElement *audio_enc = gst_bin_get_by_name(GST_BIN(pipeline), "audio_enc");
    GstElement *audio_enc2 = gst_bin_get_by_name(GST_BIN(pipeline), "audio_enc2");
    GstElement *mpegps_mux = gst_bin_get_by_name(GST_BIN(pipeline), "mpegps_mux");
    GstElement *mp4_mux = gst_bin_get_by_name(GST_BIN(pipeline), "mp4_mux");
    GstElement *mpegps_sink = gst_bin_get_by_name(GST_BIN(pipeline), "mpegps_sink");
    GstElement *mp4_sink = gst_bin_get_by_name(GST_BIN(pipeline), "mp4_sink");

    if (g_str_has_prefix(name, "audio/")) {
        // Link audio to both branches
        GstPad *sink_pad1 = gst_element_get_static_pad(queue2, "sink");
        GstPad *sink_pad2 = gst_element_get_static_pad(queue4, "sink");
        if (gst_pad_is_linked(sink_pad1) == FALSE)
            gst_pad_link(new_pad, sink_pad1);
        if (gst_pad_is_linked(sink_pad2) == FALSE)
            gst_pad_link(new_pad, sink_pad2);

        // Link for MPEG-PS: queue2 -> audio_enc -> mpegps_mux
        gst_element_link_many(queue2, audio_enc, mpegps_mux, mpegps_sink, NULL);

        // Link for MP4: queue4 -> audio_enc2 -> mp4_mux -> mp4_sink
        gst_element_link_many(queue4, audio_enc2, mp4_mux, mp4_sink, NULL);

        gst_object_unref(sink_pad1);
        gst_object_unref(sink_pad2);
    } else if (g_str_has_prefix(name, "video/")) {
        // Link video only to MPEG-PS branch
        GstPad *sink_pad = gst_element_get_static_pad(queue1, "sink");
        if (gst_pad_is_linked(sink_pad) == FALSE)
            gst_pad_link(new_pad, sink_pad);

        // Link: queue1 -> video_enc -> mpegps_mux
        gst_element_link_many(queue1, video_enc, mpegps_mux, NULL);

        gst_object_unref(sink_pad);
    }

    gst_caps_unref(caps);
    gst_object_unref(queue1);
    gst_object_unref(queue2);
    gst_object_unref(queue3);
    gst_object_unref(queue4);
    gst_object_unref(video_enc);
    gst_object_unref(audio_enc);
    gst_object_unref(audio_enc2);
    gst_object_unref(mpegps_mux);
    gst_object_unref(mp4_mux);
    gst_object_unref(mpegps_sink);
    gst_object_unref(mp4_sink);
}