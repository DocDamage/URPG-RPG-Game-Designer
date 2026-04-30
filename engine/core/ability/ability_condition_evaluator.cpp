#include "ability_condition_evaluator.h"

#include "ability_system_component.h"

#include <charconv>
#include <cctype>
#include <optional>
#include <string_view>
#include <vector>

namespace urpg::ability {
namespace {

std::string trim(std::string_view value) {
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front())) != 0) {
        value.remove_prefix(1);
    }
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back())) != 0) {
        value.remove_suffix(1);
    }
    return std::string(value);
}

std::vector<std::string> splitBy(std::string_view expression, std::string_view separator) {
    std::vector<std::string> parts;
    std::size_t start = 0;
    while (start <= expression.size()) {
        const std::size_t pos = expression.find(separator, start);
        if (pos == std::string_view::npos) {
            parts.push_back(trim(expression.substr(start)));
            break;
        }
        parts.push_back(trim(expression.substr(start, pos - start)));
        start = pos + separator.size();
    }
    return parts;
}

std::optional<float> parseFloat(std::string_view text) {
    const auto trimmed = trim(text);
    if (trimmed.empty()) {
        return std::nullopt;
    }

    float value = 0.0f;
    const char* begin = trimmed.data();
    const char* end = begin + trimmed.size();
    const auto [ptr, ec] = std::from_chars(begin, end, value);
    if (ec != std::errc{} || ptr != end) {
        return std::nullopt;
    }
    return value;
}

std::optional<bool> parseBool(std::string_view text) {
    const auto trimmed = trim(text);
    if (trimmed == "true") {
        return true;
    }
    if (trimmed == "false") {
        return false;
    }
    return std::nullopt;
}

std::optional<std::string> parseQuotedTag(std::string_view call, std::string_view prefix) {
    if (!call.starts_with(prefix) || !call.ends_with("')")) {
        return std::nullopt;
    }
    const std::size_t start = prefix.size();
    if (start > call.size() - 2) {
        return std::nullopt;
    }
    return std::string(call.substr(start, call.size() - start - 2));
}

const AbilitySystemComponent* primaryTarget(const GameplayAbility::AbilityExecutionContext* context) {
    if (context == nullptr || context->targets.empty()) {
        return nullptr;
    }
    return context->targets.front().abilitySystem;
}

std::optional<float> numericOperand(std::string_view operand, const AbilitySystemComponent& source) {
    const auto trimmed = trim(operand);
    if (trimmed == "source.hp") {
        return source.getAttribute("HP", 0.0f);
    }
    if (trimmed == "source.mp") {
        return source.getAttribute("MP", 9999.0f);
    }

    constexpr std::string_view attributePrefix = "source.attribute.";
    if (trimmed.starts_with(attributePrefix) && trimmed.size() > attributePrefix.size()) {
        return source.getAttribute(std::string(trimmed.substr(attributePrefix.size())), 0.0f);
    }

    return parseFloat(trimmed);
}

std::optional<bool> boolOperand(std::string_view operand, const AbilitySystemComponent& source,
                                const GameplayAbility::AbilityExecutionContext* context) {
    const auto trimmed = trim(operand);
    if (const auto literal = parseBool(trimmed)) {
        return literal;
    }

    if (const auto tag = parseQuotedTag(trimmed, "source.hasTag('")) {
        return source.getTags().hasTag(GameplayTag(*tag));
    }

    if (const auto tag = parseQuotedTag(trimmed, "target.hasTag('")) {
        const auto* target = primaryTarget(context);
        if (target == nullptr) {
            return false;
        }
        return target->getTags().hasTag(GameplayTag(*tag));
    }

    return std::nullopt;
}

struct Comparison {
    std::string lhs;
    std::string op;
    std::string rhs;
};

std::optional<Comparison> parseComparison(std::string_view expression) {
    static constexpr std::string_view operators[] = {">=", "<=", "==", "!=", ">", "<"};
    for (const auto op : operators) {
        const std::size_t pos = expression.find(op);
        if (pos == std::string_view::npos) {
            continue;
        }
        return Comparison{trim(expression.substr(0, pos)), std::string(op), trim(expression.substr(pos + op.size()))};
    }
    return std::nullopt;
}

std::optional<bool> evaluateComparison(const Comparison& comparison, const AbilitySystemComponent& source,
                                       const GameplayAbility::AbilityExecutionContext* context) {
    if (comparison.op == "==" || comparison.op == "!=") {
        const auto lhsBool = boolOperand(comparison.lhs, source, context);
        const auto rhsBool = boolOperand(comparison.rhs, source, context);
        if (lhsBool && rhsBool) {
            return comparison.op == "==" ? *lhsBool == *rhsBool : *lhsBool != *rhsBool;
        }
    }

    const auto lhsNumber = numericOperand(comparison.lhs, source);
    const auto rhsNumber = numericOperand(comparison.rhs, source);
    if (!lhsNumber || !rhsNumber) {
        return std::nullopt;
    }

    if (comparison.op == ">") {
        return *lhsNumber > *rhsNumber;
    }
    if (comparison.op == ">=") {
        return *lhsNumber >= *rhsNumber;
    }
    if (comparison.op == "<") {
        return *lhsNumber < *rhsNumber;
    }
    if (comparison.op == "<=") {
        return *lhsNumber <= *rhsNumber;
    }
    if (comparison.op == "==") {
        return *lhsNumber == *rhsNumber;
    }
    if (comparison.op == "!=") {
        return *lhsNumber != *rhsNumber;
    }

    return std::nullopt;
}

AbilityConditionEvaluation parseError(const std::string& expression) {
    return {false, false, "condition_parse_error", "Unsupported activeCondition: " + expression};
}

} // namespace

AbilityConditionEvaluation
AbilityConditionEvaluator::evaluate(const std::string& expression, const AbilitySystemComponent& source,
                                    const GameplayAbility::AbilityExecutionContext* context) const {
    const auto trimmedExpression = trim(expression);
    if (trimmedExpression.empty()) {
        return {};
    }

    bool anyOrBranchMatched = false;
    for (const auto& orBranch : splitBy(trimmedExpression, "||")) {
        if (orBranch.empty()) {
            return parseError(trimmedExpression);
        }

        bool andBranchValue = true;
        for (const auto& clause : splitBy(orBranch, "&&")) {
            if (clause.empty()) {
                return parseError(trimmedExpression);
            }

            const auto comparison = parseComparison(clause);
            if (!comparison) {
                return parseError(trimmedExpression);
            }

            const auto clauseValue = evaluateComparison(*comparison, source, context);
            if (!clauseValue) {
                return parseError(trimmedExpression);
            }
            andBranchValue = andBranchValue && *clauseValue;
        }

        anyOrBranchMatched = anyOrBranchMatched || andBranchValue;
    }

    if (!anyOrBranchMatched) {
        return {true, false, "active_condition_false", "Active condition evaluated false: " + trimmedExpression};
    }

    return {true, true, "", ""};
}

} // namespace urpg::ability
