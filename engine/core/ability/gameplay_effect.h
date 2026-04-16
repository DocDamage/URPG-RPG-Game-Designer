#pragma once

#include <string>
#include <variant>
#include <vector>

namespace urpg {

enum class ModifierOp {
    Add,        // Value + Modifier
    Multiply,   // Value * Modifier
    Override    // Value = Modifier (Highest priority)
};

/**
 * A single modification to a gameplay attribute (e.g., Attack, Defense).
 */
struct GameplayEffectModifier {
    std::string attributeName;
    ModifierOp operation;
    float value;
    
    // Optional tag requirements for this modifier to be active
    // (e.g. "Only active if State.Poisoned")
    std::string requiredTag; 
};

/**
 * A container for long-lived or temporary modifications.
 */
struct GameplayEffect {
    std::string name;
    std::vector<GameplayEffectModifier> modifiers;
    float duration; // -1.0 for infinite
    float elapsed = 0.0f;
    bool isExpired = false;
};

} // namespace urpg
