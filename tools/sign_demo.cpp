// sign_demo.cpp
// Contoh CLI untuk generate token baru menggunakan library license_helper.
// Usage:
//   sign_demo <private_key.pem> <output_token.token>
//
// Field token di sini masih hardcoded sebagai contoh -- sesuaikan dengan
// input asli kamu (misalnya dari argumen CLI lain, form GUI, dll).

#include "license_helper.h"
#include <fstream>
#include <iostream>

int main(int argc, char** argv) {
    if (argc != 3 && argc != 4) {
        std::cerr << "Pemakaian: " << argv[0]
                  << " <private_key.pem> <output_token.token> [hardware_id]\n"
                  << "  hardware_id opsional -- kosongkan jika tidak ingin mengunci ke 1 mesin.\n"
                  << "  Dapatkan hardware_id dari mesin target dengan verify_demo atau kode\n"
                  << "  yang memanggil license::getHardwareId().\n";
        return 1;
    }

    license::TokenData token;
    token.token_id       = "TKN-0001";
    token.issuer         = "MyCompany";
    token.product        = "MyApp";
    token.register_date  = "2026-07-08";
    token.valid_until    = "2027-07-08";
    token.registered_by  = "admin";
    token.registered_for = "PT Contoh Sejahtera";
    token.license_type   = "standard";
    token.features        = {"module_a", "module_b"};
    token.hardware_id     = (argc == 4) ? argv[3] : "";

    try {
        std::string json = license::signToken(token, argv[1]);

        std::ofstream out(argv[2], std::ios::binary);
        out << json;
        out.close();

        std::cout << "Token berhasil dibuat & ditandatangani: " << argv[2] << "\n";
    } catch (const std::exception& e) {
        std::cerr << "Gagal membuat token: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
