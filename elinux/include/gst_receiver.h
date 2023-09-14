
#include <gst/gst.h>

class GstReceiver {
public:
  GstReceiver();
  ~GstReceiver();

  void Start();
  void Stop();

private:
  GstElement *m_pipeline;
  GstElement *source;
  GstElement *decoder;
  GstElement *sink;
};
