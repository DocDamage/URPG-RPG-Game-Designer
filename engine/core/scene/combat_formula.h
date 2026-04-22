#pragma once

#include "engine/core/scene/battle_scene.h"
#include "runtimes/compat_js/data_manager.h"
#include <algorithm>
#include <cctype>
#include <optional>
#include <string>

namespace urpg::combat {

/**
 * @brief Stat processing and damage formula resolution for Phase 8.
 * Mirrors RPG Maker MZ's formula evaluation but in native C++.
 */
class CombatFormula {
public:
    struct FormulaEvaluationResult {
        int32_t value = 0;
        bool usedFallback = false;
        std::string reason;
        std::string formula;
        std::string normalizedFormula;
    };

    struct Context {
        const scene::BattleParticipant* subject;
        const scene::BattleParticipant* target;
        const compat::SkillData* skill;
        const compat::ItemData* item;
    };

    /**
     * @brief Evaluates a basic physical or magical attack.
     * Default MZ Formula: 4 * atk - 2 * def
     */
    static int32_t evaluateDamage(const Context& ctx) {
        int32_t atk = getStat(ctx.subject, 2);
        int32_t def = getStat(ctx.target, 3);
        int32_t base = (4 * atk) - (2 * def);
        return (base < 0) ? 0 : base;
    }

    /**
     * @brief Parses a basic math string formula (MZ format).
     * Supported subset: integer literals, a./b. stat symbols, + - * /, and parentheses.
     */
    static int32_t parseFormula(const std::string& formula, const Context& ctx) {
        return evaluateFormula(formula, ctx).value;
    }

    static FormulaEvaluationResult evaluateFormula(const std::string& formula, const Context& ctx) {
        FormulaEvaluationResult result;
        result.formula = formula;

        if (isBlankFormula(formula)) {
            result.value = evaluateDamage(ctx);
            result.usedFallback = true;
            result.reason = "empty_formula_fallback";
            return result;
        }

        const auto normalized = substituteSupportedSymbols(formula, ctx);
        if (!normalized.has_value()) {
            result.value = evaluateDamage(ctx);
            result.usedFallback = true;
            result.reason = substitutionError_;
            return result;
        }

        FormulaParser parser(*normalized);
        const auto parsed = parser.parse();
        if (!parsed.has_value()) {
            result.value = evaluateDamage(ctx);
            result.usedFallback = true;
            result.reason = parser.error();
            return result;
        }

        result.value = *parsed;
        result.normalizedFormula = *normalized;
        return result;
    }

private:
    class FormulaParser {
    public:
        explicit FormulaParser(const std::string& formula)
            : formula_(formula) {}

        std::optional<int32_t> parse() {
            skipWhitespace();
            auto value = parseExpression();
            if (!value.has_value()) {
                return std::nullopt;
            }

            skipWhitespace();
            if (pos_ != formula_.size()) {
                if (error_.empty()) {
                    error_ = "unsupported_formula_token";
                }
                return std::nullopt;
            }
            return value;
        }

        const std::string& error() const {
            return error_;
        }

        const std::string& normalizedExpression() const {
            return normalized_;
        }

    private:
        std::optional<int32_t> parseExpression() {
            auto value = parseTerm();
            if (!value.has_value()) {
                return std::nullopt;
            }

            while (true) {
                skipWhitespace();
                if (!match('+') && !match('-')) {
                    break;
                }

                const char op = formula_[pos_ - 1];
                normalized_.push_back(' ');
                normalized_.push_back(op);
                normalized_.push_back(' ');

                auto rhs = parseTerm();
                if (!rhs.has_value()) {
                    setErrorIfUnset("malformed_formula_expression");
                    return std::nullopt;
                }
                *value = (op == '+') ? (*value + *rhs) : (*value - *rhs);
            }
            return value;
        }

        std::optional<int32_t> parseTerm() {
            auto value = parseFactor();
            if (!value.has_value()) {
                return std::nullopt;
            }

            while (true) {
                skipWhitespace();
                if (!match('*') && !match('/')) {
                    break;
                }

                const char op = formula_[pos_ - 1];
                normalized_.push_back(' ');
                normalized_.push_back(op);
                normalized_.push_back(' ');

                auto rhs = parseFactor();
                if (!rhs.has_value()) {
                    setErrorIfUnset("malformed_formula_expression");
                    return std::nullopt;
                }

                if (op == '*') {
                    *value *= *rhs;
                } else {
                    if (*rhs == 0) {
                        setErrorIfUnset("formula_divide_by_zero");
                        return std::nullopt;
                    }
                    *value /= *rhs;
                }
            }
            return value;
        }

        std::optional<int32_t> parseFactor() {
            skipWhitespace();

            if (pos_ >= formula_.size()) {
                setErrorIfUnset("malformed_formula_expression");
                return std::nullopt;
            }

            if (match('+')) {
                normalized_.push_back('+');
                return parseFactor();
            }
            if (match('-')) {
                normalized_.push_back('-');
                auto value = parseFactor();
                if (!value.has_value()) {
                    setErrorIfUnset("malformed_formula_expression");
                    return std::nullopt;
                }
                return -*value;
            }

            if (match('(')) {
                normalized_.push_back('(');
                auto value = parseExpression();
                skipWhitespace();
                if (!match(')')) {
                    setErrorIfUnset("malformed_formula_expression");
                    return std::nullopt;
                }
                normalized_.push_back(')');
                return value;
            }

            if (const auto integer = parseInteger(); integer.has_value()) {
                normalized_ += std::to_string(*integer);
                return integer;
            }

            if (error_.empty()) {
                error_ = "unsupported_formula_token";
            }
            return std::nullopt;
        }

        std::optional<int32_t> parseInteger() {
            skipWhitespace();
            const size_t start = pos_;
            while (pos_ < formula_.size() && std::isdigit(static_cast<unsigned char>(formula_[pos_]))) {
                ++pos_;
            }
            if (start == pos_) {
                return std::nullopt;
            }
            return std::stoi(formula_.substr(start, pos_ - start));
        }

        void skipWhitespace() {
            while (pos_ < formula_.size() &&
                   std::isspace(static_cast<unsigned char>(formula_[pos_]))) {
                ++pos_;
            }
        }

        bool match(char expected) {
            skipWhitespace();
            if (pos_ < formula_.size() && formula_[pos_] == expected) {
                ++pos_;
                return true;
            }
            return false;
        }

        void setErrorIfUnset(const std::string& error) {
            if (error_.empty()) {
                error_ = error;
            }
        }

        const std::string& formula_;
        size_t pos_ = 0;
        std::string error_;
        std::string normalized_;
    };

    static std::optional<std::string> substituteSupportedSymbols(const std::string& formula, const Context& ctx) {
        substitutionError_.clear();

        std::string normalized;
        normalized.reserve(formula.size());

        size_t pos = 0;
        while (pos < formula.size()) {
            const char ch = formula[pos];
            if (std::isalpha(static_cast<unsigned char>(ch)) || ch == '_') {
                const size_t start = pos;
                while (pos < formula.size()) {
                    const char token_char = formula[pos];
                    if (std::isalnum(static_cast<unsigned char>(token_char)) || token_char == '.' || token_char == '_') {
                        ++pos;
                        continue;
                    }
                    break;
                }

                const std::string token = formula.substr(start, pos - start);
                const auto resolved = resolveSymbol(token, ctx);
                if (!resolved.has_value()) {
                    substitutionError_ = "unsupported_formula_symbol:" + token;
                    return std::nullopt;
                }
                normalized += std::to_string(*resolved);
                continue;
            }

            normalized.push_back(ch);
            ++pos;
        }

        return normalized;
    }

    static std::optional<int32_t> resolveSymbol(const std::string& token, const Context& ctx) {
        if (token == "a.atk") return getStat(ctx.subject, 2);
        if (token == "a.def") return getStat(ctx.subject, 3);
        if (token == "a.mat") return getStat(ctx.subject, 4);
        if (token == "a.mdf") return getStat(ctx.subject, 5);
        if (token == "a.agi") return getStat(ctx.subject, 6);
        if (token == "a.luk") return getStat(ctx.subject, 7);
        if (token == "a.hp") return getStat(ctx.subject, 0);
        if (token == "a.mhp") return getStat(ctx.subject, 0);
        if (token == "a.mp") return getStat(ctx.subject, 1);
        if (token == "a.mmp") return getStat(ctx.subject, 1);
        if (token == "b.atk") return getStat(ctx.target, 2);
        if (token == "b.def") return getStat(ctx.target, 3);
        if (token == "b.mat") return getStat(ctx.target, 4);
        if (token == "b.mdf") return getStat(ctx.target, 5);
        if (token == "b.agi") return getStat(ctx.target, 6);
        if (token == "b.luk") return getStat(ctx.target, 7);
        if (token == "b.hp") return getStat(ctx.target, 0);
        if (token == "b.mhp") return getStat(ctx.target, 0);
        if (token == "b.mp") return getStat(ctx.target, 1);
        if (token == "b.mmp") return getStat(ctx.target, 1);
        return std::nullopt;
    }

    static bool isBlankFormula(const std::string& formula) {
        return std::all_of(formula.begin(),
                           formula.end(),
                           [](unsigned char ch) { return std::isspace(ch) != 0; });
    }

    static inline thread_local std::string substitutionError_;

    static int32_t parseParticipantId(const scene::BattleParticipant* p) {
        if (p == nullptr) {
            return -1;
        }
        try {
            return std::stoi(p->id);
        } catch (...) {
            return -1;
        }
    }

    static int32_t getStat(const scene::BattleParticipant* p, int32_t paramId) {
        if (p == nullptr) {
            return 10;
        }
        const int32_t participantId = parseParticipantId(p);
        if (participantId <= 0) {
            return 10;
        }

        auto& dm = compat::DataManager::instance();
        if (p->isEnemy) {
            auto data = dm.getEnemy(participantId);
            if (!data) return 10;
            switch(paramId) {
                case 0: return data->mhp;
                case 1: return data->mmp;
                case 2: return data->atk;
                case 3: return data->def;
                case 4: return data->mat;
                case 5: return data->mdf;
                case 6: return data->agi;
                case 7: return data->luk;
                default: return 10;
            }
        } else {
            return dm.getActorParam(participantId, paramId, 1);
        }
    }
};

} // namespace urpg::combat
