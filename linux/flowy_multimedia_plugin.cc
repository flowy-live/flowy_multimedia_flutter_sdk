#include "include/flowy_multimedia/flowy_multimedia_plugin.h"

#include <flutter_linux/flutter_linux.h>
#include <gtk/gtk.h>
#include <memory>
#include <sys/utsname.h>

#include <iostream>

#include <cstring>

#include "include/flowy_media.h"
#include "include/video_outlet.h"
#include "flowy_multimedia_plugin_private.h"
#include "video_outlet.h"

#define FLOWY_MULTIMEDIA_PLUGIN(obj)                                                               \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), flowy_multimedia_plugin_get_type(), FlowyMultimediaPlugin))

struct _FlowyMultimediaPlugin
{
    GObject                     parent_instance;
    std::unique_ptr<FlowyMedia> m_flowy_media;
    // std::unique_ptr<FlPixelBufferTexture>      texture;
    // std::unique_ptr<FlutterDesktopPixelBuffer> pixel_buffer;
    // VideoOutlet*        local_video_outlet;
    VideoOutlet*        remote_video_outlet;
    FlTextureRegistrar* texture_registrar;
};

G_DEFINE_TYPE(FlowyMultimediaPlugin, flowy_multimedia_plugin, g_object_get_type())

// Called when a method call is received from Flutter.
static void flowy_multimedia_plugin_handle_method_call(FlowyMultimediaPlugin* self,
                                                       FlMethodCall*          method_call)
{
    g_autoptr(FlMethodResponse) response = nullptr;

    const gchar* method = fl_method_call_get_name(method_call);

    if (strcmp(method, "getPlatformVersion") == 0)
    {
        response = get_platform_version();
    }
    else if (strcmp(method, "subscribeToRoom") == 0)
    {
        FL_PIXEL_BUFFER_TEXTURE_GET_CLASS(self->remote_video_outlet)->copy_pixels
            = video_outlet_copy_pixels;
        fl_texture_registrar_register_texture(self->texture_registrar,
                                              FL_TEXTURE(self->remote_video_outlet));

        auto video_outlet_private
            = (VideoOutletPrivate*) video_outlet_get_instance_private(self->remote_video_outlet);
        video_outlet_private->texture_id
            = reinterpret_cast<int64_t>(FL_TEXTURE(self->remote_video_outlet));

        self->m_flowy_media->onReceiveVideoFrame(
            [texture_registrar    = self->texture_registrar,
             video_outlet_ptr     = self->remote_video_outlet,
             video_outlet_private = video_outlet_private](
                uint8_t* frame, uint32_t size, int32_t width, int32_t height, int32_t stride)
                -> void
            {
                video_outlet_private->buffer       = frame;
                video_outlet_private->video_width  = width;
                video_outlet_private->video_height = height;
                fl_texture_registrar_mark_texture_frame_available(texture_registrar,
                                                                  FL_TEXTURE(video_outlet_ptr));
            },
            FlowyMedia::VideoRenderType::Remote);

        self->m_flowy_media->Subscribe("test");

        response = FL_METHOD_RESPONSE(fl_method_success_response_new(fl_value_new_int(
            ((VideoOutletPrivate*) video_outlet_get_instance_private(self->remote_video_outlet))
                ->texture_id)));
    }
    else if (strcmp(method, "startPublish") == 0)
    {
        // FL_PIXEL_BUFFER_TEXTURE_GET_CLASS(self->local_video_outlet)->copy_pixels
        //     = video_outlet_copy_pixels;
        // fl_texture_registrar_register_texture(self->texture_registrar,
        //                                       FL_TEXTURE(self->local_video_outlet));
        //
        // auto video_outlet_private
        //     = (VideoOutletPrivate*) video_outlet_get_instance_private(self->local_video_outlet);
        // video_outlet_private->texture_id
        //     = reinterpret_cast<int64_t>(FL_TEXTURE(self->local_video_outlet));
        //
        // self->m_flowy_media->onReceiveVideoFrame(
        //     [texture_registrar    = self->texture_registrar,
        //      video_outlet_ptr     = self->local_video_outlet,
        //      video_outlet_private = video_outlet_private](
        //         uint8_t* frame, uint32_t size, int32_t width, int32_t height, int32_t stride)
        //         -> void
        //     {
        //         video_outlet_private->buffer       = frame;
        //         video_outlet_private->video_width  = width;
        //         video_outlet_private->video_height = height;
        //         fl_texture_registrar_mark_texture_frame_available(texture_registrar,
        //                                                           FL_TEXTURE(video_outlet_ptr));
        //     },
        //     FlowyMedia::VideoRenderType::Local);

        self->m_flowy_media->Publish();

        // response = FL_METHOD_RESPONSE(fl_method_success_response_new(fl_value_new_int(
        //     ((VideoOutletPrivate*) video_outlet_get_instance_private(self->local_video_outlet))
        //         ->texture_id)));
        response = FL_METHOD_RESPONSE(fl_method_success_response_new(fl_value_new_int(5)));
    }
    else if (strcmp(method, "stopPublish") == 0)
    {
        self->m_flowy_media->Unpublish();
        response = FL_METHOD_RESPONSE(fl_method_success_response_new(nullptr));
    }
    else if (strcmp(method, "startRecord") == 0)
    {
        self->m_flowy_media->StartRecord();
        response = FL_METHOD_RESPONSE(fl_method_success_response_new(nullptr));
    }
    else if (strcmp(method, "stopRecord") == 0)
    {
        self->m_flowy_media->StopRecord();
        response = FL_METHOD_RESPONSE(fl_method_success_response_new(nullptr));
    }
    else
    {
        response = FL_METHOD_RESPONSE(fl_method_not_implemented_response_new());
    }

    fl_method_call_respond(method_call, response, nullptr);
}

FlMethodResponse* get_platform_version()
{
    struct utsname uname_data = {};
    uname(&uname_data);
    g_autofree gchar* version = g_strdup_printf("Linux %s", uname_data.version);
    g_autoptr(FlValue) result = fl_value_new_string(version);
    return FL_METHOD_RESPONSE(fl_method_success_response_new(result));
}

static void flowy_multimedia_plugin_dispose(GObject* object)
{
    G_OBJECT_CLASS(flowy_multimedia_plugin_parent_class)->dispose(object);
}

static void flowy_multimedia_plugin_class_init(FlowyMultimediaPluginClass* klass)
{
    G_OBJECT_CLASS(klass)->dispose = flowy_multimedia_plugin_dispose;
}

static void flowy_multimedia_plugin_init(FlowyMultimediaPlugin* self)
{
    self->m_flowy_media       = std::make_unique<FlowyMedia>();
    self->remote_video_outlet = video_outlet_new();
    // self->local_video_outlet  = video_outlet_new();
}

static void method_call_cb(FlMethodChannel* channel, FlMethodCall* method_call, gpointer user_data)
{
    FlowyMultimediaPlugin* plugin = FLOWY_MULTIMEDIA_PLUGIN(user_data);
    flowy_multimedia_plugin_handle_method_call(plugin, method_call);
}

void flowy_multimedia_plugin_register_with_registrar(FlPluginRegistrar* registrar)
{
    FlowyMultimediaPlugin* plugin
        = FLOWY_MULTIMEDIA_PLUGIN(g_object_new(flowy_multimedia_plugin_get_type(), nullptr));

    g_autoptr(FlStandardMethodCodec) codec = fl_standard_method_codec_new();
    g_autoptr(FlMethodChannel) channel     = fl_method_channel_new(
        fl_plugin_registrar_get_messenger(registrar), "flowy_multimedia", FL_METHOD_CODEC(codec));

    plugin->texture_registrar = fl_plugin_registrar_get_texture_registrar(registrar);

    fl_method_channel_set_method_call_handler(
        channel, method_call_cb, g_object_ref(plugin), g_object_unref);

    g_object_unref(plugin);
}
