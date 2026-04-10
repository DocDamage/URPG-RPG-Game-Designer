/*:
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/systemsettings/
 * @target MZ
 * @base CGMZ_Core
 * @orderAfter CGMZ_Core
 * @plugindesc Adds plugin commands for additional control over system settings
 * @help
 * ============================================================================
 * For terms and conditions using this plugin in your game please visit:
 * https://www.caspergaming.com/terms-of-use/
 * ============================================================================
 * Become a Patron to get access to beta/alpha plugins plus other goodies!
 * https://www.patreon.com/CasperGamingRPGM
 * ============================================================================
 * Version: 1.2.1
 * ----------------------------------------------------------------------------
 * Compatibility: Only tested with my CGMZ plugins.
 * Made for RPG Maker MZ 1.9.0
 * ----------------------------------------------------------------------------
 * Description: Adds some plugin commands that are meant to give you more
 * control over system settings after the game starts. While there are a lot of
 * system settings in the database, these are often not very easily changed
 * during gameplay. This plugin fixes that by allowing control over the
 * system settings mid-game.
 * ----------------------------------------------------------------------------
 * Documentation:
 * -----------------------Plugin Commands--------------------------------------
 * • Set Destination Sprite
 * Enable/Disable the touch UI destination sprite
 * 
 * • Set Show Number of Key Items
 * Enable/Disable showing number of key items
 * 
 * • Set Slip Death
 * Enable/Disable if slip damage can kill
 * 
 * • Set Floor Death
 * Enable/Disable the floor damage can kill
 * 
 * • Set Display TP
 * Enable/Disable if TP is shown
 * 
 * • Set Autosave
 * Enable/Disable autosave functionality
 * 
 * • Set EXP for Reserve Members
 * Enable/Disable exp for reserve party members
 * 
 * • Set Message Skip
 * Enable/Disable if message skip is enabled
 * 
 * • Set Side View Battle
 * Enable/Disable the sideview battle perspective
 * 
 * • Set Battle System
 * Alter which battle system is used
 * 
 * • Set Item Category
 * Change item categories during item select
 * 
 * • Set Menu Commands
 * Change display of menu categories
 * 
 * • Set Currency Unit
 * Change the currency unit
 * 
 * • Set Basic Term
 * Change a basic term
 * 
 * • Set Parameter Term
 * Change a parameter term
 * 
 * • Set Command Term
 * Change a command term
 * 
 * • Reset System Settings
 * Will reset the settings to match the current db
 * ------------------------------Saved Games-----------------------------------
 * This plugin completely supports saved games.
 *
 * Since the system settings are now included in save data, changing them in
 * the database system tabs will no longer affect saved games. Changes made to
 * the settings in the database will only affect new games. Use reset command
 * to set all system settings back to what they are in the database.
 * -----------------------------Filename---------------------------------------
 * The filename for this plugin MUST remain CGMZ_SystemSettings.js
 * This is what it comes as when downloaded. The filename is used to load
 * parameters and execute plugin commands. If you change it, things will begin
 * behaving incorrectly and your game will probably crash. Please do not
 * rename the js file.
 * ------------------------Latest Version--------------------------------------
 * Hi all, this latest version removes the requirement for [CGMZ] Splash Screen
 * and [CGMZ] Press Start to be ordered before this plugin.
 * 
 * Version 1.2.1
 * - Load order no longer important with other [CGMZ] plugins
 *
 * @command Set Destination Sprite
 * @desc Enables/Disables the destination sprite from touch UI display
 *
 * @arg enable
 * @type boolean
 * @desc Enable or Disable the Destination Sprite used by Touch UI
 * @default true
 *
 * @command Set Show Number of Key Items
 * @desc Enables/Disables numbers next to key items in item lists
 *
 * @arg enable
 * @type boolean
 * @desc Enable or Disable the numbers next to key items in item lists
 * @default true
 *
 * @command Set Slip Death
 * @desc Enables/Disables the ability for party members to die from slip damage
 *
 * @arg enable
 * @type boolean
 * @desc Enable or Disable slip death
 * @default true
 *
 * @command Set Floor Death
 * @desc Enables/Disables the ability for party members to die from floor damage
 *
 * @arg enable
 * @type boolean
 * @desc Enable or Disable floor death
 * @default true
 *
 * @command Set Display TP
 * @desc Enables/Disables the displaying of TP
 *
 * @arg enable
 * @type boolean
 * @desc Enable or Disable display of TP
 * @default true
 *
 * @command Set Autosave
 * @desc Enables/Disables autosave functionality
 *
 * @arg enable
 * @type boolean
 * @desc Enable or Disable autosave
 * @default true
 *
 * @command Set EXP for Reserve Members
 * @desc Enables/Disables exp for reserve members
 *
 * @arg enable
 * @type boolean
 * @desc Enable or Disable exp for reserve members
 * @default true
 *
 * @command Set Message Skip
 * @desc Enables/Disables message skip
 *
 * @arg enable
 * @type boolean
 * @desc Enable or Disable message skip
 * @default true
 *
 * @command Set Side View Battle
 * @desc Enables/Disables side view battle system
 *
 * @arg enable
 * @type boolean
 * @desc Enable or Disable side view battle system
 * @default true
 *
 * @command Set Battle System
 * @desc Set the battle system to use
 *
 * @arg system
 * @type select
 * @option Turn-based
 * @option Time Progress (Active)
 * @option Time Progress (Wait)
 * @desc Changes to the selected battle system
 * @default Turn-based
 *
 * @command Set Item Category
 * @desc Set the item categories to display on item select screens
 *
 * @arg category
 * @type select
 * @option item
 * @option armor
 * @option weapon
 * @option key item
 * @desc The category to change
 * @default item
 *
 * @arg enable
 * @type boolean
 * @desc Enable or Disable the selected category
 * @default true
 *
 * @command Set Menu Commands
 * @desc Set the menu commands to display
 *
 * @arg category
 * @type select
 * @option item
 * @option skill
 * @option equip
 * @option status
 * @option formation
 * @option save
 * @desc The command to change
 * @default item
 *
 * @arg enable
 * @type boolean
 * @desc Enable or Disable the selected command
 * @default true
 *
 * @command Set Currency Unit
 * @desc Set the currency unit
 *
 * @arg unit
 * @desc The new currency unit
 * @default G
 *
 * @command Set Basic Term
 * @desc Set a basic term setting
 *
 * @arg term
 * @desc The term to set
 * @type select
 * @option Level
 * @value level
 * @option Level (abbr.)
 * @value levela
 * @option HP
 * @value hp
 * @option HP (abbr.)
 * @value hpa
 * @option MP
 * @value mp
 * @option MP (abbr.)
 * @value mpa
 * @option TP
 * @value tp
 * @option TP (abbr.)
 * @value tpa
 * @option EXP
 * @value exp
 * @option EXP (abbr.)
 * @value expa
 * @default level
 *
 * @arg value
 * @desc The new value to set the term to
 *
 * @command Set Parameter Term
 * @desc Set a parameter term setting
 *
 * @arg term
 * @desc The term to set
 * @type select
 * @option Max HP
 * @value mhp
 * @option Max MP
 * @value mmp
 * @option Attack
 * @value atk
 * @option Defense
 * @value def
 * @option M.Attack
 * @value mat
 * @option M.Defense
 * @value mdf
 * @option Agility
 * @value agi
 * @option Luck
 * @value luk
 * @option Hit Rate
 * @value hrt
 * @option Evasion Rate
 * @value ert
 * @default mhp
 *
 * @arg value
 * @desc The new value to set the term to
 *
 * @command Set Command Term
 * @desc Set a command term setting
 *
 * @arg term
 * @desc The term to set
 * @type select
 * @option Fight
 * @value fight
 * @option Escape
 * @value escape
 * @option Attack
 * @value attack
 * @option Guard
 * @value guard
 * @option Item
 * @value item
 * @option Skill
 * @value skill
 * @option Equip
 * @value equip
 * @option Status
 * @value status
 * @option Formation
 * @value formation
 * @option Options
 * @value options
 * @option Save
 * @value save
 * @option Game End
 * @value gameend
 * @option Weapon
 * @value weapon
 * @option Armor
 * @value armor
 * @option Key Item
 * @value keyitem
 * @option Equip 2
 * @value equip2
 * @option Optimize
 * @value optimize
 * @option Clear
 * @value clear
 * @option Buy
 * @value buy
 * @option Sell
 * @value sell
 * @option New Game
 * @value newgame
 * @option Continue
 * @value continue
 * @option To Title
 * @value totitle
 * @option Cancel
 * @value cancel
 * @default fight
 *
 * @arg value
 * @desc The new value to set the term to
 *
 * @command Reset System Settings
 * @desc Resets the system settings to what they are in the database
*/
/*:es
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/systemsettings/
 * @target MZ
 * @base CGMZ_Core
 * @orderAfter CGMZ_Core
 * @plugindesc Agrega comandos de complemento para un control adicional sobre la configuración del sistema
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
 * Versión: 1.2.1
 * ----------------------------------------------------------------------------
 * Compatibilidad: Sólo probado con mis CGMZ plugins.
 * Hecho para RPG Maker MZ 1.9.0
 * ----------------------------------------------------------------------------
 * Descripción: agrega algunos comandos de complemento que están destinados a 
 * brindarle más control sobre la configuración del sistema después de que 
 * comience el juego. Mientras que hay un montón de configuración del sistema
 * en la base de datos, a menudo no es muy fácil de cambiar durante el juego.
 * Este complemento corrige eso al permitir el control sobre la Configuración
 * del sistema a mitad del juego.
 * ----------------------------------------------------------------------------
 * Documentación:
 * ----------------------Comandos de Plugin------------------------------------
 * • Establecer objeto de destino
 * Habilitar o deshabilitar el sprite/objeto de destino de la interfaz de
 * usuario táctil.
 * 
 * • Establecer mostrar número de elementos clave
 * Habilitar/Deshabilitar la visualización del número de elementos clave
 * 
 * • Establecer muerte por deslizamiento
 * Activar/Desactivar si el daño por deslizamiento puede matar
 * 
 * • Establecer muerte por piso
 * Habilitar/Deshabilitar si el daño del piso puede matar
 * 
 * • Establecer pantalla TP
 * Habilitar/Deshabilitar si se muestra el TP
 * 
 * • Establecer guardado automático
 * Habilitar/deshabilitar la función de guardado automático
 * 
 * • Establecer EXP para miembros de reserva
 * Habilitar/deshabilitar exp para miembros del grupo de reserva
 * 
 * • Set Message Skip
 * Enable/Disable if message skip is enabled
 * 
 * • Establecer batalla de vista lateral
 * Habilitar/deshabilitar la perspectiva de batalla lateral
 * 
 * • Establecer sistema de batalla
 * Modificar qué sistema de batalla se utiliza
 * 
 * • Establecer categoría de artículo
 * Cambiar las categorías de elementos durante la selección de elementos
 * 
 * • Establecer comandos de menú
 * Cambiar la visualización de las categorías del menú
 * 
 * • Establecer unidad monetaria
 * Cambiar la unidad monetaria
 * 
 * • Set Basic Term
 * Change a basic term
 * 
 * • Set Parameter Term
 * Change a parameter term
 * 
 * • Set Command Term
 * Change a command term
 * 
 * • Restablecer la configuración del sistema
 * Restablecerá la configuración para que coincida con la base de datos actual
 * ---------------------------Juegos Guardados---------------------------------
 * Este plugin es completamente compatible con los juegos guardados.
 *
 * Since the system settings are now included in save data, changing them in
 * the database system tabs will no longer affect saved games. Changes made to
 * the settings in the database will only affect new games. Use reset command
 * to set all system settings back to what they are in the database.
 * -----------------------------Filename---------------------------------------
 * The filename for this plugin MUST remain CGMZ_SystemSettings.js
 * This is what it comes as when downloaded. The filename is used to load
 * parameters and execute plugin commands. If you change it, things will begin
 * behaving incorrectly and your game will probably crash. Please do not
 * rename the js file.
 * ------------------------Latest Version--------------------------------------
 * Hi all, this latest version removes the requirement for [CGMZ] Splash Screen
 * and [CGMZ] Press Start to be ordered before this plugin.
 * 
 * Version 1.2.1
 * - Load order no longer important with other [CGMZ] plugins
 *
 * @command Set Destination Sprite
 * @text Establecer objeto de destino
 * @desc Habilita/deshabilita el sprite/objeto de destino desde la pantalla de la interfaz de usuario táctil.
 *
 * @arg enable
 * @text Objeto de destino
 * @type boolean
 * @desc  Habilitar o deshabilitar el sprite/objeto de destino utilizado por interfaz de usuario táctil.
 * @default true
 *
 * @command Set Show Number of Key Items
 * @text Establecer mostrar número de elementos clave
 * @desc Habilita/deshabilita los números junto a los elementos clave en las listas de elementos
 *
 * @arg enable
 * @text Números
 * @type boolean
 * @desc Habilite o deshabilite los números junto a los elementos clave en las listas de elementos.
 * @default true
 *
 * @command Set Slip Death
 * @text Establecer deslizamiento de la muerte
 * @desc Habilita/deshabilita la capacidad de que los miembros del grupo mueran por daños de deslizamiento.
 *
 * @arg enable
 * @text Muerte por deslizamiento
 * @type boolean
 * @desc Habilitar o deshabilitar la muerte por deslizamiento.
 * @default true
 *
 * @command Set Floor Death
 * @text Establecer muerte por el piso
 * @desc Habilita/deshabilita la capacidad de que los miembros del grupo mueran por daños en el piso.
 *
 * @arg enable
 * @text Muerte por el piso
 * @type boolean
 * @desc Habilitar o deshabilitar la muerte por el piso
 * @default true
 *
 * @command Set Display TP
 * @text Establecer pantalla TP
 * @desc Activa/Desactiva la visualización de TP
 *
 * @arg enable
 * @text Visualización de TP
 * @type boolean
 * @desc Habilitar o deshabilitar la visualización de TP.
 * @default true
 *
 * @command Set Autosave
 * @text Establecer guardado automático
 * @desc Habilita/deshabilita la función de autoguardado.
 *
 * @arg enable
 * @text Autoguardado
 * @type boolean
 * @desc Habilitar o deshabilitar el autoguardado.
 * @default true
 *
 * @command Set EXP for Reserve Members
 * @text Establecer EXP para miembros de reserva
 * @desc Habilita/deshabilita exp para miembros de reserva.
 *
 * @arg enable
 * @text EXP para miembros de reserva
 * @type boolean
 * @desc Habilitar o deshabilitar exp para miembros de reserva.
 * @default true
 *
 * @command Set Message Skip
 * @desc Enables/Disables message skip
 *
 * @arg enable
 * @type boolean
 * @desc Enable or Disable message skip
 * @default true
 *
 * @command Set Side View Battle
 * @text Establecer batalla de vista lateral
 * @desc Habilita/deshabilita el sistema de batalla de vista lateral.
 *
 * @arg enable
 * @text Sistema de batalla de vista lateral
 * @type boolean
 * @desc Habilitar o deshabilitar el sistema de batalla de vista lateral.
 * @default true
 *
 * @command Set Battle System
 * @text Establecer sistema de batalla
 * @desc Configura el sistema de batalla para usar.
 *
 * @arg system
 * @text Sistema de batalla
 * @type select
 * @option Turn-based
 * @option Time Progress (Active)
 * @option Time Progress (Wait)
 * @desc Cambia el sistema de batalla seleccionado.
 * @default Turn-based
 *
 * @command Set Item Category
 * @text Establecer categoría de artículo
 * @desc Configura las categorías de elementos para que se muestren en las pantallas de selección de elementos.
 *
 * @arg category
 * @text Categoría
 * @type select
 * @option item
 * @option armor
 * @option weapon
 * @option key item
 * @desc La categoría a cambiar
 * @default item
 *
 * @arg enable
 * @text Categoría seleccionada
 * @type boolean
 * @desc Habilitar o deshabilitar la categoría seleccionada.
 * @default true
 *
 * @command Set Menu Commands
 * @text Establecer comandos de menú
 * @desc Establecer los comandos de menú para mostrar.
 *
 * @arg category
 * @text Comando
 * @type select
 * @option item
 * @option skill
 * @option equip
 * @option status
 * @option formation
 * @option save
 * @desc El comando para cambiar.
 * @default item
 *
 * @arg enable
 * @text Comando seleccionado
 * @type boolean
 * @desc Habilitar o deshabilitar el comando seleccionado.
 * @default true
 *
 * @command Set Currency Unit
 * @text Unidad monetaria
 * @desc Establecer la unidad monetaria
 *
 * @arg unit
 * @text Unidad nueva
 * @desc La nueva unidad monetaria
 * @default G
 *
 * @command Set Basic Term
 * @desc Set a basic term setting
 *
 * @arg term
 * @desc The term to set
 * @type select
 * @option Level
 * @value level
 * @option Level (abbr.)
 * @value levela
 * @option HP
 * @value hp
 * @option HP (abbr.)
 * @value hpa
 * @option MP
 * @value mp
 * @option MP (abbr.)
 * @value mpa
 * @option TP
 * @value tp
 * @option TP (abbr.)
 * @value tpa
 * @option EXP
 * @value exp
 * @option EXP (abbr.)
 * @value expa
 * @default level
 *
 * @arg value
 * @desc The new value to set the term to
 *
 * @command Set Parameter Term
 * @desc Set a parameter term setting
 *
 * @arg term
 * @desc The term to set
 * @type select
 * @option Max HP
 * @value mhp
 * @option Max MP
 * @value mmp
 * @option Attack
 * @value atk
 * @option Defense
 * @value def
 * @option M.Attack
 * @value mat
 * @option M.Defense
 * @value mdf
 * @option Agility
 * @value agi
 * @option Luck
 * @value luk
 * @option Hit Rate
 * @value hrt
 * @option Evasion Rate
 * @value ert
 * @default mhp
 *
 * @arg value
 * @desc The new value to set the term to
 *
 * @command Set Command Term
 * @desc Set a command term setting
 *
 * @arg term
 * @desc The term to set
 * @type select
 * @option Fight
 * @value fight
 * @option Escape
 * @value escape
 * @option Attack
 * @value attack
 * @option Guard
 * @value guard
 * @option Item
 * @value item
 * @option Skill
 * @value skill
 * @option Equip
 * @value equip
 * @option Status
 * @value status
 * @option Formation
 * @value formation
 * @option Options
 * @value options
 * @option Save
 * @value save
 * @option Game End
 * @value gameend
 * @option Weapon
 * @value weapon
 * @option Armor
 * @value armor
 * @option Key Item
 * @value keyitem
 * @option Equip 2
 * @value equip2
 * @option Optimize
 * @value optimize
 * @option Clear
 * @value clear
 * @option Buy
 * @value buy
 * @option Sell
 * @value sell
 * @option New Game
 * @value newgame
 * @option Continue
 * @value continue
 * @option To Title
 * @value totitle
 * @option Cancel
 * @value cancel
 * @default fight
 *
 * @arg value
 * @desc The new value to set the term to
 *
 * @command Reset System Settings
 * @text Restablecer la configuración del sistema
 * @desc Restablecer la configuración del sistema a lo que están en la base de datos
*/
Imported.CGMZ_SystemSettings = true;
CGMZ.Versions["System Settings"] = "1.2.1";
//=============================================================================
// CGMZ_Temp
//-----------------------------------------------------------------------------
// Registration and processing for new plugin commands
//=============================================================================
//-----------------------------------------------------------------------------
// Register Plugin Commands
//-----------------------------------------------------------------------------
const alias_CGMZ_SystemSettings_registerPluginCommands = CGMZ_Temp.prototype.registerPluginCommands;
CGMZ_Temp.prototype.registerPluginCommands = function() {
	alias_CGMZ_SystemSettings_registerPluginCommands.call(this);
	PluginManager.registerCommand("CGMZ_SystemSettings", "Set Destination Sprite", this.pluginCommandSystemSettingsSetOpts.bind(this, "destinationSprite"));
	PluginManager.registerCommand("CGMZ_SystemSettings", "Set Show Number of Key Items", this.pluginCommandSystemSettingsSetOpts.bind(this, "optKeyItemsNumber"));
	PluginManager.registerCommand("CGMZ_SystemSettings", "Set Slip Death", this.pluginCommandSystemSettingsSetOpts.bind(this, "optSlipDeath"));
	PluginManager.registerCommand("CGMZ_SystemSettings", "Set Floor Death", this.pluginCommandSystemSettingsSetOpts.bind(this, "optFloorDeath"));
	PluginManager.registerCommand("CGMZ_SystemSettings", "Set Display TP", this.pluginCommandSystemSettingsSetOpts.bind(this, "optDisplayTp"));
	PluginManager.registerCommand("CGMZ_SystemSettings", "Set Autosave", this.pluginCommandSystemSettingsSetOpts.bind(this, "optAutosave"));
	PluginManager.registerCommand("CGMZ_SystemSettings", "Set EXP for Reserve Members", this.pluginCommandSystemSettingsSetOpts.bind(this, "optExtraExp"));
	PluginManager.registerCommand("CGMZ_SystemSettings", "Set Message Skip", this.pluginCommandSystemSettingsSetOpts.bind(this, "optMessageSkip"));
	PluginManager.registerCommand("CGMZ_SystemSettings", "Set Side View Battle", this.pluginCommandSystemSettingsSetOpts.bind(this, "optSideView"));
	PluginManager.registerCommand("CGMZ_SystemSettings", "Set Battle System", this.pluginCommandSystemSettingsSetBattleSystem);
	PluginManager.registerCommand("CGMZ_SystemSettings", "Set Item Category", this.pluginCommandSystemSettingsSetItemCategory);
	PluginManager.registerCommand("CGMZ_SystemSettings", "Set Menu Commands", this.pluginCommandSystemSettingsSetMenuCommands);
	PluginManager.registerCommand("CGMZ_SystemSettings", "Set Currency Unit", this.pluginCommandSystemSettingsSetCurrencyUnit);
	PluginManager.registerCommand("CGMZ_SystemSettings", "Set Basic Term", this.pluginCommandSystemSettingsSetBasicTerm);
	PluginManager.registerCommand("CGMZ_SystemSettings", "Set Parameter Term", this.pluginCommandSystemSettingsSetParameterTerm);
	PluginManager.registerCommand("CGMZ_SystemSettings", "Set Command Term", this.pluginCommandSystemSettingsSetCommandTerm);
	PluginManager.registerCommand("CGMZ_SystemSettings", "Reset System Settings", this.pluginCommandSystemSettingsReset);
};
//-----------------------------------------------------------------------------
// Plugin Command - Enables/Disables the boolean system settings such as autosave or display TP
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandSystemSettingsSetOpts = function(setting, args) {
	const enable = (args.enable === 'true');
	$cgmz.setSystemSetting(setting, enable);
};
//-----------------------------------------------------------------------------
// Plugin Command - Sets the battle system to use
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandSystemSettingsSetBattleSystem = function(args) {
	const battleSystems = ["Turn-based", "Time Progress (Active)", "Time Progress (Wait)"];
	const id = battleSystems.indexOf(args.system);
	$cgmz.setSystemSetting("battleSystem", id);
};
//-----------------------------------------------------------------------------
// Plugin Command - Sets the item category to enable/disable
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandSystemSettingsSetItemCategory = function(args) {
	const itemCategories = ["item", "weapon", "armor", "key item"];
	const enable = (args.enable === 'true');
	const id = itemCategories.indexOf(args.category);
	const cgmzItemCategories = $cgmz.getSystemSetting("itemCategories");
	cgmzItemCategories[id] = enable;
	$cgmz.setSystemSetting("itemCategories", cgmzItemCategories);
};
//-----------------------------------------------------------------------------
// Plugin Command - Sets the menu command to enable/disable
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandSystemSettingsSetMenuCommands = function(args) {
	const menuCommands = ["item", "skill", "equip", "status", "formation", "save"];
	const enable = (args.enable === 'true');
	const id = menuCommands.indexOf(args.category);
	const cgmzMenuCommands = $cgmz.getSystemSetting("menuCommands");
	cgmzMenuCommands[id] = enable;
	$cgmz.setSystemSetting("menuCommands", cgmzMenuCommands);
};
//-----------------------------------------------------------------------------
// Plugin Command - Sets the currency unit
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandSystemSettingsSetCurrencyUnit = function(args) {
	$cgmz.setSystemSetting("currencyUnit", args.unit);
};
//-----------------------------------------------------------------------------
// Plugin Command - Sets a basic term setting
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandSystemSettingsSetBasicTerm = function(args) {
	const terms = ["level","levela","hp","hpa","mp","mpa","tp","tpa","exp","expa"];
	const id = terms.indexOf(args.term);
	$cgmz.setSystemSettingTerm("basic", id, args.value);
};
//-----------------------------------------------------------------------------
// Plugin Command - Sets a parameter term setting
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandSystemSettingsSetParameterTerm = function(args) {
	const terms = ["mhp","mmp","atk","def","mat","mdf","agi","luk","hrt","ert"];
	const id = terms.indexOf(args.term);
	$cgmz.setSystemSettingTerm("params", id, args.value);
};
//-----------------------------------------------------------------------------
// Plugin Command - Sets a command term setting
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandSystemSettingsSetCommandTerm = function(args) {
	const terms = [
		"fight","escape","attack","guard",
		"item","skill","equip","status",
		"formation","save","gameend","options",
		"weapon","armor","keyitem","equip2",
		"optimize","clear","newgame","continue",
		"UNUSED","totitle","cancel","UNUSED","buy","sell"
	];
	const id = terms.indexOf(args.term);
	$cgmz.setSystemSettingTerm("commands", id, args.value);
};
//-----------------------------------------------------------------------------
// Plugin Command - Reset system settings
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandSystemSettingsReset = function() {
	$dataSystem = JSON.parse(JSON.stringify($cgmzDeepCopyDataSystem));
	$cgmz.initSystemSettings();
};
//=============================================================================
// CGMZ_Core
//-----------------------------------------------------------------------------
// Handling of new system setting options
//=============================================================================
//-----------------------------------------------------------------------------
// Also initialize system settings
//-----------------------------------------------------------------------------
const alias_CGMZ_SystemSettings_createPluginData = CGMZ_Core.prototype.createPluginData;
CGMZ_Core.prototype.createPluginData = function() {
	alias_CGMZ_SystemSettings_createPluginData.call(this);
	this.initSystemSettings();
};
//-----------------------------------------------------------------------------
// Init cgmz system settings (if no exist) or use cgmz system settings
//-----------------------------------------------------------------------------
const alias_CGMZ_SystemSettings_onAfterLoad = CGMZ_Core.prototype.onAfterLoad;
CGMZ_Core.prototype.onAfterLoad = function() {
	alias_CGMZ_SystemSettings_onAfterLoad.call(this);
	if(!this._systemSettings) {
		$dataSystem = JSON.parse(JSON.stringify($cgmzDeepCopyDataSystem));
		this.initSystemSettings();
	} else {
		if(!this._systemSettings.terms) {
			this._systemSettings.terms = {};
			this._systemSettings.terms.basic = JSON.parse(JSON.stringify($cgmzDeepCopyDataSystem.terms.basic));
			this._systemSettings.terms.params = JSON.parse(JSON.stringify($cgmzDeepCopyDataSystem.terms.params));
			this._systemSettings.terms.commands = JSON.parse(JSON.stringify($cgmzDeepCopyDataSystem.terms.commands));
		}
		if(!this._systemSettings.optMessageSkip) this._systemSettings.optMessageSkip = $dataSystem.optMessageSkip;
		this.updateSystemDataSettings();
	}
};
//-----------------------------------------------------------------------------
// Initialize system settings
//-----------------------------------------------------------------------------
CGMZ_Core.prototype.initSystemSettings = function() {
	this._systemSettings = {};
	this._systemSettings.destinationSprite = true;
	this._systemSettings.optKeyItemsNumber = $dataSystem.optKeyItemsNumber;
	this._systemSettings.optSlipDeath = $dataSystem.optSlipDeath;
	this._systemSettings.optFloorDeath = $dataSystem.optFloorDeath;
	this._systemSettings.optDisplayTp = $dataSystem.optDisplayTp;
	this._systemSettings.optAutosave = $dataSystem.optAutosave;
	this._systemSettings.optExtraExp = $dataSystem.optExtraExp;
	this._systemSettings.optSideView = $dataSystem.optSideView;
	this._systemSettings.optMessageSkip = $dataSystem.optMessageSkip;
	this._systemSettings.battleSystem = $dataSystem.battleSystem;
	this._systemSettings.itemCategories = [...$dataSystem.itemCategories];
	this._systemSettings.menuCommands = [...$dataSystem.menuCommands];
	this._systemSettings.currencyUnit = $dataSystem.currencyUnit;
	this._systemSettings.terms = {};
	this._systemSettings.terms.basic = [...$dataSystem.terms.basic];
	this._systemSettings.terms.params = [...$dataSystem.terms.params];
	this._systemSettings.terms.commands = [...$dataSystem.terms.commands];
};
//-----------------------------------------------------------------------------
// Set data settings
//-----------------------------------------------------------------------------
CGMZ_Core.prototype.updateSystemDataSettings = function() {
	$dataSystem.optKeyItemsNumber = this._systemSettings.optKeyItemsNumber;
	$dataSystem.optSlipDeath = this._systemSettings.optSlipDeath;
	$dataSystem.optFloorDeath = this._systemSettings.optFloorDeath;
	$dataSystem.optDisplayTp = this._systemSettings.optDisplayTp;
	$dataSystem.optAutosave = this._systemSettings.optAutosave;
	$dataSystem.optExtraExp = this._systemSettings.optExtraExp;
	$dataSystem.optSideView = this._systemSettings.optSideView;
	$dataSystem.optMessageSkip = this._systemSettings.optMessageSkip;
	$dataSystem.battleSystem = this._systemSettings.battleSystem;
	$dataSystem.itemCategories = [...this._systemSettings.itemCategories];
	$dataSystem.menuCommands = [...this._systemSettings.menuCommands];
	$dataSystem.currencyUnit = this._systemSettings.currencyUnit;
	$dataSystem.terms.basic = [...this._systemSettings.terms.basic];
	$dataSystem.terms.params = [...this._systemSettings.terms.params];
	$dataSystem.terms.commands = [...this._systemSettings.terms.commands];
};
//-----------------------------------------------------------------------------
// Setter for system settings
//-----------------------------------------------------------------------------
CGMZ_Core.prototype.setSystemSetting = function(setting, value) {
	this._systemSettings[setting] = value;
	this.updateSystemDataSettings();
};
//-----------------------------------------------------------------------------
// Getter for system settings
//-----------------------------------------------------------------------------
CGMZ_Core.prototype.getSystemSetting = function(setting) {
	return this._systemSettings[setting];
};
//-----------------------------------------------------------------------------
// Setter for system settings terms
//-----------------------------------------------------------------------------
CGMZ_Core.prototype.setSystemSettingTerm = function(setting, id, value) {
	this._systemSettings.terms[setting][id] = value;
	this.updateSystemDataSettings();
};
//-----------------------------------------------------------------------------
// Getter for system settings terms
//-----------------------------------------------------------------------------
CGMZ_Core.prototype.getSystemSettingTerm = function(setting, id) {
	return this._systemSettings.terms[setting][id];
};
//=============================================================================
// Scene_Boot
//-----------------------------------------------------------------------------
// Make a deep copy of data system on database load
//=============================================================================
$cgmzDeepCopyDataSystem = null;
//-----------------------------------------------------------------------------
// Deep copy data system
//-----------------------------------------------------------------------------
const alias_CGMZ_SystemSettings_SceneBoot_startNormalGame = Scene_Boot.prototype.startNormalGame;
Scene_Boot.prototype.startNormalGame = function() {
    alias_CGMZ_SystemSettings_SceneBoot_startNormalGame.call(this);
	$cgmzDeepCopyDataSystem = JSON.parse(JSON.stringify($dataSystem));
};
//=============================================================================
// Scene_Title
//-----------------------------------------------------------------------------
// New game should also include reloading the data system file
//=============================================================================
//-----------------------------------------------------------------------------
// Reset data system to what it was on game boot
//-----------------------------------------------------------------------------
const alias_CGMZ_SystemSettings_SceneTitle_commandNewGame = Scene_Title.prototype.commandNewGame;
Scene_Title.prototype.commandNewGame = function() {
	$dataSystem = JSON.parse(JSON.stringify($cgmzDeepCopyDataSystem));
	alias_CGMZ_SystemSettings_SceneTitle_commandNewGame.call(this);
};
//=============================================================================
// Sprite_Destination
//-----------------------------------------------------------------------------
// Do not display sprite if setting is false
//=============================================================================
//-----------------------------------------------------------------------------
// Do not update or display if turned off
//-----------------------------------------------------------------------------
const alias_CGMZ_SystemSettings_SpriteDestination_update = Sprite_Destination.prototype.update;
Sprite_Destination.prototype.update = function() {
	if($cgmz.getSystemSetting("destinationSprite")) {
		alias_CGMZ_SystemSettings_SpriteDestination_update.call(this);
	} else {
		Sprite.prototype.update.call(this);
		this._frameCount = 0;
        this.visible = false;
	}
};