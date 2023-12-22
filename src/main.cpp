#include "AeroDms.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    AeroDms w;
    w.show();
    return a.exec();
}
