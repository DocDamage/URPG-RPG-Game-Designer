//=============================================================================
// VisuStella MZ - Items & Equips Core
// VisuMZ_1_ItemsEquipsCore.js
//=============================================================================

var Imported = Imported || {};
Imported.VisuMZ_1_ItemsEquipsCore = true;

var VisuMZ = VisuMZ || {};
VisuMZ.ItemsEquipsCore = VisuMZ.ItemsEquipsCore || {};
VisuMZ.ItemsEquipsCore.version = 1.60;

//=============================================================================
 /*:
 * @target MZ
 * @plugindesc [RPG Maker MZ] [Tier 1] [Version 1.60] [ItemsEquipsCore]
 * @author VisuStella
 * @url http://www.yanfly.moe/wiki/Items_and_Equips_Core_VisuStella_MZ
 * @orderAfter VisuMZ_0_CoreEngine
 *
 * @help
 * ============================================================================
 * Introduction
 * ============================================================================
 *
 * The Items & Equips Core makes improvements to the RPG Maker MZ item and
 * equipment dedicated scenes (including the shop) and how they're handled.
 * From more item categories, better parameter control, rulings, and more, game
 * devs are able to take control over key aspects of their game's items.
 *
 * Features include all (but not limited to) the following:
 *
 * * Modifying the appearances to the Item Scene, Equip Scene, and Shop Scene.
 * * Categorizing items in unique and multiple categories.
 * * Item Scene and Shop Scene will now display detailed information on items.
 * * NEW! marker can be displayed over recently acquired items in-game.
 * * Equipment notetags to adjust parameters past the editor limitations.
 * * Equipment Rulings to adjust what slot types can and can't be unequipped
 *   and/or optimized.
 * * Equipment Type Handling offers more control over equipment loadouts.
 * * Items sold in shops can be hidden/shown based on Switches.
 * * Items sold in shops can have varying prices adjusted by notetags.
 *
 * ============================================================================
 * Requirements
 * ============================================================================
 *
 * This plugin is made for RPG Maker MZ. This will not work in other iterations
 * of RPG Maker.
 *
 * ------ Tier 1 ------
 *
 * This plugin is a Tier 1 plugin. Place it under other plugins of lower tier
 * value on your Plugin Manager list (ie: 0, 1, 2, 3, 4, 5). This is to ensure
 * that your plugins will have the best compatibility with the rest of the
 * VisuStella MZ library.
 *
 * ============================================================================
 * Major Changes: New Hard-Coded Features
 * ============================================================================
 *
 * This plugin adds some new hard-coded features to RPG Maker MZ's functions.
 * The following is a list of them.
 *
 * ---
 *
 * Equipment Type Handling
 *
 * - Characters will no longer have one universal equipment slot setting.
 * Classes can have different equipment type loadouts, made possible through
 * the usage of notetags. Also, equipment types of matching names would be
 * treated as the same type, where previously, they would be different types.
 * This means if you have two "Accessory" slots, be it in the form of notetags
 * or through the Database > Types tab, they can both equip the same type of
 * accessories.
 *
 * - The Change Equip event command is now updated to reflect this new change.
 * When processing an equip change, the slot changed will go to the first
 * empty slot of matching type. If all of the actor's matching slot types are
 * equipped, then the equip will replace the last slot available.
 *
 * ---
 *
 * Shop Status Window
 *
 * - The Status Window found in the Shop Scene was originally barren and did
 * not display much information at all. This is changed through this plugin's
 * new features. While the contents of the Shop Status Window can be customized
 * through the Plugin Parameters, it is a change that cannot be reversed and
 * for the better since it gives players the much needed information revolving
 * around the game's items.
 *
 * ---
 *
 * Core Engine Compatibility: Modern Controls
 *
 * - If the VisuStella Core Engine is added to your game with Modern Controls
 * enabled, then the Item Menu Scene, Equip Menu Scene, and Shop Menu Scene's
 * controls will be changed a bit.
 *
 * - The Item Menu Scene will automatically have the Item List Window active,
 * with using the Left/Right (for single column) or Page Up/Page Down (for
 * multi-columns) to navigate between the Item Categories. Similar will occur
 * when trying to sell items in the Shop Menu Scene.
 *
 * - The Equip Menu Scene will automatically have the Equip Slots Window active
 * and only activate the command window upon moving up to it.
 *
 * ---
 *
 * ============================================================================
 * VisuStella MZ Compatibility
 * ============================================================================
 *
 * While this plugin is compatible with the majority of the VisuStella MZ
 * plugin library, it is not compatible with specific plugins or specific
 * features. This section will highlight the main plugins/features that will
 * not be compatible with this plugin or put focus on how the make certain
 * features compatible.
 * 
 * ---
 * 
 * VisuMZ_1_BattleCore
 * 
 * Changing the "Damage Multiplier" or "Healing Multiplier" vocabulary for the
 * Item and Equip Core's Shop Status Window is not done with the Item and Equip
 * Core's Plugin Parameters if you have the Battle Core installed.
 * 
 * Instead, go to Battle Core's Plugin Parameters, Damage Settings, Damage
 * Styles, and adjust the style's version of the "Damage Multiplier" or
 * "Healing Multiplier" text instead.
 * 
 * Why does this work this way? Because not all damage styles work off
 * "Multipliers" so in order for it to convey the proper message to the player,
 * each damage style has its own vocabulary to be more accurate.
 * 
 * In case you forget about that, when you visit the Item and Equip Core's
 * plugin parameters for these, it should also remind you in the parameter's
 * description on where to change it.
 * 
 * ---
 *
 * VisuMZ_2_WeaponSwapSystem
 *
 * The custom equip slots feature from the VisuStella MZ Items and Equips Core
 * allowed you to add in extra weapon slots. This is now curated up to a max
 * of one weapon slot per character. This needs to be done to make the Weapon
 * Swap System viable.
 *
 * ---
 *
 * ============================================================================
 * Notetags
 * ============================================================================
 *
 * The following are notetags that have been added through this plugin. These
 * notetags will not work with your game if this plugin is OFF or not present.
 * 
 * ---
 *
 * === General ===
 * 
 * These notetags affect the Items, Weapons, and Armors on a general scale.
 *
 * ---
 *
 * <Max: x>
 *
 * - Used for: Item, Weapon, Armor Notetags
 * - Determines the maximum quantity that can be held for this item.
 * - Replace 'x' with a number value to determine the maximum amount.
 *
 * ---
 *
 * <Color: x>
 * <Color: #rrggbb>
 *
 * - Used for: Item, Weapon, Armor, Skill Notetags
 * - Determines the color of the object inside the in-game menus.
 * - Replace 'x' with a number value depicting a window text color.
 * - Replace 'rrggbb' with a hex color code for a more custom color.
 *
 * ---
 *
 * <Category: x>
 *
 * - Used for: Item, Weapon, Armor Notetags
 * - Arranges items into certain/multiple categories to work with the Category
 *   Plugin Parameter setting: "Category:x".
 * - Replace 'x' with a category name to mark this item as.
 *
 * ---
 *
 * <Categories>
 *  x
 *  x
 * </Categories>
 *
 * - Used for: Item, Weapon, Armor Notetags
 * - Arranges items into certain/multiple categories to work with the Category
 *   Plugin Parameter setting: "Category:x".
 * - Replace each 'x' with a category name to mark this item as.
 *
 * ---
 * 
 * <Conserve: x%>
 * 
 * - Used for: Item
 * - Gives the item a percent chance when used to not consume the item.
 * - Replace 'x' with a number representing the percent chance to successfully
 *   conserve the item.
 * - If an item cannot be consumed, conserve chance will be 100% regardless.
 * 
 * ---
 * 
 * <ID Sort Priority: x>
 * 
 * - Used for: Item, Weapon, and Armor Notetags
 * - Used for Scene_Item, Scene_Equip, Scene_Battle, and Scene_Shop's sell
 *   option (only when selling).
 * - Changes sorting priority by ID for item, weapon, or armor to 'x'. 
 *   - Default priority level is '50'.
 * - Items, weapons, and armors with higher priority values will be sorted
 *   higher up on the list while lower values will be lower on the list.
 * 
 * ---
 *
 * === Item Accessibility Notetags ===
 *
 * The following notetags allow you to choose when items can/cannot be used
 * based on switches.
 *
 * ---
 *
 * <Enable Switch: x>
 *
 * <Enable All Switches: x,x,x>
 * <Enable Any Switches: x,x,x>
 *
 * - Used for: Item Notetags
 * - Determines the enabled status of the item based on switches.
 * - Replace 'x' with the switch ID to determine the item's enabled status.
 * - If 'All' notetag variant is used, item will be disabled until all
 *   switches are ON. Then, it would be enabled.
 * - If 'Any' notetag variant is used, item will be enabled if any of the
 *   switches are ON. Otherwise, it would be disabled.
 *
 * ---
 *
 * <Disable Switch: x>
 *
 * <Disable All Switches: x,x,x>
 * <Disable Any Switches: x,x,x>
 *
 * - Used for: Item Notetags
 * - Determines the enabled status of the item based on switches.
 * - Replace 'x' with the switch ID to determine the item's enabled status.
 * - If 'All' notetag variant is used, item will be enabled until all switches
 *   are ON. Then, it would be disabled.
 * - If 'Any' notetag variant is used, item will be disabled if any of the
 *   switches are ON. Otherwise, it would be enabled.
 *
 * ---
 *
 * === JavaScript Notetags: Item Accessibility ===
 *
 * The following are notetags made for users with JavaScript knowledge to
 * determine if an item can be accessible by code.
 *
 * ---
 *
 * <JS Item Enable>
 *  code
 *  code
 *  enabled = code;
 * </JS Item Enable>
 *
 * - Used for: Item Notetags
 * - Determines the enabled status of the item based on JavaScript code.
 * - If the actor this is disabled for is the only party member, it will not be
 *   visible in the item list unless the VisuStella Battle Core is installed.
 *   - If the VisuStella Battle Core is installed, then all battle scope items
 *     will be visible even if they're disabled.
 * - Replace 'code' to determine the type enabled status of the item.
 * - The 'enabled' variable returns a boolean (true/false) to determine if the
 *   item will be enabled or not.
 * - The 'user' variable refers to the user with the item.
 * - The 'item' variable refers to the item being checked.
 * - All other item conditions must be met in order for this to code to count.
 *
 * ---
 *
 * === Equipment Notetags ===
 *
 * The following notetags provide equipment-related effects from deciding what
 * equip slots can be given to classes to the base parameter changes asigned
 * to weapons and armors.
 *
 * ---
 *
 * <Equip Slots>
 *  slotName
 *  slotName
 *  slotName
 * </Equip Slots>
 *
 * - Used for: Class Notetags
 * - Changes the equipment slot loadout for any actor who is that class.
 * - Replace 'slotName' with an Equipment Type name from Database > Types.
 *   This is case-sensitive.
 * - Insert or remove as many "slotName" equipment types as needed.
 *
 * ---
 *
 * <param: +x>
 * <param: -x>
 *
 * - Used for: Weapon, Armor Notetags
 * - Changes the base parameter value for the equip item.
 * - Replace 'param' with any of the following: 'MaxHP', 'MaxMP', 'ATK', 'DEF',
 *   'MAT', 'MDF', 'AGI', or 'LUK' to change that specific parameter's value.
 *   - These notetags do NOT work with X Parameters, S Parameters, or any
 *     custom parameters. These notetags ONLY work with the base parameters.
 * - Replace 'x' with a number value to set the parameter value to.
 * - This allows you to bypass the Database Editor's number limitations.
 *
 * ---
 * 
 * <Equip Copy Limit: x>
 * 
 * - Used for: Weapon, Armor Notetags
 * - Sets a maximum number of copies that the actor can wear of this equipment.
 * - Replace 'x' with a number value to determine the copy limit.
 * - This can be bypassed using Event Commands and/or Script Calls.
 * - Usage Example: Actors can only equip one copy of the "One-of-a-Kind Ring"
 *   on at any time despite having empty accessory slots because the ring has a
 *   <Equip Copy Limit: 1> notetag.
 * 
 * ---
 * 
 * <Equip Weapon Type Limit: x>
 * 
 * - Used for: Weapon
 * - This weapon cannot be equipped with other weapons of the same type once
 *   the limited amount has been reached.
 * - Replace 'x' with a number value to determine the weapon type limit.
 * - This can be bypassed using Event Commands and/or Script Calls.
 * - Usage Example: A dualwielding warrior who can only equip one sword and a
 *   dagger but never two swords or two daggers because the swords and daggers
 *   all have the <Equip Weapon Type Limit: 1> notetags on them.
 * 
 * ---
 * 
 * <Equip Armor Type Limit: x>
 * 
 * - Used for: Armor
 * - This armor cannot be equipped with other armors of the same type once the
 *   limited amount has been reached.
 * - Replace 'x' with a number value to determine the armor type limit.
 * - This can be bypassed using Event Commands and/or Script Calls.
 * - Usage Example: People cannot equip more than two glove accessories on at a
 *   time because the glove is a "Glove" armor-type and each glove item has the
 *   <Equip Armor Type Limit: 2> notetags on them.
 * 
 * ---
 * 
 * <Party Artifact>
 * <Troop Artifact>
 * 
 * <Stackable Party Artifact>
 * <Stackable Troop Artifact>
 * 
 * - Used for: Armor
 * - This armor cannot be equipped at all. However, by simply being in the
 *   party's inventory, its parameter bonuses and traits will be applied
 *   globally throughout the whole party or troop (depending on the notetag).
 * - Add both notetags to affect both groups.
 * - The normal versions of the notetag is only applied once regardless of the
 *   number of copies are found in the party's inventory.
 * - The stackable versions of the notetag will have the bonuses and traits
 *   stacked multiple times relative to the number of copies found in the
 *   party's inventory.
 * - This item will NOT be added during the setup phase for Battle Tests.
 *   - If you want to add the item, do it manually.
 * 
 * ---
 * 
 * <Equip For Class Only: x>
 * <Equip For Classes Only: x, x, x>
 * <Equip For Class Only: name>
 * <Equip For Classes Only: name, name, name>
 * 
 * - Used for: Weapon, Armor Notetags
 * - This piece of equipment can only be worn by members with 'x' as the main
 *   class. If there are multiple classes listed, at least one of them need to
 *   be the actor's main class.
 * - Replace 'x' with a number representing the ID of the class required.
 * - For the 'name' variant, replace 'name' with the name of the required class
 *   the actor needs to have in order to equip this object.
 * 
 * ---
 * 
 * <Equip Requirements>
 *  requirement
 *  requirement
 *  requirement
 * </Equip Requirements>
 * 
 * - Used for: Weapon, Armor Notetags
 * - Defines a requirement(s) for the actor to meet in order for the equip item
 *   to be equippable.
 * - Failure to meet these requirements will cause the equipment to unequip
 *   automatically.
 *   - Keep in mind that in some cases, this will not happen immediately.
 *     Things like switches will require the actor to meet its cache clear
 *     in order to trigger the automatic unequip.
 *   - Some ways to trigger a cache clear would be to change the actor's HP/MP,
 *     or adding and then removing a state for the actor (preferrably an unused
 *     state that has no real effect).
 * - Replace 'requirement' with one of the settings bellow:
 * - Add multiple 'requirement' lines for more requirements.
 * 
 *   Requirements:
 *
 *   param > x
 *   param >= x
 *   param === x
 *   param <= x
 *   param < x
 *   - Replace 'param' with 'level', 'maxhp', 'maxmp', 'atk', 'def', 'mat',
 *     'mdf', 'agi', or 'luk'.
 *   - This will make the piece of equipment require the actor's base parameter
 *     to be greater than (>), greater than or equal to (>=), equal to (===),
 *     less than or equal to (<=), or less than (<).
 *   - This is NOT the value for the total parameter, only the base parameter.
 *   - The base parameter is calculated by the user's class parameter value and
 *     any bonuses received through permanent stat increases.
 *
 *   learned skill: x
 *   learned skill: name
 *   - This will make the piece of equipment require the actor to have learned
 *     skill 'x'. 
 *   - If 'name' is used, priority will be given to the skill with the highest
 *     ID in the database.
 *   - The actor needs to have LEARNED the skill. This means that if you have
 *     added a skill to the actor's kit through a trait, it will not count.
 *
 *   switch: x
 *   - This will require switch X to be on.
 *   - If it isn't, the piece of equipment cannot be worn.
 *   - Insert multiple of these to add more switches that are are required to
 *     be on.
 * 
 *   ***NOTE 1***
 *   There is no "class: x" for these equip requirements. Instead, use the
 *   <Equip For Class Only: x> notetags.
 * 
 *   ***NOTE 2***
 *   For those wondering where "unique only" is, that does not exist in this
 *   plugin. Instead, use the <Equip Copy Limit: x> notetag listed above.
 * 
 *   Example A:
 * 
 *     <Equip Requirements>
 *     level >= 20
 *     </Equip Requirements>
 * 
 *     - Requires the user to be at least level 20 in order to equip.
 * 
 *   Example B:
 * 
 *     <Equip Requirements>
 *     atk >= 50
 *     def <= 50
 *     </Equip Requirements>
 *     - Requires the user have at least 50 base ATK to equip.
 *     - Requires the user to be under 50 base DEF to equip.
 * 
 * ---
 * 
 * <Added EType: x>
 * <Added ETypes: x, x, x>
 * 
 * - Used for: Armor Notetags
 * - This is for armors only and does NOT work with weapons!
 * - Allows a piece of armor to belong to multiple ETypes. This means a glove
 *   can be equipped as "Armgear" or as an "Accessory" if you so choose.
 * - Replace 'x' with a number representing the ID of the EType you wish to add
 *   to the list of ETypes.
 *   - Insert multiple 'x' entries to add more than one EType ID.
 * 
 * ---
 * 
 * <Cursed>
 * 
 * - Used for: Weapon, Armor Notetags
 * - If this weapon or armor is equipped, it cannot manually be removed by the
 *   player until it is purified.
 * - To remove it, it must be done by event commands, script calls, or through
 *   the Purify-related Plugin Commands provided by this plugin.
 * - Once purified, the weapon or armor will become unequipped unless it has a
 *   purify transformation.
 *   - If the newly transformed weapon/armor is equippable, it will remain in
 *     the actor's equipment slots.
 * - If you are using VisuMZ_2_WeaponSwapSystem, weapons cannot become cursed
 *   in order to allow free weapon swapping. Weaponry will not be cursed
 *   if VisuMZ_2_WeaponSwapSystem is installed.
 * 
 * ---
 * 
 * <Purify Transform: id>
 * <Purify Transform: name>
 * 
 * - Used for: Weapon, Armor Notetags
 * - If this notetag is present on a <Cursed> weapon or armor, then upon the
 *   actor receiving purification, the weapon or armor will transform into a
 *   different item.
 * - Replace 'id' with a number representing the transformed weapon/armor's ID.
 * - Replace 'name' with text representing the transformed weapon/armor's name.
 * - Weapons can only transform into weapons.
 * - Armors can only transform into armors.
 * 
 * ---
 *
 * === JavaScript Notetags: Equipment ===
 *
 * The following are notetags made for users with JavaScript knowledge to
 * adjust the parameter through code.
 *
 * ---
 *
 * <JS Parameters>
 *  MaxHP = code;
 *  MaxMP = code;
 *  ATK = code;
 *  DEF = code;
 *  MAT = code;
 *  MDF = code;
 *  AGI = code;
 *  LUK = code;
 * </JS Parameters>
 *
 * - Used for: Weapon, Armor Notetags
 * - Uses JavaScript to determine the values for the basic parameters based on
 *   the code used to calculate its value.
 * - The variables 'MaxHP', 'MaxMP', 'ATK', 'DEF', 'MAT', 'MDF', 'AGI', and
 *   'LUK' are used to determine the finalized value of the parameter. This
 *   variable is case sensitive.
 * - If a parameter is not present, its value will be treated as +0.
 * 
 * '''WARNING!''' If you are trying to calculate a value based off a full
 * parameter value, such as "ATK = user.atk * 0.10", it's going to break and
 * will cause an infinite loop. Use base parameter values instead.
 *
 * ---
 *
 * === Status Window Notetags ===
 *
 * The following notetags will affect the Shop Status Window info. If for any
 * reason the data that is displayed is not to your liking or insufficient,
 * you can change it up using the following notetags.
 *
 * ---
 *
 * <Status Info>
 *  key: data
 *  key: data
 *  key: data
 * </Status Info>
 *
 * - Used for: Skill, Item, Weapon, Armor Notetags
 * - If you do not like the generated data that's displayed, you can change it
 *   using this notetag to display what you want.
 * - Replace 'key' with one of the following for skills and items:
 *   - Consumable
 *   - Quantity
 *   - Occasion
 *   - Scope
 *   - Speed
 *   - Success Rate
 *   - Repeat
 *   - Hit Type
 *   - Element
 *   - Damage Multiplier
 *   - HP Recovery
 *   - MP Recovery
 *   - TP Recovery
 *   - HP Damage
 *   - MP Damage
 *   - TP Damage
 *   - User TP Gain
 *   - Added Effects
 *   - Removed Effects
 * - Replace 'key' with one of the following for weapons and armors:
 *   - 'MaxHP', 'MaxMP', 'ATK', 'DEF', 'MAT', 'MDF', 'AGI', or 'LUK'
 *   - For those with VisuMZ_0_CoreEngine:
 *     - 'HIT', 'EVA', 'CRI', 'CEV', 'MEV', 'MRF', 'CNT', 'HRG', 'MRG', 'TRG'
 *     - 'TGR', 'GRD', 'REC', 'PHA', 'MCR', 'TCR', 'PDR', 'MDR', 'FDR', 'EXR'
 *   - Only relevant if the Draw Style for equipment is "classic" or "double".
 * - Replace 'data' with the text data you want to visually appear. You may use
 *   text codes for this.
 * - This only affects info entries that are already visible and won't make
 *   other categories suddenly appear.
 * - For clarity regarding "Damage Multiplier" and "HP Recovery"/"HP Damage":
 *   - "Damage Multiplier" refers to the amount determined by damage formulas.
 *   - "HP Recovery"/"HP Damage" refers to the "Recover HP" database effect.
 *   - Likewise, the same will apply to "MP Recovery"/"MP Damage" if the damage
 *     formula type is to deal MP recovery/damage instead.
 * - Insert or remove as many "key: data" lines as needed.
 *
 * ---
 *
 * <Custom Status Info>
 *  key: data
 *  key: data
 *  key: data
 * </Custom Status Info>
 *
 * - Used for: Skill, Item, Weapon, Armor Notetags
 * - If you want custom categories and data to be displayed for your items that
 *   aren't provided by the Shop Status Window Info to begin with, you can use
 *   this notetag to add in your own entries.
 *   - When used with weapon or armor database objects, this information is
 *     only relevant if the Draw Style for equipment is "classic" or "double".
 * - Replace 'key' with text of the exact label you want. You may use text
 *   codes for this.
 * - Replace 'data' with text of the exact text data you want. You may use text
 *   codes for this.
 * - Insert or remove as many "key: data" lines as needed.
 *
 * ---
 * 
 * <Status Style: Compare>
 * <Status Style: Classic>
 * <Status Style: Double>
 *
 * - Used for: Weapon, Armor Notetags
 * - Changes up the way the shop status window displays data for this database
 *   object in particular.
 *     - Compare - Compares selected equip to equipped gear
 *       - Lists all main party actors
 *       - Displays the parameter differences when equipped
 *       - Calculates custom JS values
 *     - Classic - Shows basic parameters of selected equip
 *       - Involves no actors, only shows the item's stats
 *       - Shows weapon or armor specific parameters
 *       - Does not show custom JS values as those are calculated per actor
 *       - Does not show custom parameters as those are calculated per actor
 *       - Use <Status Info> and <Custom Status Info> notetags to overwrite or
 *         add custom data to classic equip data
 *     - Double - Shows basic parameters in double columns
 *       - Involves no actors, only shows the item's stats
 *       - Shows weapon or armor specific parameters
 *       - Does not show custom JS values as those are calculated per actor
 *       - Does not show custom parameters as those are calculated per actor
 *       - Use <Status Info> and <Custom Status Info> notetags to overwrite or
 *         add custom data to classic equip data
 * 
 * ---
 * 
 * <Custom Status Parameters: name, name, name>
 * 
 * - Used for: Weapon, Armor Notetags
 * - Requires VisuMZ_0_CoreEngine!
 *   - This will not work otherwise!
 * - Customize which parameters are displayed for this equipment object's shop
 *   status window.
 *   - This ONLY applies to the shop status window and not other windows.
 * - Replace 'name' with any of the following to display custom parameters:
 *   - 'MaxHP', 'MaxMP', 'ATK', 'DEF', 'MAT', 'MDF', 'AGI', and 'LUK'
 *   - 'HIT', 'EVA', 'CRI', 'CEV', 'MEV', 'MRF', 'CNT', 'HRG', 'MRG', 'TRG'
 *   - 'TGR', 'GRD', 'REC', 'PHA', 'MCR', 'TCR', 'PDR', 'MDR', 'FDR', 'EXR'
 *   - Does not work with custom parameters as those are calculated per actor.
 * - Parameters will be displayed in the order inserted into the notetag.
 *
 * ---
 * 
 * <Shop Picture Name: filename>
 * 
 * - Used for: Skill, Item, Weapon, Armor Notetags
 * - Enables a shop picture for the status window. This image can be seen in
 *   the item scene, shop scene, and skill scene if enabled.
 * - If this notetag is not used, there will be no image.
 * - Replace 'filename' with the filename of the graphic to use from the game
 *   project's img/pictures/ folder. Filenames are case sensitive. Leave out
 *   the filename extension from the notetag.
 * - Use the supporting notetags to determine where the image appears. If not,
 *   they will default to the background, fit to the window dimensions,
 *   centered at the middle of the window.
 * 
 * ---
 * 
 * <Shop Picture Layer: Background>
 * <Shop Picture Layer: Foreground>
 * 
 * - Used for: Skill, Item, Weapon, Armor Notetags
 * - Determines which layer the graphic will be drawn on.
 * - If the background layer is selected, the picture will appear behind the
 *   data text.
 * - If the foreground layer is selected, the picture will appear in front of
 *   the data text.
 * - If this notetag is not used, it will default to the background layer.
 * 
 * ---
 * 
 * <Shop Picture Max Width: x>
 * <Shop Picture Max Height: y>
 * <Shop Picture Max Dimensions: x, y>
 * 
 * - Used for: Skill, Item, Weapon, Armor Notetags
 * - Determines the maximum width and/or height for the image.
 * - This means the image will be automatically scaled proportionally to that
 *   width or height as long as everything else does not break boundaries.
 * - Replace 'x' and 'y' with number values representing the maximum dimensions
 *   the image can be in pixels.
 * - If these notetags are not used, the image will be automatically scaled to
 *   the dimensions of the shop status window.
 * 
 * ---
 * 
 * <Shop Picture Alignment: Left>
 * <Shop Picture Alignment: Center>
 * <Shop Picture Alignment: Right>
 * 
 * - Used for: Skill, Item, Weapon, Armor Notetags
 * - Adjusts the horizontal alignment for the image.
 * - Left, center, right determines how it's aligned horizontally if the
 *   image does not horizontally fit in the width of the window.
 * - If any of these notetags are not used, the image will default to the
 *   'center' alignment.
 * 
 * ---
 * 
 * <Shop Picture Position: Top>
 * <Shop Picture Position: Middle>
 * <Shop Picture Position: Bottom>
 * 
 * - Used for: Skill, Item, Weapon, Armor Notetags
 * - Adjusts the vertical position for the image.
 * - Top, middle, bottom determines how it's positioned vertically if the
 *   image does not vertically fit in the width of the window.
 * - If any of these notetags are not used, the image will default to the
 *   'middle' position.
 * 
 * ---
 * 
 * <Shop Picture Offset X: +x>
 * <Shop Picture Offset X: -x>
 * 
 * <Shop Picture Offset Y: +y>
 * <Shop Picture Offset Y: -y>
 * 
 * <Shop Picture Offset: +x, +y>
 * <Shop Picture Offset: -y, -y>
 * 
 * - Used for: Skill, Item, Weapon, Armor Notetags
 * - Offsets the X and Y positions of the image in the shop status window.
 * - X offsets adjust the horizontal position by x pixels.
 *   - Positive goes right.
 *   - Negative goes left.
 * - Y offsets adjust the horizontal position by y pixels.
 *   - Positive goes down.
 *   - Negative goes up.
 * - Replace 'x' and 'y' with number values representing the pixels to offset
 *   the image by. The '+' and '-' signs are required.
 * - If none of these notetags are used, there will be no offsets.
 * 
 * ---
 * 
 * <Shop Picture Opacity: x>
 * <Shop Picture Opacity: x%>
 * 
 * - Used for: Skill, Item, Weapon, Armor Notetags
 * - Adjusts the opacity of the image used.
 * - When using 'x' and not 'x%', use a number between 0 and 255.
 *   - The closer to 0, the more transparent the image is.
 *   - The closer to 255, the more opaque the image is.
 * - When using 'x%' and not 'x', use a number between 0% and 100%.
 *   - The closer to 0%, the more transparent the image is.
 *   - The closer to 100%, the more opaque the image is.
 * 
 * ---
 *
 * === Shop Menu Notetags ===
 *
 * These notetags adjust how prices and such are managed inside the Shop Menu
 * as well as whether or not some items are visible depending on switch states.
 *
 * ---
 *
 * <Price: x>
 *
 * - Used for: Item, Weapon, Armor Notetags
 * - Adjusts the buying price for this item.
 * - Replace 'x' with a number depicting the desired value for the buy price.
 * - This allows you to bypass the RPG Maker MZ editor's limitation of 999,999.
 *
 * ---
 *
 * <Can Sell>
 * <Cannot Sell>
 *
 * - Used for: Item, Weapon, Armor Notetags
 * - Makes the item either always sellable or cannot be sold.
 * - This bypasses the game's internal hard-coding to prevent items with a
 *   price of 0 from being able to be sold.
 * - This bypasses the game's internal hard-coding to always allow items with
 *   a price value of being able to be sold.
 *
 * ---
 *
 * <Sell Price: x>
 *
 * - Used for: Item, Weapon, Armor Notetags
 * - Changes the sell price to be something different than the default amount.
 * - Replace 'x' with a number depicting the desired value for the sell price.
 *
 * ---
 *
 * <Show Shop Switch: x>
 *
 * <Show Shop All Switches: x,x,x>
 * <Show Shop Any Switches: x,x,x>
 *
 * - Used for: Item, Weapon, Armor Notetags
 * - Determines the visibility of the shop item based on switches.
 * - Replace 'x' with the switch ID to determine the shop item's visibility.
 * - If 'All' notetag variant is used, item will be hidden until all switches
 *   are ON. Then, it would be shown.
 * - If 'Any' notetag variant is used, item will be shown if any of the
 *   switches are ON. Otherwise, it would be hidden.
 *
 * ---
 *
 * <Hide Shop Switch: x>
 *
 * <Hide Shop All Switches: x,x,x>
 * <Hide Shop Any Switches: x,x,x>
 *
 * - Used for: Item, Weapon, Armor Notetags
 * - Determines the visibility of the shop item based on switches.
 * - Replace 'x' with the switch ID to determine the shop item's visibility.
 * - If 'All' notetag variant is used, item will be shown until all switches
 *   are ON. Then, it would be hidden.
 * - If 'Any' notetag variant is used, item will be hidden if any of the
 *   switches are ON. Otherwise, it would be shown.
 *
 * ---
 *
 * <Cannot Sell Switch: x>
 *
 * <Cannot Sell All Switches: x,x,x>
 * <Cannot Sell Any Switches: x,x,x>
 *
 * - Used for: Item, Weapon, Armor Notetags
 * - Determines the sellability of the shop item based on switches.
 * - Replace 'x' with the switch ID to determine the shop item's sellability.
 * - If 'All' notetag variant is used, item cannot be sold until all switches
 *   are ON. Otherwise, it can be sold.
 * - If 'Any' notetag variant is used, item cannot be sold if any of the
 *   switches are ON. Otherwise, it can be sold.
 *
 * ---
 * 
 * <Buy Turn On Switch: x>
 * <Buy Turn On Switches: x, x, x>
 * 
 * - Used for: Item, Weapon, Armor Notetags
 * - When this item, weapon, or armor is bought in the shop scene, turn on the
 *   switch(es) 'x'.
 * - Replace 'x' with a number representing the ID of the switch to turn on.
 *   - Insert multiple 'x' values to turn on multiple switches upon buying.
 * 
 * ---
 * 
 * <Buy Turn Off Switch: x>
 * <Buy Turn Off Switches: x, x, x>
 * 
 * - Used for: Item, Weapon, Armor Notetags
 * - When this item, weapon, or armor is bought in the shop scene, turn off the
 *   switch(es) 'x'.
 * - Replace 'x' with a number representing the ID of the switch to turn off.
 *   - Insert multiple 'x' values to turn off multiple switches upon buying.
 * 
 * ---
 * 
 * <Sell Turn On Switch: x>
 * <Sell Turn On Switches: x, x, x>
 * 
 * - Used for: Item, Weapon, Armor Notetags
 * - When this item, weapon, or armor is sold in the shop scene, turn on the
 *   switch(es) 'x'.
 * - Replace 'x' with a number representing the ID of the switch to turn on.
 *   - Insert multiple 'x' values to turn on multiple switches upon selling.
 * 
 * ---
 * 
 * <Sell Turn Off Switch: x>
 * <Sell Turn Off Switches: x, x, x>
 * 
 * - Used for: Item, Weapon, Armor Notetags
 * - When this item, weapon, or armor is sold in the shop scene, turn off the
 *   switch(es) 'x'.
 * - Replace 'x' with a number representing the ID of the switch to turn off.
 *   - Insert multiple 'x' values to turn off multiple switches upon selling.
 * 
 * ---
 *
 * === JavaScript Notetags: Shop Menu ===
 *
 * The following are notetags made for users with JavaScript knowledge. These
 * notetags are primarily aimed at Buy and Sell prices.
 *
 * ---
 *
 * <JS Buy Price>
 *  code
 *  code
 *  price = code;
 * </JS Buy Price>
 *
 * - Used for: Item, Weapon, Armor Notetags
 * - Replace 'code' to determine the buying 'price' of the item.
 * - Insert the final buy price into the 'price' variable.
 * - The 'item' variable refers to the item being bought.
 *
 * ---
 *
 * <JS Sell Price>
 *  code
 *  code
 *  price = code;
 * </JS Sell Price>
 *
 * - Used for: Item, Weapon, Armor Notetags
 * - Replace 'code' to determine the selling 'price' of the item.
 * - Insert the final sell price into the 'price' variable.
 * - The 'item' variable refers to the item being sold.
 *
 * ---
 *
 * ============================================================================
 * Plugin Commands
 * ============================================================================
 *
 * The following are Plugin Commands that come with this plugin. They can be
 * accessed through the Plugin Command event command.
 *
 * ---
 * 
 * === Actor Plugin Commands ===
 * 
 * ---
 *
 * Actor: Change Equip Slots
 * - Forcefully change the actor(s) equip slots.
 * - These will persist through class changes.
 *
 *   Actor ID(s):
 *   - Select which Actor ID(s) to affect.
 *
 *   Equip Slots:
 *   - Insert the equip slots you want the actor(s) to have.
 *   - These entries are case-sensitive.
 *
 * ---
 *
 * Actor: Reset Equip Slots
 * - Reset any forced equip slots for the actor(s).
 * - Equip slots will then be based on class.
 *
 *   Actor ID(s):
 *   - Select which Actor ID(s) to affect.
 *
 * ---
 * 
 * === Purify Plugin Commands ===
 * 
 * ---
 * 
 * Purify: Target Actor(s)
 * - Purifies target actor(s) of any cursed weapons or armors.
 * - Cannot be used in battle.
 * 
 *   Actor ID(s):
 *   - Select which Actor ID(s) to affect.
 * 
 * ---
 * 
 * Purify: Whole Party
 * - Purifies whole party of any cursed weapons or armors.
 * - Cannot be used in battle.
 * 
 * ---
 * 
 * === Shop Plugin Commands ===
 * 
 * ---
 *
 * Shop: Advanced
 * - Make it easier to put together inventories for a shop.
 * - WARNING: Does not allow for event-specific prices.
 *
 *   Step 1: Item ID's
 *   - Select which Item ID ranges to add.
 *
 *   Step 2: Weapon ID's
 *   - Select which Weapon ID ranges to add.
 *
 *   Step 3: Armor ID's
 *   - Select which Armor ID ranges to add.
 *
 *   Step 4: Purchase Only?
 *   - Make the shop purchase-only?
 * 
 *   Optional:
 * 
 *     Blacklist
 *     - A list of categories to blacklist from the shop.
 *     - Not used if empty. Mark categories with <Category: x>
 * 
 *     Whitelist
 *     - A list of categories to whitelist for the shop.
 *     - Not used if empty. Mark categories with <Category: x>
 *
 * This Plugin Command primarily functions as an alternative to the editor's
 * "Shop Processing" event command as that one requires you to add items one at
 * a time, making it extremely tedious to add large amounts of items. This
 * Plugin Command will mitigate that by allowing ID ranges to determine which
 * items to make available.
 *
 * ---
 *
 * ============================================================================
 * Plugin Parameters: Item Menu Settings
 * ============================================================================
 *
 * The Item Menu Settings allow you to adjust specifics on how key objects and
 * windows in the Item Menu Scene operate.
 *
 * ---
 *
 * General Window
 *
 *   Use Updated Layout:
 *   - Use the Updated Item Menu Layout provided by this plugin?
 *   - This will automatically enable the Status Window.
 *   - This will override the Core Engine windows settings.
 * 
 *     Disabled: Vanilla:
 *     - If NOT using the updated layout, keep all settings to pure vanilla and
 *       ignore other Plugin Parameters.
 *     - Used for those who do not wish to fiddle with the Plugin Parameter
 *       settings to return the item menu back to default MZ vanilla.
 *     - Does not apply if using updated layout.
 *     - The following settings are disabled if using vanilla:
 *       - Categories: Only the default 4
 *       - Category Names only display text and no icons
 *       - No Shop Status Window
 *       - Modern Controls Disabled
 *       - Columns: Any -> 2
 *
 *   Layout Style:
 *   - If using an updated layout, how do you want to style the menu scene?
 *     - Upper Help, Left Input
 *     - Upper Help, Right Input
 *     - Lower Help, Left Input
 *     - Lower Help, Right Input
 *
 * ---
 *
 * List Window
 * 
 *   Columns:
 *   - Number of maximum columns.
 *   - If you are using the VisuStella MZ Core Engine and the "Modern Controls"
 *     Plugin Parameter, please read through that section in case you have any
 *     questions about how to switch between categories when using multiple
 *     columns of items at a time.
 *
 * ---
 *
 * Item Quantity
 *
 *   Item Max:
 *   Weapon Max:
 *   Armor Max:
 *   - The default maximum quantity for items, weapons, and/or armors.
 * 
 *   Quantity Format:
 *   - How to display an item's quantity.
 *   - %1 - Item Quantity
 *
 *   Font Size:
 *   - Default font size for item quantity.
 *
 * ---
 *
 * Shop Status Window
 * 
 *   Show in Item Menu?:
 *   - Show the Shop Status Window in the Item Menu?
 *   - This is enabled if the Updated Layout is on.
 *
 *   Adjust List Window?:
 *   - Automatically adjust the Item List Window in the Item Menu if using the
 *     Shop Status Window?
 * 
 *   Background Type:
 *   - Select background type for this window.
 *     - 0 - Window
 *     - 1 - Dim
 *     - 2 - Transparent
 *
 *   JS: X, Y, W, H:
 *   - Code used to determine the dimensions for this Status Window in the
 *     Item Menu.
 *
 * ---
 *
 * Button Assist Window
 *
 *   Switch Category:
 *   - Button assist text used for switching categories.
 *   - For VisuStella MZ's Core Engine's Button Assist Window.
 *
 * ---
 *
 * ============================================================================
 * Plugin Parameters: Item Categories
 * ============================================================================
 *
 * Item Categories appear both in the Item Menu Scene and Shop Menu Scene (but
 * only under the Sell command). These Plugin Parameters give you the ability
 * to add in the specific categories you want displayed, remove the ones you
 * don't, and associate them with icons.
 *
 * ---
 *
 * List
 *
 *   Category List
 *   - A list of the item categories displayed in the Item/Shop menus.
 * 
 *     Type:
 *     - A list of the item categories displayed in the Item/Shop menus.
 *     - Replace x with ID numbers or text.
 *     - AllItems, RegularItems, KeyItems
 *     - HiddenItemA, HiddenItemB
 *     - Consumable, Nonconsumable
 *     - AlwaysUsable, BattleUsable, FieldUsable, NeverUsable
 *     - AllWeapons, WType:x
 *     - AllArmors, AType:x, EType:x
 *     - Category:x
 * 
 *     Icon:
 *     - Icon used for this category.
 *     - Use 0 for no icon.
 * 
 *     Visibility Switch:
 *     - This Switch must be turned ON in order for the category to show.
 *     - Use 0 for no Switch requirement.
 * 
 *     Sort By:
 *     - Sort this category (in Scene_Item and Scene_Shop only) this way.
 *
 *   Style:
 *   - How do you wish to draw categorie entries in the Category Window?
 *   - Text Only: Display only the text.
 *   - Icon Only: Display only the icon.
 *   - Icon + Text: Display the icon first, then the text.
 *   - Auto: Determine which is better to use based on the size of the cell.
 *
 *   Text Alignment
 *   - Decide how you want the text to be aligned.
 *
 * ---
 *
 * Vocabulary
 *
 *   Hidden Item A
 *   Hidden Item B
 *   Consumable
 *   Nonconsumable
 *   Always Usable
 *   Battle Usable
 *   Field Usable
 *   Never Usable
 *   - How these categories are named in the Item Menu.
 *
 * ---
 *
 * ============================================================================
 * Plugin Parameters: NEW! Labels
 * ============================================================================
 *
 * Whenever the player receives a new item(s), a NEW! Label can be placed on
 * top of the item's icon when browsing a menu displaying the item(s). This is
 * a quality of life addition from more modern RPG's to help players figure out
 * what they've recently received. The following are Plugin Parameters made to
 * adjust how the NEW! Labels are handled in-game.
 *
 * ---
 *
 * NEW! Labels
 * 
 *   Use NEW! Labels?:
 *   - Use the NEW! Labels or not?
 * 
 *   Icon:
 *   - The icon index used to represent the NEW! text.
 *   - Use 0 to not draw any icons.
 * 
 *   Text:
 *   - The text written on the NEW! Label.
 * 
 *     Font Color:
 *     - Use #rrggbb for custom colors or regular numbers for text colors from
 *       the Window Skin.
 * 
 *     Font Size:
 *     - The font size used for the NEW! text.
 * 
 *   Fade Limit:
 *   - What's the upper opaque limit before reversing?
 * 
 *   Fade Speed:
 *   - What's the fade speed of the NEW! Label?
 * 
 *   Offset X:
 *   - How much to offset the NEW! Label's X position by.
 * 
 *   Offset Y:
 *   - How much to offset the NEW! Label's Y position by.
 *
 * ---
 *
 * ============================================================================
 * Plugin Parameters: Equip Menu Settings
 * ============================================================================
 *
 * These Plugin Parameters adjust the Equipment Menu Scene, ranging from using
 * a more updated and modern layout, changing the styles of other windows, and
 * other key visual aspects of the Equip Menu Scene. Other settings here allow
 * you to adjust how equipment operate under certain rulings, too.
 *
 * ---
 *
 * General
 * 
 *   Use Updated Layout:
 *   - Use the Updated Equip Layout provided by this plugin?
 *   - This will override the Core Engine windows settings.
 * 
 *     Param Font Size:
 *     - The font size used for parameter values.
 * 
 *     Show Menu Portraits?:
 *     - If Main Menu Core is installed, display the Menu Portraits instead of
 *       the actor's face in the status window?
 * 
 *     JS: Portrait Upper:
 *     - If Menu Portraits are available, this is code used to draw the upper
 *       data like this in the Status Window.
 * 
 *     JS: Face Upper:
 *     - If faces used used, this is code used to draw the upper data like this
 *       in the Status Window.
 * 
 *     JS: Parameter Lower:
 *     - Code to determine how parameters are drawn in the Status Window.
 *
 *   Layout Style:
 *   - If using an updated layout, how do you want to style the menu scene?
 *     - Upper Help, Left Input
 *     - Upper Help, Right Input
 *     - Lower Help, Left Input
 *     - Lower Help, Right Input
 * 
 *   Status Window Width:
 *   - The usual width of the status window if using the non-Updated Equip
 *     Menu Layout.
 * 
 *   Show Back Rectangles?:
 *   - Show back rectangles of darker colors to display information better?
 * 
 *     Back Rectangle Color:
 *     - Use #rrggbb for custom colors or regular numbers for text colors
 *       from the Window Skin.
 * 
 *   Cursed Equip Popup:
 *   - Text popup appears when an actor equips a cursed weapon/armor.
 *   - Text codes allowed.
 *   - Requires VisuMZ_0_CoreEngine!
 *   - Empty to not use.
 *   -  %1 - Actor, %2 - Equip, %3 - Icon.
 *
 * ---
 *
 * Command Window
 * 
 *   Style:
 *   - How do you wish to draw commands in the Command Window?
 *   - Text Only: Display only the text.
 *   - Icon Only: Display only the icon.
 *   - Icon + Text: Display the icon first, then the text.
 *   - Auto: Determine which is better to use based on the size of the cell.
 * 
 *   Text Align:
 *   - Text alignment for the Command Window.
 * 
 *   Equip Icon:
 *   - The icon used for the Equip command.
 * 
 *     Help Description:
 *     - Help description used when this command is selected.
 *     - Text codes allowed.
 * 
 *   Add Optimize Command?:
 *   - Add the "Optimize" command to the Command Window?
 * 
 *     Help Description:
 *     - Help description used when this command is selected.
 *     - Text codes allowed.
 * 
 *     Optimize Icon:
 *     - The icon used for the Optimize command.
 * 
 *   Add Clear Command?:
 *   - Add the "Clear" command to the Command Window?
 * 
 *     Help Description:
 *     - Help description used when this command is selected.
 *     - Text codes allowed.
 * 
 *     Clear Icon:
 *     - The icon used for the Clear command.
 *
 * ---
 *
 * Remove Equip
 * 
 *   Icon:
 *   - Icon used for equipment removal.
 * 
 *   Text:
 *   - Text used for equipment removal.
 * 
 *   Use SHIFT Shortcut?:
 *   - Add the "Shift" button as a shortcut key to removing items?
 *
 * ---
 *
 * Rulings
 * 
 *   Equip-Adjust HP/MP:
 *   - Adjust HP/MP differences after changing equips with MaxHP/MaxMP values.
 * 
 *   Non-Removable Types:
 *   - Insert ID's of the Equipment Types that must always have an item
 *     equipped and cannot be empty.
 * 
 *   Non-Optimized Types:
 *   - Insert ID's of the Equipment Types that will be ignored when equipment
 *     is being optimized.
 *
 * ---
 *
 * Button Assist Window
 *
 *   SHIFT: Remove:
 *   - Button assist text used for the SHIFT Remove Shortcut.
 *   - For VisuStella MZ's Core Engine's Button Assist Window.
 *
 * ---
 *
 * ============================================================================
 * Plugin Parameters: Shop Menu Settings
 * ============================================================================
 *
 * These Plugin Parameters allow you a number of options to adjust the Shop
 * Menu Scene. These options range from enabling an updated and modern layout,
 * adjust how various key visual aspects appear, and determine how prices can
 * be affected when it comes to selling them or buying them (for coders).
 *
 * ---
 *
 * General
 * 
 *   Use Updated Layout:
 *   - Use the Updated Shop Layout provided by this plugin?
 *   - This will override the Core Engine windows settings.
 *
 *   Layout Style:
 *   - If using an updated layout, how do you want to style the menu scene?
 *     - Upper Help, Left Input
 *     - Upper Help, Right Input
 *     - Lower Help, Left Input
 *     - Lower Help, Right Input
 *
 * ---
 * 
 * Switches:
 * 
 *   Switch: Buy:
 *   - Buying items in the Shop Scene turns this Switch to ON.
 *   - Switch reverts to OFF whenever the Shop Scene opens.
 * 
 *   Switch: Sell
 *   - Selling items in the Shop Scene turns this Switch to ON.
 *   - Switch reverts to OFF whenever the Shop Scene opens.
 * 
 * ---
 *
 * Command Window
 * 
 *   Hide Unavailable?:
 *   - Hide all unavailable commands like when a shop is set to Purchase Only?
 * 
 *   Style:
 *   - How do you wish to draw commands in the Command Window?
 *   - Text Only: Display only the text.
 *   - Icon Only: Display only the icon.
 *   - Icon + Text: Display the icon first, then the text.
 *   - Auto: Determine which is better to use based on the size of the cell.
 * 
 *   Text Align:
 *   - Text alignment for the Command Window.
 * 
 *   Buy Icon:
 *   - The icon used for the Buy command.
 * 
 *   Sell Icon:
 *   - The icon used for the Sell command.
 * 
 *   Cancel Icon:
 *   - The icon used for the Cancel command.
 * 
 *   Rename "Cancel":
 *   - Rename Cancel to something more logical for the Shop Menu Scene.
 *
 * ---
 *
 * Prices
 * 
 *   Sell Price Rate:
 *   - The default sell price rate.
 * 
 *   JS: Buy Price:
 *   - Modificatons made to the buy price before finalizing it.
 * 
 *   JS: Sell Price:
 *   - Modificatons made to the sell price before finalizing it.
 *
 * ---
 *
 * Button Assist Window
 *
 *   Small Increment:
 *   Large Increment:
 *   - Text used for changing amount bought/sold.
 *   - For VisuStella MZ's Core Engine's Button Assist Window.
 *
 * ---
 *
 * ============================================================================
 * Plugin Parameters: Shop Status Window
 * ============================================================================
 *
 * These Plugin Parameters focuses on the Shop Status Window and determines how
 * its data is displayed.
 *
 * ---
 *
 * General
 * 
 *   Window Width:
 *   - The usual width of the status window.
 * 
 *   Parameter Font Size:
 *   - Font size used for parameter changes.
 * 
 *   Translucent Opacity:
 *   - Opacity setting used for translucent window objects.
 * 
 *   Show Back Rectangles?:
 *   - Show back rectangles of darker colors to display information better?
 * 
 *     Back Rectangle Color:
 *     - Use #rrggbb for custom colors or regular numbers for text colors
 *       from the Window Skin.
 *
 * ---
 *
 * Equipment Data
 * 
 *   Data Style:
 *   - How do you wish to display equipment data?
 *     - Compare - Compares selected equip to equipped gear
 *       - Lists all main party actors
 *       - Displays the parameter differences when equipped
 *       - Calculates custom JS values
 *     - Classic - Shows basic parameters of selected equip
 *       - Involves no actors, only shows the item's stats
 *       - Shows weapon or armor specific parameters
 *       - Does not show custom JS values as those are calculated per actor
 *       - Does not show custom parameters as those are calculated per actor
 *       - Use <Status Info> and <Custom Status Info> notetags to overwrite or
 *         add custom data to classic equip data
 *     - Double - Shows basic parameters in double columns
 *       - Involves no actors, only shows the item's stats
 *       - Shows weapon or armor specific parameters
 *       - Does not show custom JS values as those are calculated per actor
 *       - Does not show custom parameters as those are calculated per actor
 *       - Use <Status Info> and <Custom Status Info> notetags to overwrite or
 *         add custom data to classic equip data
 * 
 *     Compare Style:
 * 
 *       Already Equipped:
 *       - Marker used to show an actor cannot equip an item.
 * 
 *       Can't Equip:
 *       - Marker used to show an actor cannot equip an item.
 * 
 *       No Changes:
 *       - Marker used to show no changes have occurred.
 * 
 *       JS: Draw Equip Data:
 *       - Code used to draw the equipment data for the Shop Status Window.
 * 
 *     Classic Style:
 * 
 *       Added Weapon Params:
 *       Added Armor Params:
 *       - Display these parameters when a weapon/armor is selected.
 *       - Requires VisuMZ_0_CoreEngine!
 * 
 *       JS: Draw Equip Data:
 *       - Code used to draw the equipment data for the Shop Status Window.
 * 
 *     Double Style:
 * 
 *       Added Weapon Params:
 *       Added Armor Params:
 *       - Display these parameters when a weapon/armor is selected.
 *       - Requires VisuMZ_0_CoreEngine!
 * 
 *       JS: Draw Equip Data:
 *       - Code used to draw the equipment data for the Shop Status Window.
 * 
 *   Delay MS:
 *   - How many milliseconds (MS) to delay the preview update?
 *   - This is to prevent lag spikes for equips only.
 *
 * ---
 *
 * Item Data
 * 
 *   Max State/Buff Icons:
 *   - Maximum number of icons that can be displayed for Add/Remove
 *     States/Buffs.
 * 
 *   Multiplier Standard:
 *   - Constant standard to filter out random values when calculating the
 *     damage multiplier.
 * 
 *   JS: Draw Item Data:
 *   - Code used to draw the item data for the Shop Status Window.
 *
 * ---
 *
 * Vocabulary
 * 
 *   Consumable:
 *   Occasions:
 *   Scope:
 *   Speed:
 *   Success Rate:
 *   Repeats:
 *   Hit Type:
 *   Element:
 *   Damage Type:
 *   Effects:
 *   - Vocabulary used for these data entries.
 *   - Some of these have Plugin Parameters have sub-entries.
 * 
 *   NOTE: Regarding Damage Labels
 * 
 *   If Visu_1_BattleCore is installed, priority goes to its Damage Style
 *   settings. The label displayed is based on the damage style settings in
 *   place for that specific skill or item.
 * 
 *   Go to Battle Core > Plugin Parameters > Damage Settings > Style List >
 *   pick the damage style you want to edit > Damage Label and change the
 *   text settings you'd like there.
 *
 * ---
 *
 * ============================================================================
 * Terms of Use
 * ============================================================================
 *
 * 1. These plugins may be used in free or commercial games provided that they
 * have been acquired through legitimate means at VisuStella.com and/or any
 * other official approved VisuStella sources. Exceptions and special
 * circumstances that may prohibit usage will be listed on VisuStella.com.
 * 
 * 2. All of the listed coders found in the Credits section of this plugin must
 * be given credit in your games or credited as a collective under the name:
 * "VisuStella".
 * 
 * 3. You may edit the source code to suit your needs, so long as you do not
 * claim the source code belongs to you. VisuStella also does not take
 * responsibility for the plugin if any changes have been made to the plugin's
 * code, nor does VisuStella take responsibility for user-provided custom code
 * used for custom control effects including advanced JavaScript notetags
 * and/or plugin parameters that allow custom JavaScript code.
 * 
 * 4. You may NOT redistribute these plugins nor take code from this plugin to
 * use as your own. These plugins and their code are only to be downloaded from
 * VisuStella.com and other official/approved VisuStella sources. A list of
 * official/approved sources can also be found on VisuStella.com.
 *
 * 5. VisuStella is not responsible for problems found in your game due to
 * unintended usage, incompatibility problems with plugins outside of the
 * VisuStella MZ library, plugin versions that aren't up to date, nor
 * responsible for the proper working of compatibility patches made by any
 * third parties. VisuStella is not responsible for errors caused by any
 * user-provided custom code used for custom control effects including advanced
 * JavaScript notetags and/or plugin parameters that allow JavaScript code.
 *
 * 6. If a compatibility patch needs to be made through a third party that is
 * unaffiliated with VisuStella that involves using code from the VisuStella MZ
 * library, contact must be made with a member from VisuStella and have it
 * approved. The patch would be placed on VisuStella.com as a free download
 * to the public. Such patches cannot be sold for monetary gain, including
 * commissions, crowdfunding, and/or donations.
 *
 * ============================================================================
 * Credits
 * ============================================================================
 * 
 * If you are using this plugin, credit the following people in your game:
 * 
 * Team VisuStella
 * * Yanfly
 * * Arisu
 * * Olivia
 * * Irina
 *
 * ============================================================================
 * Changelog
 * ============================================================================
 * 
 * Version 1.60: March 16, 2026
 * * Documentation Update!
 * ** Help file updated for new features.
 * * New Features!
 * ** New Plugin Parameter added by Arisu:
 * *** Parameters > Item Menu Settings > Use Updated Layout > Disabled: Vanilla
 * **** If NOT using the updated layout, keep all settings to pure vanilla and
 *      ignore other Plugin Parameters.
 * **** Used for those who do not wish to fiddle with the Plugin Parameter
 *      settings to return the item menu back to default MZ vanilla.
 * **** Does not apply if using updated layout.
 * **** The following settings are disabled if using vanilla:
 * ***** Categories: Only the default 4
 * ***** Category Names only display text and no icons
 * ***** No Shop Status Window
 * ***** Modern Controls Disabled
 * ***** Columns: Any -> 2
 * 
 * Version 1.59: October 16, 2025
 * * Bug Fixes!
 * ** Fixed a bug where a changed parameter in the equipment menu would
 *    accidentally highlight the next parameter's name. Fix made by Irina.
 * 
 * Version 1.58: February 20, 2025
 * * Bug Fixes!
 * ** Optimize no longer allows player to bypass the following notetags:
 *    <Equip Copy Limit: x>, <Equip Weapon Type Limit: x>, and
 *    <Equip Armor Type Limit: x>. Fix made by Arisu.
 * * Compatibility Update!
 * ** Updated for RPG Maker MZ Core Scripts 1.9.0!
 * *** Better compatibility with different icon sizes.
 * 
 * Version 1.57: January 16, 2025
 * * Bug Fixes!
 * ** Fixed an issue where certain icons were not aligning properly at
 *    different line height settings. Fix made by Olivia.
 * * Documentation Update!
 * ** Help file updated for new features.
 * ** Added extra clarity for <Status Info> notetag:
 * *** For clarity regarding "Damage Multiplier" and "HP Recovery"/"HP Damage":
 * **** "Damage Multiplier" refers to the amount determined by damage formulas.
 * **** "HP Recovery"/"HP Damage" refers to the "Recover HP" database effect.
 * **** Likewise, the same will apply to "MP Recovery"/"MP Damage" if the
 *      damage formula type is to deal MP recovery/damage instead.
 * * New Features!
 * ** New Notetags added by Arisu:
 * *** <Status Style: type>
 * **** Changes up the way the shop status window displays data for this
 *      database object in particular.
 * *** <Custom Status Parameters: name, name, name>
 * **** Customize which parameters are displayed for this equipment object's
 *      shop status window.
 * * Optimization Update!
 * ** Plugin should run more optimized.
 * 
 * Version 1.56: December 19, 2024
 * * Bug Fixes!
 * ** Fixed a bug where newly added equipment would cause crashes upon
 *    interaction. Fix made by Irina.
 * 
 * Version 1.55: November 14, 2024
 * * Bug Fixes!
 * ** Fixed a bug where you can no longer attempt to equip an actor with zero
 *    equip slots and causing a crash. Fix made by Arisu.
 * * Documentation Update!
 * ** Help file updated for new features.
 * ** Updated <Status Info>
 * *** Used for: Skill, Item, Weapon, Armor Notetags
 * **** Replace 'key' with one of the following for weapons and armors:
 * ***** 'MaxHP', 'MaxMP', 'ATK', 'DEF', 'MAT', 'MDF', 'AGI', or 'LUK'
 * ***** For those with VisuMZ_0_CoreEngine:
 * ****** 'HIT', 'EVA', 'CRI', 'CEV', 'MEV', 'MRF', 'CNT', 'HRG', 'MRG', 'TRG'
 * ****** 'TGR', 'GRD', 'REC', 'PHA', 'MCR', 'TCR', 'PDR', 'MDR', 'FDR', 'EXR'
 * **** Only relevant if the Draw Style for equipment is "classic" or "double".
 * ** Updated <Custom Status Info> notetag:
 * *** Used for: Skill, Item, Weapon, Armor Notetags
 * **** When used with weapon or armor database objects, this information is
 *      only relevant if the Draw Style for equipment is "classic" or "double".
 * * New Feature!
 * ** New Plugin Parameters: 
 * *** Parameters > Shop Status Window > Data Style:
 * **** How do you wish to display equipment data?
 * ***** Compare - Compares selected equip to equipped gear
 * ****** Lists all main party actors
 * ****** Displays the parameter differences when equipped
 * ****** Calculates custom JS values
 * ***** Classic - Shows basic parameters of selected equip
 * ***** Double - Shows basic parameters in double columns
 * ****** Involves no actors, only shows the item's stats
 * ****** Shows weapon or armor specific parameters
 * ****** Does not show custom JS values as those are calculated per actor
 * ****** Does not show custom parameters as those are calculated per actor
 * ****** Use <Status Info> and <Custom Status Info> notetags to overwrite or
 *        add custom data to classic equip data
 * **** Data Style > Classic Style:
 * **** Data Style > Double Style:
 * ***** Added Weapon Params
 * ***** Added Armor Params
 * ****** Display these parameters when a weapon/armor is selected.
 * ****** Requires VisuMZ_0_CoreEngine!
 * 
 * Version 1.54: October 17, 2024
 * * Feature Update!
 * ** If "Modern Controls" is selected while "Remove Equip" and "Optimize" are
 *    gone from the Equip Menu, right click will exit the menu. Feature added
 *    by Arisu.
 * 
 * Version 1.53: July 18, 2024
 * * Documentation Update!
 * ** Help file updated for new features.
 * ** Added warning to <JS Parameters>:
 * *** If you are trying to calculate a value based off a full parameter value,
 *     such as "ATK = user.atk * 0.10", it's going to break and will cause an
 *     infinite loop. Use base parameter values instead.
 * * New Features!
 * ** New notetags added by Irina:
 * *** <ID Sort Priority: x>
 * **** Used for Scene_Item, Scene_Equip, Scene_Battle, and Scene_Shop's sell
 *      option (only when selling).
 * **** Changes sorting priority by ID for item, weapon, or armor to 'x'. 
 * **** Default priority level is '50'.
 * **** Items, weapons, and armors with higher priority values will be sorted
 *      higher up on the list while lower values will be lower on the list.
 * 
 * Version 1.52: May 16, 2024
 * * Documentation Update!
 * ** Help file updated for new features.
 * * New Features!
 * ** New Plugin Parameter added by Irina:
 * *** Parameters > Item Categories > Category List > Category > Sorted By:
 * **** You can now sort specific item categories by ID or Name.
 * **** Only usable within Scene_Item and Scene_Shop.
 * * Optimization Update!
 * ** Plugin should run more optimized.
 * 
 * Version 1.51: December 14, 2023
 * * Bug Fixes!
 * ** Fixed a bug where if an item is unequipped, it may cause a crash. Fix
 *    made by Arisu.
 * ** Fixed a bug where <Proxy: id> did not properly give the proxy item. Fix
 *    made by Arisu.
 * 
 * Version 1.50: November 16, 2023
 * * Bug Fixes!
 * ** <JS Buy Price> and <JS Sell Price> was not working properly. Fix made
 *    by Irina.
 * * Documentation Update!
 * ** Help file updated for new features.
 * * Optimization Update!
 * ** Plugin should run more optimized.
 * * New Features!
 * ** New Notetags added by Arisu:
 * *** <Cursed>
 * **** If this weapon or armor is equipped, it cannot manually be removed by
 *      the player until it is purified.
 * **** To remove it, it must be done by event commands, script calls, or
 *     through the Purify-related Plugin Commands provided by this plugin.
 * **** Once purified, the weapon or armor will become unequipped unless it has
 *     a purify transformation.
 * **** If the newly transformed weapon/armor is equippable, it will remain in
 *     the actor's equipment slots.
 * **** If you are using VisuMZ_2_WeaponSwapSystem, weapons cannot become
 *      cursed in order to allow free weapon swapping. Weaponry will not be
 *      cursed if VisuMZ_2_WeaponSwapSystem is installed.
 * *** <Purify Transform: id>
 * *** <Purify Transform: name>
 * **** If this notetag is present on a <Cursed> weapon or armor, then upon the
 *      actor receiving purification, the weapon or armor will transform into a
 *      different item.
 * ** New Plugin Commands added by Arisu:
 * *** Purify: Target Actor(s)
 * **** Purifies target actor(s) of any cursed weapons or armors.
 * *** Purify: Whole Party
 * **** Purifies whole party of any cursed weapons or armors.
 * ** Added "Cursed Equip Popup" to Equip Scene Plugin Parameters.
 * *** Text popup appears when an actor equips a cursed weapon/armor.
 * ** Added "Ally or Enemy" or "Enemy or Ally" scopes to Shop Status Window
 *    Plugin Parameters.
 * *** If unused, will default to "1 Ally" or "1 Enemy" like usual.
 *     Added by Irina.
 * 
 * Version 1.49: October 12, 2023
 * * Bug Fixes!
 * ** Fixed a problem where for weapon types, all weapon types are listed in
 *    the equip menu even when the actor cannot equip them (though they would
 *    be disabled). Fix made by Olivia.
 * * Documentation Update!
 * ** Help file updated for new features.
 * * New Features!
 * ** New notetags added by Olivia and sponsored by AndyL:
 * *** <Added EType: x>
 * *** <Added ETypes: x, x, x>
 * **** This is for armors only and does NOT work with weapons!
 * **** Allows a piece of armor to belong to multiple ETypes. This means a
 *      glove can be equipped as "Armgear" or as an "Accessory" if you so
 *      choose.
 * 
 * Version 1.48: September 14, 2023
 * * Bug Fixes!
 * ** Fixed a problem where the help window position of the non-updated layout
 *    would appear in the wrong position. Fix made by Irina.
 * * Optimization Update!
 * ** Plugin should run more optimized when weapons and armors exceed 2000
 *    in database quantity.
 * 
 * Version 1.47: July 13, 2023
 * * Bug Fixes!
 * ** Fixed a bug that would cause the shop status window to display incorrect
 *    removed buffs and debuffs. Fix made by Olivia.
 * * Compatibility Update!
 * ** Added compatibility functionality for future plugins.
 * * Documentation Update!
 * ** Help file updated for new features.
 * * Feature Updates!
 * ** Changes made to dynamic shop listings in order to update upon listing
 *    changes rather than having to enter and exit the shop again. Update made
 *    by Arisu.
 * * New Features!
 * ** New notetag effects added by Arisu and sponsored by AndyL:
 * *** <Conserve: x%>
 * **** Gives the item a percent chance when used to not consume the item.
 * *** <Buy Turn On Switches: x, x, x>
 * *** <Buy Turn Off Switches: x, x, x>
 * *** <Sell Turn On Switches: x, x, x>
 * *** <Sell Turn Off Switches: x, x, x>
 * **** When buying/selling an item, weapon, or armor with these notetags,
 *      turn on/off switch(es) 'x'.
 * *** New Plugin Parameters added by Arisu:
 * **** Params > Settings > Shop Status Window > Equipment Data > Delay MS:
 * ***** How many milliseconds (MS) to delay the preview update?
 * ***** This is to prevent lag spikes for equips only.
 * 
 * Version 1.46: June 15, 2023
 * * Bug Fixes!
 * ** Fixed a problem where the help and input modes are not adequately
 *    adjusted when not used with the updated layout or without the Options
 *    Core custom UI placement. Fix made by Arisu.
 * 
 * Version 1.45: May 18, 2023
 * * Bug Fixes!
 * ** Fixed a bug that would cause equip slots to not be recognized properly if
 *    the equip slot name ends in a space.
 * 
 * Version 1.44: April 13, 2023
 * * Documentation Update!
 * ** Help file updated for new features.
 * * New Features!
 * ** New notetag effects added by Arisu and sponsored by Anon:
 * *** <Equip For Class Only: x>
 * *** <Equip For Classes Only: x, x, x>
 * *** <Equip For Class Only: name>
 * *** <Equip For Classes Only: name, name, name>
 * **** The piece of equipment can only be worn by the listed classes.
 * *** <Equip Requirements> notetag added.
 * **** Define multiple requirements that the actor needs to meet in order for
 *      this equip item to be equippable.
 * **** See help file for more information on the types of requirements that
 *      can be added.
 * 
 * Version 1.43: March 16, 2023
 * * Bug Fixes!
 * ** Artifact armors should now update and refresh the party members' cache
 *    upon acquisition. Fix made by Olivia.
 * 
 * Version 1.42: February 16, 2023
 * * Bug Fixes!
 * ** Proxy items should no longer cause infinite loops if they're made to
 *    reference other proxy items in a circular fashion. Instead, they just
 *    give the exact first found proxy instead of cycling through others.
 *    Fix made by Arisu.
 * 
 * Version 1.41: December 15, 2022
 * * Compatibility Update!
 * ** Added compatibility functionality for future plugins.
 * * Documentation Update!
 * ** Help file updated for new features.
 * * New Features!
 * ** New Plugin Parameters added by Arisu and sponsored by Anon:
 * *** Equip Scene > Equip Command > Help Description
 * *** Equip Scene > Optimize Command > Help Description
 * *** Equip Scene > Clear Command > Help Description
 * **** Help description used when these commands are selected.
 * 
 * Version 1.40: October 20, 2022
 * * Feature Update!
 * ** For the shop status window, when comparing equipment of a type where
 *    there are multiple equipment slots (such as accessories), the plugin will
 *    now check for an empty equipment slot first and then make calculations
 *    there. Otherwise, it will use the first available equipment slot of that
 *    type regardless of the equipped item. Update made by Arisu.
 * * Optimization Update!
 * ** Plugin should run more optimized.
 * 
 * Version 1.39: September 29, 2022:
 * * Feature Update!
 * ** Changed the default code for the equip scene's status window display to
 *    prevent the face graphic and basic actor stats from going above the
 *    window boundaries if there are too many parameters displayed in the
 *    status window at a time.
 * ** If you already have this plugin installed the changes will not be
 *    reflected unless you do the following:
 * **** BACKUP your game project.
 * **** REMOVE this plugin from the Plugin Manager list.
 * **** REINSTALL this plugin into the Plugin Manager list.
 * **** SAVE the game project.
 * 
 * Version 1.38: March 3, 2022
 * * Documentation Update!
 * ** Help file updated for new features.
 * * New Features!
 * ** New mechanics and notetags added by Olivia and sponsored by Anon:
 * *** <Party Artifact>
 * *** <Troop Artifact>
 * *** <Stackable Party Artifact>
 * *** <Stackable Troop Artifact>
 * **** Armors only! This armor cannot be equipped at all. However, by simply
 *      being in the party's inventory, its parameter bonuses and traits will
 *      be applied globally throughout the whole party or troop (depending on
 *      the notetag). Add both notetags to affect both groups.
 * **** The normal versions of the notetag is only applied once regardless of
 *      the number of copies are found in the party's inventory.
 * **** The stackable versions of the notetag will have the bonuses and traits
 *      stacked multiple times relative to the number of copies found in the
 *      party's inventory.
 * **** This item will NOT be added during the setup phase for Battle Tests.
 * ***** If you want to add the item, do it manually.
 * 
 * Version 1.37: December 23, 2021
 * * Compatibility Update
 * ** Created foundation for proxy items to be used in any applicable system
 *    and extension plugins. Update made by Arisu.
 * 
 * Version 1.36: December 2, 2021
 * * Feature Update!
 * ** For those using custom parameters from the Core Engine and do not have
 *    the parameters all-capitalized, the plugin will automatically do it for
 *    you to prevent errors. Update made by Olivia.
 * 
 * Version 1.35: November 18, 2021
 * * Compatibility Update!
 * ** If this plugin's updated scene is disabled, the Help Window locations for
 *    the Item, Equip, and Shop Scenes should now be at their designated
 *    locations as calculated by the VisuMZ Core Engine instead of the RMMZ
 *    default location. Update made by Irina.
 * 
 * Version 1.34: October 28, 2021
 * * Feature Update
 * ** Added fail safe checks for projects that are using old data for starting
 *    equipment that no longer exist, thus preventing the game from opening.
 *    Update made by Arisu.
 * 
 * Version 1.33: August 6, 2021
 * * Documentation Update!
 * ** Removed "Weapon" and "Armor" from "Used For" for <Status Info>. This was
 *    an unintended piece of documentation.
 * 
 * Version 1.32: July 23, 2021
 * * Bug Fixes!
 * ** Fixed a bug that would cause armor duplication when changing to classes
 *    with unaligned equipment slot types. Fix made by Arisu.
 * 
 * Version 1.31: July 9, 2021
 * * Feature Update!
 * ** Added a failsafe for price manipulation JavaScript to never have prices
 *    drop below 0 if possible. Update made by Arisu.
 * 
 * Version 1.30: July 2, 2021
 * * Documentation Update!
 * ** Added an extra note to the help file for the following:
 *    Plugin Parameters > Item Menu Settings > List Window > Columns
 * *** If you are using the VisuStella MZ Core Engine and the "Modern Controls"
 *     Plugin Parameter, please read through that section in case you have any
 *     questions about how to switch between categories when using multiple
 *     columns of items at a time.
 *    
 * 
 * Version 1.29: June 25, 2021
 * * Compatibility Update
 * ** Compatibility update with RPG Maker MZ 1.3.0+.
 * * Feature Update!
 * ** Phantom data when changing equipment types in the database should no
 *    longer affect actors with cached equip ID's. Update made by Arisu.
 * 
 * Version 1.28: June 4, 2021
 * * Optimization Update!
 * ** Plugin should run more optimized.
 * 
 * Version 1.27: May 21, 2021
 * * Bug Fixes!
 * ** Using the mouse right click in the Equip Scene while inside of the item
 *    to slot window will no longer exit the Equip Scene. Fix made by Yanfly.
 * 
 * Version 1.26: April 30, 2021
 * * Documentation Update!
 * ** Help file updated for new features.
 * ** "VisuStella MZ Compatibility" added VisuMZ_1_BattleCore section regarding
 *    Damage Multiplier and Healing Multiplier vocabulary settings to reduce
 *    commonly asked questions.
 * * New Features!
 * ** New notetags added by Irina and sponsored by Archeia:
 * *** <Shop Picture Name: filename>
 * *** <Shop Picture Layer: x>
 * *** <Shop Picture Max Width: x>
 * *** <Shop Picture Max Height: y>
 * *** <Shop Picture Max Dimensions: x, y>
 * *** <Shop Picture Alignment: x>
 * *** <Shop Picture Position: y>
 * *** <Shop Picture Offset X: +x>
 * *** <Shop Picture Offset X: -x>
 * *** <Shop Picture Offset Y: +y>
 * *** <Shop Picture Offset Y: -y>
 * *** <Shop Picture Offset: +x, +y>
 * *** <Shop Picture Offset: -x, -y>
 * *** <Shop Picture Opacity: x>
 * *** <Shop Picture Opacity: x%>
 * **** Add images from the game project's img/pictures/ folder to display in
 *      the Shop Status Window.
 * 
 * Version 1.25: April 23, 2021
 * * Documentation Update!
 * ** Added clarity to the <param: +x> and <param: -x> notetags:
 * *** These notetags do NOT work with X Parameters, S Parameters, or any
 *     custom parameters. These notetags ONLY work with the base parameters.
 * 
 * Version 1.24: April 16, 2021
 * * Bug Fixes!
 * ** Changing an actor's equipment slots to past their original amount will no
 *    longer yield errors with duplicate slot types. Fix made by Arisu.
 * ** Completely selling an item should now refresh the help window to the new
 *    selected item's help description. Fix made by Arisu.
 * * Optimization Update!
 * ** Non-removable equipment restrictions for the equipment scene are now
 *    better optimized. Update made by Olivia.
 * 
 * Version 1.23: April 2, 2021
 * * Optimization Update!
 * ** Plugin should run more optimized.
 * 
 * Version 1.22: March 26, 2021
 * * Documentation Update!
 * ** Added "VisuStella MZ Compatibility" section for detailed compatibility
 *    explanations with the VisuMZ_4_BreakShields plugin.
 * 
 * Version 1.21: March 5, 2021
 * * Feature Update!
 * ** Custom equipment slots are disabled during Battle Testing for better
 *    accuracy and results.
 * 
 * Version 1.20: February 26, 2021
 * * Documentation Update!
 * ** Help file updated for new features.
 * * New Features!
 * ** New Plugin Parameters added by Irina and sponsored by AndyL:
 * *** Plugin Parameters > Shop Menu Settings > Switches > Switch: Buy
 * *** Plugin Parameters > Shop Menu Settings > Switches > Switch: Sell
 * **** Buying/selling items in the Shop Scene turns this Switch to ON.
 * **** Switch reverts to OFF whenever the Shop Scene opens.
 * **** These switches can be used after a "Shop Processing" event command to
 *      determine if the player has bought an item, bought and sold an item,
 *      sold an item, or neither.
 * 
 * Version 1.19: January 29, 2021
 * * Documentation Update!
 * ** Help file updated for new features.
 * * New Features!
 * ** New notetags added by Irina.
 * *** <Equip Copy Limit: x>
 * **** Sets a maximum number of copies that the actor can wear of this
 *      equipment. Usage Example: Actors can only equip one copy of the
 *      "One-of-a-Kind Ring" on at any time despite having empty accessory
 *      slots because the ring has a <Equip Copy Limit: 1> notetag.
 * *** <Equip Weapon Type Limit: x>
 * **** This weapon cannot be equipped with other weapons of the same type once
 *      the limited amount has been reached. Usage Example: A dualwielding
 *      warrior who can only equip one sword and a dagger but never two swords
 *      or two daggers because the swords and daggers all have the
 *      <Equip Weapon Type Limit: 1> notetags on them.
 * *** <Equip Armor Type Limit: x>
 * **** This armor cannot be equipped with other armors of the same type once
 *      the limited amount has been reached. Usage Example: People cannot equip
 *      more than two glove accessories on at a time because the glove is a
 *      "Glove" armor-type and each glove item has the
 *      <Equip Armor Type Limit: 2> notetags on them.
 * 
 * Version 1.18: January 15, 2021
 * * Bug Fixes!
 * ** Pressing "Shift" to remove equipment will now refresh the status window
 *    unlike before. Fix made by Olivia.
 * * Documentation Update!
 * ** Help file updated for new features.
 * * New Feature!
 * ** New Plugin Parameters added
 * *** Plugin Parameters > Item Menu Settings > Background Type
 * 
 * Version 1.17: January 1, 2021
 * * Bug Fixes!
 * ** Equipping should be working properly again. Fix made by Yanfly.
 * 
 * Version 1.16: December 25, 2020
 * * Bug Fixes!
 * ** Equip-Adjust HP/MP should work properly now. Fix made by Yanfly.
 * * Documentation Update!
 * ** Added more clarity for <JS Item Enable> to state that if the VisuStella
 *    Battle Core is installed, then all battle scope items are visible, but
 *    not necessarily enabled if they are disabled otherwise.
 * 
 * Version 1.15: December 18, 2020
 * * Bug Fixes!
 * ** RPG Maker MZ Bug: Unusable items on an individual-actor basis will no
 *    longer be overwritten by party-based usability for battle. Fix by Yanfly.
 * * Documentation Update!
 * ** Added more clarity for <JS Item Enable> to state that it removes the
 *    usable item from visibility as well if the actor unable to use it is the
 *    only person in the party.
 * 
 * Version 1.14: December 11, 2020
 * * Compatibility Update!
 * ** Added compatibility functionality for future plugins.
 * 
 * Version 1.13: December 4, 2020
 * * Documentation Update!
 * ** Added documentation for new feature(s)!
 * * New Features!
 * ** New Plugin Commands added by Arisu!
 * *** Actor: Change Equip Slots
 * *** Actor: Reset Equip Slots
 * **** These plugin commands allow you to forcefully change the equip slots
 *      available to an actor regardless of the slots provided by its class as
 *      well as reset them.
 * 
 * Version 1.12: November 15, 2020
 * * Optimization Update!
 * ** Plugin should run more optimized.
 * 
 * Version 1.11: November 8, 2020
 * * Bug Fix!
 * ** Font size ratio for the shop status window now scales to a hard coded
 *    value to prevent smaller font sizes from expanding icon sizes. Fix made
 *    by Arisu.
 * * Feature Update!
 * ** Currency display in the shop menu is now reflected upon how the plugin
 *    parameters set them to display. Update made by Arisu.
 * 
 * Version 1.10: November 1, 2020
 * * Feature Update!
 * ** Modern Controls compatibility with Core Engine no longer enables the
 *    Item Categories window and child classes to utilize the Home/End keys.
 * 
 * Version 1.09: October 25, 2020
 * * Bug Fixes!
 * ** "All Items" category should now display the "Items" text. Fix by Irina.
 * ** WType, AType, and EType categories now work with text. Fix by Irina.
 *
 * Version 1.08: October 18, 2020
 * * Compatibility Update!
 * ** Plugins should be more compatible with one another.
 * 
 * Version 1.07: October 11, 2020
 * * Bug Fixes!
 * ** XParams and SParams in the Window_EquipStatus window will no longer show
 *    a non-percentile difference if the original value is not a whole value.
 *    Fix made by Yanfly.
 * 
 * Version 1.06: October 4, 2020
 * * Bug Fixes!
 * ** Select Item event command now displays the default amount of columns
 *    instead of whatever setting is made with the plugin parameters.
 * 
 * Version 1.05: September 27, 2020
 * * Bug Fixes!
 * ** When using the updated shop layout, leaving the sell option will no
 *    longer cause the dummy window to appear.
 * * Documentation Update
 * ** "Use Updated Layout" plugin parameters now have the added clause:
 *    "This will override the Core Engine windows settings." to reduce
 *    confusion. Added by Irina.
 * 
 * Version 1.04: September 13, 2020
 * * Bug Fixes!
 * ** Pressing Shift to quickly remove equipment should no longer crash the
 *    game. This will also clear the help window text. Fix made by Arisu.
 * 
 * Version 1.03: September 6, 2020
 * * Bug Fixes!
 * ** If both Optimize and Clear commands have been removed and using modern
 *    controls, pressing up at the top of the slot window list will not go to
 *    the window. Fix made by Yanfly.
 * ** If both Optimize and Clear commands have been removed, the window will no
 *    longer appear and the slot window will be moved upward to fill any empty
 *    spaces. Fix made by Yanfly.
 * * New Features!
 * ** New Plugin Parameter added in NEW! Label to let you adjust the font face.
 * ** New Plugin Parameters added in Equip Menu Scene Settings for disabling
 *    the back rectangles and/or changing their colors.
 * ** New Plugin Parameters added in Shop Status Window Settings for disabling
 *    the back rectangles and/or changing their colors.
 * 
 * Version 1.02: August 30, 2020
 * * Documentation Fix!
 * ** Added: NOTE: Regarding Damage Labels
 * *** If Visu_1_BattleCore is installed, priority goes to its Damage Style
 *   settings. The label displayed is based on the damage style settings in
 *   place for that specific skill or item.
 * *** Go to Battle Core > Plugin Parameters > Damage Settings > Style List >
 *   pick the damage style you want to edit > Damage Label and change the
 *   text settings you'd like there.
 * *** Documentation update added by Yanfly.
 * 
 * Version 1.01: August 23, 2020
 * * Added failsafe to prevent non-existent equipment (because the database
 *   entries have been deleted) from being equipped as initial equipment.
 *   Fix made by Olivia.
 *
 * Version 1.00: August 20, 2020
 * * Finished Plugin!
 *
 * ============================================================================
 * End of Helpfile
 * ============================================================================
 *
 * @ --------------------------------------------------------------------------
 *
 * @command Separator_Begin
 * @text -
 * @desc -
 *
 * @ --------------------------------------------------------------------------
 *
 * @command ActorChangeEquipSlots
 * @text Actor: Change Equip Slots
 * @desc Forcefully change the actor(s) equip slots.
 * These will persist through class changes.
 *
 * @arg Actors:arraynum
 * @text Actor ID(s)
 * @type actor[]
 * @desc Select which Actor ID(s) to affect.
 * @default ["1"]
 * 
 * @arg Slots:arraystr
 * @text Equip Slots
 * @type string[]
 * @desc Insert the equip slots you want the actor(s) to have.
 * These entries are case-sensitive.
 * @default ["Weapon","Shield","Head","Body","Accessory"]
 *
 * @ --------------------------------------------------------------------------
 *
 * @command ActorResetEquipSlots
 * @text Actor: Reset Equip Slots
 * @desc Reset any forced equip slots for the actor(s).
 * Equip slots will then be based on class.
 *
 * @arg Actors:arraynum
 * @text Actor ID(s)
 * @type actor[]
 * @desc Select which Actor ID(s) to affect.
 * @default ["1"]
 *
 * @ --------------------------------------------------------------------------
 *
 * @command Separator_Purify
 * @text -
 * @desc -
 *
 * @ --------------------------------------------------------------------------
 *
 * @command PurifyActors
 * @text Purify: Target Actor(s)
 * @desc Purifies target actor(s) of any cursed weapons or armors.
 * Cannot be used in battle.
 *
 * @arg Actors:arraynum
 * @text Actor ID(s)
 * @type actor[]
 * @desc Select which Actor ID(s) to affect.
 * @default ["1"]
 *
 * @ --------------------------------------------------------------------------
 *
 * @command PurifyParty
 * @text Purify: Whole Party
 * @desc Purifies whole party of any cursed weapons or armors.
 * Cannot be used in battle.
 *
 * @ --------------------------------------------------------------------------
 *
 * @command Separator_Shop
 * @text -
 * @desc -
 *
 * @ --------------------------------------------------------------------------
 *
 * @command BatchShop
 * @text Shop: Advanced
 * @desc Make it easier to put together inventories for a shop.
 * WARNING: Does not allow for event-specific prices.
 *
 * @arg Step1
 * @text Step 1: Item ID's
 *
 * @arg Step1Start:num
 * @text Range Start
 * @parent Step1
 * @type item
 * @desc Select which Item ID to start from.
 * @default 1
 *
 * @arg Step1End:num
 * @text Range End
 * @parent Step1
 * @type item
 * @desc Select which Item ID to end at.
 * @default 4
 *
 * @arg Step2
 * @text Step 2: Weapon ID's
 *
 * @arg Step2Start:num
 * @text Range Start
 * @parent Step2
 * @type weapon
 * @desc Select which Weapon ID to start from.
 * @default 1
 *
 * @arg Step2End:num
 * @text Range End
 * @parent Step2
 * @type weapon
 * @desc Select which Weapon ID to end at.
 * @default 4
 *
 * @arg Step3
 * @text Step 3: Armor ID's
 *
 * @arg Step3Start:num
 * @text Range Start
 * @parent Step3
 * @type armor
 * @desc Select which Armor ID to start from.
 * @default 1
 *
 * @arg Step3End:num
 * @text Range End
 * @parent Step3
 * @type armor
 * @desc Select which Armor ID to end at.
 * @default 4
 *
 * @arg PurchaseOnly:eval
 * @text Step 4: Purchase Only?
 * @type boolean
 * @on Purchase-Only
 * @off Sell Accessible
 * @desc Make the shop purchase-only?
 * @default false
 * 
 * @arg Optional
 * 
 * @arg Blacklist:arraystr
 * @text Blacklisted Categories
 * @parent Optional
 * @type string[]
 * @desc A list of categories to blacklist from the shop.
 * Not used if empty. Mark categories with <Category: x>
 * @default []
 * 
 * @arg Whitelist:arraystr
 * @text Whitelisted Categories
 * @parent Optional
 * @type string[]
 * @desc A list of categories to whitelist for the shop.
 * Not used if empty. Mark categories with <Category: x>
 * @default []
 *
 * @ --------------------------------------------------------------------------
 *
 * @command Separator_End
 * @text -
 * @desc -
 *
 * @ --------------------------------------------------------------------------
 *
 * @ ==========================================================================
 * @ Plugin Parameters
 * @ ==========================================================================
 *
 * @param BreakHead
 * @text --------------------------
 * @default ----------------------------------
 *
 * @param ItemsEquipsCore
 * @default Plugin Parameters
 *
 * @param ATTENTION
 * @default READ THE HELP FILE
 *
 * @param BreakSettings
 * @text --------------------------
 * @default ----------------------------------
 *
 * @param ItemScene:struct
 * @text Item Menu Settings
 * @type struct<ItemScene>
 * @desc Change the Item Menu Scene settings.
 * @default {"General":"","EnableLayout:eval":"true","LayoutStyle:str":"upper/left","ListWindow":"","ListWindowCols:num":"1","ItemQt":"","MaxItems:num":"99","MaxWeapons:num":"99","MaxArmors:num":"99","ItemQuantityFmt:str":"×%1","ItemQuantityFontSize:num":"22","ShopStatusWindow":"","ShowShopStatus:eval":"true","ItemSceneAdjustItemList:eval":"true","ItemMenuStatusRect:func":"\"const width = this.statusWidth();\\nconst height = this._itemWindow.height;\\nconst x = Graphics.boxWidth - width;\\nconst y = this._itemWindow.y;\\nreturn new Rectangle(x, y, width, height);\"","ButtonAssist":"","buttonAssistCategory:str":"Switch Category"}
 *
 * @param Categories:struct
 * @text Item Categories
 * @parent ItemScene:struct
 * @type struct<Categories>
 * @desc Change the categories displayed in the Item/Shop menus.
 * @default {"MainList":"","List:arraystruct":"[\"{\\\"Type:str\\\":\\\"FieldUsable\\\",\\\"Icon:num\\\":\\\"208\\\"}\",\"{\\\"Type:str\\\":\\\"BattleUsable\\\",\\\"Icon:num\\\":\\\"218\\\"}\",\"{\\\"Type:str\\\":\\\"NeverUsable\\\",\\\"Icon:num\\\":\\\"302\\\"}\",\"{\\\"Type:str\\\":\\\"AllWeapons\\\",\\\"Icon:num\\\":\\\"97\\\"}\",\"{\\\"Type:str\\\":\\\"EType:2\\\",\\\"Icon:num\\\":\\\"128\\\"}\",\"{\\\"Type:str\\\":\\\"EType:3\\\",\\\"Icon:num\\\":\\\"131\\\"}\",\"{\\\"Type:str\\\":\\\"EType:4\\\",\\\"Icon:num\\\":\\\"137\\\"}\",\"{\\\"Type:str\\\":\\\"EType:5\\\",\\\"Icon:num\\\":\\\"145\\\"}\",\"{\\\"Type:str\\\":\\\"KeyItems\\\",\\\"Icon:num\\\":\\\"195\\\"}\"]","Style:str":"icon","TextAlign:str":"center","Vocabulary":"","HiddenItemA:str":"Special Items","HiddenItemB:str":"Unique Items","Consumable:str":"Consumable","Nonconsumable:str":"Nonconsumable","AlwaysUsable:str":"Usable","BattleUsable:str":"Battle","FieldUsable:str":"Field","NeverUsable:str":"Materials"}
 *
 * @param New:struct
 * @text NEW! Labels
 * @parent ItemScene:struct
 * @type struct<NewLabel>
 * @desc Change how NEW! Labels apply to your game project.
 * @default {"Enable:eval":"true","Icon:num":"0","Text:str":"NEW!","FontColor:str":"17","FontFace:str":"Verdana","FontSize:str":"16","FadeLimit:num":"360","FadeSpeed:num":"4","OffsetX:num":"0","OffsetY:num":"4"}
 *
 * @param EquipScene:struct
 * @text Equip Menu Settings
 * @type struct<EquipScene>
 * @desc Adjust the settings regarding the Equip Menu Scene.
 * @default {"General":"","EnableLayout:eval":"true","ParamValueFontSize:num":"22","MenuPortraits:eval":"true","DrawPortraitJS:func":"\"// Declare Variables\\nconst lineHeight = this.lineHeight();\\nconst padding = this.itemPadding();\\nconst x1 = padding;\\nconst x2 = this.innerWidth - 128 - padding;\\n\\n// Draw Menu Image\\nthis.drawItemActorMenuImage(this._actor, 0, 0, this.innerWidth, this.innerHeight);\\n\\n// Draw Data\\nthis.drawActorName(this._actor, x1, lineHeight * 0);\\nthis.drawActorClass(this._actor, x1, lineHeight * 1);\\nthis.drawActorIcons(this._actor, x1, lineHeight * 2);\\nthis.drawActorLevel(this._actor, x2, lineHeight * 0);\\nthis.placeBasicGauges(this._actor, x2, lineHeight * 1);\"","DrawFaceJS:func":"\"// Declare Variables\\nconst lineHeight = this.lineHeight();\\nconst gaugeLineHeight = this.gaugeLineHeight();\\nconst x = Math.floor(this.innerWidth / 2);\\nconst limitHeight = this.innerHeight - (this.actorParams().length * lineHeight);\\nconst actorX = Math.floor((x - ImageManager.faceWidth) / 2);\\nconst actorY = Math.max(0, Math.floor((limitHeight - ImageManager.faceHeight) / 2));\\nlet dataHeight = lineHeight * 3;\\ndataHeight += gaugeLineHeight * ($dataSystem.optDisplayTp ? 3 : 2);\\nconst dataY = Math.max(0, Math.floor((limitHeight - dataHeight) / 2));\\n\\n// Draw Data\\nthis.drawActorFace(this._actor, actorX, actorY, ImageManager.faceWidth, ImageManager.faceHeight);\\nthis.drawActorIcons(this._actor, actorX + 16, actorY + ImageManager.faceHeight - lineHeight);\\nthis.drawActorName(this._actor, x, dataY + lineHeight * 0);\\nthis.drawActorLevel(this._actor, x, dataY + lineHeight * 1);\\nif (this.innerHeight > 444) {\\n    this.drawActorClass(this._actor, x, dataY + lineHeight * 2);\\n    this.placeBasicGauges(this._actor, x, dataY + lineHeight * 3);\\n} else {\\n    this.placeBasicGauges(this._actor, x, dataY + lineHeight * 2);\\n}\"","DrawParamJS:func":"\"// Declare variables\\nconst params = this.actorParams();\\nconst lineHeight = this.lineHeight();\\nconst padding = this.itemPadding();\\nconst baseX = 0;\\nconst baseY = this.innerHeight - params.length * lineHeight;\\nconst baseWidth = this.innerWidth;\\nconst valueFontSize = this.paramValueFontSize();\\n\\n// Calculate Widths\\nlet paramNameWidth = Math.max(...params.map(param => this.textWidth(TextManager.param(param))));\\nparamNameWidth += padding * 2;\\nif (this.isUseParamNamesWithIcons()) {\\n    const standardWidth = ImageManager.standardIconWidth || 32;\\n    paramNameWidth += standardWidth + 4;\\n}\\nlet arrowWidth = this.rightArrowWidth();\\nconst totalDivides = this.innerWidth >= 500 ? 3 : 2;\\nlet paramValueWidth = Math.floor((baseWidth - paramNameWidth - arrowWidth) / totalDivides);\\nparamNameWidth = baseWidth - (paramValueWidth * totalDivides) - arrowWidth;\\n\\n// Draw Parameters\\nlet x = baseX;\\nlet y = baseY;\\nlet value = 0;\\nlet diffValue = 0;\\nlet alter = 2;\\nfor (const paramId of params) {\\n    // Draw Param Name\\n    this.drawItemDarkRect(x, y, paramNameWidth, lineHeight, alter);\\n    this.drawUpdatedParamName(paramId, x, y, paramNameWidth);\\n    this.resetFontSettings();\\n    x += paramNameWidth;\\n\\n    // Draw Param Before\\n    this.contents.fontSize = valueFontSize;\\n    this.drawItemDarkRect(x, y, paramValueWidth, lineHeight, alter);\\n    this.drawUpdatedBeforeParamValue(paramId, x, y, paramValueWidth);\\n    this.resetFontSettings();\\n    x += paramValueWidth;\\n\\n    // Draw Arrow\\n    this.drawItemDarkRect(x, y, arrowWidth, lineHeight, alter);\\n    this.drawRightArrow(x, y);\\n    x += arrowWidth;\\n\\n    // Draw Param After\\n    this.contents.fontSize = valueFontSize;\\n    this.drawItemDarkRect(x, y, paramValueWidth, lineHeight, alter);\\n    this.drawUpdatedAfterParamValue(paramId, x, y, paramValueWidth);\\n    x += paramValueWidth;\\n\\n    // Draw Param Change\\n    if (totalDivides > 2) {\\n        this.drawItemDarkRect(x, y, paramValueWidth, lineHeight, alter);\\n        this.drawUpdatedParamValueDiff(paramId, x, y, paramValueWidth);\\n    }\\n\\n    // Prepare Next Parameter\\n    x = baseX;\\n    y += lineHeight;\\n    alter = alter === 2 ? 1 : 2;\\n}\"","LayoutStyle:str":"upper/right","StatusWindowWidth:num":"312","DrawBackRect:eval":"true","BackRectColor:str":"19","CursedTextPopup:json":"\"%1 is cursed by %3%2!\"","Command":"","CmdStyle:str":"auto","CmdTextAlign:str":"center","CmdIconEquip:num":"136","equipCmdDesc:json":"\"Pick and choose equipment to change.\"","CommandAddOptimize:eval":"true","optimizeCmdDesc:json":"\"Equip the strongest available equipment.\"","CmdIconOptimize:num":"137","CommandAddClear:eval":"true","clearCmdDesc:json":"\"Remove all available equipment.\"","CmdIconClear:num":"135","RemoveEquip":"","RemoveEquipIcon:num":"16","RemoveEquipText:str":"Remove","ShiftShortcutKey:eval":"true","Rulings":"","EquipAdjustHpMp:eval":"true","NonRemoveETypes:arraynum":"[]","NonOptimizeETypes:arraynum":"[]","ButtonAssist":"","buttonAssistRemove:str":"Unequip"}
 *
 * @param ShopScene:struct
 * @text Shop Menu Settings
 * @type struct<ShopScene>
 * @desc Change the Shop Menu Scene settings.
 * @default {"General":"","EnableLayout:eval":"true","LayoutStyle:str":"upper/left","Command":"","CmdHideDisabled:eval":"true","CmdStyle:str":"auto","CmdTextAlign:str":"center","CmdIconBuy:num":"208","CmdIconSell:num":"314","CmdIconCancel:num":"82","CmdCancelRename:str":"Exit","Prices":"","SellPriceRate:num":"0.50","BuyPriceJS:func":"\"// Declare variables\\nlet item = arguments[0];\\nlet price = arguments[1];\\n\\n// Return the finalized price\\nreturn price;\"","SellPriceJS:func":"\"// Declare variables\\nlet item = arguments[0];\\nlet price = arguments[1];\\n\\n// Return the finalized price\\nreturn price;\"","ButtonAssist":"","buttonAssistSmallIncrement:str":"-1/+1","buttonAssistLargeIncrement:str":"-10/+10"}
 *
 * @param StatusWindow:struct
 * @text Shop Status Window
 * @parent ShopScene:struct
 * @type struct<StatusWindow>
 * @desc Change the Item Status Window settings.
 * @default {"General":"","Width:num":"352","ParamChangeFontSize:num":"22","Translucent:num":"64","DrawBackRect:eval":"true","BackRectColor:str":"19","EquipData":"","EquipDataStyle:str":"compare","EquipDataCompare":"","AlreadyEquipMarker:str":"E","CannotEquipMarker:str":"-","NoChangeMarker:str":"-","DrawEquipData:func":"\"// Set Variables\\nconst lineHeight = this.lineHeight();\\nconst paramheight = this.gaugeLineHeight() + 8;\\nlet x = 0;\\nlet y = 0;\\nlet width = this.innerWidth;\\nlet height = this.innerHeight;\\nlet hw = Math.floor(width / 2);\\nlet hx = x + width - hw;\\n\\n// Draw Item Name, Type, and Quantity\\nthis.drawItemName(this._item, x + this.itemPadding(), y, width - this.itemPadding() * 2);\\nthis.drawItemDarkRect(x, y, width);\\ny += lineHeight;\\nif (this.drawItemEquipType(x, y, hw)) y += 0;\\nif (this.drawItemQuantity(hx, y, hw)) y += lineHeight;\\n\\n// Draw Parameter Names\\nconst params = this.actorParams();\\nconst backY = y;\\ny = height - (params.length * paramheight) - 4;\\nlet paramX = x;\\nlet paramWidth = 0;\\nlet tableY = y;\\nfor (const paramId of params) {\\n    paramWidth = Math.max(this.drawParamName(paramId, x + 4, y + 4, width), paramWidth);\\n    y += paramheight;\\n}\\n\\n// Draw Actor Data\\nconst actorMax = $gameParty.maxBattleMembers();\\nconst actorWidth = Math.floor((width - paramWidth) / actorMax);\\nparamWidth = width - (actorWidth * actorMax);\\nfor (const actor of $gameParty.battleMembers()) {\\n    const index = $gameParty.battleMembers().indexOf(actor);\\n    const actorX = paramX + paramWidth + (index * actorWidth);\\n    this.changePaintOpacity(actor.canEquip(this._item));\\n    this.drawActorCharacter(actor, actorX + (actorWidth / 2), tableY);\\n    let actorY = tableY;\\n\\n    // Draw Parameter Changes\\n    for (const paramId of params) {\\n        const diffY = actorY - ((lineHeight - paramheight) / 2);\\n        this.drawActorParamDifference(actor, paramId, actorX, diffY, actorWidth);\\n        actorY += paramheight;\\n    }\\n}\\n\\n// Draw Back Rectangles\\nthis.drawItemDarkRect(paramX, backY, paramWidth, tableY - backY);\\nfor (let i = 0; i < actorMax; i++) {\\n    const actorX = paramX + paramWidth + (i * actorWidth);\\n    this.drawItemDarkRect(actorX, backY, actorWidth, tableY - backY);\\n}\\nfor (const paramId of params) {\\n    this.drawItemDarkRect(paramX, tableY, paramWidth, paramheight);\\n    for (let i = 0; i < actorMax; i++) {\\n        const actorX = paramX + paramWidth + (i * actorWidth);\\n        this.drawItemDarkRect(actorX, tableY, actorWidth, paramheight);\\n    }\\n    tableY += paramheight;\\n}\"","EquipDataClassic":"","ClassicWeaponParameters:arraystr":"[\"HIT\"]","ClassicArmorParameters:arraystr":"[\"EVA\"]","DrawEquipClassicData:func":"\"// Set Variables\\nconst lineHeight = this.lineHeight();\\nlet x = 0;\\nlet y = 0;\\nlet width = this.innerWidth;\\nlet height = this.innerHeight;\\nlet hw = Math.floor(width / 2);\\nlet hx = x + width - hw;\\n\\n// Draw Item Name, Type, and Quantity\\nthis.drawItemName(this._item, x + this.itemPadding(), y, width - this.itemPadding() * 2);\\nthis.drawItemDarkRect(x, y, width);\\ny += lineHeight;\\nif (this.drawItemEquipType(x, y, hw)) y += 0;\\nif (this.drawItemQuantity(hx, y, hw)) y += lineHeight;\\n\\n// Draw Item Weapon Type or Armor Type\\nif (this.drawItemEquipSubType(x, y, width)) y += lineHeight;\\n\\n// Draw Parameter Values\\nconst params = this.actorParams();\\nfor (const paramId of params) {\\n    if (this.isCustomParameter(paramId)) continue;\\n    this.drawActorParamClassic(paramId, x, y, width);\\n    y += lineHeight;\\n}\\n\\n// Draw Custom Entries\\ny = this.drawItemCustomEntries(x, y, width);\\n\\n// Fill Rest of the Window\\nthis.drawItemDarkRect(x, y, width, height - y);\"","EquipDataDouble":"","DoubleWeaponParameters:arraystr":"[\"HIT\",\"CNT\"]","DoubleArmorParameters:arraystr":"[\"EVA\",\"GRD\"]","DrawEquipDoubleData:func":"\"// Set Variables\\nconst lineHeight = this.lineHeight();\\nlet x = 0;\\nlet y = 0;\\nlet width = this.innerWidth;\\nlet height = this.innerHeight;\\nlet hw = Math.floor(width / 2);\\nlet hx = x + width - hw;\\n\\n// Draw Item Name, Type, and Quantity\\nthis.drawItemName(this._item, x + this.itemPadding(), y, width - this.itemPadding() * 2);\\nthis.drawItemDarkRect(x, y, width);\\ny += lineHeight;\\nif (this.drawItemEquipType(x, y, hw)) y += 0;\\nif (this.drawItemQuantity(hx, y, hw)) y += lineHeight;\\n\\n// Draw Item Weapon Type or Armor Type\\nif (this.drawItemEquipSubType(x, y, width)) y += lineHeight;\\n\\n// Draw Parameter Values\\nconst params = this.actorParams();\\nfor (const paramId of params) {\\n    if (this.isCustomParameter(paramId)) continue;\\n    this.drawActorParamClassic(paramId, x, y, hw);\\n    if (x === hw) {\\n        y += lineHeight;\\n        x = 0;\\n    } else {\\n        x = hw;\\n    }\\n}\\n// Realign\\nif (x === hw) {\\n    this.drawItemDarkRect(hw, y, hw, lineHeight);\\n    y += lineHeight;\\n    x = 0;\\n}\\n\\n// Draw Custom Entries\\ny = this.drawItemCustomEntries(x, y, width);\\n\\n// Fill Rest of the Window\\nthis.drawItemDarkRect(x, y, width, height - y);\"","EquipDelayMS:num":"240","ItemData":"","ItemGeneral":"","MaxIcons:num":"8","MultiplierStandard:num":"1000000","DrawItemData:func":"\"const lineHeight = this.lineHeight();\\nlet x = 0;\\nlet y = 0;\\nlet width = this.innerWidth;\\nlet height = this.innerHeight;\\nlet hw = Math.floor(width / 2);\\nlet hx = x + width - hw;\\n\\n// Draw Item Name and Quantity\\nthis.drawItemName(this._item, x + this.itemPadding(), y, width - this.itemPadding() * 2);\\nthis.drawItemDarkRect(x, y, width);\\ny += lineHeight;\\n\\n// Draw Main Item Properties\\nif (this.drawItemConsumable(x, y, hw)) y += 0;\\nif (this.drawItemQuantity(hx, y, hw)) y += lineHeight;\\nif (this._item.occasion < 3) {\\n    y = this.drawItemDamage(x, y, width);\\n    y = this.drawItemEffects(x, y, width);\\n}\\ny = this.drawItemCustomEntries(x, y, width);\\n\\n// Draw Remaining Item Properties\\nif (this._item.occasion < 3) {\\n    if (this.drawItemOccasion(x, y, hw)) y += 0;\\n    if (this.drawItemScope(hx, y, hw)) y += lineHeight;\\n    if (this.drawItemHitType(x, y, hw)) y += 0;\\n    if (this.drawItemSuccessRate(hx, y, hw)) y += lineHeight;\\n    if (this.drawItemSpeed(x, y, hw)) y += 0;\\n    if (this.drawItemRepeats(hx, y, hw)) y += lineHeight;\\n}\\n\\n// Fill Rest of the Window\\nthis.drawItemDarkRect(x, y, width, height - y);\"","Vocabulary":"","LabelConsume:str":"Consumable","Consumable:str":"✔","NotConsumable:str":"✘","Occasions":"","Occasion0:str":"Anytime Use","Occasion1:str":"Battle-Only","Occasion2:str":"Field-Only","Occasion3:str":"-","Scope":"","Scope0:str":"No Target","Scope1:str":"1 Foe","Scope2:str":"All Foes","Scope3:str":"Random Foe","Scope4:str":"2 Random Foes","Scope5:str":"3 Random Foes","Scope6:str":"4 Random Foes","Scope7:str":"1 Ally","Scope8:str":"Alive Allies","Scope9:str":"Dead Ally","Scope10:str":"Dead Allies","Scope11:str":"User","Scope12:str":"Any Ally","Scope13:str":"All Allies","Scope14:str":"Everybody","BattleCore":"","ScopeRandomAny:str":"%1 Random Units","ScopeRandomEnemies:str":"%1 Random Foes","ScopeRandomAllies:str":"%1 Random Allies","ScopeAlliesButUser:str":"Other Allies","ScopeAllyOrEnemy:str":"Ally/Enemy","ScopeEnemyOrAlly:str":"Enemy/Ally","LabelSpeed:str":"Speed","Speed2000:str":"Fastest","Speed1000:str":"Faster","Speed1:str":"Fast","Speed0:str":"Normal","SpeedNeg999:str":"Slow","SpeedNeg1999:str":"Slower","SpeedNeg2000:str":"Slowest","LabelSuccessRate:str":"Accuracy","LabelRepeats:str":"Hits","LabelHitType:str":"Type","HitType0:str":"Neutral","HitType1:str":"Physical","HitType2:str":"Magical","LabelElement:str":"Element","ElementWeapon:str":"\\I[97]Weapon","ElementNone:str":"\\I[160]No Element","DamageType":"","DamageType1:str":"%1 Damage Multiplier","DamageType2:str":"%1 Damage Multiplier","DamageType3:str":"%1 Recovery Multiplier","DamageType4:str":"%1 Recovery Multiplier","DamageType5:str":"%1 Drain Multiplier","DamageType6:str":"%1 Drain Multiplier","Effects":"","LabelRecoverHP:str":"%1 Recovery","LabelRecoverMP:str":"%1 Recovery","LabelRecoverTP:str":"%1 Recovery","LabelSelfGainTP:str":"User %1","LabelDamageHP:str":"%1 Damage","LabelDamageMP:str":"%1 Damage","LabelDamageTP:str":"%1 Damage","LabelApply:str":"Applies","LabelRemove:str":"Removes","EquipType":"","WeaponType:str":"Weapon Type","ArmorType:str":"Armor Type","NoEquipTypeResult:str":"-"}
 *
 * @param BreakEnd1
 * @text --------------------------
 * @default ----------------------------------
 *
 * @param End Of
 * @default Plugin Parameters
 *
 * @param BreakEnd2
 * @text --------------------------
 * @default ----------------------------------
 *
 */
/* ----------------------------------------------------------------------------
 * Item Menu Scene Settings
 * ----------------------------------------------------------------------------
 */
/*~struct~ItemScene:
 *
 * @param General
 *
 * @param EnableLayout:eval
 * @text Use Updated Layout
 * @parent General
 * @type boolean
 * @on Use
 * @off Don't Use
 * @desc Use the Updated Item Menu Layout provided by this plugin?
 * This will override the Core Engine windows settings.
 * @default true
 *
 * @param DisabledVanilla:eval
 * @text Disabled: Vanilla
 * @parent EnableLayout:eval
 * @type boolean
 * @on Pure Vanilla
 * @off Allow Options
 * @desc If NOT using the updated layout, keep all settings to
 * pure vanilla and ignore other Plugin Parameters.
 * @default true
 *
 * @param LayoutStyle:str
 * @text Layout Style
 * @parent General
 * @type select
 * @option Upper Help, Left Input
 * @value upper/left
 * @option Upper Help, Right Input
 * @value upper/right
 * @option Lower Help, Left Input
 * @value lower/left
 * @option Lower Help, Right Input
 * @value lower/right
 * @desc If using an updated layout, how do you want to style
 * the menu scene layout?
 * @default upper/left
 *
 * @param ListWindow
 * @text List Window
 *
 * @param ListWindowCols:num
 * @text Columns
 * @parent ListWindow
 * @type number
 * @min 1
 * @desc Number of maximum columns.
 * @default 1
 *
 * @param ItemQt
 * @text Item Quantity
 *
 * @param MaxItems:num
 * @text Item Max
 * @parent ItemQt
 * @desc The default maximum quantity for items.
 * @default 99
 *
 * @param MaxWeapons:num
 * @text Weapon Max
 * @parent ItemQt
 * @desc The default maximum quantity for weapons.
 * @default 99
 *
 * @param MaxArmors:num
 * @text Armor Max
 * @parent ItemQt
 * @desc The default maximum quantity for armors.
 * @default 99
 *
 * @param ItemQuantityFmt:str
 * @text Quantity Format
 * @parent ItemQt
 * @desc How to display an item's quantity.
 * %1 - Item Quantity
 * @default ×%1
 *
 * @param ItemQuantityFontSize:num
 * @text Font Size
 * @parent ItemQt
 * @desc Default font size for item quantity.
 * @default 22
 *
 * @param ShopStatusWindow
 * @text Shop Status Window
 *
 * @param ShowShopStatus:eval
 * @text Show in Item Menu?
 * @parent ShopStatusWindow
 * @type boolean
 * @on Show
 * @off Don't Show
 * @desc Show the Shop Status Window in the Item Menu?
 * This is enabled if the Updated Layout is on.
 * @default true
 *
 * @param ItemSceneAdjustItemList:eval
 * @text Adjust List Window?
 * @parent ShopStatusWindow
 * @type boolean
 * @on Adjust
 * @off Don't
 * @desc Automatically adjust the Item List Window in the Item Menu if using the Shop Status Window?
 * @default true
 *
 * @param ItemMenuStatusBgType:num
 * @text Background Type
 * @parent ShopStatusWindow
 * @type select
 * @option 0 - Window
 * @value 0
 * @option 1 - Dim
 * @value 1
 * @option 2 - Transparent
 * @value 2
 * @desc Select background type for this window.
 * @default 0
 *
 * @param ItemMenuStatusRect:func
 * @text JS: X, Y, W, H
 * @parent ShopStatusWindow
 * @type note
 * @desc Code used to determine the dimensions for this Status Window in the Item Menu.
 * @default "const width = this.statusWidth();\nconst height = this._itemWindow.height;\nconst x = Graphics.boxWidth - width;\nconst y = this._itemWindow.y;\nreturn new Rectangle(x, y, width, height);"
 *
 * @param ButtonAssist
 * @text Button Assist Window
 *
 * @param buttonAssistCategory:str
 * @text Switch Category
 * @parent ButtonAssist
 * @desc Button assist text used for switching categories.
 * For VisuStella MZ's Core Engine's Button Assist Window.
 * @default Switch Category
 *
 */
/* ----------------------------------------------------------------------------
 * Item Categories
 * ----------------------------------------------------------------------------
 */
/*~struct~Categories:
 *
 * @param MainList
 * @text List
 * 
 * @param List:arraystruct
 * @text Category List
 * @parent MainList
 * @type struct<Category>[]
 * @desc A list of the item categories displayed in the Item/Shop menus.
 * @default ["{\"Type:str\":\"RegularItems\",\"Icon:num\":\"208\"}","{\"Type:str\":\"AllWeapons\",\"Icon:num\":\"97\"}","{\"Type:str\":\"AllArmors\",\"Icon:num\":\"137\"}","{\"Type:str\":\"KeyItems\",\"Icon:num\":\"195\"}"]
 *
 * @param Style:str
 * @text Category Style
 * @parent MainList
 * @type select
 * @option Text Only
 * @value text
 * @option Icon Only
 * @value icon
 * @option Icon + Text
 * @value iconText
 * @option Automatic
 * @value auto
 * @desc How do you wish to draw categorie entries in the Category Window?
 * @default icon
 *
 * @param TextAlign:str
 * @text Text Alignment
 * @parent MainList
 * @type combo
 * @option left
 * @option center
 * @option right
 * @desc Decide how you want the text to be aligned.
 * @default center
 *
 * @param Vocabulary
 *
 * @param HiddenItemA:str
 * @text Hidden Item A
 * @parent Vocabulary
 * @desc How this category is named in the Item Menu.
 * @default Special Items
 *
 * @param HiddenItemB:str
 * @text Hidden Item B
 * @parent Vocabulary
 * @desc How this category is named in the Item Menu.
 * @default Unique Items
 *
 * @param Consumable:str
 * @text Consumable
 * @parent Vocabulary
 * @desc How this category is named in the Item Menu.
 * @default Consumable
 *
 * @param Nonconsumable:str
 * @text Nonconsumable
 * @parent Vocabulary
 * @desc How this category is named in the Item Menu.
 * @default Nonconsumable
 *
 * @param AlwaysUsable:str
 * @text Always Usable
 * @parent Vocabulary
 * @desc How this category is named in the Item Menu.
 * @default Usable
 *
 * @param BattleUsable:str
 * @text Battle Usable
 * @parent Vocabulary
 * @desc How this category is named in the Item Menu.
 * @default Battle
 *
 * @param FieldUsable:str
 * @text Field Usable
 * @parent Vocabulary
 * @desc How this category is named in the Item Menu.
 * @default Field
 *
 * @param NeverUsable:str
 * @text Never Usable
 * @parent Vocabulary
 * @desc How this category is named in the Item Menu.
 * @default Materials
 *
 */
/* ----------------------------------------------------------------------------
 * Category Settings
 * ----------------------------------------------------------------------------
 */
/*~struct~Category:
 *
 * @param Type:str
 * @text Type
 * @type combo
 * @option AllItems
 * @option 
 * @option RegularItems
 * @option KeyItems
 * @option HiddenItemA
 * @option HiddenItemB
 * @option 
 * @option Consumable
 * @option Nonconsumable
 * @option 
 * @option AlwaysUsable
 * @option BattleUsable
 * @option FieldUsable
 * @option NeverUsable
 * @option 
 * @option AllWeapons
 * @option WType:x
 * @option 
 * @option AllArmors
 * @option AType:x
 * @option 
 * @option EType:x
 * @option 
 * @option Category:x
 * @option
 * @desc A list of the item categories displayed in the Item/Shop
 * menus. Replace x with ID numbers or text.
 * @default RegularItems
 *
 * @param Icon:num
 * @text Icon
 * @desc Icon used for this category.
 * Use 0 for no icon.
 * @default 0
 *
 * @param SwitchID:num
 * @text Visibility Switch
 * @type switch
 * @desc This Switch must be turned ON in order for the category to show.
 * Use 0 for no Switch requirement.
 * @default 0
 *
 * @param SortBy:str
 * @text Sorted By
 * @type select
 * @option ID
 * @option Name
 * @desc Sort this category (in Scene_Item and Scene_Shop only) this way.
 * @default ID
 *
 */
/* ----------------------------------------------------------------------------
 * New Label Settings
 * ----------------------------------------------------------------------------
 */
/*~struct~NewLabel:
 *
 * @param Enable:eval
 * @text Use NEW! Labels?
 * @type boolean
 * @on Use
 * @off Don't Use
 * @desc Use the NEW! Labels or not?
 * @default true
 *
 * @param Icon:num
 * @text Icon
 * @desc The icon index used to represent the NEW! text.
 * Use 0 to not draw any icons.
 * @default 0
 *
 * @param Text:str
 * @text Text
 * @desc The text written on the NEW! Label.
 * @default NEW!
 *
 * @param FontColor:str
 * @text Font Color
 * @parent Text:str
 * @desc Use #rrggbb for custom colors or regular numbers
 * for text colors from the Window Skin.
 * @default 17
 *
 * @param FontFace:str
 * @text Font Face
 * @parent Text:str
 * @desc Font face used for the NEW! Label.
 * @default Verdana
 *
 * @param FontSize:str
 * @text Font Size
 * @parent Text:str
 * @desc The font size used for the NEW! text.
 * @default 16
 *
 * @param FadeLimit:num
 * @text Fade Limit
 * @desc What's the upper opaque limit before reversing?
 * @default 360
 *
 * @param FadeSpeed:num
 * @text Fade Speed
 * @desc What's the fade speed of the NEW! Label?
 * @default 4
 *
 * @param OffsetX:num
 * @text Offset X
 * @desc How much to offset the NEW! Label's X position by.
 * @default 0
 *
 * @param OffsetY:num
 * @text Offset Y
 * @desc How much to offset the NEW! Label's Y position by.
 * @default 4
 *
 */
/* ----------------------------------------------------------------------------
 * Equip Menu Scene Settings
 * ----------------------------------------------------------------------------
 */
/*~struct~EquipScene:
 *
 * @param General
 *
 * @param EnableLayout:eval
 * @text Use Updated Layout
 * @parent General
 * @type boolean
 * @on Use
 * @off Don't Use
 * @desc Use the Updated Equip Layout provided by this plugin?
 * This will override the Core Engine windows settings.
 * @default true
 *
 * @param LayoutStyle:str
 * @text Layout Style
 * @parent General
 * @type select
 * @option Upper Help, Left Input
 * @value upper/left
 * @option Upper Help, Right Input
 * @value upper/right
 * @option Lower Help, Left Input
 * @value lower/left
 * @option Lower Help, Right Input
 * @value lower/right
 * @desc If using an updated layout, how do you want to style
 * the menu scene layout?
 * @default upper/right
 *
 * @param ParamValueFontSize:num
 * @text Param Font Size
 * @parent EnableLayout:eval
 * @desc The font size used for parameter values.
 * @default 22
 *
 * @param MenuPortraits:eval
 * @text Show Menu Portraits?
 * @parent EnableLayout:eval
 * @type boolean
 * @on Use Portraits
 * @off Use Faces
 * @desc If Main Menu Core is installed, display the Menu Portraits
 * instead of the actor's face in the status window?
 * @default true
 *
 * @param DrawPortraitJS:func
 * @text JS: Portrait Upper
 * @parent EnableLayout:eval
 * @type note
 * @desc If Menu Portraits are available, this is code used to draw
 * the upper data like this in the Status Window.
 * @default "// Declare Variables\nconst lineHeight = this.lineHeight();\nconst padding = this.itemPadding();\nconst x1 = padding;\nconst x2 = this.innerWidth - 128 - padding;\n\n// Draw Menu Image\nthis.drawItemActorMenuImage(this._actor, 0, 0, this.innerWidth, this.innerHeight);\n\n// Draw Data\nthis.drawActorName(this._actor, x1, lineHeight * 0);\nthis.drawActorClass(this._actor, x1, lineHeight * 1);\nthis.drawActorIcons(this._actor, x1, lineHeight * 2);\nthis.drawActorLevel(this._actor, x2, lineHeight * 0);\nthis.placeBasicGauges(this._actor, x2, lineHeight * 1);"
 *
 * @param DrawFaceJS:func
 * @text JS: Face Upper
 * @parent EnableLayout:eval
 * @type note
 * @desc If faces used used, this is code used to draw the upper
 * data like this in the Status Window.
 * @default "// Declare Variables\nconst lineHeight = this.lineHeight();\nconst gaugeLineHeight = this.gaugeLineHeight();\nconst x = Math.floor(this.innerWidth / 2);\nconst limitHeight = this.innerHeight - (this.actorParams().length * lineHeight);\nconst actorX = Math.floor((x - ImageManager.faceWidth) / 2);\nconst actorY = Math.max(0, Math.floor((limitHeight - ImageManager.faceHeight) / 2));\nlet dataHeight = lineHeight * 3;\ndataHeight += gaugeLineHeight * ($dataSystem.optDisplayTp ? 3 : 2);\nconst dataY = Math.max(0, Math.floor((limitHeight - dataHeight) / 2));\n\n// Draw Data\nthis.drawActorFace(this._actor, actorX, actorY, ImageManager.faceWidth, ImageManager.faceHeight);\nthis.drawActorIcons(this._actor, actorX + 16, actorY + ImageManager.faceHeight - lineHeight);\nthis.drawActorName(this._actor, x, dataY + lineHeight * 0);\nthis.drawActorLevel(this._actor, x, dataY + lineHeight * 1);\nif (this.innerHeight > 444) {\n    this.drawActorClass(this._actor, x, dataY + lineHeight * 2);\n    this.placeBasicGauges(this._actor, x, dataY + lineHeight * 3);\n} else {\n    this.placeBasicGauges(this._actor, x, dataY + lineHeight * 2);\n}"
 *
 * @param DrawParamJS:func
 * @text JS: Parameter Lower
 * @parent EnableLayout:eval
 * @type note
 * @desc Code to determine how parameters are drawn in the
 * Status Window.
 * @default "// Declare variables\nconst params = this.actorParams();\nconst lineHeight = this.lineHeight();\nconst padding = this.itemPadding();\nconst baseX = 0;\nconst baseY = this.innerHeight - params.length * lineHeight;\nconst baseWidth = this.innerWidth;\nconst valueFontSize = this.paramValueFontSize();\n\n// Calculate Widths\nlet paramNameWidth = Math.max(...params.map(param => this.textWidth(TextManager.param(param))));\nparamNameWidth += padding * 2;\nif (this.isUseParamNamesWithIcons()) {\n    const standardWidth = ImageManager.standardIconWidth || 32;\n    paramNameWidth += standardWidth + 4;\n}\nlet arrowWidth = this.rightArrowWidth();\nconst totalDivides = this.innerWidth >= 500 ? 3 : 2;\nlet paramValueWidth = Math.floor((baseWidth - paramNameWidth - arrowWidth) / totalDivides);\nparamNameWidth = baseWidth - (paramValueWidth * totalDivides) - arrowWidth;\n\n// Draw Parameters\nlet x = baseX;\nlet y = baseY;\nlet value = 0;\nlet diffValue = 0;\nlet alter = 2;\nfor (const paramId of params) {\n    // Draw Param Name\n    this.drawItemDarkRect(x, y, paramNameWidth, lineHeight, alter);\n    this.drawUpdatedParamName(paramId, x, y, paramNameWidth);\n    this.resetFontSettings();\n    x += paramNameWidth;\n\n    // Draw Param Before\n    this.contents.fontSize = valueFontSize;\n    this.drawItemDarkRect(x, y, paramValueWidth, lineHeight, alter);\n    this.drawUpdatedBeforeParamValue(paramId, x, y, paramValueWidth);\n    this.resetFontSettings();\n    x += paramValueWidth;\n\n    // Draw Arrow\n    this.drawItemDarkRect(x, y, arrowWidth, lineHeight, alter);\n    this.drawRightArrow(x, y);\n    x += arrowWidth;\n\n    // Draw Param After\n    this.contents.fontSize = valueFontSize;\n    this.drawItemDarkRect(x, y, paramValueWidth, lineHeight, alter);\n    this.drawUpdatedAfterParamValue(paramId, x, y, paramValueWidth);\n    x += paramValueWidth;\n\n    // Draw Param Change\n    if (totalDivides > 2) {\n        this.drawItemDarkRect(x, y, paramValueWidth, lineHeight, alter);\n        this.drawUpdatedParamValueDiff(paramId, x, y, paramValueWidth);\n    }\n\n    // Prepare Next Parameter\n    x = baseX;\n    y += lineHeight;\n    alter = alter === 2 ? 1 : 2;\n}"
 *
 * @param StatusWindowWidth:num
 * @text Status Window Width
 * @parent General
 * @desc The usual width of the status window if using the 
 * non-Updated Equip Menu Layout.
 * @default 312
 *
 * @param DrawBackRect:eval
 * @text Show Back Rectangles?
 * @parent General
 * @type boolean
 * @on Draw
 * @off Don't Draw
 * @desc Show back rectangles of darker colors to display information better?
 * @default true
 *
 * @param BackRectColor:str
 * @text Back Rectangle Color
 * @parent DrawBackRect:eval
 * @desc Use #rrggbb for custom colors or regular numbers
 * for text colors from the Window Skin.
 * @default 19
 *
 * @param CursedTextPopup:json
 * @text Cursed Equip Popup
 * @parent General
 * @type note
 * @desc %1 - Actor, %2 - Equip, %3 - Icon. Text codes allowed.
 * Requires VisuMZ_0_CoreEngine! Empty to not use.
 * @default "%1 is cursed by %3%2!"
 *
 * @param Command
 * @text Command Window
 *
 * @param CmdStyle:str
 * @text Style
 * @parent Command
 * @type select
 * @option Text Only
 * @value text
 * @option Icon Only
 * @value icon
 * @option Icon + Text
 * @value iconText
 * @option Automatic
 * @value auto
 * @desc How do you wish to draw commands in the Command Window?
 * @default auto
 *
 * @param CmdTextAlign:str
 * @text Text Align
 * @parent Command
 * @type combo
 * @option left
 * @option center
 * @option right
 * @desc Text alignment for the Command Window.
 * @default center
 *
 * @param CmdIconEquip:num
 * @text Equip Icon
 * @parent Command
 * @desc The icon used for the Equip command.
 * @default 136
 *
 * @param equipCmdDesc:json
 * @text Help Description
 * @parent CmdIconEquip:num
 * @type note
 * @desc Help description used when this command is selected.
 * Text codes allowed.
 * @default "Pick and choose equipment to change."
 *
 * @param CommandAddOptimize:eval
 * @text Add Optimize Command?
 * @parent Command
 * @type boolean
 * @on Add
 * @off Don't
 * @desc Add the "Optimize" command to the Command Window?
 * @default true
 *
 * @param optimizeCmdDesc:json
 * @text Help Description
 * @parent CommandAddOptimize:eval
 * @type note
 * @desc Help description used when this command is selected.
 * Text codes allowed.
 * @default "Equip the strongest available equipment."
 *
 * @param CmdIconOptimize:num
 * @text Optimize Icon
 * @parent CommandAddOptimize:eval
 * @desc The icon used for the Optimize command.
 * @default 137
 *
 * @param CommandAddClear:eval
 * @text Add Clear Command?
 * @parent Command
 * @type boolean
 * @on Add
 * @off Don't
 * @desc Add the "Clear" command to the Command Window?
 * @default true
 *
 * @param clearCmdDesc:json
 * @text Help Description
 * @parent CommandAddClear:eval
 * @type note
 * @desc Help description used when this command is selected.
 * Text codes allowed.
 * @default "Remove all available equipment."
 *
 * @param CmdIconClear:num
 * @text Clear Icon
 * @parent CommandAddClear:eval
 * @desc The icon used for the Clear command.
 * @default 135
 *
 * @param RemoveEquip
 * @text Remove Equip
 *
 * @param RemoveEquipIcon:num
 * @text Icon
 * @parent RemoveEquip
 * @desc Icon used for equipment removal.
 * @default 16
 *
 * @param RemoveEquipText:str
 * @text Text
 * @parent RemoveEquip
 * @desc Text used for equipment removal.
 * @default Remove
 *
 * @param ShiftShortcutKey:eval
 * @text Use SHIFT Shortcut?
 * @parent RemoveEquip
 * @type boolean
 * @on Use
 * @off Don't
 * @desc Add the "Shift" button as a shortcut key to removing items?
 * @default true

 * @param Rulings
 *
 * @param EquipAdjustHpMp:eval
 * @text Equip-Adjust HP/MP
 * @parent Rulings
 * @type boolean
 * @on Adjust
 * @off Don't
 * @desc Adjust HP/MP differences after changing equips with MaxHP/MaxMP values.
 * @default true
 * 
 * @param NonRemoveETypes:arraynum
 * @text Non-Removable Types
 * @parent Rulings
 * @type number[]
 * @min 1
 * @max 100
 * @desc Insert ID's of the Equipment Types that must always have
 * an item equipped and cannot be empty.
 * @default []
 *
 * @param NonOptimizeETypes:arraynum
 * @text Non-Optimized Types
 * @parent Rulings
 * @type number[]
 * @min 1
 * @max 100
 * @desc Insert ID's of the Equipment Types that will be ignored
 * when equipment is being optimized.
 * @default []
 *
 * @param ButtonAssist
 * @text Button Assist Window
 *
 * @param buttonAssistRemove:str
 * @text SHIFT: Remove
 * @parent ButtonAssist
 * @desc Button assist text used for the SHIFT Remove Shortcut.
 * For VisuStella MZ's Core Engine's Button Assist Window.
 * @default Unequip
 * 
 */
/* ----------------------------------------------------------------------------
 * Shop Menu Scene Settings
 * ----------------------------------------------------------------------------
 */
/*~struct~ShopScene:
 *
 * @param General
 *
 * @param EnableLayout:eval
 * @text Use Updated Layout
 * @parent General
 * @type boolean
 * @on Use
 * @off Don't Use
 * @desc Use the Updated Shop Layout provided by this plugin?
 * This will override the Core Engine windows settings.
 * @default true
 *
 * @param LayoutStyle:str
 * @text Layout Style
 * @parent General
 * @type select
 * @option Upper Help, Left Input
 * @value upper/left
 * @option Upper Help, Right Input
 * @value upper/right
 * @option Lower Help, Left Input
 * @value lower/left
 * @option Lower Help, Right Input
 * @value lower/right
 * @desc If using an updated layout, how do you want to style
 * the menu scene layout?
 * @default upper/left
 *
 * @param Switches
 *
 * @param SwitchBuy:num
 * @text Switch: Buy
 * @parent Switches
 * @type switch
 * @desc Buying items in the Shop Scene turns this Switch to ON.
 * Switch reverts to OFF whenever the Shop Scene opens.
 * @default 0
 *
 * @param SwitchSell:num
 * @text Switch: Sell
 * @parent Switches
 * @type switch
 * @desc Selling items in the Shop Scene turns this Switch to ON.
 * Switch reverts to OFF whenever the Shop Scene opens.
 * @default 0
 *
 * @param Command
 * @text Command Window
 *
 * @param CmdHideDisabled:eval
 * @text Hide Unavailable?
 * @parent Command
 * @type boolean
 * @on Hide
 * @off Default
 * @desc Hide all unavailable commands like when a shop is set to Purchase Only?
 * @default true
 *
 * @param CmdStyle:str
 * @text Style
 * @parent Command
 * @type select
 * @option Text Only
 * @value text
 * @option Icon Only
 * @value icon
 * @option Icon + Text
 * @value iconText
 * @option Automatic
 * @value auto
 * @desc How do you wish to draw commands in the Command Window?
 * @default auto
 *
 * @param CmdTextAlign:str
 * @text Text Align
 * @parent Command
 * @type combo
 * @option left
 * @option center
 * @option right
 * @desc Text alignment for the Command Window.
 * @default center
 *
 * @param CmdIconBuy:num
 * @text Buy Icon
 * @parent Command
 * @desc The icon used for the Buy command.
 * @default 208
 *
 * @param CmdIconSell:num
 * @text Sell Icon
 * @parent Command
 * @desc The icon used for the Sell command.
 * @default 314
 *
 * @param CmdIconCancel:num
 * @text Cancel Icon
 * @parent Command
 * @desc The icon used for the Cancel command.
 * @default 82
 *
 * @param CmdCancelRename:str
 * @text Rename "Cancel"
 * @parent Command
 * @desc Rename Cancel to something more logical for the Shop Menu Scene.
 * @default Exit
 *
 * @param Prices
 *
 * @param SellPriceRate:num
 * @text Sell Price Rate
 * @parent Prices
 * @desc The default sell price rate.
 * @default 0.50
 *
 * @param BuyPriceJS:func
 * @text JS: Buy Price
 * @parent Prices
 * @type note
 * @desc Modificatons made to the buy price before finalizing it.
 * @default "// Declare variables\nlet item = arguments[0];\nlet price = arguments[1];\n\n// Return the finalized price\nreturn price;"
 *
 * @param SellPriceJS:func
 * @text JS: Sell Price
 * @parent Prices
 * @type note
 * @desc Modificatons made to the sell price before finalizing it.
 * @default "// Declare variables\nlet item = arguments[0];\nlet price = arguments[1];\n\n// Return the finalized price\nreturn price;"
 * 
 * @param ButtonAssist
 * @text Button Assist Window
 *
 * @param buttonAssistSmallIncrement:str
 * @text Small Increment
 * @parent ButtonAssist
 * @desc Text used for changing amount bought/sold.
 * For VisuStella MZ's Core Engine's Button Assist Window.
 * @default -1/+1
 *
 * @param buttonAssistLargeIncrement:str
 * @text Large Increment
 * @parent ButtonAssist
 * @desc Text used for changing amount bought/sold.
 * For VisuStella MZ's Core Engine's Button Assist Window.
 * @default -10/+10
 *
 */
/* ----------------------------------------------------------------------------
 * Shop Status Window Settings
 * ----------------------------------------------------------------------------
 */
/*~struct~StatusWindow:
 *
 * @param General
 *
 * @param Width:num
 * @text Window Width
 * @parent General
 * @desc The usual width of the status window.
 * @default 352
 *
 * @param ParamChangeFontSize:num
 * @text Parameter Font Size
 * @parent General
 * @desc Font size used for parameter changes.
 * @default 22
 *
 * @param Translucent:num
 * @text Translucent Opacity
 * @parent General
 * @desc Opacity setting used for translucent window objects.
 * @default 64
 *
 * @param DrawBackRect:eval
 * @text Show Back Rectangles?
 * @parent General
 * @type boolean
 * @on Draw
 * @off Don't Draw
 * @desc Show back rectangles of darker colors to display information better?
 * @default true
 *
 * @param BackRectColor:str
 * @text Back Rectangle Color
 * @parent DrawBackRect:eval
 * @desc Use #rrggbb for custom colors or regular numbers
 * for text colors from the Window Skin.
 * @default 19
 *
 * @param EquipData
 * @text Equipment Data
 * 
 * @param EquipDataStyle:str
 * @text Data Style
 * @parent EquipData
 * @type select
 * @option Compare - Compares selected equip to equipped gear
 * @value compare
 * @option Classic - Shows basic parameters of selected equip
 * @value classic
 * @option Double - Shows basic parameters in double columns
 * @value double
 * @desc How do you wish to display equipment data?
 * @default compare
 *
 * @param EquipDataCompare
 * @text Compare Style
 * @parent EquipDataStyle:str
 *
 * @param AlreadyEquipMarker:str
 * @text Already Equipped
 * @parent EquipDataCompare
 * @desc Marker used to show an actor cannot equip an item.
 * @default E
 *
 * @param CannotEquipMarker:str
 * @text Can't Equip
 * @parent EquipDataCompare
 * @desc Marker used to show an actor cannot equip an item.
 * @default -
 *
 * @param NoChangeMarker:str
 * @text No Changes
 * @parent EquipDataCompare
 * @desc Marker used to show no changes have occurred.
 * @default -
 *
 * @param DrawEquipData:func
 * @text JS: Draw Equip Data
 * @parent EquipDataCompare
 * @type note
 * @desc Code used to draw the equipment data for the Shop Status Window.
 * @default "// Set Variables\nconst lineHeight = this.lineHeight();\nconst paramheight = this.gaugeLineHeight() + 8;\nlet x = 0;\nlet y = 0;\nlet width = this.innerWidth;\nlet height = this.innerHeight;\nlet hw = Math.floor(width / 2);\nlet hx = x + width - hw;\n\n// Draw Item Name, Type, and Quantity\nthis.drawItemName(this._item, x + this.itemPadding(), y, width - this.itemPadding() * 2);\nthis.drawItemDarkRect(x, y, width);\ny += lineHeight;\nif (this.drawItemEquipType(x, y, hw)) y += 0;\nif (this.drawItemQuantity(hx, y, hw)) y += lineHeight;\n\n// Draw Parameter Names\nconst params = this.actorParams();\nconst backY = y;\ny = height - (params.length * paramheight) - 4;\nlet paramX = x;\nlet paramWidth = 0;\nlet tableY = y;\nfor (const paramId of params) {\n    paramWidth = Math.max(this.drawParamName(paramId, x + 4, y + 4, width), paramWidth);\n    y += paramheight;\n}\n\n// Draw Actor Data\nconst actorMax = $gameParty.maxBattleMembers();\nconst actorWidth = Math.floor((width - paramWidth) / actorMax);\nparamWidth = width - (actorWidth * actorMax);\nfor (const actor of $gameParty.battleMembers()) {\n    const index = $gameParty.battleMembers().indexOf(actor);\n    const actorX = paramX + paramWidth + (index * actorWidth);\n    this.changePaintOpacity(actor.canEquip(this._item));\n    this.drawActorCharacter(actor, actorX + (actorWidth / 2), tableY);\n    let actorY = tableY;\n\n    // Draw Parameter Changes\n    for (const paramId of params) {\n        const diffY = actorY - ((lineHeight - paramheight) / 2);\n        this.drawActorParamDifference(actor, paramId, actorX, diffY, actorWidth);\n        actorY += paramheight;\n    }\n}\n\n// Draw Back Rectangles\nthis.drawItemDarkRect(paramX, backY, paramWidth, tableY - backY);\nfor (let i = 0; i < actorMax; i++) {\n    const actorX = paramX + paramWidth + (i * actorWidth);\n    this.drawItemDarkRect(actorX, backY, actorWidth, tableY - backY);\n}\nfor (const paramId of params) {\n    this.drawItemDarkRect(paramX, tableY, paramWidth, paramheight);\n    for (let i = 0; i < actorMax; i++) {\n        const actorX = paramX + paramWidth + (i * actorWidth);\n        this.drawItemDarkRect(actorX, tableY, actorWidth, paramheight);\n    }\n    tableY += paramheight;\n}"
 *
 * @param EquipDataClassic
 * @text Classic Style
 * @parent EquipDataStyle:str
 *
 * @param ClassicWeaponParameters:arraystr
 * @text Added Weapon Params
 * @parent EquipDataClassic
 * @type combo[]
 * @option MaxHP
 * @option MaxMP
 * @option ATK
 * @option DEF
 * @option MAT
 * @option MDF
 * @option AGI
 * @option LUK
 * @option HIT
 * @option EVA
 * @option CRI
 * @option CEV
 * @option MEV
 * @option MRF
 * @option CNT
 * @option HRG
 * @option MRG
 * @option TRG
 * @option TGR
 * @option GRD
 * @option REC
 * @option PHA
 * @option MCR
 * @option TCR
 * @option PDR
 * @option MDR
 * @option FDR
 * @option EXR
 * @desc Display these parameters when a weapon is selected.
 * Requires VisuMZ_0_CoreEngine!
 * @default ["HIT"]
 *
 * @param ClassicArmorParameters:arraystr
 * @text Added Armor Params
 * @parent EquipDataClassic
 * @type combo[]
 * @option MaxHP
 * @option MaxMP
 * @option ATK
 * @option DEF
 * @option MAT
 * @option MDF
 * @option AGI
 * @option LUK
 * @option HIT
 * @option EVA
 * @option CRI
 * @option CEV
 * @option MEV
 * @option MRF
 * @option CNT
 * @option HRG
 * @option MRG
 * @option TRG
 * @option TGR
 * @option GRD
 * @option REC
 * @option PHA
 * @option MCR
 * @option TCR
 * @option PDR
 * @option MDR
 * @option FDR
 * @option EXR
 * @desc Display these parameters when an armor is selected.
 * Requires VisuMZ_0_CoreEngine!
 * @default ["EVA"]
 *
 * @param DrawEquipClassicData:func
 * @text JS: Draw Equip Data
 * @parent EquipDataClassic
 * @type note
 * @desc Code used to draw the equipment data for the Shop Status Window.
 * @default "// Set Variables\nconst lineHeight = this.lineHeight();\nlet x = 0;\nlet y = 0;\nlet width = this.innerWidth;\nlet height = this.innerHeight;\nlet hw = Math.floor(width / 2);\nlet hx = x + width - hw;\n\n// Draw Item Name, Type, and Quantity\nthis.drawItemName(this._item, x + this.itemPadding(), y, width - this.itemPadding() * 2);\nthis.drawItemDarkRect(x, y, width);\ny += lineHeight;\nif (this.drawItemEquipType(x, y, hw)) y += 0;\nif (this.drawItemQuantity(hx, y, hw)) y += lineHeight;\n\n// Draw Item Weapon Type or Armor Type\nif (this.drawItemEquipSubType(x, y, width)) y += lineHeight;\n\n// Draw Parameter Values\nconst params = this.actorParams();\nfor (const paramId of params) {\n    if (this.isCustomParameter(paramId)) continue;\n    this.drawActorParamClassic(paramId, x, y, width);\n    y += lineHeight;\n}\n\n// Draw Custom Entries\ny = this.drawItemCustomEntries(x, y, width);\n\n// Fill Rest of the Window\nthis.drawItemDarkRect(x, y, width, height - y);"
 *
 * @param EquipDataDouble
 * @text Double Style
 * @parent EquipDataStyle:str
 *
 * @param DoubleWeaponParameters:arraystr
 * @text Added Weapon Params
 * @parent EquipDataDouble
 * @type combo[]
 * @option MaxHP
 * @option MaxMP
 * @option ATK
 * @option DEF
 * @option MAT
 * @option MDF
 * @option AGI
 * @option LUK
 * @option HIT
 * @option EVA
 * @option CRI
 * @option CEV
 * @option MEV
 * @option MRF
 * @option CNT
 * @option HRG
 * @option MRG
 * @option TRG
 * @option TGR
 * @option GRD
 * @option REC
 * @option PHA
 * @option MCR
 * @option TCR
 * @option PDR
 * @option MDR
 * @option FDR
 * @option EXR
 * @desc Display these parameters when a weapon is selected.
 * Requires VisuMZ_0_CoreEngine!
 * @default ["HIT","CNT"]
 *
 * @param DoubleArmorParameters:arraystr
 * @text Added Armor Params
 * @parent EquipDataDouble
 * @type combo[]
 * @option MaxHP
 * @option MaxMP
 * @option ATK
 * @option DEF
 * @option MAT
 * @option MDF
 * @option AGI
 * @option LUK
 * @option HIT
 * @option EVA
 * @option CRI
 * @option CEV
 * @option MEV
 * @option MRF
 * @option CNT
 * @option HRG
 * @option MRG
 * @option TRG
 * @option TGR
 * @option GRD
 * @option REC
 * @option PHA
 * @option MCR
 * @option TCR
 * @option PDR
 * @option MDR
 * @option FDR
 * @option EXR
 * @desc Display these parameters when an armor is selected.
 * Requires VisuMZ_0_CoreEngine!
 * @default ["EVA","GRD"]
 *
 * @param DrawEquipDoubleData:func
 * @text JS: Draw Equip Data
 * @parent EquipDataDouble
 * @type note
 * @desc Code used to draw the equipment data for the Shop Status Window.
 * @default "// Set Variables\nconst lineHeight = this.lineHeight();\nlet x = 0;\nlet y = 0;\nlet width = this.innerWidth;\nlet height = this.innerHeight;\nlet hw = Math.floor(width / 2);\nlet hx = x + width - hw;\n\n// Draw Item Name, Type, and Quantity\nthis.drawItemName(this._item, x + this.itemPadding(), y, width - this.itemPadding() * 2);\nthis.drawItemDarkRect(x, y, width);\ny += lineHeight;\nif (this.drawItemEquipType(x, y, hw)) y += 0;\nif (this.drawItemQuantity(hx, y, hw)) y += lineHeight;\n\n// Draw Item Weapon Type or Armor Type\nif (this.drawItemEquipSubType(x, y, width)) y += lineHeight;\n\n// Draw Parameter Values\nconst params = this.actorParams();\nfor (const paramId of params) {\n    if (this.isCustomParameter(paramId)) continue;\n    this.drawActorParamClassic(paramId, x, y, hw);\n    if (x === hw) {\n        y += lineHeight;\n        x = 0;\n    } else {\n        x = hw;\n    }\n}\n// Realign\nif (x === hw) {\n    this.drawItemDarkRect(hw, y, hw, lineHeight);\n    y += lineHeight;\n    x = 0;\n}\n\n// Draw Custom Entries\ny = this.drawItemCustomEntries(x, y, width);\n\n// Fill Rest of the Window\nthis.drawItemDarkRect(x, y, width, height - y);"
 *
 * @param EquipDelayMS:num
 * @text Delay MS
 * @parent EquipData
 * @type number
 * @min 1
 * @max 999
 * @desc How many milliseconds (MS) to delay the preview update?
 * This is to prevent lag spikes for equips only.
 * @default 240
 *
 * @param ItemData
 * @text Item Data
 *
 * @param ItemGeneral
 * @parent ItemData
 * @text Data Settings
 *
 * @param MaxIcons:num
 * @text Max State/Buff Icons
 * @parent ItemGeneral
 * @desc Maximum number of icons that can be displayed for Add/Remove States/Buffs.
 * @default 8
 *
 * @param MultiplierStandard:num
 * @text Multiplier Standard
 * @parent ItemGeneral
 * @desc Constant standard to filter out random values when calculating the damage multiplier.
 * @default 1000000
 *
 * @param DrawItemData:func
 * @text JS: Draw Item Data
 * @parent ItemGeneral
 * @type note
 * @desc Code used to draw the item data for the Shop Status Window.
 * @default "const lineHeight = this.lineHeight();\nlet x = 0;\nlet y = 0;\nlet width = this.innerWidth;\nlet height = this.innerHeight;\nlet hw = Math.floor(width / 2);\nlet hx = x + width - hw;\n\n// Draw Item Name and Quantity\nthis.drawItemName(this._item, x + this.itemPadding(), y, width - this.itemPadding() * 2);\nthis.drawItemDarkRect(x, y, width);\ny += lineHeight;\n\n// Draw Main Item Properties\nif (this.drawItemConsumable(x, y, hw)) y += 0;\nif (this.drawItemQuantity(hx, y, hw)) y += lineHeight;\nif (this._item.occasion < 3) {\n    y = this.drawItemDamage(x, y, width);\n    y = this.drawItemEffects(x, y, width);\n}\ny = this.drawItemCustomEntries(x, y, width);\n\n// Draw Remaining Item Properties\nif (this._item.occasion < 3) {\n    if (this.drawItemOccasion(x, y, hw)) y += 0;\n    if (this.drawItemScope(hx, y, hw)) y += lineHeight;\n    if (this.drawItemHitType(x, y, hw)) y += 0;\n    if (this.drawItemSuccessRate(hx, y, hw)) y += lineHeight;\n    if (this.drawItemSpeed(x, y, hw)) y += 0;\n    if (this.drawItemRepeats(hx, y, hw)) y += lineHeight;\n}\n\n// Fill Rest of the Window\nthis.drawItemDarkRect(x, y, width, height - y);"
 *
 * @param Vocabulary
 * @parent ItemData
 *
 * @param LabelConsume:str
 * @text Consumable
 * @parent Vocabulary
 * @desc Vocabulary used for this data entry.
 * @default Consumable
 *
 * @param Consumable:str
 * @text Yes
 * @parent LabelConsume:str
 * @desc Vocabulary used for this data entry.
 * @default ✔
 *
 * @param NotConsumable:str
 * @text No
 * @parent LabelConsume:str
 * @desc Vocabulary used for this data entry.
 * @default ✘
 *
 * @param Occasions
 * @parent Vocabulary
 *
 * @param Occasion0:str
 * @text Always
 * @parent Occasions
 * @desc Vocabulary used for this data entry.
 * @default Anytime Use
 *
 * @param Occasion1:str
 * @text Battle Screen
 * @parent Occasions
 * @desc Vocabulary used for this data entry.
 * @default Battle-Only
 *
 * @param Occasion2:str
 * @text Menu Screen
 * @parent Occasions
 * @desc Vocabulary used for this data entry.
 * @default Field-Only
 *
 * @param Occasion3:str
 * @text Never
 * @parent Occasions
 * @desc Vocabulary used for this data entry.
 * @default -
 *
 * @param Scope
 * @parent Vocabulary
 *
 * @param Scope0:str
 * @text None
 * @parent Scope
 * @desc Vocabulary used for this data entry.
 * @default No Target
 *
 * @param Scope1:str
 * @text 1 Enemy
 * @parent Scope
 * @desc Vocabulary used for this data entry.
 * @default 1 Foe
 *
 * @param Scope2:str
 * @text All Enemies
 * @parent Scope
 * @desc Vocabulary used for this data entry.
 * @default All Foes
 *
 * @param Scope3:str
 * @text 1 Random Enemy
 * @parent Scope
 * @desc Vocabulary used for this data entry.
 * @default Random Foe
 *
 * @param Scope4:str
 * @text 2 Random Enemies
 * @parent Scope
 * @desc Vocabulary used for this data entry.
 * @default 2 Random Foes
 *
 * @param Scope5:str
 * @text 3 Random Enemies
 * @parent Scope
 * @desc Vocabulary used for this data entry.
 * @default 3 Random Foes
 *
 * @param Scope6:str
 * @text 4 Random Enemies
 * @parent Scope
 * @desc Vocabulary used for this data entry.
 * @default 4 Random Foes
 *
 * @param Scope7:str
 * @text 1 Ally
 * @parent Scope
 * @desc Vocabulary used for this data entry.
 * @default 1 Ally
 *
 * @param Scope8:str
 * @text All Allies
 * @parent Scope
 * @desc Vocabulary used for this data entry.
 * @default Alive Allies
 *
 * @param Scope9:str
 * @text 1 Ally (Dead)
 * @parent Scope
 * @desc Vocabulary used for this data entry.
 * @default Dead Ally
 *
 * @param Scope10:str
 * @text All Allies (Dead)
 * @parent Scope
 * @desc Vocabulary used for this data entry.
 * @default Dead Allies
 *
 * @param Scope11:str
 * @text The User
 * @parent Scope
 * @desc Vocabulary used for this data entry.
 * @default User
 *
 * @param Scope12:str
 * @text 1 Ally (DoA)
 * @parent Scope
 * @desc Vocabulary used for this data entry.
 * @default Any Ally
 *
 * @param Scope13:str
 * @text All Allies (DoA)
 * @parent Scope
 * @desc Vocabulary used for this data entry.
 * @default All Allies
 *
 * @param Scope14:str
 * @text Enemies & Allies
 * @parent Scope
 * @desc Vocabulary used for this data entry.
 * @default Everybody
 *
 * @param BattleCore
 * @text Battle Core Support
 * @parent Vocabulary
 *
 * @param ScopeRandomAny:str
 * @text x Random Any
 * @parent BattleCore
 * @desc Vocabulary used for <Target: x Random Any> notetag.
 * @default %1 Random Units
 *
 * @param ScopeRandomEnemies:str
 * @text x Random Enemies
 * @parent BattleCore
 * @desc Vocabulary used for <Target: x Random Enemies> notetag.
 * @default %1 Random Foes
 *
 * @param ScopeRandomAllies:str
 * @text x Random Allies
 * @parent BattleCore
 * @desc Vocabulary used for <Target: x Random Allies> notetag.
 * @default %1 Random Allies
 *
 * @param ScopeAlliesButUser:str
 * @text All Allies But User
 * @parent BattleCore
 * @desc Vocabulary used for <Target: All Allies But User> notetag.
 * @default Other Allies
 *
 * @param ScopeAllyOrEnemy:str
 * @text Ally or Enemy
 * @parent BattleCore
 * @desc Vocabulary used for <Target: Ally or Enemy> notetag.
 * @default Ally/Enemy
 *
 * @param ScopeEnemyOrAlly:str
 * @text Enemy or Ally
 * @parent BattleCore
 * @desc Vocabulary used for <Target: Enemy or Ally> notetag.
 * @default Enemy/Ally
 *
 * @param LabelSpeed:str
 * @text Speed
 * @parent Vocabulary
 * @desc Vocabulary used for this data entry.
 * @default Speed
 *
 * @param Speed2000:str
 * @text >= 2000 Speed
 * @parent LabelSpeed:str
 * @desc Vocabulary used for this data entry.
 * @default Fastest
 *
 * @param Speed1000:str
 * @text >= 1000 Speed
 * @parent LabelSpeed:str
 * @desc Vocabulary used for this data entry.
 * @default Faster
 *
 * @param Speed1:str
 * @text >= 1 Speed
 * @parent LabelSpeed:str
 * @desc Vocabulary used for this data entry.
 * @default Fast
 *
 * @param Speed0:str
 * @text == 0 Speed
 * @parent LabelSpeed:str
 * @desc Vocabulary used for this data entry.
 * @default Normal
 *
 * @param SpeedNeg999:str
 * @text >= -999 Speed
 * @parent LabelSpeed:str
 * @desc Vocabulary used for this data entry.
 * @default Slow
 *
 * @param SpeedNeg1999:str
 * @text >= -1999 Speed
 * @parent LabelSpeed:str
 * @desc Vocabulary used for this data entry.
 * @default Slower
 *
 * @param SpeedNeg2000:str
 * @text <= -2000 Speed
 * @parent LabelSpeed:str
 * @desc Vocabulary used for this data entry.
 * @default Slowest
 *
 * @param LabelSuccessRate:str
 * @text Success Rate
 * @parent Vocabulary
 * @desc Vocabulary used for this data entry.
 * @default Accuracy
 *
 * @param LabelRepeats:str
 * @text Repeats
 * @parent Vocabulary
 * @desc Vocabulary used for this data entry.
 * @default Hits
 *
 * @param LabelHitType:str
 * @text Hit Type
 * @parent Vocabulary
 * @desc Vocabulary used for this data entry.
 * @default Type
 *
 * @param HitType0:str
 * @text Certain Hit
 * @parent LabelHitType:str
 * @desc Vocabulary used for this data entry.
 * @default Neutral
 *
 * @param HitType1:str
 * @text Physical
 * @parent LabelHitType:str
 * @desc Vocabulary used for this data entry.
 * @default Physical
 *
 * @param HitType2:str
 * @text Magical
 * @parent LabelHitType:str
 * @desc Vocabulary used for this data entry.
 * @default Magical
 *
 * @param LabelElement:str
 * @text Element
 * @parent Vocabulary
 * @desc Vocabulary used for this data entry.
 * @default Element
 *
 * @param ElementWeapon:str
 * @text Weapon-Based
 * @parent LabelElement:str
 * @desc Vocabulary used for this data entry.
 * @default \I[97]Weapon
 *
 * @param ElementNone:str
 * @text Nonelement Element
 * @parent LabelElement:str
 * @desc Vocabulary used for this data entry.
 * @default \I[160]No Element
 *
 * @param DamageType
 * @text Damage Type
 * @parent Vocabulary
 *
 * @param DamageType1:str
 * @text HP Damage
 * @parent DamageType
 * @desc Vocabulary used for this data entry. If Visu_1_BattleCore
 * is installed, priority goes to its Damage Style settings.
 * @default %1 Damage Multiplier
 *
 * @param DamageType2:str
 * @text MP Damage
 * @parent DamageType
 * @desc Vocabulary used for this data entry. If Visu_1_BattleCore
 * is installed, priority goes to its Damage Style settings.
 * @default %1 Damage Multiplier
 *
 * @param DamageType3:str
 * @text HP Recovery
 * @parent DamageType
 * @desc Vocabulary used for this data entry. If Visu_1_BattleCore
 * is installed, priority goes to its Damage Style settings.
 * @default %1 Recovery Multiplier
 *
 * @param DamageType4:str
 * @text MP Recovery
 * @parent DamageType
 * @desc Vocabulary used for this data entry. If Visu_1_BattleCore
 * is installed, priority goes to its Damage Style settings.
 * @default %1 Recovery Multiplier
 *
 * @param DamageType5:str
 * @text HP Drain
 * @parent DamageType
 * @desc Vocabulary used for this data entry. If Visu_1_BattleCore
 * is installed, priority goes to its Damage Style settings.
 * @default %1 Drain Multiplier
 *
 * @param DamageType6:str
 * @text MP Drain
 * @parent DamageType
 * @desc Vocabulary used for this data entry. If Visu_1_BattleCore
 * is installed, priority goes to its Damage Style settings.
 * @default %1 Drain Multiplier
 *
 * @param Effects
 * @parent Vocabulary
 *
 * @param LabelRecoverHP:str
 * @text Recover HP
 * @parent Effects
 * @desc Vocabulary used for this data entry.
 * @default %1 Recovery
 *
 * @param LabelRecoverMP:str
 * @text Recover MP
 * @parent Effects
 * @desc Vocabulary used for this data entry.
 * @default %1 Recovery
 *
 * @param LabelRecoverTP:str
 * @text Recover TP
 * @parent Effects
 * @desc Vocabulary used for this data entry.
 * @default %1 Recovery
 *
 * @param LabelSelfGainTP:str
 * @text Self Gain TP
 * @parent Effects
 * @desc Vocabulary used for this data entry.
 * @default User %1
 *
 * @param LabelDamageHP:str
 * @text Damage HP
 * @parent Effects
 * @desc Vocabulary used for this data entry.
 * @default %1 Damage
 *
 * @param LabelDamageMP:str
 * @text Damage MP
 * @parent Effects
 * @desc Vocabulary used for this data entry.
 * @default %1 Damage
 *
 * @param LabelDamageTP:str
 * @text Damage TP
 * @parent Effects
 * @desc Vocabulary used for this data entry.
 * @default %1 Damage
 *
 * @param LabelApply:str
 * @text Add State/Buff
 * @parent Effects
 * @desc Vocabulary used for this data entry.
 * @default Applies
 *
 * @param LabelRemove:str
 * @text Remove State/Buff
 * @parent Effects
 * @desc Vocabulary used for this data entry.
 * @default Removes
 *
 * @param EquipType
 * @parent Vocabulary
 * @text Equip Type
 *
 * @param WeaponType:str
 * @text Weapon Type
 * @parent EquipType
 * @desc Vocabulary used for this data entry.
 * @default Weapon Type
 *
 * @param ArmorType:str
 * @text Armor Type
 * @parent EquipType
 * @desc Vocabulary used for this data entry.
 * @default Armor Type
 *
 * @param NoEquipTypeResult:str
 * @text No Equip Type
 * @parent EquipType
 * @desc Marker used to show an unlisted equip type.
 * @default -
 *
 */
//=============================================================================

function _0x2989(_0x192791,_0x348cb4){const _0x47f7aa=_0x47f7();return _0x2989=function(_0x2989fc,_0x49978f){_0x2989fc=_0x2989fc-0x95;let _0x284189=_0x47f7aa[_0x2989fc];return _0x284189;},_0x2989(_0x192791,_0x348cb4);}function _0x47f7(){const _0x5e8d45=['top','_statusWindow','Game_Enemy_traitObjects_artifact','Scene_Shop_helpWindowRect','updatedLayoutStyle','removeStateBuffChanges','loadPicture','itemPadding','textSizeEx','gaugeLineHeight','LUK','speed','Window_Selectable_refresh','buttonAssistKey3','English','ListWindowCols','_resetFontSize','drawTextEx','getItemDamageAmountTextBattleCore','defaultItemMax','83jvlpGx','Window_EquipItem_includes','contents','Game_Party_setupBattleTestItems_artifact','price','update','background','elements','_classIDs','_category','user','canSortListItemScene','isOpen','ShopMenuStatusStandard','_categoryNameWindow','Scene_Shop_onSellOk','lineHeight','FUNC','getShopTrackingData','mat','MEV','SUCCESS\x20RATE','middle','goldWindowRectItemsEquipsCore','Scope1','Parse_Notetags_EquipSlots','calcWindowHeight','createSlotWindow','sellPriceOfItem','isStackableArtifact','reloadMapIfUpdated','SortByIDandPriority','fontSizeRatio','playBuzzerSound','getEmptyEquipSlotOfSameEtype','getItemColor','addCancelCommand','effects','itemTextAlign','getProxyItem','getColor','_scene','MaxWeapons','NonRemoveETypes','isDrawItemNumber','Nonconsumable','_slotWindow','mmp','drawParamsItemsEquipsCore','initNewItemsList','Scene_Shop_sellingPrice','GRD','activateItemWindow','_buyWindowLastIndex','CNT','getTextColor','CRI','playCursorSound','Scene_Item_create','isHovered','placeNewLabel','Scene_Shop_goldWindowRect','weapons','FadeSpeed','optimize','Window_ItemList_updateHelp','clamp','armor','troopArtifacts','value2','buyWindowRectItemsEquipsCore','Scene_Equip_create','LabelRemove','geUpdatedLayoutStatusWidth','NAME','elementId','log','standardIconWidth','initialize','EFFECT_ADD_BUFF','_tempActorA','getEtypeIDs','isBuyCommandEnabled','DisabledVanilla','setNewItem','drawItemCustomEntries','Window_ShopCommand_initialize','isRightInputMode','_equips','damage','Window_Selectable_update','itemAt','IconSet','slotWindowRect','Translucent','onTouchSelectModernControls','_actor','drawItemEffectsMpDamage','EVAL','ITEMS_EQUIPS_CORE','EFFECT_REMOVE_DEBUFF','_allowArtifactParamBase','%1\x27s\x20version\x20does\x20not\x20match\x20plugin\x27s.\x20Please\x20update\x20it\x20in\x20the\x20Plugin\x20Manager.','helpAreaTop','paramchangeTextColor','BatchShop','itemEnableJS','Scene_Item_createCategoryWindow','REMOVED\x20EFFECTS','version','Game_Party_initialize','drawItemEffects','isBattleTest','Scene_Equip_onSlotOk','onSellOkItemsEquipsCore','gold','_resetFontColor','drawIcon','changeTextColor','buttonAssistText3','rateHP','×%1','updateChangedSlots','ParseClassNotetags','isDualWield','setItemWindow','object','addChild','MDF','translucentOpacity','slotWindowRectItemsEquipsCore','buttonAssistKey2','drawPossession','isRepeated','categoryNameWindowDrawText','isOptimizeCommandEnabled','normalColor','isEnabled','Scene_Shop_createCategoryWindow','MaxIcons','getItemEffectsHpDamageText','convertInitEquipsToItems','removeBuff','baseSellingPrice','purifyCursedEquips','innerHeight','_purchaseOnly','buttonAssistText1','makeDeepCopy','mainCommandWidth','AllArmors','categoryNameWindowCenter','MRG','cancel','_item','armor-%1','createStatusWindow','getItemEffectsRemovedStatesBuffsText','Scene_Shop_sellWindowRect','proxyItem','USER\x20TP\x20GAIN','cursorRight','hitIndex','isOptimizeCommandAdded','ScopeRandomAny','canShiftRemoveEquipment','Blacklist','getItemEffectsHpRecoveryText','smallParamFontSize','setItemDelay','traits','buttonAssistSlotWindowShift','CmdIconClear','fill','placeItemNewLabel','===','onCategoryCancel','hideAdditionalSprites','nextActor','drawItemEffectsRemovedStatesBuffs','Scene_Shop_create','weapon','onBuyCancel','getInputMultiButtonStrings','pageup','getPurifyTransformation','drawItemEquipSubType','Step1Start','EXR','isOpenAndActive','3984KHosUQ','isOptimizeEquipOk','Game_Party_gainItem','forceChangeEquip','helpAreaHeight','postCreateSellWindowItemsEquipsCore','Scene_Equip_slotWindowRect','ShowAllSwitches','_helpWindow','DEF','releaseUnequippableItems','HIT','W%1','_skillIDs','currentClass','ARRAYEVAL','Step2Start','UNDEFINED!','traitObjects','buyWindowRect','hitType','isItem','textLocale','processShopCondListingOnSellItem','BuyTurnSwitchOff','WEAPON','compare','ParseWeaponNotetags','mainFontSize','isClearCommandAdded','buy','Window_ItemCategory_setItemWindow','VisuMZ_2_WeaponSwapSystem','buyingPrice','_shopStatusMenuMode','(%1)','Step1End','SellTurnSwitchOn','statusWindowRect','isPureVanillaMode','tradeItemWithParty','onTouchSelect','drawEquipData','commandStyleCheck','MP\x20RECOVERY','LabelRecoverMP','loseItem','bind','drawItemScope','switchProxyItem','4954wKggGP','n/a','windowPadding','onCategoryCancelItemsEquipsCore','allowCreateStatusWindow','_getClassRequirements','ClassicArmorParameters','Window_EquipItem_isEnabled','DrawItemData','getEquipRequirements','isEquipCommandAdded','(+%1)','RemoveEquipText','commandBuyItemsEquipsCore','Categories','buttonAssistCategory','optimizeEquipments','getItemsEquipsCoreBackColor1','getItemSpeedLabel','buttonAssistSmallIncrement','Scene_Shop_buyWindowRect','isVisuMzLocalizationEnabled','Scene_Shop_activateSellWindow','goodsToItem','hide','drawParamText','Parse_Notetags_Sorting','isProxyItem','drawItemStyleIconText','equipAdjustHpMp','Game_Actor_changeEquip','actorId','drawItemName','mainAreaHeight','getItemSpeedText','drawItemActorMenuImage','powerDownColor','processShopCondListingOnBuyItem','onTouchCancel','members','isPurifyItemSwapOk','isToggleSkill','maxItemAmount','Game_Actor_changeClass','getEtypeIdWithName','addInnerChild','isPageChangeRequested','prepareItemCustomData','value1','sellWindowRect','colSpacing','equipSlotIndex','778596CwanUp','buttonAssistRemove','description','damageColor','getInputButtonString','createCommandNameWindow','_tempActor','getArmorIdWithName','isGoodShown','EFFECT_GAIN_TP','test','drawItemEffectsAddedStatesBuffs','Scene_Equip_createSlotWindow','_categoryWindow','_doubleTouch','processTouchModernControls','isEquipCommandEnabled','LabelHitType','ConvertNumberToString','isShowNew','isHandled','initNewLabelSprites','drawItem','OffsetY','New','_handlers','process_VisuMZ_ItemsEquipsCore_RegExp','ItemMenuStatusRect','playEquip','drawItemEffectsMpRecovery','shouldCommandWindowExist','icon','drawItemQuantity','makeItemList','left','QoL','_scrollDuration','SpeedNeg999','Game_Actor_paramPlus','getNextAvailableEtypeId','isArmor','isSceneShop','boxWidth','isSkill','mainFontFace','isNewItem','_commandNameWindow','length','drawItemDamageAmount','_cache','match','LabelApply','categoryWindowRect','categoryList','versionId','onTouchSelectModern','setObject','getItemEffectsMpRecoveryLabel','Scope7','3468114rDFTcH','Window_ShopBuy_price','FDR','setHp','cursorUp','helpWindowRect','param','loadFaceImages','playOkSound','isSoleArmorType','LabelElement','onSlotCancel','CmdIconOptimize','Scene_Equip_onSlotCancel','call','_bypassNewLabel','EquipParams','FieldUsable','commandNameWindowDrawBackground','Scene_Equip_onActorChange','SellPriceRate','systemColor','DrawPortraitJS','max','Slots','EFFECT_RECOVER_HP','playCancel','PDR','onSellCancel','FadeLimit','armorTypes','gaugeBackColor','commandWindowRectItemsEquipsCore','RegularItems','getEquipDataStyle','category','Scene_Shop_onCategoryCancel','Scene_Shop_doBuy','createTempActorEquips','calcEquipItemPerformance','meetsItemConditionsNotetags','statusWindowRectItemsEquipsCore','addLoadListener','drawItemOccasion','push','initEquips','Scene_Equip_itemWindowRect','canConsumeItem','CEV','ARRAYNUM','indexOf','optKeyItemsNumber','newLabelEnabled','EFFECT_REMOVE_BUFF','doSell','forceResetEquipSlots','tpGain','type','ARRAYSTRUCT','DrawEquipClassicData','MAXMP','PHA','Parse_Notetags_ParamJS','prepareRefreshItemsEquipsCoreLayout','equipSlots','ScopeAlliesButUser','_data','opacity','paramValueByName','getEtypeIDsCache','NUM','shift','maxVisibleItems','parseLocalizedText','VisuMZ_1_MainMenuCore','blt','drawEquipDataCompare','isCommandEnabled','allowCommandWindowCursorUp','drawItemRepeats','updateHelp','doBuy','getItemEffectsTpRecoveryText','IncludeShopItem','updateNewLabelOpacity','meetsClassRequirements','meetsShopListingConditions','value','isCursedItem','isLearnedSkill','ScopeAllyOrEnemy','StatusWindow','equip2','getItemEffectsMpDamageLabel','Type','getItemEffectsHpRecoveryLabel','armors','_newLabelSprites','Scene_Equip','_goods','getParamValueClassicCore','partyArtifacts','Window_ItemList_drawItem','removeDebuff','Game_Actor_forceChangeEquip','Window_ShopBuy_refresh','FontSize','drawText','categoryWindowRectItemsEquipsCore','contentsBack','DAMAGE\x20MULTIPLIER','clearNewLabelFromItem','initShopTrackingData','PurifyActors','Scene_Item_createItemWindow','sell','ConvertParams','1059928KSKrYE','updateCommandNameWindow','categoryStyleCheck','setValue','checkItemConditionsSwitchNotetags','addBuyCommand','TRG','number','_commandWindow','uiMenuStyle','dataId','equipTypes','onMenuImageLoad','Scene_Load_reloadMapIfUpdated','updateCategoryNameWindow','isHoverEnabled','cursorLeft','MDR','CursedTextPopup','canEquip','wtypeId','Scene_Shop_createSellWindow','loadSystem','format','agi','bitmap','toggleType','process_VisuMZ_ItemsEquipsCore_Notetags','SwitchID','ELEMENT','Window_ItemCategory_makeCommandList','CommandAddOptimize','map','Window_ItemList_maxCols','resetTextColor','drawCustomShopGraphicLoad','CoreEngine','getItemConsumableLabel','start','MCR','isShiftShortcutKeyForRemove','getShopTrackingItem','ParseItemNotetags','Scope%1','weapon-%1','prepareNewEquipSlotsOnLoad','Game_BattlerBase_param','isEquipped','Scene_Equip_helpWindowRect','isSoleWeaponType','changeClass','createCommandWindow','changeEquipBase','actor','level','Scene_Item_categoryWindowRect','Step3Start','drawItemConsumable','_calculatingJSParameters','concat','drawItemData','%1\x20is\x20missing\x20a\x20required\x20plugin.\x0aPlease\x20install\x20%2\x20into\x20the\x20Plugin\x20Manager.','addShopTrackingGoldSell','HP\x20DAMAGE','sellWindowRectItemsEquipsCore','BattleUsable','addStateBuffChanges','trim','Window_Selectable_setHelpWindowItem','isKeyItem','OffsetX','isCancelled','ItemMenuStatusBgType','Game_BattlerBase_meetsItemConditions','Scene_Equip_createCommandWindow','revertGlobalNamespaceVariables','items','goldWindowRect','drawItemEffectsTpDamage','categoryStyle','deselect','isEquipChangeOk','statusWidth','DrawBackRect','getItemEffectsSelfTpGainLabel','getItemEffectsMpDamageText','visible','ADDED\x20EFFECTS','refreshCursor','Window_EquipSlot_isEnabled','Parse_Notetags_Prices','getItemEffects','isTroopArtifact','0000','refreshItemsEquipsCoreNoMenuImage','EFFECT_ADD_DEBUFF','getItemDamageElementLabel','addShopTrackingItem','iconWidth','setShopStatusWindowMode','drawParamName','SpeedNeg2000','iconIndex','bestEquipItem','createNewLabelSprite','isCursorMovable','nonRemovableEtypes','Game_Party_numItems','_bypassProxy','Window_ItemList_item','BackRectColor','needsNewTempActor','selfTP','DrawIcons','Remove\x20all\x20available\x20equipment.','Game_Actor_discardEquip','Settings','+%1%','ClassicWeaponParameters','Consumable','processShiftRemoveShortcut','ScopeRandomAllies','drawItemCost','Scene_Item_itemWindowRect','SortBy','maxmp','setHelpWindow','LabelRepeats','commandBuy','getWeaponIdWithName','drawNewLabelIcon','AllItems','CheckCursedItemMsg','CustomParamNames','MaxArmors','NotConsumable','CannotEquipMarker','_forcedSlots','Parse_Notetags_EnableJS','_newLabelOpacity','getParamValueClassicNoCore','prepareNextScene','hpRate','sellingPrice','isPlaytest','ShopListingRegExp','commandWindowRect','drawEquipDataClassic','equips','refreshDelay','resetShopSwitches','commandEquip','?????','drawItemDarkRect','NoEquipTypeResult','some','PurchaseOnly','addItemCategory','889FnfFDu','Actors','BorderRegExp','drawItemCustomEntryLine','artifactIDs','_checkEquipRequirements','popScene','create','setHandler','_slotId','Game_Actor_artifact','code','addState','Equip\x20the\x20strongest\x20available\x20equipment.','gainTP','DrawEquipData','rateMP','\x5cI[%1]','exit','active','_tempActorB','ATK','repeats','_weaponIDs','onTouchOk','Window_EquipCommand_initialize','WeaponType','item','_numberWindow','isMainMenuCoreMenuImageOptionAvailable','hasItem','scrollTo','scope','setupItemDamageTempActors','drawUpdatedParamName','equipCmdDesc','filter','_itemWindow','processCursorSpecialCheckModernControls','Game_Actor_isEquipChangeOk','getMatchingInitEquip','meetsEquipRequirement','getItemDamageAmountTextOriginal','isUseModernControls','maxItems','NeverUsable','postCreateSlotWindowItemsEquipsCore','toLowerCase','getDamageStyle','name','Scene_Shop_categoryWindowRect','AGI','replace','Weapon\x20Type','+%1','onSlotOk','onDatabaseLoaded','addClearCommand','getItemEffectsMpRecoveryText','getItemDamageAmountLabel','CmdTextAlign','Scene_Shop_commandWindowRect','Step2End','LabelSuccessRate','helpDesc','MRF','anyEmptyEquipSlotsOfSameEtype','getItemSuccessRateText','_list','isArray','getItemDamageElementText','ExtDisplayedParams','ItemsEquipsCore','EquipDelayMS','getItemEffectsTpDamageText','Game_Party_gainItem_artifact','pop','show','CmdIconSell','Scene_Boot_onDatabaseLoaded','uiHelpPosition','determineBaseSellingPrice','cursorDown','isUseItemsEquipsCoreUpdatedLayout','drawItemDamageElement','Text','setCategory','_cache_etypeIDs','onCategoryOk','DamageType%1','Parse_Notetags_Batch','canEquipArmor','HP\x20RECOVERY','SwitchSell','move','_shopTrackingData','addShopTrackingItemSell','itemWindowRect','round','isClicked','Game_Item_setObject','Scene_Equip_commandWindowRect','HRG','ElementWeapon','remove','commandNameWindowDrawText','createCategoryNameWindow','DrawParamJS','PurifyParty','HiddenItemB','EquipAdjustHpMp','commandNameWindowCenter','CmdIconCancel','TP\x20DAMAGE','paramJS','drawItemEffectsSelfTpGain','adjustItemWidthByStatus','paramPlusItemsEquipsCoreCustomJS','down','mpRate','itypeId','ParseAllNotetags','isWeapon','CmdStyle','numberWindowRect','SCOPE','_armorIDs','CmdHideDisabled','makeCommandList','createItemWindow','equipItems','sort','addItemCategories','getItemEffectsSelfTpGainText','changePaintOpacity','meetsItemConditionsJS','fontSize','getItemScopeText','LabelRecoverHP','getItemDamageAmountText','iconText','getItemDamageAmountLabelBattleCore','addOptimizeCommand','StatusWindowWidth','CmdCancelRename','\x5cb%1\x5cb','clearCmdDesc','discardEquip','MAXHP','onBuyOk','addShopTrackingGoldBuy','prepare','getItemEffectsTpDamageLabel','Parse_Notetags_ParamValues','buttonAssistOffset3','nonOptimizeEtypes','mhp','occasion','onBuyCancelItemsEquipsCore','RemoveEquipIcon','addWindow','splice','addCommand','drawItemEquipType','getItemEffectsAddedStatesBuffsLabel','EnableLayout','setMp','clearNewItem','OCCASION','isTriggered','isEquipItem','text','alterSkillName','postCreateCategoryWindowItemsEquipsCore','_buttonAssistWindow','ItemQuantityFontSize','_itemIDs','isBottomHelpMode','setItem','NoChangeMarker','flatHP','\x5cI[%1]%2','find','_getEquipRequirements','drawItemKeyData','paintOpacity','money','CONSUMABLE','canEquipWithOptimize','atypeId','isEquipWtypeOk','ARRAYSTR','isShiftRemoveShortcutEnabled','EquipScene','EFFECT_RECOVER_MP','allMembers','buttonAssistText2','drawEquipDataDouble','numberWindowRectItemsEquipsCore','categoryNameWindowDrawBackground','auto','classic','consumeItem','EQUIP_DELAY_MS','uiInputPosition','DoubleArmorParameters','item-%1','sortPriority','meetsItemConditions','getItemHitTypeLabel','Window_ItemList_colSpacing','Scene_Shop_doSell','categories','commandSellItemsEquipsCore','getItemSuccessRateLabel','Scene_Shop_onBuyCancel','Speed1','commandSell','gainItem','CommandAddClear','MaxMP','_dummyWindow','LayoutStyle','maxCols','Scene_Shop','getItemRepeatsText','addShopTrackingItemBuy','Window_ShopBuy_goodsToItem','VisuMZ_1_BattleCore','isArtifact','FontColor','Window_ItemCategory_initialize','floor','etypeId','Window_ShopBuy_item','mdf','return\x200','toUpperCase','center','processHandling','consumable','getItemEffectsHpDamageLabel','getItemEffectsAddedStatesBuffsText','Speed0','parse','buffIconIndex','iconHeight','LabelRecoverTP','currentExt','allowShiftScrolling','_money','RegExp','onSellOk','innerWidth','includes','isPartyArtifact','cursorPageup','atk','BuyPriceJS','ScopeEnemyOrAlly','DoubleWeaponParameters','hideDisabledCommands','ARRAYJSON','MaxItems','ItemSceneAdjustItemList','postCreateItemsEquipsCore','_itemData','forceChangeEquipSlots','MANUAL','helpDescriptionText','paramPlus','status','_newItemsList','VisuMZ_0_CoreEngine','Icon','MaxHP','mainAreaBottom','index','_allowArtifactTraitObjects','ItemQuantityFmt','textColor','TP\x20RECOVERY','min','getItemQuantityText','itemDataFontSize','SPEED','maxBattleMembers','Game_Party_consumeItem','possession','drawUpdatedAfterParamValue','drawActorParamClassic','HideAnySwitches','getItemEffectsTpRecoveryLabel','customEquipParams','equipHasCustomParams','parameters','Scene_Equip_commandEquip','onSellItem','drawActorCharacter','drawRemoveItem','formula','%1-%2','Scene_Shop_commandBuy','cursorPagedown','getMenuImage','HiddenItemA','setHelpWindowItem','removeState','sortListItemScene','EquipDataStyle','meetsEquipRequirements','SwitchBuy','registerCommand','ElementNone','getItemDamageAmountLabelOriginal','values','actorParams','refresh','clear','addSellCommand','isEquipTypeSealed','_bypassReleaseUnequippableItemsItemsEquipsCore','adjustHiddenShownGoods','ShopScene','addEquipCommand','STRUCT','categoryItemTypes','removeBattleTestArtifacts','activate','Window_ItemList_makeItemList','getShopTrackingItemBuy','equip','#%1','canUse','Speed1000','REC','modifiedBuyPriceItemsEquipsCore','Pick\x20and\x20choose\x20equipment\x20to\x20change.','createSellWindow','ItemScene','deepCopy','Armor\x20Type','drawCurrencyValue','Scene_Shop_commandSell','A%1','note','troopArtifactIDs','prototype','HideAllSwitches','getItemConsumableText','Scene_Item_helpWindowRect','DrawEquipDoubleData','SpeedNeg1999','drawItemNumber','inBattle','onActorChange','drawItemStyleIcon','updateMoneyAmount','Game_BattlerBase_param_artifact','Parse_Notetags_Category','resetFontSettings','SellTurnSwitchOff','_goodsCount','drawItemEffectsTpRecovery','isSellCommandEnabled','createBitmap','makeItemData','REPEAT','_buyWindow','HIT\x20TYPE','Scene_Shop_numberWindowRect','drawItemSpeed','processCursorHomeEndTrigger','refreshActor','right','itemLineRect','deactivate','ceil','onBuyItem','double','localeCompare','split','Scene_Shop_onSellCancel','AlreadyEquipMarker','MP\x20DAMAGE','ParseArmorNotetags','484tfagWU','pagedown','4515KeUWHY','Scene_ItemBase_activateItemWindow','mainAreaTop','Damage\x20Formula\x20Error\x20for\x20%1','updateSmoothScroll','standardIconHeight','setBackgroundType','isCustomParameter','LabelSpeed','MAT','buttonAssistKey1','SetupArtifactItemIDs','flatMP','getItemOccasionText','drawUpdatedBeforeParamValue','constructor','processDownCursorSpecialCheckModernControls','Width','_customItemInfo','select','drawing','buttonAssistItemListRequirement','powerUpColor','6354450qOYvxp','helpWindowRectItemsEquipsCore','Step3End','Scene_Shop_statusWindowRect','fillRect','callUpdateHelp','processCursorMove','isClearCommandEnabled','getItemEffectsRemovedStatesBuffsLabel','getItemHitTypeText','createCategoryWindow','Style','postCreateItemWindowModernControls','every','Window_Selectable_initialize','TGR','weaponTypes','FontFace','isPressed','commandName','numItems','paramBase','_sellWindow','params','keyItem','getItemRepeatsLabel','isUseParamNamesWithIcons','width','getClassRequirements','Scene_Shop_onBuyOk','Scene_Shop_prepare','setTopRow','_shopStatusMenuAlly','EVA','getClassIdWithName','AllWeapons','setStatusWindow','Game_BattlerBase_paramPlus_artifact','changeBuff','refreshActorEquipSlotsIfUpdated','EFFECT_REMOVE_STATE','commandStyle','_etypeIDs','_paramPlus','currentSymbol','processCursorMoveModernControls','SetupProxyItemGroup','getItemsEquipsCoreBackColor2','partyArtifactIDs','_newLabelOpacityUpperLimit','Param','%1%','getShopTrackingGoldBuy','TCR','paramValueFontSize','ParamChangeFontSize','changeEquip','height','smoothSelect','ARMOR','drawItemEffectsHpRecovery','drawNewLabelText','ShiftShortcutKey','limitedPageUpDownSceneCheck','battleMembers','checkShiftRemoveShortcut','LabelDamageHP','currencyUnit'];_0x47f7=function(){return _0x5e8d45;};return _0x47f7();}const _0x39ebd3=_0x2989;(function(_0x42fa36,_0x206621){const _0x108c3f=_0x2989,_0x5742cb=_0x42fa36();while(!![]){try{const _0x6b0a7f=-parseInt(_0x108c3f(0x454))/0x1*(-parseInt(_0x108c3f(0xdf))/0x2)+-parseInt(_0x108c3f(0x113))/0x3+-parseInt(_0x108c3f(0x3e3))/0x4*(-parseInt(_0x108c3f(0x3e5))/0x5)+parseInt(_0x108c3f(0xad))/0x6*(parseInt(_0x108c3f(0x261))/0x7)+-parseInt(_0x108c3f(0x1c3))/0x8+-parseInt(_0x108c3f(0x14e))/0x9+parseInt(_0x108c3f(0x3fc))/0xa;if(_0x6b0a7f===_0x206621)break;else _0x5742cb['push'](_0x5742cb['shift']());}catch(_0x2acd5b){_0x5742cb['push'](_0x5742cb['shift']());}}}(_0x47f7,0x3ecea));var label='ItemsEquipsCore',tier=tier||0x0,dependencies=[],pluginData=$plugins[_0x39ebd3(0x285)](function(_0x51bf9e){const _0x406f59=_0x39ebd3;return _0x51bf9e[_0x406f59(0x370)]&&_0x51bf9e[_0x406f59(0x115)][_0x406f59(0x35f)]('['+label+']');})[0x0];VisuMZ[label][_0x39ebd3(0x237)]=VisuMZ[label]['Settings']||{},VisuMZ[_0x39ebd3(0x1c2)]=function(_0x5499ac,_0x514050){const _0xab436e=_0x39ebd3;for(const _0x4c2812 in _0x514050){if(_0x4c2812[_0xab436e(0x145)](/(.*):(.*)/i)){const _0x4948b8=String(RegExp['$1']),_0x15378b=String(RegExp['$2'])['toUpperCase']()[_0xab436e(0x206)]();let _0x21e5c1,_0x36022a,_0x5b6e1;switch(_0x15378b){case _0xab436e(0x194):_0x21e5c1=_0x514050[_0x4c2812]!==''?Number(_0x514050[_0x4c2812]):0x0;break;case _0xab436e(0x17f):_0x36022a=_0x514050[_0x4c2812]!==''?JSON[_0xab436e(0x355)](_0x514050[_0x4c2812]):[],_0x21e5c1=_0x36022a[_0xab436e(0x1e3)](_0x2fb67e=>Number(_0x2fb67e));break;case _0xab436e(0x4b6):_0x21e5c1=_0x514050[_0x4c2812]!==''?eval(_0x514050[_0x4c2812]):null;break;case _0xab436e(0xbc):_0x36022a=_0x514050[_0x4c2812]!==''?JSON[_0xab436e(0x355)](_0x514050[_0x4c2812]):[],_0x21e5c1=_0x36022a[_0xab436e(0x1e3)](_0x2b3b0f=>eval(_0x2b3b0f));break;case'JSON':_0x21e5c1=_0x514050[_0x4c2812]!==''?JSON['parse'](_0x514050[_0x4c2812]):'';break;case _0xab436e(0x367):_0x36022a=_0x514050[_0x4c2812]!==''?JSON[_0xab436e(0x355)](_0x514050[_0x4c2812]):[],_0x21e5c1=_0x36022a[_0xab436e(0x1e3)](_0x24db58=>JSON[_0xab436e(0x355)](_0x24db58));break;case _0xab436e(0x465):_0x21e5c1=_0x514050[_0x4c2812]!==''?new Function(JSON['parse'](_0x514050[_0x4c2812])):new Function(_0xab436e(0x34d));break;case'ARRAYFUNC':_0x36022a=_0x514050[_0x4c2812]!==''?JSON[_0xab436e(0x355)](_0x514050[_0x4c2812]):[],_0x21e5c1=_0x36022a[_0xab436e(0x1e3)](_0x3e4b15=>new Function(JSON['parse'](_0x3e4b15)));break;case'STR':_0x21e5c1=_0x514050[_0x4c2812]!==''?String(_0x514050[_0x4c2812]):'';break;case _0xab436e(0x320):_0x36022a=_0x514050[_0x4c2812]!==''?JSON[_0xab436e(0x355)](_0x514050[_0x4c2812]):[],_0x21e5c1=_0x36022a[_0xab436e(0x1e3)](_0x2c3e11=>String(_0x2c3e11));break;case _0xab436e(0x3a6):_0x5b6e1=_0x514050[_0x4c2812]!==''?JSON[_0xab436e(0x355)](_0x514050[_0x4c2812]):{},_0x5499ac[_0x4948b8]={},VisuMZ['ConvertParams'](_0x5499ac[_0x4948b8],_0x5b6e1);continue;case _0xab436e(0x188):_0x36022a=_0x514050[_0x4c2812]!==''?JSON[_0xab436e(0x355)](_0x514050[_0x4c2812]):[],_0x21e5c1=_0x36022a[_0xab436e(0x1e3)](_0x5916f1=>VisuMZ[_0xab436e(0x1c2)]({},JSON[_0xab436e(0x355)](_0x5916f1)));break;default:continue;}_0x5499ac[_0x4948b8]=_0x21e5c1;}}return _0x5499ac;},(_0x276a26=>{const _0x361743=_0x39ebd3,_0x5d7b0e=_0x276a26[_0x361743(0x292)];for(const _0x1e4118 of dependencies){if(!Imported[_0x1e4118]){alert(_0x361743(0x200)[_0x361743(0x1da)](_0x5d7b0e,_0x1e4118)),SceneManager[_0x361743(0x273)]();break;}}const _0x36937f=_0x276a26[_0x361743(0x115)];if(_0x36937f[_0x361743(0x145)](/\[Version[ ](.*?)\]/i)){const _0x41aa5b=Number(RegExp['$1']);_0x41aa5b!==VisuMZ[label][_0x361743(0x4c1)]&&(alert(_0x361743(0x4ba)[_0x361743(0x1da)](_0x5d7b0e,_0x41aa5b)),SceneManager['exit']());}if(_0x36937f['match'](/\[Tier[ ](\d+)\]/i)){const _0x5053c2=Number(RegExp['$1']);_0x5053c2<tier?(alert('%1\x20is\x20incorrectly\x20placed\x20on\x20the\x20plugin\x20list.\x0aIt\x20is\x20a\x20Tier\x20%2\x20plugin\x20placed\x20over\x20other\x20Tier\x20%3\x20plugins.\x0aPlease\x20reorder\x20the\x20plugin\x20list\x20from\x20smallest\x20to\x20largest\x20tier\x20numbers.'[_0x361743(0x1da)](_0x5d7b0e,_0x5053c2,tier)),SceneManager[_0x361743(0x273)]()):tier=Math[_0x361743(0x165)](_0x5053c2,tier);}VisuMZ['ConvertParams'](VisuMZ[label][_0x361743(0x237)],_0x276a26[_0x361743(0x388)]);})(pluginData),PluginManager['registerCommand'](pluginData['name'],'ActorChangeEquipSlots',_0x2e1b05=>{const _0x472550=_0x39ebd3;VisuMZ['ConvertParams'](_0x2e1b05,_0x2e1b05);const _0x4d2cbf=_0x2e1b05[_0x472550(0x262)][_0x472550(0x1e3)](_0x59907a=>$gameActors[_0x472550(0x1f8)](_0x59907a)),_0x22c019=_0x2e1b05[_0x472550(0x166)][_0x472550(0x1e3)](_0x17f58b=>$dataSystem[_0x472550(0x1ce)][_0x472550(0x180)](_0x17f58b['trim']()));for(const _0x2846cc of _0x4d2cbf){if(!_0x2846cc)continue;_0x2846cc[_0x472550(0x36c)](_0x22c019);}}),PluginManager[_0x39ebd3(0x399)](pluginData['name'],'ActorResetEquipSlots',_0x9ccfc2=>{const _0x5b9c08=_0x39ebd3;VisuMZ[_0x5b9c08(0x1c2)](_0x9ccfc2,_0x9ccfc2);const _0x4d65ff=_0x9ccfc2[_0x5b9c08(0x262)][_0x5b9c08(0x1e3)](_0xa1fdf6=>$gameActors['actor'](_0xa1fdf6));for(const _0x353f3d of _0x4d65ff){if(!_0x353f3d)continue;_0x353f3d[_0x5b9c08(0x185)]();}}),PluginManager['registerCommand'](pluginData[_0x39ebd3(0x292)],_0x39ebd3(0x1bf),_0x49f746=>{const _0x3a96f8=_0x39ebd3;if($gameParty[_0x3a96f8(0x3c3)]())return;VisuMZ['ConvertParams'](_0x49f746,_0x49f746);const _0x3599c7=_0x49f746[_0x3a96f8(0x262)]['map'](_0x3ccb6c=>$gameActors[_0x3a96f8(0x1f8)](_0x3ccb6c));for(const _0x2dfbcd of _0x3599c7){if(!_0x2dfbcd)continue;_0x2dfbcd[_0x3a96f8(0x4e4)]();}}),PluginManager[_0x39ebd3(0x399)](pluginData[_0x39ebd3(0x292)],_0x39ebd3(0x2cd),_0x25399b=>{const _0x31128b=_0x39ebd3;if($gameParty[_0x31128b(0x3c3)]())return;$gameParty['purifyCursedEquips']();}),PluginManager['registerCommand'](pluginData[_0x39ebd3(0x292)],_0x39ebd3(0x4bd),_0x17f3f4=>{const _0x440786=_0x39ebd3;VisuMZ['ConvertParams'](_0x17f3f4,_0x17f3f4);const _0x71c8e5=[],_0x45c388=_0x17f3f4[_0x440786(0x95)][_0x440786(0x1e3)](_0x379aeb=>_0x379aeb[_0x440786(0x34e)]()[_0x440786(0x206)]()),_0x79bd2b=_0x17f3f4['Whitelist'][_0x440786(0x1e3)](_0x2a6a18=>_0x2a6a18[_0x440786(0x34e)]()['trim']()),_0x12fa72=_0x17f3f4[_0x440786(0xd1)]>=_0x17f3f4[_0x440786(0xaa)]?_0x17f3f4['Step1Start']:_0x17f3f4['Step1End'],_0x38b368=_0x17f3f4[_0x440786(0xd1)]>=_0x17f3f4[_0x440786(0xaa)]?_0x17f3f4['Step1End']:_0x17f3f4[_0x440786(0xaa)],_0x2957f0=Array(_0x38b368-_0x12fa72+0x1)[_0x440786(0x9c)]()['map']((_0x35ed78,_0x372317)=>_0x12fa72+_0x372317);for(const _0x5c6c2c of _0x2957f0){const _0x2d3406=$dataItems[_0x5c6c2c];if(!_0x2d3406)continue;if(!VisuMZ[_0x440786(0x2a9)][_0x440786(0x1a1)](_0x2d3406,_0x45c388,_0x79bd2b))continue;_0x71c8e5[_0x440786(0x17a)]([0x0,_0x5c6c2c,0x0,_0x2d3406['price']]);}const _0x39158d=_0x17f3f4[_0x440786(0x29f)]>=_0x17f3f4[_0x440786(0xbd)]?_0x17f3f4[_0x440786(0xbd)]:_0x17f3f4[_0x440786(0x29f)],_0x393f6e=_0x17f3f4[_0x440786(0x29f)]>=_0x17f3f4[_0x440786(0xbd)]?_0x17f3f4[_0x440786(0x29f)]:_0x17f3f4[_0x440786(0xbd)],_0x1d2f46=Array(_0x393f6e-_0x39158d+0x1)[_0x440786(0x9c)]()[_0x440786(0x1e3)]((_0x119ae9,_0x38faf7)=>_0x39158d+_0x38faf7);for(const _0x238191 of _0x1d2f46){const _0x1af873=$dataWeapons[_0x238191];if(!_0x1af873)continue;if(!VisuMZ[_0x440786(0x2a9)][_0x440786(0x1a1)](_0x1af873,_0x45c388,_0x79bd2b))continue;_0x71c8e5[_0x440786(0x17a)]([0x1,_0x238191,0x0,_0x1af873[_0x440786(0x458)]]);}const _0x2a43c3=_0x17f3f4[_0x440786(0x3fe)]>=_0x17f3f4[_0x440786(0x1fb)]?_0x17f3f4['Step3Start']:_0x17f3f4['Step3End'],_0x180bd5=_0x17f3f4['Step3End']>=_0x17f3f4[_0x440786(0x1fb)]?_0x17f3f4[_0x440786(0x3fe)]:_0x17f3f4[_0x440786(0x1fb)],_0x5aab59=Array(_0x180bd5-_0x2a43c3+0x1)['fill']()[_0x440786(0x1e3)]((_0xa1e9ff,_0x43037b)=>_0x2a43c3+_0x43037b);for(const _0x25fa22 of _0x5aab59){const _0x401b6e=$dataArmors[_0x25fa22];if(!_0x401b6e)continue;if(!VisuMZ[_0x440786(0x2a9)][_0x440786(0x1a1)](_0x401b6e,_0x45c388,_0x79bd2b))continue;_0x71c8e5[_0x440786(0x17a)]([0x2,_0x25fa22,0x0,_0x401b6e[_0x440786(0x458)]]);}SceneManager['push'](Scene_Shop),SceneManager[_0x440786(0x250)](_0x71c8e5,_0x17f3f4[_0x440786(0x25f)]);}),VisuMZ[_0x39ebd3(0x2a9)]['IncludeShopItem']=function(_0x1bc3f6,_0x53de2e,_0x8edc22){const _0x4f0ddb=_0x39ebd3;if(_0x1bc3f6[_0x4f0ddb(0x292)][_0x4f0ddb(0x206)]()==='')return![];if(_0x1bc3f6[_0x4f0ddb(0x292)][_0x4f0ddb(0x145)](/-----/i))return![];const _0xef627d=_0x1bc3f6[_0x4f0ddb(0x335)];if(_0x53de2e[_0x4f0ddb(0x142)]>0x0)for(const _0x5bc88c of _0x53de2e){if(!_0x5bc88c)continue;if(_0xef627d[_0x4f0ddb(0x35f)](_0x5bc88c))return![];}if(_0x8edc22[_0x4f0ddb(0x142)]>0x0){for(const _0x38f51b of _0x8edc22){if(!_0x38f51b)continue;if(_0xef627d[_0x4f0ddb(0x35f)](_0x38f51b))return!![];}return![];}return!![];},VisuMZ['ItemsEquipsCore'][_0x39ebd3(0x2b0)]=Scene_Boot[_0x39ebd3(0x3bc)][_0x39ebd3(0x299)],Scene_Boot[_0x39ebd3(0x3bc)][_0x39ebd3(0x299)]=function(){const _0x1c2f58=_0x39ebd3;this['process_VisuMZ_ItemsEquipsCore_RegExp'](),VisuMZ[_0x1c2f58(0x2a9)][_0x1c2f58(0x2b0)][_0x1c2f58(0x15c)](this),this[_0x1c2f58(0x1de)](),VisuMZ['ItemsEquipsCore']['SetupProxyItemGroups'](),VisuMZ[_0x1c2f58(0x2a9)][_0x1c2f58(0x3f0)]();},Scene_Boot['prototype'][_0x39ebd3(0x12d)]=function(){const _0xb62433=_0x39ebd3;VisuMZ[_0xb62433(0x2a9)][_0xb62433(0x35c)]={},VisuMZ['ItemsEquipsCore']['RegExp']['EquipParams']=[],VisuMZ[_0xb62433(0x2a9)][_0xb62433(0x35c)][_0xb62433(0x263)]=[];const _0x536cc6=[_0xb62433(0x374),_0xb62433(0x33d),'ATK',_0xb62433(0xb6),_0xb62433(0x3ee),'MDF',_0xb62433(0x294),'LUK'];for(const _0x5970aa of _0x536cc6){const _0x338e42='<%1:[\x20]([\x5c+\x5c-]\x5cd+)>'[_0xb62433(0x1da)](_0x5970aa);VisuMZ[_0xb62433(0x2a9)][_0xb62433(0x35c)][_0xb62433(0x15e)]['push'](new RegExp(_0x338e42,'i'));const _0x29cba0=_0xb62433(0x2f2)[_0xb62433(0x1da)](_0x5970aa);VisuMZ[_0xb62433(0x2a9)][_0xb62433(0x35c)][_0xb62433(0x263)][_0xb62433(0x17a)](new RegExp(_0x29cba0,'g'));}},Scene_Boot[_0x39ebd3(0x3bc)][_0x39ebd3(0x1de)]=function(){const _0x5c0d24=_0x39ebd3;if(VisuMZ[_0x5c0d24(0x2da)])return;this['process_VisuMZ_ItemsEquipsCore_EquipSlots']();const _0xabbdd9=[$dataItems,$dataWeapons,$dataArmors];for(const _0x408486 of _0xabbdd9){for(const _0x59b306 of _0x408486){if(!_0x59b306)continue;VisuMZ[_0x5c0d24(0x2a9)][_0x5c0d24(0x3c8)](_0x59b306,_0x408486),VisuMZ['ItemsEquipsCore'][_0x5c0d24(0x21d)](_0x59b306,_0x408486),VisuMZ[_0x5c0d24(0x2a9)][_0x5c0d24(0x2fa)](_0x59b306,_0x408486),VisuMZ[_0x5c0d24(0x2a9)]['Parse_Notetags_ParamJS'](_0x59b306,_0x408486),VisuMZ[_0x5c0d24(0x2a9)][_0x5c0d24(0x24d)](_0x59b306,_0x408486);}}},Scene_Boot[_0x39ebd3(0x3bc)]['process_VisuMZ_ItemsEquipsCore_EquipSlots']=function(){const _0x103769=_0x39ebd3;for(const _0x55dcb0 of $dataClasses){if(!_0x55dcb0)continue;VisuMZ['ItemsEquipsCore'][_0x103769(0x46d)](_0x55dcb0);}},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x4cf)]=VisuMZ[_0x39ebd3(0x4cf)],VisuMZ['ParseClassNotetags']=function(_0x57fa62){const _0xb4f9bf=_0x39ebd3;VisuMZ[_0xb4f9bf(0x2a9)][_0xb4f9bf(0x4cf)][_0xb4f9bf(0x15c)](this,_0x57fa62),VisuMZ['ItemsEquipsCore'][_0xb4f9bf(0x46d)](_0x57fa62);},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x1ed)]=VisuMZ[_0x39ebd3(0x1ed)],VisuMZ['ParseItemNotetags']=function(_0x1c8bcb){const _0x2324c2=_0x39ebd3;VisuMZ[_0x2324c2(0x2a9)][_0x2324c2(0x1ed)]['call'](this,_0x1c8bcb),VisuMZ['ItemsEquipsCore'][_0x2324c2(0x2bb)](_0x1c8bcb,$dataItems);},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0xc8)]=VisuMZ['ParseWeaponNotetags'],VisuMZ[_0x39ebd3(0xc8)]=function(_0x1877f1){const _0xeb7122=_0x39ebd3;VisuMZ[_0xeb7122(0x2a9)][_0xeb7122(0xc8)]['call'](this,_0x1877f1),VisuMZ[_0xeb7122(0x2a9)][_0xeb7122(0x2bb)](_0x1877f1,$dataWeapons);},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x3e2)]=VisuMZ[_0x39ebd3(0x3e2)],VisuMZ[_0x39ebd3(0x3e2)]=function(_0x3cf658){const _0x12ac30=_0x39ebd3;VisuMZ[_0x12ac30(0x2a9)]['ParseArmorNotetags'][_0x12ac30(0x15c)](this,_0x3cf658),VisuMZ[_0x12ac30(0x2a9)][_0x12ac30(0x2bb)](_0x3cf658,$dataArmors);},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x46d)]=function(_0x5c6c1c){const _0x3572cc=_0x39ebd3;_0x5c6c1c['equipSlots']=[];const _0x26359d=$dataSystem[_0x3572cc(0x1ce)][_0x3572cc(0x1e3)](_0x430d9b=>_0x430d9b?_0x430d9b[_0x3572cc(0x206)]():'');if(!BattleManager[_0x3572cc(0x4c4)]()&&_0x5c6c1c[_0x3572cc(0x3ba)][_0x3572cc(0x145)](/<EQUIP SLOTS>\s*([\s\S]*)\s*<\/EQUIP SLOTS>/i)){const _0x2edb5c=String(RegExp['$1'])[_0x3572cc(0x3de)](/[\r\n]+/);for(const _0x25806c of _0x2edb5c){const _0x15ab5d=_0x26359d[_0x3572cc(0x180)](_0x25806c[_0x3572cc(0x206)]());if(_0x15ab5d>0x0)_0x5c6c1c[_0x3572cc(0x18e)][_0x3572cc(0x17a)](_0x15ab5d);}}else for(const _0x3bc10 of _0x26359d){const _0x4fd224=_0x26359d[_0x3572cc(0x180)](_0x3bc10[_0x3572cc(0x206)]());if(_0x4fd224>0x0)_0x5c6c1c[_0x3572cc(0x18e)]['push'](_0x4fd224);}},VisuMZ['ItemsEquipsCore'][_0x39ebd3(0x2bb)]=function(_0x54fcc0,_0x24d356){const _0x28e8df=_0x39ebd3;VisuMZ[_0x28e8df(0x2a9)][_0x28e8df(0x3c8)](_0x54fcc0,_0x24d356),VisuMZ['ItemsEquipsCore']['Parse_Notetags_Prices'](_0x54fcc0,_0x24d356),VisuMZ['ItemsEquipsCore'][_0x28e8df(0x2fa)](_0x54fcc0,_0x24d356),VisuMZ[_0x28e8df(0x2a9)][_0x28e8df(0x18c)](_0x54fcc0,_0x24d356),VisuMZ[_0x28e8df(0x2a9)]['Parse_Notetags_EnableJS'](_0x54fcc0,_0x24d356);},VisuMZ['ItemsEquipsCore'][_0x39ebd3(0x3c8)]=function(_0x178772,_0x4639fe){const _0x6d1729=_0x39ebd3;_0x178772['categories']=[];const _0x38bf32=_0x178772[_0x6d1729(0x3ba)]||'',_0xe44f1a=_0x38bf32[_0x6d1729(0x145)](/<(?:CATEGORY|CATEGORIES):[ ](.*)>/gi);if(_0xe44f1a)for(const _0x53f0e4 of _0xe44f1a){_0x53f0e4['match'](/<(?:CATEGORY|CATEGORIES):[ ](.*)>/gi);const _0x1ab349=String(RegExp['$1'])[_0x6d1729(0x34e)]()[_0x6d1729(0x206)]()[_0x6d1729(0x3de)](',');for(const _0x51a72f of _0x1ab349){_0x178772[_0x6d1729(0x335)]['push'](_0x51a72f[_0x6d1729(0x206)]());}}if(_0x38bf32[_0x6d1729(0x145)](/<(?:CATEGORY|CATEGORIES)>\s*([\s\S]*)\s*<\/(?:CATEGORY|CATEGORIES)>/i)){const _0x347107=RegExp['$1'][_0x6d1729(0x3de)](/[\r\n]+/);for(const _0x1c307d of _0x347107){_0x178772['categories']['push'](_0x1c307d[_0x6d1729(0x34e)]()[_0x6d1729(0x206)]());}}},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0xf9)]=function(_0xa0f05f,_0xb26664){const _0x41c126=_0x39ebd3;if(!_0xa0f05f)return;_0xa0f05f[_0x41c126(0x330)]=0x32;const _0x5712fb=_0xa0f05f[_0x41c126(0x3ba)]||'';_0x5712fb['match'](/<(?:|ID )SORT(?:|ING)[ ]PRIORITY:[ ](\d+)>/i)&&(_0xa0f05f[_0x41c126(0x330)]=Number(RegExp['$1']));},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x21d)]=function(_0x2e3d97,_0x1bd496){const _0xdf52ce=_0x39ebd3;_0x2e3d97[_0xdf52ce(0x3ba)][_0xdf52ce(0x145)](/<PRICE:[ ](\d+)>/i)&&(_0x2e3d97[_0xdf52ce(0x458)]=Number(RegExp['$1']));},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x2fa)]=function(_0x26e943,_0x2bce8c){const _0x5c4a7b=_0x39ebd3;if(_0x2bce8c===$dataItems)return;for(let _0xd81730=0x0;_0xd81730<0x8;_0xd81730++){const _0x41513e=VisuMZ[_0x5c4a7b(0x2a9)]['RegExp'][_0x5c4a7b(0x15e)][_0xd81730];_0x26e943[_0x5c4a7b(0x3ba)][_0x5c4a7b(0x145)](_0x41513e)&&(_0x26e943[_0x5c4a7b(0x413)][_0xd81730]=parseInt(RegExp['$1']));}},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x2d3)]={},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x18c)]=function(_0x157022,_0x17a46a){const _0x473204=_0x39ebd3;if(_0x17a46a===$dataItems)return;if(_0x157022['note']['match'](/<JS PARAMETERS>\s*([\s\S]*)\s*<\/JS PARAMETERS>/i)){const _0x3c05f6=String(RegExp['$1']),_0x3ca43d=(_0x17a46a===$dataWeapons?_0x473204(0xb9):_0x473204(0x3b9))[_0x473204(0x1da)](_0x157022['id']),_0x167f4b='\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20let\x20MaxHP\x20=\x200;\x20let\x20MaxMP\x20=\x200;\x20let\x20ATK\x20=\x200;\x20let\x20DEF\x20=\x200;\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20let\x20MAT\x20=\x200;\x20let\x20MDF\x20=\x200;\x20let\x20AGI\x20=\x200;\x20let\x20LUK\x20=\x200;\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20const\x20user\x20=\x20this;\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20const\x20target\x20=\x20this;\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20const\x20a\x20=\x20this;\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20const\x20b\x20=\x20this;\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20try\x20{\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20%1\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20}\x20catch\x20(e)\x20{\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20if\x20($gameTemp.isPlaytest())\x20console.log(e);\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20}\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20return\x20[MaxHP,\x20MaxMP,\x20ATK,\x20DEF,\x20MAT,\x20MDF,\x20AGI,\x20LUK][paramId];\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20'[_0x473204(0x1da)](_0x3c05f6);for(let _0x4a4e3a=0x0;_0x4a4e3a<0x8;_0x4a4e3a++){if(_0x3c05f6['match'](VisuMZ[_0x473204(0x2a9)][_0x473204(0x35c)]['BorderRegExp'][_0x4a4e3a])){const _0x3a93b7=_0x473204(0x38e)[_0x473204(0x1da)](_0x3ca43d,_0x4a4e3a);VisuMZ['ItemsEquipsCore'][_0x473204(0x2d3)][_0x3a93b7]=new Function(_0x473204(0x27c),'paramId',_0x167f4b);}}}},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x4be)]={},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x24d)]=function(_0x408879,_0x2b010a){const _0x11a1db=_0x39ebd3;if(_0x2b010a!==$dataItems)return;if(_0x408879['note']['match'](/<JS ITEM ENABLE>\s*([\s\S]*)\s*<\/JS ITEM ENABLE>/i)){const _0x3242e9=String(RegExp['$1']),_0x1b8da5='\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20let\x20enabled\x20=\x20true;\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20const\x20user\x20=\x20this;\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20const\x20target\x20=\x20this;\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20const\x20a\x20=\x20this;\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20const\x20b\x20=\x20this;\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20try\x20{\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20%1\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20}\x20catch\x20(e)\x20{\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20if\x20($gameTemp.isPlaytest())\x20console.log(e);\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20}\x0a\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20return\x20enabled;\x0a\x20\x20\x20\x20\x20\x20\x20\x20'[_0x11a1db(0x1da)](_0x3242e9);VisuMZ['ItemsEquipsCore'][_0x11a1db(0x4be)][_0x408879['id']]=new Function(_0x11a1db(0x27c),_0x1b8da5);}},DataManager[_0x39ebd3(0x208)]=function(_0x1e8fd3){const _0x630be8=_0x39ebd3;return this[_0x630be8(0xc2)](_0x1e8fd3)&&_0x1e8fd3[_0x630be8(0x2d9)]===0x2;},DataManager[_0x39ebd3(0x109)]=function(_0x4abe2e){const _0x37ef9d=_0x39ebd3;if(!_0x4abe2e)return 0x63;else return _0x4abe2e[_0x37ef9d(0x3ba)][_0x37ef9d(0x145)](/<MAX:[ ](\d+)>/i)?parseInt(RegExp['$1']):this[_0x37ef9d(0x453)](_0x4abe2e);},DataManager[_0x39ebd3(0x453)]=function(_0xc59b01){const _0x29d649=_0x39ebd3;if(this[_0x29d649(0xc2)](_0xc59b01))return VisuMZ['ItemsEquipsCore'][_0x29d649(0x237)][_0x29d649(0x3b4)][_0x29d649(0x368)];else{if(this[_0x29d649(0x2db)](_0xc59b01))return VisuMZ[_0x29d649(0x2a9)]['Settings'][_0x29d649(0x3b4)][_0x29d649(0x47e)];else{if(this[_0x29d649(0x13b)](_0xc59b01))return VisuMZ[_0x29d649(0x2a9)][_0x29d649(0x237)][_0x29d649(0x3b4)][_0x29d649(0x249)];}}},DataManager[_0x39ebd3(0x41e)]=function(_0x197166){const _0xb5a759=_0x39ebd3;_0x197166=_0x197166['toUpperCase']()[_0xb5a759(0x206)](),this[_0xb5a759(0x45c)]=this[_0xb5a759(0x45c)]||{};if(this[_0xb5a759(0x45c)][_0x197166])return this[_0xb5a759(0x45c)][_0x197166];for(const _0x2606c8 of $dataClasses){if(!_0x2606c8)continue;let _0x5d4678=_0x2606c8['name'];_0x5d4678=_0x5d4678[_0xb5a759(0x295)](/\x1I\[(\d+)\]/gi,''),_0x5d4678=_0x5d4678[_0xb5a759(0x295)](/\\I\[(\d+)\]/gi,''),this[_0xb5a759(0x45c)][_0x5d4678[_0xb5a759(0x34e)]()['trim']()]=_0x2606c8['id'];}return this[_0xb5a759(0x45c)][_0x197166]||0x0;},DataManager['getSkillIdWithName']=function(_0x1f4aa1){const _0x45976a=_0x39ebd3;_0x1f4aa1=_0x1f4aa1['toUpperCase']()['trim'](),this['_skillIDs']=this['_skillIDs']||{};if(this[_0x45976a(0xba)][_0x1f4aa1])return this[_0x45976a(0xba)][_0x1f4aa1];for(const _0x12e8ff of $dataSkills){if(!_0x12e8ff)continue;this[_0x45976a(0xba)][_0x12e8ff['name'][_0x45976a(0x34e)]()[_0x45976a(0x206)]()]=_0x12e8ff['id'];}return this[_0x45976a(0xba)][_0x1f4aa1]||0x0;},DataManager['getItemIdWithName']=function(_0x4d5a09){const _0x252711=_0x39ebd3;_0x4d5a09=_0x4d5a09[_0x252711(0x34e)]()[_0x252711(0x206)](),this[_0x252711(0x311)]=this[_0x252711(0x311)]||{};if(this[_0x252711(0x311)][_0x4d5a09])return this['_itemIDs'][_0x4d5a09];for(const _0x161ed4 of $dataItems){if(!_0x161ed4)continue;this[_0x252711(0x311)][_0x161ed4[_0x252711(0x292)][_0x252711(0x34e)]()[_0x252711(0x206)]()]=_0x161ed4['id'];}return this[_0x252711(0x311)][_0x4d5a09]||0x0;},DataManager['getWeaponIdWithName']=function(_0x5a463f){const _0x145dc5=_0x39ebd3;_0x5a463f=_0x5a463f[_0x145dc5(0x34e)]()['trim'](),this['_weaponIDs']=this['_weaponIDs']||{};if(this[_0x145dc5(0x278)][_0x5a463f])return this[_0x145dc5(0x278)][_0x5a463f];for(const _0x35f458 of $dataWeapons){if(!_0x35f458)continue;this[_0x145dc5(0x278)][_0x35f458[_0x145dc5(0x292)][_0x145dc5(0x34e)]()[_0x145dc5(0x206)]()]=_0x35f458['id'];}return this[_0x145dc5(0x278)][_0x5a463f]||0x0;},DataManager[_0x39ebd3(0x11a)]=function(_0x2275f8){const _0x59809b=_0x39ebd3;_0x2275f8=_0x2275f8[_0x59809b(0x34e)]()['trim'](),this[_0x59809b(0x2df)]=this[_0x59809b(0x2df)]||{};if(this[_0x59809b(0x2df)][_0x2275f8])return this[_0x59809b(0x2df)][_0x2275f8];for(const _0x45b4f5 of $dataArmors){if(!_0x45b4f5)continue;this[_0x59809b(0x2df)][_0x45b4f5['name'][_0x59809b(0x34e)]()[_0x59809b(0x206)]()]=_0x45b4f5['id'];}return this['_armorIDs'][_0x2275f8]||0x0;},DataManager['getEtypeIdWithName']=function(_0x9c81c0){const _0x534ba5=_0x39ebd3;_0x9c81c0=_0x9c81c0[_0x534ba5(0x34e)]()[_0x534ba5(0x206)](),this[_0x534ba5(0x426)]=this[_0x534ba5(0x426)]||{};if(this['_etypeIDs'][_0x9c81c0])return this['_etypeIDs'][_0x9c81c0];for(const _0x3e5c30 of $dataSystem[_0x534ba5(0x1ce)]){this[_0x534ba5(0x426)][_0x3e5c30[_0x534ba5(0x34e)]()[_0x534ba5(0x206)]()]=$dataSystem[_0x534ba5(0x1ce)][_0x534ba5(0x180)](_0x3e5c30);}return this[_0x534ba5(0x426)][_0x9c81c0]||0x0;},VisuMZ['ItemsEquipsCore']['SetupProxyItemGroups']=function(){const _0x54df55=_0x39ebd3;VisuMZ[_0x54df55(0x2a9)]['SetupProxyItemGroup']($dataItems),VisuMZ[_0x54df55(0x2a9)][_0x54df55(0x42a)]($dataWeapons),VisuMZ[_0x54df55(0x2a9)][_0x54df55(0x42a)]($dataArmors);},VisuMZ['ItemsEquipsCore']['SetupProxyItemGroup']=function(_0x394a91){const _0xa48b3e=_0x39ebd3;for(const _0x33d4df of _0x394a91){if(!_0x33d4df)continue;if(!DataManager['isProxyItem'](_0x33d4df))continue;const _0x1d6074=DataManager[_0xa48b3e(0x47b)](_0x33d4df),_0x5bdfd0=[_0xa48b3e(0x292),_0xa48b3e(0x229),_0xa48b3e(0x115)];for(const _0x4fb679 of _0x5bdfd0){_0x33d4df[_0x4fb679]=_0x1d6074[_0x4fb679];}}},DataManager[_0x39ebd3(0xfa)]=function(_0x554c7c){const _0x47be13=_0x39ebd3;if(!_0x554c7c)return![];if(!_0x554c7c['note'])return![];return _0x554c7c&&_0x554c7c[_0x47be13(0x3ba)][_0x47be13(0x145)](/<PROXY:[ ](.*)>/i);},DataManager[_0x39ebd3(0x47b)]=function(_0x1e84a9){const _0x1a89ef=_0x39ebd3;return this[_0x1a89ef(0xfa)](_0x1e84a9)?this[_0x1a89ef(0xde)](_0x1e84a9)||_0x1e84a9:_0x1e84a9;},DataManager[_0x39ebd3(0xde)]=function(_0x1a52ca){const _0x260df3=_0x39ebd3;_0x1a52ca['note'][_0x260df3(0x145)](/<PROXY:[ ](.*)>/i);const _0xf74eb3=RegExp['$1'][_0x260df3(0x206)](),_0x25e524=/^\d+$/[_0x260df3(0x11d)](_0xf74eb3);if(this[_0x260df3(0xc2)](_0x1a52ca)){const _0x35dcab=_0x25e524?Number(_0xf74eb3):DataManager['getItemIdWithName'](_0xf74eb3);return $dataItems[_0x35dcab]||_0x1a52ca;}else{if(this[_0x260df3(0x2db)](_0x1a52ca)){const _0x25db8f=_0x25e524?Number(_0xf74eb3):DataManager[_0x260df3(0x244)](_0xf74eb3);return $dataWeapons[_0x25db8f]||_0x1a52ca;}else{if(this['isArmor'](_0x1a52ca)){const _0x42a2ea=_0x25e524?Number(_0xf74eb3):DataManager[_0x260df3(0x11a)](_0xf74eb3);return $dataArmors[_0x42a2ea]||_0x1a52ca;}}}return _0x1a52ca;},VisuMZ['ItemsEquipsCore'][_0x39ebd3(0x230)]=Window_ItemList['prototype'][_0x39ebd3(0x27c)],Window_ItemList[_0x39ebd3(0x3bc)][_0x39ebd3(0x27c)]=function(){const _0x5a0b0b=_0x39ebd3;if($gameTemp['_bypassProxy'])return VisuMZ[_0x5a0b0b(0x2a9)][_0x5a0b0b(0x230)][_0x5a0b0b(0x15c)](this);return DataManager['getProxyItem'](VisuMZ['ItemsEquipsCore'][_0x5a0b0b(0x230)][_0x5a0b0b(0x15c)](this));},Window_ItemList[_0x39ebd3(0x3bc)][_0x39ebd3(0x4f3)]=function(){const _0xf5e870=_0x39ebd3;return VisuMZ[_0xf5e870(0x2a9)][_0xf5e870(0x230)][_0xf5e870(0x15c)](this);},VisuMZ[_0x39ebd3(0x2a9)]['Window_ShopBuy_item']=Window_ShopBuy['prototype'][_0x39ebd3(0x27c)],Window_ShopBuy['prototype']['item']=function(){const _0x5c4346=_0x39ebd3;if($gameTemp[_0x5c4346(0x22f)])return VisuMZ['ItemsEquipsCore']['Window_ShopBuy_item'][_0x5c4346(0x15c)](this);return DataManager[_0x5c4346(0x47b)](VisuMZ['ItemsEquipsCore'][_0x5c4346(0x34b)]['call'](this));},Window_ShopBuy['prototype'][_0x39ebd3(0x4f3)]=function(){const _0x722329=_0x39ebd3;return VisuMZ[_0x722329(0x2a9)][_0x722329(0x34b)][_0x722329(0x15c)](this);},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x2c5)]=Game_Item['prototype'][_0x39ebd3(0x14b)],Game_Item[_0x39ebd3(0x3bc)][_0x39ebd3(0x14b)]=function(_0x3ced08){const _0x1e99c4=_0x39ebd3;if(DataManager[_0x1e99c4(0xfa)](_0x3ced08))return;VisuMZ[_0x1e99c4(0x2a9)][_0x1e99c4(0x2c5)][_0x1e99c4(0x15c)](this,_0x3ced08);},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x3f0)]=function(){const _0x3189d3=_0x39ebd3;this[_0x3189d3(0x265)]={'partyArtifactIDs':[],'troopArtifactIDs':[]};for(const _0x54d044 of $dataArmors){if(!_0x54d044)continue;if(!DataManager['isArtifact'](_0x54d044))continue;DataManager[_0x3189d3(0x360)](_0x54d044)&&this[_0x3189d3(0x265)]['partyArtifactIDs'][_0x3189d3(0x17a)](_0x54d044['id']),DataManager['isTroopArtifact'](_0x54d044)&&this[_0x3189d3(0x265)][_0x3189d3(0x3bb)]['push'](_0x54d044['id']);}},DataManager[_0x39ebd3(0x346)]=function(_0x477f82){const _0x896762=_0x39ebd3;if(!this[_0x896762(0x13b)](_0x477f82))return![];const _0xc2d931=_0x477f82['note'];if(!_0xc2d931)return![];if(_0xc2d931[_0x896762(0x145)](/<(?:PARTY|ALLY) (?:ARTIFACT|RELIC)>/i))return!![];if(_0xc2d931['match'](/<(?:STACKABLE|STACK) (?:PARTY|ALLY) (?:ARTIFACT|RELIC)>/i))return!![];if(_0xc2d931[_0x896762(0x145)](/<(?:TROOP|FOE) (?:ARTIFACT|RELIC)>/i))return!![];if(_0xc2d931['match'](/<(?:STACKABLE|STACK) (?:TROOP|FOE) (?:ARTIFACT|RELIC)>/i))return!![];return![];},DataManager[_0x39ebd3(0x471)]=function(_0x42b479){const _0x4bedc1=_0x39ebd3;if(!this[_0x4bedc1(0x346)](_0x42b479))return![];const _0x4d1a32=_0x42b479['note'];if(!_0x4d1a32)return![];if(_0x4d1a32['match'](/<(?:STACKABLE|STACK) (?:PARTY|ALLY) (?:ARTIFACT|RELIC)>/i))return!![];if(_0x4d1a32[_0x4bedc1(0x145)](/<(?:STACKABLE|STACK) (?:TROOP|FOE) (?:ARTIFACT|RELIC)>/i))return!![];return![];},DataManager['isPartyArtifact']=function(_0x2a9b71){const _0x2931f2=_0x39ebd3;if(!this['isArtifact'](_0x2a9b71))return![];const _0x270ff2=_0x2a9b71['note'];if(!_0x270ff2)return![];if(_0x270ff2[_0x2931f2(0x145)](/<(?:PARTY|ALLY) (?:ARTIFACT|RELIC)>/i))return!![];if(_0x270ff2[_0x2931f2(0x145)](/<(?:STACKABLE|STACK) (?:PARTY|ALLY) (?:ARTIFACT|RELIC)>/i))return!![];return![];},DataManager[_0x39ebd3(0x21f)]=function(_0x292bc2){const _0x404cf3=_0x39ebd3;if(!this[_0x404cf3(0x346)](_0x292bc2))return![];const _0x428789=_0x292bc2[_0x404cf3(0x3ba)];if(!_0x428789)return![];if(_0x428789[_0x404cf3(0x145)](/<(?:TROOP|FOE) (?:ARTIFACT|RELIC)>/i))return!![];if(_0x428789[_0x404cf3(0x145)](/<(?:STACKABLE|STACK) (?:TROOP|FOE) (?:ARTIFACT|RELIC)>/i))return!![];return![];},VisuMZ[_0x39ebd3(0x2a9)]['Game_BattlerBase_canEquip_artifact']=Game_BattlerBase[_0x39ebd3(0x3bc)][_0x39ebd3(0x1d6)],Game_BattlerBase['prototype'][_0x39ebd3(0x1d6)]=function(_0x20c5ae){const _0x27bfe7=_0x39ebd3;if(DataManager['isArtifact'](_0x20c5ae))return![];if(!DataManager['meetsClassRequirements'](this,_0x20c5ae))return![];if(!DataManager[_0x27bfe7(0x397)](this,_0x20c5ae))return![];return VisuMZ[_0x27bfe7(0x2a9)]['Game_BattlerBase_canEquip_artifact'][_0x27bfe7(0x15c)](this,_0x20c5ae);},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x3c7)]=Game_BattlerBase[_0x39ebd3(0x3bc)]['param'],Game_BattlerBase[_0x39ebd3(0x3bc)]['param']=function(_0x39a1e0){const _0x1df18a=_0x39ebd3;this['_allowArtifactParamBase']=!![];const _0x3abd45=VisuMZ[_0x1df18a(0x2a9)][_0x1df18a(0x3c7)]['call'](this,_0x39a1e0);return this[_0x1df18a(0x4b9)]=undefined,_0x3abd45;},VisuMZ[_0x39ebd3(0x2a9)]['Game_Actor_artifact']=Game_Actor['prototype'][_0x39ebd3(0xbf)],Game_Actor['prototype']['traitObjects']=function(){const _0x567d72=_0x39ebd3;this[_0x567d72(0x377)]=!![];const _0x22f62b=VisuMZ[_0x567d72(0x2a9)][_0x567d72(0x26b)]['call'](this);return this['_allowArtifactTraitObjects']=undefined,_0x22f62b;},VisuMZ['ItemsEquipsCore']['Game_Actor_equips_artifacts']=Game_Actor[_0x39ebd3(0x3bc)][_0x39ebd3(0x257)],Game_Actor[_0x39ebd3(0x3bc)]['equips']=function(){const _0x125ab0=_0x39ebd3,_0x261edf=VisuMZ[_0x125ab0(0x2a9)]['Game_Actor_equips_artifacts'][_0x125ab0(0x15c)](this);if(this['_allowArtifactTraitObjects']||this[_0x125ab0(0x4b9)]){const _0x28633c=_0x261edf[_0x125ab0(0x1fe)]($gameParty['partyArtifacts']());return _0x28633c;}else return _0x261edf;},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x421)]=Game_BattlerBase[_0x39ebd3(0x3bc)][_0x39ebd3(0x36f)],Game_BattlerBase['prototype']['paramPlus']=function(_0x241e61){const _0x2354df=_0x39ebd3;let _0x24fef8=VisuMZ[_0x2354df(0x2a9)]['Game_BattlerBase_paramPlus_artifact']['call'](this,_0x241e61);if(this[_0x2354df(0x3f4)]===Game_Enemy)for(const _0x23a9ca of $gameParty['troopArtifacts']()){if(_0x23a9ca)_0x24fef8+=_0x23a9ca[_0x2354df(0x413)][_0x241e61];}return _0x24fef8;},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x442)]=Game_Enemy['prototype']['traitObjects'],Game_Enemy['prototype'][_0x39ebd3(0xbf)]=function(){const _0x1c081b=_0x39ebd3;let _0x2cc03c=VisuMZ[_0x1c081b(0x2a9)][_0x1c081b(0x442)]['call'](this);return _0x2cc03c[_0x1c081b(0x1fe)]($gameParty[_0x1c081b(0x498)]());},VisuMZ[_0x39ebd3(0x2a9)]['Game_Party_gainItem_artifact']=Game_Party[_0x39ebd3(0x3bc)][_0x39ebd3(0x33b)],Game_Party[_0x39ebd3(0x3bc)]['gainItem']=function(_0x4bca5,_0x171050,_0x175408){const _0x2d9e4a=_0x39ebd3;VisuMZ[_0x2d9e4a(0x2a9)][_0x2d9e4a(0x2ac)]['call'](this,_0x4bca5,_0x171050,_0x175408);if(DataManager[_0x2d9e4a(0x346)](_0x4bca5)){let _0x5f0f2a=$gameParty['allMembers']();if($gameParty[_0x2d9e4a(0x3c3)]())_0x5f0f2a=_0x5f0f2a['concat']($gameTroop['members']());for(const _0x54d7c6 of _0x5f0f2a){if(!_0x54d7c6)continue;_0x54d7c6[_0x2d9e4a(0x144)]={};}}},Game_Party[_0x39ebd3(0x3bc)][_0x39ebd3(0x1b3)]=function(){const _0x530930=_0x39ebd3;let _0x41abe8=[];const _0x54a460=VisuMZ[_0x530930(0x2a9)][_0x530930(0x265)][_0x530930(0x42c)];if(_0x54a460)for(const _0x56faf3 of _0x54a460){const _0x9e8e3=$dataArmors[_0x56faf3];if(!_0x9e8e3)continue;if(!this[_0x530930(0x27f)](_0x9e8e3))continue;let _0x52d0cf=0x1;if(DataManager[_0x530930(0x471)](_0x9e8e3))_0x52d0cf=this[_0x530930(0x410)](_0x9e8e3);while(_0x52d0cf--)_0x41abe8[_0x530930(0x17a)](_0x9e8e3);}return _0x41abe8;},Game_Party['prototype'][_0x39ebd3(0x498)]=function(){const _0x33a24c=_0x39ebd3;let _0x156fd9=[];const _0x32d316=VisuMZ[_0x33a24c(0x2a9)][_0x33a24c(0x265)][_0x33a24c(0x3bb)];if(_0x32d316)for(const _0x3728ee of _0x32d316){const _0xc31e48=$dataArmors[_0x3728ee];if(!_0xc31e48)continue;if(!this[_0x33a24c(0x27f)](_0xc31e48))continue;let _0x7100d6=0x1;if(DataManager[_0x33a24c(0x471)](_0xc31e48))_0x7100d6=this['numItems'](_0xc31e48);while(_0x7100d6--)_0x156fd9[_0x33a24c(0x17a)](_0xc31e48);}return _0x156fd9;},Game_Party['prototype']['artifacts']=function(){const _0x42c02c=_0x39ebd3;return this[_0x42c02c(0x1b3)]()[_0x42c02c(0x1fe)](this[_0x42c02c(0x498)]());},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x457)]=Game_Party['prototype']['setupBattleTestItems'],Game_Party[_0x39ebd3(0x3bc)]['setupBattleTestItems']=function(){const _0x5290d4=_0x39ebd3;VisuMZ[_0x5290d4(0x2a9)]['Game_Party_setupBattleTestItems_artifact']['call'](this),this[_0x5290d4(0x3a8)]();},Game_Party[_0x39ebd3(0x3bc)]['removeBattleTestArtifacts']=function(){const _0x526b1c=_0x39ebd3,_0x14885d=$gameParty['armors']()[_0x526b1c(0x285)](_0x37fc0a=>DataManager['isArtifact'](_0x37fc0a));for(const _0x263780 of _0x14885d){const _0x28d5d2=this[_0x526b1c(0x410)](_0x263780);if(_0x28d5d2)this[_0x526b1c(0xdb)](_0x263780,_0x28d5d2);}},DataManager[_0x39ebd3(0x1a3)]=function(_0x206760,_0x1585da){const _0x2fa6c1=_0x39ebd3;if(this[_0x2fa6c1(0xc2)](_0x1585da))return![];if(!_0x206760)return![];if($gameTemp[_0x2fa6c1(0x266)])return!![];if(BattleManager[_0x2fa6c1(0x4c4)]())return!![];const _0x5216bf=this[_0x2fa6c1(0x418)](_0x1585da);if(_0x5216bf[_0x2fa6c1(0x142)]<=0x0)return!![];return _0x5216bf['includes'](_0x206760[_0x2fa6c1(0xbb)]()['id']);},DataManager[_0x39ebd3(0x418)]=function(_0x247052){const _0x2a2bc6=_0x39ebd3;if(!_0x247052)return[];this[_0x2a2bc6(0xe4)]=this[_0x2a2bc6(0xe4)]||{};const _0x475221=_0x2a2bc6(0x38e)['format'](this[_0x2a2bc6(0x2db)](_0x247052)?'WEAPON':_0x2a2bc6(0x437),_0x247052['id']);if(this['_getClassRequirements'][_0x475221]!==undefined)return this[_0x2a2bc6(0xe4)][_0x475221];let _0x16c5c7=[];const _0x490f72=_0x247052[_0x2a2bc6(0x3ba)]||'';if(_0x490f72[_0x2a2bc6(0x145)](/<EQUIP FOR CLASS(?:|ES) ONLY:[ ](.*)>/i)){const _0x3dbc77=String(RegExp['$1'])['split'](',')[_0x2a2bc6(0x1e3)](_0x144769=>_0x144769[_0x2a2bc6(0x206)]());for(const _0x4e3ff8 of _0x3dbc77){const _0x1b6f9e=/^\d+$/[_0x2a2bc6(0x11d)](_0x4e3ff8);_0x1b6f9e?_0x16c5c7[_0x2a2bc6(0x17a)](Number(_0x4e3ff8)):_0x16c5c7[_0x2a2bc6(0x17a)](DataManager['getClassIdWithName'](_0x4e3ff8));}}return this['_getClassRequirements'][_0x475221]=_0x16c5c7,this[_0x2a2bc6(0xe4)][_0x475221];},DataManager[_0x39ebd3(0x397)]=function(_0x8f9dcf,_0x14283d){const _0x153d73=_0x39ebd3;if(this[_0x153d73(0xc2)](_0x14283d))return![];if(!_0x8f9dcf)return![];if($gameTemp[_0x153d73(0x266)])return!![];if(BattleManager[_0x153d73(0x4c4)]())return!![];const _0x37114e=this[_0x153d73(0xe8)](_0x14283d);for(const _0x149735 of _0x37114e){if(!this[_0x153d73(0x28a)](_0x8f9dcf,_0x149735))return![];}return!![];},DataManager[_0x39ebd3(0xe8)]=function(_0x59e30a){const _0xb9f6a2=_0x39ebd3;if(!_0x59e30a)return[];this[_0xb9f6a2(0x318)]=this[_0xb9f6a2(0x318)]||{};const _0x46798e=_0xb9f6a2(0x38e)[_0xb9f6a2(0x1da)](this[_0xb9f6a2(0x2db)](_0x59e30a)?_0xb9f6a2(0xc6):'ARMOR',_0x59e30a['id']);if(this['_getEquipRequirements'][_0x46798e]!==undefined)return this['_getEquipRequirements'][_0x46798e];let _0x6fe3a4=[];const _0x4500e3=_0x59e30a[_0xb9f6a2(0x3ba)]||'';return _0x4500e3[_0xb9f6a2(0x145)](/<EQUIP(?:|MENT)[ ]REQUIREMENT(?:|S)>\s*([\s\S]*)\s*<\/EQUIP(?:|MENT)[ ]REQUIREMENT(?:|S)>/i)&&(_0x6fe3a4=String(RegExp['$1'])['split'](/[\r\n]+/)),this['_getEquipRequirements'][_0x46798e]=_0x6fe3a4,this[_0xb9f6a2(0x318)][_0x46798e];},DataManager[_0x39ebd3(0x28a)]=function(_0x73f37b,_0x28913f){const _0x191ad5=_0x39ebd3;if(_0x28913f['match'](/(?:LEVEL|LV|LVL)[ ](>|>=|===|<=|<)[ ](\d+)/i)){const _0x70c20a=String(RegExp['$1'])[_0x191ad5(0x206)](),_0x10a601=Number(RegExp['$2']);switch(_0x70c20a){case'>':return _0x73f37b[_0x191ad5(0x1f9)]>_0x10a601;case'>=':return _0x73f37b[_0x191ad5(0x1f9)]>=_0x10a601;case'===':return _0x73f37b[_0x191ad5(0x1f9)]===_0x10a601;case'<=':return _0x73f37b[_0x191ad5(0x1f9)]<=_0x10a601;case'<':return _0x73f37b[_0x191ad5(0x1f9)]<_0x10a601;}return![];}if(_0x28913f[_0x191ad5(0x145)](/(MAXHP|MAXMP|MHP|MMP)[ ](>|>=|===|<=|<)[ ](\d+)/i)){const _0x1e3a91=String(RegExp['$1'])[_0x191ad5(0x290)]()[_0x191ad5(0x206)](),_0x2ba234=String(RegExp['$2'])['trim'](),_0x339959=Number(RegExp['$3']);let _0x3c3453=0x0;if([_0x191ad5(0x240),_0x191ad5(0x483)][_0x191ad5(0x35f)](_0x1e3a91))_0x3c3453=0x1;const _0x49131c=_0x73f37b[_0x191ad5(0x427)][_0x3c3453]||0x0;switch(_0x2ba234){case'>':return _0x73f37b['paramBase'](_0x3c3453)+_0x49131c>_0x339959;case'>=':return _0x73f37b['paramBase'](_0x3c3453)+_0x49131c>=_0x339959;case _0x191ad5(0x9e):return _0x73f37b[_0x191ad5(0x411)](_0x3c3453)+_0x49131c===_0x339959;case'<=':return _0x73f37b[_0x191ad5(0x411)](_0x3c3453)+_0x49131c<=_0x339959;case'<':return _0x73f37b[_0x191ad5(0x411)](_0x3c3453)+_0x49131c<_0x339959;}return![];}if(_0x28913f[_0x191ad5(0x145)](/(ATK|DEF|MAT|MDF|AGI|LUK)[ ](>|>=|===|<=|<)[ ](\d+)/i)){const _0x22dd6d=String(RegExp['$1'])[_0x191ad5(0x290)]()[_0x191ad5(0x206)](),_0x4acc6e=String(RegExp['$2'])[_0x191ad5(0x206)](),_0x2db4e7=Number(RegExp['$3']),_0x13896c=[_0x191ad5(0x362),'def',_0x191ad5(0x467),_0x191ad5(0x34c),_0x191ad5(0x1db),'luk'];let _0x69c0d2=_0x13896c['indexOf'](_0x22dd6d)+0x2;if(_0x69c0d2<0x2)return![];const _0x43fceb=_0x73f37b[_0x191ad5(0x427)][_0x69c0d2]||0x0;switch(_0x4acc6e){case'>':return _0x73f37b[_0x191ad5(0x411)](_0x69c0d2)+_0x43fceb>_0x2db4e7;case'>=':return _0x73f37b[_0x191ad5(0x411)](_0x69c0d2)+_0x43fceb>=_0x2db4e7;case _0x191ad5(0x9e):return _0x73f37b[_0x191ad5(0x411)](_0x69c0d2)+_0x43fceb===_0x2db4e7;case'<=':return _0x73f37b['paramBase'](_0x69c0d2)+_0x43fceb<=_0x2db4e7;case'<':return _0x73f37b[_0x191ad5(0x411)](_0x69c0d2)+_0x43fceb<_0x2db4e7;}return![];}if(_0x28913f['match'](/LEARNED SKILL:[ ](\d+)/i)){const _0x7fd0c4=Number(RegExp['$1']);return _0x73f37b[_0x191ad5(0x1a7)](_0x7fd0c4);}else{if(_0x28913f[_0x191ad5(0x145)](/LEARNED SKILL:[ ](.*)/i)){const _0x58fc47=String(RegExp['$1']),_0x28ecae=this['getSkillIdWithName'](_0x58fc47);return _0x73f37b[_0x191ad5(0x1a7)](_0x28ecae);}}if(_0x28913f['match'](/SWITCH:[ ](\d+)/i)){const _0x23f352=Number(RegExp['$1']);return $gameSwitches['value'](_0x23f352);}return!![];},DataManager[_0x39ebd3(0x4a5)]=function(_0x16c42f){const _0x5cd412=_0x39ebd3;return this[_0x5cd412(0x13b)](_0x16c42f)?this[_0x5cd412(0x193)](_0x16c42f):[_0x16c42f[_0x5cd412(0x34a)]||0x0];},DataManager[_0x39ebd3(0x193)]=function(_0x53cd23){const _0x31161d=_0x39ebd3;this[_0x31161d(0x2b8)]=this[_0x31161d(0x2b8)]||{};if(this[_0x31161d(0x2b8)][_0x53cd23['id']]!==undefined)return this[_0x31161d(0x2b8)][_0x53cd23['id']];this[_0x31161d(0x2b8)][_0x53cd23['id']]=[_0x53cd23[_0x31161d(0x34a)]||0x0];const _0x2e7d71=_0x53cd23[_0x31161d(0x3ba)]||'';if(_0x2e7d71[_0x31161d(0x145)](/<ADDED ETYPE(?:|S):[ ](.*)>/i)){const _0x47b8c1=String(RegExp['$1'])[_0x31161d(0x3de)](',')[_0x31161d(0x1e3)](_0x579106=>_0x579106[_0x31161d(0x206)]());for(const _0xc2f20 of _0x47b8c1){const _0x18b5d5=/^\d+$/[_0x31161d(0x11d)](_0xc2f20);let _0x2c9642=0x0;_0x18b5d5?_0x2c9642=Number(_0xc2f20):_0x2c9642=this[_0x31161d(0x10b)](_0xc2f20),_0x2c9642>0x1&&this[_0x31161d(0x2b8)][_0x53cd23['id']]['push'](_0x2c9642);}}return this[_0x31161d(0x2b8)][_0x53cd23['id']];},Game_BattlerBase['prototype'][_0x39ebd3(0x2bc)]=function(_0x473672){const _0x45741d=_0x39ebd3;return this['isEquipAtypeOk'](_0x473672[_0x45741d(0x31e)])&&!this[_0x45741d(0x3a1)](_0x473672[_0x45741d(0x34a)])&&DataManager[_0x45741d(0x4a5)](_0x473672)[_0x45741d(0x409)](_0x2336cf=>!this['isEquipTypeSealed'](_0x2336cf));},DataManager['isCursedItem']=function(_0x4e3c80){const _0x4b4a1c=_0x39ebd3;if(!this[_0x4b4a1c(0x2db)](_0x4e3c80)&&!this['isArmor'](_0x4e3c80))return![];if(Imported[_0x4b4a1c(0xcd)]&&this['isWeapon'](_0x4e3c80))return![];if(!_0x4e3c80[_0x4b4a1c(0x3ba)])return![];return _0x4e3c80[_0x4b4a1c(0x3ba)]['match'](/<CURSED>/i);},DataManager['getPurifyTransformation']=function(_0x592745){const _0x2b8946=_0x39ebd3;if(!_0x592745)return _0x592745;if(!this['isWeapon'](_0x592745)&&!this[_0x2b8946(0x13b)](_0x592745))return _0x592745;if(_0x592745['note']['match'](/<PURIFY TRANSFORM:[ ](.*)>/i)){const _0x21a9a8=String(RegExp['$1'])[_0x2b8946(0x206)](),_0x379963=/^\d+$/[_0x2b8946(0x11d)](_0x21a9a8);if(_0x379963){if(this[_0x2b8946(0x2db)](_0x592745))return $dataWeapons[Number(_0x21a9a8)];if(this['isArmor'](_0x592745))return $dataArmors[Number(_0x21a9a8)];}else{if(this['isWeapon'](_0x592745))return $dataWeapons[this[_0x2b8946(0x244)](_0x21a9a8)];if(this[_0x2b8946(0x13b)](_0x592745))return $dataArmors[this[_0x2b8946(0x11a)](_0x21a9a8)];}}return _0x592745;},Game_Party['prototype'][_0x39ebd3(0x4e4)]=function(){const _0xc200ff=_0x39ebd3,_0x3018ea=this[_0xc200ff(0x324)]();for(const _0x88386b of _0x3018ea){if(!_0x88386b)continue;_0x88386b['purifyCursedEquips']();}},Game_Actor[_0x39ebd3(0x3bc)][_0x39ebd3(0x4e4)]=function(){const _0x1a9e02=_0x39ebd3,_0x39b9c6=this[_0x1a9e02(0x18e)]()[_0x1a9e02(0x142)];for(let _0xe94c20=0x0;_0xe94c20<_0x39b9c6;_0xe94c20++){const _0x2e1e99=this[_0x1a9e02(0x4ac)][_0xe94c20];if(!_0x2e1e99)continue;const _0x13dae7=_0x2e1e99[_0x1a9e02(0x4d2)]();if(!DataManager[_0x1a9e02(0x1a6)](_0x13dae7))continue;let _0x1bfd21=DataManager[_0x1a9e02(0xa8)](_0x13dae7);this[_0x1a9e02(0x107)](_0x13dae7,_0x1bfd21)?(!this[_0x1a9e02(0x4ac)][_0xe94c20]&&(this['_equips'][_0xe94c20]=new Game_Item()),this[_0x1a9e02(0x4ac)][_0xe94c20][_0x1a9e02(0x14b)](_0x1bfd21),this['refresh']()):this['changeEquip'](_0xe94c20,null);}},Game_Actor[_0x39ebd3(0x3bc)]['isPurifyItemSwapOk']=function(_0x384c89,_0x2bd3f2){const _0x5ea4a6=_0x39ebd3;if(_0x384c89===_0x2bd3f2)return![];const _0x221151=DataManager[_0x5ea4a6(0x4a5)](_0x2bd3f2);if(!_0x221151[_0x5ea4a6(0x35f)](_0x384c89[_0x5ea4a6(0x34a)]))return![];if(DataManager[_0x5ea4a6(0x2db)](_0x2bd3f2))return this[_0x5ea4a6(0x31f)](_0x2bd3f2['wtypeId']);else{if(DataManager[_0x5ea4a6(0x13b)](_0x2bd3f2))return this['isEquipAtypeOk'](_0x2bd3f2['atypeId']);}return![];},TextManager[_0x39ebd3(0x4b7)]={'helpDesc':{'equip':VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x237)][_0x39ebd3(0x322)][_0x39ebd3(0x284)]??_0x39ebd3(0x3b2),'optimize':VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x237)]['EquipScene']['optimizeCmdDesc']??_0x39ebd3(0x26e),'clear':VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x237)]['EquipScene'][_0x39ebd3(0x2f3)]??_0x39ebd3(0x235)}},ColorManager[_0x39ebd3(0x477)]=function(_0x38b3b1){const _0x2f7d7e=_0x39ebd3;if(!_0x38b3b1)return this[_0x2f7d7e(0x4dc)]();else{if(_0x38b3b1[_0x2f7d7e(0x3ba)][_0x2f7d7e(0x145)](/<COLOR:[ ](\d+)>/i))return this[_0x2f7d7e(0x379)](Number(RegExp['$1'])[_0x2f7d7e(0x496)](0x0,0x1f));else return _0x38b3b1['note'][_0x2f7d7e(0x145)](/<COLOR:[ ]#(.*)>/i)?'#'+String(RegExp['$1']):this[_0x2f7d7e(0x4dc)]();}},ColorManager[_0x39ebd3(0x47c)]=function(_0x3fda8e){const _0x4ab45d=_0x39ebd3;return _0x3fda8e=String(_0x3fda8e),_0x3fda8e[_0x4ab45d(0x145)](/#(.*)/i)?_0x4ab45d(0x3ad)[_0x4ab45d(0x1da)](String(RegExp['$1'])):this['textColor'](Number(_0x3fda8e));},SceneManager[_0x39ebd3(0x13c)]=function(){const _0x4d6770=_0x39ebd3;return this[_0x4d6770(0x47d)]&&this['_scene']['constructor']===Scene_Shop;},Game_Temp['prototype'][_0x39ebd3(0x182)]=function(){const _0x5e2d97=_0x39ebd3;if(this['_bypassNewLabel'])return![];return VisuMZ[_0x5e2d97(0x2a9)][_0x5e2d97(0x237)]['New']['Enable'];},VisuMZ[_0x39ebd3(0x461)]=VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x237)][_0x39ebd3(0x1a9)]['MultiplierStandard'],VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x1f1)]=Game_BattlerBase[_0x39ebd3(0x3bc)][_0x39ebd3(0x154)],Game_BattlerBase[_0x39ebd3(0x3bc)][_0x39ebd3(0x154)]=function(_0xdf87d3){const _0x4a4ad6=_0x39ebd3;return this[_0x4a4ad6(0xcf)]?this[_0x4a4ad6(0x41c)]?VisuMZ[_0x4a4ad6(0x461)]:0x1:VisuMZ[_0x4a4ad6(0x2a9)]['Game_BattlerBase_param'][_0x4a4ad6(0x15c)](this,_0xdf87d3);},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x20c)]=Game_BattlerBase[_0x39ebd3(0x3bc)][_0x39ebd3(0x331)],Game_BattlerBase[_0x39ebd3(0x3bc)][_0x39ebd3(0x331)]=function(_0x1c0235){const _0x486fb8=_0x39ebd3;if(!_0x1c0235)return![];if(!VisuMZ[_0x486fb8(0x2a9)][_0x486fb8(0x20c)]['call'](this,_0x1c0235))return![];if(!this[_0x486fb8(0x176)](_0x1c0235))return![];if(!this[_0x486fb8(0x2e8)](_0x1c0235))return![];return!![];},Game_BattlerBase[_0x39ebd3(0x3bc)][_0x39ebd3(0x176)]=function(_0x51f5b7){const _0x5a6897=_0x39ebd3;if(!this[_0x5a6897(0x1c7)](_0x51f5b7))return![];return!![];},Game_BattlerBase[_0x39ebd3(0x3bc)]['checkItemConditionsSwitchNotetags']=function(_0x341585){const _0x10b200=_0x39ebd3,_0x3c6c34=_0x341585[_0x10b200(0x3ba)];if(_0x3c6c34[_0x10b200(0x145)](/<ENABLE[ ](?:SW|SWITCH|SWITCHES):[ ]*(\d+(?:\s*,\s*\d+)*)>/i)){const _0x1c07a3=JSON[_0x10b200(0x355)]('['+RegExp['$1'][_0x10b200(0x145)](/\d+/g)+']');for(const _0x127348 of _0x1c07a3){if(!$gameSwitches['value'](_0x127348))return![];}return!![];}if(_0x3c6c34[_0x10b200(0x145)](/<ENABLE ALL[ ](?:SW|SWITCH|SWITCHES):[ ]*(\d+(?:\s*,\s*\d+)*)>/i)){const _0x122ff2=JSON[_0x10b200(0x355)]('['+RegExp['$1'][_0x10b200(0x145)](/\d+/g)+']');for(const _0x4b9901 of _0x122ff2){if(!$gameSwitches['value'](_0x4b9901))return![];}return!![];}if(_0x3c6c34['match'](/<ENABLE ANY[ ](?:SW|SWITCH|SWITCHES):[ ]*(\d+(?:\s*,\s*\d+)*)>/i)){const _0x2aa024=JSON[_0x10b200(0x355)]('['+RegExp['$1'][_0x10b200(0x145)](/\d+/g)+']');for(const _0x86da41 of _0x2aa024){if($gameSwitches[_0x10b200(0x1a5)](_0x86da41))return!![];}return![];}if(_0x3c6c34[_0x10b200(0x145)](/<DISABLE[ ](?:SW|SWITCH|SWITCHES):[ ]*(\d+(?:\s*,\s*\d+)*)>/i)){const _0x5c8ad1=JSON[_0x10b200(0x355)]('['+RegExp['$1'][_0x10b200(0x145)](/\d+/g)+']');for(const _0x465253 of _0x5c8ad1){if(!$gameSwitches['value'](_0x465253))return!![];}return![];}if(_0x3c6c34['match'](/<DISABLE ALL[ ](?:SW|SWITCH|SWITCHES):[ ]*(\d+(?:\s*,\s*\d+)*)>/i)){const _0x25f3d5=JSON[_0x10b200(0x355)]('['+RegExp['$1'][_0x10b200(0x145)](/\d+/g)+']');for(const _0x2fbf3e of _0x25f3d5){if(!$gameSwitches[_0x10b200(0x1a5)](_0x2fbf3e))return!![];}return![];}if(_0x3c6c34['match'](/<DISABLE ANY[ ](?:SW|SWITCH|SWITCHES):[ ]*(\d+(?:\s*,\s*\d+)*)>/i)){const _0x69a0fb=JSON[_0x10b200(0x355)]('['+RegExp['$1']['match'](/\d+/g)+']');for(const _0xe73302 of _0x69a0fb){if($gameSwitches['value'](_0xe73302))return![];}return!![];}return!![];},Game_BattlerBase[_0x39ebd3(0x3bc)][_0x39ebd3(0x2e8)]=function(_0x3af6e9){const _0x5e4c4d=_0x39ebd3,_0xc6284d=_0x3af6e9[_0x5e4c4d(0x3ba)],_0x38c279=VisuMZ['ItemsEquipsCore']['itemEnableJS'];return _0x38c279[_0x3af6e9['id']]?_0x38c279[_0x3af6e9['id']][_0x5e4c4d(0x15c)](this,_0x3af6e9):!![];},Game_Actor[_0x39ebd3(0x3bc)]['clearEquipments']=function(){const _0x53df2d=_0x39ebd3,_0x5b9734=this[_0x53df2d(0x18e)]()['length'];for(let _0x5d5c79=0x0;_0x5d5c79<_0x5b9734;_0x5d5c79++){if(this['isClearEquipOk'](_0x5d5c79))this[_0x53df2d(0x434)](_0x5d5c79,null);}},Game_Actor['prototype']['isClearEquipOk']=function(_0x8e324d){const _0x251e19=_0x39ebd3;return this['nonRemovableEtypes']()['includes'](this[_0x251e19(0x18e)]()[_0x8e324d])?![]:this[_0x251e19(0x214)](_0x8e324d);},Game_Actor[_0x39ebd3(0x3bc)][_0x39ebd3(0x22d)]=function(){const _0x5f1e90=_0x39ebd3;return VisuMZ['ItemsEquipsCore']['Settings'][_0x5f1e90(0x322)][_0x5f1e90(0x47f)];},Game_Actor[_0x39ebd3(0x3bc)][_0x39ebd3(0xef)]=function(){const _0x401c66=_0x39ebd3,_0x560b8c=this[_0x401c66(0x18e)]()[_0x401c66(0x142)];for(let _0x249608=0x0;_0x249608<_0x560b8c;_0x249608++){if(this['isOptimizeEquipOk'](_0x249608))this['changeEquip'](_0x249608,null);}for(let _0x35a505=0x0;_0x35a505<_0x560b8c;_0x35a505++){if(this[_0x401c66(0xae)](_0x35a505))this[_0x401c66(0x434)](_0x35a505,this[_0x401c66(0x22a)](_0x35a505));}},Game_Actor[_0x39ebd3(0x3bc)][_0x39ebd3(0xae)]=function(_0x3648eb){const _0x41b50e=_0x39ebd3;return this[_0x41b50e(0x2fc)]()[_0x41b50e(0x35f)](this[_0x41b50e(0x18e)]()[_0x3648eb])?![]:this['isEquipChangeOk'](_0x3648eb);},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x288)]=Game_Actor[_0x39ebd3(0x3bc)][_0x39ebd3(0x214)],Game_Actor['prototype'][_0x39ebd3(0x214)]=function(_0x46ce25){const _0x6dbc32=_0x39ebd3;!this['_equips'][_0x46ce25]&&(this[_0x6dbc32(0x4ac)][_0x46ce25]=new Game_Item());const _0x1032be=this['_equips'][_0x46ce25];if(_0x1032be){const _0x74e558=_0x1032be['object']();if(DataManager[_0x6dbc32(0x1a6)](_0x74e558))return![];}return VisuMZ['ItemsEquipsCore']['Game_Actor_isEquipChangeOk'][_0x6dbc32(0x15c)](this,_0x46ce25);},Game_Actor[_0x39ebd3(0x3bc)][_0x39ebd3(0x2fc)]=function(){const _0x3ee996=_0x39ebd3;return VisuMZ[_0x3ee996(0x2a9)][_0x3ee996(0x237)][_0x3ee996(0x322)]['NonOptimizeETypes'];},VisuMZ[_0x39ebd3(0x2a9)]['Game_Actor_tradeItemWithParty']=Game_Actor[_0x39ebd3(0x3bc)]['tradeItemWithParty'],Game_Actor[_0x39ebd3(0x3bc)]['tradeItemWithParty']=function(_0x4ecc73,_0x56bd00){const _0x1d5c74=_0x39ebd3;if(this['_tempActor'])return![];$gameTemp[_0x1d5c74(0x15d)]=!![];const _0x4b37ba=VisuMZ[_0x1d5c74(0x2a9)]['Game_Actor_tradeItemWithParty'][_0x1d5c74(0x15c)](this,_0x4ecc73,_0x56bd00);return $gameTemp[_0x1d5c74(0x15d)]=![],_0x4b37ba;},Game_Actor[_0x39ebd3(0x3bc)]['changeEquipById']=function(_0x5130e0,_0x29d54a){const _0x4fa31d=_0x39ebd3,_0x154cf7=this['getNextAvailableEtypeId'](_0x5130e0);if(_0x154cf7<0x0)return;const _0x5d5731=_0x5130e0===0x1?$dataWeapons[_0x29d54a]:$dataArmors[_0x29d54a];this[_0x4fa31d(0x434)](_0x154cf7,_0x5d5731);},Game_Actor[_0x39ebd3(0x3bc)][_0x39ebd3(0x13a)]=function(_0x55dda0){const _0xcb2d27=_0x39ebd3;let _0x27b5ae=0x0;const _0x465d8b=this[_0xcb2d27(0x18e)](),_0x35bdcd=this[_0xcb2d27(0x257)]();for(let _0x1aee23=0x0;_0x1aee23<_0x465d8b['length'];_0x1aee23++){if(_0x465d8b[_0x1aee23]===_0x55dda0){_0x27b5ae=_0x1aee23;if(!_0x35bdcd[_0x1aee23])return _0x27b5ae;}}return _0x27b5ae;},VisuMZ[_0x39ebd3(0x2a9)]['Game_Actor_paramPlus']=Game_Actor[_0x39ebd3(0x3bc)][_0x39ebd3(0x36f)],Game_Actor[_0x39ebd3(0x3bc)][_0x39ebd3(0x36f)]=function(_0x46ea39){const _0x2756c4=_0x39ebd3;let _0x236057=VisuMZ[_0x2756c4(0x2a9)][_0x2756c4(0x139)]['call'](this,_0x46ea39);for(const _0x38416c of this[_0x2756c4(0x257)]()){if(_0x38416c)_0x236057+=this[_0x2756c4(0x2d6)](_0x38416c,_0x46ea39);}return _0x236057;},Game_Actor[_0x39ebd3(0x3bc)][_0x39ebd3(0x2d6)]=function(_0x2b7cb9,_0x1e1692){const _0xf6fe26=_0x39ebd3;if(this[_0xf6fe26(0x1fd)])return 0x0;const _0xbbb67b=(DataManager['isWeapon'](_0x2b7cb9)?_0xf6fe26(0xb9):_0xf6fe26(0x3b9))[_0xf6fe26(0x1da)](_0x2b7cb9['id']),_0x321bc9=_0xf6fe26(0x38e)[_0xf6fe26(0x1da)](_0xbbb67b,_0x1e1692);if(VisuMZ[_0xf6fe26(0x2a9)][_0xf6fe26(0x2d3)][_0x321bc9]){this[_0xf6fe26(0x1fd)]=!![];const _0x1ea78f=VisuMZ[_0xf6fe26(0x2a9)][_0xf6fe26(0x2d3)][_0x321bc9][_0xf6fe26(0x15c)](this,_0x2b7cb9,_0x1e1692);return this[_0xf6fe26(0x1fd)]=![],_0x1ea78f;}else return 0x0;},Game_Actor[_0x39ebd3(0x3bc)]['setShopStatusWindowMode']=function(_0x325dca){const _0x2580dd=_0x39ebd3;this[_0x2580dd(0xcf)]=!![],this['_shopStatusMenuAlly']=_0x325dca;},Game_Actor[_0x39ebd3(0x3bc)][_0x39ebd3(0x17b)]=function(_0x5a305e){const _0x47b9c8=_0x39ebd3;_0x5a305e=this[_0x47b9c8(0x4e1)](_0x5a305e);const _0x34a79c=this['equipSlots']();this[_0x47b9c8(0x4ac)]=[];for(let _0x5f37cf=0x0;_0x5f37cf<_0x34a79c[_0x47b9c8(0x142)];_0x5f37cf++){this['_equips'][_0x5f37cf]=new Game_Item();}for(let _0x3a533d=0x0;_0x3a533d<_0x34a79c[_0x47b9c8(0x142)];_0x3a533d++){const _0x287c2e=_0x34a79c[_0x3a533d],_0x1590a6=this[_0x47b9c8(0x289)](_0x5a305e,_0x287c2e);if(this[_0x47b9c8(0x1d6)](_0x1590a6))this[_0x47b9c8(0x4ac)][_0x3a533d]['setObject'](_0x1590a6);}this[_0x47b9c8(0xb7)](!![]),this[_0x47b9c8(0x39e)]();},Game_Actor['prototype']['convertInitEquipsToItems']=function(_0x1b26f6){const _0x20313d=_0x39ebd3,_0x2d7edd=[];for(let _0x5896d2=0x0;_0x5896d2<_0x1b26f6[_0x20313d(0x142)];_0x5896d2++){const _0x20b335=_0x1b26f6[_0x5896d2];if(_0x20b335<=0x0)continue;const _0x1b1330=$dataSystem[_0x20313d(0x1ce)][_0x5896d2+0x1];if(_0x1b1330===$dataSystem['equipTypes'][0x1]||_0x5896d2===0x1&&this['isDualWield']())_0x2d7edd[_0x20313d(0x17a)]($dataWeapons[_0x20b335]);else{if(BattleManager[_0x20313d(0x4c4)]()){const _0x423faf=$dataArmors[_0x20b335];_0x423faf&&_0x423faf[_0x20313d(0x34a)]===_0x5896d2+0x1&&_0x2d7edd[_0x20313d(0x17a)](_0x423faf);}else{const _0x23730d=$dataArmors[_0x20b335];_0x23730d&&_0x23730d['etypeId']===_0x5896d2+0x1&&_0x2d7edd[_0x20313d(0x17a)](_0x23730d);}}}return _0x2d7edd;},Game_Actor[_0x39ebd3(0x3bc)]['getMatchingInitEquip']=function(_0x592e65,_0x127a6f){const _0x2276f5=_0x39ebd3;for(const _0x37257f of _0x592e65){if(!_0x37257f)continue;if(_0x37257f['etypeId']===_0x127a6f)return _0x592e65[_0x2276f5(0x302)](_0x592e65[_0x2276f5(0x180)](_0x37257f),0x1),_0x37257f;}return null;},Game_Actor[_0x39ebd3(0x3bc)][_0x39ebd3(0x18e)]=function(){const _0x5c6fce=_0x39ebd3,_0x1b22c7=VisuMZ['ItemsEquipsCore'][_0x5c6fce(0x3b5)](this[_0x5c6fce(0x24c)]||this[_0x5c6fce(0xbb)]()['equipSlots']);if(_0x1b22c7[_0x5c6fce(0x142)]>=0x2&&this[_0x5c6fce(0x4d0)]())_0x1b22c7[0x1]=0x1;return _0x1b22c7;},Game_Actor[_0x39ebd3(0x3bc)]['forceChangeEquipSlots']=function(_0x138255){const _0x3339dc=_0x39ebd3;_0x138255[_0x3339dc(0x2c9)](0x0),_0x138255[_0x3339dc(0x2c9)](-0x1),this[_0x3339dc(0x24c)]=_0x138255,this['refresh'](),this[_0x3339dc(0x4ce)]();},Game_Actor[_0x39ebd3(0x3bc)][_0x39ebd3(0x185)]=function(){const _0x403d83=_0x39ebd3;this[_0x403d83(0x24c)]=undefined,this[_0x403d83(0x39e)](),this[_0x403d83(0x4ce)]();},Game_Actor[_0x39ebd3(0x3bc)]['updateChangedSlots']=function(){const _0x4209a3=_0x39ebd3;let _0x49926b=this[_0x4209a3(0x18e)]()[_0x4209a3(0x142)];while(this['_equips']['length']>_0x49926b){const _0x3e9a29=this[_0x4209a3(0x4ac)][this[_0x4209a3(0x4ac)][_0x4209a3(0x142)]-0x1];_0x3e9a29&&_0x3e9a29[_0x4209a3(0x4d2)]()&&$gameParty[_0x4209a3(0x33b)](_0x3e9a29['object'](),0x1),this[_0x4209a3(0x4ac)][_0x4209a3(0x2ad)]();}while(_0x49926b>this[_0x4209a3(0x4ac)][_0x4209a3(0x142)]){this[_0x4209a3(0x4ac)][_0x4209a3(0x17a)](new Game_Item());}},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x10a)]=Game_Actor[_0x39ebd3(0x3bc)][_0x39ebd3(0x1f5)],Game_Actor[_0x39ebd3(0x3bc)][_0x39ebd3(0x1f5)]=function(_0x5e8273,_0x1334b1){const _0x4c7358=_0x39ebd3;VisuMZ[_0x4c7358(0x2a9)][_0x4c7358(0x10a)][_0x4c7358(0x15c)](this,_0x5e8273,_0x1334b1),this[_0x4c7358(0x4ce)]();},Game_Actor['prototype'][_0x39ebd3(0x1f0)]=function(){const _0x106e90=_0x39ebd3,_0x15f996=this['equipSlots']();for(let _0x51a63e=0x0;_0x51a63e<_0x15f996['length'];_0x51a63e++){if(!this[_0x106e90(0x4ac)][_0x51a63e])this[_0x106e90(0x4ac)][_0x51a63e]=new Game_Item();}this[_0x106e90(0xb7)](![]),this[_0x106e90(0x39e)]();},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0xfd)]=Game_Actor[_0x39ebd3(0x3bc)]['changeEquip'],Game_Actor[_0x39ebd3(0x3bc)][_0x39ebd3(0x434)]=function(_0x599d36,_0x540b03){const _0x26297e=_0x39ebd3;if(!this[_0x26297e(0x119)]){const _0x503e5e=JsonEx[_0x26297e(0x4e8)](this);_0x503e5e['_tempActor']=!![],this[_0x26297e(0x1f7)](_0x599d36,_0x540b03),this[_0x26297e(0xfc)](_0x503e5e);}else this[_0x26297e(0x1f7)](_0x599d36,_0x540b03);},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x1b6)]=Game_Actor[_0x39ebd3(0x3bc)]['forceChangeEquip'],Game_Actor[_0x39ebd3(0x3bc)]['forceChangeEquip']=function(_0x4df6b0,_0x32b42d){const _0x1298e5=_0x39ebd3;!this[_0x1298e5(0x4ac)][_0x4df6b0]&&(this[_0x1298e5(0x4ac)][_0x4df6b0]=new Game_Item());if(!this[_0x1298e5(0x119)]){const _0x406673=JsonEx[_0x1298e5(0x4e8)](this);_0x406673['_tempActor']=!![],VisuMZ['ItemsEquipsCore'][_0x1298e5(0x1b6)]['call'](this,_0x4df6b0,_0x32b42d),this['equipAdjustHpMp'](_0x406673);}else VisuMZ[_0x1298e5(0x2a9)][_0x1298e5(0x1b6)][_0x1298e5(0x15c)](this,_0x4df6b0,_0x32b42d);},VisuMZ['ItemsEquipsCore'][_0x39ebd3(0x236)]=Game_Actor['prototype'][_0x39ebd3(0x2f4)],Game_Actor[_0x39ebd3(0x3bc)][_0x39ebd3(0x2f4)]=function(_0x53a6c5){const _0x222f2b=_0x39ebd3;if(!this[_0x222f2b(0x119)]){const _0x236c22=JsonEx[_0x222f2b(0x4e8)](this);_0x236c22['_tempActor']=!![],VisuMZ[_0x222f2b(0x2a9)][_0x222f2b(0x236)][_0x222f2b(0x15c)](this,_0x53a6c5),this['equipAdjustHpMp'](_0x236c22);}else VisuMZ[_0x222f2b(0x2a9)][_0x222f2b(0x236)]['call'](this,_0x53a6c5);},Game_Actor[_0x39ebd3(0x3bc)][_0x39ebd3(0xb7)]=function(_0x2b360d){const _0x5cdb74=_0x39ebd3;if(this[_0x5cdb74(0x3a2)])return;let _0x5c78db=0x0;for(;;){_0x5c78db++;if(_0x5c78db>0x3)break;const _0x30002a=this['equipSlots'](),_0x17d3f4=this[_0x5cdb74(0x257)](),_0x2d1107=_0x17d3f4[_0x5cdb74(0x142)];let _0x49f42f=![];for(let _0x405834=0x0;_0x405834<_0x2d1107;_0x405834++){const _0x9b351b=_0x17d3f4[_0x405834];if(!_0x9b351b)continue;const _0x4aa7ab=DataManager[_0x5cdb74(0x4a5)](_0x9b351b);if(!this[_0x5cdb74(0x1d6)](_0x9b351b)||!_0x4aa7ab[_0x5cdb74(0x35f)](_0x30002a[_0x405834])){!_0x2b360d&&this[_0x5cdb74(0xd5)](null,_0x9b351b);if(!this[_0x5cdb74(0x119)]){const _0x191b53=JsonEx[_0x5cdb74(0x4e8)](this);_0x191b53[_0x5cdb74(0x119)]=!![],this[_0x5cdb74(0x4ac)][_0x405834][_0x5cdb74(0x14b)](null),this[_0x5cdb74(0x3a2)]=!![],this['equipAdjustHpMp'](_0x191b53),this[_0x5cdb74(0x3a2)]=undefined;}else{if(this[_0x5cdb74(0x4ac)][_0x405834])this[_0x5cdb74(0x4ac)][_0x405834][_0x5cdb74(0x14b)](null);else continue;}_0x49f42f=!![];}}if(!_0x49f42f)break;}},Game_Actor['prototype'][_0x39ebd3(0xfc)]=function(_0x59b3f0){const _0x11f10c=_0x39ebd3;if(this['_tempActor'])return;if(!VisuMZ[_0x11f10c(0x2a9)][_0x11f10c(0x237)][_0x11f10c(0x322)][_0x11f10c(0x2cf)])return;const _0x24ed99=Math[_0x11f10c(0x2c3)](_0x59b3f0[_0x11f10c(0x251)]()*this[_0x11f10c(0x2fd)]),_0x119e95=Math[_0x11f10c(0x2c3)](_0x59b3f0[_0x11f10c(0x2d8)]()*this['mmp']);if(this['hp']>0x0)this[_0x11f10c(0x151)](_0x24ed99);if(this['mp']>0x0)this[_0x11f10c(0x307)](_0x119e95);},Game_Actor[_0x39ebd3(0x3bc)][_0x39ebd3(0x1f7)]=function(_0x36c7a3,_0x5dc5c1){const _0x55e9b4=_0x39ebd3;if(!this[_0x55e9b4(0xd5)](_0x5dc5c1,this[_0x55e9b4(0x257)]()[_0x36c7a3]))return;if(_0x5dc5c1){const _0x164601=DataManager[_0x55e9b4(0x4a5)](_0x5dc5c1);if(!_0x164601[_0x55e9b4(0x35f)](this['equipSlots']()[_0x36c7a3]))return;}!this[_0x55e9b4(0x4ac)][_0x36c7a3]&&(this['_equips'][_0x36c7a3]=new Game_Item());this[_0x55e9b4(0x4ac)][_0x36c7a3][_0x55e9b4(0x14b)](_0x5dc5c1);if(VisuMZ[_0x55e9b4(0x2a9)][_0x55e9b4(0x247)](_0x5dc5c1)){const _0x4f9b26=VisuMZ[_0x55e9b4(0x2a9)]['Settings']['EquipScene'][_0x55e9b4(0x1d5)]||'',_0x3ac48d=this[_0x55e9b4(0x292)](),_0x2b432b=_0x55e9b4(0x272)['format'](_0x5dc5c1[_0x55e9b4(0x229)]),_0x38c645=_0x5dc5c1[_0x55e9b4(0x292)]||'';let _0x17ec68=_0x4f9b26[_0x55e9b4(0x1da)](_0x3ac48d,_0x2b432b,_0x38c645);if(VisuMZ[_0x55e9b4(0x1e7)][_0x55e9b4(0x4c1)]>=1.79&&_0x17ec68['length']>0x0)$textPopup(_0x17ec68);}this[_0x55e9b4(0x39e)]();},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x247)]=function(_0x144357){const _0x3dd634=_0x39ebd3;if(!_0x144357)return![];if(!Imported[_0x3dd634(0x372)])return![];if(VisuMZ[_0x3dd634(0x1e7)]['version']<1.79)return![];return DataManager[_0x3dd634(0x1a6)](_0x144357);},Game_Actor[_0x39ebd3(0x3bc)][_0x39ebd3(0x22a)]=function(_0x1d402b){const _0x44cfe3=_0x39ebd3,_0x2c90a1=this[_0x44cfe3(0x18e)]()[_0x1d402b],_0x41b267=$gameParty[_0x44cfe3(0x2e3)]()[_0x44cfe3(0x285)](_0x44dae4=>DataManager[_0x44cfe3(0x4a5)](_0x44dae4)['includes'](_0x2c90a1)&&this[_0x44cfe3(0x1d6)](_0x44dae4)&&!DataManager['isCursedItem'](_0x44dae4));let _0x4ee2a4=null,_0x4a84b4=-0x3e8;for(let _0x11857f=0x0;_0x11857f<_0x41b267[_0x44cfe3(0x142)];_0x11857f++){const _0x1f9d76=_0x41b267[_0x11857f];if(!this[_0x44cfe3(0x31d)](_0x1f9d76))continue;const _0x5f18f7=this[_0x44cfe3(0x175)](_0x1f9d76);_0x5f18f7>_0x4a84b4&&(_0x4a84b4=_0x5f18f7,_0x4ee2a4=_0x1f9d76);}return _0x4ee2a4;},Game_Actor[_0x39ebd3(0x3bc)]['canEquipWithOptimize']=function(_0x1120b7){const _0xfbc72=_0x39ebd3;if(!_0x1120b7)return![];const _0x193a32=_0x1120b7?_0x1120b7[_0xfbc72(0x3ba)]:'';if(_0x193a32[_0xfbc72(0x145)](/<EQUIP COPY LIMIT:[ ](\d+)>/i)){const _0x451ae6=Number(RegExp['$1'])||0x1,_0x29e1b0=this[_0xfbc72(0x257)]()['filter'](_0x5dc92=>_0x5dc92===_0x1120b7);if(_0x29e1b0[_0xfbc72(0x142)]>=_0x451ae6)return![];}if(DataManager[_0xfbc72(0x2db)](_0x1120b7)){if(_0x193a32[_0xfbc72(0x145)](/<EQUIP WEAPON TYPE LIMIT:[ ](\d+)>/i)){const _0x1cdc6c=Number(RegExp['$1'])||0x1,_0x2e30cc=this[_0xfbc72(0x492)]()[_0xfbc72(0x2c9)](null)['filter'](_0x2d615b=>_0x2d615b[_0xfbc72(0x1d7)]===_0x1120b7[_0xfbc72(0x1d7)]);if(_0x2e30cc['length']>=_0x1cdc6c)return![];}{const _0x18848a=this[_0xfbc72(0x492)]()[_0xfbc72(0x2c9)](null)[_0xfbc72(0x285)](_0x298214=>_0x298214[_0xfbc72(0x1d7)]===_0x1120b7[_0xfbc72(0x1d7)]);if(_0x18848a['length']>0x0){let _0x220326=0x270f;for(const _0x870453 of _0x18848a){_0x870453[_0xfbc72(0x3ba)]['match'](/<EQUIP WEAPON TYPE LIMIT:[ ](\d+)>/i)&&(_0x220326=Math[_0xfbc72(0x37b)](_0x220326,Number(RegExp['$1'])));}if(_0x18848a['length']>=_0x220326)return![];}}}if(DataManager[_0xfbc72(0x13b)](_0x1120b7)){if(_0x193a32[_0xfbc72(0x145)](/<EQUIP ARMOR TYPE LIMIT:[ ](\d+)>/i)){const _0x3e1bff=Number(RegExp['$1'])||0x1,_0x55fddb=this[_0xfbc72(0x1ae)]()[_0xfbc72(0x2c9)](null)['filter'](_0x28f0f9=>_0x28f0f9[_0xfbc72(0x31e)]===_0x1120b7[_0xfbc72(0x31e)]);if(_0x55fddb['length']>=_0x3e1bff)return![];}{const _0x3f16b8=this[_0xfbc72(0x1ae)]()['remove'](null)[_0xfbc72(0x285)](_0x37d38c=>_0x37d38c[_0xfbc72(0x31e)]===_0x1120b7[_0xfbc72(0x31e)]);if(_0x3f16b8[_0xfbc72(0x142)]>0x0){let _0x3d25f2=0x270f;for(const _0x3ccd24 of _0x3f16b8){_0x3ccd24[_0xfbc72(0x3ba)][_0xfbc72(0x145)](/<EQUIP ARMOR TYPE LIMIT:[ ](\d+)>/i)&&(_0x3d25f2=Math[_0xfbc72(0x37b)](_0x3d25f2,Number(RegExp['$1'])));}if(_0x3f16b8[_0xfbc72(0x142)]>=_0x3d25f2)return![];}}}return!![];},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x4c2)]=Game_Party[_0x39ebd3(0x3bc)][_0x39ebd3(0x4a2)],Game_Party[_0x39ebd3(0x3bc)][_0x39ebd3(0x4a2)]=function(){const _0x279609=_0x39ebd3;VisuMZ['ItemsEquipsCore'][_0x279609(0x4c2)]['call'](this),this[_0x279609(0x485)](),this[_0x279609(0x1be)]();},Game_Party[_0x39ebd3(0x3bc)][_0x39ebd3(0x485)]=function(){const _0x46d59a=_0x39ebd3;this[_0x46d59a(0x371)]=[];},Game_Party[_0x39ebd3(0x3bc)][_0x39ebd3(0x140)]=function(_0x35849a){const _0x20db08=_0x39ebd3;if(!$gameTemp['newLabelEnabled']())return![];if(this['_newItemsList']===undefined)this[_0x20db08(0x485)]();let _0x43aefd='';if(DataManager[_0x20db08(0xc2)](_0x35849a))_0x43aefd='item-%1'[_0x20db08(0x1da)](_0x35849a['id']);else{if(DataManager[_0x20db08(0x2db)](_0x35849a))_0x43aefd='weapon-%1'[_0x20db08(0x1da)](_0x35849a['id']);else{if(DataManager[_0x20db08(0x13b)](_0x35849a))_0x43aefd='armor-%1'[_0x20db08(0x1da)](_0x35849a['id']);else return;}}return this[_0x20db08(0x371)][_0x20db08(0x35f)](_0x43aefd);},Game_Party[_0x39ebd3(0x3bc)][_0x39ebd3(0x4a8)]=function(_0x4277a0){const _0x2af9d3=_0x39ebd3;if(!$gameTemp[_0x2af9d3(0x182)]())return;if(this['_newItemsList']===undefined)this['initNewItemsList']();let _0x2bc386='';if(DataManager[_0x2af9d3(0xc2)](_0x4277a0))_0x2bc386=_0x2af9d3(0x32f)[_0x2af9d3(0x1da)](_0x4277a0['id']);else{if(DataManager['isWeapon'](_0x4277a0))_0x2bc386=_0x2af9d3(0x1ef)[_0x2af9d3(0x1da)](_0x4277a0['id']);else{if(DataManager[_0x2af9d3(0x13b)](_0x4277a0))_0x2bc386=_0x2af9d3(0x4ef)[_0x2af9d3(0x1da)](_0x4277a0['id']);else return;}}if(!this[_0x2af9d3(0x371)][_0x2af9d3(0x35f)](_0x2bc386))this[_0x2af9d3(0x371)][_0x2af9d3(0x17a)](_0x2bc386);},Game_Party[_0x39ebd3(0x3bc)][_0x39ebd3(0x308)]=function(_0x1fd877){const _0x31edca=_0x39ebd3;if(!$gameTemp[_0x31edca(0x182)]())return;if(this[_0x31edca(0x371)]===undefined)this['initNewItemsList']();let _0x591999='';if(DataManager[_0x31edca(0xc2)](_0x1fd877))_0x591999='item-%1'[_0x31edca(0x1da)](_0x1fd877['id']);else{if(DataManager[_0x31edca(0x2db)](_0x1fd877))_0x591999=_0x31edca(0x1ef)[_0x31edca(0x1da)](_0x1fd877['id']);else{if(DataManager['isArmor'](_0x1fd877))_0x591999=_0x31edca(0x4ef)[_0x31edca(0x1da)](_0x1fd877['id']);else return;}}this[_0x31edca(0x371)][_0x31edca(0x35f)](_0x591999)&&this[_0x31edca(0x371)][_0x31edca(0x302)](this[_0x31edca(0x371)][_0x31edca(0x180)](_0x591999),0x1);},VisuMZ['ItemsEquipsCore'][_0x39ebd3(0x22e)]=Game_Party[_0x39ebd3(0x3bc)][_0x39ebd3(0x410)],Game_Party[_0x39ebd3(0x3bc)]['numItems']=function(_0x24358a){const _0x4403eb=_0x39ebd3;if(DataManager[_0x4403eb(0xfa)](_0x24358a))_0x24358a=DataManager[_0x4403eb(0x47b)](_0x24358a);return VisuMZ['ItemsEquipsCore'][_0x4403eb(0x22e)][_0x4403eb(0x15c)](this,_0x24358a);},VisuMZ['ItemsEquipsCore'][_0x39ebd3(0xaf)]=Game_Party[_0x39ebd3(0x3bc)][_0x39ebd3(0x33b)],Game_Party[_0x39ebd3(0x3bc)][_0x39ebd3(0x33b)]=function(_0x3ed801,_0x4965c9,_0x151e97){const _0x1cbe1a=_0x39ebd3;if(DataManager['isProxyItem'](_0x3ed801))_0x3ed801=null;const _0x305bb2=this['numItems'](_0x3ed801);VisuMZ[_0x1cbe1a(0x2a9)]['Game_Party_gainItem'][_0x1cbe1a(0x15c)](this,_0x3ed801,_0x4965c9,_0x151e97);if(this[_0x1cbe1a(0x410)](_0x3ed801)>_0x305bb2)this[_0x1cbe1a(0x4a8)](_0x3ed801);},Game_Party[_0x39ebd3(0x3bc)][_0x39ebd3(0x28d)]=function(_0x220075){const _0x260db0=_0x39ebd3;if(DataManager[_0x260db0(0xfa)](_0x220075))_0x220075=DataManager['getProxyItem'](_0x220075);return DataManager[_0x260db0(0x109)](_0x220075);},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x380)]=Game_Party['prototype'][_0x39ebd3(0x32b)],Game_Party[_0x39ebd3(0x3bc)]['consumeItem']=function(_0x1ae932){const _0x53f4b4=_0x39ebd3;if(_0x1ae932){const _0x175c90=_0x1ae932[_0x53f4b4(0x3ba)]||'';if(_0x175c90['match'](/<(?:CONSERVE|PRESERVE):[ ](\d+)([%％])>/i)){const _0xa0fec0=Number(RegExp['$1'])*0.01;if(Math['random']()<_0xa0fec0)return;}}VisuMZ['ItemsEquipsCore'][_0x53f4b4(0x380)][_0x53f4b4(0x15c)](this,_0x1ae932);},Game_Party['prototype'][_0x39ebd3(0x1be)]=function(){const _0x4edce8=_0x39ebd3;this[_0x4edce8(0x2c0)]={'buy':{'gold':0x0,'items':{}},'sell':{'gold':0x0,'items':{}}};},Game_Party['prototype']['getShopTrackingData']=function(){const _0x283aa3=_0x39ebd3;return this['_shopTrackingData']===undefined&&this[_0x283aa3(0x1be)](),this[_0x283aa3(0x2c0)];},Game_Party[_0x39ebd3(0x3bc)][_0x39ebd3(0x1ec)]=function(_0x24d98d,_0x384c0e){const _0x178218=_0x39ebd3;if(!_0x384c0e)return 0x0;this[_0x178218(0x2c0)]===undefined&&this[_0x178218(0x1be)]();const _0x23716a=this[_0x178218(0x466)]();if(!_0x23716a[_0x24d98d])return 0x0;if(_0x384c0e==='gold')return _0x23716a[_0x24d98d][_0x178218(0x4c7)]=_0x23716a[_0x24d98d][_0x178218(0x4c7)]||0x0,_0x23716a[_0x24d98d][_0x178218(0x4c7)];else{if(DataManager[_0x178218(0xc2)](_0x384c0e))key='item-%1'['format'](_0x384c0e['id']);else{if(DataManager[_0x178218(0x2db)](_0x384c0e))key=_0x178218(0x1ef)[_0x178218(0x1da)](_0x384c0e['id']);else{if(DataManager['isArmor'](_0x384c0e))key='armor-%1'[_0x178218(0x1da)](_0x384c0e['id']);else return 0x0;}}}return _0x23716a[_0x24d98d][_0x178218(0x20f)][key]=_0x23716a[_0x24d98d][_0x178218(0x20f)][key]||0x0,_0x23716a[_0x24d98d][_0x178218(0x20f)][key];},Game_Party[_0x39ebd3(0x3bc)][_0x39ebd3(0x3ab)]=function(_0x35b645){const _0x5cac7e=_0x39ebd3;return this[_0x5cac7e(0x1ec)](_0x5cac7e(0xcb),_0x35b645);},Game_Party[_0x39ebd3(0x3bc)]['getShopTrackingItemSell']=function(_0x52e4c2){const _0x5aee9f=_0x39ebd3;return this[_0x5aee9f(0x1ec)]('sell',_0x52e4c2);},Game_Party[_0x39ebd3(0x3bc)][_0x39ebd3(0x430)]=function(){const _0x2753c1=_0x39ebd3;return this[_0x2753c1(0x1ec)]('buy',_0x2753c1(0x4c7));},Game_Party[_0x39ebd3(0x3bc)]['getShopTrackingGoldSell']=function(){const _0x2080a1=_0x39ebd3;return this[_0x2080a1(0x1ec)](_0x2080a1(0x1c1),'gold');},Game_Party[_0x39ebd3(0x3bc)][_0x39ebd3(0x224)]=function(_0x435d04,_0xbd4254,_0x11084d){const _0x4767ff=_0x39ebd3;if(!_0xbd4254)return;if(_0x11084d<=0x0)return;this['_shopTrackingData']===undefined&&this[_0x4767ff(0x1be)]();const _0x1c835d=this['getShopTrackingData']();if(!_0x1c835d[_0x435d04])return;if(_0xbd4254===_0x4767ff(0x4c7)){_0x1c835d[_0x435d04]['gold']=_0x1c835d[_0x435d04]['gold']||0x0,_0x1c835d[_0x435d04][_0x4767ff(0x4c7)]+=_0x11084d;return;}else{if(DataManager[_0x4767ff(0xc2)](_0xbd4254))key=_0x4767ff(0x32f)[_0x4767ff(0x1da)](_0xbd4254['id']);else{if(DataManager[_0x4767ff(0x2db)](_0xbd4254))key=_0x4767ff(0x1ef)[_0x4767ff(0x1da)](_0xbd4254['id']);else{if(DataManager[_0x4767ff(0x13b)](_0xbd4254))key=_0x4767ff(0x4ef)[_0x4767ff(0x1da)](_0xbd4254['id']);else return;}}}_0x1c835d[_0x435d04][_0x4767ff(0x20f)][key]=_0x1c835d[_0x435d04][_0x4767ff(0x20f)][key]||0x0,_0x1c835d[_0x435d04][_0x4767ff(0x20f)][key]+=_0x11084d;},Game_Party['prototype'][_0x39ebd3(0x343)]=function(_0x103218,_0x69595a){const _0x5ecb4f=_0x39ebd3;this[_0x5ecb4f(0x224)](_0x5ecb4f(0xcb),_0x103218,_0x69595a);},Game_Party[_0x39ebd3(0x3bc)]['addShopTrackingItemSell']=function(_0x4c865b,_0x5df62c){const _0x6975da=_0x39ebd3;this[_0x6975da(0x224)](_0x6975da(0x1c1),_0x4c865b,_0x5df62c);},Game_Party[_0x39ebd3(0x3bc)][_0x39ebd3(0x2f7)]=function(_0x3d483d){const _0xd92590=_0x39ebd3;this['addShopTrackingItem'](_0xd92590(0xcb),_0xd92590(0x4c7),_0x3d483d);},Game_Party[_0x39ebd3(0x3bc)]['addShopTrackingGoldSell']=function(_0x4415d6){const _0x3cec09=_0x39ebd3;this[_0x3cec09(0x224)]('sell',_0x3cec09(0x4c7),_0x4415d6);},VisuMZ['ItemsEquipsCore'][_0x39ebd3(0x3e6)]=Scene_ItemBase[_0x39ebd3(0x3bc)][_0x39ebd3(0x488)],Scene_ItemBase[_0x39ebd3(0x3bc)]['activateItemWindow']=function(){const _0x168e72=_0x39ebd3;VisuMZ[_0x168e72(0x2a9)]['Scene_ItemBase_activateItemWindow'][_0x168e72(0x15c)](this),this['_itemWindow'][_0x168e72(0x401)]();},Scene_Item[_0x39ebd3(0x3bc)]['isPureVanillaMode']=function(){const _0x28d88a=_0x39ebd3,_0x5dd435=VisuMZ['ItemsEquipsCore']['Settings'][_0x28d88a(0x3b4)];return!_0x5dd435['EnableLayout']&&_0x5dd435[_0x28d88a(0x4a7)];},Scene_Item[_0x39ebd3(0x3bc)][_0x39ebd3(0x312)]=function(){const _0x31ca19=_0x39ebd3;if(ConfigManager[_0x31ca19(0x1cc)]&&ConfigManager[_0x31ca19(0x2b1)]!==undefined)return ConfigManager['uiHelpPosition'];else return this[_0x31ca19(0x2b4)]()?this[_0x31ca19(0x444)]()[_0x31ca19(0x145)](/LOWER/i):Scene_ItemBase[_0x31ca19(0x3bc)][_0x31ca19(0x312)][_0x31ca19(0x15c)](this);},Scene_Item[_0x39ebd3(0x3bc)]['isRightInputMode']=function(){const _0x4ba72e=_0x39ebd3;if(ConfigManager[_0x4ba72e(0x1cc)]&&ConfigManager[_0x4ba72e(0x32d)]!==undefined)return ConfigManager['uiInputPosition'];else return this[_0x4ba72e(0x2b4)]()?this[_0x4ba72e(0x444)]()[_0x4ba72e(0x145)](/RIGHT/i):Scene_ItemBase[_0x4ba72e(0x3bc)][_0x4ba72e(0x4ab)][_0x4ba72e(0x15c)](this);},Scene_Item[_0x39ebd3(0x3bc)][_0x39ebd3(0x444)]=function(){const _0x387217=_0x39ebd3;return VisuMZ[_0x387217(0x2a9)][_0x387217(0x237)]['ItemScene']['LayoutStyle'];},Scene_Item['prototype'][_0x39ebd3(0x28c)]=function(){const _0x43f07d=_0x39ebd3,_0xa6bbd3=SceneManager[_0x43f07d(0x47d)];if(_0xa6bbd3['constructor']===Scene_Item&&_0xa6bbd3['isPureVanillaMode']())return![];return this[_0x43f07d(0x120)]&&this[_0x43f07d(0x120)][_0x43f07d(0x28c)]();},Scene_Item[_0x39ebd3(0x3bc)][_0x39ebd3(0x2b4)]=function(){const _0x39935d=_0x39ebd3;if(this[_0x39935d(0xd4)]())return![];return VisuMZ[_0x39935d(0x2a9)]['Settings'][_0x39935d(0x3b4)][_0x39935d(0x306)];},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x48e)]=Scene_Item[_0x39ebd3(0x3bc)][_0x39ebd3(0x268)],Scene_Item[_0x39ebd3(0x3bc)][_0x39ebd3(0x268)]=function(){const _0x3d37b1=_0x39ebd3;VisuMZ[_0x3d37b1(0x2a9)]['Scene_Item_create'][_0x3d37b1(0x15c)](this),this[_0x3d37b1(0x28c)]()&&this[_0x3d37b1(0x2b9)]();},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x3bf)]=Scene_Item[_0x39ebd3(0x3bc)][_0x39ebd3(0x153)],Scene_Item[_0x39ebd3(0x3bc)][_0x39ebd3(0x153)]=function(){const _0x1d040f=_0x39ebd3;return this[_0x1d040f(0x2b4)]()?this['helpWindowRectItemsEquipsCore']():VisuMZ[_0x1d040f(0x2a9)]['Scene_Item_helpWindowRect'][_0x1d040f(0x15c)](this);},Scene_Item[_0x39ebd3(0x3bc)][_0x39ebd3(0x3fd)]=function(){const _0x4b0897=_0x39ebd3,_0x353503=0x0,_0x18e3e2=this[_0x4b0897(0x4bb)](),_0x27fe86=Graphics['boxWidth'],_0x16f8d8=this[_0x4b0897(0xb1)]();return new Rectangle(_0x353503,_0x18e3e2,_0x27fe86,_0x16f8d8);},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x4bf)]=Scene_Item[_0x39ebd3(0x3bc)][_0x39ebd3(0x406)],Scene_Item[_0x39ebd3(0x3bc)][_0x39ebd3(0x406)]=function(){const _0x54d4d5=_0x39ebd3;VisuMZ[_0x54d4d5(0x2a9)][_0x54d4d5(0x4bf)][_0x54d4d5(0x15c)](this),this[_0x54d4d5(0x28c)]()&&this['postCreateCategoryWindowItemsEquipsCore']();},Scene_Item['prototype'][_0x39ebd3(0x30e)]=function(){const _0x26bfe5=_0x39ebd3;delete this['_categoryWindow'][_0x26bfe5(0x12c)]['ok'],delete this[_0x26bfe5(0x120)][_0x26bfe5(0x12c)][_0x26bfe5(0x4ed)];},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x1fa)]=Scene_Item[_0x39ebd3(0x3bc)][_0x39ebd3(0x147)],Scene_Item[_0x39ebd3(0x3bc)][_0x39ebd3(0x147)]=function(){const _0x3d9459=_0x39ebd3;return this[_0x3d9459(0x2b4)]()?this[_0x3d9459(0x1ba)]():VisuMZ[_0x3d9459(0x2a9)][_0x3d9459(0x1fa)]['call'](this);},Scene_Item[_0x39ebd3(0x3bc)][_0x39ebd3(0x1ba)]=function(){const _0x1b5d74=_0x39ebd3,_0x4d54a0=0x0,_0x2d2def=this['mainAreaTop'](),_0x15a1ce=Graphics[_0x1b5d74(0x13d)],_0x4d5e47=this[_0x1b5d74(0x46e)](0x1,!![]);return new Rectangle(_0x4d54a0,_0x2d2def,_0x15a1ce,_0x4d5e47);},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x1c0)]=Scene_Item[_0x39ebd3(0x3bc)][_0x39ebd3(0x2e2)],Scene_Item[_0x39ebd3(0x3bc)]['createItemWindow']=function(){const _0x5cfc80=_0x39ebd3;VisuMZ[_0x5cfc80(0x2a9)][_0x5cfc80(0x1c0)][_0x5cfc80(0x15c)](this),this[_0x5cfc80(0x28c)]()&&this[_0x5cfc80(0x408)](),this[_0x5cfc80(0xe3)]()&&this[_0x5cfc80(0x4f0)]();},VisuMZ['ItemsEquipsCore']['Scene_Item_itemWindowRect']=Scene_Item[_0x39ebd3(0x3bc)][_0x39ebd3(0x2c2)],Scene_Item[_0x39ebd3(0x3bc)]['itemWindowRect']=function(){const _0x398918=_0x39ebd3;if(this[_0x398918(0x2b4)]())return this['itemWindowRectItemsEquipsCore']();else{const _0x22a034=VisuMZ['ItemsEquipsCore'][_0x398918(0x23e)][_0x398918(0x15c)](this);return this[_0x398918(0xe3)]()&&this[_0x398918(0x2d5)]()&&(_0x22a034[_0x398918(0x417)]-=this[_0x398918(0x215)]()),_0x22a034;}},Scene_Item[_0x39ebd3(0x3bc)]['itemWindowRectItemsEquipsCore']=function(){const _0x4681bc=_0x39ebd3,_0x17acce=this[_0x4681bc(0x4ab)]()?this['statusWidth']():0x0,_0x49963d=this[_0x4681bc(0x120)]['y']+this[_0x4681bc(0x120)]['height'],_0x1db699=Graphics[_0x4681bc(0x13d)]-this[_0x4681bc(0x215)](),_0x45d600=this[_0x4681bc(0x375)]()-_0x49963d;return new Rectangle(_0x17acce,_0x49963d,_0x1db699,_0x45d600);},Scene_Item[_0x39ebd3(0x3bc)]['postCreateItemWindowModernControls']=function(){const _0x2e2837=_0x39ebd3;this[_0x2e2837(0x286)][_0x2e2837(0x269)](_0x2e2837(0x4ed),this['popScene'][_0x2e2837(0xdc)](this));},Scene_Item['prototype'][_0x39ebd3(0xe3)]=function(){const _0x464cdb=_0x39ebd3;if(this[_0x464cdb(0xd4)]())return![];return this[_0x464cdb(0x2b4)]()?!![]:VisuMZ[_0x464cdb(0x2a9)][_0x464cdb(0x237)][_0x464cdb(0x3b4)]['ShowShopStatus'];},Scene_Item[_0x39ebd3(0x3bc)][_0x39ebd3(0x2d5)]=function(){const _0x2cc9ab=_0x39ebd3;return VisuMZ[_0x2cc9ab(0x2a9)][_0x2cc9ab(0x237)][_0x2cc9ab(0x3b4)][_0x2cc9ab(0x369)];},Scene_Item[_0x39ebd3(0x3bc)][_0x39ebd3(0x4f0)]=function(){const _0xde9feb=_0x39ebd3,_0x5c8654=this[_0xde9feb(0xd3)]();this['_statusWindow']=new Window_ShopStatus(_0x5c8654),this[_0xde9feb(0x301)](this[_0xde9feb(0x441)]),this['_itemWindow'][_0xde9feb(0x420)](this[_0xde9feb(0x441)]);const _0x2600bf=VisuMZ[_0xde9feb(0x2a9)][_0xde9feb(0x237)][_0xde9feb(0x3b4)][_0xde9feb(0x20b)];this['_statusWindow'][_0xde9feb(0x3eb)](_0x2600bf||0x0);},Scene_Item[_0x39ebd3(0x3bc)][_0x39ebd3(0xd3)]=function(){const _0x29bf78=_0x39ebd3;return this[_0x29bf78(0x2b4)]()?this[_0x29bf78(0x177)]():VisuMZ[_0x29bf78(0x2a9)]['Settings']['ItemScene'][_0x29bf78(0x12e)]['call'](this);},Scene_Item[_0x39ebd3(0x3bc)][_0x39ebd3(0x177)]=function(){const _0x36f632=_0x39ebd3,_0x1edd14=this[_0x36f632(0x215)](),_0x227dc3=this['_itemWindow'][_0x36f632(0x435)],_0x2c07d9=this[_0x36f632(0x4ab)]()?0x0:Graphics['boxWidth']-this['statusWidth'](),_0x26e0bc=this[_0x36f632(0x286)]['y'];return new Rectangle(_0x2c07d9,_0x26e0bc,_0x1edd14,_0x227dc3);},Scene_Item[_0x39ebd3(0x3bc)][_0x39ebd3(0x215)]=function(){const _0x125eb7=_0x39ebd3;return Scene_Shop[_0x125eb7(0x3bc)]['statusWidth']();},Scene_Item['prototype'][_0x39ebd3(0x3fa)]=function(){const _0x39b7fc=_0x39ebd3;if(!this[_0x39b7fc(0x444)]())return![];if(!this[_0x39b7fc(0x28c)]())return![];if(!this[_0x39b7fc(0x286)])return![];if(!this[_0x39b7fc(0x286)]['active'])return![];return this[_0x39b7fc(0x444)]()&&this[_0x39b7fc(0x28c)]();},Scene_Item[_0x39ebd3(0x3bc)][_0x39ebd3(0x3ef)]=function(){const _0x428eeb=_0x39ebd3;if(this[_0x428eeb(0x3fa)]())return this[_0x428eeb(0x286)][_0x428eeb(0x340)]()===0x1?TextManager[_0x428eeb(0xa6)](_0x428eeb(0x135),_0x428eeb(0x3d7)):TextManager['getInputMultiButtonStrings'](_0x428eeb(0xa7),'pagedown');return Scene_ItemBase[_0x428eeb(0x3bc)]['buttonAssistKey1']['call'](this);},Scene_Item[_0x39ebd3(0x3bc)][_0x39ebd3(0x4e7)]=function(){const _0xd19c00=_0x39ebd3;if(this[_0xd19c00(0x3fa)]())return VisuMZ[_0xd19c00(0x2a9)][_0xd19c00(0x237)][_0xd19c00(0x3b4)]['buttonAssistCategory'];return Scene_ItemBase[_0xd19c00(0x3bc)][_0xd19c00(0x4e7)][_0xd19c00(0x15c)](this);},Scene_Equip[_0x39ebd3(0x3bc)][_0x39ebd3(0x1e9)]=function(){const _0x2817d0=_0x39ebd3;Scene_ItemBase[_0x2817d0(0x3bc)][_0x2817d0(0x1e9)][_0x2817d0(0x15c)](this),this[_0x2817d0(0x3d6)]();},Scene_Equip['prototype'][_0x39ebd3(0x312)]=function(){const _0xd4b2d7=_0x39ebd3;if(ConfigManager['uiMenuStyle']&&ConfigManager['uiHelpPosition']!==undefined)return ConfigManager[_0xd4b2d7(0x2b1)];else{if(this[_0xd4b2d7(0x2b4)]())return this[_0xd4b2d7(0x444)]()[_0xd4b2d7(0x145)](/LOWER/i);else Scene_MenuBase[_0xd4b2d7(0x3bc)][_0xd4b2d7(0x4ab)]['call'](this);}},Scene_Equip['prototype']['isRightInputMode']=function(){const _0x2e82ef=_0x39ebd3;if(ConfigManager[_0x2e82ef(0x1cc)]&&ConfigManager[_0x2e82ef(0x32d)]!==undefined)return ConfigManager[_0x2e82ef(0x32d)];else{if(this[_0x2e82ef(0x2b4)]())return this[_0x2e82ef(0x444)]()[_0x2e82ef(0x145)](/RIGHT/i);else Scene_MenuBase[_0x2e82ef(0x3bc)][_0x2e82ef(0x4ab)][_0x2e82ef(0x15c)](this);}},Scene_Equip[_0x39ebd3(0x3bc)][_0x39ebd3(0x444)]=function(){const _0x42de77=_0x39ebd3;return VisuMZ[_0x42de77(0x2a9)][_0x42de77(0x237)][_0x42de77(0x322)][_0x42de77(0x33f)];},Scene_Equip['prototype']['isUseModernControls']=function(){const _0x3067dd=_0x39ebd3;return this[_0x3067dd(0x1cb)]&&this[_0x3067dd(0x1cb)][_0x3067dd(0x28c)]();},Scene_Equip['prototype'][_0x39ebd3(0x2b4)]=function(){const _0x3090cd=_0x39ebd3;return VisuMZ[_0x3090cd(0x2a9)][_0x3090cd(0x237)][_0x3090cd(0x322)][_0x3090cd(0x306)];},VisuMZ['ItemsEquipsCore'][_0x39ebd3(0x49b)]=Scene_Equip[_0x39ebd3(0x3bc)][_0x39ebd3(0x268)],Scene_Equip[_0x39ebd3(0x3bc)]['create']=function(){const _0xa690ca=_0x39ebd3;VisuMZ[_0xa690ca(0x2a9)][_0xa690ca(0x49b)][_0xa690ca(0x15c)](this),this[_0xa690ca(0x28c)]()&&this['commandEquip']();},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x1f3)]=Scene_Equip[_0x39ebd3(0x3bc)][_0x39ebd3(0x153)],Scene_Equip[_0x39ebd3(0x3bc)]['helpWindowRect']=function(){const _0x3f9bad=_0x39ebd3;return this[_0x3f9bad(0x2b4)]()?this[_0x3f9bad(0x3fd)]():VisuMZ[_0x3f9bad(0x2a9)][_0x3f9bad(0x1f3)]['call'](this);},Scene_Equip['prototype'][_0x39ebd3(0x3fd)]=function(){const _0x17e578=_0x39ebd3,_0x3f63c8=0x0,_0x49f1fb=this[_0x17e578(0x4bb)](),_0x96d6b7=Graphics[_0x17e578(0x13d)],_0x28b67d=this['helpAreaHeight']();return new Rectangle(_0x3f63c8,_0x49f1fb,_0x96d6b7,_0x28b67d);},VisuMZ[_0x39ebd3(0x2a9)]['Scene_Equip_statusWindowRect']=Scene_Equip['prototype'][_0x39ebd3(0xd3)],Scene_Equip[_0x39ebd3(0x3bc)]['statusWindowRect']=function(){const _0x325198=_0x39ebd3;return this['isUseItemsEquipsCoreUpdatedLayout']()?this[_0x325198(0x177)]():VisuMZ[_0x325198(0x2a9)]['Scene_Equip_statusWindowRect']['call'](this);},Scene_Equip[_0x39ebd3(0x3bc)][_0x39ebd3(0x177)]=function(){const _0xfd3257=_0x39ebd3,_0xdcce5d=this[_0xfd3257(0x4ab)]()?0x0:Graphics['boxWidth']-this[_0xfd3257(0x215)](),_0x1124f1=this[_0xfd3257(0x3e7)](),_0x1d9495=this[_0xfd3257(0x215)](),_0x4ad9fd=this[_0xfd3257(0x100)]();return new Rectangle(_0xdcce5d,_0x1124f1,_0x1d9495,_0x4ad9fd);},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x20d)]=Scene_Equip[_0x39ebd3(0x3bc)][_0x39ebd3(0x1f6)],Scene_Equip[_0x39ebd3(0x3bc)][_0x39ebd3(0x1f6)]=function(){const _0x6cacb3=_0x39ebd3;VisuMZ[_0x6cacb3(0x2a9)][_0x6cacb3(0x20d)]['call'](this);if(this[_0x6cacb3(0xb5)])this['_commandWindow'][_0x6cacb3(0x241)](this['_helpWindow']);},VisuMZ['ItemsEquipsCore']['Scene_Equip_commandWindowRect']=Scene_Equip[_0x39ebd3(0x3bc)][_0x39ebd3(0x255)],Scene_Equip['prototype'][_0x39ebd3(0x255)]=function(){const _0x59a1d7=_0x39ebd3;return this['isUseItemsEquipsCoreUpdatedLayout']()?this[_0x59a1d7(0x16e)]():VisuMZ[_0x59a1d7(0x2a9)][_0x59a1d7(0x2c6)][_0x59a1d7(0x15c)](this);},Scene_Equip['prototype'][_0x39ebd3(0x131)]=function(){const _0x558c3f=_0x39ebd3,_0x283a48=VisuMZ['ItemsEquipsCore'][_0x558c3f(0x237)][_0x558c3f(0x322)];return _0x283a48['CommandAddOptimize']||_0x283a48[_0x558c3f(0x33c)];},Scene_Equip[_0x39ebd3(0x3bc)][_0x39ebd3(0x16e)]=function(){const _0x2f502a=_0x39ebd3,_0xaf6082=this['shouldCommandWindowExist'](),_0xa4d1a4=this[_0x2f502a(0x4ab)]()?this[_0x2f502a(0x215)]():0x0,_0x50147a=this[_0x2f502a(0x3e7)](),_0x402a64=Graphics['boxWidth']-this[_0x2f502a(0x215)](),_0x456227=_0xaf6082?this['calcWindowHeight'](0x1,!![]):0x0;return new Rectangle(_0xa4d1a4,_0x50147a,_0x402a64,_0x456227);},VisuMZ['ItemsEquipsCore']['Scene_Equip_createSlotWindow']=Scene_Equip[_0x39ebd3(0x3bc)][_0x39ebd3(0x46f)],Scene_Equip[_0x39ebd3(0x3bc)][_0x39ebd3(0x46f)]=function(){const _0x174cce=_0x39ebd3;VisuMZ[_0x174cce(0x2a9)][_0x174cce(0x11f)][_0x174cce(0x15c)](this),this[_0x174cce(0x28c)]()&&this[_0x174cce(0x28f)]();},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0xb3)]=Scene_Equip[_0x39ebd3(0x3bc)][_0x39ebd3(0x4b1)],Scene_Equip['prototype'][_0x39ebd3(0x4b1)]=function(){const _0x2f23f7=_0x39ebd3;return this[_0x2f23f7(0x2b4)]()?this[_0x2f23f7(0x4d6)]():VisuMZ['ItemsEquipsCore'][_0x2f23f7(0xb3)]['call'](this);},Scene_Equip[_0x39ebd3(0x3bc)][_0x39ebd3(0x4d6)]=function(){const _0x13856f=_0x39ebd3,_0x1c9f8a=this[_0x13856f(0x255)](),_0x11a209=this[_0x13856f(0x4ab)]()?this[_0x13856f(0x215)]():0x0,_0x32f29d=_0x1c9f8a['y']+_0x1c9f8a[_0x13856f(0x435)],_0x579e27=Graphics['boxWidth']-this[_0x13856f(0x215)](),_0x544dea=this[_0x13856f(0x100)]()-_0x1c9f8a[_0x13856f(0x435)];return new Rectangle(_0x11a209,_0x32f29d,_0x579e27,_0x544dea);},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x17c)]=Scene_Equip[_0x39ebd3(0x3bc)][_0x39ebd3(0x2c2)],Scene_Equip[_0x39ebd3(0x3bc)]['itemWindowRect']=function(){const _0x398578=_0x39ebd3;return this['isUseItemsEquipsCoreUpdatedLayout']()?this[_0x398578(0x4b1)]():VisuMZ[_0x398578(0x2a9)][_0x398578(0x17c)][_0x398578(0x15c)](this);},Scene_Equip[_0x39ebd3(0x3bc)][_0x39ebd3(0x215)]=function(){const _0x937f89=_0x39ebd3;return this[_0x937f89(0x2b4)]()?this[_0x937f89(0x49d)]():VisuMZ['ItemsEquipsCore'][_0x937f89(0x237)]['EquipScene'][_0x937f89(0x2f0)];},Scene_Equip[_0x39ebd3(0x3bc)]['geUpdatedLayoutStatusWidth']=function(){const _0xd192cc=_0x39ebd3;return Math[_0xd192cc(0x349)](Graphics['boxWidth']/0x2);},Scene_Equip[_0x39ebd3(0x3bc)]['postCreateSlotWindowItemsEquipsCore']=function(){const _0x37f85f=_0x39ebd3;this['_slotWindow']['setHandler']('cancel',this[_0x37f85f(0x267)][_0x37f85f(0xdc)](this)),this[_0x37f85f(0x482)]['setHandler'](_0x37f85f(0x3e4),this[_0x37f85f(0xa1)][_0x37f85f(0xdc)](this)),this[_0x37f85f(0x482)][_0x37f85f(0x269)](_0x37f85f(0xa7),this['previousActor'][_0x37f85f(0xdc)](this));},VisuMZ['ItemsEquipsCore']['Scene_Equip_commandEquip']=Scene_Equip[_0x39ebd3(0x3bc)]['commandEquip'],Scene_Equip['prototype']['commandEquip']=function(){const _0x5a1707=_0x39ebd3;this[_0x5a1707(0x28c)]()&&(this[_0x5a1707(0x1cb)]['deselect'](),this[_0x5a1707(0x1cb)]['deactivate']()),VisuMZ[_0x5a1707(0x2a9)][_0x5a1707(0x389)][_0x5a1707(0x15c)](this);},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x4c5)]=Scene_Equip[_0x39ebd3(0x3bc)]['onSlotOk'],Scene_Equip[_0x39ebd3(0x3bc)][_0x39ebd3(0x298)]=function(){const _0x2674c0=_0x39ebd3;this['_slotWindow'][_0x2674c0(0x376)]()>=0x0?(VisuMZ['ItemsEquipsCore'][_0x2674c0(0x4c5)][_0x2674c0(0x15c)](this),this['onSlotOkAutoSelect']()):(this[_0x2674c0(0x482)]['smoothSelect'](0x0),this[_0x2674c0(0x482)][_0x2674c0(0x3a9)]());},Scene_Equip['prototype']['onSlotOkAutoSelect']=function(){const _0x50f763=_0x39ebd3;this[_0x50f763(0x286)][_0x50f763(0x39e)]();const _0x4f763b=this[_0x50f763(0x482)][_0x50f763(0x27c)](),_0x33ee6d=this['_itemWindow'][_0x50f763(0x190)][_0x50f763(0x180)](_0x4f763b),_0xbae206=Math[_0x50f763(0x349)](this[_0x50f763(0x286)][_0x50f763(0x196)]()/0x2)-0x1;this[_0x50f763(0x286)][_0x50f763(0x436)](_0x33ee6d>=0x0?_0x33ee6d:0x0),this[_0x50f763(0x286)][_0x50f763(0x137)]>0x1&&(this[_0x50f763(0x286)]['_scrollDuration']=0x1,this[_0x50f763(0x286)][_0x50f763(0x3e9)]()),this[_0x50f763(0x286)][_0x50f763(0x41b)](this[_0x50f763(0x286)][_0x50f763(0x376)]()-_0xbae206);},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x15b)]=Scene_Equip[_0x39ebd3(0x3bc)][_0x39ebd3(0x159)],Scene_Equip[_0x39ebd3(0x3bc)]['onSlotCancel']=function(){const _0x26afef=_0x39ebd3;VisuMZ[_0x26afef(0x2a9)][_0x26afef(0x15b)]['call'](this),this['isUseModernControls']()&&(this[_0x26afef(0x1cb)][_0x26afef(0x436)](0x0),this[_0x26afef(0x482)][_0x26afef(0x3d9)]());},VisuMZ[_0x39ebd3(0x2a9)]['Scene_Equip_onActorChange']=Scene_Equip['prototype'][_0x39ebd3(0x3c4)],Scene_Equip['prototype']['onActorChange']=function(){const _0x22102a=_0x39ebd3;VisuMZ[_0x22102a(0x2a9)][_0x22102a(0x161)]['call'](this),this[_0x22102a(0x28c)]()&&(this[_0x22102a(0x1cb)][_0x22102a(0x3d9)](),this[_0x22102a(0x1cb)][_0x22102a(0x213)](),this[_0x22102a(0x482)]['smoothSelect'](0x0),this[_0x22102a(0x482)][_0x22102a(0x3a9)]());},Scene_Equip[_0x39ebd3(0x3bc)][_0x39ebd3(0x9a)]=function(){const _0x3e4e25=_0x39ebd3;if(!this[_0x3e4e25(0x482)])return![];if(!this['_slotWindow'][_0x3e4e25(0x274)])return![];return this[_0x3e4e25(0x482)]['isShiftRemoveShortcutEnabled']();},Scene_Equip[_0x39ebd3(0x3bc)][_0x39ebd3(0x44d)]=function(){const _0x57f717=_0x39ebd3;if(this['buttonAssistSlotWindowShift']())return TextManager[_0x57f717(0x117)](_0x57f717(0x195));return Scene_MenuBase['prototype']['buttonAssistKey3'][_0x57f717(0x15c)](this);},Scene_Equip[_0x39ebd3(0x3bc)][_0x39ebd3(0x4cb)]=function(){const _0x58a711=_0x39ebd3;if(this[_0x58a711(0x9a)]())return VisuMZ['ItemsEquipsCore'][_0x58a711(0x237)][_0x58a711(0x322)][_0x58a711(0x114)];return Scene_MenuBase['prototype'][_0x58a711(0x4cb)][_0x58a711(0x15c)](this);},Scene_Equip[_0x39ebd3(0x3bc)][_0x39ebd3(0x2fb)]=function(){const _0x12f038=_0x39ebd3;if(this[_0x12f038(0x9a)]())return this[_0x12f038(0x30f)]['width']/0x5/-0x3;return Scene_MenuBase[_0x12f038(0x3bc)][_0x12f038(0x2fb)][_0x12f038(0x15c)](this);},Scene_Equip[_0x39ebd3(0x3bc)][_0x39ebd3(0x267)]=function(){const _0x39df60=_0x39ebd3;SceneManager[_0x39df60(0x2ad)]();},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x1d0)]=Scene_Load[_0x39ebd3(0x3bc)][_0x39ebd3(0x472)],Scene_Load[_0x39ebd3(0x3bc)][_0x39ebd3(0x472)]=function(){const _0x4c518a=_0x39ebd3;VisuMZ[_0x4c518a(0x2a9)]['Scene_Load_reloadMapIfUpdated'][_0x4c518a(0x15c)](this),this[_0x4c518a(0x423)]();},Scene_Load['prototype'][_0x39ebd3(0x423)]=function(){const _0x5bc088=_0x39ebd3;if($gameSystem[_0x5bc088(0x149)]()!==$dataSystem['versionId'])for(const _0x445d62 of $gameActors[_0x5bc088(0x190)]){if(_0x445d62)_0x445d62[_0x5bc088(0x1f0)]();}},Scene_Shop[_0x39ebd3(0x3bc)][_0x39ebd3(0x312)]=function(){const _0xf8129e=_0x39ebd3;if(ConfigManager['uiMenuStyle']&&ConfigManager[_0xf8129e(0x2b1)]!==undefined)return ConfigManager[_0xf8129e(0x2b1)];else{if(this['isUseItemsEquipsCoreUpdatedLayout']())return this[_0xf8129e(0x444)]()['match'](/LOWER/i);else Scene_MenuBase[_0xf8129e(0x3bc)][_0xf8129e(0x4ab)][_0xf8129e(0x15c)](this);}},Scene_Shop[_0x39ebd3(0x3bc)][_0x39ebd3(0x4ab)]=function(){const _0x53eb59=_0x39ebd3;if(ConfigManager['uiMenuStyle']&&ConfigManager['uiInputPosition']!==undefined)return ConfigManager['uiInputPosition'];else{if(this[_0x53eb59(0x2b4)]())return this[_0x53eb59(0x444)]()['match'](/RIGHT/i);else Scene_MenuBase[_0x53eb59(0x3bc)][_0x53eb59(0x4ab)]['call'](this);}},Scene_Shop[_0x39ebd3(0x3bc)]['updatedLayoutStyle']=function(){const _0x2a29f5=_0x39ebd3;return VisuMZ[_0x2a29f5(0x2a9)][_0x2a29f5(0x237)][_0x2a29f5(0x3a4)]['LayoutStyle'];},Scene_Shop[_0x39ebd3(0x3bc)][_0x39ebd3(0x28c)]=function(){const _0x3e625b=_0x39ebd3;return this[_0x3e625b(0x120)]&&this['_categoryWindow']['isUseModernControls']();},Scene_Shop[_0x39ebd3(0x3bc)][_0x39ebd3(0x2b4)]=function(){const _0x506abc=_0x39ebd3;return VisuMZ[_0x506abc(0x2a9)][_0x506abc(0x237)][_0x506abc(0x3a4)][_0x506abc(0x306)];},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x41a)]=Scene_Shop[_0x39ebd3(0x3bc)][_0x39ebd3(0x2f8)],Scene_Shop[_0x39ebd3(0x3bc)]['prepare']=function(_0x29082c,_0x377373){const _0x133f61=_0x39ebd3;_0x29082c=VisuMZ['ItemsEquipsCore'][_0x133f61(0x3b5)](_0x29082c),VisuMZ[_0x133f61(0x2a9)][_0x133f61(0x41a)][_0x133f61(0x15c)](this,_0x29082c,_0x377373),this[_0x133f61(0x3a3)]();},Scene_Shop[_0x39ebd3(0x3bc)][_0x39ebd3(0x3a3)]=function(){const _0x50deeb=_0x39ebd3;this['_goodsCount']=0x0;const _0x20c908=[];for(const _0xb2c420 of this[_0x50deeb(0x1b1)]){this[_0x50deeb(0x11b)](_0xb2c420)?this[_0x50deeb(0x3cb)]++:_0x20c908[_0x50deeb(0x17a)](_0xb2c420);}for(const _0x3bae16 of _0x20c908){this[_0x50deeb(0x1b1)]['remove'](_0x3bae16);}},Scene_Shop['prototype']['isGoodShown']=function(_0x149108){if(_0x149108[0x0]>0x2||_0x149108[0x0]<0x0)return![];const _0x489a3a=[$dataItems,$dataWeapons,$dataArmors][_0x149108[0x0]][_0x149108[0x1]];if(!_0x489a3a)return![];return!![];},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0xa3)]=Scene_Shop[_0x39ebd3(0x3bc)][_0x39ebd3(0x268)],Scene_Shop[_0x39ebd3(0x3bc)]['create']=function(){const _0x33d327=_0x39ebd3;VisuMZ[_0x33d327(0x2a9)][_0x33d327(0xa3)][_0x33d327(0x15c)](this),this['isUseItemsEquipsCoreUpdatedLayout']()&&this[_0x33d327(0x36a)](),this[_0x33d327(0x259)]();},Scene_Shop['prototype'][_0x39ebd3(0x36a)]=function(){const _0x1d70ad=_0x39ebd3;this['_dummyWindow']['hide'](),this[_0x1d70ad(0x3d1)][_0x1d70ad(0x2ae)](),this[_0x1d70ad(0x3d1)]['deselect'](),this[_0x1d70ad(0x441)][_0x1d70ad(0x2ae)]();},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x443)]=Scene_Shop[_0x39ebd3(0x3bc)]['helpWindowRect'],Scene_Shop[_0x39ebd3(0x3bc)][_0x39ebd3(0x153)]=function(){const _0x1f9ec6=_0x39ebd3;return this['isUseItemsEquipsCoreUpdatedLayout']()?this[_0x1f9ec6(0x3fd)]():VisuMZ['ItemsEquipsCore'][_0x1f9ec6(0x443)][_0x1f9ec6(0x15c)](this);},Scene_Shop[_0x39ebd3(0x3bc)]['helpWindowRectItemsEquipsCore']=function(){const _0x5be075=_0x39ebd3,_0x37d741=0x0,_0x2fc2d8=this[_0x5be075(0x4bb)](),_0x32bd0c=Graphics[_0x5be075(0x13d)],_0x5bedf6=this[_0x5be075(0xb1)]();return new Rectangle(_0x37d741,_0x2fc2d8,_0x32bd0c,_0x5bedf6);},VisuMZ['ItemsEquipsCore'][_0x39ebd3(0x491)]=Scene_Shop[_0x39ebd3(0x3bc)][_0x39ebd3(0x210)],Scene_Shop[_0x39ebd3(0x3bc)][_0x39ebd3(0x210)]=function(){const _0x2ef138=_0x39ebd3;return this[_0x2ef138(0x2b4)]()?this[_0x2ef138(0x46b)]():VisuMZ[_0x2ef138(0x2a9)][_0x2ef138(0x491)][_0x2ef138(0x15c)](this);},Scene_Shop[_0x39ebd3(0x3bc)][_0x39ebd3(0x46b)]=function(){const _0x39cb7b=_0x39ebd3,_0x150509=this[_0x39cb7b(0x4e9)](),_0x2dd465=this[_0x39cb7b(0x46e)](0x1,!![]),_0x30eec7=this[_0x39cb7b(0x4ab)]()?0x0:Graphics[_0x39cb7b(0x13d)]-_0x150509,_0x28b375=this[_0x39cb7b(0x3e7)]();return new Rectangle(_0x30eec7,_0x28b375,_0x150509,_0x2dd465);},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x29e)]=Scene_Shop[_0x39ebd3(0x3bc)][_0x39ebd3(0x255)],Scene_Shop['prototype'][_0x39ebd3(0x255)]=function(){const _0x289aa4=_0x39ebd3;return this['isUseItemsEquipsCoreUpdatedLayout']()?this['commandWindowRectItemsEquipsCore']():VisuMZ[_0x289aa4(0x2a9)]['Scene_Shop_commandWindowRect'][_0x289aa4(0x15c)](this);},Scene_Shop['prototype'][_0x39ebd3(0x16e)]=function(){const _0x38fae8=_0x39ebd3,_0x316d46=this['isRightInputMode']()?this[_0x38fae8(0x4e9)]():0x0,_0xcf6858=this['mainAreaTop'](),_0xa32a6d=Graphics[_0x38fae8(0x13d)]-this[_0x38fae8(0x4e9)](),_0x6451e0=this[_0x38fae8(0x46e)](0x1,!![]);return new Rectangle(_0x316d46,_0xcf6858,_0xa32a6d,_0x6451e0);},VisuMZ[_0x39ebd3(0x2a9)]['Scene_Shop_numberWindowRect']=Scene_Shop[_0x39ebd3(0x3bc)][_0x39ebd3(0x2dd)],Scene_Shop['prototype'][_0x39ebd3(0x2dd)]=function(){const _0x2cdd1b=_0x39ebd3;return this[_0x2cdd1b(0x2b4)]()?this[_0x2cdd1b(0x327)]():VisuMZ[_0x2cdd1b(0x2a9)][_0x2cdd1b(0x3d3)][_0x2cdd1b(0x15c)](this);},Scene_Shop[_0x39ebd3(0x3bc)][_0x39ebd3(0x327)]=function(){const _0x2fac1e=_0x39ebd3,_0x262adc=this[_0x2fac1e(0x1cb)]['y']+this[_0x2fac1e(0x1cb)][_0x2fac1e(0x435)],_0x43a61c=Graphics[_0x2fac1e(0x13d)]-this[_0x2fac1e(0x215)](),_0x408f6d=this[_0x2fac1e(0x4ab)]()?Graphics[_0x2fac1e(0x13d)]-_0x43a61c:0x0,_0x3c811a=this[_0x2fac1e(0x100)]()-this[_0x2fac1e(0x1cb)][_0x2fac1e(0x435)];return new Rectangle(_0x408f6d,_0x262adc,_0x43a61c,_0x3c811a);},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x3ff)]=Scene_Shop['prototype'][_0x39ebd3(0xd3)],Scene_Shop['prototype'][_0x39ebd3(0xd3)]=function(){const _0x41df6a=_0x39ebd3;return this[_0x41df6a(0x2b4)]()?this[_0x41df6a(0x177)]():VisuMZ[_0x41df6a(0x2a9)]['Scene_Shop_statusWindowRect'][_0x41df6a(0x15c)](this);},Scene_Shop[_0x39ebd3(0x3bc)][_0x39ebd3(0x177)]=function(){const _0x21e152=_0x39ebd3,_0x4d2d45=this[_0x21e152(0x215)](),_0x4c90d9=this[_0x21e152(0x100)]()-this[_0x21e152(0x1cb)]['height'],_0x550e30=this[_0x21e152(0x4ab)]()?0x0:Graphics[_0x21e152(0x13d)]-_0x4d2d45,_0x46ca15=this['_commandWindow']['y']+this[_0x21e152(0x1cb)][_0x21e152(0x435)];return new Rectangle(_0x550e30,_0x46ca15,_0x4d2d45,_0x4c90d9);},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0xf3)]=Scene_Shop[_0x39ebd3(0x3bc)][_0x39ebd3(0xc0)],Scene_Shop[_0x39ebd3(0x3bc)][_0x39ebd3(0xc0)]=function(){const _0x43dbbf=_0x39ebd3;return this[_0x43dbbf(0x2b4)]()?this[_0x43dbbf(0x49a)]():VisuMZ[_0x43dbbf(0x2a9)][_0x43dbbf(0xf3)][_0x43dbbf(0x15c)](this);},Scene_Shop[_0x39ebd3(0x3bc)][_0x39ebd3(0x49a)]=function(){const _0x2cfb7b=_0x39ebd3,_0x32e39a=this[_0x2cfb7b(0x1cb)]['y']+this[_0x2cfb7b(0x1cb)][_0x2cfb7b(0x435)],_0x2914c2=Graphics[_0x2cfb7b(0x13d)]-this['statusWidth'](),_0x3e55da=this['mainAreaHeight']()-this[_0x2cfb7b(0x1cb)][_0x2cfb7b(0x435)],_0x37d07a=this[_0x2cfb7b(0x4ab)]()?Graphics['boxWidth']-_0x2914c2:0x0;return new Rectangle(_0x37d07a,_0x32e39a,_0x2914c2,_0x3e55da);},VisuMZ[_0x39ebd3(0x2a9)]['Scene_Shop_createCategoryWindow']=Scene_Shop[_0x39ebd3(0x3bc)]['createCategoryWindow'],Scene_Shop[_0x39ebd3(0x3bc)][_0x39ebd3(0x406)]=function(){const _0x3abdff=_0x39ebd3;VisuMZ[_0x3abdff(0x2a9)][_0x3abdff(0x4de)]['call'](this),this[_0x3abdff(0x28c)]()&&this[_0x3abdff(0x30e)]();},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x293)]=Scene_Shop['prototype'][_0x39ebd3(0x147)],Scene_Shop['prototype'][_0x39ebd3(0x147)]=function(){const _0x2ec4ba=_0x39ebd3;return this[_0x2ec4ba(0x2b4)]()?this[_0x2ec4ba(0x1ba)]():VisuMZ['ItemsEquipsCore']['Scene_Shop_categoryWindowRect'][_0x2ec4ba(0x15c)](this);},Scene_Shop[_0x39ebd3(0x3bc)][_0x39ebd3(0x1ba)]=function(){const _0x13a490=_0x39ebd3,_0x5d7cc7=this[_0x13a490(0x1cb)]['y'],_0x1f67f1=this[_0x13a490(0x1cb)]['width'],_0x60bdbc=this[_0x13a490(0x46e)](0x1,!![]),_0x1add45=this[_0x13a490(0x4ab)]()?Graphics['boxWidth']-_0x1f67f1:0x0;return new Rectangle(_0x1add45,_0x5d7cc7,_0x1f67f1,_0x60bdbc);},Scene_Shop['prototype']['postCreateCategoryWindowItemsEquipsCore']=function(){const _0x4b688f=_0x39ebd3;delete this[_0x4b688f(0x120)][_0x4b688f(0x12c)]['ok'],delete this[_0x4b688f(0x120)][_0x4b688f(0x12c)][_0x4b688f(0x4ed)];},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x1d8)]=Scene_Shop[_0x39ebd3(0x3bc)][_0x39ebd3(0x3b3)],Scene_Shop[_0x39ebd3(0x3bc)][_0x39ebd3(0x3b3)]=function(){const _0x31ffd0=_0x39ebd3;VisuMZ['ItemsEquipsCore']['Scene_Shop_createSellWindow'][_0x31ffd0(0x15c)](this),this[_0x31ffd0(0x2b4)]()&&this[_0x31ffd0(0xb2)]();},VisuMZ[_0x39ebd3(0x2a9)]['Scene_Shop_sellWindowRect']=Scene_Shop[_0x39ebd3(0x3bc)][_0x39ebd3(0x110)],Scene_Shop[_0x39ebd3(0x3bc)][_0x39ebd3(0x110)]=function(){const _0x1f8500=_0x39ebd3;return this[_0x1f8500(0x2b4)]()?this[_0x1f8500(0x203)]():VisuMZ['ItemsEquipsCore'][_0x1f8500(0x4f2)]['call'](this);},Scene_Shop[_0x39ebd3(0x3bc)][_0x39ebd3(0x203)]=function(){const _0x174ebd=_0x39ebd3,_0x17a5ee=this['_categoryWindow']['y']+this[_0x174ebd(0x120)][_0x174ebd(0x435)],_0x1c1c47=Graphics['boxWidth']-this[_0x174ebd(0x215)](),_0x36ff0a=this[_0x174ebd(0x100)]()-this[_0x174ebd(0x120)][_0x174ebd(0x435)],_0x55a5af=this[_0x174ebd(0x4ab)]()?Graphics['boxWidth']-_0x1c1c47:0x0;return new Rectangle(_0x55a5af,_0x17a5ee,_0x1c1c47,_0x36ff0a);},Scene_Shop[_0x39ebd3(0x3bc)]['postCreateSellWindowItemsEquipsCore']=function(){const _0x687ce=_0x39ebd3;this[_0x687ce(0x412)][_0x687ce(0x420)](this[_0x687ce(0x441)]);},Scene_Shop[_0x39ebd3(0x3bc)][_0x39ebd3(0x215)]=function(){const _0x482867=_0x39ebd3;return VisuMZ['ItemsEquipsCore'][_0x482867(0x237)][_0x482867(0x1a9)][_0x482867(0x3f6)];},VisuMZ['ItemsEquipsCore'][_0x39ebd3(0xf5)]=Scene_Shop[_0x39ebd3(0x3bc)]['activateSellWindow'],Scene_Shop['prototype']['activateSellWindow']=function(){const _0x1912a6=_0x39ebd3;VisuMZ[_0x1912a6(0x2a9)][_0x1912a6(0xf5)]['call'](this),this['isUseItemsEquipsCoreUpdatedLayout']()&&this[_0x1912a6(0x441)][_0x1912a6(0x2ae)](),this['_sellWindow']['updateHelp']();},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x38f)]=Scene_Shop[_0x39ebd3(0x3bc)]['commandBuy'],Scene_Shop[_0x39ebd3(0x3bc)][_0x39ebd3(0x243)]=function(){const _0x3a126a=_0x39ebd3;VisuMZ[_0x3a126a(0x2a9)][_0x3a126a(0x38f)][_0x3a126a(0x15c)](this),this['isUseItemsEquipsCoreUpdatedLayout']()&&this[_0x3a126a(0xec)]();},Scene_Shop['prototype'][_0x39ebd3(0xec)]=function(){const _0xbf41a9=_0x39ebd3;this['_buyWindowLastIndex']=this[_0xbf41a9(0x489)]||0x0,this[_0xbf41a9(0x3d1)]['smoothSelect'](this[_0xbf41a9(0x489)]);},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x3b8)]=Scene_Shop[_0x39ebd3(0x3bc)][_0x39ebd3(0x33a)],Scene_Shop['prototype'][_0x39ebd3(0x33a)]=function(){const _0x25d175=_0x39ebd3;VisuMZ[_0x25d175(0x2a9)][_0x25d175(0x3b8)][_0x25d175(0x15c)](this),this['isUseItemsEquipsCoreUpdatedLayout']()&&this[_0x25d175(0x336)](),this[_0x25d175(0x28c)]()&&(this[_0x25d175(0x120)][_0x25d175(0x436)](0x0),this['onCategoryOk']());},Scene_Shop[_0x39ebd3(0x3bc)]['commandSellItemsEquipsCore']=function(){const _0x8df13a=_0x39ebd3;this['_buyWindow'][_0x8df13a(0xf7)](),this[_0x8df13a(0x1cb)]['hide']();},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x338)]=Scene_Shop[_0x39ebd3(0x3bc)][_0x39ebd3(0xa5)],Scene_Shop[_0x39ebd3(0x3bc)][_0x39ebd3(0xa5)]=function(){const _0x194439=_0x39ebd3;VisuMZ[_0x194439(0x2a9)][_0x194439(0x338)][_0x194439(0x15c)](this),this[_0x194439(0x2b4)]()&&this['onBuyCancelItemsEquipsCore']();},Scene_Shop[_0x39ebd3(0x3bc)][_0x39ebd3(0x2ff)]=function(){const _0xb0ec2b=_0x39ebd3;this[_0xb0ec2b(0x489)]=this[_0xb0ec2b(0x3d1)]['index'](),this['_buyWindow']['show'](),this['_buyWindow'][_0xb0ec2b(0x213)](),this['_buyWindow']['smoothScrollTo'](0x0,0x0),this['_statusWindow'][_0xb0ec2b(0x2ae)](),this['_dummyWindow'][_0xb0ec2b(0xf7)]();},VisuMZ[_0x39ebd3(0x2a9)]['Scene_Shop_onCategoryCancel']=Scene_Shop[_0x39ebd3(0x3bc)][_0x39ebd3(0x9f)],Scene_Shop[_0x39ebd3(0x3bc)]['onCategoryCancel']=function(){const _0x13931e=_0x39ebd3;VisuMZ[_0x13931e(0x2a9)][_0x13931e(0x172)]['call'](this),this[_0x13931e(0x2b4)]()&&this[_0x13931e(0xe2)]();},Scene_Shop[_0x39ebd3(0x3bc)][_0x39ebd3(0xe2)]=function(){const _0x20c530=_0x39ebd3;this[_0x20c530(0x3d1)][_0x20c530(0x2ae)](),this[_0x20c530(0x1cb)][_0x20c530(0x2ae)]();},VisuMZ[_0x39ebd3(0x2a9)]['Scene_Shop_onBuyOk']=Scene_Shop[_0x39ebd3(0x3bc)][_0x39ebd3(0x2f6)],Scene_Shop['prototype'][_0x39ebd3(0x2f6)]=function(){const _0x4cae0b=_0x39ebd3;$gameTemp[_0x4cae0b(0x22f)]=!![],VisuMZ[_0x4cae0b(0x2a9)][_0x4cae0b(0x419)][_0x4cae0b(0x15c)](this),$gameTemp[_0x4cae0b(0x22f)]=![],this[_0x4cae0b(0x4ee)]=this[_0x4cae0b(0x3d1)]['item']();},VisuMZ[_0x39ebd3(0x2a9)]['Scene_Shop_buyingPrice']=Scene_Shop[_0x39ebd3(0x3bc)]['buyingPrice'],Scene_Shop[_0x39ebd3(0x3bc)][_0x39ebd3(0xce)]=function(){const _0x5e7909=_0x39ebd3;$gameTemp['_bypassProxy']=!![],this[_0x5e7909(0x4ee)]=this['_buyWindow'][_0x5e7909(0x27c)]();const _0x9431c4=VisuMZ[_0x5e7909(0x2a9)]['Scene_Shop_buyingPrice'][_0x5e7909(0x15c)](this);return $gameTemp['_bypassProxy']=![],this[_0x5e7909(0x4ee)]=this[_0x5e7909(0x3d1)]['item'](),_0x9431c4;},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x463)]=Scene_Shop['prototype'][_0x39ebd3(0x35d)],Scene_Shop[_0x39ebd3(0x3bc)][_0x39ebd3(0x35d)]=function(){const _0xa36683=_0x39ebd3;VisuMZ[_0xa36683(0x2a9)][_0xa36683(0x463)]['call'](this),this[_0xa36683(0x2b4)]()&&this[_0xa36683(0x4c6)]();},Scene_Shop['prototype']['onSellOkItemsEquipsCore']=function(){const _0x457eb1=_0x39ebd3;this['_categoryWindow'][_0x457eb1(0x2ae)]();},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x3df)]=Scene_Shop[_0x39ebd3(0x3bc)][_0x39ebd3(0x16a)],Scene_Shop[_0x39ebd3(0x3bc)]['onSellCancel']=function(){const _0x3ed61f=_0x39ebd3;VisuMZ[_0x3ed61f(0x2a9)]['Scene_Shop_onSellCancel'][_0x3ed61f(0x15c)](this),this[_0x3ed61f(0x28c)]()&&this[_0x3ed61f(0x9f)](),this[_0x3ed61f(0x2b4)]()&&this[_0x3ed61f(0x33e)][_0x3ed61f(0xf7)]();},Scene_Shop['prototype'][_0x39ebd3(0x470)]=function(_0x4275f2){const _0x18e18c=_0x39ebd3,_0x4a4555=this['_item'];this[_0x18e18c(0x4ee)]=_0x4275f2;const _0x3e9fd2=this[_0x18e18c(0x252)]();return this[_0x18e18c(0x4ee)]=_0x4a4555,_0x3e9fd2;},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x486)]=Scene_Shop[_0x39ebd3(0x3bc)][_0x39ebd3(0x252)],Scene_Shop[_0x39ebd3(0x3bc)][_0x39ebd3(0x252)]=function(){const _0x28e9ab=_0x39ebd3;let _0x15bbcf=this['determineBaseSellingPrice']();const _0x2d9289=this['_item'];return _0x15bbcf=VisuMZ['ItemsEquipsCore']['Settings'][_0x28e9ab(0x3a4)]['SellPriceJS'][_0x28e9ab(0x15c)](this,_0x2d9289,_0x15bbcf),_0x15bbcf;},Scene_Shop[_0x39ebd3(0x3bc)][_0x39ebd3(0x2b2)]=function(){const _0x2fbd88=_0x39ebd3;let _0x46b0da=this[_0x2fbd88(0x4ee)][_0x2fbd88(0x458)];if(!this[_0x2fbd88(0x4ee)])return 0x0;else{if(this[_0x2fbd88(0x4ee)][_0x2fbd88(0x3ba)][_0x2fbd88(0x145)](/<JS SELL PRICE>\s*([\s\S]*)\s*<\/JS SELL PRICE>/i)){const _0x40e06f=String(RegExp['$1']);window[_0x2fbd88(0x27c)]=this['_item'],window[_0x2fbd88(0x458)]=_0x46b0da*this['sellPriceRate']();try{eval(_0x40e06f);}catch(_0x1fd68d){if($gameTemp[_0x2fbd88(0x253)]())console[_0x2fbd88(0x4a0)](_0x1fd68d);}let _0x283700=window['price'];window[_0x2fbd88(0x27c)]=undefined,window[_0x2fbd88(0x458)]=undefined;if(isNaN(_0x283700))_0x283700=0x0;return Math[_0x2fbd88(0x349)](_0x283700);}else return this[_0x2fbd88(0x4ee)][_0x2fbd88(0x3ba)][_0x2fbd88(0x145)](/<SELL PRICE:[ ](\d+)>/i)?parseInt(RegExp['$1']):Math[_0x2fbd88(0x349)](this[_0x2fbd88(0x4e3)]());}},Scene_Shop['prototype'][_0x39ebd3(0x4e3)]=function(){const _0x13cbaf=_0x39ebd3;return this[_0x13cbaf(0x4ee)]['price']*this['sellPriceRate']();},Scene_Shop['prototype']['sellPriceRate']=function(){const _0x4d88a6=_0x39ebd3;return VisuMZ[_0x4d88a6(0x2a9)][_0x4d88a6(0x237)][_0x4d88a6(0x3a4)][_0x4d88a6(0x162)];},Scene_Shop[_0x39ebd3(0x3bc)][_0x39ebd3(0x3fa)]=function(){const _0x40e448=_0x39ebd3;if(!this['updatedLayoutStyle']())return![];if(!this[_0x40e448(0x28c)]())return![];if(!this[_0x40e448(0x412)])return![];if(!this[_0x40e448(0x412)][_0x40e448(0x274)])return![];return this[_0x40e448(0x444)]()&&this[_0x40e448(0x28c)]();},Scene_Shop[_0x39ebd3(0x3bc)][_0x39ebd3(0x3ef)]=function(){const _0x3c8442=_0x39ebd3;if(this[_0x3c8442(0x3fa)]())return this[_0x3c8442(0x412)]['maxCols']()===0x1?TextManager[_0x3c8442(0xa6)]('left','right'):TextManager['getInputMultiButtonStrings']('pageup',_0x3c8442(0x3e4));else{if(this[_0x3c8442(0x27d)]&&this['_numberWindow'][_0x3c8442(0x274)])return TextManager[_0x3c8442(0xa6)](_0x3c8442(0x135),_0x3c8442(0x3d7));}return Scene_MenuBase[_0x3c8442(0x3bc)][_0x3c8442(0x3ef)][_0x3c8442(0x15c)](this);},Scene_Shop[_0x39ebd3(0x3bc)][_0x39ebd3(0x4d7)]=function(){const _0x2e11b4=_0x39ebd3;if(this[_0x2e11b4(0x27d)]&&this[_0x2e11b4(0x27d)]['active'])return TextManager[_0x2e11b4(0xa6)]('up',_0x2e11b4(0x2d7));return Scene_MenuBase[_0x2e11b4(0x3bc)]['buttonAssistKey2'][_0x2e11b4(0x15c)](this);},Scene_Shop[_0x39ebd3(0x3bc)][_0x39ebd3(0x4e7)]=function(){const _0x1383bb=_0x39ebd3;if(this[_0x1383bb(0x3fa)]())return VisuMZ[_0x1383bb(0x2a9)][_0x1383bb(0x237)][_0x1383bb(0x3b4)][_0x1383bb(0xee)];else{if(this[_0x1383bb(0x27d)]&&this[_0x1383bb(0x27d)][_0x1383bb(0x274)])return VisuMZ[_0x1383bb(0x2a9)][_0x1383bb(0x237)][_0x1383bb(0x3a4)][_0x1383bb(0xf2)];}return Scene_MenuBase[_0x1383bb(0x3bc)]['buttonAssistText1'][_0x1383bb(0x15c)](this);},Scene_Shop[_0x39ebd3(0x3bc)][_0x39ebd3(0x325)]=function(){const _0x3060f2=_0x39ebd3;if(this[_0x3060f2(0x27d)]&&this[_0x3060f2(0x27d)]['active'])return VisuMZ['ItemsEquipsCore'][_0x3060f2(0x237)]['ShopScene']['buttonAssistLargeIncrement'];return Scene_MenuBase[_0x3060f2(0x3bc)]['buttonAssistText2'][_0x3060f2(0x15c)](this);},Scene_Shop[_0x39ebd3(0x3bc)]['resetShopSwitches']=function(){const _0x3d841d=_0x39ebd3;if(!SceneManager[_0x3d841d(0x13c)]())return;const _0x1252c0=VisuMZ[_0x3d841d(0x2a9)][_0x3d841d(0x237)]['ShopScene'];_0x1252c0[_0x3d841d(0x398)]&&$gameSwitches['setValue'](_0x1252c0[_0x3d841d(0x398)],![]),_0x1252c0['SwitchSell']&&$gameSwitches['setValue'](_0x1252c0['SwitchSell'],![]);},VisuMZ[_0x39ebd3(0x2a9)]['Scene_Shop_doBuy']=Scene_Shop[_0x39ebd3(0x3bc)][_0x39ebd3(0x19f)],Scene_Shop[_0x39ebd3(0x3bc)][_0x39ebd3(0x19f)]=function(_0x462b99){const _0x256b9c=_0x39ebd3;VisuMZ[_0x256b9c(0x2a9)][_0x256b9c(0x173)][_0x256b9c(0x15c)](this,_0x462b99),this[_0x256b9c(0x3db)](this['_item'],_0x462b99);if(_0x462b99<=0x0)return;const _0x18fd8b=VisuMZ['ItemsEquipsCore'][_0x256b9c(0x237)]['ShopScene'];_0x18fd8b[_0x256b9c(0x398)]&&$gameSwitches[_0x256b9c(0x1c6)](_0x18fd8b['SwitchBuy'],!![]),this[_0x256b9c(0x3d1)][_0x256b9c(0x39e)](),this[_0x256b9c(0x412)]['refresh']();},Scene_Shop[_0x39ebd3(0x3bc)]['onBuyItem']=function(_0x30d975,_0x274d15){const _0x9419c8=_0x39ebd3;this['processShopCondListingOnBuyItem'](_0x30d975,_0x274d15),$gameParty[_0x9419c8(0x343)](_0x30d975,_0x274d15),$gameParty[_0x9419c8(0x2f7)](_0x274d15*this[_0x9419c8(0xce)]());},Scene_Shop[_0x39ebd3(0x3bc)][_0x39ebd3(0x104)]=function(_0x145b9b,_0x47d90b){const _0x267cea=_0x39ebd3;if(!_0x145b9b)return;if(!_0x47d90b)return;const _0x39f2f3=VisuMZ[_0x267cea(0x2a9)]['ShopListingRegExp'],_0x1306ed=_0x145b9b[_0x267cea(0x3ba)]||'';if(_0x1306ed[_0x267cea(0x145)](_0x39f2f3['BuyTurnSwitchOn'])){const _0x44422=String(RegExp['$1'])[_0x267cea(0x3de)](',')[_0x267cea(0x1e3)](_0x58598b=>Number(_0x58598b));for(const _0x5276d9 of _0x44422){$gameSwitches[_0x267cea(0x1c6)](_0x5276d9,!![]);}}if(_0x1306ed[_0x267cea(0x145)](_0x39f2f3[_0x267cea(0xc5)])){const _0x50bc7c=String(RegExp['$1'])[_0x267cea(0x3de)](',')['map'](_0x351037=>Number(_0x351037));for(const _0x3a5e1d of _0x50bc7c){$gameSwitches[_0x267cea(0x1c6)](_0x3a5e1d,![]);}}},VisuMZ['ItemsEquipsCore'][_0x39ebd3(0x334)]=Scene_Shop['prototype'][_0x39ebd3(0x184)],Scene_Shop[_0x39ebd3(0x3bc)][_0x39ebd3(0x184)]=function(_0x3ba154){const _0x239ddf=_0x39ebd3;VisuMZ[_0x239ddf(0x2a9)]['Scene_Shop_doSell'][_0x239ddf(0x15c)](this,_0x3ba154),this[_0x239ddf(0x38a)](this[_0x239ddf(0x4ee)],_0x3ba154);if(_0x3ba154<=0x0)return;const _0x464fc3=VisuMZ[_0x239ddf(0x2a9)][_0x239ddf(0x237)][_0x239ddf(0x3a4)];_0x464fc3[_0x239ddf(0x398)]&&$gameSwitches[_0x239ddf(0x1c6)](_0x464fc3[_0x239ddf(0x2be)],!![]),this[_0x239ddf(0x3d1)][_0x239ddf(0x39e)](),this[_0x239ddf(0x412)][_0x239ddf(0x39e)]();},Scene_Shop[_0x39ebd3(0x3bc)]['onSellItem']=function(_0x4c5f18,_0x496a2a){const _0x2965ba=_0x39ebd3;this[_0x2965ba(0xc4)](_0x4c5f18,_0x496a2a),$gameParty[_0x2965ba(0x2c1)](_0x4c5f18,_0x496a2a),$gameParty[_0x2965ba(0x201)](_0x496a2a*this['sellingPrice']());},Scene_Shop[_0x39ebd3(0x3bc)][_0x39ebd3(0xc4)]=function(_0x44ad77,_0x351342){const _0x1117bf=_0x39ebd3;if(!_0x44ad77)return;if(!_0x351342)return;const _0x1b6a3a=VisuMZ[_0x1117bf(0x2a9)][_0x1117bf(0x254)],_0x54a0f3=_0x44ad77['note']||'';if(_0x54a0f3[_0x1117bf(0x145)](_0x1b6a3a[_0x1117bf(0xd2)])){const _0x78e4b4=String(RegExp['$1'])[_0x1117bf(0x3de)](',')[_0x1117bf(0x1e3)](_0x1161c0=>Number(_0x1161c0));for(const _0x202512 of _0x78e4b4){$gameSwitches[_0x1117bf(0x1c6)](_0x202512,!![]);}}if(_0x54a0f3[_0x1117bf(0x145)](_0x1b6a3a[_0x1117bf(0x3ca)])){const _0x47e392=String(RegExp['$1'])['split'](',')[_0x1117bf(0x1e3)](_0x5a8343=>Number(_0x5a8343));for(const _0x525a68 of _0x47e392){$gameSwitches[_0x1117bf(0x1c6)](_0x525a68,![]);}}};function Sprite_NewLabel(){this['initialize'](...arguments);}Sprite_NewLabel[_0x39ebd3(0x3bc)]=Object[_0x39ebd3(0x268)](Sprite[_0x39ebd3(0x3bc)]),Sprite_NewLabel['prototype'][_0x39ebd3(0x3f4)]=Sprite_NewLabel,Sprite_NewLabel['prototype'][_0x39ebd3(0x4a2)]=function(){const _0x467d99=_0x39ebd3;Sprite[_0x467d99(0x3bc)][_0x467d99(0x4a2)]['call'](this),this[_0x467d99(0x3ce)]();},Sprite_NewLabel['prototype'][_0x39ebd3(0x3ce)]=function(){const _0x2647a2=_0x39ebd3,_0x1ab7a2=0x20,_0x11e781=0x20;this[_0x2647a2(0x1dc)]=new Bitmap(_0x1ab7a2,_0x11e781),this[_0x2647a2(0x245)](),this[_0x2647a2(0x439)]();},Sprite_NewLabel[_0x39ebd3(0x3bc)][_0x39ebd3(0x245)]=function(){const _0x32c23a=_0x39ebd3,_0xbfe3b7=VisuMZ[_0x32c23a(0x2a9)][_0x32c23a(0x237)][_0x32c23a(0x12b)][_0x32c23a(0x373)];if(_0xbfe3b7<=0x0)return;const _0x7c6c8c=ImageManager[_0x32c23a(0x1d9)]('IconSet'),_0x14a86d=ImageManager['iconWidth'],_0xa7a0f=ImageManager[_0x32c23a(0x357)],_0x95c511=_0xbfe3b7%0x10*_0x14a86d,_0x56ca1b=Math['floor'](_0xbfe3b7/0x10)*_0xa7a0f;this['bitmap'][_0x32c23a(0x199)](_0x7c6c8c,_0x95c511,_0x56ca1b,_0x14a86d,_0xa7a0f,0x0,0x0);},Sprite_NewLabel[_0x39ebd3(0x3bc)][_0x39ebd3(0x439)]=function(){const _0x1aa94d=_0x39ebd3,_0x1297f3=VisuMZ[_0x1aa94d(0x2a9)]['Settings'][_0x1aa94d(0x12b)],_0x563b94=_0x1297f3[_0x1aa94d(0x2b6)];if(_0x563b94==='')return;const _0x410767=0x20,_0x1d2ee7=0x20;this[_0x1aa94d(0x1dc)]['fontFace']=_0x1297f3[_0x1aa94d(0x40d)]||$gameSystem[_0x1aa94d(0x13f)](),this['bitmap'][_0x1aa94d(0x379)]=this['getTextColor'](),this[_0x1aa94d(0x1dc)][_0x1aa94d(0x2e9)]=_0x1297f3[_0x1aa94d(0x1b8)],this[_0x1aa94d(0x1dc)][_0x1aa94d(0x1b9)](_0x563b94,0x0,_0x1d2ee7/0x2,_0x410767,_0x1d2ee7/0x2,_0x1aa94d(0x34f));},Sprite_NewLabel[_0x39ebd3(0x3bc)][_0x39ebd3(0x48b)]=function(){const _0x12d591=_0x39ebd3,_0x5ce8c=VisuMZ['ItemsEquipsCore'][_0x12d591(0x237)][_0x12d591(0x12b)][_0x12d591(0x347)];return _0x5ce8c[_0x12d591(0x145)](/#(.*)/i)?'#'+String(RegExp['$1']):ColorManager[_0x12d591(0x379)](_0x5ce8c);},Window_Base[_0x39ebd3(0x3bc)][_0x39ebd3(0xff)]=function(_0x4a016e,_0x30198d,_0xeacea2,_0x32049d){const _0x32687e=_0x39ebd3;if(_0x4a016e){const _0x22f85b=ImageManager[_0x32687e(0x4a1)]||0x20,_0x2f36c9=_0x22f85b-ImageManager[_0x32687e(0x225)],_0x252906=_0x22f85b+0x4,_0x1988a7=_0xeacea2+(this[_0x32687e(0x464)]()-ImageManager[_0x32687e(0x357)])/0x2,_0x3d6bad=Math[_0x32687e(0x165)](0x0,_0x32049d-_0x252906);this['changeTextColor'](ColorManager[_0x32687e(0x477)](_0x4a016e)),this[_0x32687e(0x4c9)](_0x4a016e[_0x32687e(0x229)],_0x30198d+Math[_0x32687e(0x3da)](_0x2f36c9/0x2),_0x1988a7),this['drawText'](_0x4a016e['name'],_0x30198d+_0x252906,_0xeacea2,_0x3d6bad),this[_0x32687e(0x1e5)]();}},Window_Base[_0x39ebd3(0x3bc)][_0x39ebd3(0x3c2)]=function(_0x11c7b5,_0x1c3591,_0x196673,_0x46553f){const _0x69ff4e=_0x39ebd3;if(this[_0x69ff4e(0x480)](_0x11c7b5)){this[_0x69ff4e(0x3c9)]();const _0x5bb8ca=VisuMZ[_0x69ff4e(0x2a9)][_0x69ff4e(0x237)][_0x69ff4e(0x3b4)],_0x2ae9ae=_0x5bb8ca[_0x69ff4e(0x378)],_0x509946=_0x2ae9ae[_0x69ff4e(0x1da)]($gameParty[_0x69ff4e(0x410)](_0x11c7b5));this[_0x69ff4e(0x456)][_0x69ff4e(0x2e9)]=_0x5bb8ca[_0x69ff4e(0x310)],this[_0x69ff4e(0x1b9)](_0x509946,_0x1c3591,_0x196673,_0x46553f,_0x69ff4e(0x3d7)),this[_0x69ff4e(0x3c9)]();}},Window_Base[_0x39ebd3(0x3bc)][_0x39ebd3(0x480)]=function(_0x16b9e6){const _0x277030=_0x39ebd3;if(DataManager[_0x277030(0x208)](_0x16b9e6))return $dataSystem[_0x277030(0x181)];return!![];},Window_Base['prototype'][_0x39ebd3(0x25c)]=function(_0x2226e3,_0x44e3c8,_0x354ab7,_0x19edbf,_0x30ad08){const _0x36465e=_0x39ebd3;_0x30ad08=Math[_0x36465e(0x165)](_0x30ad08||0x1,0x1);while(_0x30ad08--){_0x19edbf=_0x19edbf||this[_0x36465e(0x464)](),this['contentsBack'][_0x36465e(0x31a)]=0xa0;const _0x18c0d7=ColorManager[_0x36465e(0x16d)]();this[_0x36465e(0x1bb)][_0x36465e(0x400)](_0x2226e3+0x1,_0x44e3c8+0x1,_0x354ab7-0x2,_0x19edbf-0x2,_0x18c0d7),this[_0x36465e(0x1bb)][_0x36465e(0x31a)]=0xff;}},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x40a)]=Window_Selectable['prototype'][_0x39ebd3(0x4a2)],Window_Selectable[_0x39ebd3(0x3bc)][_0x39ebd3(0x4a2)]=function(_0x3f6201){const _0x38ed8f=_0x39ebd3;this[_0x38ed8f(0x128)](),VisuMZ[_0x38ed8f(0x2a9)][_0x38ed8f(0x40a)]['call'](this,_0x3f6201);},Window_Selectable[_0x39ebd3(0x3bc)][_0x39ebd3(0x128)]=function(){const _0x5317a5=_0x39ebd3;this[_0x5317a5(0x1af)]={},this[_0x5317a5(0x24e)]=0xff,this['_newLabelOpacityChange']=VisuMZ[_0x5317a5(0x2a9)][_0x5317a5(0x237)][_0x5317a5(0x12b)][_0x5317a5(0x493)],this[_0x5317a5(0x42d)]=VisuMZ[_0x5317a5(0x2a9)][_0x5317a5(0x237)][_0x5317a5(0x12b)][_0x5317a5(0x16b)];},Window_Selectable[_0x39ebd3(0x3bc)][_0x39ebd3(0x126)]=function(){return![];},VisuMZ['ItemsEquipsCore']['Window_Selectable_setHelpWindowItem']=Window_Selectable[_0x39ebd3(0x3bc)][_0x39ebd3(0x393)],Window_Selectable[_0x39ebd3(0x3bc)][_0x39ebd3(0x393)]=function(_0x239c4d){const _0x5dca27=_0x39ebd3;VisuMZ[_0x5dca27(0x2a9)][_0x5dca27(0x207)][_0x5dca27(0x15c)](this,_0x239c4d);if(this[_0x5dca27(0x126)]())this[_0x5dca27(0x1bd)](_0x239c4d);},Window_Selectable[_0x39ebd3(0x3bc)][_0x39ebd3(0x1bd)]=function(_0x11d00c){const _0x52a1e9=_0x39ebd3;if(!_0x11d00c)return;$gameParty[_0x52a1e9(0x308)](_0x11d00c);let _0x705d9a='';if(DataManager[_0x52a1e9(0xc2)](_0x11d00c))_0x705d9a=_0x52a1e9(0x32f)[_0x52a1e9(0x1da)](_0x11d00c['id']);else{if(DataManager[_0x52a1e9(0x2db)](_0x11d00c))_0x705d9a='weapon-%1'[_0x52a1e9(0x1da)](_0x11d00c['id']);else{if(DataManager[_0x52a1e9(0x13b)](_0x11d00c))_0x705d9a='armor-%1'[_0x52a1e9(0x1da)](_0x11d00c['id']);else return;}}const _0x4642a8=this[_0x52a1e9(0x1af)][_0x705d9a];if(_0x4642a8)_0x4642a8[_0x52a1e9(0xf7)]();},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x44c)]=Window_Selectable['prototype'][_0x39ebd3(0x39e)],Window_Selectable['prototype']['refresh']=function(){const _0x1addde=_0x39ebd3;this['hideNewLabelSprites'](),VisuMZ[_0x1addde(0x2a9)][_0x1addde(0x44c)][_0x1addde(0x15c)](this);},Window_Selectable[_0x39ebd3(0x3bc)]['hideNewLabelSprites']=function(){const _0x13d823=_0x39ebd3;for(const _0xc5e3b1 of Object[_0x13d823(0x39c)](this['_newLabelSprites'])){_0xc5e3b1[_0x13d823(0xf7)]();}},VisuMZ['ItemsEquipsCore'][_0x39ebd3(0x4ae)]=Window_Selectable['prototype'][_0x39ebd3(0x459)],Window_Selectable[_0x39ebd3(0x3bc)][_0x39ebd3(0x459)]=function(){const _0x14680e=_0x39ebd3;this['updateNewLabelOpacity'](),VisuMZ[_0x14680e(0x2a9)]['Window_Selectable_update'][_0x14680e(0x15c)](this);},Window_Selectable[_0x39ebd3(0x3bc)][_0x39ebd3(0x1a2)]=function(){const _0x44bf96=_0x39ebd3;if(!this['isShowNew']())return;const _0x18967b=this[_0x44bf96(0x42d)];this['_newLabelOpacity']+=this['_newLabelOpacityChange'];(this['_newLabelOpacity']>=_0x18967b||this[_0x44bf96(0x24e)]<=0x0)&&(this['_newLabelOpacityChange']*=-0x1);this[_0x44bf96(0x24e)]=this[_0x44bf96(0x24e)]['clamp'](0x0,_0x18967b);for(const _0x963a66 of Object[_0x44bf96(0x39c)](this[_0x44bf96(0x1af)])){_0x963a66['opacity']=this[_0x44bf96(0x24e)];}},Window_Selectable[_0x39ebd3(0x3bc)]['createNewLabelSprite']=function(_0x288915){const _0x11fa51=_0x39ebd3,_0x1f4f24=this[_0x11fa51(0x1af)];if(_0x1f4f24[_0x288915])return _0x1f4f24[_0x288915];else{const _0x1d0cf3=new Sprite_NewLabel();return _0x1f4f24[_0x288915]=_0x1d0cf3,this[_0x11fa51(0x10c)](_0x1d0cf3),_0x1d0cf3;}},Window_Selectable[_0x39ebd3(0x3bc)][_0x39ebd3(0x490)]=function(_0x422691,_0x4707bf,_0x11f588){const _0x1f8c72=_0x39ebd3;let _0x8cd971='';if(DataManager['isItem'](_0x422691))_0x8cd971=_0x1f8c72(0x32f)['format'](_0x422691['id']);else{if(DataManager[_0x1f8c72(0x2db)](_0x422691))_0x8cd971=_0x1f8c72(0x1ef)[_0x1f8c72(0x1da)](_0x422691['id']);else{if(DataManager[_0x1f8c72(0x13b)](_0x422691))_0x8cd971=_0x1f8c72(0x4ef)['format'](_0x422691['id']);else return;}}const _0x406031=this[_0x1f8c72(0x22b)](_0x8cd971);_0x406031[_0x1f8c72(0x2bf)](_0x4707bf,_0x11f588),_0x406031[_0x1f8c72(0x2ae)](),_0x406031['opacity']=this[_0x1f8c72(0x24e)];},Window_ItemCategory[_0x39ebd3(0x148)]=VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x237)]['Categories']['List'],Window_ItemCategory[_0x39ebd3(0x3a7)]=[_0x39ebd3(0x392),_0x39ebd3(0x2ce),_0x39ebd3(0x481),_0x39ebd3(0x23a),'AlwaysUsable',_0x39ebd3(0x204),_0x39ebd3(0x15f),_0x39ebd3(0x28e)],VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x348)]=Window_ItemCategory[_0x39ebd3(0x3bc)]['initialize'],Window_ItemCategory[_0x39ebd3(0x3bc)][_0x39ebd3(0x4a2)]=function(_0x10a96b){const _0x250b03=_0x39ebd3;VisuMZ[_0x250b03(0x2a9)][_0x250b03(0x348)][_0x250b03(0x15c)](this,_0x10a96b),this[_0x250b03(0x2cb)](_0x10a96b);},Window_ItemCategory['prototype']['createCategoryNameWindow']=function(_0x44b3cf){const _0x13c6bb=_0x39ebd3,_0x4a4ecf=new Rectangle(0x0,0x0,_0x44b3cf[_0x13c6bb(0x417)],_0x44b3cf[_0x13c6bb(0x435)]);this[_0x13c6bb(0x462)]=new Window_Base(_0x4a4ecf),this[_0x13c6bb(0x462)][_0x13c6bb(0x191)]=0x0,this[_0x13c6bb(0x4d3)](this['_categoryNameWindow']),this[_0x13c6bb(0x1d1)]();},Window_ItemCategory[_0x39ebd3(0x3bc)]['isUseModernControls']=function(){const _0x2a7060=_0x39ebd3,_0x27c65d=SceneManager[_0x2a7060(0x47d)];if(_0x27c65d['constructor']===Scene_Item&&_0x27c65d[_0x2a7060(0xd4)]())return![];return Imported[_0x2a7060(0x372)]&&Window_HorzCommand[_0x2a7060(0x3bc)][_0x2a7060(0x28c)][_0x2a7060(0x15c)](this);},Window_ItemCategory[_0x39ebd3(0x3bc)][_0x39ebd3(0x3d5)]=function(){},Window_ItemCategory[_0x39ebd3(0x3bc)][_0x39ebd3(0x156)]=function(){const _0x1f70cb=_0x39ebd3;if(!this[_0x1f70cb(0x28c)]())Window_HorzCommand[_0x1f70cb(0x3bc)]['playOkSound'][_0x1f70cb(0x15c)](this);},Window_ItemCategory[_0x39ebd3(0x3bc)][_0x39ebd3(0x340)]=function(){const _0x356829=_0x39ebd3;return this[_0x356829(0x2a5)]?this[_0x356829(0x28d)]():0x4;},Window_ItemCategory['prototype'][_0x39ebd3(0x459)]=function(){const _0xfd76f3=_0x39ebd3;Window_HorzCommand[_0xfd76f3(0x3bc)][_0xfd76f3(0x459)][_0xfd76f3(0x15c)](this),this[_0xfd76f3(0x286)]&&this[_0xfd76f3(0x286)][_0xfd76f3(0x2b7)](this[_0xfd76f3(0x359)]());},Window_ItemCategory['prototype']['processCursorMoveModernControls']=function(){const _0x23ebc4=_0x39ebd3;if(this['isCursorMovable']()){const _0x1dd536=this[_0x23ebc4(0x376)]();if(this[_0x23ebc4(0x286)]&&this[_0x23ebc4(0x286)]['maxCols']()<=0x1)Input[_0x23ebc4(0x4d9)]('right')&&this['cursorRight'](Input[_0x23ebc4(0x30a)]('right')),Input['isRepeated'](_0x23ebc4(0x135))&&this['cursorLeft'](Input[_0x23ebc4(0x30a)]('left'));else this[_0x23ebc4(0x286)]&&this[_0x23ebc4(0x286)][_0x23ebc4(0x340)]()>0x1&&(Input[_0x23ebc4(0x4d9)]('pagedown')&&!Input[_0x23ebc4(0x40e)](_0x23ebc4(0x195))&&this['cursorRight'](Input['isTriggered']('pagedown')),Input[_0x23ebc4(0x4d9)](_0x23ebc4(0xa7))&&!Input[_0x23ebc4(0x40e)](_0x23ebc4(0x195))&&this[_0x23ebc4(0x1d3)](Input['isTriggered'](_0x23ebc4(0xa7))));this['index']()!==_0x1dd536&&this['playCursorSound']();}},Window_ItemCategory['prototype'][_0x39ebd3(0x350)]=function(){const _0x8b111f=_0x39ebd3;if(this['isUseModernControls']())return;Window_HorzCommand[_0x8b111f(0x3bc)]['processHandling'][_0x8b111f(0x15c)](this);},Window_ItemCategory[_0x39ebd3(0x3bc)][_0x39ebd3(0x1d2)]=function(){const _0x1925f1=_0x39ebd3;return this[_0x1925f1(0x28c)]()?![]:Window_HorzCommand[_0x1925f1(0x3bc)][_0x1925f1(0x1d2)]['call'](this);},Window_ItemCategory['prototype'][_0x39ebd3(0x122)]=function(){const _0x3ff153=_0x39ebd3;if(this[_0x3ff153(0xac)]()){TouchInput[_0x3ff153(0x30a)]()&&this['onTouchSelect'](!![]);if(TouchInput['isClicked']())this['onTouchOk']();else TouchInput[_0x3ff153(0x20a)]()&&this['onTouchCancel']();}},Window_ItemCategory[_0x39ebd3(0x3bc)][_0x39ebd3(0xd6)]=function(_0x3d681e){const _0x3646a1=_0x39ebd3;this[_0x3646a1(0x28c)]()?this[_0x3646a1(0x14a)](!![]):Window_HorzCommand[_0x3646a1(0x3bc)][_0x3646a1(0xd6)][_0x3646a1(0x15c)](this,_0x3d681e);},Window_ItemCategory['prototype'][_0x39ebd3(0x14a)]=function(_0x232db7){const _0x900fd7=_0x39ebd3;this[_0x900fd7(0x121)]=![];if(this[_0x900fd7(0x22c)]()){const _0xf459d2=this[_0x900fd7(0x376)](),_0x14ea61=this['hitIndex']();_0x14ea61>=0x0&&_0x14ea61!==this[_0x900fd7(0x376)]()&&this['select'](_0x14ea61),_0x232db7&&this[_0x900fd7(0x376)]()!==_0xf459d2&&this['playCursorSound']();}},VisuMZ['ItemsEquipsCore'][_0x39ebd3(0x1e1)]=Window_ItemCategory['prototype'][_0x39ebd3(0x2e1)],Window_ItemCategory[_0x39ebd3(0x3bc)][_0x39ebd3(0x2e1)]=function(){const _0x1010b6=_0x39ebd3;this[_0x1010b6(0x2e5)](),this[_0x1010b6(0x3f8)](this[_0x1010b6(0x376)]());},Window_ItemCategory[_0x39ebd3(0x3bc)][_0x39ebd3(0x2e5)]=function(){const _0x5bfe4b=_0x39ebd3;let _0x379608=Window_ItemCategory[_0x5bfe4b(0x148)];const _0x56c16a=SceneManager['_scene'];_0x56c16a['constructor']===Scene_Item&&_0x56c16a[_0x5bfe4b(0xd4)]()&&(_0x379608=[{'Type':_0x5bfe4b(0x16f),'Icon':0x0,'SwitchID':0x0,'SortBy':'ID'},{'Type':_0x5bfe4b(0x41f),'Icon':0x0,'SwitchID':0x0,'SortBy':'ID'},{'Type':_0x5bfe4b(0x4ea),'Icon':0x0,'SwitchID':0x0,'SortBy':'ID'},{'Type':'KeyItems','Icon':0x0,'SwitchID':0x0,'SortBy':'ID'}]);for(const _0x1d83b7 of _0x379608){this[_0x5bfe4b(0x260)](_0x1d83b7);}},Window_ItemCategory[_0x39ebd3(0x3bc)]['addItemCategory']=function(_0x51ebfa){const _0x4d36b4=_0x39ebd3,_0x581b94=_0x51ebfa[_0x4d36b4(0x1ac)],_0x1fbb44=_0x51ebfa[_0x4d36b4(0x373)],_0x596ef2=_0x51ebfa[_0x4d36b4(0x1df)]||0x0;if(_0x596ef2>0x0&&!$gameSwitches[_0x4d36b4(0x1a5)](_0x596ef2))return;let _0xa187ae='',_0x3a18cd=_0x4d36b4(0x171),_0x5b96e9=_0x581b94;if(_0x581b94['match'](/Category:(.*)/i))_0xa187ae=String(RegExp['$1'])['trim']();else{if(Window_ItemCategory['categoryItemTypes'][_0x4d36b4(0x35f)](_0x581b94))_0xa187ae=VisuMZ[_0x4d36b4(0x2a9)][_0x4d36b4(0x237)][_0x4d36b4(0xed)][_0x581b94];else{if(['AllItems',_0x4d36b4(0x16f)][_0x4d36b4(0x35f)](_0x581b94))_0xa187ae=TextManager[_0x4d36b4(0x27c)];else{if(_0x581b94==='KeyItems')_0xa187ae=TextManager[_0x4d36b4(0x414)];else{if(_0x581b94==='AllWeapons')_0xa187ae=TextManager[_0x4d36b4(0xa4)];else{if(_0x581b94==='AllArmors')_0xa187ae=TextManager[_0x4d36b4(0x497)];else{if(_0x581b94[_0x4d36b4(0x145)](/WTYPE:(\d+)/i))_0xa187ae=$dataSystem[_0x4d36b4(0x40c)][Number(RegExp['$1'])]||'';else{if(_0x581b94[_0x4d36b4(0x145)](/ATYPE:(\d+)/i))_0xa187ae=$dataSystem[_0x4d36b4(0x16c)][Number(RegExp['$1'])]||'';else _0x581b94[_0x4d36b4(0x145)](/ETYPE:(\d+)/i)&&(_0xa187ae=$dataSystem[_0x4d36b4(0x1ce)][Number(RegExp['$1'])]||'');}}}}}}}if(TextManager[_0x4d36b4(0x197)]&&TextManager[_0x4d36b4(0xf4)]()){const _0x4f5623=_0xa187ae[_0x4d36b4(0x290)]()[_0x4d36b4(0x206)]();if($dataLocalization[_0x4f5623]&&_0x4f5623[_0x4d36b4(0x142)]>0x0){const _0xa2540=ConfigManager[_0x4d36b4(0xc3)]||_0x4d36b4(0x44e);_0xa187ae=$dataLocalization[_0x4f5623][_0xa2540]||_0x4d36b4(0xbe);}}_0x1fbb44>0x0&&this[_0x4d36b4(0x212)]()!=='text'&&(_0xa187ae=_0x4d36b4(0x316)[_0x4d36b4(0x1da)](_0x1fbb44,_0xa187ae)),this[_0x4d36b4(0x303)](_0xa187ae,_0x3a18cd,!![],_0x5b96e9);},Window_ItemCategory['prototype']['itemTextAlign']=function(){const _0x588ef4=_0x39ebd3;return VisuMZ[_0x588ef4(0x2a9)][_0x588ef4(0x237)][_0x588ef4(0xed)]['TextAlign'];},Window_ItemCategory[_0x39ebd3(0x3bc)]['drawItem']=function(_0x9365df){const _0x2f0627=_0x39ebd3,_0x8a1110=this['categoryStyleCheck'](_0x9365df);if(_0x8a1110===_0x2f0627(0x2ed))this[_0x2f0627(0xfb)](_0x9365df);else _0x8a1110===_0x2f0627(0x132)?this[_0x2f0627(0x3c5)](_0x9365df):Window_HorzCommand[_0x2f0627(0x3bc)][_0x2f0627(0x129)][_0x2f0627(0x15c)](this,_0x9365df);},Window_ItemCategory[_0x39ebd3(0x3bc)][_0x39ebd3(0x212)]=function(){const _0x2d23d3=_0x39ebd3,_0x550971=SceneManager[_0x2d23d3(0x47d)];if(_0x550971['constructor']===Scene_Item&&_0x550971['isPureVanillaMode']())return _0x2d23d3(0x30c);return VisuMZ[_0x2d23d3(0x2a9)][_0x2d23d3(0x237)][_0x2d23d3(0xed)][_0x2d23d3(0x407)];},Window_ItemCategory[_0x39ebd3(0x3bc)][_0x39ebd3(0x1c5)]=function(_0x3a9d0c){const _0x4504bf=_0x39ebd3;if(_0x3a9d0c<0x0)return _0x4504bf(0x30c);const _0x46676c=this[_0x4504bf(0x212)]();if(_0x46676c!==_0x4504bf(0x329))return _0x46676c;else{const _0x219cfe=this[_0x4504bf(0x40f)](_0x3a9d0c);if(_0x219cfe[_0x4504bf(0x145)](/\\I\[(\d+)\]/i)){const _0xa70af2=this[_0x4504bf(0x3d8)](_0x3a9d0c),_0x4e2624=this[_0x4504bf(0x448)](_0x219cfe)[_0x4504bf(0x417)];return _0x4e2624<=_0xa70af2[_0x4504bf(0x417)]?'iconText':'icon';}else return _0x4504bf(0x30c);}},Window_ItemCategory[_0x39ebd3(0x3bc)]['drawItemStyleIconText']=function(_0x212493){const _0x371f91=_0x39ebd3,_0xc02ef2=this[_0x371f91(0x3d8)](_0x212493),_0x316678=this[_0x371f91(0x40f)](_0x212493),_0x4979ea=this[_0x371f91(0x448)](_0x316678)['width'];this[_0x371f91(0x2e7)](this[_0x371f91(0x19b)](_0x212493));const _0x2bd55b=this[_0x371f91(0x47a)]();if(_0x2bd55b==='right')this[_0x371f91(0x451)](_0x316678,_0xc02ef2['x']+_0xc02ef2[_0x371f91(0x417)]-_0x4979ea,_0xc02ef2['y'],_0x4979ea);else{if(_0x2bd55b===_0x371f91(0x34f)){const _0x247949=_0xc02ef2['x']+Math[_0x371f91(0x349)]((_0xc02ef2['width']-_0x4979ea)/0x2);this[_0x371f91(0x451)](_0x316678,_0x247949,_0xc02ef2['y'],_0x4979ea);}else this[_0x371f91(0x451)](_0x316678,_0xc02ef2['x'],_0xc02ef2['y'],_0x4979ea);}},Window_ItemCategory[_0x39ebd3(0x3bc)][_0x39ebd3(0x3c5)]=function(_0x2d8e1c){const _0x5e6690=_0x39ebd3,_0x10f6a9=this[_0x5e6690(0x40f)](_0x2d8e1c);if(_0x10f6a9[_0x5e6690(0x145)](/\\I\[(\d+)\]/i)){const _0x32d037=Number(RegExp['$1'])||0x0,_0x4120bd=this[_0x5e6690(0x3d8)](_0x2d8e1c),_0x45f037=_0x4120bd['x']+Math[_0x5e6690(0x349)]((_0x4120bd['width']-ImageManager[_0x5e6690(0x225)])/0x2),_0x3bb46f=_0x4120bd['y']+(_0x4120bd[_0x5e6690(0x435)]-ImageManager[_0x5e6690(0x357)])/0x2;this[_0x5e6690(0x4c9)](_0x32d037,_0x45f037,_0x3bb46f);}},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0xcc)]=Window_ItemCategory[_0x39ebd3(0x3bc)][_0x39ebd3(0x4d1)],Window_ItemCategory[_0x39ebd3(0x3bc)][_0x39ebd3(0x4d1)]=function(_0x2113bf){const _0x5afde1=_0x39ebd3;VisuMZ['ItemsEquipsCore'][_0x5afde1(0xcc)][_0x5afde1(0x15c)](this,_0x2113bf),_0x2113bf[_0x5afde1(0x120)]=this;},Window_ItemCategory[_0x39ebd3(0x3bc)][_0x39ebd3(0x401)]=function(){const _0x4a9813=_0x39ebd3;Window_HorzCommand[_0x4a9813(0x3bc)][_0x4a9813(0x401)][_0x4a9813(0x15c)](this);if(this['_categoryNameWindow'])this[_0x4a9813(0x1d1)]();},Window_ItemCategory[_0x39ebd3(0x3bc)][_0x39ebd3(0x1d1)]=function(){const _0x58d1a5=_0x39ebd3,_0x17ed38=this[_0x58d1a5(0x462)];_0x17ed38[_0x58d1a5(0x456)][_0x58d1a5(0x39f)]();const _0x42d766=this[_0x58d1a5(0x1c5)](this[_0x58d1a5(0x376)]());if(_0x42d766==='icon'){const _0x26ed37=this[_0x58d1a5(0x3d8)](this[_0x58d1a5(0x376)]());let _0x81c964=this[_0x58d1a5(0x40f)](this['index']());_0x81c964=_0x81c964[_0x58d1a5(0x295)](/\\I\[(\d+)\]/gi,''),_0x17ed38['resetFontSettings'](),this[_0x58d1a5(0x328)](_0x81c964,_0x26ed37),this[_0x58d1a5(0x4da)](_0x81c964,_0x26ed37),this[_0x58d1a5(0x4eb)](_0x81c964,_0x26ed37);}},Window_ItemCategory['prototype'][_0x39ebd3(0x328)]=function(_0x14a850,_0x1366ef){},Window_ItemCategory['prototype']['categoryNameWindowDrawText']=function(_0x111b6d,_0x521e28){const _0x1b336e=_0x39ebd3,_0x450dc4=this['_categoryNameWindow'];_0x450dc4[_0x1b336e(0x1b9)](_0x111b6d,0x0,_0x521e28['y'],_0x450dc4['innerWidth'],_0x1b336e(0x34f));},Window_ItemCategory[_0x39ebd3(0x3bc)][_0x39ebd3(0x4eb)]=function(_0x305569,_0x23508e){const _0x4a18ca=_0x39ebd3,_0x3d3cdb=this[_0x4a18ca(0x462)],_0xffd1f1=$gameSystem[_0x4a18ca(0xe1)](),_0x11a9c4=_0x23508e['x']+Math[_0x4a18ca(0x349)](_0x23508e[_0x4a18ca(0x417)]/0x2)+_0xffd1f1;_0x3d3cdb['x']=_0x3d3cdb[_0x4a18ca(0x417)]/-0x2+_0x11a9c4,_0x3d3cdb['y']=Math[_0x4a18ca(0x349)](_0x23508e[_0x4a18ca(0x435)]/0x2);},Window_ItemList[_0x39ebd3(0x3bc)][_0x39ebd3(0x429)]=function(){const _0x13718b=_0x39ebd3;if(this[_0x13718b(0x22c)]()){const _0x383339=this[_0x13718b(0x376)]();if(this[_0x13718b(0x340)]()<=0x1)!this[_0x13718b(0x127)](_0x13718b(0x3e4))&&Input['isTriggered']('pagedown')&&this['cursorPagedown'](),!this['isHandled'](_0x13718b(0xa7))&&Input[_0x13718b(0x30a)]('pageup')&&this[_0x13718b(0x361)]();else this[_0x13718b(0x340)]()>0x1&&(Input[_0x13718b(0x4d9)](_0x13718b(0x3d7))&&this[_0x13718b(0x4f5)](Input[_0x13718b(0x30a)](_0x13718b(0x3d7))),Input[_0x13718b(0x4d9)](_0x13718b(0x135))&&this[_0x13718b(0x1d3)](Input['isTriggered'](_0x13718b(0x135))),this[_0x13718b(0x43b)]()?(Input[_0x13718b(0x30a)](_0x13718b(0x3e4))&&Input[_0x13718b(0x40e)]('shift')&&this[_0x13718b(0x390)](),Input[_0x13718b(0x30a)](_0x13718b(0xa7))&&Input[_0x13718b(0x40e)]('shift')&&this[_0x13718b(0x361)]()):(Input[_0x13718b(0x30a)](_0x13718b(0x3e4))&&this[_0x13718b(0x390)](),Input['isTriggered']('pageup')&&this['cursorPageup']()));Input[_0x13718b(0x4d9)](_0x13718b(0x2d7))&&(Input[_0x13718b(0x40e)](_0x13718b(0x195))&&this['allowShiftScrolling']()?this[_0x13718b(0x390)]():this[_0x13718b(0x2b3)](Input[_0x13718b(0x30a)](_0x13718b(0x2d7)))),Input[_0x13718b(0x4d9)]('up')&&(Input[_0x13718b(0x40e)]('shift')&&this[_0x13718b(0x35a)]()?this[_0x13718b(0x361)]():this[_0x13718b(0x152)](Input[_0x13718b(0x30a)]('up'))),Imported[_0x13718b(0x372)]&&this[_0x13718b(0x3d5)](),this[_0x13718b(0x376)]()!==_0x383339&&this[_0x13718b(0x48d)]();}},Window_ItemList[_0x39ebd3(0x3bc)][_0x39ebd3(0x43b)]=function(){const _0xf14845=_0x39ebd3,_0x4fb4fb=SceneManager[_0xf14845(0x47d)],_0x40a180=[Scene_Item,Scene_Shop];return _0x40a180[_0xf14845(0x35f)](_0x4fb4fb['constructor']);},Window_ItemList[_0x39ebd3(0x3bc)][_0x39ebd3(0x3a9)]=function(){const _0x4619dc=_0x39ebd3;Window_Selectable[_0x4619dc(0x3bc)][_0x4619dc(0x3a9)]['call'](this),this[_0x4619dc(0x120)]&&this[_0x4619dc(0x120)]['isUseModernControls']()&&this[_0x4619dc(0x120)][_0x4619dc(0x3a9)]();},Window_ItemList[_0x39ebd3(0x3bc)][_0x39ebd3(0x3d9)]=function(){const _0x1d6727=_0x39ebd3;Window_Selectable[_0x1d6727(0x3bc)][_0x1d6727(0x3d9)]['call'](this),this[_0x1d6727(0x120)]&&this['_categoryWindow'][_0x1d6727(0x28c)]()&&this['_categoryWindow']['deactivate']();},Window_ItemList[_0x39ebd3(0x3bc)][_0x39ebd3(0x2b7)]=function(_0x55d52b){const _0x42830e=_0x39ebd3;this[_0x42830e(0x45d)]!==_0x55d52b&&(this[_0x42830e(0x45d)]=_0x55d52b,this[_0x42830e(0x39e)](),this[_0x42830e(0x120)]&&this[_0x42830e(0x120)][_0x42830e(0x28c)]()?this[_0x42830e(0x436)](0x0):this[_0x42830e(0x280)](0x0,0x0));},VisuMZ['ItemsEquipsCore'][_0x39ebd3(0x1e4)]=Window_ItemList[_0x39ebd3(0x3bc)][_0x39ebd3(0x340)],Window_ItemList[_0x39ebd3(0x3bc)][_0x39ebd3(0x340)]=function(){const _0xc0a38=_0x39ebd3,_0x31fbb4=SceneManager[_0xc0a38(0x47d)];if(_0x31fbb4[_0xc0a38(0x3f4)]===Scene_Item&&_0x31fbb4[_0xc0a38(0xd4)]())return 0x2;if(SceneManager[_0xc0a38(0x47d)][_0xc0a38(0x3f4)]===Scene_Battle)return VisuMZ[_0xc0a38(0x2a9)]['Window_ItemList_maxCols'][_0xc0a38(0x15c)](this);else return SceneManager[_0xc0a38(0x47d)]['constructor']===Scene_Map?VisuMZ[_0xc0a38(0x2a9)][_0xc0a38(0x1e4)][_0xc0a38(0x15c)](this):VisuMZ[_0xc0a38(0x2a9)]['Settings'][_0xc0a38(0x3b4)][_0xc0a38(0x44f)];},VisuMZ['ItemsEquipsCore'][_0x39ebd3(0x333)]=Window_ItemList['prototype'][_0x39ebd3(0x111)],Window_ItemList['prototype'][_0x39ebd3(0x111)]=function(){const _0xa343f2=_0x39ebd3;return this[_0xa343f2(0x340)]()<=0x1?Window_Selectable['prototype'][_0xa343f2(0x111)]['call'](this):VisuMZ['ItemsEquipsCore']['Window_ItemList_colSpacing']['call'](this);},Window_ItemList[_0x39ebd3(0x3bc)]['includes']=function(_0x121fc4){const _0x2724c3=_0x39ebd3;switch(this['_category']){case _0x2724c3(0x246):return DataManager['isItem'](_0x121fc4);case _0x2724c3(0x16f):return DataManager[_0x2724c3(0xc2)](_0x121fc4)&&_0x121fc4[_0x2724c3(0x2d9)]===0x1;case'KeyItems':return DataManager[_0x2724c3(0xc2)](_0x121fc4)&&_0x121fc4['itypeId']===0x2;case'HiddenItemA':return DataManager[_0x2724c3(0xc2)](_0x121fc4)&&_0x121fc4[_0x2724c3(0x2d9)]===0x3;case'HiddenItemB':return DataManager[_0x2724c3(0xc2)](_0x121fc4)&&_0x121fc4['itypeId']===0x4;case _0x2724c3(0x23a):return DataManager[_0x2724c3(0xc2)](_0x121fc4)&&_0x121fc4[_0x2724c3(0x351)];case _0x2724c3(0x481):return DataManager[_0x2724c3(0xc2)](_0x121fc4)&&!_0x121fc4['consumable'];case'AlwaysUsable':return DataManager[_0x2724c3(0xc2)](_0x121fc4)&&[0x0][_0x2724c3(0x35f)](_0x121fc4[_0x2724c3(0x2fe)]);case'BattleUsable':return DataManager[_0x2724c3(0xc2)](_0x121fc4)&&[0x0,0x1]['includes'](_0x121fc4['occasion']);case'FieldUsable':return DataManager[_0x2724c3(0xc2)](_0x121fc4)&&[0x0,0x2]['includes'](_0x121fc4['occasion']);case _0x2724c3(0x28e):return DataManager[_0x2724c3(0xc2)](_0x121fc4)&&[0x3][_0x2724c3(0x35f)](_0x121fc4[_0x2724c3(0x2fe)]);case _0x2724c3(0x41f):return DataManager[_0x2724c3(0x2db)](_0x121fc4);case _0x2724c3(0x4ea):return DataManager[_0x2724c3(0x13b)](_0x121fc4);default:if(this[_0x2724c3(0x45d)][_0x2724c3(0x145)](/WTYPE:(\d+)/i))return DataManager[_0x2724c3(0x2db)](_0x121fc4)&&_0x121fc4[_0x2724c3(0x1d7)]===Number(RegExp['$1']);else{if(this[_0x2724c3(0x45d)][_0x2724c3(0x145)](/WTYPE:(.*)/i)){const _0xb1735=$dataSystem['weaponTypes'][_0x2724c3(0x180)](String(RegExp['$1'])[_0x2724c3(0x206)]());return DataManager['isWeapon'](_0x121fc4)&&_0x121fc4[_0x2724c3(0x1d7)]===_0xb1735;}else{if(this[_0x2724c3(0x45d)][_0x2724c3(0x145)](/ATYPE:(\d+)/i))return DataManager[_0x2724c3(0x13b)](_0x121fc4)&&_0x121fc4[_0x2724c3(0x31e)]===Number(RegExp['$1']);else{if(this['_category']['match'](/ATYPE:(.*)/i)){const _0x37b9ce=$dataSystem[_0x2724c3(0x16c)][_0x2724c3(0x180)](String(RegExp['$1'])[_0x2724c3(0x206)]());return DataManager[_0x2724c3(0x13b)](_0x121fc4)&&_0x121fc4['atypeId']===_0x37b9ce;}else{if(this[_0x2724c3(0x45d)][_0x2724c3(0x145)](/ETYPE:(\d+)/i))return!!_0x121fc4&&_0x121fc4[_0x2724c3(0x34a)]===Number(RegExp['$1']);else{if(this[_0x2724c3(0x45d)]['match'](/ETYPE:(.*)/i)){const _0x4637e3=$dataSystem[_0x2724c3(0x1ce)][_0x2724c3(0x180)](String(RegExp['$1'])[_0x2724c3(0x206)]());return DataManager[_0x2724c3(0x13b)](_0x121fc4)&&_0x121fc4['etypeId']===_0x4637e3;}else{if(this[_0x2724c3(0x45d)][_0x2724c3(0x145)](/Category:(.*)/i))return!!_0x121fc4&&_0x121fc4[_0x2724c3(0x335)][_0x2724c3(0x35f)](String(RegExp['$1'])[_0x2724c3(0x34e)]()['trim']());}}}}}}}return![];},VisuMZ['ItemsEquipsCore'][_0x39ebd3(0x3aa)]=Window_ItemList[_0x39ebd3(0x3bc)][_0x39ebd3(0x134)],Window_ItemList[_0x39ebd3(0x3bc)][_0x39ebd3(0x134)]=function(){const _0x45053a=_0x39ebd3;VisuMZ[_0x45053a(0x2a9)]['Window_ItemList_makeItemList'][_0x45053a(0x15c)](this);if(this[_0x45053a(0x45f)]())this['sortListItemScene']();},Window_ItemList[_0x39ebd3(0x3bc)][_0x39ebd3(0x45f)]=function(){const _0x2ffd55=_0x39ebd3,_0x4b6a4b=['Scene_Battle','Scene_Item',_0x2ffd55(0x1b0),_0x2ffd55(0x341)],_0x1adcff=SceneManager[_0x2ffd55(0x47d)];return _0x4b6a4b[_0x2ffd55(0x35f)](_0x1adcff[_0x2ffd55(0x3f4)][_0x2ffd55(0x292)]);},Window_ItemList['prototype'][_0x39ebd3(0x395)]=function(){const _0x1c7e49=_0x39ebd3,_0x446482=Window_ItemCategory['categoryList'],_0x5e651c=_0x446482[_0x1c7e49(0x317)](_0x19982f=>_0x19982f['Type']===this['_category']);if(!_0x5e651c){VisuMZ[_0x1c7e49(0x2a9)][_0x1c7e49(0x473)](this['_data']);return;}const _0x411384=((_0x5e651c[_0x1c7e49(0x23f)]??'ID')||'ID')[_0x1c7e49(0x34e)]()[_0x1c7e49(0x206)]();_0x411384===_0x1c7e49(0x49e)?this[_0x1c7e49(0x190)][_0x1c7e49(0x2e4)]((_0x7b5fc7,_0x56e6e4)=>{const _0x18d8a7=_0x1c7e49;if(!!_0x7b5fc7&&!!_0x56e6e4)return _0x7b5fc7[_0x18d8a7(0x292)][_0x18d8a7(0x3dd)](_0x56e6e4[_0x18d8a7(0x292)]);return 0x0;}):VisuMZ[_0x1c7e49(0x2a9)]['SortByIDandPriority'](this['_data']);},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x473)]=function(_0x20345b){const _0x47a568=_0x39ebd3;return _0x20345b[_0x47a568(0x2e4)]((_0x53de1d,_0x4ce2de)=>{const _0x45bb22=_0x47a568;if(!!_0x53de1d&&!!_0x4ce2de){if(_0x53de1d[_0x45bb22(0x330)]===undefined)VisuMZ['ItemsEquipsCore']['Parse_Notetags_Sorting'](_0x53de1d);if(_0x4ce2de[_0x45bb22(0x330)]===undefined)VisuMZ[_0x45bb22(0x2a9)][_0x45bb22(0xf9)](_0x4ce2de);const _0x8a39aa=_0x53de1d[_0x45bb22(0x330)],_0x38c884=_0x4ce2de['sortPriority'];if(_0x8a39aa!==_0x38c884)return _0x38c884-_0x8a39aa;return _0x53de1d['id']-_0x4ce2de['id'];}return 0x0;}),_0x20345b;},Window_ItemList[_0x39ebd3(0x3bc)][_0x39ebd3(0x126)]=function(){return!![];},VisuMZ['ItemsEquipsCore'][_0x39ebd3(0x1b4)]=Window_ItemList[_0x39ebd3(0x3bc)][_0x39ebd3(0x129)],Window_ItemList[_0x39ebd3(0x3bc)][_0x39ebd3(0x129)]=function(_0x1541d7){const _0x2a1814=_0x39ebd3;VisuMZ[_0x2a1814(0x2a9)][_0x2a1814(0x1b4)]['call'](this,_0x1541d7),this[_0x2a1814(0x9d)](_0x1541d7);},Window_ItemList[_0x39ebd3(0x3bc)][_0x39ebd3(0x3c2)]=function(_0x11b232,_0x22e1d3,_0x568d0c,_0x5a489b){const _0x25673a=_0x39ebd3;Window_Selectable[_0x25673a(0x3bc)][_0x25673a(0x3c2)][_0x25673a(0x15c)](this,_0x11b232,_0x22e1d3,_0x568d0c,_0x5a489b);},Window_ItemList[_0x39ebd3(0x3bc)]['placeItemNewLabel']=function(_0x3d0727){const _0x5603e5=_0x39ebd3,_0x1724b5=this[_0x5603e5(0x4af)](_0x3d0727);if(!_0x1724b5||!this[_0x5603e5(0x126)]())return;if(!$gameParty[_0x5603e5(0x140)](_0x1724b5))return;const _0x368a68=this['itemLineRect'](_0x3d0727),_0x83b537=_0x368a68['x'],_0x31bdb5=_0x368a68['y']+(this[_0x5603e5(0x464)]()-0x20)/0x2,_0x4a2b49=VisuMZ['ItemsEquipsCore']['Settings'][_0x5603e5(0x12b)][_0x5603e5(0x209)],_0x139151=VisuMZ[_0x5603e5(0x2a9)][_0x5603e5(0x237)][_0x5603e5(0x12b)][_0x5603e5(0x12a)];this[_0x5603e5(0x490)](_0x1724b5,_0x83b537+_0x4a2b49,_0x31bdb5+_0x139151);},Window_ItemList[_0x39ebd3(0x3bc)]['setStatusWindow']=function(_0x4f571d){const _0x34bc7d=_0x39ebd3;this[_0x34bc7d(0x441)]=_0x4f571d,this[_0x34bc7d(0x401)]();},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x495)]=Window_ItemList[_0x39ebd3(0x3bc)][_0x39ebd3(0x19e)],Window_ItemList[_0x39ebd3(0x3bc)][_0x39ebd3(0x19e)]=function(){const _0x399aea=_0x39ebd3;VisuMZ[_0x399aea(0x2a9)][_0x399aea(0x495)][_0x399aea(0x15c)](this),this[_0x399aea(0x441)]&&this['_statusWindow']['constructor']===Window_ShopStatus&&this[_0x399aea(0x441)][_0x399aea(0x313)](this[_0x399aea(0x27c)]());},Window_BattleItem[_0x39ebd3(0x3bc)][_0x39ebd3(0x4dd)]=function(_0x4570c1){const _0x1876c6=_0x39ebd3;return BattleManager[_0x1876c6(0x1f8)]()?BattleManager[_0x1876c6(0x1f8)]()[_0x1876c6(0x3ae)](_0x4570c1):Window_ItemList[_0x1876c6(0x3bc)][_0x1876c6(0x4dd)]['call'](this,_0x4570c1);},Window_EventItem[_0x39ebd3(0x3bc)]['isShowNew']=function(){return![];},Window_EquipStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x2b4)]=function(){const _0x1e0d65=_0x39ebd3;return VisuMZ[_0x1e0d65(0x2a9)][_0x1e0d65(0x237)][_0x1e0d65(0x322)][_0x1e0d65(0x306)];},VisuMZ[_0x39ebd3(0x2a9)]['Window_EquipStatus_refresh']=Window_EquipStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x39e)],Window_EquipStatus['prototype']['refresh']=function(){const _0x28d5d8=_0x39ebd3;this[_0x28d5d8(0xa0)](),this[_0x28d5d8(0x3c9)]();if(this[_0x28d5d8(0x4b4)])this[_0x28d5d8(0x4b4)][_0x28d5d8(0x39e)]();this['isUseItemsEquipsCoreUpdatedLayout']()?this[_0x28d5d8(0x18d)]():VisuMZ['ItemsEquipsCore']['Window_EquipStatus_refresh']['call'](this);},Window_EquipStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x18d)]=function(){const _0x1342aa=_0x39ebd3;this[_0x1342aa(0x456)][_0x1342aa(0x39f)]();if(!this[_0x1342aa(0x4b4)])return;if(this[_0x1342aa(0x27e)]()){const _0x2e8805=ImageManager[_0x1342aa(0x446)](this[_0x1342aa(0x4b4)][_0x1342aa(0x391)]());_0x2e8805[_0x1342aa(0x178)](this[_0x1342aa(0x1cf)]['bind'](this));}else this[_0x1342aa(0x221)]();},Window_EquipStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x27e)]=function(){const _0x58caff=_0x39ebd3;return Imported[_0x58caff(0x198)]&&this['_actor'][_0x58caff(0x391)]()!==''&&VisuMZ[_0x58caff(0x2a9)][_0x58caff(0x237)][_0x58caff(0x322)]['MenuPortraits'];},Window_EquipStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x1cf)]=function(){const _0xb7a332=_0x39ebd3;VisuMZ['ItemsEquipsCore'][_0xb7a332(0x237)]['EquipScene'][_0xb7a332(0x164)]['call'](this),this['drawParamsItemsEquipsCore']();},Window_EquipStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x221)]=function(){const _0x431ca6=_0x39ebd3;VisuMZ[_0x431ca6(0x2a9)]['Settings'][_0x431ca6(0x322)]['DrawFaceJS'][_0x431ca6(0x15c)](this),this['drawParamsItemsEquipsCore']();},Window_EquipStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x484)]=function(){const _0xc46de6=_0x39ebd3;this['resetFontSettings'](),VisuMZ[_0xc46de6(0x2a9)][_0xc46de6(0x237)][_0xc46de6(0x322)][_0xc46de6(0x2cc)]['call'](this);},Window_EquipStatus['prototype'][_0x39ebd3(0x102)]=function(_0x43e297,_0x7000fa,_0x3e8c39,_0x2973be,_0x2dc82e){const _0x457d24=_0x39ebd3,_0x319704=ImageManager[_0x457d24(0x446)](_0x43e297[_0x457d24(0x391)]()),_0x4e7312=this[_0x457d24(0x35e)]-_0x319704['width'];_0x7000fa+=_0x4e7312/0x2;if(_0x4e7312<0x0)_0x2973be-=_0x4e7312;Window_StatusBase['prototype'][_0x457d24(0x102)]['call'](this,_0x43e297,_0x7000fa,_0x3e8c39,_0x2973be,_0x2dc82e);},Window_EquipStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x39d)]=function(){const _0x1bab8b=_0x39ebd3;return Imported[_0x1bab8b(0x372)]?VisuMZ[_0x1bab8b(0x1e7)]['Settings'][_0x1bab8b(0x42e)][_0x1bab8b(0x2a8)]:[0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7];},Window_EquipStatus['prototype'][_0x39ebd3(0x432)]=function(){const _0x1b70c8=_0x39ebd3;return VisuMZ[_0x1b70c8(0x2a9)][_0x1b70c8(0x237)]['EquipScene']['ParamValueFontSize'];},Window_EquipStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x416)]=function(){const _0x4a0879=_0x39ebd3;return Imported['VisuMZ_0_CoreEngine']&&VisuMZ[_0x4a0879(0x1e7)][_0x4a0879(0x237)]['Param'][_0x4a0879(0x234)];},Window_EquipStatus['prototype'][_0x39ebd3(0x283)]=function(_0x3f2cb0,_0x425188,_0x477c10,_0x17fae1){const _0x5c9bac=_0x39ebd3,_0x2f6667=this[_0x5c9bac(0x447)]();Imported['VisuMZ_0_CoreEngine']?this[_0x5c9bac(0xf8)](_0x425188+_0x2f6667,_0x477c10,_0x17fae1,_0x3f2cb0,![]):(this[_0x5c9bac(0x4ca)](ColorManager[_0x5c9bac(0x163)]()),this[_0x5c9bac(0x1b9)](TextManager['param'](_0x3f2cb0),_0x425188+_0x2f6667,_0x477c10,_0x17fae1));},Window_EquipStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x3f3)]=function(_0x56f7c4,_0xd1481e,_0x35df28,_0x24cbf2){const _0x517ac5=_0x39ebd3,_0x511390=this['itemPadding']();let _0x180a1a=0x0;Imported[_0x517ac5(0x372)]?_0x180a1a=this[_0x517ac5(0x4b4)]['paramValueByName'](_0x56f7c4,!![]):_0x180a1a=this[_0x517ac5(0x4b4)]['param'](_0x56f7c4);const _0x2e70fd=_0x180a1a;this[_0x517ac5(0x1b9)](_0x180a1a,_0xd1481e,_0x35df28,_0x24cbf2-_0x511390,_0x517ac5(0x3d7));},Window_EquipStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x382)]=function(_0x33bd99,_0x2e6482,_0x1c61c7,_0xac0656){const _0x50449c=_0x39ebd3,_0x2e191c=this[_0x50449c(0x447)]();let _0x108fb0=0x0,_0x2984ba=0x0,_0xb4edb6='';if(this['_tempActor']){Imported[_0x50449c(0x372)]?(_0x108fb0=this[_0x50449c(0x4b4)][_0x50449c(0x192)](_0x33bd99,![]),_0x2984ba=this[_0x50449c(0x119)]['paramValueByName'](_0x33bd99,![]),_0xb4edb6=this[_0x50449c(0x119)][_0x50449c(0x192)](_0x33bd99,!![])):(_0x108fb0=this[_0x50449c(0x4b4)][_0x50449c(0x154)](_0x33bd99),_0x2984ba=this[_0x50449c(0x119)][_0x50449c(0x154)](_0x33bd99),_0xb4edb6=this[_0x50449c(0x119)][_0x50449c(0x154)](_0x33bd99));const _0x11b0c7=_0x108fb0,_0x2dddb1=_0x2984ba;diffValue=_0x2dddb1-_0x11b0c7,this[_0x50449c(0x4ca)](ColorManager[_0x50449c(0x4bc)](diffValue)),this[_0x50449c(0x1b9)](_0xb4edb6,_0x2e6482,_0x1c61c7,_0xac0656-_0x2e191c,'right');}},Window_EquipStatus[_0x39ebd3(0x3bc)]['drawUpdatedParamValueDiff']=function(_0x546dad,_0x589290,_0x1c7498,_0x49067d){const _0x494429=_0x39ebd3,_0x226ecb=this[_0x494429(0x447)]();let _0x15ba3d=0x0,_0x542678=0x0,_0x29ca3a=![];if(this[_0x494429(0x119)]){Imported[_0x494429(0x372)]?(_0x15ba3d=this[_0x494429(0x4b4)][_0x494429(0x192)](_0x546dad,![]),_0x542678=this['_tempActor'][_0x494429(0x192)](_0x546dad,![]),_0x29ca3a=String(this[_0x494429(0x4b4)]['paramValueByName'](_0x546dad,!![]))['match'](/([%％])/i)):(_0x15ba3d=this[_0x494429(0x4b4)][_0x494429(0x154)](_0x546dad),_0x542678=this[_0x494429(0x119)]['param'](_0x546dad),_0x29ca3a=_0x15ba3d%0x1!==0x0||_0x542678%0x1!==0x0);const _0x3082d8=_0x15ba3d,_0x2fb015=_0x542678,_0x3b26b9=_0x2fb015-_0x3082d8;let _0x278ff8=_0x3b26b9;if(_0x29ca3a)_0x278ff8=Math[_0x494429(0x2c3)](_0x3b26b9*0x64)+'%';_0x3b26b9!==0x0&&(this['changeTextColor'](ColorManager[_0x494429(0x4bc)](_0x3b26b9)),_0x278ff8=(_0x3b26b9>0x0?_0x494429(0xea):_0x494429(0xd0))[_0x494429(0x1da)](_0x278ff8),this[_0x494429(0x1b9)](_0x278ff8,_0x589290+_0x226ecb,_0x1c7498,_0x49067d,_0x494429(0x135)));}},Window_EquipStatus['prototype'][_0x39ebd3(0x25c)]=function(_0x4d7aef,_0x116149,_0x1f0f42,_0x3c719b,_0xd786de){const _0x2549f3=_0x39ebd3;if(VisuMZ[_0x2549f3(0x2a9)]['Settings']['EquipScene'][_0x2549f3(0x216)]===![])return;_0xd786de=Math[_0x2549f3(0x165)](_0xd786de||0x1,0x1);while(_0xd786de--){_0x3c719b=_0x3c719b||this[_0x2549f3(0x464)](),this[_0x2549f3(0x456)][_0x2549f3(0x31a)]=0xa0;const _0x5a29a4=ColorManager['getItemsEquipsCoreBackColor2']();this[_0x2549f3(0x456)][_0x2549f3(0x400)](_0x4d7aef+0x1,_0x116149+0x1,_0x1f0f42-0x2,_0x3c719b-0x2,_0x5a29a4),this[_0x2549f3(0x456)]['paintOpacity']=0xff;}},ColorManager[_0x39ebd3(0x42b)]=function(){const _0x567f8b=_0x39ebd3,_0x36d0ec=VisuMZ[_0x567f8b(0x2a9)][_0x567f8b(0x237)]['EquipScene'];let _0x348030=_0x36d0ec[_0x567f8b(0x231)]!==undefined?_0x36d0ec['BackRectColor']:0x13;return ColorManager[_0x567f8b(0x47c)](_0x348030);},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x27a)]=Window_EquipCommand['prototype'][_0x39ebd3(0x4a2)],Window_EquipCommand[_0x39ebd3(0x3bc)][_0x39ebd3(0x4a2)]=function(_0x5e7d57){const _0x4298b8=_0x39ebd3;VisuMZ[_0x4298b8(0x2a9)][_0x4298b8(0x27a)]['call'](this,_0x5e7d57),this[_0x4298b8(0x118)](_0x5e7d57);},Window_EquipCommand[_0x39ebd3(0x3bc)]['createCommandNameWindow']=function(_0x2b0fb2){const _0x3de253=_0x39ebd3,_0x213837=new Rectangle(0x0,0x0,_0x2b0fb2[_0x3de253(0x417)],_0x2b0fb2[_0x3de253(0x435)]);this[_0x3de253(0x141)]=new Window_Base(_0x213837),this[_0x3de253(0x141)][_0x3de253(0x191)]=0x0,this[_0x3de253(0x4d3)](this[_0x3de253(0x141)]),this[_0x3de253(0x1c4)]();},Window_EquipCommand[_0x39ebd3(0x3bc)][_0x39ebd3(0x401)]=function(){const _0x58ba4e=_0x39ebd3;Window_HorzCommand['prototype'][_0x58ba4e(0x401)]['call'](this);if(this[_0x58ba4e(0x141)])this[_0x58ba4e(0x1c4)]();},Window_EquipCommand[_0x39ebd3(0x3bc)][_0x39ebd3(0x1c4)]=function(){const _0x16e6c9=_0x39ebd3,_0x5190a7=this[_0x16e6c9(0x141)];_0x5190a7[_0x16e6c9(0x456)][_0x16e6c9(0x39f)]();const _0x4444c6=this[_0x16e6c9(0xd8)](this[_0x16e6c9(0x376)]());if(_0x4444c6===_0x16e6c9(0x132)){const _0x1c4073=this[_0x16e6c9(0x3d8)](this[_0x16e6c9(0x376)]());let _0x3c6875=this[_0x16e6c9(0x40f)](this[_0x16e6c9(0x376)]());_0x3c6875=_0x3c6875[_0x16e6c9(0x295)](/\\I\[(\d+)\]/gi,''),_0x5190a7[_0x16e6c9(0x3c9)](),this[_0x16e6c9(0x160)](_0x3c6875,_0x1c4073),this[_0x16e6c9(0x2ca)](_0x3c6875,_0x1c4073),this[_0x16e6c9(0x2d0)](_0x3c6875,_0x1c4073);}},Window_EquipCommand[_0x39ebd3(0x3bc)][_0x39ebd3(0x160)]=function(_0x3783b0,_0x29b836){},Window_EquipCommand[_0x39ebd3(0x3bc)]['commandNameWindowDrawText']=function(_0x2039bd,_0x2d48c3){const _0x3e30e4=_0x39ebd3,_0x6d74fd=this[_0x3e30e4(0x141)];_0x6d74fd['drawText'](_0x2039bd,0x0,_0x2d48c3['y'],_0x6d74fd[_0x3e30e4(0x35e)],_0x3e30e4(0x34f));},Window_EquipCommand[_0x39ebd3(0x3bc)][_0x39ebd3(0x2d0)]=function(_0x455b5b,_0x4dd3ae){const _0x2e7ca3=_0x39ebd3,_0x1d32be=this[_0x2e7ca3(0x141)],_0x2e737c=$gameSystem[_0x2e7ca3(0xe1)](),_0x3634a8=_0x4dd3ae['x']+Math[_0x2e7ca3(0x349)](_0x4dd3ae[_0x2e7ca3(0x417)]/0x2)+_0x2e737c;_0x1d32be['x']=_0x1d32be[_0x2e7ca3(0x417)]/-0x2+_0x3634a8,_0x1d32be['y']=Math['floor'](_0x4dd3ae[_0x2e7ca3(0x435)]/0x2);},Window_EquipCommand[_0x39ebd3(0x3bc)][_0x39ebd3(0x28c)]=function(){const _0x42dfb4=_0x39ebd3;return Imported[_0x42dfb4(0x372)]&&Window_HorzCommand[_0x42dfb4(0x3bc)]['isUseModernControls']['call'](this);},Window_EquipCommand[_0x39ebd3(0x3bc)][_0x39ebd3(0x156)]=function(){const _0x3b85fc=_0x39ebd3;if(this[_0x3b85fc(0x428)]()===_0x3b85fc(0x3ac))Window_HorzCommand[_0x3b85fc(0x3bc)][_0x3b85fc(0x156)]['call'](this);},Window_EquipCommand[_0x39ebd3(0x3bc)][_0x39ebd3(0x429)]=function(){const _0x62ab88=_0x39ebd3;!this[_0x62ab88(0x287)]()&&Window_HorzCommand[_0x62ab88(0x3bc)][_0x62ab88(0x429)][_0x62ab88(0x15c)](this);},Window_EquipCommand[_0x39ebd3(0x3bc)][_0x39ebd3(0x287)]=function(){const _0x536e7a=_0x39ebd3;if(!this['isCursorMovable']())return![];if(SceneManager[_0x536e7a(0x47d)][_0x536e7a(0x3f4)]!==Scene_Equip)return![];return Input[_0x536e7a(0x30a)](_0x536e7a(0x2d7))&&this['processDownCursorSpecialCheckModernControls'](),![];},Window_EquipCommand[_0x39ebd3(0x3bc)][_0x39ebd3(0x3f5)]=function(){const _0x5cdb89=_0x39ebd3;this[_0x5cdb89(0x48d)](),SceneManager[_0x5cdb89(0x47d)][_0x5cdb89(0x25a)](),SceneManager['_scene'][_0x5cdb89(0x482)][_0x5cdb89(0x436)](-0x1);},Window_EquipCommand['prototype'][_0x39ebd3(0x340)]=function(){return this['_list']?this['_list']['length']:0x3;},Window_EquipCommand[_0x39ebd3(0x3bc)][_0x39ebd3(0x122)]=function(){const _0x2c569a=_0x39ebd3;if(this['isOpen']()&&this[_0x2c569a(0x219)]&&SceneManager[_0x2c569a(0x47d)][_0x2c569a(0x3f4)]===Scene_Equip){if(this[_0x2c569a(0x1d2)]()&&TouchInput[_0x2c569a(0x48f)]())this[_0x2c569a(0x4b3)](![]);else TouchInput[_0x2c569a(0x30a)]()&&this[_0x2c569a(0x4b3)](!![]);TouchInput[_0x2c569a(0x2c4)]()&&this[_0x2c569a(0x279)]();}},Window_EquipCommand['prototype'][_0x39ebd3(0x4b3)]=function(_0x331ddc){const _0x366462=_0x39ebd3;this[_0x366462(0x121)]=![];const _0x20e44e=this[_0x366462(0x376)](),_0x22199d=this[_0x366462(0x4f6)](),_0x1ddd5d=SceneManager[_0x366462(0x47d)]['_slotWindow'];if(_0x1ddd5d['isOpen']()&&_0x1ddd5d[_0x366462(0x219)]){if(_0x22199d>=0x0)_0x22199d===this['index']()&&(this[_0x366462(0x121)]=!![]),this[_0x366462(0x3a9)](),this[_0x366462(0x3f8)](_0x22199d);else _0x1ddd5d[_0x366462(0x4f6)]()>=0x0&&(this[_0x366462(0x3d9)](),this[_0x366462(0x213)]());}_0x331ddc&&this[_0x366462(0x376)]()!==_0x20e44e&&this[_0x366462(0x48d)]();},Window_EquipCommand[_0x39ebd3(0x3bc)][_0x39ebd3(0x2e1)]=function(){const _0x9e86e0=_0x39ebd3;this[_0x9e86e0(0x3a5)](),this[_0x9e86e0(0x2ef)](),this[_0x9e86e0(0x29a)]();},Window_EquipCommand[_0x39ebd3(0x3bc)][_0x39ebd3(0x39e)]=function(){const _0xe400e2=_0x39ebd3;Window_HorzCommand[_0xe400e2(0x3bc)][_0xe400e2(0x39e)][_0xe400e2(0x15c)](this),this['refreshCursor']();},Window_EquipCommand[_0x39ebd3(0x3bc)][_0x39ebd3(0x3a5)]=function(){const _0x2b8ca3=_0x39ebd3;if(!this[_0x2b8ca3(0xe9)]())return;const _0x1233fe=this['commandStyle'](),_0x4615fb=VisuMZ[_0x2b8ca3(0x2a9)][_0x2b8ca3(0x237)][_0x2b8ca3(0x322)]['CmdIconEquip'],_0x462cff=_0x1233fe===_0x2b8ca3(0x30c)?TextManager[_0x2b8ca3(0x1aa)]:_0x2b8ca3(0x316)[_0x2b8ca3(0x1da)](_0x4615fb,TextManager[_0x2b8ca3(0x1aa)]),_0xff3db5=this[_0x2b8ca3(0x123)]();this['addCommand'](_0x462cff,_0x2b8ca3(0x3ac),_0xff3db5);},Window_EquipCommand['prototype'][_0x39ebd3(0xe9)]=function(){const _0x234707=_0x39ebd3;return!this[_0x234707(0x28c)]();},Window_EquipCommand[_0x39ebd3(0x3bc)]['isEquipCommandEnabled']=function(){return!![];},Window_EquipCommand[_0x39ebd3(0x3bc)][_0x39ebd3(0x2ef)]=function(){const _0x2b51a9=_0x39ebd3;if(!this[_0x2b51a9(0x4f7)]())return;const _0x54332b=this['commandStyle'](),_0x9cda96=VisuMZ[_0x2b51a9(0x2a9)][_0x2b51a9(0x237)][_0x2b51a9(0x322)][_0x2b51a9(0x15a)],_0x13b83b=_0x54332b===_0x2b51a9(0x30c)?TextManager[_0x2b51a9(0x494)]:_0x2b51a9(0x316)['format'](_0x9cda96,TextManager['optimize']),_0x2c3bd6=this[_0x2b51a9(0x4db)]();this[_0x2b51a9(0x303)](_0x13b83b,_0x2b51a9(0x494),_0x2c3bd6);},Window_EquipCommand[_0x39ebd3(0x3bc)]['isOptimizeCommandAdded']=function(){const _0x2db2ed=_0x39ebd3;return VisuMZ[_0x2db2ed(0x2a9)][_0x2db2ed(0x237)][_0x2db2ed(0x322)][_0x2db2ed(0x1e2)];},Window_EquipCommand['prototype'][_0x39ebd3(0x4db)]=function(){return!![];},Window_EquipCommand[_0x39ebd3(0x3bc)]['addClearCommand']=function(){const _0x40e0d9=_0x39ebd3;if(!this[_0x40e0d9(0xca)]())return;const _0x2f18e8=this[_0x40e0d9(0x425)](),_0x269b31=VisuMZ[_0x40e0d9(0x2a9)][_0x40e0d9(0x237)]['EquipScene'][_0x40e0d9(0x9b)],_0x19d69d=_0x2f18e8===_0x40e0d9(0x30c)?TextManager['clear']:_0x40e0d9(0x316)[_0x40e0d9(0x1da)](_0x269b31,TextManager[_0x40e0d9(0x39f)]),_0x2bc9b4=this[_0x40e0d9(0x403)]();this[_0x40e0d9(0x303)](_0x19d69d,_0x40e0d9(0x39f),_0x2bc9b4);},Window_EquipCommand[_0x39ebd3(0x3bc)][_0x39ebd3(0xca)]=function(){const _0xfc7aaf=_0x39ebd3;return VisuMZ['ItemsEquipsCore'][_0xfc7aaf(0x237)][_0xfc7aaf(0x322)][_0xfc7aaf(0x33c)];},Window_EquipCommand[_0x39ebd3(0x3bc)][_0x39ebd3(0x403)]=function(){return!![];},Window_EquipCommand[_0x39ebd3(0x3bc)][_0x39ebd3(0x47a)]=function(){const _0x586086=_0x39ebd3;return VisuMZ[_0x586086(0x2a9)][_0x586086(0x237)][_0x586086(0x322)][_0x586086(0x29d)];},Window_EquipCommand[_0x39ebd3(0x3bc)][_0x39ebd3(0x129)]=function(_0x38fcda){const _0x4ab621=_0x39ebd3,_0x520457=this['commandStyleCheck'](_0x38fcda);if(_0x520457===_0x4ab621(0x2ed))this[_0x4ab621(0xfb)](_0x38fcda);else _0x520457===_0x4ab621(0x132)?this['drawItemStyleIcon'](_0x38fcda):Window_HorzCommand[_0x4ab621(0x3bc)]['drawItem'][_0x4ab621(0x15c)](this,_0x38fcda);},Window_EquipCommand[_0x39ebd3(0x3bc)][_0x39ebd3(0x425)]=function(){const _0x4a3723=_0x39ebd3;return VisuMZ[_0x4a3723(0x2a9)][_0x4a3723(0x237)][_0x4a3723(0x322)][_0x4a3723(0x2dc)];},Window_EquipCommand[_0x39ebd3(0x3bc)][_0x39ebd3(0xd8)]=function(_0x4c422c){const _0x3752af=_0x39ebd3;if(_0x4c422c<0x0)return _0x3752af(0x30c);const _0x3abc97=this[_0x3752af(0x425)]();if(_0x3abc97!==_0x3752af(0x329))return _0x3abc97;else{if(this[_0x3752af(0x28d)]()>0x0){const _0x15da52=this[_0x3752af(0x40f)](_0x4c422c);if(_0x15da52[_0x3752af(0x145)](/\\I\[(\d+)\]/i)){const _0x1d6f86=this[_0x3752af(0x3d8)](_0x4c422c),_0x19a244=this['textSizeEx'](_0x15da52)[_0x3752af(0x417)];return _0x19a244<=_0x1d6f86[_0x3752af(0x417)]?_0x3752af(0x2ed):_0x3752af(0x132);}}}return _0x3752af(0x30c);},Window_EquipCommand[_0x39ebd3(0x3bc)][_0x39ebd3(0xfb)]=function(_0x265ffe){const _0x5a3fab=_0x39ebd3,_0x432d72=this[_0x5a3fab(0x3d8)](_0x265ffe),_0x44e6a0=this[_0x5a3fab(0x40f)](_0x265ffe),_0x12fbba=this[_0x5a3fab(0x448)](_0x44e6a0)[_0x5a3fab(0x417)];this[_0x5a3fab(0x2e7)](this['isCommandEnabled'](_0x265ffe));const _0x23744f=this[_0x5a3fab(0x47a)]();if(_0x23744f===_0x5a3fab(0x3d7))this[_0x5a3fab(0x451)](_0x44e6a0,_0x432d72['x']+_0x432d72[_0x5a3fab(0x417)]-_0x12fbba,_0x432d72['y'],_0x12fbba);else{if(_0x23744f===_0x5a3fab(0x34f)){const _0xab737f=_0x432d72['x']+Math[_0x5a3fab(0x349)]((_0x432d72[_0x5a3fab(0x417)]-_0x12fbba)/0x2);this[_0x5a3fab(0x451)](_0x44e6a0,_0xab737f,_0x432d72['y'],_0x12fbba);}else this['drawTextEx'](_0x44e6a0,_0x432d72['x'],_0x432d72['y'],_0x12fbba);}},Window_EquipCommand[_0x39ebd3(0x3bc)][_0x39ebd3(0x3c5)]=function(_0x460a40){const _0xc2f4f9=_0x39ebd3;this[_0xc2f4f9(0x40f)](_0x460a40)[_0xc2f4f9(0x145)](/\\I\[(\d+)\]/i);const _0x483e78=Number(RegExp['$1'])||0x0,_0x16bace=this[_0xc2f4f9(0x3d8)](_0x460a40),_0x6c7b1f=_0x16bace['x']+Math[_0xc2f4f9(0x349)]((_0x16bace['width']-ImageManager[_0xc2f4f9(0x225)])/0x2),_0x4a1c19=_0x16bace['y']+(_0x16bace[_0xc2f4f9(0x435)]-ImageManager[_0xc2f4f9(0x357)])/0x2;this[_0xc2f4f9(0x4c9)](_0x483e78,_0x6c7b1f,_0x4a1c19);},Window_EquipCommand[_0x39ebd3(0x3bc)][_0x39ebd3(0x1f8)]=function(){const _0x22723b=_0x39ebd3,_0x4804c2=SceneManager[_0x22723b(0x47d)];if(_0x4804c2&&_0x4804c2[_0x22723b(0x45e)])return _0x4804c2[_0x22723b(0x45e)]();return null;},Window_EquipCommand[_0x39ebd3(0x3bc)][_0x39ebd3(0x19e)]=function(){const _0x4653a8=_0x39ebd3;Window_Command[_0x4653a8(0x3bc)][_0x4653a8(0x19e)][_0x4653a8(0x15c)](this),this[_0x4653a8(0xb5)]['setText'](this[_0x4653a8(0x36e)]());},Window_EquipCommand[_0x39ebd3(0x3bc)]['helpDescriptionText']=function(){const _0x166a3c=_0x39ebd3,_0x1d9ebb=this['currentSymbol']();switch(_0x1d9ebb){case _0x166a3c(0x3ac):return TextManager[_0x166a3c(0x4b7)][_0x166a3c(0x2a1)]['equip'];case _0x166a3c(0x494):return TextManager[_0x166a3c(0x4b7)][_0x166a3c(0x2a1)][_0x166a3c(0x494)];case _0x166a3c(0x39f):return TextManager[_0x166a3c(0x4b7)][_0x166a3c(0x2a1)][_0x166a3c(0x39f)];default:return'';}},Window_EquipSlot[_0x39ebd3(0x3bc)]['isUseModernControls']=function(){const _0x35ccf8=_0x39ebd3;return Imported['VisuMZ_0_CoreEngine']&&Window_HorzCommand[_0x35ccf8(0x3bc)][_0x35ccf8(0x28c)][_0x35ccf8(0x15c)](this);},Window_EquipSlot[_0x39ebd3(0x3bc)][_0x39ebd3(0x3a9)]=function(){const _0x4bf7cc=_0x39ebd3;Window_StatusBase[_0x4bf7cc(0x3bc)][_0x4bf7cc(0x3a9)]['call'](this),this[_0x4bf7cc(0x401)]();},Window_EquipSlot[_0x39ebd3(0x3bc)][_0x39ebd3(0x402)]=function(){const _0x52b99b=_0x39ebd3;Window_StatusBase[_0x52b99b(0x3bc)][_0x52b99b(0x402)][_0x52b99b(0x15c)](this),this[_0x52b99b(0x43d)]();},Window_EquipSlot['prototype'][_0x39ebd3(0x43d)]=function(){const _0x50f34c=_0x39ebd3;if(!this[_0x50f34c(0x321)]())return;if(Input[_0x50f34c(0x30a)](_0x50f34c(0x195))&&this['item']()){const _0xf09fdf=SceneManager[_0x50f34c(0x47d)][_0x50f34c(0x4b4)];_0xf09fdf&&(this['canShiftRemoveEquipment'](this[_0x50f34c(0x376)]())?(this[_0x50f34c(0x23b)](),this['updateHelp']()):this[_0x50f34c(0x475)]());}},Window_EquipSlot[_0x39ebd3(0x3bc)][_0x39ebd3(0x4f9)]=function(_0x1cf095){const _0x47ffc0=_0x39ebd3,_0x31cddf=SceneManager[_0x47ffc0(0x47d)]['_actor'];if(!_0x31cddf)return;if(!_0x31cddf[_0x47ffc0(0x214)](_0x1cf095))return![];const _0x4f62fb=_0x31cddf['equipSlots']()[_0x1cf095];if(_0x31cddf[_0x47ffc0(0x22d)]()[_0x47ffc0(0x35f)](_0x4f62fb))return![];return!![];;},Window_EquipSlot[_0x39ebd3(0x3bc)][_0x39ebd3(0x23b)]=function(){const _0x4e21b3=_0x39ebd3;SoundManager[_0x4e21b3(0x12f)]();const _0x2510b7=SceneManager['_scene'][_0x4e21b3(0x4b4)];_0x2510b7[_0x4e21b3(0x434)](this['index'](),null),this[_0x4e21b3(0x39e)](),this['_itemWindow'][_0x4e21b3(0x39e)](),this[_0x4e21b3(0x401)]();const _0x582a63=SceneManager[_0x4e21b3(0x47d)]['_statusWindow'];if(_0x582a63)_0x582a63[_0x4e21b3(0x39e)]();},Window_EquipSlot[_0x39ebd3(0x3bc)][_0x39ebd3(0x321)]=function(){const _0x53561b=_0x39ebd3;if(!this[_0x53561b(0x274)])return![];if(!VisuMZ[_0x53561b(0x2a9)]['Settings'][_0x53561b(0x322)]['ShiftShortcutKey'])return![];return!![];},Window_EquipSlot[_0x39ebd3(0x3bc)][_0x39ebd3(0x429)]=function(){const _0x4e4add=_0x39ebd3;!this[_0x4e4add(0x287)]()&&Window_StatusBase['prototype'][_0x4e4add(0x429)][_0x4e4add(0x15c)](this);},Window_EquipSlot[_0x39ebd3(0x3bc)][_0x39ebd3(0x287)]=function(){const _0x329d55=_0x39ebd3;if(!this[_0x329d55(0x22c)]())return![];if(SceneManager[_0x329d55(0x47d)][_0x329d55(0x3f4)]!==Scene_Equip)return![];if(this[_0x329d55(0x19c)]())return this['playCursorSound'](),Input[_0x329d55(0x39f)](),SceneManager[_0x329d55(0x47d)][_0x329d55(0x159)](),![];else{if(Input[_0x329d55(0x4d9)](_0x329d55(0x2d7))){const _0x3eded3=this[_0x329d55(0x376)]();return Input[_0x329d55(0x40e)]('shift')?this[_0x329d55(0x390)]():this[_0x329d55(0x2b3)](Input[_0x329d55(0x30a)](_0x329d55(0x2d7))),this['index']()!==_0x3eded3&&this[_0x329d55(0x48d)](),!![];}else{if(this[_0x329d55(0x1eb)]()&&Input[_0x329d55(0x30a)](_0x329d55(0x195)))return!![];}}return![];},Window_EquipSlot[_0x39ebd3(0x3bc)]['allowCommandWindowCursorUp']=function(){const _0x3a3bdb=_0x39ebd3;if(this[_0x3a3bdb(0x376)]()!==0x0)return![];const _0x4f5f0d=VisuMZ[_0x3a3bdb(0x2a9)][_0x3a3bdb(0x237)][_0x3a3bdb(0x322)];if(!_0x4f5f0d['CommandAddOptimize']&&!_0x4f5f0d['CommandAddClear'])return![];return Input[_0x3a3bdb(0x30a)]('up');},Window_EquipSlot['prototype'][_0x39ebd3(0x1eb)]=function(){const _0x46b9aa=_0x39ebd3;return VisuMZ[_0x46b9aa(0x2a9)]['Settings']['EquipScene'][_0x46b9aa(0x43a)];},Window_EquipSlot[_0x39ebd3(0x3bc)]['processTouchModernControls']=function(){const _0x426612=_0x39ebd3;if(this['isOpen']()&&this[_0x426612(0x219)]&&SceneManager[_0x426612(0x47d)][_0x426612(0x3f4)]===Scene_Equip){if(this[_0x426612(0x1d2)]()&&TouchInput[_0x426612(0x48f)]())this['onTouchSelectModernControls'](![]);else TouchInput[_0x426612(0x30a)]()&&this[_0x426612(0x4b3)](!![]);if(TouchInput[_0x426612(0x2c4)]())this['onTouchOk']();else{if(TouchInput[_0x426612(0x20a)]()){const _0xf7b6b3=VisuMZ[_0x426612(0x2a9)][_0x426612(0x237)][_0x426612(0x322)];this[_0x426612(0x28c)]()&&this[_0x426612(0x274)]&&!_0xf7b6b3['CommandAddOptimize']&&!_0xf7b6b3['CommandAddClear']?(SoundManager[_0x426612(0x168)](),SceneManager[_0x426612(0x2ad)]()):this[_0x426612(0x105)]();}}}},Window_EquipSlot[_0x39ebd3(0x3bc)]['onTouchSelectModernControls']=function(_0xda8be1){const _0x16d877=_0x39ebd3;this[_0x16d877(0x121)]=![];const _0xf63a9=this[_0x16d877(0x376)](),_0x20ccd9=this['hitIndex'](),_0x5d61aa=SceneManager[_0x16d877(0x47d)]['_commandWindow'];if(_0x5d61aa[_0x16d877(0x460)]()&&_0x5d61aa[_0x16d877(0x219)]){if(_0x20ccd9>=0x0)_0x20ccd9===this[_0x16d877(0x376)]()&&(this[_0x16d877(0x121)]=!![]),this[_0x16d877(0x3a9)](),this[_0x16d877(0x3f8)](_0x20ccd9),_0x5d61aa['deactivate']();else _0x5d61aa['hitIndex']()>=0x0&&(this[_0x16d877(0x3d9)](),this[_0x16d877(0x213)](),_0x5d61aa[_0x16d877(0x3a9)]());}_0xda8be1&&this[_0x16d877(0x376)]()!==_0xf63a9&&this['playCursorSound']();},Window_EquipSlot['prototype']['equipSlotIndex']=function(){const _0x1e6467=_0x39ebd3;return this[_0x1e6467(0x376)]();},VisuMZ['ItemsEquipsCore'][_0x39ebd3(0x21c)]=Window_EquipSlot['prototype']['isEnabled'],Window_EquipSlot['prototype'][_0x39ebd3(0x4dd)]=function(_0xd98993){const _0x59eb7b=_0x39ebd3;if(this[_0x59eb7b(0x28d)]()<=0x0)return![];return VisuMZ[_0x59eb7b(0x2a9)][_0x59eb7b(0x21c)][_0x59eb7b(0x15c)](this,_0xd98993);},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x455)]=Window_EquipItem[_0x39ebd3(0x3bc)][_0x39ebd3(0x35f)],Window_EquipItem[_0x39ebd3(0x3bc)][_0x39ebd3(0x35f)]=function(_0x455a41){const _0x278b9e=_0x39ebd3;if(_0x455a41===null&&this['nonRemovableEtypes']()[_0x278b9e(0x35f)](this[_0x278b9e(0x34a)]()))return![];else{$gameTemp[_0x278b9e(0x266)]=!![];let _0x226e05=VisuMZ[_0x278b9e(0x2a9)][_0x278b9e(0x455)][_0x278b9e(0x15c)](this,_0x455a41);if(!_0x226e05&&_0x455a41&&DataManager['isArmor'](_0x455a41)){const _0x200bec=_0x455a41[_0x278b9e(0x31e)]||0x0;if(this[_0x278b9e(0x4b4)]&&this[_0x278b9e(0x4b4)]['isEquipAtypeOk'](_0x200bec)){const _0x52ba5d=DataManager[_0x278b9e(0x4a5)](_0x455a41);_0x52ba5d[_0x278b9e(0x35f)](this[_0x278b9e(0x34a)]())&&(_0x226e05=!![]);}}return $gameTemp[_0x278b9e(0x266)]=undefined,_0x226e05;}},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0xe6)]=Window_EquipItem[_0x39ebd3(0x3bc)]['isEnabled'],Window_EquipItem[_0x39ebd3(0x3bc)]['isEnabled']=function(_0x583528){const _0x783e9=_0x39ebd3;if(_0x583528&&this[_0x783e9(0x4b4)]){if(this['itemHasEquipLimit'](_0x583528))return![];if(this['isSoleWeaponType'](_0x583528))return![];if(this[_0x783e9(0x157)](_0x583528))return![];if(!this[_0x783e9(0x4b4)][_0x783e9(0x1d6)](_0x583528))return![];}if(!_0x583528)return!this[_0x783e9(0x22d)]()[_0x783e9(0x35f)](this[_0x783e9(0x34a)]());return VisuMZ[_0x783e9(0x2a9)][_0x783e9(0xe6)][_0x783e9(0x15c)](this,_0x583528);},Window_EquipItem['prototype']['itemHasEquipLimit']=function(_0x508904){const _0x132e22=_0x39ebd3,_0x42c8bc=_0x508904[_0x132e22(0x3ba)];if(_0x42c8bc[_0x132e22(0x145)](/<EQUIP COPY LIMIT:[ ](\d+)>/i)){const _0x273cfe=Number(RegExp['$1'])||0x1;let _0x5d0c80=0x0;const _0x25acad=this['_actor'][_0x132e22(0x257)](),_0x475387=SceneManager[_0x132e22(0x47d)]['_slotWindow'][_0x132e22(0x112)]();_0x25acad[_0x475387]=null;for(const _0x3fa8ca of _0x25acad){if(!_0x3fa8ca)continue;if(DataManager['isWeapon'](_0x508904)===DataManager['isWeapon'](_0x3fa8ca)){if(_0x508904['id']===_0x3fa8ca['id'])_0x5d0c80+=0x1;}}return _0x5d0c80>=_0x273cfe;}else return![];},Window_EquipItem[_0x39ebd3(0x3bc)][_0x39ebd3(0x1f4)]=function(_0x1c5eb7){const _0x18768b=_0x39ebd3;if(!DataManager[_0x18768b(0x2db)](_0x1c5eb7))return![];const _0x4c1fe2=/<EQUIP WEAPON TYPE LIMIT:[ ](\d+)>/i;let _0x1a12b3=0x0;const _0x16c60d=this[_0x18768b(0x4b4)][_0x18768b(0x257)](),_0x4a7dd1=SceneManager[_0x18768b(0x47d)][_0x18768b(0x482)][_0x18768b(0x112)]();_0x16c60d[_0x4a7dd1]=null;for(const _0x3b07dc of _0x16c60d){if(!_0x3b07dc)continue;if(!DataManager['isWeapon'](_0x3b07dc))continue;if(_0x1c5eb7[_0x18768b(0x1d7)]===_0x3b07dc['wtypeId']){_0x1a12b3+=0x1;if(_0x1c5eb7[_0x18768b(0x3ba)][_0x18768b(0x145)](_0x4c1fe2)){const _0x201a24=Number(RegExp['$1'])||0x1;if(_0x1a12b3>=_0x201a24)return!![];}if(_0x3b07dc[_0x18768b(0x3ba)]['match'](_0x4c1fe2)){const _0x47721b=Number(RegExp['$1'])||0x1;if(_0x1a12b3>=_0x47721b)return!![];}}}return![];},Window_EquipItem['prototype'][_0x39ebd3(0x157)]=function(_0x551b98){const _0x56165a=_0x39ebd3;if(!DataManager[_0x56165a(0x13b)](_0x551b98))return![];const _0x522da5=/<EQUIP ARMOR TYPE LIMIT:[ ](\d+)>/i;let _0x2debc2=0x0;const _0xd3a350=this[_0x56165a(0x4b4)][_0x56165a(0x257)](),_0x1e1ecc=SceneManager[_0x56165a(0x47d)]['_slotWindow'][_0x56165a(0x112)]();_0xd3a350[_0x1e1ecc]=null;for(const _0x16a0b7 of _0xd3a350){if(!_0x16a0b7)continue;if(!DataManager['isArmor'](_0x16a0b7))continue;if(_0x551b98[_0x56165a(0x31e)]===_0x16a0b7[_0x56165a(0x31e)]){_0x2debc2+=0x1;if(_0x551b98[_0x56165a(0x3ba)][_0x56165a(0x145)](_0x522da5)){const _0x307847=Number(RegExp['$1'])||0x1;if(_0x2debc2>=_0x307847)return!![];}if(_0x16a0b7['note'][_0x56165a(0x145)](_0x522da5)){const _0xa75480=Number(RegExp['$1'])||0x1;if(_0x2debc2>=_0xa75480)return!![];}}}return![];},Window_EquipItem[_0x39ebd3(0x3bc)][_0x39ebd3(0x22d)]=function(){const _0x4e80fc=_0x39ebd3;return VisuMZ['ItemsEquipsCore'][_0x4e80fc(0x237)][_0x4e80fc(0x322)]['NonRemoveETypes'];},Window_EquipItem[_0x39ebd3(0x3bc)][_0x39ebd3(0x129)]=function(_0x1d4143){const _0x4596fa=_0x39ebd3,_0x51765a=this[_0x4596fa(0x4af)](_0x1d4143);_0x51765a?Window_ItemList[_0x4596fa(0x3bc)][_0x4596fa(0x129)][_0x4596fa(0x15c)](this,_0x1d4143):this[_0x4596fa(0x38c)](_0x1d4143);},Window_EquipItem[_0x39ebd3(0x3bc)]['drawRemoveItem']=function(_0x43c3a7){const _0xa5a646=_0x39ebd3;this['changePaintOpacity'](this[_0xa5a646(0x4dd)](null));const _0xe8145=ImageManager[_0xa5a646(0x4a1)]||0x20,_0x154f69=_0xe8145-ImageManager[_0xa5a646(0x225)],_0x2856d1=_0xe8145+0x4,_0xe55362=VisuMZ[_0xa5a646(0x2a9)][_0xa5a646(0x237)][_0xa5a646(0x322)],_0x1f72de=this[_0xa5a646(0x3d8)](_0x43c3a7),_0x2cdea4=_0x1f72de['y']+(this['lineHeight']()-ImageManager[_0xa5a646(0x357)])/0x2,_0x288d5e=Math[_0xa5a646(0x165)](0x0,_0x1f72de[_0xa5a646(0x417)]-_0x2856d1);this[_0xa5a646(0x1e5)](),this[_0xa5a646(0x4c9)](_0xe55362[_0xa5a646(0x300)],_0x1f72de['x']+Math[_0xa5a646(0x3da)](_0x154f69/0x2),_0x2cdea4),this[_0xa5a646(0x1b9)](_0xe55362[_0xa5a646(0xeb)],_0x1f72de['x']+_0x2856d1,_0x1f72de['y'],_0x288d5e),this[_0xa5a646(0x2e7)](!![]);},Window_EquipItem[_0x39ebd3(0x3bc)][_0x39ebd3(0x19e)]=function(){const _0x21fb7c=_0x39ebd3;Window_ItemList[_0x21fb7c(0x3bc)][_0x21fb7c(0x19e)][_0x21fb7c(0x15c)](this);if(this[_0x21fb7c(0x4b4)]&&this[_0x21fb7c(0x441)]&&this[_0x21fb7c(0x26a)]>=0x0){const _0x1cb38d=JsonEx[_0x21fb7c(0x4e8)](this['_actor']);_0x1cb38d[_0x21fb7c(0x119)]=!![],_0x1cb38d[_0x21fb7c(0xb0)](this['_slotId'],this[_0x21fb7c(0x27c)]()),this[_0x21fb7c(0x441)]['setTempActor'](_0x1cb38d);}},VisuMZ['ItemsEquipsCore'][_0x39ebd3(0x4aa)]=Window_ShopCommand[_0x39ebd3(0x3bc)][_0x39ebd3(0x4a2)],Window_ShopCommand[_0x39ebd3(0x3bc)][_0x39ebd3(0x4a2)]=function(_0x3272fd){const _0x497185=_0x39ebd3;VisuMZ['ItemsEquipsCore'][_0x497185(0x4aa)][_0x497185(0x15c)](this,_0x3272fd),this[_0x497185(0x118)](_0x3272fd);},Window_ShopCommand[_0x39ebd3(0x3bc)][_0x39ebd3(0x118)]=function(_0x5a2ef1){const _0x2b3731=_0x39ebd3,_0x1a9082=new Rectangle(0x0,0x0,_0x5a2ef1['width'],_0x5a2ef1[_0x2b3731(0x435)]);this[_0x2b3731(0x141)]=new Window_Base(_0x1a9082),this[_0x2b3731(0x141)][_0x2b3731(0x191)]=0x0,this['addChild'](this[_0x2b3731(0x141)]),this[_0x2b3731(0x1c4)]();},Window_ShopCommand[_0x39ebd3(0x3bc)][_0x39ebd3(0x401)]=function(){const _0x2e8960=_0x39ebd3;Window_HorzCommand[_0x2e8960(0x3bc)][_0x2e8960(0x401)][_0x2e8960(0x15c)](this);if(this['_commandNameWindow'])this['updateCommandNameWindow']();},Window_ShopCommand[_0x39ebd3(0x3bc)]['updateCommandNameWindow']=function(){const _0x396b37=_0x39ebd3,_0x11fa58=this[_0x396b37(0x141)];_0x11fa58[_0x396b37(0x456)][_0x396b37(0x39f)]();const _0x246569=this[_0x396b37(0xd8)](this[_0x396b37(0x376)]());if(_0x246569===_0x396b37(0x132)){const _0x40de35=this[_0x396b37(0x3d8)](this[_0x396b37(0x376)]());let _0x1e25c9=this['commandName'](this[_0x396b37(0x376)]());_0x1e25c9=_0x1e25c9[_0x396b37(0x295)](/\\I\[(\d+)\]/gi,''),_0x11fa58[_0x396b37(0x3c9)](),this['commandNameWindowDrawBackground'](_0x1e25c9,_0x40de35),this[_0x396b37(0x2ca)](_0x1e25c9,_0x40de35),this[_0x396b37(0x2d0)](_0x1e25c9,_0x40de35);}},Window_ShopCommand[_0x39ebd3(0x3bc)][_0x39ebd3(0x160)]=function(_0x30ed96,_0x1841b0){},Window_ShopCommand[_0x39ebd3(0x3bc)][_0x39ebd3(0x2ca)]=function(_0x8e607c,_0x28ab1b){const _0x39f5c4=_0x39ebd3,_0x568f8f=this['_commandNameWindow'];_0x568f8f['drawText'](_0x8e607c,0x0,_0x28ab1b['y'],_0x568f8f[_0x39f5c4(0x35e)],'center');},Window_ShopCommand[_0x39ebd3(0x3bc)][_0x39ebd3(0x2d0)]=function(_0x369c34,_0xd25773){const _0x3057e0=_0x39ebd3,_0x31daa9=this[_0x3057e0(0x141)],_0x3c0eaa=$gameSystem[_0x3057e0(0xe1)](),_0x1e53dd=_0xd25773['x']+Math[_0x3057e0(0x349)](_0xd25773[_0x3057e0(0x417)]/0x2)+_0x3c0eaa;_0x31daa9['x']=_0x31daa9[_0x3057e0(0x417)]/-0x2+_0x1e53dd,_0x31daa9['y']=Math[_0x3057e0(0x349)](_0xd25773[_0x3057e0(0x435)]/0x2);},Window_ShopCommand['prototype'][_0x39ebd3(0x340)]=function(){const _0x3baf0c=_0x39ebd3;return this['_list']?this[_0x3baf0c(0x2a5)][_0x3baf0c(0x142)]:0x3;},Window_ShopCommand[_0x39ebd3(0x3bc)][_0x39ebd3(0x366)]=function(){const _0x359de2=_0x39ebd3;return VisuMZ[_0x359de2(0x2a9)][_0x359de2(0x237)][_0x359de2(0x3a4)][_0x359de2(0x2e0)];},Window_ShopCommand[_0x39ebd3(0x3bc)]['makeCommandList']=function(){const _0x567ef7=_0x39ebd3;this[_0x567ef7(0x1c8)](),this[_0x567ef7(0x3a0)](),this[_0x567ef7(0x478)]();},Window_ShopCommand['prototype'][_0x39ebd3(0x39e)]=function(){const _0x4b45b5=_0x39ebd3;Window_HorzCommand[_0x4b45b5(0x3bc)][_0x4b45b5(0x39e)][_0x4b45b5(0x15c)](this),this[_0x4b45b5(0x21b)]();},Window_ShopCommand[_0x39ebd3(0x3bc)]['addBuyCommand']=function(){const _0xe4f259=_0x39ebd3,_0x1c7c74=this['commandStyle'](),_0x4b3fb1=VisuMZ[_0xe4f259(0x2a9)][_0xe4f259(0x237)]['ShopScene']['CmdIconBuy'],_0x1c0926=_0x1c7c74==='text'?TextManager[_0xe4f259(0xcb)]:_0xe4f259(0x316)[_0xe4f259(0x1da)](_0x4b3fb1,TextManager[_0xe4f259(0xcb)]),_0x1f1a3c=this[_0xe4f259(0x4a6)]();if(this[_0xe4f259(0x366)]()&&!_0x1f1a3c)return;this['addCommand'](_0x1c0926,_0xe4f259(0xcb),_0x1f1a3c);},Window_ShopCommand['prototype'][_0x39ebd3(0x4a6)]=function(){const _0xc6c1b3=_0x39ebd3;return SceneManager['_scene'][_0xc6c1b3(0x3f4)]===Scene_Shop?SceneManager[_0xc6c1b3(0x47d)][_0xc6c1b3(0x3cb)]>0x0:!![];},Window_ShopCommand['prototype'][_0x39ebd3(0x3a0)]=function(){const _0x48686a=_0x39ebd3,_0x623bb3=this[_0x48686a(0x425)](),_0x215948=VisuMZ[_0x48686a(0x2a9)][_0x48686a(0x237)][_0x48686a(0x3a4)][_0x48686a(0x2af)],_0x1e3b9b=_0x623bb3===_0x48686a(0x30c)?TextManager[_0x48686a(0x1c1)]:_0x48686a(0x316)['format'](_0x215948,TextManager[_0x48686a(0x1c1)]),_0x29f8be=this[_0x48686a(0x3cd)]();if(this['hideDisabledCommands']()&&!_0x29f8be)return;this['addCommand'](_0x1e3b9b,_0x48686a(0x1c1),_0x29f8be);},Window_ShopCommand['prototype'][_0x39ebd3(0x3cd)]=function(){const _0x49407f=_0x39ebd3;return!this[_0x49407f(0x4e6)];},Window_ShopCommand[_0x39ebd3(0x3bc)][_0x39ebd3(0x478)]=function(){const _0x702b1e=_0x39ebd3,_0x1164f4=this['commandStyle'](),_0x2d461d=VisuMZ[_0x702b1e(0x2a9)][_0x702b1e(0x237)][_0x702b1e(0x3a4)][_0x702b1e(0x2d1)],_0x4dc027=VisuMZ[_0x702b1e(0x2a9)][_0x702b1e(0x237)]['ShopScene'][_0x702b1e(0x2f1)],_0x17ff34=_0x1164f4==='text'?_0x4dc027:'\x5cI[%1]%2'[_0x702b1e(0x1da)](_0x2d461d,_0x4dc027);this[_0x702b1e(0x303)](_0x17ff34,_0x702b1e(0x4ed));},Window_ShopCommand[_0x39ebd3(0x3bc)][_0x39ebd3(0x47a)]=function(){const _0x230c55=_0x39ebd3;return VisuMZ[_0x230c55(0x2a9)][_0x230c55(0x237)]['ShopScene'][_0x230c55(0x29d)];},Window_ShopCommand[_0x39ebd3(0x3bc)][_0x39ebd3(0x129)]=function(_0x464975){const _0x1d4d34=_0x39ebd3,_0x3afc9d=this[_0x1d4d34(0xd8)](_0x464975);if(_0x3afc9d===_0x1d4d34(0x2ed))this[_0x1d4d34(0xfb)](_0x464975);else _0x3afc9d===_0x1d4d34(0x132)?this[_0x1d4d34(0x3c5)](_0x464975):Window_HorzCommand['prototype'][_0x1d4d34(0x129)][_0x1d4d34(0x15c)](this,_0x464975);},Window_ShopCommand['prototype'][_0x39ebd3(0x425)]=function(){const _0x141d41=_0x39ebd3;return VisuMZ[_0x141d41(0x2a9)][_0x141d41(0x237)]['ShopScene']['CmdStyle'];},Window_ShopCommand[_0x39ebd3(0x3bc)]['commandStyleCheck']=function(_0x1f8d79){const _0x4afbbe=_0x39ebd3;if(_0x1f8d79<0x0)return _0x4afbbe(0x30c);const _0x18a747=this['commandStyle']();if(_0x18a747!==_0x4afbbe(0x329))return _0x18a747;else{if(this[_0x4afbbe(0x28d)]()>0x0){const _0x9e8dbf=this[_0x4afbbe(0x40f)](_0x1f8d79);if(_0x9e8dbf['match'](/\\I\[(\d+)\]/i)){const _0x1b99f7=this['itemLineRect'](_0x1f8d79),_0x187a1e=this['textSizeEx'](_0x9e8dbf)['width'];return _0x187a1e<=_0x1b99f7[_0x4afbbe(0x417)]?_0x4afbbe(0x2ed):_0x4afbbe(0x132);}}}return _0x4afbbe(0x30c);},Window_ShopCommand[_0x39ebd3(0x3bc)][_0x39ebd3(0xfb)]=function(_0x4a4d9f){const _0x1f7813=_0x39ebd3,_0x9b3329=this[_0x1f7813(0x3d8)](_0x4a4d9f),_0x1a1dc6=this[_0x1f7813(0x40f)](_0x4a4d9f),_0x467eb5=this['textSizeEx'](_0x1a1dc6)[_0x1f7813(0x417)];this[_0x1f7813(0x2e7)](this[_0x1f7813(0x19b)](_0x4a4d9f));const _0x107173=this[_0x1f7813(0x47a)]();if(_0x107173===_0x1f7813(0x3d7))this[_0x1f7813(0x451)](_0x1a1dc6,_0x9b3329['x']+_0x9b3329[_0x1f7813(0x417)]-_0x467eb5,_0x9b3329['y'],_0x467eb5);else{if(_0x107173===_0x1f7813(0x34f)){const _0x53d71c=_0x9b3329['x']+Math[_0x1f7813(0x349)]((_0x9b3329['width']-_0x467eb5)/0x2);this['drawTextEx'](_0x1a1dc6,_0x53d71c,_0x9b3329['y'],_0x467eb5);}else this[_0x1f7813(0x451)](_0x1a1dc6,_0x9b3329['x'],_0x9b3329['y'],_0x467eb5);}},Window_ShopCommand[_0x39ebd3(0x3bc)][_0x39ebd3(0x3c5)]=function(_0x1149da){const _0x4d2cf0=_0x39ebd3;this[_0x4d2cf0(0x40f)](_0x1149da)[_0x4d2cf0(0x145)](/\\I\[(\d+)\]/i);const _0x571400=Number(RegExp['$1'])||0x0,_0x2bd0b1=this[_0x4d2cf0(0x3d8)](_0x1149da),_0x5ca8db=_0x2bd0b1['x']+Math[_0x4d2cf0(0x349)]((_0x2bd0b1[_0x4d2cf0(0x417)]-ImageManager[_0x4d2cf0(0x225)])/0x2),_0x842184=_0x2bd0b1['y']+(_0x2bd0b1[_0x4d2cf0(0x435)]-ImageManager[_0x4d2cf0(0x357)])/0x2;this['drawIcon'](_0x571400,_0x5ca8db,_0x842184);},VisuMZ['ItemsEquipsCore'][_0x39ebd3(0x1b7)]=Window_ShopBuy['prototype'][_0x39ebd3(0x39e)],Window_ShopBuy[_0x39ebd3(0x3bc)][_0x39ebd3(0x39e)]=function(){const _0x3820d6=_0x39ebd3;this[_0x3820d6(0x3c6)](),VisuMZ[_0x3820d6(0x2a9)][_0x3820d6(0x1b7)][_0x3820d6(0x15c)](this);},Window_ShopBuy[_0x39ebd3(0x3bc)]['updateMoneyAmount']=function(){const _0x368564=_0x39ebd3;SceneManager[_0x368564(0x47d)]['constructor']===Scene_Shop&&(this[_0x368564(0x35b)]=SceneManager[_0x368564(0x47d)][_0x368564(0x31b)]());},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x14f)]=Window_ShopBuy[_0x39ebd3(0x3bc)][_0x39ebd3(0x458)],Window_ShopBuy['prototype'][_0x39ebd3(0x458)]=function(_0x5c55e6){const _0x1e357f=_0x39ebd3;if(!_0x5c55e6)return 0x0;let _0x1d17bc=VisuMZ[_0x1e357f(0x2a9)][_0x1e357f(0x14f)][_0x1e357f(0x15c)](this,_0x5c55e6);return Math[_0x1e357f(0x165)](0x0,this['modifiedBuyPriceItemsEquipsCore'](_0x5c55e6,_0x1d17bc));},Window_ShopBuy[_0x39ebd3(0x3bc)][_0x39ebd3(0x3b1)]=function(_0x491c03,_0x51fc74){const _0x17ded8=_0x39ebd3,_0x263df6=_0x491c03[_0x17ded8(0x3ba)]||'';if(_0x263df6[_0x17ded8(0x145)](/<JS BUY PRICE>\s*([\s\S]*)\s*<\/JS BUY PRICE>/i)){const _0x36020e=String(RegExp['$1']);window[_0x17ded8(0x458)]=_0x51fc74,window[_0x17ded8(0x27c)]=_0x491c03;try{eval(_0x36020e);}catch(_0x56c406){if($gameTemp[_0x17ded8(0x253)]())console[_0x17ded8(0x4a0)](_0x56c406);}_0x51fc74=window[_0x17ded8(0x458)],window[_0x17ded8(0x458)]=undefined,window[_0x17ded8(0x27c)]=undefined;}_0x51fc74=VisuMZ[_0x17ded8(0x2a9)][_0x17ded8(0x237)][_0x17ded8(0x3a4)][_0x17ded8(0x363)]['call'](this,_0x491c03,_0x51fc74);if(isNaN(_0x51fc74))_0x51fc74=0x0;return Math[_0x17ded8(0x349)](_0x51fc74);},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x344)]=Window_ShopBuy[_0x39ebd3(0x3bc)]['goodsToItem'],Window_ShopBuy[_0x39ebd3(0x3bc)][_0x39ebd3(0xf6)]=function(_0x4cab99){const _0x2a91be=_0x39ebd3,_0x4857c4=VisuMZ['ItemsEquipsCore'][_0x2a91be(0x344)][_0x2a91be(0x15c)](this,_0x4cab99);return _0x4857c4&&!this[_0x2a91be(0x1a4)](_0x4857c4)?null:_0x4857c4;},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x254)]={'ShowAllSwitches':/<SHOW SHOP (?:ALL |)SWITCH(?:|ES):[ ](.*)>/i,'ShowAnySwitches':/<SHOW SHOP ANY SWITCH(?:|ES):[ ](.*)>/i,'HideAllSwitches':/<HIDE SHOP (?:ALL |)SWITCH(?:|ES):[ ](.*)>/i,'HideAnySwitches':/<HIDE SHOP ANY SWITCH(?:|ES):[ ](.*)>/i,'BuyTurnSwitchOn':/<BUY TURN ON SWITCH(?:|ES):[ ](.*)>/i,'BuyTurnSwitchOff':/<BUY TURN OFF SWITCH(?:|ES):[ ](.*)>/i,'SellTurnSwitchOn':/<SELL TURN ON SWITCH(?:|ES):[ ](.*)>/i,'SellTurnSwitchOff':/<SELL TURN OFF SWITCH(?:|ES):[ ](.*)>/i},Window_ShopBuy[_0x39ebd3(0x3bc)]['meetsShopListingConditions']=function(_0x178c87){const _0x343135=_0x39ebd3;if(!_0x178c87)return![];const _0x3f54e8=VisuMZ[_0x343135(0x2a9)][_0x343135(0x254)],_0x4f0d9a=_0x178c87?_0x178c87[_0x343135(0x3ba)]||'':'';if(_0x4f0d9a['match'](_0x3f54e8[_0x343135(0xb4)])){const _0x147fe2=String(RegExp['$1'])[_0x343135(0x3de)](',')['map'](_0x144ff3=>Number(_0x144ff3));if(_0x147fe2[_0x343135(0x25e)](_0x53da86=>!$gameSwitches[_0x343135(0x1a5)](_0x53da86)))return![];}if(_0x4f0d9a['match'](_0x3f54e8['ShowAnySwitches'])){const _0x546691=String(RegExp['$1'])[_0x343135(0x3de)](',')['map'](_0x1404d6=>Number(_0x1404d6));if(_0x546691['every'](_0x23cb85=>!$gameSwitches[_0x343135(0x1a5)](_0x23cb85)))return![];}if(_0x4f0d9a[_0x343135(0x145)](_0x3f54e8[_0x343135(0x3bd)])){const _0x6848da=String(RegExp['$1'])[_0x343135(0x3de)](',')[_0x343135(0x1e3)](_0x2f698a=>Number(_0x2f698a));if(_0x6848da['every'](_0xdbba75=>$gameSwitches[_0x343135(0x1a5)](_0xdbba75)))return![];}if(_0x4f0d9a['match'](_0x3f54e8[_0x343135(0x384)])){const _0x2f0994=String(RegExp['$1'])[_0x343135(0x3de)](',')[_0x343135(0x1e3)](_0x14797a=>Number(_0x14797a));if(_0x2f0994[_0x343135(0x25e)](_0xa84d73=>$gameSwitches[_0x343135(0x1a5)](_0xa84d73)))return![];}return!![];},Window_ShopBuy[_0x39ebd3(0x3bc)][_0x39ebd3(0x129)]=function(_0x5d52a3){const _0x5a13a4=_0x39ebd3;this[_0x5a13a4(0x3c9)]();const _0x21db93=this[_0x5a13a4(0x4af)](_0x5d52a3),_0x269a58=this[_0x5a13a4(0x3d8)](_0x5d52a3),_0x504995=_0x269a58[_0x5a13a4(0x417)];this[_0x5a13a4(0x2e7)](this[_0x5a13a4(0x4dd)](_0x21db93)),this[_0x5a13a4(0xff)](_0x21db93,_0x269a58['x'],_0x269a58['y'],_0x504995),this[_0x5a13a4(0x23d)](_0x21db93,_0x269a58),this[_0x5a13a4(0x2e7)](!![]);},Window_ShopBuy['prototype'][_0x39ebd3(0x23d)]=function(_0x1c3dcc,_0x30469f){const _0x131516=_0x39ebd3,_0xe37e81=this[_0x131516(0x458)](_0x1c3dcc);this[_0x131516(0x3b7)](_0xe37e81,TextManager[_0x131516(0x43f)],_0x30469f['x'],_0x30469f['y'],_0x30469f[_0x131516(0x417)]);},Window_ShopSell[_0x39ebd3(0x3bc)][_0x39ebd3(0x340)]=function(){const _0x2f8797=_0x39ebd3;return SceneManager[_0x2f8797(0x47d)]['isUseItemsEquipsCoreUpdatedLayout']()?0x1:0x2;},VisuMZ[_0x39ebd3(0x2a9)]['Window_ShopSell_isEnabled']=Window_ShopSell[_0x39ebd3(0x3bc)][_0x39ebd3(0x4dd)],Window_ShopSell['prototype'][_0x39ebd3(0x4dd)]=function(_0x406651){const _0x306bf3=_0x39ebd3;if(!_0x406651)return![];const _0x183f24=_0x406651[_0x306bf3(0x3ba)];if(_0x183f24['match'](/<CANNOT SELL>/i))return![];if(_0x183f24['match'](/<CAN SELL>/i))return!![];if(_0x183f24[_0x306bf3(0x145)](/<CANNOT SELL[ ](?:SW|SWITCH|SWITCHES):[ ]*(\d+(?:\s*,\s*\d+)*)>/i)){const _0x55291a=JSON[_0x306bf3(0x355)]('['+RegExp['$1'][_0x306bf3(0x145)](/\d+/g)+']');for(const _0x3e43a3 of _0x55291a){if(!$gameSwitches[_0x306bf3(0x1a5)](_0x3e43a3))return![];}}if(_0x183f24[_0x306bf3(0x145)](/<CANNOT SELL ALL[ ](?:SW|SWITCH|SWITCHES):[ ]*(\d+(?:\s*,\s*\d+)*)>/i)){const _0x4993fd=JSON[_0x306bf3(0x355)]('['+RegExp['$1'][_0x306bf3(0x145)](/\d+/g)+']');for(const _0x502788 of _0x4993fd){if(!$gameSwitches[_0x306bf3(0x1a5)](_0x502788))return![];}}if(_0x183f24[_0x306bf3(0x145)](/<CANNOT SELL ANY[ ](?:SW|SWITCH|SWITCHES):[ ]*(\d+(?:\s*,\s*\d+)*)>/i)){const _0x3eff52=JSON[_0x306bf3(0x355)]('['+RegExp['$1']['match'](/\d+/g)+']');for(const _0x5defb2 of _0x3eff52){if($gameSwitches['value'](_0x5defb2))return![];}}return VisuMZ[_0x306bf3(0x2a9)]['Window_ShopSell_isEnabled'][_0x306bf3(0x15c)](this,_0x406651);},Window_ShopStatus[_0x39ebd3(0x32c)]=VisuMZ['ItemsEquipsCore'][_0x39ebd3(0x237)][_0x39ebd3(0x1a9)][_0x39ebd3(0x2aa)]??0xf0,VisuMZ[_0x39ebd3(0x2a9)]['Window_ShopStatus_setItem']=Window_ShopStatus[_0x39ebd3(0x3bc)]['setItem'],Window_ShopStatus['prototype'][_0x39ebd3(0x313)]=function(_0x3724e8){const _0x48cf6c=_0x39ebd3;_0x3724e8=DataManager[_0x48cf6c(0x47b)](_0x3724e8),DataManager[_0x48cf6c(0x2db)](_0x3724e8)||DataManager[_0x48cf6c(0x13b)](_0x3724e8)?this['setItemDelay'](_0x3724e8):VisuMZ['ItemsEquipsCore']['Window_ShopStatus_setItem'][_0x48cf6c(0x15c)](this,_0x3724e8);},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x98)]=function(_0xfd46a8){const _0x508f6a=_0x39ebd3;this[_0x508f6a(0x4ee)]=_0xfd46a8;const _0x546854=Window_ShopStatus[_0x508f6a(0x32c)];setTimeout(this[_0x508f6a(0x258)][_0x508f6a(0xdc)](this,_0xfd46a8),_0x546854);},Window_ShopStatus['prototype'][_0x39ebd3(0x258)]=function(_0x40b578){this['_item']===_0x40b578&&this['refresh']();},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x10d)]=function(){return![];},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x155)]=function(){const _0x158719=_0x39ebd3;Window_StatusBase[_0x158719(0x3bc)]['loadFaceImages'][_0x158719(0x15c)](this);for(const _0x172958 of $gameParty[_0x158719(0x106)]()){ImageManager['loadCharacter'](_0x172958['characterName']());}},Window_ShopStatus['prototype'][_0x39ebd3(0x4d5)]=function(){const _0x501505=_0x39ebd3;return VisuMZ[_0x501505(0x2a9)][_0x501505(0x237)][_0x501505(0x1a9)][_0x501505(0x4b2)];},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x39e)]=function(){const _0x1dc544=_0x39ebd3;this[_0x1dc544(0x456)][_0x1dc544(0x39f)](),this['contentsBack'][_0x1dc544(0x39f)](),this[_0x1dc544(0x4ee)]&&(this[_0x1dc544(0x3c9)](),this['changePaintOpacity'](!![]),this[_0x1dc544(0x10e)](),this[_0x1dc544(0x30b)]()?this[_0x1dc544(0xd7)]():this[_0x1dc544(0x1ff)](),this['drawCustomShopGraphic']());},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x4d8)]=function(_0x3ac995,_0x36ce8){const _0x116884=_0x39ebd3;if(!this[_0x116884(0x30b)]()&&!DataManager[_0x116884(0xc2)](this['_item']))return;const _0x317a1d=this[_0x116884(0x35e)]-this[_0x116884(0x447)]()-_0x3ac995,_0x1f8203=this['textWidth'](_0x116884(0x220));this[_0x116884(0x4ca)](ColorManager[_0x116884(0x163)]()),this[_0x116884(0x1b9)](TextManager[_0x116884(0x381)],_0x3ac995+this[_0x116884(0x447)](),_0x36ce8,_0x317a1d-_0x1f8203),this[_0x116884(0x1e5)](),this[_0x116884(0x3c2)](this['_item'],_0x3ac995,_0x36ce8,_0x317a1d);},Window_ShopStatus[_0x39ebd3(0x3bc)]['drawItemDarkRect']=function(_0xa4ee48,_0x2019d6,_0x4aa26b,_0x25145b,_0x7ef0f1){const _0x472cc4=_0x39ebd3;if(VisuMZ[_0x472cc4(0x2a9)][_0x472cc4(0x237)][_0x472cc4(0x1a9)][_0x472cc4(0x216)]===![])return;_0x7ef0f1=Math['max'](_0x7ef0f1||0x1,0x1);while(_0x7ef0f1--){_0x25145b=_0x25145b||this[_0x472cc4(0x464)](),this[_0x472cc4(0x1bb)]['paintOpacity']=0xa0;const _0x5d9712=ColorManager['getItemsEquipsCoreBackColor1']();this[_0x472cc4(0x1bb)][_0x472cc4(0x400)](_0xa4ee48+0x1,_0x2019d6+0x1,_0x4aa26b-0x2,_0x25145b-0x2,_0x5d9712),this[_0x472cc4(0x1bb)][_0x472cc4(0x31a)]=0xff;}},ColorManager[_0x39ebd3(0xf0)]=function(){const _0xe8ac50=_0x39ebd3,_0x259ac9=VisuMZ[_0xe8ac50(0x2a9)][_0xe8ac50(0x237)][_0xe8ac50(0x1a9)];let _0x463b31=_0x259ac9[_0xe8ac50(0x231)]!==undefined?_0x259ac9['BackRectColor']:0x13;return ColorManager[_0xe8ac50(0x47c)](_0x463b31);},Window_ShopStatus[_0x39ebd3(0x3bc)]['drawEquipData']=function(){const _0x33667e=_0x39ebd3,_0x45169b=this[_0x33667e(0x170)]();if(_0x45169b===_0x33667e(0xc7))this[_0x33667e(0x19a)]();else _0x45169b===_0x33667e(0x3dc)?this[_0x33667e(0x326)]():this[_0x33667e(0x256)]();},Window_ShopStatus['prototype']['getEquipDataStyle']=function(){const _0x57175e=_0x39ebd3;let _0x3e12bc=VisuMZ[_0x57175e(0x2a9)]['Settings'][_0x57175e(0x1a9)][_0x57175e(0x396)]??'compare';const _0x436b16=/<STATUS STYLE:[ ](.*)>/i;return this[_0x57175e(0x4ee)]&&this[_0x57175e(0x4ee)][_0x57175e(0x3ba)]&&this[_0x57175e(0x4ee)]['note'][_0x57175e(0x145)](_0x436b16)&&(_0x3e12bc=String(RegExp['$1'])['toLowerCase']()['trim']()),_0x3e12bc;},Window_ShopStatus[_0x39ebd3(0x3bc)]['drawEquipDataCompare']=function(){const _0x15d2df=_0x39ebd3;this[_0x15d2df(0x119)]=null;if(VisuMZ[_0x15d2df(0x2a9)][_0x15d2df(0x237)][_0x15d2df(0x1a9)][_0x15d2df(0x270)]){VisuMZ[_0x15d2df(0x2a9)][_0x15d2df(0x237)][_0x15d2df(0x1a9)][_0x15d2df(0x270)][_0x15d2df(0x15c)](this);return;}const _0x5f03ad=this[_0x15d2df(0x464)](),_0x2d0f88=this[_0x15d2df(0x449)]()+0x8;let _0x4d283b=0x0,_0x424e96=0x0,_0x8f5e2c=this[_0x15d2df(0x35e)],_0x415700=this['innerHeight'],_0x2dae5e=Math[_0x15d2df(0x349)](_0x8f5e2c/0x2),_0x309c06=_0x4d283b+_0x8f5e2c-_0x2dae5e;this[_0x15d2df(0xff)](this['_item'],_0x4d283b+this['itemPadding'](),_0x424e96,_0x8f5e2c-this[_0x15d2df(0x447)]()*0x2),this['drawItemDarkRect'](_0x4d283b,_0x424e96,_0x8f5e2c),_0x424e96+=_0x5f03ad;if(this[_0x15d2df(0x304)](_0x4d283b,_0x424e96,_0x2dae5e))_0x424e96+=0x0;if(this['drawItemQuantity'](_0x309c06,_0x424e96,_0x2dae5e))_0x424e96+=_0x5f03ad;const _0x10cdbc=this['actorParams'](),_0x232b14=_0x424e96;_0x424e96=_0x415700-_0x10cdbc[_0x15d2df(0x142)]*_0x2d0f88-0x4;let _0x2b392c=_0x4d283b,_0x9e12c3=0x0,_0x585acd=_0x424e96;for(const _0x4f8094 of _0x10cdbc){_0x9e12c3=Math[_0x15d2df(0x165)](this[_0x15d2df(0x227)](_0x4f8094,_0x4d283b+0x4,_0x424e96+0x4,_0x8f5e2c),_0x9e12c3),_0x424e96+=_0x2d0f88;}const _0x65b357=$gameParty[_0x15d2df(0x37f)](),_0x103749=Math[_0x15d2df(0x349)]((_0x8f5e2c-_0x9e12c3)/_0x65b357);_0x9e12c3=_0x8f5e2c-_0x103749*_0x65b357;for(const _0x5eb1d9 of $gameParty[_0x15d2df(0x43c)]()){const _0x399438=$gameParty['battleMembers']()['indexOf'](_0x5eb1d9),_0x4535c8=_0x2b392c+_0x9e12c3+_0x399438*_0x103749;this[_0x15d2df(0x2e7)](_0x5eb1d9[_0x15d2df(0x1d6)](this['_item'])),this[_0x15d2df(0x38b)](_0x5eb1d9,_0x4535c8+_0x103749/0x2,_0x585acd);let _0x393454=_0x585acd;for(const _0x5d8fec of _0x10cdbc){const _0x3886ea=_0x393454-(_0x5f03ad-_0x2d0f88)/0x2;this['drawActorParamDifference'](_0x5eb1d9,_0x5d8fec,_0x4535c8,_0x3886ea,_0x103749),_0x393454+=_0x2d0f88;}}this[_0x15d2df(0x25c)](_0x2b392c,_0x232b14,_0x9e12c3,_0x585acd-_0x232b14);for(let _0x7df2d2=0x0;_0x7df2d2<_0x65b357;_0x7df2d2++){const _0x4aa029=_0x2b392c+_0x9e12c3+_0x7df2d2*_0x103749;this[_0x15d2df(0x25c)](_0x4aa029,_0x232b14,_0x103749,_0x585acd-_0x232b14);}for(const _0x229280 of _0x10cdbc){this['drawItemDarkRect'](_0x2b392c,_0x585acd,_0x9e12c3,_0x2d0f88);for(let _0x50ae43=0x0;_0x50ae43<_0x65b357;_0x50ae43++){const _0x3a15bc=_0x2b392c+_0x9e12c3+_0x50ae43*_0x103749;this[_0x15d2df(0x25c)](_0x3a15bc,_0x585acd,_0x103749,_0x2d0f88);}_0x585acd+=_0x2d0f88;}},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x256)]=function(){const _0x1ecb94=_0x39ebd3;this['_tempActor']=null;if(VisuMZ['ItemsEquipsCore'][_0x1ecb94(0x237)][_0x1ecb94(0x1a9)][_0x1ecb94(0x189)]){VisuMZ[_0x1ecb94(0x2a9)]['Settings'][_0x1ecb94(0x1a9)]['DrawEquipClassicData'][_0x1ecb94(0x15c)](this);return;}const _0x48e92f=this[_0x1ecb94(0x464)]();let _0x19414b=0x0,_0x4529ec=0x0,_0x4f3aab=this['innerWidth'],_0x69f758=this[_0x1ecb94(0x4e5)],_0x5477a9=Math[_0x1ecb94(0x349)](_0x4f3aab/0x2),_0x361df5=_0x19414b+_0x4f3aab-_0x5477a9;this['drawItemName'](this[_0x1ecb94(0x4ee)],_0x19414b+this[_0x1ecb94(0x447)](),_0x4529ec,_0x4f3aab-this['itemPadding']()*0x2),this['drawItemDarkRect'](_0x19414b,_0x4529ec,_0x4f3aab),_0x4529ec+=_0x48e92f;if(this[_0x1ecb94(0x304)](_0x19414b,_0x4529ec,_0x5477a9))_0x4529ec+=0x0;if(this[_0x1ecb94(0x133)](_0x361df5,_0x4529ec,_0x5477a9))_0x4529ec+=_0x48e92f;if(this[_0x1ecb94(0xa9)](_0x19414b,_0x4529ec,_0x4f3aab))_0x4529ec+=_0x48e92f;const _0x3a733f=this[_0x1ecb94(0x39d)]();for(const _0x5291d3 of _0x3a733f){if(this[_0x1ecb94(0x3ec)](_0x5291d3))continue;this[_0x1ecb94(0x383)](_0x5291d3,_0x19414b,_0x4529ec,_0x4f3aab),_0x4529ec+=_0x48e92f;}_0x4529ec=this[_0x1ecb94(0x4a9)](_0x19414b,_0x4529ec,_0x4f3aab),this[_0x1ecb94(0x25c)](_0x19414b,_0x4529ec,_0x4f3aab,_0x69f758-_0x4529ec);},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x326)]=function(){const _0x40b330=_0x39ebd3;this[_0x40b330(0x119)]=null;if(VisuMZ[_0x40b330(0x2a9)]['Settings'][_0x40b330(0x1a9)][_0x40b330(0x3c0)]){VisuMZ['ItemsEquipsCore'][_0x40b330(0x237)][_0x40b330(0x1a9)]['DrawEquipDoubleData'][_0x40b330(0x15c)](this);return;}const _0x4851d1=this[_0x40b330(0x464)]();let _0x536730=0x0,_0x863258=0x0,_0x500ec6=this[_0x40b330(0x35e)],_0x308c28=this[_0x40b330(0x4e5)],_0x42e3c1=Math[_0x40b330(0x349)](_0x500ec6/0x2),_0x3c2092=_0x536730+_0x500ec6-_0x42e3c1;this[_0x40b330(0xff)](this[_0x40b330(0x4ee)],_0x536730+this['itemPadding'](),_0x863258,_0x500ec6-this['itemPadding']()*0x2),this['drawItemDarkRect'](_0x536730,_0x863258,_0x500ec6),_0x863258+=_0x4851d1;if(this[_0x40b330(0x304)](_0x536730,_0x863258,_0x42e3c1))_0x863258+=0x0;if(this['drawItemQuantity'](_0x3c2092,_0x863258,_0x42e3c1))_0x863258+=_0x4851d1;if(this[_0x40b330(0xa9)](_0x536730,_0x863258,_0x500ec6))_0x863258+=_0x4851d1;const _0x4992d9=this[_0x40b330(0x39d)]();for(const _0xad0489 of _0x4992d9){if(this[_0x40b330(0x3ec)](_0xad0489))continue;this[_0x40b330(0x383)](_0xad0489,_0x536730,_0x863258,_0x42e3c1),_0x536730===_0x42e3c1?(_0x863258+=_0x4851d1,_0x536730=0x0):_0x536730=_0x42e3c1;}_0x536730===_0x42e3c1&&(this[_0x40b330(0x25c)](_0x42e3c1,_0x863258,_0x42e3c1,_0x4851d1),_0x863258+=_0x4851d1,_0x536730=0x0),_0x863258=this[_0x40b330(0x4a9)](_0x536730,_0x863258,_0x500ec6),this[_0x40b330(0x25c)](_0x536730,_0x863258,_0x500ec6,_0x308c28-_0x863258);},Window_ShopStatus['prototype'][_0x39ebd3(0x304)]=function(_0x4042c4,_0x127cbb,_0x51d576){const _0x10733e=_0x39ebd3;if(!this['isEquipItem']())return![];const _0x79dc0f=$dataSystem[_0x10733e(0x1ce)][this[_0x10733e(0x4ee)]['etypeId']];return this['drawItemKeyData'](_0x79dc0f,_0x4042c4,_0x127cbb,_0x51d576,!![]),this[_0x10733e(0x25c)](_0x4042c4,_0x127cbb,_0x51d576),this[_0x10733e(0x3c9)](),!![];},Window_ShopStatus[_0x39ebd3(0x3bc)]['drawItemEquipSubType']=function(_0x38ac90,_0x2599b8,_0xefb64b){const _0x30fece=_0x39ebd3;if(!this[_0x30fece(0x30b)]())return![];let _0x2771bf='',_0x449604='';const _0x9eb7c2=VisuMZ['ItemsEquipsCore']['Settings'][_0x30fece(0x1a9)];return DataManager[_0x30fece(0x2db)](this[_0x30fece(0x4ee)])?(_0x2771bf=_0x9eb7c2[_0x30fece(0x27b)]??_0x30fece(0x296),_0x449604=$dataSystem[_0x30fece(0x40c)][this[_0x30fece(0x4ee)]['wtypeId']]||_0x9eb7c2[_0x30fece(0x25d)]||'-'):(_0x2771bf=_0x9eb7c2['ArmorType']??_0x30fece(0x3b6),_0x449604=$dataSystem[_0x30fece(0x16c)][this[_0x30fece(0x4ee)][_0x30fece(0x31e)]]||_0x9eb7c2[_0x30fece(0x25d)]||'-'),this['drawItemKeyData'](_0x2771bf,_0x38ac90,_0x2599b8,_0xefb64b,!![]),this[_0x30fece(0x319)](_0x449604,_0x38ac90,_0x2599b8,_0xefb64b,![],_0x30fece(0x3d7)),this[_0x30fece(0x25c)](_0x38ac90,_0x2599b8,_0xefb64b),this['resetFontSettings'](),!![];},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x37c)]=function(){const _0x6dac6e=_0x39ebd3,_0x5519e8=VisuMZ[_0x6dac6e(0x2a9)][_0x6dac6e(0x237)]['ItemScene'][_0x6dac6e(0x378)];return _0x5519e8[_0x6dac6e(0x1da)]($gameParty[_0x6dac6e(0x410)](this['_item']));},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x39d)]=function(){const _0x37f3f1=_0x39ebd3;let _0x32ebbd=[0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7];if(Imported[_0x37f3f1(0x372)]){_0x32ebbd=VisuMZ['CoreEngine'][_0x37f3f1(0x237)]['Param'][_0x37f3f1(0x2a8)];if(this[_0x37f3f1(0x387)]())return this[_0x37f3f1(0x386)]();const _0x5e9f4f=VisuMZ[_0x37f3f1(0x2a9)][_0x37f3f1(0x237)][_0x37f3f1(0x1a9)];if(this[_0x37f3f1(0x170)]()===_0x37f3f1(0x32a)){if(DataManager[_0x37f3f1(0x2db)](this['_item']))_0x32ebbd=_0x32ebbd[_0x37f3f1(0x1fe)](_0x5e9f4f[_0x37f3f1(0x239)]||[]);if(DataManager[_0x37f3f1(0x13b)](this[_0x37f3f1(0x4ee)]))_0x32ebbd=_0x32ebbd['concat'](_0x5e9f4f[_0x37f3f1(0xe5)]||[]);}else{if(this[_0x37f3f1(0x170)]()===_0x37f3f1(0x3dc)){if(DataManager[_0x37f3f1(0x2db)](this[_0x37f3f1(0x4ee)]))_0x32ebbd=_0x32ebbd[_0x37f3f1(0x1fe)](_0x5e9f4f[_0x37f3f1(0x365)]||[]);if(DataManager['isArmor'](this['_item']))_0x32ebbd=_0x32ebbd[_0x37f3f1(0x1fe)](_0x5e9f4f[_0x37f3f1(0x32e)]||[]);}}}return _0x32ebbd=_0x32ebbd[_0x37f3f1(0x1e3)](_0x54bd34=>typeof _0x54bd34===_0x37f3f1(0x1ca)?_0x54bd34:_0x54bd34[_0x37f3f1(0x34e)]()[_0x37f3f1(0x206)]()),_0x32ebbd;},Window_ShopStatus['prototype'][_0x39ebd3(0x97)]=function(){const _0xb28ec7=_0x39ebd3;return VisuMZ['ItemsEquipsCore'][_0xb28ec7(0x237)][_0xb28ec7(0x1a9)]['ParamChangeFontSize'];},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x227)]=function(_0x313139,_0x4cdcab,_0x1feef8,_0x1a66a7){const _0x1182ec=_0x39ebd3;this[_0x1182ec(0x3c9)](),this[_0x1182ec(0x456)]['fontSize']=this[_0x1182ec(0x97)]();let _0x5b0cd5=this['textWidth'](TextManager[_0x1182ec(0x154)](_0x313139))+0x4+_0x4cdcab;return Imported['VisuMZ_0_CoreEngine']?(this['drawParamText'](_0x4cdcab,_0x1feef8,_0x1a66a7,_0x313139,!![]),VisuMZ[_0x1182ec(0x1e7)]['Settings'][_0x1182ec(0x42e)][_0x1182ec(0x234)]&&(_0x5b0cd5+=ImageManager['iconWidth']+0x4)):(this[_0x1182ec(0x4ca)](ColorManager['systemColor']()),this['drawText'](TextManager[_0x1182ec(0x154)](_0x313139),_0x4cdcab,_0x1feef8,_0x1a66a7)),this[_0x1182ec(0x3c9)](),_0x5b0cd5;},Window_ShopStatus[_0x39ebd3(0x3bc)]['drawActorParamDifference']=function(_0x38a42e,_0x2573d4,_0x5c353f,_0x4905a7,_0x397253){const _0x3e56b5=_0x39ebd3;_0x5c353f+=this[_0x3e56b5(0x447)](),_0x397253-=this[_0x3e56b5(0x447)]()*0x2;const _0x33aa51=VisuMZ['ItemsEquipsCore'][_0x3e56b5(0x237)][_0x3e56b5(0x1a9)];this[_0x3e56b5(0x456)][_0x3e56b5(0x2e9)]=_0x33aa51[_0x3e56b5(0x433)],this[_0x3e56b5(0x2e7)](_0x38a42e['canEquip'](this[_0x3e56b5(0x4ee)]));if(_0x38a42e[_0x3e56b5(0x1f2)](this[_0x3e56b5(0x4ee)])&&!_0x38a42e[_0x3e56b5(0x2a3)](this[_0x3e56b5(0x4ee)])){const _0xc7bf87=_0x33aa51[_0x3e56b5(0x3e0)];this[_0x3e56b5(0x1b9)](_0xc7bf87,_0x5c353f,_0x4905a7,_0x397253,_0x3e56b5(0x34f));}else{if(_0x38a42e[_0x3e56b5(0x1d6)](this[_0x3e56b5(0x4ee)])){const _0x3b41ef=this[_0x3e56b5(0x174)](_0x38a42e);let _0x2b5e20=0x0,_0xa7b575=0x0,_0x3af1dc=0x0;Imported[_0x3e56b5(0x372)]?(_0x2b5e20=_0x3b41ef[_0x3e56b5(0x192)](_0x2573d4),_0xa7b575=_0x2b5e20-_0x38a42e[_0x3e56b5(0x192)](_0x2573d4),this[_0x3e56b5(0x4ca)](ColorManager[_0x3e56b5(0x4bc)](_0xa7b575)),_0x3af1dc=(_0xa7b575>=0x0?'+':'')+VisuMZ[_0x3e56b5(0x125)](_0xa7b575,0x0,_0x2573d4)):(_0x2b5e20=_0x3b41ef[_0x3e56b5(0x154)](_0x2573d4),_0xa7b575=_0x2b5e20-_0x38a42e[_0x3e56b5(0x154)](_0x2573d4),this[_0x3e56b5(0x4ca)](ColorManager[_0x3e56b5(0x4bc)](_0xa7b575)),_0x3af1dc=(_0xa7b575>=0x0?'+':'')+_0xa7b575),_0x3af1dc==='+0'&&(_0x3af1dc=_0x33aa51[_0x3e56b5(0x314)]),this[_0x3e56b5(0x1b9)](_0x3af1dc,_0x5c353f,_0x4905a7,_0x397253,_0x3e56b5(0x34f));}else{const _0x2ab6b7=_0x33aa51[_0x3e56b5(0x24b)];this['drawText'](_0x2ab6b7,_0x5c353f,_0x4905a7,_0x397253,'center');}}this[_0x3e56b5(0x3c9)](),this['changePaintOpacity'](!![]);},Window_ShopStatus['prototype']['createTempActorEquips']=function(_0x5c8116){const _0x4a3ca4=_0x39ebd3;if(this['needsNewTempActor'](_0x5c8116)){const _0x280bf1=JsonEx[_0x4a3ca4(0x4e8)](_0x5c8116);_0x280bf1[_0x4a3ca4(0x119)]=!![];const _0x4cb5ee=_0x280bf1[_0x4a3ca4(0x476)](this['_item']);_0x4cb5ee>=0x0&&_0x280bf1[_0x4a3ca4(0xb0)](_0x4cb5ee,this[_0x4a3ca4(0x4ee)]),this[_0x4a3ca4(0x119)]=_0x280bf1;}return this['_tempActor'];},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x232)]=function(_0x3f0ca3){const _0x17f1f8=_0x39ebd3;if(!this[_0x17f1f8(0x119)])return!![];return this['_tempActor']['actorId']()!==_0x3f0ca3[_0x17f1f8(0xfe)]();},Game_Actor[_0x39ebd3(0x3bc)][_0x39ebd3(0x2a3)]=function(_0x3decc2){const _0x339159=_0x39ebd3;if(!_0x3decc2)return![];const _0x4c8734=_0x3decc2[_0x339159(0x34a)],_0x32782f=this[_0x339159(0x18e)]();for(let _0x11f0f6=0x0;_0x11f0f6<_0x32782f[_0x339159(0x142)];_0x11f0f6++){const _0x50b189=_0x32782f[_0x11f0f6];if(_0x50b189!==_0x4c8734)continue;if(!this['equips']()[_0x11f0f6])return!![];}return![];},Game_Actor[_0x39ebd3(0x3bc)][_0x39ebd3(0x476)]=function(_0x4e87a1){const _0x1e1236=_0x39ebd3;if(!_0x4e87a1)return-0x1;const _0x5c6544=_0x4e87a1[_0x1e1236(0x34a)],_0x169b4b=this[_0x1e1236(0x18e)]();let _0x3a45cd=-0x1;for(let _0x10a7e9=0x0;_0x10a7e9<_0x169b4b[_0x1e1236(0x142)];_0x10a7e9++){const _0x251515=_0x169b4b[_0x10a7e9];if(_0x251515!==_0x5c6544)continue;if(!this[_0x1e1236(0x257)]()[_0x10a7e9])return _0x10a7e9;if(_0x3a45cd<0x0)_0x3a45cd=_0x10a7e9;}return _0x3a45cd;},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x383)]=function(_0x5d60e4,_0x270757,_0x1f6781,_0x2c1e5f){const _0x31d8e8=_0x39ebd3,_0x1c96d2=TextManager['param'](_0x5d60e4);this['drawItemKeyData'](_0x1c96d2,_0x270757,_0x1f6781,_0x2c1e5f,!![]);let _0x276821='+0';Imported[_0x31d8e8(0x372)]?_0x276821=this[_0x31d8e8(0x1b2)](_0x5d60e4):_0x276821=this[_0x31d8e8(0x24f)](_0x5d60e4),this['drawItemKeyData'](_0x276821,_0x270757,_0x1f6781,_0x2c1e5f,![],'right'),this[_0x31d8e8(0x25c)](_0x270757,_0x1f6781,_0x2c1e5f);},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x3ec)]=function(_0x1f04fc){const _0x5a4af7=_0x39ebd3;return Imported[_0x5a4af7(0x372)]?!!VisuMZ['CoreEngine'][_0x5a4af7(0x248)][_0x1f04fc]:![];},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x1b2)]=function(_0x5692fe){const _0x1d0024=_0x39ebd3;if(this[_0x1d0024(0x3f7)][_0x5692fe])return this[_0x1d0024(0x3f7)][_0x5692fe];const _0x5dd39d=[_0x1d0024(0x2f5),'MAXMP',_0x1d0024(0x276),_0x1d0024(0xb6),'MAT',_0x1d0024(0x4d4),_0x1d0024(0x294),_0x1d0024(0x44a)],_0x207008=[_0x1d0024(0xb8),_0x1d0024(0x41d),_0x1d0024(0x48c),'CEV',_0x1d0024(0x468),_0x1d0024(0x2a2),_0x1d0024(0x48a),_0x1d0024(0x2c7),_0x1d0024(0x4ec),_0x1d0024(0x1c9)],_0x5d49e3=[_0x1d0024(0x40b),_0x1d0024(0x487),_0x1d0024(0x3b0),_0x1d0024(0x18b),_0x1d0024(0x1ea),_0x1d0024(0x431),_0x1d0024(0x169),_0x1d0024(0x1d4),_0x1d0024(0x150),'EXR'];if(_0x5dd39d['includes'](_0x5692fe)){const _0x1ae20b=_0x5dd39d['indexOf'](_0x5692fe),_0x502694=this[_0x1d0024(0x4ee)]['params'][_0x1ae20b]||0x0;return this[_0x1d0024(0x4ca)](ColorManager[_0x1d0024(0x4bc)](_0x502694)),(_0x502694>=0x0?'+':'')+String(_0x502694);}else{if(_0x207008[_0x1d0024(0x35f)](_0x5692fe)){const _0x2f8662=_0x207008[_0x1d0024(0x180)](_0x5692fe);let _0xc1a33e=0x0;for(const _0x211f6f of this[_0x1d0024(0x4ee)][_0x1d0024(0x99)]){if(_0x211f6f[_0x1d0024(0x26c)]!==0x16)continue;_0x211f6f[_0x1d0024(0x1cd)]===_0x2f8662&&(_0xc1a33e+=_0x211f6f[_0x1d0024(0x1a5)]||0x0);}return this[_0x1d0024(0x4ca)](ColorManager[_0x1d0024(0x4bc)](_0xc1a33e)),_0xc1a33e*=0x64,(_0xc1a33e>=0x0?'+':'')+String(_0xc1a33e)+'%';}else{if(_0x5d49e3[_0x1d0024(0x35f)](_0x5692fe)){const _0x5b8c04=_0x5d49e3[_0x1d0024(0x180)](_0x5692fe);let _0x5e943b=0x1;for(const _0x5f2b2b of this[_0x1d0024(0x4ee)][_0x1d0024(0x99)]){if(_0x5f2b2b[_0x1d0024(0x26c)]!==0x17)continue;_0x5f2b2b[_0x1d0024(0x1cd)]===_0x5b8c04&&(_0x5e943b*=_0x5f2b2b['value']||0x0);}let _0x1739c4=0x1;if(['TGR','MCR',_0x1d0024(0x169),_0x1d0024(0x1d4),_0x1d0024(0x150)][_0x1d0024(0x35f)](_0x5692fe))_0x1739c4=-0x1;return this[_0x1d0024(0x4ca)](ColorManager[_0x1d0024(0x4bc)]((_0x5e943b-0x1)*_0x1739c4)),_0x5e943b*=0x64,String(_0x5e943b)+'%';}}}return'';},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x24f)]=function(_0x3b3455){const _0x4c2abe=_0x39ebd3,_0x17ec9c=[_0x4c2abe(0x2f5),_0x4c2abe(0x18a),'ATK',_0x4c2abe(0xb6),'MAT',_0x4c2abe(0x4d4),_0x4c2abe(0x294),'LUK'],_0x277715=_0x17ec9c[_0x3b3455]||_0x4c2abe(0xe0);if(this[_0x4c2abe(0x3f7)][_0x277715])return this[_0x4c2abe(0x3f7)][_0x277715];const _0x429e19=Number(this['_item'][_0x4c2abe(0x413)][_0x3b3455]||0x0);return this['changeTextColor'](ColorManager[_0x4c2abe(0x4bc)](_0x429e19)),(_0x429e19>=0x0?'+':'')+String(_0x429e19);},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x387)]=function(){const _0x177a33=_0x39ebd3,_0x1e0c26=/<CUSTOM STATUS PARAM(?:|S|ETERS):[ ](.*)>/i;return this[_0x177a33(0x4ee)]&&this[_0x177a33(0x4ee)][_0x177a33(0x3ba)]&&this[_0x177a33(0x4ee)]['note'][_0x177a33(0x145)](_0x1e0c26);},Window_ShopStatus['prototype'][_0x39ebd3(0x386)]=function(){const _0x5f0eb2=_0x39ebd3,_0x3c94c7=/<CUSTOM STATUS PARAM(?:|S|ETERS):[ ](.*)>/i;this[_0x5f0eb2(0x4ee)][_0x5f0eb2(0x3ba)][_0x5f0eb2(0x145)](_0x3c94c7);const _0x34d006=String(RegExp['$1'])[_0x5f0eb2(0x3de)](',')[_0x5f0eb2(0x1e3)](_0x360f29=>_0x360f29[_0x5f0eb2(0x34e)]()[_0x5f0eb2(0x206)]()),_0x3246c7=[_0x5f0eb2(0x2f5),_0x5f0eb2(0x18a),_0x5f0eb2(0x276),_0x5f0eb2(0xb6),'MAT','MDF',_0x5f0eb2(0x294),_0x5f0eb2(0x44a)],_0x1497c6=[_0x5f0eb2(0xb8),_0x5f0eb2(0x41d),_0x5f0eb2(0x48c),_0x5f0eb2(0x17e),_0x5f0eb2(0x468),_0x5f0eb2(0x2a2),'CNT',_0x5f0eb2(0x2c7),_0x5f0eb2(0x4ec),_0x5f0eb2(0x1c9)],_0x228fad=[_0x5f0eb2(0x40b),'GRD',_0x5f0eb2(0x3b0),_0x5f0eb2(0x18b),_0x5f0eb2(0x1ea),_0x5f0eb2(0x431),'PDR',_0x5f0eb2(0x1d4),'FDR',_0x5f0eb2(0xab)];let _0x58ff90=[];for(const _0x30f1cd of _0x34d006){if(_0x3246c7[_0x5f0eb2(0x35f)](_0x30f1cd))_0x58ff90[_0x5f0eb2(0x17a)](_0x30f1cd);if(_0x1497c6[_0x5f0eb2(0x35f)](_0x30f1cd))_0x58ff90['push'](_0x30f1cd);if(_0x228fad[_0x5f0eb2(0x35f)](_0x30f1cd))_0x58ff90['push'](_0x30f1cd);}return _0x58ff90;},Window_ShopStatus['prototype'][_0x39ebd3(0x1ff)]=function(){const _0x45198c=_0x39ebd3;VisuMZ[_0x45198c(0x2a9)][_0x45198c(0x237)][_0x45198c(0x1a9)][_0x45198c(0xe7)][_0x45198c(0x15c)](this);},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0xff)]=function(_0x3141ad,_0x5ca079,_0x4c27f3,_0xaf6dd9){const _0x319997=_0x39ebd3,_0x1239ba=DataManager[_0x319997(0x13e)](_0x3141ad,_0x5ca079,_0x4c27f3,_0xaf6dd9)&&Imported['VisuMZ_1_SkillsStatesCore'],_0x3c57ef=_0x3141ad?_0x3141ad['name']:'';if(_0x1239ba)Window_SkillList[_0x319997(0x3bc)][_0x319997(0x30d)][_0x319997(0x15c)](this,_0x3141ad);Window_Base[_0x319997(0x3bc)][_0x319997(0xff)][_0x319997(0x15c)](this,_0x3141ad,_0x5ca079,_0x4c27f3,_0xaf6dd9);if(_0x1239ba)_0x3141ad[_0x319997(0x292)]=_0x3c57ef;},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x10e)]=function(){const _0x3f29ee=_0x39ebd3;this[_0x3f29ee(0x3f7)]={};if(!this[_0x3f29ee(0x4ee)])return;const _0x4a022c=this[_0x3f29ee(0x4ee)][_0x3f29ee(0x3ba)];if(_0x4a022c[_0x3f29ee(0x145)](/<STATUS INFO>\s*([\s\S]*)\s*<\/STATUS INFO>/i)){const _0x4c3ee5=String(RegExp['$1'])[_0x3f29ee(0x3de)](/[\r\n]+/);for(const _0x48ea67 of _0x4c3ee5){if(_0x48ea67['match'](/(.*):[ ](.*)/i)){const _0x3fa8e2=String(RegExp['$1'])[_0x3f29ee(0x34e)]()[_0x3f29ee(0x206)](),_0x45490d=String(RegExp['$2'])[_0x3f29ee(0x206)]();this[_0x3f29ee(0x3f7)][_0x3fa8e2]=_0x45490d;}}}},Window_ShopStatus['prototype']['itemDataFontSize']=function(){const _0x4207f8=_0x39ebd3;return Math['max'](0x1,$gameSystem[_0x4207f8(0xc9)]()-0x4);},Window_ShopStatus['prototype'][_0x39ebd3(0x3c9)]=function(){const _0x5ee598=_0x39ebd3;Window_StatusBase[_0x5ee598(0x3bc)][_0x5ee598(0x3c9)][_0x5ee598(0x15c)](this),this[_0x5ee598(0x456)][_0x5ee598(0x2e9)]=this[_0x5ee598(0x450)]||this[_0x5ee598(0x456)]['fontSize'],this[_0x5ee598(0x456)][_0x5ee598(0x379)]=this[_0x5ee598(0x4c8)]||this['contents']['textColor'];},Window_ShopStatus['prototype'][_0x39ebd3(0x474)]=function(){const _0x18a6c3=_0x39ebd3;return this[_0x18a6c3(0x456)]['fontSize']/$gameSystem[_0x18a6c3(0xc9)]();},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x4c9)]=function(_0x1371bd,_0x1fa79d,_0x5f1f0b){const _0x5324f4=_0x39ebd3,_0x1af0d7=ImageManager['loadSystem'](_0x5324f4(0x4b0)),_0x513e14=ImageManager[_0x5324f4(0x225)],_0x54416f=ImageManager[_0x5324f4(0x357)],_0x522ebe=_0x1371bd%0x10*_0x513e14,_0x2c6e45=Math[_0x5324f4(0x349)](_0x1371bd/0x10)*_0x54416f,_0x469927=Math[_0x5324f4(0x3da)](_0x513e14*this[_0x5324f4(0x474)]()),_0x1a0e91=Math[_0x5324f4(0x3da)](_0x54416f*this['fontSizeRatio']());this['contents'][_0x5324f4(0x199)](_0x1af0d7,_0x522ebe,_0x2c6e45,_0x513e14,_0x54416f,_0x1fa79d,_0x5f1f0b,_0x469927,_0x1a0e91);},Window_ShopStatus[_0x39ebd3(0x3bc)]['processDrawIcon']=function(_0x424a7f,_0x223625){const _0x353e50=_0x39ebd3,_0x3a5699=ImageManager['standardIconWidth']||0x20,_0x4d4276=ImageManager[_0x353e50(0x3ea)]||0x20;if(_0x223625[_0x353e50(0x3f9)]){const _0x5be8df=_0x3a5699-ImageManager['iconWidth'],_0x5908b0=_0x4d4276-ImageManager[_0x353e50(0x357)];let _0x500ca7=0x2,_0x4a9177=0x2;this[_0x353e50(0x464)]()!==0x24&&(_0x4a9177=Math[_0x353e50(0x349)]((this[_0x353e50(0x464)]()-_0x4d4276)/0x2));const _0xed6e4f=_0x223625['x']+Math[_0x353e50(0x3da)](Math[_0x353e50(0x349)](_0x5be8df/0x2)*this[_0x353e50(0x474)]())+_0x500ca7,_0x19d2c1=_0x223625['y']+Math['ceil'](Math[_0x353e50(0x349)](_0x5908b0/0x2)*this[_0x353e50(0x474)]())+_0x4a9177;this[_0x353e50(0x4c9)](_0x424a7f,_0xed6e4f,_0x19d2c1);}_0x223625['x']+=Math['ceil'](_0x3a5699*this[_0x353e50(0x474)]()),_0x223625['x']+=Math[_0x353e50(0x3da)](0x4*this[_0x353e50(0x474)]());},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x319)]=function(_0x94cca3,_0x30e751,_0x431344,_0xacb136,_0xbd4048,_0x227d83){const _0x3699fc=_0x39ebd3;_0x94cca3=_0x94cca3||'',_0x227d83=_0x227d83||_0x3699fc(0x135),this[_0x3699fc(0x450)]=this[_0x3699fc(0x37d)](),this[_0x3699fc(0x4c8)]=_0xbd4048?ColorManager['systemColor']():this[_0x3699fc(0x456)][_0x3699fc(0x379)],_0x30e751+=this['itemPadding'](),_0xacb136-=this[_0x3699fc(0x447)]()*0x2;const _0x1911e7=this[_0x3699fc(0x448)](_0x94cca3);if(_0x227d83===_0x3699fc(0x34f))_0x30e751=_0x30e751+Math['floor']((_0xacb136-_0x1911e7[_0x3699fc(0x417)])/0x2);else _0x227d83===_0x3699fc(0x3d7)&&(_0x30e751=_0x30e751+_0xacb136-_0x1911e7[_0x3699fc(0x417)]);_0x431344+=(this[_0x3699fc(0x464)]()-_0x1911e7['height'])/0x2,this[_0x3699fc(0x451)](_0x94cca3,_0x30e751,_0x431344,_0xacb136),this[_0x3699fc(0x450)]=undefined,this[_0x3699fc(0x4c8)]=undefined,this['resetFontSettings']();},Window_ShopStatus['prototype'][_0x39ebd3(0x1fc)]=function(_0x5011ec,_0x5e41b9,_0x10b0b0){const _0x2629e3=_0x39ebd3;if(!DataManager[_0x2629e3(0xc2)](this[_0x2629e3(0x4ee)]))return![];const _0x2fd152=this['getItemConsumableLabel']();this[_0x2629e3(0x319)](_0x2fd152,_0x5011ec,_0x5e41b9,_0x10b0b0,!![]);const _0x4b17d4=this[_0x2629e3(0x3be)]();return this[_0x2629e3(0x319)](_0x4b17d4,_0x5011ec,_0x5e41b9,_0x10b0b0,![],_0x2629e3(0x3d7)),this[_0x2629e3(0x25c)](_0x5011ec,_0x5e41b9,_0x10b0b0),this[_0x2629e3(0x3c9)](),!![];},Window_ShopStatus['prototype'][_0x39ebd3(0x1e8)]=function(){const _0x25ae1c=_0x39ebd3;return VisuMZ['ItemsEquipsCore'][_0x25ae1c(0x237)][_0x25ae1c(0x1a9)]['LabelConsume'];},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x3be)]=function(){const _0x3e88f6=_0x39ebd3,_0x446151=_0x3e88f6(0x31c);if(this[_0x3e88f6(0x3f7)][_0x446151])return this[_0x3e88f6(0x3f7)][_0x446151];return this[_0x3e88f6(0x17d)]()?VisuMZ[_0x3e88f6(0x2a9)][_0x3e88f6(0x237)][_0x3e88f6(0x1a9)][_0x3e88f6(0x23a)]:VisuMZ[_0x3e88f6(0x2a9)][_0x3e88f6(0x237)][_0x3e88f6(0x1a9)][_0x3e88f6(0x24a)];},Window_ShopStatus['prototype']['canConsumeItem']=function(){const _0x3aad19=_0x39ebd3;return VisuMZ[_0x3aad19(0x1e7)]&&VisuMZ['CoreEngine'][_0x3aad19(0x237)][_0x3aad19(0x136)]['KeyItemProtect']&&DataManager['isKeyItem'](this[_0x3aad19(0x4ee)])?![]:this['_item']['consumable'];},Window_ShopStatus[_0x39ebd3(0x3bc)]['drawItemQuantity']=function(_0x4366ff,_0x1f3e33,_0x1b8065){const _0x5174a9=_0x39ebd3;if(!this['isEquipItem']()&&!DataManager[_0x5174a9(0xc2)](this['_item']))return![];if(DataManager[_0x5174a9(0x208)](this[_0x5174a9(0x4ee)])&&!$dataSystem[_0x5174a9(0x181)]){const _0x1a7331=TextManager[_0x5174a9(0x414)];this['drawItemKeyData'](_0x1a7331,_0x4366ff,_0x1f3e33,_0x1b8065,!![],'center');}else{const _0x541488=TextManager['possession'];this[_0x5174a9(0x319)](_0x541488,_0x4366ff,_0x1f3e33,_0x1b8065,!![]);const _0x25cff0=this[_0x5174a9(0x37c)]();this[_0x5174a9(0x319)](_0x25cff0,_0x4366ff,_0x1f3e33,_0x1b8065,![],_0x5174a9(0x3d7));}return this[_0x5174a9(0x25c)](_0x4366ff,_0x1f3e33,_0x1b8065),this[_0x5174a9(0x3c9)](),!![];},Window_ShopStatus['prototype'][_0x39ebd3(0x37c)]=function(){const _0x5363b1=_0x39ebd3,_0xf054ac='QUANTITY';if(this[_0x5363b1(0x3f7)][_0xf054ac])return this[_0x5363b1(0x3f7)][_0xf054ac];const _0x1aa371=VisuMZ[_0x5363b1(0x2a9)][_0x5363b1(0x237)][_0x5363b1(0x3b4)][_0x5363b1(0x378)];return _0x1aa371[_0x5363b1(0x1da)]($gameParty[_0x5363b1(0x410)](this[_0x5363b1(0x4ee)]));},Window_ShopStatus['prototype'][_0x39ebd3(0x179)]=function(_0x231808,_0xd059d8,_0x5c6fc8){const _0x2bad5f=_0x39ebd3,_0x4c3c83=this[_0x2bad5f(0x3f2)]();return this[_0x2bad5f(0x319)](_0x4c3c83,_0x231808,_0xd059d8,_0x5c6fc8,![],_0x2bad5f(0x34f)),this[_0x2bad5f(0x25c)](_0x231808,_0xd059d8,_0x5c6fc8),this[_0x2bad5f(0x3c9)](),!![];},Window_ShopStatus[_0x39ebd3(0x3bc)]['getItemOccasionText']=function(){const _0x36eae6=_0x39ebd3,_0x1951f1=_0x36eae6(0x309);if(this[_0x36eae6(0x3f7)][_0x1951f1])return this[_0x36eae6(0x3f7)][_0x1951f1];const _0x5292e9=VisuMZ[_0x36eae6(0x2a9)][_0x36eae6(0x237)][_0x36eae6(0x1a9)],_0x1b3515='Occasion%1'[_0x36eae6(0x1da)](this['_item']['occasion']);return _0x5292e9[_0x1b3515];},Window_ShopStatus['prototype'][_0x39ebd3(0xdd)]=function(_0x49ae04,_0x3bf0bf,_0x3c2497){const _0x31240e=_0x39ebd3,_0x13e5b6=this[_0x31240e(0x2ea)]();return this[_0x31240e(0x319)](_0x13e5b6,_0x49ae04,_0x3bf0bf,_0x3c2497,![],_0x31240e(0x34f)),this[_0x31240e(0x25c)](_0x49ae04,_0x3bf0bf,_0x3c2497),this[_0x31240e(0x3c9)](),!![];},Window_ShopStatus['prototype'][_0x39ebd3(0x2ea)]=function(){const _0x5bb8bb=_0x39ebd3,_0x7437f5=_0x5bb8bb(0x2de);if(this[_0x5bb8bb(0x3f7)][_0x7437f5])return this[_0x5bb8bb(0x3f7)][_0x7437f5];const _0x2cfae5=VisuMZ[_0x5bb8bb(0x2a9)][_0x5bb8bb(0x237)][_0x5bb8bb(0x1a9)];if(Imported['VisuMZ_1_BattleCore']){const _0x1fa5a3=this['_item']['note'];if(_0x1fa5a3[_0x5bb8bb(0x145)](/<TARGET:[ ](.*)>/i)){const _0x2c0fe8=String(RegExp['$1']);if(_0x2c0fe8[_0x5bb8bb(0x145)](/(\d+) RANDOM ANY/i))return _0x2cfae5[_0x5bb8bb(0x4f8)][_0x5bb8bb(0x1da)](Number(RegExp['$1']));else{if(_0x2c0fe8['match'](/(\d+) RANDOM (?:ENEMY|ENEMIES|FOE|FOES)/i))return _0x2cfae5['ScopeRandomEnemies'][_0x5bb8bb(0x1da)](Number(RegExp['$1']));else{if(_0x2c0fe8[_0x5bb8bb(0x145)](/(\d+) RANDOM (?:ALLY|ALLIES|FRIEND|FRIENDS)/i))return _0x2cfae5[_0x5bb8bb(0x23c)][_0x5bb8bb(0x1da)](Number(RegExp['$1']));else{if(_0x2c0fe8[_0x5bb8bb(0x145)](/ALL (?:ALLY|ALLIES|FRIEND|FRIENDS) (?:BUT|EXCEPT) (?:USER|SELF)/i))return _0x2cfae5[_0x5bb8bb(0x18f)];else{if(_0x2c0fe8[_0x5bb8bb(0x145)](/ALLY OR ENEMY/i))return _0x2cfae5[_0x5bb8bb(0x1a8)]||_0x2cfae5[_0x5bb8bb(0x14d)];else{if(_0x2c0fe8[_0x5bb8bb(0x145)](/ENEMY OR ALLY/i))return _0x2cfae5[_0x5bb8bb(0x364)]||_0x2cfae5[_0x5bb8bb(0x46c)];}}}}}}}const _0x60182a=_0x5bb8bb(0x1ee)[_0x5bb8bb(0x1da)](this[_0x5bb8bb(0x4ee)][_0x5bb8bb(0x281)]);return _0x2cfae5[_0x60182a];},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x3d4)]=function(_0x3181ef,_0x4094f7,_0x56eee3){const _0x2c7a40=_0x39ebd3,_0x6113ea=this[_0x2c7a40(0xf1)]();this[_0x2c7a40(0x319)](_0x6113ea,_0x3181ef,_0x4094f7,_0x56eee3,!![]);const _0x4800ac=this['getItemSpeedText']();return this['drawItemKeyData'](_0x4800ac,_0x3181ef,_0x4094f7,_0x56eee3,![],'right'),this[_0x2c7a40(0x25c)](_0x3181ef,_0x4094f7,_0x56eee3),this[_0x2c7a40(0x3c9)](),!![];},Window_ShopStatus['prototype']['getItemSpeedLabel']=function(){const _0x18806e=_0x39ebd3;return VisuMZ[_0x18806e(0x2a9)][_0x18806e(0x237)][_0x18806e(0x1a9)][_0x18806e(0x3ed)];},Window_ShopStatus['prototype'][_0x39ebd3(0x101)]=function(){const _0x8f9626=_0x39ebd3,_0x457c0c=_0x8f9626(0x37e);if(this[_0x8f9626(0x3f7)][_0x457c0c])return this[_0x8f9626(0x3f7)][_0x457c0c];const _0x192c84=this[_0x8f9626(0x4ee)][_0x8f9626(0x44b)];if(_0x192c84>=0x7d0)return VisuMZ[_0x8f9626(0x2a9)][_0x8f9626(0x237)][_0x8f9626(0x1a9)]['Speed2000'];else{if(_0x192c84>=0x3e8)return VisuMZ[_0x8f9626(0x2a9)][_0x8f9626(0x237)]['StatusWindow'][_0x8f9626(0x3af)];else{if(_0x192c84>0x0)return VisuMZ[_0x8f9626(0x2a9)][_0x8f9626(0x237)][_0x8f9626(0x1a9)][_0x8f9626(0x339)];else{if(_0x192c84===0x0)return VisuMZ[_0x8f9626(0x2a9)]['Settings'][_0x8f9626(0x1a9)][_0x8f9626(0x354)];else{if(_0x192c84>-0x3e8)return VisuMZ[_0x8f9626(0x2a9)][_0x8f9626(0x237)]['StatusWindow'][_0x8f9626(0x138)];else{if(_0x192c84>-0x7d0)return VisuMZ['ItemsEquipsCore'][_0x8f9626(0x237)][_0x8f9626(0x1a9)][_0x8f9626(0x3c1)];else return _0x192c84<=-0x7d0?VisuMZ[_0x8f9626(0x2a9)][_0x8f9626(0x237)]['StatusWindow'][_0x8f9626(0x228)]:_0x8f9626(0x25b);}}}}}},Window_ShopStatus[_0x39ebd3(0x3bc)]['drawItemSuccessRate']=function(_0x5594b2,_0x3dcb11,_0x2f4964){const _0x3afed0=_0x39ebd3,_0x3ad334=this['getItemSuccessRateLabel']();this[_0x3afed0(0x319)](_0x3ad334,_0x5594b2,_0x3dcb11,_0x2f4964,!![]);const _0x1eee62=this['getItemSuccessRateText']();return this[_0x3afed0(0x319)](_0x1eee62,_0x5594b2,_0x3dcb11,_0x2f4964,![],_0x3afed0(0x3d7)),this[_0x3afed0(0x25c)](_0x5594b2,_0x3dcb11,_0x2f4964),this[_0x3afed0(0x3c9)](),!![];},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x337)]=function(){const _0x21541d=_0x39ebd3;return VisuMZ['ItemsEquipsCore'][_0x21541d(0x237)][_0x21541d(0x1a9)][_0x21541d(0x2a0)];},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x2a4)]=function(){const _0x3155bb=_0x39ebd3,_0x1ad552=_0x3155bb(0x469);if(this['_customItemInfo'][_0x1ad552])return this[_0x3155bb(0x3f7)][_0x1ad552];if(Imported[_0x3155bb(0x345)]){const _0x40e005=this[_0x3155bb(0x4ee)][_0x3155bb(0x3ba)];if(_0x40e005['match'](/<ALWAYS HIT>/i))return'100%';else{if(_0x40e005[_0x3155bb(0x145)](/<ALWAYS HIT RATE: (\d+)([%％])>/i))return _0x3155bb(0x42f)['format'](Number(RegExp['$1']));}}return'%1%'[_0x3155bb(0x1da)](this['_item']['successRate']);},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x19d)]=function(_0x382e30,_0x2c6b8f,_0x4d5b4e){const _0x5ed01d=_0x39ebd3,_0x58400e=this[_0x5ed01d(0x415)]();this['drawItemKeyData'](_0x58400e,_0x382e30,_0x2c6b8f,_0x4d5b4e,!![]);const _0x269c35=this[_0x5ed01d(0x342)]();return this[_0x5ed01d(0x319)](_0x269c35,_0x382e30,_0x2c6b8f,_0x4d5b4e,![],'right'),this[_0x5ed01d(0x25c)](_0x382e30,_0x2c6b8f,_0x4d5b4e),this[_0x5ed01d(0x3c9)](),!![];},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x415)]=function(){const _0x4700ef=_0x39ebd3;return VisuMZ[_0x4700ef(0x2a9)][_0x4700ef(0x237)]['StatusWindow'][_0x4700ef(0x242)];},Window_ShopStatus['prototype'][_0x39ebd3(0x342)]=function(){const _0x29d808=_0x39ebd3,_0x202111=_0x29d808(0x3d0);if(this[_0x29d808(0x3f7)][_0x202111])return this[_0x29d808(0x3f7)][_0x202111];const _0x1887e1=_0x29d808(0x4cd);return _0x1887e1[_0x29d808(0x1da)](this['_item'][_0x29d808(0x277)]);},Window_ShopStatus[_0x39ebd3(0x3bc)]['drawItemHitType']=function(_0x1bae75,_0x46e2d6,_0x19446d){const _0xc13383=_0x39ebd3,_0x4c9733=this[_0xc13383(0x332)]();this[_0xc13383(0x319)](_0x4c9733,_0x1bae75,_0x46e2d6,_0x19446d,!![]);const _0x2c015e=this[_0xc13383(0x405)]();return this[_0xc13383(0x319)](_0x2c015e,_0x1bae75,_0x46e2d6,_0x19446d,![],_0xc13383(0x3d7)),this[_0xc13383(0x25c)](_0x1bae75,_0x46e2d6,_0x19446d),this[_0xc13383(0x3c9)](),!![];},Window_ShopStatus['prototype'][_0x39ebd3(0x332)]=function(){const _0x3157cb=_0x39ebd3;return VisuMZ[_0x3157cb(0x2a9)][_0x3157cb(0x237)]['StatusWindow'][_0x3157cb(0x124)];},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x405)]=function(){const _0x492d8f=_0x39ebd3,_0x153233=_0x492d8f(0x3d2);if(this[_0x492d8f(0x3f7)][_0x153233])return this['_customItemInfo'][_0x153233];if(DataManager['isToggleSkill']&&DataManager[_0x492d8f(0x108)](this[_0x492d8f(0x4ee)]))return TextManager[_0x492d8f(0x1dd)];const _0x438a88=VisuMZ['ItemsEquipsCore'][_0x492d8f(0x237)][_0x492d8f(0x1a9)],_0x2d0c3e='HitType%1'['format'](this[_0x492d8f(0x4ee)][_0x492d8f(0xc1)]);return _0x438a88[_0x2d0c3e];},Window_ShopStatus[_0x39ebd3(0x3bc)]['drawItemDamage']=function(_0x42f5a7,_0x3c09cb,_0x57b198){const _0x307fb3=_0x39ebd3;if(this[_0x307fb3(0x4ee)][_0x307fb3(0x4ad)]['type']<=0x0)return _0x3c09cb;if(this[_0x307fb3(0x2b5)](_0x42f5a7,_0x3c09cb,_0x57b198))_0x3c09cb+=this[_0x307fb3(0x464)]();if(this[_0x307fb3(0x143)](_0x42f5a7,_0x3c09cb,_0x57b198))_0x3c09cb+=this[_0x307fb3(0x464)]();return this[_0x307fb3(0x3c9)](),_0x3c09cb;},Window_ShopStatus[_0x39ebd3(0x3bc)]['drawItemDamageElement']=function(_0x522c7f,_0x550970,_0xba1f5d){const _0x278f18=_0x39ebd3,_0xe5f6e4=this['getItemDamageElementLabel']();this['drawItemKeyData'](_0xe5f6e4,_0x522c7f,_0x550970,_0xba1f5d,!![]);const _0x221cb9=this[_0x278f18(0x2a7)]();return this[_0x278f18(0x319)](_0x221cb9,_0x522c7f,_0x550970,_0xba1f5d,![],'right'),this[_0x278f18(0x25c)](_0x522c7f,_0x550970,_0xba1f5d),this[_0x278f18(0x3c9)](),!![];},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x223)]=function(){const _0x21bdc5=_0x39ebd3;return VisuMZ[_0x21bdc5(0x2a9)][_0x21bdc5(0x237)][_0x21bdc5(0x1a9)][_0x21bdc5(0x158)];},Window_ShopStatus[_0x39ebd3(0x3bc)]['getItemDamageElementText']=function(){const _0x5b0691=_0x39ebd3,_0x531b41=_0x5b0691(0x1e0);if(this['_customItemInfo'][_0x531b41])return this[_0x5b0691(0x3f7)][_0x531b41];if(this[_0x5b0691(0x4ee)][_0x5b0691(0x4ad)][_0x5b0691(0x49f)]<=-0x1)return VisuMZ[_0x5b0691(0x2a9)][_0x5b0691(0x237)]['StatusWindow'][_0x5b0691(0x2c8)];else return this['_item'][_0x5b0691(0x4ad)][_0x5b0691(0x49f)]===0x0?VisuMZ[_0x5b0691(0x2a9)]['Settings'][_0x5b0691(0x1a9)][_0x5b0691(0x39a)]:$dataSystem[_0x5b0691(0x45b)][this[_0x5b0691(0x4ee)][_0x5b0691(0x4ad)][_0x5b0691(0x49f)]];},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x143)]=function(_0x183a19,_0x17f422,_0x43ad91){const _0xfb9e13=_0x39ebd3,_0x515c78=this['getItemDamageAmountLabel']();this[_0xfb9e13(0x319)](_0x515c78,_0x183a19,_0x17f422,_0x43ad91,!![]),this['setupItemDamageTempActors']();const _0xce0ab3=this['getItemDamageAmountText'](),_0xec4fcb=ColorManager['damageColor']([0x0,0x0,0x2,0x1,0x3,0x1,0x3][this[_0xfb9e13(0x4ee)]['damage'][_0xfb9e13(0x187)]]);return this['changeTextColor'](_0xec4fcb),this['drawItemKeyData'](_0xce0ab3,_0x183a19,_0x17f422,_0x43ad91,![],_0xfb9e13(0x3d7)),this[_0xfb9e13(0x25c)](_0x183a19,_0x17f422,_0x43ad91),this['resetFontSettings'](),!![];},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x29c)]=function(){const _0x5a7a87=_0x39ebd3;return Imported['VisuMZ_1_BattleCore']&&DataManager['getDamageStyle'](this['_item'])!==_0x5a7a87(0x36d)?this[_0x5a7a87(0x2ee)]():this[_0x5a7a87(0x39b)]();},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x39b)]=function(){const _0x3eccff=_0x39ebd3,_0x24b037=VisuMZ[_0x3eccff(0x2a9)][_0x3eccff(0x237)][_0x3eccff(0x1a9)],_0x51ba8c=_0x3eccff(0x2ba)[_0x3eccff(0x1da)](this[_0x3eccff(0x4ee)][_0x3eccff(0x4ad)][_0x3eccff(0x187)]),_0x1f63d0=[null,TextManager['hp'],TextManager['mp'],TextManager['hp'],TextManager['mp'],TextManager['hp'],TextManager['mp']][this[_0x3eccff(0x4ee)][_0x3eccff(0x4ad)]['type']];return _0x24b037[_0x51ba8c]['format'](_0x1f63d0);},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x282)]=function(){const _0x49805a=_0x39ebd3,_0x153541=$gameActors[_0x49805a(0x1f8)](0x1);this[_0x49805a(0x4a4)]=JsonEx[_0x49805a(0x4e8)](_0x153541),this[_0x49805a(0x275)]=JsonEx[_0x49805a(0x4e8)](_0x153541);},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x2ec)]=function(){const _0xa60770=_0x39ebd3,_0x234bb4=_0xa60770(0x1bc);if(this[_0xa60770(0x3f7)][_0x234bb4])return this[_0xa60770(0x3f7)][_0x234bb4];return Imported['VisuMZ_1_BattleCore']&&DataManager[_0xa60770(0x291)](this[_0xa60770(0x4ee)])!==_0xa60770(0x36d)?this[_0xa60770(0x452)]():this[_0xa60770(0x28b)]();},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x28b)]=function(){const _0x289778=_0x39ebd3;window['a']=this[_0x289778(0x4a4)],window['b']=this[_0x289778(0x275)],this[_0x289778(0x4a4)][_0x289778(0x226)](!![]),this[_0x289778(0x275)][_0x289778(0x226)]([0x3,0x4]['includes'](this[_0x289778(0x4ee)][_0x289778(0x4ad)]['type']));let _0x3654dd=this[_0x289778(0x4ee)][_0x289778(0x4ad)][_0x289778(0x38d)];try{const _0x2d9652=Math[_0x289778(0x165)](eval(_0x3654dd),0x0)/window['a'][_0x289778(0x362)];return this[_0x289778(0x20e)](),isNaN(_0x2d9652)?_0x289778(0x25b):_0x289778(0x42f)[_0x289778(0x1da)](Math[_0x289778(0x2c3)](_0x2d9652*0x64));}catch(_0x2a3bcc){return $gameTemp['isPlaytest']()&&(console[_0x289778(0x4a0)](_0x289778(0x3e8)[_0x289778(0x1da)](this[_0x289778(0x4ee)]['name'])),console[_0x289778(0x4a0)](_0x2a3bcc)),this[_0x289778(0x20e)](),_0x289778(0x25b);}},Window_ShopStatus[_0x39ebd3(0x3bc)]['revertGlobalNamespaceVariables']=function(){window['a']=undefined,window['b']=undefined;},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x4c3)]=function(_0x490603,_0x54a7f5,_0x348422){const _0x519ac1=_0x39ebd3;if(!this[_0x519ac1(0x3cf)]())return _0x54a7f5;if(this['drawItemEffectsHpRecovery'](_0x490603,_0x54a7f5,_0x348422))_0x54a7f5+=this['lineHeight']();if(this['drawItemEffectsMpRecovery'](_0x490603,_0x54a7f5,_0x348422))_0x54a7f5+=this['lineHeight']();if(this[_0x519ac1(0x3cc)](_0x490603,_0x54a7f5,_0x348422))_0x54a7f5+=this[_0x519ac1(0x464)]();if(this['drawItemEffectsHpDamage'](_0x490603,_0x54a7f5,_0x348422))_0x54a7f5+=this[_0x519ac1(0x464)]();if(this[_0x519ac1(0x4b5)](_0x490603,_0x54a7f5,_0x348422))_0x54a7f5+=this[_0x519ac1(0x464)]();if(this[_0x519ac1(0x211)](_0x490603,_0x54a7f5,_0x348422))_0x54a7f5+=this[_0x519ac1(0x464)]();if(this[_0x519ac1(0x2d4)](_0x490603,_0x54a7f5,_0x348422))_0x54a7f5+=this[_0x519ac1(0x464)]();if(this['drawItemEffectsAddedStatesBuffs'](_0x490603,_0x54a7f5,_0x348422))_0x54a7f5+=this['lineHeight']();if(this[_0x519ac1(0xa2)](_0x490603,_0x54a7f5,_0x348422))_0x54a7f5+=this['lineHeight']();return this[_0x519ac1(0x3c9)](),_0x54a7f5;},Window_ShopStatus[_0x39ebd3(0x3bc)]['getItemEffects']=function(){const _0x1994fc=_0x39ebd3;return this[_0x1994fc(0x4ee)][_0x1994fc(0x479)];},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x3cf)]=function(){const _0x6e723e=_0x39ebd3;let _0x505288=![];this[_0x6e723e(0x36b)]={'rateHP':0x0,'flatHP':0x0,'rateMP':0x0,'flatMP':0x0,'gainTP':0x0,'selfTP':0x0,'addState':[],'removeState':[],'changeBuff':[0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0],'removeBuff':[],'removeDebuff':[],'addStateBuffChanges':![],'removeStateBuffChanges':![]};const _0x432a97=this[_0x6e723e(0x21e)]();for(const _0x3c65af of _0x432a97){switch(_0x3c65af['code']){case Game_Action[_0x6e723e(0x167)]:this[_0x6e723e(0x36b)][_0x6e723e(0x4cc)]+=_0x3c65af[_0x6e723e(0x10f)],this['_itemData'][_0x6e723e(0x315)]+=_0x3c65af[_0x6e723e(0x499)],_0x505288=!![];break;case Game_Action[_0x6e723e(0x323)]:this['_itemData']['rateMP']+=_0x3c65af['value1'],this[_0x6e723e(0x36b)][_0x6e723e(0x3f1)]+=_0x3c65af[_0x6e723e(0x499)],_0x505288=!![];break;case Game_Action[_0x6e723e(0x11c)]:this[_0x6e723e(0x36b)][_0x6e723e(0x26f)]+=_0x3c65af[_0x6e723e(0x10f)],_0x505288=!![];break;case Game_Action['EFFECT_ADD_STATE']:this['_itemData'][_0x6e723e(0x26d)][_0x6e723e(0x17a)](_0x3c65af[_0x6e723e(0x1cd)]),_0x505288=!![];break;case Game_Action[_0x6e723e(0x424)]:this['_itemData']['removeState'][_0x6e723e(0x17a)](_0x3c65af['dataId']),this[_0x6e723e(0x36b)][_0x6e723e(0x445)]=!![],_0x505288=!![];break;case Game_Action[_0x6e723e(0x4a3)]:this['_itemData'][_0x6e723e(0x422)][_0x3c65af[_0x6e723e(0x1cd)]]+=0x1,_0x505288=!![];break;case Game_Action[_0x6e723e(0x222)]:this[_0x6e723e(0x36b)]['changeBuff'][_0x3c65af[_0x6e723e(0x1cd)]]-=0x1,_0x505288=!![];break;case Game_Action[_0x6e723e(0x183)]:this[_0x6e723e(0x36b)]['removeBuff'][_0x6e723e(0x17a)](_0x3c65af[_0x6e723e(0x1cd)]),this[_0x6e723e(0x36b)][_0x6e723e(0x445)]=!![],_0x505288=!![];break;case Game_Action[_0x6e723e(0x4b8)]:this[_0x6e723e(0x36b)][_0x6e723e(0x1b5)][_0x6e723e(0x17a)](_0x3c65af[_0x6e723e(0x1cd)]),this[_0x6e723e(0x36b)][_0x6e723e(0x445)]=!![],_0x505288=!![];break;}}if(this['_itemData'][_0x6e723e(0x26d)]['length']>0x0)this[_0x6e723e(0x36b)][_0x6e723e(0x205)]=!![];for(let _0x44391d=0x0;_0x44391d<this['_itemData']['changeBuff']['length'];_0x44391d++){if(this[_0x6e723e(0x36b)][_0x6e723e(0x422)][_0x44391d]!==0x0)this[_0x6e723e(0x36b)][_0x6e723e(0x205)]=!![];}this[_0x6e723e(0x4ee)]['tpGain']!==0x0&&(this['_itemData'][_0x6e723e(0x233)]=this[_0x6e723e(0x4ee)][_0x6e723e(0x186)],_0x505288=!![]);const _0x50e00d=[_0x6e723e(0x2bd),_0x6e723e(0xd9),'TP\x20RECOVERY','HP\x20DAMAGE','MP\x20DAMAGE',_0x6e723e(0x2d2),'USER\x20TP\x20GAIN','ADDED\x20EFFECTS',_0x6e723e(0x4c0)];for(const _0x31594c of _0x50e00d){if(this[_0x6e723e(0x3f7)][_0x31594c]){_0x505288=!![];break;}}return _0x505288;},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x438)]=function(_0x5caa4f,_0x3ca4d9,_0x445f7c){const _0x36d673=_0x39ebd3,_0x17aa2b=_0x36d673(0x2bd);if(this['_itemData'][_0x36d673(0x4cc)]<=0x0&&this['_itemData'][_0x36d673(0x315)]<=0x0&&!this[_0x36d673(0x3f7)][_0x17aa2b])return![];const _0x30f1b8=this[_0x36d673(0x1ad)]();this[_0x36d673(0x319)](_0x30f1b8,_0x5caa4f,_0x3ca4d9,_0x445f7c,!![]);const _0x1e9299=this[_0x36d673(0x96)]();return this[_0x36d673(0x4ca)](ColorManager[_0x36d673(0x116)](0x1)),this[_0x36d673(0x319)](_0x1e9299,_0x5caa4f,_0x3ca4d9,_0x445f7c,![],_0x36d673(0x3d7)),this['drawItemDarkRect'](_0x5caa4f,_0x3ca4d9,_0x445f7c),this[_0x36d673(0x3c9)](),!![];},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x1ad)]=function(){const _0x29e534=_0x39ebd3,_0x53c925=VisuMZ[_0x29e534(0x2a9)][_0x29e534(0x237)][_0x29e534(0x1a9)][_0x29e534(0x2eb)];return _0x53c925[_0x29e534(0x1da)](TextManager['hp']);},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x96)]=function(){const _0x3d7502=_0x39ebd3,_0x510934=_0x3d7502(0x2bd);if(this['_customItemInfo'][_0x510934])return this[_0x3d7502(0x3f7)][_0x510934];let _0x2a4377='';if(this[_0x3d7502(0x36b)][_0x3d7502(0x4cc)]>0x0)_0x2a4377+=_0x3d7502(0x238)['format'](Math[_0x3d7502(0x349)](this[_0x3d7502(0x36b)]['rateHP']*0x64));if(this['_itemData'][_0x3d7502(0x4cc)]>0x0&&this[_0x3d7502(0x36b)][_0x3d7502(0x315)]>0x0)_0x2a4377+='\x20';if(this[_0x3d7502(0x36b)]['flatHP']>0x0)_0x2a4377+=_0x3d7502(0x297)[_0x3d7502(0x1da)](this['_itemData'][_0x3d7502(0x315)]);return _0x2a4377;},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x130)]=function(_0x31cbfb,_0x19065d,_0x2a8ad3){const _0x1b5a69=_0x39ebd3,_0x49115f=_0x1b5a69(0xd9);if(this['_itemData'][_0x1b5a69(0x271)]<=0x0&&this['_itemData'][_0x1b5a69(0x3f1)]<=0x0&&!this[_0x1b5a69(0x3f7)][_0x49115f])return![];const _0x2aa160=this[_0x1b5a69(0x14c)]();this[_0x1b5a69(0x319)](_0x2aa160,_0x31cbfb,_0x19065d,_0x2a8ad3,!![]);const _0x535020=this[_0x1b5a69(0x29b)]();return this[_0x1b5a69(0x4ca)](ColorManager['damageColor'](0x3)),this[_0x1b5a69(0x319)](_0x535020,_0x31cbfb,_0x19065d,_0x2a8ad3,![],_0x1b5a69(0x3d7)),this['drawItemDarkRect'](_0x31cbfb,_0x19065d,_0x2a8ad3),this[_0x1b5a69(0x3c9)](),!![];},Window_ShopStatus[_0x39ebd3(0x3bc)]['getItemEffectsMpRecoveryLabel']=function(){const _0x365901=_0x39ebd3,_0x100325=VisuMZ[_0x365901(0x2a9)][_0x365901(0x237)][_0x365901(0x1a9)][_0x365901(0xda)];return _0x100325['format'](TextManager['mp']);},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x29b)]=function(){const _0x21e4e7=_0x39ebd3,_0x45ad2b=_0x21e4e7(0xd9);if(this[_0x21e4e7(0x3f7)][_0x45ad2b])return this[_0x21e4e7(0x3f7)][_0x45ad2b];let _0x10edd8='';if(this[_0x21e4e7(0x36b)][_0x21e4e7(0x271)]>0x0)_0x10edd8+='+%1%'[_0x21e4e7(0x1da)](Math[_0x21e4e7(0x349)](this['_itemData']['rateMP']*0x64));if(this[_0x21e4e7(0x36b)]['rateMP']>0x0&&this['_itemData'][_0x21e4e7(0x3f1)]>0x0)_0x10edd8+='\x20';if(this['_itemData']['flatMP']>0x0)_0x10edd8+=_0x21e4e7(0x297)['format'](this[_0x21e4e7(0x36b)]['flatMP']);return _0x10edd8;},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x3cc)]=function(_0x4e4cce,_0x14178d,_0x20e8bc){const _0x5cdb22=_0x39ebd3,_0x75e14b=_0x5cdb22(0x37a);if(this[_0x5cdb22(0x36b)][_0x5cdb22(0x26f)]<=0x0&&!this[_0x5cdb22(0x3f7)][_0x75e14b])return![];const _0xb818c=this[_0x5cdb22(0x385)]();this['drawItemKeyData'](_0xb818c,_0x4e4cce,_0x14178d,_0x20e8bc,!![]);const _0x5055fa=this[_0x5cdb22(0x1a0)]();return this[_0x5cdb22(0x4ca)](ColorManager[_0x5cdb22(0x3fb)]()),this[_0x5cdb22(0x319)](_0x5055fa,_0x4e4cce,_0x14178d,_0x20e8bc,![],'right'),this[_0x5cdb22(0x25c)](_0x4e4cce,_0x14178d,_0x20e8bc),this['resetFontSettings'](),!![];},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x385)]=function(){const _0x476a56=_0x39ebd3,_0x225dd2=VisuMZ[_0x476a56(0x2a9)][_0x476a56(0x237)][_0x476a56(0x1a9)][_0x476a56(0x358)];return _0x225dd2[_0x476a56(0x1da)](TextManager['tp']);},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x1a0)]=function(){const _0xc1125a=_0x39ebd3,_0x509de7=_0xc1125a(0x37a);if(this[_0xc1125a(0x3f7)][_0x509de7])return this[_0xc1125a(0x3f7)][_0x509de7];let _0x301ee7='';return _0x301ee7+=_0xc1125a(0x297)['format'](this['_itemData'][_0xc1125a(0x26f)]),_0x301ee7;},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x2d4)]=function(_0x35584d,_0x343df4,_0x4c826b){const _0x5d2ec3=_0x39ebd3,_0x43467e=_0x5d2ec3(0x4f4);if(this[_0x5d2ec3(0x36b)][_0x5d2ec3(0x233)]===0x0&&!this[_0x5d2ec3(0x3f7)][_0x43467e])return![];const _0x2b35d1=this[_0x5d2ec3(0x217)]();this[_0x5d2ec3(0x319)](_0x2b35d1,_0x35584d,_0x343df4,_0x4c826b,!![]);const _0x450e0f=this[_0x5d2ec3(0x2e6)]();return this[_0x5d2ec3(0x36b)][_0x5d2ec3(0x233)]>0x0?this[_0x5d2ec3(0x4ca)](ColorManager[_0x5d2ec3(0x3fb)]()):this[_0x5d2ec3(0x4ca)](ColorManager[_0x5d2ec3(0x103)]()),this[_0x5d2ec3(0x319)](_0x450e0f,_0x35584d,_0x343df4,_0x4c826b,![],_0x5d2ec3(0x3d7)),this[_0x5d2ec3(0x25c)](_0x35584d,_0x343df4,_0x4c826b),this['resetFontSettings'](),!![];},Window_ShopStatus['prototype'][_0x39ebd3(0x217)]=function(){const _0x5f4147=_0x39ebd3,_0x3e57fc=VisuMZ['ItemsEquipsCore']['Settings'][_0x5f4147(0x1a9)]['LabelSelfGainTP'];return _0x3e57fc[_0x5f4147(0x1da)](TextManager['tp']);},Window_ShopStatus[_0x39ebd3(0x3bc)]['getItemEffectsSelfTpGainText']=function(){const _0x4bb1a2=_0x39ebd3,_0x214b82=_0x4bb1a2(0x4f4);if(this[_0x4bb1a2(0x3f7)][_0x214b82])return this[_0x4bb1a2(0x3f7)][_0x214b82];let _0x25196e='';return this['_itemData'][_0x4bb1a2(0x233)]>0x0?_0x25196e+=_0x4bb1a2(0x297)[_0x4bb1a2(0x1da)](this['_itemData'][_0x4bb1a2(0x233)]):_0x25196e+='%1'[_0x4bb1a2(0x1da)](this[_0x4bb1a2(0x36b)][_0x4bb1a2(0x233)]),_0x25196e;},Window_ShopStatus[_0x39ebd3(0x3bc)]['drawItemEffectsHpDamage']=function(_0x13fa09,_0x4e6363,_0x4021e5){const _0x4e800a=_0x39ebd3,_0x3e2cea=_0x4e800a(0x202);if(this['_itemData']['rateHP']>=0x0&&this[_0x4e800a(0x36b)]['flatHP']>=0x0&&!this['_customItemInfo'][_0x3e2cea])return![];const _0x5c5404=this[_0x4e800a(0x352)]();this[_0x4e800a(0x319)](_0x5c5404,_0x13fa09,_0x4e6363,_0x4021e5,!![]);const _0x1fecb1=this[_0x4e800a(0x4e0)]();return this['changeTextColor'](ColorManager[_0x4e800a(0x116)](0x0)),this['drawItemKeyData'](_0x1fecb1,_0x13fa09,_0x4e6363,_0x4021e5,![],_0x4e800a(0x3d7)),this[_0x4e800a(0x25c)](_0x13fa09,_0x4e6363,_0x4021e5),this['resetFontSettings'](),!![];},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x352)]=function(){const _0x22a2d8=_0x39ebd3,_0x16f1ba=VisuMZ['ItemsEquipsCore']['Settings'][_0x22a2d8(0x1a9)][_0x22a2d8(0x43e)];return _0x16f1ba[_0x22a2d8(0x1da)](TextManager['hp']);},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x4e0)]=function(){const _0x147463=_0x39ebd3,_0x277227='HP\x20DAMAGE';if(this[_0x147463(0x3f7)][_0x277227])return this[_0x147463(0x3f7)][_0x277227];let _0x3a0e3a='';if(this[_0x147463(0x36b)][_0x147463(0x4cc)]<0x0)_0x3a0e3a+=_0x147463(0x42f)['format'](Math[_0x147463(0x349)](this[_0x147463(0x36b)][_0x147463(0x4cc)]*0x64));if(this[_0x147463(0x36b)][_0x147463(0x4cc)]<0x0&&this[_0x147463(0x36b)]['flatHP']<0x0)_0x3a0e3a+='\x20';if(this['_itemData'][_0x147463(0x315)]<0x0)_0x3a0e3a+='%1'[_0x147463(0x1da)](this['_itemData']['flatHP']);return _0x3a0e3a;},Window_ShopStatus['prototype'][_0x39ebd3(0x4b5)]=function(_0x32d683,_0x1d56c9,_0x34a2cb){const _0x3f8209=_0x39ebd3,_0x39f1ce=_0x3f8209(0x3e1);if(this['_itemData'][_0x3f8209(0x271)]>=0x0&&this['_itemData']['flatMP']>=0x0&&!this[_0x3f8209(0x3f7)][_0x39f1ce])return![];const _0x50bea9=this[_0x3f8209(0x1ab)]();this[_0x3f8209(0x319)](_0x50bea9,_0x32d683,_0x1d56c9,_0x34a2cb,!![]);const _0x35df77=this[_0x3f8209(0x218)]();return this['changeTextColor'](ColorManager['damageColor'](0x2)),this[_0x3f8209(0x319)](_0x35df77,_0x32d683,_0x1d56c9,_0x34a2cb,![],'right'),this['drawItemDarkRect'](_0x32d683,_0x1d56c9,_0x34a2cb),this[_0x3f8209(0x3c9)](),!![];},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x1ab)]=function(){const _0x2f447a=_0x39ebd3,_0x38ad6b=VisuMZ['ItemsEquipsCore'][_0x2f447a(0x237)][_0x2f447a(0x1a9)]['LabelDamageMP'];return _0x38ad6b[_0x2f447a(0x1da)](TextManager['mp']);},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x218)]=function(){const _0x4c0ac1=_0x39ebd3,_0x5dc351=_0x4c0ac1(0x3e1);if(this[_0x4c0ac1(0x3f7)][_0x5dc351])return this['_customItemInfo'][_0x5dc351];let _0x57d3df='';if(this['_itemData']['rateMP']<0x0)_0x57d3df+=_0x4c0ac1(0x42f)['format'](Math['floor'](this['_itemData']['rateMP']*0x64));if(this['_itemData'][_0x4c0ac1(0x271)]<0x0&&this[_0x4c0ac1(0x36b)]['flatMP']<0x0)_0x57d3df+='\x20';if(this[_0x4c0ac1(0x36b)][_0x4c0ac1(0x3f1)]<0x0)_0x57d3df+='%1'[_0x4c0ac1(0x1da)](this[_0x4c0ac1(0x36b)]['flatMP']);return _0x57d3df;},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x211)]=function(_0x42ada9,_0x271a11,_0xf95b7){const _0x5cfbed=_0x39ebd3,_0x1a87fa='TP\x20DAMAGE';if(this[_0x5cfbed(0x36b)][_0x5cfbed(0x26f)]>=0x0&&!this[_0x5cfbed(0x3f7)][_0x1a87fa])return![];const _0x4e7ba9=this[_0x5cfbed(0x2f9)]();this[_0x5cfbed(0x319)](_0x4e7ba9,_0x42ada9,_0x271a11,_0xf95b7,!![]);const _0x3e0960=this['getItemEffectsTpDamageText']();return this[_0x5cfbed(0x4ca)](ColorManager[_0x5cfbed(0x103)]()),this[_0x5cfbed(0x319)](_0x3e0960,_0x42ada9,_0x271a11,_0xf95b7,![],_0x5cfbed(0x3d7)),this[_0x5cfbed(0x25c)](_0x42ada9,_0x271a11,_0xf95b7),this[_0x5cfbed(0x3c9)](),!![];},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x2f9)]=function(){const _0x37650b=_0x39ebd3,_0x51871d=VisuMZ[_0x37650b(0x2a9)][_0x37650b(0x237)][_0x37650b(0x1a9)]['LabelDamageTP'];return _0x51871d[_0x37650b(0x1da)](TextManager['tp']);},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x2ab)]=function(){const _0x27d838=_0x39ebd3,_0x33f7ea=_0x27d838(0x2d2);if(this[_0x27d838(0x3f7)][_0x33f7ea])return this[_0x27d838(0x3f7)][_0x33f7ea];let _0x21780='';return _0x21780+='%1'[_0x27d838(0x1da)](this[_0x27d838(0x36b)][_0x27d838(0x26f)]),_0x21780;},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x11e)]=function(_0x344479,_0x5a65d7,_0x448107){const _0x4c10ec=_0x39ebd3,_0x20e367=_0x4c10ec(0x21a);if(!this[_0x4c10ec(0x36b)][_0x4c10ec(0x205)]&&!this['_customItemInfo'][_0x20e367])return![];const _0x4cfa9c=this[_0x4c10ec(0x353)]();if(_0x4cfa9c[_0x4c10ec(0x142)]<=0x0)return![];const _0x481094=this[_0x4c10ec(0x305)]();return this[_0x4c10ec(0x319)](_0x481094,_0x344479,_0x5a65d7,_0x448107,!![]),this[_0x4c10ec(0x319)](_0x4cfa9c,_0x344479,_0x5a65d7,_0x448107,![],_0x4c10ec(0x3d7)),this[_0x4c10ec(0x25c)](_0x344479,_0x5a65d7,_0x448107),this['resetFontSettings'](),!![];},Window_ShopStatus[_0x39ebd3(0x3bc)]['getItemEffectsAddedStatesBuffsLabel']=function(){const _0x26ddde=_0x39ebd3;return VisuMZ['ItemsEquipsCore'][_0x26ddde(0x237)][_0x26ddde(0x1a9)][_0x26ddde(0x146)];},Window_ShopStatus['prototype'][_0x39ebd3(0x353)]=function(){const _0x26ff23=_0x39ebd3,_0x52e7fe=_0x26ff23(0x21a);if(this[_0x26ff23(0x3f7)][_0x52e7fe])return this[_0x26ff23(0x3f7)][_0x52e7fe];let _0x18df25='',_0x2571b3=0x0;const _0xde53a2=0x8;for(const _0x47236d of this[_0x26ff23(0x36b)]['addState']){const _0xb77c5f=$dataStates[_0x47236d];if(_0xb77c5f&&_0xb77c5f[_0x26ff23(0x229)]>0x0){_0x18df25+=_0x26ff23(0x272)[_0x26ff23(0x1da)](_0xb77c5f['iconIndex']),_0x2571b3++;if(_0x2571b3>=_0xde53a2)return _0x18df25;}}for(let _0x162123=0x0;_0x162123<this[_0x26ff23(0x36b)]['changeBuff']['length'];_0x162123++){const _0x15f7d5=this[_0x26ff23(0x36b)][_0x26ff23(0x422)][_0x162123],_0x560724=Game_BattlerBase[_0x26ff23(0x3bc)]['buffIconIndex'](_0x15f7d5,_0x162123);if(_0x560724>0x0){_0x18df25+=_0x26ff23(0x272)[_0x26ff23(0x1da)](_0x560724),_0x2571b3++;if(_0x2571b3>=_0xde53a2)return _0x18df25;}}return _0x18df25;},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0xa2)]=function(_0x8f7836,_0x3af5a3,_0x2fc24b){const _0x4ae12d=_0x39ebd3,_0x2e3913=_0x4ae12d(0x4c0);if(!this[_0x4ae12d(0x36b)]['removeStateBuffChanges']&&!this['_customItemInfo'][_0x2e3913])return![];const _0x33b595=this[_0x4ae12d(0x404)]();this[_0x4ae12d(0x319)](_0x33b595,_0x8f7836,_0x3af5a3,_0x2fc24b,!![]);const _0x135acc=this[_0x4ae12d(0x4f1)]();return this[_0x4ae12d(0x319)](_0x135acc,_0x8f7836,_0x3af5a3,_0x2fc24b,![],_0x4ae12d(0x3d7)),this['drawItemDarkRect'](_0x8f7836,_0x3af5a3,_0x2fc24b),this[_0x4ae12d(0x3c9)](),!![];},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x404)]=function(){const _0x52ecb6=_0x39ebd3;return VisuMZ[_0x52ecb6(0x2a9)]['Settings'][_0x52ecb6(0x1a9)][_0x52ecb6(0x49c)];},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x4f1)]=function(){const _0x1beceb=_0x39ebd3,_0x520988=_0x1beceb(0x4c0);if(this[_0x1beceb(0x3f7)][_0x520988])return this['_customItemInfo'][_0x520988];let _0x4e042e='',_0x5ac6fd=0x0;const _0x160fbd=VisuMZ[_0x1beceb(0x2a9)][_0x1beceb(0x237)][_0x1beceb(0x1a9)][_0x1beceb(0x4df)];for(const _0x29e866 of this[_0x1beceb(0x36b)][_0x1beceb(0x394)]){const _0x2121ef=$dataStates[_0x29e866];if(_0x2121ef&&_0x2121ef[_0x1beceb(0x229)]>0x0){_0x4e042e+=_0x1beceb(0x272)[_0x1beceb(0x1da)](_0x2121ef[_0x1beceb(0x229)]),_0x5ac6fd++;if(_0x5ac6fd>=_0x160fbd)return _0x4e042e;}}for(let _0x55c73b=0x0;_0x55c73b<this['_itemData'][_0x1beceb(0x4e2)][_0x1beceb(0x142)];_0x55c73b++){const _0x5a8b9f=this[_0x1beceb(0x36b)][_0x1beceb(0x4e2)][_0x55c73b],_0x2d64b9=Game_BattlerBase[_0x1beceb(0x3bc)][_0x1beceb(0x356)](0x1,_0x5a8b9f);if(_0x2d64b9>0x0){_0x4e042e+='\x5cI[%1]'[_0x1beceb(0x1da)](_0x2d64b9),_0x5ac6fd++;if(_0x5ac6fd>=_0x160fbd)return _0x4e042e;}}for(let _0x182201=0x0;_0x182201<this[_0x1beceb(0x36b)][_0x1beceb(0x1b5)][_0x1beceb(0x142)];_0x182201++){const _0x10c5d0=this['_itemData'][_0x1beceb(0x1b5)][_0x182201],_0x4940ac=Game_BattlerBase[_0x1beceb(0x3bc)][_0x1beceb(0x356)](-0x1,_0x10c5d0);if(_0x4940ac>0x0){_0x4e042e+=_0x1beceb(0x272)[_0x1beceb(0x1da)](_0x4940ac),_0x5ac6fd++;if(_0x5ac6fd>=_0x160fbd)return _0x4e042e;}}return _0x4e042e;},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x4a9)]=function(_0x4394a2,_0x5033f4,_0x2dd054){const _0x1022e8=_0x39ebd3;if(this[_0x1022e8(0x4ee)][_0x1022e8(0x3ba)]['match'](/<CUSTOM STATUS INFO>\s*([\s\S]*)\s*<\/CUSTOM STATUS INFO>/i)){const _0x9b7cf6=String(RegExp['$1'])['split'](/[\r\n]+/);for(const _0x14fa67 of _0x9b7cf6){if(_0x14fa67['match'](/(.*):[ ](.*)/i)){const _0x16592b=String(RegExp['$1'])[_0x1022e8(0x206)](),_0x1f0324=String(RegExp['$2'])[_0x1022e8(0x206)]();this[_0x1022e8(0x264)](_0x16592b,_0x1f0324,_0x4394a2,_0x5033f4,_0x2dd054),_0x5033f4+=this[_0x1022e8(0x464)]();}}}return this[_0x1022e8(0x3c9)](),_0x5033f4;},Window_ShopStatus['prototype'][_0x39ebd3(0x264)]=function(_0x186db5,_0x1ec022,_0x38cd32,_0x1cd1ba,_0x34a0ea){const _0x14b8e1=_0x39ebd3;this['drawItemKeyData'](_0x186db5,_0x38cd32,_0x1cd1ba,_0x34a0ea,!![]),this[_0x14b8e1(0x319)](_0x1ec022,_0x38cd32,_0x1cd1ba,_0x34a0ea,![],_0x14b8e1(0x3d7)),this[_0x14b8e1(0x25c)](_0x38cd32,_0x1cd1ba,_0x34a0ea),this[_0x14b8e1(0x3c9)]();},Window_ShopStatus['prototype']['drawCustomShopGraphic']=function(){const _0x3ec654=_0x39ebd3;if(!this['_item'])return;const _0x39221f=this[_0x3ec654(0x4ee)][_0x3ec654(0x3ba)],_0x58e680=/<SHOP (?:PICTURE|IMAGE|PICTURE NAME|PICTURE FILENAME|IMAGE NAME|IMAGE FILENAME):[ ](.*)>/gi,_0x1d5341=_0x39221f[_0x3ec654(0x145)](_0x58e680);if(_0x1d5341)for(const _0x42f645 of _0x1d5341){_0x42f645[_0x3ec654(0x145)](_0x58e680);const _0x4698d0=String(RegExp['$1'])['trim']()||'';if(_0x4698d0==='')continue;const _0x1abf88=ImageManager['loadPicture'](_0x4698d0);_0x1abf88[_0x3ec654(0x178)](this[_0x3ec654(0x1e6)][_0x3ec654(0xdc)](this,_0x1abf88,this['_item']));}},Window_ShopStatus[_0x39ebd3(0x3bc)][_0x39ebd3(0x1e6)]=function(_0x3bbc5f,_0xdc754a){const _0x3b0564=_0x39ebd3;if(this[_0x3b0564(0x4ee)]!==_0xdc754a)return;if(!_0x3bbc5f)return;if(_0x3bbc5f[_0x3b0564(0x417)]<=0x0||_0x3bbc5f[_0x3b0564(0x435)]<=0x0)return;const _0x3f98c9=_0xdc754a[_0x3b0564(0x3ba)];let _0x2b8105=_0x3b0564(0x45a);_0x3f98c9[_0x3b0564(0x145)](/<SHOP (?:PICTURE|IMAGE) LAYER:[ ]FOREGROUND>/i)&&(_0x2b8105='foreground');const _0x1546ff=_0x2b8105===_0x3b0564(0x45a)?this[_0x3b0564(0x1bb)]:this['contents'];let _0x4455cc=this[_0x3b0564(0x35e)],_0x4f218d=this[_0x3b0564(0x4e5)];_0x3f98c9[_0x3b0564(0x145)](/<SHOP (?:PICTURE|IMAGE) MAX WIDTH:[ ](\d+)>/i)&&(_0x4455cc=Number(RegExp['$1']));_0x3f98c9['match'](/<SHOP (?:PICTURE|IMAGE) MAX HEIGHT:[ ](\d+)>/i)&&(_0x4f218d=Number(RegExp['$1']));_0x3f98c9[_0x3b0564(0x145)](/<SHOP (?:PICTURE|IMAGE) MAX DIMENSIONS:[ ](\d+),[ ]*(\d+)>/i)&&(_0x4455cc=Number(RegExp['$1']),_0x4f218d=Number(RegExp['$2']));const _0x5c3e2a=Math[_0x3b0564(0x37b)](0x1,_0x4455cc/_0x3bbc5f[_0x3b0564(0x417)],_0x4f218d/_0x3bbc5f[_0x3b0564(0x435)]);let _0x5c9d30=0x0,_0x534a9b=0x0,_0xe5f914=Math[_0x3b0564(0x349)](_0x3bbc5f[_0x3b0564(0x417)]*_0x5c3e2a),_0x2772a3=Math['floor'](_0x3bbc5f[_0x3b0564(0x435)]*_0x5c3e2a),_0xcaa181=_0x3b0564(0x34f);_0x3f98c9[_0x3b0564(0x145)](/<SHOP (?:PICTURE|IMAGE) (?:ALIGN|ALIGNMENT):[ ](LEFT|CENTER|RIGHT)>/i)&&(_0xcaa181=String(RegExp['$1'])[_0x3b0564(0x290)]()[_0x3b0564(0x206)]());if(_0xcaa181===_0x3b0564(0x135))_0x5c9d30=0x0;else _0xcaa181===_0x3b0564(0x34f)?_0x5c9d30=Math[_0x3b0564(0x2c3)]((this[_0x3b0564(0x35e)]-_0xe5f914)/0x2):_0x5c9d30=this['innerWidth']-_0xe5f914;let _0x3d451f=_0x3b0564(0x46a);_0x3f98c9[_0x3b0564(0x145)](/<SHOP (?:PICTURE|IMAGE) POSITION:[ ](TOP|MIDDLE|BOTTOM)>/i)&&(_0x3d451f=String(RegExp['$1'])['toLowerCase']()[_0x3b0564(0x206)]());if(_0x3d451f===_0x3b0564(0x440))_0x534a9b=0x0;else _0x3d451f==='middle'?_0x534a9b=Math[_0x3b0564(0x2c3)]((this[_0x3b0564(0x4e5)]-_0x2772a3)/0x2):_0x534a9b=this['innerHeight']-_0x2772a3;_0x3f98c9[_0x3b0564(0x145)](/<SHOP (?:PICTURE|IMAGE) OFFSET X:[ ]([\+\-]\d+)>/i)&&(_0x5c9d30+=Number(RegExp['$1']));_0x3f98c9[_0x3b0564(0x145)](/<SHOP (?:PICTURE|IMAGE) OFFSET Y:[ ]([\+\-]\d+)>/i)&&(_0x534a9b+=Number(RegExp['$1']));_0x3f98c9['match'](/<SHOP (?:PICTURE|IMAGE) OFFSET:[ ]([\+\-]\d+),[ ]*([\+\-]\d+)>/i)&&(_0x5c9d30+=Number(RegExp['$1']),_0x534a9b+=Number(RegExp['$2']));let _0x2788ad=0xff;if(_0x3f98c9['match'](/<SHOP (?:PICTURE|IMAGE) OPACITY:[ ](\d+)>/i))_0x2788ad=Number(RegExp['$1']);else _0x3f98c9['match'](/<SHOP (?:PICTURE|IMAGE) OPACITY:[ ](\d+)([%％])>/i)&&(_0x2788ad=Math[_0x3b0564(0x2c3)](Number(RegExp['$1'])*0.01*0xff)[_0x3b0564(0x496)](0x0,0xff));_0x1546ff['paintOpacity']=_0x2788ad,_0x1546ff['blt'](_0x3bbc5f,0x0,0x0,_0x3bbc5f['width'],_0x3bbc5f[_0x3b0564(0x435)],_0x5c9d30,_0x534a9b,_0xe5f914,_0x2772a3),_0x1546ff[_0x3b0564(0x31a)]=0xff;},VisuMZ[_0x39ebd3(0x2a9)][_0x39ebd3(0x3b5)]=function(_0x1cc852){const _0x17bb6b=_0x39ebd3;if(_0x1cc852===null||typeof _0x1cc852!==_0x17bb6b(0x4d2))return _0x1cc852;const _0x4ae657=Array[_0x17bb6b(0x2a6)](_0x1cc852)?[]:Object[_0x17bb6b(0x268)](Object['getPrototypeOf'](_0x1cc852));for(const _0x46c49a in _0x1cc852){Object['prototype']['hasOwnProperty'][_0x17bb6b(0x15c)](_0x1cc852,_0x46c49a)&&(_0x4ae657[_0x46c49a]=typeof _0x1cc852[_0x46c49a]==='object'&&_0x1cc852[_0x46c49a]!==null?VisuMZ[_0x17bb6b(0x2a9)][_0x17bb6b(0x3b5)](_0x1cc852[_0x46c49a]):_0x1cc852[_0x46c49a]);}return _0x4ae657;};