#pragma once

#include <string>
#include <vector>
#include <unordered_set>
#include <string_view>

namespace urpg::ability {

/**
 * @brief Represents a hierarchical Gameplay Tag (e.g. "Effect.Burn", "State.Stunned").
 * Inspired by Leshy/GAS4Godot and Unreal GAS.
 */
class GameplayTag {
public:
    GameplayTag() = default;
    explicit GameplayTag(std::string name) : m_name(std::move(name)) {}

    const std::string& getName() const { return m_name; }

    /**
     * @brief Checks if this tag matches another tag (including hierarchical parent matches).
     * Example: "Effect.Burn.VFX" matches "Effect.Burn"
     */
    bool matches(const GameplayTag& other) const {
        if (m_name == other.m_name) return true;
        if (m_name.find(other.m_name + ".") == 0) return true;
        return false;
    }

    bool operator==(const GameplayTag& other) const { return m_name == other.m_name; }
    
    struct Hash {
        size_t operator()(const GameplayTag& tag) const {
            return std::hash<std::string>{}(tag.getName());
        }
    };

private:
    std::string m_name;
};

/**
 * @brief A collection of Gameplay Tags for efficient querying.
 */
class GameplayTagContainer {
public:
    void addTag(const GameplayTag& tag) { m_tags.insert(tag); }
    void removeTag(const GameplayTag& tag) { m_tags.erase(tag); }

    bool hasTag(const GameplayTag& tag) const {
        for (const auto& t : m_tags) {
            if (t.matches(tag)) return true;
        }
        return false;
    }

    bool hasAllTags(const std::vector<GameplayTag>& otherTags) const {
        for (const auto& t : otherTags) {
            if (!hasTag(t)) return false;
        }
        return true;
    }

    bool hasAnyTags(const std::vector<GameplayTag>& otherTags) const {
        for (const auto& t : otherTags) {
            if (hasTag(t)) return true;
        }
        return false;
    }

    const std::unordered_set<GameplayTag, GameplayTag::Hash>& getTags() const { return m_tags; }

private:
    std::unordered_set<GameplayTag, GameplayTag::Hash> m_tags;
};

} // namespace urpg::ability
