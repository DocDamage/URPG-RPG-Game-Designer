#pragma once

#include <algorithm>
#include <cstdint>
#include <functional>
#include <utility>
#include <memory>
#include <typeindex>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace urpg {

using EntityID = uint32_t;

class World {
public:
    EntityID CreateEntity() {
        const EntityID id = next_id_++;
        alive_.push_back(id);
        return id;
    }

    void DestroyEntity(EntityID id) {
        const auto iter = std::lower_bound(alive_.begin(), alive_.end(), id);
        if (iter != alive_.end() && *iter == id) {
            alive_.erase(iter);
        }
    }

    template <typename T>
    T& AddComponent(EntityID id, const T& component) {
        auto& storage = TypedStorage<T>();
        storage[id] = component;
        return storage[id];
    }

    template <typename T>
    T* GetComponent(EntityID id) {
        auto& storage = TypedStorage<T>();
        auto iter = storage.find(id);
        if (iter == storage.end()) {
            return nullptr;
        }
        return &iter->second;
    }

    template <typename... Ts, typename Fn>
    void ForEachWith(Fn&& fn) {
        for (EntityID id : alive_) {
            if constexpr (sizeof...(Ts) == 0) {
                fn(id);
            } else {
                if ((GetComponent<Ts>(id) && ...)) {
                    fn(id, *GetComponent<Ts>(id)...);
                }
            }
        }
    }

private:
    struct IStorage {
        virtual ~IStorage() = default;
    };

    template <typename T>
    struct Storage final : IStorage {
        std::unordered_map<EntityID, T> data;
    };

    EntityID next_id_ = 1;
    std::vector<EntityID> alive_;
    std::unordered_map<std::type_index, std::unique_ptr<IStorage>> storages_;

    template <typename T>
    std::unordered_map<EntityID, T>& TypedStorage() {
        const std::type_index key = std::type_index(typeid(T));
        auto iter = storages_.find(key);
        if (iter == storages_.end()) {
            auto storage = std::make_unique<Storage<T>>();
            auto* raw = storage.get();
            storages_[key] = std::move(storage);
            return raw->data;
        }

        return static_cast<Storage<T>*>(iter->second.get())->data;
    }
};

} // namespace urpg
