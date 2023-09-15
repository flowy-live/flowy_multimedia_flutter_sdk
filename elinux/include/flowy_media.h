
#include <gst/gst.h>

#include <memory>
#include <string>

class FlowyMedia
{
public:
    FlowyMedia();
    ~FlowyMedia();

    /**
     * Initializes pipeline and elements and sets state to paused
     * NOTE: only audio pipeline is initialized for now
     */
    void Init();

    void StartLiveAudio();
    void StopLiveAudio();

    void StartLiveVideo();
    void StopLiveVideo();

    void         StartRecord();
    std::string& StopRecord();

private:
    std::unique_ptr<GstElement> m_audio_pipeline;
    std::unique_ptr<GstElement> m_video_pipeline;
    std::unique_ptr<GstElement> m_record_pipeline;
};
