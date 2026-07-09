// hwid.cpp
// Jalankan di mesin target untuk mendapatkan hardware_id yang perlu dikirim
// ke pemegang private key sebelum token dibuat.
// Usage: hwid

#include "license_helper.h"
#include <iostream>

int main() {
    try {
        std::cout << "Hardware ID mesin ini: " << license::getHardwareId() << "\n";
    } catch (const std::exception& e) {
        std::cerr << "Gagal membaca hardware ID: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
