/*:
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/infinitecolors/
 * @target MZ
 * @plugindesc Define your own colors for use in messages
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
 * Description: This plugin allows you to define your own colors for use in
 * messages or anywhere else they are supported. It uses the same color escape
 * code \c[x] where x is the id of the color you want to access.
 * ----------------------------------------------------------------------------
 * Documentation:
 * The name attribute has no function in the plugin. It is simply there to
 * help you remember what the color is.
 *
 * Colors support hex format. You can google any hex color picker and it will
 * be the code shown with a # before it. For example, #ffffff is white.
 *
 * Colors support rgb format. Most color pickers provide red, blue, green
 * values which you will put in the form rgb(x, y, z) where x, y, and z are
 * the amounts of red, blue, and green the color has.
 *
 * The color ID provided is what number you type in the \c[x] code to switch
 * color. It begins at 32 since there are 31 colors available by default.
 * This plugin preserves the original 31 colors.
 * ---------------------------Color Parameters---------------------------------
 * When selecting a color for a color parameter in a plugin, you might notice
 * that you are limited by default to the windowskin colors. You can get
 * around this by swapping to the Text tab when setting the color and typing in
 * your custom color index.
 *
 * Give it a try in some of the System Colors parameters in this plugin!
 * ------------------------------Saved Games-----------------------------------
 * This plugin is fully compatible with saved games. This means you can:
 *
 * ✓ Add this plugin to a saved game and it will work as expected
 * ✓ Change any plugin params and changes will be reflected in saved games
 * ✓ Remove the plugin with no issue to save data
 * -----------------------------Filename---------------------------------------
 * The filename for this plugin MUST remain CGMZ_InfiniteColors.js
 * This is what it comes as when downloaded. The filename is used to load
 * parameters and execute plugin commands. If you change it, things will begin
 * behaving incorrectly and your game will probably crash. Please do not
 * rename the js file.
 * --------------------------Latest Version------------------------------------
 * Hi all, this latest version adds a way for you to change which color index
 * is used for various colors throughout the game. Previously, these were all
 * hard coded by the engine and would require you to change the windowskin to
 * change the color or in some cases were not possible to change at all.
 *
 * When selecting a color for a color parameter in a plugin, you might notice
 * that you are limited by default to the windowskin colors. You can get
 * around this by swapping to the Text tab when setting the color and typing in
 * your custom color index.
 *
 * Version 1.1.0
 * - Added options to change hard coded game colors
 *
 * @param Color Options
 *
 * @param Colors
 * @parent Color Options
 * @type struct<Color>[]
 * @desc Set up additional colors here
 * @default []
 *
 * @param System Colors
 *
 * @param Normal Color
 * @parent System Colors
 * @type color
 * @desc The normal color. 0 = no effect
 * @default 0
 *
 * @param System Color
 * @parent System Colors
 * @type color
 * @desc The system color. 0 = no effect
 * @default 0
 *
 * @param Crisis Color
 * @parent System Colors
 * @type color
 * @desc The crisis color. 0 = no effect
 * @default 0
 *
 * @param Death Color
 * @parent System Colors
 * @type color
 * @desc The death color. 0 = no effect
 * @default 0
 *
 * @param Gauge Back Color
 * @parent System Colors
 * @type color
 * @desc The gauge back color. 0 = no effect
 * @default 0
 *
 * @param HP Gauge Color 1
 * @parent System Colors
 * @type color
 * @desc The first color in the hp gauge gradient. 0 = no effect
 * @default 0
 *
 * @param HP Gauge Color 2
 * @parent System Colors
 * @type color
 * @desc The second color in the hp gauge gradient. 0 = no effect
 * @default 0
 *
 * @param HP Damage Color
 * @parent System Colors
 * @type color
 * @desc The HP damage color. 0 = no effect
 * @default 0
 *
 * @param HP Recover Color
 * @parent System Colors
 * @type color
 * @desc The HP recover color. 0 = no effect
 * @default 0
 *
 * @param MP Gauge Color 1
 * @parent System Colors
 * @type color
 * @desc The first color in the mp gauge gradient. 0 = no effect
 * @default 0
 *
 * @param MP Gauge Color 2
 * @parent System Colors
 * @type color
 * @desc The second color in the mp gauge gradient. 0 = no effect
 * @default 0
 *
 * @param MP Cost Color
 * @parent System Colors
 * @type color
 * @desc The MP cost color. 0 = no effect
 * @default 0
 *
 * @param MP Color
 * @parent System Colors
 * @type color
 * @desc The MP color. 0 = no effect
 * @default 0
 *
 * @param MP Damage Color
 * @parent System Colors
 * @type color
 * @desc The MP damage color. 0 = no effect
 * @default 0
 *
 * @param MP Recover Color
 * @parent System Colors
 * @type color
 * @desc The MP recover color. 0 = no effect
 * @default 0
 *
 * @param Power Up Color
 * @parent System Colors
 * @type color
 * @desc The power up color. 0 = no effect
 * @default 0
 *
 * @param Power Down Color
 * @parent System Colors
 * @type color
 * @desc The power down color. 0 = no effect
 * @default 0
 *
 * @param CT Gauge Color 1
 * @parent System Colors
 * @type color
 * @desc The first color in the ct gauge gradient. 0 = no effect
 * @default 0
 *
 * @param CT Gauge Color 2
 * @parent System Colors
 * @type color
 * @desc The second color in the ct gauge gradient. 0 = no effect
 * @default 0
 *
 * @param TP Gauge Color 1
 * @parent System Colors
 * @type color
 * @desc The first color in the tp gauge gradient. 0 = no effect
 * @default 0
 *
 * @param TP Gauge Color 2
 * @parent System Colors
 * @type color
 * @desc The second color in the tp gauge gradient. 0 = no effect
 * @default 0
 *
 * @param TP Cost Color
 * @parent System Colors
 * @type color
 * @desc The TP cost color. 0 = no effect
 * @default 0
 *
 * @param TP Color
 * @parent System Colors
 * @type color
 * @desc The TP color. 0 = no effect
 * @default 0
 *
 * @param Outline Color
 * @parent System Colors
 * @type color
 * @desc The outline color. 0 = no effect
 * @default 0
 *
 * @param Dim Color 1
 * @parent System Colors
 * @type color
 * @desc The first dim color. 0 = no effect
 * @default 0
 *
 * @param Dim Color 2
 * @parent System Colors
 * @type color
 * @desc The first dim color. 0 = no effect
 * @default 0
 *
 * @param Item Back Color 1
 * @parent System Colors
 * @type color
 * @desc The first item back color. 0 = no effect
 * @default 0
 *
 * @param Item Back Color 2
 * @parent System Colors
 * @type color
 * @desc The first item back color. 0 = no effect
 * @default 0
*/
/*~struct~Color:
 * @param Name
 * @desc Give a name to your color to more easily remember it.
 *
 * @param Color Value
 * @default #ffffff
 * @desc The color value of the color you want. Supports Hex and rgb. Hex format: #ffffff, RGB format: rgb(255, 255, 255)
 *
 * @param ID
 * @type number
 * @min 32
 * @default 32
 * @desc the ID of the color. This will be what you type for x when typing \c[x] into a message
*/
/*:es
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/infinitecolors/
 * @target MZ
 * @plugindesc Define tus propios colores para usar en los mensajes
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
 * Descripción: Este plugin le permite definir sus propios colores para usar en 
 * mensajes o en cualquier otro lugar donde sean compatibles. Utiliza el mismo 
 * código de escape de color \c[x] donde x es la identificación del color al
 * que desea acceder.
 * ----------------------------------------------------------------------------
 * Documentación:
 * El atributo de nombre no tiene ninguna función en el plugin. Simplemente
 * está ahí para ayudarte a recordar cuál es el color.
 *
 * Los colores admiten el formato hexadecimal. Puede buscar en Google cualquier 
 * selector de color hexadecimal y será el código que se muestra con un #  
 * antes. Por ejemplo, #ffffff es blanco.
 *
 * Los colores admiten el formato rgb. La mayoría de los selectores de color 
 * proporcionan valores de rojo, azul y verde que pondrás en la forma rgb(x, 
 * y, z) donde x, y y z son las cantidades de rojo, azul y verde que tiene el
 * color.
 *
 * La identificación de color proporcionada es el número que ingresa en el  
 * código \c[x] para cambiar de color. Comienza en 32 ya que hay 31 colores 
 * disponibles por defecto. Este plugin conserva los 31 colores originales.
 * ---------------------------Color Parameters---------------------------------
 * When selecting a color for a color parameter in a plugin, you might notice
 * that you are limited by default to the windowskin colors. You can get
 * around this by swapping to the Text tab when setting the color and typing in
 * your custom color index.
 *
 * Give it a try in some of the System Colors parameters in this plugin!
 * ------------------------------Saved Games-----------------------------------
 * This plugin is fully compatible with saved games. This means you can:
 *
 * ✓ Add this plugin to a saved game and it will work as expected
 * ✓ Change any plugin params and changes will be reflected in saved games
 * ✓ Remove the plugin with no issue to save data
 * -----------------------------Filename---------------------------------------
 * The filename for this plugin MUST remain CGMZ_InfiniteColors.js
 * This is what it comes as when downloaded. The filename is used to load
 * parameters and execute plugin commands. If you change it, things will begin
 * behaving incorrectly and your game will probably crash. Please do not
 * rename the js file.
 * --------------------------Latest Version------------------------------------
 * Hi all, this latest version adds a way for you to change which color index
 * is used for various colors throughout the game. Previously, these were all
 * hard coded by the engine and would require you to change the windowskin to
 * change the color or in some cases were not possible to change at all.
 *
 * When selecting a color for a color parameter in a plugin, you might notice
 * that you are limited by default to the windowskin colors. You can get
 * around this by swapping to the Text tab when setting the color and typing in
 * your custom color index.
 *
 * Version 1.1.0
 * - Added options to change hard coded game colors
 *
 * @param Color Options
 * @text Opciones de color
 *
 * @param Colors
 * @text Colores
 * @parent Color Options
 * @type struct<Color>[]
 * @desc Configura colores adicionales aquí.
 * @default []
 *
 * @param System Colors
 *
 * @param Normal Color
 * @parent System Colors
 * @type color
 * @desc The normal color. 0 = no effect
 * @default 0
 *
 * @param System Color
 * @parent System Colors
 * @type color
 * @desc The system color. 0 = no effect
 * @default 0
 *
 * @param Crisis Color
 * @parent System Colors
 * @type color
 * @desc The crisis color. 0 = no effect
 * @default 0
 *
 * @param Death Color
 * @parent System Colors
 * @type color
 * @desc The death color. 0 = no effect
 * @default 0
 *
 * @param Gauge Back Color
 * @parent System Colors
 * @type color
 * @desc The gauge back color. 0 = no effect
 * @default 0
 *
 * @param HP Gauge Color 1
 * @parent System Colors
 * @type color
 * @desc The first color in the hp gauge gradient. 0 = no effect
 * @default 0
 *
 * @param HP Gauge Color 2
 * @parent System Colors
 * @type color
 * @desc The second color in the hp gauge gradient. 0 = no effect
 * @default 0
 *
 * @param HP Damage Color
 * @parent System Colors
 * @type color
 * @desc The HP damage color. 0 = no effect
 * @default 0
 *
 * @param HP Recover Color
 * @parent System Colors
 * @type color
 * @desc The HP recover color. 0 = no effect
 * @default 0
 *
 * @param MP Gauge Color 1
 * @parent System Colors
 * @type color
 * @desc The first color in the mp gauge gradient. 0 = no effect
 * @default 0
 *
 * @param MP Gauge Color 2
 * @parent System Colors
 * @type color
 * @desc The second color in the mp gauge gradient. 0 = no effect
 * @default 0
 *
 * @param MP Cost Color
 * @parent System Colors
 * @type color
 * @desc The MP cost color. 0 = no effect
 * @default 0
 *
 * @param MP Color
 * @parent System Colors
 * @type color
 * @desc The MP color. 0 = no effect
 * @default 0
 *
 * @param MP Damage Color
 * @parent System Colors
 * @type color
 * @desc The MP damage color. 0 = no effect
 * @default 0
 *
 * @param MP Recover Color
 * @parent System Colors
 * @type color
 * @desc The MP recover color. 0 = no effect
 * @default 0
 *
 * @param Power Up Color
 * @parent System Colors
 * @type color
 * @desc The power up color. 0 = no effect
 * @default 0
 *
 * @param Power Down Color
 * @parent System Colors
 * @type color
 * @desc The power down color. 0 = no effect
 * @default 0
 *
 * @param CT Gauge Color 1
 * @parent System Colors
 * @type color
 * @desc The first color in the ct gauge gradient. 0 = no effect
 * @default 0
 *
 * @param CT Gauge Color 2
 * @parent System Colors
 * @type color
 * @desc The second color in the ct gauge gradient. 0 = no effect
 * @default 0
 *
 * @param TP Gauge Color 1
 * @parent System Colors
 * @type color
 * @desc The first color in the tp gauge gradient. 0 = no effect
 * @default 0
 *
 * @param TP Gauge Color 2
 * @parent System Colors
 * @type color
 * @desc The second color in the tp gauge gradient. 0 = no effect
 * @default 0
 *
 * @param TP Cost Color
 * @parent System Colors
 * @type color
 * @desc The TP cost color. 0 = no effect
 * @default 0
 *
 * @param TP Color
 * @parent System Colors
 * @type color
 * @desc The TP color. 0 = no effect
 * @default 0
 *
 * @param Outline Color
 * @parent System Colors
 * @type color
 * @desc The outline color. 0 = no effect
 * @default 0
 *
 * @param Dim Color 1
 * @parent System Colors
 * @type color
 * @desc The first dim color. 0 = no effect
 * @default 0
 *
 * @param Dim Color 2
 * @parent System Colors
 * @type color
 * @desc The first dim color. 0 = no effect
 * @default 0
 *
 * @param Item Back Color 1
 * @parent System Colors
 * @type color
 * @desc The first item back color. 0 = no effect
 * @default 0
 *
 * @param Item Back Color 2
 * @parent System Colors
 * @type color
 * @desc The first item back color. 0 = no effect
 * @default 0
*/
/*~struct~Color:es
 * @param Name
 * @text Nombre
 * @desc Dale un nombre a tu color para recordarlo más fácilmente.
 *
 * @param Color Value
 * @text Valor del Color
 * @default #ffffff
 * @desc El valor de color del color que desea. Soporta hexadecimal y rgb. Formato hexadecimal: #ffffff, formato RGB: rgb (255, 255, 255).
 *
 * @param ID
 * @text ID del color
 * @type number
 * @min 32
 * @default 32
 * @desc la ID del color. Esto será lo que escriba para x cuando escriba \c[x] en un mensaje.
 */
Imported.CGMZ_InfiniteColors = true;
CGMZ.Versions["Infinite Colors"] = "1.1.0";
CGMZ.InfiniteColors = {};
CGMZ.InfiniteColors.parameters = PluginManager.parameters('CGMZ_InfiniteColors');
CGMZ.InfiniteColors.NormalColor = Number(CGMZ.InfiniteColors.parameters["Normal Color"]);
CGMZ.InfiniteColors.SystemColor = Number(CGMZ.InfiniteColors.parameters["System Color"]);
CGMZ.InfiniteColors.CrisisColor = Number(CGMZ.InfiniteColors.parameters["Crisis Color"]);
CGMZ.InfiniteColors.DeathColor = Number(CGMZ.InfiniteColors.parameters["Death Color"]);
CGMZ.InfiniteColors.GaugeBackColor = Number(CGMZ.InfiniteColors.parameters["Gauge Back Color"]);
CGMZ.InfiniteColors.HPGaugeColor1 = Number(CGMZ.InfiniteColors.parameters["HP Gauge Color 1"]);
CGMZ.InfiniteColors.HPGaugeColor2 = Number(CGMZ.InfiniteColors.parameters["HP Gauge Color 2"]);
CGMZ.InfiniteColors.HPDamageColor = Number(CGMZ.InfiniteColors.parameters["HP Damage Color"]);
CGMZ.InfiniteColors.HPRecoverColor = Number(CGMZ.InfiniteColors.parameters["HP Recover Color"]);
CGMZ.InfiniteColors.MPGaugeColor1 = Number(CGMZ.InfiniteColors.parameters["MP Gauge Color 1"]);
CGMZ.InfiniteColors.MPGaugeColor2 = Number(CGMZ.InfiniteColors.parameters["MP Gauge Color 2"]);
CGMZ.InfiniteColors.MPCostColor = Number(CGMZ.InfiniteColors.parameters["MP Cost Color"]);
CGMZ.InfiniteColors.MPColor = Number(CGMZ.InfiniteColors.parameters["MP Color"]);
CGMZ.InfiniteColors.MPDamageColor = Number(CGMZ.InfiniteColors.parameters["MP Damage Color"]);
CGMZ.InfiniteColors.MPRecoverColor = Number(CGMZ.InfiniteColors.parameters["MP Recover Color"]);
CGMZ.InfiniteColors.PowerUpColor = Number(CGMZ.InfiniteColors.parameters["Power Up Color"]);
CGMZ.InfiniteColors.PowerDownColor = Number(CGMZ.InfiniteColors.parameters["Power Down Color"]);
CGMZ.InfiniteColors.CTGaugeColor1 = Number(CGMZ.InfiniteColors.parameters["CT Gauge Color 1"]);
CGMZ.InfiniteColors.CTGaugeColor2 = Number(CGMZ.InfiniteColors.parameters["CT Gauge Color 2"]);
CGMZ.InfiniteColors.TPGaugeColor1 = Number(CGMZ.InfiniteColors.parameters["TP Gauge Color 1"]);
CGMZ.InfiniteColors.TPGaugeColor2 = Number(CGMZ.InfiniteColors.parameters["TP Gauge Color 2"]);
CGMZ.InfiniteColors.TPCostColor = Number(CGMZ.InfiniteColors.parameters["TP Cost Color"]);
CGMZ.InfiniteColors.TPColor = Number(CGMZ.InfiniteColors.parameters["TP Color"]);
CGMZ.InfiniteColors.OutlineColor = Number(CGMZ.InfiniteColors.parameters["Outline Color"]);
CGMZ.InfiniteColors.DimColor1 = Number(CGMZ.InfiniteColors.parameters["Dim Color 1"]);
CGMZ.InfiniteColors.DimColor2 = Number(CGMZ.InfiniteColors.parameters["Dim Color 2"]);
CGMZ.InfiniteColors.ItemBackColor1 = Number(CGMZ.InfiniteColors.parameters["Item Back Color 1"]);
CGMZ.InfiniteColors.ItemBackColor2 = Number(CGMZ.InfiniteColors.parameters["Item Back Color 2"]);
CGMZ.InfiniteColors.Colors = CGMZ_Utils.parseJSON(CGMZ.InfiniteColors.parameters["Colors"], [], "[CGMZ] Infinite Colors", "Your Colors parameter had invalid JSON and could not be read.");
//=============================================================================
// CGMZ_Temp
//-----------------------------------------------------------------------------
// Manage Color Data. Use temp class since this info doesn't need to be saved.
//=============================================================================
//-----------------------------------------------------------------------------
// Also initialize color data
//-----------------------------------------------------------------------------
const alias_CGMZ_InfiniteColors_createPluginData = CGMZ_Temp.prototype.createPluginData;
CGMZ_Temp.prototype.createPluginData = function() {
	alias_CGMZ_InfiniteColors_createPluginData.call(this);
	this.initializeInfiniteColorsData();
};
//-----------------------------------------------------------------------------
// Initialize color data
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.initializeInfiniteColorsData = function() {
	this._infiniteColors = {};
	for(const colorJSON of CGMZ.InfiniteColors.Colors) {
		const colorParse = CGMZ_Utils.parseJSON(colorJSON, null, "[CGMZ] Infinite Colors", "One of your colors had invalid JSON and could not be parsed.")
		if(!colorParse) continue;
		const colorValue = colorParse["Color Value"];
		this._infiniteColors[Number(colorParse.ID)] = colorValue;
	}
};
//-----------------------------------------------------------------------------
// Get Infinite Color
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.getInfiniteColor = function(id) {
	return this._infiniteColors[id];
};
//=============================================================================
// Color Manager
//-----------------------------------------------------------------------------
// Load additional text colors
//=============================================================================
//-----------------------------------------------------------------------------
// Return CGMZ color data if n > 31
//-----------------------------------------------------------------------------
const alias_CGMZ_InfiniteColors_textColor = ColorManager.textColor;
ColorManager.textColor = function(n) {
	if(n > 31) {
		return $cgmzTemp.getInfiniteColor(n);
	}
	return alias_CGMZ_InfiniteColors_textColor.call(this, n);
};
//-----------------------------------------------------------------------------
// Change normal color if set
//-----------------------------------------------------------------------------
if(CGMZ.InfiniteColors.NormalColor) {
ColorManager.normalColor = function() {
    return this.textColor(CGMZ.InfiniteColors.NormalColor);
};
}
//-----------------------------------------------------------------------------
// Change system color if set
//-----------------------------------------------------------------------------
if(CGMZ.InfiniteColors.SystemColor) {
ColorManager.systemColor = function() {
    return this.textColor(CGMZ.InfiniteColors.SystemColor);
};
}
//-----------------------------------------------------------------------------
// Change crisis color if set
//-----------------------------------------------------------------------------
if(CGMZ.InfiniteColors.CrisisColor) {
ColorManager.crisisColor = function() {
    return this.textColor(CGMZ.InfiniteColors.CrisisColor);
};
}
//-----------------------------------------------------------------------------
// Change death color if set
//-----------------------------------------------------------------------------
if(CGMZ.InfiniteColors.DeathColor) {
ColorManager.deathColor = function() {
    return this.textColor(CGMZ.InfiniteColors.DeathColor);
};
}
//-----------------------------------------------------------------------------
// Change gauge back color if set
//-----------------------------------------------------------------------------
if(CGMZ.InfiniteColors.GaugeBackColor) {
ColorManager.gaugeBackColor = function() {
    return this.textColor(CGMZ.InfiniteColors.GaugeBackColor);
};
}
//-----------------------------------------------------------------------------
// Change hp gauge color 1 if set
//-----------------------------------------------------------------------------
if(CGMZ.InfiniteColors.HPGaugeColor1) {
ColorManager.hpGaugeColor1 = function() {
    return this.textColor(CGMZ.InfiniteColors.HPGaugeColor1);
};
}
//-----------------------------------------------------------------------------
// Change hp gauge color 2 if set
//-----------------------------------------------------------------------------
if(CGMZ.InfiniteColors.HPGaugeColor2) {
ColorManager.hpGaugeColor2 = function() {
    return this.textColor(CGMZ.InfiniteColors.HPGaugeColor2);
};
}
//-----------------------------------------------------------------------------
// Change mp gauge color 1 if set
//-----------------------------------------------------------------------------
if(CGMZ.InfiniteColors.MPGaugeColor1) {
ColorManager.mpGaugeColor1 = function() {
    return this.textColor(CGMZ.InfiniteColors.MPGaugeColor1);
};
}
//-----------------------------------------------------------------------------
// Change mp gauge color 2 if set
//-----------------------------------------------------------------------------
if(CGMZ.InfiniteColors.MPGaugeColor2) {
ColorManager.mpGaugeColor2 = function() {
    return this.textColor(CGMZ.InfiniteColors.MPGaugeColor2);
};
}
//-----------------------------------------------------------------------------
// Change mp cost color if set
//-----------------------------------------------------------------------------
if(CGMZ.InfiniteColors.MPCostColor) {
ColorManager.mpCostColor = function() {
    return this.textColor(CGMZ.InfiniteColors.MPCostColor);
};
}
//-----------------------------------------------------------------------------
// Change power up color if set
//-----------------------------------------------------------------------------
if(CGMZ.InfiniteColors.PowerUpColor) {
ColorManager.powerUpColor = function() {
    return this.textColor(CGMZ.InfiniteColors.PowerUpColor);
};
}
//-----------------------------------------------------------------------------
// Change power down color if set
//-----------------------------------------------------------------------------
if(CGMZ.InfiniteColors.PowerDownColor) {
ColorManager.powerDownColor = function() {
    return this.textColor(CGMZ.InfiniteColors.PowerDownColor);
};
}
//-----------------------------------------------------------------------------
// Change CT Gauge color 1 if set
//-----------------------------------------------------------------------------
if(CGMZ.InfiniteColors.CTGaugeColor1) {
ColorManager.ctGaugeColor1 = function() {
    return this.textColor(CGMZ.InfiniteColors.CTGaugeColor1);
};
}
//-----------------------------------------------------------------------------
// Change CT Gauge color 2 if set
//-----------------------------------------------------------------------------
if(CGMZ.InfiniteColors.CTGaugeColor2) {
ColorManager.ctGaugeColor2 = function() {
    return this.textColor(CGMZ.InfiniteColors.CTGaugeColor2);
};
}
//-----------------------------------------------------------------------------
// Change TP Gauge color 1 if set
//-----------------------------------------------------------------------------
if(CGMZ.InfiniteColors.TPGaugeColor1) {
ColorManager.tpGaugeColor1 = function() {
    return this.textColor(CGMZ.InfiniteColors.TPGaugeColor1);
};
}
//-----------------------------------------------------------------------------
// Change TP Gauge color 2 if set
//-----------------------------------------------------------------------------
if(CGMZ.InfiniteColors.TPGaugeColor2) {
ColorManager.tpGaugeColor2 = function() {
    return this.textColor(CGMZ.InfiniteColors.TPGaugeColor2);
};
}
//-----------------------------------------------------------------------------
// Change TP cost color if set
//-----------------------------------------------------------------------------
if(CGMZ.InfiniteColors.TPCostColor) {
ColorManager.tpCostColor = function() {
    return this.textColor(CGMZ.InfiniteColors.TPCostColor);
};
}
//-----------------------------------------------------------------------------
// Change MP color if set
//-----------------------------------------------------------------------------
if(CGMZ.InfiniteColors.MPColor) {
ColorManager.mpColor = function(/*actor*/) {
    return this.textColor(CGMZ.InfiniteColors.MPColor);
};
}
//-----------------------------------------------------------------------------
// Change TP color if set
//-----------------------------------------------------------------------------
if(CGMZ.InfiniteColors.TPColor) {
ColorManager.tpColor = function(/*actor*/) {
    return this.textColor(CGMZ.InfiniteColors.TPColor);
};
}
//-----------------------------------------------------------------------------
// Change damage color if set
//-----------------------------------------------------------------------------
const alias_CGMZInfiniteColors_ColorManager_damageColor = ColorManager.damageColor;
ColorManager.damageColor = function(colorType) {
    switch (colorType) {
        case 0: // HP damage
            return (CGMZ.InfiniteColors.HPDamageColor) ? this.textColor(CGMZ.InfiniteColors.HPDamageColor) : alias_CGMZInfiniteColors_ColorManager_damageColor.call(this, colorType);
        case 1: // HP recover
            return (CGMZ.InfiniteColors.HPRecoverColor) ? this.textColor(CGMZ.InfiniteColors.HPRecoverColor) : alias_CGMZInfiniteColors_ColorManager_damageColor.call(this, colorType);
        case 2: // MP damage
            return (CGMZ.InfiniteColors.MPDamageColor) ? this.textColor(CGMZ.InfiniteColors.MPDamageColor) : alias_CGMZInfiniteColors_ColorManager_damageColor.call(this, colorType);
        case 3: // MP recover
            return (CGMZ.InfiniteColors.MPRecoverColor) ? this.textColor(CGMZ.InfiniteColors.MPRecoverColor) : alias_CGMZInfiniteColors_ColorManager_damageColor.call(this, colorType);
        default:
            return alias_CGMZInfiniteColors_ColorManager_damageColor.call(this, colorType);
    }
};
//-----------------------------------------------------------------------------
// Change outline color if set
//-----------------------------------------------------------------------------
if(CGMZ.InfiniteColors.OutlineColor) {
ColorManager.outlineColor = function() {
    return this.textColor(CGMZ.InfiniteColors.OutlineColor);
};
}
//-----------------------------------------------------------------------------
// Change dim color 1 if set
//-----------------------------------------------------------------------------
if(CGMZ.InfiniteColors.DimColor1) {
ColorManager.dimColor1 = function() {
    return this.textColor(CGMZ.InfiniteColors.DimColor1);
};
}
//-----------------------------------------------------------------------------
// Change dim color 2 if set
//-----------------------------------------------------------------------------
if(CGMZ.InfiniteColors.DimColor2) {
ColorManager.dimColor2 = function() {
    return this.textColor(CGMZ.InfiniteColors.DimColor2);
};
}
//-----------------------------------------------------------------------------
// Change item back color 1 if set
//-----------------------------------------------------------------------------
if(CGMZ.InfiniteColors.ItemBackColor1) {
ColorManager.itemBackColor1 = function() {
    return this.textColor(CGMZ.InfiniteColors.ItemBackColor1);
};
}
//-----------------------------------------------------------------------------
// Change item back color 2 if set
//-----------------------------------------------------------------------------
if(CGMZ.InfiniteColors.ItemBackColor2) {
ColorManager.itemBackColor2 = function() {
    return this.textColor(CGMZ.InfiniteColors.ItemBackColor2);
};
}