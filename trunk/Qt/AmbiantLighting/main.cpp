#include "AmbiantLighting.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    AMBIANT_LIGHTING w;
    w.show();

    return a.exec();
}
