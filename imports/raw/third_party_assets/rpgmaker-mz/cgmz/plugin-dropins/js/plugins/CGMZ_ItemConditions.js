/*:
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/itemconditions/
 * @target MZ
 * @plugindesc Add conditions to use items, such as only in dungeons for an escape rope
 * battles
 * @help
 * ============================================================================
 * For terms and conditions using this plugin in your game please visit:
 * https://www.caspergaming.com/terms-of-use/
 * ============================================================================
 * Become a Patron to get access to beta/alpha plugins plus other goodies!
 * https://www.patreon.com/CasperGamingRPGM
 * ============================================================================
 * Version: 1.3.0
 * ----------------------------------------------------------------------------
 * Compatibility: Only tested with my CGMZ plugins.
 * Made for RPG Maker MZ 1.9.0
 * ----------------------------------------------------------------------------
 * Description: This plugin allows you to restrict the player from using
 * certain items in certain maps or against certain troops. For example, an
 * escape rope item that only works in your dungeon maps or an item that only
 * works in a boss battle.
 * ----------------------------------------------------------------------------
 * Documentation:
 * Item restrictions will be applied by troop ID (if in battle) or by map ID
 * (if not in battle). Profession restrictions are always applied. Switch and
 * variable conditions are always applied.
 *
 * No restrictions set up in this plugin will be applied if the "Lift
 * Restrictions" switch is set to ON.
 * -------------------------Plugin Commands------------------------------------
 * This plugin does not have any plugin commands.
 * ------------------------------Saved Games-----------------------------------
 * This plugin is fully compatible with saved games. This means you can:
 *
 * ✓ Add this plugin to a saved game and it will work as expected
 * ✓ Change any plugin params and changes will be reflected in saved games
 * ✓ Remove the plugin with no issue to save data
 * -----------------------------Filename---------------------------------------
 * The filename for this plugin MUST remain CGMZ_ItemConditions.js
 * This is what it comes as when downloaded. The filename is used to load
 * parameters and execute plugin commands. If you change it, things will begin
 * behaving incorrectly and your game will probably crash. Please do not
 * rename the js file.
 * --------------------------Latest Version------------------------------------
 * Hi all, this latest version adds actor conditions, so only certain actors
 * can use certain items. These actor conditions only affect battle, as you do
 * not select an actor before using items in the menu. The default is that
 * actors can use items, and any listed actor id is prevented from using the
 * item.
 *
 * Version 1.3.0
 * - Added actor condition
 *
 * @param Item Restrictions
 *
 * @param Items
 * @parent Item Restrictions
 * @type struct<ItemRestriction>[]
 * @default []
 * @desc Set up item restrictions here
 *
 * @param Lift Item Restriction Switch
 * @parent Item Restrictions
 * @type switch
 * @default 0
 * @desc When this switch is ON, all item restrictions will be lifted. When OFF, restrictions apply.
*/
/*~struct~ItemRestriction:
 * @param Item
 * @type item
 * @default 0
 * @desc The item to add item restrictions to
 * 
 * @param Restricted Maps
 * @type map[]
 * @default []
 * @desc The map IDs to restrict the item from use
 * 
 * @param Restricted Troops
 * @type troop[]
 * @default []
 * @desc The troop IDs to restrict the item from use
 * 
 * @param Profession Restrictions
 * @type struct<Profession>[]
 * @default []
 * @desc Professions and required level to use the item
 * 
 * @param Switches
 * @type switch[]
 * @default []
 * @desc Switch IDs that must be ON before the item will be usable.
 * 
 * @param Variables
 * @type variable[]
 * @default []
 * @desc Variable IDs that must be >= 1 before the item will be usable.
 * 
 * @param Actors
 * @type actor[]
 * @default []
 * @desc Actor IDs that cannot use the item, only affects battle
*/
/*~struct~Profession:
 * @param Name
 * @desc The name of the profession
 * 
 * @param Actor
 * @type actor
 * @default 0
 * @desc The actor the profession belongs to. Leave as 0 for party-wide profession
 * 
 * @param Level
 * @type number
 * @min 1
 * @default 1
 * @desc The minimum level required
*/
/*:zh-CN
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/itemconditions/
 * @target MZ
 * @plugindesc 物品和技能的使用限制插件
 * @help
 * ============================================================================
 * 【使用条款】
 * 1、本插件可作商用或非商用。
 * 2、须注明插件作者"Casper Gaming"。
 * 3、须提供该插件的作者网站链接。
 * 4、最终使用条款以作者官网公告为准。https://www.caspergaming.com/terms-of-use/
 * ============================================================================
 * 【赞助支持】
 * 您可以登陆以下网站并对作者进行支持和赞助。
 * 然后获得作者和其插件的最新资讯，以及测试版插件的试用。
 * https://www.patreon.com/CasperGamingRPGM
 * ============================================================================
 * 【插件版本】 V 1.3.0
 * ----------------------------------------------------------------------------
 * 【兼容性】仅测试作者所制作的插件
 * 【RM版本】RPG Maker MZ 1.9.0
 * ----------------------------------------------------------------------------
 * 【插件描述】可以限制指定的技能和物品在某个地图或战斗中无法使用。
 * ----------------------------------------------------------------------------
 * 【使用说明】
 * 指定某些技能和物品，使其在指定的地图中，或与指定的敌群战斗时，无法使用。
 * 设置开关，然后可以在游戏中通过开关解除或重新激活这些限制。
 *
 * Profession restrictions are applied in both maps and battle.
 * -------------------------Plugin Commands------------------------------------
 * This plugin does not have any plugin commands.
 * ------------------------------Saved Games-----------------------------------
 * This plugin is fully compatible with saved games. This means you can:
 *
 * ✓ Add this plugin to a saved game and it will work as expected
 * ✓ Change any plugin params and changes will be reflected in saved games
 * ✓ Remove the plugin with no issue to save data
 * -----------------------------Filename---------------------------------------
 * The filename for this plugin MUST remain CGMZ_ItemConditions.js
 * This is what it comes as when downloaded. The filename is used to load
 * parameters and execute plugin commands. If you change it, things will begin
 * behaving incorrectly and your game will probably crash. Please do not
 * rename the js file.
 * --------------------------Latest Version------------------------------------
 * Hi all, this latest version adds actor conditions, so only certain actors
 * can use certain items. These actor conditions only affect battle, as you do
 * not select an actor before using items in the menu. The default is that
 * actors can use items, and any listed actor id is prevented from using the
 * item.
 *
 * Version 1.3.0
 * - Added actor condition
 *
 * @param Item Restrictions
 * @text 物品限制设置
 * @default 关于物品限制的参数设置
 *
 * @param Items
 * @text 物品限制
 * @parent Item Restrictions
 * @type struct<ItemRestriction>[]
 * @default []
 * @desc 设置物品限制
 *
 * @param Lift Item Restriction Switch
 * @text 解除物品限制的开关
 * @parent Item Restrictions
 * @type switch
 * @default 0
 * @desc 指定一个开关，当其打开时(ON)会解除所有物品限制，当其关闭时(OFF)会重新激活所有限制。
*/
/*~struct~ItemRestriction:zh-CN
 * @param Item
 * @text 物品
 * @type item
 * @default 0
 * @desc  设置限制使用的物品ID
 * 
 * @param Restricted Maps
 * @text 地图限制
 * @type map[]
 * @default []
 * @desc 指定一个地图ID，使队伍在该地图中时不能使用这个物品
 * 
 * @param Restricted Troops
 * @text 战斗限制
 * @type troop[]
 * @default []
 * @desc 指定一个敌群ID，使队伍与该敌群战斗时不能使用这个物品
 * 
 * @param Profession Restrictions
 * @type struct<Profession>[]
 * @default []
 * @desc Professions and required level to use the item
 * 
 * @param Switches
 * @type switch[]
 * @default []
 * @desc Switch IDs that must be ON before the item will be usable.
 * 
 * @param Variables
 * @type variable[]
 * @default []
 * @desc Variable IDs that must be >= 1 before the item will be usable.
 * 
 * @param Actors
 * @type actor[]
 * @default []
 * @desc Actor IDs that cannot use the item, only affects battle
*/
/*~struct~Profession:zh-CN
 * @param Name
 * @desc The name of the profession
 * 
 * @param Actor
 * @type actor
 * @default 0
 * @desc The actor the profession belongs to. Leave as 0 for party-wide profession
 * 
 * @param Level
 * @type number
 * @min 1
 * @default 1
 * @desc The minimum level required
*/
/*:es
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/itemconditions/
 * @target MZ
 * @plugindesc Restringe el uso de elementos y habilidades en ciertos mapas
 * battles
 * @help
 * ============================================================================
 * Para términos y condiciones de uso de este pluging en tu juego, por favor
 * visita:
 * https://www.caspergaming.com/terms-of-use/
 * ============================================================================
 * ¡Conviértete en un Patrocinador para obtener acceso a los plugings beta y
 * alfa, ademas de otras cosas geniales!
 * https://www.patreon.com/CasperGamingRPGM
 * ============================================================================
 * Versión: 1.3.0
 * ----------------------------------------------------------------------------
 * Compatibilidad: Sólo probado con mis CGMZ plugins.
 * Hecho para RPG Maker MZ 1.9.0
 * ----------------------------------------------------------------------------
 * Descripción: este complemento le permite restringir al jugador el uso de 
 * ciertos elementos o habilidades en ciertos mapas o contra ciertas tropas. 
 * Por ejemplo, un elemento de cuerda de escape que solo funciona en tus mapas 
 * de mazmorras o un elemento que solo funciona en una batalla de jefes.
 * ----------------------------------------------------------------------------
 * Documentación:
 * Las restricciones de artículos/habilidades se aplicarán por ID de tropa (si 
 * está en batalla) o por ID del mapa (si no está en batalla).
 *
 * No se aplicarán restricciones configuradas en este complemento si el
 * respectivo El interruptor "Restricciones de elevación" está activado.
 *
 * Profession restrictions are applied in both maps and battle.
 * -------------------------Plugin Commands------------------------------------
 * This plugin does not have any plugin commands.
 * ------------------------------Saved Games-----------------------------------
 * This plugin is fully compatible with saved games. This means you can:
 *
 * ✓ Add this plugin to a saved game and it will work as expected
 * ✓ Change any plugin params and changes will be reflected in saved games
 * ✓ Remove the plugin with no issue to save data
 * -----------------------------Filename---------------------------------------
 * The filename for this plugin MUST remain CGMZ_ItemConditions.js
 * This is what it comes as when downloaded. The filename is used to load
 * parameters and execute plugin commands. If you change it, things will begin
 * behaving incorrectly and your game will probably crash. Please do not
 * rename the js file.
 * --------------------------Latest Version------------------------------------
 * Hi all, this latest version adds actor conditions, so only certain actors
 * can use certain items. These actor conditions only affect battle, as you do
 * not select an actor before using items in the menu. The default is that
 * actors can use items, and any listed actor id is prevented from using the
 * item.
 *
 * Version 1.3.0
 * - Added actor condition
 *
 * @param Item Restrictions
 * @text Restricciones de artículos
 *
 * @param Items
 * @text Artículos
 * @parent Item Restrictions
 * @type struct<ItemRestriction>[]
 * @default []
 * @desc Configure las restricciones de artículos aquí.
 *
 * @param Lift Item Restriction Switch
 * @text Interruptor de restricción de elevación de elementos
 * @parent Item Restrictions
 * @type switch
 * @default 0
 * @desc Cuando este interruptor está en ON, se eliminarán todas las restricciones de artículos. Cuando está APAGADO, se aplican restricciones.
*/
/*~struct~ItemRestriction:es
 * @param Item
 * @text Artículo
 * @type item
 * @default 0
 * @desc El elemento a la cual agregar restricciones de elementos.
 * 
 * @param Restricted Maps
 * @text Mapas restringidos
 * @type map[]
 * @default []
 * @desc Los ID del mapa para restringir el uso del artículo.
 * 
 * @param Restricted Troops
 * @text Tropas restringidas
 * @type troop[]
 * @default []
 * @desc Las ID de tropa para restringir el uso del artículo.
 * 
 * @param Profession Restrictions
 * @type struct<Profession>[]
 * @default []
 * @desc Professions and required level to use the item
 * 
 * @param Switches
 * @type switch[]
 * @default []
 * @desc Switch IDs that must be ON before the item will be usable.
 * 
 * @param Variables
 * @type variable[]
 * @default []
 * @desc Variable IDs that must be >= 1 before the item will be usable.
 * 
 * @param Actors
 * @type actor[]
 * @default []
 * @desc Actor IDs that cannot use the item, only affects battle
*/
/*~struct~Profession:es
 * @param Name
 * @desc The name of the profession
 * 
 * @param Actor
 * @type actor
 * @default 0
 * @desc The actor the profession belongs to. Leave as 0 for party-wide profession
 * 
 * @param Level
 * @type number
 * @min 1
 * @default 1
 * @desc The minimum level required
*/
Imported.CGMZ_ItemConditions = true;
CGMZ.Versions["Item Conditions"] = "1.3.0";
CGMZ.ItemConditions = {};
CGMZ.ItemConditions.parameters = PluginManager.parameters('CGMZ_ItemConditions');
CGMZ.ItemConditions.ItemSwitchLiftRestrictions = Number(CGMZ.ItemConditions.parameters["Lift Item Restriction Switch"]);
CGMZ.ItemConditions.ItemRestrictions = CGMZ_Utils.parseJSON(CGMZ.ItemConditions.parameters["Items"], [], "[CGMZ] Item Conditions", "Your Item restrictions are set up incorrectly, no item restrictions could be loaded.");
//=============================================================================
// CGMZ_Data_ItemCondition
//-----------------------------------------------------------------------------
// Store and manage item condition data. Not saved.
//=============================================================================
function CGMZ_Data_ItemCondition() {
    this.initialize.apply(this, arguments);
}
//-----------------------------------------------------------------------------
// Initialize
//-----------------------------------------------------------------------------
CGMZ_Data_ItemCondition.prototype.initialize = function(condition) {
	const id = condition.Item;
	this.actors = CGMZ_Utils.parseJSON(condition["Actors"], [], "[CGMZ] Item Conditions", `Invalid JSON in item id: ${id}'s actor restriction parameter.`).map(actorId => Number(actorId));
	this.maps = CGMZ_Utils.parseJSON(condition["Restricted Maps"], [], "[CGMZ] Item Conditions", `Invalid JSON in item id: ${id}'s map restriction parameter.`).map(mapId => Number(mapId));
	this.troops = CGMZ_Utils.parseJSON(condition["Restricted Troops"], [], "[CGMZ] Item Conditions", `Invalid JSON in item id: ${id}'s troop restriction parameter.`).map(troopId => Number(troopId));
	this.switches = CGMZ_Utils.parseJSON(condition["Switches"], [], "[CGMZ] Item Conditions", `Invalid JSON in item id: ${id}'s switch restriction parameter.`).map(switchId => Number(switchId));
	this.variables = CGMZ_Utils.parseJSON(condition["Variables"], [], "[CGMZ] Item Conditions", `Invalid JSON in item id: ${id}'s variable restriction parameter.`).map(varId => Number(varId));
	this.professions = [];
	if(Imported.CGMZ_Professions) {
		const professionsJSON = CGMZ_Utils.parseJSON(condition["Profession Restrictions"], [], "[CGMZ] Item Conditions", `Invalid JSON in item id: ${id}'s profession restriction parameter.`);
		for(const professionJSON of professionsJSON) {
			const profession = CGMZ_Utils.parseJSON(professionJSON, null, "[CGMZ] Item Conditions", `Invalid JSON in profession restriction parameter for item id: ${id}`);
			if(profession) {
				const profObj = {name: profession.Name, actor: Number(profession.Actor), level: Number(profession.Level)};
				this.professions.push(profObj);
			}
		}
	}
};
//=============================================================================
// CGMZ_Temp
//-----------------------------------------------------------------------------
// Adds restriction data to CGMZ Temp
//=============================================================================
//-----------------------------------------------------------------------------
// Add set up item conditions data
//-----------------------------------------------------------------------------
const alias_CGMZItemConditions_CGMZTemp_createPluginData = CGMZ_Temp.prototype.createPluginData
CGMZ_Temp.prototype.createPluginData = function() {
	alias_CGMZItemConditions_CGMZTemp_createPluginData.call(this);
	this._itemRestrictions = {};
	for(const item of CGMZ.ItemConditions.ItemRestrictions) {
		const itemObj = CGMZ_Utils.parseJSON(item, null, "[CGMZ] Item Conditions", `Invalid JSON in item restriction: ${item}`);
		if(!itemObj) continue;
		const id = Number(itemObj.Item);
		this._itemRestrictions[id] = new CGMZ_Data_ItemCondition(itemObj);
	}
};
//-----------------------------------------------------------------------------
// Get restrictions for an item by id
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.getItemCondition = function(id) {
	return this._itemRestrictions[id];
};
//=============================================================================
// Game_BattlerBase
//-----------------------------------------------------------------------------
// Additional checking for correct map to use item
//=============================================================================
//-----------------------------------------------------------------------------
// Check restrictions on the item before allowing use
//-----------------------------------------------------------------------------
const alias_CGMZItemConditions_canUse = Game_BattlerBase.prototype.canUse;
Game_BattlerBase.prototype.canUse = function(item) {
	const origReturn = alias_CGMZItemConditions_canUse.call(this, item);
	if(origReturn && DataManager.isItem(item)) {
		if(!$gameSwitches.value(CGMZ.ItemConditions.ItemSwitchLiftRestrictions)) {
			return this.CGMZ_ItemConditions_checkItemRestrictions(item);
		}
	}
    return origReturn;
};
//-----------------------------------------------------------------------------
// Check items for restrictions
//-----------------------------------------------------------------------------
Game_BattlerBase.prototype.CGMZ_ItemConditions_checkItemRestrictions = function(item) {
	const condition = $cgmzTemp.getItemCondition(item.id);
	if(!condition) return true; // no conditions
	if(!this.CGMZ_ItemConditions_meetsProfessionCondition(condition)) return false;
	if(!this.CGMZ_ItemConditions_meetsSwitchCondition(condition.switches)) return false;
	if(!this.CGMZ_ItemConditions_meetsVariableCondition(condition.variables)) return false;
	if($gameParty.inBattle()) {
		return this.CGMZ_ItemConditions_checkBattleRestrictions(condition);
	}
	return this.CGMZ_ItemConditions_checkMapRestrictions(condition);
};
//-----------------------------------------------------------------------------
// Check battle restrictions
//-----------------------------------------------------------------------------
Game_BattlerBase.prototype.CGMZ_ItemConditions_checkBattleRestrictions = function(condition) {
	if(condition.troops.includes($gameTroop._troopId)) return false;
	const currentActorId = BattleManager.actor()?.actorId();
	if(currentActorId && condition.actors.includes(currentActorId)) return false;
	return true;
};
//-----------------------------------------------------------------------------
// Check map restrictions
//-----------------------------------------------------------------------------
Game_BattlerBase.prototype.CGMZ_ItemConditions_checkMapRestrictions = function(condition) {
	return (!condition.maps.includes($gameMap.mapId()));
};
//-----------------------------------------------------------------------------
// Check profession restrictions
//-----------------------------------------------------------------------------
Game_BattlerBase.prototype.CGMZ_ItemConditions_meetsProfessionCondition = function(condition) {
	for(const professionCondition of condition.professions) {
		const profession = $cgmz.getProfession(professionCondition.name, professionCondition.actor);
		if(!profession) continue;
		if(profession.getBuffedLevel() < professionCondition.level) return false;
	}
	return true;
};
//-----------------------------------------------------------------------------
// Check switch restrictions
//-----------------------------------------------------------------------------
Game_BattlerBase.prototype.CGMZ_ItemConditions_meetsSwitchCondition = function(switchA) {
	for(const switchId of switchA) {
		if(switchId && !$gameSwitches.value(switchId)) return false;
	}
	return true;
};
//-----------------------------------------------------------------------------
// Check variable restrictions
//-----------------------------------------------------------------------------
Game_BattlerBase.prototype.CGMZ_ItemConditions_meetsVariableCondition = function(variableA) {
	for(const varId of variableA) {
		if(varId && $gameVariables.value(varId) < 1) return false;
	}
	return true;
};