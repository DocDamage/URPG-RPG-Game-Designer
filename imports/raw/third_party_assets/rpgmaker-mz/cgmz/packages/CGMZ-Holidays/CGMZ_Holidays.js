/*:
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/holidays/
 * @target MZ
 * @plugindesc Allows you to add weekly, monthly, or annual repeating events.
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
 * Description: This plugin allows you to create daily, weekly, monthly, or
 * annual repeating events. It will turn a switch ON or OFF depending on if
 * the time is within the start/stop time set by you. Some uses may be daily
 * quests, christmas, or any other holiday.
 * ----------------------------------------------------------------------------
 * Documentation:
 * By default, the plugin checks each holiday every time a new map is entered.
 * If you require more frequent checking, you can do so via plugin command.
 * --------------------------Holiday Switch------------------------------------
 * The switch property for each holiday will be turned ON if the current time
 * is within the range provided. If the current time is outside of the range
 * provided in the plugin parameters, the switch will be OFF.
 * -------------------------Holiday Variable-----------------------------------
 * The holiday variable will be set to 1 immediately when the Holiday starts,
 * and will increase by 1 for every hour afterward. For example, if it is 45
 * minutes after start time the variable will still be 1, however if it is 1
 * hour 15 minutes after start time the variable will be 2.
 *
 * Once the end time passes, the variable will be reset to 0. This is done for
 * convenience, if you are not planning to use a switch, you can instead use
 * the variable to check if the holiday is active by checking if it is 0 or
 * not.
 * -------------------------Plugin Commands------------------------------------
 * The following plugin commands are supported:
 * 
 * • Check Holidays
 * Checks holidays for activation / deactivation of holiday switches
 * ------------------------------Saved Games-----------------------------------
 * This plugin is fully compatible with saved games. This means you can:
 *
 * ✓ Add this plugin to a saved game and it will work as expected
 * ✓ Change any plugin params and changes will be reflected in saved games
 * ✓ Remove the plugin with no issue to save data
 * -----------------------------Filename---------------------------------------
 * The filename for this plugin MUST remain CGMZ_Holidays.js
 * This is what it comes as when downloaded. The filename is used to load
 * parameters and execute plugin commands. If you change it, things will begin
 * behaving incorrectly and your game will probably crash. Please do not
 * rename the js file.
 * -------------------------Latest Version-------------------------------------
 * Hi all, this latest version adds options to control when holidays are
 * checked automatically. Previously, this plugin would check holidays on a
 * map being loaded automatically. You can now turn this off, if desired. You
 * can also additionally check holidays when a saved game is loaded, though
 * this is redundant if using the map holiday check so it is off by default.
 *
 * Version 1.2.0
 * - Added options for when holidays are automatically checked
 *
 * @command Check Holidays
 * @desc Checks holidays to see if any should start/end.
 *
 * @param Holidays
 *
 * @param Daily Events
 * @parent Holidays
 * @type struct<DailyEvent>[]
 * @default []
 * @desc Set up daily events here. These repeat once per day
 *
 * @param Weekly Events
 * @parent Holidays
 * @type struct<WeeklyEvent>[]
 * @default []
 * @desc Set up weekly events here. These repeat once per week
 *
 * @param Monthly Events
 * @parent Holidays
 * @type struct<MonthlyEvent>[]
 * @default []
 * @desc Set up monthly events here. These repeat once per month
 *
 * @param Annual Events
 * @parent Holidays
 * @type struct<AnnualEvent>[]
 * @default []
 * @desc Set up annual events here. These repeat once per year
 *
 * @param Mechanics
 *
 * @param Map Checks Holidays
 * @parent Mechanics
 * @type boolean
 * @default true
 * @desc If true, holidays are automatically checked every time a map loads.
 *
 * @param Load Checks Holidays
 * @parent Mechanics
 * @type boolean
 * @default false
 * @desc If true, holidays are automatically checked every time a save loads.
*/
/*~struct~DailyEvent:
 * @param Switch
 * @type switch
 * @default 0
 * @desc Switch to turn on/off depending on event time.
 * 
 * @param Variable
 * @type variable
 * @default 0
 * @desc Variable that will start counting up during the holiday time
 *
 * @param Start Hour
 * @type number
 * @min 0
 * @max 24
 * @default 0
 * @desc Hour of the event start/stop (24hr clock)
 *
 * @param End Hour
 * @type number
 * @min 0
 * @max 24
 * @default 0
 * @desc Hour of the event start/stop (24hr clock)
*/
/*~struct~WeeklyEvent:
 * @param Switch
 * @type switch
 * @default 0
 * @desc Switch to turn on/off depending on event time.
 * 
 * @param Variable
 * @type variable
 * @default 0
 * @desc Variable that will start counting up during the holiday time
 *
 * @param Start
 * @type struct<WeekTime>
 * @default {"Week Day":"0","Hour":"0"}
 * @desc Weekday to start. Sunday = 0, Saturday = 6
 *
 * @param End
 * @type struct<WeekTime>
 * @default {"Week Day":"0","Hour":"0"}
 * @desc Weekday to stop. Sunday = 0, Saturday = 6
*/
/*~struct~WeekTime:
 * @param Week Day
 * @type select
 * @option Sunday
 * @value 0
 * @option Monday
 * @value 1
 * @option Tuesday
 * @value 2
 * @option Wednesday
 * @value 3
 * @option Thursday
 * @value 4
 * @option Friday
 * @value 5
 * @option Saturday
 * @value 6
 * @default 0
 * @desc Weekday to start/stop.
 *
 * @param Hour
 * @type number
 * @min 0
 * @max 23
 * @default 0
 * @desc Hour of the event start/stop (24hr clock)
*/
/*~struct~MonthlyEvent:
 * @param Switch
 * @type switch
 * @default 0
 * @desc Switch to turn on/off depending on event time.
 * 
 * @param Variable
 * @type variable
 * @default 0
 * @desc Variable that will start counting up during the holiday time
 *
 * @param Start
 * @type struct<MonthlyTime>
 * @default {"Day":"1","Hour":"0"}
 * @desc Start time of the monthly event
 *
 * @param End
 * @type struct<MonthlyTime>
 * @default {"Day":"1","Hour":"0"}
 * @desc End time of the monthly event
*/
/*~struct~MonthlyTime:
 * @param Day
 * @type number
 * @min 1
 * @max 31
 * @default 1
 * @desc Day of the holiday start/stop
 *
 * @param Hour
 * @type number
 * @min 0
 * @max 23
 * @default 0
 * @desc Hour of the holiday start/stop (24hr clock)
*/
/*~struct~AnnualEvent:
 * @param Switch
 * @type switch
 * @default 0
 * @desc Switch to turn on/off depending on event time.
 * 
 * @param Variable
 * @type variable
 * @default 0
 * @desc Variable that will start counting up during the holiday time
 *
 * @param Start
 * @type struct<AnnualTime>
 * @default {"Month":"1","Day":"1","Hour":"0"}
 * @desc End time of the annual event
 *
 * @param End
 * @type struct<AnnualTime>
 * @default {"Month":"1","Day":"1","Hour":"0"}
 * @desc Start time of the annual event
*/
/*~struct~AnnualTime:
 * @param Month
 * @type number
 * @min 1
 * @max 12
 * @default 1
 * @desc Month of the event start/stop
 *
 * @param Day
 * @type number
 * @min 1
 * @max 31
 * @default 1
 * @desc Day of the event start/stop
 *
 * @param Hour
 * @type number
 * @min 0
 * @max 23
 * @default 0
 * @desc Hour of the event start/stop (24hr clock)
*/
/*:es
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/holidays/
 * @target MZ
 * @plugindesc Te permite agregar eventos repetitivos semanales, mensuales o anuales.
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
 * Versión: 1.2.0
 * ----------------------------------------------------------------------------
 * Compatibilidad: Sólo probado con mis CGMZ plugins.
 * Hecho para RPG Maker MZ 1.10.0
 * ----------------------------------------------------------------------------
 * Descripción: este complemento le permite crear eventos repetitivos diarios, 
 * semanales, mensuales o anuales. Encenderá o apagará un interruptor
 * dependiendo de si la hora está dentro de la hora de inicio/parada
 * establecida por usted. Algunos usos pueden ser misiones diarias, Navidad o
 * cualquier otra festividad.
 * ----------------------------------------------------------------------------
 * Documentación:
 * De forma predeterminada, el plugin verifica cada día festivo cada vez que 
 * se ingresa un nuevo mapa. Si necesita una verificación más frecuente, puede 
 * hacerlo a través del comando del complemento.
 *
 * El plugin del complemento verifica las vacaciones si es verdadero. No tiene
 * funcionalidad si es falso.
 * -------------------------Plugin Commands------------------------------------
 * The following plugin commands are supported:
 * 
 * • Check Holidays
 * Checks holidays for activation / deactivation of holiday switches
 * ------------------------------Saved Games-----------------------------------
 * This plugin is fully compatible with saved games. This means you can:
 *
 * ✓ Add this plugin to a saved game and it will work as expected
 * ✓ Change any plugin params and changes will be reflected in saved games
 * ✓ Remove the plugin with no issue to save data
 * -----------------------------Filename---------------------------------------
 * The filename for this plugin MUST remain CGMZ_Holidays.js
 * This is what it comes as when downloaded. The filename is used to load
 * parameters and execute plugin commands. If you change it, things will begin
 * behaving incorrectly and your game will probably crash. Please do not
 * rename the js file.
 * -------------------------Latest Version-------------------------------------
 * Hi all, this latest version adds options to control when holidays are
 * checked automatically. Previously, this plugin would check holidays on a
 * map being loaded automatically. You can now turn this off, if desired. You
 * can also additionally check holidays when a saved game is loaded, though
 * this is redundant if using the map holiday check so it is off by default.
 *
 * Version 1.2.0
 * - Added options for when holidays are automatically checked
 *
 * @command Check Holidays
 * @text Consultar días festivos
 * @desc Comprueba las vacaciones si es cierto. Sin funcionalidad si es falso.
 *
 * @param Holidays
 * @text Días festivos
 *
 * @param Daily Events
 * @text Eventos diarios
 * @parent Holidays
 * @type struct<DailyEvent>[]
 * @default []
 * @desc Configura los eventos diarios aquí. Estos se repiten una vez al día.
 *
 * @param Weekly Events
 * @text Eventos semanales
 * @parent Holidays
 * @type struct<WeeklyEvent>[]
 * @default []
 * @desc Configura eventos semanales aquí. Estos se repiten una vez por semana.
 *
 * @param Monthly Events
 * @text Eventos mensuales
 * @parent Holidays
 * @type struct<MonthlyEvent>[]
 * @default []
 * @desc Configura eventos mensuales aquí. Estos se repiten una vez al mes.
 *
 * @param Annual Events
 * @text Eventos anuales
 * @parent Holidays
 * @type struct<AnnualEvent>[]
 * @default []
 * @desc Configura eventos anuales aquí. Estos se repiten una vez al año.
 *
 * @param Mechanics
 *
 * @param Map Checks Holidays
 * @parent Mechanics
 * @type boolean
 * @default true
 * @desc If true, holidays are automatically checked every time a map loads.
 *
 * @param Load Checks Holidays
 * @parent Mechanics
 * @type boolean
 * @default false
 * @desc If true, holidays are automatically checked every time a save loads.
*/
/*~struct~DailyEvent:es
 * @param Switch
 * @text Interruptor
 * @type switch
 * @default 0
 * @desc Interruptor para encender/apagar según la hora del evento.
 * 
 * @param Variable
 * @type variable
 * @default 0
 * @desc Variable that will start counting up during the holiday time
 *
 * @param Start Hour
 * @text Hora de inicio
 * @type number
 * @min 0
 * @max 24
 * @default 0
 * @desc Hora de inicio/fin del evento (reloj de 24 horas).
 *
 * @param End Hour
 * @text Hora de finalización
 * @type number
 * @min 0
 * @max 24
 * @default 0
 * @desc Hora de inicio/fin del evento (reloj de 24 horas).
*/
/*~struct~WeeklyEvent:es
 * @param Switch
 * @text Interruptor
 * @type switch
 * @default 0
 * @desc Interruptor para encender/apagar según la hora del evento.
 * 
 * @param Variable
 * @type variable
 * @default 0
 * @desc Variable that will start counting up during the holiday time
 *
 * @param Start
 * @text Inicio
 * @type struct<WeekTime>
 * @default {"Week Day":"0","Hour":"0"}
 * @desc Día de la semana para empezar. domingo = 0, sábado = 6.
 *
 * @param End
 * @text Final
 * @type struct<WeekTime>
 * @default {"Week Day":"0","Hour":"0"}
 * @desc Día de la semana para parar. domingo = 0, sábado = 6.
*/
/*~struct~WeekTime:es
 * @param Week Day
 * @text Día de la semana
 * @type select
 * @option domingo
 * @value 0
 * @option lunes
 * @value 1
 * @option martes
 * @value 2
 * @option miércoles
 * @value 3
 * @option jueves
 * @value 4
 * @option viernes
 * @value 5
 * @option sábado
 * @value 6
 * @default 0
 * @desc Día de la semana para iniciar/detener.
 *
 * @param Hour
 * @text Hora
 * @type number
 * @min 0
 * @max 23
 * @default 0
 * @desc Hora de inicio/fin del evento (reloj de 24 horas).
*/
/*~struct~MonthlyEvent:es
 * @param Switch
 * @text Interruptor
 * @type switch
 * @default 0
 * @desc Interruptor para encender/apagar según la hora del evento.
 * 
 * @param Variable
 * @type variable
 * @default 0
 * @desc Variable that will start counting up during the holiday time
 *
 * @param Start
 * @text Inicio
 * @type struct<MonthlyTime>
 * @default {"Day":"1","Hour":"0"}
 * @desc Hora de inicio del evento mensual.
 *
 * @param End
 * @text Final
 * @type struct<MonthlyTime>
 * @default {"Day":"1","Hour":"0"}
 * @desc Hora de finalización del evento mensual.
*/
/*~struct~MonthlyTime:es
 * @param Day
 * @text Día
 * @type number
 * @min 1
 * @max 31
 * @default 1
 * @desc Día de inicio/fin de vacaciones.
 *
 * @param Hour
 * @text Hora
 * @type number
 * @min 0
 * @max 23
 * @default 0
 * @desc Hora de inicio/fin de vacaciones (reloj de 24 horas).
*/
/*~struct~AnnualEvent:es
 * @param Switch
 * @text Interruptor
 * @type switch
 * @default 0
 * @desc Interruptor para encender/apagar según la hora del evento.
 * 
 * @param Variable
 * @type variable
 * @default 0
 * @desc Variable that will start counting up during the holiday time
 *
 * @param Start
 * @text Inicio
 * @type struct<AnnualTime>
 * @default {"Month":"1","Day":"1","Hour":"0"}
 * @desc Hora de finalización del evento anual.
 *
 * @param End
 * @text Final
 * @type struct<AnnualTime>
 * @default {"Month":"1","Day":"1","Hour":"0"}
 * @desc Hora de inicio del evento anual.
*/
/*~struct~AnnualTime:es
 * @param Month
 * @text Mes
 * @type number
 * @min 1
 * @max 12
 * @default 1
 * @desc Mes de inicio/fin del evento.
 *
 * @param Day
 * @text Día
 * @type number
 * @min 1
 * @max 31
 * @default 1
 * @desc Día de inicio/fin del evento.
 *
 * @param Hour
 * @text Hora
 * @type number
 * @min 0
 * @max 23
 * @default 0
 * @desc Hora de inicio/fin del evento (reloj de 24 horas).
*/
Imported.CGMZ_Holidays = true;
CGMZ.Versions["Holidays"] = "1.2.0";
CGMZ.Holidays = {};
CGMZ.Holidays.parameters = PluginManager.parameters('CGMZ_Holidays');
CGMZ.Holidays.MapChecksHolidays = (CGMZ.Holidays.parameters["Map Checks Holidays"] === 'true');
CGMZ.Holidays.LoadChecksHolidays = (CGMZ.Holidays.parameters["Load Checks Holidays"] === 'true');
CGMZ.Holidays.DailyEvents = CGMZ_Utils.parseJSON(CGMZ.Holidays.parameters["Daily Events"], [], "[CGMZ] Holidays", "Your Daily Events parameter was set up incorrectly and could not be read.");
CGMZ.Holidays.WeeklyEvents = CGMZ_Utils.parseJSON(CGMZ.Holidays.parameters["Weekly Events"], [], "[CGMZ] Holidays", "Your Weekly Events parameter was set up incorrectly and could not be read.");
CGMZ.Holidays.MonthlyEvents = CGMZ_Utils.parseJSON(CGMZ.Holidays.parameters["Monthly Events"], [], "[CGMZ] Holidays", "Your Monthly Events parameter was set up incorrectly and could not be read.");
CGMZ.Holidays.AnnualEvents = CGMZ_Utils.parseJSON(CGMZ.Holidays.parameters["Annual Events"], [], "[CGMZ] Holidays", "Your Annual Events parameter was set up incorrectly and could not be read.");
//=============================================================================
// CGMZ_Holiday
//-----------------------------------------------------------------------------
// Class that stores the data associated with holidays
//=============================================================================
function CGMZ_Holiday(holiday) {
	this.initialize.apply(this, arguments);
}
//-----------------------------------------------------------------------------
// Initialize Data
//-----------------------------------------------------------------------------
CGMZ_Holiday.prototype.initialize = function(holiday) {
	this._switchId = Number(holiday["Switch"]);
	this._variableId = Number(holiday["Variable"]);
	this._active = false;
};
//-----------------------------------------------------------------------------
// Get current active/inactive status
//-----------------------------------------------------------------------------
CGMZ_Holiday.prototype.isActive = function() {
	return this._active;
};
//-----------------------------------------------------------------------------
// Set whether the holiday is active or not
//-----------------------------------------------------------------------------
CGMZ_Holiday.prototype.setActiveStatus = function(activeStatus) {
	this._active = $gameSwitches.value(this._switchId);
	if(activeStatus && !this.isActive()) {
		this.activate();
	} else if(!activeStatus && this.isActive()){
		this.deactivate();
	}
};
//-----------------------------------------------------------------------------
// Activate the Holiday
//-----------------------------------------------------------------------------
CGMZ_Holiday.prototype.activate = function() {
	this._active = true;
	$gameSwitches.setValue(this._switchId, this._active);
	$gameVariables.setValue(this._variableId, 1);
};
//-----------------------------------------------------------------------------
// Deactivate the Holiday
//-----------------------------------------------------------------------------
CGMZ_Holiday.prototype.deactivate = function() {
	this._active = false;
	$gameSwitches.setValue(this._switchId, this._active);
	$gameVariables.setValue(this._variableId, 0);
};
//-----------------------------------------------------------------------------
// Check time for the holiday, activate if ready, deactivate if not ready
//-----------------------------------------------------------------------------
CGMZ_Holiday.prototype.checkTime = function() {
	//
};
//=============================================================================
// CGMZ_DailyHoliday
//-----------------------------------------------------------------------------
// Class that represents a daily holiday
//=============================================================================
function CGMZ_DailyHoliday(holiday) {
	this.initialize.apply(this, arguments);
}
CGMZ_DailyHoliday.prototype = Object.create(CGMZ_Holiday.prototype);
CGMZ_DailyHoliday.prototype.constructor = CGMZ_DailyHoliday;
//-----------------------------------------------------------------------------
// Initialize Data for a daily event
//-----------------------------------------------------------------------------
CGMZ_DailyHoliday.prototype.initialize = function(holiday) {
	CGMZ_Holiday.prototype.initialize.call(this, holiday);
	this._startHour = Number(holiday["Start Hour"]);
	this._endHour = Number(holiday["End Hour"]);
};
//-----------------------------------------------------------------------------
// Check time for the holiday, activate if ready, deactivate if not ready
//-----------------------------------------------------------------------------
CGMZ_DailyHoliday.prototype.checkTime = function() {
	const currentTime = new Date();
	const currentHour = currentTime.getHours();
	const currentYear = currentTime.getFullYear();
	const currentMonth = currentTime.getMonth();
	const currentDay = currentTime.getDate();
	const startTime = new Date(currentYear, currentMonth, currentDay, this._startHour);
	const stopTime = new Date(currentYear, currentMonth, currentDay, this._endHour);
	this.setActiveStatus(currentTime > startTime && currentTime < stopTime);
	if(this._active) {
		const dif = currentTime - startTime;
		const difHours = Math.floor(dif / (1000 * 60 * 60));
		$gameVariables.setValue(this._variableId, difHours + 1);
	}
};
//=============================================================================
// CGMZ_WeeklyHoliday
//-----------------------------------------------------------------------------
// Class that represents a weekly holiday
//=============================================================================
function CGMZ_WeeklyHoliday(holiday) {
	this.initialize.apply(this, arguments);
}
CGMZ_WeeklyHoliday.prototype = Object.create(CGMZ_Holiday.prototype);
CGMZ_WeeklyHoliday.prototype.constructor = CGMZ_WeeklyHoliday;
//-----------------------------------------------------------------------------
// Initialize Data for a weekly event
//-----------------------------------------------------------------------------
CGMZ_WeeklyHoliday.prototype.initialize = function(holiday) {
	CGMZ_Holiday.prototype.initialize.call(this, holiday);
	const def = {"Hour":1,"Week Day":1};
	const start = CGMZ_Utils.parseJSON(holiday.Start, def, "[CGMZ] Holidays", "A weekly holiday had invalid JSON in its start parameter.");
	const end = CGMZ_Utils.parseJSON(holiday.End, def, "[CGMZ] Holidays", "A weekly holiday had invalid JSON in its end parameter.");
	this._startHour = Number(start.Hour);
	this._startWeekDay = Number(start["Week Day"]);
	this._endHour = Number(end.Hour);
	this._endWeekDay = Number(end["Week Day"]);
};
//-----------------------------------------------------------------------------
// Check time for the holiday, activate if ready, deactivate if not ready
//-----------------------------------------------------------------------------
CGMZ_WeeklyHoliday.prototype.checkTime = function() {
	const currentTime = new Date();
	const currentWeekday = currentTime.getDay();
	const currentHour = currentTime.getHours();
	const currentYear = currentTime.getFullYear();
	const currentMonth = currentTime.getMonth();
	// Set the dates to the weekday + 1 from start of month (because can't have 0th date of month).
	// This will not be a correct date but should be accurate to determine if the time is within a weekly range
	currentTime.setDate(currentWeekday+1);
	const startDate = new Date(currentYear, currentMonth, this._startWeekDay+1, this._startHour);
	const stopDate = new Date(currentYear, currentMonth, this._endWeekDay+1, this._endHour);
	this.setActiveStatus(currentTime > startDate && currentTime < stopDate);
	if(this._active) {
		const dif = currentTime - startDate;
		const difHours = Math.floor(dif / (1000 * 60 * 60));
		$gameVariables.setValue(this._variableId, difHours + 1);
	}
};
//=============================================================================
// CGMZ_MonthlyHoliday
//-----------------------------------------------------------------------------
// Class that represents a monthly holiday
//=============================================================================
function CGMZ_MonthlyHoliday(holiday) {
	this.initialize.apply(this, arguments);
}
CGMZ_MonthlyHoliday.prototype = Object.create(CGMZ_Holiday.prototype);
CGMZ_MonthlyHoliday.prototype.constructor = CGMZ_MonthlyHoliday;
//-----------------------------------------------------------------------------
// Initialize Data for a monthly event
//-----------------------------------------------------------------------------
CGMZ_MonthlyHoliday.prototype.initialize = function(holiday) {
	CGMZ_Holiday.prototype.initialize.call(this, holiday);
	const def = {"Hour":1,"Day":1};
	const start = CGMZ_Utils.parseJSON(holiday.Start, def, "[CGMZ] Holidays", "A monthly holiday had invalid JSON in its start parameter.");
	const end = CGMZ_Utils.parseJSON(holiday.End, def, "[CGMZ] Holidays", "A monthly holiday had invalid JSON in its end parameter.");
	this._startHour = Number(start.Hour);
	this._startDay = Number(start.Day);
	this._endHour = Number(end.Hour);
	this._endDay = Number(end.Day);
};
//-----------------------------------------------------------------------------
// Check time for the holiday, activate if ready, deactivate if not ready
//-----------------------------------------------------------------------------
CGMZ_MonthlyHoliday.prototype.checkTime = function() {
	const currentTime = new Date();
	const currentYear = currentTime.getFullYear();
	const currentMonth = currentTime.getMonth();
	const startDate = new Date(currentYear, currentMonth, this._startDay, this._startHour);
	const stopDate = new Date(currentYear, currentMonth, this._endDay, this._endHour);
	this.setActiveStatus(currentTime > startDate && currentTime < stopDate);
	if(this._active) {
		const dif = currentTime - startDate;
		const difHours = Math.floor(dif / (1000 * 60 * 60));
		$gameVariables.setValue(this._variableId, difHours + 1);
	}
};
//=============================================================================
// CGMZ_AnnualHoliday
//-----------------------------------------------------------------------------
// Class that represents an annual holiday
//=============================================================================
function CGMZ_AnnualHoliday(holiday) {
	this.initialize.apply(this, arguments);
}
CGMZ_AnnualHoliday.prototype = Object.create(CGMZ_Holiday.prototype);
CGMZ_AnnualHoliday.prototype.constructor = CGMZ_AnnualHoliday;
//-----------------------------------------------------------------------------
// Initialize Data for an annual event
//-----------------------------------------------------------------------------
CGMZ_AnnualHoliday.prototype.initialize = function(holiday) {
	CGMZ_Holiday.prototype.initialize.call(this, holiday);
	const def = {"Hour":1,"Day":1,"Month":1};
	const start = CGMZ_Utils.parseJSON(holiday.Start, def, "[CGMZ] Holidays", "An annual holiday had invalid JSON in its start parameter.");
	const end = CGMZ_Utils.parseJSON(holiday.End, def, "[CGMZ] Holidays", "An annual holiday had invalid JSON in its end parameter.");;
	this._startHour = Number(start.Hour);
	this._startDay = Number(start.Day);
	this._startMonth = Number(start.Month) - 1;
	this._endHour = Number(end.Hour);
	this._endDay = Number(end.Day);
	this._endMonth = Number(end.Month) - 1;
};
//-----------------------------------------------------------------------------
// Check time for the holiday, activate if ready, deactivate if not ready
//-----------------------------------------------------------------------------
CGMZ_AnnualHoliday.prototype.checkTime = function() {
	const currentTime = new Date();
	const currentYear = currentTime.getFullYear();
	const startDate = new Date(currentYear, this._startMonth, this._startDay, this._startHour);
	const stopDate = new Date(currentYear, this._endMonth, this._endDay, this._endHour);
	this.setActiveStatus(currentTime > startDate && currentTime < stopDate);
	if(this._active) {
		const dif = currentTime - startDate;
		const difHours = Math.floor(dif / (1000 * 60 * 60));
		$gameVariables.setValue(this._variableId, difHours + 1);
	}
};
//=============================================================================
// CGMZ_Temp
//-----------------------------------------------------------------------------
// Create holiday data
//=============================================================================
//-----------------------------------------------------------------------------
// Create Region Switch data
//-----------------------------------------------------------------------------
const alias_CGMZHolidays_CGMZTemp_createPluginData = CGMZ_Temp.prototype.createPluginData;
CGMZ_Temp.prototype.createPluginData = function() {
	alias_CGMZHolidays_CGMZTemp_createPluginData.call(this);
	this._holidays = [];
	for(const dayJSON of CGMZ.Holidays.DailyEvents) {
		const dayInfo = CGMZ_Utils.parseJSON(dayJSON, null, "[CGMZ] Holidays", "One of your daily holidays was set up incorrectly and could not be read.");
		if(!dayInfo) continue;
		const holiday = new CGMZ_DailyHoliday(dayInfo);
		this._holidays.push(holiday);
	}
	for(const weekJSON of CGMZ.Holidays.WeeklyEvents) {
		const weekInfo = CGMZ_Utils.parseJSON(weekJSON, null, "[CGMZ] Holidays", "One of your weekly holidays was set up incorrectly and could not be read.");
		if(!weekInfo) continue;
		const holiday = new CGMZ_WeeklyHoliday(weekInfo);
		this._holidays.push(holiday);
	}
	for(const monthJSON of CGMZ.Holidays.MonthlyEvents) {
		const monthInfo = CGMZ_Utils.parseJSON(monthJSON, null, "[CGMZ] Holidays", "One of your monthly holidays was set up incorrectly and could not be read.");
		if(!monthInfo) continue;
		const holiday = new CGMZ_MonthlyHoliday(monthInfo);
		this._holidays.push(holiday);
	}
	for(const yearJSON of CGMZ.Holidays.AnnualEvents) {
		const yearInfo = CGMZ_Utils.parseJSON(yearJSON, null, "[CGMZ] Holidays", "One of your annual holidays was set up incorrectly and could not be read.");
		if(!yearInfo) continue;
		const holiday = new CGMZ_AnnualHoliday(yearInfo);
		this._holidays.push(holiday);
	}
};
//-----------------------------------------------------------------------------
// Register Plugin Commands
//-----------------------------------------------------------------------------
const alias_CGMZHolidays_CGMZTemp_registerPluginCommands = CGMZ_Temp.prototype.registerPluginCommands;
CGMZ_Temp.prototype.registerPluginCommands = function() {
	alias_CGMZHolidays_CGMZTemp_registerPluginCommands.call(this);
	PluginManager.registerCommand("CGMZ_Holidays", "Check Holidays", this.pluginCommandCheckHolidays);
};
//-----------------------------------------------------------------------------
// Plugin Command - Check Holidays
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandCheckHolidays = function() {
	$cgmzTemp.checkHolidays();
};
//-----------------------------------------------------------------------------
// Check Holidays
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.checkHolidays = function() {
	for(const holiday of this._holidays) {
		holiday.checkTime();
	}
};
//=============================================================================
// CGMZ_Core
//-----------------------------------------------------------------------------
// Check holidays after load if set up
//=============================================================================
//-----------------------------------------------------------------------------
// Check holidays after load
//-----------------------------------------------------------------------------
const alias_CGMZHolidays_CGMZCore_onAfterLoad = CGMZ_Core.prototype.onAfterLoad;
CGMZ_Core.prototype.onAfterLoad = function() {
	alias_CGMZHolidays_CGMZCore_onAfterLoad.call(this);
	if(CGMZ.Holidays.LoadChecksHolidays) $cgmzTemp.checkHolidays();
};
//=============================================================================
// Game_Map
//-----------------------------------------------------------------------------
// Check on map setup for holidays if set up
//=============================================================================
//-----------------------------------------------------------------------------
// Check holidays on map setup
//-----------------------------------------------------------------------------
const alias_CGMZHolidays_GameMap_setup = Game_Map.prototype.setup;
Game_Map.prototype.setup = function(mapId) {
	alias_CGMZHolidays_GameMap_setup.call(this, mapId);
	if(CGMZ.Holidays.MapChecksHolidays) $cgmzTemp.checkHolidays();
};