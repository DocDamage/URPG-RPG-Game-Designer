#include <cstddef>
#include <cstdint>

int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    (void)data;
    (void)size;

    // Contract placeholder:
    // - Feed mutated JSON into migration runner.
    // - Assert: never throws uncaught exceptions.
    // - Result must be Ok(valid_json) or explicit MigrationError.
    return 0;
}
