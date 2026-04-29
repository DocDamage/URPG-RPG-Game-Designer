/*:
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/gameendwindow/
 * @target MZ
 * @base CGMZ_Core
 * @orderAfter CGMZ_Core
 * @orderAfter CGMZ_ExitToDesktop
 * @plugindesc Manage the game end command window
 * @help
 * ============================================================================
 * For terms and conditions using this plugin in your game please visit:
 * https://www.caspergaming.com/terms-of-use/
 * ============================================================================
 * Become a Patron to get access to beta/alpha plugins plus other goodies!
 * https://www.patreon.com/CasperGamingRPGM
 * ============================================================================
 * Version: 1.5.0
 * ----------------------------------------------------------------------------
 * Compatibility: Only tested with my CGMZ plugins.
 * Made for RPG Maker MZ 1.9.0
 * ----------------------------------------------------------------------------
 * Description: Use this plugin to easily manage the command window in the
 * game end scene. It allows you to re-arrange commands or use JavaScript to 
 * add custom commands which are capable of calling custom plugin scenes or
 * functions.
 * ----------------------------------------------------------------------------
 * Documentation:
 * This plugin will overwrite the default game end window if keep originals is
 * off. It is best to place this below any other plugins that add commands to
 * the game end window if this option is used.
 *
 * The command symbol should be unique and not blank for every command. This
 * symbol is how the plugin knows internally which JS code to run.
 *
 * Some Command Symbols can have special meanings, mainly when they represent
 * the original commands.
 * The following symbols represent the original commands (case sensitive):
 * toTitle - Will handle like the original to title command
 * cancel - Will handle like the original cancel command
 * 
 * It is important that you do not use these strings as the Command Symbol
 * property unless you mean to refer to the original commands.
 *
 * To use pictures instead of command window, set the Visible Commands to 0.
 *
 * To Title command:
 * {"Command Name":"To Title","Command Symbol":"toTitle","JS Command":"\"\""}
 *
 * Cancel command:
 * {"Command Name":"Cancel","Command Symbol":"cancel","JS Command":"\"\""}
 *
 * Exit to Desktop (if using CGMZ Exit To Desktop):
 * {"Command Name":"Exit To Desktop","Command Symbol":"CGMZ_exitToDesktop","JS Command":"\"if(Utils.isNwjs() || !CGMZ.ExitToDesktop.HideInBrowser) {\\nSceneManager.exit();\\n}\""}
 * -----------------------------Latest Version---------------------------------
 * Hi all, this latest version adds the option to use pictures instead of the
 * command window in the game end scene. How it works is that you still set up
 * your commands for the command window, but you then set the visible commands
 * to 0 so the window does not show and you set up the new picture parameters
 * instead. This allows you to link each picture to a command.
 *
 * Pictures can be positioned individually with x/y offsets, and you can also
 * choose a starting x/y and an x/y offset that gets added to each image. This
 * should make it easy to make vertical, horizontal, or diagonal arrangements
 * without needing to calculate the x/y coordinates yourself, with x/y offsets
 * for any fine tuning required.
 *
 * Version 1.5.0
 * - Added pictures to represent commands
 *
 * @param Window Size
 *
 * @param Visible Commands
 * @parent Window Size
 * @type number
 * @min 0
 * @default 3
 * @desc This is the number of commands that will be visible in the window without scrolling
 *
 * @param Window Width
 * @parent Window Size
 * @type number
 * @min -1
 * @default -1
 * @desc Width (as percent of game screen). Set to -1 for default.
 *
 * @param Window X
 * @parent Window Size
 * @type number
 * @min -1
 * @default -1
 * @desc X value of the upper left corner of the window. Set to -1 for default.
 *
 * @param Window Y
 * @parent Window Size
 * @type number
 * @min -1
 * @default -1
 * @desc Y value of the upper left corner of the window. Set to -1 for default.
 *
 * @param Alignment
 * @type select
 * @option left
 * @option center
 * @option right
 * @default center
 * @desc The alignment of the command text in the window
 *
 * @param Icon Alignment
 * @type select
 * @option left
 * @option right
 * @default left
 * @desc The alignment of the icon parameter in the window
 *
 * @param Keep Original Commands
 * @type boolean
 * @default true
 * @desc Determine whether to show the original commands in their original order.
 *
 * @param Commands
 * @type struct<Handler>[]
 * @desc Command Name and associated js commands
 * @default []
 *
 * @param Picture Setup
 *
 * @param Pictures
 * @parent Picture Setup
 * @type struct<Picture>[]
 * @desc Selectable images that can be used in place of the command window
 * @default []
 *
 * @param Picture Base X
 * @parent Picture Setup
 * @type number
 * @default 0
 * @min -9999
 * @desc The base X of all pictures
 *
 * @param Picture Base Y
 * @parent Picture Setup
 * @type number
 * @default 0
 * @min -9999
 * @desc The base Y of all pictures
 *
 * @param Picture Add X
 * @parent Picture Setup
 * @type number
 * @default 0
 * @min -9999
 * @desc Amount added to picture X after each image
 *
 * @param Picture Add Y
 * @parent Picture Setup
 * @type number
 * @default 0
 * @min -9999
 * @desc Amount added to picture Y after each image
 *
 * @param Integrations
 *
 * @param Scene Background
 * @parent Integrations
 * @desc [CGMZ] Scene Backgrounds preset id to use for the scene
 *
 * @param Window Background
 * @parent Integrations
 * @desc [CGMZ] Window Backgrounds preset id to use for the window
 *
 * @param Window Settings
 * @parent Integrations
 * @desc [CGMZ] Window Settings preset id to use for the window
 *
 * @param Controls Window
 * @parent Integrations
 * @desc [CGMZ] Controls Window preset id to use for the scene
*/
/*~struct~Handler:
 * @param Command Name
 * @desc Name of the command to display in the command window.
 *
 * @param Icon
 * @type icon
 * @default 0
 * @desc An icon to show for the command, if 0 will not show any icon
 *
 * @param Command Symbol
 * @desc This symbol is used internally to recognize the command.
 * Special meaning for original commands (see documentation).
 *
 * @param JS Command
 * @type multiline_string
 * @desc JavaScript to run when command is selected.
 *
 * @param Show JS
 * @type multiline_string
 * @default return true;
 * @desc JavaScript to run to determine if the command is shown
 *
 * @param Enable JS
 * @type multiline_string
 * @default return true;
 * @desc JavaScript to run to determine if the command is enabled
 *
 * @param Background Image
 * @type file
 * @dir img
 * @desc A background image to use for the command. Blank = default black rectangle
 *
 * @param Background Image X
 * @type number
 * @default 0
 * @min 0
 * @desc The x coordinate to start the background image from the source image (upper left corner)
 *
 * @param Background Image Y
 * @type number
 * @default 0
 * @min 0
 * @desc The y coordinate to start the background image from the source image (upper left corner)
 *
 * @param Selected Info
 *
 * @param Selected Text
 * @parent Selected Info
 * @desc Text to display when selected. Blank = same as default command name
 *
 * @param Selected Icon
 * @parent Selected Info
 * @type icon
 * @default 0
 * @desc An icon to show aligned separately from the command text. 0 = same as default icon
 *
 * @param Selected Back Img
 * @parent Selected Info
 * @type file
 * @dir img/
 * @desc A background image to use for the command. Blank = default back image
 *
 * @param Selected Back Img X
 * @parent Selected Info
 * @type number
 * @default 0
 * @min 0
 * @desc The x coordinate to start the background image from the source image (upper left corner) while selected
 *
 * @param Selected Back Img Y
 * @parent Selected Info
 * @type number
 * @default 0
 * @min 0
 * @desc The y coordinate to start the background image from the source image (upper left corner) while selected
*/
/*~struct~Picture:
 * @param Command Symbol
 * @desc The command symbol this picture will activate
 *
 * @param Inactive Image
 * @type file
 * @dir img
 * @desc Image file to use when not active
 *
 * @param Active Image
 * @type file
 * @dir img
 * @desc Image file to use when active
 *
 * @param X Offset
 * @type number
 * @default 0
 * @min -9999
 * @desc Amount of pixels to offset the x for this picture
 *
 * @param Y Offset
 * @type number
 * @default 0
 * @min -9999
 * @desc Amount of pixels to offset the y for this picture
*/
/*:es
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/gameendwindow/
 * @target MZ
 * @base CGMZ_Core
 * @orderAfter CGMZ_Core
 * @orderAfter CGMZ_ExitToDesktop
 * @plugindesc Administra la ventana de comandos de finalización del juego
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
 * Versión: 1.5.0
 * ----------------------------------------------------------------------------
 * Compatibilidad: Sólo probado con mis CGMZ plugins.
 * Hecho para RPG Maker MZ 1.9.0
 * ----------------------------------------------------------------------------
 * Descripción: Usa este plugin para administrar fácilmente la ventana de
 * comandos en la escena final del juego. Le permite reorganizar los comandos
 * o usar JavaScript para agregar comandos personalizados que son capaces de
 * llamar a escenas o funciones de complementos personalizados.
 * ----------------------------------------------------------------------------
 * Documentación:
 * Este plugin sobrescribirá la ventana de finalización del juego
 * predeterminada si no se conservan los originales. Es mejor colocar esto
 * debajo de cualquier otro plugin que agregue comandos a la ventana de
 * finalización del juego si se usa esta opción.
 *
 * El símbolo de comando debe ser único y no estar en blanco para cada comando. 
 * Este símbolo es cómo el plugin sabe internamente qué código JS ejecutar.
 *
 * Algunos Símbolos de Comando pueden tener significados especiales,
 * principalmente cuando representan los comandos originales.
 * Los siguientes símbolos representan los comandos originales (distingue entre 
 * mayúsculas y minúsculas):
 * toTitle: se manejará como el comando original to title
 * cancel: se manejará como el comando de cancelación original
 *
 * Es importante que no use estas cadenas como la propiedad Símbolo de comando
 *  a menos que quiera hacer referencia a los comandos originales.
 *
 * To use pictures instead of command window, set the Visible Commands to 0.
 *
 * Al comando Título:
 * {"Command Name":"To Title","Command Symbol":"toTitle","JS Command":"\"\""}
 *
 * Cancelar comando:
 * {"Command Name":"Cancel","Command Symbol":"cancel","JS Command":"\"\""}
 *
 * Salir al escritorio (si usa CGMZ Salir al escritorio):
 * {"Command Name":"Exit To Desktop","Command Symbol":"CGMZ_exitToDesktop","JS Command":"\"if(Utils.isNwjs() || !CGMZ.ExitToDesktop.HideInBrowser) {\\nSceneManager.exit();\\n}\""}
 * -----------------------------Latest Version---------------------------------
 * Hi all, this latest version adds the option to use pictures instead of the
 * command window in the game end scene. How it works is that you still set up
 * your commands for the command window, but you then set the visible commands
 * to 0 so the window does not show and you set up the new picture parameters
 * instead. This allows you to link each picture to a command.
 *
 * Pictures can be positioned individually with x/y offsets, and you can also
 * choose a starting x/y and an x/y offset that gets added to each image. This
 * should make it easy to make vertical, horizontal, or diagonal arrangements
 * without needing to calculate the x/y coordinates yourself, with x/y offsets
 * for any fine tuning required.
 *
 * Version 1.5.0
 * - Added pictures to represent commands
 *
 * @param Window Size
 *
 * @param Visible Commands
 * @parent Window Size
 * @text Comandos Visibles
 * @type number
 * @min 0
 * @default 3
 * @desc Este es el número de comandos que serán visibles en la ventana sin necesidad de desplazarse.
 *
 * @param Window Width
 * @parent Window Size
 * @type number
 * @min -1
 * @default -1
 * @desc Width (as percent of game screen). Set to -1 for default.
 *
 * @param Window X
 * @parent Window Size
 * @type number
 * @min -1
 * @default -1
 * @desc X value of the upper left corner of the window. Set to -1 for default.
 *
 * @param Window Y
 * @parent Window Size
 * @type number
 * @min -1
 * @default -1
 * @desc Y value of the upper left corner of the window. Set to -1 for default.
 *
 * @param Alignment
 * @text Alineación
 * @type select
 * @option left
 * @option center
 * @option right
 * @default center
 * @desc La alineación del texto del comando en la ventana.
 *
 * @param Icon Alignment
 * @type select
 * @option left
 * @option right
 * @default left
 * @desc The alignment of the icon parameter in the window
 *
 * @param Keep Original Commands
 * @text Mantener los comandos originales
 * @type boolean
 * @default true
 * @desc Determine si desea mostrar los comandos originales en su orden original.
 *
 * @param Commands
 * @text Comandos
 * @type struct<Handler>[]
 * @desc Nombre del comando y comandos js asociados.
 * @default []
 *
 * @param Picture Setup
 *
 * @param Pictures
 * @parent Picture Setup
 * @type struct<Picture>[]
 * @desc Selectable images that can be used in place of the command window
 * @default []
 *
 * @param Picture Base X
 * @parent Picture Setup
 * @type number
 * @default 0
 * @min -9999
 * @desc The base X of all pictures
 *
 * @param Picture Base Y
 * @parent Picture Setup
 * @type number
 * @default 0
 * @min -9999
 * @desc The base Y of all pictures
 *
 * @param Picture Add X
 * @parent Picture Setup
 * @type number
 * @default 0
 * @min -9999
 * @desc Amount added to picture X after each image
 *
 * @param Picture Add Y
 * @parent Picture Setup
 * @type number
 * @default 0
 * @min -9999
 * @desc Amount added to picture Y after each image
 *
 * @param Integrations
 *
 * @param Scene Background
 * @parent Integrations
 * @desc [CGMZ] Scene Backgrounds preset id to use for the scene
 *
 * @param Window Background
 * @parent Integrations
 * @desc [CGMZ] Window Backgrounds preset id to use for the window
 *
 * @param Window Settings
 * @parent Integrations
 * @desc [CGMZ] Window Settings preset id to use for the window
 *
 * @param Controls Window
 * @parent Integrations
 * @desc [CGMZ] Controls Window preset id to use for the scene
*/
/*~struct~Handler:es
 * @param Command Name
 * @text Nombre de comando
 * @desc Nombre del comando que se mostrará en la ventana de comandos.
 *
 * @param Icon
 * @type icon
 * @default 0
 * @desc An icon to show for the command, if 0 will not show any icon
 *
 * @param Command Symbol
 * @text Símbolo de comando
 * @desc Este símbolo se usa internamente para reconocer el comando.
 * Significado especial para comandos originales (ver documentación).
 *
 * @param JS Command
 * @text Comando JS
 * @type multiline_string
 * @desc JavaScript para ejecutar cuando se selecciona el comando.
 *
 * @param Show JS
 * @type multiline_string
 * @default return true;
 * @desc JavaScript to run to determine if the command is shown
 *
 * @param Enable JS
 * @type multiline_string
 * @default return true;
 * @desc JavaScript to run to determine if the command is enabled
 *
 * @param Background Image
 * @type file
 * @dir img
 * @desc A background image to use for the command. Blank = default black rectangle
 *
 * @param Background Image X
 * @type number
 * @default 0
 * @min 0
 * @desc The x coordinate to start the background image from the source image (upper left corner)
 *
 * @param Background Image Y
 * @type number
 * @default 0
 * @min 0
 * @desc The y coordinate to start the background image from the source image (upper left corner)
 *
 * @param Selected Info
 *
 * @param Selected Text
 * @parent Selected Info
 * @desc Text to display when selected. Blank = same as default command name
 *
 * @param Selected Icon
 * @parent Selected Info
 * @type icon
 * @default 0
 * @desc An icon to show aligned separately from the command text. 0 = same as default icon
 *
 * @param Selected Back Img
 * @parent Selected Info
 * @type file
 * @dir img/
 * @desc A background image to use for the command. Blank = default back image
 *
 * @param Selected Back Img X
 * @parent Selected Info
 * @type number
 * @default 0
 * @min 0
 * @desc The x coordinate to start the background image from the source image (upper left corner) while selected
 *
 * @param Selected Back Img Y
 * @parent Selected Info
 * @type number
 * @default 0
 * @min 0
 * @desc The y coordinate to start the background image from the source image (upper left corner) while selected
*/
/*~struct~Picture:es
 * @param Command Symbol
 * @desc The command symbol this picture will activate
 *
 * @param Inactive Image
 * @type file
 * @dir img
 * @desc Image file to use when not active
 *
 * @param Active Image
 * @type file
 * @dir img
 * @desc Image file to use when active
 *
 * @param X Offset
 * @type number
 * @default 0
 * @min -9999
 * @desc Amount of pixels to offset the x for this picture
 *
 * @param Y Offset
 * @type number
 * @default 0
 * @min -9999
 * @desc Amount of pixels to offset the y for this picture
*/
Imported.CGMZ_GameEndWindow = true;
CGMZ.Versions["Game End Window"] = "1.5.0";
CGMZ.GameEndWindow = {};
CGMZ.GameEndWindow.parameters = PluginManager.parameters('CGMZ_GameEndWindow');
CGMZ.GameEndWindow.Alignment = CGMZ.GameEndWindow.parameters["Alignment"];
CGMZ.GameEndWindow.IconAlignment = CGMZ.GameEndWindow.parameters["Icon Alignment"];
CGMZ.GameEndWindow.SceneBackground = CGMZ.GameEndWindow.parameters["Scene Background"];
CGMZ.GameEndWindow.WindowBackground = CGMZ.GameEndWindow.parameters["Window Background"];
CGMZ.GameEndWindow.ControlsWindow = CGMZ.GameEndWindow.parameters["Controls Window"];
CGMZ.GameEndWindow.WindowSettings = CGMZ.GameEndWindow.parameters["Window Settings"];
CGMZ.GameEndWindow.VisibleCommands = Number(CGMZ.GameEndWindow.parameters["Visible Commands"]);
CGMZ.GameEndWindow.WindowWidth = Number(CGMZ.GameEndWindow.parameters["Window Width"]);
CGMZ.GameEndWindow.WindowX = Number(CGMZ.GameEndWindow.parameters["Window X"]);
CGMZ.GameEndWindow.WindowY = Number(CGMZ.GameEndWindow.parameters["Window Y"]);
CGMZ.GameEndWindow.PictureBaseX = Number(CGMZ.GameEndWindow.parameters["Picture Base X"]);
CGMZ.GameEndWindow.PictureBaseY = Number(CGMZ.GameEndWindow.parameters["Picture Base Y"]);
CGMZ.GameEndWindow.PictureAddX = Number(CGMZ.GameEndWindow.parameters["Picture Add X"]);
CGMZ.GameEndWindow.PictureAddY = Number(CGMZ.GameEndWindow.parameters["Picture Add Y"]);
CGMZ.GameEndWindow.KeepOriginals = (CGMZ.GameEndWindow.parameters["Keep Original Commands"] === "true");
CGMZ.GameEndWindow.Commands = CGMZ_Utils.parseJSON(CGMZ.GameEndWindow.parameters["Commands"], [], "[CGMZ] Game End Window", "Your Commands parameter had invalid JSON and could not be read.").map(commandJSON => {;
	const cmd = CGMZ_Utils.parseJSON(commandJSON, null, "[CGMZ] Game End Window", "One of your game end commands had invalid JSON and could not be read.")
	if(!cmd) return null;
	return {
		icon: Number(cmd.Icon),
		symbol: cmd["Command Symbol"] || Math.random().toString(36),
		name: cmd["Command Name"],
		js: cmd["JS Command"],
		showJS: cmd["Show JS"],
		enableJS: cmd["Enable JS"],
		backImg: {
			img: cmd["Background Image"],
			x: Number(cmd["Background Image X"]),
			y: Number(cmd["Background Image Y"])
		},
		selectedInfo: {
			icon: Number(cmd["Selected Icon"]) || Number(cmd["Icon"]),
			name: cmd["Selected Text"] || cmd["Command Name"],
			backImg: {
				img: cmd["Selected Back Img"] || cmd["Background Image"],
				x: Number(cmd["Selected Back Img X"]) || Number(cmd["Background Image X"]),
				y: Number(cmd["Selected Back Img Y"]) || Number(cmd["Background Image Y"])
			}
		}
	};
}).filter(x => !!x);
CGMZ.GameEndWindow.Pictures = CGMZ_Utils.parseJSON(CGMZ.GameEndWindow.parameters["Pictures"], [], "[CGMZ] Game End Window", "Your Pictures parameter had invalid JSON and could not be read.").map(picJSON => {;
	const pic = CGMZ_Utils.parseJSON(picJSON, null, "[CGMZ] Game End Window", "One of your game end pictures had invalid JSON and could not be read.")
	if(!pic) return null;
	return {
		symbol: pic["Command Symbol"],
		xOffset: Number(pic["X Offset"]),
		yOffset: Number(pic["Y Offset"]),
		imgActive: CGMZ_Utils.getImageData(pic["Active Image"], "img"),
		imgInactive: CGMZ_Utils.getImageData(pic["Inactive Image"], "img")
	};
}).filter(x => !!x);
//=============================================================================
// Scene Game End
//-----------------------------------------------------------------------------
// Handling for command window entries
//=============================================================================
//-----------------------------------------------------------------------------
// Initialize command select
//-----------------------------------------------------------------------------
const alias_CGMZGameEndWindow_SceneGameEnd_initialize = Scene_GameEnd.prototype.initialize;
Scene_GameEnd.prototype.initialize = function() {
	alias_CGMZGameEndWindow_SceneGameEnd_initialize.call(this);
	this._cgmz_currentCommand = "";
	this._cgmz_commandPictures = [];
};
//-----------------------------------------------------------------------------
// Handling for custom Commands added through the plugin
//-----------------------------------------------------------------------------
Scene_GameEnd.prototype.CGMZ_GameEndWindow_commandCustom = function() {
	for(const cmd of CGMZ.GameEndWindow.Commands) {
		if(this._commandWindow.currentSymbol() === cmd.symbol) {
			try {
				const hookFunc = new Function(cmd.js);
				hookFunc.call(this);
			}
			catch (e) {
				const origin = "[CGMZ] Game End Window";
				const suggestion = "Check your JavaScript command";
				CGMZ_Utils.reportError(e.message, origin, suggestion);
			}
			break;
		}
	}
};
//-----------------------------------------------------------------------------
// Add additional commands.
//-----------------------------------------------------------------------------
const alias_CGMZGameEndWindow_SceneGameEnd_createCommandWindow = Scene_GameEnd.prototype.createCommandWindow;
Scene_GameEnd.prototype.createCommandWindow = function() {
	alias_CGMZGameEndWindow_SceneGameEnd_createCommandWindow.call(this);
	for(const cmd of CGMZ.GameEndWindow.Commands) {
		if(this.CGMZ_GameEndWindow_isCustomCommand(cmd.symbol)) {
			this._commandWindow.setHandler(cmd.symbol, this.CGMZ_GameEndWindow_commandCustom.bind(this));
		}
	}
	if(!CGMZ.GameEndWindow.VisibleCommands) {
		this.CGMZ_createCommandPictures();
	}
};
//-----------------------------------------------------------------------------
// Add command pictures
//-----------------------------------------------------------------------------
Scene_GameEnd.prototype.CGMZ_createCommandPictures = function() {
	let order = 0;
	for(const picture of CGMZ.GameEndWindow.Pictures) {
		const symbol = picture.symbol;
		for(const cmd of this._commandWindow._list) {
			if(cmd.symbol === symbol) {
				const sprite = new CGMZ_Sprite_GameEndPicture(picture, order++, this._commandWindow);
				this._cgmz_commandPictures.push(sprite);
				this.addChild(sprite);
			}
		}
	}
};
//-----------------------------------------------------------------------------
// Determine if command is a custom command in need of custom handler
//-----------------------------------------------------------------------------
Scene_GameEnd.prototype.CGMZ_GameEndWindow_isCustomCommand = function(symbol) {
	return (symbol !== 'cancel' && symbol !== 'toTitle');
};
//-----------------------------------------------------------------------------
// Change the rectangle height based on number of visible commands
//-----------------------------------------------------------------------------
const alias_CGMZGameEndWindow_SceneGameEnd_commandWindowRect = Scene_GameEnd.prototype.commandWindowRect;
Scene_GameEnd.prototype.commandWindowRect = function() {
	const rect = alias_CGMZGameEndWindow_SceneGameEnd_commandWindowRect.call(this);
	rect.height = this.calcWindowHeight(CGMZ.GameEndWindow.VisibleCommands, true);
	if(CGMZ.GameEndWindow.WindowX >= 0) rect.x = CGMZ.GameEndWindow.WindowX;
	if(CGMZ.GameEndWindow.WindowY >= 0) rect.y = CGMZ.GameEndWindow.WindowY;
	if(!CGMZ.GameEndWindow.VisibleCommands) {
		rect.x = Graphics.width + 100;
	}
	return rect;
};
//-----------------------------------------------------------------------------
// Change the main command width if not set to use default
//-----------------------------------------------------------------------------
const alias_CGMZGameEndWindow_SceneGameEnd_mainCommandWidth = Scene_GameEnd.prototype.mainCommandWidth;
Scene_GameEnd.prototype.mainCommandWidth = function() {
	if(CGMZ.GameEndWindow.WindowWidth < 0) return alias_CGMZGameEndWindow_SceneGameEnd_mainCommandWidth.call(this);
	return Graphics.boxWidth * (CGMZ.GameEndWindow.WindowWidth / 100.0);
};
//-----------------------------------------------------------------------------
// Update command pictures if necessary
//-----------------------------------------------------------------------------
const alias_CGMZGameEndWindow_SceneGameEnd_update = Scene_GameEnd.prototype.update;
Scene_GameEnd.prototype.update = function() {
	alias_CGMZGameEndWindow_SceneGameEnd_update.call(this);
	if(!CGMZ.GameEndWindow.VisibleCommands) {
		this.CGMZ_updateCommandPictures();
	}
};
//-----------------------------------------------------------------------------
// Update command pictures
//-----------------------------------------------------------------------------
Scene_GameEnd.prototype.CGMZ_updateCommandPictures = function() {
	const currentCommand = this._commandWindow.currentSymbol();
	if(currentCommand && currentCommand !== this._cgmz_currentCommand) {
		this.CGMZ_unselectPicture(this._cgmz_currentCommand);
		this._cgmz_currentCommand = currentCommand;
		this.CGMZ_selectPicture(this._cgmz_currentCommand);
	}
};
//-----------------------------------------------------------------------------
// Unselect a picture by symbol
//-----------------------------------------------------------------------------
Scene_GameEnd.prototype.CGMZ_unselectPicture = function(symbol) {
	for(const picture of this._cgmz_commandPictures) {
		if(picture._symbol === symbol) {
			picture.setBitmapToInactive();
			break;
		}
	}
};
//-----------------------------------------------------------------------------
// Select a picture by symbol
//-----------------------------------------------------------------------------
Scene_GameEnd.prototype.CGMZ_selectPicture = function(symbol) {
	for(const picture of this._cgmz_commandPictures) {
		if(picture._symbol === symbol) {
			picture.setBitmapToActive();
			break;
		}
	}
};
//-----------------------------------------------------------------------------
// Get the scene's custom scene background
// No need to check if Scene Backgrounds is installed because this custom func
// is only called by that plugin
//-----------------------------------------------------------------------------
Scene_GameEnd.prototype.CGMZ_getCustomSceneBackground = function() {
	return $cgmzTemp.sceneBackgroundPresets[CGMZ.GameEndWindow.SceneBackground];
};
//-----------------------------------------------------------------------------
// Get controls window preset for [CGMZ] Controls Window
// No need to check if Controls Window is installed because this custom func
// is only called by that plugin
//-----------------------------------------------------------------------------
Scene_GameEnd.prototype.CGMZ_getControlsWindowOtherPreset = function() {
	return $cgmzTemp.getControlWindowPresetOther(CGMZ.GameEndWindow.ControlsWindow);
};
//=============================================================================
// Window Game End
//-----------------------------------------------------------------------------
// Change commands in the command window
//=============================================================================
const alias_CGMZGameEndWindow_WindowGameEnd_initialize = Window_GameEnd.prototype.initialize;
Window_GameEnd.prototype.initialize = function(rect) {
	alias_CGMZGameEndWindow_WindowGameEnd_initialize.call(this, rect);
	if(Imported.CGMZ_WindowSettings && CGMZ.GameEndWindow.WindowSettings) this.CGMZ_setWindowSettings(CGMZ.GameEndWindow.WindowSettings);
	if(Imported.CGMZ_WindowBackgrounds && CGMZ.GameEndWindow.WindowBackground) this.CGMZ_setWindowBackground(CGMZ.GameEndWindow.WindowBackground);
};
//-----------------------------------------------------------------------------
// Add original commands in original order if user wishes
//-----------------------------------------------------------------------------
const alias_CGMZGameEndWindow_WindowGameEnd_makeCommandList = Window_GameEnd.prototype.makeCommandList;
Window_GameEnd.prototype.makeCommandList = function() {
	if(CGMZ.GameEndWindow.KeepOriginals) {
		alias_CGMZGameEndWindow_WindowGameEnd_makeCommandList.call(this);
	}
	for(const cmd of CGMZ.GameEndWindow.Commands) {
		const showFunc = new Function(cmd.showJS);
		if(!showFunc.call(this)) continue;
		const enableFunc = new Function(cmd.enableJS);
		const enabled = enableFunc.call(this);
		this.addCommand(cmd.name, cmd.symbol, enabled, {icon: cmd.icon, img: cmd.backImg, selectedInfo: cmd.selectedInfo});
	}
};
//-----------------------------------------------------------------------------
// Change alignment of command text
//-----------------------------------------------------------------------------
Window_GameEnd.prototype.itemTextAlign = function() {
	return CGMZ.GameEndWindow.Alignment;
};
//-----------------------------------------------------------------------------
// Get the command icon
//-----------------------------------------------------------------------------
Window_GameEnd.prototype.CGMZ_icon = function(index) {
	const ext = this._list[index]?.ext;
	if(!ext) return 0;
	const selected = (this._cgmz_lastIndex === index);
	let icon = ext.icon;
	if(selected && ext.selectedInfo?.icon) icon = ext.selectedInfo.icon;
	return icon;
};
//-----------------------------------------------------------------------------
// Get the command name
//-----------------------------------------------------------------------------
const alias_CGMZGameEndWindow_WindowGameEnd_commandName = Window_GameEnd.prototype.commandName;
Window_GameEnd.prototype.commandName = function(index) {
	const selected = (this._cgmz_lastIndex === index);
	const ext = this._list[index]?.ext;
	const originalName = alias_CGMZGameEndWindow_WindowGameEnd_commandName.call(this, index);
	if(!selected || !ext) return originalName;
    return ext.selectedInfo?.name || originalName;
};
//-----------------------------------------------------------------------------
// Allow use of text codes in command
//-----------------------------------------------------------------------------
Window_GameEnd.prototype.drawItem = function(index) {
	const rect = this.itemLineRect(index);
	const align = this.itemTextAlign();
	const icon = this.CGMZ_icon(index);
	this.resetTextColor();
	this.changePaintOpacity(this.isCommandEnabled(index));
	if(icon) {
		const iconX = (CGMZ.GameEndWindow.IconAlignment === 'left') ? rect.x : rect.x + rect.width - ImageManager.iconWidth;
		this.drawIcon(icon, iconX, rect.y + 2);
		rect.x += (ImageManager.iconWidth + 2) * (CGMZ.GameEndWindow.IconAlignment === 'left');
		rect.width -= ImageManager.iconWidth + 2;
	}
	this.CGMZ_drawTextLine(this.commandName(index), rect.x, rect.y, rect.width, align);
};
//-----------------------------------------------------------------------------
// Get selectable cgmz options
//-----------------------------------------------------------------------------
Window_GameEnd.prototype.CGMZ_getSelectableCGMZOptions = function(index) {
	const ext = this._list[index].ext;
	const selected = (this._cgmz_lastIndex === index);
	if(selected && ext?.selectedInfo?.backImg?.img) {
		const selectBg = {
			img: ext.selectedInfo.backImg.img,
			imgX: ext.selectedInfo.backImg.x,
			imgY: ext.selectedInfo.backImg.y
		}
		return {bg: selectBg};
	}
	if(ext && ext.img && ext.img.img) {
		const bg = {
			img: ext.img.img,
			imgX: ext.img.x,
			imgY: ext.img.y
		}
		return {bg: bg};
	}
	return Window_Command.prototype.CGMZ_getSelectableCGMZOptions.call(this, index);
};
//-----------------------------------------------------------------------------
// Redraw old index
//-----------------------------------------------------------------------------
Window_GameEnd.prototype.CGMZ_handleSelectionChangePrevious = function(index) {
	if(this._list?.[index]) this.redrawItem(index);
};
//-----------------------------------------------------------------------------
// Redraw new index
//-----------------------------------------------------------------------------
Window_GameEnd.prototype.CGMZ_handleSelectionChangeNext = function(index) {
	if(this._list?.[index]) this.redrawItem(index);
};
//=============================================================================
// CGMZ_Sprite_GameEndPicture
//-----------------------------------------------------------------------------
// Pictures for commands
//=============================================================================
function CGMZ_Sprite_GameEndPicture() {
    this.initialize(...arguments);
}
CGMZ_Sprite_GameEndPicture.prototype = Object.create(Sprite_Clickable.prototype);
CGMZ_Sprite_GameEndPicture.prototype.constructor = CGMZ_Sprite_GameEndPicture;
//-----------------------------------------------------------------------------
// Initialize
//-----------------------------------------------------------------------------
CGMZ_Sprite_GameEndPicture.prototype.initialize = function(data, order, cmdWindow) {
	Sprite_Clickable.prototype.initialize.call(this);
	this._commandWindow = cmdWindow;
	this._symbol = data.symbol;
	this.anchor.x = 0;
	this.anchor.y = 0;
	const x = this.calculateX(data.xOffset, order);
	const y = this.calculateY(data.yOffset, order);
	this.move(x, y);
	this._inactiveBitmap = ImageManager.loadBitmap(data.imgInactive.folder, data.imgInactive.filename);
	this._activeBitmap = ImageManager.loadBitmap(data.imgActive.folder, data.imgActive.filename);
	this.bitmap = this._inactiveBitmap;
	this.CGMZ_setExact(true);
	this.show();
};
//-----------------------------------------------------------------------------
// Get picture x coordinate
//-----------------------------------------------------------------------------
CGMZ_Sprite_GameEndPicture.prototype.calculateX = function(offset, order) {
	const base = CGMZ.GameEndWindow.PictureBaseX + (CGMZ.GameEndWindow.PictureAddX * order);
	return base + offset;
};
//-----------------------------------------------------------------------------
// Get picture y coordinate
//-----------------------------------------------------------------------------
CGMZ_Sprite_GameEndPicture.prototype.calculateY = function(offset, order) {
	const base = CGMZ.GameEndWindow.PictureBaseY + (CGMZ.GameEndWindow.PictureAddY * order);
	return base + offset;
};
//-----------------------------------------------------------------------------
// Set the bitmap to the active bitmap
//-----------------------------------------------------------------------------
CGMZ_Sprite_GameEndPicture.prototype.setBitmapToActive = function() {
    this.bitmap = this._activeBitmap;
};
//-----------------------------------------------------------------------------
// Set the bitmap to the inactive bitmap
//-----------------------------------------------------------------------------
CGMZ_Sprite_GameEndPicture.prototype.setBitmapToInactive = function() {
    this.bitmap = this._inactiveBitmap;
};
//-----------------------------------------------------------------------------
// If mouse over, select the pic symbol
//-----------------------------------------------------------------------------
CGMZ_Sprite_GameEndPicture.prototype.onMouseEnter = function() {
    this._commandWindow.selectSymbol(this._symbol);
};
//-----------------------------------------------------------------------------
// Call the command associated with the picture when clicked
//-----------------------------------------------------------------------------
CGMZ_Sprite_GameEndPicture.prototype.onClick = function() {
	this._commandWindow.selectSymbol(this._symbol);
	this._commandWindow.callOkHandler();
};