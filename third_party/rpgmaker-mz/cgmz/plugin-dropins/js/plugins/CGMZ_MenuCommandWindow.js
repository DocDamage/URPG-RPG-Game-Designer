/*:
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/menucommandwindow/
 * @target MZ
 * @base CGMZ_Core
 * @orderAfter CGMZ_Core
 * @plugindesc Manage the menu command window
 * @help
 * ============================================================================
 * For terms and conditions using this plugin in your game please visit:
 * https://www.caspergaming.com/terms-of-use/
 * ============================================================================
 * Become a Patron to get access to beta/alpha plugins plus other goodies!
 * https://www.patreon.com/CasperGamingRPGM
 * ============================================================================
 * Version: 1.5.2
 * ----------------------------------------------------------------------------
 * Compatibility: Only tested with my CGMZ plugins.
 * Made for RPG Maker MZ 1.10.0
 * ----------------------------------------------------------------------------
 * Description: Use this plugin to easily manage the command window in the
 * menu scene. It allows you to re-arrange commands or use JavaScript to 
 * add custom commands which are capable of calling custom plugin scenes or
 * functions.
 * ----------------------------------------------------------------------------
 * Documentation:
 * The command symbol should be unique and not blank for every command. This
 * symbol is how the plugin knows internally which JS code to run.
 *
 * Some Command Symbols can have special meanings, mainly
 * when they represent the original 8 commands.
 * The following symbols represent the original 8 commands (case sensitive):
 * item - Will handle like the original item command
 * skill - Will handle like the original skill command
 * equip - Will handle like the original equip command
 * status - Will handle like the original status command
 * formation - Will handle like the original formation command
 * options - Will handle like the original options command
 * save - Will handle like the original save command
 * gameEnd - will handle like the original game end command
 * 
 * It is important that you do not use these strings as the Command Symbol
 * property unless you mean to refer to the original commands.
 * 
 * If you set the parameter "Keep Original Commands" to true, the 8 original
 * commands will be untouched and custom commands will go where the makers of
 * RPG Maker MZ intended them to go in the list of menu items. This is the
 * beginner-friendly option.
 *
 * If you set the parameter "Keep Original Commands" to false, no commands will
 * be added by default and you will need to add any menu item you wish to use
 * even if it is one of the ones that come with the maker (such as the Item
 * command). However, with this option you have more control over where in the
 * list each entry appears and you can also easily hide or disable menu entries
 * with the switches associated with them.
 * 
 * Below you can find the default 8 commands which you can copy+paste into the
 * text part of the parameter setup if using this option. You can still change
 * the order, the command name, and modify switches to enable/disable and
 * hide/show the option.
 *
 * Item command:
 * {"Command Name":"Item","Icon":"0","Command Symbol":"item","JS Command":"","JS Show Condition":"return true;","JS Enable Condition":"return true;","Enable Switch":"0","Show Switch":"0","Subcategory":"","Required Item":"0","Actor Select":"false","Background Image":"","Background Image X":"0","Background Image Y":"0"}
 *
 * Skill command:
 * {"Command Name":"Skill","Icon":"0","Command Symbol":"skill","JS Command":"","JS Show Condition":"return true;","JS Enable Condition":"return true;","Enable Switch":"0","Show Switch":"0","Subcategory":"","Required Item":"0","Actor Select":"false","Background Image":"","Background Image X":"0","Background Image Y":"0"}
 *
 * Equip command:
 * {"Command Name":"Equip","Icon":"0","Command Symbol":"equip","JS Command":"","JS Show Condition":"return true;","JS Enable Condition":"return true;","Enable Switch":"0","Show Switch":"0","Subcategory":"","Required Item":"0","Actor Select":"false","Background Image":"","Background Image X":"0","Background Image Y":"0"}
 *
 * Status command:
 * {"Command Name":"Status","Icon":"0","Command Symbol":"status","JS Command":"","JS Show Condition":"return true;","JS Enable Condition":"return true;","Enable Switch":"0","Show Switch":"0","Subcategory":"","Required Item":"0","Actor Select":"false","Background Image":"","Background Image X":"0","Background Image Y":"0"}
 *
 * Formation command:
 * {"Command Name":"Formation","Icon":"0","Command Symbol":"formation","JS Command":"","JS Show Condition":"return true;","JS Enable Condition":"return true;","Enable Switch":"0","Show Switch":"0","Subcategory":"","Required Item":"0","Actor Select":"false","Background Image":"","Background Image X":"0","Background Image Y":"0"}
 *
 * Options command:
 * {"Command Name":"Options","Icon":"0","Command Symbol":"options","JS Command":"","JS Show Condition":"return true;","JS Enable Condition":"return true;","Enable Switch":"0","Show Switch":"0","Subcategory":"","Required Item":"0","Actor Select":"false","Background Image":"","Background Image X":"0","Background Image Y":"0"}
 *
 * Save command:
 * {"Command Name":"Save","Icon":"0","Command Symbol":"save","JS Command":"","JS Show Condition":"return true;","JS Enable Condition":"return true;","Enable Switch":"0","Show Switch":"0","Subcategory":"","Required Item":"0","Actor Select":"false","Background Image":"","Background Image X":"0","Background Image Y":"0"}
 *
 * Game End command:
 * {"Command Name":"Game End","Icon":"0","Command Symbol":"gameEnd","JS Command":"","JS Show Condition":"return true;","JS Enable Condition":"return true;","Enable Switch":"0","Show Switch":"0","Subcategory":"","Required Item":"0","Actor Select":"false","Background Image":"","Background Image X":"0","Background Image Y":"0"}
 * --------------------------Subcategories-------------------------------------
 * Subcategories work by assigning a subcategory to a command. You can have
 * as many commands in one subcategory as you want. These commands will only
 * display when that subcategory is active, assuming they meet other display
 * criteria such as the Show Switch.
 *
 * To have a command that activates that subcategory, use the following code
 * in the JS Command parameter:
 * this.CGMZ_pushCategory("mySubcategory");
 *
 * For example, if you had a subcategory named Collectibles, to show the
 * commands associated with that subcategory you would make a command with a
 * JS Command parameter set to:
 * this.CGMZ_pushCategory("Collectibles");
 *
 * By default, commands with no subcategory will display in the menu.
 * -------------------------Plugin Commands------------------------------------
 * This plugin does not have any plugin commands.
 * ---------------------------Saved Games--------------------------------------
 * This plugin fully supports saved games.
 * -----------------------------Filename---------------------------------------
 * The filename for this plugin MUST remain CGMZ_MenuCommandWindow.js
 * This is what it comes as when downloaded. The filename is used to load
 * parameters and execute plugin commands. If you change it, things will begin
 * behaving incorrectly and your game will probably crash. Please do not
 * rename the js file.
 * --------------------------Latest Version------------------------------------
 * Hi all, this latest version fixes a bug that could cause the menu
 * subcategory to not properly reset. It will now always reset if the map is
 * loaded, which should cover any case that was not already handled such as
 * other plugin scenes leading directly back to the map, to title commands,
 * etc.
 *
 * Version 1.5.2
 * - Fix bug with menu subcategory not always properly resetting
 *
 * @param Commands
 * @type struct<Handler>[]
 * @desc Command Name and associated js commands
 * @default []
 *
 * @param Command Padding
 * @type number
 * @default 8
 * @min 0
 * @desc The amount of padding inside the command rect
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
 * @desc The alignment of the icons for commands in the window
 *
 * @param Keep Original Commands
 * @type boolean
 * @default true
 * @desc Determine whether to show the original commands in their original order.
 *
 * @param Report Command Size
 * @type boolean
 * @default false
 * @desc In playtest, print command rect dimensions to console to get background image dimensions?
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
 * @param JS Show Condition
 * @type multiline_string
 * @desc JavaScript condition to show the command
 * @default return true;
 *
 * @param JS Enable Condition
 * @type multiline_string
 * @desc JavaScript condition to enable the command
 * @default return true;
 *
 * @param Enable Switch
 * @type switch
 * @default 0
 * @desc Turning this switch on will enable the command.
 *
 * @param Show Switch
 * @type switch
 * @default 0
 * @desc Turning this switch on will show the command.
 *
 * @param Subcategory
 * @desc The subcategory that needs to be active to show the command
 *
 * @param Required Item
 * @type item
 * @default 0
 * @desc Item that must be in the inventory
 *
 * @param Actor Select
 * @type boolean
 * @default false
 * @desc Set true if the command first requires the player to select an actor
 *
 * @param Common Event
 * @type common_event
 * @default 0
 * @desc If this is set, the command will run the given common event
 *
 * @param Exit After Common Event
 * @type boolean
 * @default true
 * @desc If true, the command will exit the menu if it has a common event set up
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
/*:es
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/menucommandwindow/
 * @target MZ
 * @base CGMZ_Core
 * @orderAfter CGMZ_Core
 * @plugindesc Administrar la ventana de comandos del menú
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
 * Versión: 1.5.2
 * ----------------------------------------------------------------------------
 * Compatibilidad: Sólo probado con mis CGMZ plugins.
 * Hecho para RPG Maker MZ 1.10.0
 * ----------------------------------------------------------------------------
 * Descripción: Usa este complemento para administrar fácilmente la ventana de 
 * comandos en la escena del menú. Le permite reorganizar los comandos o usar 
 * JavaScript para agregar comandos personalizados que son capaces de llamar a 
 * escenas o funciones de complementos personalizados.
 * ----------------------------------------------------------------------------
 * Documentación:
 * El símbolo de comando debe ser único y no estar en blanco para cada
 * comando. Este símbolo es cómo el complemento sabe internamente qué código
 * JS ejecutar.
 *
 * Algunos Símbolos de Comando pueden tener significados especiales,
 * principalmente cuando representan los 8 comandos originales.
 * Los siguientes símbolos representan los 8 comandos originales (se distingue 
 * entre mayúsculas y minúsculas):
 * item - Se manejará como el comando del elemento original
 * skill - Se manejará como el comando de habilidad original
 * equip - Se manejará como el comando de equipo original
 * status - Se manejará como el comando de estado original
 * formation - Se manejará como el comando de formación original
 * options - Se manejará como el comando de opciones original
 * save - Se manejará como el comando de guardado original
 * gameEnd - Se manejará como el comando de finalización del juego original
 * 
 * Es importante que no utilices estas cadenas como la propiedad de Símbolo de 
 * comando a menos que desee hacer referencia a los comandos originales.
 *
 * Si establece el parámetro "Mantener los comandos originales" en verdadero, 
 * los 8 comandos originales no se modificarán y los comandos personalizados irán
 * donde los creadores de RPG Maker MZ pretendía que estuvieran en la lista de 
 * elementos del menú. Esta es la opción para principiantes.
 *
 * Si configura el parámetro "Mantener los comandos originales" en falso, no se
 * agregarán comandos de forma predeterminada y deberá agregar cualquier elemento 
 * de menú que desee usar, incluso si es uno de los que vienen con el fabricante 
 * (como el comando Elemento). Sin embargo, con esta opción tiene más control sobre 
 * dónde aparece cada entrada en la lista y también puede ocultar o deshabilitar 
 * fácilmente las entradas del menú con los interruptores asociados a ellas.
 *
 * A continuación puede encontrar los 8 comandos predeterminados que puede copiar 
 * y pegar en la parte de texto de la configuración de parámetros si usa esta opción. 
 * Todavía puede cambiar el orden, el nombre del comando y modificar los 
 * interruptores para habilitar/deshabilitar y ocultar/mostrar la opción.
 *
 * Comando de artículo:
 * {"Command Name":"Item","Icon":"0","Command Symbol":"item","JS Command":"","JS Show Condition":"return true;","JS Enable Condition":"return true;","Enable Switch":"0","Show Switch":"0","Subcategory":"","Required Item":"0","Actor Select":"false","Background Image":"","Background Image X":"0","Background Image Y":"0"}
 *
 * Comando de habilidad:
 * {"Command Name":"Skill","Icon":"0","Command Symbol":"skill","JS Command":"","JS Show Condition":"return true;","JS Enable Condition":"return true;","Enable Switch":"0","Show Switch":"0","Subcategory":"","Required Item":"0","Actor Select":"false","Background Image":"","Background Image X":"0","Background Image Y":"0"}
 *
 * Comando Equipar:
 * {"Command Name":"Equip","Icon":"0","Command Symbol":"equip","JS Command":"","JS Show Condition":"return true;","JS Enable Condition":"return true;","Enable Switch":"0","Show Switch":"0","Subcategory":"","Required Item":"0","Actor Select":"false","Background Image":"","Background Image X":"0","Background Image Y":"0"}
 *
 * Comando de Estado:
 * {"Command Name":"Status","Icon":"0","Command Symbol":"status","JS Command":"","JS Show Condition":"return true;","JS Enable Condition":"return true;","Enable Switch":"0","Show Switch":"0","Subcategory":"","Required Item":"0","Actor Select":"false","Background Image":"","Background Image X":"0","Background Image Y":"0"}
 *
 * Comando de formación:
 * {"Command Name":"Formation","Icon":"0","Command Symbol":"formation","JS Command":"","JS Show Condition":"return true;","JS Enable Condition":"return true;","Enable Switch":"0","Show Switch":"0","Subcategory":"","Required Item":"0","Actor Select":"false","Background Image":"","Background Image X":"0","Background Image Y":"0"}
 *
 * Comando de opciones:
 * {"Command Name":"Options","Icon":"0","Command Symbol":"options","JS Command":"","JS Show Condition":"return true;","JS Enable Condition":"return true;","Enable Switch":"0","Show Switch":"0","Subcategory":"","Required Item":"0","Actor Select":"false","Background Image":"","Background Image X":"0","Background Image Y":"0"}
 *
 * Guardar comando:
 * {"Command Name":"Save","Icon":"0","Command Symbol":"save","JS Command":"","JS Show Condition":"return true;","JS Enable Condition":"return true;","Enable Switch":"0","Show Switch":"0","Subcategory":"","Required Item":"0","Actor Select":"false","Background Image":"","Background Image X":"0","Background Image Y":"0"}
 *
 * Comando Fin del juego:
 * {"Command Name":"Game End","Icon":"0","Command Symbol":"gameEnd","JS Command":"","JS Show Condition":"return true;","JS Enable Condition":"return true;","Enable Switch":"0","Show Switch":"0","Subcategory":"","Required Item":"0","Actor Select":"false","Background Image":"","Background Image X":"0","Background Image Y":"0"}
 * --------------------------Subcategories-------------------------------------
 * Subcategories work by assigning a subcategory to a command. You can have
 * as many commands in one subcategory as you want. These commands will only
 * display when that subcategory is active, assuming they meet other display
 * criteria such as the Show Switch.
 *
 * To have a command that activates that subcategory, use the following code
 * in the JS Command parameter:
 * this.CGMZ_pushCategory("mySubcategory");
 *
 * For example, if you had a subcategory named Collectibles, to show the
 * commands associated with that subcategory you would make a command with a
 * JS Command parameter set to:
 * this.CGMZ_pushCategory("Collectibles");
 *
 * By default, commands with no subcategory will display in the menu.
 * ----------------------- Comandos de Plugin----------------------------------
 * Este plugin no tiene ningún comando de plugin.
 * --------------------------Juegos guardados----------------------------------
 * Este plugin es totalmente compatible con los juegos guardados.
 * -------------------------Nombre del archivo---------------------------------
 * El nombre de archivo de este complemento DEBE seguir siendo
 * CGMZ_MenuCommandWindow.js. Esto es lo que se obtiene cuando se descarga. El
 * nombre de archivo se usa para cargar parámetros y ejecutar comandos de
 * complemento. Si lo cambias, las cosas comenzarán a comportarse
 * incorrectamente y tu juego probablemente fallará. No cambies el nombre del
 * archivo js.
 * --------------------------Latest Version------------------------------------
 * Hi all, this latest version fixes a bug that could cause the menu
 * subcategory to not properly reset. It will now always reset if the map is
 * loaded, which should cover any case that was not already handled such as
 * other plugin scenes leading directly back to the map, to title commands,
 * etc.
 *
 * Version 1.5.2
 * - Fix bug with menu subcategory not always properly resetting
 *
 * @param Commands
 * @text Comandos
 * @type struct<Handler>[]
 * @desc Nombre del comando y comandos js asociados.
 * @default []
 *
 * @param Command Padding
 * @type number
 * @default 8
 * @min 0
 * @desc The amount of padding inside the command rect
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
 * @text Alineación de icono
 * @type select
 * @option left
 * @option right
 * @default left
 * @desc La alineación del icono del comando en la ventana.
 *
 * @param Keep Original Commands
 * @text Mantener los comandos originales
 * @type boolean
 * @default true
 * @desc Determina si desea mostrar los comandos originales en su orden original.
 *
 * @param Report Command Size
 * @type boolean
 * @default false
 * @desc In playtest, print command rect dimensions to console to get background image dimensions?
*/
/*~struct~Handler:es
 * @param Command Name
 * @text Nombre de Comando
 * @desc Nombre del comando que se mostrará en la ventana de comandos.
 *
 * @param Icon
 * @type icon
 * @default 0
 * @desc An icon to show for the command, if 0 will not show any icon
 *
 * @param Command Symbol
 * @text Símbolo de Comando
 * @desc Este símbolo se usa internamente para reconocer el comando.
 * Special meaning for original commands (see documentation).
 *
 * @param JS Command
 * @text Comando JS
 * @type multiline_string
 * @desc JavaScript para ejecutar cuando se selecciona el comando.
 *
 * @param JS Show Condition
 * @type multiline_string
 * @desc JavaScript condition to show the command
 * @default return true;
 *
 * @param JS Enable Condition
 * @type multiline_string
 * @desc JavaScript condition to enable the command
 * @default return true;
 *
 * @param Subcategory
 * @desc The subcategory that needs to be active to show the command
 *
 * @param Enable Switch
 * @text Habilitar interruptor
 * @type switch
 * @default 0
 * @desc Encender este interruptor habilitará el comando.
 *
 * @param Show Switch
 * @text Mostrar interruptor
 * @type switch
 * @default 0
 * @desc Al encender este interruptor se mostrará el comando.
 *
 * @param Required Item
 * @text Elemento requerido
 * @type item
 * @default 0
 * @desc Artículo que debe estar en el inventario.
 *
 * @param Actor Select
 * @type boolean
 * @default false
 * @desc Set true if the command first requires the player to select an actor
 *
 * @param Common Event
 * @type common_event
 * @default 0
 * @desc If this is set, the command will run the common event
 *
 * @param Exit After Common Event
 * @type boolean
 * @default true
 * @desc If true, the command will exit the menu if it has a common event set up
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
Imported.CGMZ_Menu_CommandWindow = true;
CGMZ.Versions["Menu Command Window"] = "1.5.2";
CGMZ.Menu_CommandWindow = {};
CGMZ.Menu_CommandWindow.parameters = PluginManager.parameters('CGMZ_MenuCommandWindow');
CGMZ.Menu_CommandWindow.Alignment = CGMZ.Menu_CommandWindow.parameters["Alignment"];
CGMZ.Menu_CommandWindow.IconAlignment = CGMZ.Menu_CommandWindow.parameters["Icon Alignment"];
CGMZ.Menu_CommandWindow.CommandPadding = Number(CGMZ.Menu_CommandWindow.parameters["Command Padding"]);
CGMZ.Menu_CommandWindow.KeepOriginals = (CGMZ.Menu_CommandWindow.parameters["Keep Original Commands"] === "true");
CGMZ.Menu_CommandWindow.ReportCommandSize = (CGMZ.Menu_CommandWindow.parameters["Report Command Size"] === "true");
CGMZ.Menu_CommandWindow.Commands = CGMZ_Utils.parseJSON(CGMZ.Menu_CommandWindow.parameters["Commands"], [], "[CGMZ] Menu Command Window", "Your Commands parameter had invalid JSON and could not be read").map(commandJSON => {
	const cmd = CGMZ_Utils.parseJSON(commandJSON, null, "[CGMZ] Menu Command Window", "One of your commands had invalid JSON and could not be parsed");
	if(!cmd) return null;
	return {
		enableSwitch: Number(cmd["Enable Switch"]),
		showSwitch: Number(cmd["Show Switch"]),
		reqItem: Number(cmd["Required Item"]),
		icon: Number(cmd.Icon),
		commonEvent: Number(cmd["Common Event"]),
		exitAfterCommonEvent: (cmd["Exit After Common Event"] === 'true'),
		actorSelect: (cmd["Actor Select"] === 'true'),
		symbol: cmd["Command Symbol"] || Math.random().toString(36),
		name: cmd["Command Name"],
		subcategory: cmd.Subcategory,
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
				y: Number(cmd["Selected Back Img Y"]) || Number(cmd["Background Image Y"])
			}
		}
	};
}).filter(x => !!x);
//=============================================================================
// Scene Menu
//-----------------------------------------------------------------------------
// Handling for command window entries
//=============================================================================
//-----------------------------------------------------------------------------
// Handling for custom Commands added through the plugin
//-----------------------------------------------------------------------------
Scene_Menu.prototype.CGMZ_MenuCommand_commandCustom = function() {
	for(const cmd of CGMZ.Menu_CommandWindow.Commands) {
		if(this._commandWindow.currentSymbol() === cmd.symbol) {
			try {
				const func = new Function(cmd.js);
				func.call(this);
			}
			catch (e) {
				const origin = "[CGMZ] Menu Command Window";
				const suggestion = "Check your JavaScript command";
				CGMZ_Utils.reportError(e.message, origin, suggestion);
			}
			break;
		}
	}
};
//-----------------------------------------------------------------------------
// Handling for common event commands
//-----------------------------------------------------------------------------
Scene_Menu.prototype.CGMZ_MenuCommand_commandCommonEvent = function(id, needsExit) {
	$gameTemp.reserveCommonEvent(id);
	if(needsExit) {
		this.popScene();
	} else {
		this._commandWindow.activate();
	}
};
//-----------------------------------------------------------------------------
// Add additional commands.
//-----------------------------------------------------------------------------
const alias_CGMZ_MenuCommandWindow_createCommandWindow = Scene_Menu.prototype.createCommandWindow;
Scene_Menu.prototype.createCommandWindow = function() {
	alias_CGMZ_MenuCommandWindow_createCommandWindow.call(this);
	this._commandWindow.setHandler('cancel', this.CGMZ_popCommandCategory.bind(this));
	for(const cmd of CGMZ.Menu_CommandWindow.Commands) {
		if(this.CGMZ_MenuCommandWindow_isCustomCommand(cmd.symbol)) {
			if(cmd.actorSelect) {
				this._commandWindow.setHandler(cmd.symbol, this.commandPersonal.bind(this));
			} else if(cmd.commonEvent) {
				this._commandWindow.setHandler(cmd.symbol, this.CGMZ_MenuCommand_commandCommonEvent.bind(this, cmd.commonEvent, cmd.exitAfterCommonEvent));
			} else {
				this._commandWindow.setHandler(cmd.symbol, this.CGMZ_MenuCommand_commandCustom.bind(this));
			}
		}
	}
};
//-----------------------------------------------------------------------------
// Handling for cancel - first try to pop command window category
//-----------------------------------------------------------------------------
Scene_Menu.prototype.CGMZ_popCommandCategory = function() {
	if(!this._commandWindow.CGMZ_popCategory()) this.popScene();
};
//-----------------------------------------------------------------------------
// Method to push a new category to the menu command window
//-----------------------------------------------------------------------------
Scene_Menu.prototype.CGMZ_pushCategory = function(category) {
	this._commandWindow.CGMZ_pushCategory(category);
};
//-----------------------------------------------------------------------------
// Handling for custom commands after actor select
//-----------------------------------------------------------------------------
const alias_CGMZ_MenuCommandWindow_onPersonalOk = Scene_Menu.prototype.onPersonalOk;
Scene_Menu.prototype.onPersonalOk = function() {
	alias_CGMZ_MenuCommandWindow_onPersonalOk.call(this);
	const defaultPersonalCommands = ['skill','equip','status'];
	if(defaultPersonalCommands.includes(this._commandWindow.currentSymbol())) return;
	this.CGMZ_MenuCommand_commandCustom();
};
//-----------------------------------------------------------------------------
// Determine if command is a custom command in need of custom handler
//-----------------------------------------------------------------------------
Scene_Menu.prototype.CGMZ_MenuCommandWindow_isCustomCommand = function(symbol) {
	if(symbol === 'item' || symbol === 'skill' || symbol === 'equip' || symbol === 'status' ||
	symbol === 'formation' || symbol === 'options' || symbol === 'save' || symbol === 'gameEnd') {
		return false;
	}
	return true;
};
//=============================================================================
// Window MenuCommand
//-----------------------------------------------------------------------------
// Change amount of commands displayed at once and add new original commands
//=============================================================================
Window_MenuCommand.CGMZ_lastSubcategory = [""];
Window_MenuCommand.CGMZ_lastSubcategorySymbol = [];
//-----------------------------------------------------------------------------
// Initialize window
//-----------------------------------------------------------------------------
const alias_CGMZ_MenuCommandWindow_initialize = Window_MenuCommand.prototype.initialize;
Window_MenuCommand.prototype.initialize = function(rect) {
	alias_CGMZ_MenuCommandWindow_initialize.call(this, rect);
	this._CGMZ_category = Window_MenuCommand.CGMZ_lastSubcategory[Window_MenuCommand.CGMZ_lastSubcategory.length - 1];
	if($gameTemp.isPlaytest() && CGMZ.Menu_CommandWindow.ReportCommandSize) {
		const rect = this.itemRect(0);
		CGMZ_Utils.reportDimensions(rect.width, rect.height, '[CGMZ] Menu Command Window - Command Rect Dimensions');
	}
};
//-----------------------------------------------------------------------------
// Push a new category
//-----------------------------------------------------------------------------
Window_MenuCommand.prototype.CGMZ_pushCategory = function(newCategory) {
	if(typeof newCategory !== "string") {
		CGMZ_Utils.reportError("Category not of type string", "[CGMZ] Menu Command Window", "Your JS command for a new category must include quotes around the subcategory name");
		this.activate();
		return;
	}
	Window_MenuCommand.CGMZ_lastSubcategorySymbol.push(this.currentSymbol());
	Window_MenuCommand.CGMZ_lastSubcategory.push(newCategory);
	this.refresh();
	this.smoothSelect(0);
	this.activate();
};
//-----------------------------------------------------------------------------
// Pop a category. Returns true if could pop, false if could not pop
//-----------------------------------------------------------------------------
Window_MenuCommand.prototype.CGMZ_popCategory = function() {
	if(Window_MenuCommand.CGMZ_lastSubcategory.length <= 1) return false;
	Window_MenuCommand.CGMZ_lastSubcategory.pop();
	this.refresh();
	const lastSymbol = (Window_MenuCommand.CGMZ_lastSubcategorySymbol.length > 0) ? Window_MenuCommand.CGMZ_lastSubcategorySymbol.pop() : "";
	const symbolIndex = this.findSymbol(lastSymbol);
	this.smoothSelect((symbolIndex >= 0) ? symbolIndex : 0);
	this.activate();
	return true;
};
//-----------------------------------------------------------------------------
// Add original commands.
//-----------------------------------------------------------------------------
const alias_CGMZ_MenuCommandWindow_addOriginalCommands = Window_MenuCommand.prototype.addOriginalCommands;
Window_MenuCommand.prototype.addOriginalCommands = function() {
	alias_CGMZ_MenuCommandWindow_addOriginalCommands.call(this);
	for(const cmd of CGMZ.Menu_CommandWindow.Commands) {
		if(this.CGMZ_MenuCommandWindow_needsCommand(cmd)) {
			const enabled = this.CGMZ_MenuCommandWindow_getEnabledStatus(cmd);
			this.addCommand(cmd.name, cmd.symbol, enabled, {icon: cmd.icon, img: cmd.backImg, selectedInfo: cmd.selectedInfo});
		}
	}
};
//-----------------------------------------------------------------------------
// Determine if Command should show
//-----------------------------------------------------------------------------
Window_MenuCommand.prototype.CGMZ_MenuCommandWindow_needsCommand = function(cmd) {
	if(cmd.showSwitch > 0 && !$gameSwitches.value(cmd.showSwitch)) return false;
	if(cmd.subcategory !== Window_MenuCommand.CGMZ_lastSubcategory[Window_MenuCommand.CGMZ_lastSubcategory.length - 1]) return false;
	const showFunc = new Function(cmd.jsShow);
	if(!showFunc.call(this)) return false;
	return this.needsCommand(cmd.symbol);
};
//-----------------------------------------------------------------------------
// Determine if Command should show
//-----------------------------------------------------------------------------
Window_MenuCommand.prototype.CGMZ_MenuCommandWindow_getEnabledStatus = function(cmd) {
	const switchId = cmd.enableSwitch;
	if(switchId > 0 && !$gameSwitches.value(switchId)) return false;
	const itemId = cmd.reqItem;
	if(itemId > 0 && !$gameParty.hasItem($dataItems[itemId], false)) return false;
	switch(cmd.symbol) {
		case 'item':
		case 'skill':
		case 'equip':
		case 'status': return this.areMainCommandsEnabled();
		case 'formation': return this.isFormationEnabled();
		case 'options': return this.isOptionsEnabled();
		case 'save': return this.isSaveEnabled();
		case 'gameEnd': return this.isGameEndEnabled();
	}
	const enableFunc = new Function(cmd.jsEnable);
	return enableFunc.call(this);
};
//-----------------------------------------------------------------------------
// Add main commands only if original commands should not be touched
//-----------------------------------------------------------------------------
const alias_CGMZ_MenuCommandWindow_addMainCommands = Window_MenuCommand.prototype.addMainCommands;
Window_MenuCommand.prototype.addMainCommands = function() {
	if(CGMZ.Menu_CommandWindow.KeepOriginals) {
		alias_CGMZ_MenuCommandWindow_addMainCommands.call(this);
	}
};
//-----------------------------------------------------------------------------
// Add formation command only if original commands should not be touched
//-----------------------------------------------------------------------------
const alias_CGMZ_MenuCommandWindow_addFormationCommand = Window_MenuCommand.prototype.addFormationCommand;
Window_MenuCommand.prototype.addFormationCommand = function() {
	if(CGMZ.Menu_CommandWindow.KeepOriginals) {
		alias_CGMZ_MenuCommandWindow_addFormationCommand.call(this);
	}
};
//-----------------------------------------------------------------------------
// Add options command only if original commands should not be touched
//-----------------------------------------------------------------------------
const alias_CGMZ_MenuCommandWindow_addOptionsCommand = Window_MenuCommand.prototype.addOptionsCommand;
Window_MenuCommand.prototype.addOptionsCommand = function() {
	if(CGMZ.Menu_CommandWindow.KeepOriginals) {
		alias_CGMZ_MenuCommandWindow_addOptionsCommand.call(this);
	}
};
//-----------------------------------------------------------------------------
// Add save command only if original commands should not be touched
//-----------------------------------------------------------------------------
const alias_CGMZ_MenuCommandWindow_addSaveCommand = Window_MenuCommand.prototype.addSaveCommand;
Window_MenuCommand.prototype.addSaveCommand = function() {
	if(CGMZ.Menu_CommandWindow.KeepOriginals) {
		alias_CGMZ_MenuCommandWindow_addSaveCommand.call(this);
	}
};
//-----------------------------------------------------------------------------
// Add game end command only if original commands should not be touched
//-----------------------------------------------------------------------------
const alias_CGMZ_MenuCommandWindow_addGameEndCommand = Window_MenuCommand.prototype.addGameEndCommand;
Window_MenuCommand.prototype.addGameEndCommand = function() {
	if(CGMZ.Menu_CommandWindow.KeepOriginals) {
		alias_CGMZ_MenuCommandWindow_addGameEndCommand.call(this);
	}
};
//-----------------------------------------------------------------------------
// Change alignment of command text
//-----------------------------------------------------------------------------
Window_MenuCommand.prototype.itemTextAlign = function() {
	return CGMZ.Menu_CommandWindow.Alignment;
};
//-----------------------------------------------------------------------------
// Change padding of the command rect
//-----------------------------------------------------------------------------
Window_MenuCommand.prototype.itemPadding = function() {
	return CGMZ.Menu_CommandWindow.CommandPadding;
};
//-----------------------------------------------------------------------------
// Get the command icon
//-----------------------------------------------------------------------------
Window_MenuCommand.prototype.CGMZ_icon = function(index) {
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
const alias_CGMZ_MenuCommandWindow_MenuCommand_commandName = Window_MenuCommand.prototype.commandName;
Window_MenuCommand.prototype.commandName = function(index) {
	const selected = (this._cgmz_lastIndex === index);
	const ext = this._list[index]?.ext;
	const originalName = alias_CGMZ_MenuCommandWindow_MenuCommand_commandName.call(this, index);
	if(!selected || !ext) return originalName;
	return ext.selectedInfo?.name || originalName;
};
//-----------------------------------------------------------------------------
// Allow use of text codes in command
//-----------------------------------------------------------------------------
Window_MenuCommand.prototype.drawItem = function(index) {
	const rect = this.itemLineRect(index);
	const align = this.itemTextAlign();
	const icon = this.CGMZ_icon(index);
	this.resetTextColor();
	this.changePaintOpacity(this.isCommandEnabled(index));
	if(icon) {
		const iconX = (CGMZ.Menu_CommandWindow.IconAlignment === 'left') ? rect.x : rect.x + rect.width - ImageManager.iconWidth;
		this.drawIcon(this.CGMZ_icon(index), iconX, rect.y + 2);
		rect.x += ImageManager.iconWidth * (CGMZ.Menu_CommandWindow.IconAlignment === 'left');
		rect.width -= ImageManager.iconWidth;
	}
	this.CGMZ_drawTextLine(this.commandName(index), rect.x, rect.y, rect.width, align);
};
//-----------------------------------------------------------------------------
// Get selectable cgmz options
//-----------------------------------------------------------------------------
Window_MenuCommand.prototype.CGMZ_getSelectableCGMZOptions = function(index) {
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
Window_MenuCommand.prototype.CGMZ_handleSelectionChangePrevious = function(index) {
	if(this._list?.[index]) this.redrawItem(index);
};
//-----------------------------------------------------------------------------
// Redraw new index
//-----------------------------------------------------------------------------
Window_MenuCommand.prototype.CGMZ_handleSelectionChangeNext = function(index) {
	if(this._list?.[index]) this.redrawItem(index);
};
//=============================================================================
// Scene_Map
//-----------------------------------------------------------------------------
// Reset menu subcategory whenever map loaded
//=============================================================================
//-----------------------------------------------------------------------------
// Also reset manu subcategory
//-----------------------------------------------------------------------------
const alias_CGMZMenuCommandWindow_SceneMap_initialize = Scene_Map.prototype.initialize;
Scene_Map.prototype.initialize = function() {
	alias_CGMZMenuCommandWindow_SceneMap_initialize.call(this);
	Window_MenuCommand.CGMZ_lastSubcategory = [""];
	Window_MenuCommand.CGMZ_lastSubcategorySymbol = [];
};