
#include <gst/gst.h>

#include <string>

class FlowyMedia {
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

  void StartRecord();
  std::string &StopRecord();

private:
  GstElement *m_audio_pipeline;
  GstElement *m_video_pipeline;
  GstElement *m_record_pipeline;
};
