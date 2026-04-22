#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace urpg::social {

    /**
     * @brief Cloud Provider abstraction for different platforms.
     */
    enum class CloudProvider {
        SteamPayload,
        EpicOnline,
        GOGGalaxy,
        GenericHTTP,
        LocalSimulated
    };

    /**
     * @brief Result of a cloud operation.
     */
    struct CloudResult {
        bool success;
        std::string message;
        int64_t timestamp;
    };

    /**
     * @brief Cloud-sync abstraction consumed by AI/save orchestration layers.
     * Part of Phase 4 Ecosystem integration, but production provider bindings are
     * still pending; current in-tree usage is primarily exercised through a
     * process-local double.
     */
    class ICloudService {
    public:
        virtual ~ICloudService() = default;

        /**
         * @brief Initialize the cloud link with the specified provider.
         * The in-tree local-memory implementation only accepts LocalSimulated and
         * does not establish a live provider session.
         */
        virtual CloudResult initialize(CloudProvider provider, const std::string& apiKey) = 0;

        /**
         * @brief Sync local data to the backing store.
         * In-tree local-memory implementations only copy bytes into process-local memory.
         */
        virtual CloudResult syncToCloud(const std::string& key, const std::vector<uint8_t>& data) = 0;

        /**
         * @brief Retrieve data from the backing store.
         * In-tree local-memory implementations only read from process-local memory.
         */
        virtual std::vector<uint8_t> fetchFromCloud(const std::string& key) = 0;

        /**
         * @brief List available keys in the backing store namespace.
         */
        virtual std::vector<std::string> listRemoteKeys() = 0;

        /**
         * @brief Check if the current environment allows cloud operations.
         * For stubs, this only reflects local stub readiness, not internet reachability.
         */
        virtual bool isOnline() const = 0;

        /**
         * @brief Get status of the last sync operation.
         */
        virtual CloudResult getStatus() const = 0;
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
            m_provider = provider;
            if (provider != CloudProvider::LocalSimulated) {
                m_online = false;
                return {false, "NOT LIVE: LocalInMemoryCloudService only supports CloudProvider::LocalSimulated and cannot initialize live provider bindings.", 123456789};
            }

            m_online = true;
            return {true, "NOT LIVE: LocalInMemoryCloudService initialized process-local memory only; no remote provider connection was made.", 123456789};
        }

        CloudResult syncToCloud(const std::string& key, const std::vector<uint8_t>& data) override {
            if (!m_online) {
                return {false, "NOT LIVE: LocalInMemoryCloudService is offline; initialize CloudProvider::LocalSimulated before using process-local storage.", 123456790};
            }

            m_storage[key] = data;
            return {true, "NOT LIVE: data stored in process-local memory only; nothing was uploaded to a remote provider.", 123456791};
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
            return {m_online, m_online ? "NOT LIVE: LocalInMemoryCloudService is ready for process-local storage only; no remote transport is configured." : "NOT LIVE: LocalInMemoryCloudService is offline.", 0};
        }

    private:
        CloudProvider m_provider = CloudProvider::LocalSimulated;
        bool m_online = false;
        std::map<std::string, std::vector<uint8_t>> m_storage;
    };

    /**
     * @brief Backward-compatible alias for the old stub name.
     * Prefer LocalInMemoryCloudService at new call sites so the local-only scope
     * is explicit at the type level.
     */
    using CloudServiceStub = LocalInMemoryCloudService;

} // namespace urpg::social
