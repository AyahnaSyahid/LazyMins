#include "src/database/databasemanager.h"
#include "src/database/database_config.h"
#include "src/mainwindow/mainwindow.h"
#include "src/setup/setupwindow.h"
#include "src/printer/printservice.h"
#include "src/utils/sessionmanager.h"
#include "src/managers/basemanager.h"

#include <QApplication>
#include <QSettings>
#include <QDebug>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    Q_INIT_RESOURCE(database_resources);

    app.setOrganizationName("BlackCircle");
    app.setApplicationName("LazyMins");
    QSettings::setDefaultFormat(QSettings::IniFormat);

    PrintService::instance().loadSettings();

    QSettings s;
    auto dbPath = s.value(Config::Database::SETTINGS_KEY_DBPATH, "").toString();
    qDebug() << "[DBPath] resolved:" << dbPath << "(isEmpty:" << dbPath.isEmpty() << ")";

    // Force DatabaseManager singleton to open the database now.
    // Its constructor reads dbPath from QSettings and opens it; on first-run
    // the path is empty so it stays closed until initializeFromSetup replaces it.
    DatabaseManager &dbm = DatabaseManager::instance();
    if (!dbPath.isEmpty()) {
        BaseManager::connection = dbm.database();
    }

    MainWindow mainWindow;

    if (dbPath.isEmpty()) {
        qDebug() << "[Flow] first-run: showing SetupWindow";
        SetupWindow sw;
        QObject::connect(&sw, &SetupWindow::setupFinished,
                         &mainWindow, &MainWindow::continueSetup);
        if (sw.exec() != QDialog::Accepted) {
            app.quit();
            return 0;
        }
    } else {
        qDebug() << "[Flow] existing DB at" << dbPath << "- skipping setup";
        mainWindow.continueSetup();
    }
    app.installEventFilter(&SessionManager::instance());
    mainWindow.openLoginForm();
    return app.exec();
}
