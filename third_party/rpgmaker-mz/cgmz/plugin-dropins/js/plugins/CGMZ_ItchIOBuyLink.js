/*:
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/itchiobuylink/
 * @target MZ
 * @base CGMZ_Core
 * @orderAfter CGMZ_Core
 * @plugindesc Adds a link to your game's itch io page.
 * @help
 * ============================================================================
 * For terms and conditions using this plugin in your game please visit:
 * https://www.caspergaming.com/terms-of-use/
 * ============================================================================
 * Become a Patron to get access to beta/alpha plugins plus other goodies!
 * https://www.patreon.com/CasperGamingRPGM
 * ============================================================================
 * Version: 1.0.2
 * ----------------------------------------------------------------------------
 * Compatibility: Only tested with my CGMZ plugins.
 * Made for RPG Maker MZ 1.9.0
 * ----------------------------------------------------------------------------
 * Description: This plugin adds a scene where it will load and display info
 * from your game's itch io page, such as game title, a clickable URL, and any
 * sale info about your game. You can also display an image.
 * ----------------------------------------------------------------------------
 * Documentation:
 * ----------------------------Mandatory Set up--------------------------------
 * You must set up the plugin parameters before this plugin will work.
 * 
 * The username parameter should be your itch IO username as seen in your URL.
 * The domain parameter should probably be itch.io.
 * The game name parameter should be your games name as seen in the game URL
 * If your game uses a secret (not public yet), you can fill that parameter in
 * but it is recommended only for testing purposes since anyone can view it.
 *
 * Example of how to find what to set plugin parameters:
 * casper-gaming.itch.io/example-game-name?secret=85pXFixTXzDnGPpVyaC82vGRP1Y
 * |  username  |domain |    game name    |      |          Secret           |
 * -----------------------------Calling the API--------------------------------
 * This plugin will automatically call the API for you when the game loads,
 * and it will store the data temporarily while the game is playing. If this
 * first API call returns unsuccessfully, Itch IO data will not be able to be
 * displayed. You can optionally set the plugin to retry the API if previous
 * calls have failed when exiting the Itch IO scene. However, once a call has
 * succeeded, this plugin will no longer make API calls.
 * ----------------------------Plugin Commands---------------------------------
 * This plugin supports the following command:
 * 
 * • Call Scene
 * This will call the itch IO buy link scene.
 * ------------------------------JavaScript------------------------------------
 * You can manually call the Itch IO buy link scene with the following JS:
 * SceneManager.push(CGMZ_Scene_ItchIO);
 * ------------------------------Saved Games-----------------------------------
 * This plugin is fully compatible with saved games. This means you can:
 *
 * ✓ Add this plugin to a saved game and it will work as expected
 * ✓ Change any plugin params and changes will be reflected in saved games
 * ✓ Remove the plugin with no issue to save data
 * -----------------------------Filename---------------------------------------
 * The filename for this plugin MUST remain CGMZ_ItchIOBuyLink.js
 * This is what it comes as when downloaded. The filename is used to load
 * parameters and execute plugin commands. If you change it, things will begin
 * behaving incorrectly and your game will probably crash. Please do not
 * rename the js file.
 * --------------------------Latest Version------------------------------------
 * Hi all, this latest version adds an integration with [CGMZ] Window Settings
 * which is how you will now control each window's windowskins, tones, styles,
 * and more. This integration allows me to add more window settings without
 * needing to update each plugin with windows individually, and also provides
 * an easy to re-use preset for you. It also added an integration with [CGMZ]
 * Window Backgrounds so you can now show a background image as the window
 * background, including animated scrolling parallax images.
 *
 * Version 1.0.2
 * - Added [CGMZ] Window Settings integration
 * - Added [CGMZ] Window Backgrounds integration
 *
 * @command Call Scene
 * @desc Calls the Itch IO buy link scene with purchase/cancel buttons.
 *
 * @param Username
 * @desc Your itch io username (part of the URL).
 *
 * @param Domain
 * @default itch.io
 * @desc The main domain to use for your game
 *
 * @param Game Name
 * @desc The game name as seen in your games URL
 *
 * @param Secret
 * @desc Your games secret if not public yet (recommended for testing purposes only).
 *
 * @param Retry API
 * @type boolean
 * @desc If true, will retry the Itch.io API after closing the Itch.io scene if there was an error
 * @default false
 *
 * @param Window Options
 *
 * @param Window Width
 * @parent Window Options
 * @type number
 * @min 0
 * @max 100
 * @desc The width as a % of game screen of the Itch IO window
 * @default 50
 *
 * @param Window Height
 * @parent Window Options
 * @type number
 * @min 0
 * @max 100
 * @desc The height as a % of game screen of the Itch IO window
 * @default 50
 *
 * @param Text Options
 *
 * @param Title Color
 * @parent Text Options
 * @type color
 * @desc The color to show the game title in
 * @default 1
 *
 * @param Price Color
 * @parent Text Options
 * @type color
 * @desc The color to show the current price text in
 * @default 3
 *
 * @param Original Price Color
 * @parent Text Options
 * @type color
 * @desc The color to show the original price text in (if sale)
 * @default 2
 *
 * @param Buy Now Text
 * @parent Text Options
 * @desc The buy now text to show on the command window
 * @default Buy Now
 *
 * @param Cancel Text
 * @parent Text Options
 * @desc The cancel text to show on the command window
 * @default No Thanks
 *
 * @param Error Text
 * @parent Text Options
 * @desc The text to show if itch io data failed to load
 * @default Could not connect to Itch IO, please check your internet connection.
 *
 * @param Free Text
 * @parent Text Options
 * @desc The text to show if price is $0
 * @default Free
 *
 * @param Sale Text
 * @parent Text Options
 * @desc Text to show after the sale % off
 * @default % Off!
 *
 * @param Header Text
 * @parent Text Options
 * @desc Text to show at the top of the window. Set to blank to not use
 * @default Available on Itch IO!
 *
 * @param Original Price Label
 * @parent Text Options
 * @desc Text to show before the original price (if sale)
 * @default Original Price:
 *
 * @param Sale Price Label
 * @parent Text Options
 * @desc Text to show before the sale price (if sale)
 * @default Sale Price:
 *
 * @param Integrations
 *
 * @param Scene Background
 * @parent Integrations
 * @desc The [CGMZ] Scene Backgrounds preset id to use
 *
 * @param Controls Window
 * @parent Integrations
 * @desc The [CGMZ] Controls Window preset id to use
 *
 * @param Detail Window Settings
 * @parent Integrations
 * @desc The [CGMZ] Window Settings preset id to use in the details window
 *
 * @param Command Window Settings
 * @parent Integrations
 * @desc The [CGMZ] Window Settings preset id to use in the command window
 *
 * @param Detail Window Background
 * @parent Integrations
 * @desc The [CGMZ] Window Backgrounds preset id to use in the details window
 *
 * @param Command Window Background
 * @parent Integrations
 * @desc The [CGMZ] Window Backgrounds preset id to use in the command window
*/
Imported.CGMZ_ItchIO = true;
CGMZ.Versions["Itch IO Buy Link"] = "1.0.2";
CGMZ.ItchIO = {};
CGMZ.ItchIO.parameters = PluginManager.parameters('CGMZ_ItchIOBuyLink');
CGMZ.ItchIO.Username = CGMZ.ItchIO.parameters["Username"];
CGMZ.ItchIO.Domain = CGMZ.ItchIO.parameters["Domain"];
CGMZ.ItchIO.GameName = CGMZ.ItchIO.parameters["Game Name"];
CGMZ.ItchIO.Secret = CGMZ.ItchIO.parameters["Secret"];
CGMZ.ItchIO.BuyNowText = CGMZ.ItchIO.parameters["Buy Now Text"];
CGMZ.ItchIO.CancelText = CGMZ.ItchIO.parameters["Cancel Text"];
CGMZ.ItchIO.ErrorText = CGMZ.ItchIO.parameters["Error Text"];
CGMZ.ItchIO.FreeText = CGMZ.ItchIO.parameters["Free Text"];
CGMZ.ItchIO.SaleText = CGMZ.ItchIO.parameters["Sale Text"];
CGMZ.ItchIO.HeaderText = CGMZ.ItchIO.parameters["Header Text"];
CGMZ.ItchIO.SalePriceLabel = CGMZ.ItchIO.parameters["Sale Price Label"];
CGMZ.ItchIO.OriginalPriceLabel = CGMZ.ItchIO.parameters["Original Price Label"];
CGMZ.ItchIO.CommandWindowSettings = CGMZ.ItchIO.parameters["Command Window Settings"];
CGMZ.ItchIO.DetailsWindowSettings = CGMZ.ItchIO.parameters["Details Window Settings"];
CGMZ.ItchIO.CommandWindowBackground = CGMZ.ItchIO.parameters["Command Window Background"];
CGMZ.ItchIO.DetailsWindowBackground = CGMZ.ItchIO.parameters["Details Window Background"];
CGMZ.ItchIO.SceneBackground = CGMZ.ItchIO.parameters["Scene Background"];
CGMZ.ItchIO.ControlsWindow = CGMZ.ItchIO.parameters["Controls Window"];
CGMZ.ItchIO.TitleColor = Number(CGMZ.ItchIO.parameters["Title Color"]);
CGMZ.ItchIO.PriceColor = Number(CGMZ.ItchIO.parameters["Price Color"]);
CGMZ.ItchIO.OriginalPriceColor = Number(CGMZ.ItchIO.parameters["Original Price Color"]);
CGMZ.ItchIO.WindowWidth = Number(CGMZ.ItchIO.parameters["Window Width"]);
CGMZ.ItchIO.WindowHeight = Number(CGMZ.ItchIO.parameters["Window Height"]);
CGMZ.ItchIO.RetryAPI = (CGMZ.ItchIO.parameters["Retry API"] === 'true');
CGMZ.ItchIO.GameData = null;
//=============================================================================
// Scene_Boot
//-----------------------------------------------------------------------------
// Call itch io API
//=============================================================================
//-----------------------------------------------------------------------------
// Call itch io API
//-----------------------------------------------------------------------------
const alias_CGMZ_ItchIO_Scene_Boot_start = Scene_Boot.prototype.start;
Scene_Boot.prototype.start = function() {
    alias_CGMZ_ItchIO_Scene_Boot_start.call(this);
	let url = "https://" + CGMZ.ItchIO.Username + "." + CGMZ.ItchIO.Domain + "/" + CGMZ.ItchIO.GameName + "/data.json";
	if(CGMZ.ItchIO.Secret) {
		url += "?secret=" + CGMZ.ItchIO.Secret;
	}
	$cgmzTemp.requestResponse(url, $cgmzTemp.setItchIOData);
};
//=============================================================================
// CGMZ_Temp
//-----------------------------------------------------------------------------
// Setup Itch IO data and add plugin commands
//=============================================================================
//-----------------------------------------------------------------------------
// Setup Itch IO data
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.setItchIOData = function(gameData) {
	CGMZ.ItchIO.GameData = gameData;
};
//-----------------------------------------------------------------------------
// Alias - Register Plugin Commands
//-----------------------------------------------------------------------------
const CGMZ_itchIoIntegration_cgmz_temp_registerPluginCommands = CGMZ_Temp.prototype.registerPluginCommands;
CGMZ_Temp.prototype.registerPluginCommands = function() {
	CGMZ_itchIoIntegration_cgmz_temp_registerPluginCommands.call(this);
	PluginManager.registerCommand("CGMZ_ItchIOBuyLink", "Call Scene", this.pluginCommandItchIOCallScene);
};
//-----------------------------------------------------------------------------
// Plugin Command - Call Scene
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandItchIOCallScene = function() {
	SceneManager.push(CGMZ_Scene_ItchIO);
};
//=============================================================================
// CGMZ_Scene_ItchIO
//-----------------------------------------------------------------------------
// Handle the itch io scene
//=============================================================================
function CGMZ_Scene_ItchIO() {
    this.initialize.apply(this, arguments);
}
CGMZ_Scene_ItchIO.prototype = Object.create(Scene_MenuBase.prototype);
CGMZ_Scene_ItchIO.prototype.constructor = CGMZ_Scene_ItchIO;
//-----------------------------------------------------------------------------
// Create itch io windows
//-----------------------------------------------------------------------------
CGMZ_Scene_ItchIO.prototype.create = function() {
    Scene_MenuBase.prototype.create.call(this);
	this.createDisplayWindow();
    this.createCommandWindow();
};
//-----------------------------------------------------------------------------
// Create display window
//-----------------------------------------------------------------------------
CGMZ_Scene_ItchIO.prototype.createDisplayWindow = function() {
    const rect = this.displayWindowRect();
    this._displayWindow = new CGMZ_Window_ItchIODisplay(rect);
	this._displayWindow.refresh();
    this.addWindow(this._displayWindow);
};
//-----------------------------------------------------------------------------
// Get display window rect
//-----------------------------------------------------------------------------
CGMZ_Scene_ItchIO.prototype.displayWindowRect = function() {
	const w = Graphics.boxWidth * (CGMZ.ItchIO.WindowWidth / 100.0);
	const h = Graphics.boxHeight * (CGMZ.ItchIO.WindowHeight / 100.0);
	const x = (Graphics.boxWidth - w) / 2; 
	const y = (Graphics.boxHeight - h) / 2;
	return new Rectangle(x, y, w, h);
};
//-----------------------------------------------------------------------------
// Create display window
//-----------------------------------------------------------------------------
CGMZ_Scene_ItchIO.prototype.createCommandWindow = function() {
    const rect = this.commandWindowRect();
    this._commandWindow = new CGMZ_Window_ItchIOCommand(rect);
	this._commandWindow.setHandler('purchase', this.onPurchase.bind(this));
	this._commandWindow.setHandler('cancel', this.onCancel.bind(this));
    this.addWindow(this._commandWindow);
};
//-----------------------------------------------------------------------------
// Get display window rect
//-----------------------------------------------------------------------------
CGMZ_Scene_ItchIO.prototype.commandWindowRect = function() {
	const x = this._displayWindow.x;
	const y = this._displayWindow.y + this._displayWindow.height;
	const w = this._displayWindow.width;
	const h = this.calcWindowHeight(1, true);
	return new Rectangle(x, y, w, h);
};
//-----------------------------------------------------------------------------
// On purchase
//-----------------------------------------------------------------------------
CGMZ_Scene_ItchIO.prototype.onPurchase = function() {
	const url = "https://" + CGMZ.ItchIO.Username + "." + CGMZ.ItchIO.Domain + "/" + CGMZ.ItchIO.GameName;
	(Utils.isNwjs()) ? require('nw.gui').Shell.openExternal(url) : window.open(url);
	this._commandWindow.activate();
};
//-----------------------------------------------------------------------------
// On cancel
//-----------------------------------------------------------------------------
CGMZ_Scene_ItchIO.prototype.onCancel = function() {
	if(CGMZ.ItchIO.RetryAPI && CGMZ.ItchIO.GameData === null) {
		let url = "https://" + CGMZ.ItchIO.Username + "." + CGMZ.ItchIO.Domain + "/" + CGMZ.ItchIO.GameName + "/data.json";
		if(CGMZ.ItchIO.Secret) {
			url += "?secret=" + CGMZ.ItchIO.Secret;
		}
		$cgmzTemp.requestResponse(url, $cgmzTemp.setItchIOData);
	}
	this.popScene();
};
//-----------------------------------------------------------------------------
// Get the scene's custom scene background
// No need to check if Scene Backgrounds is installed because this custom func
// is only called by that plugin
//-----------------------------------------------------------------------------
CGMZ_Scene_ItchIO.prototype.CGMZ_getCustomSceneBackground = function() {
	return $cgmzTemp.sceneBackgroundPresets[CGMZ.ItchIO.SceneBackground];
};
//-----------------------------------------------------------------------------
// Get controls window preset for [CGMZ] Controls Window
// No need to check if Controls Window is installed because this custom func
// is only called by that plugin
//-----------------------------------------------------------------------------
CGMZ_Scene_ItchIO.prototype.CGMZ_getControlsWindowOtherPreset = function() {
	return $cgmzTemp.getControlWindowPresetOther(CGMZ.ItchIO.ControlsWindow);
};
//=============================================================================
// CGMZ_Window_ItchIOCommand
//-----------------------------------------------------------------------------
// Command window for choosing to buy now or cancel the itch io scene
//=============================================================================
function CGMZ_Window_ItchIOCommand(rect) {
    this.initialize.apply(this, arguments);
}
CGMZ_Window_ItchIOCommand.prototype = Object.create(Window_HorzCommand.prototype);
CGMZ_Window_ItchIOCommand.prototype.constructor = CGMZ_Window_ItchIOCommand;
//-----------------------------------------------------------------------------
// Initialize the window
//-----------------------------------------------------------------------------
CGMZ_Window_ItchIOCommand.prototype.initialize = function(rect) {
	Window_HorzCommand.prototype.initialize.call(this, rect);
	if(Imported.CGMZ_WindowSettings && CGMZ.ItchIO.CommandWindowSettings) this.CGMZ_setWindowSettings(CGMZ.ItchIO.CommandWindowSettings);
	if(Imported.CGMZ_WindowBackgrounds && CGMZ.ItchIO.CommandWindowBackground) this.CGMZ_setWindowBackground(CGMZ.ItchIO.CommandWindowBackground);
};
//-----------------------------------------------------------------------------
// Max columns to display
//-----------------------------------------------------------------------------
CGMZ_Window_ItchIOCommand.prototype.maxCols = function() {
    return 2;
};
//-----------------------------------------------------------------------------
// Make list of commands to display
//-----------------------------------------------------------------------------
CGMZ_Window_ItchIOCommand.prototype.makeCommandList = function() {
	this.addCommand(CGMZ.ItchIO.BuyNowText, 'purchase', true);
	this.addCommand(CGMZ.ItchIO.CancelText, 'cancel', true);
};
//=============================================================================
// CGMZ_Window_ItchIODisplay
//-----------------------------------------------------------------------------
// Window displaying itch io game info
//=============================================================================
function CGMZ_Window_ItchIODisplay() {
    this.initialize.apply(this, arguments);
}
CGMZ_Window_ItchIODisplay.prototype = Object.create(Window_Base.prototype);
CGMZ_Window_ItchIODisplay.prototype.constructor = CGMZ_Window_ItchIODisplay;
//-----------------------------------------------------------------------------
// Initialize the window
//-----------------------------------------------------------------------------
CGMZ_Window_ItchIODisplay.prototype.initialize = function(rect) {
	Window_Base.prototype.initialize.call(this, rect);
	if(Imported.CGMZ_WindowSettings && CGMZ.ItchIO.DetailsWindowSettings) this.CGMZ_setWindowSettings(CGMZ.ItchIO.DetailsWindowSettings);
	if(Imported.CGMZ_WindowBackgrounds && CGMZ.ItchIO.DetailsWindowBackground) this.CGMZ_setWindowBackground(CGMZ.ItchIO.DetailsWindowBackground);
};
//-----------------------------------------------------------------------------
// Refresh
//-----------------------------------------------------------------------------
CGMZ_Window_ItchIODisplay.prototype.refresh = function() {
	if(!CGMZ.ItchIO.GameData || (CGMZ.ItchIO.GameData.errors && CGMZ.ItchIO.GameData.errors.length > 0)) {
		this.CGMZ_drawText(CGMZ.ItchIO.ErrorText, 0, 0, 0, this.contents.width, 'center');
		return;
	}
	this.CGMZ_drawTextLine(CGMZ.ItchIO.HeaderText, 0, 0, this.contents.width, 'center');
	if(CGMZ.ItchIO.GameData.sale) {
		this.drawSaleInfo();
	} else {
		this.drawRegularInfo();
	}
};
//-----------------------------------------------------------------------------
// Draw Regular Info
//-----------------------------------------------------------------------------
CGMZ_Window_ItchIODisplay.prototype.drawRegularInfo = function() {
	let y = this.lineHeight() * 2 * (CGMZ.ItchIO.HeaderText !== "");
	y += this.CGMZ_drawText('\\c[' + CGMZ.ItchIO.TitleColor + ']' + CGMZ.ItchIO.GameData.title, 0, 0, y, this.contents.width, 'center');
	const price = Number(CGMZ.ItchIO.GameData.price.replace(/[^0-9\.]+/g,""));
	priceString = (price <= 0) ? CGMZ.ItchIO.FreeText : CGMZ.ItchIO.GameData.price;
	this.contents.fontBold = true;
	this.changeTextColor(ColorManager.textColor(CGMZ.ItchIO.PriceColor));
	this.drawText(priceString, 0, y, this.contents.width, 'center');
	this.changeTextColor(ColorManager.normalColor());
	this.contents.fontBold = false;
};
//-----------------------------------------------------------------------------
// Draw Sale Info
//-----------------------------------------------------------------------------
CGMZ_Window_ItchIODisplay.prototype.drawSaleInfo = function() {
	let y = this.lineHeight() * 2 * (CGMZ.ItchIO.HeaderText !== "");
	y += this.CGMZ_drawText('\\c[' + CGMZ.ItchIO.TitleColor + ']' + CGMZ.ItchIO.GameData.title, 0, 0, y, this.contents.width, 'center');
	y += this.CGMZ_drawText(CGMZ.ItchIO.GameData.sale.title + " - " + CGMZ.ItchIO.GameData.sale.rate + CGMZ.ItchIO.SaleText, 0, 0, y, this.contents.width, 'center');
	const price = Number(CGMZ.ItchIO.GameData.price.replace(/[^0-9\.]+/g,""));
	priceString = (price <= 0) ? CGMZ.ItchIO.FreeText : CGMZ.ItchIO.GameData.price;
	this.changeTextColor(ColorManager.textColor(CGMZ.ItchIO.OriginalPriceColor));
	this.drawText(CGMZ.ItchIO.OriginalPriceLabel + CGMZ.ItchIO.GameData.original_price, 0, y, this.contents.width, 'center');
	y += this.lineHeight();
	this.contents.fontBold = true;
	this.changeTextColor(ColorManager.textColor(CGMZ.ItchIO.PriceColor));
	this.drawText(CGMZ.ItchIO.SalePriceLabel + priceString, 0, y, this.contents.width, 'center');
	this.changeTextColor(ColorManager.normalColor());
	this.contents.fontBold = false;
};