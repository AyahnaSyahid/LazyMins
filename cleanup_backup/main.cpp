// setup.cpp

#include <QSettings>
#include <QFileInfo>
#include <QSqlDatabase>
#include <QFile>
#include <QDir>
#include <QFileDialog>
#include <QVariantMap>

namespace GLB {
  void setup(QVariantMap &vm);
}

void GLB::setup(QVariantMap &vm)
{
  QSettings s;
  bool isSettingsFileExists = QFileInfo::exists(s.fileName());
  if (!isSettingsFileExists) {
    auto path = QFileDialog::getExistingDirectory(nullptr, "Pilih direktori database", QDir::homePath());
    if(path.isEmpty()) {
      vm["success"] = false;
      vm["setup_message"] = "Anda membatalkan pemilihan path database";
      return ;
    }
    s.setValue("Database/database_file", QDir(path).absoluteFilePath("LAdminDB.sqlite3"));
    s.sync();
    vm["success"] = true;
    vm["run_init"] = true;
    return ;
  } else {
    // Run Verification
  }
}