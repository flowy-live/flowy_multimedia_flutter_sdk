
#include <gst/gst.h>

#include <memory>
#include <string>

class FlowyMedia
{
public:
    FlowyMedia();
    ~FlowyMedia();

    /**
     * Initializes pipeline and elements and starts receiving media
     * NOTE: only audio pipeline is initialized for now
     */
    void Init();

    void StartSendLiveAudio();
    void StopSendLiveAudio();

    void StartSendLiveVideo();
    void StopSendLiveVideo();

    void         StartRecord();
    std::string& StopRecord();

private:
    GstElement* m_audio_pipeline;
    GstElement* m_video_pipeline;
    GstElement* m_record_pipeline;
};
