#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace urpg::social {

/**
 * @brief Provider label for the in-tree cloud-service abstraction.
 * The shipped tree only supports the process-local LocalSimulated path.
 */
enum class CloudProvider { LocalSimulated };

/**
 * @brief Result of a cloud-service operation.
 * For the in-tree local-memory path, this reports process-local stub behavior
 * rather than any live remote transport.
 */
struct CloudResult {
    bool success;
    std::string message;
    int64_t timestamp;
};

struct CloudServiceVisibility {
    bool release_visible = false;
    bool remote_transport = false;
    std::string reason = "No reviewed remote cloud provider is configured.";
};

/**
 * @brief Cloud-service abstraction consumed by AI/save orchestration layers.
 * The accepted in-tree contract is deliberately local-only: production
 * provider bindings are not implemented here, and the only shipped
 * implementation is LocalInMemoryCloudService, which keeps data in
 * process-local memory with no live transport. Any real cloud backend is
 * future or out-of-tree feature work.
 */
class ICloudService {
  public:
    virtual ~ICloudService() = default;

    /**
     * @brief Initialize the cloud link with the specified provider.
     * The in-tree implementation only supports the process-local
     * LocalSimulated path and does not establish a live provider session.
     */
    virtual CloudResult initialize(CloudProvider provider, const std::string& apiKey) = 0;

    /**
     * @brief Sync local data to the configured backing store.
     * In-tree local-memory implementations only copy bytes into process-local memory.
     */
    virtual CloudResult syncToCloud(const std::string& key, const std::vector<uint8_t>& data) = 0;

    /**
     * @brief Retrieve data from the configured backing store.
     * In-tree local-memory implementations only read from process-local memory.
     */
    virtual std::vector<uint8_t> fetchFromCloud(const std::string& key) = 0;

    /**
     * @brief List available keys in the backing-store namespace.
     * In-tree local-memory implementations only enumerate process-local keys.
     */
    virtual std::vector<std::string> listRemoteKeys() = 0;

    /**
     * @brief Check whether the configured backing store is ready.
     * For the in-tree local-memory path, this only reflects local stub readiness,
     * not internet reachability or remote account state.
     */
    virtual bool isOnline() const = 0;

    /**
     * @brief Get status of the last backing-store operation.
     */
    virtual CloudResult getStatus() const = 0;

    /**
     * @brief Whether this service may be surfaced as production cloud sync.
     *
     * In-tree implementations default to hidden because the repository ships
     * only process-local storage. Real providers must override this after
     * review so UI can distinguish live cloud sync from local harness plumbing.
     */
    virtual CloudServiceVisibility releaseVisibility() const {
        return {false, false, "In-tree cloud service is local/dev-only; no reviewed remote provider is configured."};
    }
};

/**
 * @brief Process-local cloud-service double for tests and harness scenarios only.
 * This implementation does not perform any network transport, account
 * authentication, disk persistence, or cross-device synchronization. It only
 * mirrors bytes inside the current process and should not be presented as a
 * live provider integration path.
 */
class LocalInMemoryCloudService : public ICloudService {
  public:
    CloudResult initialize(CloudProvider provider, const std::string& apiKey) override {
        static_cast<void>(apiKey);
        static_cast<void>(provider);

        m_online = true;
        return {true,
                "NOT LIVE: LocalInMemoryCloudService initialized process-local memory only; no remote provider "
                "connection was made.",
                123456789};
    }

    CloudResult syncToCloud(const std::string& key, const std::vector<uint8_t>& data) override {
        if (!m_online) {
            return {false,
                    "NOT LIVE: LocalInMemoryCloudService is offline; initialize CloudProvider::LocalSimulated before "
                    "using process-local storage.",
                    123456790};
        }

        m_storage[key] = data;
        return {true, "NOT LIVE: data stored in process-local memory only; nothing was uploaded to a remote provider.",
                123456791};
    }

    std::vector<uint8_t> fetchFromCloud(const std::string& key) override {
        if (!m_online) {
            return {};
        }

        const auto found = m_storage.find(key);
        if (found != m_storage.end()) {
            return found->second;
        }
        return {};
    }

    std::vector<std::string> listRemoteKeys() override {
        if (!m_online) {
            return {};
        }

        std::vector<std::string> keys;
        for (auto const& [key, _] : m_storage) {
            keys.push_back(key);
        }
        return keys;
    }

    bool isOnline() const override { return m_online; }
    CloudResult getStatus() const override {
        return {m_online,
                m_online ? "NOT LIVE: LocalInMemoryCloudService is ready for process-local storage only; no remote "
                           "transport is configured."
                         : "NOT LIVE: LocalInMemoryCloudService is offline.",
                0};
    }

    CloudServiceVisibility releaseVisibility() const override {
        return {false, false,
                "LocalInMemoryCloudService is process-local test/dev storage only and must not be presented as "
                "cross-device or remote cloud sync."};
    }

  private:
    bool m_online = false;
    std::map<std::string, std::vector<uint8_t>> m_storage;
};

} // namespace urpg::social
