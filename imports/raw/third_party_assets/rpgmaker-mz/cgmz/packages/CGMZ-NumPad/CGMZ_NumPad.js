/*:
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/numpad/
 * @target MZ
 * @base CGMZ_Core
 * @orderAfter CGMZ_Core
 * @plugindesc Number input processing that looks like a phone numpad
 * @help
 * ============================================================================
 * For terms and conditions using this plugin in your game please visit:
 * https://www.caspergaming.com/terms-of-use/
 * ============================================================================
 * Become a Patron to get access to beta/alpha plugins plus other goodies!
 * https://www.patreon.com/CasperGamingRPGM
 * ============================================================================
 * Version: 1.2.0
 * ----------------------------------------------------------------------------
 * Compatibility: Only tested with my CGMZ plugins.
 * Made for RPG Maker MZ 1.9.0
 * ----------------------------------------------------------------------------
 * Description: Adds a new number input processing command which looks similar
 * to a phone numpad. You can specify how many numbers to use and which
 * variable the number will be stored in. It can also be stored as a string if
 * leading zeros are important.
 * ----------------------------------------------------------------------------
 * Documentation:
 * ----------------------------Plugin Commands---------------------------------
 * This plugin supports the following plugin commands:
 * 
 * • Call Scene
 * Calls the numpad scene, see further documentation below.
 * -----------------------------Calling Scene----------------------------------
 * Calling the numpad scene with number=true will lose leading 0s. This means
 * 007 will become 7, because with numbers the leading 0s are insignificant.
 *
 * If leading 0s are important, set number to false when calling the numpad
 * scene. If number is set to false it will be stored as a string.
 * ------------------------------Saved Games-----------------------------------
 * This plugin is fully compatible with saved games. This means you can:
 *
 * ✓ Add this plugin to a saved game and it will work as expected
 * ✓ Change any plugin params and changes will be reflected in saved games
 * ✓ Remove the plugin with no issue to save data
 * -----------------------------Filename---------------------------------------
 * The filename for this plugin MUST remain CGMZ_NumPad.js
 * This is what it comes as when downloaded. The filename is used to load
 * parameters and execute plugin commands. If you change it, things will begin
 * behaving incorrectly and your game will probably crash. Please do not
 * rename the js file.
 * --------------------------Latest Version------------------------------------
 * Hi all, this latest version adds a private property to the numpad scene
 * call, which will cause numbers the player enters to be obscured with an
 * asterisk. This can help mimic things like inputting a private PIN or other
 * number that you want to be private.
 * 
 * Version 1.2.0
 * - Added private number option
 *
 * @command Call Scene
 * @desc Calls the Numpad scene
 *
 * @arg variable
 * @type variable
 * @desc The variable to save the numpad entry to
 * @default 0
 *
 * @arg number
 * @type boolean
 * @desc Convert the input to a number (leading 0s will be lost)? If false, the variable will store a string representation of the input
 * @default true
 *
 * @arg Max Numbers
 * @type number
 * @desc Max amount of numbers to allow the user to input
 * @default 9
 * @min 1
 *
 * @arg Min Numbers
 * @type number
 * @desc Min amount of numbers to allow the user to input
 * @default 0
 * @min 0
 *
 * @arg Placeholder
 * @desc Character to use when a number has not yet been entered
 * @default _
 *
 * @arg Private
 * @type boolean
 * @desc If true, numbers entered will be obscured by the * character
 * @default false
 *
 * @arg Integrations
 *
 * @arg Scene Background
 * @parent Integrations
 * @desc [CGMZ] Scene Backgrounds preset id to use for this scene call
 *
 * @arg Select Window Background
 * @parent Integrations
 * @desc [CGMZ] Scene Backgrounds preset id to use for this scene call - select window
 *
 * @arg Current Window Background
 * @parent Integrations
 * @desc [CGMZ] Scene Backgrounds preset id to use for this scene call - current number window
 *
 * @arg Select Window Settings
 * @parent Integrations
 * @desc [CGMZ] Scene Settings preset id to use for this scene call - select window
 *
 * @arg Current Window Settings
 * @parent Integrations
 * @desc [CGMZ] Scene Settings preset id to use for this scene call - current number window
 *
 * @param Scene Options
 *
 * @param Window Width
 * @parent Scene Options
 * @type number
 * @min 0
 * @max 100
 * @default 35
 * @desc Width (as a percentage of the screen) the windows take up
 *
 * @param Sound Options
 *
 * @param Number SE
 * @parent Sound Options
 * @type struct<SoundEffect>
 * @default {"Name":"","Volume":"90","Pitch":"100","Pan":"0"}
 * @desc The custom sound effect to play when entering numbers.
 *
 * @param Volume Variance
 * @parent Sound Options
 * @type number
 * @default 5
 * @desc The amount to vary the volume by when the SE is played
 *
 * @param Pitch Variance
 * @parent Sound Options
 * @type number
 * @default 5
 * @desc The amount to vary the pitch by when the SE is played
 *
 * @param Text Options
 *
 * @param DEL Text
 * @parent Text Options
 * @desc Text to show in the "DEL" option on num pad
 * @default DEL
 *
 * @param OK Text
 * @parent Text Options
 * @desc Text to show in the "OK" option on num pad
 * @default OK
 *
 * @param Integrations
 *
 * @param Scene Background
 * @parent Integrations
 * @desc Default [CGMZ] Scene Backgrounds preset id to use for the numpad scene
 *
 * @param Select Window Background
 * @parent Integrations
 * @desc Default [CGMZ] Window Backgrounds preset id to use for the numpad scene - number select window
 *
 * @param Current Window Background
 * @parent Integrations
 * @desc Default [CGMZ] Window Backgrounds preset id to use for the numpad scene - current number window
 *
 * @param Select Window Settings
 * @parent Integrations
 * @desc Default [CGMZ] Window Settings preset id to use for the numpad scene - number select window
 *
 * @param Current Window Settings
 * @parent Integrations
 * @desc Default [CGMZ] Window Settings preset id to use for the numpad scene - current number window
*/
/*~struct~SoundEffect:
 * @param Name
 * @type file
 * @dir audio/se
 * @desc Sound Effect file to play. Leave blank to not use.
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
Imported.CGMZ_NumPad = true;
CGMZ.Versions["NumPad"] = "1.2.0";
CGMZ.NumPad = {};
CGMZ.NumPad.parameters = PluginManager.parameters('CGMZ_NumPad');
CGMZ.NumPad.DelText = CGMZ.NumPad.parameters["DEL Text"];
CGMZ.NumPad.OkText = CGMZ.NumPad.parameters["OK Text"];
CGMZ.NumPad.SceneBackground = CGMZ.NumPad.parameters["Scene Background"];
CGMZ.NumPad.SelectWindowBackground = CGMZ.NumPad.parameters["Select Window Background"];
CGMZ.NumPad.CurrentWindowBackground = CGMZ.NumPad.parameters["Current Window Background"];
CGMZ.NumPad.SelectWindowSettings = CGMZ.NumPad.parameters["Select Window Settings"];
CGMZ.NumPad.CurrentWindowSettings = CGMZ.NumPad.parameters["Current Window Settings"];
CGMZ.NumPad.WindowWidth = Number(CGMZ.NumPad.parameters["Window Width"]);
CGMZ.NumPad.VolumeVariance = Number(CGMZ.NumPad.parameters["Volume Variance"]);
CGMZ.NumPad.PitchVariance = Number(CGMZ.NumPad.parameters["Pitch Variance"]);
CGMZ.NumPad.NumberSE = CGMZ_Utils.parseSoundEffectJSON(CGMZ.NumPad.parameters["Number SE"], "CGMZ NumPad");
//=============================================================================
// CGMZ_Temp
//-----------------------------------------------------------------------------
// Add plugin command for numpad processing
//=============================================================================
//-----------------------------------------------------------------------------
// Register Numpad Plugin Commands
//-----------------------------------------------------------------------------
const alias_CGMZ_NumPad_CGMZ_Temp_registerPluginCommands = CGMZ_Temp.prototype.registerPluginCommands;
CGMZ_Temp.prototype.registerPluginCommands = function() {
	alias_CGMZ_NumPad_CGMZ_Temp_registerPluginCommands.call(this);
	PluginManager.registerCommand("CGMZ_NumPad", "Call Scene", this.pluginCommandNumPadCallScene);
};
//-----------------------------------------------------------------------------
// Plugin Command - Call numpad scene
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandNumPadCallScene = function(args) {
	SceneManager.push(CGMZ_Scene_NumPad);
	const opts = {
		isPrivate: (args["Private"] === 'true'),
		sceneBackground: args["Scene Background"],
		selectWindowBackground: args["Select Window Background"],
		currentWindowBackground: args["Current Window Background"],
		selectWindowSettings: args["Select Window Settings"],
		currentWindowSettings: args["Current Window Settings"]
	}
	SceneManager.prepareNextScene(Number(args.variable), args.number === "true", Number(args["Max Numbers"]), Number(args["Min Numbers"]), args.Placeholder, opts);
};
//=============================================================================
// CGMZ_Scene_NumPad
//-----------------------------------------------------------------------------
// Handle the numpad scene
//=============================================================================
function CGMZ_Scene_NumPad() {
	this.initialize.apply(this, arguments);
}
CGMZ_Scene_NumPad.prototype = Object.create(Scene_MenuBase.prototype);
CGMZ_Scene_NumPad.prototype.constructor = CGMZ_Scene_NumPad;
//-----------------------------------------------------------------------------
// Prepare scene option
//-----------------------------------------------------------------------------
CGMZ_Scene_NumPad.prototype.prepare = function(gameVar, isNumber, maxNumbers, minNumbers, placeholder, opts) {
	this._gameVar = gameVar;
	this._isNumber = isNumber;
	this._maxNumbers = maxNumbers;
	this._minNumbers = minNumbers;
	this._placeholder = placeholder;
	this._opts = opts;
};
//-----------------------------------------------------------------------------
// Create numpad windows
//-----------------------------------------------------------------------------
CGMZ_Scene_NumPad.prototype.create = function() {
	Scene_MenuBase.prototype.create.call(this);
	this.createCurrentEntryWindow();
	this.createListWindow();
};
//-----------------------------------------------------------------------------
// Create current numpad entry window
//-----------------------------------------------------------------------------
CGMZ_Scene_NumPad.prototype.createCurrentEntryWindow = function() {
	const rect = this.currentEntryWindowRect();
	this._currentEntryWindow = new CGMZ_Window_NumPad_CurrentEntry(rect, this._opts);
	this._currentEntryWindow.setMax(this._maxNumbers);
	this._currentEntryWindow.setMin(this._minNumbers);
	this._currentEntryWindow.setPlaceholder(this._placeholder);
	this.addWindow(this._currentEntryWindow);
};
//-----------------------------------------------------------------------------
// Get current numpad entry window rect
//-----------------------------------------------------------------------------
CGMZ_Scene_NumPad.prototype.currentEntryWindowRect = function() {
	const width = Graphics.boxWidth * (CGMZ.NumPad.WindowWidth / 100.0);
	const x = (Graphics.boxWidth - width) / 2;
	const height = this.calcWindowHeight(1, false);
	const y = Graphics.boxHeight / 4;
	return new Rectangle(x, y, width, height);
};
//-----------------------------------------------------------------------------
// Create numpad list window
//-----------------------------------------------------------------------------
CGMZ_Scene_NumPad.prototype.createListWindow = function() {
	const rect = this.listWindowRect();
	this._listWindow = new CGMZ_Window_NumPad_NumberSelect(rect, this._opts);
	this._listWindow.setHandler('cancel', this.popScene.bind(this));
	this._listWindow.setHandler('ok', this.onListOk.bind(this));
	this._listWindow.setEntryWindow(this._currentEntryWindow);
	this.addWindow(this._listWindow);
};
//-----------------------------------------------------------------------------
// Get numpad list window rect
//-----------------------------------------------------------------------------
CGMZ_Scene_NumPad.prototype.listWindowRect = function() {
	const x = this._currentEntryWindow.x;
	const width = this._currentEntryWindow.width;
	const height = this.calcWindowHeight(4, true);
	const y = this._currentEntryWindow.y + this._currentEntryWindow.height;
	return new Rectangle(x, y, width, height);
};
//-----------------------------------------------------------------------------
// On List Ok
//-----------------------------------------------------------------------------
CGMZ_Scene_NumPad.prototype.onListOk = function() {
	const entry = this._listWindow.item();
	if(entry === CGMZ.NumPad.DelText) {
		this._currentEntryWindow.removeNumber();
	} else if(entry === CGMZ.NumPad.OkText) {
		this.submitEntry(this._currentEntryWindow._entry);
	} else {
		this._currentEntryWindow.addNumber(entry);
	}
	this._listWindow.refresh();
	this._listWindow.activate();
};
//-----------------------------------------------------------------------------
// On entry submit
//-----------------------------------------------------------------------------
CGMZ_Scene_NumPad.prototype.submitEntry = function(entry) {
	const value = this._isNumber ? Number(entry) : entry;
	$gameVariables.setValue(this._gameVar, value);
	this.popScene();
};
//-----------------------------------------------------------------------------
// Get the scene's custom scene background
// No need to check if Scene Backgrounds is installed because this custom func
// is only called by that plugin
//-----------------------------------------------------------------------------
CGMZ_Scene_NumPad.prototype.CGMZ_getCustomSceneBackground = function() {
	return $cgmzTemp.sceneBackgroundPresets[this._opts?.sceneBackground || CGMZ.NumPad.SceneBackground];
};
//=============================================================================
// CGMZ_Window_NumPad_CurrentEntry
//-----------------------------------------------------------------------------
// Shows current numpad entry
//=============================================================================
function CGMZ_Window_NumPad_CurrentEntry(rect) {
	this.initialize.apply(this, arguments);
}
CGMZ_Window_NumPad_CurrentEntry.prototype = Object.create(Window_Base.prototype);
CGMZ_Window_NumPad_CurrentEntry.prototype.constructor = CGMZ_Window_NumPad_CurrentEntry;
//-----------------------------------------------------------------------------
// Initialize
//-----------------------------------------------------------------------------
CGMZ_Window_NumPad_CurrentEntry.prototype.initialize = function(rect, opts) {
	Window_Base.prototype.initialize.call(this, rect);
	const windowSettings = opts?.currentWindowSettings || CGMZ.NumPad.CurrentWindowSettings;
	const windowBg = opts?.currentWindowBackground || CGMZ.NumPad.CurrentWindowBackground;
	if(Imported.CGMZ_WindowSettings && windowSettings) this.CGMZ_setWindowSettings(windowSettings);
	if(Imported.CGMZ_WindowBackgrounds && windowBg) this.CGMZ_setWindowBackground(windowBg);
	this._isPrivate = opts.isPrivate;
	this._entry = "";
	this._maxNumbers = 0;
	this._minNumbers = 0;
	this.refresh();
};
//-----------------------------------------------------------------------------
// Refresh
//-----------------------------------------------------------------------------
CGMZ_Window_NumPad_CurrentEntry.prototype.refresh = function() {
	this.contents.clear();
	this.drawCurrentEntry();
};
//-----------------------------------------------------------------------------
// Draw current entry
//-----------------------------------------------------------------------------
CGMZ_Window_NumPad_CurrentEntry.prototype.drawCurrentEntry = function() {
	const baseString = (this._isPrivate) ? '*'.repeat(this._entry.length) : this._entry;
	const string = baseString.padEnd(this._maxNumbers, this._placeholder);
	this.drawText(string, 0, 0, this.contents.width, 'center');
};
//-----------------------------------------------------------------------------
// Add Number
//-----------------------------------------------------------------------------
CGMZ_Window_NumPad_CurrentEntry.prototype.addNumber = function(number) {
	this._entry = this._entry.concat(number);
	this.refresh();
};
//-----------------------------------------------------------------------------
// Remove Number
//-----------------------------------------------------------------------------
CGMZ_Window_NumPad_CurrentEntry.prototype.removeNumber = function() {
	this._entry = this._entry.slice(0, -1);
	this.refresh();
};
//-----------------------------------------------------------------------------
// Set max numbers
//-----------------------------------------------------------------------------
CGMZ_Window_NumPad_CurrentEntry.prototype.setMax = function(maxNumbers) {
	this._maxNumbers = maxNumbers;
	this.refresh();
};
//-----------------------------------------------------------------------------
// Set min numbers
//-----------------------------------------------------------------------------
CGMZ_Window_NumPad_CurrentEntry.prototype.setMin = function(minNumbers) {
	this._minNumbers = minNumbers;
	this.refresh();
};
//-----------------------------------------------------------------------------
// Set placeholder
//-----------------------------------------------------------------------------
CGMZ_Window_NumPad_CurrentEntry.prototype.setPlaceholder = function(placeholder) {
	this._placeholder = placeholder;
	this.refresh();
};
//-----------------------------------------------------------------------------
// Is more numbers ok?
//-----------------------------------------------------------------------------
CGMZ_Window_NumPad_CurrentEntry.prototype.canEnterNumber = function() {
	return this._maxNumbers > this._entry.length;
};
//-----------------------------------------------------------------------------
// Is submit ok?
//-----------------------------------------------------------------------------
CGMZ_Window_NumPad_CurrentEntry.prototype.canSubmit = function() {
	return this._minNumbers <= this._entry.length;
};
//=============================================================================
// CGMZ_Window_NumPad_NumberSelect
//-----------------------------------------------------------------------------
// Selectable window for choosing a number in the numpad
//=============================================================================
function CGMZ_Window_NumPad_NumberSelect(rect) {
	this.initialize.apply(this, arguments);
}
CGMZ_Window_NumPad_NumberSelect.prototype = Object.create(Window_Selectable.prototype);
CGMZ_Window_NumPad_NumberSelect.prototype.constructor = CGMZ_Window_NumPad_NumberSelect;
//-----------------------------------------------------------------------------
// Initialize
//-----------------------------------------------------------------------------
CGMZ_Window_NumPad_NumberSelect.prototype.initialize = function(rect, opts) {
	Window_Selectable.prototype.initialize.call(this, rect);
	const windowSettings = opts?.selectWindowSettings || CGMZ.NumPad.SelectWindowSettings;
	const windowBg = opts?.selectWindowBackground || CGMZ.NumPad.SelectWindowBackground;
	if(Imported.CGMZ_WindowSettings && windowSettings) this.CGMZ_setWindowSettings(windowSettings);
	if(Imported.CGMZ_WindowBackgrounds && windowBg) this.CGMZ_setWindowBackground(windowBg);
	this._entryWindow = null;
	this.select(0);
	this.activate();
	this.refresh();
};
//-----------------------------------------------------------------------------
// Max columns
//-----------------------------------------------------------------------------
CGMZ_Window_NumPad_NumberSelect.prototype.maxCols = function() {
	return 3;
};
//-----------------------------------------------------------------------------
// Max items
//-----------------------------------------------------------------------------
CGMZ_Window_NumPad_NumberSelect.prototype.maxItems = function() {
	return this._data ? this._data.length : 1;
};
//-----------------------------------------------------------------------------
// Current item
//-----------------------------------------------------------------------------
CGMZ_Window_NumPad_NumberSelect.prototype.item = function() {
	return this._data[this.index()];
};
//-----------------------------------------------------------------------------
// Refresh
//-----------------------------------------------------------------------------
CGMZ_Window_NumPad_NumberSelect.prototype.refresh = function() {
	this.makeItemList();
	this.createContents();
	this.drawAllItems();
};
//-----------------------------------------------------------------------------
// Determine if number entry is enabled
//-----------------------------------------------------------------------------
CGMZ_Window_NumPad_NumberSelect.prototype.isEnabled = function(item) {
	if(item === CGMZ.NumPad.DelText) return true;
	if(!this._entryWindow) return true;
	if(item === CGMZ.NumPad.OkText) return this._entryWindow.canSubmit();
	return this._entryWindow.canEnterNumber();
};
//-----------------------------------------------------------------------------
// Determine if current selected item is enabled
//-----------------------------------------------------------------------------
CGMZ_Window_NumPad_NumberSelect.prototype.isCurrentItemEnabled = function() {
	return this.isEnabled(this.item());
};
//-----------------------------------------------------------------------------
// Make item list
//-----------------------------------------------------------------------------
CGMZ_Window_NumPad_NumberSelect.prototype.makeItemList = function() {
	this._data = ["1", "2", "3", "4", "5", "6", "7", "8", "9", CGMZ.NumPad.DelText, "0", CGMZ.NumPad.OkText];
};
//-----------------------------------------------------------------------------
// Draw item in list
//-----------------------------------------------------------------------------
CGMZ_Window_NumPad_NumberSelect.prototype.drawItem = function(index) {
	const item = this._data[index];
	const rect = this.itemRectWithPadding(index);
	this.changePaintOpacity(this.isEnabled(item));
	this.drawText(item, rect.x, rect.y, rect.width, 'center');
};
//-----------------------------------------------------------------------------
// Set Entry Window
//-----------------------------------------------------------------------------
CGMZ_Window_NumPad_NumberSelect.prototype.setEntryWindow = function(entryWindow) {
	this._entryWindow = entryWindow;
	this.refresh();
};
//-----------------------------------------------------------------------------
// Play the OK sound
//-----------------------------------------------------------------------------
CGMZ_Window_NumPad_NumberSelect.prototype.playOkSound = function() {
	if(CGMZ.NumPad.NumberSE.name) {
		const se = JSON.parse(JSON.stringify(CGMZ.NumPad.NumberSE));
		se.volume = CGMZ_Utils.applyVariance(se.volume, CGMZ.NumPad.VolumeVariance + 1);
		se.pitch = CGMZ_Utils.applyVariance(se.pitch, CGMZ.NumPad.PitchVariance + 1);
		AudioManager.playSe(se);
	} else {
		Window_Selectable.prototype.playOkSound.call(this);
	}
};