#include "engine/core/ui/menu_authoring_document.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <sstream>
#include <unordered_set>

namespace urpg::ui {

namespace {

constexpr size_t kHistoryLimit = 128;

bool validId(std::string_view id) {
    if (id.empty() || id.size() > 128) return false;
    return std::all_of(id.begin(), id.end(), [](unsigned char ch) {
        return std::isalnum(ch) != 0 || ch == '_' || ch == '-' || ch == '.';
    });
}

template <typename T>
void pushBounded(std::vector<T>& history, T state) {
    if (history.size() == kHistoryLimit) history.erase(history.begin());
    history.push_back(std::move(state));
}

std::string kindName(MenuElementKind kind) {
    switch (kind) {
    case MenuElementKind::Panel: return "panel";
    case MenuElementKind::Text: return "text";
    case MenuElementKind::Button: return "button";
    case MenuElementKind::Image: return "image";
    case MenuElementKind::Progress: return "progress";
    case MenuElementKind::List: return "list";
    case MenuElementKind::Slot: return "slot";
    }
    return "panel";
}

MenuElementKind parseKind(std::string_view value) {
    if (value == "text") return MenuElementKind::Text;
    if (value == "button") return MenuElementKind::Button;
    if (value == "image") return MenuElementKind::Image;
    if (value == "progress") return MenuElementKind::Progress;
    if (value == "list") return MenuElementKind::List;
    if (value == "slot") return MenuElementKind::Slot;
    return MenuElementKind::Panel;
}

std::string sourceName(MenuBindingSource source) {
    switch (source) {
    case MenuBindingSource::Project: return "project";
    case MenuBindingSource::Runtime: return "runtime";
    case MenuBindingSource::Save: return "save";
    case MenuBindingSource::Localization: return "localization";
    }
    return "runtime";
}

MenuBindingSource parseSource(std::string_view value) {
    if (value == "project") return MenuBindingSource::Project;
    if (value == "save") return MenuBindingSource::Save;
    if (value == "localization") return MenuBindingSource::Localization;
    return MenuBindingSource::Runtime;
}

std::string typeName(MenuBindingValueType type) {
    switch (type) {
    case MenuBindingValueType::String: return "string";
    case MenuBindingValueType::Integer: return "integer";
    case MenuBindingValueType::Number: return "number";
    case MenuBindingValueType::Boolean: return "boolean";
    }
    return "string";
}

MenuBindingValueType parseType(std::string_view value) {
    if (value == "integer") return MenuBindingValueType::Integer;
    if (value == "number") return MenuBindingValueType::Number;
    if (value == "boolean") return MenuBindingValueType::Boolean;
    return MenuBindingValueType::String;
}

nlohmann::json bindingValueJson(const MenuBindingValue& value) {
    return std::visit([](const auto& item) { return nlohmann::json(item); }, value);
}

std::optional<MenuBindingValue> parseBindingValue(const nlohmann::json& value,
                                                  MenuBindingValueType type) {
    try {
        switch (type) {
        case MenuBindingValueType::String:
            if (value.is_string()) return value.get<std::string>();
            break;
        case MenuBindingValueType::Integer:
            if (value.is_number_integer()) return value.get<int64_t>();
            break;
        case MenuBindingValueType::Number:
            if (value.is_number()) return value.get<double>();
            break;
        case MenuBindingValueType::Boolean:
            if (value.is_boolean()) return value.get<bool>();
            break;
        }
    } catch (...) {
    }
    return std::nullopt;
}

bool valueMatches(const MenuBindingValue& value, MenuBindingValueType type) {
    switch (type) {
    case MenuBindingValueType::String: return std::holds_alternative<std::string>(value);
    case MenuBindingValueType::Integer: return std::holds_alternative<int64_t>(value);
    case MenuBindingValueType::Number:
        return std::holds_alternative<double>(value) || std::holds_alternative<int64_t>(value);
    case MenuBindingValueType::Boolean: return std::holds_alternative<bool>(value);
    }
    return false;
}

std::string renderValue(const MenuBindingValue& value) {
    return std::visit([](const auto& item) {
        using T = std::decay_t<decltype(item)>;
        if constexpr (std::is_same_v<T, bool>) {
            return std::string(item ? "true" : "false");
        } else if constexpr (std::is_same_v<T, double>) {
            std::ostringstream stream;
            stream << std::fixed << std::setprecision(2) << item;
            return stream.str();
        } else if constexpr (std::is_same_v<T, int64_t>) {
            return std::to_string(item);
        } else {
            return item;
        }
    }, value);
}

std::string applyFormat(std::string format, std::string value) {
    if (format.empty()) return value;
    const auto marker = format.find("{}");
    if (marker == std::string::npos) return format + value;
    format.replace(marker, 2, value);
    return format;
}

} // namespace

bool MenuAuthoringDocument::setCanvas(MenuDesignCanvas canvas) {
    if (!canvas.isValid()) return false;
    recordMutation();
    canvas_ = canvas;
    return true;
}

bool MenuAuthoringDocument::setSafeAreaMargin(int margin) {
    if (margin < 0 || margin * 2 >= canvas_.width || margin * 2 >= canvas_.height) return false;
    recordMutation();
    safe_area_margin_ = margin;
    return true;
}

bool MenuAuthoringDocument::setZoom(float zoom) {
    if (!std::isfinite(zoom) || zoom < 0.1F || zoom > 8.0F) return false;
    recordMutation();
    zoom_ = zoom;
    return true;
}

void MenuAuthoringDocument::setSnapGrid(int grid) {
    if (grid < 1 || grid > 256 || grid == snap_grid_) return;
    recordMutation();
    snap_grid_ = grid;
}

bool MenuAuthoringDocument::addNode(MenuCanvasNode node) {
    if (!validId(node.id) || findNode(node.id) || !node.layout.isValid() || !parentChainValid(node)) return false;
    recordMutation();
    nodes_.push_back(std::move(node));
    return true;
}

bool MenuAuthoringDocument::removeNode(std::string_view id) {
    if (!findNode(id)) return false;
    recordMutation();
    std::set<std::string> removed{std::string(id)};
    bool grew = true;
    while (grew) {
        grew = false;
        for (const auto& node : nodes_) {
            if (removed.contains(node.parent_id) && !removed.contains(node.id)) {
                removed.insert(node.id);
                grew = true;
            }
        }
    }
    std::erase_if(nodes_, [&](const MenuCanvasNode& node) { return removed.contains(node.id); });
    std::erase_if(selection_, [&](const std::string& selected) { return removed.contains(selected); });
    return true;
}

MenuCanvasNode* MenuAuthoringDocument::findNode(std::string_view id) {
    const auto found = std::find_if(nodes_.begin(), nodes_.end(),
                                    [&](const MenuCanvasNode& node) { return node.id == id; });
    return found == nodes_.end() ? nullptr : &*found;
}

const MenuCanvasNode* MenuAuthoringDocument::findNode(std::string_view id) const {
    const auto found = std::find_if(nodes_.begin(), nodes_.end(),
                                    [&](const MenuCanvasNode& node) { return node.id == id; });
    return found == nodes_.end() ? nullptr : &*found;
}

std::vector<const MenuCanvasNode*> MenuAuthoringDocument::childrenOf(std::string_view parent_id) const {
    std::vector<const MenuCanvasNode*> result;
    for (const auto& node : nodes_) if (node.parent_id == parent_id) result.push_back(&node);
    return result;
}

bool MenuAuthoringDocument::select(std::vector<std::string> ids) {
    std::sort(ids.begin(), ids.end());
    ids.erase(std::unique(ids.begin(), ids.end()), ids.end());
    if (std::any_of(ids.begin(), ids.end(), [&](const std::string& id) { return !findNode(id); })) return false;
    selection_ = std::move(ids);
    return true;
}

MenuCanvasMutationResult MenuAuthoringDocument::moveSelection(int dx, int dy, bool snap) {
    MenuCanvasMutationResult result;
    if (selection_.empty()) {
        result.diagnostics.push_back("No canvas nodes are selected.");
        return result;
    }
    recordMutation();
    for (const auto& id : selection_) {
        auto* node = findNode(id);
        node->layout.x = snap ? snapped(node->layout.x + dx) : node->layout.x + dx;
        node->layout.y = snap ? snapped(node->layout.y + dy) : node->layout.y + dy;
    }
    result.changed = true;
    return result;
}

MenuCanvasMutationResult MenuAuthoringDocument::resizeNode(std::string_view id, int width, int height,
                                                           bool snap) {
    MenuCanvasMutationResult result;
    auto* node = findNode(id);
    if (!node) {
        result.diagnostics.push_back("Canvas node was not found.");
        return result;
    }
    const int resolved_width = snap ? snapped(width) : width;
    const int resolved_height = snap ? snapped(height) : height;
    if (resolved_width < node->layout.min_width || resolved_height < node->layout.min_height ||
        resolved_width > 8192 || resolved_height > 8192) {
        result.diagnostics.push_back("Requested size violates the node constraints.");
        return result;
    }
    recordMutation();
    node = findNode(id);
    node->layout.width = resolved_width;
    node->layout.height = resolved_height;
    result.changed = true;
    return result;
}

MenuCanvasMutationResult MenuAuthoringDocument::alignSelection(MenuCanvasAlignment alignment) {
    MenuCanvasMutationResult result;
    if (selection_.size() < 2) {
        result.diagnostics.push_back("Alignment requires at least two selected nodes.");
        return result;
    }
    std::vector<MenuCanvasNode*> selected;
    for (const auto& id : selection_) selected.push_back(findNode(id));
    const auto& anchor = selected.front()->layout;
    recordMutation();
    for (auto* stale : selected) {
        auto* node = findNode(stale->id);
        switch (alignment) {
        case MenuCanvasAlignment::Left: node->layout.x = anchor.x; break;
        case MenuCanvasAlignment::HorizontalCenter:
            node->layout.x = anchor.x + (anchor.width - node->layout.width) / 2; break;
        case MenuCanvasAlignment::Right:
            node->layout.x = anchor.x + anchor.width - node->layout.width; break;
        case MenuCanvasAlignment::Top: node->layout.y = anchor.y; break;
        case MenuCanvasAlignment::VerticalCenter:
            node->layout.y = anchor.y + (anchor.height - node->layout.height) / 2; break;
        case MenuCanvasAlignment::Bottom:
            node->layout.y = anchor.y + anchor.height - node->layout.height; break;
        }
    }
    result.changed = true;
    return result;
}

MenuCanvasMutationResult MenuAuthoringDocument::distributeSelection(MenuCanvasDistribution distribution) {
    MenuCanvasMutationResult result;
    if (selection_.size() < 3) {
        result.diagnostics.push_back("Distribution requires at least three selected nodes.");
        return result;
    }
    std::vector<std::string> ordered = selection_;
    std::sort(ordered.begin(), ordered.end(), [&](const std::string& left, const std::string& right) {
        const auto* a = findNode(left);
        const auto* b = findNode(right);
        return distribution == MenuCanvasDistribution::Horizontal ? a->layout.x < b->layout.x
                                                                   : a->layout.y < b->layout.y;
    });
    const auto* first = findNode(ordered.front());
    const auto* last = findNode(ordered.back());
    const int start = distribution == MenuCanvasDistribution::Horizontal ? first->layout.x : first->layout.y;
    const int finish = distribution == MenuCanvasDistribution::Horizontal ? last->layout.x : last->layout.y;
    recordMutation();
    for (size_t index = 1; index + 1 < ordered.size(); ++index) {
        auto* node = findNode(ordered[index]);
        const int position = start + static_cast<int>((finish - start) * index / (ordered.size() - 1));
        if (distribution == MenuCanvasDistribution::Horizontal) node->layout.x = position;
        else node->layout.y = position;
    }
    result.changed = true;
    return result;
}

bool MenuAuthoringDocument::updateNode(MenuCanvasNode node) {
    auto* existing = findNode(node.id);
    if (!existing || !node.layout.isValid() || !parentChainValid(node)) return false;
    recordMutation();
    *findNode(node.id) = std::move(node);
    return true;
}

bool MenuAuthoringDocument::undo() {
    if (undo_.empty()) return false;
    pushBounded(redo_, capture());
    auto state = std::move(undo_.back());
    undo_.pop_back();
    restore(std::move(state));
    return true;
}

bool MenuAuthoringDocument::redo() {
    if (redo_.empty()) return false;
    pushBounded(undo_, capture());
    auto state = std::move(redo_.back());
    redo_.pop_back();
    restore(std::move(state));
    return true;
}

nlohmann::json MenuAuthoringDocument::toJson() const {
    nlohmann::json root{{"schema", "urpg.menu_authoring.v1"},
                        {"canvas", {{"width", canvas_.width}, {"height", canvas_.height}}},
                        {"safe_area_margin", safe_area_margin_}, {"zoom", zoom_},
                        {"snap_grid", snap_grid_}, {"nodes", nlohmann::json::array()}};
    for (const auto& node : nodes_) {
        nlohmann::json item{{"id", node.id}, {"parent_id", node.parent_id},
                            {"kind", kindName(node.kind)}, {"label", node.label},
                            {"accessible_label", node.accessible_label}, {"visible", node.visible},
                            {"enabled", node.enabled}, {"focusable", node.focusable},
                            {"required_action", node.required_action}, {"focus_next_id", node.focus_next_id},
                            {"route", static_cast<uint32_t>(node.route)}, {"custom_route_id", node.custom_route_id},
                            {"component_id", node.component_id}, {"component_version", node.component_version},
                            {"instance_overrides", node.instance_overrides},
                            {"layout", {{"x", node.layout.x}, {"y", node.layout.y},
                                        {"width", node.layout.width}, {"height", node.layout.height},
                                        {"z_order", node.layout.z_order}, {"focus_order", node.layout.focus_order},
                                        {"anchor_left", node.layout.anchor_left},
                                        {"anchor_top", node.layout.anchor_top},
                                        {"anchor_right", node.layout.anchor_right},
                                        {"anchor_bottom", node.layout.anchor_bottom},
                                        {"min_width", node.layout.min_width},
                                        {"min_height", node.layout.min_height}}},
                            {"bindings", nlohmann::json::array()},
                            {"state_styles", nlohmann::json::array()},
                            {"transitions", nlohmann::json::array()}};
        for (const auto& binding : node.bindings) {
            item["bindings"].push_back({{"property", binding.property},
                                         {"source", sourceName(binding.source)}, {"path", binding.path},
                                         {"value_type", typeName(binding.value_type)},
                                         {"fallback", bindingValueJson(binding.fallback)},
                                         {"format", binding.format}, {"required", binding.required}});
        }
        for (const auto& style : node.state_styles) {
            item["state_styles"].push_back({{"state", static_cast<int>(style.state)},
                                             {"properties", style.properties}});
        }
        for (const auto& transition : node.transitions) {
            item["transitions"].push_back({{"from", static_cast<int>(transition.from)},
                                            {"to", static_cast<int>(transition.to)},
                                            {"duration_ms", transition.duration_ms},
                                            {"interruption", static_cast<int>(transition.interruption)},
                                            {"audio_hook", transition.audio_hook},
                                            {"entrance", transition.entrance}, {"exit", transition.exit}});
        }
        root["nodes"].push_back(std::move(item));
    }
    return root;
}

std::optional<MenuAuthoringDocument> MenuAuthoringDocument::fromJson(
    const nlohmann::json& json, std::vector<std::string>* diagnostics) {
    auto fail = [&](std::string message) -> std::optional<MenuAuthoringDocument> {
        if (diagnostics) diagnostics->push_back(std::move(message));
        return std::nullopt;
    };
    try {
        if (!json.is_object() || json.value("schema", "") != "urpg.menu_authoring.v1" ||
            !json.contains("canvas") || !json.contains("nodes") || !json["nodes"].is_array()) {
            return fail("Menu authoring document schema is invalid.");
        }
        MenuAuthoringDocument document;
        document.canvas_ = {json["canvas"].value("width", 0), json["canvas"].value("height", 0)};
        document.safe_area_margin_ = json.value("safe_area_margin", 32);
        document.zoom_ = json.value("zoom", 1.0F);
        document.snap_grid_ = json.value("snap_grid", 8);
        if (!document.canvas_.isValid() || document.safe_area_margin_ < 0 ||
            document.safe_area_margin_ * 2 >= document.canvas_.width ||
            document.safe_area_margin_ * 2 >= document.canvas_.height ||
            !std::isfinite(document.zoom_) || document.zoom_ < 0.1F || document.zoom_ > 8.0F ||
            document.snap_grid_ < 1 || document.snap_grid_ > 256) {
            return fail("Menu authoring canvas settings are invalid.");
        }
        for (const auto& item : json["nodes"]) {
            MenuCanvasNode node;
            node.id = item.value("id", "");
            node.parent_id = item.value("parent_id", "");
            node.kind = parseKind(item.value("kind", "panel"));
            node.label = item.value("label", "");
            node.accessible_label = item.value("accessible_label", "");
            node.visible = item.value("visible", true);
            node.enabled = item.value("enabled", true);
            node.focusable = item.value("focusable", false);
            node.required_action = item.value("required_action", false);
            node.focus_next_id = item.value("focus_next_id", "");
            const auto route = item.value("route", 0U);
            if ((route > static_cast<uint32_t>(MenuRouteTarget::Encyclopedia) &&
                 route != static_cast<uint32_t>(MenuRouteTarget::Custom)))
                return fail("Menu command route is invalid.");
            node.route = static_cast<MenuRouteTarget>(route);
            node.custom_route_id = item.value("custom_route_id", "");
            if (node.route == MenuRouteTarget::Custom && !validId(node.custom_route_id))
                return fail("Custom menu command route ID is invalid.");
            node.component_id = item.value("component_id", "");
            node.component_version = item.value("component_version", 0U);
            node.instance_overrides = item.value("instance_overrides", std::map<std::string, std::string>{});
            const auto& layout = item.at("layout");
            node.layout.x = layout.value("x", 0); node.layout.y = layout.value("y", 0);
            node.layout.width = layout.value("width", 320); node.layout.height = layout.value("height", 180);
            node.layout.z_order = layout.value("z_order", 0); node.layout.focus_order = layout.value("focus_order", -1);
            node.layout.anchor_left = layout.value("anchor_left", true);
            node.layout.anchor_top = layout.value("anchor_top", true);
            node.layout.anchor_right = layout.value("anchor_right", false);
            node.layout.anchor_bottom = layout.value("anchor_bottom", false);
            node.layout.min_width = layout.value("min_width", 1);
            node.layout.min_height = layout.value("min_height", 1);
            if (item.contains("bindings")) {
                for (const auto& source : item["bindings"]) {
                    MenuDataBinding binding;
                    binding.property = source.value("property", "");
                    binding.source = parseSource(source.value("source", "runtime"));
                    binding.path = source.value("path", "");
                    binding.value_type = parseType(source.value("value_type", "string"));
                    const auto fallback = parseBindingValue(source.at("fallback"), binding.value_type);
                    if (!fallback) return fail("Menu binding fallback type is invalid.");
                    binding.fallback = *fallback;
                    binding.format = source.value("format", "");
                    binding.required = source.value("required", false);
                    node.bindings.push_back(std::move(binding));
                }
            }
            if (item.contains("state_styles")) {
                for (const auto& source : item["state_styles"]) {
                    const int state = source.value("state", -1);
                    if (state < 0 || state > static_cast<int>(MenuVisualState::Error))
                        return fail("Menu visual state is invalid.");
                    node.state_styles.push_back({static_cast<MenuVisualState>(state),
                        source.value("properties", std::map<std::string, std::string>{})});
                }
            }
            if (item.contains("transitions")) {
                for (const auto& source : item["transitions"]) {
                    const int from = source.value("from", -1);
                    const int to = source.value("to", -1);
                    const int interruption = source.value("interruption", -1);
                    if (from < 0 || from > static_cast<int>(MenuVisualState::Error) || to < 0 ||
                        to > static_cast<int>(MenuVisualState::Error) || interruption < 0 ||
                        interruption > static_cast<int>(MenuTransitionInterruption::Blend))
                        return fail("Menu transition state or interruption policy is invalid.");
                    node.transitions.push_back({static_cast<MenuVisualState>(from),
                        static_cast<MenuVisualState>(to), source.value("duration_ms", 0U),
                        static_cast<MenuTransitionInterruption>(interruption),
                        source.value("audio_hook", ""), source.value("entrance", false),
                        source.value("exit", false)});
                }
            }
            if (!document.addNode(std::move(node))) return fail("Menu node hierarchy or geometry is invalid.");
            document.undo_.clear();
        }
        return document;
    } catch (...) {
        return fail("Menu authoring document could not be parsed.");
    }
}

MenuAuthoringDocument::State MenuAuthoringDocument::capture() const {
    return {canvas_, safe_area_margin_, zoom_, snap_grid_, nodes_, selection_};
}

void MenuAuthoringDocument::restore(State state) {
    canvas_ = state.canvas;
    safe_area_margin_ = state.safe_area_margin;
    zoom_ = state.zoom;
    snap_grid_ = state.snap_grid;
    nodes_ = std::move(state.nodes);
    selection_ = std::move(state.selection);
}

void MenuAuthoringDocument::recordMutation() {
    pushBounded(undo_, capture());
    redo_.clear();
}

bool MenuAuthoringDocument::parentChainValid(const MenuCanvasNode& candidate) const {
    if (candidate.parent_id.empty()) return true;
    const auto* parent = findNode(candidate.parent_id);
    if (!parent || candidate.parent_id == candidate.id) return false;
    std::unordered_set<std::string> visited{candidate.id};
    while (parent) {
        if (!visited.insert(parent->id).second) return false;
        if (parent->parent_id.empty()) return true;
        parent = findNode(parent->parent_id);
    }
    return false;
}

int MenuAuthoringDocument::snapped(int value) const {
    return static_cast<int>(std::lround(static_cast<double>(value) / snap_grid_)) * snap_grid_;
}

bool MenuComponentLibrary::define(MenuComponentDefinition definition) {
    if (!validId(definition.id) || definition.version == 0) return false;
    std::set<std::string> slots;
    for (const auto& slot : definition.slots) if (!validId(slot) || !slots.insert(slot).second) return false;
    auto found = definitions_.find(definition.id);
    if (found != definitions_.end() && definition.version <= found->second.version) return false;
    definitions_[definition.id] = std::move(definition);
    return true;
}

const MenuComponentDefinition* MenuComponentLibrary::find(std::string_view id) const {
    const auto found = definitions_.find(std::string(id));
    return found == definitions_.end() ? nullptr : &found->second;
}

std::optional<MenuCanvasNode> MenuComponentLibrary::instantiate(
    std::string_view component_id, std::string instance_id, MenuPaneLayout layout,
    std::map<std::string, std::string> overrides) const {
    const auto* definition = find(component_id);
    if (!definition || !validId(instance_id) || !layout.isValid()) return std::nullopt;
    MenuCanvasNode node;
    node.id = std::move(instance_id);
    node.kind = MenuElementKind::Panel;
    node.layout = layout;
    node.component_id = definition->id;
    node.component_version = definition->version;
    node.instance_overrides = std::move(overrides);
    node.state_styles = definition->variants;
    const auto label = node.instance_overrides.find("label");
    const auto default_label = definition->defaults.find("label");
    node.label = label != node.instance_overrides.end() ? label->second
               : default_label != definition->defaults.end() ? default_label->second : definition->id;
    node.accessible_label = node.label;
    return node;
}

MenuComponentImpact MenuComponentLibrary::previewUpdate(
    const MenuAuthoringDocument& document, const MenuComponentDefinition& replacement) const {
    MenuComponentImpact impact;
    const auto* current = find(replacement.id);
    if (!current) {
        impact.diagnostics.push_back("Component definition does not exist.");
        return impact;
    }
    if (replacement.version <= current->version) {
        impact.diagnostics.push_back("Component update version must increase.");
        return impact;
    }
    impact.valid = true;
    std::set<std::string> override_keys;
    for (const auto& node : document.nodes()) {
        if (node.component_id != replacement.id) continue;
        impact.instance_ids.push_back(node.id);
        for (const auto& [key, value] : node.instance_overrides) {
            (void)value;
            override_keys.insert(key);
        }
    }
    impact.preserved_override_keys.assign(override_keys.begin(), override_keys.end());
    return impact;
}

MenuComponentImpact MenuComponentLibrary::applyUpdate(MenuAuthoringDocument& document,
                                                       MenuComponentDefinition replacement) {
    auto impact = previewUpdate(document, replacement);
    if (!impact.valid) return impact;
    for (const auto& id : impact.instance_ids) {
        auto node = *document.findNode(id);
        node.component_version = replacement.version;
        node.state_styles = replacement.variants;
        if (!node.instance_overrides.contains("label")) {
            const auto label = replacement.defaults.find("label");
            if (label != replacement.defaults.end()) node.label = label->second;
        }
        if (!document.updateNode(std::move(node))) {
            impact.valid = false;
            impact.diagnostics.push_back("Component instance update failed for " + id + ".");
            return impact;
        }
    }
    definitions_[replacement.id] = std::move(replacement);
    return impact;
}

bool MenuComponentLibrary::defineStyleToken(MenuStyleToken token) {
    if (!validId(token.id) || token.value.empty() || token.value.size() > 256) return false;
    style_tokens_[std::move(token.id)] = std::move(token.value);
    return true;
}

std::optional<std::string> MenuComponentLibrary::resolveStyleToken(std::string_view id) const {
    const auto found = style_tokens_.find(std::string(id));
    return found == style_tokens_.end() ? std::nullopt : std::optional(found->second);
}

MenuResolvedTransition resolveMenuTransition(const MenuCanvasNode& node, MenuVisualState from,
                                             MenuVisualState to, bool reduced_motion,
                                             bool audio_enabled) {
    const auto found = std::find_if(node.transitions.begin(), node.transitions.end(),
                                    [&](const MenuStateTransition& transition) {
                                        return transition.from == from && transition.to == to;
                                    });
    if (found == node.transitions.end()) return {};
    MenuResolvedTransition result{std::min(found->duration_ms, 2000U), found->interruption,
                                  audio_enabled ? found->audio_hook : std::string{}, false};
    if (reduced_motion) result.duration_ms = 0;
    result.immediate = result.duration_ms == 0;
    return result;
}

MenuBindingResolution resolveMenuBinding(const MenuDataBinding& binding,
                                         const MenuBindingContext& context) {
    MenuBindingResolution result;
    std::optional<MenuBindingValue> value;
    if (binding.source == MenuBindingSource::Localization) {
        const auto found = context.localization.find(binding.path);
        if (found != context.localization.end()) value = found->second;
    } else {
        const std::map<std::string, MenuBindingValue>* source = &context.runtime;
        if (binding.source == MenuBindingSource::Project) source = &context.project;
        else if (binding.source == MenuBindingSource::Save) source = &context.save;
        const auto found = source->find(binding.path);
        if (found != source->end()) value = found->second;
    }
    if (!value) {
        result.used_fallback = true;
        value = binding.fallback;
        result.diagnostics.push_back("Binding source is missing: " + sourceName(binding.source) + "." + binding.path);
        if (binding.required) result.diagnostics.push_back("Required binding cannot be packaged with fallback-only data.");
    }
    if (!valueMatches(*value, binding.value_type)) {
        result.diagnostics.push_back("Binding value type does not match its declaration.");
        result.rendered = applyFormat(binding.format, renderValue(binding.fallback));
        return result;
    }
    result.rendered = applyFormat(binding.format, renderValue(*value));
    result.valid = !(binding.required && result.used_fallback);
    return result;
}

MenuAuthoringAuditResult auditMenuAuthoringDocument(const MenuAuthoringDocument& document,
                                                    const MenuAuthoringAuditOptions& options) {
    MenuAuthoringAuditResult result;
    result.controller_glyph_set = options.input == MenuInputPreview::Controller ? "generic-controller"
                                 : options.input == MenuInputPreview::TouchLike ? "touch-like"
                                                                               : "keyboard-mouse";
    auto issue = [&](std::string code, const MenuCanvasNode& node, std::string message, bool blocking) {
        result.issues.push_back({std::move(code), node.id, std::move(message), blocking});
    };
    std::vector<const MenuCanvasNode*> focusable;
    for (const auto& node : document.nodes()) {
        const auto resolved = resolveMenuPaneLayoutForCanvas(node.layout, document.canvas(), options.target_canvas);
        if (resolved.x < 0 || resolved.y < 0 || resolved.x + resolved.width > options.target_canvas.width ||
            resolved.y + resolved.height > options.target_canvas.height) {
            issue("target_overflow", node, "Node overflows the selected target resolution.", true);
        }
        if (options.safe_area_overlay && node.required_action &&
            (resolved.x < document.safeAreaMargin() || resolved.y < document.safeAreaMargin() ||
             resolved.x + resolved.width > options.target_canvas.width - document.safeAreaMargin() ||
             resolved.y + resolved.height > options.target_canvas.height - document.safeAreaMargin())) {
            issue("unsafe_required_action", node, "Required action is outside the safe area.", true);
        }
        if (node.focusable) {
            focusable.push_back(&node);
            if (node.accessible_label.empty()) issue("missing_label", node, "Focusable node lacks an accessible label.", true);
            if (node.focus_next_id.empty() || !document.findNode(node.focus_next_id))
                issue("broken_focus_edge", node, "Focusable node has no valid next-focus edge.", true);
        }
        const float estimated_text_width = static_cast<float>(node.label.size()) * 8.0F * options.text_expansion;
        if (!node.label.empty() && estimated_text_width > static_cast<float>(resolved.width))
            issue("text_overflow", node, "Expanded preview text does not fit the node.", true);
    }
    if (!focusable.empty()) {
        std::set<std::string> visited;
        const MenuCanvasNode* cursor = focusable.front();
        while (cursor && visited.insert(cursor->id).second) cursor = document.findNode(cursor->focus_next_id);
        for (const auto* node : focusable) {
            if (!visited.contains(node->id)) issue("focus_trap", *node, "Node is unreachable from the focus cycle.", true);
        }
    }
    result.package_safe = std::none_of(result.issues.begin(), result.issues.end(),
                                       [](const MenuAuthoringIssue& item) { return item.blocking; });
    return result;
}

std::vector<MenuDesignCanvas> menuTargetResolutionPresets() {
    return {{1280, 720}, {1920, 1080}, {2560, 1440}, {3840, 2160}, {1024, 768}};
}

MenuStarterTemplateLibrary MenuStarterTemplateLibrary::originalUrpgTemplates() {
    MenuStarterTemplateLibrary library;
    const std::vector<std::string> ids = {"title", "save_load", "settings", "pause", "inventory",
        "equipment", "quest_log", "dialogue", "shop", "battle_hud", "results"};
    for (const auto& id : ids) {
        MenuStarterTemplate item;
        item.id = id;
        MenuCanvasNode root;
        root.id = id + ".root";
        root.kind = MenuElementKind::Panel;
        root.layout = {32, 32, 1216, 656};
        root.layout.anchor_right = true;
        root.layout.anchor_bottom = true;
        root.layout.min_width = 320;
        root.layout.min_height = 240;
        MenuCanvasNode action;
        action.id = id + ".primary";
        action.parent_id = root.id;
        action.kind = MenuElementKind::Button;
        action.layout = {64, 560, 320, 64};
        action.layout.anchor_top = false;
        action.layout.anchor_bottom = true;
        action.label = id + " primary";
        action.accessible_label = action.label;
        action.focusable = true;
        action.required_action = true;
        action.focus_next_id = action.id;
        action.route = MenuRouteTarget::Custom;
        action.custom_route_id = "template." + id + ".primary";
        item.document.addNode(std::move(root));
        item.document.addNode(std::move(action));
        item.document.select({id + ".primary"});
        const auto audit = auditMenuAuthoringDocument(item.document, {});
        item.package_safe = audit.package_safe;
        item.accessible = audit.package_safe;
        library.templates_.push_back(std::move(item));
    }
    return library;
}

const MenuStarterTemplate* MenuStarterTemplateLibrary::find(std::string_view id) const {
    const auto found = std::find_if(templates_.begin(), templates_.end(),
                                    [&](const MenuStarterTemplate& item) { return item.id == id; });
    return found == templates_.end() ? nullptr : &*found;
}

MenuRuntimeMaterialization materializeMenuAuthoringDocument(const MenuAuthoringDocument& document,
                                                            std::string scene_id,
                                                            const MenuBindingContext* binding_context) {
    MenuRuntimeMaterialization result;
    if (!validId(scene_id)) {
        result.diagnostics.push_back("Runtime menu scene ID is invalid.");
        return result;
    }
    auto scene = std::make_shared<MenuScene>(std::move(scene_id));
    scene->setDesignCanvas(document.canvas());
    const auto roots = document.childrenOf({});
    for (const auto* root : roots) {
        if (root->kind != MenuElementKind::Panel && root->kind != MenuElementKind::List) continue;
        MenuPane pane;
        pane.id = root->id;
        pane.displayName = root->label.empty() ? root->id : root->label;
        pane.isVisible = root->visible;
        pane.layout = root->layout;
        std::vector<const MenuCanvasNode*> descendants;
        std::vector<std::string> frontier{root->id};
        while (!frontier.empty()) {
            const auto parent = frontier.back();
            frontier.pop_back();
            for (const auto* child : document.childrenOf(parent)) {
                frontier.push_back(child->id);
                if (child->kind == MenuElementKind::Button) descendants.push_back(child);
            }
        }
        std::stable_sort(descendants.begin(), descendants.end(), [](const auto* left, const auto* right) {
            if (left->layout.focus_order != right->layout.focus_order)
                return left->layout.focus_order < right->layout.focus_order;
            return left->id < right->id;
        });
        for (const auto* button : descendants) {
            bool visible = button->visible;
            bool enabled = button->enabled;
            std::string resolved_label = button->label;
            for (const auto& binding : button->bindings) {
                const auto resolution = resolveMenuBinding(binding, binding_context ? *binding_context : MenuBindingContext{});
                for (const auto& diagnostic : resolution.diagnostics)
                    result.diagnostics.push_back(button->id + ": " + diagnostic);
                if (binding.property == "label") resolved_label = resolution.rendered;
                else if (binding.property == "enabled") enabled = resolution.rendered == "true";
                else if (binding.property == "visible") visible = resolution.rendered == "true";
                else result.diagnostics.push_back(button->id + ": unsupported runtime binding property " + binding.property + ".");
            }
            if (!visible) {
                result.hidden_command_ids.push_back(button->id);
                continue;
            }
            if (resolved_label.empty()) {
                result.diagnostics.push_back(button->id + ": runtime button label is missing.");
                continue;
            }
            MenuCommandMeta command;
            command.id = button->id;
            command.label = std::move(resolved_label);
            command.route = button->route;
            command.custom_route_id = button->custom_route_id;
            pane.commands.push_back(std::move(command));
            if (!enabled) result.disabled_command_ids.push_back(button->id);
        }
        scene->addPane(pane);
    }
    if (scene->getPanes().empty()) {
        result.diagnostics.push_back("Menu authoring document has no root panel or list to materialize.");
        return result;
    }
    bool active_set = false;
    for (auto& pane : scene->getPanesMutable()) {
        if (!active_set && pane.isVisible && !pane.commands.empty()) {
            pane.isActive = true;
            active_set = true;
        }
    }
    if (!active_set) result.diagnostics.push_back("Materialized menu has no visible navigable pane.");
    result.scene = std::move(scene);
    return result;
}

} // namespace urpg::ui
