#include "engine/core/threading/thread_roles.h"

#include <atomic>
#include <catch2/catch_test_macros.hpp>
#include <thread>
#include <vector>

TEST_CASE("Thread roles map to correct script access", "[threading]") {
    REQUIRE(urpg::ScriptAccessForRole(urpg::ThreadRole::Logic) == urpg::ScriptAccess::Full);
    REQUIRE(urpg::ScriptAccessForRole(urpg::ThreadRole::AssetStreaming) == urpg::ScriptAccess::CallbackOnly);
    REQUIRE(urpg::ScriptAccessForRole(urpg::ThreadRole::Render) == urpg::ScriptAccess::None);
    REQUIRE(urpg::ScriptAccessForRole(urpg::ThreadRole::Audio) == urpg::ScriptAccess::None);
}

TEST_CASE("Thread registry identifies current role", "[threading]") {
    urpg::ThreadRegistry registry;
    registry.RegisterCurrentThread(urpg::ThreadRole::Logic);

    REQUIRE(registry.IsCurrentThread(urpg::ThreadRole::Logic));
    REQUIRE(registry.CurrentScriptAccess() == urpg::ScriptAccess::Full);
    REQUIRE(registry.CanRunScriptDirectly());
}

TEST_CASE("Thread registry tolerates concurrent registration and queries", "[threading]") {
    urpg::ThreadRegistry registry;
    std::atomic<bool> start{false};
    std::atomic<bool> stop{false};
    std::atomic<int> querySuccesses{0};
    std::atomic<bool> writerObservedFailure{false};

    std::vector<std::thread> threads;
    threads.emplace_back([&]() {
        while (!start.load(std::memory_order_acquire)) {
        }

        for (int i = 0; i < 1000; ++i) {
            registry.RegisterCurrentThread(urpg::ThreadRole::Logic);
            const bool roleMatches = registry.IsCurrentThread(urpg::ThreadRole::Logic);
            const bool accessMatches = registry.CurrentScriptAccess() == urpg::ScriptAccess::Full;
            const bool canRunScript = registry.CanRunScriptDirectly();
            if (!roleMatches || !accessMatches || !canRunScript) {
                writerObservedFailure.store(true, std::memory_order_release);
                break;
            }
        }

        stop.store(true, std::memory_order_release);
    });

    for (int i = 0; i < 3; ++i) {
        threads.emplace_back([&]() {
            while (!start.load(std::memory_order_acquire)) {
            }

            while (!stop.load(std::memory_order_acquire)) {
                (void)registry.IsCurrentThread(urpg::ThreadRole::Logic);
                (void)registry.CurrentScriptAccess();
                (void)registry.CanRunScriptDirectly();
                querySuccesses.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    start.store(true, std::memory_order_release);

    for (auto& thread : threads) {
        thread.join();
    }

    REQUIRE_FALSE(writerObservedFailure.load(std::memory_order_acquire));
    REQUIRE(querySuccesses.load(std::memory_order_relaxed) > 0);
}
