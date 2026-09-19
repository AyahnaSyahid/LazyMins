// licensegate.h
// Gate untuk validasi offline instalasi: trial clock + token verification.
//
// Dipanggil sekali di startup (main.cpp), sebelum MainWindow ditampilkan.
// Menentukan apakah aplikasi masuk mode trial, licensed, reminder, atau blocked.
#pragma once

#include <QString>
#include <QDate>

namespace licensegate {

enum class GateState {
    Trial,        // belum ada token, masih dalam periode trial (< 60 hari)
    Reminder,     // belum ada token, sudah 60-90 hari, tampilkan reminder
    Blocked,      // belum ada token, sudah >= 90 hari, app harus di-block
    Licensed,     // ada token valid
    Error         // ada token tapi tidak valid, atau error membaca trial record
};

struct GateResult {
    GateState state;
    int daysUsed;           // 0 jika tidak ada trial record
    QString installDate;    // "yyyy-MM-dd" atau kosong
    QString tokenPath;      // path file token yang diverifikasi (atau kosong)
    QString errorMessage;   // deskripsi error (atau kosong)
    QString hardwareId;     // HWID mesin ini (untuk dikirim ke developer)
};

// Inisialisasi sekali di startup. Memuat public key terenkapsul dan
// membaca trial record + token file. Pemanggilan pertama adalah yang
// menentukan state; pemanggilan berikutnya hanya membaca state yang tersimpan.
void initialize();

// Kembalikan hasil gateway setelah initialize() dipanggil.
GateResult result();

// Tampilkan dialog reminder (non-blocking) untuk pengguna yang belum
// memiliki token tapi sudah melewati 60 hari. Pemanggil bertanggung jawab
// atas parent widget.
void showReminder(int daysUsed, const QString &hardwareId, QWidget *parent);

// Tampilkan dialog blocking untuk pengguna yang sudah >= 90 hari tanpa token.
// Mengembalikan true jika pengguna berhasil memuat token valid, false jika
// membatalkan/dialog ditutup.
bool showBlockDialog(const QString &hardwareId, QWidget *parent);

// Coba verifikasi ulang token dari path yang diberikan pengguna (untuk
// dialog block yang memungkinkan browse file). Mengembalikan true jika
// token valid.
bool tryLoadToken(const QString &tokenPath);

} // namespace licensegate
