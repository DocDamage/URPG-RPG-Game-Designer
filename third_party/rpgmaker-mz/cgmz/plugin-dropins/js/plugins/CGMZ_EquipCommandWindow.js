/*:
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/equipcommandwindow/
 * @target MZ
 * @base CGMZ_Core
 * @orderAfter CGMZ_Core
 * @plugindesc Manage the equip commands in the equip command window
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
 * equip scene. It allows you to re-arrange commands or use JavaScript to 
 * add custom commands which are capable of calling custom plugin scenes or
 * functions.
 * ----------------------------------------------------------------------------
 * Documentation:
 * -----------------------------Compatibility----------------------------------
 * This plugin will overwrite the default equip command window if keep
 * originals is off. It is best to place this below any other plugins that add
 * commands to the equip command window if this option is used.
 * -----------------------------Command Symbol---------------------------------
 * The command symbol should be unique and not blank for every command. This
 * symbol is how the plugin knows internally which JS code to run.
 *
 * Some Command Symbols can have special meanings, mainly when they represent
 * the original commands.
 * The following symbols represent the original commands (case sensitive):
 * equip - Will handle like the original equip command
 * optimize - Will handle like the original optimize command
 * clear - Will handle like the original clear command
 *
 * Please note that there are additional symbols that are not shown, which
 * should also not be used. These are "cancel" "pageup" and "pagedown". You
 * can create these if you would like them to display.
 * 
 * It is important that you do not use these strings as the Command Symbol
 * property unless you mean to refer to the original commands.
 * -------------------------Skip Command Window--------------------------------
 * This plugin allows you to totally skip the command window. To do so, set
 * the Command Lines parameter to 0 (no visible commands). When this is set to
 * 0, the command window will be hidden and the slot window will be selected
 * and function as the controlling window of the scene. If you have commands
 * you still want the player to be able to use, such as optimize or clear,
 * you can set a hotkey for those commands and allow them to work while the
 * command window is inactive.
 * ------------------------------Saved Games-----------------------------------
 * This plugin is fully compatible with saved games
 *
 * This means the following will work in saved games:
 * ✓ Add this plugin to your game
 * ✓ Modify plugin parameters
 * ✓ Remove this plugin from your game
 * -----------------------------Filename---------------------------------------
 * The filename for this plugin MUST remain CGMZ_EquipCommandWindow.js
 * This is what it comes as when downloaded. The filename is used to load
 * parameters and execute plugin commands. If you change it, things will begin
 * behaving incorrectly and your game will probably crash. Please do not
 * rename the js file.
 * ------------------------Latest Version--------------------------------------
 * Hi all, welcome to the full release of this plugin! If you are new here,
 * this plugin allows you to add any command you want to the equip scene's
 * command window. This includes being able to remove default commands or set
 * the order of the default commands, add icons or background images, etc.
 *
 * For existing users, this update adds the ability to totally hide the equip
 * command window. When the equip command window is hidden, the slot window
 * will act as the driving window of the scene. Without the command window,
 * you would not be able to activate shortcuts such as clear or optimize.
 * For this reason, this update also adds the option to let your commands
 * work via scene hotkeys even when the command window is not active. This
 * means you can still allow your player to press a button to clear or
 * optimize their gear (or any other command) without needing the command
 * window at all.
 * 
 * Version 1.0.0
 * - Added option to hide command window
 * - Added ability to activate scene hotkeys without command window
 *
 * @param Command Lines
 * @type number
 * @min 0
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
 * @type text
 * @desc Name of the command to display in the command window.
 *
 * @param Icon
 * @type icon
 * @default 0
 * @desc An icon to show for the command, if 0 will not show any icon
 *
 * @param Key
 * @desc A hotkey that, when pressed, will activate this command
 *
 * @param Gamepad Button
 * @type select
 * @option None
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
 * @desc A gamepad hotkey that, when pressed, will activate this command
 *
 * @param Inactive Hotkey
 * @default false
 * @type boolean
 * @desc If true, the hotkey will work even if the command window is not active
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
Imported.CGMZ_EquipCommandWindow = true;
CGMZ.Versions["Equip Command Window"] = "1.0.0";
CGMZ.EquipCommandWindow = {};
CGMZ.EquipCommandWindow.parameters = PluginManager.parameters('CGMZ_EquipCommandWindow');
CGMZ.EquipCommandWindow.Alignment = CGMZ.EquipCommandWindow.parameters["Alignment"];
CGMZ.EquipCommandWindow.IconAlignment = CGMZ.EquipCommandWindow.parameters["Icon Alignment"];
CGMZ.EquipCommandWindow.CommandLines = Number(CGMZ.EquipCommandWindow.parameters["Command Lines"]);
CGMZ.EquipCommandWindow.CommandColumns = Number(CGMZ.EquipCommandWindow.parameters["Command Columns"]);
CGMZ.EquipCommandWindow.KeepOriginals = (CGMZ.EquipCommandWindow.parameters["Keep Original Commands"] === "true");
CGMZ.EquipCommandWindow.Commands = CGMZ_Utils.parseJSON(CGMZ.EquipCommandWindow.parameters["Commands"], [], "[CGMZ] Equip Command Window", "Your Commands parameter had invalid JSON and could not be read.").map((command) => {
	const cmd = CGMZ_Utils.parseJSON(command, null, "[CGMZ] Equip Command Window", "One of your equip commands had invalid JSON and could not be read.");
	if(!cmd) return null;
	return {
		icon: Number(cmd.Icon),
		hotkeyGamepad: Number(cmd["Gamepad Button"]),
		symbol: cmd["Command Symbol"] || Math.random().toString(36),
		name: cmd["Command Name"],
		hotkey: cmd["Key"],
		inactiveHotkey: (cmd["Inactive Hotkey"] === 'true'),
		js: cmd["JS Command"],
		jsShow: cmd["JS Show Condition"],
		jsEnable: cmd["JS Enable Condition"],
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
// Scene Equip
//-----------------------------------------------------------------------------
// Handling for command window entries
//=============================================================================
//-----------------------------------------------------------------------------
// Handling for custom Commands added through the plugin
//-----------------------------------------------------------------------------
Scene_Equip.prototype.CGMZ_EquipCommandWindow_commandCustom = function() {
	for(const cmd of CGMZ.EquipCommandWindow.Commands) {
		if(this._commandWindow.currentSymbol() === cmd.symbol) {
			try {
				const hookFunc = new Function(cmd.js);
				hookFunc.call(this);
			}
			catch (e) {
				const origin = "[CGMZ] Equip Command Window";
				const suggestion = "Check your JavaScript command";
				CGMZ_Utils.reportError(e.message, origin, suggestion);
			}
			break;
		}
	}
};
//-----------------------------------------------------------------------------
// Set active window if equip command window is hidden
//-----------------------------------------------------------------------------
const alias_CGMZ_EquipCommandWindow_create = Scene_Equip.prototype.create;
Scene_Equip.prototype.create = function() {
    alias_CGMZ_EquipCommandWindow_create.call(this);
    if(!CGMZ.EquipCommandWindow.CommandLines) {
		this._commandWindow.deactivate();
		this._commandWindow.hide();
		this.commandEquip();
	}
};
//-----------------------------------------------------------------------------
// Add additional commands.
//-----------------------------------------------------------------------------
const alias_CGMZ_EquipCommandWindow_createCommandWindow = Scene_Equip.prototype.createCommandWindow;
Scene_Equip.prototype.createCommandWindow = function() {
	alias_CGMZ_EquipCommandWindow_createCommandWindow.call(this);
	for(const cmd of CGMZ.EquipCommandWindow.Commands) {
		if(this.CGMZ_EquipCommandWindow_isCustomCommand(cmd.symbol)) {
			this._commandWindow.setHandler(cmd.symbol, this.CGMZ_EquipCommandWindow_commandCustom.bind(this));
		}
		const hotkeyData = {symbol: cmd.symbol, inactive: cmd.inactiveHotkey};
		if(cmd.hotkey) this.CGMZ_registerSceneHotkey("keyboard", cmd.hotkey, hotkeyData);
		if(cmd.hotkeyGamepad >= 0) this.CGMZ_registerSceneHotkey("gamepad", cmd.hotkeyGamepad, hotkeyData);
	}
};
//-----------------------------------------------------------------------------
// Change behavior if command window is hidden
//-----------------------------------------------------------------------------
const alias_CGMZ_EquipCommandWindow_commandOptimize = Scene_Equip.prototype.commandOptimize;
Scene_Equip.prototype.commandOptimize = function() {
    alias_CGMZ_EquipCommandWindow_commandOptimize.call(this);
	if(!CGMZ.EquipCommandWindow.CommandLines) {
		this._commandWindow.deactivate();
		this._itemWindow.refresh();
		this.hideItemWindow();
		this._itemWindow.deactivate();
	}
};
//-----------------------------------------------------------------------------
// Change behavior if command window is hidden
//-----------------------------------------------------------------------------
const alias_CGMZ_EquipCommandWindow_commandClear = Scene_Equip.prototype.commandClear;
Scene_Equip.prototype.commandClear = function() {
    alias_CGMZ_EquipCommandWindow_commandClear.call(this);
	if(!CGMZ.EquipCommandWindow.CommandLines) {
		this._commandWindow.deactivate();
		this._itemWindow.refresh();
		this.hideItemWindow();
		this._itemWindow.deactivate();
	}
};
//-----------------------------------------------------------------------------
// Make cancel pop scene in slot window if command window is hidden
//-----------------------------------------------------------------------------
const alias_CGMZ_EquipCommandWindow_onSlotCancel = Scene_Equip.prototype.onSlotCancel;
Scene_Equip.prototype.onSlotCancel = function() {
	if(!CGMZ.EquipCommandWindow.CommandLines) {
		this.popScene();
	} else {
		alias_CGMZ_EquipCommandWindow_onSlotCancel.call(this);
	}
};
//-----------------------------------------------------------------------------
// Select equip automatically if actor changed and command window hidden
//-----------------------------------------------------------------------------
const alias_CGMZ_EquipCommandWindow_onActorChange = Scene_Equip.prototype.onActorChange;
Scene_Equip.prototype.onActorChange = function() {
    alias_CGMZ_EquipCommandWindow_onActorChange.call(this);
    if(!CGMZ.EquipCommandWindow.CommandLines) this.commandEquip();
};
//-----------------------------------------------------------------------------
// Determine if command is a custom command in need of custom handler
//-----------------------------------------------------------------------------
Scene_Equip.prototype.CGMZ_EquipCommandWindow_isCustomCommand = function(symbol) {
	return (symbol !== 'cancel' && symbol !== 'clear' && symbol !== 'optimize' && symbol !== 'equip' && symbol !== 'pageup' && symbol !== 'pagedown');
};
//-----------------------------------------------------------------------------
// Change the rectangle height based on number of command lines
//-----------------------------------------------------------------------------
const alias_CGMZ_EquipCommandWindow_commandWindowRect = Scene_Equip.prototype.commandWindowRect;
Scene_Equip.prototype.commandWindowRect = function() {
    const rect = alias_CGMZ_EquipCommandWindow_commandWindowRect.call(this);
	rect.height = (CGMZ.EquipCommandWindow.CommandLines) ? this.calcWindowHeight(CGMZ.EquipCommandWindow.CommandLines, true) : 0;
	return rect;
};
//-----------------------------------------------------------------------------
// Process a hotkey
//-----------------------------------------------------------------------------
Scene_Equip.prototype.CGMZ_processHotkey = function(data) {
	if(!this._commandWindow.active && !data.inactive) return;
	const commandWindowActiveState = this._commandWindow.active;
	const commandWindowActiveSymbol = this._commandWindow.currentSymbol();
	if(this._commandWindow.isHandled(data.symbol)) {
		if(data.symbol === 'cancel') {
			this._commandWindow.processCancel();
		} else {
			this._commandWindow.selectSymbol(data.symbol);
			this._commandWindow.processOk();
		}
	}
	if(!commandWindowActiveState && data.symbol !== 'cancel') {
		this._commandWindow.selectSymbol(commandWindowActiveSymbol);
		this._commandWindow.deactivate();
	}
};
//=============================================================================
// Window Equip Command
//-----------------------------------------------------------------------------
// Change commands in the command window
//=============================================================================
//-----------------------------------------------------------------------------
// Add original commands in original order if user wishes
//-----------------------------------------------------------------------------
const alias_CGMZ_EquipCommandWindow_makeCommandList = Window_EquipCommand.prototype.makeCommandList;
Window_EquipCommand.prototype.makeCommandList = function() {
	if(CGMZ.EquipCommandWindow.KeepOriginals) {
		alias_CGMZ_EquipCommandWindow_makeCommandList.call(this);
	}
	for(const cmd of CGMZ.EquipCommandWindow.Commands) {
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
Window_EquipCommand.prototype.maxCols = function() {
    return CGMZ.EquipCommandWindow.CommandColumns;
};
//-----------------------------------------------------------------------------
// Change alignment of command text
//-----------------------------------------------------------------------------
Window_EquipCommand.prototype.itemTextAlign = function() {
    return CGMZ.EquipCommandWindow.Alignment;
};
//-----------------------------------------------------------------------------
// Get the command icon
//-----------------------------------------------------------------------------
Window_EquipCommand.prototype.CGMZ_icon = function(index) {
	const ext = this._list[index]?.ext;
	if(!ext) return 0;
	const selected = (this._cgmz_lastIndex === index);
	return (selected && ext.selectedInfo?.icon) ? ext.selectedInfo.icon : ext.icon;
};
//-----------------------------------------------------------------------------
// Get the command name
//-----------------------------------------------------------------------------
const alias_CGMZ_EquipCommandWindow_EquipCommand_commandName = Window_EquipCommand.prototype.commandName;
Window_EquipCommand.prototype.commandName = function(index) {
	const selected = (this._cgmz_lastIndex === index);
	const ext = this._list[index]?.ext;
	const originalName = alias_CGMZ_EquipCommandWindow_EquipCommand_commandName.call(this, index);
	if(!selected || !ext) return originalName;
    return ext.selectedInfo?.name || originalName;
};
//-----------------------------------------------------------------------------
// Get selectable cgmz options
//-----------------------------------------------------------------------------
Window_EquipCommand.prototype.CGMZ_getSelectableCGMZOptions = function(index) {
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
Window_EquipCommand.prototype.drawItem = function(index) {
	const rect = this.itemLineRect(index);
	const align = this.itemTextAlign();
	const icon = this.CGMZ_icon(index);
	this.resetTextColor();
	this.changePaintOpacity(this.isCommandEnabled(index));
	if(icon) {
		const iconX = (CGMZ.EquipCommandWindow.IconAlignment === 'left') ? rect.x : rect.x + rect.width - ImageManager.iconWidth;
		this.drawIcon(icon, iconX, rect.y + 2);
		rect.x += (ImageManager.iconWidth + 2) * (CGMZ.EquipCommandWindow.IconAlignment === 'left');
		rect.width -= ImageManager.iconWidth + 2;
	}
	this.CGMZ_drawTextLine(this.commandName(index), rect.x, rect.y, rect.width, align);
};
//-----------------------------------------------------------------------------
// Redraw old index
//-----------------------------------------------------------------------------
Window_EquipCommand.prototype.CGMZ_handleSelectionChangePrevious = function(index) {
	if(this._list?.[index]) this.redrawItem(index);
};
//-----------------------------------------------------------------------------
// Redraw new index
//-----------------------------------------------------------------------------
Window_EquipCommand.prototype.CGMZ_handleSelectionChangeNext = function(index) {
	if(this._list?.[index]) this.redrawItem(index);
};