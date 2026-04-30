#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace urpg::map {

struct GridPartInstanceState {
    std::string instance_id;
    bool enabled = true;
    bool visible = true;
    bool consumed = false;
    std::unordered_map<std::string, std::string> state;
};

class GridPartRuntimeState {
  public:
    GridPartInstanceState& getOrCreate(const std::string& instance_id);
    const GridPartInstanceState* find(const std::string& instance_id) const;

    bool setFlag(const std::string& instance_id, const std::string& key, const std::string& value);
    std::string getFlag(const std::string& instance_id, const std::string& key, const std::string& fallback = "") const;

    std::vector<GridPartInstanceState> states() const;

  private:
    std::unordered_map<std::string, GridPartInstanceState> states_;
};

} // namespace urpg::map
