#pragma once
#include "engine/core/events/native_event_command_matrix.h"
#include <functional>
#include <map>
#include <optional>
#include <set>

namespace urpg::editor {
struct EventCommandSequenceRow { urpg::events::EventCommand command; int32_t indent=0; bool collapsed=false; };
struct EventCommandSearchResult { std::string id, category, authoring_ui; bool favorite=false, recent=false; };
struct EventCommandEditResult { bool success=false; std::string code; size_t affected_count=0; };
enum class EventSequenceKeyAction { MoveUp, MoveDown, Copy, Paste, Delete, ToggleCollapse };
class EventCommandSequenceEditor {
  public:
    void load(std::vector<EventCommandSequenceRow> rows);
    std::vector<std::string> validateNesting() const;
    std::vector<EventCommandSearchResult> search(std::string query) const;
    bool setFavorite(std::string command_id,bool favorite);
    bool registerTemplate(std::string id,std::vector<EventCommandSequenceRow> rows);
    EventCommandEditResult insert(size_t index,EventCommandSequenceRow row);
    EventCommandEditResult insertTemplate(size_t index,const std::string& id);
    EventCommandEditResult copy(size_t first,size_t count);
    EventCommandEditResult paste(size_t index);
    EventCommandEditResult reorder(size_t first,size_t count,size_t destination);
    EventCommandEditResult multiEdit(const std::vector<size_t>& indices,std::string target,std::string value,int64_t amount);
    EventCommandEditResult keyAction(EventSequenceKeyAction action,size_t index,size_t count=1);
    std::vector<EventCommandSequenceRow> visibleRows() const;
    const std::vector<EventCommandSequenceRow>& rows() const{return rows_;}
  private:
    EventCommandEditResult mutate(const std::function<void()>& operation,size_t affected,std::string code);
    void recordRecent(const std::string& id);
    std::vector<EventCommandSequenceRow> rows_,clipboard_;
    std::map<std::string,std::vector<EventCommandSequenceRow>> templates_;
    std::set<std::string> favorites_; std::vector<std::string> recent_; size_t copy_serial_=0;
};
} // namespace urpg::editor
