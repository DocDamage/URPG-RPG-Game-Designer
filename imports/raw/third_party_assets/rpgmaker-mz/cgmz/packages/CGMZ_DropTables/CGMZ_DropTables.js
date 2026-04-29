/*:
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/droptables/
 * @target MZ
 * @base CGMZ_Core
 * @orderAfter CGMZ_Core
 * @plugindesc Allows the use of drop tables to get random loot
 * @help
 * ============================================================================
 * For terms and conditions using this plugin in your game please visit:
 * https://www.caspergaming.com/terms-of-use/
 * ============================================================================
 * Become a Patron to get access to beta/alpha plugins plus other goodies!
 * https://www.patreon.com/CasperGamingRPGM
 * ============================================================================
 * Version: Beta
 * ----------------------------------------------------------------------------
 * Compatibility: Only tested with my CGMZ plugins.
 * Made for RPG Maker MZ 1.9.0
 * ----------------------------------------------------------------------------
 * Description: Implements a drop table system where you can call a drop
 * table to get a random loot item. For example, you might have a potion drop
 * table with different chances to get a regular potion, a super potion, or an
 * ultra potion. Drop tables can call other drop tables, handle a nothing drop,
 * and gold drops.
 * ----------------------------------------------------------------------------
 * Documentation:
 * --------------------------Drop Table Chances--------------------------------
 * The chances of your drop table along with the Nothing Chance parameter
 * should add up to 100% exactly. This plugin supports up to 2 decimal places
 * for the drop chance of a specific item. If the percentages do not add up to
 * 100%, weird behavior may be experienced such as some items not dropping at
 * all or a nothing drop being more common than expected.
 * ---------------------------Drop Table Items---------------------------------
 * Each drop table item parameter should only have 1 of the possible drop
 * types: item, weapon, armor, gold, drop table. If you want a drop table to
 * have 1 of each type (item, weapon, armor, gold), make 4 different drops for
 * each individual item rather than 1 drop item with all 4 drops.
 * --------------------------Amount and Variance-------------------------------
 * The amount parameter determines the base amount to drop (min 1). The
 * variance parameter will add a random number between 0 and the parameter
 * which is then added to the base amount. If you want 5-7 of an item to drop,
 * you would set the Amount to 5 and the Variance to 2.
 * --------------------------Drop's Text Param-------------------------------
 * The toString parameter for a drop is what will be output to the message
 * window when the object drops. When another drop table drops, the toString
 * of the drop table object will be ignored, and the toString of the dropped
 * item will be used in its place.
 *
 * This param supports some special text codes:
 * %amt - Will be replaced by the amount that dropped
 * %name - Will be replaced by the item/armor/weapon/currency name
 * %icon - Will be replaced by the item/wep/armor/currency icon
 * %cunit - Will be replaced by the currency unit
 * %ccolor - Will be replaced by the currency color (CGMZ CurrencySystem only)
 * -------------------------------Notetags-------------------------------------
 * This plugin supports the following notetags:
 *
 * In the enemy notebox:
 * <cgmzdroptable:id>
 *
 * For example, if you had a drop table with id "potions" then your note tag
 * would look like:
 * <cgmzdroptable:potions>
 * You can also chain ids together with commas separating them. If you wanted
 * two drops from the potions table, you could make the notetag:
 * <cgmzdroptable:potions,potions>
 * -------------------------------Variables------------------------------------
 * The type variable will be set to one of the following numbers when a drop
 * is generated:
 *
 * 0 = Nothing dropped
 * 1 = Item dropped
 * 2 = Weapon dropped
 * 3 = Armor dropped
 * 4 = Gold dropped (or [CGMZ] Custom Currency)
 *
 * The amount variable will be set to the amount that dropped.
 *
 * The ID variable will be set to the item/weapon/armor id. If gold, it will
 * be set to a string that is the currency id (if using [CGMZ] Currency System)
 * and will be set to 0 when nothing drops. You can compare strings by using
 * script: $gameVariables.value(1) === 'myString';
 * -----------------------------Integrations-----------------------------------
 * This plugin has special functions when used with other CGMZ Plugins:
 *
 * • [CGMZ] Currency System 
 * A drop table can drop Currency System currencies. Use the currency ID param
 * to have the drop table drop currency with the ID from the currency system.
 *
 * • [CGMZ] Toast Manager
 * Can display the drop message as a toast instead of adding to the game
 * message window. First, create a preset in the Toast Manager and then type
 * your preset id in to this plugin. You can use %txt to show the drop's text
 * string in either line of your toast preset.
 * ----------------------------------FAQ---------------------------------------
 * Q: I added 5 items with 25% chance to drop each but I only ever get the
 *    first four in the drop table?
 *
 * A: The first 4 items add up to 100%, so the 5th item will never be possible
 *    to drop. If you want each item to have an equal chance and have 5 items,
 *    set to 20% chance each.
 * ------------------------------Saved Games-----------------------------------
 * This plugin is fully compatible with saved games. This means you can:
 *
 * ✓ Add this plugin to a saved game and it will work as expected
 * ✓ Change any plugin params and changes will be reflected in saved games
 * ✓ Remove the plugin with no issue to save data
 * -----------------------------Filename---------------------------------------
 * The filename for this plugin MUST remain CGMZ_DropTables.js
 * This is what it comes as when downloaded. The filename is used to load
 * parameters and execute plugin commands. If you change it, things will begin
 * behaving incorrectly and your game will probably crash. Please do not
 * rename the js file.
 * -----------------------------Latest Version---------------------------------
 * Hi all, welcome to the Beta release of this plugin! If you are new here,
 * this plugin allows you to generate a random drop from a table of possible
 * drops, which can include currency, items, weapons, or armors. Drop tables
 * can also drop something from another drop table. This works both in battle
 * as well as via plugin command for things like treasure chest loot.
 *
 * For existing users, this update changes the Drop Toast parameter to use the
 * new toast manager preset id system. In your toast presets, you can use the
 * string %txt to show the drop's text string. Since presets are more flexible,
 * this update also now allows you to override the default drop toast with a
 * specific toast id in your plugin commands, in case you want to use a special
 * toast for certain plugin command calls.
 *
 * Beta Release
 * - Added toast override for plugin command
 * - Updated toast integration
 *
 * @command Generate Drop
 * @desc Generates an item from specified drop table.
 *
 * @arg id
 * @desc The id of the drop table to generate a drop from.
 *
 * @arg toast id
 * @desc A [CGMZ] Toast Manager preset id to use, overrides plugin default drop toast id
 *
 * @param Drop Table Setup
 *
 * @param Drop Tables
 * @parent Drop Table Setup
 * @type struct<DropTable>[]
 * @default []
 * @desc Set up drop tables here
 *
 * @param Message Options
 *
 * @param Nothing Message
 * @parent Message Options
 * @default Nothing dropped!
 * @desc Text to send to the game message window when nothing is dropped.
 *
 * @param Show Game Message
 * @parent Message Options
 * @type boolean
 * @default true
 * @desc If true, show drop message via game message window
 *
 * @param Mechanics
 *
 * @param Type Variable
 * @parent Mechanics
 * @type variable
 * @default 0
 * @desc A variable ID that will be set to the last drop's type. See documentation.
 *
 * @param ID Variable
 * @parent Mechanics
 * @type variable
 * @default 0
 * @desc A variable ID that will be set to the last drop's item/weapon/armor ID.
 *
 * @param Amount Variable
 * @parent Mechanics
 * @type variable
 * @default 0
 * @desc A variable ID that will be set to the last drop's amount
 *
 * @param Integrations
 *
 * @param Drop Toast ID
 * @parent Integrations
 * @desc [CGMZ] Toast Window preset shown when a drop occurs
*/
/*~struct~DropTable:
 * @param Id
 * @desc The unique id of the drop table. Does not need to be a number, just unique.
 *
 * @param Nothing Chance
 * @type number
 * @default 0.00
 * @decimals 2
 * @min 0.00
 * @max 100.00
 * @desc The odds (0-100) of getting nothing from this table
 * 
 * @param Drops
 * @type struct<Drop>[]
 * @default []
 * @desc Set up individual drops here
*/
/*~struct~Drop:
 * @param Chance
 * @type number
 * @default 0.00
 * @decimals 2
 * @min 0.00
 * @max 100.00
 * @desc The odds (0-100) of getting this drop
 * 
 * @param Amount
 * @type number
 * @default 1
 * @min 1
 * @desc The amount of items to drop
 * 
 * @param Variance
 * @type number
 * @default 0
 * @min 0
 * @desc The random amount to add to the Amount parameter.
 * 
 * @param Item
 * @type item
 * @default 0
 * @desc Item ID.
 * 
 * @param Weapon
 * @type weapon
 * @default 0
 * @desc Weapon ID.
 * 
 * @param Armor
 * @type armor
 * @default 0
 * @desc Armor ID.
 * 
 * @param Gold
 * @type number
 * @default 0
 * @desc Set to non-zero if currency drop.
 * 
 * @param Currency ID
 * @desc Currency ID of gold type (if using CGMZ Currency System, leave blank otherwise).
 * 
 * @param Table
 * @desc Other Drop Table ID. If using this, do not use other drop types.
 * 
 * @param Txt
 * @default Received \c[1]%amtx\c[0] %name!
 * @desc String representation of this drop. See documentation.
*/
Imported.CGMZ_DropTables = true;
CGMZ.Versions["Drop Tables"] = "Beta";
CGMZ.DropTables = {};
CGMZ.DropTables.parameters = PluginManager.parameters('CGMZ_DropTables');
CGMZ.DropTables.NothingMsg = CGMZ.DropTables.parameters["Nothing Message"];
CGMZ.DropTables.DropToastID = CGMZ.DropTables.parameters["Drop Toast ID"];
CGMZ.DropTables.TypeVariable = Number(CGMZ.DropTables.parameters["Type Variable"]);
CGMZ.DropTables.IDVariable = Number(CGMZ.DropTables.parameters["ID Variable"]);
CGMZ.DropTables.AmountVariable = Number(CGMZ.DropTables.parameters["Amount Variable"]);
CGMZ.DropTables.ShowGameMessage = (CGMZ.DropTables.parameters["Show Game Message"] === 'true');
CGMZ.DropTables.Tables = CGMZ_Utils.parseJSON(CGMZ.DropTables.parameters["Drop Tables"], [], "[CGMZ] Drop Tables", "Your Drop Tables parameter was set up incorrectly and could not be read.");
//=============================================================================
// CGMZ_DropTable
//-----------------------------------------------------------------------------
// Temp data class used to track drop table properties
//=============================================================================
function CGMZ_DropTable() {
	this.initialize(...arguments);
}
//-----------------------------------------------------------------------------
// Initialize
//-----------------------------------------------------------------------------
CGMZ_DropTable.prototype.initialize = function(table) {
	this._id = table.Id;
	this._drops = this.setupDrops(CGMZ_Utils.parseJSON(table.Drops, [], "[CGMZ] Drop Tables", `Your Drops parameter in table ${this._id} was set up incorrectly and could not be read.`));
};
//-----------------------------------------------------------------------------
// Setup drops
//-----------------------------------------------------------------------------
CGMZ_DropTable.prototype.setupDrops = function(dropArray) {
	const drops = [];
	for(const dropJSON of dropArray) {
		const dropParse = CGMZ_Utils.parseJSON(dropJSON, null, "[CGMZ] Drop Tables", `One of your drops in table ${this._id} was set up incorrectly and could not be read.`);
		if(!dropParse) continue;
		const drop = new CGMZ_DropTableDrop(dropParse);
		drops.push(drop);
	}
	return drops;
};
//-----------------------------------------------------------------------------
// Generate a drop object
//-----------------------------------------------------------------------------
CGMZ_DropTable.prototype.makeDrop = function() {
	let dropObj = {amount: 0, dropItem: null};
	const roll = Math.random() * 100.00;
	let chance = 0.00
	for(const drop of this._drops) {
		chance += drop._chance;
		if(chance >= roll) {
			if(drop._drop.type === 'table') {
				dropObj = $cgmzTemp.getDropTableDrop(drop._drop.id);
			} else {
				dropObj.amount = drop._amount + Math.randomInt(drop._variance + 1);
				dropObj.dropItem = drop._drop;
			}
			break;
		}
	}
	this.setDropVariables(dropObj);
	return dropObj;
};
//-----------------------------------------------------------------------------
// Set variables for dropped object
//-----------------------------------------------------------------------------
CGMZ_DropTable.prototype.setDropVariables = function(dropObj) {
	if(dropObj.amount > 0) {
		const item = dropObj.dropItem;
		switch(item.type) {
			case 'item': $gameVariables.setValue(CGMZ.DropTables.TypeVariable, 1); break;
			case 'weapon': $gameVariables.setValue(CGMZ.DropTables.TypeVariable, 2); break;
			case 'armor': $gameVariables.setValue(CGMZ.DropTables.TypeVariable, 3); break;
			case 'gold': $gameVariables.setValue(CGMZ.DropTables.TypeVariable, 4); break;
		}
		$gameVariables.setValue(CGMZ.DropTables.IDVariable, item.id);
	} else {
		$gameVariables.setValue(CGMZ.DropTables.TypeVariable, 0);
		$gameVariables.setValue(CGMZ.DropTables.IDVariable, 0);
	}
	$gameVariables.setValue(CGMZ.DropTables.AmountVariable, dropObj.amount);
};
//=============================================================================
// CGMZ_DropTableDrop
//-----------------------------------------------------------------------------
// Temp data class used to track drop table drops
//=============================================================================
function CGMZ_DropTableDrop() {
	this.initialize(...arguments);
}
//-----------------------------------------------------------------------------
// Initialize
//-----------------------------------------------------------------------------
CGMZ_DropTableDrop.prototype.initialize = function(drop) {
	this._chance = parseFloat(drop.Chance);
	this._amount = Number(drop.Amount);
	this._variance = Number(drop.Variance);
	this._drop = {};
	if(drop.Item !== "0") {
		this._drop.id = Number(drop.Item);
		this._drop.type = "item";
	}
	if(drop.Weapon !== "0") {
		this._drop.id = Number(drop.Weapon);
		this._drop.type = "weapon";
	}
	if(drop.Armor !== "0") {
		this._drop.id = Number(drop.Armor);
		this._drop.type = "armor";
	}
	if(drop.Gold !== "0") {
		this._drop.id = drop["Currency ID"];
		this._drop.type = "gold";
	}
	if(drop.Table) {
		this._drop.id = drop.Table;
		this._drop.type = "table";
	}
	this._drop.string = drop.Txt;
};
//=============================================================================
// CGMZ_Temp
//-----------------------------------------------------------------------------
// Add temp drop table data and plugin commands
//=============================================================================
//-----------------------------------------------------------------------------
// Also initialize drop table data
//-----------------------------------------------------------------------------
const alias_CGMZ_DropTables_CGMZ_Temp_createPluginData = CGMZ_Temp.prototype.createPluginData;
CGMZ_Temp.prototype.createPluginData = function() {
	alias_CGMZ_DropTables_CGMZ_Temp_createPluginData.call(this);
	this.initializeDropTableData();
};
//-----------------------------------------------------------------------------
// Initialize drop table data
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.initializeDropTableData = function() {
	this._dropTables = {};
	for(const tableJSON of CGMZ.DropTables.Tables) {
		const tableObj = CGMZ_Utils.parseJSON(tableJSON, null, "[CGMZ] Drop Tables", "One of your drop tables was set up incorrectly and could not be read.");
		if(!tableObj) continue;
		this._dropTables[tableObj.Id] = new CGMZ_DropTable(tableObj);
	}
	this._tmpDropTableCurrencies = [];
};
//-----------------------------------------------------------------------------
// Get drop table data
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.getDropTable = function(id) {
	return this._dropTables[id];
};
//-----------------------------------------------------------------------------
// Get a drop object from a drop table
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.getDropTableDrop = function(id) {
	const table = this.getDropTable(id);
	return table.makeDrop();
};
//-----------------------------------------------------------------------------
// Add a temporary tracker for drop table currencies
// Used to reward extra currency from enemy drops
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.addTmpDropTableCurrency = function(id, amount) {
	const drop = {id:id,amount:amount};
	this._tmpDropTableCurrencies.push(drop);
};
//-----------------------------------------------------------------------------
// Get the temporary tracker for drop table currencies
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.tmpDropTableCurrencyTracker = function() {
	return this._tmpDropTableCurrencies;
};
//-----------------------------------------------------------------------------
// Clear the temporary tracker for drop table currencies
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.clearTmpDropTableCurrency = function() {
	this._tmpDropTableCurrencies = [];
};
//-----------------------------------------------------------------------------
// Show drop table toast
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.showDropTableToast = function(txt, toastId = "") {
	if(!Imported.CGMZ_ToastManager) return;
	const toast = this.getToastObjectFromPreset(toastId || CGMZ.DropTables.DropToastID);
	if(!toast) return;
	toast.lineOne = toast.lineOne.replace('%txt', txt);
	toast.lineTwo = toast.lineTwo.replace('%txt', txt);
	this.createNewToast(toast);
};
//-----------------------------------------------------------------------------
// Register drop table Plugin Commands
//-----------------------------------------------------------------------------
const alias_CGMZ_DropTables_CGMZ_Temp_registerPluginCommands = CGMZ_Temp.prototype.registerPluginCommands;
CGMZ_Temp.prototype.registerPluginCommands = function() {
	alias_CGMZ_DropTables_CGMZ_Temp_registerPluginCommands.call(this);
	PluginManager.registerCommand("CGMZ_DropTables", "Generate Drop", this.pluginCommandDropTableGenerateDrop);
};
//-----------------------------------------------------------------------------
// Plugin Command - Generate Drop
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandDropTableGenerateDrop = function(args) {
	const drop = $cgmzTemp.getDropTableDrop(args.id);
	if(drop.amount > 0) {
		const txt = $cgmzTemp.makeDropTableItemText(drop.dropItem, drop.amount);
		if(CGMZ.DropTables.ShowGameMessage) $gameMessage.add(txt);
		$cgmzTemp.showDropTableToast(txt, args["toast id"]);
		$cgmzTemp.awardDropTableDrop(drop);
	} else {
		if(CGMZ.DropTables.ShowGameMessage) $gameMessage.add(CGMZ.DropTables.NothingMsg);
		$cgmzTemp.showDropTableToast(CGMZ.DropTables.NothingMsg, args["toast id"]);
	}
};
//-----------------------------------------------------------------------------
// Give the player a drop table reward
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.awardDropTableDrop = function(drop) {
	if(drop.amount > 0) {
		switch(drop.dropItem.type) {
			case "item": 
			case "weapon": 
			case "armor": $gameParty.gainItem(CGMZ_Utils.lookupItem(drop.dropItem.type, drop.dropItem.id), drop.amount, false); break;
			case "gold": 
				if(Imported.CGMZ_CurrencySystem) {
					const currency = $cgmz.getCurrency(drop.dropItem.id);
					(currency) ? currency.gainCurrency(drop.amount) : $gameParty.gainGold(drop.amount);
				} else {
					$gameParty.gainGold(drop.amount);
				}
				break;
		}
	}
};
//-----------------------------------------------------------------------------
// Create text for drop item
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.makeDropTableItemText = function(item, amount) {
	let string = item.string.replace(/%amt/g, amount);
	switch(item.type) {
		case "item": string = string.replace(/%name/g, $dataItems[item.id].name).replace(/%icon/g, `\\i[${$dataItems[item.id].iconIndex}]`); break;
		case "weapon": string = string.replace(/%name/g, $dataWeapons[item.id].name).replace(/%icon/g, `\\i[${$dataWeapons[item.id].iconIndex}]`); break;
		case "armor": string = string.replace(/%name/g, $dataArmors[item.id].name).replace(/%icon/g, `\\i[${$dataArmors[item.id].iconIndex}]`); break;
		case "gold": 
			if(Imported.CGMZ_CurrencySystem) {
				const currency = $cgmzTemp.getCurrency(item.id);
				if(currency) {
					string = string.replace(/%cunit/g, currency._unit).replace(/%ccolor/g, `\\c[${currency._color}]`).replace(/%icon/g, `\\i[${currency._iconIndex}]`).replace(/%name/g, currency._name)
				} else {
					string = string.replace(/%cunit/g, TextManager.currencyUnit).replace(/%ccolor/g, `\\c[${CGMZ.CurrencySystem.DefaultColor}]`).replace(/%icon/g, `\\i[${CGMZ.CurrencySystem.DefaultIconIndex}]`).replace(/%name/g, CGMZ.CurrencySystem.DefaultName);
				}
			} else {
				string = string.replace(/%cunit/g, TextManager.currencyUnit);
			}
	}
	return string;
};
//=============================================================================
// Game_Enemy
//-----------------------------------------------------------------------------
// Add drop table drop items to enemy drops
//=============================================================================
//-----------------------------------------------------------------------------
// Add drop table drops to return. If currency drop, add to cgmz temp drop tracker.
//-----------------------------------------------------------------------------
const alias_CGMZ_DropTables_GameEnemy_makeDropItems = Game_Enemy.prototype.makeDropItems;
Game_Enemy.prototype.makeDropItems = function() {
	const oldReturn = alias_CGMZ_DropTables_GameEnemy_makeDropItems.apply(this, arguments);
	const enemyData = this.enemy();
	if(enemyData.meta?.cgmzdroptable) {
		const dropTableIds = enemyData.meta.cgmzdroptable.split(",");
		for(const dropTableId of dropTableIds) {
			const drop = $cgmzTemp.getDropTableDrop(dropTableId);
			if(drop.amount > 0) {
				switch(drop.dropItem.type) {
					case "item": 
					case "weapon": 
					case "armor":
						for(let i = 0; i < drop.amount; i++) {
							oldReturn.push(CGMZ_Utils.lookupItem(drop.dropItem.type, drop.dropItem.id));
						}
						break;
					case "gold": $cgmzTemp.addTmpDropTableCurrency(drop.dropItem.id, drop.amount);
				}
			}
		}
	}
	return oldReturn;
};
//=============================================================================
// BattleManager
//-----------------------------------------------------------------------------
// Check if any extra gold drop rewards, and add to gold reward total
//=============================================================================
//-----------------------------------------------------------------------------
// Add drop table gold drops to the gold total
//-----------------------------------------------------------------------------
const alias_CGMZ_DropTables_BattleManager_makeRewards = BattleManager.makeRewards;
BattleManager.makeRewards = function() {
	alias_CGMZ_DropTables_BattleManager_makeRewards.apply(this, arguments);
	for(const currencyDrop of $cgmzTemp.tmpDropTableCurrencyTracker()) {
		if(currencyDrop.id && Imported.CGMZ_CurrencySystem) {
			const currency = $cgmz.getCurrency(currencyDrop.id);
			if(currency) {
				if(!this._rewards.cgmz_custom_currency) this._rewards.cgmz_custom_currency = {};
				if(!this._rewards.cgmz_custom_currency[currencyDrop.id]) this._rewards.cgmz_custom_currency[currencyDrop.id] = 0;
				this._rewards.cgmz_custom_currency[currencyDrop.id] += currencyDrop.amount;
			} else {
				this._rewards.gold += currencyDrop.amount;
			}
		} else {
			this._rewards.gold += currencyDrop.amount;
		}
	}
	$cgmzTemp.clearTmpDropTableCurrency();
};