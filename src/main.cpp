#include <QApplication>
#include <QDebug>
#include <QMessageBox>
#include <QSettings>

#include "src/database/database_config.h"
#include "src/database/databasemanager.h"
#include "src/mainwindow/mainwindow.h"
#include "src/managers/basemanager.h"
#include "src/printer/printservice.h"
#include "src/setup/setupwindow.h"
#include "src/utils/licensegate.h"
#include "src/utils/sessionmanager.h"

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
  qDebug() << "[DBPath] resolved:" << dbPath << "(isEmpty:" << dbPath.isEmpty()
           << ")";

  // Force DatabaseManager singleton to open the database now.
  // Its constructor reads dbPath from QSettings and opens it; on first-run
  // the path is empty so it stays closed until initializeFromSetup replaces it.
  DatabaseManager &dbm = DatabaseManager::instance();
  if (!dbPath.isEmpty())
  {
    BaseManager::connection = dbm.database();
  }

  MainWindow mainWindow;

  bool firstInstall = true;

  if (dbPath.isEmpty())
  {
    qDebug() << "[Flow] first-run: showing SetupWindow";
    SetupWindow sw;
    QObject::connect(&sw, &SetupWindow::setupFinished, &mainWindow,
                     &MainWindow::continueSetup);
    if (sw.exec() != QDialog::Accepted)
    {
      app.quit();
      return 0;
    }
  }
  else
  {
    qDebug() << "[Flow] existing DB at" << dbPath << "- skipping setup";
    mainWindow.continueSetup();
    firstInstall = false;
  }
  // ——— License gate ———
  licensegate::initialize();
  auto gateRes = licensegate::result();

  if (gateRes.state == licensegate::GateState::Reminder)
  {
    licensegate::showReminder(gateRes.daysUsed, gateRes.hardwareId,
                              &mainWindow);
  }
  else if (gateRes.state == licensegate::GateState::Blocked)
  {
    if (!licensegate::showBlockDialog(gateRes.hardwareId, &mainWindow))
    {
      app.quit();
      return 0;
    }
  }
  else if (gateRes.state == licensegate::GateState::Error)
  {
    QMessageBox::warning(
        &mainWindow, "Peringatan Lisensi",
        QString("Terjadi masalah dengan record lisensi:\n%1\n\n"
                "Aplikasi akan ditutup, silahkan hubungi developer.")
            .arg(gateRes.errorMessage),
        QMessageBox::Ok);
    // Keluar aplikasi jika terdeteksi error di record lisensi
    app.quit();
    return 0;
  }
  app.installEventFilter(&SessionManager::instance());
  if (firstInstall)
    mainWindow.show();
  else
    mainWindow.openLoginForm();
  return app.exec();
}
