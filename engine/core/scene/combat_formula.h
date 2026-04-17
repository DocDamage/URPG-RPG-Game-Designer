#pragma once

#include "engine/core/scene/battle_scene.h"
#include "runtimes/compat_js/data_manager.h"
#include <string>
#include <algorithm>

namespace urpg::combat {

/**
 * @brief Stat processing and damage formula resolution for Phase 8.
 * Mirrors RPG Maker MZ's formula evaluation but in native C++.
 */
class CombatFormula {
public:
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
     * Supported: a.atk, b.def, a.mat, b.mdf, a.hp, b.mhp etc.
     */
    static int32_t parseFormula(const std::string& formula, const Context& ctx) {
        if (formula.empty()) return evaluateDamage(ctx);
        
        // Very basic lexer substitute for demo:
        // Replace "a.atk" with actual stat etc.
        std::string processed = formula;
        replaceSymbol(processed, "a.atk", std::to_string(getStat(ctx.subject, 2)));
        replaceSymbol(processed, "b.def", std::to_string(getStat(ctx.target, 3)));
        
        // For a true implementation, we'd use a shunting-yard or simple recursive descent.
        // For Phase 8 placeholder, we return the evaluateDamage if parsing logic is complex.
        return evaluateDamage(ctx);
    }

private:
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

    static void replaceSymbol(std::string& s, const std::string& from, const std::string& to) {
        size_t pos = 0;
        while((pos = s.find(from, pos)) != std::string::npos) {
            s.replace(pos, from.length(), to);
            pos += to.length();
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
