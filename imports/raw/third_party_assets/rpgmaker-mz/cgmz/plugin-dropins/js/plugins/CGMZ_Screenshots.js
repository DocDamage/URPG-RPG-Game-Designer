/*:
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/screenshots/
 * @target MZ
 * @base CGMZ_Core
 * @orderAfter CGMZ_Core
 * @plugindesc Allows you to take screenshots within the game
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
 * Made for RPG Maker MZ 1.10.0
 * ----------------------------------------------------------------------------
 * Description: Lets you take screenshots within the game. Screenshots can
 * capture the game screen and be saved directly to computer or prompted to
 * save at the user's choice location.
 * ----------------------------------------------------------------------------
 * Documentation:
 * The Screenshots folder will automatically be created if it does not exist.
 * For web-hosted games, the game will prompt the user to download the
 * screenshot.
 * ----------------------Background Color Parameter----------------------------
 * By default, some things rendered by the engine are somewhat transparent.
 * Also by default, the HTML of the page has a background color of black. This
 * bg color is not captured by the screenshot as the screenshot captures only
 * what is rendered by the engine. You can add this bg color back in using the
 * background color parameter. If you're not sure, it is suggested to leave it
 * as "black". Set to blank if you want to preserve the transparency.
 * -------------------------Plugin Commands------------------------------------
 * The following plugin commands are supported:
 * 
 * • Take Screenshot
 * Takes a screenshot as if the player had pressed the Print Screen key. Also
 * has an optional parameter to name the screenshot. If no filename is
 * provided, it will be the default timestampe filename.
 * Note: The filename should not contain spaces.
 * ------------------------------Saved Games-----------------------------------
 * This plugin is fully compatible with saved games. This means you can:
 *
 * ✓ Add this plugin to a saved game and it will work as expected
 * ✓ Change any plugin params and changes will be reflected in saved games
 * ✓ Remove the plugin with no issue to save data
 * ----------------------------Required Plugin---------------------------------
 * Please note that all [CGMZ] plugins require [CGMZ] Core to be installed
 * above them in the plugin manager. You can download it from my website:
 * https://www.caspergaming.com/plugins/cgmz/core/
 * -----------------------------Filename---------------------------------------
 * The filename for this plugin MUST remain CGMZ_Screenshots.js
 * This is what it comes as when downloaded. The filename is used to load
 * parameters and execute plugin commands. If you change it, things will begin
 * behaving incorrectly and your game will probably crash. Please do not
 * rename the js file.
 * --------------------------Latest Version------------------------------------
 * Hi all, this latest version adds the option to play a sound effect when a
 * screenshot is taken. This occurs automatically if the sound effect is set 
 * up, but there is a flag in the screenshot function to prevent the sound from
 * playing for integrating plugins.
 *
 * Version 1.3.0
 * - Add option to play a sound effect when screenshot is taken
 *
 * @command Take Screenshot
 * @desc Takes a screenshot of the current game screen
 *
 * @arg Filename
 * @desc The filename to save the screenshot as. Leave blank to use timestamp as filename
 *
 * @param Automatic Screenshot
 * @type boolean
 * @default true
 * @desc Automatically takes a screenshot of the screen when pressing Print Screen.
 *
 * @param Screenshot Folder
 * @default screenshots
 * @desc The folder (from game project folder root) to save screenshots. Will be created automatically
 *
 * @param Background Color
 * @default black
 * @desc Background color of the screenshot. See documentation.
 *
 * @param Screenshot Key
 * @desc An additional key that can be pressed to take a screenshot.
 *
 * @param Screenshot Gamepad	
 * @desc Gamepad button that when pressed will take a screenshot
 * @type select
 * @option N/A
 * @value -1
 * @option A
 * @value 0
 * @option B
 * @value 1
 * @option X
 * @value 2
 * @option Y
 * @value 3
 * @option LB
 * @value 4
 * @option RB
 * @value 5
 * @option LT
 * @value 6
 * @option RT
 * @value 7
 * @option Back / Select
 * @value 8
 * @option Start
 * @value 9
 * @option Left Stick
 * @value 10
 * @option Right Stick
 * @value 11
 * @option Dpad Up
 * @value 12
 * @option Dpad Down
 * @value 13
 * @option Dpad Left
 * @value 14
 * @option Dpad Right
 * @value 15
 * @default 8
 *
 * @param Screenshot Sound Effect
 * @type struct<SE>
 * @default {"Name":"","Volume":"90","Pitch":"100","Pan":"0"}
 * @desc A sound effect that can play when a screenshot is taken
 *
 * @param Integrations
 *
 * @param Copy To Clipboard
 * @parent Integrations
 * @type boolean
 * @default false
 * @desc If true, any screenshot taken will also be copied to the player's clipboard
*/
/*~struct~SE:
 * @param Name
 * @type file
 * @dir audio/se
 * @desc Sound Effect file to play
 *
 * @param Volume
 * @type number
 * @default 90
 * @min 0
 * @max 100
 * @desc Volume of the sound effect
 *
 * @param Pitch
 * @type number
 * @default 100
 * @min 50
 * @max 150
 * @desc Pitch of the sound effect
 *
 * @param Pan
 * @type number
 * @default 0
 * @min -100
 * @max 100
 * @desc Pan of the sound effect
*/
/*:es
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/screenshots/
 * @target MZ
 * @base CGMZ_Core
 * @orderAfter CGMZ_Core
 * @plugindesc Te permite tomar capturas de pantalla dentro del juego
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
 * Hecho para RPG Maker MZ 1.10.0
 * ----------------------------------------------------------------------------
 * Descripción: Te permite tomar capturas de pantalla dentro del juego. Las 
 * capturas de pantalla pueden capturar la pantalla del juego y guardarla 
 * directamente en la computadora o pedirle que la guarde en la ubicación 
 * elegida por el usuario.
 * ----------------------------------------------------------------------------
 * Documentación:
 * La carpeta Capturas de pantalla se creará automáticamente si no existe.
 * Para juegos alojados en la web, el juego solicitará al usuario que descargue
 * la captura de pantalla.
 * ----------------------Parámetro de color de fondo----------------------------
 * Por defecto, algunas cosas renderizadas por el motor son algo transparentes.
 * También por defecto, el HTML de la página tiene un color de fondo negro. Este
 * color de fondo no se captura en la captura de pantalla, ya que la captura de
 * pantalla solo captura lo que es renderizado por el motor. Puede volver a 
 * agregar este color de fondo usando el parámetro de color de fondo. Si no 
 * está seguro, se sugiere dejarlo como "negro". Establézcalo en blanco si desea
 * conservar la transparencia.
 * -------------------------Plugin Commands------------------------------------
 * The following plugin commands are supported:
 * 
 * • Take Screenshot
 * Takes a screenshot as if the player had pressed the Print Screen key. Also
 * has an optional parameter to name the screenshot. If no filename is
 * provided, it will be the default timestampe filename.
 * Note: The filename should not contain spaces.
 * ------------------------------Saved Games-----------------------------------
 * This plugin is fully compatible with saved games. This means you can:
 *
 * ✓ Add this plugin to a saved game and it will work as expected
 * ✓ Change any plugin params and changes will be reflected in saved games
 * ✓ Remove the plugin with no issue to save data
 * ----------------------------Required Plugin---------------------------------
 * Please note that all [CGMZ] plugins require [CGMZ] Core to be installed
 * above them in the plugin manager. You can download it from my website:
 * https://www.caspergaming.com/plugins/cgmz/core/
 * -----------------------------Filename---------------------------------------
 * The filename for this plugin MUST remain CGMZ_Screenshots.js
 * This is what it comes as when downloaded. The filename is used to load
 * parameters and execute plugin commands. If you change it, things will begin
 * behaving incorrectly and your game will probably crash. Please do not
 * rename the js file.
 * --------------------------Latest Version------------------------------------
 * Hi all, this latest version adds the option to play a sound effect when a
 * screenshot is taken. This occurs automatically if the sound effect is set 
 * up, but there is a flag in the screenshot function to prevent the sound from
 * playing for integrating plugins.
 *
 * Version 1.3.0
 * - Add option to play a sound effect when screenshot is taken
 *
 * @command Take Screenshot
 * @text Tomar captura de pantalla
 * @desc Toma una captura de pantalla de la pantalla del juego actual.
 *
 * @arg Filename
 * @desc The filename to save the screenshot as. Leave blank to use timestamp as filename
 *
 * @param Automatic Screenshot
 * @text Captura de pantalla automática
 * @type boolean
 * @default true
 * @desc Toma automáticamente una captura de la pantalla al presionar Imprimir pantalla.
 *
 * @param Screenshot Folder
 * @text Carpeta de captura de pantalla
 * @default screenshots
 * @desc La carpeta (desde la raíz de la carpeta del proyecto del juego) para guardar capturas de pantalla. Se creará automáticamente.
 *
 * @param Background Color
 * @text Color de fondo
 * @default black
 * @desc Color de fondo de la captura de pantalla. Ver documentación.
 *
 * @param Screenshot Key
 * @desc An additional key that can be pressed to take a screenshot.
 *
 * @param Screenshot Gamepad	
 * @desc Gamepad button that when pressed will take a screenshot
 * @type select
 * @option N/A
 * @value -1
 * @option A
 * @value 0
 * @option B
 * @value 1
 * @option X
 * @value 2
 * @option Y
 * @value 3
 * @option LB
 * @value 4
 * @option RB
 * @value 5
 * @option LT
 * @value 6
 * @option RT
 * @value 7
 * @option Back / Select
 * @value 8
 * @option Start
 * @value 9
 * @option Left Stick
 * @value 10
 * @option Right Stick
 * @value 11
 * @option Dpad Up
 * @value 12
 * @option Dpad Down
 * @value 13
 * @option Dpad Left
 * @value 14
 * @option Dpad Right
 * @value 15
 * @default 8
 *
 * @param Screenshot Sound Effect
 * @type struct<SE>
 * @default {"Name":"","Volume":"90","Pitch":"100","Pan":"0"}
 * @desc A sound effect that can play when a screenshot is taken
 *
 * @param Integrations
 *
 * @param Copy To Clipboard
 * @parent Integrations
 * @type boolean
 * @default false
 * @desc If true, any screenshot taken will also be copied to the player's clipboard
*/
/*~struct~SE:es
 * @param Name
 * @type file
 * @dir audio/se
 * @desc Sound Effect file to play
 *
 * @param Volume
 * @type number
 * @default 90
 * @min 0
 * @max 100
 * @desc Volume of the sound effect
 *
 * @param Pitch
 * @type number
 * @default 100
 * @min 50
 * @max 150
 * @desc Pitch of the sound effect
 *
 * @param Pan
 * @type number
 * @default 0
 * @min -100
 * @max 100
 * @desc Pan of the sound effect
*/
Imported.CGMZ_Screenshots = true;
CGMZ.Versions["Screenshots"] = "1.3.0";
CGMZ.Screenshots = {};
CGMZ.Screenshots.parameters = PluginManager.parameters('CGMZ_Screenshots');
CGMZ.Screenshots.ScreenshotFolder = CGMZ.Screenshots.parameters["Screenshot Folder"];
CGMZ.Screenshots.BGColor = CGMZ.Screenshots.parameters["Background Color"];
CGMZ.Screenshots.ScreenshotKey = CGMZ.Screenshots.parameters["Screenshot Key"];
CGMZ.Screenshots.ScreenshotGamepad = Number(CGMZ.Screenshots.parameters["Screenshot Gamepad"]);
CGMZ.Screenshots.AutomaticScreenshot = (CGMZ.Screenshots.parameters["Automatic Screenshot"] === 'true');
CGMZ.Screenshots.CopyToClipboard = (CGMZ.Screenshots.parameters["Copy To Clipboard"] === 'true');
CGMZ.Screenshots.SoundEffect = CGMZ_Utils.parseSoundEffectJSON(CGMZ.Screenshots.parameters["Screenshot Sound Effect"], '[CGMZ] Screenshots');
//=============================================================================
// CGMZ_Temp
//-----------------------------------------------------------------------------
// Handle plugin command for taking a screenshot and capturing of print screen
//=============================================================================
//-----------------------------------------------------------------------------
// Also create screenshot data
//-----------------------------------------------------------------------------
const alias_CGMZScreenshots_CGMZTemp_createPluginData = CGMZ_Temp.prototype.createPluginData;
CGMZ_Temp.prototype.createPluginData = function() {
	alias_CGMZScreenshots_CGMZTemp_createPluginData.call(this);
	this._canScreenshot = true;
	this._canGamepadScreenshot = true;
};
//-----------------------------------------------------------------------------
// Register Plugin Commands
//-----------------------------------------------------------------------------
const alias_CGMZ_Screenshots_registerPluginCommands = CGMZ_Temp.prototype.registerPluginCommands;
CGMZ_Temp.prototype.registerPluginCommands = function() {
	alias_CGMZ_Screenshots_registerPluginCommands.call(this);
	PluginManager.registerCommand("CGMZ_Screenshots", "Take Screenshot", this.pluginCommandScreenshotsTakeScreenshot);
};
//-----------------------------------------------------------------------------
// Plugin command for taking screenshot
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandScreenshotsTakeScreenshot = function(args) {
	$cgmzTemp.takeScreenshot(args.Filename);
};
//-----------------------------------------------------------------------------
// Create the screenshot sprite
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.createScreenshotSprite = function() {
	const width = Graphics.width;
	const height = Graphics.height;
	const bitmap1 = new Bitmap(width, height);
	const bitmap2 = Bitmap.snap(SceneManager._scene);
	if(CGMZ.Screenshots.BGColor) bitmap1.fillAll(CGMZ.Screenshots.BGColor);
	bitmap1.blt(bitmap2, 0, 0, width, height, 0, 0, width, height);
	return new Sprite(bitmap1);
};
//-----------------------------------------------------------------------------
// Take a screenshot
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.takeScreenshot = function(filename = "", ext = {}) {
	const sprite = this.createScreenshotSprite();
	if(Utils.isNwjs()) {
		const data = Graphics.app.renderer.extract.canvas(sprite).toDataURL('image/png');
		this.saveScreenshot(data, filename);
	} else {
		Graphics.app.renderer.extract.canvas(sprite).toBlob(this.promptScreenshotDownload(filename), 'image/png');
	}
	if(Imported.CGMZ_Clipboard && CGMZ.Screenshots.CopyToClipboard) {
		Graphics.app.renderer.extract.canvas(sprite).toBlob(CGMZ_Clipboard.writeImageBlob, 'image/png');
	}
	if(CGMZ.Screenshots.SoundEffect?.name && !ext.skipSe) AudioManager.playSe(CGMZ.Screenshots.SoundEffect);
};
//-----------------------------------------------------------------------------
// Save the screenshot
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.saveScreenshot = function(data, filename) {
	data = data.replace(/^data:image\/png;base64,/, "");
	const date = new Date();
	const timestamp = `${date.getFullYear()}-${date.getMonth()+1}-${date.getDate()}_${date.getHours().padZero(2)}${date.getMinutes().padZero(0)}${date.getSeconds().padZero(2)}${date.getMilliseconds().padZero(3)}`;
	const folder = CGMZ.Screenshots.ScreenshotFolder + "/";
	const file = filename || "Screenshot_" + timestamp;
	const ext = ".png";
	CGMZ_Utils.saveToLocalFile(folder, file, ext, data);
};
//-----------------------------------------------------------------------------
// Prompt user to download screenshot
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.promptScreenshotDownload = function(filename) {
	return function(file){
		const a = document.createElement('a');
		document.body.append(a);
		const date = new Date();
		const timestamp = `${date.getFullYear()}-${date.getMonth()+1}-${date.getDate()}_${date.getHours().padZero(2)}${date.getMinutes().padZero(0)}${date.getSeconds().padZero(2)}${date.getMilliseconds().padZero(3)}`;
		a.download = filename || "Screenshot_" + timestamp;
		a.href = URL.createObjectURL(file);
		a.click();
		a.remove();
	};
};
//-----------------------------------------------------------------------------
// Processing on key down
//-----------------------------------------------------------------------------
const alias_CGMZScreenshots_CGMZ_Temp_refreshForKeysDown = CGMZ_Temp.prototype.refreshForKeysDown;
CGMZ_Temp.prototype.refreshForKeysDown = function() {
	alias_CGMZScreenshots_CGMZ_Temp_refreshForKeysDown.call(this);
	if(CGMZ.Screenshots.AutomaticScreenshot && this._canScreenshot && this.isScreenshotKeyPressed()) {
		this.takeScreenshot();
		this._canScreenshot = false;
	}
};
//-----------------------------------------------------------------------------
// Processing on key up
//-----------------------------------------------------------------------------
const alias_CGMZ_Screenshots_refreshForKeysUp = CGMZ_Temp.prototype.refreshForKeysUp;
CGMZ_Temp.prototype.refreshForKeysUp = function() {
	alias_CGMZ_Screenshots_refreshForKeysUp.call(this);
	if(CGMZ.Screenshots.AutomaticScreenshot && this._canScreenshot && this._inputCurrentState.hasOwnProperty("PrintScreen")) {
		this.takeScreenshot();
		delete this._inputCurrentState["PrintScreen"];
	}
	if(!this._canScreenshot) {
		this._canScreenshot = !this.isScreenshotKeyPressed();
	}
};
//-----------------------------------------------------------------------------
// Check if screenshot key is pressed
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.isScreenshotKeyPressed = function() {
	if(CGMZ.Screenshots.ScreenshotKey && this._inputCurrentState[CGMZ.Screenshots.ScreenshotKey]) return true;
	return false;
};
//-----------------------------------------------------------------------------
// Check if screenshot gamepad button is pressed
//-----------------------------------------------------------------------------
const alias_CGMZScreenshots_CGMZTemp_updateLastGamepad = CGMZ_Temp.prototype.updateLastGamepad;
CGMZ_Temp.prototype.updateLastGamepad = function(gamepad) {
	alias_CGMZScreenshots_CGMZTemp_updateLastGamepad.call(this, gamepad);
	if(CGMZ.Screenshots.AutomaticScreenshot && this._canGamepadScreenshot && this.isScreenshotGamepadPressed(gamepad)) {
		this.takeScreenshot();
		this._canGamepadScreenshot = false;
	}
};
//-----------------------------------------------------------------------------
// Check if screenshot gamepad button is no longer pressed
//-----------------------------------------------------------------------------
const alias_CGMZScreenshots_CGMZTemp_updateGamepadRelease = CGMZ_Temp.prototype.updateGamepadRelease;
CGMZ_Temp.prototype.updateGamepadRelease = function(gamepad) {
	alias_CGMZScreenshots_CGMZTemp_updateGamepadRelease.call(this, gamepad);
	if(!this._canGamepadScreenshot) {
		this._canGamepadScreenshot = !this.isScreenshotGamepadPressed(gamepad);
	}
};
//-----------------------------------------------------------------------------
// Check if screenshot gamepad button is pressed
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.isScreenshotGamepadPressed = function(gamepad) {
	if(CGMZ.Screenshots.ScreenshotGamepad < 0) return false
	return (gamepad && gamepad.buttons[CGMZ.Screenshots.ScreenshotGamepad].pressed);
};