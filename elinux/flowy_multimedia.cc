
#include "include/flowy_media.h"

#include <gst/gst.h>

FlowyMedia::FlowyMedia()
{
    gst_init(nullptr, nullptr);

    m_audio_pipeline  = nullptr;
    m_video_pipeline  = nullptr;
    m_record_pipeline = nullptr;
}

FlowyMedia::~FlowyMedia() { gst_deinit(); }
