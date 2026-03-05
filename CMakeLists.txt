cmake_minimum_required(VERSION 3.23)
project(urpg LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(URPG_FETCH_CATCH2 "Fetch Catch2 via CMake" ON)

add_library(urpg_core
    engine/core/debug/debug_session.cpp
    engine/core/semver.cpp
    engine/core/threading/thread_roles.cpp
    engine/core/events/event_edit_guard.cpp
    engine/core/events/event_runtime.cpp
    engine/core/format/canonical_json.cpp
    engine/core/migrate/migration_runner.cpp
    engine/core/sync/source_authority.cpp
    engine/core/save/save_journal.cpp
    engine/core/save/save_recovery.cpp
    engine/core/save/save_runtime.cpp
    engine/gameplay/combat/combat_calc.cpp
    editor/diagnostics/event_authority_diagnostics.cpp
    editor/diagnostics/event_authority_panel.cpp
    editor/compat/compat_report_panel.cpp
    runtimes/compat_js/quickjs_runtime.cpp
    runtimes/compat_js/window_compat.cpp
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

add_executable(urpg_migrate
    tools/migrate/migrate_cli.cpp
)

target_link_libraries(urpg_migrate PRIVATE urpg_core)
target_include_directories(urpg_migrate PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(urpg_tests
    tests/unit/test_semver.cpp
    tests/unit/test_fixed32.cpp
    tests/unit/test_frame_budget.cpp
    tests/unit/test_ecs_world.cpp
    tests/unit/test_combat_calc.cpp
    tests/unit/test_bridge_roundtrip.cpp
    tests/unit/test_thread_roles.cpp
    tests/unit/test_render_tier.cpp
    tests/unit/test_save_journal.cpp
    tests/unit/test_canonical_json.cpp
    tests/unit/test_migration_runner.cpp
    tests/unit/test_source_authority.cpp
    tests/unit/test_save_recovery.cpp
    tests/unit/test_save_runtime.cpp
    tests/unit/test_event_edit_guard.cpp
    tests/unit/test_event_authority_diagnostics.cpp
    tests/unit/test_event_authority_panel.cpp
    tests/unit/test_event_runtime.cpp
    tests/unit/test_debug_session.cpp
    tests/unit/test_quickjs_runtime.cpp
    tests/unit/test_window_compat.cpp
    tests/unit/test_compat_report_panel.cpp
    tests/unit/test_battlemgr.cpp
    tests/unit/test_data_manager.cpp
    tests/unit/test_audio_manager.cpp
    tests/unit/test_input_manager.cpp
    tests/unit/test_plugin_manager.cpp
)

target_link_libraries(urpg_tests PRIVATE urpg_core Catch2::Catch2WithMain)
target_include_directories(urpg_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(urpg_integration_tests
    tests/integration/test_integration_runtime_recovery.cpp
)

target_link_libraries(urpg_integration_tests PRIVATE urpg_core Catch2::Catch2WithMain)
target_include_directories(urpg_integration_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(urpg_snapshot_tests
    tests/snapshot/test_snapshot_canonical_outputs.cpp
)

target_link_libraries(urpg_snapshot_tests PRIVATE urpg_core Catch2::Catch2WithMain)
target_include_directories(urpg_snapshot_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(urpg_compat_tests
    tests/compat/test_compat_authority_suite.cpp
    tests/compat/test_compat_window_plugin_profiles.cpp
    tests/compat/test_compat_plugin_fixtures.cpp
)

target_link_libraries(urpg_compat_tests PRIVATE urpg_core Catch2::Catch2WithMain)
target_include_directories(urpg_compat_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(urpg_compat_tests PRIVATE URPG_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

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
