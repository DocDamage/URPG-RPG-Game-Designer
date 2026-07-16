#include "engine/core/events/native_event_command_matrix.h"
#include <algorithm>

namespace urpg::events {
const std::vector<NativeEventCommandCapability>& nativeEventCommandMatrix(){
    static const std::vector<NativeEventCommandCapability> matrix={
        {EventCommandKind::Message,"message","narrative","message_editor","message","show_message",{"event_message_empty"}},
        {EventCommandKind::Condition,"condition","control_flow","condition_builder","condition","evaluate_condition",{"event_condition_invalid"}},
        {EventCommandKind::ElseBranch,"else","control_flow","branch_editor","else","enter_else",{"event_else_unmatched"}},
        {EventCommandKind::EndBranch,"end_branch","control_flow","branch_editor","end_branch","leave_branch",{"event_branch_unclosed"}},
        {EventCommandKind::Loop,"loop","control_flow","loop_editor","loop","enter_loop",{"event_loop_unbounded"}},
        {EventCommandKind::BreakLoop,"break_loop","control_flow","loop_editor","break_loop","break_loop",{"event_break_outside_loop"}},
        {EventCommandKind::EndLoop,"end_loop","control_flow","loop_editor","end_loop","repeat_loop",{"event_loop_unclosed"}},
        {EventCommandKind::Wait,"wait","timing","duration_editor","wait","wait_frames",{"event_wait_invalid"}},
        {EventCommandKind::Timer,"timer","timing","timer_editor","timer","set_timer",{"event_timer_invalid"}},
        {EventCommandKind::Parallel,"parallel","control_flow","parallel_lane_editor","parallel","start_parallel_lane",{"event_parallel_unsafe_write"}},
        {EventCommandKind::Switch,"switch","state","switch_picker","switch","set_switch",{"missing_switch_reference"}},
        {EventCommandKind::Variable,"variable","state","variable_picker","variable","set_variable",{"missing_variable_reference"}},
        {EventCommandKind::SelfSwitch,"self_switch","state","self_switch_picker","self_switch","set_self_switch",{"event_self_switch_invalid"}},
        {EventCommandKind::MovementRoute,"movement_route","world","movement_route_editor","movement_route","move_entity",{"event_movement_route_invalid"}},
        {EventCommandKind::Camera,"camera","presentation","camera_editor","camera","camera_action",{"event_camera_target_missing"}},
        {EventCommandKind::Animation,"animation","presentation","animation_picker","animation","play_animation",{"event_animation_missing"}},
        {EventCommandKind::Sound,"audio","presentation","audio_picker","sound","play_audio",{"event_audio_missing"}},
        {EventCommandKind::CommonEvent,"common_event","composition","common_event_picker","common_event","call_common_event",{"missing_common_event"}},
        {EventCommandKind::Transfer,"transfer","world","map_entrance_picker","transfer","transfer_player",{"missing_transfer_map"}},
        {EventCommandKind::Battle,"battle","gameplay","encounter_picker","battle","start_battle",{"event_battle_missing"}},
        {EventCommandKind::Shop,"shop","gameplay","vendor_picker","shop","open_shop",{"event_shop_missing"}},
        {EventCommandKind::Item,"item","gameplay","item_picker","item","change_item",{"event_item_missing"}},
        {EventCommandKind::Gold,"gold","gameplay","amount_editor","gold","change_gold",{"event_amount_invalid"}},
        {EventCommandKind::Fade,"fade","presentation","fade_editor","fade","fade_screen",{"event_fade_invalid"}},
        {EventCommandKind::Script,"script","extension","controlled_call_picker","script","call_controlled_script",{"event_script_not_allowed"},true,true},
        {EventCommandKind::Extension,"extension","extension","controlled_call_picker","extension","call_native_extension",{"event_extension_not_allowed"},true,true},
    };return matrix;
}
std::vector<EventDiagnostic> validateNativeEventCommandMatrix(){std::vector<EventDiagnostic> diagnostics;std::set<std::string> ids,keys;
    for(const auto& row:nativeEventCommandMatrix()){if(row.id.empty()||row.authoring_ui.empty()||row.serialization_key.empty()||row.runtime_effect.empty()||row.diagnostic_codes.empty()||!ids.insert(row.id).second||!keys.insert(row.serialization_key).second)
        diagnostics.push_back({EventDiagnosticSeverity::Error,"native_event_command_matrix_invalid","Native event command capability is incomplete or duplicated.","","",row.id});}return diagnostics;}
NativeEventCommandResult executeNativeEventCommand(const EventCommand& command,NativeEventRuntimeState& state){NativeEventCommandResult result;const auto found=std::find_if(nativeEventCommandMatrix().begin(),nativeEventCommandMatrix().end(),[&](const auto& row){return row.kind==command.kind;});
    if(found==nativeEventCommandMatrix().end()){result.code="native_event_command_unsupported";return result;}result.effect=found->runtime_effect;
    if(found->controlled_call&&!state.allowed_calls.contains(command.target)){result.code=command.kind==EventCommandKind::Script?"event_script_not_allowed":"event_extension_not_allowed";return result;}
    const auto change=[&](std::string value){state.changes.push_back(value);result.changes.push_back(std::move(value));};
    switch(command.kind){case EventCommandKind::Switch:state.switches[command.target]=command.value=="true"||command.amount!=0;change("switch:"+command.target);break;
    case EventCommandKind::Variable:state.variables[command.target]=command.amount;change("variable:"+command.target);break;
    case EventCommandKind::SelfSwitch:state.self_switches[command.target]=command.value=="true"||command.amount!=0;change("self_switch:"+command.target);break;
    case EventCommandKind::Condition:{const auto it=state.switches.find(command.target);result.branch_taken=it!=state.switches.end()&&it->second; ++state.branch_depth;break;}
    case EventCommandKind::ElseBranch:result.branch_taken=true;break;case EventCommandKind::EndBranch:if(state.branch_depth==0){result.code="event_branch_unmatched";return result;}--state.branch_depth;break;
    case EventCommandKind::Loop:++state.loop_depth;break;case EventCommandKind::BreakLoop:if(state.loop_depth==0){result.code="event_break_outside_loop";return result;}--state.loop_depth;break;
    case EventCommandKind::EndLoop:if(state.loop_depth==0){result.code="event_loop_unmatched";return result;}break;
    case EventCommandKind::Wait:case EventCommandKind::Timer:state.wait_frames=command.amount;result.waits=command.amount>0;break;
    case EventCommandKind::Parallel:++state.parallel_lane_count;break;
    default:state.actions.push_back(found->runtime_effect+":"+command.target+":"+command.value);break;}
    result.success=true;result.code="native_event_command_executed";return result;}
} // namespace urpg::events
