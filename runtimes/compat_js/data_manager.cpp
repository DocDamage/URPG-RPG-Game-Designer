// DataManager - MZ Data/Save/Load Semantics - Implementation
// Phase 2 - Compat Layer

#include "data_manager.h"
#include "runtimes/compat_js/data_manager_support.h"
#include "runtimes/compat_js/data_manager_internal_state.h"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <optional>
#include <sstream>
#include <utility>
#include <nlohmann/json.hpp>

namespace urpg {
namespace compat {

// Static member definitions
std::unordered_map<std::string, CompatStatus> DataManager::methodStatus_;
std::unordered_map<std::string, std::string> DataManager::methodDeviations_;
std::string DataManager::dataDirectory_;

void DataManager::setDataDirectory(const std::string& path) {
    dataDirectory_ = path;
    if (!dataDirectory_.empty() && dataDirectory_.back() != '/' && dataDirectory_.back() != '\\') {
        dataDirectory_.push_back('/');
    }
}

const std::string& DataManager::getDataDirectory() {
    return dataDirectory_;
}

namespace {
using json = nlohmann::json;

} // namespace

// Internal implementation
DataManager::DataManager()
    : impl_(std::make_unique<DataManagerImpl>())
{
    impl_->playtimeStart = std::chrono::steady_clock::now();

    // Initialize method status registry
    if (methodStatus_.empty()) {
        const auto setStatus = [](const std::string& method,
                                  CompatStatus status,
                                  const std::string& deviation = "") {
            methodStatus_[method] = status;
            if (deviation.empty()) {
                methodDeviations_.erase(method);
            } else {
                methodDeviations_[method] = deviation;
            }
        };

        // Save/Load
        setStatus("loadDatabase", CompatStatus::FULL);
        setStatus("saveGame", CompatStatus::FULL);
        setStatus("saveGameWithHeader", CompatStatus::FULL);
        setStatus("loadGame", CompatStatus::FULL);
        setStatus("deleteSaveFile", CompatStatus::FULL);
        setStatus("doesSaveFileExist", CompatStatus::FULL);
        setStatus("getMaxSaveFiles", CompatStatus::FULL);
        setStatus("getSaveHeader", CompatStatus::FULL);
        setStatus("getAllSaveHeaders", CompatStatus::FULL);
        setStatus("saveAutosave", CompatStatus::FULL);
        setStatus("loadAutosave", CompatStatus::FULL);
        setStatus("isAutosaveEnabled", CompatStatus::FULL);
        setStatus("setAutosaveEnabled", CompatStatus::FULL);
        setStatus("setSaveHeaderExtension", CompatStatus::FULL);
        setStatus("getSaveHeaderExtension", CompatStatus::FULL);

        // Database access
        setStatus("getActors", CompatStatus::FULL);
        setStatus("getSkills", CompatStatus::FULL);
        setStatus("getItems", CompatStatus::FULL);
        setStatus("getWeapons", CompatStatus::FULL);
        setStatus("getArmors", CompatStatus::FULL);
        setStatus("getEnemies", CompatStatus::FULL);
        setStatus("getTroops", CompatStatus::FULL);
        setStatus("getStates", CompatStatus::FULL);
        setStatus("getActor", CompatStatus::FULL);
        setStatus("getSkill", CompatStatus::FULL);
        setStatus("getItem", CompatStatus::FULL);

        // Global state
        setStatus("setupNewGame", CompatStatus::FULL);
        setStatus("getPartySize", CompatStatus::FULL);
        setStatus("getPartyMember", CompatStatus::FULL);
        setStatus("getGold", CompatStatus::FULL);
        setStatus("setGold", CompatStatus::FULL);
        setStatus("gainGold", CompatStatus::FULL);
        setStatus("loseGold", CompatStatus::FULL);
        setStatus("getItemCount", CompatStatus::FULL);
        setStatus("gainItem", CompatStatus::FULL);
        setStatus("loseItem", CompatStatus::FULL);
        setStatus("hasItem", CompatStatus::FULL);
        setStatus("getSwitch", CompatStatus::FULL);
        setStatus("setSwitch", CompatStatus::FULL);
        setStatus("getVariable", CompatStatus::FULL);
        setStatus("setVariable", CompatStatus::FULL);
        setStatus("getSelfSwitch", CompatStatus::FULL);
        setStatus("setSelfSwitch", CompatStatus::FULL);
        setStatus("getPlaytime", CompatStatus::FULL);
        setStatus("getPlaytimeString", CompatStatus::FULL);
        setStatus("getSteps", CompatStatus::FULL);
        setStatus("incrementSteps", CompatStatus::FULL);
        setStatus("getPlayerMapId", CompatStatus::FULL);
        setStatus("getPlayerX", CompatStatus::FULL);
        setStatus("getPlayerY", CompatStatus::FULL);
        setStatus("setPlayerPosition", CompatStatus::FULL);
        setStatus("reserveTransfer", CompatStatus::FULL);
        setStatus("isTransferring", CompatStatus::FULL);
        setStatus("processTransfer", CompatStatus::FULL);

        // Plugin commands
        setStatus("registerPluginCommand", CompatStatus::FULL);
        setStatus("unregisterPluginCommand", CompatStatus::FULL);
        setStatus("executePluginCommand", CompatStatus::FULL);
    }
}

DataManager::~DataManager() = default;

DataManager& DataManager::instance() {
    static DataManager instance;
    return instance;
}

// ============================================================================
// Global State
// ============================================================================

void DataManager::setEventCallback(EventCallback callback) {
    eventCallback_ = std::move(callback);
}

void DataManager::registerPluginCommand(const std::string& command, PluginCommandHandler handler) {
    pluginCommands_[command] = std::move(handler);
}

void DataManager::unregisterPluginCommand(const std::string& command) {
    pluginCommands_.erase(command);
}

bool DataManager::executePluginCommand(const std::string& command, const std::vector<Value>& args, Value& result) {
    auto it = pluginCommands_.find(command);
    if (it == pluginCommands_.end()) {
        return false;
    }
    result = it->second(args);
    return true;
}

} // namespace compat
} // namespace urpg
