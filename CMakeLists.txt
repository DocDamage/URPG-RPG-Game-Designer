cmake_minimum_required(VERSION 3.23)
project(urpg LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/FS /EHsc)
endif()

option(URPG_FETCH_CATCH2 "Fetch Catch2 via CMake" ON)
option(URPG_USE_SCCACHE "Use sccache as compiler launcher when available" ON)
option(
    URPG_WINDOWS_UNLOCK_TEST_BINARIES
    "On Windows, stop stale local test runner processes before relinking test executables"
    ON
)

if(URPG_USE_SCCACHE)
    find_program(SCCACHE_PROGRAM NAMES sccache)
    if(SCCACHE_PROGRAM)
        if(NOT CMAKE_C_COMPILER_LAUNCHER)
            set(CMAKE_C_COMPILER_LAUNCHER "${SCCACHE_PROGRAM}" CACHE STRING "C compiler launcher")
        endif()
        if(NOT CMAKE_CXX_COMPILER_LAUNCHER)
            set(CMAKE_CXX_COMPILER_LAUNCHER "${SCCACHE_PROGRAM}" CACHE STRING "C++ compiler launcher")
        endif()
        message(STATUS "URPG: using sccache compiler launcher: ${SCCACHE_PROGRAM}")
    else()
        message(WARNING "URPG_USE_SCCACHE=ON but 'sccache' was not found on PATH")
    endif()
endif()

if(WIN32 AND URPG_WINDOWS_UNLOCK_TEST_BINARIES)
    find_program(URPG_POWERSHELL_EXE NAMES pwsh powershell)
    if(NOT URPG_POWERSHELL_EXE)
        message(WARNING "URPG_WINDOWS_UNLOCK_TEST_BINARIES=ON but no PowerShell executable was found")
    endif()
endif()

function(urpg_enable_windows_test_binary_unlock target_name)
    if(NOT WIN32 OR NOT URPG_WINDOWS_UNLOCK_TEST_BINARIES OR NOT URPG_POWERSHELL_EXE)
        return()
    endif()

    add_custom_command(
        TARGET ${target_name}
        PRE_LINK
        COMMAND
            ${URPG_POWERSHELL_EXE}
            -NoProfile
            -ExecutionPolicy
            Bypass
            -File
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/ci/unlock-test-binary.ps1"
            -ProcessName
            "$<TARGET_FILE_BASE_NAME:${target_name}>"
            -BinaryPath
            "$<TARGET_FILE:${target_name}>"
        VERBATIM
    )
endfunction()

add_library(urpg_core
    engine/core/debug/debug_session.cpp
    engine/core/engine_shell.cpp
    engine/core/global_state_hub.cpp
    engine/core/platform/opengl_renderer.cpp
    engine/core/platform/sdl_surface.cpp
    engine/core/platform/gl_shader.cpp
    engine/core/platform/gl_texture.cpp
    engine/core/render/asset_loader.cpp
    engine/core/render/tilemap_renderer.cpp
    engine/core/render/sprite_animator.cpp
    engine/core/sprite_batcher.cpp
    engine/core/clock.cpp
    engine/core/semver.cpp
    engine/core/threading/thread_roles.cpp
    engine/core/events/event_edit_guard.cpp
    engine/core/events/event_runtime.cpp
    engine/core/format/canonical_json.cpp
    engine/core/migrate/migration_runner.cpp
    engine/core/sync/source_authority.cpp
    engine/core/save/save_catalog.cpp
    engine/core/save/save_journal.cpp
    engine/core/save/save_recovery.cpp
    engine/core/save/save_runtime.cpp
    engine/core/battle/battle_core.cpp
    engine/gameplay/combat/combat_calc.cpp
    engine/core/ui/ui_window.cpp
    engine/core/ui/ui_command_list.cpp
    engine/core/message/message_core.cpp
    engine/core/message/message_migration.cpp
    editor/diagnostics/event_authority_diagnostics.cpp
    editor/diagnostics/event_authority_panel.cpp
    editor/diagnostics/diagnostics_workspace.cpp
    editor/diagnostics/diagnostics_facade.cpp
    editor/compat/compat_report_panel.cpp
    editor/save/save_inspector_model.cpp
    editor/save/save_inspector_panel.cpp
    editor/battle/battle_inspector_model.cpp
    editor/battle/battle_preview_panel.cpp
    editor/battle/battle_inspector_panel.cpp
    editor/message/message_inspector_model.cpp
    editor/message/message_inspector_panel.cpp
    editor/ui/menu_inspector_model.cpp
    editor/ui/menu_inspector_panel.cpp
    editor/ui/menu_preview_panel.cpp
    editor/ability/ability_inspector_model.cpp
    editor/ability/ability_inspector_panel.cpp
    editor/ability/pattern_field_model.cpp
    editor/ability/pattern_field_panel.cpp
    engine/core/ability/gameplay_ability.cpp
    engine/core/ability/ability_state_machine.cpp
    engine/core/ability/pattern_field.cpp
    engine/core/ability/pattern_field_presets.cpp
    engine/core/level/level_assembly.cpp
    engine/core/ui/menu_serializer.cpp
    engine/core/scene/battle_scene.cpp
    engine/core/scene/map_scene.cpp
    engine/core/scene/menu_scene.cpp
    runtimes/compat_js/quickjs_runtime.cpp
    runtimes/compat_js/window_compat.cpp
    runtimes/compat_js/global_state_bridge.cpp
    runtimes/compat_js/battle_manager.cpp
    runtimes/compat_js/data_manager.cpp
    runtimes/compat_js/audio_manager.cpp
    runtimes/compat_js/input_manager.cpp
    runtimes/compat_js/plugin_manager.cpp
)

target_include_directories(urpg_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

enable_testing()

if(URPG_FETCH_CATCH2)
    include(FetchContent)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.6.0
    )
    FetchContent_MakeAvailable(Catch2)
else()
    find_package(Catch2 3 REQUIRED)
endif()

find_package(nlohmann_json QUIET)
if(NOT nlohmann_json_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
    )
    FetchContent_MakeAvailable(nlohmann_json)
endif()

target_link_libraries(urpg_core PUBLIC nlohmann_json::nlohmann_json)

# Platform Dependencies (SDL2, OpenGL)
option(URPG_SKIP_OPENGL "Skip OpenGL requirement (for headless CI builds)" OFF)
set(SDL2_DISABLE_INSTALL ON CACHE BOOL "" FORCE)
set(SDL2_TESTS OFF CACHE BOOL "" FORCE)
find_package(SDL2 QUIET)
if(NOT SDL2_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        SDL2
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG release-2.30.0 # Stable baseline for URPG
    )
    FetchContent_MakeAvailable(SDL2)
endif()

# Fetch stb_image
include(FetchContent)
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG master
)
FetchContent_MakeAvailable(stb)

target_include_directories(urpg_core PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${stb_SOURCE_DIR}
)

if(NOT URPG_SKIP_OPENGL)
    find_package(OpenGL REQUIRED)
endif()

if(TARGET SDL2::SDL2)
    set(URPG_SDL_TARGET SDL2::SDL2)
elseif(TARGET SDL2)
    set(URPG_SDL_TARGET SDL2)
endif()

target_link_libraries(urpg_core PUBLIC 
    nlohmann_json::nlohmann_json 
    ${URPG_SDL_TARGET}
)

if(NOT URPG_SKIP_OPENGL)
    target_link_libraries(urpg_core PUBLIC OpenGL::GL)
else()
    target_compile_definitions(urpg_core PUBLIC URPG_HEADLESS=1)
    message(STATUS "URPG: Building in HEADLESS mode (OpenGL skipped)")
endif()

add_executable(urpg_migrate
    tools/migrate/migrate_cli.cpp
)

target_link_libraries(urpg_migrate PRIVATE urpg_core)
target_include_directories(urpg_migrate PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(urpg_tests
    tests/unit/test_engine_shell.cpp
    tests/unit/test_global_state.cpp
    tests/unit/test_semver.cpp
    tests/unit/test_fixed32.cpp
    tests/unit/test_frame_budget.cpp
    tests/unit/test_ecs_world.cpp
    tests/unit/test_ai_system.cpp
    tests/unit/test_movement_system.cpp
    tests/unit/test_player_control_system.cpp
    tests/unit/test_collision_system.cpp
    tests/unit/test_lighting_system.cpp
    tests/unit/test_camera_system.cpp
    tests/unit/test_camera_follow_system.cpp
    tests/unit/test_camera_shake_system.cpp
    tests/unit/test_animation_system.cpp
    tests/unit/test_system_orchestrator.cpp
    tests/unit/test_interaction_system.cpp
    tests/unit/test_health_system.cpp
    tests/unit/test_progression_system.cpp
    tests/unit/test_status_effects.cpp
    tests/unit/test_trigger_system.cpp
    tests/unit/test_combat_calc.cpp
    tests/unit/test_battle_core.cpp
    tests/unit/test_bridge_roundtrip.cpp
    tests/unit/test_thread_roles.cpp
    tests/unit/test_render_tier.cpp
    tests/unit/test_save_journal.cpp
    tests/unit/test_canonical_json.cpp
    tests/unit/test_migration_runner.cpp
    tests/unit/test_source_authority.cpp
    tests/unit/test_diagnostics_facade.cpp
    tests/unit/test_save_catalog.cpp
    tests/unit/test_save_inspector_model.cpp
    tests/unit/test_save_inspector_panel.cpp
    tests/unit/test_save_recovery.cpp
    tests/unit/test_save_runtime.cpp
    tests/unit/test_save_serialization.cpp
    tests/unit/test_battle_inspector_model.cpp
    tests/unit/test_battle_preview_panel.cpp
    tests/unit/test_battle_inspector_panel.cpp
    tests/unit/test_engine_shell.cpp
    tests/unit/test_event_edit_guard.cpp
    tests/unit/test_event_authority_diagnostics.cpp
    tests/unit/test_event_authority_panel.cpp
    tests/unit/test_diagnostics_workspace.cpp
    tests/unit/test_event_runtime.cpp
    tests/unit/test_debug_session.cpp
    tests/unit/test_quickjs_runtime.cpp
    tests/unit/test_window_compat.cpp
    tests/unit/test_compat_report_panel.cpp
    tests/unit/test_battlemgr.cpp
    tests/unit/test_battle_scene_native.cpp
    tests/unit/test_data_manager.cpp
    tests/unit/test_audio_manager.cpp
    tests/unit/test_input_manager.cpp
    tests/unit/test_plugin_manager.cpp
    tests/unit/test_menu_core.cpp
    tests/unit/test_input_core.cpp
    tests/unit/test_audio_core.cpp
    tests/unit/test_message_text_core.cpp
    tests/unit/test_message_migration.cpp
    tests/unit/test_message_schema_contracts.cpp
    tests/unit/test_message_inspector_model.cpp
    tests/unit/test_message_inspector_panel.cpp
    tests/unit/test_menu_inspector_model.cpp
    tests/unit/test_menu_legacy_import.cpp
    tests/unit/test_state_audio_sync.cpp
    tests/unit/test_scene_manager.cpp
    tests/unit/test_gameplay_tags.cpp
    tests/unit/test_ability_activation.cpp
    tests/unit/test_pattern_field.cpp
    tests/unit/test_wave3_gaf.cpp
    tests/unit/test_level_assembly.cpp
    tests/unit/test_debug_crash.cpp
    tests/unit/test_audio_inspector.cpp
    tests/unit/test_audio_resolver.cpp
    tests/unit/test_save_state_sync.cpp
    tests/unit/test_battle_migration.cpp
    tests/unit/test_menu_migration_logic.cpp
    tests/unit/test_migration_wizard.cpp
    tests/unit/test_ai_bridge_regex.cpp
    tests/engine/core/test_render_assets.cpp
    tests/engine/core/test_engine_shell.cpp
)

target_link_libraries(urpg_tests PRIVATE urpg_core Catch2::Catch2WithMain)
target_include_directories(urpg_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(urpg_tests PRIVATE URPG_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
urpg_enable_windows_test_binary_unlock(urpg_tests)

add_executable(urpg_integration_tests
    tests/integration/test_integration_runtime_recovery.cpp
)

target_link_libraries(urpg_integration_tests PRIVATE urpg_core Catch2::Catch2WithMain)
target_include_directories(urpg_integration_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(urpg_integration_tests PRIVATE URPG_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
urpg_enable_windows_test_binary_unlock(urpg_integration_tests)

add_executable(urpg_snapshot_tests
    tests/snapshot/test_snapshot_canonical_outputs.cpp
)

target_link_libraries(urpg_snapshot_tests PRIVATE urpg_core Catch2::Catch2WithMain)
target_include_directories(urpg_snapshot_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(urpg_snapshot_tests PRIVATE URPG_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
urpg_enable_windows_test_binary_unlock(urpg_snapshot_tests)

add_executable(urpg_compat_tests
    tests/compat/test_compat_authority_suite.cpp
    tests/compat/test_compat_window_plugin_profiles.cpp
    tests/compat/test_compat_plugin_fixtures.cpp
    tests/compat/test_compat_plugin_failure_diagnostics.cpp
)

target_link_libraries(urpg_compat_tests PRIVATE urpg_core Catch2::Catch2WithMain)
target_include_directories(urpg_compat_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(urpg_compat_tests PRIVATE URPG_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
urpg_enable_windows_test_binary_unlock(urpg_compat_tests)

include(CTest)
include(Catch)
catch_discover_tests(
    urpg_tests
    DISCOVERY_MODE PRE_TEST
    PROPERTIES LABELS "pr"
)

catch_discover_tests(
    urpg_integration_tests
    DISCOVERY_MODE PRE_TEST
    PROPERTIES LABELS "nightly"
)

catch_discover_tests(
    urpg_snapshot_tests
    DISCOVERY_MODE PRE_TEST
    PROPERTIES LABELS "nightly"
)

catch_discover_tests(
    urpg_compat_tests
    DISCOVERY_MODE PRE_TEST
    PROPERTIES LABELS "weekly"
)
