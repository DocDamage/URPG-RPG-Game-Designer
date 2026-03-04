cmake_minimum_required(VERSION 3.23)
project(urpg LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(URPG_FETCH_CATCH2 "Fetch Catch2 via CMake" ON)

add_library(urpg_core
    engine/core/semver.cpp
    engine/core/threading/thread_roles.cpp
    engine/core/format/canonical_json.cpp
    engine/core/migrate/migration_runner.cpp
    engine/core/sync/source_authority.cpp
    engine/core/save/save_journal.cpp
    engine/core/save/save_recovery.cpp
    engine/gameplay/combat/combat_calc.cpp
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
)

target_link_libraries(urpg_tests PRIVATE urpg_core Catch2::Catch2WithMain)
target_include_directories(urpg_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

include(CTest)
include(Catch)
catch_discover_tests(urpg_tests TEST_LIST urpg_discovered_tests)

if(urpg_discovered_tests)
    set_tests_properties(${urpg_discovered_tests} PROPERTIES LABELS "pr")
endif()

add_test(NAME ci.nightly.placeholder COMMAND ${CMAKE_COMMAND} -E echo "nightly gate placeholder")
set_tests_properties(ci.nightly.placeholder PROPERTIES LABELS "nightly")

add_test(NAME ci.weekly.placeholder COMMAND ${CMAKE_COMMAND} -E echo "weekly gate placeholder")
set_tests_properties(ci.weekly.placeholder PROPERTIES LABELS "weekly")
