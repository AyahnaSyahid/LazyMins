// license_helper.h
// Public API untuk library license_helper (statis).
// Link project kamu ke liblicense_helper.a + libssl.a/libcrypto.a (OpenSSL).
#pragma once

#include <string>
#include <vector>
#include <optional>

namespace license {

// Struktur data token. `signature` diisi otomatis oleh signToken()
// dan dibaca kembali oleh verifyTokenFile()/scanTokenFolder().
struct TokenData {
    std::string token_id;
    std::string issuer;
    std::string product;
    std::string register_date;   // format: yyyy-MM-dd
    std::string valid_until;     // format: yyyy-MM-dd
    std::string registered_by;
    std::string registered_for;
    std::string license_type;
    std::vector<std::string> features; // opsional, boleh kosong
    std::string hardware_id;           // opsional; kosong = tidak dikunci ke mesin tertentu
    std::string signature;             // base64, JANGAN diisi manual
};

enum class ValidationResult {
    Valid,
    InvalidSignature,
    Expired,
    HardwareMismatch,   // token.hardware_id tidak kosong tapi tidak cocok dengan mesin ini
    ParseError,
    FileNotFound,
    KeyError
};

// Konversi enum ke string untuk logging/print.
const char* toString(ValidationResult r);

// ------------------------------------------------------------------
// HARDWARE ID
// Windows : volume serial number drive C:\ (format 8 karakter hex, misal "A1B2C3D4").
// Linux   : fallback ke /etc/machine-id, HANYA untuk keperluan testing lintas platform.
// Melempar std::runtime_error jika gagal membaca.
// ------------------------------------------------------------------
std::string getHardwareId();

// ------------------------------------------------------------------
// SIGNING
// Dipakai HANYA di tool internal kamu yang memegang private_key.pem.
// JANGAN pernah link fungsi ini ke aplikasi yang didistribusikan ke user.
// Mengembalikan string JSON siap tulis ke file (sudah termasuk signature).
// Melempar std::runtime_error jika gagal (key tidak valid, dsb).
// ------------------------------------------------------------------
std::string signToken(TokenData token, const std::string& privateKeyPemPath);

// ------------------------------------------------------------------
// VERIFICATION (PEM file path — existing API, unchanged)
// Dipakai di aplikasi yang didistribusikan, hanya butuh public_key.pem.
// outToken (opsional) akan diisi field-field token jika parsing berhasil,
// walau signature ternyata invalid (agar kamu tetap bisa lihat isi token).
// ------------------------------------------------------------------
ValidationResult verifyTokenFile(const std::string& tokenFilePath,
                                  const std::string& publicKeyPemPath,
                                  TokenData* outToken = nullptr);

// Verifikasi langsung dari string JSON (misal token diambil dari network/DB,
// bukan dari file lokal).
ValidationResult verifyTokenString(const std::string& jsonContent,
                                    const std::string& publicKeyPemPath,
                                    TokenData* outToken = nullptr);

// ------------------------------------------------------------------
// FOLDER SCAN
// Scan semua file berekstensi tertentu di sebuah folder dan validasi semua.
// ------------------------------------------------------------------
struct ScanEntry {
    std::string filepath;
    ValidationResult result;
    TokenData token; // hanya valid jika parsing berhasil
};

std::vector<ScanEntry> scanTokenFolder(const std::string& folderPath,
                                        const std::string& publicKeyPemPath = "",
                                        const std::string& extension = ".tok");

// ------------------------------------------------------------------
// EMBEDDED PUBLIC KEY (baru — untuk aplikasi distribusi tanpa file PEM terpisah)
// Inisialisasi satu kali dari DER bytes public key. Public key dicache
// di dalam library; semua verify*Embedded() berikutnya pakai cache ini.
// Thread safety: inisialisasi harus dilakukan sebelum verify call,
// idealnya di startup single-threaded.
// ------------------------------------------------------------------

// Inisialisasi public key dari DER bytes. Melempar std::runtime_error
// jika DER tidak valid atau bukan public key ECDSA yang didukung.
void initEmbeddedPublicKey(std::string_view derBytes);

// Hapus public key yang dicache. Setelah dipanggil, verify*Embedded()
// akan mengembalikan KeyError sampai initEmbeddedPublicKey() dipanggil lagi.
void clearEmbeddedPublicKey();

// Verifikasi token dari string JSON menggunakan embedded public key.
// outToken (opsional) diisi jika parsing berhasil.
// Melemmpat std::runtime_error jika belum diinisialisasi (akan return KeyError
// tanpa throw — lihat implementasi).
ValidationResult verifyTokenStringEmbedded(std::string_view json,
                                            TokenData* outToken = nullptr);

// Verifikasi token dari file .token menggunakan embedded public key.
ValidationResult verifyTokenFileEmbedded(std::string_view filePath,
                                         TokenData* outToken = nullptr);

// ------------------------------------------------------------------
// TRIAL RECORD (baru — untuk offline trial clock)
// Menulis/membaca file .lm_trial yang ditandatangani HMAC-SHA256.
// ------------------------------------------------------------------

// Struktur hasil baca trial record.
struct TrialRecord {
    std::string install_date;   // tanggal install (format yyyy-MM-dd)
    bool valid;                  // true jika HMAC cocok (tidak dimodifikasi)
};

// Tulis trial record baru dengan install_date yang diberikan.
// File disimpan di per-user AppData path dengan nama ".lm_trial".
// Mengembalikan absolute path file yang ditulis.
// Melempar std::runtime_error jika gagal menulis.
std::string writeTrialRecord(std::string_view installDate);

// Baca trial record dari file. Mengembalikan nullopt jika file tidak ada
// atau HMAC tidak cocok (file rusak/dimodifikasi).
std::optional<TrialRecord> readTrialRecord();

} // namespace license
