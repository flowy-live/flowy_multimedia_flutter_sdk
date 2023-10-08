
#include <gst/gstelement.h>
#include "include/flowy_media.h"

#include <algorithm>
#include <gst/gst.h>

#include <iostream>
#include <memory>
#include <mutex>
#include <string>

FlowyMedia::FlowyMedia()
{
    gst_init(nullptr, nullptr);

    m_audio_send_pipeline    = nullptr;
    m_audio_receive_pipeline = nullptr;
    m_record_pipeline        = nullptr;
    m_video_receive_pipeline = nullptr;
}

void FlowyMedia::InitAudio()
{
    if (m_audio_send_pipeline != nullptr)
    {
        std::cerr << "MEDIA: already initialized pipeline" << std::endl;
        return;
    }

    // TODO: use opusenc and rtpopuspay
    m_audio_send_pipeline = gst_parse_launch(
        "alsasrc ! audioconvert ! audioresample ! "
        "audio/x-raw,format=S16LE,rate=8000,channels=1 ! alawenc ! rtppcmapay ! queue ! "
        "udpsink host=192.168.50.92 port=5002",
        NULL);
    gst_element_set_state(m_audio_send_pipeline, GST_STATE_PAUSED);

    m_audio_receive_pipeline = gst_parse_launch(
        "udpsrc port=5003 ! "
        "application/x-rtp,media=audio,payload=8,clock-rate=8000,encoding-name=PCMA ! queue ! "
        "rtppcmadepay ! alawdec ! audioconvert ! audioresample ! alsasink",
        NULL);
    std::cout << "Starting receive audio pipeline" << std::endl;
    gst_element_set_state(m_audio_receive_pipeline, GST_STATE_PLAYING);

    // TODO: may need to fix based on camera device on embedded
    // TODO: use opus maybe for audio here
    GstElement* audio_src     = gst_element_factory_make("audiotestsrc", "audio-source");
    GstElement* video_src     = gst_element_factory_make("v4l2src", "video-source");
    GstElement* video_convert = gst_element_factory_make("videoconvert", "video-convert");
    GstElement* audio_enc     = gst_element_factory_make("voaacenc", "audio-enc");
    GstElement* video_enc     = gst_element_factory_make("x264enc", "video-enc");
    GstElement* audio_queue   = gst_element_factory_make("queue", "audio-queue");
    GstElement* mux           = gst_element_factory_make("avimux", "record-mux");
    GstElement* file_sink     = gst_element_factory_make("filesink", "record-sink");

    // set caps for video src
    GstCaps* video_caps = gst_caps_from_string("video/x-raw,width=640,height=480,framerate=30/1");
    GstElement* video_filter = gst_element_factory_make("capsfilter", "video-filter");
    g_object_set(G_OBJECT(video_filter), "caps", video_caps, NULL);

    m_record_pipeline = gst_pipeline_new("record-pipeline");

    if (!audio_src || !video_src || !video_convert || !audio_enc || !video_enc || !audio_queue
        || !mux || !file_sink || !m_record_pipeline)
    {
        std::cerr << "MEDIA: failed to create elements" << std::endl;
        return;
    }

    gst_bin_add_many(GST_BIN(m_record_pipeline),
                     video_src,
                     video_filter,
                     video_convert,
                     audio_src,
                     audio_queue,
                     audio_enc,
                     video_enc,
                     mux,
                     file_sink,
                     NULL);
    if (!gst_element_link_many(audio_src, audio_enc, audio_queue, mux, NULL))
    {
        g_printerr("Error(audio): Element could't be lined.\n");
        return;
    }
    if (!gst_element_link_many(
            video_src, video_filter, video_convert, video_enc, mux, file_sink, NULL))
    {
        g_printerr("Error(video): Element could't be lined.\n");
        return;
    }

    g_object_set(file_sink, "location", "test.mp4", NULL);
}

void FlowyMedia::InitVideo()
{
    if (m_video_receive_pipeline != nullptr)
    {
        std::cerr << "MEDIA: already initialized pipeline" << std::endl;
        return;
    }

    std::cout << "initializing video receive" << std::endl;

    m_video_receive_pipeline->src = gst_element_factory_make("videotestsrc", "video");
    m_video_receive_pipeline->video_convert
        = gst_element_factory_make("videoconvert", "videoconvert");
    m_video_receive_pipeline->video_sink = gst_element_factory_make("appsink", "sink");

    // set callback for appsink
    g_object_set(G_OBJECT(m_video_receive_pipeline->video_sink),
                 "emit-signals",
                 TRUE,
                 "sync",
                 FALSE,
                 nullptr);
    g_signal_connect(
        m_video_receive_pipeline->video_sink, "new-sample", G_CALLBACK(on_new_sample), this);

    if (!m_video_receive_pipeline->src || !m_video_receive_pipeline->video_convert
        || !m_video_receive_pipeline->video_sink)
    {
        std::cerr << "failed to create elements" << std::endl;
        g_printerr("Elements could not be linked.\n");
        return;
    }

    m_video_receive_pipeline->pipeline = gst_pipeline_new("local_capture");

    gst_bin_add_many(GST_BIN(m_video_receive_pipeline->pipeline),
                     m_video_receive_pipeline->src,
                     m_video_receive_pipeline->video_convert,
                     m_video_receive_pipeline->video_sink,
                     nullptr);
    if (!gst_element_link_many(m_video_receive_pipeline->src,
                               m_video_receive_pipeline->video_convert,
                               m_video_receive_pipeline->video_sink,
                               nullptr))
    {
        g_printerr("Elements could not be linked.\n");
        return;
    }

    gst_element_set_state(m_video_receive_pipeline->pipeline, GST_STATE_PAUSED);

    m_video_receive_pipeline->bus = gst_element_get_bus(m_video_receive_pipeline->pipeline);
    gst_bus_add_watch(
        m_video_receive_pipeline->bus,
        [](GstBus* bus, GstMessage* msg, gpointer data)
        {
            std::cout << "message type: " << GST_MESSAGE_TYPE(msg) << std::endl;

            switch (GST_MESSAGE_TYPE(msg))
            {
                case GST_MESSAGE_ERROR:
                {
                    GError* err;
                    gchar*  debug;
                    gst_message_parse_error(msg, &err, &debug);
                    g_printerr("Error: %s\n", err->message);
                    g_error_free(err);
                    g_free(debug);
                    break;
                }
                case GST_MESSAGE_EOS:
                    g_print("End of stream\n");
                    break;
                default:
                    break;
            }

            return (int) G_SOURCE_CONTINUE;
        },
        nullptr);

    std::cout << "video receive set up" << std::endl;
}

int FlowyMedia::on_new_sample(GstElement* sink, gpointer user_data)
{
    GstSample* sample;
    auto*      self = reinterpret_cast<FlowyMedia*>(user_data);

    /* Retrieve the buffer */
    g_signal_emit_by_name(sink, "pull-sample", &sample);
    if (sample)
    {
        GstBuffer* buffer                      = gst_sample_get_buffer(sample);
        self->m_video_receive_pipeline->buffer = std::move(buffer);

        GstMapInfo map;
        gst_buffer_map(buffer, &map, GST_MAP_READ);
        std::cout << "received " << map.size << " bytes" << std::endl;

        gst_sample_unref(sample);
        return GST_FLOW_OK;
    }

    return GST_FLOW_ERROR;
}

FlowyMedia::~FlowyMedia()
{
    if (m_audio_send_pipeline != nullptr)
    {
        gst_element_set_state(m_audio_send_pipeline, GST_STATE_NULL);
        gst_object_unref(m_audio_send_pipeline);
    }

    if (m_record_pipeline != nullptr)
    {
        gst_element_set_state(m_audio_receive_pipeline, GST_STATE_NULL);
        gst_object_unref(m_audio_receive_pipeline);
    }

    if (m_record_pipeline != nullptr)
    {
        gst_element_set_state(m_record_pipeline, GST_STATE_NULL);
        gst_object_unref(m_record_pipeline);
    }
    if (m_video_receive_pipeline->video_sink)
    {
        g_object_set(
            G_OBJECT(m_video_receive_pipeline->video_sink), "signal-handoffs", FALSE, NULL);
    }

    if (m_video_receive_pipeline->pipeline)
    {
        gst_element_set_state(m_video_receive_pipeline->pipeline, GST_STATE_NULL);
    }

    if (m_video_receive_pipeline->buffer)
    {
        gst_buffer_unref(m_video_receive_pipeline->buffer);
        m_video_receive_pipeline->buffer = nullptr;
    }

    if (m_video_receive_pipeline->bus)
    {
        gst_object_unref(m_video_receive_pipeline->bus);
        m_video_receive_pipeline->bus = nullptr;
    }

    if (m_video_receive_pipeline->pixels)
    {
        m_video_receive_pipeline->pixels.reset();
    }

    if (m_video_receive_pipeline->src)
    {
        gst_object_unref(m_video_receive_pipeline->src);
        m_video_receive_pipeline->src = nullptr;
    }

    if (m_video_receive_pipeline->video_convert)
    {
        gst_object_unref(m_video_receive_pipeline->video_convert);
        m_video_receive_pipeline->video_convert = nullptr;
    }

    gst_deinit();
}

void FlowyMedia::StartSendLiveAudio()
{
    std::cout << "Starting send live audio" << std::endl;
    gst_element_set_state(m_audio_send_pipeline, GST_STATE_PLAYING);
}

// TODO: figure out if we need to stop the pipeline or just pause it
// record pipeline might not work if this is paused
void FlowyMedia::StopSendLiveAudio()
{
    std::cout << "Stopping send live audio" << std::endl;
    gst_element_set_state(m_audio_send_pipeline, GST_STATE_PAUSED);
}

void FlowyMedia::StartSendLiveVideo()
{
    std::cerr << "Not implemented!!" << std::endl;
}

void FlowyMedia::StopSendLiveVideo()
{
    std::cerr << "Not implemented!!" << std::endl;
}

void FlowyMedia::StartRecord()
{
    std::cout << "Starting record" << std::endl;
    gst_element_set_state(m_record_pipeline, GST_STATE_PLAYING);
}

std::string FlowyMedia::StopRecord()
{
    std::cout << "Stopping record" << std::endl;
    gst_element_set_state(m_record_pipeline, GST_STATE_NULL);

    // get full path to file
    return std::string("./test.mp4");
}

GstBusSyncReply FlowyMedia::HandleGstMessage(GstBus* bus, GstMessage* message, gpointer user_data)
{
    switch (GST_MESSAGE_TYPE(message))
    {
        case GST_MESSAGE_EOS:
        {
            break;
        }
        case GST_MESSAGE_WARNING:
        {
            gchar*  debug;
            GError* error;
            gst_message_parse_warning(message, &error, &debug);
            g_printerr(
                "WARNING from element %s: %s\n", GST_OBJECT_NAME(message->src), error->message);
            g_printerr("Warning details: %s\n", debug);
            g_free(debug);
            g_error_free(error);
            break;
        }
        case GST_MESSAGE_ERROR:
        {
            gchar*  debug;
            GError* error;
            gst_message_parse_error(message, &error, &debug);
            g_printerr(
                "ERROR from element %s: %s\n", GST_OBJECT_NAME(message->src), error->message);
            g_printerr("Error details: %s\n", debug);
            g_free(debug);
            g_error_free(error);
            break;
        }
        default:
            break;
    }
    return GST_BUS_PASS;
}

void FlowyMedia::HandoffHandler(GstElement* fakesink,
                                GstBuffer*  buf,
                                GstPad*     new_pad,
                                gpointer    user_data)
{
    auto* self      = reinterpret_cast<FlowyMedia*>(user_data);
    auto* caps      = gst_pad_get_current_caps(new_pad);
    auto* structure = gst_caps_get_structure(caps, 0);

    int width;
    int height;
    gst_structure_get_int(structure, "width", &width);
    gst_structure_get_int(structure, "height", &height);
    if (width != self->m_video_receive_pipeline->width
        || height != self->m_video_receive_pipeline->height)
    {
        self->m_video_receive_pipeline->width  = width;
        self->m_video_receive_pipeline->height = height;
        self->m_video_receive_pipeline->pixels.reset(new uint32_t[width * height]);
        std::cout << "Pixel buffer size: width = " << width << ", height = " << height << std::endl;
    }

    if (self->m_video_receive_pipeline->last_buffer)
    {
        gst_buffer_unref(self->m_video_receive_pipeline->last_buffer);
        self->m_video_receive_pipeline->last_buffer = nullptr;
    }
    self->m_video_receive_pipeline->last_buffer = gst_buffer_ref(buf);
}

void FlowyMedia::StartReceiveVideo()
{
    std::cout << "Starting receive video" << std::endl;
    gst_element_set_state(m_video_receive_pipeline->pipeline, GST_STATE_PLAYING);
}

void FlowyMedia::StopReceiveVideo()
{
    std::cout << "Stopping receive video" << std::endl;
    gst_element_set_state(m_video_receive_pipeline->pipeline, GST_STATE_PAUSED);
}
