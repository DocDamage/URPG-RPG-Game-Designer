#pragma once
#include <cstdint>
#include <vector>

namespace urpg {
namespace compat {

// Forward declarations
class BattleManager;
class QuickJSContext;

class GameTroop {
public:
    // Members are enemy database IDs (not BattleSubject indices)
    const std::vector<int32_t>& members() const { return members_; }
    void setMembers(const std::vector<int32_t>& members);
    int32_t size() const;
    bool isEmpty() const;
    
    // Total EXP / Gold this troop would yield (sum of enemy data)
    int32_t totalExp() const;
    int32_t totalGold() const;
    
    // QuickJS API registration
    static void registerAPI(QuickJSContext& ctx, GameTroop& troop);
    
private:
    std::vector<int32_t> members_;
};

} // namespace compat
} // namespace urpg
