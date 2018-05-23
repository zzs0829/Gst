#include <QCoreApplication>
#include <QDebug>
#include <gst/gst.h>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    GstElement *player;
    const gchar *factoryname = "oggvorbisplayer";
    const gchar *name = "player";
    gst_init (&argc, &argv);

    GstElementFactory *factory;
    factory = gst_element_factory_find (factoryname);
    if(!factory) {
        qDebug("Failed to find factory of type '%s'.\n", factoryname);
        return -1;
    }

    player = gst_element_factory_make (factoryname, name);

    if(!player) {
        qDebug("Failed to create element of type '%s'.\n", factoryname);
        return -1;
    }else{
        qDebug("Successed to create element of type '%s'.\n", factoryname);
    }

    g_object_set (player, "location", "/media/disk/mingyueye.ogg", NULL);

    gst_element_set_state (GST_ELEMENT (player), GST_STATE_PLAYING);

    return false && a.exec();
}
