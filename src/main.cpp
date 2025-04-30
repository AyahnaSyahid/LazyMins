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

    if(!uman.nameExists("root")) {
        uman.createUser("root", "holis", "Na Ha La Ka Ma Ra Da");
    }

    AdminWindow window;

    LoginForm lf(&uman);
    lf.connect(&lf, &QDialog::accepted, &window, &QMainWindow::show);
    lf.open();
    return app.exec();
}