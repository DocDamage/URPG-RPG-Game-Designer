#pragma once

#include "engine/core/character/character_identity.h"
#include "engine/core/database/rpg_database.h"
#include "engine/core/dialogue/dialogue_graph.h"
#include "engine/core/events/event_document.h"

#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace urpg::project {

struct ContextualCreatorProjectResult {
    bool success = false;
    std::string code;
    std::string message;
    std::vector<std::string> diagnostics;
};

// The persisted handoff contract used by Map-context authoring.  It owns only
// project data and delegates editing/runtime behavior to the existing event,
// dialogue, character, and database subsystem owners.
class ContextualCreatorProject {
  public:
    static constexpr const char* kRelativePath = "content/contextual_authoring.json";

    ContextualCreatorProjectResult open(const std::filesystem::path& projectRoot);
    ContextualCreatorProjectResult save() const;
    nlohmann::json snapshot() const;

    void setEventDocument(events::EventDocument document);
    const events::EventDocument& eventDocument() const { return event_document_; }
    void setDialogue(std::string id, dialogue::DialogueGraph graph);
    const std::map<std::string, dialogue::DialogueGraph>& dialogues() const { return dialogues_; }
    void setCharacter(std::string id, character::CharacterIdentity identity);
    const std::map<std::string, character::CharacterIdentity>& characters() const { return characters_; }
    void setDatabase(database::RpgDatabase database);
    const database::RpgDatabase& database() const { return database_; }
    const std::filesystem::path& projectRoot() const { return project_root_; }

  private:
    ContextualCreatorProjectResult validate() const;

    std::filesystem::path project_root_;
    events::EventDocument event_document_;
    std::map<std::string, dialogue::DialogueGraph> dialogues_;
    std::map<std::string, character::CharacterIdentity> characters_;
    database::RpgDatabase database_;
};

} // namespace urpg::project
