/*:
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/changelog/
 * @target MZ
 * @plugindesc Creates a changelog option on the title screen
 * @help
 * ============================================================================
 * For terms and conditions using this plugin in your game please visit:
 * https://www.caspergaming.com/terms-of-use/
 * ============================================================================
 * Become a Patron to get access to beta/alpha plugins plus other goodies!
 * https://www.patreon.com/CasperGamingRPGM
 * ============================================================================
 * Version: 1.3.3
 * ----------------------------------------------------------------------------
 * Compatibility: Only tested with my CGMZ plugins.
 * Made for RPG Maker MZ 1.9.0
 * ----------------------------------------------------------------------------
 * Description: This plugin adds a "changelog" scene where you can provide info
 * for the player to read about any changes you have made since the last update
 * to your game. This can be easily added to the menu or title with CGMZ
 * command window plugins, or you can use plugin command to call the scene.
 * ----------------------------------------------------------------------------
 * Documentation:
 * Bullet points are optional. If you do not want to display a bullet point
 * before each change, leave the option blank.
 *
 * Bullet points can be numbered. If you want changes to be numbered, set the
 * bullet option to "num". Otherwise, the text entered for the bullet option
 * will be displayed before each change in the changelog. For example, "- "
 * will display a - with a space after it before each change.
 * ---------------------------Integrations-------------------------------------
 * You can use CGMZ Title Command Window to easily add the changelog scene to
 * your game title screen. To do so, use the JavaScript command below:
 * this._commandWindow.close();
 * SceneManager.push(CGMZ_Scene_Changelog);
 *
 * You can use CGMZ Menu Command Window to easily add the changelog scene to
 * your game menu. To do so, use the JavaScript command below:
 * SceneManager.push(CGMZ_Scene_Changelog);
 * -------------------------Plugin Commands------------------------------------
 * This plugin supports the following plugin commands:
 * 
 * • Call Scene
 * Calls the changelog scene.
 * ------------------------------Saved Games-----------------------------------
 * This plugin is fully compatible with saved games. This means you can:
 *
 * ✓ Add this plugin to a saved game and it will work as expected
 * ✓ Change any plugin params and changes will be reflected in saved games
 * ✓ Remove the plugin with no issue to save data
 * -----------------------------Filename---------------------------------------
 * The filename for this plugin MUST remain CGMZ_Changelog.js
 * This is what it comes as when downloaded. The filename is used to load
 * parameters and execute plugin commands. If you change it, things will begin
 * behaving incorrectly and your game will probably crash. Please do not
 * rename the js file.
 * --------------------------Latest Version------------------------------------
 * Hi all, this latest version optimizes the changelog code to no longer need
 * to preload the changelog images to calculate their height. This will also
 * allow me to make future updates that allow for more customization of how
 * your changelogs appear. During this update, I removed legacy code from a
 * few years ago where changelogs had to be in the pictures folder. If you
 * still have changelogs from back then, you will need to set up their image
 * parameters again.
 *
 * Version 1.3.3
 * - Removed need to preload changelog images
 *
 * @command Call Scene
 * @desc Calls the Changelog Scene
 *
 * @param Change Setup
 *
 * @param Changes
 * @parent Change Setup
 * @type struct<Change>[]
 * @default []
 * @desc Set up different change log entries here
 *
 * @param Window Options
 *
 * @param Date Alignment
 * @parent Window Options
 * @type select
 * @option left
 * @option center
 * @option right
 * @desc The alignment of the date text.
 * @default center
 *
 * @param Log Alignment
 * @parent Window Options
 * @type select
 * @option left
 * @option center
 * @option right
 * @desc The alignment of the log entry text.
 * @default left
 *
 * @param Changelog Separation
 * @parent Window Options
 * @type number
 * @desc The amount of blank vertical space to leave after a changelog entry
 * @default 12
 *
 * @param Scroll Speed
 * @parent Window Options
 * @type number
 * @min 0
 * @desc speed at which the changelog window display scrolls (if needed)
 * @default 1
 *
 * @param Scroll Wait
 * @parent Window Options
 * @type number
 * @min 0
 * @desc amount of time (in frames) to wait before beginning to scroll
 * @default 300
 *
 * @param Scroll Deceleration
 * @parent Window Options
 * @type number
 * @min 0.01
 * @max 0.99
 * @decimals 2
 * @desc Rate of deceleration after letting go of touch
 * @default 0.92
 *
 * @param Auto Scroll
 * @parent Window Options
 * @type boolean
 * @desc Determine if the window should automatically scroll after so long of no user input
 * @default true
 *
 * @param Bullet
 * @parent Window Options
 * @desc Character to use as a bullet point before each entry. Set to "num" to number the changes and leave blank to not use a bullet point.
 *
 * @param Order
 * @parent Window Options
 * @type boolean
 * @on Descending
 * @off Ascending
 * @desc If descending, shows the changelog entries from last to first. If ascending, shows entries from first to last.
 * @default true
 *
 * @param Changelog Text
 * @parent Window Options
 * @desc Text to display at the top of the changelog window and in title command window.
 * @default Changelog
 *
 * @param Window Width
 * @parent Window Options
 * @type number
 * @min 0
 * @max 100
 * @default 80
 * @desc The width of the changelog window, as a percentage of the total screen width
 *
 * @param Window Height
 * @parent Window Options
 * @type number
 * @min 0
 * @max 100
 * @default 80
 * @desc The height of the changelog window, as a percentage of the total screen height
 *
 * @param Window X Offset
 * @parent Window Options
 * @type number
 * @min -9999
 * @default 0
 * @desc The x offset of the window
 *
 * @param Window Y Offset
 * @parent Window Options
 * @type number
 * @min -9999
 * @default 0
 * @desc The y offset of the window
 *
 * @param Integrations
 *
 * @param Scene Background
 * @parent Integrations
 * @desc The [CGMZ] Scene Backgrounds preset id to use
 *
 * @param Window Background
 * @parent Integrations
 * @desc The [CGMZ] Window Backgrounds preset id to use
 *
 * @param Window Settings
 * @parent Integrations
 * @desc The [CGMZ] Window Settings preset id to use
 *
 * @param Controls Window
 * @parent Integrations
 * @desc The [CGMZ] Controls Window preset id to use
*/
/*~struct~Change:
 * @param Entry Date
 * @desc The date the changes were made.
 *
 * @param Entry Image
 * @type file
 * @dir img/
 * @desc The image to represent the change
 *
 * @param Entries
 * @type text[]
 * @default []
 * @desc The changes made, such as additions, tuning, bug fixes, etc.
*/
/*:es
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/changelog/
 * @target MZ
 * @plugindesc Crea una opción de registro de cambios en la pantalla de título
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
 * Versión: 1.3.3
 * ----------------------------------------------------------------------------
 * Compatibilidad: Sólo probado con mis CGMZ plugins.
 * Hecho para RPG Maker MZ 1.9.0
 * ----------------------------------------------------------------------------
 * Descripción: Este plugin agrega una escena de "registro de cambios" donde 
 * puede proporcionar información para que el jugador lea sobre cualquier 
 * cambio que haya realizado desde la última actualización de su juego. Esto
 * se puede agregar fácilmente al menú o título con los complementos de la
 * ventana de comandos CGMZ, o puede usar el comando del plugin para llamar a
 * la escena.
 * ----------------------------------------------------------------------------
 * Documentación:
 * Las viñetas son opcionales. Si no deseas mostrar una viñeta antes de cada 
 * cambio, deje la opción en blanco.
 *
 * Las viñetas se pueden numerar. Si deseas que los cambios se numeren, 
 * establece la opción de viñeta en "num". De lo contrario, el texto ingresado
 * para la  opción de viñeta se mostrará antes de cada cambio en el registro de
 * cambios. Por ejemplo, "-" mostrará un - con un espacio después de cada
 * cambio.
 * ---------------------------Integrations-------------------------------------
 * Puedes usar la ventana de comandos de título de CGMZ para agregar fácilmente 
 * la escena de registro de cambios a la pantalla de título de su juego. Para 
 * hacerlo, use el siguiente comando de JavaScript:
 * this._commandWindow.close();
 * SceneManager.push(CGMZ_Scene_Changelog);
 *
 * Puede usar la ventana de comandos del menú CGMZ para agregar fácilmente la 
 * escena de registro de cambios a su menú de juego. Para hacerlo, use el 
 * siguiente comando de JavaScript:
 * SceneManager.push(CGMZ_Scene_Changelog);
 * -------------------------Plugin Commands------------------------------------
 * Este complemento admite los siguientes comandos de complemento:
 * 
 * • Escena de llamada
 * Llama a la escena de registro de cambios.
 * ------------------------------Saved Games-----------------------------------
 * This plugin is fully compatible with saved games. This means you can:
 *
 * ✓ Add this plugin to a saved game and it will work as expected
 * ✓ Change any plugin params and changes will be reflected in saved games
 * ✓ Remove the plugin with no issue to save data
 * -----------------------------Filename---------------------------------------
 * The filename for this plugin MUST remain CGMZ_Changelog.js
 * This is what it comes as when downloaded. The filename is used to load
 * parameters and execute plugin commands. If you change it, things will begin
 * behaving incorrectly and your game will probably crash. Please do not
 * rename the js file.
 * --------------------------Latest Version------------------------------------
 * Hi all, this latest version optimizes the changelog code to no longer need
 * to preload the changelog images to calculate their height. This will also
 * allow me to make future updates that allow for more customization of how
 * your changelogs appear. During this update, I removed legacy code from a
 * few years ago where changelogs had to be in the pictures folder. If you
 * still have changelogs from back then, you will need to set up their image
 * parameters again.
 *
 * Version 1.3.3
 * - Removed need to preload changelog images
 *
 * @command Call Scene
 * @text Escena de llamada
 * @desc Llama a la escena Changelog
 *
 * @param Change Setup
 *
 * @param Changes
 * @text Cambios
 * @parent Change Setup
 * @type struct<Change>[]
 * @default []
 * @desc Configure diferentes entradas de registro de cambios aquí.
 *
 * @param Window Options
 * @text Opciones de ventana
 *
 * @param Date Alignment
 * @text Alineación de fechas
 * @parent Window Options
 * @type select
 * @option left
 * @option center
 * @option right
 * @desc La alineación del texto de la fecha.
 * @default center
 *
 * @param Log Alignment
 * @text Alineación de registro
 * @parent Window Options
 * @type select
 * @option left
 * @option center
 * @option right
 * @desc La alineación del texto de la entrada de registro.
 * @default left
 *
 * @param Changelog Separation
 * @parent Window Options
 * @type number
 * @desc The amount of blank vertical space to leave after a changelog entry
 * @default 12
 *
 * @param Scroll Speed
 * @text Velocidad de desplazamiento
 * @parent Window Options
 * @type number
 * @min 0
 * @desc Velocidad a la que se desplaza la visualización de la ventana de registro de cambios (si es necesario).
 * @default 1
 *
 * @param Scroll Wait
 * @text Espera de desplazamiento
 * @parent Window Options
 * @type number
 * @min 0
 * @desc Cantidad de tiempo (en fotogramas) a esperar antes de comenzar a desplazarse
 * @default 300
 *
 * @param Scroll Deceleration
 * @text Desaceleración de desplazamiento
 * @parent Window Options
 * @type number
 * @min 0.01
 * @max 0.99
 * @decimals 2
 * @desc Tasa de desaceleración después de soltar el toque.
 * @default 0.92
 *
 * @param Auto Scroll
 * @text Desplazamiento automático
 * @parent Window Options
 * @type boolean
 * @desc Determinar si la ventana debe desplazarse automáticamente después de tanto tiempo sin intervención del usuario.
 * @default true
 *
 * @param Bullet
 * @text Bala
 * @parent Window Options
 * @desc Carácter para usar como viñeta antes de cada entrada. Establézcalo en "num" para numerar los cambios y déjelo en blanco para no usar una viñeta.
 *
 * @param Order
 * @text Orden
 * @parent Window Options
 * @type boolean
 * @on Descending
 * @off Ascending
 * @desc Si es descendente, muestra las entradas del registro de cambios de la última a la primera. Si es ascendente, muestra las entradas de la primera a la última.
 * @default true
 *
 * @param Changelog Text
 * @text Texto del registro de cambios
 * @parent Window Options
 * @desc Texto para mostrar en la parte superior de la ventana de registro de cambios y en la ventana de comandos del título.
 * @default Changelog
 *
 * @param Window Width
 * @parent Window Options
 * @type number
 * @min 0
 * @max 100
 * @default 80
 * @desc The width of the changelog window, as a percentage of the total screen width
 *
 * @param Window Height
 * @parent Window Options
 * @type number
 * @min 0
 * @max 100
 * @default 80
 * @desc The height of the changelog window, as a percentage of the total screen height
 *
 * @param Window X Offset
 * @parent Window Options
 * @type number
 * @min -9999
 * @default 0
 * @desc The x offset of the window
 *
 * @param Window Y Offset
 * @parent Window Options
 * @type number
 * @min -9999
 * @default 0
 * @desc The y offset of the window
 *
 * @param Integrations
 *
 * @param Scene Background
 * @parent Integrations
 * @desc The [CGMZ] Scene Backgrounds preset id to use
 *
 * @param Window Background
 * @parent Integrations
 * @desc The [CGMZ] Window Backgrounds preset id to use
 *
 * @param Window Settings
 * @parent Integrations
 * @desc The [CGMZ] Window Settings preset id to use
 *
 * @param Controls Window
 * @parent Integrations
 * @desc The [CGMZ] Controls Window preset id to use
*/
/*~struct~Change:es
 * @param Entry Date
 * @text Fecha de entrada
 * @desc La fecha en que se realizaron los cambios.
 *
 * @param Entry Image
 * @text Imagen de entrada
 * @type file
 * @dir img/
 * @desc La imagen para representar el cambio.
 *
 * @param Entries
 * @text Entradas
 * @type text[]
 * @default []
 * @desc Los cambios realizados, como adiciones, ajustes, corrección de errores, etc.
*/
Imported.CGMZ_Changelog = true;
CGMZ.Versions["Changelog"] = "1.3.3";
CGMZ.Changelog = {};
CGMZ.Changelog.parameters = PluginManager.parameters('CGMZ_Changelog');
CGMZ.Changelog.Bullet = CGMZ.Changelog.parameters["Bullet"];
CGMZ.Changelog.WindowTitle = CGMZ.Changelog.parameters["Changelog Text"];
CGMZ.Changelog.LogAlignment = CGMZ.Changelog.parameters["Log Alignment"];
CGMZ.Changelog.DateAlignment = CGMZ.Changelog.parameters["Date Alignment"];
CGMZ.Changelog.SceneBackground = CGMZ.Changelog.parameters["Scene Background"];
CGMZ.Changelog.WindowBackground = CGMZ.Changelog.parameters["Window Background"];
CGMZ.Changelog.ControlsWindow = CGMZ.Changelog.parameters["Controls Window"];
CGMZ.Changelog.WindowSettings = CGMZ.Changelog.parameters["Window Settings"];
CGMZ.Changelog.ScrollSpeed = Number(CGMZ.Changelog.parameters["Scroll Speed"]);
CGMZ.Changelog.ScrollWait = Number(CGMZ.Changelog.parameters["Scroll Wait"]);
CGMZ.Changelog.WindowWidth = Number(CGMZ.Changelog.parameters["Window Width"]);
CGMZ.Changelog.WindowHeight = Number(CGMZ.Changelog.parameters["Window Height"]);
CGMZ.Changelog.WindowXOffset = Number(CGMZ.Changelog.parameters["Window X Offset"]);
CGMZ.Changelog.WindowYOffset = Number(CGMZ.Changelog.parameters["Window Y Offset"]);
CGMZ.Changelog.ChangelogSeparation = Number(CGMZ.Changelog.parameters["Changelog Separation"]);
CGMZ.Changelog.ScrollDeceleration = parseFloat(CGMZ.Changelog.parameters["Scroll Deceleration"]);
CGMZ.Changelog.Order = (CGMZ.Changelog.parameters["Order"] === "true");
CGMZ.Changelog.AutoScroll = (CGMZ.Changelog.parameters["Auto Scroll"] === "true");
CGMZ.Changelog.Changes = CGMZ_Utils.parseJSON(CGMZ.Changelog.parameters["Changes"], [], "[CGMZ] Changelog", "Your Changes parameter had invalid JSON and could not be read.");
if(CGMZ.Changelog.Order) CGMZ.Changelog.Changes.reverse();
//=============================================================================
// CGMZ_ChangelogEntry
//-----------------------------------------------------------------------------
// Store and manage changelog data
//=============================================================================
function CGMZ_ChangelogEntry() {
    this.initialize.apply(this, arguments);
}
//-----------------------------------------------------------------------------
// Initialize Entry
//-----------------------------------------------------------------------------
CGMZ_ChangelogEntry.prototype.initialize = function(entry) {
	this.date = entry["Entry Date"];
	this.img = entry["Entry Image"];
	this.changes = CGMZ_Utils.parseJSON(entry["Entries"], [], "[CGMZ] Changelog", `Your changelog entries for date '${this._date}' had invalid JSON and could not be read.`);
};
//=============================================================================
// CGMZ_Temp
//-----------------------------------------------------------------------------
// Manage unsaved Changelog Data
//=============================================================================
//-----------------------------------------------------------------------------
// Also initialize changelog data
//-----------------------------------------------------------------------------
const alias_CGMZ_Changelog_createPluginData = CGMZ_Temp.prototype.createPluginData;
CGMZ_Temp.prototype.createPluginData = function() {
	alias_CGMZ_Changelog_createPluginData.call(this);
	this.initializeChangelogData();
};
//-----------------------------------------------------------------------------
// Initialize changelog data
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.initializeChangelogData = function() {
	this._changelogEntries = [];
	for(const change of CGMZ.Changelog.Changes) {
		const parsed = CGMZ_Utils.parseJSON(change, null, "[CGMZ] Changelog", "One of your changelog entries had invalid JSON and could not be read.");
		if(!parsed) continue;
		const entry = new CGMZ_ChangelogEntry(parsed);
		this._changelogEntries.push(entry);
	}
};
//-----------------------------------------------------------------------------
// Get changelogs
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.getChangelogs = function() {
	return this._changelogEntries;
};
//-----------------------------------------------------------------------------
// Get a specific changelog
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.getChangelog = function(index) {
	return this._changelogEntries[index];
};
//-----------------------------------------------------------------------------
// Register Plugin Commands
//-----------------------------------------------------------------------------
const alias_CGMZ_Changelog_registerPluginCommands = CGMZ_Temp.prototype.registerPluginCommands;
CGMZ_Temp.prototype.registerPluginCommands = function() {
	alias_CGMZ_Changelog_registerPluginCommands.call(this);
	PluginManager.registerCommand("CGMZ_Changelog", "Call Scene", this.pluginCommandChangelogCallScene);
};
//-----------------------------------------------------------------------------
// Plugin Command - Call Scene
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandChangelogCallScene = function() {
	SceneManager.push(CGMZ_Scene_Changelog);
};
//=============================================================================
// CGMZ_Scene_Changelog
//-----------------------------------------------------------------------------
// Handles the changelog scene
//=============================================================================
function CGMZ_Scene_Changelog() {
    this.initialize(...arguments);
}
CGMZ_Scene_Changelog.prototype = Object.create(Scene_MenuBase.prototype);
CGMZ_Scene_Changelog.prototype.constructor = CGMZ_Scene_Changelog;
//-----------------------------------------------------------------------------
// Create changelog window
//-----------------------------------------------------------------------------
CGMZ_Scene_Changelog.prototype.create = function() {
    Scene_MenuBase.prototype.create.call(this);
	this.createChangelogWindow();
};
//-----------------------------------------------------------------------------
// Create display window
//-----------------------------------------------------------------------------
CGMZ_Scene_Changelog.prototype.createChangelogWindow = function() {
	const rect = this.changelogWindowRect();
    this._changelogWindow = new CGMZ_Window_ChangelogDisplay(rect);
	this._changelogWindow.setHandler('cancel', this.popScene.bind(this));
    this.addWindow(this._changelogWindow);
};
//-----------------------------------------------------------------------------
// Get the rect for the changelog window
//-----------------------------------------------------------------------------
CGMZ_Scene_Changelog.prototype.changelogWindowRect = function() {
	const height = Graphics.boxHeight * (CGMZ.Changelog.WindowHeight / 100.0);
	const width = Graphics.boxWidth * (CGMZ.Changelog.WindowWidth / 100.0);
	const x = ((Graphics.boxWidth - width) / 2) + CGMZ.Changelog.WindowXOffset;
	const y = ((Graphics.boxHeight - height) / 2) + CGMZ.Changelog.WindowYOffset;
	return new Rectangle(x, y, width, height);
};
//-----------------------------------------------------------------------------
// Get the scene's custom scene background
// No need to check if Scene Backgrounds is installed because this custom func
// is only called by that plugin
//-----------------------------------------------------------------------------
CGMZ_Scene_Changelog.prototype.CGMZ_getCustomSceneBackground = function() {
	return $cgmzTemp.sceneBackgroundPresets[CGMZ.Changelog.SceneBackground];
};
//-----------------------------------------------------------------------------
// Get controls window preset for [CGMZ] Controls Window
// No need to check if Controls Window is installed because this custom func
// is only called by that plugin
//-----------------------------------------------------------------------------
CGMZ_Scene_Changelog.prototype.CGMZ_getControlsWindowOtherPreset = function() {
	return $cgmzTemp.getControlWindowPresetOther(CGMZ.Changelog.ControlsWindow);
};
//=============================================================================
// CGMZ_Window_ChangelogDisplay
//-----------------------------------------------------------------------------
// Window displaying changelog information
//=============================================================================
function CGMZ_Window_ChangelogDisplay() {
    this.initialize.apply(this, arguments);
}
CGMZ_Window_ChangelogDisplay.prototype = Object.create(CGMZ_Window_Scrollable.prototype);
CGMZ_Window_ChangelogDisplay.prototype.constructor = CGMZ_Window_ChangelogDisplay;
//-----------------------------------------------------------------------------
// Initialize
//-----------------------------------------------------------------------------
CGMZ_Window_ChangelogDisplay.prototype.initialize = function(rect) {
	const heightMultiplier = 100; // maximum of 100 windows tall of data to scroll
    CGMZ_Window_Scrollable.prototype.initialize.call(this, rect, heightMultiplier, CGMZ.Changelog.ScrollWait,
													 CGMZ.Changelog.ScrollSpeed, CGMZ.Changelog.AutoScroll,
													 CGMZ.Changelog.ScrollDeceleration);
	if(Imported.CGMZ_WindowSettings && CGMZ.Changelog.WindowSettings) this.CGMZ_setWindowSettings(CGMZ.Changelog.WindowSettings);
	if(Imported.CGMZ_WindowBackgrounds && CGMZ.Changelog.WindowBackground) this.CGMZ_setWindowBackground(CGMZ.Changelog.WindowBackground);
	this.refresh();
};
//-----------------------------------------------------------------------------
// Refresh
//-----------------------------------------------------------------------------
CGMZ_Window_ChangelogDisplay.prototype.refresh = function() {
	this.setupWindowForNewEntry();
	this._changelogIndex = 0;
	if(CGMZ.Changelog.WindowTitle) this.drawTitle();
	this.drawCurrentChange();
};
//-----------------------------------------------------------------------------
// Draw Title of window
//-----------------------------------------------------------------------------
CGMZ_Window_ChangelogDisplay.prototype.drawTitle = function() {
	this.contents.fontBold = true;
	this._neededHeight += this.CGMZ_drawTextLine(CGMZ.Changelog.WindowTitle, 0, 0, this.contents.width, 'center');
	this.contents.fontBold = false;
};
//-----------------------------------------------------------------------------
// Draw all changes, called repeatedly as it draws 1 change at a time
//-----------------------------------------------------------------------------
CGMZ_Window_ChangelogDisplay.prototype.drawCurrentChange = function() {
	const changelog = $cgmzTemp.getChangelog(this._changelogIndex);
	if(!changelog) {
		this._neededHeight -= CGMZ.Changelog.ChangelogSeparation;
		this._neededHeight += this.padding * 2;
		this.checkForScroll();
		this.activate();
	} else {
		if(changelog.img) {
			this.loadChangelogImage(changelog);
		} else {
			this.drawChangelogEntry(changelog);
		}
	}
};
//-----------------------------------------------------------------------------
// Draw the text parts of a changelog entry
//-----------------------------------------------------------------------------
CGMZ_Window_ChangelogDisplay.prototype.drawChangelogEntry = function(changelog) {
	const num = (CGMZ.Changelog.Bullet === "num");
	this.contents.fontBold = true;
	this._neededHeight += this.CGMZ_drawTextLine(changelog.date, 0, this._neededHeight, this.contents.width, CGMZ.Changelog.DateAlignment);
	this.contents.fontBold = false;
	const changes = changelog.changes;
	for(let i = 0; i < changes.length; i++) {
		const changeCount = i+1;
		const bulletText = (num) ? changeCount + ". " : CGMZ.Changelog.Bullet;
		this._neededHeight += this.CGMZ_drawText(bulletText + changes[i], 0, 0, this._neededHeight, this.contents.width, CGMZ.Changelog.LogAlignment);
	}
	this._neededHeight += CGMZ.Changelog.ChangelogSeparation;
	this._changelogIndex++;
	this.drawCurrentChange();
};
//-----------------------------------------------------------------------------
// Create changelog sprite and start loading the image
//-----------------------------------------------------------------------------
CGMZ_Window_ChangelogDisplay.prototype.loadChangelogImage = function(changelog) {
	const sprite = new Sprite();
	this.addInnerChild(sprite);
	const imgData = CGMZ_Utils.getImageData(changelog.img, 'img');
	sprite.bitmap = ImageManager.loadBitmap(imgData.folder, imgData.filename);
	sprite.x = this.contents.width / 2;
	sprite.y = this._neededHeight;
	sprite.anchor.x = 0.5;
	sprite.bitmap.addLoadListener(this.onChangelogImageLoaded.bind(this, sprite.bitmap, changelog));
};
//-----------------------------------------------------------------------------
// After changelog image loaded
//-----------------------------------------------------------------------------
CGMZ_Window_ChangelogDisplay.prototype.onChangelogImageLoaded = function(bitmap, changelog) {
	this._neededHeight += bitmap.height;
	this.drawChangelogEntry(changelog);
};