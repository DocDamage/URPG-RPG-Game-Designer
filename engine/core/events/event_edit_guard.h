#pragma once

#include "engine/core/sync/source_authority.h"

#include <string>

namespace urpg {

struct EventEditRequest {
    std::string event_id;
    std::string block_id;
    AuthorityTag authority;
    EditOperation operation = EditOperation::EditUrpgAst;
    std::string timestamp_utc;
};

struct EventEditResult {
    bool allowed = false;
    std::string diagnostic_jsonl;
};

class EventEditGuard {
public:
    static EventEditResult ValidateAndDiagnose(const EventEditRequest& request);
};

} // namespace urpg
