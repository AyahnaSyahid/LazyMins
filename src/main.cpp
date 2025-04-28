#include <QApplication>
#include <QMainWindow>
#include "adminwindow.h"
#include "usermanager.h"
#include "database.h"
#include "loginform.h"

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    Database database;
    UserManager uman;
    AdminWindow window;
    LoginForm lf(&uman);
    lf.open();
    return app.exec();
}