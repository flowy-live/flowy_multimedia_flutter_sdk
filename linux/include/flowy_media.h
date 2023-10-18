
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

    /// Set up socket connections, gstreamer pipelines, etc.
    void Init();

    /**
     * Starts receiving all external media from specified roomId
     * accomplishes udp hole-punching for NAT traversal.
     */
    void Subscribe(const std::string& roomId);

    /**
     * Starts publishing local audio and video to the media server
     */
    void Publish();

    void Unpublish();

    /**
     * Stops receiving all external media from any room
     */
    void Leave();

    void StartRecord();
    /**
     * @return path to recorded file
     */
    std::string StopRecord();

    enum VideoRenderType
    {
        Local, Remote
    };

    void onReceiveVideoFrame(VideoFrameCallback callback, VideoRenderType type);

private:
    void InitAudioReceivePipeline();
    void InitAudioSendPipeline();
    void InitVideoReceivePipeline();
    void InitVideoSendPipeline();
    void InitRecordPipeline();

    GstElement* m_audio_send_pipeline;
    GstElement* m_audio_receive_pipeline;

    GstElement* m_record_pipeline;

    struct GstVideoPipeline
    {
        GstElement* pipeline;
        GstElement* video_sink;

        int64_t                   width;
        int64_t                   height;
        std::unique_ptr<uint32_t> pixels;
        GstBuffer*                last_buffer;

        // TODO: make private and friend FlowyMedia?
        VideoFrameCallback        callback;
    };
    GstVideoPipeline*    m_video_send_pipeline;
    GstVideoPipeline*    m_video_receive_pipeline;
    static gboolean      HandleGstMessage(GstBus* bus, GstMessage* message, gpointer user_data);
    static GstFlowReturn on_new_sample_local(GstElement* sink, gpointer user_data);
    static GstFlowReturn on_new_sample_remote(GstElement* sink, gpointer user_data);
};
