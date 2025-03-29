#pragma once
#include <string>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <cstring>

// Helper to convert byte array to hex string
std::string toHex(const unsigned char* data, size_t len) {
    std::ostringstream oss;
    for (size_t i = 0; i < len; ++i)
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)data[i];
    return oss.str();
}

// Helper to convert hex string back to bytes
std::string fromHex(const std::string& hex) {
    std::string bytes;
    for (size_t i = 0; i < hex.length(); i += 2) {
        unsigned int byte;
        std::stringstream ss;
        ss << std::hex << hex.substr(i, 2);
        ss >> byte;
        bytes += static_cast<char>(byte);
    }
    return bytes;
}

std::string generateHash(const std::string& password) {
    unsigned char salt[16];
    if (!RAND_bytes(salt, sizeof(salt))) {
        throw std::runtime_error("Failed to generate salt");
    }

    std::string salted = std::string(reinterpret_cast<char*>(salt), sizeof(salt)) + password;

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(salted.c_str()), salted.length(), hash);

    return toHex(salt, sizeof(salt)) + toHex(hash, SHA256_DIGEST_LENGTH);
}

bool validatePassword(const std::string& password, const std::string& stored) {
    if (stored.length() != (16 + SHA256_DIGEST_LENGTH) * 2) return false;

    std::string saltHex = stored.substr(0, 32);
    std::string expectedHex = stored.substr(32);

    std::string salt = fromHex(saltHex);
    std::string salted = salt + password;

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(salted.c_str()), salted.length(), hash);

    std::string actualHex = toHex(hash, SHA256_DIGEST_LENGTH);
    return actualHex == expectedHex;
}
