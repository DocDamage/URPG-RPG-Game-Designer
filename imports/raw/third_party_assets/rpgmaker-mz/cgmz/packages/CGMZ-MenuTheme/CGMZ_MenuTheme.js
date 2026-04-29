/*:
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/menutheme/
 * @target MZ
 * @base CGMZ_Core
 * @orderAfter CGMZ_Core
 * @plugindesc Add a BGM to the menu
 * @help
 * ============================================================================
 * For terms and conditions using this plugin in your game please visit:
 * https://www.caspergaming.com/terms-of-use/
 * ============================================================================
 * Become a Patron to get access to beta/alpha plugins plus other goodies!
 * https://www.patreon.com/CasperGamingRPGM
 * ============================================================================
 * Version: 1.1.3
 * ----------------------------------------------------------------------------
 * Compatibility: Only tested with my CGMZ plugins.
 * Made for RPG Maker MZ 1.8.1
 * ----------------------------------------------------------------------------
 * Description: Adds a BGM to the menu. It will autoplay the previous BGM
 * when exiting menu. The menu theme will persist through all different menus
 * within the main menu (such as item, status, or save).
 * ----------------------------------------------------------------------------
 * Documentation:
 * If NOT using variables to control menu theme, leave variable set to 0. Then
 * set the default menu theme.
 *
 * If using variables to control  menu theme, the first variable menu theme
 * corresponds to a variable value of 0. The second variable menu theme
 * corresponds to a variable value of 1, and so on. If your variable has a
 * value greater than the number of menu themes possible, it will default to
 * the first menu theme in the variable menu themes array.
 * -------------------------Plugin Commands------------------------------------
 * This plugin does not have any plugin commands.
 * ------------------------------Saved Games-----------------------------------
 * This plugin is fully compatible with saved games. This means you can:
 *
 * ✓ Add this plugin to a saved game and it will work as expected
 * ✓ Change any plugin params and changes will be reflected in saved games
 * ✓ Remove the plugin with no issue to save data
 * -----------------------------Filename---------------------------------------
 * The filename for this plugin MUST remain CGMZ_MenuTheme.js
 * This is what it comes as when downloaded. The filename is used to load
 * parameters and execute plugin commands. If you change it, things will begin
 * behaving incorrectly and your game will probably crash. Please do not
 * rename the js file.
 * --------------------------Latest Version------------------------------------
 * Hi all, this latest version adds volume, pitch, and pan parameters to your
 * bgms. These are no longer hard coded to be the RPG Maker defaults.
 *
 * You can now also have the menu theme start playing from the beginning every
 * time the menu is opened.
 *
 * Version 1.1.3
 * - Added volume, pitch, and pan settings for menu themes
 * - Added option to reset the menu theme every time it starts playing
 *
 * @param Default Menu Theme
 * @type struct<BGM>
 * @desc The menu theme BGM to play if not using variable
 *
 * @param Reset Position
 * @type boolean
 * @default false
 * @desc If true, will restart the menu theme from the beginning every time it starts playing
 *
 * @param Menu Theme Variable
 * @type variable
 * @default 0
 * @desc The variable which controls which menu theme is played
 *
 * @param Variable Menu Themes
 * @type struct<BGM>[]
 * @desc The menu theme BGMs to play by variable value (0 = first, 1 = second, etc).
*/
/*~struct~BGM:
 * @param Name
 * @type file
 * @dir audio/bgm/
 * @desc The BGM file to play
 * 
 * @param Volume
 * @type number
 * @min 0
 * @max 100
 * @default 90
 * @desc The volume of the BGM
 * 
 * @param Pitch
 * @type number
 * @min 50
 * @max 150
 * @default 100
 * @desc The pitch of the BGM
 * 
 * @param Pan
 * @type number
 * @min -100
 * @max 100
 * @default 0
 * @desc The pan of the BGM
*/
/*:es
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/menutheme/
 * @target MZ
 * @base CGMZ_Core
 * @orderAfter CGMZ_Core
 * @plugindesc Agregar una música de fondo al menú
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
 * Versión: 1.1.3
 * ----------------------------------------------------------------------------
 * Compatibilidad: Sólo probado con mis CGMZ plugins.
 * Hecho para RPG Maker MZ 1.8.1
 * ----------------------------------------------------------------------------
 * Descripción: Agrega música de fondo al menú. Se reproducirá automáticamente 
 * la musica se fondo anterior al salir del menú. El tema del menú persistirá 
 * a través de todos los diferentes menús dentro del menú principal (como 
 * elemento, estado o guardar).
 * ----------------------------------------------------------------------------
 * Documentación:
 * Si NO usa variables para controlar el tema del menú, laeve la variable 
 * establecida en 0. Entonces establecer el tema del menú predeterminado.
 *
 * Si usa variables para controlar el tema del menú, el primer tema del menú 
 * variable corresponde a un valor de variable de 0. El segundo tema del menú 
 * de variable corresponde a un valor de variable de 1, y así sucesivamente. 
 * Si su variable tiene un valor mayor que la cantidad de temas de menú 
 * posibles, se establecerá de forma predeterminada en el primer tema de menú
 * en la matriz de temas de menú de variables.
 * -------------------------Plugin Commands------------------------------------
 * This plugin does not have any plugin commands.
 * ------------------------------Saved Games-----------------------------------
 * This plugin is fully compatible with saved games. This means you can:
 *
 * ✓ Add this plugin to a saved game and it will work as expected
 * ✓ Change any plugin params and changes will be reflected in saved games
 * ✓ Remove the plugin with no issue to save data
 * -----------------------------Filename---------------------------------------
 * The filename for this plugin MUST remain CGMZ_MenuTheme.js
 * This is what it comes as when downloaded. The filename is used to load
 * parameters and execute plugin commands. If you change it, things will begin
 * behaving incorrectly and your game will probably crash. Please do not
 * rename the js file.
 * --------------------------Latest Version------------------------------------
 * Hi all, this latest version adds volume, pitch, and pan parameters to your
 * bgms. These are no longer hard coded to be the RPG Maker defaults.
 *
 * You can now also have the menu theme start playing from the beginning every
 * time the menu is opened.
 *
 * Version 1.1.3
 * - Added volume, pitch, and pan settings for menu themes
 * - Added option to reset the menu theme every time it starts playing
 *
 * @param Default Menu Theme
 * @text Tema de menú predeterminado
 * @type struct<BGM>
 * @desc El elemento del menú Música de fondo para reproducir si no se utiliza ninguna variable.
 *
 * @param Reset Position
 * @type boolean
 * @default false
 * @desc If true, will restart the menu theme from the beginning every time it starts playing
 *
 * @param Menu Theme Variable
 * @text Variable de tema de menú
 * @type variable
 * @default 0
 * @desc La variable que controla qué tema del menú se reproduce.
 *
 * @param Variable Menu Themes
 * @text Temas de menú variables
 * @type struct<BGM>[]
 * @desc El tema del menú Música de fondo para reproducir por valor variable (0 = primero, 1 = segundo, etc.).
*/
/*~struct~BGM:es
 * @param Name
 * @type file
 * @dir audio/bgm/
 * @desc The BGM file to play
 * 
 * @param Volume
 * @type number
 * @min 0
 * @max 100
 * @default 90
 * @desc The volume of the BGM
 * 
 * @param Pitch
 * @type number
 * @min 50
 * @max 150
 * @default 100
 * @desc The pitch of the BGM
 * 
 * @param Pan
 * @type number
 * @min -100
 * @max 100
 * @default 0
 * @desc The pan of the BGM
*/
Imported.CGMZ_MenuTheme = true;
CGMZ.Versions["Menu Theme"] = "1.1.3";
CGMZ.MenuTheme = {};
CGMZ.MenuTheme.parameters = PluginManager.parameters('CGMZ_MenuTheme');
CGMZ.MenuTheme.Variable = Number(CGMZ.MenuTheme.parameters["Menu Theme Variable"]);
CGMZ.MenuTheme.ResetPosition = (CGMZ.MenuTheme.parameters["Reset Position"] === 'true');
CGMZ.MenuTheme.Theme = CGMZ_Utils.parseSoundEffectJSON(CGMZ.MenuTheme.parameters["Default Menu Theme"]);
CGMZ.MenuTheme.VariableThemes = CGMZ_Utils.parseJSON(CGMZ.MenuTheme.parameters["Variable Menu Themes"], [], '[CGMZ] Menu Theme', "Your Variable Themes had invalid json and could not be read.").map(themeJ => {
	const theme = CGMZ_Utils.parseSoundEffectJSON(themeJ, '[CGMZ] Menu Theme');
	if(!theme.name) return null;
	return theme;
}).filter(x => !!x);
//=============================================================================
// Game System
//-----------------------------------------------------------------------------
// Do not save menu BGM (instead save previous BGM)
//=============================================================================
//-----------------------------------------------------------------------------
// Save the correct BGM (not Menu Theme)
//-----------------------------------------------------------------------------
const alias_CGMZ_MenuTheme_GameSystem_onBeforeSave = Game_System.prototype.onBeforeSave;
Game_System.prototype.onBeforeSave = function() {
	alias_CGMZ_MenuTheme_GameSystem_onBeforeSave.call(this);
	if($cgmzTemp.isPlayingMenuTheme()) {
		this._bgmOnSave = $cgmzTemp.getSavedBgmForMenuTheme();
		this._bgsOnSave = $cgmzTemp.getSavedBgsForMenuTheme();
	}
};
//=============================================================================
// Scene Menu
//-----------------------------------------------------------------------------
// Handling for playing menu theme
//=============================================================================
//-----------------------------------------------------------------------------
// Play menu theme if player came from map
//-----------------------------------------------------------------------------
const alias_CGMZ_MenuTheme_SceneMenu_start = Scene_Menu.prototype.start;
Scene_Menu.prototype.start = function() {
	alias_CGMZ_MenuTheme_SceneMenu_start.call(this);
	if(SceneManager.isPreviousScene(Scene_Map)) {
		$cgmzTemp.playMenuTheme();
	}
};
//=============================================================================
// Scene Map
//-----------------------------------------------------------------------------
// Handling for replaying map bgm/bgs after menu
//=============================================================================
//-----------------------------------------------------------------------------
// Replay map music if playing menu theme
//-----------------------------------------------------------------------------
const alias_CGMZ_MenuTheme_SceneMap_start = Scene_Map.prototype.start;
Scene_Map.prototype.start = function() {
	alias_CGMZ_MenuTheme_SceneMap_start.call(this);
	if($cgmzTemp._previousBGMForMenuTheme || $cgmzTemp._previousBGSForMenuTheme) {
		$cgmzTemp.replayPreviousMenuThemeBgm();
	}
};
//=============================================================================
// CGMZ Temp
//-----------------------------------------------------------------------------
// Handles saving past BGM and switching which BGM is playing
//=============================================================================
//-----------------------------------------------------------------------------
// Initialize the previous BGM for menu theme to empty sound file
//-----------------------------------------------------------------------------
const alias_CGMZ_MenuTheme_CGMZTemp_createPluginData = CGMZ_Temp.prototype.createPluginData;
CGMZ_Temp.prototype.createPluginData = function() {
	alias_CGMZ_MenuTheme_CGMZTemp_createPluginData.call(this);
	this._previousBGMForMenuTheme = null;
	this._previousBGSForMenuTheme = null;
	this._isPlayingMenuTheme = false;
	if(CGMZ.MenuTheme.Variable === 0) {
		this._menuThemeBgm = CGMZ.MenuTheme.Theme;
	} else {
		const variableValue = $gameVariables.value(CGMZ.MenuTheme.Variable);
		this._menuThemeBgm = CGMZ.MenuTheme.VariableThemes[variableValue] || CGMZ.MenuTheme.Theme;
	}
	this._menuThemeBgm.pos = 0;
};
//-----------------------------------------------------------------------------
// Set the menu theme to play
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.setMenuThemeToPlay = function() {
	if(CGMZ.MenuTheme.Variable !== 0) {
		const variableValue = $gameVariables.value(CGMZ.MenuTheme.Variable);
		const name = CGMZ.MenuTheme.VariableThemes[variableValue].name;
		if(name !== this._menuThemeBgm.name) {
			this._menuThemeBgm = CGMZ.MenuTheme.VariableThemes[variableValue] || CGMZ.MenuTheme.Theme;
			this._menuThemeBgm.pos = 0;
		}
	}
};
//-----------------------------------------------------------------------------
// Play menu theme, save previous bgm
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.playMenuTheme = function() {
	this._isPlayingMenuTheme = true;
	this._previousBGMForMenuTheme = AudioManager.saveBgm();
	this._previousBGSForMenuTheme = AudioManager.saveBgs();
	AudioManager.stopBgs();
	this.setMenuThemeToPlay();
	if(this._menuThemeBgm.pos !== 0 && !CGMZ.MenuTheme.ResetPosition) {
		AudioManager.replayBgm(this._menuThemeBgm);
	} else {
		AudioManager.playBgm(this._menuThemeBgm);
	}
};
//-----------------------------------------------------------------------------
// Replay saved bgm for menu theme
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.replayPreviousMenuThemeBgm = function() {
	this._isPlayingMenuTheme = false;
	if(this._previousBGSForMenuTheme) {
		AudioManager.replayBgs(this._previousBGSForMenuTheme);
		this._previousBGSForMenuTheme = null;
	}
    if(this._previousBGMForMenuTheme) {
		this._menuThemeBgm = AudioManager.saveBgm();
		AudioManager.replayBgm(this._previousBGMForMenuTheme);
		this._previousBGMForMenuTheme = null;
	}
};
//-----------------------------------------------------------------------------
// Determine if currently playing menu theme
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.isPlayingMenuTheme = function() {
	return this._isPlayingMenuTheme;
};
//-----------------------------------------------------------------------------
// Get the saved BGM if menu theme is playing
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.getSavedBgmForMenuTheme = function() {
	if(this._previousBGMForMenuTheme) {
		return this._previousBGMForMenuTheme;
	}
	return AudioManager.makeEmptyAudioObject();
};
//-----------------------------------------------------------------------------
// Get the saved BGS if menu theme is playing
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.getSavedBgsForMenuTheme = function() {
	if(this._previousBGSForMenuTheme) {
		return this._previousBGSForMenuTheme;
	}
	return AudioManager.makeEmptyAudioObject();
};
//=============================================================================
// Game_Player
//-----------------------------------------------------------------------------
// Also clear any saved bgm/bgs when transferred
//=============================================================================
//-----------------------------------------------------------------------------
// Also clear saved bgm/bgs.
// Needed for compatibility with CGMZ Fast Travel (and probably other plugins that transfer from the menu without first going to the map)
//-----------------------------------------------------------------------------
const alias_CGMZ_MenuTheme_GamePlayer_clearTransferInfo = Game_Player.prototype.clearTransferInfo;
Game_Player.prototype.clearTransferInfo = function() {
	$cgmzTemp._previousBGMForMenuTheme = null;
	$cgmzTemp._previousBGSForMenuTheme = null;
	alias_CGMZ_MenuTheme_GamePlayer_clearTransferInfo.call(this);
};