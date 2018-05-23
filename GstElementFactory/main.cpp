#include <QCoreApplication>
#include <QDebug>
#include <gst/gst.h>

#define CREATE_FUNC 1

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    GstElement *element;
    const gchar *factoryname = "audiotestsrc";
    const gchar *name = "source";
    gst_init(&argc, &argv);

#if (0 == CREATE_FUNC) // gst_element_factory_make
    element = gst_element_factory_make (factoryname, name);

#elif (1 == CREATE_FUNC) // gst_element_factory_create
    GstElementFactory *factory;
    factory = gst_element_factory_find (factoryname);
    if(!factory) {
        qDebug("Failed to find factory of type '%s'.\n", factoryname);
        return -1;
    }
    element = gst_element_factory_create (factory, name);

#endif

    if(!element) {
        qDebug("Failed to create element of type '%s' [ func:%d ].\n", factoryname, CREATE_FUNC);
        return -1;
    }else{
        qDebug("Successed to create element of type '%s' [ func:%d ].\n", factoryname, CREATE_FUNC);
    }

#if (1 == CREATE_FUNC)
    /* display information */
    qDebug ("The '%s' element is a member of the category %s. \n"
            "Description: %s.\n",
            gst_plugin_feature_get_name (GST_PLUGIN_FEATURE (factory)),
            gst_element_factory_get_klass (factory),
            gst_element_factory_get_description (factory) );
#endif

    /* get name */
    gchar *ele_name;
    g_object_get (G_OBJECT(element), "name", &ele_name, NULL);
    qDebug("The name of the element is '%s'.\n", ele_name);
    g_free(ele_name);



    gst_object_unref (GST_OBJECT( element ));

    return false && a.exec();
}
