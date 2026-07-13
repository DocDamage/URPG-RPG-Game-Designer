#include "engine/core/security/script_transform.h"

#include "engine/core/security/sha256.h"

#include <cctype>
#include <cstdint>
#include <nlohmann/json.hpp>
#include <vector>

namespace urpg::security {

namespace {

constexpr char kTransformId[] = "urpg_script_minify_v1";

std::vector<std::uint8_t> toBytes(std::string_view text) {
    return std::vector<std::uint8_t>(text.begin(), text.end());
}

std::string sha256Hex(std::string_view text) {
    return Sha256::toHex(Sha256::compute(toBytes(text)));
}

bool isPunctuationTightener(char ch) {
    switch (ch) {
    case '(':
    case ')':
    case '{':
    case '}':
    case '[':
    case ']':
    case ';':
    case ',':
    case ':':
    case '=':
    case '*':
    case '%':
    case '<':
    case '>':
    case '!':
    case '?':
    case '&':
    case '|':
        return true;
    default:
        return false;
    }
}

void appendCollapsedSpace(std::string& output) {
    if (!output.empty() && output.back() != ' ' && !isPunctuationTightener(output.back())) {
        output.push_back(' ');
    }
}

std::string minifyJavaScriptLikeSource(std::string_view source) {
    std::string output;
    output.reserve(source.size());

    bool inString = false;
    char stringQuote = '\0';
    bool escaped = false;

    for (std::size_t i = 0; i < source.size(); ++i) {
        const char ch = source[i];

        if (inString) {
            output.push_back(ch);
            if (escaped) {
                escaped = false;
            } else if (ch == '\\') {
                escaped = true;
            } else if (ch == stringQuote) {
                inString = false;
                stringQuote = '\0';
            }
            continue;
        }

        if ((ch == '"' || ch == '\'' || ch == '`')) {
            if (!output.empty() && output.back() == ' ' && isPunctuationTightener(ch)) {
                output.pop_back();
            }
            output.push_back(ch);
            inString = true;
            stringQuote = ch;
            continue;
        }

        if (ch == '/' && (i + 1u) < source.size() && source[i + 1u] == '/') {
            i += 2u;
            while (i < source.size() && source[i] != '\n' && source[i] != '\r') {
                ++i;
            }
            appendCollapsedSpace(output);
            continue;
        }

        if (std::isspace(static_cast<unsigned char>(ch)) != 0) {
            appendCollapsedSpace(output);
            continue;
        }

        if (isPunctuationTightener(ch)) {
            if (!output.empty() && output.back() == ' ') {
                output.pop_back();
            }
            output.push_back(ch);
            continue;
        }

        output.push_back(ch);
    }

    while (!output.empty() && output.back() == ' ') {
        output.pop_back();
    }

    return output;
}

} // namespace

nlohmann::json ScriptTransformResult::toJson() const {
    return {
        {"transform_id", transformId},
        {"script_id", scriptId},
        {"source_sha256", sourceSha256},
        {"transformed_sha256", transformedSha256},
        {"source_bytes", sourceBytes},
        {"transformed_bytes", transformedBytes},
    };
}

ScriptTransformResult TransformScriptForRelease(std::string_view source, std::string_view scriptId) {
    ScriptTransformResult result;
    result.transformId = kTransformId;
    result.scriptId = std::string(scriptId);
    result.transformedSource = minifyJavaScriptLikeSource(source);
    result.sourceSha256 = sha256Hex(source);
    result.transformedSha256 = sha256Hex(result.transformedSource);
    result.sourceBytes = source.size();
    result.transformedBytes = result.transformedSource.size();
    return result;
}

} // namespace urpg::security
