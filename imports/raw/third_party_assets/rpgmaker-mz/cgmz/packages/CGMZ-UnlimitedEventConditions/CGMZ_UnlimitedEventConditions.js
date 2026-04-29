/*:
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/unlimitedeventconditions/
 * @target MZ
 * @base CGMZ_Core
 * @orderAfter CGMZ_Core
 * @plugindesc Unlimited event conditions on each event page
 * @help
 * ============================================================================
 * For terms and conditions using this plugin in your game please visit:
 * https://www.caspergaming.com/terms-of-use/
 * ============================================================================
 * Become a Patron to get access to beta/alpha plugins plus other goodies!
 * https://www.patreon.com/CasperGamingRPGM
 * ============================================================================
 * Version: 1.2.0
 * ----------------------------------------------------------------------------
 * Compatibility: Only tested with my CGMZ plugins.
 * Made for RPG Maker MZ 1.9.0
 * ----------------------------------------------------------------------------
 * Description: This plugin allows unlimited event conditions on each page.
 * By default, you are limited to only two switches, one variable, and some
 * other conditions. This removes those limitations.
 * ----------------------------------------------------------------------------
 * Documentation:
 * To use extra conditions, create a Comment event command and set the first
 * line in the comment to be:
 * CGMZ Event Conditions
 *
 * Afterwards, each line in that comment command will be one extra condition.
 * This plugin supports the following conditions:
 * 
 * Switch: [id] [ON|OFF]
 * - This will check if the provided switch with id is ON or OFF.
 * - Example: Switch: 1 OFF
 * 
 * Selfswitch: [mapId] [eventId] [A|B|C|D] [ON|OFF]
 * - This will check if the provided self switch with map and event id is
 *   ON or OFF.
 * - Example: Selfswitch: 10 8 A OFF
 * 
 * Variable: [id] [>|>=|=|<|<=|!=] [amount]
 * - This will use the provided operation to compare a variable with the
 *   given amount
 * - Example: Variable: 5 >= 100
 * 
 * Item: [id] [>|>=|=|<|<=|!=] [amount]
 * - This will check if the party has the provided amount of the item
 * - Example: Item: 2 > 9
 * 
 * Weapon: [id] [>|>=|=|<|<=|!=|equipped] [amount]
 * - This will check if the party has the provided amount of a weapon
 *   in their inventory or equipped. If using Equipped parameter, the
 *   amount will be ignored.
 * - Example: Weapon: 5 > 1
 * - Example: Weapon: 5 equipped
 * 
 * Armor: [id] [>|>=|=|<|<=|!=|equipped] [amount]
 * - This will check if the party has the provided amount of an armor
 *   in their inventory or equipped. If using Equipped parameter, the
 *   amount will be ignored.
 * - Example: Armor: 2 = 1
 * - Example: Armor: 2 equipped
 * 
 * Gold: [>|>=|=|<|<=|!=] [amount]
 * - This will check if the party has >, <, or = the provided amount of gold.
 * - Example: Gold: >= 100
 * 
 * Actor: [id] [in|out]
 * - This will check if the given actor is in or out of the party.
 * - Example: Actor: 4 out
 *
 * Vehicle: [boat|ship|airship] [on|off]
 * - This will check if the player is in a given vehicle.
 * - Example: Vehicle: boat on
 * 
 * Timer: [>|>=|=|<|<=|!=] [amount]
 * - This will check if the timer has >, <, or = the provided amount of time
 *  (in frames) left. Always false if timer is not active.
 * - Example: Timer: >= 60
 * - Please note that the timer does not update events when ticking down, so
 *   these conditions will mostly be useful when combined with some other
 *   condition or something that does cause the event to be updated.
 *
 * Skill: [actorId] [skillId] [has|no]
 * - This will check if the given actor has or does not have the given skill
 * - Example: Skill: 1 53 has
 * - The above example will check if actor id 1 has learned skill id 53.
 * - Please note that if Refresh After Skill Change is set to "false", events
 *   will not update to reflect skill changes until some other event occurs
 *   which causes events to be refreshed.
 *
 * State: [actorId] [stateId] [has|no]
 * - This will check if the given actor has or does not have the given state
 * - Example: State: 1 10 has
 * - The above example will check if actor id 1 is affected by state id 10.
 * - Please note that if Refresh After State Change is set to "false", events
 *   will not update to reflect state changes until some other event occurs
 *   which causes events to be refreshed.
 *
 * Script: [id]
 * - Runs the associated script which should return true / false
 * - Please note that events do not refresh until an action occurs that
 *   causes events to check their pages, such as switch or variable changes.
 *
 * You can also chain these comments together, just make sure that the first
 * line in each comment is "CGMZ Event Conditions" with one condition per
 * line.
 *
 * IMPORTANT:
 * This plugin reports errors! Press F8 in-game and open the console
 * to see if any events with additional conditions have incorrect formatting!
 * --------------------------Refresh Params------------------------------------
 * This plugin comes with some parameters such as "Refresh After Skill Change"
 *
 * These parameters will cause the map to refresh after a certain action, 
 * when normally the map would not refresh after this action, such as a skill 
 * learn or forget event.
 *
 * When the map refreshes, this makes events recheck their event pages and
 * therefore their conditions for each page. Without a map refresh, it is
 * possible the event would use the page that it used to meet conditions for
 * but no longer does (at least until the next refresh).
 *
 * Certain actions, such as turning switches on or off, automatically causes
 * a refresh and this is part of RMMZ base code. Other actions, such as skill
 * learn and forget do not. Turning these refresh params on will make the map
 * refresh more often and your events will use the correct event page at all
 * times, at the cost of checking event pages more frequently. In most cases,
 * this should not increase lag noticeably. However, if your game is expected
 * to run on low end hardware such as web/phones or you use maps with tons of
 * events, you may notice performance issues. In these cases, you might be
 * better off leaving these options off. If you do not use those event
 * conditions anywhere, it is also better to turn the refresh params off.
 * ---------------------------Saved Games--------------------------------------
 * This plugin fully supports saved games.
 * 
 * This means the following should cause no issues even in saved games:
 * ✓ Add this plugin to your game
 * ✓ Modify this plugin's parameters
 * ✓ Remove this plugin from your game
 * -----------------------------Filename---------------------------------------
 * The filename for this plugin MUST remain CGMZ_UnlimitedEventConditions.js
 * This is what it comes as when downloaded. The filename is used to load
 * parameters and execute plugin commands. If you change it, things will begin
 * behaving incorrectly and your game will probably crash. Please do not
 * rename the js file.
 * --------------------------Latest Version------------------------------------
 * Hi all, this latest version adds custom js conditions. You can set up custom
 * JS that should return true or false in the plugin parameters, assign it an
 * id, and then call that script via comment by typing the id into your
 * script comment.
 *
 * Events only check their page when the map is refreshed, which may not occur
 * when whatever your custom js is checking changes. This update adds a plugin
 * command to manually refresh the map so you can make events check their
 * pages as needed.
 *
 * Version 1.2.0
 * - Added custom js to conditions
 * - Added plugin command to refresh the map
 *
 * @command Refresh Map
 * @desc Refreshes the map, which will cause events to check their page
 *
 * @param Refresh After Skill Change
 * @type boolean
 * @default true
 * @desc Refresh events after an actor's skill changes?
 *
 * @param Refresh After State Change
 * @type boolean
 * @default true
 * @desc Refresh events after an actor's state changes?
 *
 * @param Scripts
 * @type struct<Script>[]
 * @default []
 * @desc Scripts that can be called by id in your conditions
*/
/*~struct~Script:
 * @param id
 * @desc The id to type into the event condition comment
 *
 * @param JS
 * @type multiline_string
 * @default return true;
 * @desc JS code to run. Should return true/false.
*/
Imported.CGMZ_UnlimitedEventConditions = true;
CGMZ.Versions["Unlimited Event Conditions"] = "1.2.0";
CGMZ.UnlimitedEventConditions = {};
CGMZ.UnlimitedEventConditions.parameters = PluginManager.parameters('CGMZ_UnlimitedEventConditions');
CGMZ.UnlimitedEventConditions.RefreshAfterSkillChange = (CGMZ.UnlimitedEventConditions.parameters["Refresh After Skill Change"] === "true");
CGMZ.UnlimitedEventConditions.RefreshAfterStateChange = (CGMZ.UnlimitedEventConditions.parameters["Refresh After State Change"] === "true");
CGMZ.UnlimitedEventConditions.Scripts = CGMZ_Utils.parseJSON(CGMZ.UnlimitedEventConditions.parameters["Scripts"], [], '[CGMZ] Event Conditions', 'Your Scripts parameter was set up incorrectly and could not be read.');
//=============================================================================
// CGMZ Temp
//-----------------------------------------------------------------------------
// Add unlimited event condition data
//=============================================================================
//-----------------------------------------------------------------------------
// Also initialize unlimited event condition data
//-----------------------------------------------------------------------------
const alias_CGMZUnlimitedEventConditions_CGMZTemp_createPluginData = CGMZ_Temp.prototype.createPluginData;
CGMZ_Temp.prototype.createPluginData = function() {
	alias_CGMZUnlimitedEventConditions_CGMZTemp_createPluginData.call(this);
	this.initializeUnlimitedEventConditionsData();
};
//-----------------------------------------------------------------------------
// Initialize unlimited event condition data
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.initializeUnlimitedEventConditionsData = function() {
	this.scriptEventConditions = {};
	for(const scriptJSON of CGMZ.UnlimitedEventConditions.Scripts) {
		const script = CGMZ_Utils.parseJSON(scriptJSON, null, '[CGMZ] Event Conditions', 'One of your scripts was set up incorrectly and could not be read');
		if(!script) continue;
		this.scriptEventConditions[script.id] = script.JS;
	}
};
//-----------------------------------------------------------------------------
// Get unlimited event condition script
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.getScriptEventCondition = function(id) {
	return this.scriptEventConditions[id];
};
//-----------------------------------------------------------------------------
// Register Plugin Commands
//-----------------------------------------------------------------------------
const alias_CGMZUnlimitedEventConditions_CGMZTemp_registerPluginCommands = CGMZ_Temp.prototype.registerPluginCommands;
CGMZ_Temp.prototype.registerPluginCommands = function() {
	alias_CGMZUnlimitedEventConditions_CGMZTemp_registerPluginCommands.call(this);
	PluginManager.registerCommand("CGMZ_UnlimitedEventConditions", "Refresh Map", this.pluginCommandUnlimitedEventConditionsRefreshMap);
};
//-----------------------------------------------------------------------------
// Plugin Command - Refresh Map
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandUnlimitedEventConditionsRefreshMap = function() {
	$gameMap.requestRefresh();
};
//=============================================================================
// Game_Event
//-----------------------------------------------------------------------------
// Check for additional conditions before allowing the page to activate
//=============================================================================
//-----------------------------------------------------------------------------
// Additional condition checks for event comments
//-----------------------------------------------------------------------------
const alias_CGMZ_EventConditions_meetsConditions = Game_Event.prototype.meetsConditions;
Game_Event.prototype.meetsConditions = function(page) {
    if(!alias_CGMZ_EventConditions_meetsConditions.call(this, page)) {
		return false;
	}
	let hasExtraConditions = false;
	for(const command of page.list) {
		if(hasExtraConditions) {
			if(command.code !== 408) {
				if(command.code === 108 && command.parameters[0].trim() === "CGMZ Event Conditions") {
					continue;
				} else {
					break;
				}
			}
			if(!this.CGMZ_EventConditions_meetsExtraCondition(command.parameters[0].trim())) {
				return false;
			}
		} else {
			hasExtraConditions = (command.code === 108 && command.parameters[0].trim() === "CGMZ Event Conditions");
		}
	}
	return true;
};
//-----------------------------------------------------------------------------
// Route extra condition to correct method for check and return result of condition
//-----------------------------------------------------------------------------
Game_Event.prototype.CGMZ_EventConditions_meetsExtraCondition = function(condition) {
	const params = condition.toLowerCase().split(" ");
    switch(params[0]) {
		case "switch:": return this.CGMZ_EventConditions_checkSwitch(params);
		case "selfswitch:": return this.CGMZ_EventConditions_checkSelfSwitch(params);
		case "variable:": return this.CGMZ_EventConditions_checkVariable(params);
		case "item:": return this.CGMZ_EventConditions_checkItem(params);
		case "weapon:": return this.CGMZ_EventConditions_checkWeapon(params);
		case "armor:": return this.CGMZ_EventConditions_checkArmor(params);
		case "gold:": return this.CGMZ_EventConditions_checkGold(params);
		case "actor:": return this.CGMZ_EventConditions_checkActor(params);
		case "vehicle:": return this.CGMZ_EventConditions_checkVehicle(params);
		case "timer:": return this.CGMZ_EventConditions_checkTimer(params);
		case "skill:": return this.CGMZ_EventConditions_checkSkill(params);
		case "state:": return this.CGMZ_EventConditions_checkState(params);
		case "script:": return this.CGMZ_EventConditions_checkScript(params);
	}
	return this.CGMZ_EventConditions_checkOtherCondition(params);
};
//-----------------------------------------------------------------------------
// Check switch extra condition
//-----------------------------------------------------------------------------
Game_Event.prototype.CGMZ_EventConditions_checkSwitch = function(params) {
	// Expected Format: Switch: [id] [ON|OFF]
	if(params.length === 3 && !isNaN(params[1])) {
		const expectedResult = (params[2] === 'on');
		return $gameSwitches.value(Number(params[1])) === expectedResult;
	}
	const error = `Switch Condition formatted incorrectly in event id = ${this._eventId}. Skipping condition.`;
	const solution = 'Change condition formatting to match: {Switch: [id] [ON|OFF]}';
	CGMZ_Utils.reportError(error, "[CGMZ] Event Conditions", solution);
	return true;
};
//-----------------------------------------------------------------------------
// Check self switch extra condition
//-----------------------------------------------------------------------------
Game_Event.prototype.CGMZ_EventConditions_checkSelfSwitch = function(params) {
	// Expected Format: Selfswitch: [mapId] [eventId] [A|B|C|D] [ON|OFF]
	if(params.length === 5) {
		const expectedResult = (params[4] === 'on');
		const key = [Number(params[1]), Number(params[2]), params[3].toUpperCase()]
		return $gameSelfSwitches.value(key) === expectedResult;
	}
	const error = `Self Switch Condition formatted incorrectly in event id = ${this._eventId}. Skipping condition.`;
	const solution = 'Change condition formatting to match: {Selfswitch: [mapId] [eventId] [A|B|C|D] [ON|OFF]}';
	CGMZ_Utils.reportError(error, "[CGMZ] Event Conditions", solution);
	return true;
};
//-----------------------------------------------------------------------------
// Check variable extra condition
//-----------------------------------------------------------------------------
Game_Event.prototype.CGMZ_EventConditions_checkVariable = function(params) {
	// Expected Format: Variable: [id] [>|>=|=|<|<=|!=] [amount]
	if(params.length === 4 && !isNaN(params[1]) && !isNaN(params[3])) {
		const value = $gameVariables.value(Number(params[1]));
		const amount = Number(params[3]);
		const comparator = params[2];
		return CGMZ_Utils.numberValueCompare(value, amount, comparator);
	}
	const error = `Variable Condition formatted incorrectly in event id = ${this._eventId}. Skipping condition.`;
	const solution = 'Change condition formatting to match: {Variable: [id] [>|>=|=|<|<=|!=] [amount]}';
	CGMZ_Utils.reportError(error, "[CGMZ] Event Conditions", solution);
	return true;
};
//-----------------------------------------------------------------------------
// Check item extra condition
//-----------------------------------------------------------------------------
Game_Event.prototype.CGMZ_EventConditions_checkItem = function(params) {
	// Expected Format: Item: [id] [>|>=|=|<|<=|!=] [amount]
	if(params.length === 4 && !isNaN(params[1]) && !isNaN(params[3])) {
		const item = $dataItems[Number(params[1])];
		const value = $gameParty.numItems(item);
		const amount = Number(params[3]);
		const comparator = params[2];
		return CGMZ_Utils.numberValueCompare(value, amount, comparator);
	}
	const error = `Item Condition formatted incorrectly in event id = ${this._eventId}. Skipping condition.`;
	const solution = 'Change condition formatting to match: {Item: [id] [>|>=|=|<|<=|!=] [amount]}';
	CGMZ_Utils.reportError(error, "[CGMZ] Event Conditions", solution);
	return true;
};
//-----------------------------------------------------------------------------
// Check weapon extra condition
//-----------------------------------------------------------------------------
Game_Event.prototype.CGMZ_EventConditions_checkWeapon = function(params) {
	// Expected Format: Weapon: [id] [>|>=|=|<|<=|!=|equipped] [amount]
	if(!isNaN(params[1])) {
		const weapon = $dataWeapons[Number(params[1])];
		if(params[2] === 'equipped') return $gameParty.isAnyMemberEquipped(weapon);
		const value = $gameParty.numItems(weapon);
		const amount = Number(params[3]);
		const comparator = params[2];
		return CGMZ_Utils.numberValueCompare(value, amount, comparator);
	}
	const error = `Weapon Condition formatted incorrectly in event id = ${this._eventId}. Skipping condition.`;
	const solution = 'Change condition formatting to match: {Weapon: [id] [>|>=|=|<|<=|!=|equipped] [amount]}';
	CGMZ_Utils.reportError(error, "[CGMZ] Event Conditions", solution);
	return true;
};
//-----------------------------------------------------------------------------
// Check armor extra condition
//-----------------------------------------------------------------------------
Game_Event.prototype.CGMZ_EventConditions_checkArmor = function(params) {
	// Expected Format: Armor: [id] [>|>=|=|<|<=|!=|equipped] [amount]
	if(params.length >= 3 && !isNaN(params[1])) {
		const armor = $dataArmors[Number(params[1])];
		if(params[2] === 'equipped') return $gameParty.isAnyMemberEquipped(armor);
		const value = $gameParty.numItems(armor);
		const amount = Number(params[3]);
		const comparator = params[2];
		return CGMZ_Utils.numberValueCompare(value, amount, comparator);
	}
	const error = `Armor Condition formatted incorrectly in event id = ${this._eventId}. Skipping condition.`;
	const solution = 'Change condition formatting to match: {Armor: [id] [>|>=|=|<|<=|!=|equipped] [amount]}';
	CGMZ_Utils.reportError(error, "[CGMZ] Event Conditions", solution);
	return true;
};
//-----------------------------------------------------------------------------
// Check gold extra condition
//-----------------------------------------------------------------------------
Game_Event.prototype.CGMZ_EventConditions_checkGold = function(params) {
	// Expected Format: Gold: [>|>=|=|<|<=|!=] [amount]
	if(params.length === 3) {
		const value = $gameParty.gold();
		const amount = Number(params[2]);
		const comparator = params[1];
		return CGMZ_Utils.numberValueCompare(value, amount, comparator);
	}
	const error = `Gold Condition formatted incorrectly in event id = ${this._eventId}. Skipping condition.`;
	const solution = 'Change condition formatting to match: {Gold: [>|>=|=|<|<=|!=] [amount]}';
	CGMZ_Utils.reportError(error, "[CGMZ] Event Conditions", solution);
	return true;
};
//-----------------------------------------------------------------------------
// Check actor extra condition
//-----------------------------------------------------------------------------
Game_Event.prototype.CGMZ_EventConditions_checkActor = function(params) {
	// Expected Format: Actor: [id] [in|out]
	if(params.length === 3 && !isNaN(params[1])) {
		const actor = $gameActors.actor(Number(params[1]));
        if (params[2] === 'in') {
            return $gameParty.members().includes(actor);
        } else {
			return !$gameParty.members().includes(actor);
		}
	}
	const error = `Actor Condition formatted incorrectly in event id = ${this._eventId}. Skipping condition.`;
	const solution = 'Change condition formatting to match: {Actor: [id] [in|out]}';
	CGMZ_Utils.reportError(error, "[CGMZ] Event Conditions", solution);
	return true;
};
//-----------------------------------------------------------------------------
// Check vehicle extra condition
//-----------------------------------------------------------------------------
Game_Event.prototype.CGMZ_EventConditions_checkVehicle = function(params) {
	// Expected Format: Vehicle: [boat|ship|airship] [on|off]
	if(params.length === 3) {
		switch(params[1].toLowerCase()) {
			case "boat": return params[2] === 'on' ? $gamePlayer.isInBoat() : !$gamePlayer.isInBoat();
			case "ship": return params[2] === 'on' ? $gamePlayer.isInShip() : !$gamePlayer.isInShip();
			case "airship": return params[2] === 'on' ? $gamePlayer.isInAirship() : !$gamePlayer.isInAirship();
		}
	}
	const error = `Vehicle Condition formatted incorrectly in event id = ${this._eventId}. Skipping condition.`;
	const solution = 'Change condition formatting to match: {Vehicle: [boat|ship|airship] [in|out]}';
	CGMZ_Utils.reportError(error, "[CGMZ] Event Conditions", solution);
	return true;
};
//-----------------------------------------------------------------------------
// Check timer extra condition
//-----------------------------------------------------------------------------
Game_Event.prototype.CGMZ_EventConditions_checkTimer = function(params) {
	// Expected Format: Timer: [>|>=|=|<|<=|!=] [amount]
	if(!$gameTimer.isWorking()) return false;
	if(params.length === 3) {
		const value = $gameTimer.frames();
		const amount = Number(params[2]);
		const comparator = params[1];
		return CGMZ_Utils.numberValueCompare(value, amount, comparator);
	}
	const error = `Timer Condition formatted incorrectly in event id = ${this._eventId}. Skipping condition.`;
	const solution = 'Change condition formatting to match: {Timer: [>|>=|=|<|<=|!=] [amount]}';
	CGMZ_Utils.reportError(error, "[CGMZ] Event Conditions", solution);
	return true;
};
//-----------------------------------------------------------------------------
// Check skill extra condition
//-----------------------------------------------------------------------------
Game_Event.prototype.CGMZ_EventConditions_checkSkill = function(params) {
	// Expected Format: Skill: [actorId] [skillId] [has|no]
	if(params.length === 4) {
		const actor = $gameActors.actor(Number(params[1]));
		const hasSkill = actor.hasSkill(Number(params[2]));
		const positiveCheck = params[3] === 'has';
		return (positiveCheck) ? hasSkill : !hasSkill;
	}
	const error = `Skill Condition formatted incorrectly in event id = ${this._eventId}. Skipping condition.`;
	const solution = 'Change condition formatting to match: {Skill: [actorId] [skillId] [has|no]}';
	CGMZ_Utils.reportError(error, "[CGMZ] Event Conditions", solution);
	return true;
};
//-----------------------------------------------------------------------------
// Check state extra condition
//-----------------------------------------------------------------------------
Game_Event.prototype.CGMZ_EventConditions_checkState = function(params) {
	// Expected Format: State: [actorId] [stateId] [has|no]
	if(params.length === 4) {
		const actor = $gameActors.actor(Number(params[1]));
		const hasState = actor.isStateAffected(Number(params[2]));
		const positiveCheck = params[3] === 'has';
		return (positiveCheck) ? hasState : !hasState;
	}
	const error = `State Condition formatted incorrectly in event id = ${this._eventId}. Skipping condition.`;
	const solution = 'Change condition formatting to match: {State: [actorId] [stateId] [has|no]}';
	CGMZ_Utils.reportError(error, "[CGMZ] Event Conditions", solution);
	return true;
};
//-----------------------------------------------------------------------------
// Check script extra condition
//-----------------------------------------------------------------------------
Game_Event.prototype.CGMZ_EventConditions_checkScript = function(params) {
	// Expected Format: Script: id
	if(params.length === 2) {
		const script = $cgmzTemp.getScriptEventCondition(params[1].trim());
		if(script) {
			const fn = new Function(script);
			return fn.call(this);
		} else {
			const error = `Script Condition not found for script id ${params[1]}.`;
			const solution = 'Make sure id exists (should be all lowercase)';
			CGMZ_Utils.reportError(error, "[CGMZ] Event Conditions", solution);
			return true;
		}
	}
	const error = `Script Condition formatted incorrectly in event id = ${this._eventId}. Skipping condition.`;
	const solution = 'Change condition formatting to match: {Script: id}';
	CGMZ_Utils.reportError(error, "[CGMZ] Event Conditions", solution);
	return true;
};
//-----------------------------------------------------------------------------
// Check unknown extra condition (modify this if expanding plugin functionality)
// This should return true if the condition is satisfied, and false if not.
//-----------------------------------------------------------------------------
Game_Event.prototype.CGMZ_EventConditions_checkOtherCondition = function(params) {
	return true;
};
//=============================================================================
// Game_Party
//-----------------------------------------------------------------------------
// Request a refresh when gold changes
//=============================================================================
//-----------------------------------------------------------------------------
// Also refresh map when gold changes
//-----------------------------------------------------------------------------
const alias_CGMZ_EventConditions_Game_Party_gainGold = Game_Party.prototype.gainGold;
Game_Party.prototype.gainGold = function(amount) {
	alias_CGMZ_EventConditions_Game_Party_gainGold.call(this, amount);
	$gameMap.requestRefresh();
};
//=============================================================================
// Game_Player
//-----------------------------------------------------------------------------
// Request a refresh when vehicle changes
//=============================================================================
//-----------------------------------------------------------------------------
// Also refresh map when getting on vehicle
//-----------------------------------------------------------------------------
const alias_CGMZ_EventConditions_Game_Player_getOnVehicle = Game_Player.prototype.getOnVehicle;
Game_Player.prototype.getOnVehicle = function() {
	const returnVal = alias_CGMZ_EventConditions_Game_Player_getOnVehicle.call(this);
	$gameMap.requestRefresh();
	return returnVal;
};
//-----------------------------------------------------------------------------
// Also refresh map when getting off vehicle
//-----------------------------------------------------------------------------
const alias_CGMZ_EventConditions_Game_Player_updateVehicleGetOff = Game_Player.prototype.updateVehicleGetOff;
Game_Player.prototype.updateVehicleGetOff = function() {
	if (!this.areFollowersGathering() && this.vehicle().isLowest()) {
		$gameMap.requestRefresh();
	}
	alias_CGMZ_EventConditions_Game_Player_updateVehicleGetOff.call(this);
};
//=============================================================================
// Game_Actor
//-----------------------------------------------------------------------------
// Request a refresh when skills change
//=============================================================================
//-----------------------------------------------------------------------------
// Also refresh map when actor learns skill if set to do so
//-----------------------------------------------------------------------------
const alias_CGMZUnlimitedEventConditions_GameActor_learnSkill = Game_Actor.prototype.learnSkill;
Game_Actor.prototype.learnSkill = function(skillId) {
	alias_CGMZUnlimitedEventConditions_GameActor_learnSkill.apply(this, arguments);
	if(CGMZ.UnlimitedEventConditions.RefreshAfterSkillChange) $gameMap.requestRefresh();
};
//-----------------------------------------------------------------------------
// Also refresh map when actor forgets skill if set to do so
//-----------------------------------------------------------------------------
const alias_CGMZUnlimitedEventConditions_GameActor_forgetSkill = Game_Actor.prototype.forgetSkill;
Game_Actor.prototype.forgetSkill = function(skillId) {
    alias_CGMZUnlimitedEventConditions_GameActor_forgetSkill.apply(this, arguments);
	if(CGMZ.UnlimitedEventConditions.RefreshAfterSkillChange) $gameMap.requestRefresh();
};
//-----------------------------------------------------------------------------
// Also refresh map when state is erased
//-----------------------------------------------------------------------------
const alias_CGMZUnlimitedEventConditions_GameActor_addState = Game_Actor.prototype.addState;
Game_Actor.prototype.addState = function(stateId) {
    alias_CGMZUnlimitedEventConditions_GameActor_addState.apply(this, arguments);
	if(CGMZ.UnlimitedEventConditions.RefreshAfterStateChange) $gameMap.requestRefresh();
};
//-----------------------------------------------------------------------------
// Also refresh map when state is added
//-----------------------------------------------------------------------------
const alias_CGMZUnlimitedEventConditions_GameActor_removeState = Game_Actor.prototype.removeState;
Game_Actor.prototype.removeState = function(stateId) {
    alias_CGMZUnlimitedEventConditions_GameActor_removeState.apply(this, arguments);
	if(CGMZ.UnlimitedEventConditions.RefreshAfterStateChange) $gameMap.requestRefresh();
};