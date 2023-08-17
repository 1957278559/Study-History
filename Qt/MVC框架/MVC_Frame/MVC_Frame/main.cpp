#include "mvc_frame.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MVC_Frame w;
    w.show();
    return a.exec();
}
