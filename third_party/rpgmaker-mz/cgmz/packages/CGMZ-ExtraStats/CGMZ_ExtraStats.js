/*:
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/extrastats/
 * @target MZ
 * @base CGMZ_Core
 * @orderAfter CGMZ_Core
 * @plugindesc Tracks additional game data not otherwise easily available
 * @help
 * ============================================================================
 * For terms and conditions using this plugin in your game please visit:
 * https://www.caspergaming.com/terms-of-use/
 * ============================================================================
 * Become a Patron to get access to beta/alpha plugins plus other goodies!
 * https://www.patreon.com/CasperGamingRPGM
 * ============================================================================
 * Version: 1.5.1
 * ----------------------------------------------------------------------------
 * Compatibility: Only tested with my CGMZ plugins.
 * Made for RPG Maker MZ 1.10.0
 * ----------------------------------------------------------------------------
 * Description: Tracks additional game data such as gold spent at shops,
 * damage taken, items used, and many more. This data is stored in variables
 * so it is easy to access in events. Turn tracking on/off any time.
 * ----------------------------------------------------------------------------
 * Documentation:
 * --------------------------Stats Tracked-------------------------------------
 * The following stats are tracked for the entire party:
 * • Gold spent at shops
 * • Gold earned at shops
 * • Items bought from shops
 * • Items sold to shops
 * • Damage taken
 * • Damage dealt
 * • Items used
 * • Gold looted from battle
 * • Amount of each individual item bought/sold
 * • Amount of each enemy defeated
 *
 * The following stats are tracked per actor:
 * • Times attacked
 * • Times attacked with
 * • Times used skills
 * • Amount of deaths
 * --------------------------Script Calls--------------------------------------
 * These stats are also always stored in CGMZ data separate from the in-game
 * variables. To access these values, use the following javascript in any
 * "script" command (script, command variables->script, etc):
 *
 * $cgmz.getExtraStats("itemsBought")
 * $cgmz.getExtraStats("itemsSold")
 * $cgmz.getExtraStats("itemsUsed")
 * $cgmz.getExtraStats("goldSpent")
 * $cgmz.getExtraStats("goldProfit")
 * $cgmz.getExtraStats("goldLooted")
 * $cgmz.getExtraStats("damageDealt")
 * $cgmz.getExtraStats("damageTaken")
 *
 * This can help track these stats without needing to dedicate in-game
 * variables to them as you can always look them up on the fly.
 *
 * To call the statistics scene using JS, use the following call:
 * SceneManager.push(CGMZ_Scene_Statistics);
 * ----------------------------Plugin Commands---------------------------------
 * • Initialize
 * This command will re-initialize all CGMZ Extra Stats data. Does not affect
 * in-game variables, only internal CGMZ data.
 *
 * • Tracking
 * This command will turn all tracking ON or OFF. Tracking is ON by default.
 * When tracking is OFF, both in-game variables and internal CGMZ extra stat
 * data is not tracked.
 *
 * • Get Party Stat
 * Sets a game variable to the selected party stat.
 *
 * • Get Actor Stat
 * Sets a game variable to the selected actor-specific stat.
 *
 * • Get Enemies Killed
 * Sets a game variable to the selected enemy's defeated count
 *
 * • Get Item Stat
 * Sets a game variable to the selected item-specific stat
 *
 * • Call Scene
 * Calls the statistics scene
 * ------------------------------Saved Games-----------------------------------
 * This plugin is fully compatible with saved games. This means you can:
 * 
 * ✓ Add plugin to a saved game and stats will start being tracked
 * ✓ Modify parameters and updates will be reflected in saved games
 * ✓ Remove the plugin with no issues to save data
 * ------------------------Legacy Stat Info------------------------------------
 * Legacy stats are the first 8 stats tracked via this plugin.
 *
 * These stats were automatically tracked by variables set up in parameters.
 * New stats do not have the ability to be tracked automatically by variable,
 * as this approach has several issues for per-actor stats. Instead, it is
 * recommended to use the plugin command to get these stats into variables.
 * -----------------------------Filename---------------------------------------
 * The filename for this plugin MUST remain CGMZ_ExtraStats.js
 * This is what it comes as when downloaded. The filename is used to load
 * parameters and execute plugin commands. If you change it, things will begin
 * behaving incorrectly and your game will probably crash. Please do not
 * rename the js file.
 * --------------------------Latest Version------------------------------------
 * Hi all, this latest version fixes a slight inconvenience in the divider
 * display line item not taking up a full line of space if there are an odd
 * number of columns before it, overlapping the previous column. Before, you
 * would have to work around this by using a custom space or a blank line to
 * position the divider.
 *
 * Version 1.5.1
 * - Divider now automatically takes up its own line
 *
 * @command Initialize
 * @desc Re-initializes CGMZ extra stat data. Only call this if you know what you are doing.
 * Will reset all CGMZ extra stat data as if you started a new game.
 *
 * @command Tracking
 * @desc Turns tracking of extra stats on/off
 *
 * @arg track
 * @type boolean
 * @desc Turns tracking for all extra stats on/off.
 * @default true
 *
 * @command Get Party Stat
 * @desc Get a party-wide stat into a variable
 *
 * @arg stat
 * @type select
 * @option Items Bought
 * @value itemsBought
 * @option Items Sold
 * @value itemsSold
 * @option Gold Profit
 * @value goldProfit
 * @option Gold Spent
 * @value goldSpent
 * @option Items Used
 * @value itemsUsed
 * @option Gold Looted
 * @value goldLooted
 * @option Damage Taken
 * @value damageTaken
 * @option Damage Dealt
 * @value damageDealt
 * @desc The stat to get
 *
 * @arg variable
 * @type variable
 * @desc Variable to store the stat in
 * @default 0
 *
 * @command Get Actor Stat
 * @desc Get an actor-specific stat into a variable
 *
 * @arg stat
 * @type select
 * @option Times Attacked
 * @value PAtimesAttacked
 * @option Times Attacked With
 * @value PAtimesAttackedWith
 * @option Times Used Skills
 * @value PAtimesUsedSkills
 * @option Times Died
 * @value PAtimesDied
 * @desc The stat to get
 *
 * @arg variable
 * @type variable
 * @desc Variable to store the stat in
 * @default 0
 *
 * @arg actor
 * @type actor
 * @desc Actor to get the tracked stat from
 * @default 0
 *
 * @command Get Enemies Killed
 * @desc Get the amount of an enemy killed
 *
 * @arg variable
 * @type variable
 * @desc Variable to store the stat in
 * @default 0
 *
 * @arg Enemy
 * @type enemy
 * @desc The enemy to get the info for
 * @default 0
 *
 * @command Get Item Stat
 * @desc Get an item-specific stat into a variable
 *
 * @arg stat
 * @type select
 * @option Times Used
 * @value Times Used
 * @option Times Bought
 * @value Times Bought
 * @option Times Sold
 * @value Times Sold
 * @default Times Bought
 * @desc The stat to get
 *
 * @arg Variable
 * @type variable
 * @desc Variable to store the stat in
 * @default 0
 *
 * @arg Item
 * @type item
 * @desc Item to get the tracked stat from. If using this, do not set weapon/armor
 * @default 0
 *
 * @arg Weapon
 * @type weapon
 * @desc Weapon to get the tracked stat from. If using this, do not set item/armor
 * @default 0
 *
 * @arg Armor
 * @type armor
 * @desc Armor to get the tracked stat from. If using this, do not set weapon/item
 * @default 0
 *
 * @command Call Scene
 * @desc Calls the statistics scene
 *
 * @param Variable Options
 *
 * @param Items Bought
 * @parent Variable Options
 * @type variable
 * @desc Variable to store items bought from shop count
 * @default 0
 *
 * @param Items Sold
 * @parent Variable Options
 * @type variable
 * @desc Variable to store items sold from shop count
 * @default 0
 *
 * @param Gold Profit
 * @parent Variable Options
 * @type variable
 * @desc Variable to store gold gained from shop sales
 * @default 0
 *
 * @param Gold Spent
 * @parent Variable Options
 * @type variable
 * @desc Variable to store gold lost from shop buy
 * @default 0
 *
 * @param Items Used
 * @parent Variable Options
 * @type variable
 * @desc Variable to store items used from menu or from battle
 * @default 0
 *
 * @param Gold Looted
 * @parent Variable Options
 * @type variable
 * @desc Variable to store gold looted from battle
 * @default 0
 *
 * @param Damage Taken
 * @parent Variable Options
 * @type variable
 * @desc Variable to store damage taken
 * @default 0
 *
 * @param Damage Dealt
 * @parent Variable Options
 * @type variable
 * @desc Variable to store damage dealt
 * @default 0
 *
 * @param Ignored Skills
 * @type skill[]
 * @desc Skills that, when used, do not add to actor's Times Used Skills
 * @default []
 *
 * @param Statistics Scene
 *
 * @param Categories
 * @parent Statistics Scene
 * @type struct<Category>[]
 * @desc Set up categories here
 * @default []
 *
 * @param Disable Touch UI
 * @parent Statistics Scene
 * @type boolean
 * @desc If true and touch UI is OFF, will not reserve space for touch UI
 * @default false
 *
 * @param Category Columns
 * @parent Statistics Scene
 * @type number
 * @min 1
 * @desc Number of columns to show in the category window on each row
 * @default 4
 *
 * @param Category Rows
 * @parent Statistics Scene
 * @type number
 * @min 1
 * @desc Number of rows to show in the category window before scroll
 * @default 1
 *
 * @param Category Alignment
 * @type select
 * @option left
 * @option center
 * @option right
 * @desc Alignment of the text in the category window
 * @default center
 * @parent Statistics Scene
 *
 * @param Icon Alignment
 * @type select
 * @option left
 * @option right
 * @desc Alignment of the icon in the category window
 * @default left
 * @parent Statistics Scene
 *
 * @param Data Columns
 * @parent Statistics Scene
 * @type number
 * @min 1
 * @desc Columns to display the data in
 * @default 2
 *
 * @param Data Column Padding
 * @parent Statistics Scene
 * @type number
 * @desc Padding for data column
 * @default 2
 *
 * @param Data Column Text Padding
 * @parent Statistics Scene
 * @type number
 * @desc Padding for data column text
 * @default 8
 *
 * @param Divider Lines
 * @parent Statistics Scene
 * @type boolean
 * @desc If true, will draw lines along with dividers
 * @default true
 *
 * @param Divider Padding
 * @parent Statistics Scene
 * @type number
 * @min -1
 * @desc Divider padding. Set to -1 to use game default
 * @default -1
 *
 * @param Divider Color 1
 * @parent Statistics Scene
 * @type color
 * @desc First gradient color to use in divider lines
 * @default 1
 *
 * @param Divider Color 2
 * @parent Statistics Scene
 * @type color
 * @desc Second gradient color to use in divider lines
 * @default 0
 *
 * @param Label Color
 * @parent Statistics Scene
 * @type color
 * @desc Color to display data labels in
 * @default 1
 *
 * @param Custom Space
 * @parent Statistics Scene
 * @type number
 * @desc Amount of blank space (in pixels) to leave when drawing custom space line item
 * @default 6
 *
 * @param Text Options
 * @parent Statistics Scene
 *
 * @param Steps Text
 * @parent Text Options
 * @desc Text label for step count data
 * @default Steps:
 *
 * @param Saves Text
 * @parent Text Options
 * @desc Text label for save count data
 * @default Save Count:
 *
 * @param Playtime Text
 * @parent Text Options
 * @desc Text label for playtime data
 * @default Playtime:
 *
 * @param Gold Spent Text
 * @parent Text Options
 * @desc Text label for gold spent data
 * @default Gold Spent:
 *
 * @param Gold Profit Text
 * @parent Text Options
 * @desc Text label for gold profit data
 * @default Gold Profit:
 *
 * @param Gold Looted Text
 * @parent Text Options
 * @desc Text label for gold looted data
 * @default Gold Looted:
 *
 * @param Items Used Text
 * @parent Text Options
 * @desc Text label for items used data
 * @default Items Used:
 *
 * @param Items Bought Text
 * @parent Text Options
 * @desc Text label for items bought data
 * @default Items Bought:
 *
 * @param Items Sold Text
 * @parent Text Options
 * @desc Text label for items sold data
 * @default Items Sold:
 *
 * @param Damage Dealt Text
 * @parent Text Options
 * @desc Text label for damage dealt data
 * @default Damage Dealt:
 *
 * @param Damage Taken Text
 * @parent Text Options
 * @desc Text label for damage taken data
 * @default Damage Taken:
 *
 * @param Item Bought Most Text
 * @parent Text Options
 * @desc Text label for item bought most data
 * @default Item Bought Most:
 *
 * @param Item Sold Most Text
 * @parent Text Options
 * @desc Text label for item sold most data
 * @default Item Sold Most:
 *
 * @param Item Used Most Text
 * @parent Text Options
 * @desc Text label for item used most data
 * @default Item Used Most:
 *
 * @param Weapon Bought Most Text
 * @parent Text Options
 * @desc Text label for weapon bought most data
 * @default Weapon Bought Most:
 *
 * @param Weapon Sold Most Text
 * @parent Text Options
 * @desc Text label for weapon sold most data
 * @default Weapon Sold Most:
 *
 * @param Armor Bought Most Text
 * @parent Text Options
 * @desc Text label for armor bought most data
 * @default Armor Bought Most:
 *
 * @param Armor Sold Most Text
 * @parent Text Options
 * @desc Text label for armor sold most data
 * @default Armor Sold Most:
 *
 * @param Enemies Defeated Text
 * @parent Text Options
 * @desc Text label for enemies defeated data
 * @default Enemies Defeated:
 *
 * @param Enemy Defeated Most Text
 * @parent Text Options
 * @desc Text label for enemy defeated most data
 * @default Enemy Defeated Most:
 *
 * @param Times Died Text
 * @parent Text Options
 * @desc Text label for times died data
 * @default Times Died:
 *
 * @param Actor Died Most Text
 * @parent Text Options
 * @desc Text label for actor died most data
 * @default Actor Died Most:
 *
 * @param None Text
 * @parent Text Options
 * @desc Text for when no data exists (no enemy defeated most, etc)
 * @default N/A
 *
 * @param Integrations
 *
 * @param Idle Time Text
 * @parent Integrations
 * @desc Text label for idle time data (Requires [CGMZ] Idle)
 * @default Time Idle:
 *
 * @param New Game Plus Times Text
 * @parent Integrations
 * @desc Text label for new game plus times data (Requires [CGMZ] New Game Plus)
 * @default NG+ Times:
 *
 * @param Achievement Points Text
 * @parent Integrations
 * @desc Text label for achievement points data (Requires [CGMZ] Achievements)
 * @default Achievement Points:
 *
 * @param Achievements Earned Text
 * @parent Integrations
 * @desc Text label for achievements earned data (Requires [CGMZ] Achievements)
 * @default Achievements Earned:
 *
 * @param Scene Background
 * @parent Integrations
 * @desc [CGMZ] Scene Backgrounds preset id to use for the statistics scene
 *
 * @param Controls Window
 * @parent Integrations
 * @desc [CGMZ] Controls Window preset id to use for the statistics scene
 *
 * @param Category Window Background
 * @parent Integrations
 * @desc [CGMZ] Window Backgrounds preset id to use for the statistics scene category window
 *
 * @param Data Window Background
 * @parent Integrations
 * @desc [CGMZ] Window Backgrounds preset id to use for the statistics scene data window
 *
 * @param Category Window Settings
 * @parent Integrations
 * @desc [CGMZ] Window Settings preset id to use for the statistics scene category window
 *
 * @param Data Window Settings
 * @parent Integrations
 * @desc [CGMZ] Window Settings preset id to use for the statistics scene data window
*/
/*~struct~Category:
 * @param id
 * @desc The category id, used to refer to this category later.
 *
 * @param Category Window
 *
 * @param Display Name
 * @parent Category Window
 * @desc The name displayed in the category window
 *
 * @param Icon
 * @parent Category Window
 * @type icon
 * @default 0
 * @desc The icon to represent the category
 *
 * @param Background Image
 * @parent Category Window
 * @type file
 * @dir img
 * @desc A background image to use for the category. Blank = default black rectangle
 *
 * @param Background Image X
 * @parent Category Window
 * @type number
 * @default 0
 * @min 0
 * @desc The x coordinate to start the background image from the source image (upper left corner)
 *
 * @param Background Image Y
 * @parent Category Window
 * @type number
 * @default 0
 * @min 0
 * @desc The y coordinate to start the background image from the source image (upper left corner)
 *
 * @param Data Window
 *
 * @param Dividers
 * @parent Data Window
 * @type text[]
 * @default []
 * @desc If displaying dividers in this category, will go down this list for the divider name
 *
 * @param Variables
 * @parent Data Window
 * @type struct<Variable>[]
 * @default []
 * @desc Set up custom variable display data here
 *
 * @param Display Info
 * @parent Data Window
 * @type select[]
 * @option Steps
 * @option Saves
 * @option Playtime
 * @option Gold Spent
 * @option Gold Profit
 * @option Gold Looted
 * @option Items Bought
 * @option Items Sold
 * @option Items Used
 * @option Item Used Most
 * @option Item Bought Most
 * @option Item Sold Most
 * @option Weapon Bought Most
 * @option Weapon Sold Most
 * @option Armor Bought Most
 * @option Armor Sold Most
 * @option Damage Dealt
 * @option Damage Taken
 * @option Enemies Defeated
 * @option Enemy Defeated Most
 * @option Times Died
 * @option Actor Died Most
 * @option Custom Variable
 * @option Divider
 * @option Blank Line
 * @option Custom Space
 * @option --Integrations--
 * @option Idle Time
 * @option New Game Plus Times
 * @option Achievement Points
 * @option Achievements Earned
 * @default []
 * @desc Data to display and the order to display it for this category
*/
/*~struct~Variable:
 * @param Variable ID
 * @type variable
 * @desc The variable that stores the value to display
 * @default 0
 *
 * @param Label
 * @desc The label to display for the variable data
*/
/*:es
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/extrastats/
 * @target MZ
 * @base CGMZ_Core
 * @orderAfter CGMZ_Core
 * @plugindesc Rastrea datos adicionales del juego que de otro modo no estarían fácilmente disponibles
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
 * Versión: 1.5.1
 * ----------------------------------------------------------------------------
 * Compatibilidad: Sólo probado con mis CGMZ plugins.
 * Hecho para RPG Maker MZ 1.10.0
 * ----------------------------------------------------------------------------
 * Descripción: Realiza un seguimiento de los datos adicionales del juego,
 * como el oro gastado en las tiendas, el daño recibido, los elementos
 * utilizados y muchos más. Estos datos se almacenan en variables, por lo que
 * es fácil acceder a ellos en los eventos. Activa o desactiva el seguimiento
 * en cualquier momento.
 * ----------------------------------------------------------------------------
 * Documentación:
 * ------------------------Estadísticas rastreadas-----------------------------
 * Las siguientes estadísticas se registran para todo el grupo:
 * • Oro gastado en tiendas
 * • Oro ganado en las tiendas
 * • Artículos comprados en tiendas
 * • Artículos vendidos a tiendas
 * • Daño recibido
 * • Daño infligido
 * • Elementos utilizados
 * • Oro saqueado de la batalla
 * • Amount of each individual item bought/sold
 * • Amount of each enemy defeated
 *
 * Las siguientes estadísticas se rastrean por actor:
 * • Tiempos atacados
 * • Tiempos atacados con
 * • Habilidades veces utilizadas
 * • Amount of deaths
 * -------------------------Llamadas de guión----------------------------------
 * Estas estadísticas también se almacenan siempre en datos CGMZ separados de 
 * las variables del juego. Para acceder a estos valores, use el siguiente 
 * javascript en cualquier comando "guión" (guión, variables de comando->guión,
 * etc.):
 *
 * $cgmz.getExtraStats("itemsBought") > Articulos comprados
 * $cgmz.getExtraStats("itemsSold")   > Articulos vendidos
 * $cgmz.getExtraStats("itemsUsed")   > Articulos usados
 * $cgmz.getExtraStats("goldSpent")   > Oro gastado
 * $cgmz.getExtraStats("goldProfit")  > Ganancia de oro
 * $cgmz.getExtraStats("goldLooted")  > Oro saqueado
 * $cgmz.getExtraStats("damageDealt") > Daño inflingido
 * $cgmz.getExtraStats("damageTaken") > Daño recibido
 *
 * Esto puede ayudar a rastrear estas estadísticas sin necesidad de dedicarles 
 * variables en el juego, ya que siempre puedes buscarlas sobre la marcha.
 *
 * To call the statistics scene using JS, use the following call:
 * SceneManager.push(CGMZ_Scene_Statistics);
 * ------------------------Comandos de Plugin----------------------------------
 * • Inicializar
 * Este plugin reiniciará todos los datos de CGMZ Extra Stats. No afecta las 
 * variables del juego, solo los datos internos de CGMZ.
 *
 * • Seguimiento
 * Este plugin activará o desactivará todo el seguimiento. El seguimiento está 
 * activado de forma predeterminada. Cuando el seguimiento está desactivadp, no 
 * se realiza el seguimiento de las variables del juego ni de las estadísticas 
 * adicionales internas de CGMZ.
 *
 * • Obtener estadísticas de grupo
 * Establece una variable de juego a la estadística del grupo seleccionado.
 *
 * • Obtener estadísticas de actor
 * Establece una variable de juego a la estadística específica del actor
 * seleccionado.
 *
 * • Get Enemies Killed
 * Sets a game variable to the selected enemy's defeated count
 *
 * • Get Item Stat
 * Sets a game variable to the selected item-specific stat
 *
 * • Call Scene
 * Calls the statistics scene
 * -------------------------Juegos Gardados------------------------------------
 * Este plugin es totalmente compatible con juegos guardados. Esto significa
 * que puedes:
 *
 * ✓ Agregue un plugin a un juego guardado y las estadísticas comenzarán a
 *   rastrearse.
 * ✓ Modificar parámetros y actualizaciones se reflejarán en partidas
 *   guardadas.
 * ✓ Eliminar el plugin sin problemas para guardar datos
 * -----------------Información de estadísticas heredadas----------------------
 * Las estadísticas heredadas son las primeras 8 estadísticas rastreadas a
 * través de este complemento.
 *
 * Estas estadísticas fueron rastreadas automáticamente por variables 
 * configuradas en parámetros.
 * Las nuevas estadísticas no tienen la capacidad de ser rastreadas
 * automáticamente por variable, ya que este enfoque tiene varios problemas
 * para las estadísticas por actor. En su lugar, se recomienda usar el comando
 * del complemento para convertir estas estadísticas en variables.
 * -------------------------Nombre de Archivo----------------------------------
 * El nombre de archivo de este plugin DEBE seguir siendo CGMZ_ExtraStats.js
 * Esto es lo que se obtiene cuando se descarga. El nombre de archivo se usa 
 * para cargar parámetros y ejecutar comandos de plugin. Si lo cambias, las 
 * cosas comenzarán a comportarse incorrectamente y tu juego probablemente se 
 * bloquee. Por favor no cambies el nombre del archivo js.
 * --------------------------Latest Version------------------------------------
 * Hi all, this latest version fixes a slight inconvenience in the divider
 * display line item not taking up a full line of space if there are an odd
 * number of columns before it, overlapping the previous column. Before, you
 * would have to work around this by using a custom space or a blank line to
 * position the divider.
 *
 * Version 1.5.1
 * - Divider now automatically takes up its own line
 *
 * @command Initialize
 * @desc Re-initializes CGMZ extra stat data. Only call this if you know what you are doing.
 * Will reset all CGMZ extra stat data as if you started a new game.
 *
 * @command Tracking
 * @text Seguimiento
 * @desc Activa o desactiva el seguimiento de estadísticas adicionales.
 *
 * @arg track
 * @text Activar/desactivar seguimiento
 * @type boolean
 * @desc Activa o desactiva el seguimiento de todas las estadísticas adicionales.
 * @default true
 *
 * @command Get Party Stat
 * @text Obtener estadísticas de partido
 * @desc Obtener una estadística de todo el partido en una variable.
 *
 * @arg stat
 * @text Estadística
 * @type select
 * @option Items Bought
 * @value itemsBought
 * @option Items Sold
 * @value itemsSold
 * @option Gold Profit
 * @value goldProfit
 * @option Gold Spent
 * @value goldSpent
 * @option Items Used
 * @value itemsUsed
 * @option Gold Looted
 * @value goldLooted
 * @option Damage Taken
 * @value damageTaken
 * @option Damage Dealt
 * @value damageDealt
 * @desc La estadística para obtener.
 *
 * @arg variable
 * @text Variable
 * @type variable
 * @desc Variable en la cual almacenar la estadística.
 * @default 0
 *
 * @command Get Actor Stat
 * @text Obtener estadísticas de actor
 * @desc Obtener una estadística específica del actor en una variable.
 *
 * @arg stat
 * @text Estadística
 * @type select
 * @option Times Attacked
 * @value PAtimesAttacked
 * @option Times Attacked With
 * @value PAtimesAttackedWith
 * @option Times Used Skills
 * @value PAtimesUsedSkills
 * @option Times Died
 * @value PAtimesDied
 * @desc La estadística para obtener.
 *
 * @arg variable
 * @text Varible
 * @type variable
 * @desc Variable en la cual almacenar la estadística.
 * @default 0
 *
 * @arg actor
 * @text Actor 
 * @type actor
 * @desc Actor desde el cual obtener la estadística rastreada.
 * @default 0
 *
 * @command Get Enemies Killed
 * @desc Get the amount of an enemy killed
 *
 * @arg variable
 * @type variable
 * @desc Variable to store the stat in
 * @default 0
 *
 * @arg Enemy
 * @type enemy
 * @desc The enemy to get the info for
 * @default 0
 *
 * @command Get Item Stat
 * @desc Get an item-specific stat into a variable
 *
 * @arg stat
 * @type select
 * @option Times Used
 * @value Times Used
 * @option Times Bought
 * @value Times Bought
 * @option Times Sold
 * @value Times Sold
 * @default Times Bought
 * @desc The stat to get
 *
 * @arg Variable
 * @type variable
 * @desc Variable to store the stat in
 * @default 0
 *
 * @arg Item
 * @type item
 * @desc Item to get the tracked stat from. If using this, do not set weapon/armor
 * @default 0
 *
 * @arg Weapon
 * @type weapon
 * @desc Weapon to get the tracked stat from. If using this, do not set item/armor
 * @default 0
 *
 * @arg Armor
 * @type armor
 * @desc Armor to get the tracked stat from. If using this, do not set weapon/item
 * @default 0
 *
 * @command Call Scene
 * @desc Calls the statistics scene
 *
 * @param Variable Options
 * @text Opciones de Variable
 *
 * @param Items Bought
 * @text Artículos comprados
 * @parent Variable Options
 * @type variable
 * @desc Variable para almacenar artículos comprados en la tienda
 * @default 0
 *
 * @param Items Sold
 * @text Artículos vendidos
 * @parent Variable Options
 * @type variable
 * @desc Variable para almacenar artículos vendidos del recuento de tiendas.
 * @default 0
 *
 * @param Gold Profit
 * @text Ganancias de oro
 * @parent Variable Options
 * @type variable
 * @desc Variable para almacenar oro obtenido de las ventas en tiendas.
 * @default 0
 *
 * @param Gold Spent
 * @text Oro gastado
 * @parent Variable Options
 * @type variable
 * @desc Variable para almacenar oro perdido por compra en tienda.
 * @default 0
 *
 * @param Items Used
 * @text Artículos usados
 * @parent Variable Options
 * @type variable
 * @desc Variable para almacenar elementos utilizados desde el menú o desde la batalla.
 * @default 0
 *
 * @param Gold Looted
 * @text Oro saqueado
 * @parent Variable Options
 * @type variable
 * @desc Variable para almacenar el oro saqueado de la batalla.
 * @default 0
 *
 * @param Damage Taken
 * @text Daño recibido
 * @parent Variable Options
 * @type variable
 * @desc Variable para almacenar el daño recibido.
 * @default 0
 *
 * @param Damage Dealt
 * @text Daño infligido
 * @parent Variable Options
 * @type variable
 * @desc Variable para almacenar el daño infligido.
 * @default 0
 *
 * @param Ignored Skills
 * @text Habilidades ignoradas
 * @type skill[]
 * @desc Habilidades que, cuando se usan, no se suman a las habilidades usadas por el actor.
 * @default []
 *
 * @param Statistics Scene
 *
 * @param Categories
 * @parent Statistics Scene
 * @type struct<Category>[]
 * @desc Set up categories here
 * @default []
 *
 * @param Disable Touch UI
 * @parent Statistics Scene
 * @type boolean
 * @desc If true and touch UI is OFF, will not reserve space for touch UI
 * @default false
 *
 * @param Category Columns
 * @parent Statistics Scene
 * @type number
 * @min 1
 * @desc Number of columns to show in the category window on each row
 * @default 4
 *
 * @param Category Rows
 * @parent Statistics Scene
 * @type number
 * @min 1
 * @desc Number of rows to show in the category window before scroll
 * @default 1
 *
 * @param Category Alignment
 * @type select
 * @option left
 * @option center
 * @option right
 * @desc Alignment of the text in the category window
 * @default center
 * @parent Statistics Scene
 *
 * @param Icon Alignment
 * @type select
 * @option left
 * @option right
 * @desc Alignment of the icon in the category window
 * @default left
 * @parent Statistics Scene
 *
 * @param Data Columns
 * @parent Statistics Scene
 * @type number
 * @min 1
 * @desc Columns to display the data in
 * @default 2
 *
 * @param Data Column Padding
 * @parent Statistics Scene
 * @type number
 * @desc Padding for data column
 * @default 2
 *
 * @param Data Column Text Padding
 * @parent Statistics Scene
 * @type number
 * @desc Padding for data column text
 * @default 8
 *
 * @param Divider Lines
 * @parent Statistics Scene
 * @type boolean
 * @desc If true, will draw lines along with dividers
 * @default true
 *
 * @param Divider Padding
 * @parent Statistics Scene
 * @type number
 * @min -1
 * @desc Divider padding. Set to -1 to use game default
 * @default -1
 *
 * @param Divider Color 1
 * @parent Statistics Scene
 * @type color
 * @desc First gradient color to use in divider lines
 * @default 1
 *
 * @param Divider Color 2
 * @parent Statistics Scene
 * @type color
 * @desc Second gradient color to use in divider lines
 * @default 0
 *
 * @param Label Color
 * @parent Statistics Scene
 * @type color
 * @desc Color to display data labels in
 * @default 1
 *
 * @param Custom Space
 * @parent Statistics Scene
 * @type number
 * @desc Amount of blank space (in pixels) to leave when drawing custom space line item
 * @default 6
 *
 * @param Text Options
 * @parent Statistics Scene
 *
 * @param Steps Text
 * @parent Text Options
 * @desc Text label for step count data
 * @default Steps:
 *
 * @param Saves Text
 * @parent Text Options
 * @desc Text label for save count data
 * @default Save Count:
 *
 * @param Playtime Text
 * @parent Text Options
 * @desc Text label for playtime data
 * @default Playtime:
 *
 * @param Gold Spent Text
 * @parent Text Options
 * @desc Text label for gold spent data
 * @default Gold Spent:
 *
 * @param Gold Profit Text
 * @parent Text Options
 * @desc Text label for gold profit data
 * @default Gold Profit:
 *
 * @param Gold Looted Text
 * @parent Text Options
 * @desc Text label for gold looted data
 * @default Gold Looted:
 *
 * @param Items Used Text
 * @parent Text Options
 * @desc Text label for items used data
 * @default Items Used:
 *
 * @param Items Bought Text
 * @parent Text Options
 * @desc Text label for items bought data
 * @default Items Bought:
 *
 * @param Items Sold Text
 * @parent Text Options
 * @desc Text label for items sold data
 * @default Items Sold:
 *
 * @param Damage Dealt Text
 * @parent Text Options
 * @desc Text label for damage dealt data
 * @default Damage Dealt:
 *
 * @param Damage Taken Text
 * @parent Text Options
 * @desc Text label for damage taken data
 * @default Damage Taken:
 *
 * @param Item Bought Most Text
 * @parent Text Options
 * @desc Text label for item bought most data
 * @default Item Bought Most:
 *
 * @param Item Sold Most Text
 * @parent Text Options
 * @desc Text label for item sold most data
 * @default Item Sold Most:
 *
 * @param Item Used Most Text
 * @parent Text Options
 * @desc Text label for item used most data
 * @default Item Used Most:
 *
 * @param Weapon Bought Most Text
 * @parent Text Options
 * @desc Text label for weapon bought most data
 * @default Weapon Bought Most:
 *
 * @param Weapon Sold Most Text
 * @parent Text Options
 * @desc Text label for weapon sold most data
 * @default Weapon Sold Most:
 *
 * @param Armor Bought Most Text
 * @parent Text Options
 * @desc Text label for armor bought most data
 * @default Armor Bought Most:
 *
 * @param Armor Sold Most Text
 * @parent Text Options
 * @desc Text label for armor sold most data
 * @default Armor Sold Most:
 *
 * @param Enemies Defeated Text
 * @parent Text Options
 * @desc Text label for enemies defeated data
 * @default Enemies Defeated:
 *
 * @param Enemy Defeated Most Text
 * @parent Text Options
 * @desc Text label for enemy defeated most data
 * @default Enemy Defeated Most:
 *
 * @param Times Died Text
 * @parent Text Options
 * @desc Text label for times died data
 * @default Times Died:
 *
 * @param Actor Died Most Text
 * @parent Text Options
 * @desc Text label for actor died most data
 * @default Actor Died Most:
 *
 * @param None Text
 * @parent Text Options
 * @desc Text for when no data exists (no enemy defeated most, etc)
 * @default N/A
 *
 * @param Integrations
 *
 * @param Idle Time Text
 * @parent Integrations
 * @desc Text label for idle time data (Requires [CGMZ] Idle)
 * @default Time Idle:
 *
 * @param New Game Plus Times Text
 * @parent Integrations
 * @desc Text label for new game plus times data (Requires [CGMZ] New Game Plus)
 * @default NG+ Times:
 *
 * @param Achievement Points Text
 * @parent Integrations
 * @desc Text label for achievement points data (Requires [CGMZ] Achievements)
 * @default Achievement Points:
 *
 * @param Achievements Earned Text
 * @parent Integrations
 * @desc Text label for achievements earned data (Requires [CGMZ] Achievements)
 * @default Achievements Earned:
 *
 * @param Scene Background
 * @parent Integrations
 * @desc [CGMZ] Scene Backgrounds preset id to use for the statistics scene
 *
 * @param Controls Window
 * @parent Integrations
 * @desc [CGMZ] Controls Window preset id to use for the statistics scene
 *
 * @param Category Window Background
 * @parent Integrations
 * @desc [CGMZ] Window Backgrounds preset id to use for the statistics scene category window
 *
 * @param Data Window Background
 * @parent Integrations
 * @desc [CGMZ] Window Backgrounds preset id to use for the statistics scene data window
 *
 * @param Category Window Settings
 * @parent Integrations
 * @desc [CGMZ] Window Settings preset id to use for the statistics scene category window
 *
 * @param Data Window Settings
 * @parent Integrations
 * @desc [CGMZ] Window Settings preset id to use for the statistics scene data window
*/
/*~struct~Category:es
 * @param id
 * @desc The category id, used to refer to this category later.
 *
 * @param Category Window
 *
 * @param Display Name
 * @parent Category Window
 * @desc The name displayed in the category window
 *
 * @param Icon
 * @parent Category Window
 * @type icon
 * @default 0
 * @desc The icon to represent the category
 *
 * @param Background Image
 * @parent Category Window
 * @type file
 * @dir img
 * @desc A background image to use for the category. Blank = default black rectangle
 *
 * @param Background Image X
 * @parent Category Window
 * @type number
 * @default 0
 * @min 0
 * @desc The x coordinate to start the background image from the source image (upper left corner)
 *
 * @param Background Image Y
 * @parent Category Window
 * @type number
 * @default 0
 * @min 0
 * @desc The y coordinate to start the background image from the source image (upper left corner)
 *
 * @param Data Window
 *
 * @param Dividers
 * @parent Data Window
 * @type text[]
 * @default []
 * @desc If displaying dividers in this category, will go down this list for the divider name
 *
 * @param Variables
 * @parent Data Window
 * @type struct<Variable>[]
 * @default []
 * @desc Set up custom variable display data here
 *
 * @param Display Info
 * @parent Data Window
 * @type select[]
 * @option Steps
 * @option Saves
 * @option Playtime
 * @option Gold Spent
 * @option Gold Profit
 * @option Gold Looted
 * @option Items Bought
 * @option Items Sold
 * @option Items Used
 * @option Item Used Most
 * @option Item Bought Most
 * @option Item Sold Most
 * @option Weapon Bought Most
 * @option Weapon Sold Most
 * @option Armor Bought Most
 * @option Armor Sold Most
 * @option Damage Dealt
 * @option Damage Taken
 * @option Enemies Defeated
 * @option Enemy Defeated Most
 * @option Times Died
 * @option Actor Died Most
 * @option Custom Variable
 * @option Divider
 * @option Blank Line
 * @option Custom Space
 * @option --Integrations--
 * @option Idle Time
 * @option New Game Plus Times
 * @option Achievement Points
 * @option Achievements Earned
 * @default []
 * @desc Data to display and the order to display it for this category
*/
/*~struct~Variable:es
 * @param Variable ID
 * @type variable
 * @desc The variable that stores the value to display
 * @default 0
 *
 * @param Label
 * @desc The label to display for the variable data
*/
Imported.CGMZ_ExtraStats = true;
CGMZ.Versions["Extra Stats"] = "1.5.1";
CGMZ.ExtraStats = {};
CGMZ.ExtraStats.parameters = PluginManager.parameters('CGMZ_ExtraStats');
CGMZ.ExtraStats.SceneBackground = CGMZ.ExtraStats.parameters["Scene Background"];
CGMZ.ExtraStats.ControlsWindow = CGMZ.ExtraStats.parameters["Controls Window"];
CGMZ.ExtraStats.CategoryAlignment = CGMZ.ExtraStats.parameters["Category Alignment"];
CGMZ.ExtraStats.IconAlignment = CGMZ.ExtraStats.parameters["Icon Alignment"];
CGMZ.ExtraStats.StepsText = CGMZ.ExtraStats.parameters["Steps Text"];
CGMZ.ExtraStats.SavesText = CGMZ.ExtraStats.parameters["Saves Text"];
CGMZ.ExtraStats.PlaytimeText = CGMZ.ExtraStats.parameters["Playtime Text"];
CGMZ.ExtraStats.GoldSpentText = CGMZ.ExtraStats.parameters["Gold Spent Text"];
CGMZ.ExtraStats.GoldProfitText = CGMZ.ExtraStats.parameters["Gold Profit Text"];
CGMZ.ExtraStats.GoldLootedText = CGMZ.ExtraStats.parameters["Gold Looted Text"];
CGMZ.ExtraStats.ItemsUsedText = CGMZ.ExtraStats.parameters["Items Used Text"];
CGMZ.ExtraStats.ItemsBoughtText = CGMZ.ExtraStats.parameters["Items Bought Text"];
CGMZ.ExtraStats.ItemsSoldText = CGMZ.ExtraStats.parameters["Items Sold Text"];
CGMZ.ExtraStats.DamageDealtText = CGMZ.ExtraStats.parameters["Damage Dealt Text"];
CGMZ.ExtraStats.DamageTakenText = CGMZ.ExtraStats.parameters["Damage Taken Text"];
CGMZ.ExtraStats.ItemBoughtMostText = CGMZ.ExtraStats.parameters["Item Bought Most Text"];
CGMZ.ExtraStats.ItemSoldMostText = CGMZ.ExtraStats.parameters["Item Sold Most Text"];
CGMZ.ExtraStats.ItemUsedMostText = CGMZ.ExtraStats.parameters["Item Used Most Text"];
CGMZ.ExtraStats.WeaponBoughtMostText = CGMZ.ExtraStats.parameters["Weapon Bought Most Text"];
CGMZ.ExtraStats.WeaponSoldMostText = CGMZ.ExtraStats.parameters["Weapon Sold Most Text"];
CGMZ.ExtraStats.ArmorBoughtMostText = CGMZ.ExtraStats.parameters["Armor Bought Most Text"];
CGMZ.ExtraStats.ArmorSoldMostText = CGMZ.ExtraStats.parameters["Armor Sold Most Text"];
CGMZ.ExtraStats.EnemyDefeatedMostText = CGMZ.ExtraStats.parameters["Enemy Defeated Most Text"];
CGMZ.ExtraStats.EnemiesDefeatedText = CGMZ.ExtraStats.parameters["Enemies Defeated Text"];
CGMZ.ExtraStats.ActorDiedMostText = CGMZ.ExtraStats.parameters["Actor Died Most Text"];
CGMZ.ExtraStats.TimesDiedText = CGMZ.ExtraStats.parameters["Times Died Text"];
CGMZ.ExtraStats.NoneText = CGMZ.ExtraStats.parameters["None Text"];
CGMZ.ExtraStats.IdleTimeText = CGMZ.ExtraStats.parameters["Idle Time Text"];
CGMZ.ExtraStats.NewGamePlusTimesText = CGMZ.ExtraStats.parameters["New Game Plus Times Text"];
CGMZ.ExtraStats.AchievementPointsText = CGMZ.ExtraStats.parameters["Achievement Points Text"];
CGMZ.ExtraStats.AchievementsEarnedText = CGMZ.ExtraStats.parameters["Achievements Earned Text"];
CGMZ.ExtraStats.CategoryWindowBackground = CGMZ.ExtraStats.parameters["Category Window Background"];
CGMZ.ExtraStats.DataWindowBackground = CGMZ.ExtraStats.parameters["Data Window Background"];
CGMZ.ExtraStats.CategoryWindowSettings = CGMZ.ExtraStats.parameters["Category Window Settings"];
CGMZ.ExtraStats.DataWindowSettings = CGMZ.ExtraStats.parameters["Data Window Settings"];
CGMZ.ExtraStats.ItemsBought = Number(CGMZ.ExtraStats.parameters["Items Bought"]);
CGMZ.ExtraStats.ItemsSold = Number(CGMZ.ExtraStats.parameters["Items Sold"]);
CGMZ.ExtraStats.GoldProfit = Number(CGMZ.ExtraStats.parameters["Gold Profit"]);
CGMZ.ExtraStats.GoldSpent = Number(CGMZ.ExtraStats.parameters["Gold Spent"]);
CGMZ.ExtraStats.ItemsUsed = Number(CGMZ.ExtraStats.parameters["Items Used"]);
CGMZ.ExtraStats.GoldLooted = Number(CGMZ.ExtraStats.parameters["Gold Looted"]);
CGMZ.ExtraStats.DamageTaken = Number(CGMZ.ExtraStats.parameters["Damage Taken"]);
CGMZ.ExtraStats.DamageDealt = Number(CGMZ.ExtraStats.parameters["Damage Dealt"]);
CGMZ.ExtraStats.CategoryColumns = Number(CGMZ.ExtraStats.parameters["Category Columns"]);
CGMZ.ExtraStats.CategoryRows = Number(CGMZ.ExtraStats.parameters["Category Rows"]);
CGMZ.ExtraStats.DataColumns = Number(CGMZ.ExtraStats.parameters["Data Columns"]);
CGMZ.ExtraStats.DataColumnPadding = Number(CGMZ.ExtraStats.parameters["Data Column Padding"]);
CGMZ.ExtraStats.DataColumnTextPadding = Number(CGMZ.ExtraStats.parameters["Data Column Text Padding"]);
CGMZ.ExtraStats.LabelColor = Number(CGMZ.ExtraStats.parameters["Label Color"]);
CGMZ.ExtraStats.WindowType = Number(CGMZ.ExtraStats.parameters["Window Type"]);
CGMZ.ExtraStats.DividerPadding = Number(CGMZ.ExtraStats.parameters["Divider Padding"]);
CGMZ.ExtraStats.DividerColor1 = Number(CGMZ.ExtraStats.parameters["Divider Color 1"]);
CGMZ.ExtraStats.DividerColor2 = Number(CGMZ.ExtraStats.parameters["Divider Color 2"]);
CGMZ.ExtraStats.CustomSpace = Number(CGMZ.ExtraStats.parameters["Custom Space"]);
CGMZ.ExtraStats.DisableTouchUI = (CGMZ.ExtraStats.parameters["Disable Touch UI"] === 'true');
CGMZ.ExtraStats.DividerLines = (CGMZ.ExtraStats.parameters["Divider Lines"] === 'true');
CGMZ.ExtraStats.IgnoredSkills = CGMZ_Utils.parseJSON(CGMZ.ExtraStats.parameters["Ignored Skills"], [], "[CGMZ] Extra Stats", "Your Ignored Skills parameter is set up incorrectly.").map(x => Number(x));
CGMZ.ExtraStats.Categories = CGMZ_Utils.parseJSON(CGMZ.ExtraStats.parameters["Categories"], [], "[CGMZ] Extra Stats", "Your Categories parameter is set up incorrectly.");
//=============================================================================
// CGMZ_ExtraStatCategory
//-----------------------------------------------------------------------------
// Data class used to store extra stat category data. Not saved
//=============================================================================
function CGMZ_ExtraStatCategory() {
    this.initialize(...arguments);
}
//-----------------------------------------------------------------------------
// Initialize
//-----------------------------------------------------------------------------
CGMZ_ExtraStatCategory.prototype.initialize = function(cat) {
	this.id = cat.id;
	this.displayName = cat["Display Name"];
	this.icon = Number(cat.Icon);
	if(cat["Background Image"]) {
		this.img = {
			img: cat["Background Image"],
			imgX: Number(cat["Background Image X"]),
			imgY: Number(cat["Background Image Y"])
		};
	}
	this.dividers = CGMZ_Utils.parseJSON(cat["Dividers"], [], "[CGMZ] Extra Stats", `Your category with id ${this.id} had its Dividers parameter set up incorrectly.`);
	this.displayInfo = CGMZ_Utils.parseJSON(cat["Display Info"], [], "[CGMZ] Extra Stats", `Your category with id ${this.id} had its Display Info parameter set up incorrectly.`);
	this.customVar = CGMZ_Utils.parseJSON(cat["Variables"], [], "[CGMZ] Extra Stats", `Your category with id ${this.id} had its Variables parameter set up incorrectly.`).map((v) => {
		const pv = CGMZ_Utils.parseJSON(v, null, "[CGMZ] Extra Stats", `Your category with id ${this.id} had a Variables parameter set up incorrectly.`);
		if(!pv) return null;
		return {id: Number(pv["Variable ID"]), label: pv.Label};
	}).filter(x => !!x);
};
//=============================================================================
// CGMZ_Temp
//-----------------------------------------------------------------------------
// Add plugin commands for CGMZ Extra Stats, initialize unsaved stat data
//=============================================================================
//-----------------------------------------------------------------------------
// Add unsaved statistics data
//-----------------------------------------------------------------------------
const alias_CGMZExtraStats_CGMZTemp_createPluginData = CGMZ_Temp.prototype.createPluginData;
CGMZ_Temp.prototype.createPluginData = function() {
	alias_CGMZExtraStats_CGMZTemp_createPluginData.call(this);
	this.initializeExtraStatData();
};
//-----------------------------------------------------------------------------
// Initialize extra stat data
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.initializeExtraStatData = function() {
	this.extraStatCategories = {};
	for(const catJSON of CGMZ.ExtraStats.Categories) {
		const parsedCat = CGMZ_Utils.parseJSON(catJSON, null, "[CGMZ] Extra Stats", "Could not parse category JSON. Please configure your Categories parameter.");
		if(!parsedCat) continue;
		const category = new CGMZ_ExtraStatCategory(parsedCat);
		this.extraStatCategories[category.id] = category;
	}
};
//-----------------------------------------------------------------------------
// Get all extra stat category ids
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.getExtraStatsCategories = function() {
	return Object.keys(this.extraStatCategories);
};
//-----------------------------------------------------------------------------
// Get an extra stats category by id
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.getExtraStatsCategory = function(id) {
	return this.extraStatCategories[id];
};
//-----------------------------------------------------------------------------
// Register Plugin Commands
//-----------------------------------------------------------------------------
const alias_CGMZ_ExtraStats_registerPluginCommands = CGMZ_Temp.prototype.registerPluginCommands;
CGMZ_Temp.prototype.registerPluginCommands = function() {
	alias_CGMZ_ExtraStats_registerPluginCommands.call(this);
	PluginManager.registerCommand("CGMZ_ExtraStats", "Initialize", this.pluginCommandExtraStatsReinitialize);
	PluginManager.registerCommand("CGMZ_ExtraStats", "Tracking", this.pluginCommandExtraStatsTracking);
	PluginManager.registerCommand("CGMZ_ExtraStats", "Get Party Stat", this.pluginCommandExtraStatsGetPartyStat);
	PluginManager.registerCommand("CGMZ_ExtraStats", "Get Actor Stat", this.pluginCommandExtraStatsGetActorStat);
	PluginManager.registerCommand("CGMZ_ExtraStats", "Get Enemies Killed", this.pluginCommandExtraStatsGetEnemiesKilled);
	PluginManager.registerCommand("CGMZ_ExtraStats", "Get Item Stat", this.pluginCommandExtraStatsGetItemStat);
	PluginManager.registerCommand("CGMZ_ExtraStats", "Call Scene", this.pluginCommandExtraStatsCallScene);
};
//-----------------------------------------------------------------------------
// Plugin Command - Initialize
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandExtraStatsReinitialize = function(args) {
	$cgmz.initExtraStatsVars(true);
};
//-----------------------------------------------------------------------------
// Plugin Command - Tracking
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandExtraStatsTracking = function(args) {
	$cgmz.setExtraStatsTracking(args.track === "true");
};
//-----------------------------------------------------------------------------
// Plugin Command - Get Party Stat
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandExtraStatsGetPartyStat = function(args) {
	const variable = Number(args.variable);
	const value = $cgmz.getExtraStats(args.stat);
	$gameVariables.setValue(variable, value);
};
//-----------------------------------------------------------------------------
// Plugin Command - Get Actor Stat
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandExtraStatsGetActorStat = function(args) {
	const variable = Number(args.variable);
	const actorId = Number(args.actor);
	const value = $cgmz.getExtraStatsActor(args.stat, actorId);
	$gameVariables.setValue(variable, value);
};
//-----------------------------------------------------------------------------
// Plugin Command - Get Enemies Killed
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandExtraStatsGetEnemiesKilled = function(args) {
	const variable = Number(args.variable);
	const enemyId = Number(args.Enemy);
	const value = $cgmz.getExtraStatsActor('individualEnemiesKilled', enemyId);
	$gameVariables.setValue(variable, value);
};
//-----------------------------------------------------------------------------
// Plugin Command - Get Item Stat
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandExtraStatsGetItemStat = function(args) {
	const variable = Number(args.Variable);
	const stat = args.stat;
	let id, key;
	if(Number(args.Item)) {
		id = Number(args.Item);
		key = (stat === 'Times Used') ? 'individualItemsUsed' : (stat === 'Times Bought') ? 'individualItemsBought' : 'individualItemsSold';
	}
	if(Number(args.Weapon)) {
		id = Number(args.Weapon);
		key = (stat === 'Times Bought') ? 'individualWeaponsBought' : 'individualWeaponsSold';
	}
	if(Number(args.Armor)) {
		id = Number(args.Armor);
		key = (stat === 'Times Bought') ? 'individualArmorsBought' : 'individualArmorsSold';
	}
	if(!key) return;
	const value = $cgmz.getExtraStatsActor(key, id);
	$gameVariables.setValue(variable, value);
};
//-----------------------------------------------------------------------------
// Plugin Command - Call Scene
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandExtraStatsCallScene = function(args) {
	SceneManager.push(CGMZ_Scene_Statistics);
};
//=============================================================================
// CGMZ
//-----------------------------------------------------------------------------
// Add new tracked stats to the save data
//=============================================================================
//-----------------------------------------------------------------------------
// Method used by CGMZ for creating plugin data
//-----------------------------------------------------------------------------
const alias_CGMZ_ExtraStats_createPluginData = CGMZ_Core.prototype.createPluginData;
CGMZ_Core.prototype.createPluginData = function() {
	alias_CGMZ_ExtraStats_createPluginData.call(this);
	this.initExtraStatsVars();
};
//-----------------------------------------------------------------------------
// Check for new data after game load
//-----------------------------------------------------------------------------
const alias_CGMZ_ExtraStats_CGMZ_Core_onAfterLoad = CGMZ_Core.prototype.onAfterLoad;
CGMZ_Core.prototype.onAfterLoad = function() {
	alias_CGMZ_ExtraStats_CGMZ_Core_onAfterLoad.call(this);
	this.initExtraStatsVars();
};
//-----------------------------------------------------------------------------
// Initialize Extra Stats variables
//-----------------------------------------------------------------------------
CGMZ_Core.prototype.initExtraStatsVars = function(reinitialize = false) {
	if(!this._extraStats || reinitialize) {
		this._extraStatsVersion = '1.2.0';
		this._extraStatsTracking = true;
		this._extraStats = {
			'itemsBought': 0,
			'itemsSold': 0,
			'goldProfit': 0,
			'goldSpent': 0,
			'itemsUsed': 0,
			'goldLooted': 0,
			'damageTaken': 0,
			'damageDealt': 0,
			'PAtimesAttacked': {},
			'PAtimesAttackedWith': {},
			'PAtimesUsedSkills': {},
			'PAtimesDied': {},
			'individualItemsBought': {},
			'individualItemsSold': {},
			'individualWeaponsBought': {},
			'individualWeaponsSold': {},
			'individualArmorsBought': {},
			'individualArmorsSold': {},
			'individualItemsUsed': {},
			'individualEnemiesKilled': {}
		};
	}
	if(!this._extraStatsVersion) { // Patch version 1.0 to 1.1.0
		this._extraStatsVersion = '1.1.0';
		this._extraStats.PAtimesAttacked = {};
		this._extraStats.PAtimesAttackedWith = {};
		this._extraStats.PAtimesUsedSkills = {};
	} // End patch version 1.0 to 1.1.0
	if(this._extraStatsVersion === '1.1.0') { // Patch version 1.1.0 to 1.2.0
		this._extraStatsVersion = '1.2.0';
		this._extraStats.PAtimesDied = {};
		this._extraStats.individualItemsBought = {};
		this._extraStats.individualItemsSold = {};
		this._extraStats.individualWeaponsBought = {};
		this._extraStats.individualWeaponsSold = {};
		this._extraStats.individualArmorsBought = {};
		this._extraStats.individualArmorsSold = {};
		this._extraStats.individualItemsUsed = {};
		this._extraStats.individualEnemiesKilled = {};
	} // End patch version 1.1.0 to 1.2.0
};
//-----------------------------------------------------------------------------
// Getter for whether to track stats or not
//-----------------------------------------------------------------------------
CGMZ_Core.prototype.isTrackingExtraStats = function() {
	return this._extraStatsTracking;
};
//-----------------------------------------------------------------------------
// Setter for whether to track stats or not
//-----------------------------------------------------------------------------
CGMZ_Core.prototype.setExtraStatsTracking = function(tracking) {
	this._extraStatsTracking = tracking;
};
//-----------------------------------------------------------------------------
// Getter for party extra stats
//-----------------------------------------------------------------------------
CGMZ_Core.prototype.getExtraStats = function(key) {
	return this._extraStats[key];
};
//-----------------------------------------------------------------------------
// Setter for party extra stats.
//-----------------------------------------------------------------------------
CGMZ_Core.prototype.setExtraStats = function(key, num) {
	this._extraStats[key] = num;
};
//-----------------------------------------------------------------------------
// Add method for party extra stats
//-----------------------------------------------------------------------------
CGMZ_Core.prototype.addExtraStats = function(key, num) {
	const value = this.getExtraStats(key);
	this.setExtraStats(key, num + value);
};
//-----------------------------------------------------------------------------
// Getter for actor extra stats
// These functions also handle individual stats (such as enemies)
//-----------------------------------------------------------------------------
CGMZ_Core.prototype.getExtraStatsActor = function(key, actorId) {
	return this.getExtraStats(key)[actorId] || 0;
};
//-----------------------------------------------------------------------------
// Setter for actor extra stats
// These functions also handle individual stats (such as enemies)
//-----------------------------------------------------------------------------
CGMZ_Core.prototype.setExtraStatsActor = function(key, actorId, num) {
	this._extraStats[key][actorId] = num;
};
//-----------------------------------------------------------------------------
// Add method for actor extra stats
// These functions also handle individual stats (such as enemies)
//-----------------------------------------------------------------------------
CGMZ_Core.prototype.addExtraStatsActor = function(key, actorId, num) {
	const value = this.getExtraStatsActor(key, actorId);
	this.setExtraStatsActor(key, actorId, num + value);
};
//-----------------------------------------------------------------------------
// Calculation for total of an extra stat from an individual stat, should pass in a type
// from the this._extraStats array for an individual stat (ex: individualEnemiesKilled)
//-----------------------------------------------------------------------------
CGMZ_Core.prototype.calculateTotalExtraStat = function(type) {
	const ids = Object.keys(this._extraStats[type]);
	let total = 0;
	for(const id of ids) {
		total += this._extraStats[type][id];
	}
	return total;
};
//-----------------------------------------------------------------------------
// Calculation for most common of an extra stat type, should pass in a type
// from the this._extraStats array for an individual stat (ex: individualEnemiesKilled)
//-----------------------------------------------------------------------------
CGMZ_Core.prototype.calculateMostCommonExtraStat = function(type) {
	const ids = Object.keys(this._extraStats[type]);
	let returnId = 0;
	let biggestCount = 0;
	for(const id of ids) {
		const count = this._extraStats[type][id];
		if(count > biggestCount) {
			returnId = id;
			biggestCount = count;
		}
	}
	return Number(returnId);
};
//=============================================================================
// Scene_Shop
//-----------------------------------------------------------------------------
// Automatic tracking for items bought, sold, and gold gained from sell, lost from buy
//=============================================================================
//-----------------------------------------------------------------------------
// Track items bought, gold spent on items.
//-----------------------------------------------------------------------------
const alias_CGMZ_ExtraStats_SceneShop_doBuy = Scene_Shop.prototype.doBuy;
Scene_Shop.prototype.doBuy = function(number) {
	alias_CGMZ_ExtraStats_SceneShop_doBuy.call(this, number);
	if($cgmz.isTrackingExtraStats()) {
		const oldItemBuyCount = $gameVariables.value(CGMZ.ExtraStats.ItemsBought);
		$gameVariables.setValue(CGMZ.ExtraStats.ItemsBought, oldItemBuyCount + number);
		$cgmz.addExtraStats('itemsBought', number);
		const oldSpentCount = $gameVariables.value(CGMZ.ExtraStats.GoldSpent);
		const amount = number * this.buyingPrice();
		$gameVariables.setValue(CGMZ.ExtraStats.GoldSpent, oldSpentCount + amount);
		$cgmz.addExtraStats("goldSpent", amount);
		if(DataManager.isItem(this._item)) {
			$cgmz.addExtraStatsActor('individualItemsBought', this._item.id, number);
		} else if(DataManager.isWeapon(this._item)) {
			$cgmz.addExtraStatsActor('individualWeaponsBought', this._item.id, number);
		} else if(DataManager.isArmor(this._item)) {
			$cgmz.addExtraStatsActor('individualArmorsBought', this._item.id, number);
		}
	}
};
//-----------------------------------------------------------------------------
// Track items sold, gold gained from sale
//-----------------------------------------------------------------------------
const alias_CGMZ_ExtraStats_SceneShop_doSell = Scene_Shop.prototype.doSell;
Scene_Shop.prototype.doSell = function(number) {
	alias_CGMZ_ExtraStats_SceneShop_doSell.call(this, number);
	if($cgmz.isTrackingExtraStats()) {
		const oldItemSellCount = $gameVariables.value(CGMZ.ExtraStats.ItemsSold);
		$gameVariables.setValue(CGMZ.ExtraStats.ItemsSold, oldItemSellCount + number);
		$cgmz.addExtraStats("itemsSold", number);
		const oldProfitCount = $gameVariables.value(CGMZ.ExtraStats.GoldProfit);
		const amount = number * this.sellingPrice();
		$gameVariables.setValue(CGMZ.ExtraStats.GoldProfit, oldProfitCount + amount);
		$cgmz.addExtraStats("goldProfit", amount);
		if(DataManager.isItem(this._item)) {
			$cgmz.addExtraStatsActor('individualItemsSold', this._item.id, number);
		} else if(DataManager.isWeapon(this._item)) {
			$cgmz.addExtraStatsActor('individualWeaponsSold', this._item.id, number);
		} else if(DataManager.isArmor(this._item)) {
			$cgmz.addExtraStatsActor('individualArmorsSold', this._item.id, number);
		}
	}
};
//=============================================================================
// Game_Party
//-----------------------------------------------------------------------------
// Automatic tracking for items used
//=============================================================================
//-----------------------------------------------------------------------------
// Track items used
//-----------------------------------------------------------------------------
const alias_CGMZ_ExtraStats_GameParty_consumeItem = Game_Party.prototype.consumeItem;
Game_Party.prototype.consumeItem = function(item) {
	alias_CGMZ_ExtraStats_GameParty_consumeItem.call(this, item);
	if(DataManager.isItem(item) && $cgmz.isTrackingExtraStats()) {
		const oldItemsUsed = $gameVariables.value(CGMZ.ExtraStats.ItemsUsed);
		$gameVariables.setValue(CGMZ.ExtraStats.ItemsUsed, oldItemsUsed + 1);
		$cgmz.addExtraStats("itemsUsed", 1);
		$cgmz.addExtraStatsActor('individualItemsUsed', item.id, 1);
	}
};
//=============================================================================
// Game_Actor
//-----------------------------------------------------------------------------
// Automatic tracking for actor deaths
//=============================================================================
//-----------------------------------------------------------------------------
// Track amount of deaths for actor
//-----------------------------------------------------------------------------
const alias_CGMZExtraStats_GameActor_performCollapse = Game_Actor.prototype.performCollapse;
Game_Actor.prototype.performCollapse = function() {
	alias_CGMZExtraStats_GameActor_performCollapse.call(this);
	const id = this.actorId();
	$cgmz.addExtraStatsActor('PAtimesDied', id, 1);
};
//=============================================================================
// Game_Enemy
//-----------------------------------------------------------------------------
// Automatic tracking for enemy defeated
//=============================================================================
//-----------------------------------------------------------------------------
// Track amount of enemies of this type defeated
//-----------------------------------------------------------------------------
const alias_CGMZExtraStats_GameEnemy_performCollapse = Game_Enemy.prototype.performCollapse;
Game_Enemy.prototype.performCollapse = function() {
	alias_CGMZExtraStats_GameEnemy_performCollapse.call(this);
	const id = this.enemyId();
	$cgmz.addExtraStatsActor('individualEnemiesKilled', id, 1);
};
//=============================================================================
// BattleManager
//-----------------------------------------------------------------------------
// Automatic tracking for gold looted from battle
//=============================================================================
//-----------------------------------------------------------------------------
// Track gold looted
//-----------------------------------------------------------------------------
const alias_CGMZ_ExtraStats_BattleManager_gainGold = BattleManager.gainGold;
BattleManager.gainGold = function() {
	alias_CGMZ_ExtraStats_BattleManager_gainGold.call(this);
	if($cgmz.isTrackingExtraStats()) {
		const oldGoldLooted = $gameVariables.value(CGMZ.ExtraStats.GoldLooted);
		$gameVariables.setValue(CGMZ.ExtraStats.GoldLooted, oldGoldLooted + this._rewards.gold);
		$cgmz.addExtraStats("goldLooted", this._rewards.gold);
	}
};
//=============================================================================
// Game_Action
//-----------------------------------------------------------------------------
// Automatic tracking for damage taken/dealt
//=============================================================================
//-----------------------------------------------------------------------------
// Track damage taken/dealt
//-----------------------------------------------------------------------------
const alias_CGMZ_ExtraStats_GameAction_executeHpDamage = Game_Action.prototype.executeHpDamage;
Game_Action.prototype.executeHpDamage = function(target, value) {
	alias_CGMZ_ExtraStats_GameAction_executeHpDamage.call(this, target, value);
	if(target.isActor() && value > 0 && $cgmz.isTrackingExtraStats()) {
		const oldDamageTaken = $gameVariables.value(CGMZ.ExtraStats.DamageTaken);
		$gameVariables.setValue(CGMZ.ExtraStats.DamageTaken, oldDamageTaken + value);
		$cgmz.addExtraStats("damageTaken", value);
	}
	else if(target.isEnemy() && value > 0 && $cgmz.isTrackingExtraStats()) {
		const oldDamageDealt = $gameVariables.value(CGMZ.ExtraStats.DamageDealt);
		$gameVariables.setValue(CGMZ.ExtraStats.DamageDealt, oldDamageDealt + value);
		$cgmz.addExtraStats("damageDealt", value);
	}
};
//-----------------------------------------------------------------------------
// Track damage taken/dealt
//-----------------------------------------------------------------------------
const alias_CGMZ_ExtraStats_GameAction_apply = Game_Action.prototype.apply;
Game_Action.prototype.apply = function(target) {
	alias_CGMZ_ExtraStats_GameAction_apply.call(this, target);
	if($cgmz.isTrackingExtraStats()) {
		if(target.isActor() && this.subject().isEnemy()) {
			$cgmz.addExtraStatsActor('PAtimesAttacked', target.actorId(), 1);
		} else if(target.isEnemy() && this.subject().isActor()) {
			$cgmz.addExtraStatsActor('PAtimesAttackedWith', this.subject().actorId(), 1);
			if(this.isSkill() && !CGMZ.ExtraStats.IgnoredSkills.includes(this.item().id)) {
				$cgmz.addExtraStatsActor('PAtimesUsedSkills', this.subject().actorId(), 1);
			}
		} else if(this.subject().isActor() && this.isSkill() && !CGMZ.ExtraStats.IgnoredSkills.includes(this.item().id)) {
			$cgmz.addExtraStatsActor('PAtimesUsedSkills', this.subject().actorId(), 1);
		}
	}
};
//=============================================================================
// CGMZ_Scene_Statistics
//-----------------------------------------------------------------------------
// Handle the statistics scene
//=============================================================================
function CGMZ_Scene_Statistics() {
    this.initialize.apply(this, arguments);
}
CGMZ_Scene_Statistics.prototype = Object.create(Scene_MenuBase.prototype);
CGMZ_Scene_Statistics.prototype.constructor = CGMZ_Scene_Statistics;
//-----------------------------------------------------------------------------
// Create statistics windows
//-----------------------------------------------------------------------------
CGMZ_Scene_Statistics.prototype.create = function() {
    Scene_MenuBase.prototype.create.call(this);
	this.createCategoryWindow();
	this.createDataWindow();
};
//-----------------------------------------------------------------------------
// Create category window
//-----------------------------------------------------------------------------
CGMZ_Scene_Statistics.prototype.createCategoryWindow = function() {
	const rect = this.categoryWindowRect();
    this._categoryWindow = new CGMZ_Window_ExtraStats_Category(rect);
	this._categoryWindow.setHandler('cancel', this.popScene.bind(this));
	this._categoryWindow.setHandler('ok', this.onCategoryOk.bind(this));
    this.addWindow(this._categoryWindow);
};
//-----------------------------------------------------------------------------
// Get category window rect
//-----------------------------------------------------------------------------
CGMZ_Scene_Statistics.prototype.categoryWindowRect = function() {
	const x = 0;
	const y = this.hasTouchUI() ? this.buttonAreaHeight() : 0;
	const width = Graphics.boxWidth;
	const height = this.calcWindowHeight(CGMZ.ExtraStats.CategoryRows, true);
	return new Rectangle(x, y, width, height);
};
//-----------------------------------------------------------------------------
// Create data window
//-----------------------------------------------------------------------------
CGMZ_Scene_Statistics.prototype.createDataWindow = function() {
	const rect = this.dataWindowRect();
    this._dataWindow = new CGMZ_Window_ExtraStats_Data(rect);
	this._dataWindow.setHandler('cancel', this.onDataCancel.bind(this));
	this._categoryWindow.setDataWindow(this._dataWindow);
    this.addWindow(this._dataWindow);
};
//-----------------------------------------------------------------------------
// Get data window rect
//-----------------------------------------------------------------------------
CGMZ_Scene_Statistics.prototype.dataWindowRect = function() {
	const x = 0;
	const y = this._categoryWindow.y + this._categoryWindow.height;
	const width = Graphics.boxWidth;
	const height = Graphics.boxHeight - y;
	return new Rectangle(x, y, width, height);
};
//-----------------------------------------------------------------------------
// On category window ok
//-----------------------------------------------------------------------------
CGMZ_Scene_Statistics.prototype.onCategoryOk = function() {
	this._categoryWindow.deactivate();
	this._dataWindow.activate();
};
//-----------------------------------------------------------------------------
// On data window cancel
//-----------------------------------------------------------------------------
CGMZ_Scene_Statistics.prototype.onDataCancel = function() {
	this._dataWindow.deactivate();
	this._categoryWindow.activate();
};
//-----------------------------------------------------------------------------
// Check if should make room for Touch UI
//-----------------------------------------------------------------------------
CGMZ_Scene_Statistics.prototype.hasTouchUI = function() {
	return !CGMZ.ExtraStats.DisableTouchUI || ConfigManager.touchUI;
};
//-----------------------------------------------------------------------------
// Get the scene's custom scene background for [CGMZ] Scene Backgrounds
// No need to check if Scene Backgrounds is installed because this custom func is only called by that plugin
//-----------------------------------------------------------------------------
CGMZ_Scene_Statistics.prototype.CGMZ_getCustomSceneBackground = function() {
	return $cgmzTemp.sceneBackgroundPresets[CGMZ.ExtraStats.SceneBackground];
};
//-----------------------------------------------------------------------------
// Get controls window preset for [CGMZ] Controls Window
// No need to check if plugin is installed because this custom func is only called by that plugin
//-----------------------------------------------------------------------------
CGMZ_Scene_Statistics.prototype.CGMZ_getControlsWindowOtherPreset = function() {
	return $cgmzTemp.getControlWindowPresetOther(CGMZ.ExtraStats.ControlsWindow);
};
//=============================================================================
// CGMZ_Window_ExtraStats_Category
//-----------------------------------------------------------------------------
// Selectable window for choosing a stat category in a list.
//=============================================================================
function CGMZ_Window_ExtraStats_Category(rect) {
    this.initialize.apply(this, arguments);
}
CGMZ_Window_ExtraStats_Category.prototype = Object.create(Window_Selectable.prototype);
CGMZ_Window_ExtraStats_Category.prototype.constructor = CGMZ_Window_ExtraStats_Category;
//-----------------------------------------------------------------------------
// Initialize
//-----------------------------------------------------------------------------
CGMZ_Window_ExtraStats_Category.prototype.initialize = function(rect) {
    Window_Selectable.prototype.initialize.call(this, rect);
	if(Imported.CGMZ_WindowBackgrounds && CGMZ.ExtraStats.CategoryWindowBackground) this.CGMZ_setWindowBackground(CGMZ.ExtraStats.CategoryWindowBackground);
	if(Imported.CGMZ_WindowSettings && CGMZ.ExtraStats.CategoryWindowSettings) this.CGMZ_setWindowSettings(CGMZ.ExtraStats.CategoryWindowSettings);
	this.refresh();
	this.activate();
	this.select(0);
};
//-----------------------------------------------------------------------------
// Max items
//-----------------------------------------------------------------------------
CGMZ_Window_ExtraStats_Category.prototype.maxItems = function() {
    return this._data ? this._data.length : 1;
};
//-----------------------------------------------------------------------------
// Max columns
//-----------------------------------------------------------------------------
CGMZ_Window_ExtraStats_Category.prototype.maxCols = function() {
    return CGMZ.ExtraStats.CategoryColumns;
};
//-----------------------------------------------------------------------------
// Current item
//-----------------------------------------------------------------------------
CGMZ_Window_ExtraStats_Category.prototype.item = function() {
    return this._data[this.index()];
};
//-----------------------------------------------------------------------------
// Refresh
//-----------------------------------------------------------------------------
CGMZ_Window_ExtraStats_Category.prototype.refresh = function() {
    this.makeItemList();
    Window_Selectable.prototype.refresh.call(this);
};
//-----------------------------------------------------------------------------
// Make item list
//-----------------------------------------------------------------------------
CGMZ_Window_ExtraStats_Category.prototype.makeItemList = function() {
	this._data = $cgmzTemp.getExtraStatsCategories();
};
//-----------------------------------------------------------------------------
// Get the command icon
//-----------------------------------------------------------------------------
CGMZ_Window_ExtraStats_Category.prototype.CGMZ_icon = function(index) {
	const id = this._data[index];
	const category = $cgmzTemp.getExtraStatsCategory(id);
	return category?.icon;
};
//-----------------------------------------------------------------------------
// Get selectable cgmz options
//-----------------------------------------------------------------------------
CGMZ_Window_ExtraStats_Category.prototype.CGMZ_getSelectableCGMZOptions = function(index) {
	const id = this._data[index];
	const category = $cgmzTemp.getExtraStatsCategory(id);
	if(category && category.img) {
		return {bg: category.img};
	}
	return Window_Selectable.prototype.CGMZ_getSelectableCGMZOptions.call(this, index);
};
//-----------------------------------------------------------------------------
// Draw item in list
//-----------------------------------------------------------------------------
CGMZ_Window_ExtraStats_Category.prototype.drawItem = function(index) {
    const id = this._data[index];
	const category = $cgmzTemp.getExtraStatsCategory(id);
	const icon = this.CGMZ_icon(index);
    const rect = this.itemRectWithPadding(index);
	if(icon) {
		const iconX = (CGMZ.ExtraStats.IconAlignment === 'left') ? rect.x : rect.x + rect.width - ImageManager.iconWidth;
		this.drawIcon(icon, iconX, rect.y + 2);
		rect.x += (ImageManager.iconWidth + 2) * (CGMZ.ExtraStats.IconAlignment === 'left');
		rect.width -= ImageManager.iconWidth + 2;
	}
    this.CGMZ_drawTextLine(category.displayName, rect.x, rect.y, rect.width, CGMZ.ExtraStats.CategoryAlignment);
};
//-----------------------------------------------------------------------------
// Set display window
//-----------------------------------------------------------------------------
CGMZ_Window_ExtraStats_Category.prototype.setDataWindow = function(dataWindow) {
    this._dataWindow = dataWindow;
    this.callUpdateHelp();
};
//-----------------------------------------------------------------------------
// See if can update data window
//-----------------------------------------------------------------------------
CGMZ_Window_ExtraStats_Category.prototype.callUpdateHelp = function() {
    if(this.active && this._dataWindow) {
		this._dataWindow.setItem(this.item());
	}
};
//=============================================================================
// CGMZ_Window_ExtraStats_Data
//-----------------------------------------------------------------------------
// Window displaying statistic data
//=============================================================================
function CGMZ_Window_ExtraStats_Data() {
	this.initialize.apply(this, arguments);
}
CGMZ_Window_ExtraStats_Data.prototype = Object.create(CGMZ_Window_Scrollable.prototype);
CGMZ_Window_ExtraStats_Data.prototype.constructor = CGMZ_Window_ExtraStats_Data;
//-----------------------------------------------------------------------------
// Initialize
//-----------------------------------------------------------------------------
CGMZ_Window_ExtraStats_Data.prototype.initialize = function(rect) {
	const heightMultiplier = 20; // maximum of 20 windows tall of data to scroll
	CGMZ_Window_Scrollable.prototype.initialize.call(this, rect, heightMultiplier, CGMZ.ExtraStats.ScrollWait, CGMZ.ExtraStats.ScrollSpeed, CGMZ.ExtraStats.AutoScroll, CGMZ.ExtraStats.ScrollDeceleration);
	if(Imported.CGMZ_WindowBackgrounds && CGMZ.ExtraStats.DataWindowBackground) this.CGMZ_setWindowBackground(CGMZ.ExtraStats.DataWindowBackground);
	if(Imported.CGMZ_WindowSettings && CGMZ.ExtraStats.DataWindowSettings) this.CGMZ_setWindowSettings(CGMZ.ExtraStats.DataWindowSettings);
	this._category = "";
	this._col = 0;
	this._itemWidth = this.contents.width / CGMZ.ExtraStats.DataColumns;
	this.deactivate();
};
//-----------------------------------------------------------------------------
// Set the category to be displayed (do nothing if already being displayed)
//-----------------------------------------------------------------------------
CGMZ_Window_ExtraStats_Data.prototype.setItem = function(category) {
	if(this._category === category) return;
	this._category = category;
	this.setupWindowForNewEntry();
	this.contentsBack.clear();
	this._col = 0;
	this.requestRefresh();
};
//-----------------------------------------------------------------------------
// Refresh
//-----------------------------------------------------------------------------
CGMZ_Window_ExtraStats_Data.prototype.refresh = function() {
	if(!this._category) return;
	const cat = $cgmzTemp.getExtraStatsCategory(this._category);
	if(!cat) return;
	let dividerIndex = 0;
	let customIndex = 0;
	for(const type of cat.displayInfo) {
		switch(type) {
			case 'Steps': this.drawStandardItem(CGMZ.ExtraStats.StepsText, CGMZ_Utils.numberSplit($gameParty.steps())); break;
			case 'Playtime': this.drawStandardItem(CGMZ.ExtraStats.PlaytimeText, $gameSystem.playtimeText()); break;
			case 'Saves': this.drawStandardItem(CGMZ.ExtraStats.SavesText, CGMZ_Utils.numberSplit($gameSystem.saveCount())); break;
			case 'Gold Spent': this.drawStandardItem(CGMZ.ExtraStats.GoldSpentText, CGMZ_Utils.numberSplit($cgmz.getExtraStats("goldSpent"))); break;
			case 'Gold Profit': this.drawStandardItem(CGMZ.ExtraStats.GoldProfitText, CGMZ_Utils.numberSplit($cgmz.getExtraStats("goldProfit"))); break;
			case 'Gold Looted': this.drawStandardItem(CGMZ.ExtraStats.GoldLootedText, CGMZ_Utils.numberSplit($cgmz.getExtraStats("goldLooted"))); break;
			case 'Items Bought': this.drawStandardItem(CGMZ.ExtraStats.ItemsBoughtText, CGMZ_Utils.numberSplit($cgmz.getExtraStats("itemsBought"))); break;
			case 'Items Sold': this.drawStandardItem(CGMZ.ExtraStats.ItemsSoldText, CGMZ_Utils.numberSplit($cgmz.getExtraStats("itemsSold"))); break;
			case 'Items Used': this.drawStandardItem(CGMZ.ExtraStats.ItemsUsedText, CGMZ_Utils.numberSplit($cgmz.getExtraStats("itemsUsed"))); break;
			case 'Item Used Most': this.drawStandardItem(CGMZ.ExtraStats.ItemUsedMostText, this.getObjName($cgmz.calculateMostCommonExtraStat("individualItemsUsed"), "item")); break;
			case 'Item Bought Most': this.drawStandardItem(CGMZ.ExtraStats.ItemBoughtMostText, this.getObjName($cgmz.calculateMostCommonExtraStat("individualItemsBought"), "item")); break;
			case 'Item Sold Most': this.drawStandardItem(CGMZ.ExtraStats.ItemSoldMostText, this.getObjName($cgmz.calculateMostCommonExtraStat("individualItemsSold"), "item")); break;
			case 'Weapon Bought Most': this.drawStandardItem(CGMZ.ExtraStats.WeaponBoughtMostText, this.getObjName($cgmz.calculateMostCommonExtraStat("individualWeaponsBought"), "weapon")); break;
			case 'Weapon Sold Most': this.drawStandardItem(CGMZ.ExtraStats.WeaponSoldMostText, this.getObjName($cgmz.calculateMostCommonExtraStat("individualWeaponsSold"), "weapon")); break;
			case 'Armor Bought Most': this.drawStandardItem(CGMZ.ExtraStats.ArmorBoughtMostText, this.getObjName($cgmz.calculateMostCommonExtraStat("individualArmorsBought"), "armor")); break;
			case 'Armor Sold Most': this.drawStandardItem(CGMZ.ExtraStats.ArmorSoldMostText, this.getObjName($cgmz.calculateMostCommonExtraStat("individualArmorsSold"), "armor")); break;
			case 'Enemy Defeated Most': this.drawStandardItem(CGMZ.ExtraStats.EnemyDefeatedMostText, this.getObjName($cgmz.calculateMostCommonExtraStat("individualEnemiesKilled"), "enemy")); break;
			case 'Enemies Defeated': this.drawStandardItem(CGMZ.ExtraStats.EnemiesDefeatedText, CGMZ_Utils.numberSplit($cgmz.calculateTotalExtraStat("individualEnemiesKilled"))); break;
			case 'Actor Died Most': this.drawStandardItem(CGMZ.ExtraStats.ActorDiedMostText, this.getActorName($cgmz.calculateMostCommonExtraStat("PAtimesDied"))); break;
			case 'Times Died': this.drawStandardItem(CGMZ.ExtraStats.TimesDiedText, CGMZ_Utils.numberSplit($cgmz.calculateTotalExtraStat("PAtimesDied"))); break;
			case 'Damage Dealt': this.drawStandardItem(CGMZ.ExtraStats.DamageDealtText, CGMZ_Utils.numberSplit($cgmz.getExtraStats("damageDealt"))); break;
			case 'Damage Taken': this.drawStandardItem(CGMZ.ExtraStats.DamageTakenText, CGMZ_Utils.numberSplit($cgmz.getExtraStats("damageTaken"))); break;
			case 'Custom Variable': this.drawCustom(cat.customVar[customIndex++]); break;
			case 'Divider': this.drawDivider(cat.dividers[dividerIndex++]); break;
			case 'Blank Line': this.addBlankLine(); break;
			case 'Custom Space': this._neededHeight += CGMZ.ExtraStats.CustomSpace; break;
			case 'Idle Time': this.drawIdleTime(); break;
			case 'New Game Plus Times': this.drawNewGamePlusTimes(); break;
			case 'Achievement Points': this.drawAchievementPoints(); break;
			case 'Achievements Earned': this.drawAchievementsEarned(); break;
		}
	}
};
//-----------------------------------------------------------------------------
// Get an object name
//-----------------------------------------------------------------------------
CGMZ_Window_ExtraStats_Data.prototype.getObjName = function(id, type) {
	const obj = CGMZ_Utils.lookupItem(type, id);
	if(obj) return obj.name;
	return CGMZ.ExtraStats.NoneText;
};
//-----------------------------------------------------------------------------
// Get an actor name
//-----------------------------------------------------------------------------
CGMZ_Window_ExtraStats_Data.prototype.getActorName = function(id) {
	if(id < 1) return CGMZ.ExtraStats.NoneText;
	return $gameActors.actor(id).name();
};
//-----------------------------------------------------------------------------
// Add a blank line
//-----------------------------------------------------------------------------
CGMZ_Window_ExtraStats_Data.prototype.addBlankLine = function() {
	if(this._col !== 0) {
		this._neededHeight += this.lineHeight();
		this._col = 0;
	}
	this._neededHeight += this.lineHeight();
};
//-----------------------------------------------------------------------------
// Adjust the current column
//-----------------------------------------------------------------------------
CGMZ_Window_ExtraStats_Data.prototype.adjustCol = function() {
	this._col++;
	if(this._col >= CGMZ.ExtraStats.DataColumns) {
		this._col = 0;
		this._neededHeight += this.lineHeight();
	}
};
//-----------------------------------------------------------------------------
// Get the current item's rect with padding
//-----------------------------------------------------------------------------
CGMZ_Window_ExtraStats_Data.prototype.itemRect = function() {
	const x = this._col * this._itemWidth + CGMZ.ExtraStats.DataColumnTextPadding;
	const y = this._neededHeight;
	const height = this.lineHeight();
	const width = this._itemWidth - CGMZ.ExtraStats.DataColumnTextPadding * 2;
	return new Rectangle(x, y, width, height);
};
//-----------------------------------------------------------------------------
// Get the current item's background rect with padding
//-----------------------------------------------------------------------------
CGMZ_Window_ExtraStats_Data.prototype.bgRect = function() {
	const x = this._col * this._itemWidth + CGMZ.ExtraStats.DataColumnPadding;
	const y = this._neededHeight + 2;
	const height = this.lineHeight() - 4;
	const width = this._itemWidth - CGMZ.ExtraStats.DataColumnPadding * 2;
	return new Rectangle(x, y, width, height);
};
//-----------------------------------------------------------------------------
// Draw a standard piece of data
//-----------------------------------------------------------------------------
CGMZ_Window_ExtraStats_Data.prototype.drawStandardItem = function(label, data) {
	const rect = this.itemRect();
	const bgRect = this.bgRect();
	this.CGMZ_drawBackgroundRectangle(bgRect);
	this.CGMZ_drawTextLine(`\\c[${CGMZ.ExtraStats.LabelColor}]${label}\\c[0]`, rect.x, rect.y, rect.width/2, 'left');
	this.CGMZ_drawTextLine(data, rect.x+rect.width/2, rect.y, rect.width/2, 'right');
	this.adjustCol();
};
//-----------------------------------------------------------------------------
// Draw a divider
//-----------------------------------------------------------------------------
CGMZ_Window_ExtraStats_Data.prototype.drawDivider = function(divider) {
	if(!divider) return;
	if(this._col !== 0) {
		this._neededHeight += this.lineHeight();
	}
	const opts = {drawDividers: CGMZ.ExtraStats.DividerLines, padding: (CGMZ.ExtraStats.DividerPadding >= 0) ? CGMZ.ExtraStats.DividerPadding : null};
	this.CGMZ_drawHeader(divider, this._neededHeight, CGMZ.ExtraStats.DividerColor1, CGMZ.ExtraStats.DividerColor2, opts);
	this._neededHeight += this.lineHeight();
	this._col = 0;
};
//-----------------------------------------------------------------------------
// Draw a custom stat
//-----------------------------------------------------------------------------
CGMZ_Window_ExtraStats_Data.prototype.drawCustom = function(custom) {
	if(!custom) return;
	const varVal = $gameVariables.value(custom.id);
	if(typeof varVal === 'number') {
		this.drawStandardItem(custom.label, CGMZ_Utils.numberSplit(varVal));
	} else {
		this.drawStandardItem(custom.label, String(varVal));
	}
};
//-----------------------------------------------------------------------------
// Draw idle time
//-----------------------------------------------------------------------------
CGMZ_Window_ExtraStats_Data.prototype.drawIdleTime = function() {
	if(!Imported.CGMZ_Idle) return;
	this.drawStandardItem(CGMZ.ExtraStats.IdleTimeText, CGMZ_Utils.timeSplit($cgmz.totalIdleFrames));
};
//-----------------------------------------------------------------------------
// Draw new game plus times
//-----------------------------------------------------------------------------
CGMZ_Window_ExtraStats_Data.prototype.drawNewGamePlusTimes = function() {
	if(!Imported.CGMZ_NewGamePlus) return;
	this.drawStandardItem(CGMZ.ExtraStats.NewGamePlusTimesText, CGMZ_Utils.numberSplit($cgmz.getNewGamePlusTimes()));
};
//-----------------------------------------------------------------------------
// Draw achievement points
//-----------------------------------------------------------------------------
CGMZ_Window_ExtraStats_Data.prototype.drawAchievementPoints = function() {
	if(!Imported.CGMZ_Achievements) return;
	this.drawStandardItem(CGMZ.ExtraStats.AchievementPointsText, CGMZ_Utils.numberSplit($cgmz.countEarnedAchievementPoints()));
};
//-----------------------------------------------------------------------------
// Draw achievements earned
//-----------------------------------------------------------------------------
CGMZ_Window_ExtraStats_Data.prototype.drawAchievementsEarned = function() {
	if(!Imported.CGMZ_Achievements) return;
	this.drawStandardItem(CGMZ.ExtraStats.AchievementsEarnedText, CGMZ_Utils.numberSplit($cgmz.countEarnedAchievements()));
};