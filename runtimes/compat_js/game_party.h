#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace urpg {
namespace compat {

// Forward declaration
class QuickJSContext;

class GameParty {
public:
    // Member management
    const std::vector<int32_t>& members() const { return members_; }
    void setMembers(const std::vector<int32_t>& members);
    int32_t size() const;
    bool isEmpty() const;
    bool exists(int32_t actorId) const;
    void addActor(int32_t actorId);
    void removeActor(int32_t actorId);
    
    // Gold
    int32_t gold() const { return gold_; }
    void setGold(int32_t gold);
    void gainGold(int32_t amount);
    void loseGold(int32_t amount);
    bool hasGold(int32_t amount) const;
    
    // Items
    int32_t numItems(int32_t itemId) const;
    void gainItem(int32_t itemId, int32_t count);
    void loseItem(int32_t itemId, int32_t count);
    bool hasItem(int32_t itemId) const;
    
    // Weapons
    int32_t numWeapons(int32_t weaponId) const;
    void gainWeapon(int32_t weaponId, int32_t count);
    void loseWeapon(int32_t weaponId, int32_t count);
    
    // Armors
    int32_t numArmors(int32_t armorId) const;
    void gainArmor(int32_t armorId, int32_t count);
    void loseArmor(int32_t armorId, int32_t count);
    
    // Steps / playtime (delegated from GlobalState)
    int32_t steps() const { return steps_; }
    void setSteps(int32_t steps) { steps_ = steps; }
    void increaseSteps(int32_t amount = 1);
    
    // QuickJS API registration
    static void registerAPI(QuickJSContext& ctx, GameParty& party);
    
private:
    std::vector<int32_t> members_;
    int32_t gold_ = 0;
    std::unordered_map<int32_t, int32_t> items_;    // itemId -> count
    std::unordered_map<int32_t, int32_t> weapons_;  // weaponId -> count
    std::unordered_map<int32_t, int32_t> armors_;   // armorId -> count
    int32_t steps_ = 0;
};

} // namespace compat
} // namespace urpg
