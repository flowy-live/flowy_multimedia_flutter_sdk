
#include "gst/gstelement.h"
#include "include/flowy_media.h"

#include <gst/gst.h>

#include <iostream>
#include <memory>
#include <string>

FlowyMedia::FlowyMedia()
{
    gst_init(nullptr, nullptr);

    m_audio_send_pipeline    = nullptr;
    m_audio_receive_pipeline = nullptr;
    m_record_pipeline        = nullptr;
}

void FlowyMedia::Init()
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
        "udpsrc port=5003 ! application/x-rtp,media=audio,payload=8,clock-rate=8000,encoding-name=PCMA ! queue ! rtppcmadepay ! alawdec ! audioconvert ! audioresample ! alsasink",
        NULL);
    std::cout << "Starting receive audio pipeline" << std::endl;
    gst_element_set_state(m_audio_receive_pipeline, GST_STATE_PLAYING);

    // TODO: may need to fix based on camera device on embedded
    // TODO: use opus maybe for audio here
    GstElement* audio_src   = gst_element_factory_make("audiotestsrc", "audio-source");
    GstElement* video_src   = gst_element_factory_make("videotestsrc", "video-source");
    GstElement* audio_enc   = gst_element_factory_make("voaacenc", "audio-enc");
    GstElement* video_enc   = gst_element_factory_make("x264enc", "video-enc");
    GstElement* audio_queue = gst_element_factory_make("queue", "audio-queue");
    GstElement* mux         = gst_element_factory_make("avimux", "record-mux");
    GstElement* file_sink   = gst_element_factory_make("filesink", "record-sink");

    m_record_pipeline = gst_pipeline_new("record-pipeline");

    if (!audio_src || !video_src || !audio_enc || !video_enc || !audio_queue || !mux || !file_sink
        || !m_record_pipeline)
    {
        std::cerr << "MEDIA: failed to create elements" << std::endl;
        return;
    }

    gst_bin_add_many(GST_BIN(m_record_pipeline),
                     video_src,
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
    if (!gst_element_link_many(video_src, video_enc, mux, file_sink, NULL))
    {
        g_printerr("Error(video): Element could't be lined.\n");
        return;
    }

    g_object_set(file_sink, "location", "test.mp4", NULL);
}

FlowyMedia::~FlowyMedia()
{
    gst_element_set_state(m_audio_send_pipeline, GST_STATE_NULL);
    gst_element_set_state(m_audio_receive_pipeline, GST_STATE_NULL);
    gst_element_set_state(m_record_pipeline, GST_STATE_NULL);

    gst_object_unref(m_audio_send_pipeline);
    gst_object_unref(m_audio_receive_pipeline);
    gst_object_unref(m_record_pipeline);

    delete m_audio_send_pipeline;
    delete m_audio_receive_pipeline;
    delete m_record_pipeline;

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
