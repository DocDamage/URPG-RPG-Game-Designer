/*:
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/skillconditions/
 * @target MZ
 * @plugindesc Adds conditions to when skills can be used
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
 * Description: This plugin allows you to restrict the player from using
 * certain skills in certain maps or against certain troops. For example, your
 * ultimate skill that only functions against the big bad.
 * ----------------------------------------------------------------------------
 * Documentation:
 * Skill restrictions will be applied by troop ID (if in battle) or by map ID
 * (if not in battle). Profession restrictions are always applied.
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
 * The filename for this plugin MUST remain CGMZ_SkillConditions.js
 * This is what it comes as when downloaded. The filename is used to load
 * parameters and execute plugin commands. If you change it, things will begin
 * behaving incorrectly and your game will probably crash. Please do not
 * rename the js file.
 * --------------------------Latest Version------------------------------------
 * Hi all, this latest version adds items, weapons, and armors to the possible
 * skill restrictions. These settings allow you to make sure the player has
 * an item, weapon, or armor in their inventory before a skill can be used.
 * You might use this to make sure they have an Arrow item before they can use
 * a bow skill. Note that these are checked in the inventory only, and do not
 * include equips.
 *
 * This update was a minor re-write of how the plugin works internally to make
 * adding new restrictions and advanced features easier in future updates.
 *
 * Version 1.1.0
 * - Added item, weapon, and armor inventory requirements
 *
 * @param Restriction Setup
 *
 * @param Skills
 * @parent Restriction Setup
 * @type struct<SkillRestriction>[]
 * @default []
 * @desc Set up skill restrictions here
 *
 * @param Mechanics
 *
 * @param Lift Restriction Switch
 * @parent Mechanics
 * @type switch
 * @default 0
 * @desc When this switch is ON, all skill restrictions will be lifted. When OFF, restrictions apply.
*/
/*~struct~SkillRestriction:
 * @param Skill
 * @type skill
 * @default 0
 * @desc The skill to add skill restrictions to
 * 
 * @param Restricted Maps
 * @type map[]
 * @default []
 * @desc The map IDs to restrict the skill from use
 * 
 * @param Restricted Troops
 * @type troop[]
 * @default []
 * @desc The troop IDs to restrict the skill from use
 * 
 * @param Profession Restrictions
 * @type struct<Profession>[]
 * @default []
 * @desc Professions and required level to use the skill
 * 
 * @param Switches
 * @type switch[]
 * @default []
 * @desc Switch IDs that must be ON before the skill will be usable
 * 
 * @param Variables
 * @type variable[]
 * @default []
 * @desc Variable IDs that must be >= 1 before the skill will be usable
 * 
 * @param Items
 * @type item[]
 * @default []
 * @desc Item IDs that the party must have >= 1 before the skill will be usable
 * 
 * @param Armors
 * @type armor[]
 * @default []
 * @desc Armor IDs that the party must have >= 1 before the skill will be usable (in inventory, not equips)
 * 
 * @param Weapons
 * @type weapon[]
 * @default []
 * @desc Weapon IDs that the party must have >= 1 before the skill will be usable (in inventory, not equips)
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
 * @url https://www.caspergaming.com/plugins/cgmz/skillconditions/
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
 * 【插件版本】 V 1.1.0
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
 * The filename for this plugin MUST remain CGMZ_SkillConditions.js
 * This is what it comes as when downloaded. The filename is used to load
 * parameters and execute plugin commands. If you change it, things will begin
 * behaving incorrectly and your game will probably crash. Please do not
 * rename the js file.
 * --------------------------Latest Version------------------------------------
 * Hi all, this latest version adds items, weapons, and armors to the possible
 * skill restrictions. These settings allow you to make sure the player has
 * an item, weapon, or armor in their inventory before a skill can be used.
 * You might use this to make sure they have an Arrow item before they can use
 * a bow skill. Note that these are checked in the inventory only, and do not
 * include equips.
 *
 * This update was a minor re-write of how the plugin works internally to make
 * adding new restrictions and advanced features easier in future updates.
 *
 * Version 1.1.0
 * - Added item, weapon, and armor inventory requirements
 *
 * @param Restriction Setup
 * @text 技能限制设置
 * @default 关于技能限制的参数设置
 *
 * @param Skills
 * @text 技能限制
 * @parent Restriction Setup
 * @type struct<SkillRestriction>[]
 * @default []
 * @desc 设置技能限制
 *
 * @param Mechanics
 *
 * @param Lift Restriction Switch
 * @text 解除技能限制的开关
 * @parent Mechanics
 * @type switch
 * @default 0
 * @desc 指定一个开关，当其打开时(ON)会解除所有技能限制，当其关闭时(OFF)会重新激活所有限制。
*/
/*~struct~SkillRestriction:zh-CN
 * @param Skill
 * @text 技能
 * @type skill
 * @default 0
 * @desc 设置限制使用的技能ID
 * 
 * @param Restricted Maps
 * @text 地图限制
 * @type map[]
 * @default []
 * @desc 指定一个地图ID，使队伍在该地图中时不能使用这个技能
 * 
 * @param Restricted Troops
 * @text 战斗限制
 * @type troop[]
 * @default []
 * @desc 指定一个敌群ID，使队伍与该敌群战斗时不能使用这个技能
 * 
 * @param Profession Restrictions
 * @type struct<Profession>[]
 * @default []
 * @desc Professions and required level to use the item
 * 
 * @param Switches
 * @type switch[]
 * @default []
 * @desc Switch IDs that must be ON before the skill will be usable.
 * 
 * @param Variables
 * @type variable[]
 * @default []
 * @desc Variable IDs that must be >= 1 before the skill will be usable.
 * 
 * @param Items
 * @type item[]
 * @default []
 * @desc Item IDs that the party must have >= 1 before the skill will be usable
 * 
 * @param Armors
 * @type armor[]
 * @default []
 * @desc Armor IDs that the party must have >= 1 before the skill will be usable (in inventory, not equips)
 * 
 * @param Weapons
 * @type weapon[]
 * @default []
 * @desc Weapon IDs that the party must have >= 1 before the skill will be usable (in inventory, not equips)
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
 * @url https://www.caspergaming.com/plugins/cgmz/skillconditions/
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
 * Versión: 1.1.0
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
 * The filename for this plugin MUST remain CGMZ_SkillConditions.js
 * This is what it comes as when downloaded. The filename is used to load
 * parameters and execute plugin commands. If you change it, things will begin
 * behaving incorrectly and your game will probably crash. Please do not
 * rename the js file.
 * --------------------------Latest Version------------------------------------
 * Hi all, this latest version adds items, weapons, and armors to the possible
 * skill restrictions. These settings allow you to make sure the player has
 * an item, weapon, or armor in their inventory before a skill can be used.
 * You might use this to make sure they have an Arrow item before they can use
 * a bow skill. Note that these are checked in the inventory only, and do not
 * include equips.
 *
 * This update was a minor re-write of how the plugin works internally to make
 * adding new restrictions and advanced features easier in future updates.
 *
 * Version 1.1.0
 * - Added item, weapon, and armor inventory requirements
 *
 * @param Restriction Setup
 * @text Restricciones de habilidad
 *
 * @param Skills
 * @text Habilidades
 * @parent Restriction Setup
 * @type struct<SkillRestriction>[]
 * @default []
 * @desc Configura las restricciones de habilidad aquí.
 *
 * @param Mechanics
 *
 * @param Lift Restriction Switch
 * @text Interruptor de restricción de elevación de habilidades
 * @parent Mechanics
 * @type switch
 * @default 0
 * @desc Cuando este interruptor está activado, se eliminarán todas las restricciones de habilidad. Cuando está APAGADO, se aplican restricciones.
*/
/*~struct~SkillRestriction:es
 * @param Skill
 * @text Habilidad
 * @type skill
 * @default 0
 * @desc La habilidad a la cual agregar restricciones de habilidad.
 * 
 * @param Restricted Maps
 * @text Mapas restringidos
 * @type map[]
 * @default []
 * @desc Los ID del mapa para restringir el uso de la habilidad.
 * 
 * @param Restricted Troops
 * @text Tropas restringidas
 * @type troop[]
 * @default []
 * @desc Los ID de tropa para restringir el uso de la habilidad.
 * 
 * @param Profession Restrictions
 * @type struct<Profession>[]
 * @default []
 * @desc Professions and required level to use the item
 * 
 * @param Switches
 * @type switch[]
 * @default []
 * @desc Switch IDs that must be ON before the skill will be usable
 * 
 * @param Variables
 * @type variable[]
 * @default []
 * @desc Variable IDs that must be >= 1 before the skill will be usable
 * 
 * @param Items
 * @type item[]
 * @default []
 * @desc Item IDs that the party must have >= 1 before the skill will be usable
 * 
 * @param Armors
 * @type armor[]
 * @default []
 * @desc Armor IDs that the party must have >= 1 before the skill will be usable (in inventory, not equips)
 * 
 * @param Weapons
 * @type weapon[]
 * @default []
 * @desc Weapon IDs that the party must have >= 1 before the skill will be usable (in inventory, not equips)
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
Imported.CGMZ_SkillConditions = true;
CGMZ.Versions["Skill Conditions"] = "1.1.0";
CGMZ.SkillConditions = {};
CGMZ.SkillConditions.parameters = PluginManager.parameters('CGMZ_SkillConditions');
CGMZ.SkillConditions.SwitchLiftRestrictions = Number(CGMZ.SkillConditions.parameters["Lift Restriction Switch"]);
CGMZ.SkillConditions.SkillRestrictions = CGMZ_Utils.parseJSON(CGMZ.SkillConditions.parameters["Skills"], [], "[CGMZ] Skill Conditions", "Your Skill restrictions parameter was set up incorrectly, no skill restrictions could be loaded.");
//=============================================================================
// CGMZ_Data_SkillCondition
//-----------------------------------------------------------------------------
// Store and manage skill condition data. Not saved.
//=============================================================================
function CGMZ_Data_SkillCondition() {
    this.initialize.apply(this, arguments);
}
//-----------------------------------------------------------------------------
// Initialize
//-----------------------------------------------------------------------------
CGMZ_Data_SkillCondition.prototype.initialize = function(condition) {
	const id = condition.Skill;
	this.maps = CGMZ_Utils.parseJSON(condition["Restricted Maps"], [], "[CGMZ] Skill Conditions", `Invalid JSON in skill id: ${id}'s map restriction parameter.`).map(mapId => Number(mapId));
	this.troops = CGMZ_Utils.parseJSON(condition["Restricted Troops"], [], "[CGMZ] Skill Conditions", `Invalid JSON in skill id: ${id}'s troop restriction parameter.`).map(troopId => Number(troopId));
	this.switches = CGMZ_Utils.parseJSON(condition["Switches"], [], "[CGMZ] Skill Conditions", `Invalid JSON in skill id: ${id}'s switch restriction parameter.`).map(switchId => Number(switchId));
	this.variables = CGMZ_Utils.parseJSON(condition["Variables"], [], "[CGMZ] Skill Conditions", `Invalid JSON in skill id: ${id}'s variable restriction parameter.`).map(varId => Number(varId));
	this.items = CGMZ_Utils.parseJSON(condition["Items"], [], "[CGMZ] Skill Conditions", `Invalid JSON in skill id: ${id}'s items restriction parameter.`).map(itemId => Number(itemId));
	this.weapons = CGMZ_Utils.parseJSON(condition["Weapons"], [], "[CGMZ] Skill Conditions", `Invalid JSON in skill id: ${id}'s weapons restriction parameter.`).map(wepId => Number(wepId));
	this.armors = CGMZ_Utils.parseJSON(condition["Armors"], [], "[CGMZ] Skill Conditions", `Invalid JSON in skill id: ${id}'s armors restriction parameter.`).map(armorId => Number(armorId));
	this.professions = [];
	if(Imported.CGMZ_Professions) {
		const professionsJSON = CGMZ_Utils.parseJSON(condition["Profession Restrictions"], [], "[CGMZ] Skill Conditions", `Invalid JSON in skill id: ${id}'s profession restriction parameter.`);
		for(const professionJSON of professionsJSON) {
			const profession = CGMZ_Utils.parseJSON(professionJSON, null, "[CGMZ] Skill Conditions", `Invalid JSON in profession restriction parameter for skill id: ${id}`);
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
// Adds location restriction data to CGMZ Temp
//=============================================================================
//-----------------------------------------------------------------------------
// Add set up skill conditions data
//-----------------------------------------------------------------------------
const alias_CGMZSkillConditions_CGMZTemp_createPluginData = CGMZ_Temp.prototype.createPluginData
CGMZ_Temp.prototype.createPluginData = function() {
	alias_CGMZSkillConditions_CGMZTemp_createPluginData.call(this);
	this._skillRestrictions = [];
	for(const skill of CGMZ.SkillConditions.SkillRestrictions) {
		const skillObj = CGMZ_Utils.parseJSON(skill, null, "[CGMZ] Skill Conditions", `Invalid JSON in skill restriction: ${skill}`);
		if(!skillObj) continue;
		const id = Number(skillObj.Skill);
		const obj = new CGMZ_Data_SkillCondition(skillObj);
		this._skillRestrictions[id] = obj;
	}
};
//-----------------------------------------------------------------------------
// Get restrictions for a skill by id
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.getSkillCondition = function(id) {
	return this._skillRestrictions[id];
};
//=============================================================================
// Game_BattlerBase
//-----------------------------------------------------------------------------
// Additional checking for correct map to use skill
//=============================================================================
//-----------------------------------------------------------------------------
// Check restrictions on the skill before allowing use
//-----------------------------------------------------------------------------
const alias_CGMZSkillConditions_GameBattlerBase_canUse = Game_BattlerBase.prototype.canUse;
Game_BattlerBase.prototype.canUse = function(item) {
	const origReturn = alias_CGMZSkillConditions_GameBattlerBase_canUse.call(this, item);
	if(origReturn && DataManager.isSkill(item) && this.isActor()) {
		if(!$gameSwitches.value(CGMZ.SkillConditions.SwitchLiftRestrictions)) {
			return this.CGMZ_SkillConditions_checkSkillRestrictions(item);
		}
	}
    return origReturn;
};
//-----------------------------------------------------------------------------
// Check skills for restrictions
//-----------------------------------------------------------------------------
Game_BattlerBase.prototype.CGMZ_SkillConditions_checkSkillRestrictions = function(skill) {
	const condition = $cgmzTemp.getSkillCondition(skill.id);
	if(!condition) return true; // no conditions
	if(!this.CGMZ_SkillConditions_meetsProfessionCondition(condition)) return false;
	if(!this.CGMZ_SkillConditions_meetsSwitchCondition(condition.switches)) return false;
	if(!this.CGMZ_SkillConditions_meetsVariableCondition(condition.variables)) return false;
	if(!this.CGMZ_SkillConditions_meetsItemCondition(condition.items)) return false;
	if(!this.CGMZ_SkillConditions_meetsWeaponCondition(condition.weapons)) return false;
	if(!this.CGMZ_SkillConditions_meetsArmorCondition(condition.armors)) return false;
	if($gameParty.inBattle()) {
		return this.CGMZ_SkillConditions_checkBattleRestrictions(condition);
	}
	return this.CGMZ_SkillConditions_checkMapRestrictions(condition);
};
//-----------------------------------------------------------------------------
// Check battle restrictions
//-----------------------------------------------------------------------------
Game_BattlerBase.prototype.CGMZ_SkillConditions_checkBattleRestrictions = function(condition) {
	return (!condition.troops.includes($gameTroop._troopId));
};
//-----------------------------------------------------------------------------
// Check map restrictions
//-----------------------------------------------------------------------------
Game_BattlerBase.prototype.CGMZ_SkillConditions_checkMapRestrictions = function(condition) {
	return (!condition.maps.includes($gameMap.mapId()));
};
//-----------------------------------------------------------------------------
// Check profession restrictions
//-----------------------------------------------------------------------------
Game_BattlerBase.prototype.CGMZ_SkillConditions_meetsProfessionCondition = function(condition) {
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
Game_BattlerBase.prototype.CGMZ_SkillConditions_meetsSwitchCondition = function(switchA) {
	for(const switchId of switchA) {
		if(switchId && !$gameSwitches.value(switchId)) return false;
	}
	return true;
};
//-----------------------------------------------------------------------------
// Check variable restrictions
//-----------------------------------------------------------------------------
Game_BattlerBase.prototype.CGMZ_SkillConditions_meetsVariableCondition = function(variableA) {
	for(const varId of variableA) {
		if(varId && $gameVariables.value(varId) < 1) return false;
	}
	return true;
};
//-----------------------------------------------------------------------------
// Check item inventory restrictions
//-----------------------------------------------------------------------------
Game_BattlerBase.prototype.CGMZ_SkillConditions_meetsItemCondition = function(itemA) {
	for(const itemId of itemA) {
		const item = CGMZ_Utils.lookupItem('item', itemId);
		if(!$gameParty.hasItem(item, false)) return false;
	}
	return true;
};
//-----------------------------------------------------------------------------
// Check armor inventory restrictions
//-----------------------------------------------------------------------------
Game_BattlerBase.prototype.CGMZ_SkillConditions_meetsArmorCondition = function(armorA) {
	for(const armorId of armorA) {
		const armor = CGMZ_Utils.lookupItem('armor', armorId);
		if(!$gameParty.hasItem(armor, false)) return false;
	}
	return true;
};
//-----------------------------------------------------------------------------
// Check weapon inventory restrictions
//-----------------------------------------------------------------------------
Game_BattlerBase.prototype.CGMZ_SkillConditions_meetsWeaponCondition = function(wepA) {
	for(const wepId of wepA) {
		const wep = CGMZ_Utils.lookupItem('weapon', wepId);
		if(!$gameParty.hasItem(wep, false)) return false;
	}
	return true;
};