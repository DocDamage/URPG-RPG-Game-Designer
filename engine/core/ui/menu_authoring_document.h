#pragma once

#include "engine/core/ui/menu_scene_graph.h"

#include <cstdint>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include <nlohmann/json.hpp>

namespace urpg::ui {

enum class MenuElementKind { Panel, Text, Button, Image, Progress, List, Slot };
enum class MenuVisualState { Default, Focus, Pressed, Disabled, Selected, Loading, Error };
enum class MenuTransitionInterruption { Replace, Queue, Blend };
enum class MenuBindingSource { Project, Runtime, Save, Localization };
enum class MenuBindingValueType { String, Integer, Number, Boolean };
enum class MenuCanvasAlignment { Left, HorizontalCenter, Right, Top, VerticalCenter, Bottom };
enum class MenuCanvasDistribution { Horizontal, Vertical };
enum class MenuInputPreview { KeyboardMouse, Controller, TouchLike };

using MenuBindingValue = std::variant<std::string, int64_t, double, bool>;

struct MenuStyleToken {
    std::string id;
    std::string value;
};

struct MenuStateStyle {
    MenuVisualState state = MenuVisualState::Default;
    std::map<std::string, std::string> properties;
};

struct MenuStateTransition {
    MenuVisualState from = MenuVisualState::Default;
    MenuVisualState to = MenuVisualState::Default;
    uint32_t duration_ms = 0;
    MenuTransitionInterruption interruption = MenuTransitionInterruption::Replace;
    std::string audio_hook;
    bool entrance = false;
    bool exit = false;
};

struct MenuDataBinding {
    std::string property;
    MenuBindingSource source = MenuBindingSource::Runtime;
    std::string path;
    MenuBindingValueType value_type = MenuBindingValueType::String;
    MenuBindingValue fallback = std::string{};
    std::string format;
    bool required = false;
};

struct MenuCanvasNode {
    std::string id;
    std::string parent_id;
    MenuElementKind kind = MenuElementKind::Panel;
    MenuPaneLayout layout;
    std::string label;
    std::string accessible_label;
    bool visible = true;
    bool enabled = true;
    bool focusable = false;
    bool required_action = false;
    std::string focus_next_id;
    MenuRouteTarget route = MenuRouteTarget::None;
    std::string custom_route_id;
    std::string component_id;
    uint32_t component_version = 0;
    std::map<std::string, std::string> instance_overrides;
    std::vector<MenuDataBinding> bindings;
    std::vector<MenuStateStyle> state_styles;
    std::vector<MenuStateTransition> transitions;
};

struct MenuCanvasMutationResult {
    bool changed = false;
    std::vector<std::string> diagnostics;
};

class MenuAuthoringDocument {
public:
    bool setCanvas(MenuDesignCanvas canvas);
    const MenuDesignCanvas& canvas() const { return canvas_; }
    bool setSafeAreaMargin(int margin);
    int safeAreaMargin() const { return safe_area_margin_; }
    bool setZoom(float zoom);
    float zoom() const { return zoom_; }
    void setSnapGrid(int grid);
    int snapGrid() const { return snap_grid_; }

    bool addNode(MenuCanvasNode node);
    bool removeNode(std::string_view id);
    const std::vector<MenuCanvasNode>& nodes() const { return nodes_; }
    MenuCanvasNode* findNode(std::string_view id);
    const MenuCanvasNode* findNode(std::string_view id) const;
    std::vector<const MenuCanvasNode*> childrenOf(std::string_view parent_id) const;
    bool select(std::vector<std::string> ids);
    const std::vector<std::string>& selection() const { return selection_; }

    MenuCanvasMutationResult moveSelection(int dx, int dy, bool snap);
    MenuCanvasMutationResult resizeNode(std::string_view id, int width, int height, bool snap);
    MenuCanvasMutationResult alignSelection(MenuCanvasAlignment alignment);
    MenuCanvasMutationResult distributeSelection(MenuCanvasDistribution distribution);
    bool updateNode(MenuCanvasNode node);
    bool canUndo() const { return !undo_.empty(); }
    bool canRedo() const { return !redo_.empty(); }
    bool undo();
    bool redo();

    nlohmann::json toJson() const;
    static std::optional<MenuAuthoringDocument> fromJson(const nlohmann::json& json,
                                                        std::vector<std::string>* diagnostics = nullptr);

private:
    struct State {
        MenuDesignCanvas canvas;
        int safe_area_margin = 0;
        float zoom = 1.0F;
        int snap_grid = 8;
        std::vector<MenuCanvasNode> nodes;
        std::vector<std::string> selection;
    };

    State capture() const;
    void restore(State state);
    void recordMutation();
    bool parentChainValid(const MenuCanvasNode& candidate) const;
    int snapped(int value) const;

    MenuDesignCanvas canvas_;
    int safe_area_margin_ = 32;
    float zoom_ = 1.0F;
    int snap_grid_ = 8;
    std::vector<MenuCanvasNode> nodes_;
    std::vector<std::string> selection_;
    std::vector<State> undo_;
    std::vector<State> redo_;
};

struct MenuComponentDefinition {
    std::string id;
    uint32_t version = 1;
    std::vector<std::string> slots;
    std::map<std::string, std::string> defaults;
    std::vector<MenuStateStyle> variants;
};

struct MenuComponentImpact {
    bool valid = false;
    std::vector<std::string> instance_ids;
    std::vector<std::string> preserved_override_keys;
    std::vector<std::string> diagnostics;
};

class MenuComponentLibrary {
public:
    bool define(MenuComponentDefinition definition);
    const MenuComponentDefinition* find(std::string_view id) const;
    std::optional<MenuCanvasNode> instantiate(std::string_view component_id, std::string instance_id,
                                             MenuPaneLayout layout,
                                             std::map<std::string, std::string> overrides = {}) const;
    MenuComponentImpact previewUpdate(const MenuAuthoringDocument& document,
                                      const MenuComponentDefinition& replacement) const;
    MenuComponentImpact applyUpdate(MenuAuthoringDocument& document, MenuComponentDefinition replacement);
    bool defineStyleToken(MenuStyleToken token);
    std::optional<std::string> resolveStyleToken(std::string_view id) const;

private:
    std::map<std::string, MenuComponentDefinition> definitions_;
    std::map<std::string, std::string> style_tokens_;
};

struct MenuResolvedTransition {
    uint32_t duration_ms = 0;
    MenuTransitionInterruption interruption = MenuTransitionInterruption::Replace;
    std::string audio_hook;
    bool immediate = true;
};

MenuResolvedTransition resolveMenuTransition(const MenuCanvasNode& node, MenuVisualState from,
                                             MenuVisualState to, bool reduced_motion,
                                             bool audio_enabled);

struct MenuBindingContext {
    std::map<std::string, MenuBindingValue> project;
    std::map<std::string, MenuBindingValue> runtime;
    std::map<std::string, MenuBindingValue> save;
    std::map<std::string, std::string> localization;
};

struct MenuBindingResolution {
    bool valid = false;
    bool used_fallback = false;
    std::string rendered;
    std::vector<std::string> diagnostics;
};

MenuBindingResolution resolveMenuBinding(const MenuDataBinding& binding,
                                         const MenuBindingContext& context);

struct MenuAuthoringAuditOptions {
    MenuDesignCanvas target_canvas{1280, 720};
    MenuInputPreview input = MenuInputPreview::KeyboardMouse;
    float text_expansion = 1.0F;
    bool safe_area_overlay = true;
};

struct MenuAuthoringIssue {
    std::string code;
    std::string node_id;
    std::string message;
    bool blocking = false;
};

struct MenuAuthoringAuditResult {
    bool package_safe = false;
    std::string controller_glyph_set;
    std::vector<MenuAuthoringIssue> issues;
};

MenuAuthoringAuditResult auditMenuAuthoringDocument(const MenuAuthoringDocument& document,
                                                    const MenuAuthoringAuditOptions& options);
std::vector<MenuDesignCanvas> menuTargetResolutionPresets();

struct MenuStarterTemplate {
    std::string id;
    MenuAuthoringDocument document;
    bool editable = true;
    bool package_safe = true;
    bool accessible = true;
};

class MenuStarterTemplateLibrary {
public:
    static MenuStarterTemplateLibrary originalUrpgTemplates();
    const std::vector<MenuStarterTemplate>& templates() const { return templates_; }
    const MenuStarterTemplate* find(std::string_view id) const;

private:
    std::vector<MenuStarterTemplate> templates_;
};

struct MenuRuntimeMaterialization {
    std::shared_ptr<MenuScene> scene;
    std::vector<std::string> disabled_command_ids;
    std::vector<std::string> hidden_command_ids;
    std::vector<std::string> diagnostics;
};

MenuRuntimeMaterialization materializeMenuAuthoringDocument(const MenuAuthoringDocument& document,
                                                            std::string scene_id,
                                                            const MenuBindingContext* binding_context = nullptr);

} // namespace urpg::ui
