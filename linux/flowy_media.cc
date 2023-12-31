
#include <gst/gstelement.h>
#include "gst/gstpad.h"
#include "gst/gstparse.h"
#include "gst/gstpipeline.h"
#include "include/flowy_media.h"
#include <gst/video/video.h>

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

    m_video_send_pipeline              = new GstVideoPipeline();
    m_video_send_pipeline->pipeline    = nullptr;
    m_video_receive_pipeline           = new GstVideoPipeline();
    m_video_receive_pipeline->pipeline = nullptr;

    this->InitVideoReceivePipeline();
    this->InitVideoSendPipeline();
}

void FlowyMedia::Subscribe(const std::string& roomId)
{
    if (m_video_receive_pipeline == nullptr || m_video_receive_pipeline->pipeline == nullptr)
    {
        std::cerr << "MEDIA: not initialized" << std::endl;
        return;
    }

    gst_element_set_state(m_video_receive_pipeline->pipeline, GST_STATE_PLAYING);
}

void FlowyMedia::InitAudioReceivePipeline()
{
    if (m_audio_receive_pipeline != nullptr)
    {
        std::cerr << "MEDIA: already initialized pipeline" << std::endl;
        return;
    }

    m_audio_receive_pipeline = gst_parse_launch(
        "udpsrc port=5003 ! "
        "application/x-rtp,media=audio,payload=8,clock-rate=8000,encoding-name=PCMA ! queue ! "
        "rtppcmadepay ! alawdec ! audioconvert ! audioresample ! alsasink",
        NULL);
    std::cout << "Starting receive audio pipeline" << std::endl;
    gst_element_set_state(m_audio_receive_pipeline, GST_STATE_PLAYING);
}

void FlowyMedia::InitAudioSendPipeline()
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
}

void FlowyMedia::InitVideoReceivePipeline()
{
    if (m_video_receive_pipeline->pipeline != nullptr)
    {
        std::cerr << "MEDIA: already initialized pipeline" << std::endl;
        return;
    }

    std::cout << "initializing video receive" << std::endl;

    m_video_receive_pipeline->pipeline = gst_parse_launch(
        "udpsrc port=5001 address=0.0.0.0 ! application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264, payload=(int)96 ! "
        "rtph264depay ! h264parse ! decodebin ! videoconvert ! video/x-raw,format=RGBA ! appsink name=sink",
        NULL);

    // get sink element from pipeline
    m_video_receive_pipeline->video_sink
        = gst_bin_get_by_name(GST_BIN(m_video_receive_pipeline->pipeline), "sink");
    g_assert(m_video_receive_pipeline->video_sink);

    // set callback for appsink
    g_object_set(G_OBJECT(m_video_receive_pipeline->video_sink),
                 "emit-signals",
                 TRUE,
                 "sync",
                 FALSE,
                 nullptr);
    g_signal_connect(
        m_video_receive_pipeline->video_sink, "new-sample", G_CALLBACK(on_new_sample_remote), this);

    // add bus callback
    GstBus* bus = gst_element_get_bus(m_video_receive_pipeline->pipeline);
    gst_bus_add_watch(bus, HandleGstMessage, nullptr);

    gst_element_set_state(m_video_receive_pipeline->pipeline, GST_STATE_PAUSED);

    std::cout << "video receive set up" << std::endl;
}

void FlowyMedia::InitVideoSendPipeline()
{
    if (m_video_send_pipeline->pipeline != nullptr)
    {
        std::cerr << "MEDIA: already initialized pipeline" << std::endl;
        return;
    }

    std::cout << "initializing video send" << std::endl;

    m_video_send_pipeline->pipeline = gst_parse_launch(
        "videotestsrc ! x264enc tune=zerolatency bitrate=500 speed-preset=superfast ! rtph264pay ! udpsink port=5000 host=0.0.0.0",
        NULL);

    // add bus callback
    GstBus* bus = gst_element_get_bus(m_video_send_pipeline->pipeline);
    gst_bus_add_watch(bus, HandleGstMessage, nullptr);

    gst_element_set_state(m_video_send_pipeline->pipeline, GST_STATE_PAUSED);

    std::cout << "video receive set up" << std::endl;
}

void FlowyMedia::InitRecordPipeline()
{
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

GstFlowReturn FlowyMedia::on_new_sample_local(GstElement* sink, gpointer gSelf)
{
    GstSample* sample = NULL;
    GstMapInfo bufferInfo;

    FlowyMedia* self = static_cast<FlowyMedia*>(gSelf);
    g_signal_emit_by_name(sink, "pull-sample", &sample);

    std::cout << "got video sample" << std::endl;

    if (sample != NULL)
    {
        GstBuffer* buffer_ = gst_sample_get_buffer(sample);
        if (buffer_ != NULL)
        {
            gst_buffer_map(buffer_, &bufferInfo, GST_MAP_READ);

            // Get video width and height
            GstVideoFrame vframe;
            GstVideoInfo  video_info;
            GstCaps*      sampleCaps = gst_sample_get_caps(sample);
            gst_video_info_from_caps(&video_info, sampleCaps);
            gst_video_frame_map(&vframe, &video_info, buffer_, GST_MAP_READ);

            self->m_video_send_pipeline->callback((uint8_t*) bufferInfo.data,
                                                  video_info.size,
                                                  video_info.width,
                                                  video_info.height,
                                                  video_info.stride[0]);

            gst_buffer_unmap(buffer_, &bufferInfo);
            gst_video_frame_unmap(&vframe);
        }
        gst_sample_unref(sample);
    }

    return GST_FLOW_OK;
}

GstFlowReturn FlowyMedia::on_new_sample_remote(GstElement* sink, gpointer gSelf)
{
    GstSample* sample = NULL;
    GstMapInfo bufferInfo;

    FlowyMedia* self = static_cast<FlowyMedia*>(gSelf);
    g_signal_emit_by_name(sink, "pull-sample", &sample);

    std::cout << "got sample" << std::endl;

    if (sample != NULL)
    {
        GstBuffer* buffer_ = gst_sample_get_buffer(sample);
        if (buffer_ != NULL)
        {
            // self->m_video_receive_pipeline->last_buffer = gst_buffer_copy(buffer_);

            gst_buffer_map(buffer_, &bufferInfo, GST_MAP_READ);

            // Get video width and height
            GstVideoFrame vframe;
            GstVideoInfo  video_info;
            GstCaps*      sampleCaps = gst_sample_get_caps(sample);
            gst_video_info_from_caps(&video_info, sampleCaps);
            gst_video_frame_map(&vframe, &video_info, buffer_, GST_MAP_READ);

            if (self->m_video_receive_pipeline->callback != nullptr)
            {
                self->m_video_receive_pipeline->callback((uint8_t*) bufferInfo.data,
                                                         video_info.size,
                                                         video_info.width,
                                                         video_info.height,
                                                         video_info.stride[0]);
            }
            else
            {
                std::cerr << "MEDIA: no callback set for on frame" << std::endl;
            }


            gst_buffer_unmap(buffer_, &bufferInfo);
            gst_video_frame_unmap(&vframe);
        }
        gst_sample_unref(sample);
    }

    return GST_FLOW_OK;
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

    if (m_video_receive_pipeline->last_buffer)
    {
        gst_buffer_unref(m_video_receive_pipeline->last_buffer);
        m_video_receive_pipeline->last_buffer = nullptr;
    }

    if (m_video_receive_pipeline->pixels)
    {
        m_video_receive_pipeline->pixels.reset();
    }

    gst_deinit();
}

void FlowyMedia::Publish()
{
    std::cout << "publishing media" << std::endl;
    // gst_element_set_state(m_audio_send_pipeline, GST_STATE_PLAYING);
    gst_element_set_state(m_video_send_pipeline->pipeline, GST_STATE_PLAYING);
}

// TODO: figure out if we need to stop the pipeline or just pause it
// record pipeline might not work if this is paused
void FlowyMedia::Unpublish()
{
    std::cout << "unpublishing media" << std::endl;
    // gst_element_set_state(m_audio_send_pipeline, GST_STATE_PAUSED);
    gst_element_set_state(m_video_send_pipeline->pipeline, GST_STATE_PAUSED);
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

gboolean FlowyMedia::HandleGstMessage(GstBus* bus, GstMessage* message, gpointer user_data)
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

void FlowyMedia::onReceiveVideoFrame(VideoFrameCallback callback, VideoRenderType type)
{
    switch (type)
    {
        case VideoRenderType::Local:
            m_video_send_pipeline->callback = callback;
            break;
        case VideoRenderType::Remote:
            m_video_receive_pipeline->callback = callback;
            break;
    }
}
