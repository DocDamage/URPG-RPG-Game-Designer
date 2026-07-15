#include "engine/core/localization/pseudo_localization.h"

#include <cctype>

namespace urpg::localization {

std::string pseudoLocalize(const std::string_view source) {
    std::string result = "［";
    size_t letterCount = 0;
    for (const unsigned char character : source) {
        switch (character) {
        case 'a': result += "à"; break;
        case 'A': result += "À"; break;
        case 'e': result += "ë"; break;
        case 'E': result += "Ë"; break;
        case 'i': result += "ï"; break;
        case 'I': result += "Ï"; break;
        case 'o': result += "ô"; break;
        case 'O': result += "Ô"; break;
        case 'u': result += "ü"; break;
        case 'U': result += "Ü"; break;
        case 'y': result += "ÿ"; break;
        case 'Y': result += "Ÿ"; break;
        default: result.push_back(static_cast<char>(character)); break;
        }
        if (std::isalpha(character) != 0) ++letterCount;
    }
    result.append(letterCount / 3, '~');
    result += "］";
    return result;
}

} // namespace urpg::localization
