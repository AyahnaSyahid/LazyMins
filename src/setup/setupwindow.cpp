#include "setupwindow.h"
#include "ui_setupwindow.h"
#include "src/database/databasemanager.h"
#include "src/managers/adminmanager.h"
#include "src/managers/appsettingsmanager.h"

#include <QDir>
#include <QFileInfo>
#include <QFileDialog>
#include <QFileDialog>
#include <QSettings>
#include <QMessageBox>
#include <QTimer>
#include <QRegularExpression>

SetupWindow::SetupWindow(QWidget *p) :
  ui(new Ui::SetupWindow), m_abort(true), QDialog(p)
{
  ui->setupUi(this);
}

SetupWindow::~SetupWindow()
{
  delete ui;
  if(m_abort)
    emit setupFailed();
  else
    emit setupFinished();
}

void SetupWindow::on_browseButton_clicked() 
{
  auto dr = ui->lineEdit->text();
  if (!QFileInfo::exists(dr)) dr = QDir::homePath();
  
  auto databaseDir = QFileDialog::getExistingDirectory(this, "Pilih penyimpanan database", dr);
  if (databaseDir.isEmpty()) return;
  ui->lineEdit->setText(databaseDir);
}

void SetupWindow::on_installButton_clicked()
{
    // === 1. Kumpulkan Input ===
    QMap<QString, QString> params = {
        {"dbDir",       ui->lineEdit->text().trimmed()},
        {"compName",    ui->nameEdit->text().trimmed()},
        {"compTelp",    ui->telpEdit->text().trimmed()},
        {"compEmail",   ui->emailEdit->text().trimmed()},
        {"compAddress", ui->addressEdit->toPlainText().trimmed()},
        {"username",    ui->usernameEdit->text().trimmed()},
        {"password",    ui->passwordEdit->text()},
        {"rePassword",  ui->rePasswordEdit->text()},
        {"fullname",    ui->fullnameEdit->text().trimmed()},
        {"telpUser",    ui->telpUserEdit->text().trimmed()},
        {"addrUser",    ui->addressUserEdit->toPlainText().trimmed()}
    };

    // === 2. Validasi Input ===
    QStringList errors;

    if (params["dbDir"].isEmpty())        errors << "• Lokasi penyimpanan database";
    if (params["compName"].isEmpty())     errors << "• Nama perusahaan";
    if (params["compTelp"].isEmpty())     errors << "• Nomor telepon perusahaan";
    if (params["compEmail"].isEmpty())    errors << "• Email perusahaan";
    if (params["compAddress"].isEmpty())  errors << "• Alamat perusahaan";
    if (params["username"].isEmpty())     errors << "• Username Super Admin";
    if (params["password"].isEmpty())     errors << "• Password";
    if (params["rePassword"].isEmpty())   errors << "• Verifikasi Password";
    if (params["fullname"].isEmpty())     errors << "• Nama lengkap admin";
    if (params["telpUser"].isEmpty())     errors << "• Nomor telepon admin";
    if (params["addrUser"].isEmpty())     errors << "• Alamat admin";

    if (!errors.isEmpty()) {
        QMessageBox::warning(this, "Field Belum Lengkap",
            "Pastikan semua field berikut telah diisi:\n\n" + errors.join("\n"));
        return;
    }

    // Validasi tambahan
    if (params["password"].length() < 6) {
        QMessageBox::warning(this, "Password Terlalu Pendek", 
                             "Password minimal harus 6 karakter.");
        return;
    }

    if (params["password"] != params["rePassword"]) {
        QMessageBox::warning(this, "Password Tidak Cocok", 
                             "Password dan verifikasi password tidak sama.");
        return;
    }

    // Validasi sederhana email (opsional tapi direkomendasikan)
    QRegularExpression emailRegex(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    if (!emailRegex.match(params["compEmail"]).hasMatch()) {
        QMessageBox::warning(this, "Format Email Salah", 
                             "Mohon masukkan alamat email perusahaan yang valid.");
        return;
    }

    // === 3. Persiapan Path Database ===
    QDir dbDirectory(params["dbDir"]);
    if (!dbDirectory.exists() && !dbDirectory.mkpath(".")) {
        QMessageBox::critical(this, "Gagal Membuat Folder", 
                              "Tidak dapat membuat folder database di lokasi:\n" + params["dbDir"]);
        return;
    }

    QString dbFilePath = dbDirectory.absoluteFilePath("LAdmins.db");

    // === 4. Inisialisasi Database ===
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "SetupConnection");
    db.setDatabaseName(dbFilePath);

    if (!db.open()) {
        QMessageBox::critical(this, "Gagal Membuka Database", 
                              "Tidak dapat membuat file database:\n" + db.lastError().text());
        return;
    }
    
    auto &dbm = DatabaseManager::instance();
    
    // Jalankan skema database lengkap
    if (!dbm.initSchema(db)) {
        db.close();
        qDebug() << db.lastError().text();
        emit setupFailed();
        QSqlDatabase::removeDatabase("SetupConnection");
        return;   // Pesan error sudah ditampilkan di dalam initializeDatabase()
    }
    dbm.setDatabase(db);
    BaseManager::connection = db;
    // === 5. Simpan Pengaturan Perusahaan ===
    auto app_sm = AppSettingsManager();
    app_sm.saveSettings("company_name", {{"setting_value", params["compName"]}, {"data_type", "string"}, {"description", "Nama Perusahaan"}, {"updated_by", 1}});
    app_sm.saveSettings("company_address", {{"setting_value", params["compAddress"]}, {"data_type", "string"}, {"description", "Nama Perusahaan"}, {"updated_by", 1}});
    app_sm.saveSettings("company_phone", {{"setting_value", params["compTelp"]}, {"data_type", "string"}, {"description", "Nama Perusahaan"}, {"updated_by", 1}});
    app_sm.saveSettings("company_email", {{"setting_value", params["compEmail"]}, {"data_type", "string"}, {"description", "Nama Perusahaan"}, {"updated_by", 1}});
    
    // === 6. Buat Akun Super Admin ===
    auto adm = AdminManager();
    QVariantMap va {
      { "username",         params["username"] },
      { "role_id",          1 },
      { "literal_password", params["password"] },
      { "nama_lengkap",     params["fullname"] },
      { "email",            params["compEmail"] },
      { "nomor_telp",       params["telpUser"] },
      { "is_active",        QVariant(true)} 
    };

    auto opt_adm = adm.create(va);
    if (!opt_adm.has_value()) {
      QMessageBox::critical(this, "Gagal Menyimpan Pengaturan", 
                            "Gagal membuat Super_Admin:\n" + adm.errorString());
      db.close();
      return ;
    }

    // === 7. Simpan Path Database ke QSettings ===
    QSettings settings;
    settings.setValue("Database/databasePath", dbFilePath);
    settings.sync();
    // === 8. Berhasil - Tutup Setup dan Lanjut ke MainWindow ===
    QMessageBox::information(this, "Setup Berhasil", 
                             "Database dan akun Super Admin telah berhasil dibuat.\n\n"
                             "Aplikasi akan melanjutkan ke halaman utama.");

    emit setupFinished();     // Signal yang sudah Anda definisikan
    // close() akan dilakukan di main() melalui lambda yang terhubung ke signal ini
}