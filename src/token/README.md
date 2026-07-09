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

`private_key.pem` **jangan pernah** ikut didistribusikan ke aplikasi client —
hanya dipakai di tool internal kamu untuk membuat token.

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
