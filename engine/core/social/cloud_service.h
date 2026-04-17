#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include "core/asset/asset_license_audit.h"

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
     * still pending; current in-tree usage is primarily exercised through the stub.
     */
    class ICloudService {
    public:
        virtual ~ICloudService() = default;

        /**
         * @brief Initialize the cloud link with the specified provider.
         */
        virtual CloudResult initialize(CloudProvider provider, const std::string& apiKey) = 0;

        /**
         * @brief Sync local data to the cloud.
         */
        virtual CloudResult syncToCloud(const std::string& key, const std::vector<uint8_t>& data) = 0;

        /**
         * @brief Retrieve data from the cloud.
         */
        virtual std::vector<uint8_t> fetchFromCloud(const std::string& key) = 0;

        /**
         * @brief List available remote saves.
         */
        virtual std::vector<std::string> listRemoteKeys() = 0;

        /**
         * @brief Check if the current environment allows cloud operations.
         */
        virtual bool isOnline() const = 0;

        /**
         * @brief Get status of the last sync operation.
         */
        virtual CloudResult getStatus() const = 0;
    };

    /**
     * @brief In-memory cloud-service stub for local testing only.
     * This implementation does not perform any real network transport, account
     * authentication, or cross-device persistence, and should not be described as
     * a production cloud integration path.
     */
    class CloudServiceStub : public ICloudService {
    public:
        CloudResult initialize(CloudProvider provider, const std::string& apiKey) override {
            m_provider = provider;
            m_online = true;
            return {true, "Connected to in-memory provider stub.", 123456789};
        }

        CloudResult syncToCloud(const std::string& key, const std::vector<uint8_t>& data) override {
            m_storage[key] = data;
            return {true, "Data stored in local memory stub.", 123456790};
        }

        std::vector<uint8_t> fetchFromCloud(const std::string& key) override {
            if (m_storage.find(key) != m_storage.end()) {
                return m_storage[key];
            }
            return {};
        }

        std::vector<std::string> listRemoteKeys() override {
            std::vector<std::string> keys;
            for (auto const& [key, _] : m_storage) {
                keys.push_back(key);
            }
            return keys;
        }

        bool isOnline() const override { return m_online; }
        CloudResult getStatus() const override { return {true, "Idle (memory stub)", 0}; }

    private:
        CloudProvider m_provider = CloudProvider::LocalSimulated;
        bool m_online = false;
        std::map<std::string, std::vector<uint8_t>> m_storage;
    };

} // namespace urpg::social
