#include "dialog.h"
#include <QApplication>
#include <QTranslator>
#include <QLockFile>
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTranslator t;
    t.load(":/NHotel.qm");
    a.installTranslator(&t);

    QDir d;
    if (!d.exists(d.homePath() + "/NHotel"))
        d.mkdir(d.homePath() + "/NHotel");
    QFile file(d.homePath() + "/NHotel/" + "/lock.pid");
    file.remove();

    QLockFile lockFile(d.homePath() + "/NHotel" + "/lock.pid");
    if (!lockFile.tryLock())
        return -1;

    Dialog w;
    w.setWindowFlags(Qt::Window);
    w.show();

    return a.exec();
}
