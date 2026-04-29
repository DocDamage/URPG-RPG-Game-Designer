/*:
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/idle/
 * @target MZ
 * @base CGMZ_Core
 * @orderAfter CGMZ_Core
 * @plugindesc Detect when the player is idle
 * @help
 * ============================================================================
 * For terms and conditions using this plugin in your game please visit:
 * https://www.caspergaming.com/terms-of-use/
 * ============================================================================
 * Become a Patron to get access to beta/alpha plugins plus other goodies!
 * https://www.patreon.com/CasperGamingRPGM
 * ============================================================================
 * Version: 1.1.0
 * ----------------------------------------------------------------------------
 * Compatibility: Only tested with my CGMZ plugins.
 * Made for RPG Maker MZ 1.9.0
 * ----------------------------------------------------------------------------
 * Description: Allows you to detect when the player is idle (no input for a
 * certain amount of time) and run events based on that.
 * ----------------------------------------------------------------------------
 * Documentation:
 * -------------------------Plugin Commands------------------------------------
 * • Check Idle
 * Set a switch to the player's current idle status
 *
 * • Check Idle Frames
 * Set a variable to the current amount of frames the player has been idle for
 *
 * • Force Idle
 * Force the player to become idle (no effect if already idle). Ignores the
 * setting for if the player can be idle or not.
 *
 * • Clear Idle
 * Forces the player to no longer be idle (no effect if already not idle).
 * Ignores setting for if the player can be idle or not.
 * ----------------------Idle Switch/Variable----------------------------------
 * The Idle Switch will be turned ON when the player is idle, and OFF when the
 * player is not idle. This process is automatic, if you do not wish to use a
 * switch for this, you can set the Idle Switch to 0 to not use this feature.
 *
 * An alternative to the automatic switch is using the Plugin Command to check
 * if the player is idle as needed. Using the plugin command in a parallel
 * process event will be much less efficient than using the automatic Idle
 * Switch functionality.
 *
 * The Idle Variable will be set to 0 while the player is not idle. When idle,
 * the variable will automatically be set to the amount of frames the player
 * has been idle for. If you do not wish to use this feature, set the Idle
 * Variable to 0.
 *
 * An alternative to the automatic variable is using the Plugin Command to
 * check idle frames as needed. Using the plugin command in a parallel
 * process event will be slightly less efficient than using the automatic Idle
 * Variable functionality.
 * --------------------------Eventing Tips-------------------------------------
 * You can event idle behavior by checking if the switch is ON in an event that
 * runs parallel process. If ON, the player is idle and you can run whatever
 * happens when the player is idle. If OFF, you can reset the player back to
 * normal. For one-time results, use a second switch to compare both and
 * determine if the player is newly idle or has been idle for a while.
 *
 * You can event idle behavior by checking the Idle Variable's value. If you
 * want repeating idle behavior, you can compare the variable's mod (%) value
 * against a certain number. For example, checking if Idle Variable % 300 is
 * equal to 1 will run the event every 300 frames (5 seconds). Do not check
 * if the variable mod some number is equal to 0 as this will also run while
 * not idle.
 * ---------------------------Saved Games--------------------------------------
 * This plugin fully supports saved games.
 * ✓ You should be able to add this  plugin to a saved game
 * ✓ You can modify parameters and it will reflect accurately in game
 * ✓ You can remove this plugin with no issue to save data
 * -----------------------------Filename---------------------------------------
 * The filename of this plugin's JavaScript file MUST be CGMZ_Idle.js
 * This is what it comes as when downloaded. The filename is used to load
 * parameters and execute plugin commands. If you change it, things will begin
 * behaving incorrectly and your game will probably crash. Please do not
 * rename the js file.
 * --------------------------Latest Version------------------------------------
 * Hi all, this latest version changes the Change Idle plugin command which
 * used to be how you enabled/disabled the player from changing idle status
 * to a game switch instead. This should be much easier and more familiar to
 * use in your events or script calls. The functionality has not changed. If
 * you do not set this new switch parameter up in the plugin parameters, the
 * player will always be able to enter/exit idle state.
 *
 * Version 1.1.0
 * - Converted idle state change freeze to game switch from plugin command
 * 
 * @command Check Idle
 * @desc Sets a switch to the player's current idle status
 * 
 * @arg Switch
 * @type switch
 * @desc The id of the switch to set to the current idle status
 * @default 0
 * 
 * @command Check Idle Frames
 * @desc Set a variable to the current amount of frames the player has been idle for
 * 
 * @arg Variable
 * @type variable
 * @desc The id of the variable to store the current idle frame count in
 * @default 0
 * 
 * @command Force Idle
 * @desc Force a player to become idle
 * 
 * @command Clear Idle
 * @desc Force a player to no longer be idle
 * 
 * @command Get Total Idle Frames
 * @desc Set a variable to the total amount of frames the player has been idle throughout the game
 * 
 * @arg Variable
 * @type variable
 * @desc The id of the variable to store the idle frame count in
 * @default 0
 *
 * @param Idle Settings
 *
 * @param Allow Idle Switch
 * @parent Idle Settings
 * @type switch
 * @desc If set, this switch must be ON or the player will not enter the idle state
 * @default 0
 *
 * @param Idle Frames
 * @parent Idle Settings
 * @type number
 * @min 0
 * @desc Amount of frames with no input before player is considered idle (60f = 1s)
 * @default 18000
 *
 * @param Idle Switch
 * @parent Idle Settings
 * @type switch
 * @desc Switch which turns ON/OFF automatically when idle/not idle
 * @default 0
 *
 * @param Idle Variable
 * @parent Idle Settings
 * @type variable
 * @desc Variable which will count the amount of frames starting after the player is marked idle
 * @default 0
 *
 * @param Idle Events
 *
 * @param Idle Start Event
 * @parent Idle Events
 * @type common_event
 * @desc Common Event that will run when entering the idle state
 * @default 0
 *
 * @param Idle End Event
 * @parent Idle Events
 * @type common_event
 * @desc Common Event that will run when exiting the idle state
 * @default 0
 *
 * @param Idle Event On Map Only
 * @parent Idle Events
 * @type boolean
 * @desc Run idle start/stop common event only if the player is on the map?
 * @default true
 *
 * @param Battle Idle Start Event
 * @parent Idle Events
 * @type common_event
 * @desc Common Event that will run when entering the idle state while in battle
 * @default 0
 *
 * @param Battle Idle End Event
 * @parent Idle Events
 * @type common_event
 * @desc Common Event that will run when exiting the idle state while in battle
 * @default 0
*/
Imported.CGMZ_Idle = true;
CGMZ.Versions["Idle"] = "1.1.0";
CGMZ.Idle = {};
CGMZ.Idle.parameters = PluginManager.parameters('CGMZ_Idle');
CGMZ.Idle.AllowIdleSwitch = Number(CGMZ.Idle.parameters["Allow Idle Switch"]);
CGMZ.Idle.Frames = Number(CGMZ.Idle.parameters["Idle Frames"]);
CGMZ.Idle.Switch = Number(CGMZ.Idle.parameters["Idle Switch"]);
CGMZ.Idle.Variable = Number(CGMZ.Idle.parameters["Idle Variable"]);
CGMZ.Idle.IdleStartEvent = Number(CGMZ.Idle.parameters["Idle Start Event"]);
CGMZ.Idle.IdleEndEvent = Number(CGMZ.Idle.parameters["Idle End Event"]);
CGMZ.Idle.BattleIdleStartEvent = Number(CGMZ.Idle.parameters["Battle Idle Start Event"]);
CGMZ.Idle.BattleIdleEndEvent = Number(CGMZ.Idle.parameters["Battle Idle End Event"]);
CGMZ.Idle.IdleEventOnMapOnly = (CGMZ.Idle.parameters["Idle Event On Map Only"] === 'true');
//=============================================================================
// CGMZ
//-----------------------------------------------------------------------------
// Manage saved idle data.
//=============================================================================
//-----------------------------------------------------------------------------
// Initialize idle data
//-----------------------------------------------------------------------------
const alias_CGMZIdle_createPluginData = CGMZ_Core.prototype.createPluginData;
CGMZ_Core.prototype.createPluginData = function() {
	alias_CGMZIdle_createPluginData.call(this);
	this.totalIdleFrames = 0;
};
//-----------------------------------------------------------------------------
// Initialize total idle frame variable if not already
//-----------------------------------------------------------------------------
const alias_CGMZIdle_createAfterLoad = CGMZ_Core.prototype.createAfterLoad;
CGMZ_Core.prototype.createAfterLoad = function() {
	alias_CGMZIdle_createAfterLoad.call(this);
	if(!this.totalIdleFrames) this.totalIdleFrames = 0;
	$gameSwitches.setValue(CGMZ.Idle.Switch, false);
	$gameVariables.setValue(CGMZ.Idle.Variable, 0);
};
//=============================================================================
// CGMZ_Temp
//-----------------------------------------------------------------------------
// Add idle features
//=============================================================================
//-----------------------------------------------------------------------------
// Also set initial idleStatus flag to false
//-----------------------------------------------------------------------------
const alias_CGMZ_Idle_CGMZ_Temp_createPluginData = CGMZ_Temp.prototype.createPluginData;
CGMZ_Temp.prototype.createPluginData = function() {
	alias_CGMZ_Idle_CGMZ_Temp_createPluginData.call(this);
	this._idleStatus = false;
};
//-----------------------------------------------------------------------------
// Check if the player is idle
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.isIdle = function() {
	return this._idleCounter > CGMZ.Idle.Frames;
};
//-----------------------------------------------------------------------------
// Get amount of Idle Frames
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.getIdleFrames = function() {
	return this._idleCounter - CGMZ.Idle.Frames;
};
//-----------------------------------------------------------------------------
// Handle updating idle status flag changes, variable processing when idle
//-----------------------------------------------------------------------------
const alias_CGMZ_Idle_CGMZ_Temp_updateIdleTimer = CGMZ_Temp.prototype.updateIdleTimer;
CGMZ_Temp.prototype.updateIdleTimer = function() {
	alias_CGMZ_Idle_CGMZ_Temp_updateIdleTimer.call(this);
	if(this.isIdle()) {
		if(!this._idleStatus && this.canEnterIdleState()) {
			this.onIdleStart();
		}
		$gameVariables.setValue(CGMZ.Idle.Variable, this.getIdleFrames());
		$cgmz.totalIdleFrames++;
	} else {
		if(this._idleStatus && this.canExitIdleState()) {
			this.onIdleStop();
		}
	}
};
//-----------------------------------------------------------------------------
// Check if can enter the idle state
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.canEnterIdleState = function() {
	if(CGMZ.Idle.AllowIdleSwitch && !$gameSwitches.value(CGMZ.Idle.AllowIdleSwitch)) return false;
	return true;
};
//-----------------------------------------------------------------------------
// Check if can exit the idle state
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.canExitIdleState = function() {
	if(CGMZ.Idle.AllowIdleSwitch && !$gameSwitches.value(CGMZ.Idle.AllowIdleSwitch)) return false;
	return true;
};
//-----------------------------------------------------------------------------
// Processing when idle status starts
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.onIdleStart = function() {
	this._idleStatus = true;
	if(CGMZ.Idle.IdleStartEvent && this.canStartIdleEvent()) $gameTemp.reserveCommonEvent(CGMZ.Idle.IdleStartEvent);
	if(CGMZ.Idle.BattleIdleStartEvent && SceneManager._scene?.constructor.name === 'Scene_Battle') $gameTemp.reserveCommonEvent(CGMZ.Idle.BattleIdleStartEvent);
	$gameSwitches.setValue(CGMZ.Idle.Switch, true);
};
//-----------------------------------------------------------------------------
// Processing when idle status stops
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.onIdleStop = function() {
	this._idleStatus = false;
	if(CGMZ.Idle.IdleEndEvent && this.canStartIdleEvent()) $gameTemp.reserveCommonEvent(CGMZ.Idle.IdleEndEvent);
	if(CGMZ.Idle.BattleIdleEndEvent && SceneManager._scene?.constructor.name === 'Scene_Battle') $gameTemp.reserveCommonEvent(CGMZ.Idle.BattleIdleEndEvent);
	$gameSwitches.setValue(CGMZ.Idle.Switch, false);
	$gameVariables.setValue(CGMZ.Idle.Variable, 0);
};
//-----------------------------------------------------------------------------
// Check if the idle event should be queued
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.canStartIdleEvent = function() {
	if(!CGMZ.Idle.IdleEventOnMapOnly) return true;
	return SceneManager._scene?.constructor.name === 'Scene_Map';
};
//-----------------------------------------------------------------------------
// Register Idle Plugin Commands
//-----------------------------------------------------------------------------
const alias_CGMZ_Idle_CGMZ_Temp_registerPluginCommands = CGMZ_Temp.prototype.registerPluginCommands;
CGMZ_Temp.prototype.registerPluginCommands = function() {
	alias_CGMZ_Idle_CGMZ_Temp_registerPluginCommands.call(this);
	PluginManager.registerCommand("CGMZ_Idle", "Check Idle", this.pluginCommandIdleCheck);
	PluginManager.registerCommand("CGMZ_Idle", "Check Idle Frames", this.pluginCommandIdleCheckFrames);
	PluginManager.registerCommand("CGMZ_Idle", "Force Idle", this.pluginCommandIdleForceIdle);
	PluginManager.registerCommand("CGMZ_Idle", "Clear Idle", this.pluginCommandIdleClearIdle);
	PluginManager.registerCommand("CGMZ_Idle", "Get Total Idle Frames", this.pluginCommandIdleGetTotalIdleFrames);
};
//-----------------------------------------------------------------------------
// Plugin Command - Check Idle Status
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandIdleCheck = function(args) {
	$gameSwitches.setValue(Number(args.Switch), $cgmzTemp.isIdle());
};
//-----------------------------------------------------------------------------
// Plugin Command - Check Idle Frames
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandIdleCheckFrames = function(args) {
	$gameVariables.setValue(Number(args.Variable), $cgmzTemp.getIdleFrames());
};
//-----------------------------------------------------------------------------
// Plugin Command - Force Idle
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandIdleForceIdle = function() {
	if(!$cgmzTemp._idleStatus) {
		$cgmzTemp._idleCounter = CGMZ.Idle.Frames + 1;
		$cgmzTemp.onIdleStart();
	}
};
//-----------------------------------------------------------------------------
// Plugin Command - Clear Idle
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandIdleClearIdle = function() {
	if($cgmzTemp._idleStatus) {
		$cgmzTemp.resetIdleTimer();
		$cgmzTemp.onIdleStop();
	}
};
//-----------------------------------------------------------------------------
// Plugin Command - Get Total Idle Frames
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandIdleGetTotalIdleFrames = function(args) {
	$gameVariables.setValue(Number(args.Variable), $cgmz.totalIdleFrames);
};
//=============================================================================
// Game_System
//-----------------------------------------------------------------------------
// Reset Idle states before save
//=============================================================================
//-----------------------------------------------------------------------------
// Also reset idle variable/switch before save
//-----------------------------------------------------------------------------
const alias_CGMZ_Idle_GameSystem_onBeforeSave = Game_System.prototype.onBeforeSave;
Game_System.prototype.onBeforeSave = function() {
	alias_CGMZ_Idle_GameSystem_onBeforeSave.call(this);
    $gameSwitches.setValue(CGMZ.Idle.Switch, false);
	$gameVariables.setValue(CGMZ.Idle.Variable, 0);
};