
#include <gst/gst.h>

#include <string>

class FlowyMedia {
public:
  FlowyMedia();
  ~FlowyMedia();

  void StartLiveAudio();
  void StopLiveAudio();

  void StartLiveVideo();
  void StopLiveVideo();

  void StartRecord();
  std::string& StopRecord();

private:
  GstElement *m_pipeline;
  GstElement *source;
  GstElement *decoder;
  GstElement *sink;
};
