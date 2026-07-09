// license_helper.cpp
#include "license_helper.h"

#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/err.h>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <ctime>
#include <cstring>
#include <cctype>
#include <cstdio>

#ifdef _WIN32
  #include <windows.h>
#endif

namespace fs = std::filesystem;

namespace license {

// ============================================================
// Util: ambil pesan error OpenSSL terakhir (untuk debugging)
// ============================================================
static std::string opensslLastError() {
    unsigned long code = ERR_get_error();
    if (code == 0) return "unknown OpenSSL error";
    char buf[256];
    ERR_error_string_n(code, buf, sizeof(buf));
    return std::string(buf);
}

// ============================================================
// Base64 (pakai OpenSSL EVP_En/DecodeBlock, tanpa dependency lain)
// ============================================================
static std::string base64Encode(const std::vector<unsigned char>& data) {
    if (data.empty()) return "";
    int outLen = 4 * ((static_cast<int>(data.size()) + 2) / 3);
    std::vector<unsigned char> out(outLen + 1, 0);
    int written = EVP_EncodeBlock(out.data(), data.data(), static_cast<int>(data.size()));
    return std::string(reinterpret_cast<char*>(out.data()), written);
}

static std::vector<unsigned char> base64Decode(const std::string& in) {
    if (in.empty()) return {};
    std::vector<unsigned char> out(in.size()); // cukup besar
    int written = EVP_DecodeBlock(out.data(),
                                   reinterpret_cast<const unsigned char*>(in.data()),
                                   static_cast<int>(in.size()));
    if (written < 0) throw std::runtime_error("base64 decode gagal");

    // EVP_DecodeBlock tidak menghitung padding '=' dengan benar, koreksi manual:
    int padding = 0;
    if (in.size() >= 1 && in[in.size() - 1] == '=') padding++;
    if (in.size() >= 2 && in[in.size() - 2] == '=') padding++;
    out.resize(written - padding);
    return out;
}

// ============================================================
// Mini JSON writer/parser KHUSUS untuk skema TokenData.
// Ini BUKAN parser JSON umum -- sengaja sederhana & tanpa dependency
// supaya mudah di-link ke proyek apapun. Jangan pakai untuk JSON lain.
// ============================================================
static std::string jsonEscape(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            default:   out += c;      break;
        }
    }
    return out;
}

static std::string jsonUnescape(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '\\' && i + 1 < s.size()) {
            char n = s[i + 1];
            switch (n) {
                case '"':  out += '"';  i++; break;
                case '\\': out += '\\'; i++; break;
                case 'n':  out += '\n'; i++; break;
                case 'r':  out += '\r'; i++; break;
                case 't':  out += '\t'; i++; break;
                default:   out += s[i];      break;
            }
        } else {
            out += s[i];
        }
    }
    return out;
}

std::string tokenToJson(const TokenData& t) {
    std::ostringstream o;
    o << "{\n";
    o << "  \"token_id\": \""        << jsonEscape(t.token_id)        << "\",\n";
    o << "  \"issuer\": \""          << jsonEscape(t.issuer)          << "\",\n";
    o << "  \"product\": \""         << jsonEscape(t.product)        << "\",\n";
    o << "  \"register_date\": \""   << jsonEscape(t.register_date)  << "\",\n";
    o << "  \"valid_until\": \""     << jsonEscape(t.valid_until)    << "\",\n";
    o << "  \"registered_by\": \""   << jsonEscape(t.registered_by)  << "\",\n";
    o << "  \"registered_for\": \""  << jsonEscape(t.registered_for) << "\",\n";
    o << "  \"license_type\": \""    << jsonEscape(t.license_type)   << "\",\n";
    o << "  \"features\": [";
    for (size_t i = 0; i < t.features.size(); ++i) {
        o << "\"" << jsonEscape(t.features[i]) << "\"";
        if (i + 1 < t.features.size()) o << ", ";
    }
    o << "],\n";
    o << "  \"hardware_id\": \""     << jsonEscape(t.hardware_id)    << "\",\n";
    o << "  \"signature\": \""       << jsonEscape(t.signature)      << "\"\n";
    o << "}\n";
    return o.str();
}

// Cari value string untuk "key": "value" di dalam teks JSON sederhana.
static bool findStringField(const std::string& json, const std::string& key, std::string& out) {
    std::string pattern = "\"" + key + "\"";
    size_t pos = json.find(pattern);
    if (pos == std::string::npos) return false;
    pos = json.find(':', pos);
    if (pos == std::string::npos) return false;
    pos = json.find('"', pos);
    if (pos == std::string::npos) return false;
    size_t start = pos + 1;
    size_t end = start;
    std::string raw;
    while (end < json.size()) {
        if (json[end] == '\\') { raw += json[end]; raw += json[end + 1]; end += 2; continue; }
        if (json[end] == '"') break;
        raw += json[end];
        end++;
    }
    out = jsonUnescape(raw);
    return true;
}

// Ambil array string untuk "key": ["a", "b"]
static bool findStringArrayField(const std::string& json, const std::string& key,
                                  std::vector<std::string>& out) {
    std::string pattern = "\"" + key + "\"";
    size_t pos = json.find(pattern);
    if (pos == std::string::npos) return false;
    size_t arrStart = json.find('[', pos);
    size_t arrEnd = json.find(']', arrStart);
    if (arrStart == std::string::npos || arrEnd == std::string::npos) return false;

    std::string inner = json.substr(arrStart + 1, arrEnd - arrStart - 1);
    size_t i = 0;
    while (i < inner.size()) {
        size_t q1 = inner.find('"', i);
        if (q1 == std::string::npos) break;
        size_t q2 = q1 + 1;
        std::string raw;
        while (q2 < inner.size() && inner[q2] != '"') {
            if (inner[q2] == '\\') { raw += inner[q2]; raw += inner[q2 + 1]; q2 += 2; continue; }
            raw += inner[q2];
            q2++;
        }
        out.push_back(jsonUnescape(raw));
        i = q2 + 1;
    }
    return true;
}

static bool jsonToToken(const std::string& json, TokenData& t) {
    bool ok = true;
    ok &= findStringField(json, "token_id", t.token_id);
    ok &= findStringField(json, "issuer", t.issuer);
    ok &= findStringField(json, "product", t.product);
    ok &= findStringField(json, "register_date", t.register_date);
    ok &= findStringField(json, "valid_until", t.valid_until);
    ok &= findStringField(json, "registered_by", t.registered_by);
    ok &= findStringField(json, "registered_for", t.registered_for);
    ok &= findStringField(json, "license_type", t.license_type);
    ok &= findStringField(json, "signature", t.signature);
    findStringArrayField(json, "features", t.features);      // opsional, boleh gagal
    findStringField(json, "hardware_id", t.hardware_id);      // opsional, boleh gagal (backward-compat)
    return ok;
}

// ============================================================
// Canonicalization: urutan field TETAP, dipakai sebagai input sign/verify.
// signature TIDAK ikut serta (karena itu hasil, bukan input).
// ============================================================
static std::string canonicalize(const TokenData& t) {
    std::ostringstream o;
    o << "token_id="       << t.token_id       << "|"
      << "issuer="         << t.issuer         << "|"
      << "product="        << t.product        << "|"
      << "register_date="  << t.register_date  << "|"
      << "valid_until="    << t.valid_until    << "|"
      << "registered_by="  << t.registered_by  << "|"
      << "registered_for=" << t.registered_for << "|"
      << "license_type="   << t.license_type   << "|"
      << "features=";
    for (size_t i = 0; i < t.features.size(); ++i) {
        o << t.features[i];
        if (i + 1 < t.features.size()) o << ",";
    }
    o << "|hardware_id=" << t.hardware_id;
    return o.str();
}

// ============================================================
// Load key dari file PEM
// ============================================================
static EVP_PKEY* loadKeyFromFile(const std::string& path, bool isPrivate) {
    FILE* fp = fopen(path.c_str(), "rb");
    if (!fp) return nullptr;

    EVP_PKEY* key = isPrivate
        ? PEM_read_PrivateKey(fp, nullptr, nullptr, nullptr)
        : PEM_read_PUBKEY(fp, nullptr, nullptr, nullptr);

    fclose(fp);
    return key;
}

// ============================================================
// Sign / Verify mentah (bytes)
// ============================================================
static std::vector<unsigned char> rawSign(const std::string& data, EVP_PKEY* key) {
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) throw std::runtime_error("EVP_MD_CTX_new gagal");

    std::vector<unsigned char> sig;
    size_t sigLen = 0;

    if (EVP_DigestSignInit(ctx, nullptr, EVP_sha256(), nullptr, key) <= 0) {
        EVP_MD_CTX_free(ctx);
        throw std::runtime_error("DigestSignInit gagal: " + opensslLastError());
    }
    if (EVP_DigestSignUpdate(ctx, data.data(), data.size()) <= 0) {
        EVP_MD_CTX_free(ctx);
        throw std::runtime_error("DigestSignUpdate gagal: " + opensslLastError());
    }
    if (EVP_DigestSignFinal(ctx, nullptr, &sigLen) <= 0) {
        EVP_MD_CTX_free(ctx);
        throw std::runtime_error("DigestSignFinal (size) gagal: " + opensslLastError());
    }
    sig.resize(sigLen);
    if (EVP_DigestSignFinal(ctx, sig.data(), &sigLen) <= 0) {
        EVP_MD_CTX_free(ctx);
        throw std::runtime_error("DigestSignFinal gagal: " + opensslLastError());
    }
    sig.resize(sigLen);
    EVP_MD_CTX_free(ctx);
    return sig;
}

static bool rawVerify(const std::string& data, const std::vector<unsigned char>& sig, EVP_PKEY* key) {
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) return false;

    bool ok = false;
    if (EVP_DigestVerifyInit(ctx, nullptr, EVP_sha256(), nullptr, key) > 0) {
        if (EVP_DigestVerifyUpdate(ctx, data.data(), data.size()) > 0) {
            int r = EVP_DigestVerifyFinal(ctx, sig.data(), sig.size());
            ok = (r == 1);
        }
    }
    EVP_MD_CTX_free(ctx);
    return ok;
}

// ============================================================
// Hardware ID
// ============================================================
#ifdef _WIN32
std::string getHardwareId() {
    DWORD serialNumber = 0;
    if (!GetVolumeInformationA("C:\\", nullptr, 0, &serialNumber,
                               nullptr, nullptr, nullptr, 0)) {
        throw std::runtime_error("Gagal membaca volume serial number drive C:\\ (error code: "
                                  + std::to_string(GetLastError()) + ")");
    }
    char buf[16];
    snprintf(buf, sizeof(buf), "%08X", static_cast<unsigned int>(serialNumber));
    return std::string(buf);
}
#else
// Fallback HANYA untuk build/test di Linux (misal saat development di sandbox ini).
// Untuk deployment Windows sungguhan, cabang _WIN32 di atas yang dipakai.
std::string getHardwareId() {
    std::ifstream f("/etc/machine-id");
    if (!f) throw std::runtime_error("Tidak bisa membaca /etc/machine-id (fallback non-Windows)");
    std::string id;
    std::getline(f, id);
    if (id.empty()) throw std::runtime_error("/etc/machine-id kosong");
    return id;
}
#endif

// ============================================================
// Cek tanggal expired (format yyyy-MM-dd, dibandingkan ke tanggal hari ini)
// ============================================================
static bool isExpired(const std::string& validUntil) {
    std::tm tm{};
    std::istringstream ss(validUntil);
    ss >> std::get_time(&tm, "%Y-%m-%d");
    if (ss.fail()) throw std::runtime_error("Format valid_until salah, harus yyyy-MM-dd");

    // Set ke akhir hari (23:59:59) supaya tanggal valid_until masih dianggap valid
    // sampai hari itu selesai.
    tm.tm_hour = 23; tm.tm_min = 59; tm.tm_sec = 59;
    std::time_t expiry = std::mktime(&tm);
    std::time_t now = std::time(nullptr);
    return now > expiry;
}

// ============================================================
// Public API
// ============================================================
const char* toString(ValidationResult r) {
    switch (r) {
        case ValidationResult::Valid:            return "VALID";
        case ValidationResult::InvalidSignature:  return "INVALID_SIGNATURE";
        case ValidationResult::Expired:            return "EXPIRED";
        case ValidationResult::HardwareMismatch:    return "HARDWARE_MISMATCH";
        case ValidationResult::ParseError:         return "PARSE_ERROR";
        case ValidationResult::FileNotFound:       return "FILE_NOT_FOUND";
        case ValidationResult::KeyError:           return "KEY_ERROR";
    }
    return "UNKNOWN";
}

std::string signToken(TokenData token, const std::string& privateKeyPemPath) {
    EVP_PKEY* key = loadKeyFromFile(privateKeyPemPath, true);
    if (!key) throw std::runtime_error("Gagal memuat private key: " + privateKeyPemPath);

    std::string canon = canonicalize(token);
    std::vector<unsigned char> sig;
    try {
        sig = rawSign(canon, key);
    } catch (...) {
        EVP_PKEY_free(key);
        throw;
    }
    EVP_PKEY_free(key);

    token.signature = base64Encode(sig);
    return tokenToJson(token);
}

ValidationResult verifyTokenString(const std::string& jsonContent,
                                    const std::string& publicKeyPemPath,
                                    TokenData* outToken) {
    TokenData t;
    if (!jsonToToken(jsonContent, t)) {
        return ValidationResult::ParseError;
    }
    if (outToken) *outToken = t;

    EVP_PKEY* key = loadKeyFromFile(publicKeyPemPath, false);
    if (!key) return ValidationResult::KeyError;

    std::string canon = canonicalize(t);
    std::vector<unsigned char> sig;
    try {
        sig = base64Decode(t.signature);
    } catch (...) {
        EVP_PKEY_free(key);
        return ValidationResult::ParseError;
    }

    bool sigOk = rawVerify(canon, sig, key);
    EVP_PKEY_free(key);

    if (!sigOk) return ValidationResult::InvalidSignature;

    try {
        if (isExpired(t.valid_until)) return ValidationResult::Expired;
    } catch (...) {
        return ValidationResult::ParseError;
    }

    // hardware_id kosong = token tidak dikunci ke mesin tertentu (backward-compat).
    if (!t.hardware_id.empty()) {
        std::string currentHwId;
        try {
            currentHwId = getHardwareId();
        } catch (...) {
            return ValidationResult::KeyError; // gagal baca HW ID mesin ini
        }
        if (currentHwId != t.hardware_id) {
            return ValidationResult::HardwareMismatch;
        }
    }

    return ValidationResult::Valid;
}

ValidationResult verifyTokenFile(const std::string& tokenFilePath,
                                  const std::string& publicKeyPemPath,
                                  TokenData* outToken) {
    if (!fs::exists(tokenFilePath)) return ValidationResult::FileNotFound;

    std::ifstream f(tokenFilePath, std::ios::binary);
    if (!f) return ValidationResult::FileNotFound;

    std::ostringstream ss;
    ss << f.rdbuf();
    return verifyTokenString(ss.str(), publicKeyPemPath, outToken);
}

std::vector<ScanEntry> scanTokenFolder(const std::string& folderPath,
                                        const std::string& publicKeyPemPath,
                                        const std::string& extension) {
    std::vector<ScanEntry> results;
    if (!fs::exists(folderPath) || !fs::is_directory(folderPath)) {
        return results;
    }

    for (const auto& entry : fs::directory_iterator(folderPath)) {
        if (!entry.is_regular_file()) continue;
        if (entry.path().extension() != extension) continue;

        ScanEntry se;
        se.filepath = entry.path().string();
        se.result = verifyTokenFile(se.filepath, publicKeyPemPath, &se.token);
        results.push_back(std::move(se));
    }
    return results;
}

} // namespace license
