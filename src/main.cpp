#include "src/database/databasemanager.h"
#include "src/mainwindow/mainwindow.h"
#include "src/setup/setupwindow.h"
#include "src/printer/printservice.h"
#include "src/utils/sessionmanager.h"

#include <QApplication>
#include <QSettings>
#include <QMessageBox>
#include <QSqlError>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    Q_INIT_RESOURCE(database_resources);

    app.setOrganizationName(APP_ORGANIZATION);
    app.setApplicationName(APP_NAME);
    QSettings::setDefaultFormat(QSettings::IniFormat);

    PrintService::instance().loadSettings();

    DatabaseManager &dbm = DatabaseManager::instance();

    if (dbm.isFirstRun()) {
        SetupWindow setupWindow;

        QObject::connect(&setupWindow, &SetupWindow::setupFinished,
                         &setupWindow, &QDialog::accept);
        QObject::connect(&setupWindow, &SetupWindow::setupFailed,
                         qApp, &QApplication::quit);

        if (setupWindow.exec() != QDialog::Accepted) {
            return 0;
        }
    }

    if (!dbm.isOpen()) {
        QMessageBox::critical(nullptr, "Fatal Error",
                              "Tidak dapat membuka database:\n"
                              + dbm.lastError().text());
        return 1;
    }

    app.installEventFilter(&SessionManager::instance());
    MainWindow mainWindow;
    // show() called from login dialog
    
    return app.exec();
}