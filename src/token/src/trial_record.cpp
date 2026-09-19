// trial_record.cpp
// Implementasi trial record: write/read file .lm_trial dengan HMAC-SHA256.
//
// Secret di-embed sebagai constexpr byte array. HMAC dikomputasi dari
// string "install_date=<date>" dan disimpan bersamaan dengan tanggal.
//
// Catatan: implementasi ini bergantung pada OpenSSL untuk HMAC (sudah
// tersedia dari libcrypto yang di-link oleh license_helper) dan std::filesystem
// untuk path handling. Tidak memerlukan Qt.
#include "license_helper.h"

#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <openssl/err.h>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cstring>

namespace fs = std::filesystem;

namespace license {

// ============================================================
// HMAC secret — di-embed dalam binary, tidak dibaca dari file.
// Ganti nilai ini dengan secret production yang aman.
// Ukuran: 32 byte (256 bit), disimpan sebagai array hex.
// ============================================================
static constexpr const unsigned char TRIAL_HMAC_SECRET[] = {
    0x5A, 0x7B, 0x3C, 0x9D, 0xE1, 0xF2, 0x4A, 0x6B,
    0x8C, 0x0D, 0xE3, 0xF5, 0x7A, 0x9B, 0x1C, 0x3E,
    0x4F, 0x6D, 0x8A, 0x0B, 0x2C, 0x4E, 0x6F, 0x8D,
    0x0A, 0x2B, 0x4C, 0x6E, 0x8F, 0x01, 0x23, 0x45
};
static constexpr size_t TRIAL_HMAC_SECRET_LEN = sizeof(TRIAL_HMAC_SECRET);

// Nama file trial record (hidden file di Linux/macOS, visible di Windows)
static constexpr const char* TRIAL_FILENAME = ".lm_trial";

// ============================================================
// Dapatkan path file trial record di per-user AppData location.
// Menggunakan std::filesystem + environment variable fallback.
// ============================================================
static fs::path getTrialRecordPath() {
    // Coba XDG_DATA_HOME (Linux), APPDATA (Windows), atau ~/Library/Application Support (macOS)
    // Fallback ke ~/.local/share atau ~/AppData/Roaming
    std::string baseDir;

#ifdef _WIN32
    const char* appData = std::getenv("APPDATA");
    if (appData && appData[0]) {
        baseDir = appData;
    } else {
        // Fallback ke profile directory
        char buf[260];
        if (GetEnvironmentVariableA("USERPROFILE", buf, sizeof(buf)) > 0) {
            baseDir = buf;
            baseDir += "\\AppData\\Roaming";
        } else {
            baseDir = ".";
        }
    }
#else
    const char* xdgData = std::getenv("XDG_DATA_HOME");
    if (xdgData && xdgData[0]) {
        baseDir = xdgData;
    } else {
        const char* home = std::getenv("HOME");
        if (home && home[0]) {
            baseDir = home;
            baseDir += "/.local/share";
        } else {
            baseDir = ".";
        }
    }
#endif

    fs::path dir(baseDir);
    fs::path fullPath = dir / TRIAL_FILENAME;

    return fullPath;
}

// ============================================================
// HMAC-SHA256 helper
// ============================================================
static std::string opensslErrorStr() {
    unsigned long code = ERR_get_error();
    if (code == 0) return "unknown OpenSSL error";
    char buf[256];
    ERR_error_string_n(code, buf, sizeof(buf));
    return std::string(buf);
}

static std::string hmacSha256Hex(const std::string& data) {
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hashLen = 0;

    if (!HMAC(EVP_sha256(),
              TRIAL_HMAC_SECRET, TRIAL_HMAC_SECRET_LEN,
              reinterpret_cast<const unsigned char*>(data.data()),
              data.size(), hash, &hashLen)) {
        throw std::runtime_error("HMAC-SHA256 gagal: " + opensslErrorStr());
    }

    // Encode ke hex string (128 char untuk SHA-256 = 32 byte)
    std::string hex;
    hex.reserve(hashLen * 2);
    for (unsigned int i = 0; i < hashLen; ++i) {
        char buf[4];
        snprintf(buf, sizeof(buf), "%02x", hash[i]);
        hex += buf;
    }
    return hex;
}

// ============================================================
// Public API: writeTrialRecord
// ============================================================
std::string writeTrialRecord(std::string_view installDate) {
    fs::path filePath = getTrialRecordPath();

    // Buat parent directory jika belum ada
    fs::path parent = filePath.parent_path();
    if (!parent.empty() && !fs::exists(parent)) {
        std::error_code ec;
        fs::create_directories(parent, ec);
        if (ec) {
            throw std::runtime_error("Gagal membuat directory: " + parent.string() +
                                     " (" + ec.message() + ")");
        }
    }

    // Build konten: install_date\nHMAC
    std::string contentLine = "install_date=" + std::string(installDate);
    std::string hmac = hmacSha256Hex(contentLine);

    std::ofstream out(filePath, std::ios::binary);
    if (!out) {
        throw std::runtime_error("Gagal membuka file trial record untuk menulis: " +
                                 filePath.string());
    }

    out << contentLine << "\n" << hmac << "\n";
    out.close();

    if (!out) {
        fs::remove(filePath); // coba bersihkan file partial
        throw std::runtime_error("Gagal menulis file trial record: " + filePath.string());
    }

    return filePath.string();
}

// ============================================================
// Public API: readTrialRecord
// ============================================================
std::optional<TrialRecord> readTrialRecord() {
    fs::path filePath = getTrialRecordPath();

    if (!fs::exists(filePath) || !fs::is_regular_file(filePath)) {
        return std::nullopt;
    }

    std::ifstream in(filePath, std::ios::binary);
    if (!in) {
        return std::nullopt;
    }

    std::string line1, line2;
    if (!std::getline(in, line1) || !std::getline(in, line2)) {
        return std::nullopt; // file tidak cukup lengkap
    }

    // Parse install_date dari "install_date=<date>"
    if (line1.rfind("install_date=", 0) != 0) {
        return std::nullopt; // format tidak sesuai
    }
    std::string installDate = line1.substr(std::string("install_date=").size());

    // Verifikasi HMAC
    std::string expectedHmac = hmacSha256Hex(line1);
    if (line2 != expectedHmac) {
        // HMAC tidak cocok — file dimodifikasi
        return TrialRecord{installDate, false};
    }

    return TrialRecord{installDate, true};
}

} // namespace license
