
#include "src/database/databasemanager.h"
#include "src/managers/basemanager.h"
#include "src/mainwindow/mainwindow.h"
#include "src/setup/setupwindow.h"

#include <QtDebug>
#include <QSqlRecord>
#include <QApplication>
#include <QSettings>
#include <QVariantMap>
#include <QMessageBox>
#include <QTimer>

void startApp() {
  MainWindow *mw = new MainWindow();
  mw->show();
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    Q_INIT_RESOURCE(database_resources);

    app.setOrganizationName("AksaraJaya");
    app.setApplicationName("LazyAdmins");
    QSettings::setDefaultFormat(QSettings::IniFormat);

    QSettings settings;
    QString dbPath = settings.value("Database/databasePath", ":memory:").toString();

    if (dbPath == ":memory:") {
        // === Mode Setup Pertama Kali ===
        SetupWindow *setupWindow = new SetupWindow();

        // Hubungkan signal setupFinished untuk membuat MainWindow
        QObject::connect(setupWindow, &SetupWindow::setupFinished, &startApp);
        QObject::connect(setupWindow, &SetupWindow::setupFinished, setupWindow, &QDialog::accept);
        QObject::connect(setupWindow, &SetupWindow::setupFinished, setupWindow, &QDialog::deleteLater);

        // Hubungkan signal gagal
        QObject::connect(setupWindow, &SetupWindow::setupFailed,
                         qApp, &QApplication::quit);

        setupWindow->open();                    // Tampilkan sebagai jendela utama sementara
    } 
    else {
        // === Mode Normal (Database sudah ada) ===
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName(dbPath);

        if (!db.open()) {
            QMessageBox::critical(nullptr, "Fatal Error", 
                                  "Tidak dapat membuka database:\n" 
                                  + db.lastError().text());
            return 1;
        }

        // Inisialisasi DatabaseManager sebelum membuat MainWindow
        DatabaseManager::instance().setDatabase(db);
        BaseManager::connection = db;   // sesuaikan dengan implementasi Anda

        // Baru buat MainWindow setelah database siap
        MainWindow *mainWindow = new MainWindow();
        QTimer::singleShot(0, mainWindow, &MainWindow::openLoginForm);
    }

    return app.exec();
}