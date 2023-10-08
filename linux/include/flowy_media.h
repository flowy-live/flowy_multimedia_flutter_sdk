
#include <gst/gstbus.h>
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

    // TODO: implement
    void StartSendLiveVideo();
    void StopSendLiveVideo();

    void StartRecord();

    /**
     * @return path to recorded file
     */
    std::string StopRecord();

private:
    GstElement* m_audio_send_pipeline;
    GstElement* m_audio_receive_pipeline;
    GstElement* m_record_pipeline;

    struct GstVideoPipeline
    {
        GstElement* pipeline;
        GstElement* src;
        GstElement* video_convert;
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
    static int             on_new_sample(GstElement* sink, gpointer user_data);

    void HandoffHandler(GstElement* fakesink, GstBuffer* buf, GstPad* new_pad, gpointer user_data);
};
