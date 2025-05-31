#include "menubarmodifierinterface.h"
#include <QApplication>
#include <QMainWindow>
#include <QDir>
#include <QPluginLoader>
#include <QtDebug>
#include <QtPlugin>

// Q_IMPORT_PLUGIN(MenuBarPlugin)

int main(int argc, char** argv) {
    QApplication* app = new QApplication(argc, argv);
    QMainWindow mw;
    qDebug() << app->libraryPaths();
    QPluginLoader plug(QDir(app->applicationDirPath()).absoluteFilePath("plugins/libMenuBarPlugin.dll"), &mw);
    if(!plug.load()) {
        app->quit();
        qInfo() << "Unable to load plugins";
        return 1;
    }
    qobject_cast<MenuBarModifierInterface*>(plug.instance())->setupPlugin(&mw);
    mw.show();
    return app->exec();
}