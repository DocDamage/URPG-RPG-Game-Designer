/*:
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/vehicleencounters/
 * @base CGMZ_Core
 * @orderAfter CGMZ_Core
 * @orderAfter CGMZ_AnimatedBattleBacks
 * @target MZ
 * @plugindesc Allows you to have encounters while on a vehicle
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
 * Made for RPG Maker MZ 1.10.0
 * ----------------------------------------------------------------------------
 * Description: This plugin lets you have encounters on the boat, ship, and
 * airship. It allows for custom encounter steps, battlebacks, and encounter
 * lists per vehicle.
 * ----------------------------------------------------------------------------
 * Documentation:
 * -------------------------Plugin Commands------------------------------------
 * This plugin does not have any plugin commands.
 * ---------------------------Saved Games--------------------------------------
 * This plugin fully supports saved games.
 * ✓ You should be able to add this plugin to a saved game
 * ✓ You can modify plugin parameters and have them reflected in saved games
 * ✓ You can remove this plugin with no issue to save data
 * -----------------------------Filename---------------------------------------
 * This plugin's JS filename MUST remain CGMZ_VehicleEncounters.js
 * This is what it comes as when downloaded. The filename is used to load
 * parameters and execute plugin commands. If you change it, things will begin
 * behaving incorrectly and your game will probably crash. Please do not
 * rename the js file.
 * --------------------------Latest Version------------------------------------
 * Hi all, this latest version adds the weight feature to your encounters set
 * up through this plugin. The ultimate goal is to allow you to also add map
 * encounters set up as normal in your vehicles as well, though that will come
 * in a future update. For now, this change will allow you to make certain
 * troops more of less likely to be encountered in your vehicles.
 *
 * Unfortunately, you will need to set up your vehicle encounter lists again,
 * since before they were just simple troop id lists and now they are a proper
 * struct.
 *
 * Version 1.2.0
 * - Added weights to encounter troops
 * 
 * @param Boat Options
 *
 * @param Boat Encounters
 * @type struct<Encounter>[]
 * @default []
 * @parent Boat Options
 * @desc Troops that the vehicle can encounter. Leaving this empty means no encounters.
 *
 * @param Boat Encounter Steps
 * @type number
 * @min 1
 * @default 60
 * @parent Boat Options
 * @desc Average steps before encounter in this vehicle.
 *
 * @param Boat Battleback1
 * @type file
 * @dir img/battlebacks1
 * @default 
 * @parent Boat Options
 * @desc Battle back 1 to use for encounters in this vehicle.
 *
 * @param Boat Battleback2
 * @type file
 * @dir img/battlebacks2
 * @default 
 * @parent Boat Options
 * @desc Battle back 2 to use for encounters in this vehicle.
 *
 * @param Boat Encounter Switch
 * @type switch
 * @parent Boat Options
 * @default 0
 * @desc Switch that turns on/off encounters in this vehicle.
 *
 * @param Ship Options
 *
 * @param Ship Encounters
 * @type struct<Encounter>[]
 * @default []
 * @parent Ship Options
 * @desc Troops that the vehicle can encounter. Leaving this empty means no encounters.
 *
 * @param Ship Encounter Steps
 * @type number
 * @min 1
 * @default 60
 * @parent Ship Options
 * @desc Average steps before encounter in this vehicle.
 *
 * @param Ship Battleback1
 * @type file
 * @dir img/battlebacks1
 * @parent Ship Options
 * @desc Battle back 1 to use for encounters in this vehicle.
 
 * @param Ship Battleback2
 * @type file
 * @dir img/battlebacks2
 * @parent Ship Options
 * @desc Battle back 2 to use for encounters in this vehicle.
 *
 * @param Ship Encounter Switch
 * @type switch
 * @parent Ship Options
 * @default 0
 * @desc Switch that turns on/off encounters in this vehicle.
 *
 * @param Airship Options
 *
 * @param Airship Encounters
 * @type struct<Encounter>[]
 * @default []
 * @parent Airship Options
 * @desc Troops that the vehicle can encounter. Leaving this empty means no encounters.
 *
 * @param Airship Encounter Steps
 * @type number
 * @min 1
 * @default 60
 * @parent Airship Options
 * @desc Average steps before encounter in this vehicle.
 *
 * @param Airship Battleback1
 * @type file
 * @dir img/battlebacks1
 * @parent Airship Options
 * @desc Battle back 1 to use for encounters in this vehicle.
 
 * @param Airship Battleback2
 * @type file
 * @dir img/battlebacks2
 * @parent Airship Options
 * @desc Battle back 2 to use for encounters in this vehicle.
 *
 * @param Airship Encounter Switch
 * @type switch
 * @parent Airship Options
 * @default 0
 * @desc Switch that turns on/off encounters in this vehicle.
 *
 * @param Integrations
 *
 * @param Boat Animated Back
 * @parent Integrations
 * @desc Animated battle back id to use when in the boat (if any)
 *
 * @param Ship Animated Back
 * @parent Integrations
 * @desc Animated battle back id to use when in the ship (if any)
 *
 * @param Airship Animated Back
 * @parent Integrations
 * @desc Animated battle back id to use when in the airship (if any)
*/
/*~struct~Encounter:
 * @param Troop
 * @type troop
 * @default 0
 * @desc The troop to encounter
 *
 * @param Weight
 * @type number
 * @min 0
 * @default 5
 * @desc The weight of the troop. Higher number = more likely to encounter
*/
/*:es
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/vehicleencounters/
 * @target MZ
 * @plugindesc Te permite tener encuentros mientras se está en un vehículo
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
 * Versión: 1.2.0
 * ----------------------------------------------------------------------------
 * Compatibilidad: Sólo probado con mis CGMZ plugins.
 * Hecho para RPG Maker MZ 1.10.0
 * ----------------------------------------------------------------------------
 * Descripción: Este plugin te permite tener encuentros en el bote, el barco y
 * dirigible. Permite pasos de encuentro personalizados, batallas y listas de 
 * encuentros por vehículo.
 * ----------------------------------------------------------------------------
 * Documentación:
 * -------------------------Plugin Commands------------------------------------
 * This plugin does not have any plugin commands.
 * ---------------------------Saved Games--------------------------------------
 * This plugin fully supports saved games.
 * ✓ You should be able to add this plugin to a saved game
 * ✓ You can modify plugin parameters and have them reflected in saved games
 * ✓ You can remove this plugin with no issue to save data
 * -----------------------------Filename---------------------------------------
 * This plugin's JS filename MUST remain CGMZ_VehicleEncounters.js
 * This is what it comes as when downloaded. The filename is used to load
 * parameters and execute plugin commands. If you change it, things will begin
 * behaving incorrectly and your game will probably crash. Please do not
 * rename the js file.
 * --------------------------Latest Version------------------------------------
 * Hi all, this latest version adds the weight feature to your encounters set
 * up through this plugin. The ultimate goal is to allow you to also add map
 * encounters set up as normal in your vehicles as well, though that will come
 * in a future update. For now, this change will allow you to make certain
 * troops more of less likely to be encountered in your vehicles.
 *
 * Unfortunately, you will need to set up your vehicle encounter lists again,
 * since before they were just simple troop id lists and now they are a proper
 * struct.
 *
 * Version 1.2.0
 * - Added weights to encounter troops
 * 
 * @param Boat Options
 * @text Opciones de Bote
 *
 * @param Boat Encounters
 * @text Encuentros de Bote
 * @type struct<Encounter>[]
 * @default []
 * @parent Boat Options
 * @desc Tropas que el vehículo puede encontrar. Dejar esto vacío significa que no hay encuentros.
 *
 * @param Boat Encounter Steps
 * @text Pasos de encuentro en bote
 * @type number
 * @min 1
 * @default 60
 * @parent Boat Options
 * @desc Promedio de pasos antes del encuentro en este vehículo.
 *
 * @param Boat Battleback1
 * @text Contrabatalla de Botes1
 * @type file
 * @dir img/battlebacks1
 * @parent Boat Options
 * @desc Contrabatalla 1 para usar en encuentros en este vehículo.
 
 * @param Boat Battleback2
 * @text Contrabatalla de Botes2
 * @type file
 * @dir img/battlebacks2
 * @parent Boat Options
 * @desc Contrabatalla 2 para usar en encuentros en este vehículo.
 *
 * @param Boat Encounter Switch
 * @text Interruptor de encuentro con bote
 * @type switch
 * @parent Boat Options
 * @default 0
 * @desc Interruptor que enciende/apaga los encuentros en este vehículo.
 *
 * @param Ship Options
 * @text Opciones de Barco
 *
 * @param Ship Encounters
 * @text Encuentros de Barco
 * @type struct<Encounter>[]
 * @default []
 * @parent Ship Options
 * @desc Tropas que el vehículo puede encontrar. Dejar esto vacío significa que no hay encuentros.
 *
 * @param Ship Encounter Steps
 * @text Pasos de encuentro de barcos
 * @type number
 * @min 1
 * @default 60
 * @parent Ship Options
 * @desc Promedio de pasos antes del encuentro en este vehículo.
 *
 * @param Ship Battleback1
 * @text Contrabatalla de Barcos1
 * @type file
 * @dir img/battlebacks1
 * @parent Ship Options
 * @desc Contrabatalla 1 para usar en encuentros en este vehículo.
 
 * @param Ship Battleback2
 * @text Contrabatalla de Barcos2
 * @type file
 * @dir img/battlebacks2
 * @parent Ship Options
 * @desc Contrabatalla 2 para usar en encuentros en este vehículo.
 *
 * @param Ship Encounter Switch
 * @text Interruptor de encuentro de barcos
 * @type switch
 * @parent Ship Options
 * @default 0
 * @desc Interruptor que enciende/apaga los encuentros en este vehículo.
 *
 * @param Airship Options
 * @text Opciones de Dirigible
 *
 * @param Airship Encounters
 * @text Encuentros de aeronaves
 * @type struct<Encounter>[]
 * @default []
 * @parent Airship Options
 * @desc Tropas que el vehículo puede encontrar. Dejar esto vacío significa que no hay encuentros.
 *
 * @param Airship Encounter Steps
 * @text Pasos de encuentro de dirigible
 * @type number
 * @min 1
 * @default 60
 * @parent Airship Options
 * @desc Promedio de pasos antes del encuentro en este vehículo.
 *
 * @param Airship Battleback1
 * @text Contrabatalla de Dirigibles1
 * @type file
 * @dir img/battlebacks1
 * @parent Airship Options
 * @desc Contrabatalla 1 para usar en encuentros en este vehículo.
 
 * @param Airship Battleback2
 * @text Contrabatalla de Dirigibles2
 * @type file
 * @dir img/battlebacks2
 * @parent Airship Options
 * @desc Contrabatalla 2 para usar en encuentros en este vehículo.
 *
 * @param Airship Encounter Switch
 * @text Interruptor de encuentro de dirigible
 * @type switch
 * @parent Airship Options
 * @default 0
 * @desc Interruptor que enciende/apaga los encuentros en este vehículo.
 *
 * @param Integrations
 *
 * @param Boat Animated Back
 * @parent Integrations
 * @desc Animated battle back id to use when in the boat (if any)
 *
 * @param Ship Animated Back
 * @parent Integrations
 * @desc Animated battle back id to use when in the ship (if any)
 *
 * @param Airship Animated Back
 * @parent Integrations
 * @desc Animated battle back id to use when in the airship (if any)
*/
/*~struct~Encounter:es
 * @param Troop
 * @type troop
 * @default 0
 * @desc The troop to encounter
 *
 * @param Weight
 * @type number
 * @min 0
 * @default 5
 * @desc The weight of the troop. Higher number = more likely to encounter
*/
Imported.CGMZ_VehicleEncounters = true;
CGMZ.Versions["Vehicle Encounters"] = "1.2.0";
CGMZ.VehicleEncounters = {};
CGMZ.VehicleEncounters.parameters = PluginManager.parameters('CGMZ_VehicleEncounters');
CGMZ.VehicleEncounters.BoatBattleback1 = CGMZ.VehicleEncounters.parameters["Boat Battleback1"];
CGMZ.VehicleEncounters.BoatBattleback2 = CGMZ.VehicleEncounters.parameters["Boat Battleback2"];
CGMZ.VehicleEncounters.ShipBattleback1 = CGMZ.VehicleEncounters.parameters["Ship Battleback1"];
CGMZ.VehicleEncounters.ShipBattleback2 = CGMZ.VehicleEncounters.parameters["Ship Battleback2"];
CGMZ.VehicleEncounters.AirshipBattleback1 = CGMZ.VehicleEncounters.parameters["Airship Battleback1"];
CGMZ.VehicleEncounters.AirshipBattleback2 = CGMZ.VehicleEncounters.parameters["Airship Battleback2"];
CGMZ.VehicleEncounters.BoatAnimatedBack = CGMZ.VehicleEncounters.parameters["Boat Animated Back"];
CGMZ.VehicleEncounters.ShipAnimatedBack = CGMZ.VehicleEncounters.parameters["Ship Animated Back"];
CGMZ.VehicleEncounters.AirshipAnimatedBack = CGMZ.VehicleEncounters.parameters["Airship Animated Back"];
CGMZ.VehicleEncounters.BoatEncounterSteps = Number(CGMZ.VehicleEncounters.parameters["Boat Encounter Steps"]);
CGMZ.VehicleEncounters.ShipEncounterSteps = Number(CGMZ.VehicleEncounters.parameters["Ship Encounter Steps"]);
CGMZ.VehicleEncounters.AirshipEncounterSteps = Number(CGMZ.VehicleEncounters.parameters["Airship Encounter Steps"]);
CGMZ.VehicleEncounters.BoatEncounterSwitch = Number(CGMZ.VehicleEncounters.parameters["Boat Encounter Switch"]);
CGMZ.VehicleEncounters.ShipEncounterSwitch = Number(CGMZ.VehicleEncounters.parameters["Ship Encounter Switch"]);
CGMZ.VehicleEncounters.AirshipEncounterSwitch = Number(CGMZ.VehicleEncounters.parameters["Airship Encounter Switch"]);
CGMZ.VehicleEncounters.BoatEncounters = CGMZ_Utils.parseJSON(CGMZ.VehicleEncounters.parameters["Boat Encounters"], [], "[CGMZ] Vehicle Encounters", "Your Boat Encounters parameter did not have valid JSON and could not be read.");
CGMZ.VehicleEncounters.ShipEncounters = CGMZ_Utils.parseJSON(CGMZ.VehicleEncounters.parameters["Ship Encounters"], [], "[CGMZ] Vehicle Encounters", "Your Ship Encounters parameter did not have valid JSON and could not be read.");
CGMZ.VehicleEncounters.AirshipEncounters = CGMZ_Utils.parseJSON(CGMZ.VehicleEncounters.parameters["Airship Encounters"], [], "[CGMZ] Vehicle Encounters", "Your Airship Encounters parameter did not have valid JSON and could not be read.");
//=============================================================================
// CGMZ_Temp
//-----------------------------------------------------------------------------
// Parse troop json into usable objects
//=============================================================================
//-----------------------------------------------------------------------------
// Set up encounter data for vehicles
//-----------------------------------------------------------------------------
const alias_CGMZVehicleEncounters_CGMZTemp_createPluginData = CGMZ_Temp.prototype.createPluginData;
CGMZ_Temp.prototype.createPluginData = function() {
	alias_CGMZVehicleEncounters_CGMZTemp_createPluginData.call(this);
	this._vehicleEncounters = {
		boat: [],
		ship: [],
		airship: []
	};
	for(const boatJSON of CGMZ.VehicleEncounters.BoatEncounters) {
		const encounter = CGMZ_Utils.parseJSON(boatJSON, null, "[CGMZ] Vehicle Encounters", "One of your boat encounters was malformed and could not be parsed.");
		if(!encounter) continue;
		this._vehicleEncounters['boat'].push({
			troop: Number(encounter.Troop),
			weight: Number(encounter.Weight)
		});
	}
	for(const shipJSON of CGMZ.VehicleEncounters.ShipEncounters) {
		const encounter = CGMZ_Utils.parseJSON(shipJSON, null, "[CGMZ] Vehicle Encounters", "One of your ship encounters was malformed and could not be parsed.");
		if(!encounter) continue;
		this._vehicleEncounters['ship'].push({
			troop: Number(encounter.Troop),
			weight: Number(encounter.Weight)
		});
	}
	for(const airshipJSON of CGMZ.VehicleEncounters.AirshipEncounters) {
		const encounter = CGMZ_Utils.parseJSON(airshipJSON, null, "[CGMZ] Vehicle Encounters", "One of your airship encounters was malformed and could not be parsed.");
		if(!encounter) continue;
		this._vehicleEncounters['airship'].push({
			troop: Number(encounter.Troop),
			weight: Number(encounter.Weight)
		});
	}
};
//-----------------------------------------------------------------------------
// Get custom vehicle encounter list for a vehicle
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.getVehicleEncounterList = function(vehicleId) {
	return this._vehicleEncounters[vehicleId];
};
//=============================================================================
// Game_Player
//-----------------------------------------------------------------------------
// Update to check if vehicle interior map should be called, encounters in vehicle
//=============================================================================
//-----------------------------------------------------------------------------
// Make encounter count (avg steps differ based on vehicle)
//-----------------------------------------------------------------------------
const alias_CGMZ_VehicleEncounters_makeEncounterCount = Game_Player.prototype.makeEncounterCount;
Game_Player.prototype.makeEncounterCount = function() {
	const n = this.isInBoat()*CGMZ.VehicleEncounters.BoatEncounterSteps +
			this.isInShip()*CGMZ.VehicleEncounters.ShipEncounterSteps +
			this.isInAirship()*CGMZ.VehicleEncounters.AirshipEncounterSteps;
	if(n > 0) {
		this._encounterCount = Math.randomInt(n) + Math.randomInt(n) + 1;
	} else {
		alias_CGMZ_VehicleEncounters_makeEncounterCount.call(this);
	}
};
//-----------------------------------------------------------------------------
// Can encounter on all vehicles, if switch is on.
//-----------------------------------------------------------------------------
const alias_CGMZ_VehicleEncounters_canEncounter = Game_Player.prototype.canEncounter;
Game_Player.prototype.canEncounter = function() {
	if(this._vehicleGettingOn || this._vehicleGettingOff) {
		return false;
	}
	if(!$gameParty.hasEncounterNone() && $gameSystem.isEncounterEnabled() && !this.isMoveRouteForcing() && !this.isDebugThrough()) {
		if(this.isInBoat() && (CGMZ.VehicleEncounters.BoatEncounterSwitch === 0 || $gameSwitches.value(CGMZ.VehicleEncounters.BoatEncounterSwitch))) {
			return true;
		} else if(this.isInShip() && (CGMZ.VehicleEncounters.ShipEncounterSwitch === 0 || $gameSwitches.value(CGMZ.VehicleEncounters.ShipEncounterSwitch))) {
			return true;
		} else if(this.isInAirship() && (CGMZ.VehicleEncounters.AirshipEncounterSwitch === 0 || $gameSwitches.value(CGMZ.VehicleEncounters.AirshipEncounterSwitch))) {
			return true;
		}
	}
	if(!this.isInBoat() && !this.isInShip()) {
		return alias_CGMZ_VehicleEncounters_canEncounter.call(this);
	}
	return false;
};
//-----------------------------------------------------------------------------
// Different encounter lists for vehicles
//-----------------------------------------------------------------------------
const alias_CGMZ_VehicleEncounters_makeEncounterTroopId = Game_Player.prototype.makeEncounterTroopId;
Game_Player.prototype.makeEncounterTroopId = function() {
	if(this.isInBoat()) {
		return (this.CGMZ_makeEncounterTroopIdForVehicle('boat') || alias_CGMZ_VehicleEncounters_makeEncounterTroopId.call(this));
	} else if(this.isInShip()) {
		return (this.CGMZ_makeEncounterTroopIdForVehicle('ship') || alias_CGMZ_VehicleEncounters_makeEncounterTroopId.call(this));
	} else if(this.isInAirship()) {
		return (this.CGMZ_makeEncounterTroopIdForVehicle('airship') || alias_CGMZ_VehicleEncounters_makeEncounterTroopId.call(this));
	}
	return alias_CGMZ_VehicleEncounters_makeEncounterTroopId.call(this);
};
//-----------------------------------------------------------------------------
// Get the encounter troop id for a vehicle
//-----------------------------------------------------------------------------
Game_Player.prototype.CGMZ_makeEncounterTroopIdForVehicle = function(vehicleId) {
	const encounterList = [];
	let weightSum = 0;
	for(const encounter of $cgmzTemp.getVehicleEncounterList(vehicleId)) {
		encounterList.push(encounter);
		weightSum += encounter.weight;
	}
	if(weightSum > 0) {
		let value = Math.randomInt(weightSum);
		for(const encounter of encounterList) {
			value -= encounter.weight;
			if(value < 0) {
				return encounter.troop;
			}
		}
	}
	return 0;
};
//-----------------------------------------------------------------------------
// Undo 1/2 encounter rate for ship
//-----------------------------------------------------------------------------
const alias_CGMZ_VehicleEncounters_encounterProgressValue = Game_Player.prototype.encounterProgressValue;
Game_Player.prototype.encounterProgressValue = function() {
	let val = alias_CGMZ_VehicleEncounters_encounterProgressValue.call(this);
	if(this.isInShip()) val *= 2;
	return val;
};
//=============================================================================
// Sprite_Battleback
//-----------------------------------------------------------------------------
// Show battlebacks for vehicle encounters
//=============================================================================
//-----------------------------------------------------------------------------
// Show different battleback1 depending on vehicle
//-----------------------------------------------------------------------------
const alias_CGMZ_VehicleEncounters_battleback1Name = Sprite_Battleback.prototype.battleback1Name
Sprite_Battleback.prototype.battleback1Name = function() {
	if($gamePlayer.isInBoat() && !CGMZ.VehicleEncounters.BoatAnimatedBack) {
		return CGMZ.VehicleEncounters.BoatBattleback1;
	} else if($gamePlayer.isInShip() && !CGMZ.VehicleEncounters.ShipAnimatedBack) {
		return CGMZ.VehicleEncounters.ShipBattleback1;
	} else if($gamePlayer.isInAirship() && !CGMZ.VehicleEncounters.AirshipAnimatedBack) {
		return CGMZ.VehicleEncounters.AirshipBattleback1;
	} else {
		return alias_CGMZ_VehicleEncounters_battleback1Name.call(this);
	}
};
//-----------------------------------------------------------------------------
// Show different battleback2 depending on vehicle
//-----------------------------------------------------------------------------
const alias_CGMZ_VehicleEncounters_battleback2Name = Sprite_Battleback.prototype.battleback2Name
Sprite_Battleback.prototype.battleback2Name = function() {
	if($gamePlayer.isInBoat() && !CGMZ.VehicleEncounters.BoatAnimatedBack) {
		return CGMZ.VehicleEncounters.BoatBattleback2;
	} else if($gamePlayer.isInShip() && !CGMZ.VehicleEncounters.ShipAnimatedBack) {
		return CGMZ.VehicleEncounters.ShipBattleback2;
	} else if($gamePlayer.isInAirship() && !CGMZ.VehicleEncounters.AirshipAnimatedBack) {
		return CGMZ.VehicleEncounters.AirshipBattleback2;
	} else {
		return alias_CGMZ_VehicleEncounters_battleback2Name.call(this);
	}
};
if(Imported.CGMZ_AnimatedBattleBacks) {
//=============================================================================
// Game_Map
//-----------------------------------------------------------------------------
// Get animated battle back when on vehicle
//=============================================================================
//-----------------------------------------------------------------------------
// Get the animated battle back id
//-----------------------------------------------------------------------------
const alias_CGMZVehicleEncounters_GameMap_CGMZ_animatedBattleBackId = Game_Map.prototype.CGMZ_animatedBattleBackId;
Game_Map.prototype.CGMZ_animatedBattleBackId = function() {
	if($gamePlayer.isInVehicle()) {
		if($gamePlayer.isInBoat() && CGMZ.VehicleEncounters.BoatAnimatedBack) {
			return CGMZ.VehicleEncounters.BoatAnimatedBack;
		} else if($gamePlayer.isInShip() && CGMZ.VehicleEncounters.ShipAnimatedBack) {
			return CGMZ.VehicleEncounters.ShipAnimatedBack;
		} else if($gamePlayer.isInAirship() && CGMZ.VehicleEncounters.AirshipAnimatedBack) {
			return CGMZ.VehicleEncounters.AirshipAnimatedBack;
		}
	}
	return alias_CGMZVehicleEncounters_GameMap_CGMZ_animatedBattleBackId.call(this);
};
}