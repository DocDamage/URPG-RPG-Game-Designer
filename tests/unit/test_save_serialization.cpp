#include <catch2/catch_test_macros.hpp>
#include "engine/core/save/save_serialization_hub.h"
#include <vector>

using namespace urpg::save;

TEST_CASE("Save Serialization Hub: JSON to Binary", "[save][serialization][core]") {
    std::string testJson = "{\"actorId\":1,\"hp\":100,\"mp\":50}";
    
    auto binary = SaveSerializationHub::jsonToBinary(testJson, SaveSerializationHub::CompressionLevel::None);
    
    // Header check (URSV + Version 1.0 + None)
    REQUIRE(binary.size() > 4);
    REQUIRE(binary[0] == 'U');
    REQUIRE(binary[1] == 'R');
    REQUIRE(binary[2] == 'S');
    REQUIRE(binary[3] == 'V');
    
    // Compression check
    REQUIRE(binary[6] == 0); // None
    
    // Recovery check
    std::string recovered = SaveSerializationHub::binaryToJson(binary);
    REQUIRE(recovered == testJson);
}

TEST_CASE("Save Serialization Hub: Corruption Detection", "[save][serialization][core]") {
    std::string testJson = "{\"actorId\":1,\"hp\":100,\"mp\":50}";
    auto binary = SaveSerializationHub::jsonToBinary(testJson, SaveSerializationHub::CompressionLevel::None);
    
    // Modifying one byte should trigger a checksum failure
    binary[12] = (binary[12] == 0xF) ? 1 : 0;
    
    std::string recovered = SaveSerializationHub::binaryToJson(binary);
    REQUIRE(recovered == ""); // Corruption detected
}

TEST_CASE("Save Serialization Hub: Length Verification", "[save][serialization][core]") {
    std::string testJson = "{\"test\":true}";
    auto binary = SaveSerializationHub::jsonToBinary(testJson, SaveSerializationHub::CompressionLevel::None);
    
    // Read the packed length (bytes 7-10)
    uint32_t len = binary[7] | (binary[8] << 8) | (binary[9] << 16) | (binary[10] << 24);
    
    REQUIRE(len == (uint32_t)testJson.length());
}
