#include <QApplication>
#include <QMainWindow>
#include "adminwindow.h"

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    AdminWindow window;
    window.show();
    return app.exec();
}