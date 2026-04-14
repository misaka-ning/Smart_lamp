#include "smart_lamp.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Smart_lamp w;
    w.show();
    return a.exec();
}
