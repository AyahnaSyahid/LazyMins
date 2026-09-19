// licensegate.cpp
#include "licensegate.h"

#include "license_helper.h"
#include "embedded_pubkey.h"

#include <QApplication>
#include <QStandardPaths>
#include <QFile>
#include <QMessageBox>
#include <QFileDialog>
#include <QDate>
#include <QDir>
#include <QWidget>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QPushButton>
#include <QDialog>
#include <QLabel>

#include <memory>

namespace licensegate {

namespace {

// ————————————————————————————————————————————————————————————————————
// State yang disimpan setelah initialize() dipanggil
// ————————————————————————————————————————————————————————————————————
struct State {
    GateResult result;
    bool initialized = false;
};

State g_state;

// ————————————————————————————————————————————————————————————————————
// Helper: buka file .lm_token, baca isi, verifikasi dengan embedded key
// ————————————————————————————————————————————————————————————————————
bool verifyTokenAtPath(const QString &path, license::TokenData *outToken) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    QString json = QString::fromUtf8(f.readAll());
    f.close();

    license::ValidationResult vr = license::verifyTokenFileEmbedded(
        path.toStdString(), outToken);
    return (vr == license::ValidationResult::Valid);
}

// ————————————————————————————————————————————————————————————————————
// Helper: dapatkan path file token (mengikuti QSettings::IniFormat behavior)
// ————————————————————————————————————————————————————————————————————
QString tokenFilePath() {
    QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (appData.isEmpty()) {
        return QString();
    }
    QDir dir(appData);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    return dir.filePath(".lm_token");
}

// ————————————————————————————————————————————————————————————————————
// Helper: hitung hari antara install date dan hari ini
// ————————————————————————————————————————————————————————————————————
int daysSinceInstall(const QString &installDateStr) {
    QDate install = QDate::fromString(installDateStr, "yyyy-MM-dd");
    if (!install.isValid()) {
        return 0;
    }
    QDate today = QDate::currentDate();
    return install.daysTo(today); // negative jika today < install (tidak mungkin)
}

// ————————————————————————————————————————————————————————————————————
// Helper: ambil HWID
// ————————————————————————————————————————————————————————————————————
QString getHwid() {
    try {
        std::string h = license::getHardwareId();
        return QString::fromStdString(h);
    } catch (const std::exception &e) {
        return QString("Error membaca HWID: %1").arg(e.what());
    }
}

} // namespace

// ————————————————————————————————————————————————————————————————————
// initialize()
// ————————————————————————————————————————————————————————————————————
void initialize() {
    if (g_state.initialized) {
        return;
    }

    // 1. Load embedded public key
    try {
        license::initEmbeddedPublicKey(
            std::string_view(
                reinterpret_cast<const char *>(embedded::PUBLIC_KEY_DER),
                embedded::PUBLIC_KEY_DER_LEN));
    } catch (const std::exception &e) {
        g_state.result.state = GateState::Error;
        g_state.result.errorMessage = QString("Gagal memuat public key: %1").arg(e.what());
        g_state.initialized = true;
        return;
    }

    // 2. Baca trial record
    auto trialOpt = license::readTrialRecord();
    GateResult res;
    res.hardwareId = getHwid();

    if (!trialOpt.has_value()) {
        // Tidak ada trial record → ini instalasi pertama
        // Tulis trial record baru dengan tanggal hari ini
        QString todayStr = QDate::currentDate().toString("yyyy-MM-dd");
        try {
            std::string path = license::writeTrialRecord(todayStr.toStdString());
            // path dikembalikan tapi kita tidak butuh nilai kembali di sini
        } catch (const std::exception &e) {
            res.state = GateState::Error;
            res.errorMessage = QString("Gagal menulis trial record: %1").arg(e.what());
            g_state.result = res;
            g_state.initialized = true;
            return;
        }
        res.installDate = todayStr;
        res.daysUsed = 0;
        // Lanjut ke pemeriksaan token di bawah
    } else {
        const auto &rec = trialOpt.value();
        res.installDate = QString::fromStdString(rec.install_date);
        res.daysUsed = daysSinceInstall(res.installDate);
        if (!rec.valid) {
            // Trial record ada tapi dimodifikasi (HMAC tidak cocok)
            res.state = GateState::Error;
            res.errorMessage = "Trial record tidak valid (dimodifikasi). "
                               "Uninstall dan install ulang untuk memulai ulang.";
            g_state.result = res;
            g_state.initialized = true;
            return;
        }
    }

    // 3. Cek token file
    QString tokenPath = tokenFilePath();
    res.tokenPath = tokenPath;

    if (!tokenPath.isEmpty()) {
        QFileInfo fi(tokenPath);
        if (fi.exists() && fi.isFile()) {
            license::TokenData tokenData;
            if (verifyTokenAtPath(tokenPath, &tokenData)) {
                // Token valid → licensed
                res.state = GateState::Licensed;
                g_state.result = res;
                g_state.initialized = true;
                return;
            } else {
                // Token ada tapi tidak valid
                res.state = GateState::Error;
                res.errorMessage = "File token tidak valid atau telah expired. "
                                   "Silakan hapus file token dan request ulang ke developer.";
                g_state.result = res;
                g_state.initialized = true;
                return;
            }
        }
    }

    // 4. Belum ada token → tentukan berdasarkan trial days
    int days = res.daysUsed;
    if (days >= 90) {
        res.state = GateState::Blocked;
    } else if (days >= 60) {
        res.state = GateState::Reminder;
    } else {
        res.state = GateState::Trial;
    }

    g_state.result = res;
    g_state.initialized = true;
}

// ————————————————————————————————————————————————————————————————————
// result()
// ————————————————————————————————————————————————————————————————————
GateResult result() {
    if (!g_state.initialized) {
        // Belum diinisialisasi → kembalikan state kosong
        GateResult res;
        res.state = GateState::Trial;
        res.daysUsed = 0;
        res.hardwareId = getHwid();
        return res;
    }
    return g_state.result;
}

// ————————————————————————————————————————————————————————————————————
// showReminder()
// ————————————————————————————————————————————————————————————————————
void showReminder(int daysUsed, const QString &hardwareId, QWidget *parent) {
    QMessageBox::information(
        parent,
        "Reminder Lisensi",
        QString("Anda telah menggunakan aplikasi ini selama %1 hari.\n\n"
                "Mohon hubungi developer untuk mendapatkan file token lisensi.\n\n"
                "HWID mesin ini:\n%2\n\n"
                "Kirim HWID di atas ke developer untuk meminta file token."
        ).arg(daysUsed).arg(hardwareId),
        QMessageBox::Ok | QMessageBox::Ignore,
        QMessageBox::Ignore
    );
}

// ————————————————————————————————————————————————————————————————————
// showBlockDialog()
// ————————————————————————————————————————————————————————————————————
bool showBlockDialog(const QString &hardwareId, QWidget *parent) {
    QDialog dlg(parent);
    dlg.setWindowTitle("Lisensi Diperlukan");
    dlg.setModal(true);
    dlg.setMinimumWidth(480);

    QVBoxLayout *layout = new QVBoxLayout(&dlg);

    QTextEdit *msg = new QTextEdit(&dlg);
    msg->setReadOnly(true);
    msg->setPlainText(
        QString("Periode trial aplikasi telah berakhir (90 hari).\n\n"
                "Anda harus memuat file token lisensi yang valid untuk "
                "melanjutkan penggunaan aplikasi.\n\n"
                "HWID mesin ini:\n%1\n\n"
                "Kirim HWID di atas ke developer untuk mendapatkan file token, "
                "kemudian pilih file tersebut di bawah."
        ).arg(hardwareId)
    );
    layout->addWidget(msg);

    QPushButton *btnBrowse = new QPushButton("Pilih File Token (.lm_token)", &dlg);
    layout->addWidget(btnBrowse);

    QPushButton *btnCancel = new QPushButton("Keluar", &dlg);
    layout->addWidget(btnCancel);

    QLabel *statusLabel = new QLabel(&dlg);
    statusLabel->setWordWrap(true);
    layout->addWidget(statusLabel);

    QObject::connect(btnBrowse, &QPushButton::clicked, [&]() {
        QString filePath = QFileDialog::getOpenFileName(
            &dlg,
            "Pilih File Token",
            QStandardPaths::writableLocation(QStandardPaths::AppDataLocation),
            "File Token (*.lm_token);;Semua File (*)"
        );
        if (filePath.isEmpty()) {
            statusLabel->setText("Pemilihan file dibatalkan.");
            return;
        }
        license::TokenData tokenData;
        bool ok = verifyTokenAtPath(filePath, &tokenData);
        if (ok) {
            // Token valid: simpan ke path default biar tidak perlu pilih lagi
            QString defaultPath = tokenFilePath();
            if (!defaultPath.isEmpty()) {
                QFile src(filePath);
                QFile dst(defaultPath);
                if (src.open(QIODevice::ReadOnly) && dst.open(QIODevice::WriteOnly)) {
                    dst.write(src.readAll());
                    src.close();
                    dst.close();
                }
            }
            statusLabel->setText(QString("Token valid! Token disimpan di:\n%1").arg(defaultPath));
            btnBrowse->setEnabled(false);
            btnCancel->setText("Lanjutkan");
        } else {
            statusLabel->setText("File token tidak valid atau expired. Silakan pilih file yang berbeda.");
        }
    });

    QObject::connect(btnCancel, &QPushButton::clicked, [&]() {
        dlg.accept(); // tutup dialog, showBlockDialog kembalikan false
    });

    // Jika sudah ada token yang berhasil dimuat sebelumnya (dari tryLoadToken),
    // langsung tutup dengan status sukses
    if (g_state.result.state == GateState::Licensed) {
        dlg.accept();
        return true;
    }

    int ret = dlg.exec();
    return (ret == QDialog::Accepted && g_state.result.state == GateState::Licensed);
}

// ————————————————————————————————————————————————————————————————————
// tryLoadToken()
// ————————————————————————————————————————————————————————————————————
bool tryLoadToken(const QString &tokenPath) {
    license::TokenData tokenData;
    bool ok = verifyTokenAtPath(tokenPath, &tokenData);
    if (ok) {
        // Simpan ke path default
        QString defaultPath = tokenFilePath();
        if (!defaultPath.isEmpty() && defaultPath != tokenPath) {
            QFile src(tokenPath);
            QFile dst(defaultPath);
            if (src.open(QIODevice::ReadOnly) && dst.open(QIODevice::WriteOnly)) {
                dst.write(src.readAll());
                src.close();
                dst.close();
            }
        }
        g_state.result.state = GateState::Licensed;
        g_state.result.tokenPath = defaultPath.isEmpty() ? tokenPath : defaultPath;
    }
    return ok;
}

} // namespace licensegate
