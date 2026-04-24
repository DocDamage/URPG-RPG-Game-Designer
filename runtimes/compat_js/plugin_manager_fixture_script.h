#pragma once

#include "plugin_manager.h"

#include <functional>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include <unordered_map>
#include <vector>

namespace urpg::compat::plugin_manager_detail {

using FixtureCommandInvoker =
    std::function<Value(const std::string& pluginName,
                        const std::string& commandName,
                        const std::vector<Value>& args)>;
using FixtureCommandByNameInvoker =
    std::function<Value(const std::string& fullName, const std::vector<Value>& args)>;

Value executeFixtureScript(const nlohmann::json& script,
                           const std::string& pluginName,
                           const std::string& commandName,
                           const std::vector<Value>& args,
                           const std::unordered_map<std::string, Value>& parameters,
                           const FixtureCommandInvoker& commandInvoker,
                           const FixtureCommandByNameInvoker& commandByNameInvoker);

} // namespace urpg::compat::plugin_manager_detail
