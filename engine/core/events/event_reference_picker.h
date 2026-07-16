#pragma once
#include "engine/core/events/event_document.h"
#include <optional>

namespace urpg::events {
enum class EventReferenceKind { Map, MapEntry, Event, Actor, Item, Skill, Switch, Variable, CommonEvent, Audio, Animation, Quest, Dialogue };
struct EventReferenceOption { EventReferenceKind kind=EventReferenceKind::Map; std::string stable_id,label,detail,owner_id; bool available=true; };
struct EventReferenceBindingResult { bool success=false; std::string code; EventCommand command; };
class EventReferencePickerCatalog {
  public:
    bool add(EventReferenceOption option);
    std::vector<EventReferenceOption> search(EventReferenceKind kind,std::string query={}) const;
    std::optional<EventReferenceOption> resolve(EventReferenceKind kind,const std::string& stable_id) const;
    EventReferenceBindingResult bind(EventCommand command,EventReferenceKind kind,const std::string& stable_id) const;
    std::vector<EventReferenceKind> supportedKinds() const;
  private: std::vector<EventReferenceOption> options_;
};
const char* eventReferenceKindId(EventReferenceKind kind);
std::optional<EventReferenceKind> pickerKindForEventCommand(EventCommandKind kind);
} // namespace urpg::events
