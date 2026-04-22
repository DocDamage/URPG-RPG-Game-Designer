#include "engine/core/save/lzstring_decompressor.h"

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace urpg::save::lzstring {

namespace {

constexpr char kBase64Chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

int base64DecodeChar(char c) {
    for (int i = 0; kBase64Chars[i] != '\0'; ++i) {
        if (kBase64Chars[i] == c) return i;
    }
    return -1;
}

// Generic LZString decompressor.
// length: number of elements in the input stream
// resetValue: initial position mask (32 for Base64, 32768 for uint16)
// getNextValue: function that returns the next element value
template <typename GetNextValue>
std::optional<std::string> decompress(int length, int resetValue, GetNextValue getNextValue) {
    std::unordered_map<int, std::string> dictionary;
    double enlargeIn = 4;
    int dictSize = 4;
    int numBits = 3;
    std::string entry;
    std::string result;

    struct Data {
        int val;
        int position;
        int index;
    };

    Data data;
    data.val = getNextValue(0);
    data.position = resetValue;
    data.index = 1;

    for (int i = 0; i < 3; ++i) {
        dictionary[i] = std::to_string(i);
    }

    auto readBits = [&](int numBitsToRead) -> int {
        int bits = 0;
        int power = 1;
        for (int i = 0; i < numBitsToRead; ++i) {
            int resb = data.val & data.position;
            data.position >>= 1;
            if (data.position == 0) {
                data.position = resetValue;
                data.val = getNextValue(data.index);
                data.index += 1;
            }
            if (resb > 0) {
                bits |= power;
            }
            power <<= 1;
        }
        return bits;
    };

    int next = readBits(2);
    std::string c;
    if (next == 0) {
        int bits = readBits(8);
        c = std::string(1, static_cast<char>(bits));
    } else if (next == 1) {
        int bits = readBits(16);
        c = std::string(1, static_cast<char>(bits));
    } else if (next == 2) {
        return "";
    } else {
        return std::nullopt;
    }

    dictionary[3] = c;
    std::string w = c;
    result += c;

    while (true) {
        if (data.index > length) {
            return "";
        }

        int code = readBits(numBits);

        if (code == 0) {
            int bits = readBits(8);
            dictionary[dictSize] = std::string(1, static_cast<char>(bits));
            dictSize += 1;
            code = dictSize - 1;
            enlargeIn -= 1;
        } else if (code == 1) {
            int bits = readBits(16);
            dictionary[dictSize] = std::string(1, static_cast<char>(bits));
            dictSize += 1;
            code = dictSize - 1;
            enlargeIn -= 1;
        } else if (code == 2) {
            return result;
        }

        if (enlargeIn == 0) {
            enlargeIn = 1 << numBits;
            numBits += 1;
        }

        if (dictionary.find(code) != dictionary.end()) {
            entry = dictionary[code];
        } else {
            if (code == dictSize) {
                entry = w + w[0];
            } else {
                return std::nullopt;
            }
        }
        result += entry;

        dictionary[dictSize] = w + entry[0];
        dictSize += 1;
        enlargeIn -= 1;

        w = entry;
        if (enlargeIn == 0) {
            enlargeIn = 1 << numBits;
            numBits += 1;
        }
    }
}

} // namespace

std::optional<std::string> decompressFromBase64(const std::string& input) {
    if (input.empty()) {
        return "";
    }

    // Strip invalid characters and collect valid Base64 values
    std::vector<int> values;
    values.reserve(input.size());
    for (char c : input) {
        int v = base64DecodeChar(c);
        if (v >= 0) {
            values.push_back(v);
        }
    }

    if (values.empty()) {
        return std::nullopt;
    }

    return decompress(static_cast<int>(values.size()), 32,
        [&](int index) -> int {
            if (index < static_cast<int>(values.size())) {
                return values[index];
            }
            return 0;
        });
}

} // namespace urpg::save::lzstring
