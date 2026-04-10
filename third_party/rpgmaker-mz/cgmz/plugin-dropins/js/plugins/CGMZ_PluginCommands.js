/*:
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/plugincommands/
 * @target MZ
 * @base CGMZ_Core
 * @orderAfter CGMZ_Core
 * @plugindesc Adds plugin commands meant to complement event commands
 * @help
 * ============================================================================
 * For terms and conditions using this plugin in your game please visit:
 * https://www.caspergaming.com/terms-of-use/
 * ============================================================================
 * Become a Patron to get access to beta/alpha plugins plus other goodies!
 * https://www.patreon.com/CasperGamingRPGM
 * ============================================================================
 * Version: 1.8.0
 * ----------------------------------------------------------------------------
 * Compatibility: Only tested with my CGMZ plugins.
 * Made for RPG Maker MZ 1.10.0
 * ----------------------------------------------------------------------------
 * Description: Adds some plugin commands that are meant to make some things
 * with the default editor easier, such as turning a switch on/off if an event
 * is within certain xy coordinates or controlling self switches for any event
 * ----------------------------------------------------------------------------
 * Documentation:
 * -------------------------Plugin Commands------------------------------------
 * • Control Switch Random
 * This command will set the provided switch to either ON or OFF randomly.
 *
 * • Control Switch Region ID
 * This command will set the provided switch to ON or OFF if the given event
 * is in the given region
 * 
 * • Control Switch Terrain Tag
 * This command will set the provided switch to ON or OFF if the given event
 * is in the given terrain tag
 * 
 * • Control Switch Coordinates
 * This command will set the provided switch to ON or OFF if the given event
 * is in the given x, y coordinates
 * 
 * • Control Switch Save Access
 * This command will set the provided switch to ON or OFF if the player has
 * save access
 * 
 * • Control Switch Menu Access
 * This command will set the provided switch to ON or OFF if the player has
 * menu access
 * 
 * • Control Switch Formation Access
 * This command will set the provided switch to ON or OFF if the player has
 * formation access
 * 
 * • Control Switch Encounters
 * This command will set the provided switch to ON or OFF if encounters are
 * enabled
 * 
 * • Control Switch Self Switch
 * This command will set the provided switch to ON or OFF if the given self
 * switch is ON or OFF
 * 
 * • Control Self Switch
 * This command will set the provided self switch to ON or OFF
 * 
 * • Control Timer Add
 * This command will add the given amount of seconds to the timer
 * 
 * • Control Timer Subtract
 * This command will subtract the given amount of seconds to the timer
 * 
 * • Call Scene
 * This command will call the provided scene
 * 
 * • Request Autosave
 * This command will request an autosave (does not guarantee the save succeeds)
 * 
 * • Control Variable String
 * This command will set the provided variable to the provided text
 * 
 * • Control Variable Float
 * This command will set the provided variable to the provided float number
 * 
 * • Conditional Branch Variable
 * This command will check a variable against a provided constant
 * 
 * • Select Weapon
 * Like the select item event command, but for weapons.
 * 
 * • Select Armor
 * Like the select item event command, but for armors.
 * 
 * • Select Skill
 * Like the select item event command, but for skills.
 * 
 * • Change Weather Power
 * Add or subtract from the current weather power.
 * 
 * • Change Tint
 * Add or subtract from the current screen tint.
 * 
 * • Stop All Sound
 * Stops all currently playing BGM/BGS/SE/ME.
 * 
 * • Erase All Pictures
 * Erases all currently shown pictures.
 * 
 * • Change Battle Lose
 * Enable or disable gameover when random battle is lost
 * 
 * • Wait (Range)
 * Wait for a random amount within a given range
 * 
 * • Wait (Variable)
 * Wait for an amount equal to the value of a game variable
 * 
 * • Wait (Input)
 * Wait for the player to press OK input (gamepad / keyboard / touch)
 * 
 * • Get BGM Pos
 * Get the current BGM position into a variable
 * 
 * • Get BGS Pos
 * Get the current BGS position into a variable
 * 
 * • Change Option
 * Change touch ui, always dash, or command remember for the player
 * 
 * • Change Volume Option
 * Change bgm, bgs, me, or se volume for the player
 * ------------------------------Saved Games-----------------------------------
 * This plugin is fully compatible with saved games. This means you can:
 *
 * ✓ Add this plugin to a saved game and it will work as expected
 * ✓ Change any plugin params and changes will be reflected in saved games
 * ✓ Remove the plugin with no issue to save data
 * -----------------------------Filename---------------------------------------
 * The filename for this plugin MUST remain CGMZ_PluginCommands.js
 * This is what it comes as when downloaded. The filename is used to load
 * parameters and execute plugin commands. If you change it, things will begin
 * behaving incorrectly and your game will probably crash. Please do not
 * rename the js file.
 * --------------------------Latest Version------------------------------------
 * Hi all, this latest version adds a plugin command to change the weather
 * power and screen tint values by adding or subtracting from the current
 * value. By default, you can only set a new power (for example, snow 6 or
 * snow 9). Sometimes, you may want to add or subtract from this value, and
 * you may not always know what to set it to without some complex event or 
 * scripting.
 *
 * These new plugin commands allow you to simple add or subtract 1, 2, 3, etc
 * from the current weather value. The tint plugin command allows you to add
 * or subtract red, green, blue, or gray. The tints have safety checks to
 * ensure you do not go out of range (-255 to 255 or 0 to 255 for gray). The
 * weather has a safety check to ensure you do not go below 0, but you can
 * exceed the default limit of weather power 9 by adding more to your weather
 * if you want.
 *
 * Version 1.8.0
 * - Added plugin command to add or subtract weather power
 * - Added plugin command to add or subtract screen tint values
 *
 * @command --- Switch Commands ---
 * @desc Not a real command, just a separator.
 *
 * @command controlSwitchRandom
 * @text Control Switch: Random
 * @desc Sets the provided switch ID ON/OFF randomly
 *
 * @arg switchId
 * @type switch
 * @text Switch ID
 * @desc The switch ID to be randomly set ON/OFF
 * @default 1
 *
 * @command controlSwitchRegion
 * @text Control Switch: Region ID
 * @desc Sets the provided switch ID ON/OFF based on if the event is in the region
 *
 * @arg switchId
 * @type switch
 * @text Switch ID
 * @desc The switch ID to be set ON/OFF
 * @default 1
 *
 * @arg regionId
 * @type number
 * @text Region ID
 * @desc The regionId to check for
 * @default 1
 * @min 0
 *
 * @arg eventId
 * @type number
 * @text Event ID
 * @desc The event to check for. Special values: -1 = player, 0 = this event.
 * @default -1
 * @min -1
 *
 * @command controlSwitchTerrain
 * @text Control Switch: Terrain Tag
 * @desc Sets the provided switch ID ON/OFF based on if the event is in the terrain tag
 *
 * @arg switchId
 * @type switch
 * @text Switch ID
 * @desc The switch ID to be set ON/OFF
 * @default 1
 *
 * @arg tagId
 * @type number
 * @text Terrain Tag ID
 * @desc The terrain tag to check for
 * @default 1
 * @min 0
 *
 * @arg eventId
 * @type number
 * @text Event ID
 * @desc The event to check for. Special values: -1 = player, 0 = this event.
 * @default -1
 * @min -1
 *
 * @command controlSwitchXy
 * @text Control Switch: Coordinates
 * @desc Sets the provided switch ID ON/OFF based on if the event is at x, y coordinates
 *
 * @arg switchId
 * @type switch
 * @text Switch ID
 * @desc The switch ID to be set ON/OFF
 * @default 1
 *
 * @arg x
 * @type number
 * @text X Coordinate
 * @desc The x coordinate to check for
 * @default 0
 * @min 0
 *
 * @arg y
 * @type number
 * @text Y Coordinate
 * @desc The y coordinate to check for
 * @default 0
 * @min 0
 *
 * @arg eventId
 * @type number
 * @text Event ID
 * @desc The event to check for. Special values: -1 = player, 0 = this event.
 * @default -1
 * @min -1
 *
 * @command controlSwitchMenu
 * @text Control Switch: Menu Access
 * @desc Sets the provided switch ID ON/OFF based on if the player has access to the menu
 *
 * @arg switchId
 * @type switch
 * @text Switch ID
 * @desc The switch ID to be set ON/OFF based on if the player has access to the menu
 * @default 1
 *
 * @command controlSwitchSave
 * @text Control Switch: Save Access
 * @desc Sets the provided switch ID ON/OFF based on if the player has access to saving
 *
 * @arg switchId
 * @type switch
 * @text Switch ID
 * @desc The switch ID to be set ON/OFF based on if the player has access to saving
 * @default 1
 *
 * @command controlSwitchFormation
 * @text Control Switch: Formation Access
 * @desc Sets the provided switch ID ON/OFF based on if the player can change formation
 *
 * @arg switchId
 * @type switch
 * @text Switch ID
 * @desc The switch ID to be set ON/OFF based on if the player can change formation
 * @default 1
 *
 * @command controlSwitchEncounters
 * @text Control Switch: Encounters
 * @desc Sets the provided switch ID ON/OFF based on if encounters are enabled/disabled
 *
 * @arg switchId
 * @type switch
 * @text Switch ID
 * @desc The switch ID to be set ON/OFF based on if encounters are enabled/disabled
 * @default 1
 *
 * @command controlSwitchSelfSwitch
 * @text Control Switch: Self Switch
 * @desc Sets the provided switch ON/OFF based on the provided Self Switch value
 *
 * @arg switchId
 * @type switch
 * @text Switch ID
 * @desc The switch ID to be set ON/OFF based on the provided self switch
 * @default 1
 *
 * @arg switchName
 * @type select
 * @option A
 * @option B
 * @option C
 * @option D
 * @text Self Switch
 * @desc The self switch to be set ON/OFF
 * @default A
 *
 * @arg mapId
 * @type number
 * @text Map ID
 * @desc The map ID of the event
 * @default 1
 * @min 1
 *
 * @arg eventId
 * @type number
 * @text Event ID
 * @desc The event with the self switch
 * @default 1
 * @min 1
 *
 * @command --- Variable Commands ---
 * @desc Not a real command, just a separator.
 *
 * @command controlVariableString
 * @text Control Variable: String
 * @desc Sets the provided variable ID to the provided string
 *
 * @arg variableId
 * @type variable
 * @text Variable ID
 * @desc The variable ID to be set
 * @default 1
 *
 * @arg paramText
 * @text String
 * @desc The string to set the variable to
 *
 * @command controlVariableFloat
 * @text Control Variable: Float
 * @desc Sets the provided variable ID to the provided float (decimal) number
 *
 * @arg variableId
 * @type variable
 * @text Variable ID
 * @desc The variable ID to be set
 * @default 1
 *
 * @arg paramFloat
 * @type number
 * @decimals 3
 * @text Float
 * @desc The float (decimal number) to set the variable to
 * @default 0.000
 *
 * @command conditionalBranchVariable
 * @text Conditional Branch: Variable
 * @desc checks if the provided variable is equal to some value
 *
 * @arg variableId
 * @type variable
 * @text Variable ID
 * @desc The variable ID to check
 * @default 1
 *
 * @arg switchId
 * @type switch
 * @text Switch ID
 * @desc The switch to set ON/OFF based on the result of the check
 * @default 1
 *
 * @arg paramValue
 * @text Param Value
 * @desc The value to compare the variable with
 *
 * @arg paramType
 * @type select
 * @option String
 * @option Float
 * @text Param Type
 * @desc The type of the param value to expect
 * @default String
 *
 * @command --- Self Switch Commands ---
 * @desc Not a real command, just a separator.
 *
 * @command controlSelfSwitch
 * @text Control Self Switch
 * @desc Sets the provided self switch ON/OFF
 *
 * @arg switchName
 * @type select
 * @option A
 * @option B
 * @option C
 * @option D
 * @text Self Switch
 * @desc The self switch to be set ON/OFF
 * @default A
 *
 * @arg value
 * @type boolean
 * @text Value
 * @desc Whether to set the self switch ON or OFF
 * @default true
 *
 * @arg mapId
 * @type number
 * @text Map ID
 * @desc The map ID of the event
 * @default 1
 * @min 1
 *
 * @arg eventId
 * @type number
 * @text Event ID
 * @desc The event to check for.
 * @default 1
 * @min 1
 *
 * @command --- Timer Commands ---
 * @desc Not a real command, just a separator.
 *
 * @command controlTimerAdd
 * @text Control Timer: Add Seconds
 * @desc Add seconds to the timer
 *
 * @arg seconds
 * @type number
 * @text Seconds
 * @desc Amount of seconds to add to the timer
 * @default 1
 * @min 0
 *
 * @command controlTimerSubtract
 * @text Control Timer: Subtract Seconds
 * @desc Subtract seconds from the timer
 *
 * @arg seconds
 * @type number
 * @text Seconds
 * @desc Amount of seconds to subtract from the timer
 * @default 1
 * @min 0
 *
 * @command --- Select Commands ---
 * @desc Not a real command, just a separator.
 *
 * @command Select Weapon
 * @desc Allows the player to select a weapon from possession and stores the id in a variable.
 *
 * @arg variableId
 * @type variable
 * @text Variable ID
 * @desc The variable ID to store the weapon ID in
 * @default 1
 *
 * @command Select Armor
 * @desc Allows the player to select an armor from possession and stores the id in a variable.
 *
 * @arg variableId
 * @type variable
 * @text Variable ID
 * @desc The variable ID to store the armor ID in
 * @default 1
 *
 * @command Select Skill
 * @desc Allows the player to select a skill and stores the id in a variable.
 *
 * @arg variableId
 * @type variable
 * @text Variable ID
 * @desc The variable ID to store the armor ID in
 * @default 1
 *
 * @command --- Screen Commands ---
 * @desc Not a real command, just a separator.
 *
 * @command Change Weather Power
 * @desc A way to change the power of the weather by addition / subtraction
 *
 * @arg Power Change
 * @type number
 * @default 0
 * @min -9999
 * @desc The amount to change the current weather power by
 *
 * @arg Duration
 * @type number
 * @default 1
 * @min 0
 * @desc The time (in frames) the change will take to complete
 *
 * @arg Wait
 * @type boolean
 * @default false
 * @desc If true, event command processing will wait for power change to finish
 *
 * @command Change Tint
 * @desc A way to change the tint by adding / subtracting from current tint
 *
 * @arg Red
 * @type number
 * @default 0
 * @min -9999
 * @desc The amount to change the current tint red value by
 *
 * @arg Green
 * @type number
 * @default 0
 * @min -9999
 * @desc The amount to change the current tint green value by
 *
 * @arg Blue
 * @type number
 * @default 0
 * @min -9999
 * @desc The amount to change the current tint blue value by
 *
 * @arg Gray
 * @type number
 * @default 0
 * @min -9999
 * @desc The amount to change the current tint gray value by
 *
 * @arg Duration
 * @type number
 * @default 1
 * @min 0
 * @desc The time (in frames) the change will take to complete
 *
 * @arg Wait
 * @type boolean
 * @default false
 * @desc If true, event command processing will wait for tint change to finish
 *
 * @command --- Convenience Commands ---
 * @desc Not a real command, just a separator.
 *
 * @command Stop All Sound
 * @desc Stops all bgm/bgs/me/se in one command.
 *
 * @command Erase All Pictures
 * @desc Erases all pictures being shown
 *
 * @command --- Battle Commands ---
 * @desc Not a real command, just a separator.
 *
 * @command Change Battle Lose
 * @desc Change if the player game overs if they lose in random battle
 *
 * @arg Allow Lose
 * @type boolean
 * @desc If true, the player will no longer game over if they lose a random battle
 * @default true
 *
 * @command --- Wait Commands ---
 * @desc Not a real command, just a separator.
 *
 * @command Wait (Range)
 * @desc Wait for a random amount within a given range
 *
 * @arg Min
 * @type number
 * @default 1
 * @desc Minimum amount of frames to wait
 *
 * @arg Max
 * @type number
 * @default 1
 * @desc Maximum amount of frames to wait
 *
 * @command Wait (Variable)
 * @desc Wait for an amount of frames equal to a game variable's value
 *
 * @arg Variable
 * @type variable
 * @default 0
 * @desc The variable that stores the amount of frames to wait
 *
 * @command Wait (Input)
 * @desc Wait for OK input (touch or keyboard or gamepad)
 *
 * @command --- Audio Commands ---
 * @desc Not a real command, just a separator.
 *
 * @command Get BGM Pos
 * @desc Get the position of the current BGM in a variable.
 *
 * @arg Variable
 * @type variable
 * @default 0
 * @desc The variable that stores the current BGM position
 *
 * @command Get BGS Pos
 * @desc Get the position of the current BGS in a variable.
 *
 * @arg Variable
 * @type variable
 * @default 0
 * @desc The variable that stores the current BGS position
 *
 * @command --- Option Commands ---
 * @desc Not a real command, just a separator.
 *
 * @command Change Option
 * @desc Change an option for the player
 *
 * @arg Option
 * @type select
 * @option Always Dash
 * @option Touch UI
 * @option Command Remember
 * @default Always Dash
 * @desc The option to change
 *
 * @arg Value
 * @type boolean
 * @default true
 * @desc The new value of the option
 *
 * @command Change Volume Option
 * @desc Change a volume option for the player
 *
 * @arg Option
 * @type select
 * @option BGM Volume
 * @option BGS Volume
 * @option ME Volume
 * @option SE Volume
 * @default BGM Volume
 * @desc The option to change
 *
 * @arg Value
 * @type number
 * @min 0
 * @max 100
 * @default 100
 * @desc The new value of the option
 *
 * @command --- Misc Commands ---
 * @desc Not a real command, just a separator.
 *
 * @command callScene
 * @text Call Scene
 * @desc Go to a specific game scene
 *
 * @arg scene
 * @text Scene
 * @desc The scene to call
 * @default Scene_Item
 *
 * @command autosave
 * @text Autosave
 * @desc Requests an autosave (may still fail)
*/
/*:es
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/plugincommands/
 * @target MZ
 * @base CGMZ_Core
 * @orderAfter CGMZ_Core
 * @plugindesc Agrega comandos de plugin destinados a complementar los comandos de eventos
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
 * Versión: 1.8.0
 * ----------------------------------------------------------------------------
 * Compatibilidad: Sólo probado con mis CGMZ plugins.
 * Hecho para RPG Maker MZ 1.10.0
 * ----------------------------------------------------------------------------
 * Descripción: Agrega algunos comandos de complemento que están destinados a 
 * facilitar algunas cosas con el editor predeterminado, como encender/apagar 
 * un interruptor si un evento está dentro de ciertas coordenadas xy o
 * controlar los interruptores automáticos para cualquier evento.
 * ----------------------------------------------------------------------------
 * Documentación:
 * -----------------------Comandos de Plugin-----------------------------------
 * • Interruptor de control aleatorio
 * Este comando activará o desactivará aleatoriamente el interruptor
 * proporcionado.
 * 
 * • ID de región del interruptor de control
 * Este comando activará o desactivará el interruptor provisto si el evento
 * dado está en la región dada.
 * 
 * • Etiqueta de terreno del interruptor de control
 * Este comando activará o desactivará el interruptor provisto si el evento
 * dado está en la etiqueta de terreno dada.
 * 
 * • Coordenadas del interruptor de control
 * Este comando activará o desactivará el interruptor provisto si el evento
 * dado está en las coordenadas x, y dadas
 * 
 * • Botón de control para guardar acceso
 * Este comando activará o desactivará el interruptor proporcionado si el
 * jugador tiene acceso para guardar.
 * 
 * • Acceso al menú del interruptor de control
 * Este comando activará o desactivará el interruptor provisto si el jugador
 * tiene acceso al menú.
 * 
 * • Acceso a la formación del interruptor de control
 * Este comando activará o desactivará el interruptor provisto si el jugador
 * tiene acceso a la formación.
 * 
 * • Encuentros de interruptor de control
 * Este comando activará o desactivará el interruptor provisto si los
 * encuentros están habilitados.
 * 
 * • Interruptor automático del interruptor de control
 * Este comando activará o desactivará el interruptor proporcionado si el
 * interruptor automático dado está activado o desactivado.
 * 
 * • Interruptor automático de control
 * Este comando configurará el interruptor automático proporcionado en ON
 * o OFF.
 * 
 * • Agregar temporizador de control
 * Este comando agregará la cantidad dada de segundos al temporizador.
 * 
 * • Restar temporizador de control
 * Este comando restará la cantidad dada de segundos al temporizador.
 * 
 * • Escena de llamada
 * Este comando llamará a la escena proporcionada.
 * 
 * • Solicitar guardado automático
 * Este comando solicitará un guardado automático (no garantiza que el
 * guardado sea exitoso).
 * 
 * • Cadena de variable de control
 * Este comando establecerá la variable proporcionada el texto proporcionado.
 * 
 * • Variable de control flotante
 * Este comando establecerá la variable proporcionada en el número flotante
 * proporcionado.
 * 
 * • Variable de rama condicional
 * Este comando comparará una variable con una constante proporcionada.
 * 
 * • Seleccionar arma
 * Como el comando de evento de selección de elemento, pero para armas.
 * 
 * • Seleccionar armadura
 * Como el comando de evento de elemento seleccionado, pero para armaduras.
 * 
 * • Seleccionar habilidad
 * Como el comando de evento de elemento seleccionado, pero para habilidades.
 * 
 * • Change Weather Power
 * Add or subtract from the current weather power.
 * 
 * • Change Tint
 * Add or subtract from the current screen tint.
 * 
 * • Stop All Sound
 * Stops all currently playing BGM/BGS/SE/ME.
 * 
 * • Erase All Pictures
 * Erases all currently shown pictures.
 * 
 * • Change Battle Lose
 * Enable or disable gameover when random battle is lost
 * 
 * • Wait (Range)
 * Wait for a random amount within a given range
 * 
 * • Wait (Variable)
 * Wait for an amount equal to the value of a game variable
 * 
 * • Wait (Input)
 * Wait for the player to press OK input (gamepad / keyboard / touch)
 * 
 * • Get BGM Pos
 * Get the current BGM position into a variable
 * 
 * • Get BGS Pos
 * Get the current BGS position into a variable
 * 
 * • Change Option
 * Change touch ui, always dash, or command remember for the player
 * 
 * • Change Volume Option
 * Change bgm, bgs, me, or se volume for the player
 * --------------------------Latest Version------------------------------------
 * Hi all, this latest version adds a plugin command to change the weather
 * power and screen tint values by adding or subtracting from the current
 * value. By default, you can only set a new power (for example, snow 6 or
 * snow 9). Sometimes, you may want to add or subtract from this value, and
 * you may not always know what to set it to without some complex event or 
 * scripting.
 *
 * These new plugin commands allow you to simple add or subtract 1, 2, 3, etc
 * from the current weather value. The tint plugin command allows you to add
 * or subtract red, green, blue, or gray. The tints have safety checks to
 * ensure you do not go out of range (-255 to 255 or 0 to 255 for gray). The
 * weather has a safety check to ensure you do not go below 0, but you can
 * exceed the default limit of weather power 9 by adding more to your weather
 * if you want.
 *
 * Version 1.8.0
 * - Added plugin command to add or subtract weather power
 * - Added plugin command to add or subtract screen tint values
 *
 * @command --- Switch Commands ---
 * @desc Not a real command, just a separator.
 *
 * @command controlSwitchRandom
 * @text Interruptor de control: Aleatorio
 * @desc Establece el ID de interruptor proporcionado en ON/OFF aleatoriamente.
 *
 * @arg switchId
 * @type switch
 * @text ID de interruptor
 * @desc El ID del interruptor se activará/desactivará aleatoriamente.
 * @default 1
 *
 * @command controlSwitchRegion
 * @text Interruptor de control: ID de región
 * @desc Establece el ID de interruptor proporcionado en ON/OFF en función de si el evento está en la región.
 *
 * @arg switchId
 * @type switch
 * @text ID de interruptor
 * @desc El ID del interruptor que se activará/desactivará.
 * @default 1
 *
 * @arg regionId
 * @type number
 * @text ID de la región
 * @desc El Id de region para verificar.
 * @default 1
 * @min 0
 *
 * @arg eventId
 * @type number
 * @text Id de evento
 * @desc El evento a verificar. Valores especiales: -1 = jugador, 0 = este evento.
 * @default -1
 * @min -1
 *
 * @command controlSwitchTerrain
 * @text Interruptor de control: etiqueta de terreno
 * @desc Establece el ID de interruptor proporcionado en ON/OFF en función de si el evento está en la etiqueta de terreno.
 *
 * @arg switchId
 * @type switch
 * @text ID de interruptor
 * @desc El ID del interruptor que se activará/desactivará.
 * @default 1
 *
 * @arg tagId
 * @type number
 * @text ID de etiqueta de terreno
 * @desc La etiqueta de terreno para verificar.
 * @default 1
 * @min 0
 *
 * @arg eventId
 * @type number
 * @text ID de Evento
 * @desc El evento a verificar. Valores especiales: -1 = jugador, 0 = este evento.
 * @default -1
 * @min -1
 *
 * @command controlSwitchXy
 * @text Interruptor de control: coordenadas
 * @desc Establece el ID de interruptor proporcionado en ON/OFF en función de si el evento está en las coordenadas x, y.
 *
 * @arg switchId
 * @type switch
 * @text ID de interruptor
 * @desc El ID del interruptor que se activará/desactivará.
 * @default 1
 *
 * @arg x
 * @type number
 * @text Coordenada X
 * @desc La coordenada X a verificar.
 * @default 0
 * @min 0
 *
 * @arg y
 * @type number
 * @text Coordenada Y
 * @desc La coordenada Y para verificar.
 * @default 0
 * @min 0
 *
 * @arg eventId
 * @type number
 * @text ID de Evento
 * @desc El evento a verificar. Valores especiales: -1 = jugador, 0 = este evento.
 * @default -1
 * @min -1
 *
 * @command controlSwitchMenu
 * @text Interruptor de control: acceso al menú
 * @desc Establece la ID del interruptor proporcionado en ON/OFF en función de si el jugador tiene acceso al menú.
 *
 * @arg switchId
 * @type switch
 * @text ID de interruptor
 * @desc El ID del interruptor que se activará/desactivará en función de si el jugador tiene acceso al menú.
 * @default 1
 *
 * @command controlSwitchSave
 * @text Interruptor de control: guardar acceso
 * @desc Establece el ID de interruptor proporcionado en ON/OFF en función de si el jugador tiene acceso a guardar.
 *
 * @arg switchId
 * @type switch
 * @text ID de interruptor
 * @desc El ID del interruptor que se activará/desactivará en función de si el jugador tiene acceso para guardar.
 * @default 1
 *
 * @command controlSwitchFormation
 * @text Interruptor de control: Acceso a la formación
 * @desc Establece el ID de interruptor proporcionado en ON/OFF en función de si el jugador puede cambiar de formación.
 *
 * @arg switchId
 * @type switch
 * @text ID de interruptor
 * @desc El ID del interruptor que se activará/desactivará en función de si el jugador puede cambiar de formación.
 * @default 1
 *
 * @command controlSwitchEncounters
 * @text Interruptor de control: Encuentros
 * @descEstablece el ID de interruptor proporcionado en ON/OFF en función de si los encuentros están habilitados/deshabilitados.
 *
 * @arg switchId
 * @type switch
 * @text ID de interruptor
 * @desc El ID del interruptor que se activará/desactivará en función de si los encuentros están activados/desactivados.
 * @default 1
 *
 * @command controlSwitchSelfSwitch
 * @text Interruptor de control: interruptor automático
 * @desc Establece el interruptor proporcionado ON/OFF en función del valor de Self Switch (interruptor automatico) proporcionado.
 *
 * @arg switchId
 * @type switch
 * @text ID de interruptor
 * @desc El ID del interruptor que se activará/desactivará en función del interruptor automático proporcionado.
 * @default 1
 *
 * @arg switchName
 * @type select
 * @option A
 * @option B
 * @option C
 * @option D
 * @text Interruptor automático
 * @desc El interruptor automático que se debe configurar en ON/OFF.
 * @default A
 *
 * @arg mapId
 * @type number
 * @text ID de Map
 * @desc El ID del mapa del evento.
 * @default 1
 * @min 1
 *
 * @arg eventId
 * @type number
 * @text ID de Evento
 * @desc El evento con el interruptor automático.
 * @default 1
 * @min 1
  *
 * @command --- Variable Commands ---
 * @desc Not a real command, just a separator.
 *
 * @command controlVariableString
 * @text Variable de control: Cadena
 * @desc Establece el ID de variable proporcionado en la cadena proporcionada.
 *
 * @arg variableId
 * @type variable
 * @text ID de Variable
 * @desc El ID de la variable que se establecerá.
 * @default 1
 *
 * @arg paramText
 * @text Cadena
 * @desc La cadena en la cual establecer la variable.
 *
 * @command controlVariableFloat
 * @text Variable de control: Flotante
 * @desc Establece el ID de variable proporcionado en el número flotante (decimal) proporcionado.
 *
 * @arg variableId
 * @type variable
 * @text ID de Variable
 * @desc El ID de la variable que se establecerá.
 * @default 1
 *
 * @arg paramFloat
 * @type number
 * @decimals 3
 * @text Flotante
 * @desc El flotante (número decimal) en el cual establecer la variable.
 * @default 0.000
 *
 * @command conditionalBranchVariable
 * @text Rama Condicional: Variable
 * @desc Comprueba si la variable proporcionada es igual a algún valor.
 *
 * @arg variableId
 * @type variable
 * @text ID de Variable
 * @desc El ID de la variable a comprobar.
 * @default 1
 *
 * @arg switchId
 * @type switch
 * @text ID de interruptor
 * @desc El interruptor para configurar ON/OFF basado en el resultado de la verificación.
 * @default 1
 *
 * @arg paramValue
 * @text Valor de parámetro
 * @desc El valor con el que comparar la variable.
 *
 * @arg paramType
 * @type select
 * @option String
 * @option Float
 * @text Tipo de parámetro
 * @desc El tipo del valor del parámetro que se espera.
 * @default String
 *
 * @command --- Self Switch Commands ---
 * @desc Not a real command, just a separator.
 *
 * @command controlSelfSwitch
 * @text Interruptor automático de control
 * @desc Establece el interruptor automático provisto ON/OFF
 *
 * @arg switchName
 * @type select
 * @option A
 * @option B
 * @option C
 * @option D
 * @text Interruptor automático
 * @desc El interruptor automático que se debe configurar en ON/OFF.
 * @default A
 *
 * @arg value
 * @type boolean
 * @text Valor
 * @desc Ya sea para activar o desactivar el interruptor automático.
 * @default true
 *
 * @arg mapId
 * @type number
 * @text ID de Mapa
 * @desc El ID del mapa del evento.
 * @default 1
 * @min 1
 *
 * @arg eventId
 * @type number
 * @text ID de Evento
 * @desc El evento a verificar.
 * @default 1
 * @min 1
 *
 * @command --- Timer Commands ---
 * @desc Not a real command, just a separator.
 *
 * @command controlTimerAdd
 * @text Temporizador de control: agregar segundos
 * @desc Agregar segundos al temporizador.
 *
 * @arg seconds
 * @type number
 * @text Segundos
 * @desc Cantidad de segundos para agregar al temporizador.
 * @default 1
 * @min 0
 *
 * @command controlTimerSubtract
 * @text Temporizador de control: restar segundos
 * @desc Restar segundos del temporizador.
 *
 * @arg seconds
 * @type number
 * @text Segundos
 * @desc Cantidad de segundos a restar del temporizador.
 * @default 1
 * @min 0
 *
 * @command --- Select Commands ---
 * @desc Not a real command, just a separator.
 *
 * @command Select Weapon
 * @text Seleccionar arma
 * @desc Le permite al jugador seleccionar un arma de la posesión y almacena la identificación en una variable.
 *
 * @arg variableId
 * @type variable
 * @text ID de Variable
 * @desc La identificación de la variable en la cual almacenar la identificación del arma.
 * @default 1
 *
 * @command Select Armor
 * @text Seleccionar armadura
 * @desc Le permite al jugador seleccionar una armadura de posesión y almacena la identificación en una variable.
 *
 * @arg variableId
 * @type variable
 * @text ID de Variable
 * @desc El ID de la variable en la cual almacenar el ID de la armadura.
 * @default 1
 *
 * @command Select Skill
 * @text Seleccionar habilidad
 * @desc Le ermite al jugador seleccionar una habilidad y almacena la identificación en una variable.
 *
 * @arg variableId
 * @type variable
 * @text ID de Variable
 * @desc El ID de la variable en la cual almacenar el ID de la armadura.
 * @default 1
 *
 * @command --- Screen Commands ---
 * @desc Not a real command, just a separator.
 *
 * @command Change Weather Power
 * @desc A way to change the power of the weather by addition / subtraction
 *
 * @arg Power Change
 * @type number
 * @default 0
 * @min -9999
 * @desc The amount to change the current weather power by
 *
 * @arg Duration
 * @type number
 * @default 1
 * @min 0
 * @desc The time (in frames) the change will take to complete
 *
 * @arg Wait
 * @type boolean
 * @default false
 * @desc If true, event command processing will wait for power change to finish
 *
 * @command Change Tint
 * @desc A way to change the tint by adding / subtracting from current tint
 *
 * @arg Red
 * @type number
 * @default 0
 * @min -9999
 * @desc The amount to change the current tint red value by
 *
 * @arg Green
 * @type number
 * @default 0
 * @min -9999
 * @desc The amount to change the current tint green value by
 *
 * @arg Blue
 * @type number
 * @default 0
 * @min -9999
 * @desc The amount to change the current tint blue value by
 *
 * @arg Gray
 * @type number
 * @default 0
 * @min -9999
 * @desc The amount to change the current tint gray value by
 *
 * @arg Duration
 * @type number
 * @default 1
 * @min 0
 * @desc The time (in frames) the change will take to complete
 *
 * @arg Wait
 * @type boolean
 * @default false
 * @desc If true, event command processing will wait for tint change to finish
 *
 * @command --- Convenience Commands ---
 * @desc Not a real command, just a separator.
 *
 * @command Stop All Sound
 * @desc Stops all bgm/bgs/me/se in one command.
 *
 * @command Erase All Pictures
 * @desc Erases all pictures being shown
 *
 * @command --- Battle Commands ---
 * @desc Not a real command, just a separator.
 *
 * @command Change Battle Lose
 * @desc Change if the player game overs if they lose in random battle
 *
 * @arg Allow Lose
 * @type boolean
 * @desc If true, the player will no longer game over if they lose a random battle
 * @default true
 *
 * @command --- Wait Commands ---
 * @desc Not a real command, just a separator.
 *
 * @command Wait (Range)
 * @desc Wait for a random amount within a given range
 *
 * @arg Min
 * @type number
 * @default 1
 * @desc Minimum amount of frames to wait
 *
 * @arg Max
 * @type number
 * @default 1
 * @desc Maximum amount of frames to wait
 *
 * @command Wait (Variable)
 * @desc Wait for an amount of frames equal to a game variable's value
 *
 * @arg Variable
 * @type variable
 * @default 0
 * @desc The variable that stores the amount of frames to wait
 *
 * @command Wait (Input)
 * @desc Wait for OK input (touch or keyboard or gamepad)
 *
 * @command --- Audio Commands ---
 * @desc Not a real command, just a separator.
 *
 * @command Get BGM Pos
 * @desc Get the position of the current BGM in a variable.
 *
 * @arg Variable
 * @type variable
 * @default 0
 * @desc The variable that stores the current BGM position
 *
 * @command Get BGS Pos
 * @desc Get the position of the current BGS in a variable.
 *
 * @arg Variable
 * @type variable
 * @default 0
 * @desc The variable that stores the current BGS position
 *
 * @command --- Option Commands ---
 * @desc Not a real command, just a separator.
 *
 * @command Change Option
 * @desc Change an option for the player
 *
 * @arg Option
 * @type select
 * @option Always Dash
 * @option Touch UI
 * @option Command Remember
 * @default Always Dash
 * @desc The option to change
 *
 * @arg Value
 * @type boolean
 * @default true
 * @desc The new value of the option
 *
 * @command Change Volume Option
 * @desc Change a volume option for the player
 *
 * @arg Option
 * @type select
 * @option BGM Volume
 * @option BGS Volume
 * @option ME Volume
 * @option SE Volume
 * @default BGM Volume
 * @desc The option to change
 *
 * @arg Value
 * @type number
 * @min 0
 * @max 100
 * @default 100
 * @desc The new value of the option
 *
 * @command --- Misc Commands ---
 * @desc Not a real command, just a separator.
 *
 * @command callScene
 * @text LLamar escena
 * @desc Ir a una escena de juego específica.
 *
 * @arg scene
 * @text Escena
 * @desc La escena a llamar.
 * @default Scene_Item
 *
 * @command autosave
 * @text Guardado automático
 * @desc Solicita un guardado automático (todavía puede fallar).
*/
Imported.CGMZ_PluginCommands = true;
CGMZ.Versions["Plugin Commands"] = "1.8.0";
//=============================================================================
// CGMZ
//-----------------------------------------------------------------------------
// Handle saved data
//=============================================================================
//-----------------------------------------------------------------------------
// Also initialize plugin command data
//-----------------------------------------------------------------------------
const alias_CGMZ_PluginCommands_CGMZCore_createPluginData = CGMZ_Core.prototype.createPluginData;
CGMZ_Core.prototype.createPluginData = function() {
	alias_CGMZ_PluginCommands_CGMZCore_createPluginData.call(this);
	this.initializePluginCommandData();
};
//-----------------------------------------------------------------------------
// Initialize Plugin Command data
//-----------------------------------------------------------------------------
CGMZ_Core.prototype.initializePluginCommandData = function() {
	if(typeof this.pluginCommandAllowBattleLose === 'undefined') {
		this.pluginCommandAllowBattleLose = false;
	}
};
//-----------------------------------------------------------------------------
// Load new plugin command data after load
//-----------------------------------------------------------------------------
const alias_CGMZ_PluginCommands_CGMZCore_createAfterLoad = CGMZ_Core.prototype.createAfterLoad;
CGMZ_Core.prototype.createAfterLoad = function() {
	alias_CGMZ_PluginCommands_CGMZCore_createAfterLoad.call(this);
	this.initializePluginCommandData();
};
//=============================================================================
// CGMZ_Temp
//-----------------------------------------------------------------------------
// Registration and processing for new plugin commands
//=============================================================================
//-----------------------------------------------------------------------------
// Register Plugin Commands
//-----------------------------------------------------------------------------
const alias_CGMZ_PluginCommands_registerPluginCommands = CGMZ_Temp.prototype.registerPluginCommands;
CGMZ_Temp.prototype.registerPluginCommands = function() {
	alias_CGMZ_PluginCommands_registerPluginCommands.call(this);
	PluginManager.registerCommand("CGMZ_PluginCommands", "controlSwitchRandom", this.pluginCommandPluginCommandsControlSwitchRandom);
	PluginManager.registerCommand("CGMZ_PluginCommands", "controlSwitchRegion", this.pluginCommandPluginCommandsControlSwitchRegion);
	PluginManager.registerCommand("CGMZ_PluginCommands", "controlSwitchTerrain", this.pluginCommandPluginCommandsControlSwitchTerrain);
	PluginManager.registerCommand("CGMZ_PluginCommands", "controlSwitchXy", this.pluginCommandPluginCommandsControlSwitchXy);
	PluginManager.registerCommand("CGMZ_PluginCommands", "controlSwitchMenu", this.pluginCommandPluginCommandsControlSwitchMenu);
	PluginManager.registerCommand("CGMZ_PluginCommands", "controlSwitchSave", this.pluginCommandPluginCommandsControlSwitchSave);
	PluginManager.registerCommand("CGMZ_PluginCommands", "controlSwitchFormation", this.pluginCommandPluginCommandsControlSwitchFormation);
	PluginManager.registerCommand("CGMZ_PluginCommands", "controlSwitchEncounters", this.pluginCommandPluginCommandsControlSwitchEncounters);
	PluginManager.registerCommand("CGMZ_PluginCommands", "controlSwitchSelfSwitch", this.pluginCommandPluginCommandsControlSwitchSelfSwitch);
	PluginManager.registerCommand("CGMZ_PluginCommands", "controlSelfSwitch", this.pluginCommandPluginCommandsControlSelfSwitch);
	PluginManager.registerCommand("CGMZ_PluginCommands", "controlTimerAdd", this.pluginCommandPluginCommandsControlTimerAdd);
	PluginManager.registerCommand("CGMZ_PluginCommands", "controlTimerSubtract", this.pluginCommandPluginCommandsControlTimerSubtract);
	PluginManager.registerCommand("CGMZ_PluginCommands", "callScene", this.pluginCommandPluginCommandsCallScene);
	PluginManager.registerCommand("CGMZ_PluginCommands", "autosave", this.pluginCommandPluginCommandsAutosave);
	PluginManager.registerCommand("CGMZ_PluginCommands", "controlVariableFloat", this.pluginCommandPluginCommandsControlVariableFloat);
	PluginManager.registerCommand("CGMZ_PluginCommands", "controlVariableString", this.pluginCommandPluginCommandsControlVariableString);
	PluginManager.registerCommand("CGMZ_PluginCommands", "conditionalBranchVariable", this.pluginCommandPluginCommandsConditionalBranchVariable);
	PluginManager.registerCommand("CGMZ_PluginCommands", "Select Weapon", this.pluginCommandPluginCommandsSelectWeapon);
	PluginManager.registerCommand("CGMZ_PluginCommands", "Select Armor", this.pluginCommandPluginCommandsSelectArmor);
	PluginManager.registerCommand("CGMZ_PluginCommands", "Select Skill", this.pluginCommandPluginCommandsSelectSkill);
	PluginManager.registerCommand("CGMZ_PluginCommands", "Change Weather Power", this.pluginCommandPluginCommandsChangeWeatherPower);
	PluginManager.registerCommand("CGMZ_PluginCommands", "Change Tint", this.pluginCommandPluginCommandsChangeTint);
	PluginManager.registerCommand("CGMZ_PluginCommands", "Stop All Sound", this.pluginCommandPluginCommandsStopAllSound);
	PluginManager.registerCommand("CGMZ_PluginCommands", "Erase All Pictures", this.pluginCommandPluginCommandsEraseAllPictures);
	PluginManager.registerCommand("CGMZ_PluginCommands", "Change Battle Lose", this.pluginCommandPluginCommandsChangeBattleLose);
	PluginManager.registerCommand("CGMZ_PluginCommands", "Wait (Range)", this.pluginCommandPluginCommandsWaitRange);
	PluginManager.registerCommand("CGMZ_PluginCommands", "Wait (Variable)", this.pluginCommandPluginCommandsWaitVariable);
	PluginManager.registerCommand("CGMZ_PluginCommands", "Wait (Input)", this.pluginCommandPluginCommandsWaitInput);
	PluginManager.registerCommand("CGMZ_PluginCommands", "Get BGM Pos", this.pluginCommandPluginCommandsGetBGMPos);
	PluginManager.registerCommand("CGMZ_PluginCommands", "Get BGS Pos", this.pluginCommandPluginCommandsGetBGSPos);
	PluginManager.registerCommand("CGMZ_PluginCommands", "Change Option", this.pluginCommandPluginCommandsChangeOption);
	PluginManager.registerCommand("CGMZ_PluginCommands", "Change Volume Option", this.pluginCommandPluginCommandsChangeVolumeOption);
};
//-----------------------------------------------------------------------------
// Sets the provided switch to either be ON or OFF at random
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPluginCommandsControlSwitchRandom = function(args) {
	const switchId = Number(args.switchId);
	const value = (Math.randomInt(2) === 1);
	$gameSwitches.setValue(switchId, value);
};
//-----------------------------------------------------------------------------
// Sets the provided switch to either be ON or OFF by menu access
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPluginCommandsControlSwitchMenu = function(args) {
	const switchId = Number(args.switchId);
	const value = $gameSystem.isMenuEnabled();
	$gameSwitches.setValue(switchId, value);
};
//-----------------------------------------------------------------------------
// Sets the provided switch to either be ON or OFF by save access
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPluginCommandsControlSwitchSave = function(args) {
	const switchId = Number(args.switchId);
	const value = $gameSystem.isSaveEnabled();
	$gameSwitches.setValue(switchId, value);
};
//-----------------------------------------------------------------------------
// Sets the provided switch to either be ON or OFF by formation access
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPluginCommandsControlSwitchFormation = function(args) {
	const switchId = Number(args.switchId);
	const value = $gameSystem.isFormationEnabled();
	$gameSwitches.setValue(switchId, value);
};
//-----------------------------------------------------------------------------
// Sets the provided switch to either be ON or OFF by encounters active
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPluginCommandsControlSwitchEncounters = function(args) {
	const switchId = Number(args.switchId);
	const value = $gameSystem.isEncounterEnabled();
	$gameSwitches.setValue(switchId, value);
};
//-----------------------------------------------------------------------------
// Add seconds to the timer
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPluginCommandsControlTimerAdd = function(args) {
	const seconds = Number(args.seconds) * 60;
	const currentSeconds = ($gameTimer.seconds() + 1) * 60;
	$gameTimer.start(seconds + currentSeconds);
};
//-----------------------------------------------------------------------------
// Subtract seconds from the timer
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPluginCommandsControlTimerSubtract = function(args) {
	const seconds = Number(args.seconds) * 60;
	const currentSeconds = ($gameTimer.seconds() + 1) * 60;
	let setSeconds = currentSeconds - seconds;
	if(setSeconds >= 0) {
		$gameTimer.start(currentSeconds - seconds);
	} else {
		$gameTimer.start(1);
	}
};
//-----------------------------------------------------------------------------
// Call provided scene
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPluginCommandsCallScene = function(args) {
	try {
		eval("SceneManager.push(" + args.scene + ");");
	} catch(e) {
		console.warn("Caught error in Call Scene plugin command: " + e.name + ": " + e.message);
	}
};
//-----------------------------------------------------------------------------
// Request an autosave
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPluginCommandsAutosave = function() {
	try {
		SceneManager._scene.requestAutosave();
	} catch(e) {
		console.warn("Autosave failed: " + e.name + ": " + e.message);
	}
};
//-----------------------------------------------------------------------------
// Sets the provided switch to either be ON or OFF based on the event ID's
// region ID
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPluginCommandsControlSwitchRegion = function(args) {
	const switchId = Number(args.switchId);
	const regionId = Number(args.regionId);
	const eventId = Number(args.eventId);
    const character = this.character(eventId);
	const characterRegion = $gameMap.regionId(character.x, character.y);
	const value = characterRegion === regionId;
	$gameSwitches.setValue(switchId, value);
};
//-----------------------------------------------------------------------------
// Sets the provided switch to either be ON or OFF based on the event ID's
// terrain tag ID
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPluginCommandsControlSwitchTerrain = function(args) {
	const switchId = Number(args.switchId);
	const tagId = Number(args.tagId);
	const eventId = Number(args.eventId);
    const character = this.character(eventId);
	const characterTag = $gameMap.terrainTag(character.x, character.y);
	const value = characterTag === tagId;
	$gameSwitches.setValue(switchId, value);
};
//-----------------------------------------------------------------------------
// Sets the provided switch to either be ON or OFF based on the event ID's
// x, y coordinates
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPluginCommandsControlSwitchXy = function(args) {
	const switchId = Number(args.switchId);
	const x = Number(args.x);
	const y = Number(args.y);
	const eventId = Number(args.eventId);
    const character = this.character(eventId);
	const value = (character.x === x && character.y === y);
	$gameSwitches.setValue(switchId, value);
};
//-----------------------------------------------------------------------------
// Sets the given event's self switch to ON or OFF
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPluginCommandsControlSelfSwitch = function(args) {
	const switchName = args.switchName;
	const eventId = Number(args.eventId);
    const mapId = Number(args.mapId);
	const key = [mapId, eventId, switchName];
	const value = (args.value === "true");
	$gameSelfSwitches.setValue(key, value);
};
//-----------------------------------------------------------------------------
// Sets the given event's self switch to ON or OFF
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPluginCommandsControlSwitchSelfSwitch = function(args) {
	const switchId = Number(args.switchId);
	const switchName = args.switchName;
	const eventId = Number(args.eventId);
    const mapId = Number(args.mapId);
	const key = [mapId, eventId, switchName];
	const value = $gameSelfSwitches.value(key);
	$gameSwitches.setValue(switchId, value);
};
//-----------------------------------------------------------------------------
// Sets the provided variable to the provided string
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPluginCommandsControlVariableString = function(args) {
	const variableId = Number(args.variableId);
	$gameVariables.setValue(variableId, args.paramText);
};
//-----------------------------------------------------------------------------
// Sets the provided variable to the provided float number
// Note: Stores as a string since default game behavior floors numbers.
//       perform float check on evaluation.
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPluginCommandsControlVariableFloat = function(args) {
	const variableId = Number(args.variableId);
	$gameVariables.setValue(variableId, args.paramFloat);
};
//-----------------------------------------------------------------------------
// Checks a variable for non-number data types, sets a switch ON/OFF by result
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPluginCommandsConditionalBranchVariable = function(args) {
	const variableId = Number(args.variableId);
	const variableValue = (args.paramType === 'String') ? $gameVariables.value(variableId) : parseFloat($gameVariables.value(variableId));
	const switchId = Number(args.switchId);
	const paramValue = (args.paramType === 'String') ? args.paramValue : parseFloat(args.paramValue);
	$gameSwitches.setValue(switchId, (paramValue === variableValue));
};
//-----------------------------------------------------------------------------
// Checks a variable for non-number data types, sets a switch ON/OFF by result
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPluginCommandsSelectWeapon = function(args) {
	const variableId = Number(args.variableId);
	const itemType = "weapon";
	const params = [variableId, itemType];
    this.setupItemChoice(params);
    this.setWaitMode("message");
};
//-----------------------------------------------------------------------------
// Checks a variable for non-number data types, sets a switch ON/OFF by result
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPluginCommandsSelectArmor = function(args) {
	const variableId = Number(args.variableId);
	const itemType = "armor";
	const params = [variableId, itemType];
    this.setupItemChoice(params);
    this.setWaitMode("message");
};
//-----------------------------------------------------------------------------
// Checks a variable for non-number data types, sets a switch ON/OFF by result
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPluginCommandsSelectSkill = function(args) {
	const variableId = Number(args.variableId);
	const itemType = "skill";
	const params = [variableId, itemType];
    this.setupItemChoice(params);
    this.setWaitMode("message");
};
//-----------------------------------------------------------------------------
// Plugin Command - Change Weather Power
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPluginCommandsChangeWeatherPower = function(args) {
	const power = ($gameScreen.weatherPower() + Number(args["Power Change"])).clamp(1, 99);
	const duration = Number(args.Duration);
	$gameScreen.changeWeather($gameScreen.weatherType(), power, duration);
    if(args.Wait === 'true') this.wait(duration);
};
//-----------------------------------------------------------------------------
// Plugin Command - Change Tint
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPluginCommandsChangeTint = function(args) {
	const currentTone = $gameScreen.tone();
	const red = (currentTone[0] + Number(args.Red)).clamp(-255, 255);
	const green = (currentTone[1] + Number(args.Green)).clamp(-255, 255);
	const blue = (currentTone[2] + Number(args.Blue)).clamp(-255, 255);
	const gray = (currentTone[3] + Number(args.Gray)).clamp(0, 255);
	const duration = Number(args.Duration);
	$gameScreen.startTint([red, green, blue, gray], duration);
    if(args.Wait === 'true') this.wait(duration);
};
//-----------------------------------------------------------------------------
// Stop all bgm/bgs/me/se
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPluginCommandsStopAllSound = function() {
	AudioManager.stopBgm();
	AudioManager.stopBgs();
	AudioManager.stopMe();
	AudioManager.stopSe();
};
//-----------------------------------------------------------------------------
// Erase all pictures currently being shown
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPluginCommandsEraseAllPictures = function() {
	$gameScreen.clearPictures();
};
//-----------------------------------------------------------------------------
// Change battle lose status
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPluginCommandsChangeBattleLose = function(args) {
	$cgmz.pluginCommandAllowBattleLose = (args["Allow Lose"] === 'true');
};
//-----------------------------------------------------------------------------
// Plugin Command - Wait (Range)
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPluginCommandsWaitRange = function(args) {
	const amount = CGMZ_Utils.randomNumberInRange(Number(args.Min), Number(args.Max));
	this.wait(amount);
};
//-----------------------------------------------------------------------------
// Plugin Command - Wait (Variable)
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPluginCommandsWaitVariable = function(args) {
	const amount = $gameVariables.value(Number(args.Variable));
	this.wait(amount);
};
//-----------------------------------------------------------------------------
// Plugin Command - Wait (Input)
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPluginCommandsWaitInput = function() {
	this.setWaitMode('CGMZ_inputWait');
};
//-----------------------------------------------------------------------------
// Get the current BGM position
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPluginCommandsGetBGMPos = function(args) {
	const varId = Number(args.Variable);
	const pos = AudioManager._bgmBuffer ? AudioManager._bgmBuffer.seek() : 0;
	$gameVariables.setValue(varId, pos);
};
//-----------------------------------------------------------------------------
// Get the current BGS position
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPluginCommandsGetBGSPos = function(args) {
	const varId = Number(args.Variable);
	const pos = AudioManager._bgsBuffer ? AudioManager._bgsBuffer.seek() : 0;
	$gameVariables.setValue(varId, pos);
};
//-----------------------------------------------------------------------------
// Change an option for the player
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPluginCommandsChangeOption = function(args) {
	const value = (args.Value === 'true');
	switch(args.Option) {
		case 'Always Dash': ConfigManager.alwaysDash = value; break;
		case 'Touch UI': ConfigManager.touchUI = value; break;
		case 'Command Remember': ConfigManager.commandRemember = value; break;
	}
	ConfigManager.save();
};
//-----------------------------------------------------------------------------
// Change a volume option for the player
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPluginCommandsChangeVolumeOption = function(args) {
	const value = Number(args.Value);
	switch(args.Option) {
		case 'BGM Volume': ConfigManager.bgmVolume = value; break;
		case 'BGS Volume': ConfigManager.bgsVolume = value; break;
		case 'ME Volume': ConfigManager.meVolume = value; break;
		case 'SE Volume': ConfigManager.seVolume = value; break;
	}
	ConfigManager.save();
};
//=============================================================================
// Window_EventItem
//-----------------------------------------------------------------------------
// Selection of more than just items
//=============================================================================
//-----------------------------------------------------------------------------
// Alias. Handling for skills / weapons / armors
//-----------------------------------------------------------------------------
const alias_CGMZ_PluginCommands_WindowEventItem_includes = Window_EventItem.prototype.includes;
Window_EventItem.prototype.includes = function(item) {
    const itypeId = $gameMessage.itemChoiceItypeId();
	switch(itypeId) {
		case "armor": return DataManager.isArmor(item);
		case "weapon": return DataManager.isWeapon(item);
		case "skill": return DataManager.isSkill(item) && !this._data.some(data => item.name === data.name);
		default: return alias_CGMZ_PluginCommands_WindowEventItem_includes.call(this, item);
	}
};
//-----------------------------------------------------------------------------
// Alias. Set data to skills if itypeId is skill
//-----------------------------------------------------------------------------
const alias_CGMZ_PluginCommands_WindowEventItem_makeItemList = Window_EventItem.prototype.makeItemList;
Window_EventItem.prototype.makeItemList = function() {
	const itypeId = $gameMessage.itemChoiceItypeId();
	if(itypeId === "skill") {
		this._data = [];
		for(const actor of $gameParty.members()) {
			this._data = this._data.concat(actor.skills().filter(skill => this.includes(skill)));
		}
	} else {
		alias_CGMZ_PluginCommands_WindowEventItem_makeItemList.call(this);
	}
};
//-----------------------------------------------------------------------------
// Alias. Do not need number if skill
//-----------------------------------------------------------------------------
const alias_CGMZ_PluginCommands_WindowEventItem_needsNumber = Window_EventItem.prototype.needsNumber;
Window_EventItem.prototype.needsNumber = function() {
    const itypeId = $gameMessage.itemChoiceItypeId();
    if (itypeId === "skill") {
		return false;
	}
	return alias_CGMZ_PluginCommands_WindowEventItem_needsNumber.call(this);
};
//=============================================================================
// Battle Manager
//-----------------------------------------------------------------------------
// Change can lose for random encounters if set via plugin command
//=============================================================================
//-----------------------------------------------------------------------------
// Allow Can Lose if plugin command enabled it
// This function does not seem to be called on battle processing event command,
// which is how we differentiate between random and pre-determined encounter.
//-----------------------------------------------------------------------------
const alias_CGMZ_PluginCommands_BattleManager_onEncounter = BattleManager.onEncounter;
BattleManager.onEncounter = function() {
	alias_CGMZ_PluginCommands_BattleManager_onEncounter.call(this);
    if($cgmz.pluginCommandAllowBattleLose) this._canLose = true;
};