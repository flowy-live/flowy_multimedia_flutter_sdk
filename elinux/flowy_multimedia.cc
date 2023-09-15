
#include "gst/gstelement.h"
#include "include/flowy_media.h"

#include <gst/gst.h>

#include <iostream>
#include <memory>
#include <string>

FlowyMedia::FlowyMedia()
{
    gst_init(nullptr, nullptr);

    m_audio_pipeline  = nullptr;
    m_video_pipeline  = nullptr;
    m_record_pipeline = nullptr;
}

void FlowyMedia::Init()
{
    m_audio_pipeline
        = gst_parse_launch("alsasrc ! audioconvert ! audioresample ! "
                           "audio/x-raw,format=S16LE,rate=8000,channels=1 ! alawenc ! rtppcmapay ! "
                           "udpsink host=192.168.50.92 port=5002",
                           NULL);
}

FlowyMedia::~FlowyMedia()
{
    delete m_audio_pipeline;
    delete m_video_pipeline;
    delete m_record_pipeline;

    gst_deinit();
}

void FlowyMedia::StartSendLiveAudio()
{
    gst_element_set_state(m_audio_pipeline, GST_STATE_PLAYING);
}

// TODO: figure out if we need to stop the pipeline or just pause it
// record pipeline might not work if this is paused
void FlowyMedia::StopSendLiveAudio()
{
    gst_element_set_state(m_audio_pipeline, GST_STATE_PAUSED);
}

void FlowyMedia::StartSendLiveVideo()
{
    std::cerr << "Not implemented!!" << std::endl;
}

void FlowyMedia::StopSendLiveVideo()
{
    std::cerr << "Not implemented!!" << std::endl;
}
