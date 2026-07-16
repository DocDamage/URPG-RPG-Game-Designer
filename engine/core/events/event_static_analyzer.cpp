#include "engine/core/events/event_static_analyzer.h"
#include <algorithm>
#include <map>
#include <set>
#include <tuple>

namespace urpg::events {
std::vector<EventStaticFinding> analyzeEventDocument(const EventDocument& document,const EventWorldState& state){std::vector<EventStaticFinding> findings;
    const auto add=[&](std::string severity,std::string code,std::string message,std::string suggestion,const std::string& event,const std::string& page,const std::string& command){findings.push_back({std::move(severity),std::move(code),std::move(message),std::move(suggestion),event,page,command});};
    for(const auto& diagnostic:document.validate(state))add(toString(diagnostic.severity),diagnostic.code,diagnostic.message,"Open the linked reference and choose a valid project object.",diagnostic.event_id,diagnostic.page_id,diagnostic.command_id);
    std::map<std::string,std::set<std::string>> outgoingTransfers;
    for(const auto& event:document.events())for(const auto& page:event.pages)for(const auto& command:page.commands)if(command.kind==EventCommandKind::Transfer)outgoingTransfers[event.map_id].insert(command.target);
    for(const auto& event:document.events())for(const auto& page:event.pages){bool terminal=false;bool stateMutation=false;std::map<std::string,size_t> parallelWrites;std::vector<size_t> loops;
        for(size_t index=0;index<page.commands.size();++index){const auto& command=page.commands[index];if(terminal)add("warning","event_command_unreachable","Command follows an unconditional terminal command.","Move it before the terminal command or remove it.",event.id,page.id,command.id);
            if(command.payload.value("terminal",false))terminal=true;
            if(command.kind==EventCommandKind::Switch||command.kind==EventCommandKind::Variable||command.kind==EventCommandKind::SelfSwitch){stateMutation=true;if(page.trigger==EventTrigger::Parallel||command.payload.value("parallel_lane",false)){const auto count=++parallelWrites[command.target];if(count>1)add("error","unsafe_parallel_mutation","Parallel lanes write the same state target.","Serialize these writes or assign one owner lane.",event.id,page.id,command.id);}}
            if(command.kind==EventCommandKind::Loop)loops.push_back(index);
            if(command.kind==EventCommandKind::EndLoop&&!loops.empty()){const auto start=loops.back();loops.pop_back();bool yields=false,breaks=false;for(size_t nested=start+1;nested<index;++nested){yields|=page.commands[nested].kind==EventCommandKind::Wait||page.commands[nested].kind==EventCommandKind::Timer;breaks|=page.commands[nested].kind==EventCommandKind::BreakLoop||page.commands[nested].kind==EventCommandKind::Condition;}
                if(!yields)add("error","tight_loop","Loop can execute without yielding a frame.","Insert a wait/timer or bounded asynchronous operation.",event.id,page.id,command.id);
                if(!breaks)add("error","infinite_loop","Loop has no visible break or condition.","Add a condition and reachable break.",event.id,page.id,command.id);
            }
            if(command.kind==EventCommandKind::Transfer&&document.maps().contains(command.target)&&outgoingTransfers[command.target].empty())add("warning","transfer_dead_end","Transfer target has no authored outgoing transfer.","Add a return/continuation route or mark the destination terminal.",event.id,page.id,command.id);
            if(command.kind==EventCommandKind::Message){if(command.payload.value("localization_id","").empty())add("warning","localization_id_missing","Message has no localization ID.","Choose or create a localized string.",event.id,page.id,command.id);if(!command.payload.value("voice_id","").empty()&&command.payload.value("caption_id","").empty())add("error","voice_caption_missing","Voiced message has no caption link.","Choose a caption/localization entry for this voice take.",event.id,page.id,command.id);}}
        if(page.trigger==EventTrigger::Autorun&&!stateMutation)add("warning","likely_autorun_softlock","Autorun page does not change state and may restart forever.","End by changing its page condition or transferring control.",event.id,page.id,"");}
    std::sort(findings.begin(),findings.end(),[](const auto& a,const auto& b){return std::tie(a.event_id,a.page_id,a.command_id,a.code)<std::tie(b.event_id,b.page_id,b.command_id,b.code);});return findings;}
} // namespace urpg::events
