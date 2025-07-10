#include <gst/gst.h>
#include <glib.h>

static void on_pad_added(GstElement *src, GstPad *new_pad, gpointer user_data);

int main(int argc, char *argv[]) {
    GstElement *pipeline, *source, *demux, *tee, *queue1, *queue2, *queue3;
    GstElement *audio_convert1, *audio_convert2, *audio_enc1, *audio_enc2;
    GstElement *video_enc, *mp4_mux, *mpegps_mux;
    GstElement *mp4_sink, *mpegps_sink;
    GstBus *bus;
    GMainLoop *loop;
    gboolean ret;

    gst_init(&argc, &argv);

    // Create elements
    pipeline = gst_pipeline_new("split-pipeline");
    source = gst_element_factory_make("filesrc", "source");
    demux = gst_element_factory_make("decodebin", "demux");
    tee = gst_element_factory_make("tee", "tee");
    queue1 = gst_element_factory_make("queue", "queue1");
    queue2 = gst_element_factory_make("queue", "queue2");
    queue3 = gst_element_factory_make("queue", "queue3");
    audio_convert1 = gst_element_factory_make("audioconvert", "audio_convert1");
    audio_convert2 = gst_element_factory_make("audioconvert", "audio_convert2");
    audio_enc1 = gst_element_factory_make("faac", "audio_enc1");
    audio_enc2 = gst_element_factory_make("faac", "audio_enc2");
    video_enc = gst_element_factory_make("x264enc", "video_enc");
    mp4_mux = gst_element_factory_make("mp4mux", "mp4_mux");
    mpegps_mux = gst_element_factory_make("mpegpsmux", "mpegps_mux");
    mp4_sink = gst_element_factory_make("filesink", "mp4_sink");
    mpegps_sink = gst_element_factory_make("filesink", "mpegps_sink");

    if (!pipeline || !source || !demux || !tee || !queue1 || !queue2 || !queue3 ||
        !audio_convert1 || !audio_convert2 || !audio_enc1 || !audio_enc2 ||
        !video_enc || !mp4_mux || !mpegps_mux || !mp4_sink || !mpegps_sink) {
        g_printerr("Not all elements could be created.\n");
        return -1;
    }

    // Set properties
    g_object_set(source, "location", "input.mp4", NULL);
    g_object_set(mp4_sink, "location", "output.mp4", NULL);
    g_object_set(mpegps_sink, "location", "output.mps", NULL);

    // Build pipeline
    gst_bin_add_many(GST_BIN(pipeline), source, demux, tee, queue1, queue2, queue3,
                     audio_convert1, audio_convert2, audio_enc1, audio_enc2,
                     video_enc, mp4_mux, mpegps_mux, mp4_sink, mpegps_sink, NULL);

    // Link source to demux
    if (!gst_element_link(source, demux)) {
        g_printerr("Source and demux could not be linked.\n");
        gst_object_unref(pipeline);
        return -1;
    }

    // Link demux to tee (this will be done in pad-added callback)
    g_signal_connect(demux, "pad-added", G_CALLBACK(on_pad_added), pipeline);

    // Set up bus
    bus = gst_element_get_bus(pipeline);
    gst_bus_add_signal_watch(bus);

    // Main loop
    loop = g_main_loop_new(NULL, FALSE);
    
    ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Unable to set the pipeline to the playing state.\n");
        gst_object_unref(pipeline);
        return -1;
    }

    g_print("Running GStreamer pipeline...\n");
    g_main_loop_run(loop);

    // Clean up
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(bus);
    gst_object_unref(pipeline);
    g_main_loop_unref(loop);

    return 0;
}

static void on_pad_added(GstElement *src, GstPad *new_pad, gpointer user_data) {
    GstElement *pipeline = (GstElement *)user_data;
    GstCaps *caps = gst_pad_get_current_caps(new_pad);
    const gchar *name = gst_structure_get_name(gst_caps_get_structure(caps, 0));

    GstElement *tee = gst_bin_get_by_name(GST_BIN(pipeline), "tee");
    GstElement *queue1 = gst_bin_get_by_name(GST_BIN(pipeline), "queue1");
    GstElement *queue2 = gst_bin_get_by_name(GST_BIN(pipeline), "queue2");
    GstElement *queue3 = gst_bin_get_by_name(GST_BIN(pipeline), "queue3");
    GstElement *audio_convert1 = gst_bin_get_by_name(GST_BIN(pipeline), "audio_convert1");
    GstElement *audio_convert2 = gst_bin_get_by_name(GST_BIN(pipeline), "audio_convert2");
    GstElement *audio_enc1 = gst_bin_get_by_name(GST_BIN(pipeline), "audio_enc1");
    GstElement *audio_enc2 = gst_bin_get_by_name(GST_BIN(pipeline), "audio_enc2");
    GstElement *video_enc = gst_bin_get_by_name(GST_BIN(pipeline), "video_enc");
    GstElement *mp4_mux = gst_bin_get_by_name(GST_BIN(pipeline), "mp4_mux");
    GstElement *mpegps_mux = gst_bin_get_by_name(GST_BIN(pipeline), "mpegps_mux");
    GstElement *mp4_sink = gst_bin_get_by_name(GST_BIN(pipeline), "mp4_sink");
    GstElement *mpegps_sink = gst_bin_get_by_name(GST_BIN(pipeline), "mpegps_sink");

    if (g_str_has_prefix(name, "audio/")) {
        g_print("Linking audio pad to tee\n");
        // Link demux audio pad to tee
        GstPad *tee_sink = gst_element_get_static_pad(tee, "sink");
        if (gst_pad_is_linked(tee_sink) == FALSE) {
            gst_pad_link(new_pad, tee_sink);
        }

        // Link tee to both audio branches
        GstPad *tee_src1 = gst_element_request_pad_simple(tee, "src_%u");
        GstPad *tee_src2 = gst_element_request_pad_simple(tee, "src_%u");
        GstPad *queue1_sink = gst_element_get_static_pad(queue1, "sink");
        GstPad *queue2_sink = gst_element_get_static_pad(queue2, "sink");

        gst_pad_link(tee_src1, queue1_sink);
        gst_pad_link(tee_src2, queue2_sink);

        // Link MP4 branch: queue1 -> audio_convert1 -> audio_enc1 -> mp4_mux -> mp4_sink
        gst_element_link_many(queue1, audio_convert1, audio_enc1, mp4_mux, mp4_sink, NULL);

        // Link MPEG-PS audio branch: queue2 -> audio_convert2 -> audio_enc2 -> mpegps_mux
        gst_element_link_many(queue2, audio_convert2, audio_enc2, mpegps_mux, NULL);

        gst_object_unref(tee_sink);
        gst_object_unref(tee_src1);
        gst_object_unref(tee_src2);
        gst_object_unref(queue1_sink);
        gst_object_unref(queue2_sink);

    } else if (g_str_has_prefix(name, "video/")) {
        g_print("Linking video pad to queue3\n");
        // Link demux video pad to queue3
        GstPad *queue3_sink = gst_element_get_static_pad(queue3, "sink");
        if (gst_pad_is_linked(queue3_sink) == FALSE) {
            gst_pad_link(new_pad, queue3_sink);
        }

        // Link video branch: queue3 -> video_enc -> mpegps_mux
        gst_element_link_many(queue3, video_enc, mpegps_mux, NULL);

        gst_object_unref(queue3_sink);
    }

    gst_caps_unref(caps);
    gst_object_unref(tee);
    gst_object_unref(queue1);
    gst_object_unref(queue2);
    gst_object_unref(queue3);
    gst_object_unref(audio_convert1);
    gst_object_unref(audio_convert2);
    gst_object_unref(audio_enc1);
    gst_object_unref(audio_enc2);
    gst_object_unref(video_enc);
    gst_object_unref(mp4_mux);
    gst_object_unref(mpegps_mux);
    gst_object_unref(mp4_sink);
    gst_object_unref(mpegps_sink);
} 