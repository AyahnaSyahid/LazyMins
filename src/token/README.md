# license_tool

Static library + executable untuk membuat dan memverifikasi token lisensi
bertanda tangan digital (ECDSA P-256 + SHA-256) menggunakan OpenSSL.

## Isi project

- `include/license_helper.h` — header publik, **ini yang kamu `#include` di project kamu**.
- `src/license_helper.cpp` — implementasi (compile jadi `liblicense_helper.a`).
- `tools/keygen.cpp` — executable generate private/public key pair.
- `tools/sign_demo.cpp` — contoh membuat & menandatangani token (jalankan di tool internal kamu).
- `tools/verify_demo.cpp` — contoh scan folder & verifikasi semua token.

## Build (default, pakai OpenSSL sistem)

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j4
```

Hasil:
- `keygen` — executable
- `liblicense_helper.a` — static library
- `sign_demo`, `verify_demo` — contoh pemakaian

## Build dengan static OpenSSL dari ClamWin

Karena kamu punya build OpenSSL statis sendiri (bukan dari sistem), pakai
variabel `OPENSSL_CUSTOM_*` supaya CMake tidak mencoba mencari OpenSSL sistem:

```bash
cmake .. \
  -DOPENSSL_CUSTOM_INCLUDE=/path/ke/clamwin/openssl/include \
  -DOPENSSL_CUSTOM_SSL_LIB=/path/ke/clamwin/lib/libssl.a \
  -DOPENSSL_CUSTOM_CRYPTO_LIB=/path/ke/clamwin/lib/libcrypto.a
make -j4
```

Ganti path di atas sesuai lokasi file `.a` (Linux/MinGW) atau `.lib` (MSVC)
milik ClamWin kamu. Kalau nama file library-nya beda (misalnya `ssl.lib` dan
`crypto.lib` tanpa prefix `lib`), cukup ganti path lengkapnya saja.

> Catatan: pastikan versi header (`include`) yang dipakai **sama** dengan versi
> `.a`/`.lib` binary-nya. Header dan binary OpenSSL yang beda versi bisa
> menyebabkan crash aneh saat runtime (ABI mismatch), terutama antara OpenSSL
> 1.1.x dan 3.x.

## Cara pakai di project kamu sendiri

1. Copy folder `include/` dan `src/` (atau langsung link `liblicense_helper.a`
   yang sudah dibangun) ke project kamu.
2. `#include "license_helper.h"`
3. Link ke `liblicense_helper.a` + `libssl.a` + `libcrypto.a` (dan
   `ws2_32`, `crypt32` kalau target Windows).

### Generate key pair (sekali saja)

```bash
./keygen private_key.pem public_key.pem
```

`private_key.pem` **jangan pernah** ikut didistribusikan ke aplikasi client —\nhanya dipakai di tool internal kamu untuk membuat token.

### Meng-generate key pair baru untuk production

Jika kamu belum punya key pair production, atau ingin melakukan rotasi key:

```bash
# 1. Generate pasangan kunci baru
./keygen production_private.pem production_public.pem

# 2. Simpan private key di tempat AMAN (bukan di repo, bukan di shared drive)
#    Contoh: module encrypted storage, password-manager, atau USB terenkripsi.
#    JANGAN pernah commit ke git atau ikut sertakan di aplikasi client.

# 3. Convert public key ke DER (untuk embed di aplikasi)
openssl pkey -pubin -in production_public.pem -outform DER -out production_public.der

# 4. Baca byte array dari public.der
xxd -p production_public.der
# Atau di Linux/macOS: hexdump -v -e '/1 "%02x "' production.der

# 5. Salin byte array hasil langkah 4 ke embedded_pubkey.h (atau source code kamu)
```

### Update embedded public key di source code

Setelah generate key pair baru dan punya `public.der`, update file yang berisi byte array:

1. Buka `src/token/include/embedded_pubkey.h` (atau file header yang kamu buat untuk embedded key)
2. Ganti array `PUBLIC_KEY_DER` dengan byte baru dari `public.der`
3. Update `PUBLIC_KEY_DER_LEN` jika ukuran berubah (biasanya tetap 91 byte untuk P-256)
4. Recompile aplikasi yang memuat embedded key

```bash
# Contoh proses update embedded key di project LazyMins:
./keygen production_private.pem production_public.pem
openssl pkey -pubin -in production_public.pem -outform DER -out production_public.der
xxd -p production_public.der   # copy output ini ke embedded_pubkey.h
```

> **Peringatan**: Setelah update embedded public key dan recompile, **semua token lama yang ditandatangani dengan private key lama tidak akan valid lagi**. Untuk migrasi tanpa putus, tetap gunakan private key lama sampai semua client menerima token baru, atau implementasikan multi-key support (lihat catatan di bawah).

### Rotasi key (lanjutan)

Jika kamu ingin rotasi key tanpa memutuskan client yang sudah memiliki token lama:

1. Simpan kedua pasangan key (lama + baru)
2. Update embedded key ke public key **baru**
3. Mulai tanda-tangani token baru dengan private key **baru**
4. Biarkan private key **lama** tetap tersedia di tool internal kamu untuk verifikasi/memperbarui token lama jika perlu

Untuk mendukung **multiple public key** dalam satu binary (verifikasi mencoba semua key sampai menemukan yang cocok), tambahkan array public key dan loop verifikasi. Fitur ini belum terimplementasi di versi saat ini.

### Membuat token (di tool internal kamu)

```cpp
#include "license_helper.h"

license::TokenData token;
token.token_id       = "TKN-0001";
token.issuer         = "MyCompany";
token.product        = "MyApp";
token.register_date  = "2026-07-08";
token.valid_until    = "2027-07-08";
token.registered_by  = "admin";
token.registered_for = "PT Contoh Sejahtera";
token.license_type   = "standard";
token.features       = {"module_a", "module_b"};

std::string json = license::signToken(token, "private_key.pem");
// tulis `json` ke file .token
```

### Verifikasi (di aplikasi yang didistribusikan)

```cpp
#include "license_helper.h"

auto results = license::scanTokenFolder("C:/path/ke/folder/token", "public_key.pem");
for (auto& r : results) {
    if (r.result == license::ValidationResult::Valid) {
        // aktifkan fitur
    } else {
        // tolak, log alasan: license::toString(r.result)
    }
}
```

## Catatan keamanan

- Public key sebaiknya di-**hardcode** sebagai byte array di source code
  (bukan file `.pem` terpisah) agar tidak mudah diganti orang lain saat
  reverse engineering. Kamu bisa generate byte array-nya dari isi
  `public_key.pem` dan taruh langsung sebagai `const char*` di source.
- Parser JSON di `license_helper.cpp` sengaja minimalis (bukan parser umum) —
  hanya menangani skema `TokenData` di atas. Kalau kamu menambah field baru,
  update juga fungsi `tokenToJson()` dan `jsonToToken()` di
  `src/license_helper.cpp`.
- Belum ada hardware/machine binding (sesuai permintaan awal) — kalau nanti
  dibutuhkan, tinggal tambahkan field `hardware_id` ke `TokenData`, ikutkan di
  `canonicalize()`, dan bandingkan dengan fingerprint device saat verifikasi.

## Embedded public key (tanpa file PEM terpisah)

Secara default, verifikasi token memerlukan file `public_key.pem` yang terpisah.
Untuk aplikasi distribusi yang ingin memvalidasi token tanpa dependensi file eksternal,
library mendukung **embedded public key**: public key di-embed sebagai byte array
di dalam binary dan diinisialisasi satu kali saat startup.

### Generate DER bytes dari public key

Setelah membuat key pair dengan `keygen`:

```bash
./keygen private.pem public.pem
openssl pkey -pubin -in public.pem -outform DER -out public.der
```

Baca `public.der` sebagai raw bytes dan embed di source code:

```cpp
// Di source code aplikasi (contoh)
constexpr unsigned char EMBEDDED_PUB_KEY[] = {
    0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2A, 0x86,
    0x48, 0xCE, 0x3D, 0x02, 0x01, 0x06, 0x08, 0x2A,
    // ... sisa byte dari public.der
};
constexpr size_t EMBEDDED_PUB_KEY_LEN = sizeof(EMBEDDED_PUB_KEY);
```

### Inisialisasi dan verifikasi dengan embedded key

```cpp
#include "license_helper.h"

// Panggil sekali di startup (sebelum verifikasi apapun)
license::initEmbeddedPublicKey(
    std::string_view(reinterpret_cast<const char*>(EMBEDDED_PUB_KEY),
                     EMBEDDED_PUB_KEY_LEN));

// Verifikasi token dari string JSON
license::TokenData token;
auto result = license::verifyTokenStringEmbedded(jsonString, &token);
if (result == license::ValidationResult::Valid) {
    // Token valid
}

// Verifikasi token dari file
auto fileResult = license::verifyTokenFileEmbedded("token.lm_token", &token);

// Hapus cached key (misal saat logout/logout schema change)
license::clearEmbeddedPublicKey();
```

### Kapan menggunakan embedded key vs PEM file

- **Embedded key**: aplikasi distribusi ke user akhir, tidak ingin dependensi file PEM
- **PEM file**: tool internal developer, development/testing, key rotation sementara

### Trial record (.lm_trial)

Trial record adalah file yang disimpan di per-user AppData location untuk
memantau durasi trial offline. File ini ditandatangani HMAC-SHA256 dengan
secret yang di-embed dalam binary.

#### API

```cpp
#include "license_helper.h"

// Tulis trial record baru (misal saat pertama kali install)
std::string path = license::writeTrialRecord("2026-09-19");
// path akan bernilai misal: /home/user/.local/share/.lm_trial (Linux)
// atau C:\Users\...\AppData\Roaming\.lm_trial (Windows)

// Baca trial record
auto record = license::readTrialRecord();
if (record) {
    if (record->valid) {
        // File tidak dimodifikasi, install_date = record->install_date
        std::cout << "Installed: " << record->install_date << "\n";
    } else {
        // File dimodifikasi (HMAC tidak cocok) — tindak lanjut sesuai kebijakan
    }
} else {
    // File tidak ada — fresh install
}
```

#### Navigasi file

- **Linux**: `$XDG_DATA_HOME/.lm_trial` atau `~/.local/share/.lm_trial`
- **Windows**: `%APPDATA%\.lm_trial` atau ``%USERPROFILE%\AppData\Roaming\.lm_trial``
- **macOS**: `~/Library/Application Support/.lm_trial`

File menggunakan nama `.lm_trial` (hidden di Linux/macOS). HMAC secret
di-embed dalam binary dan tidak bisa diubah tanpa recompilasi.

#### Sifat keamanan

- **Tamper-evident**: modifikasi isi file terdeteksi (HMAC tidak cocok)
- **Bukan delete-proof**: penghapusan file = sinyal fresh install (diterima)
- Secret di-embed dalam binary — tidak bisa diganti tanpa recompilasi
