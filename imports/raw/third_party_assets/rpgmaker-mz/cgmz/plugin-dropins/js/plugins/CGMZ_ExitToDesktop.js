/*:
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/exittodesktop/
 * @target MZ
 * @base CGMZ_Core
 * @orderAfter CGMZ_Core
 * @orderAfter CGMZ_GameOver
 * @plugindesc Adds options to close the game window (exit to desktop)
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
 * Description: This plugin adds an option to close the game completely in 
 * the title screen, game end screen, and (if using CGMZ GameOver) in the game
 * over screen as well.
 * ----------------------------------------------------------------------------
 * Documentation:
 * -------------------------Plugin Commands------------------------------------
 * This plugin includes the following plugin commands:
 *
 * • Exit Game
 * Exits the game, note that this will not work in browser deploys (it is set
 * to do nothing in browser deploys).
 * ------------------------------Saved Games-----------------------------------
 * This plugin is fully compatible with saved games. This means you can:
 *
 * ✓ Add this plugin to a saved game and it will work as expected
 * ✓ Change any plugin params and changes will be reflected in saved games
 * ✓ Remove the plugin with no issue to save data
 * -----------------------------Filename---------------------------------------
 * The filename for this plugin MUST remain CGMZ_ExitToDesktop.js
 * This is what it comes as when downloaded. The filename is used to load
 * parameters and execute plugin commands. If you change it, things will begin
 * behaving incorrectly and your game will probably crash. Please do not
 * rename the js file.
 * --------------------------Latest Version------------------------------------
 * Hi all, this latest version adds a plugin command that can exit the game,
 * allowing you to event an exit game choice or otherwise use the option to
 * exit the game in your events.
 *
 * Version 1.1.0
 * - Added plugin command to exit the game
 *
 * @command Exit Game
 * @desc Exits the game. No effect in browser deploys.
 *
 * @param Command Name
 * @default Exit Game
 * @desc The text to show for the command
 *
 * @param Hide in Browser
 * @type boolean
 * @default true
 * @desc Hide exit game command if the player is in web browser?
 *
 * @param Hide in Title
 * @type boolean
 * @default false
 * @desc Hide exit game command in the title screen?
 *
 * @param Hide in Game End
 * @type boolean
 * @default false
 * @desc Hide exit game command in the game end screen?
 *
 * @param Hide in Game Over
 * @type boolean
 * @default false
 * @desc Hide exit game command in the game over screen (Requires [CGMZ] Game Over to display)?
*/
/*:zh-CN
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/exittodesktop/
 * @target MZ
 * @base CGMZ_Core
 * @orderAfter CGMZ_Core
 * @orderAfter CGMZ_GameOver
 * @plugindesc 退出游戏插件（增加直接关闭游戏的按钮）
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
 * 【插件版本】V 1.1.0
 * ----------------------------------------------------------------------------
 * 【兼容性】仅测试作者所制作的插件
 * 【RM版本】RPG Maker MZ 1.9.0
 * ----------------------------------------------------------------------------
 * 【插件描述】
 * 在标题画面、游戏菜单的结束游戏时以及游戏GAME OVER界面增加一个退出游戏的选项。
 * 游戏GAME OVER画面的退出选项需要使用CGMZ_GameOver插件。
 * 
 * 【搭配插件】
 * CGMZ Core:核心插件，运行作者插件的必须插件!!!
 * CGMZ Game Over:游戏结束画面增强插件，GAME OVER时可以设置读档、回到标题画面或关闭游戏。
 * 注：本插件在插件列表中必须置于上述插件之下。
 * ----------------------------------------------------------------------------
 * 【使用说明】本插件不支持插件指令。
 * ---------------------------------------------------------------------------
 *【版本更新历史】
 * Hi all, this latest version adds a plugin command that can exit the game,
 * allowing you to event an exit game choice or otherwise use the option to
 * exit the game in your events.
 *
 * Version 1.1.0
 * - Added plugin command to exit the game
 *
 * @command Exit Game
 * @desc Exits the game. No effect in browser deploys.
 *
 * @param Command Name
 * @text 退出游戏的描述
 * @default 退出游戏
 * @desc 关于退出游戏选项的文字描述。
 *
 * @param Hide in Browser
 * @text 浏览器中隐藏
 * @type boolean
 * @default true
 * @desc 当玩家在网页浏览器中游戏时，隐藏退出游戏的选项？
 *
 * @param Hide in Title
 * @type boolean
 * @default false
 * @desc Hide exit game command in the title screen?
 *
 * @param Hide in Game End
 * @type boolean
 * @default false
 * @desc Hide exit game command in the game end screen?
 *
 * @param Hide in Game Over
 * @type boolean
 * @default false
 * @desc Hide exit game command in the game over screen (Requires [CGMZ] Game Over to display)?
*/
/*:es
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/exittodesktop/
 * @target MZ
 * @base CGMZ_Core
 * @orderAfter CGMZ_Core
 * @orderAfter CGMZ_GameOver
 * @plugindesc Agrega opciones para cerrar la ventana del juego (salir al escritorio)
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
 * Descripción: Este plugin agrega una opción para cerrar el juego por completo 
 * en la pantalla de título, la pantalla final del juego y (si usa CGMZ
 * GameOver) también en la pantalla de fin del juego.
 * ----------------------------------------------------------------------------
 * Documentación:
 * -------------------------Plugin Commands------------------------------------
 * This plugin includes the following plugin commands:
 *
 * • Exit Game
 * Exits the game, note that this will not work in browser deploys (it is set
 * to do nothing in browser deploys).
 * ------------------------------Saved Games-----------------------------------
 * This plugin is fully compatible with saved games. This means you can:
 *
 * ✓ Add this plugin to a saved game and it will work as expected
 * ✓ Change any plugin params and changes will be reflected in saved games
 * ✓ Remove the plugin with no issue to save data
 * -----------------------------Filename---------------------------------------
 * The filename for this plugin MUST remain CGMZ_ExitToDesktop.js
 * This is what it comes as when downloaded. The filename is used to load
 * parameters and execute plugin commands. If you change it, things will begin
 * behaving incorrectly and your game will probably crash. Please do not
 * rename the js file.
 * --------------------------Latest Version------------------------------------
 * Hi all, this latest version adds a plugin command that can exit the game,
 * allowing you to event an exit game choice or otherwise use the option to
 * exit the game in your events.
 *
 * Version 1.1.0
 * - Added plugin command to exit the game
 *
 * @command Exit Game
 * @desc Exits the game. No effect in browser deploys.
 *
 * @param Command Name
 * @text Nombre de comando
 * @default Exit Game
 * @desc El texto a mostrar para el comando.
 *
 * @param Hide in Browser
 * @text Ocultar en el navegador
 * @type boolean
 * @default true
 * @desc ¿Ocultar el comando de salida del juego si el jugador está en el navegador web?
 *
 * @param Hide in Title
 * @type boolean
 * @default false
 * @desc Hide exit game command in the title screen?
 *
 * @param Hide in Game End
 * @type boolean
 * @default false
 * @desc Hide exit game command in the game end screen?
 *
 * @param Hide in Game Over
 * @type boolean
 * @default false
 * @desc Hide exit game command in the game over screen (Requires [CGMZ] Game Over to display)?
*/
Imported.CGMZ_ExitToDesktop = true;
CGMZ.Versions["Exit to Desktop"] = "1.1.0";
CGMZ.ExitToDesktop = {};
CGMZ.ExitToDesktop.parameters = PluginManager.parameters('CGMZ_ExitToDesktop');
CGMZ.ExitToDesktop.CommandName = CGMZ.ExitToDesktop.parameters["Command Name"];
CGMZ.ExitToDesktop.HideInBrowser = (CGMZ.ExitToDesktop.parameters["Hide in Browser"] === "true");
CGMZ.ExitToDesktop.HideInTitle = (CGMZ.ExitToDesktop.parameters["Hide in Title"] === "true");
CGMZ.ExitToDesktop.HideInGameEnd = (CGMZ.ExitToDesktop.parameters["Hide in Game End"] === "true");
CGMZ.ExitToDesktop.HideInGameOver = (CGMZ.ExitToDesktop.parameters["Hide in Game Over"] === "true");
//=============================================================================
// CGMZ_Temp
//-----------------------------------------------------------------------------
// Add plugin commands
//=============================================================================
//-----------------------------------------------------------------------------
// Register Plugin Commands
//-----------------------------------------------------------------------------
const alias_CGMZExitToDesktop_CGMZTemp_registerPluginCommands = CGMZ_Temp.prototype.registerPluginCommands;
CGMZ_Temp.prototype.registerPluginCommands = function() {
	alias_CGMZExitToDesktop_CGMZTemp_registerPluginCommands.call(this);
	PluginManager.registerCommand("CGMZ_ExitToDesktop", "Exit Game", this.pluginCommandExitToDesktopExitGame);
};
//-----------------------------------------------------------------------------
// Plugin Command - Exit Game
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandExitToDesktopExitGame = function() {
	if(Utils.isNwjs()) SceneManager.exit();
};
//=============================================================================
// Scene_Title
//-----------------------------------------------------------------------------
// Add additional command to the title window
//=============================================================================
//-----------------------------------------------------------------------------
// Add command to exit the game to the command window
//-----------------------------------------------------------------------------
const alias_CGMZ_ExitToDesktop_SceneTitle_createCommandWindow = Scene_Title.prototype.createCommandWindow;
Scene_Title.prototype.createCommandWindow = function() {
	alias_CGMZ_ExitToDesktop_SceneTitle_createCommandWindow.call(this);
	this._commandWindow.setHandler("CGMZ_exitToDesktop", this.CGMZ_commandExitToDesktop.bind(this));
};
//-----------------------------------------------------------------------------
// Exit the game handling
//-----------------------------------------------------------------------------
Scene_Title.prototype.CGMZ_commandExitToDesktop = function() {
	SceneManager.exit();
};
//=============================================================================
// Window_TitleCommand
//-----------------------------------------------------------------------------
// Add additional command to the title window
//=============================================================================
//-----------------------------------------------------------------------------
// Add command to exit the game to the command window
//-----------------------------------------------------------------------------
const alias_CGMZ_ExitToDesktop_WindowTitleCommand_makeCommandList = Window_TitleCommand.prototype.makeCommandList;
Window_TitleCommand.prototype.makeCommandList = function() {
    alias_CGMZ_ExitToDesktop_WindowTitleCommand_makeCommandList.call(this);
	if(!CGMZ.ExitToDesktop.HideInTitle && (Utils.isNwjs() || !CGMZ.ExitToDesktop.HideInBrowser)) {
		this.addCommand(CGMZ.ExitToDesktop.CommandName, "CGMZ_exitToDesktop");
	}
};
//=============================================================================
// Scene_GameEnd
//-----------------------------------------------------------------------------
// Add additional command to the game end command window
//=============================================================================
//-----------------------------------------------------------------------------
// Add command to exit the game to the command window
//-----------------------------------------------------------------------------
const alias_CGMZ_ExitToDesktop_SceneGameEnd_createCommandWindow = Scene_GameEnd.prototype.createCommandWindow;
Scene_GameEnd.prototype.createCommandWindow = function() {
    alias_CGMZ_ExitToDesktop_SceneGameEnd_createCommandWindow.call(this);
	this._commandWindow.setHandler("CGMZ_exitToDesktop", this.CGMZ_commandExitToDesktop.bind(this));
};
//-----------------------------------------------------------------------------
// Set visible lines to 3 in the gameEnd window
//-----------------------------------------------------------------------------
const alias_CGMZ_ExitToDesktop_SceneGameEnd_commandWindowRect = Scene_GameEnd.prototype.commandWindowRect;
Scene_GameEnd.prototype.commandWindowRect = function() {
    const rect = alias_CGMZ_ExitToDesktop_SceneGameEnd_commandWindowRect.call(this);
	if(!CGMZ.ExitToDesktop.HideInGameEnd && (Utils.isNwjs() || !CGMZ.ExitToDesktop.HideInBrowser)) {
		const amount = this.calcWindowHeight(2, true) - this.calcWindowHeight(1, true);
		rect.height += amount;
	}
	return rect;
};
//-----------------------------------------------------------------------------
// Exit the game handling
//-----------------------------------------------------------------------------
Scene_GameEnd.prototype.CGMZ_commandExitToDesktop = function() {
	SceneManager.exit();
};
//=============================================================================
// Window_GameEnd
//-----------------------------------------------------------------------------
// Add additional command to the game end window
//=============================================================================
//-----------------------------------------------------------------------------
// Add command to exit the game to the command window
//-----------------------------------------------------------------------------
const alias_CGMZ_ExitToDesktop_WindowGameEnd_makeCommandList = Window_GameEnd.prototype.makeCommandList;
Window_GameEnd.prototype.makeCommandList = function() {
    alias_CGMZ_ExitToDesktop_WindowGameEnd_makeCommandList.call(this);
	if(!CGMZ.ExitToDesktop.HideInGameEnd && (Utils.isNwjs() || !CGMZ.ExitToDesktop.HideInBrowser)) {
		this.addCommand(CGMZ.ExitToDesktop.CommandName, "CGMZ_exitToDesktop");
	}
};
//-----------------------------------------------------------------------------
// Below code only if CGMZ Game Over imported
//-----------------------------------------------------------------------------
if(Imported.CGMZ_GameOver && CGMZ.GameOver.ShowCommandWindow) {
//=============================================================================
// Scene_Gameover
//-----------------------------------------------------------------------------
// Add additional command to the game over command window
//=============================================================================
//-----------------------------------------------------------------------------
// Add game end option to game over command window
//-----------------------------------------------------------------------------
const alias_CGMZ_ExitToDesktop_SceneGameOver_addCustomHandlers = Scene_Gameover.prototype.CGMZ_GameOver_addCustomHandlers;
Scene_Gameover.prototype.CGMZ_GameOver_addCustomHandlers = function() {
	alias_CGMZ_ExitToDesktop_SceneGameOver_addCustomHandlers.call(this);
	this._commandWindow.setHandler("CGMZ_exitToDesktop", this.CGMZ_commandExitToDesktop.bind(this));
};
//-----------------------------------------------------------------------------
// Number of commands in command window
//-----------------------------------------------------------------------------
const alias_CGMZ_ExitToDesktop_SceneGameOver_numCommands = Scene_Gameover.prototype.CGMZ_GameOver_numCommands;
Scene_Gameover.prototype.CGMZ_GameOver_numCommands = function() {
	const amount = 1 * (!CGMZ.ExitToDesktop.HideInGameOver && (Utils.isNwjs() || !CGMZ.ExitToDesktop.HideInBrowser));
	return amount + alias_CGMZ_ExitToDesktop_SceneGameOver_numCommands.call(this);
};
//-----------------------------------------------------------------------------
// Exit the game handling
//-----------------------------------------------------------------------------
Scene_Gameover.prototype.CGMZ_commandExitToDesktop = function() {
	SceneManager.exit();
};
//=============================================================================
// CGMZ_Window_GameOverCommand
//-----------------------------------------------------------------------------
// Add additional command to the game over command window
//=============================================================================
//-----------------------------------------------------------------------------
// Add game exit command
//-----------------------------------------------------------------------------
const alias_CGMZ_ExitToDesktop_WindowGameOver_makeCommandList = CGMZ_Window_GameOverCommand.prototype.makeCommandList;
CGMZ_Window_GameOverCommand.prototype.makeCommandList = function() {
	alias_CGMZ_ExitToDesktop_WindowGameOver_makeCommandList.call(this);
	if(!CGMZ.ExitToDesktop.HideInGameOver && (Utils.isNwjs() || !CGMZ.ExitToDesktop.HideInBrowser)) {
		this.addCommand(CGMZ.ExitToDesktop.CommandName, 'CGMZ_exitToDesktop');
	}
};
}