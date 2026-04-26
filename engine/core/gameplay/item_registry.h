#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace urpg {

enum class ItemType { Consumable, Weapon, Armor, Material, KeyItem };

struct ItemData {
    std::string id;
    std::string name;
    std::string description;
    ItemType type;
    int value;
    int maxStack;

    // Potential stats for gear
    int attackPower = 0;
    int defensePower = 0;
};

class ItemRegistry {
  public:
    static ItemRegistry& getInstance() {
        static ItemRegistry instance;
        return instance;
    }

    void registerItem(const ItemData& item) { m_items[item.id] = item; }

    const ItemData* getItem(const std::string& id) const {
        auto it = m_items.find(id);
        if (it != m_items.end()) {
            return &it->second;
        }
        return nullptr;
    }

  private:
    ItemRegistry() = default;
    std::map<std::string, ItemData> m_items;
};

} // namespace urpg
