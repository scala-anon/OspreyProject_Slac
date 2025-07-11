#include "PacketParser.h"
#include <stdexcept>
#include <array>

// --- Helper: Convert 2 bytes (big-endian) to uint16 ---
static uint16_t toBigEndian16(const uint8_t* data) {
    return (static_cast<uint16_t>(data[0]) << 8) | static_cast<uint16_t>(data[1]); 
}

// --- Helper: Convert a 4 bytes (big-endian) to uint32 ---
static uint32_t toBigEndian32(const uint8_t* data) {
    return (static_cast<uint32_t>(data[0]) << 24) |
           (static_cast<uint32_t>(data[1]) << 16) |
           (static_cast<uint32_t>(data[2]) << 8)  |
            static_cast<uint32_t>(data[3]); 
}

// --- Constructor ---
PacketParser::PacketParser(const std::string & filePath) : filePath_(filePath){
    inFile_.open(filePath_, std::ios::binary);
    if(!inFile_){
        throw std::runtime_error("Failed to open file: " + filePath_);
    }
}

// --- Read one packet (header + body) at a time ---
bool PacketParser::nextChunk(ParsedChunk& out) {
    std::array<uint8_t, 16> headerBytes;

    // Attempt to read the 16-byte header
    if (!inFile_.read(reinterpret_cast<char*>(headerBytes.data()), headerBytes.size())) {
        return false; // EOF reached or failure
    }

    // Validate framing marker "PS"
    if (headerBytes[0] != 'P' || headerBytes[1] != 'S') {
        throw std::runtime_error("Invalid framing marker (expected 'P' 'S')");
    }

    out.messageId   = toBigEndian16(&headerBytes[2]);
    uint32_t bodyLen = toBigEndian32(&headerBytes[4]);
    out.seconds     = toBigEndian32(&headerBytes[8]);
    out.nanoseconds = toBigEndian32(&headerBytes[12]);

    // Read body
    out.rawBody.resize(bodyLen);
    if (!inFile_.read(reinterpret_cast<char*>(out.rawBody.data()), bodyLen)) {
        throw std::runtime_error("Failed to read body for message ID: " + std::to_string(out.messageId));
    }

    return true;
}
