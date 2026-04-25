#pragma once

#include <map>
#include <string>
#include <vector>

namespace urpg::narrative {

struct ChoiceConsequence {
    std::string choice_id;
    std::string state_key;
    int delta = 0;
};

class ChoiceConsequenceTracker {
public:
    void registerConsequence(ChoiceConsequence consequence);
    void applyChoice(const std::string& choice_id, std::map<std::string, int>& state) const;
    std::vector<ChoiceConsequence> consequencesFor(const std::string& choice_id) const;

private:
    std::vector<ChoiceConsequence> consequences_;
};

} // namespace urpg::narrative
