#pragma once

#include "engine/core/audio/audio_core.h"
#include "engine/core/clock.h"
#include "engine/core/ecs/health_system.h"
#include "engine/core/ecs/world.h"
#include "engine/core/gameplay/inventory_components.h"
#include "engine/core/gameplay/quest_system.h"
#include "engine/core/input/input_core.h"
#include "engine/core/message/dialogue_database.h"
#include "engine/core/message/dialogue_project_loader.h"
#include "engine/core/message/dialogue_registry.h"
#include "engine/core/platform/platform_surface.h"
#include "engine/core/platform/renderer_backend.h"
#include "engine/core/render/render_layer.h"
#include "engine/core/runtime_startup_services.h"
#include "engine/core/scene/scene_manager.h"
#include "engine/core/sprite_batcher.h"
#include "runtimes/compat_js/audio_manager.h"
#include <atomic>
#include <charconv>
#include <chrono>
#include <filesystem>
#include <memory>
#include <string_view>
#include <utility>
#include <vector>

namespace urpg {

/**
 * @brief The central coordinator for the native game loop (Tick -> Update -> Render).
 * This shell ties together the SceneManager, InputCore, and Surface management.
 */
class EngineShell {
  public:
    struct StartupOptions {
        StartupOptions() : project_root(std::filesystem::current_path()) {}
        explicit StartupOptions(std::filesystem::path root) : project_root(std::move(root)) {}

        std::filesystem::path project_root;
    };

    static EngineShell& getInstance() {
        static EngineShell instance;
        return instance;
    }

    /**
     * @brief Initializes engine systems with a platform surface and renderer.
     */
    bool startup(std::unique_ptr<IPlatformSurface> surface, std::unique_ptr<RendererBackend> renderer,
                 StartupOptions options = StartupOptions()) {
        m_platform = std::move(surface);
        m_renderer = std::move(renderer);

        if (m_renderer && !m_renderer->initialize(m_platform.get())) {
            return false;
        }

        // Initialize Dialogue Database from project content only.
        auto& dialogueRegistry = message::DialogueRegistry::getInstance();
        dialogueRegistry.clear();
        m_dialogueLoadResult = message::DialogueProjectLoader::loadProject(dialogueRegistry, options.project_root);
        initializeDialogueCommandState();
        setupDialogueCommands();
        compat::AudioManager::instance().bindAudioCore(&m_audioCore);
        m_runtimeStartupReport = RuntimeStartupServices::initialize(
            options.project_root, m_audioCore, m_inputCore,
            compat::AudioManager::instance().boundAudioCore() == &m_audioCore);

        m_batcher = std::make_unique<SpriteBatcher>();
        m_clock.reset();
        m_isRunning = true;
        return true;
    }

    /**
     * @brief Performs a single engine tick.
     */
    void tick() {
        if (!m_isRunning)
            return;

        // 1. Poll Platform Events
        if (m_platform && !m_platform->pollEvents()) {
            shutdown();
            return;
        }

        // 2. Calculate Delta Time
        float dt = m_clock.getDelta();
        if (dt > 0.1f)
            dt = 0.016f;

        // 3. Update active scene
        auto& sceneMgr = scene::SceneManager::getInstance();
        auto activeScene = sceneMgr.getActiveScene();
        if (activeScene) {
            activeScene->handleInput(m_inputCore);
            activeScene->onUpdate(dt);
        }

        // 4. Record and Render Frame
        if (m_renderer && m_batcher) {
            m_renderer->beginFrame();

            // Collect draw commands from scene
            m_batcher->begin();
            if (activeScene) {
                activeScene->draw(*m_batcher);
            }
            m_batcher->end();

            // Submit batches to the hardware backend
            m_renderer->renderBatches(m_batcher->getBatches());

            // Process the frame-owned RenderLayer buffer through the backend adapter.
            m_renderer->processFrameCommands(RenderLayer::getInstance().getFrameCommands());
            RenderLayer::getInstance().flush();

            m_renderer->endFrame();
        }
    }

    void shutdown() {
        m_isRunning = false;
        if (m_renderer) {
            m_renderer->shutdown();
        }
        if (m_platform) {
            m_platform->shutdown();
        }
        compat::AudioManager::instance().bindAudioCore(nullptr);
        m_audioCore.stopAll();
        RenderLayer::getInstance().flush();
    }

    bool isRunning() const { return m_isRunning; }
    input::InputCore& getInput() { return m_inputCore; }
    audio::AudioCore& getAudio() { return m_audioCore; }
    const RuntimeStartupReport& getRuntimeStartupReport() const { return m_runtimeStartupReport; }
    IPlatformSurface* getPlatform() { return m_platform.get(); }
    RendererBackend* getRenderer() { return m_renderer.get(); }
    /**
     * @brief Exports the current engine dialogue state to a metadata map.
     */
    nlohmann::json exportDialogueMetadata() {
        return message::DialogueDatabase::exportMetadata(message::DialogueRegistry::getInstance());
    }

    nlohmann::json exportDialogueLoadDiagnostics() const {
        nlohmann::json diagnostics = nlohmann::json::array();
        for (const auto& diagnostic : m_dialogueLoadResult.diagnostics) {
            diagnostics.push_back({
                {"severity", dialogueSeverityToString(diagnostic.severity)},
                {"code", diagnostic.code},
                {"path", diagnostic.path.string()},
                {"message", diagnostic.message},
            });
        }
        return diagnostics;
    }

    const message::DialogueProjectLoadResult& getDialogueLoadResult() const { return m_dialogueLoadResult; }

    message::DialogueCommandProcessor::CommandResult executeDialogueCommand(const std::string& command) {
        auto result = m_dialogueProcessor.execute(command);
        m_dialogueCommandHistory.push_back(result);
        return result;
    }

    void configureDialogueQuest(const std::string& quest_id, int required_objective_count = 1,
                                QuestState state = QuestState::Active) {
        auto* quest = m_dialogueCommandWorld.GetComponent<QuestComponent>(m_dialoguePlayerEntity);
        if (!quest) {
            quest = &m_dialogueCommandWorld.AddComponent(m_dialoguePlayerEntity, QuestComponent{});
        }

        quest->questId = quest_id;
        quest->state = state;
        quest->currentObjectiveProgress = 0;
        quest->requiredObjectiveCount = required_objective_count > 0 ? required_objective_count : 1;
    }

    nlohmann::json exportDialogueCommandDiagnostics() const {
        nlohmann::json diagnostics = nlohmann::json::array();
        for (const auto& result : m_dialogueCommandHistory) {
            diagnostics.push_back(commandResultToJson(result));
        }
        return diagnostics;
    }

    nlohmann::json exportDialogueGameplaySnapshot() const {
        nlohmann::json snapshot;
        snapshot["entity_id"] = m_dialoguePlayerEntity;

        if (const auto* inventory = m_dialogueCommandWorld.GetComponent<InventoryComponent>(m_dialoguePlayerEntity)) {
            snapshot["inventory"] = nlohmann::json::array();
            for (const auto& slot : inventory->slots) {
                snapshot["inventory"].push_back({
                    {"item_id", slot.itemId},
                    {"count", slot.count},
                });
            }
            snapshot["inventory_max_slots"] = inventory->maxSlots;
        } else {
            snapshot["inventory"] = nullptr;
        }

        if (const auto* health = m_dialogueCommandWorld.GetComponent<HealthComponent>(m_dialoguePlayerEntity)) {
            snapshot["health"] = {
                {"current", health->current},
                {"max", health->max},
                {"alive", health->isAlive},
            };
        } else {
            snapshot["health"] = nullptr;
        }

        if (const auto* quest = m_dialogueCommandWorld.GetComponent<QuestComponent>(m_dialoguePlayerEntity)) {
            snapshot["quest"] = {
                {"id", quest->questId},
                {"state", questStateToString(quest->state)},
                {"current_progress", quest->currentObjectiveProgress},
                {"required_progress", quest->requiredObjectiveCount},
            };
        } else {
            snapshot["quest"] = nullptr;
        }

        return snapshot;
    }

  private:
    EngineShell() : m_isRunning(false) {}

    void initializeDialogueCommandState() {
        m_dialogueCommandWorld = World{};
        m_dialoguePlayerEntity = m_dialogueCommandWorld.CreateEntity();
        m_dialogueCommandWorld.AddComponent(m_dialoguePlayerEntity, InventoryComponent{});
        m_dialogueCommandWorld.AddComponent(m_dialoguePlayerEntity, HealthComponent{75, 100, true, false});
        m_dialogueCommandHistory.clear();
        m_dialogueProcessor.clearHandlers();
    }

    void setupDialogueCommands() {
        m_dialogueProcessor.registerHandler("GIVE_ITEM", [this](const std::string& arg) {
            auto result = makeCommandResult("give_item", false, "invalid_item", "Item id is required.");
            const auto [item_id, count] = parseIdAndCount(arg, 1);
            if (item_id.empty() || count <= 0) {
                return result;
            }

            auto* inventory = m_dialogueCommandWorld.GetComponent<InventoryComponent>(m_dialoguePlayerEntity);
            if (!inventory) {
                return makeCommandResult("give_item", false, "inventory_missing",
                                         "Dialogue command entity has no inventory.");
            }

            if (!inventory->addItem(item_id, count)) {
                return makeCommandResult("give_item", false, "inventory_full", "Inventory is full.");
            }

            return makeCommandResult("give_item", true, "item_granted", "Granted item '" + item_id + "'.");
        });
        m_dialogueProcessor.registerHandler("HEAL_PLAYER", [this](const std::string& arg) {
            const auto amount = parsePositiveInt(arg, 0);
            if (amount <= 0) {
                return makeCommandResult("heal_player", false, "invalid_heal_amount", "Heal amount must be positive.");
            }

            if (!m_dialogueHealthSystem.heal(m_dialogueCommandWorld, m_dialoguePlayerEntity, amount)) {
                return makeCommandResult("heal_player", false, "heal_failed", "Player health could not be healed.");
            }

            return makeCommandResult("heal_player", true, "player_healed", "Player healed.");
        });
        m_dialogueProcessor.registerHandler("QUEST_ADVANCE", [this](const std::string& arg) {
            const auto [quest_id, amount] = parseIdAndCount(arg, 1);
            if (quest_id.empty() || amount <= 0) {
                return makeCommandResult("quest_advance", false, "invalid_quest_argument",
                                         "Quest id and positive amount are required.");
            }

            if (!m_dialogueQuestSystem.advanceQuest(m_dialogueCommandWorld, m_dialoguePlayerEntity, quest_id, amount)) {
                return makeCommandResult("quest_advance", false, "quest_not_active", "Quest is missing or not active.");
            }

            return makeCommandResult("quest_advance", true, "quest_advanced", "Quest advanced.");
        });
    }

    static message::DialogueCommandProcessor::CommandResult makeCommandResult(std::string prefix, bool success,
                                                                              std::string code, std::string message) {
        message::DialogueCommandProcessor::CommandResult result;
        result.prefix = std::move(prefix);
        result.handled = true;
        result.success = success;
        result.code = std::move(code);
        result.message = std::move(message);
        return result;
    }

    static int parsePositiveInt(std::string_view text, int fallback) {
        int value = fallback;
        if (text.empty()) {
            return fallback;
        }

        const auto* begin = text.data();
        const auto* end = begin + text.size();
        const auto parse_result = std::from_chars(begin, end, value);
        if (parse_result.ec != std::errc{} || parse_result.ptr != end) {
            return fallback;
        }
        return value;
    }

    static std::pair<std::string, int> parseIdAndCount(const std::string& arg, int default_count) {
        const size_t colon = arg.find(':');
        if (colon == std::string::npos) {
            return {arg, default_count};
        }

        return {
            arg.substr(0, colon),
            parsePositiveInt(std::string_view(arg).substr(colon + 1), default_count),
        };
    }

    static nlohmann::json commandResultToJson(const message::DialogueCommandProcessor::CommandResult& result) {
        return {
            {"command", result.command}, {"prefix", result.prefix},   {"argument", result.argument},
            {"handled", result.handled}, {"success", result.success}, {"code", result.code},
            {"message", result.message},
        };
    }

    static std::string questStateToString(QuestState state) {
        switch (state) {
        case QuestState::NotStarted:
            return "not_started";
        case QuestState::Active:
            return "active";
        case QuestState::Completed:
            return "completed";
        case QuestState::Failed:
            return "failed";
        }
        return "unknown";
    }

    std::unique_ptr<IPlatformSurface> m_platform;
    std::unique_ptr<RendererBackend> m_renderer;
    std::unique_ptr<SpriteBatcher> m_batcher;
    std::atomic<bool> m_isRunning;
    input::InputCore m_inputCore;
    audio::AudioCore m_audioCore;
    Clock m_clock;
    message::DialogueCommandProcessor m_dialogueProcessor;
    message::DialogueProjectLoadResult m_dialogueLoadResult;
    RuntimeStartupReport m_runtimeStartupReport;
    World m_dialogueCommandWorld;
    EntityID m_dialoguePlayerEntity = 0;
    HealthSystem m_dialogueHealthSystem;
    QuestSystem m_dialogueQuestSystem;
    std::vector<message::DialogueCommandProcessor::CommandResult> m_dialogueCommandHistory;

    static std::string dialogueSeverityToString(message::DialogueLoadSeverity severity) {
        switch (severity) {
        case message::DialogueLoadSeverity::Info:
            return "info";
        case message::DialogueLoadSeverity::Warning:
            return "warning";
        case message::DialogueLoadSeverity::Error:
            return "error";
        }
        return "unknown";
    }
};

} // namespace urpg
