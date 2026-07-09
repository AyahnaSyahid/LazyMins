// verify_demo.cpp
// Contoh CLI scan folder token dan verifikasi semuanya.
// Usage:
//   verify_demo <folder_token> <public_key.pem>

#include "license_helper.h"
#include <iostream>

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Pemakaian: " << argv[0] << " <folder_token> <public_key.pem>\n";
        return 1;
    }

    auto results = license::scanTokenFolder(argv[1], argv[2], ".token");

    if (results.empty()) {
        std::cout << "Tidak ada file .token ditemukan di: " << argv[1] << "\n";
        return 0;
    }

    for (const auto& r : results) {
        std::cout << "[" << license::toString(r.result) << "] " << r.filepath;
        if (r.result == license::ValidationResult::Valid ||
            r.result == license::ValidationResult::Expired) {
            std::cout << "  (registered_for=" << r.token.registered_for
                       << ", valid_until=" << r.token.valid_until << ")";
        }
        std::cout << "\n";
    }
    return 0;
}
