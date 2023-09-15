
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
    m_audio_send_pipeline
        = gst_parse_launch("alsasrc ! audioconvert ! audioresample ! "
                           "audio/x-raw,format=S16LE,rate=8000,channels=1 ! alawenc ! rtppcmapay ! "
                           "udpsink host=192.168.50.92 port=5002",
                           NULL);

    m_audio_receive_pipeline = gst_parse_launch(
        "udpsrc port=5003 ! application/x-rtp,media=audio,payload=8,clock-rate=8000,encoding-name=PCMA ! rtppcmadepay ! alawdec ! audioconvert ! audioresample ! alsasink",
        NULL);
    std::cout << "Starting receive audio pipeline" << std::endl;
    gst_element_set_state(m_audio_receive_pipeline, GST_STATE_PLAYING);
}

FlowyMedia::~FlowyMedia()
{
    gst_element_set_state(m_audio_send_pipeline, GST_STATE_NULL);
    gst_element_set_state(m_audio_receive_pipeline, GST_STATE_NULL);

    delete m_audio_send_pipeline;
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
