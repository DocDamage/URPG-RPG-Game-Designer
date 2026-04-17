#pragma once

#include <cstdint>
#include <vector>
#include <cstddef>

namespace urpg::presentation {

/**
 * @brief Simple linear arena allocator for per-frame intent data.
 * ADR-010: Zero heap allocation in the hot path.
 */
class PresentationArena {
public:
    PresentationArena(size_t capacityBytes)
        : m_capacity(capacityBytes)
        , m_offset(0) {
        m_buffer.resize(m_capacity);
    }

    /**
     * @brief Allocate raw memory from the arena.
     */
    void* Allocate(size_t size, size_t alignment = alignof(std::max_align_t)) {
        size_t current = reinterpret_cast<size_t>(m_buffer.data() + m_offset);
        size_t aligned = (current + alignment - 1) & ~(alignment - 1);
        size_t newOffset = (aligned - reinterpret_cast<size_t>(m_buffer.data())) + size;

        if (newOffset > m_capacity) {
            // Section 19: Arena overflow is a diagnostic/error condition
            return nullptr; 
        }

        m_offset = newOffset;
        return reinterpret_cast<void*>(aligned);
    }

    /**
     * @brief Reset the arena for the next frame.
     */
    void Reset() {
        m_offset = 0;
    }

    size_t GetUsedBytes() const { return m_offset; }
    size_t GetCapacity() const { return m_capacity; }

private:
    std::vector<uint8_t> m_buffer;
    size_t m_capacity;
    size_t m_offset;
};

} // namespace urpg::presentation
