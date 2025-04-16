#include "CRC32.h"

// Constructor: Precompute the CRC32 lookup table
uint32_t CRC32::HashToUInt32(const std::vector<uint8_t>& source) {
    return ~Update(UINT32_MAX, source);
}

uint32_t CRC32::Update(uint32_t crc, const std::vector<uint8_t>& source) {
    return UpdateScalar(crc, source);
}

uint32_t CRC32::UpdateScalar(uint32_t crc, const std::vector<uint8_t>& source) {
    for (uint8_t byte : source) {
        uint8_t b = static_cast<uint8_t>(crc);
        b ^= byte;
        crc = CrcLookup[b] ^ (crc >> 8);
    }
    return crc;
}

std::string ToLower(const std::string& str) {
    std::string result = str;
    for (char& c : result) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return result;
}

// Computes the hash value
int ComputeHash(const std::string& name, CRC32 crc) {

    std::string lowerName = ToLower(name);
    std::vector<uint8_t> bytes(lowerName.begin(), lowerName.end());
    return static_cast<int>(crc.HashToUInt32(bytes)) & 0x7FFFFFFF;
}