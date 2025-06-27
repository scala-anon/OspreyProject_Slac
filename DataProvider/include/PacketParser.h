#ifndef PACKETPARSER_H
#define PACKETPARSER_H

#include <vector>
#include <string>
#include <cstdint>
#include <fstream>

class PacketParser {
public:
    explicit PacketParser(const std::string& filePath);

    // Struct representing one parsed packet (header + body)
    struct ParsedChunk {
        uint16_t messageId;
        uint32_t seconds;
        uint32_t nanoseconds;
        std::vector<uint8_t> rawBody;
    };

    // Returns true if a packet was read successfully, false if EOF
    bool nextChunk(ParsedChunk& out);

private:
    std::string filePath_;
    std::ifstream inFile_;
};

#endif // PACKETPARSER_H

