/*:
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/shopcommandwindow/
 * @target MZ
 * @base CGMZ_Core
 * @orderAfter CGMZ_Core
 * @plugindesc Manage the shop commands in the shop command window
 * @help
 * ============================================================================
 * For terms and conditions using this plugin in your game please visit:
 * https://www.caspergaming.com/terms-of-use/
 * ============================================================================
 * Become a Patron to get access to beta/alpha plugins plus other goodies!
 * https://www.patreon.com/CasperGamingRPGM
 * ============================================================================
 * Version: 1.0.0
 * ----------------------------------------------------------------------------
 * Compatibility: Only tested with my CGMZ plugins.
 * Made for RPG Maker MZ 1.9.0
 * ----------------------------------------------------------------------------
 * Description: Use this plugin to easily manage the command window in the
 * shop scene. It allows you to re-arrange commands or use JavaScript to 
 * add custom commands which are capable of calling custom plugin scenes or
 * functions.
 * ----------------------------------------------------------------------------
 * Documentation:
 * -----------------------------Compatibility----------------------------------
 * This plugin will overwrite the default shop command window if keep originals
 * is off. It is best to place this below any other plugins that add commands
 * to the shop command window if this option is used.
 * -----------------------------Command Symbol---------------------------------
 * The command symbol should be unique and not blank for every command. This
 * symbol is how the plugin knows internally which JS code to run.
 *
 * Some Command Symbols can have special meanings, mainly when they represent
 * the original commands.
 * The following symbols represent the original commands (case sensitive):
 * buy - Will handle like the original buy command
 * sell - Will handle like the original sell command
 * cancel - Will handle like the original cancel command
 * 
 * It is important that you do not use these strings as the Command Symbol
 * property unless you mean to refer to the original commands.
 * ------------------------------Saved Games-----------------------------------
 * This plugin is fully compatible with saved games
 *
 * This means the following will work in saved games:
 * ✓ Add this plugin to your game
 * ✓ Modify plugin parameters
 * ✓ Remove this plugin from your game
 * -----------------------------Filename---------------------------------------
 * The filename for this plugin MUST remain CGMZ_ShopCommandWindow.js
 * This is what it comes as when downloaded. The filename is used to load
 * parameters and execute plugin commands. If you change it, things will begin
 * behaving incorrectly and your game will probably crash. Please do not
 * rename the js file.
 * --------------------------Latest Version------------------------------------
 * Hi all, welcome to the full release of this plugin! If you are new here,
 * this plugin allows you to add custom commands to the shop command window.
 * You can also do things like add conditions to commands, so if you do not
 * want Sell to appear in a buy-only shop, you can do that.
 *
 * For existing users, this adds the option to make commands work even when the
 * command window is inactive. This means you can swap between buy and sell by
 * hotkey even if the buy or sell window is active, without needing to cancel
 * back to the command window and selecting the other command.
 *
 * Version 1.0.0
 * - Added option to use hotkeys even when command window inactive
 *
 * @param Command Lines
 * @type number
 * @min 1
 * @default 1
 * @desc This is the number of lines of commands to show before scrolling
 *
 * @param Command Columns
 * @type number
 * @min 1
 * @default 3
 * @desc This is the number of commands in each line of the window
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
 * @param JS Enable Condition
 * @type multiline_string
 * @default return true;
 * @desc JavaScript to run to determine if the command is enabled
 *
 * @param JS Show Condition
 * @type multiline_string
 * @default return true;
 * @desc JavaScript to run to determine if the command is shown
 *
 * @param Command Background
 *
 * @param Background Image
 * @parent Command Background
 * @type file
 * @dir img
 * @desc A background image to use for the command. Blank = default black rectangle
 *
 * @param Background Image X
 * @parent Command Background
 * @type number
 * @default 0
 * @min 0
 * @desc The x coordinate to start the background image from the source image (upper left corner)
 *
 * @param Background Image Y
 * @parent Command Background
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
 *
 * @param Command Hotkey
 *
 * @param Keyboard Hotkey
 * @parent Command Hotkey
 * @desc The key that, when pressed, will perform the function of the command
 *
 * @param Gamepad Hotkey
 * @parent Command Hotkey
 * @desc Gamepad button that, when pressed, will perform the function of the command
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
 * @default -1
 *
 * @param Inactive Hotkey
 * @parent Command Hotkey
 * @default false
 * @type boolean
 * @desc If true, the hotkey will work even if the command window is not active.
*/
Imported.CGMZ_ShopCommandWindow = true;
CGMZ.Versions["Shop Command Window"] = "1.0.0";
CGMZ.ShopCommandWindow = {};
CGMZ.ShopCommandWindow.parameters = PluginManager.parameters('CGMZ_ShopCommandWindow');
CGMZ.ShopCommandWindow.Alignment = CGMZ.ShopCommandWindow.parameters["Alignment"];
CGMZ.ShopCommandWindow.IconAlignment = CGMZ.ShopCommandWindow.parameters["Icon Alignment"];
CGMZ.ShopCommandWindow.CommandLines = Number(CGMZ.ShopCommandWindow.parameters["Command Lines"]);
CGMZ.ShopCommandWindow.CommandColumns = Number(CGMZ.ShopCommandWindow.parameters["Command Columns"]);
CGMZ.ShopCommandWindow.KeepOriginals = (CGMZ.ShopCommandWindow.parameters["Keep Original Commands"] === "true");
CGMZ.ShopCommandWindow.Commands = CGMZ_Utils.parseJSON(CGMZ.ShopCommandWindow.parameters["Commands"], [], "[CGMZ] Shop Command Window", "Your Commands parameter had invalid JSON and could not be read.").map((command) => {
	const cmd = CGMZ_Utils.parseJSON(command, null, "[CGMZ] Shop Command Window", "One of your shop commands had invalid JSON and could not be read.");
	if(!cmd) return null;
	return {
		icon: Number(cmd.Icon),
		symbol: cmd["Command Symbol"] || Math.random().toString(36),
		name: cmd["Command Name"],
		js: cmd["JS Command"],
		jsShow: cmd["JS Show Condition"],
		jsEnable: cmd["JS Enable Condition"],
		hotkey: cmd["Keyboard Hotkey"],
		hotbutton: Number(cmd["Gamepad Hotkey"]),
		inactiveHotkey: (cmd["Inactive Hotkey"] === 'true'),
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
				y: Number(cmd["Selected Back Img Y"]) ||Number(cmd["Background Image Y"])
			}
		}
	};
}).filter(x => !!x);
//=============================================================================
// Scene Shop
//-----------------------------------------------------------------------------
// Handling for command window entries
//=============================================================================
//-----------------------------------------------------------------------------
// Handling for custom Commands added through the plugin
//-----------------------------------------------------------------------------
Scene_Shop.prototype.CGMZ_ShopCommandWindow_commandCustom = function() {
	for(const cmd of CGMZ.ShopCommandWindow.Commands) {
		if(this._commandWindow.currentSymbol() === cmd.symbol) {
			try {
				const hookFunc = new Function(cmd.js);
				hookFunc.call(this);
			}
			catch (e) {
				const origin = "[CGMZ] Shop Command Window";
				const suggestion = "Check your JavaScript command";
				CGMZ_Utils.reportError(e.message, origin, suggestion);
			}
			break;
		}
	}
};
//-----------------------------------------------------------------------------
// Add additional commands
//-----------------------------------------------------------------------------
const alias_CGMZ_ShopCommandWindow_createCommandWindow = Scene_Shop.prototype.createCommandWindow;
Scene_Shop.prototype.createCommandWindow = function() {
	alias_CGMZ_ShopCommandWindow_createCommandWindow.call(this);
	for(const cmd of CGMZ.ShopCommandWindow.Commands) {
		if(this.CGMZ_ShopCommandWindow_isCustomCommand(cmd.symbol)) {
			this._commandWindow.setHandler(cmd.symbol, this.CGMZ_ShopCommandWindow_commandCustom.bind(this));
		}
		const data = {symbol: cmd.symbol, inactive: cmd.inactiveHotkey};
		if(cmd.hotkey) this.CGMZ_registerSceneHotkey("keyboard", cmd.hotkey, data);
		if(cmd.hotbutton >= 0) this.CGMZ_registerSceneHotkey("gamepad", cmd.hotbutton, data);
	}
};
//-----------------------------------------------------------------------------
// Determine if command is a custom command in need of custom handler
//-----------------------------------------------------------------------------
Scene_Shop.prototype.CGMZ_ShopCommandWindow_isCustomCommand = function(symbol) {
	return (symbol !== 'cancel' && symbol !== 'buy' && symbol !== 'sell');
};
//-----------------------------------------------------------------------------
// Change the rectangle height based on number of command lines
//-----------------------------------------------------------------------------
const alias_CGMZ_ShopCommandWindow_commandWindowRect = Scene_Shop.prototype.commandWindowRect;
Scene_Shop.prototype.commandWindowRect = function() {
    const rect = alias_CGMZ_ShopCommandWindow_commandWindowRect.call(this);
	rect.height = this.calcWindowHeight(CGMZ.ShopCommandWindow.CommandLines, true);
	return rect;
};
//-----------------------------------------------------------------------------
// Process a hotkey
//-----------------------------------------------------------------------------
Scene_Shop.prototype.CGMZ_processHotkey = function(data) {
	if(!data.inactive && !this._commandWindow.active) return;
	if(this._numberWindow.active) return;
	const commandWindowActiveSymbol = this._commandWindow.currentSymbol();
	const hasCommand = this._commandWindow.findSymbol(data.symbol);
	if(this._commandWindow.isHandled(data.symbol) && hasCommand >= 0) {
		if(data.symbol === 'cancel') {
			this._commandWindow.processCancel();
		} else {
			this._commandWindow.selectSymbol(data.symbol);
			this._commandWindow.processOk();
			if(this._commandWindow.isCurrentItemEnabled()) {
				if(data.symbol === 'sell') {
					this._buyWindow.deactivate();
					this._buyWindow.hide();
					if(this._categoryWindow.active) {
						this._sellWindow.deactivate();
					}
				} else if(data.symbol === 'buy') {
					this._sellWindow.deactivate();
					this._sellWindow.hide();
					this._categoryWindow.deactivate();
					this._categoryWindow.hide();
				}
			} else {
				this._commandWindow.selectSymbol(commandWindowActiveSymbol);
			}
		}
	}
};
//=============================================================================
// Window Shop Command
//-----------------------------------------------------------------------------
// Change commands in the command window
//=============================================================================
//-----------------------------------------------------------------------------
// Add original commands in original order if user wishes
//-----------------------------------------------------------------------------
const alias_CGMZ_ShopCommandWindow_makeCommandList = Window_ShopCommand.prototype.makeCommandList;
Window_ShopCommand.prototype.makeCommandList = function() {
	if(CGMZ.ShopCommandWindow.KeepOriginals) {
		alias_CGMZ_ShopCommandWindow_makeCommandList.call(this);
	}
	for(const cmd of CGMZ.ShopCommandWindow.Commands) {
		const showFunc = new Function(cmd.jsShow);
		if(showFunc.call(this)) {
			const enabledFunc = new Function(cmd.jsEnable);
			const enabled = enabledFunc.call(this);
			this.addCommand(cmd.name, cmd.symbol, enabled, {icon: cmd.icon, backImg: cmd.backImg, selectedInfo: cmd.selectedInfo});
		}
	}
};
//-----------------------------------------------------------------------------
// Change columns
//-----------------------------------------------------------------------------
Window_ShopCommand.prototype.maxCols = function() {
    return CGMZ.ShopCommandWindow.CommandColumns;
};
//-----------------------------------------------------------------------------
// Change alignment of command text
//-----------------------------------------------------------------------------
Window_ShopCommand.prototype.itemTextAlign = function() {
    return CGMZ.ShopCommandWindow.Alignment;
};
//-----------------------------------------------------------------------------
// Get the command icon
//-----------------------------------------------------------------------------
Window_ShopCommand.prototype.CGMZ_icon = function(index) {
	const ext = this._list[index]?.ext;
	if(!ext) return 0;
	const selected = (this._cgmz_lastIndex === index);
	return (selected && ext.selectedInfo?.icon) ? ext.selectedInfo.icon : ext.icon;
};
//-----------------------------------------------------------------------------
// Get the command name
//-----------------------------------------------------------------------------
const alias_CGMZ_ShopCommandWindow_ShopCommand_commandName = Window_ShopCommand.prototype.commandName;
Window_ShopCommand.prototype.commandName = function(index) {
	const selected = (this._cgmz_lastIndex === index);
	const ext = this._list[index]?.ext;
	const originalName = alias_CGMZ_ShopCommandWindow_ShopCommand_commandName.call(this, index);
	if(!selected || !ext) return originalName;
    return ext.selectedInfo?.name || originalName;
};
//-----------------------------------------------------------------------------
// Get selectable cgmz options
//-----------------------------------------------------------------------------
Window_ShopCommand.prototype.CGMZ_getSelectableCGMZOptions = function(index) {
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
	if(ext && ext.backImg) {
		const bg = {
			img: ext.backImg.img,
			imgX: ext.backImg.x,
			imgY: ext.backImg.y
		}
		return {bg: bg};
	}
	return Window_HorzCommand.prototype.CGMZ_getSelectableCGMZOptions.call(this, index);
};
//-----------------------------------------------------------------------------
// Draw new command item with potential icon and text codes
//-----------------------------------------------------------------------------
Window_ShopCommand.prototype.drawItem = function(index) {
	const rect = this.itemLineRect(index);
	const align = this.itemTextAlign();
	const icon = this.CGMZ_icon(index);
	this.resetTextColor();
	this.changePaintOpacity(this.isCommandEnabled(index));
	if(icon) {
		const iconX = (CGMZ.ShopCommandWindow.IconAlignment === 'left') ? rect.x : rect.x + rect.width - ImageManager.iconWidth;
		this.drawIcon(icon, iconX, rect.y + 2);
		rect.x += (ImageManager.iconWidth + 2) * (CGMZ.ShopCommandWindow.IconAlignment === 'left');
		rect.width -= ImageManager.iconWidth + 2;
	}
	this.CGMZ_drawTextLine(this.commandName(index), rect.x, rect.y, rect.width, align);
};
//-----------------------------------------------------------------------------
// Redraw old index
//-----------------------------------------------------------------------------
Window_ShopCommand.prototype.CGMZ_handleSelectionChangePrevious = function(index) {
	if(this._list?.[index]) this.redrawItem(index);
};
//-----------------------------------------------------------------------------
// Redraw new index
//-----------------------------------------------------------------------------
Window_ShopCommand.prototype.CGMZ_handleSelectionChangeNext = function(index) {
	if(this._list?.[index]) this.redrawItem(index);
};