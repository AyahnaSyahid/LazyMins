#include "src/database/databasemanager.h"
#include "src/mainwindow/mainwindow.h"
#include "src/setup/setupwindow.h"
#include "src/printer/printservice.h"
#include "src/utils/sessionmanager.h"

#include <QApplication>
#include <QSettings>
#include <QMessageBox>
#include <QSqlError>
#include <QFileInfo>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    Q_INIT_RESOURCE(database_resources);
    app.setOrganizationName("BlackCircle");
    app.setApplicationName("LazyMins");
    QSettings::setDefaultFormat(QSettings::IniFormat);

    QSettings s;
    auto dbPath = s.value(Config::Database::SETTINGS_KEY_DBPATH, "").toString();
    PrintService::instance().loadSettings();
    DatabaseManager &dbm = DatabaseManager::instance();
    MainWindow mainWindow;
    if (dbPath.isEmpty())
    { // Configure
        SetupWindow sw;
        if(sw.exec() != QDialog::Accepted) {
           app.quit();
           return 0;
        }
    }

    app.installEventFilter(&SessionManager::instance());
    // show() called from login dialog
    return app.exec();
}