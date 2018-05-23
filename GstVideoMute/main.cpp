#include <gst/gst.h>
#include <QCoreApplication>
#include "rever_interface.h"

static GstElement*  g_pipeline = NULL;
static GstElement*  g_source = NULL;
static GstElement*  g_videoqueue = NULL;
static GstElement*  g_audioqueue = NULL;
static GstElement*  g_audiosink = NULL;
static GstElement*  g_videosink = NULL;
static GstElement*  g_videobin = NULL;
static GstElement*  g_audiobin = NULL;
static GstElement*  g_videofakesink = NULL;
static GMainLoop*   g_loop = NULL;

static void pad_added_handler(GstElement* src, GstPad* new_pad, gpointer data)
{
  GstCaps* new_pad_caps = gst_pad_query_caps(new_pad, NULL);
  GstPadLinkReturn ret;
  GstStructure *new_pad_struct = new_pad_struct = gst_caps_get_structure (new_pad_caps, 0);
  const gchar* new_pad_type = gst_structure_get_name (new_pad_struct);

  /* link the video or audio sink elements */
  if( g_str_has_prefix(new_pad_type, "audio/x-raw")) {
    GstPad* sink_pad = gst_element_get_static_pad( GST_ELEMENT(g_audiobin), "sink");
    ret = gst_pad_link (new_pad, sink_pad);
  }
  else if(g_str_has_prefix(new_pad_type, "video/x-raw")){
    GstPad* sink_pad = gst_element_get_static_pad( GST_ELEMENT(g_videobin), "sink");
    ret = gst_pad_link (new_pad, sink_pad);
  }
}

static GstPadProbeReturn block_pad_probe_video_mute_cb(GstPad* pad, GstPadProbeInfo* info, gpointer user_data)
{
    ComHsaeReverseCtrlInterface *rever = static_cast<ComHsaeReverseCtrlInterface *>(user_data);

  gst_pad_remove_probe(pad, GST_PAD_PROBE_INFO_ID (info));

  g_videofakesink = gst_element_factory_make("fakesink", NULL);
  g_assert(g_videofakesink != NULL);

  g_object_set(GST_OBJECT(g_videofakesink), "sync", TRUE, NULL);
  gst_element_set_state(g_videosink, GST_STATE_NULL);
  gst_bin_remove(GST_BIN(g_videobin), g_videosink);

  gst_bin_add(GST_BIN(g_videobin),g_videofakesink);
  gst_element_set_state(g_videofakesink, GST_STATE_PLAYING);
  gst_element_link_many(g_videoqueue, g_videofakesink, NULL);

  rever->StartPlayReverse();
}

static GstPadProbeReturn block_pad_probe_video_unmute_cb(GstPad* pad, GstPadProbeInfo* info, gpointer user_data)
{

  /* remove this probe*/
  gst_pad_remove_probe(pad, GST_PAD_PROBE_INFO_ID (info));

  /* remove the fakesink */
  gst_element_set_state(g_videofakesink, GST_STATE_NULL);
  gst_bin_remove(GST_BIN(g_videobin), g_videofakesink);

  /* add the v4l2sink */
  g_videosink = gst_element_factory_make("overlaysink", NULL);
  g_assert(g_videosink != NULL);
  gst_bin_add(GST_BIN(g_videobin),g_videosink);
  gst_element_set_state(g_videosink, GST_STATE_PLAYING);
  gst_element_link_many(g_videoqueue, g_videosink, NULL);
}

static void video_mute(gpointer data)
{
//    gst_element_set_state(g_source, GST_STATE_PAUSED);

  GstPad* blockpad = gst_element_get_static_pad(g_videoqueue, "src");
  gst_pad_add_probe( blockpad, GST_PAD_PROBE_TYPE_BLOCK_DOWNSTREAM, block_pad_probe_video_mute_cb, data, NULL);
}

static void video_unmute()
{
//    gst_element_set_state(g_source, GST_STATE_PLAYING);

  GstPad* blockpad = gst_element_get_static_pad(g_videoqueue, "src");
  gst_pad_add_probe( blockpad, GST_PAD_PROBE_TYPE_BLOCK_DOWNSTREAM, block_pad_probe_video_unmute_cb, NULL, NULL);
}

static gboolean muted = FALSE;
static gboolean mute = FALSE;
static gboolean timer_hander(gpointer data)
{
    ComHsaeReverseCtrlInterface *rever = static_cast<ComHsaeReverseCtrlInterface *>(data);

  if(muted != mute) {
      mute = muted;

      if( mute)
      {
        video_mute(data);
//        mute = FALSE;
        g_print("video_mute");
      }
      else
      {
        video_unmute();
//        mute = TRUE;
        g_print("video_unmute");
      }
  }
}

static gboolean timer_mute_hander(gpointer data)
{
    video_mute(data);
}

static gboolean timer_unmute_hander(gpointer data)
{
    video_unmute();
}

static gboolean bus_event_hander(GstBus* bus, GstMessage* msg, gpointer data)
{
  switch(GST_MESSAGE_TYPE(msg))
  {
    /* EOS quit main loop*/
    case GST_MESSAGE_EOS:
//      g_main_loop_quit(g_loop);
    break;
        default:
    break;
  }
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);


  gst_init(&argc, &argv);

  /* create pipeline and components */
  const gchar *factoryname = "playbin";
//  g_pipeline = gst_pipeline_new("player");
  g_pipeline = gst_element_factory_make(factoryname, "palyer");
  g_assert(g_pipeline != NULL);
  g_source = gst_element_factory_make("uridecodebin", NULL);
  g_assert(g_source != NULL);
  g_videoqueue = gst_element_factory_make("queue", NULL);
  g_assert(g_videoqueue != NULL);
  g_audioqueue = gst_element_factory_make("queue", NULL);
  g_assert(g_audioqueue != NULL);
  g_audiosink = gst_element_factory_make("alsasink", NULL);
  g_assert(g_audiosink != NULL);
  g_videosink = gst_element_factory_make("overlaysink", NULL);
  g_assert(g_videosink != NULL);

  { /* create video bin */
    g_videobin = gst_bin_new("videobin");
    g_assert(g_videobin != NULL);
    gst_bin_add_many(GST_BIN(g_videobin), g_videoqueue, g_videosink,  NULL);
    gst_element_link_many(g_videoqueue, g_videosink, NULL);
    GstPad* pad = gst_element_get_static_pad(g_videoqueue, "sink");
    GstPad* ghost_pad = gst_ghost_pad_new ("sink", pad);
    gst_pad_set_active(ghost_pad, TRUE);
    gst_element_add_pad(g_videobin, ghost_pad);
    gst_object_unref(pad);
  }
  { /*create audio bin*/
    g_audiobin = gst_bin_new("audiobin");
    g_assert(g_audiobin != NULL);
    gst_bin_add_many(GST_BIN(g_audiobin), g_audioqueue, g_audiosink, NULL);
    gst_element_link_many(g_audioqueue, g_audiosink, NULL);
    GstPad* pad = gst_element_get_static_pad(g_audioqueue, "sink");
    GstPad* ghost_pad = gst_ghost_pad_new ("sink", pad);
    gst_pad_set_active(ghost_pad, TRUE);
    gst_element_add_pad(GST_ELEMENT(g_audiobin), ghost_pad);
    gst_object_unref(pad);
  }

  /* add components to pipeline*/
//  gst_bin_add_many(GST_BIN(g_pipeline), g_source, g_audiobin, g_videobin, NULL);
  g_object_set(G_OBJECT(g_pipeline), "video-sink", g_videobin, NULL);


  /* set uri to uridecodebin */
//  g_object_set(g_source, "uri", "file:///media/disk/testvideo.mpeg", NULL);
  g_object_set(g_pipeline, "uri", "file:///media/disk/testvideo.mpeg", NULL);

  /* register pad-added callback */
  g_signal_connect(g_source, "pad-added", G_CALLBACK(pad_added_handler), NULL);

  /* watch the bus event */
  guint bus_id = gst_bus_add_watch(GST_ELEMENT_BUS(g_pipeline), bus_event_hander, NULL);

  /* play */
  gst_element_set_state(g_pipeline, GST_STATE_PLAYING);
//  gst_element_set_state(g_pipeline, GST_STATE_PAUSED);

  /* set timer to switch video mute status */


  QDBusConnection connection = QDBusConnection::systemBus();
  ComHsaeReverseCtrlInterface *rever = new ComHsaeReverseCtrlInterface(QStringLiteral("com.hsae.reverse.ctrl"),
                                                         QStringLiteral("/com/hsae/reverse"), connection);
  rever->setTimeout(5000);
  guint timer_id = g_timeout_add(10, timer_hander, rever);
  rever->connect(rever, &ComHsaeReverseCtrlInterface::UpdateReverseState, [&](uchar state){
      qDebug() << Q_FUNC_INFO << state;
      if(state) {
          muted = true;
//          rever->StartPlayReverse();
      }else{
          muted = false;
      }
  });


  /* run the main loop*/
//  g_loop = g_main_loop_new(NULL, FALSE);
//  g_main_loop_run(g_loop);
  app.exec();

  gst_element_set_state(g_pipeline, GST_STATE_NULL);
  gst_object_unref(g_pipeline);
  return 0;
}
