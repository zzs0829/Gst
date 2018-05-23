#include <QCoreApplication>
#include <QDebug>
#include <gst/gst.h>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    GstElement *pipeline;
    GstElement *source, *filter, *sink;

    gst_init (&argc, &argv);

    pipeline = gst_pipeline_new ("my-pipeline");

    source = gst_element_factory_make ("fakesrc", "source");
    filter = gst_element_factory_make ("identity", "filter");
    sink = gst_element_factory_make ("fakesink", "sink");

    gst_bin_add_many (GST_BIN (pipeline), source, filter, sink, NULL);

    if (!gst_element_link_many (source, filter, sink, NULL)) {
        qWarning ("Failed to link elements!\n");
    }else{
        qDebug ("Successed to link elements!\n");
    }

    return false && a.exec();
}
