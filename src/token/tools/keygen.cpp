// keygen.cpp
// Tool CLI untuk generate key pair ECDSA (P-256 / prime256v1).
// Usage:
//   keygen <private_out.pem> <public_out.pem>
//
// PENTING: private_out.pem HANYA untuk dipakai di tool signer internal kamu.
// Jangan pernah ikut sertakan file ini ke aplikasi yang didistribusikan.

#include <openssl/evp.h>
#include <openssl/ec.h>
#include <openssl/pem.h>
#include <openssl/err.h>

#include <cstdio>
#include <string>
#include <iostream>

static void printOpensslErrors() {
    ERR_print_errors_fp(stderr);
}

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Pemakaian: " << argv[0] << " <private_out.pem> <public_out.pem>\n";
        return 1;
    }

    const std::string privPath = argv[1];
    const std::string pubPath = argv[2];

    EVP_PKEY_CTX* pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, nullptr);
    if (!pctx) { printOpensslErrors(); return 1; }

    if (EVP_PKEY_keygen_init(pctx) <= 0) {
        printOpensslErrors();
        EVP_PKEY_CTX_free(pctx);
        return 1;
    }

    if (EVP_PKEY_CTX_set_ec_paramgen_curve_nid(pctx, NID_X9_62_prime256v1) <= 0) {
        printOpensslErrors();
        EVP_PKEY_CTX_free(pctx);
        return 1;
    }

    EVP_PKEY* pkey = nullptr;
    if (EVP_PKEY_keygen(pctx, &pkey) <= 0) {
        printOpensslErrors();
        EVP_PKEY_CTX_free(pctx);
        return 1;
    }
    EVP_PKEY_CTX_free(pctx);

    // Tulis private key
    FILE* privFp = fopen(privPath.c_str(), "wb");
    if (!privFp) {
        std::cerr << "Tidak bisa membuka file: " << privPath << "\n";
        EVP_PKEY_free(pkey);
        return 1;
    }
    if (!PEM_write_PrivateKey(privFp, pkey, nullptr, nullptr, 0, nullptr, nullptr)) {
        printOpensslErrors();
        fclose(privFp);
        EVP_PKEY_free(pkey);
        return 1;
    }
    fclose(privFp);

    // Tulis public key
    FILE* pubFp = fopen(pubPath.c_str(), "wb");
    if (!pubFp) {
        std::cerr << "Tidak bisa membuka file: " << pubPath << "\n";
        EVP_PKEY_free(pkey);
        return 1;
    }
    if (!PEM_write_PUBKEY(pubFp, pkey)) {
        printOpensslErrors();
        fclose(pubFp);
        EVP_PKEY_free(pkey);
        return 1;
    }
    fclose(pubFp);

    EVP_PKEY_free(pkey);

    std::cout << "Key pair berhasil dibuat:\n"
              << "  Private key : " << privPath << "  (RAHASIA, simpan aman, jangan didistribusikan)\n"
              << "  Public key  : " << pubPath  << "  (boleh di-embed ke aplikasi client)\n";
    return 0;
}
