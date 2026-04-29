/*:
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/vehiclelandingrestrictions/
 * @target MZ
 * @plugindesc Allows you to restrict where a vehicle can dock by region
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
 * Description: This plugin allows you to restrict where vehicles can land/dock
 * based on a region. There are separate restrictions on a per-vehicle basis.
 * ----------------------------------------------------------------------------
 * Documentation:
 * This plugin adds additional requirements for landing a vehicle, it still
 * obeys the default landing restrictions.
 *
 * The region IDs specified for each vehicle stop it from being able to land
 * while it is currently on that region ID. To stop boat/ship from landing
 * somewhere, put the region on the sea tiles rather than the land tiles.
 * -------------------------Plugin Commands------------------------------------
 * This plugin does not have any plugin commands.
 * ---------------------------Saved Games--------------------------------------
 * This plugin fully supports saved games.
 * ✓ You should be able to add this plugin to a saved game
 * ✓ You can modify plugin parameters and have them reflected in saved games
 * ✓ You can remove this plugin with no issue to save data
 * -----------------------------Filename---------------------------------------
 * This plugin's JS filename MUST remain CGMZ_VehicleLandingRestrictions.js
 * This is what it comes as when downloaded. The filename is used to load
 * parameters and execute plugin commands. If you change it, things will begin
 * behaving incorrectly and your game will probably crash. Please do not
 * rename the js file.
 * --------------------------Latest Version------------------------------------
 * Hi all, this latest version adds terrain tags as possible landing
 * restrictions for your vehicles. This is an alternative to using region IDs,
 * since those are likely to be widely used already in other plugins. You can
 * use terrain tags in their place, or use both terrain tag and regions to
 * restrict where the player can land.
 * 
 * Version 1.3.0
 * - Added terrain tag landing restrictions
 * 
 * @param Boat Options
 *
 * @param Boat Landing Regions
 * @type number[]
 * @min 1
 * @default []
 * @parent Boat Options
 * @desc Region IDs that disallow the vehicle to dock. Leaving this empty = skips this check
 *
 * @param Boat Landing Tags
 * @type number[]
 * @min 0
 * @default []
 * @parent Boat Options
 * @desc Terrain Tags that disallow the vehicle to dock. Leaving this empty = skips this check
 *
 * @param Boat Switch
 * @parent Boat Options
 * @type switch
 * @default 0
 * @desc If set, this switch must be ON for the boat landing restrictions to apply.
 *
 * @param Ship Options
 *
 * @param Ship Landing Regions
 * @type number[]
 * @min 1
 * @default []
 * @parent Ship Options
 * @desc Region IDs that disallow the vehicle to dock. Leaving this empty = skips this check
 *
 * @param Ship Landing Tags
 * @type number[]
 * @min 0
 * @default []
 * @parent Ship Options
 * @desc Terrain Tags that disallow the vehicle to dock. Leaving this empty = skips this check
 *
 * @param Ship Switch
 * @parent Ship Options
 * @type switch
 * @default 0
 * @desc If set, this switch must be ON for the ship landing restrictions to apply.
 *
 * @param Airship Options
 *
 * @param Airship Landing Regions
 * @type number[]
 * @min 1
 * @default []
 * @parent Airship Options
 * @desc Region IDs that disallow the vehicle to land. Leaving this empty = skips this check
 *
 * @param Airship Landing Tags
 * @type number[]
 * @min 0
 * @default []
 * @parent Airship Options
 * @desc Terrain Tags that disallow the vehicle to dock. Leaving this empty = skips this check
 *
 * @param Airship Switch
 * @parent Airship Options
 * @type switch
 * @default 0
 * @desc If set, this switch must be ON for the airship landing restrictions to apply.
 *
 * @param Mechanics
 *
 * @param Common Event
 * @type common_event
 * @default 0
 * @parent Mechanics
 * @desc A common event ID that will be run when the player tries landing where they cannot due to region restriction.
*/
/*:es
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/vehiclelandingrestrictions/
 * @target MZ
 * @plugindesc Te permite restringir dónde puede atracar o aterrizar un vehículo por región.
 * @help
 * ============================================================================
 * Para terminos y condiciones de uso de este pluging en tu juego, por favor
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
 * Descripción: Este plugin te permite restringir dónde pueden
 * aterrizar/atracar los vehículos basado en una región. Hay restricciones
 * separadas por vehículo.
 * ----------------------------------------------------------------------------
 * Documentación:
 * Este plugin agrega requisitos adicionales para aterrizar un vehículo, aún
 * obedece las restricciones de aterrizaje predeterminadas.
 *
 * Los ID de región especificados para cada vehículo impiden que pueda
 * aterrizar mientras se encuentra actualmente en ese ID de región. Para evitar
 * que el barco/barco aterrice en algún lugar, coloca la región en las fichas
 * de mar en lugar de las fichas de tierra.
 * -------------------------Plugin Commands------------------------------------
 * This plugin does not have any plugin commands.
 * ---------------------------Saved Games--------------------------------------
 * This plugin fully supports saved games.
 * ✓ You should be able to add this plugin to a saved game
 * ✓ You can modify plugin parameters and have them reflected in saved games
 * ✓ You can remove this plugin with no issue to save data
 * -----------------------------Filename---------------------------------------
 * This plugin's JS filename MUST remain CGMZ_VehicleLandingRestrictions.js
 * This is what it comes as when downloaded. The filename is used to load
 * parameters and execute plugin commands. If you change it, things will begin
 * behaving incorrectly and your game will probably crash. Please do not
 * rename the js file.
 * --------------------------Latest Version------------------------------------
 * Hi all, this latest version adds terrain tags as possible landing
 * restrictions for your vehicles. This is an alternative to using region IDs,
 * since those are likely to be widely used already in other plugins. You can
 * use terrain tags in their place, or use both terrain tag and regions to
 * restrict where the player can land.
 * 
 * Version 1.3.0
 * - Added terrain tag landing restrictions
 * 
 * @param Boat Options
 * @text Opciones de Bote
 *
 * @param Boat Landing Regions
 * @text Regiones de aterrizaje de botes
 * @type number[]
 * @min 1
 * @default []
 * @parent Boat Options
 * @desc ID de región que no permiten que el vehículo se acople. Dejar esto vacío significa que puede acoplarse en cualquier lugar.
 *
 * @param Boat Landing Tags
 * @type number[]
 * @min 0
 * @default []
 * @parent Boat Options
 * @desc Terrain Tags that disallow the vehicle to dock. Leaving this empty = skips this check
 *
 * @param Boat Switch
 * @parent Boat Options
 * @type switch
 * @default 0
 * @desc If set, this switch must be ON for the boat landing restrictions to apply.
 *
 * @param Ship Options
 * @text Opciones de Barco
 *
 * @param Ship Landing Regions
 * @text Regiones de aterrizaje de barcos
 * @type number[]
 * @min 1
 * @default []
 * @parent Ship Options
 * @desc ID de región que no permiten que el vehículo se acople. Dejar esto vacío significa que puede acoplarse en cualquier lugar
 *
 * @param Ship Landing Tags
 * @type number[]
 * @min 0
 * @default []
 * @parent Ship Options
 * @desc Terrain Tags that disallow the vehicle to dock. Leaving this empty = skips this check
 *
 * @param Ship Switch
 * @parent Ship Options
 * @type switch
 * @default 0
 * @desc If set, this switch must be ON for the ship landing restrictions to apply.
 *
 * @param Airship Options
 * @text Opciones de Dirigible
 *
 * @param Airship Landing Regions
 * @text Regiones de aterrizaje de dirigibles
 * @type number[]
 * @min 1
 * @default []
 * @parent Airship Options
 * @desc ID de región que impiden que el vehículo aterrice. Dejar esto vacío significa que puede aterrizar en cualquier lugar.
 *
 * @param Airship Landing Tags
 * @type number[]
 * @min 0
 * @default []
 * @parent Airship Options
 * @desc Terrain Tags that disallow the vehicle to dock. Leaving this empty = skips this check
 *
 * @param Airship Switch
 * @parent Airship Options
 * @type switch
 * @default 0
 * @desc If set, this switch must be ON for the airship landing restrictions to apply.
 *
 * @param Mechanics
 *
 * @param Common Event
 * @type common_event
 * @default 0
 * @parent Mechanics
 * @desc A common event ID that will be run when the player tries landing where they cannot due to region restriction.
*/
Imported.CGMZ_VehicleLandingRestrictions = true;
CGMZ.Versions["Vehicle Landing Restrictions"] = "1.3.0";
CGMZ.VehicleLandingRestrictions = {};
CGMZ.VehicleLandingRestrictions.parameters = PluginManager.parameters('CGMZ_VehicleLandingRestrictions');
CGMZ.VehicleLandingRestrictions.CommonEvent = Number(CGMZ.VehicleLandingRestrictions.parameters["Common Event"]);
CGMZ.VehicleLandingRestrictions.BoatSwitch = Number(CGMZ.VehicleLandingRestrictions.parameters["Boat Switch"]);
CGMZ.VehicleLandingRestrictions.ShipSwitch = Number(CGMZ.VehicleLandingRestrictions.parameters["Ship Switch"]);
CGMZ.VehicleLandingRestrictions.AirshipSwitch = Number(CGMZ.VehicleLandingRestrictions.parameters["Airship Switch"]);
CGMZ.VehicleLandingRestrictions.BoatDockRegions = CGMZ_Utils.parseJSON(CGMZ.VehicleLandingRestrictions.parameters["Boat Landing Regions"], [], "[CGMZ] Vehicle Landing Restrictions", "Your Boat region restrictions had invalid JSON, check it is not blank").map(x => Number(x));
CGMZ.VehicleLandingRestrictions.ShipDockRegions = CGMZ_Utils.parseJSON(CGMZ.VehicleLandingRestrictions.parameters["Ship Landing Regions"], [], "[CGMZ] Vehicle Landing Restrictions", "Your Ship region restrictions had invalid JSON, check it is not blank").map(x => Number(x));
CGMZ.VehicleLandingRestrictions.AirshipDockRegions = CGMZ_Utils.parseJSON(CGMZ.VehicleLandingRestrictions.parameters["Airship Landing Regions"], [], "[CGMZ] Vehicle Landing Restrictions", "Your Airship region restrictions had invalid JSON, check it is not blank").map(x => Number(x));
CGMZ.VehicleLandingRestrictions.BoatDockTags = CGMZ_Utils.parseJSON(CGMZ.VehicleLandingRestrictions.parameters["Boat Landing Tags"], [], "[CGMZ] Vehicle Landing Restrictions", "Your Boat Landing Tags had invalid JSON, check it is not blank").map(x => Number(x));
CGMZ.VehicleLandingRestrictions.ShipDockTags = CGMZ_Utils.parseJSON(CGMZ.VehicleLandingRestrictions.parameters["Ship Landing Tags"], [], "[CGMZ] Vehicle Landing Restrictions", "Your Ship Landing Tags had invalid JSON, check it is not blank").map(x => Number(x));
CGMZ.VehicleLandingRestrictions.AirshipDockTags = CGMZ_Utils.parseJSON(CGMZ.VehicleLandingRestrictions.parameters["Airship Landing Tags"], [], "[CGMZ] Vehicle Landing Restrictions", "Your Airship Landing Tags had invalid JSON, check it is not blank").map(x => Number(x));
//=============================================================================
// Game_Vehicle
//-----------------------------------------------------------------------------
// Modify the vehicle object for additional options
//=============================================================================
//-----------------------------------------------------------------------------
// Also check for landing regions
//-----------------------------------------------------------------------------
const alias_CGMZVehicleLandingRestrictions_isLandOk = Game_Vehicle.prototype.isLandOk;
Game_Vehicle.prototype.isLandOk = function(x, y, d) {
	const vehicleLandingRestriction = this.CGMZ_VehicleLandingRestrictions_canLand(x, y, d);
	const oldReturn = alias_CGMZVehicleLandingRestrictions_isLandOk.call(this, x, y, d);
	if(CGMZ.VehicleLandingRestrictions.CommonEvent && oldReturn && !vehicleLandingRestriction) {
		$gameTemp.reserveCommonEvent(CGMZ.VehicleLandingRestrictions.CommonEvent);
	}
	return oldReturn && vehicleLandingRestriction;
};
//-----------------------------------------------------------------------------
// Check the region the player is in for a no-land region
//-----------------------------------------------------------------------------
Game_Vehicle.prototype.CGMZ_VehicleLandingRestrictions_canLand = function(x, y, d) {
	if(this.isBoat()) {
		if(this.CGMZ_canApplyBoatRestrictions()) {
			return this.CGMZ_canLandBoatWithRestrictions(x, y, d);
		}
	} else if(this.isShip()) {
		if(this.CGMZ_canApplyShipRestrictions()) {
			return this.CGMZ_canLandShipWithRestrictions(x, y, d);
		}
	} else if(this.isAirship()) {
		if(this.CGMZ_canApplyAirshipRestrictions()) {
			return this.CGMZ_canLandAirshipWithRestrictions(x, y, d);
		}
	}
	return true;
};
//-----------------------------------------------------------------------------
// Check if boat restrictions apply
//-----------------------------------------------------------------------------
Game_Vehicle.prototype.CGMZ_canApplyBoatRestrictions = function() {
	if(CGMZ.VehicleLandingRestrictions.BoatSwitch && !$gameSwitches.value(CGMZ.VehicleLandingRestrictions.BoatSwitch)) return false;
	return true;
};
//-----------------------------------------------------------------------------
// Check if ship restrictions apply
//-----------------------------------------------------------------------------
Game_Vehicle.prototype.CGMZ_canApplyShipRestrictions = function() {
	if(CGMZ.VehicleLandingRestrictions.ShipSwitch && !$gameSwitches.value(CGMZ.VehicleLandingRestrictions.ShipSwitch)) return false;
	return true;
};
//-----------------------------------------------------------------------------
// Check if airship restrictions apply
//-----------------------------------------------------------------------------
Game_Vehicle.prototype.CGMZ_canApplyAirshipRestrictions = function() {
	if(CGMZ.VehicleLandingRestrictions.AirshipSwitch && !$gameSwitches.value(CGMZ.VehicleLandingRestrictions.AirshipSwitch)) return false;
	return true;
};
//-----------------------------------------------------------------------------
// Check if boat can land apply
//-----------------------------------------------------------------------------
Game_Vehicle.prototype.CGMZ_canLandBoatWithRestrictions = function(x, y, d) {
	const regionId = $gameMap.regionId(x, y);
	const terrainTag = $gameMap.terrainTag(x, y);
	if(CGMZ.VehicleLandingRestrictions.BoatDockRegions.length > 0 && CGMZ.VehicleLandingRestrictions.BoatDockRegions.includes(regionId)) return false;
	if(CGMZ.VehicleLandingRestrictions.BoatDockTags.length > 0 && CGMZ.VehicleLandingRestrictions.BoatDockTags.includes(terrainTag)) return false;
	return true;
};
//-----------------------------------------------------------------------------
// Check if ship can land apply
//-----------------------------------------------------------------------------
Game_Vehicle.prototype.CGMZ_canLandShipWithRestrictions = function(x, y, d) {
	const regionId = $gameMap.regionId(x, y);
	const terrainTag = $gameMap.terrainTag(x, y);
	if(CGMZ.VehicleLandingRestrictions.ShipDockRegions.length > 0 && CGMZ.VehicleLandingRestrictions.ShipDockRegions.includes(regionId)) return false;
	if(CGMZ.VehicleLandingRestrictions.ShipDockTags.length > 0 && CGMZ.VehicleLandingRestrictions.ShipDockTags.includes(terrainTag)) return false;
	return true;
};
//-----------------------------------------------------------------------------
// Check if airship can land apply
//-----------------------------------------------------------------------------
Game_Vehicle.prototype.CGMZ_canLandAirshipWithRestrictions = function(x, y, d) {
	const regionId = $gameMap.regionId(x, y);
	const terrainTag = $gameMap.terrainTag(x, y);
	if(CGMZ.VehicleLandingRestrictions.AirshipDockRegions.length > 0 && CGMZ.VehicleLandingRestrictions.AirshipDockRegions.includes(regionId)) return false;
	if(CGMZ.VehicleLandingRestrictions.AirshipDockTags.length > 0 && CGMZ.VehicleLandingRestrictions.AirshipDockTags.includes(terrainTag)) return false;
	return true;
};