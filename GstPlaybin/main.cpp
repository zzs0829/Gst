#include <QCoreApplication>
#include <QDebug>
#include <gst/gst.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
typedef struct _TMXCFBCTL
{
    int gbl_alpha_enable;
    int gbl_alpha_val;
    int color_key_enable;
    int color_key_val;
}TMXCFBCTL;

struct mxcfb_color_key {
    int enable;
    __u32 color_key;
};
struct mxcfb_gbl_alpha {
    int enable;
    int alpha;
};
#define MXCFB_SET_GBL_ALPHA     _IOW('F', 0x21, struct mxcfb_gbl_alpha)

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    GstElement *m_playbin;
    const gchar *factoryname = "playbin";
    const gchar *name = "playbin";
    gst_init(&argc, &argv);

    m_playbin = gst_element_factory_make (factoryname, name);

    if(!m_playbin) {
        qDebug("Failed to create element of type '%s' .\n", factoryname);
        return -1;
    }else{
        qDebug("Successed to create element of type '%s' .\n", factoryname);
    }

    /* get name */
    gchar *ele_name;
    g_object_get (G_OBJECT(m_playbin), "name", &ele_name, NULL);
    qDebug("The name of the element is '%s'.\n", ele_name);
    g_free(ele_name);

    gint flags;
//    g_object_set(G_OBJECT(m_playbin), "flags", 0x42, NULL);
    g_object_get (G_OBJECT(m_playbin), "flags", &flags, NULL);
    qDebug("The flags of the playbin is '%d'.\n", flags);

    const QByteArray envAudioSink = qgetenv("HS_GSTREAMER_PLAYBIN_AUDIOSINK");
    GstElement *audioSink = gst_element_factory_make(envAudioSink.isEmpty() ? "alsasink" : envAudioSink, "audiosink");
    if(!audioSink) {
        qDebug("Failed to create element of type 'alsasink' .\n");
        return -1;
    }else{
        qDebug("Successed to create element of type 'alsasink' .\n");
    }
//    g_object_set(audioSink, "sync", true, NULL);
//    g_object_set(audioSink, "device", "default", NULL);
    g_object_set(m_playbin, "audio-sink", audioSink, NULL);


//    GstElement *m_volumeElement = gst_element_factory_make("volume", "volumeelement");
//    if(!m_volumeElement) {
//        qDebug("Failed to create element of type 'volume' .\n");
//        return -1;
//    }else{
//        qDebug("Successed to create element of type 'volume' .\n");
//    }

//    // 这是一个哑巴element仅仅把输入数据不加修改的传过来。它也有一些有用的调试函数，例如偏移量和时间戳检测，或者丢弃缓冲。想要进一步了解请阅读它的文档。
//    GstElement *m_videoIdentity = gst_element_factory_make("identity", NULL); // floating ref
//    if(!m_videoIdentity) {
//        qDebug("Failed to create element of type 'identity' .\n");
//        return -1;
//    }else{
//        qDebug("Successed to create element of type 'identity' .\n");
//    }

//    // fakesink仅仅简单的抛弃所有输入的数据。在调试过程中它是很有用的，你可以用它来取代你想使用的sink来确保不会出现意外。它在gst-lauch命令行并出现-v参数后会显得非常冗余，如果你觉得无用信息太多，那么可以设置silent属性。
//    GstElement *m_nullVideoSink = gst_element_factory_make("fakesink", NULL); // m_nullVideoSink
//    if(!m_nullVideoSink) {
//        qDebug("Failed to create element of type 'fakesink' .\n");
//        return -1;
//    }else{
//        qDebug("Successed to create element of type 'fakesink' .\n");
//    }
//    g_object_set(G_OBJECT(m_nullVideoSink), "sync", true, NULL);
//    gst_object_ref(GST_OBJECT(m_nullVideoSink));

//    GstElement *m_videoOutputBin = gst_bin_new("video-output-bin");
//    if(!m_videoOutputBin) {
//        qDebug("Failed to create element of type 'video-output-bin' .\n");
//        return -1;
//    }else{
//        qDebug("Successed to create element of type 'video-output-bin' .\n");
//    }
//    // might not get a parent, take ownership to avoid leak
//    gst_object_ref_sink(GST_OBJECT(m_videoOutputBin));
//    gst_bin_add_many(GST_BIN(m_videoOutputBin), m_videoIdentity, m_nullVideoSink, NULL);
//    gst_element_link(m_videoIdentity, m_nullVideoSink);

//    GstElement *m_videoSink = m_nullVideoSink;

//    // add ghostpads
//    GstPad *pad = gst_element_get_static_pad(m_videoIdentity,"sink");
//    if(!pad) {
//        qDebug("Failed to create element of type 'pad' .\n");
//        return -1;
//    }else{
//        qDebug("Successed to create element of type 'pad' .\n");
//    }
//    gst_element_add_pad(GST_ELEMENT(m_videoOutputBin), gst_ghost_pad_new("sink", pad));
//    gst_object_unref(GST_OBJECT(pad));

    if (m_playbin != 0) {
        // Sort out messages
//        g_object_set(G_OBJECT(m_playbin), "uri", "file:///media/disk/mingyueye.ogg", NULL);
//        g_object_set(G_OBJECT(m_playbin), "uri", "file:///media/disk/testvideo.mpeg", NULL);
        g_object_set(G_OBJECT(m_playbin),  "uri", "file:///home/dev/Music/333.mp4", NULL);


//        struct mxcfb_color_key key;
//        struct mxcfb_gbl_alpha gbl_alpha;
//        int fd_fb0 = 0;
//        static int iLastGlobalAlphaEnable = -1;
//        TMXCFBCTL overlay={1,0xD0,1,0x00000000};
//        if( (fd_fb0 = open( "/dev/fb0", O_RDWR, 0 )) < 0 )
//        {
//            printf( "Unable to open /dev/fb0\n" );
//        }
//        else
//        {
//            gbl_alpha.enable = overlay.gbl_alpha_enable;
//            gbl_alpha.alpha = overlay.gbl_alpha_val;
//            ioctl(fd_fb0, MXCFB_SET_GBL_ALPHA, &gbl_alpha);
//            close( fd_fb0 );
//            iLastGlobalAlphaEnable = overlay.gbl_alpha_enable;
//        }


//        GstElement* auto_video_sink = NULL;
//        GstElement* actual_video_sink = NULL;
//        g_object_get(GST_OBJECT(m_playbin), "video-sink", &auto_video_sink, NULL);
//        if(NULL == auto_video_sink)
//        {
//            printf("gstMW : %s(): Can not find auto_video_sink\n");
//            return -1;
//        }

//        GstElement* videosink = gst_element_factory_make("fakesink", "videosink");
//        if(!videosink) {
//            qDebug("Failed to create element of type 'fakesink' .\n");
//            return -1;
//        }else{
//            qDebug("Successed to create element of type 'fakesink' .\n");
//        }
//        g_object_set(G_OBJECT(m_playbin), "video-sink", videosink, NULL);

        if (gst_element_set_state(m_playbin, GST_STATE_READY) == GST_STATE_CHANGE_FAILURE) {
            qWarning() << "GStreamer; Unable to GST_STATE_READY";
        } else {
            qWarning() << "GStreamer; GST_STATE_READY";
        }
        if (gst_element_set_state(m_playbin, GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE) {
            qWarning() << "GStreamer; Unable to play -";
        } else {
            qWarning() << "GStreamer; Playing";
        }

    }





    gst_object_unref (GST_OBJECT( m_playbin ));

    return a.exec();
}
