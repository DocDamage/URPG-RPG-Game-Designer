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
        dataDirectory_.push_back('\\');
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
        setStatus("loadDatabase", CompatStatus::PARTIAL);
        setStatus("saveGame", CompatStatus::PARTIAL);
        setStatus("saveGameWithHeader", CompatStatus::PARTIAL);
        setStatus("loadGame", CompatStatus::PARTIAL);
        setStatus("deleteSaveFile", CompatStatus::PARTIAL);
        setStatus("doesSaveFileExist", CompatStatus::PARTIAL);
        setStatus("getMaxSaveFiles", CompatStatus::PARTIAL);
        setStatus("getSaveHeader", CompatStatus::PARTIAL);
        setStatus("getAllSaveHeaders", CompatStatus::PARTIAL);
        setStatus("saveAutosave", CompatStatus::PARTIAL);
        setStatus("loadAutosave", CompatStatus::PARTIAL);
        setStatus("isAutosaveEnabled", CompatStatus::PARTIAL);
        setStatus("setAutosaveEnabled", CompatStatus::PARTIAL);
        setStatus("setSaveHeaderExtension", CompatStatus::PARTIAL);
        setStatus("getSaveHeaderExtension", CompatStatus::PARTIAL);

        // Database access
        setStatus("getActors", CompatStatus::PARTIAL);
        setStatus("getSkills", CompatStatus::PARTIAL);
        setStatus("getItems", CompatStatus::PARTIAL);
        setStatus("getWeapons", CompatStatus::PARTIAL);
        setStatus("getArmors", CompatStatus::PARTIAL);
        setStatus("getEnemies", CompatStatus::PARTIAL);
        setStatus("getTroops", CompatStatus::PARTIAL);
        setStatus("getStates", CompatStatus::PARTIAL);
        setStatus("getActor", CompatStatus::PARTIAL);
        setStatus("getSkill", CompatStatus::PARTIAL);
        setStatus("getItem", CompatStatus::PARTIAL);

        // Global state
        setStatus("setupNewGame", CompatStatus::PARTIAL);
        setStatus("getPartySize", CompatStatus::PARTIAL);
        setStatus("getPartyMember", CompatStatus::PARTIAL);
        setStatus("getGold", CompatStatus::PARTIAL);
        setStatus("setGold", CompatStatus::PARTIAL);
        setStatus("gainGold", CompatStatus::PARTIAL);
        setStatus("loseGold", CompatStatus::PARTIAL);
        setStatus("getItemCount", CompatStatus::PARTIAL);
        setStatus("gainItem", CompatStatus::PARTIAL);
        setStatus("loseItem", CompatStatus::PARTIAL);
        setStatus("hasItem", CompatStatus::PARTIAL);
        setStatus("getSwitch", CompatStatus::PARTIAL);
        setStatus("setSwitch", CompatStatus::PARTIAL);
        setStatus("getVariable", CompatStatus::PARTIAL);
        setStatus("setVariable", CompatStatus::PARTIAL);
        setStatus("getSelfSwitch", CompatStatus::PARTIAL);
        setStatus("setSelfSwitch", CompatStatus::PARTIAL);
        setStatus("getPlaytime", CompatStatus::PARTIAL);
        setStatus("getPlaytimeString", CompatStatus::PARTIAL);
        setStatus("getSteps", CompatStatus::PARTIAL);
        setStatus("incrementSteps", CompatStatus::PARTIAL);
        setStatus("getPlayerMapId", CompatStatus::PARTIAL);
        setStatus("getPlayerX", CompatStatus::PARTIAL);
        setStatus("getPlayerY", CompatStatus::PARTIAL);
        setStatus("setPlayerPosition", CompatStatus::PARTIAL);
        setStatus("reserveTransfer", CompatStatus::PARTIAL);
        setStatus("isTransferring", CompatStatus::PARTIAL);
        setStatus("processTransfer", CompatStatus::PARTIAL);

        // Plugin commands
        setStatus("registerPluginCommand", CompatStatus::PARTIAL);
        setStatus("unregisterPluginCommand", CompatStatus::PARTIAL);
        setStatus("executePluginCommand", CompatStatus::PARTIAL);
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
