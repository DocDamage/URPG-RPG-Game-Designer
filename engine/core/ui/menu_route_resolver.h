#pragma once

#include "ui_types.h"
#include <map>
#include <string>
#include <functional>
#include <iostream>

namespace urpg::ui {

/**
 * @brief Resolves MenuRouteTarget IDs into concrete actions.
 * 
 * In a native scenario, this system maps "Save" to the SavePanel view
 * and "Item" to the ItemSelection view, etc.
 */
class MenuRouteResolver {
public:
    using RouteCallback = std::function<void(const MenuCommandMeta&)>;

    /**
     * @brief Maps a native RouteTarget to a callback function.
     */
    void bindRoute(MenuRouteTarget target, RouteCallback callback) {
        _native_routes[target] = std::move(callback);
    }

    /**
     * @brief Maps a custom string RouteID to a callback function.
     */
    void bindCustomRoute(const std::string& custom_id, RouteCallback callback) {
        _custom_routes[custom_id] = std::move(callback);
    }

    /**
     * @brief Executes a jump to a specific route determined by the command.
     */
    bool resolve(const MenuCommandMeta& command) const {
        if (resolveTarget(command, command.route, command.custom_route_id)) {
            return true;
        }
        if (resolveTarget(command, command.fallback_route, command.fallback_custom_route_id)) {
            return true;
        }
        return false;
    }

private:
    bool resolveTarget(const MenuCommandMeta& command,
                       MenuRouteTarget target,
                       const std::string& customRouteId) const {
        if (target == MenuRouteTarget::None) {
            return false;
        }

        if (target == MenuRouteTarget::Custom) {
            const auto it = _custom_routes.find(customRouteId);
            if (it != _custom_routes.end()) {
                it->second(command);
                return true;
            }
            return false;
        }

        const auto it = _native_routes.find(target);
        if (it != _native_routes.end()) {
            it->second(command);
            return true;
        }
        return false;
    }

    std::map<MenuRouteTarget, RouteCallback> _native_routes;
    std::map<std::string, RouteCallback> _custom_routes;
};

} // namespace urpg
