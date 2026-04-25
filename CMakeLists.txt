cmake_minimum_required(VERSION 3.23)
project(urpg LANGUAGES CXX)

include(CheckCXXSourceCompiles)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/FS /EHsc)
endif()

option(URPG_WARNINGS_AS_ERRORS "Treat project warnings as errors through urpg_project_warnings" OFF)

add_library(urpg_project_warnings INTERFACE)
target_compile_options(urpg_project_warnings INTERFACE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<AND:$<BOOL:${URPG_WARNINGS_AS_ERRORS}>,$<CXX_COMPILER_ID:MSVC>>:/WX>
    $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-Wall -Wextra>
    $<$<AND:$<BOOL:${URPG_WARNINGS_AS_ERRORS}>,$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>>:-Werror>
)

function(urpg_normalize_path out_var input_path)
    if(NOT input_path)
        set(${out_var} "" PARENT_SCOPE)
        return()
    endif()

    file(TO_CMAKE_PATH "${input_path}" _normalized_path)
    string(REGEX REPLACE "/+$" "" _normalized_path "${_normalized_path}")
    string(TOLOWER "${_normalized_path}" _normalized_path)
    set(${out_var} "${_normalized_path}" PARENT_SCOPE)
endfunction()

function(urpg_get_compiler_root out_var)
    if(NOT CMAKE_CXX_COMPILER)
        set(${out_var} "" PARENT_SCOPE)
        return()
    endif()

    get_filename_component(_compiler_bin_dir "${CMAKE_CXX_COMPILER}" DIRECTORY)
    get_filename_component(_compiler_root "${_compiler_bin_dir}" DIRECTORY)
    urpg_normalize_path(_compiler_root "${_compiler_root}")
    set(${out_var} "${_compiler_root}" PARENT_SCOPE)
endfunction()

option(URPG_FETCH_CATCH2 "Fetch Catch2 via CMake" ON)
option(
    URPG_MINGW_VENDORED_SDL_SHARED
    "Build vendored SDL shared libraries on MinGW fallback toolchains"
    ON
)
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
    find_program(URPG_WINDOWS_POWERSHELL_EXE NAMES powershell)
    if(URPG_WINDOWS_POWERSHELL_EXE)
        set(URPG_POWERSHELL_EXE "${URPG_WINDOWS_POWERSHELL_EXE}")
    else()
        find_program(URPG_POWERSHELL_EXE NAMES pwsh powershell)
    endif()
    if(NOT URPG_POWERSHELL_EXE)
        message(WARNING "URPG_WINDOWS_UNLOCK_TEST_BINARIES=ON but no PowerShell executable was found")
    endif()
endif()

if(MINGW)
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    set(URPG_WINDRES_WRAPPER_CMD
        "${CMAKE_BINARY_DIR}/urpg-windres-wrapper.cmd"
    )
    get_filename_component(URPG_MINGW_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
    set(URPG_REAL_WINDRES "${URPG_MINGW_BIN_DIR}/windres.exe")
    if(NOT EXISTS "${URPG_REAL_WINDRES}")
        unset(URPG_REAL_WINDRES)
        find_program(
            URPG_REAL_WINDRES
            NAMES windres
            HINTS "${URPG_MINGW_BIN_DIR}"
            NO_CACHE
        )
    endif()

    if(URPG_REAL_WINDRES)
        set(URPG_REAL_WINDRES_COMPILER "${URPG_REAL_WINDRES}" CACHE INTERNAL
            "Underlying MinGW windres compiler path"
        )
        string(CONCAT URPG_WINDRES_WRAPPER_CONTENT
            "@echo off\r\n"
            "\"${Python3_EXECUTABLE}\" "
            "\"${CMAKE_CURRENT_SOURCE_DIR}/tools/ci/windres_wrapper.py\" "
            "\"${URPG_REAL_WINDRES_COMPILER}\" %*\r\n"
        )
        file(GENERATE OUTPUT "${URPG_WINDRES_WRAPPER_CMD}" CONTENT "${URPG_WINDRES_WRAPPER_CONTENT}")
        set(CMAKE_RC_COMPILER "${URPG_WINDRES_WRAPPER_CMD}" CACHE FILEPATH
            "MinGW RC compiler wrapper for path-safe windres invocations"
            FORCE
        )
        unset(CMAKE_RC_COMPILER_ARG1 CACHE)
        unset(CMAKE_RC_COMPILER_LAUNCHER CACHE)
        message(STATUS
            "URPG: wrapping windres so MinGW resource builds remain path-safe in workspaces with spaces.")
    endif()
endif()

function(urpg_enable_windows_test_binary_unlock target_name)
    if(NOT WIN32 OR NOT URPG_WINDOWS_UNLOCK_TEST_BINARIES OR NOT URPG_POWERSHELL_EXE)
        return()
    endif()

    if(MINGW AND CMAKE_BINARY_DIR MATCHES " ")
        message(STATUS
            "URPG: skipping Windows test-binary unlock hooks for ${target_name} because "
            "CMake/Ninja emits a broken PRE_LINK cmd wrapper under MinGW when the build path contains spaces.")
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
    engine/core/action/controller_binding_runtime.cpp
    engine/core/debug/debug_session.cpp
    engine/core/debug/debug_ai_bridge.cpp
    engine/core/engine_shell.cpp
    engine/core/global_state_hub.cpp
    engine/core/input/input_remap_store.cpp
    engine/core/platform/opengl_renderer.cpp
    engine/core/platform/sdl_surface.cpp
    engine/core/platform/gl_shader.cpp
    engine/core/platform/gl_texture.cpp
    engine/core/render/asset_loader.cpp
    engine/core/render/tilemap_renderer.cpp
    engine/core/render/sprite_animator.cpp
    engine/core/sprite_batcher.cpp
    engine/core/clock.cpp
    engine/core/perf/perf_profiler.cpp
    engine/core/testing/visual_regression_harness.cpp
    engine/core/analytics/analytics_dispatcher.cpp
    engine/core/analytics/analytics_dispatcher_validator.cpp
    engine/core/analytics/analytics_uploader.cpp
    engine/core/analytics/analytics_privacy_controller.cpp
    engine/core/semver.cpp
    engine/core/asset_compressor.cpp
    engine/core/threading/thread_roles.cpp
    engine/core/events/event_edit_guard.cpp
    engine/core/events/event_debugger.cpp
    engine/core/events/event_dependency_graph.cpp
    engine/core/events/event_document.cpp
    engine/core/events/event_macro_recorder.cpp
    engine/core/events/event_runtime.cpp
    engine/core/format/canonical_json.cpp
    engine/core/migrate/migration_runner.cpp
    engine/core/plugin/plugin_compatibility_score.cpp
    engine/core/quest/quest_registry.cpp
    engine/core/quest/quest_validator.cpp
    engine/core/dialogue/dialogue_graph.cpp
    engine/core/narrative/choice_consequence_tracker.cpp
    engine/core/narrative/ending_route_manager.cpp
    engine/core/narrative/narrative_continuity_checker.cpp
    engine/core/relationship/relationship_registry.cpp
    engine/core/timeline/timeline_document.cpp
    engine/core/timeline/timeline_player.cpp
    engine/core/replay/replay_recorder.cpp
    engine/core/replay/replay_player.cpp
    engine/core/replay/replay_gallery.cpp
    engine/core/replay/golden_replay_lane.cpp
    engine/core/database/rpg_database.cpp
    engine/core/balance/economy_simulator.cpp
    engine/core/balance/encounter_table.cpp
    engine/core/shop/vendor_catalog.cpp
    engine/core/rest/rest_point.cpp
    engine/core/items/loot_affix_generator.cpp
    engine/core/progression/class_progression.cpp
    engine/core/ability/skill_combo_rules.cpp
    engine/core/sync/source_authority.cpp
    engine/core/save/save_catalog.cpp
    engine/core/save/save_compatibility_preview.cpp
    engine/core/save/save_corruption_lab.cpp
    engine/core/save/save_debugger.cpp
    engine/core/save/save_journal.cpp
    engine/core/save/save_migration.cpp
    engine/core/save/save_recovery.cpp
    engine/core/save/save_runtime.cpp
    engine/core/save/lzstring_decompressor.cpp
    engine/core/save/rpgmaker_save_reader.cpp
    engine/core/battle/battle_core.cpp
    engine/core/battle/battle_ai_bridge.cpp
    engine/core/battle/battle_formula_probe.cpp
    engine/core/battle/battle_presentation_profile.cpp
    engine/core/battle/boss_profile.cpp
    engine/core/battle/enemy_ai_profile.cpp
    engine/core/battle/party_tactics_profile.cpp
    engine/core/map/tile_layer_document.cpp
    engine/core/map/terrain_brush.cpp
    engine/core/map/map_region_rules.cpp
    engine/core/map/procedural_map_generator.cpp
    engine/core/map/spawn_table.cpp
    engine/core/map/tactical_grid_preview.cpp
    engine/core/character/character_identity.cpp
    engine/core/character/character_identity_catalog.cpp
    engine/core/character/character_identity_validator.cpp
    engine/core/character/character_creation_screen.cpp
    engine/core/character/character_identity_system.cpp
    engine/core/achievement/achievement_registry.cpp
    engine/core/achievement/achievement_trigger.cpp
    engine/core/achievement/achievement_validator.cpp
    engine/core/presentation/presentation_bridge.cpp
    engine/core/presentation/presentation_runtime.cpp
    engine/core/presentation/effects/effect_catalog.cpp
    engine/core/presentation/effects/effect_resolver.cpp
    engine/core/presentation/effects/effect_translator.cpp
    engine/gameplay/combat/combat_calc.cpp
    engine/core/audio/audio_ai_bridge.cpp
    engine/core/assets/asset_library.cpp
    engine/core/assets/asset_cleanup_planner.cpp
    engine/core/audio/audio_mix_presets.cpp
    engine/core/audio/audio_mix_validator.cpp
    engine/core/animation/animation_ai_bridge.cpp
    engine/core/ui/ui_window.cpp
    engine/core/ui/chat_window.cpp
    engine/core/ui/ui_command_list.cpp
    engine/core/message/message_core.cpp
    engine/core/message/ai_sync_coordinator.cpp
    engine/core/message/dialogue_registry.cpp
    engine/core/message/message_migration.cpp
    engine/core/localization/locale_catalog.cpp
    engine/core/localization/completeness_checker.cpp
    engine/core/accessibility/accessibility_auditor.cpp
    editor/diagnostics/event_authority_diagnostics.cpp
    editor/diagnostics/event_authority_panel.cpp
    editor/diagnostics/diagnostics_workspace.cpp
    editor/diagnostics/diagnostics_workspace_export.cpp
    editor/diagnostics/diagnostics_workspace_serialization.cpp
    editor/diagnostics/diagnostics_facade.cpp
    editor/diagnostics/project_audit_panel.cpp
    editor/diagnostics/project_health_model.cpp
    editor/diagnostics/project_health_panel.cpp
    editor/events/event_authoring_model.cpp
    editor/events/event_authoring_panel.cpp
    editor/plugin/plugin_inspector_model.cpp
    editor/plugin/plugin_inspector_panel.cpp
    editor/project/new_project_wizard_model.cpp
    editor/project/new_project_wizard_panel.cpp
    editor/quest/quest_panel.cpp
    editor/dialogue/dialogue_graph_panel.cpp
    editor/narrative/narrative_continuity_panel.cpp
    editor/relationship/relationship_panel.cpp
    editor/timeline/timeline_panel.cpp
    editor/replay/replay_panel.cpp
    editor/database/database_panel.cpp
    editor/balance/balance_panel.cpp
    editor/shop/vendor_panel.cpp
    editor/assets/asset_library_model.cpp
    editor/assets/asset_library_panel.cpp
    editor/export/export_diagnostics_panel.cpp
    editor/compat/compat_report_panel.cpp
    editor/character/character_creator_model.cpp
    editor/character/character_creator_panel.cpp
    editor/achievement/achievement_panel.cpp
    editor/action/controller_binding_panel.cpp
    editor/save/save_debugger_panel.cpp
    editor/save/save_inspector_model.cpp
    editor/save/save_inspector_panel.cpp
    editor/save/save_migration_preview_panel.cpp
    editor/battle/battle_inspector_model.cpp
    editor/battle/battle_presentation_panel.cpp
    editor/battle/boss_designer_panel.cpp
    editor/battle/formula_debugger_panel.cpp
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
    editor/perf/perf_diagnostics_model.cpp
    editor/perf/perf_diagnostics_panel.cpp
    editor/spatial/elevation_brush_panel.cpp
    editor/spatial/terrain_brush_panel.cpp
    editor/spatial/region_rules_panel.cpp
    editor/spatial/procedural_map_panel.cpp
    editor/spatial/prop_placement_panel.cpp
    editor/spatial/map_ability_binding_panel.cpp
    editor/spatial/spatial_ability_canvas_panel.cpp
    editor/spatial/spatial_authoring_workspace.cpp
    editor/sprite/sprite_animation_preview_panel.cpp
    editor/accessibility/accessibility_panel.cpp
    editor/accessibility/accessibility_menu_adapter.cpp
    editor/accessibility/accessibility_spatial_adapter.cpp
    editor/accessibility/accessibility_audio_adapter.cpp
    editor/accessibility/accessibility_battle_adapter.cpp
    editor/audio/audio_mix_panel.cpp
    editor/audio/audio_inspector_model.cpp
    editor/analytics/analytics_panel.cpp
    editor/mod/mod_manager_panel.cpp
    engine/core/editor/plugin_api.cpp
    engine/core/editor/security_manager.cpp
    engine/core/ability/gameplay_ability.cpp
    engine/core/ability/ability_state_machine.cpp
    engine/core/ability/pattern_field.cpp
    engine/core/ability/pattern_field_presets.cpp
    engine/core/mod/mod_registry.cpp
    engine/core/mod/mod_registry_validator.cpp
    engine/core/mod/mod_loader.cpp
    engine/core/export/creator_package_manifest.cpp
    engine/core/export/export_artifact_compare.cpp
    engine/core/export/export_validator.cpp
    engine/core/export/patch_manifest.cpp
    engine/core/export/runtime_bundle_loader.cpp
    engine/core/tools/export_packager.cpp
    engine/core/tools/export_packager_bundle_writer.cpp
    engine/core/tools/export_packager_executable_staging.cpp
    engine/core/tools/export_packager_license_audit.cpp
    engine/core/tools/export_packager_payload_builder.cpp
    engine/core/project/dev_room_generator.cpp
    engine/core/project/project_snapshot_store.cpp
    engine/core/project/project_template_generator.cpp
    engine/core/tutorial/tutorial_lesson.cpp
    engine/core/level/level_assembly.cpp
    engine/core/ui/menu_serializer.cpp
    engine/core/scene/battle_scene.cpp
    engine/core/scene/map_scene.cpp
    engine/core/scene/menu_scene.cpp
    runtimes/compat_js/quickjs_runtime.cpp
    runtimes/compat_js/window_color_parser.cpp
    runtimes/compat_js/window_compat.cpp
    runtimes/compat_js/window_render_helpers.cpp
    runtimes/compat_js/window_sprite_compat.cpp
    runtimes/compat_js/window_text_layout.cpp
    runtimes/compat_js/global_state_bridge.cpp
  runtimes/compat_js/battle_manager_api.cpp
  runtimes/compat_js/battle_manager_actions.cpp
  runtimes/compat_js/battle_manager_animations.cpp
  runtimes/compat_js/battle_manager.cpp
  runtimes/compat_js/battle_manager_conditions.cpp
  runtimes/compat_js/battle_manager_escape_support.cpp
  runtimes/compat_js/battle_manager_effects.cpp
    runtimes/compat_js/battle_manager_events.cpp
    runtimes/compat_js/battle_manager_lifecycle.cpp
    runtimes/compat_js/battle_manager_rewards.cpp
    runtimes/compat_js/battle_manager_support.cpp
    runtimes/compat_js/data_manager_accessors.cpp
    runtimes/compat_js/data_manager_api.cpp
    runtimes/compat_js/data_manager_battle_loading.cpp
    runtimes/compat_js/data_manager.cpp
    runtimes/compat_js/data_manager_database.cpp
    runtimes/compat_js/data_manager_enemy_troop_loading.cpp
    runtimes/compat_js/data_manager_map_loading.cpp
    runtimes/compat_js/data_manager_save.cpp
    runtimes/compat_js/data_manager_state.cpp
    runtimes/compat_js/data_manager_support.cpp
    runtimes/compat_js/data_manager_value_projection.cpp
    runtimes/compat_js/data_manager_visual_loading.cpp
    runtimes/compat_js/audio_manager.cpp
    runtimes/compat_js/input_manager.cpp
    runtimes/compat_js/plugin_manager.cpp
    runtimes/compat_js/plugin_manager_directory_scan.cpp
    runtimes/compat_js/plugin_manager_fixture_script.cpp
    runtimes/compat_js/plugin_manager_status.cpp
    runtimes/compat_js/plugin_manager_support.cpp
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
set(URPG_DISABLE_VENDORED_SDL_WINDOWS_JOYSTICK_STACK OFF)

if(MINGW)
    check_cxx_source_compiles(
        "
        #include <windows.h>
        #include <xinput.h>
        int main(void) {
            XINPUT_CAPABILITIES_EX capabilities = {0};
            return (int)capabilities.ProductId;
        }
        "
        URPG_MINGW_XINPUT_HEADER_HAS_CAPABILITIES_EX
    )

    if(URPG_MINGW_XINPUT_HEADER_HAS_CAPABILITIES_EX)
        set(SDL_XINPUT OFF CACHE BOOL
            "Disable vendored SDL XInput backend on MinGW toolchains whose headers already define XINPUT_CAPABILITIES_EX"
            FORCE
        )
        set(URPG_DISABLE_VENDORED_SDL_WINDOWS_JOYSTICK_STACK ON)
        message(STATUS
            "URPG: disabling vendored SDL XInput backend for MinGW because the active xinput.h "
            "already defines XINPUT_CAPABILITIES_EX and SDL 2.30.0's fallback typedef would conflict.")
    endif()
endif()

if(MINGW)
    urpg_get_compiler_root(URPG_MINGW_COMPILER_ROOT)
    if(URPG_MINGW_COMPILER_ROOT)
        message(STATUS "URPG: probing SDL2 under MinGW compiler root: ${URPG_MINGW_COMPILER_ROOT}")
        find_package(
            SDL2 QUIET CONFIG
            PATHS
                "${URPG_MINGW_COMPILER_ROOT}"
                "${URPG_MINGW_COMPILER_ROOT}/lib/cmake"
                "${URPG_MINGW_COMPILER_ROOT}/lib"
            NO_DEFAULT_PATH
        )
        if(NOT SDL2_FOUND)
            message(STATUS
                "URPG: SDL2 was not found under compiler root ${URPG_MINGW_COMPILER_ROOT}; "
                "falling back to FetchContent to avoid mixed-toolchain headers/libraries.")
        endif()
    endif()
elseif(MSVC)
    message(STATUS
        "URPG: skipping host SDL2 package discovery for MSVC to avoid mixing MinGW/MSYS headers into "
        "Visual Studio builds; using vendored SDL instead.")
elseif(NOT SDL2_FOUND)
    find_package(SDL2 QUIET)
endif()

if(NOT SDL2_FOUND)
    if(MINGW AND URPG_DISABLE_VENDORED_SDL_WINDOWS_JOYSTICK_STACK)
        set(SDL_JOYSTICK OFF CACHE BOOL
            "Disable vendored SDL joystick stack on MinGW when XInput compatibility shims are unavailable"
            FORCE
        )
        set(SDL_HAPTIC OFF CACHE BOOL
            "Disable vendored SDL haptic stack on MinGW when joystick backends are unavailable"
            FORCE
        )
        message(STATUS
            "URPG: disabling vendored SDL joystick/haptic support for the MinGW fallback build because "
            "SDL 2.30.0's Windows joystick sources still assume XInput helpers that are not available "
            "with this header set. Install SDL2 into the active MinGW root to restore controller support.")
    endif()
    if(MINGW)
        set(SDL_SHARED ${URPG_MINGW_VENDORED_SDL_SHARED} CACHE BOOL
            "Build a shared version of the library"
            FORCE
        )
        set(SDL_STATIC ON CACHE BOOL
            "Build a static version of the library"
            FORCE
        )
    endif()
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
    GIT_TAG f58f558c120e9b32c217290b80bad1a0729fbb2c
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
elseif(TARGET SDL2::SDL2-static)
    set(URPG_SDL_TARGET SDL2::SDL2-static)
elseif(TARGET SDL2-static)
    set(URPG_SDL_TARGET SDL2-static)
endif()

foreach(_urpg_sdl_vendor_target IN ITEMS SDL2 SDL2-static)
    if(TARGET ${_urpg_sdl_vendor_target})
        get_target_property(_urpg_sdl_vendor_imported ${_urpg_sdl_vendor_target} IMPORTED)
        if(NOT _urpg_sdl_vendor_imported)
            target_compile_options(${_urpg_sdl_vendor_target} PRIVATE
                $<$<OR:$<C_COMPILER_ID:Clang>,$<C_COMPILER_ID:AppleClang>>:-Wno-shadow>
            )
        endif()
    endif()
endforeach()

target_link_libraries(urpg_core PUBLIC
    urpg_project_warnings
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

add_executable(urpg_project_audit
    tools/audit/project_audit_artifact_governance.cpp
    tools/audit/project_audit_asset_report.cpp
    tools/audit/project_audit_issue_collectors.cpp
    tools/audit/project_audit_support.cpp
    tools/audit/project_audit_template_governance.cpp
    tools/audit/project_audit_template_spec.cpp
    tools/audit/urpg_project_audit.cpp
)

target_link_libraries(urpg_project_audit PRIVATE urpg_core)
target_include_directories(urpg_project_audit PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(urpg_pack_cli
    tools/pack/pack_cli.cpp
)

target_link_libraries(urpg_pack_cli PRIVATE urpg_core)
target_include_directories(urpg_pack_cli PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
urpg_enable_windows_test_binary_unlock(urpg_pack_cli)

add_executable(urpg_presentation_release_validation
    engine/core/presentation/release_validation.cpp
)

target_link_libraries(urpg_presentation_release_validation PRIVATE urpg_core)
target_include_directories(urpg_presentation_release_validation PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
urpg_enable_windows_test_binary_unlock(urpg_presentation_release_validation)

add_executable(urpg_profile_arena
    engine/core/presentation/profile_arena.cpp
)

target_link_libraries(urpg_profile_arena PRIVATE urpg_core)
target_include_directories(urpg_profile_arena PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(urpg_export_smoke_app
    tools/export/export_smoke_app.cpp
)

target_link_libraries(urpg_export_smoke_app PRIVATE urpg_core)
target_include_directories(urpg_export_smoke_app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(WIN32)
    add_custom_command(
        TARGET urpg_export_smoke_app
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:urpg_export_smoke_app>
            $<TARGET_FILE_DIR:urpg_export_smoke_app>
        COMMAND_EXPAND_LISTS
    )
endif()

add_executable(urpg_tests
    tests/unit/test_engine_shell.cpp
    tests/unit/test_global_state.cpp
    tests/unit/test_semver.cpp
    tests/unit/test_fixed32.cpp
    tests/unit/test_frame_budget.cpp
    tests/unit/test_perf_profiler.cpp
    tests/unit/test_perf_diagnostics_panel.cpp
    tests/unit/test_ecs_world.cpp
    tests/unit/test_ai_system.cpp
    tests/unit/test_movement_system.cpp
    tests/unit/test_player_control_system.cpp
    tests/unit/test_collision_system.cpp
    tests/unit/test_lighting_system.cpp
    tests/unit/test_camera_system.cpp
    tests/unit/test_camera_follow_system.cpp
    tests/unit/test_camera_shake_system.cpp
    tests/unit/test_character_identity.cpp
    tests/unit/test_character_identity_ecs.cpp
    tests/unit/test_character_creator_model.cpp
    tests/unit/test_character_creator_panel.cpp
    tests/unit/test_character_creation_screen.cpp
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
    tests/unit/test_choice_consequence_tracker.cpp
    tests/unit/test_dialogue_graph.cpp
    tests/unit/test_ending_route_manager.cpp
    tests/unit/test_narrative_continuity_checker.cpp
    tests/unit/test_quest_registry.cpp
    tests/unit/test_relationship_registry.cpp
    tests/unit/test_render_tier.cpp
    tests/unit/test_project_snapshot_store.cpp
    tests/unit/test_save_compatibility_preview.cpp
    tests/unit/test_save_corruption_lab.cpp
    tests/unit/test_save_debugger.cpp
    tests/unit/test_save_journal.cpp
    tests/unit/test_canonical_json.cpp
    tests/unit/test_migration_runner.cpp
    tests/unit/test_source_authority.cpp
    tests/unit/test_diagnostics_facade.cpp
    tests/unit/test_project_audit_panel.cpp
    tests/unit/test_project_health_model.cpp
    tests/unit/test_project_health_panel.cpp
    tests/unit/test_project_template_generator.cpp
    tests/unit/test_plugin_compatibility_score.cpp
    tests/unit/test_plugin_inspector_panel.cpp
    tests/unit/test_asset_library.cpp
    tests/unit/test_asset_cleanup_planner.cpp
    tests/unit/test_asset_library_panel.cpp
    tests/unit/test_controller_binding_runtime.cpp
    tests/unit/test_controller_binding_panel.cpp
    tests/unit/test_save_catalog.cpp
    tests/unit/test_save_inspector_model.cpp
    tests/unit/test_save_inspector_panel.cpp
    tests/unit/test_lzstring_decompressor.cpp
    tests/unit/test_rpgmaker_save_reader.cpp
    tests/unit/test_save_recovery.cpp
    tests/unit/test_save_runtime.cpp
    tests/unit/test_save_migration.cpp
    tests/unit/test_save_serialization.cpp
    tests/unit/test_battle_inspector_model.cpp
    tests/unit/test_battle_authoring_suite.cpp
    tests/unit/test_battle_preview_panel.cpp
    tests/unit/test_battle_inspector_panel.cpp
    tests/unit/test_event_edit_guard.cpp
    tests/unit/test_event_document.cpp
    tests/unit/test_event_dependency_graph.cpp
    tests/unit/test_event_debugger.cpp
    tests/unit/test_event_authoring_panel.cpp
    tests/unit/test_event_authority_diagnostics.cpp
    tests/unit/test_event_authority_panel.cpp
    tests/unit/test_diagnostics_workspace.cpp
    tests/unit/test_event_runtime.cpp
    tests/unit/test_debug_session.cpp
    tests/unit/test_quickjs_runtime.cpp
    tests/unit/test_window_compat.cpp
    tests/unit/test_compat_report_panel.cpp
    tests/unit/test_battlemgr.cpp
    tests/unit/test_mod_registry.cpp
    tests/unit/test_mod_manager_panel.cpp
    tests/unit/test_battle_effect_cues.cpp
    tests/unit/test_battle_scene_native.cpp
    tests/unit/test_data_manager.cpp
    tests/unit/test_audio_manager.cpp
    tests/unit/test_input_manager.cpp
    tests/unit/test_plugin_manager.cpp
    tests/unit/test_plugin_api.cpp
    tests/unit/test_menu_core.cpp
    tests/unit/test_menu_orchestration.cpp
    tests/unit/test_input_core.cpp
    tests/unit/test_input_remap_store.cpp
    tests/unit/test_audio_core.cpp
    tests/unit/test_audio_mix_presets.cpp
    tests/unit/test_message_text_core.cpp
    tests/unit/test_message_migration.cpp
    tests/unit/test_message_schema_contracts.cpp
    tests/unit/test_locale_catalog.cpp
    tests/unit/test_completeness_checker.cpp
    tests/unit/test_save_schema_contracts.cpp
    tests/unit/test_save_policy_governance.cpp
    tests/unit/test_message_inspector_model.cpp
    tests/unit/test_message_inspector_panel.cpp
    tests/unit/test_menu_inspector_panel.cpp
    tests/unit/test_menu_inspector_model.cpp
    tests/unit/test_menu_preview_panel.cpp
    tests/unit/test_menu_legacy_import.cpp
    tests/unit/test_state_audio_sync.cpp
    tests/unit/test_scene_manager.cpp
    tests/unit/test_gameplay_tags.cpp
    tests/unit/test_presentation_bridge.cpp
    tests/unit/test_presentation_runtime.cpp
    tests/unit/test_presentation_effect_translation.cpp
    tests/unit/test_project_audit_cli.cpp
    tests/unit/test_effect_cue.cpp
    tests/unit/test_spatial_editor.cpp
    tests/unit/test_map_worldbuilding_suite.cpp
    tests/unit/test_huggingface_curated_assets.cpp
    tests/unit/test_ability_activation.cpp
    tests/unit/test_ability_e2e.cpp
    tests/unit/test_ability_battle_integration.cpp
    tests/unit/test_pattern_field.cpp
    tests/unit/test_effect_resolver.cpp
    tests/unit/test_wave3_gaf.cpp
    tests/unit/test_level_assembly.cpp
    tests/unit/test_sprite_animator.cpp
    tests/unit/test_sprite_animation_preview_panel.cpp
    tests/unit/test_procedural_toolkit.cpp
    tests/unit/test_optional_lanes.cpp
    tests/unit/test_debug_crash.cpp
    tests/unit/test_audio_inspector.cpp
    tests/unit/test_audio_resolver.cpp
    tests/unit/test_save_state_sync.cpp
    tests/unit/test_battle_migration.cpp
    tests/unit/test_menu_migration_logic.cpp
    tests/unit/test_migration_wizard.cpp
    tests/unit/test_ai_bridge_regex.cpp
    tests/unit/test_ai_sync_coordinator.cpp
    tests/unit/test_diagnostics_workspace_migration.cpp
    tests/unit/test_diagnostics_workspace_audio_event.cpp
    tests/unit/test_diagnostics_workspace_message_menu.cpp
    tests/unit/test_diagnostics_workspace_battle.cpp
    tests/unit/test_achievement_registry.cpp
    tests/unit/test_achievement_panel.cpp
    tests/unit/test_achievement_triggers.cpp
    tests/unit/test_accessibility_auditor.cpp
    tests/unit/test_accessibility_panel.cpp
    tests/unit/test_creator_package_manifest.cpp
    tests/unit/test_export_artifact_compare.cpp
    tests/unit/test_export_validator.cpp
    tests/unit/test_export_packager.cpp
    tests/unit/test_export_diagnostics_panel.cpp
    tests/unit/test_patch_manifest.cpp
    tests/unit/test_runtime_bundle_loader.cpp
    tests/unit/test_visual_regression_harness.cpp
    tests/unit/test_truth_reconciler.cpp
    tests/unit/test_governance_regression.cpp
    tests/unit/test_analytics_dispatcher.cpp
    tests/unit/test_analytics_panel.cpp
    tests/unit/test_template_acceptance.cpp
    tests/unit/test_template_bar_quality.cpp
    tests/unit/test_dev_room_generator.cpp
    tests/unit/test_tutorial_lesson.cpp
    tests/unit/test_timeline_player.cpp
    tests/unit/test_event_macro_recorder.cpp
    tests/unit/test_replay_recorder.cpp
    tests/unit/test_timeline_replay_panels.cpp
    tests/unit/test_rpg_database_suite.cpp
    tests/unit/test_economy_simulator.cpp
    tests/unit/test_encounter_table.cpp
    tests/unit/test_vendor_catalog.cpp
    tests/unit/test_rest_point.cpp
    tests/unit/test_loot_affix_generator.cpp
    tests/unit/test_class_progression.cpp
    tests/unit/test_skill_combo_rules.cpp
    tests/unit/test_database_balance_panels.cpp
    tests/unit/test_s31_template_evidence_snapshots.cpp
    tests/unit/test_s32_wysiwyg_lanes.cpp
    tests/engine/core/test_render_assets.cpp
    tests/test_asset_license_audit.cpp
    tests/test_copilot_kernel.cpp
    tests/test_headless_play_mode.cpp
    tests/test_plugin_manifest.cpp
    tests/test_snapshot_validator.cpp
    tests/test_ability_inspector.cpp
    tests/test_ability_pattern_integration.cpp
    tests/test_ability_state_machine.cpp
    tests/test_ability_tasks.cpp
    tests/test_effect_modifiers.cpp
    tests/test_pattern_field_editor.cpp
    tests/test_pattern_serialization.cpp
)

target_link_libraries(urpg_tests PRIVATE urpg_core Catch2::Catch2WithMain)
target_include_directories(urpg_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(urpg_tests PRIVATE URPG_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
add_dependencies(urpg_tests urpg_project_audit)
add_dependencies(urpg_tests urpg_export_smoke_app)
add_dependencies(urpg_tests urpg_pack_cli)
target_compile_definitions(urpg_tests PRIVATE URPG_EXPORT_SMOKE_APP_PATH="$<TARGET_FILE:urpg_export_smoke_app>")
target_compile_definitions(urpg_tests PRIVATE URPG_PACK_CLI_PATH="$<TARGET_FILE:urpg_pack_cli>")
urpg_enable_windows_test_binary_unlock(urpg_tests)

add_executable(urpg_integration_tests
    tests/integration/test_integration_runtime_recovery.cpp
    tests/integration/test_battle_save_integration.cpp
    tests/integration/test_event_authoring_integration.cpp
    tests/integration/test_replay_integration.cpp
    tests/integration/test_wave1_closure_integration.cpp
)

target_link_libraries(urpg_integration_tests PRIVATE urpg_core Catch2::Catch2WithMain)
target_include_directories(urpg_integration_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(urpg_integration_tests PRIVATE URPG_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
urpg_enable_windows_test_binary_unlock(urpg_integration_tests)

add_executable(urpg_snapshot_tests
    tests/snapshot/test_snapshot_canonical_outputs.cpp
    tests/snapshot/test_renderer_backed_visual_capture.cpp
)

target_link_libraries(urpg_snapshot_tests PRIVATE urpg_core Catch2::Catch2WithMain)
target_include_directories(urpg_snapshot_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(urpg_snapshot_tests PRIVATE URPG_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
urpg_enable_windows_test_binary_unlock(urpg_snapshot_tests)

add_executable(urpg_compat_tests
    tests/compat/test_compat_authority_suite.cpp
    tests/compat/test_compat_window_plugin_profiles.cpp
    tests/compat/test_compat_plugin_fixtures.cpp
    tests/compat/test_compat_plugin_fixtures_content.cpp
    tests/compat/test_compat_plugin_fixtures_menu_presentation.cpp
    tests/compat/test_compat_plugin_fixtures_battle.cpp
    tests/compat/test_compat_plugin_failure_diagnostics.cpp
    tests/compat/test_compat_plugin_failure_mixed_chain.cpp
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

add_test(
    NAME urpg_snapshot_regression_lane
    COMMAND urpg_snapshot_tests "[snapshot][regression]"
)
set_tests_properties(
    urpg_snapshot_regression_lane
    PROPERTIES LABELS "nightly;regression"
)

catch_discover_tests(
    urpg_compat_tests
    DISCOVERY_MODE PRE_TEST
    PROPERTIES LABELS "weekly"
)

find_package(Python3 QUIET COMPONENTS Interpreter)
if(Python3_FOUND)
    add_test(
        NAME urpg_retrieval_adapter_resolution_test
        COMMAND ${Python3_EXECUTABLE} tools/retrieval/tests/test_build_retrieval_bundle.py
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
    set_tests_properties(
        urpg_retrieval_adapter_resolution_test
        PROPERTIES LABELS "pr"
    )
else()
    message(STATUS "Skipping urpg_retrieval_adapter_resolution_test because Python3 interpreter was not found.")
endif()

if(URPG_POWERSHELL_EXE)
    add_test(
        NAME urpg_golden_replay_lane
        COMMAND
            ${URPG_POWERSHELL_EXE}
            -NoProfile
            -ExecutionPolicy
            Bypass
            -File
            "${CMAKE_SOURCE_DIR}/tools/ci/check_golden_replays.ps1"
            -RepoRoot
            "${CMAKE_SOURCE_DIR}"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
    set_tests_properties(
        urpg_golden_replay_lane
        PROPERTIES LABELS "pr;replay"
    )
else()
    message(STATUS "Skipping urpg_golden_replay_lane because PowerShell was not found.")
endif()

add_test(
    NAME urpg_presentation_release_validation
    COMMAND urpg_presentation_release_validation
)
set_tests_properties(
    urpg_presentation_release_validation
    PROPERTIES LABELS "nightly;presentation"
)

add_test(
    NAME urpg_presentation_unit_lane
    COMMAND urpg_tests "[presentation]"
)
set_tests_properties(
    urpg_presentation_unit_lane
    PROPERTIES LABELS "pr;presentation"
)

add_test(
    NAME urpg_spatial_editor_lane
    COMMAND urpg_tests "[editor][spatial]"
)
set_tests_properties(
    urpg_spatial_editor_lane
    PROPERTIES LABELS "pr;presentation;editor"
)
