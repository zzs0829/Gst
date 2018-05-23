#include <QCoreApplication>
#include <QAbstractEventDispatcher>
#include <QDebug>
#include <QFileSystemWatcher>
#include <QDir>
#include <gst/gst.h>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    const gchar *nano_str;
    guint major, minor, micro, nano;
    gst_init (&argc, &argv);
    gst_version (&major, &minor, &micro, &nano);

    if (nano == 1)
        nano_str = "(Git)";
    else if (nano == 2)
        nano_str = "(Prerelease)";
    else
        nano_str = "(Release)";

    printf ("This program is linked against GStreamer %d.%d.%d_%d %s\n",
            major, minor, micro, nano, nano_str);


    QAbstractEventDispatcher *dispatcher = QCoreApplication::eventDispatcher();
    const bool hasGlib = dispatcher && dispatcher->inherits("QEventDispatcherGlib");
    if (!hasGlib) {
        qDebug() << "no hasGlib " << dispatcher;
    }else{
        qDebug() << "hasGlib " << dispatcher;
    }

    fflush(stdout);


    QFileSystemWatcher *watcher = new QFileSystemWatcher();
    watcher->addPath("/media/disk");
    watcher->connect(watcher, &QFileSystemWatcher::directoryChanged, [](const QString &dir){
        qDebug() << dir;
    });
    watcher->connect(watcher, &QFileSystemWatcher::fileChanged, [](const QString &file){
        qDebug() << file;

    });
    return a.exec();
}
