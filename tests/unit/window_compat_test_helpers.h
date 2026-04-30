#pragma once

#include "engine/core/render/render_layer.h"
#include "runtimes/compat_js/data_manager.h"
#include "runtimes/compat_js/input_manager.h"
#include "runtimes/compat_js/quickjs_runtime.h"
#include "runtimes/compat_js/window_compat.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <string>
#include <type_traits>
#include <utility>

using namespace urpg::compat;

namespace {

template<typename LayerT> const auto& renderFrameCommands(const LayerT& layer) {
    if constexpr (requires { layer.getFrameCommands(); }) {
        return layer.getFrameCommands();
    } else {
        return layer.getCommands();
    }
}

template<typename StoredCommand> urpg::RenderCmdType renderCommandType(const StoredCommand& command) {
    if constexpr (requires { command->type; }) {
        return command->type;
    } else {
        return command.type;
    }
}

template<typename CommandT, typename StoredCommand> const CommandT* renderCommandAs(const StoredCommand& command) {
    if constexpr (requires { command.template tryGet<CommandT>(); }) {
        return command.template tryGet<CommandT>();
    } else if constexpr (requires { command.get(); }) {
        return dynamic_cast<const CommandT*>(command.get());
    } else if constexpr (std::is_pointer_v<std::remove_cvref_t<StoredCommand>>) {
        return dynamic_cast<const CommandT*>(command);
    } else if constexpr (std::is_base_of_v<CommandT, std::remove_cvref_t<StoredCommand>>) {
        return &command;
    } else {
        return nullptr;
    }
}

[[maybe_unused]] urpg::Value colorObject(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
    urpg::Object object;
    object["r"] = urpg::Value::Int(r);
    object["g"] = urpg::Value::Int(g);
    object["b"] = urpg::Value::Int(b);
    object["a"] = urpg::Value::Int(a);
    return urpg::Value::Obj(std::move(object));
}

[[maybe_unused]] urpg::Value stringValue(const std::string& value) {
    urpg::Value out;
    out.v = value;
    return out;
}

} // namespace
