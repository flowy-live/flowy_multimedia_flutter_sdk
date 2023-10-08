
#include "gst/gstpad.h"
#include <functional>
#include <gst/gstbus.h>
#include <gst/gst.h>

#include <memory>
#include <string>

typedef std::function<void(uint8_t*, uint32_t, int32_t, int32_t, int32_t)> VideoFrameCallback;

class FlowyMedia
{
public:
    FlowyMedia();
    ~FlowyMedia();

    void InitAudio();
    void InitVideo();

    void StartSendLiveAudio();
    void StopSendLiveAudio();

    // TODO: implement
    void StartSendLiveVideo();
    void StopSendLiveVideo();

    void StartReceiveVideo();
    void StopReceiveVideo();

    void StartRecord();

    /**
     * @return path to recorded file
     */
    std::string StopRecord();

    void onReceiveVideoFrame(VideoFrameCallback callback);

private:
    GstElement* m_audio_send_pipeline;
    GstElement* m_audio_receive_pipeline;
    GstElement* m_record_pipeline;

    struct GstVideoPipeline
    {
        GstElement* pipeline;
        GstElement* video_sink;
        GstBus*     bus;
        GstBuffer*  buffer;

        int64_t                   width;
        int64_t                   height;
        std::unique_ptr<uint32_t> pixels;
        GstBuffer*                last_buffer;
    };
    GstVideoPipeline*      m_video_receive_pipeline;
    static GstBusSyncReply HandleGstMessage(GstBus* bus, GstMessage* message, gpointer user_data);
    static GstFlowReturn   on_new_sample(GstElement* sink, gpointer user_data);
    void HandoffHandler(GstElement* fakesink, GstBuffer* buf, GstPad* new_pad, gpointer user_data);

    VideoFrameCallback video_callback_;
};
