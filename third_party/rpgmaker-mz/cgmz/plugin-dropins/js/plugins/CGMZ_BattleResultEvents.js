/*:
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/battleresultevents/
 * @target MZ
 * @base CGMZ_Core
 * @orderAfter CGMZ_Core
 * @plugindesc Runs a common event after battle
 * @help
 * ============================================================================
 * For terms and conditions using this plugin in your game please visit:
 * https://www.caspergaming.com/terms-of-use/
 * ============================================================================
 * Become a Patron to get access to beta/alpha plugins plus other goodies!
 * https://www.patreon.com/CasperGamingRPGM
 * ============================================================================
 * Version: 1.0.1
 * ----------------------------------------------------------------------------
 * Compatibility: Only tested with my CGMZ plugins.
 * Made for RPG Maker MZ 1.9.0
 * ----------------------------------------------------------------------------
 * Description: Runs a common event after a battle ends. It will also set a
 * game variable to the troop ID faced and the result (win/lose/escape). This
 * will allow you to have conditional event processing for random battles,
 * similar to the battle processing event command.
 * ----------------------------------------------------------------------------
 * Documentation:
 * ----------------------------Result Variable---------------------------------
 * The result variable will be set to 0, 1, or 2 based on result (by default).
 * 0 = win
 * 1 = escape
 * 2 = loss
 * This variable is only set once at the end of a battle, so can be re-used
 * for other temporary things if you wish to cut down on variables used.
 *
 * If it is not being set to these numbers, you have another plugin that
 * changes the default battle end values - consult that plugin to see what
 * to expect for each battle result.
 * ----------------------------Can Lose Switch---------------------------------
 * This switch determines whether the party can continue after losing a random
 * battle instead of getting a game over. It sets this status when the battle
 * begins. If you change this switch value during the battle, this will NOT
 * change whether the party can lose. It will go by whatever the value of the
 * switch was at the start of battle.
 *
 * This switch should not impact evented battles, only random encounters.
 * ----------------------------Plugin Commands---------------------------------
 * This plugin does not use any plugin commands.
 * ------------------------------Saved Games-----------------------------------
 * This plugin fully supports saved games.
 * ✓ You should be able to add this  plugin to a saved game
 * ✓ You can modify parameters and it will reflect accurately in game
 * ✓ You can remove this plugin with no issue to save data
 * --------------------------Latest Version------------------------------------
 * Hi all, this latest version changes the functionality of the Enable Switch
 * parameter slightly. Before, it had to be set or this plugin would do
 * nothing, and the switch also had to be turned ON. This has been changed so
 * that the plugin default is to assume you always want the events to run if
 * you have not set the enable switch parameter up.
 *
 * Version 1.0.1
 * - Enable Switch functionality changed
 *
 * @param Troop ID Variable
 * @type variable
 * @desc The variable ID to store the troop ID in
 * @default 0
 *
 * @param Result Variable
 * @type variable
 * @desc The variable ID to store the result in. See documentation
 * @default 0
 *
 * @param Common Event ID
 * @type common_event
 * @desc The common event to run after battle
 * @default 0
 *
 * @param Win Common Event ID
 * @type common_event
 * @desc The common event to run after battle, if the result is a win
 * @default 0
 *
 * @param Escape Common Event ID
 * @type common_event
 * @desc The common event to run after battle, if the result is an escape
 * @default 0
 *
 * @param Lose Common Event ID
 * @type common_event
 * @desc The common event to run after battle, if the result is a loss
 * @default 0
 *
 * @param Can Lose Switch
 * @type switch
 * @desc Switch that determines if the player can lose random encounters and continue the game (no automatic gameover)
 * @default 0
 *
 * @param Enabled Switch
 * @type switch
 * @desc Switch that controls if the plugin functionality is enabled. ON = events run, OFF = events do not run
 * @default 0
*/
Imported.CGMZ_BattleResultEvents = true;
CGMZ.Versions["Battle Result Events"] = "1.0.1";
CGMZ.BattleResultEvents = {};
CGMZ.BattleResultEvents.parameters = PluginManager.parameters('CGMZ_BattleResultEvents');
CGMZ.BattleResultEvents.TroopIDVariable = Number(CGMZ.BattleResultEvents.parameters["Troop ID Variable"]);
CGMZ.BattleResultEvents.ResultVariable = Number(CGMZ.BattleResultEvents.parameters["Result Variable"]);
CGMZ.BattleResultEvents.CommonEventID = Number(CGMZ.BattleResultEvents.parameters["Common Event ID"]);
CGMZ.BattleResultEvents.WinCommonEventID = Number(CGMZ.BattleResultEvents.parameters["Win Common Event ID"]);
CGMZ.BattleResultEvents.EscapeCommonEventID = Number(CGMZ.BattleResultEvents.parameters["Escape Common Event ID"]);
CGMZ.BattleResultEvents.LoseCommonEventID = Number(CGMZ.BattleResultEvents.parameters["Lose Common Event ID"]);
CGMZ.BattleResultEvents.CanLoseSwitch = Number(CGMZ.BattleResultEvents.parameters["Can Lose Switch"]);
CGMZ.BattleResultEvents.EnabledSwitch = Number(CGMZ.BattleResultEvents.parameters["Enabled Switch"]);
//=============================================================================
// Battle Manager
//-----------------------------------------------------------------------------
// Set variables at battle end and reserve common event
//=============================================================================
//-----------------------------------------------------------------------------
// Check if the can lose switch is ON, if so, allow losing in battle
// This function does not seem to be called on battle processing event command,
// which is how we differentiate between random and pre-determined encounter.
//-----------------------------------------------------------------------------
const alias_CGMZBattleResultEvents_BattleManager_onEncounter = BattleManager.onEncounter;
BattleManager.onEncounter = function() {
	alias_CGMZBattleResultEvents_BattleManager_onEncounter.call(this);
    if(CGMZ.BattleResultEvents.CanLoseSwitch && $gameSwitches.value(CGMZ.BattleResultEvents.CanLoseSwitch)) this._canLose = true;
};
//-----------------------------------------------------------------------------
// Set variables and queue common event at end of battle
//-----------------------------------------------------------------------------
const alias_CGMZBattleResultEvents_BattleManager_endBattle = BattleManager.endBattle;
BattleManager.endBattle = function(result) {
    alias_CGMZBattleResultEvents_BattleManager_endBattle.call(this, result);
	if(!CGMZ.BattleResultEvents.EnabledSwitch || $gameSwitches.value(CGMZ.BattleResultEvents.EnabledSwitch)) {
		$gameVariables.setValue(CGMZ.BattleResultEvents.TroopIDVariable, $gameTroop._troopId);
		$gameVariables.setValue(CGMZ.BattleResultEvents.ResultVariable, result);
		if(CGMZ.BattleResultEvents.CommonEventID) $gameTemp.reserveCommonEvent(CGMZ.BattleResultEvents.CommonEventID);
		switch(result) {
			case 0: if(CGMZ.BattleResultEvents.WinCommonEventID) $gameTemp.reserveCommonEvent(CGMZ.BattleResultEvents.WinCommonEventID); break;
			case 1: if(CGMZ.BattleResultEvents.EscapeCommonEventID) $gameTemp.reserveCommonEvent(CGMZ.BattleResultEvents.EscapeCommonEventID); break;
			case 2: if(CGMZ.BattleResultEvents.LoseCommonEventID) $gameTemp.reserveCommonEvent(CGMZ.BattleResultEvents.LoseCommonEventID); break;
		}
	}
};