/*:
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/savefile/
 * @target MZ
 * @base CGMZ_Core
 * @orderAfter CGMZ_Core
 * @plugindesc Changes the default save / load screens
 * @help
 * ============================================================================
 * For terms and conditions using this plugin in your game please visit:
 * https://www.caspergaming.com/terms-of-use/
 * ============================================================================
 * Become a Patron to get access to beta/alpha plugins plus other goodies!
 * https://www.patreon.com/CasperGamingRPGM
 * ============================================================================
 * Version: 1.6.0
 * ----------------------------------------------------------------------------
 * Compatibility: Only tested with my CGMZ plugins.
 * Made for RPG Maker MZ 1.10.0
 * ----------------------------------------------------------------------------
 * Description: This plugin modifies the save / load screens to show more
 * game information as well as providing additional customization options.
 * ----------------------------------------------------------------------------
 * Documentation:
 * ---------------------------Save Images--------------------------------------
 * If using a save image:
 * Image Dimensions (default resolution) are 582 x 160
 * Image height can be changed in parameters below
 * ----------------------------Note Tags---------------------------------------
 * To set location, use note tag <cgmzname:yourName>
 * To set the save image for each map, use note tag <cgmzsaveimg:yourImage>
 * These note tags go in the map properties note box.
 * ------------------------Custom Save Data------------------------------------
 * The custom save data are game variables. To assign a game variable to store
 * text, use the script call:
 * $gameVariables.setValue(13, "The Hero Returns");
 * where 13 would be the variable id, and the string would be what shows up on
 * the save screen.
 * --------------------------Integrations--------------------------------------
 * This plugin can display New Game Plus text from [CGMZ] New Game Plus. To 
 * set this up, add New Game Plus as a line item in the Display Info parameter.
 * This option in the Display Info parameter will have no effect if not using
 * [CGMZ] New Game Plus.
 * ------------------------------Saved Games-----------------------------------
 * This plugin is fully compatible with saved games. This means you can:
 *
 * ✓ Add this plugin to a saved game and it will work as expected
 * ✓ Change any plugin params and changes will be reflected in saved games
 * ✓ Remove the plugin with no issue to save data
 * -----------------------------Filename---------------------------------------
 * The filename for this plugin MUST remain CGMZ_SaveFile.js
 * This is what it comes as when downloaded. The filename is used to load
 * parameters and execute plugin commands. If you change it, things will begin
 * behaving incorrectly and your game will probably crash. Please do not
 * rename the js file.
 * --------------------------Latest Version------------------------------------
 * Hi all, this latest version adds support for drawing the save file [CGMZ]
 * Hardcore status on the info window, which you can add to the Display Info
 * parameter now. This will draw the hardcore text set up in [CGMZ] Hardcore
 * if the save file is hardcore, or be skipped if it is not hardcore.
 *
 * This version also fixes a bug that could cause a crash when trying to load
 * old save files from before this plugin was added, specifically with actor
 * names above characters or faces if set to draw those.
 *
 * Version 1.6.0
 * - Add support for drawing [CGMZ] Hardcore status
 * - Fix bug with old save files from before plugin added crashing
 *
 * @command Save File
 * @desc Saves the current game in the current save slot
 *
 * @arg Slot
 * @type number
 * @min -1
 * @desc Slot id to save over. If -1, will automatically pick slot to save in
 * @default -1
 *
 * @param Autosave Options
 * 
 * @param Show Autosave in Save Mode
 * @parent Autosave Options
 * @type boolean
 * @desc Show autosave in the save screen?
 * @default false
 * 
 * @param Show Autosave in Load Mode
 * @parent Autosave Options
 * @type boolean
 * @desc Show autosave in the load screen?
 * @default false
 *
 * @param File Options
 *
 * @param Max Save Files
 * @parent File Options
 * @type number
 * @min 1
 * @desc Change the number of files the player can select
 * @default 20
 *
 * @param Max Filename Length
 * @parent File Options
 * @type number
 * @min 1
 * @desc The number of characters possible to use in a file name
 * @default 20
 *
 * @param File Icon Unused
 * @parent File Options
 * @type icon
 * @desc Icon ID to show next to a File that has no save data. Set to 0 for no icon
 * @default 229
 *
 * @param File Icon Used
 * @parent File Options
 * @type icon
 * @desc Icon ID to show next to a File that hassave data. Set to 0 for no icon
 * @default 230
 *
 * @param Image Options
 *
 * @param Default Image
 * @parent Image Options
 * @type file
 * @dir img
 * @desc Default image to show (if showing images) if no other image exists
 *
 * @param Image Height
 * @parent Image Options
 * @type number
 * @desc Force the height of save images to be this height (0 = height of the image file)
 * @default 0
 *
 * @param Stretch Image
 * @parent Image Options
 * @type boolean
 * @desc If true, will stretch the width to fit the window width
 * @default false
 *
 * @param Center Image
 * @parent Image Options
 * @type boolean
 * @desc If true, will center the image on the window
 * @default false
 *
 * @param Location Fade Opacity
 * @parent Image Options
 * @type number
 * @desc Opacity (0-255) of the black rectangle behind the location
 * @default 120
 * 
 * @param Custom Options
 * 
 * @param Custom Save Variables
 * @parent Custom Options
 * @type struct<CustomSaveInfo>[]
 * @desc Set up custom save variables here
 * @default []
 * 
 * @param Window Options
 *
 * @param Show Delete Option
 * @parent Window Options
 * @type boolean
 * @desc If true, will allow the player to delete save files
 * @default false
 *
 * @param Show Rename Option
 * @parent Window Options
 * @type boolean
 * @desc If true, will allow the player to rename save files
 * @default false
 *
 * @param Show Confirm Window
 * @parent Window Options
 * @type boolean
 * @desc If true, will ask the player to confirm they want to save over that file
 * @default false
 *
 * @param Show Delete Confirm Window
 * @parent Window Options
 * @type boolean
 * @desc If true, will ask the player to confirm they want to delete that file
 * @default true
 * 
 * @param Display Options
 *
 * @param Label Color
 * @parent Display Options
 * @default 16
 * @type color
 * @desc The color to use for information headers (example - location: or gold:)
 *
 * @param Disable Touch UI Space
 * @parent Display Options
 * @type boolean
 * @desc If true, will not leave space for Touch UI buttons if Touch UI is disabled
 * @default false
 *
 * @param List Window Width
 * @parent Display Options
 * @type number
 * @min 0
 * @max 100
 * @desc The width (as a percentage of screen width) to make the List Window
 * @default 25
 *
 * @param Scroll Speed
 * @parent Display Options
 * @type number
 * @min 0
 * @desc Speed at which the save display window scrolls (if needed)
 * @default 1
 *
 * @param Scroll Wait
 * @parent Display Options
 * @type number
 * @min 0
 * @desc Amount of time (in frames) to wait before beginning to scroll
 * @default 300
 *
 * @param Scroll Deceleration
 * @parent Display Options
 * @type number
 * @min 0.01
 * @max 0.99
 * @decimals 2
 * @desc Rate of deceleration after letting go of touch
 * @default 0.92
 *
 * @param Auto Scroll
 * @parent Display Options
 * @type boolean
 * @desc Determine if the display window should automatically scroll after so long of no user input
 * @default true
 *
 * @param Show Step Anim
 * @parent Display Options
 * @type boolean
 * @desc If true, will show step animation for characters display item
 * @default true
 *
 * @param Character Height
 * @parent Display Options
 * @type number
 * @min 0
 * @desc The vertical space to give the characters on the save file window
 * @default 72
 *
 * @param Display Info
 * @parent Display Options
 * @type select[]
 * @option Image
 * @option Location
 * @option Image And Location
 * @option Date
 * @option Playtime
 * @option Gold
 * @option Custom Info
 * @option Custom Text
 * @option Custom Header
 * @option Faces
 * @option Characters
 * @option Achievements
 * @option Achievement Points
 * @option Blank Line
 * @option Custom Space
 * @option New Game Plus
 * @option Hardcore
 * @desc Determines the order and what info the display window shows.
 * @default ["Date","Playtime","Faces"]
 *
 * @param Custom Space
 * @parent Display Options
 * @type number
 * @min 0
 * @desc Amount of blank vertical space to leave when Custom Space is used in Display Info param
 * @default 6
 *
 * @param Custom Texts
 * @parent Display Options
 * @type multiline_string[]
 * @desc Custom text elements to draw on the save file display window, in order listed here
 * @default []
 *
 * @param Custom Header Options
 * @parent Display Options
 *
 * @param Custom Headers
 * @parent Custom Header Options
 * @type text[]
 * @desc Custom header elements to draw on the save file display window, in order listed here
 * @default []
 *
 * @param Header Color 1
 * @parent Custom Header Options
 * @default 0
 * @type color
 * @desc The first color to use for dividing header element line gradients
 *
 * @param Header Color 2
 * @parent Custom Header Options
 * @default 1
 * @type color
 * @desc The second color to use for dividing header element line gradients
 *
 * @param Header Padding
 * @parent Custom Header Options
 * @default 12
 * @type number
 * @min 0
 * @desc The padding of header dividing lines
 *
 * @param Draw Header Dividers
 * @parent Custom Header Options
 * @default true
 * @type boolean
 * @desc If true, will draw dividing lines on each side of the header element
 *
 * @param Text Options
 * 
 * @param Hide Levels
 * @parent Text Options
 * @type boolean
 * @desc If true, actor level and associated text will not be displayed on face/character images
 * @default false
 * 
 * @param Empty Text
 * @parent Text Options
 * @type multiline_string
 * @desc Text to show when no save file information exists
 * @default No save data exists.
 * 
 * @param Location Text
 * @parent Text Options
 * @desc Text to describe the "location" string
 * @default Location: 
 * 
 * @param Playtime Text
 * @parent Text Options
 * @desc Text to describe the "playtime" string
 * @default Playtime: 
 * 
 * @param Gold Text
 * @parent Text Options
 * @desc Text to describe the "gold" string
 * @default Gold: 
 * 
 * @param Achievement Text
 * @parent Text Options
 * @desc Text to describe the "achievement" string. Requires CGMZ Achievements
 * @default Achievements: 
 * 
 * @param Achievement Point Text
 * @parent Text Options
 * @desc Text to describe the "achievement points" string. Requires CGMZ Achievements
 * @default Achievement Points: 
 * 
 * @param Level Text
 * @parent Text Options
 * @desc Text to describe the "level" string
 * @default Lv. 
 * 
 * @param Confirm Window Text
 * @parent Text Options
 * @desc Text at the top of the confirm window
 * @default Are You Sure?
 * 
 * @param Confirm Option Text
 * @parent Text Options
 * @desc Text to describe the "Confirm" string
 * @default Confirm
 * 
 * @param Cancel Option Text
 * @parent Text Options
 * @desc Text to describe the "Cancel" string
 * @default Cancel
 * 
 * @param Delete Option Text
 * @parent Text Options
 * @desc Text to describe the "Delete" string
 * @default Delete
 * 
 * @param Rename Option Text
 * @parent Text Options
 * @desc Text to describe the "Rename" string
 * @default Rename
 * 
 * @param Save Text
 * @parent Text Options
 * @desc Text to describe the "Save" string
 * @default Save
 * 
 * @param Load Text
 * @parent Text Options
 * @desc Text to describe the "Load" string
 * @default Load
 *
 * @param Integrations
 *
 * @param Load Background
 * @parent Integrations
 * @desc The [CGMZ] Scene Backgrounds preset id to use in the load screen
 *
 * @param Save Background
 * @parent Integrations
 * @desc The [CGMZ] Scene Backgrounds preset id to use in the save screen
 *
 * @param List Window Settings
 * @parent Integrations
 * @desc The [CGMZ] Window Settings preset id to use in the file screen - list window
 *
 * @param Display Window Settings
 * @parent Integrations
 * @desc The [CGMZ] Window Settings preset id to use in the file screen - display window
 *
 * @param Help Window Settings
 * @parent Integrations
 * @desc The [CGMZ] Window Settings preset id to use in the file screen - help window
 *
 * @param Option Window Settings
 * @parent Integrations
 * @desc The [CGMZ] Window Settings preset id to use in the file screen - option window
 *
 * @param Confirm Window Settings
 * @parent Integrations
 * @desc The [CGMZ] Window Settings preset id to use in the file screen - confirm window
 *
 * @param Delete Confirm Window Settings
 * @parent Integrations
 * @desc The [CGMZ] Window Settings preset id to use in the file screen - delete confirm window
 *
 * @param Name Window Settings
 * @parent Integrations
 * @desc The [CGMZ] Window Settings preset id to use in the file screen - name window
 *
 * @param Input Window Settings
 * @parent Integrations
 * @desc The [CGMZ] Window Settings preset id to use in the file screen - input window
 *
 * @param List Window Background
 * @parent Integrations
 * @desc The [CGMZ] Window Backgrounds preset id to use in the file screen - list window
 *
 * @param Display Window Background
 * @parent Integrations
 * @desc The [CGMZ] Window Backgrounds preset id to use in the file screen - display window
 *
 * @param Help Window Background
 * @parent Integrations
 * @desc The [CGMZ] Window Backgrounds preset id to use in the file screen - help window
 *
 * @param Option Window Background
 * @parent Integrations
 * @desc The [CGMZ] Window Backgrounds preset id to use in the file screen - option window
 *
 * @param Confirm Window Background
 * @parent Integrations
 * @desc The [CGMZ] Window Backgrounds preset id to use in the file screen - confirm window
 *
 * @param Delete Confirm Window Background
 * @parent Integrations
 * @desc The [CGMZ] Window Backgrounds preset id to use in the file screen - delete confirm window
 *
 * @param Name Window Background
 * @parent Integrations
 * @desc The [CGMZ] Window Backgrounds preset id to use in the file screen - name window
 *
 * @param Input Window Background
 * @parent Integrations
 * @desc The [CGMZ] Window Backgrounds preset id to use in the file screen - input window
*/
/*~struct~CustomSaveInfo:
 * @param Variable
 * @type variable
 * @desc The variable value to show as a custom option in the save screen
 *
 * @param Description
 * @desc The text to describe the variable (ex: Chapter: )
 *
 * @param Trailing Text
 * @desc Text to show after the variable (ex: %)
 *
 * @param Icon
 * @type number
 * @min -1
 * @desc The icon to show next to the variable. Set to -1 to not use
 * @default -1
*/
/*:es
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/savefile/
 * @target MZ
 * @base CGMZ_Core
 * @orderAfter CGMZ_Core
 * @plugindesc Cambia las pantallas predeterminadas de guardar/cargar
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
 * Versión: 1.6.0
 * ----------------------------------------------------------------------------
 * Compatibilidad: Sólo probado con mis CGMZ plugins.
 * Hecho para RPG Maker MZ 1.10.0
 * ----------------------------------------------------------------------------
 * Descripción: este complemento modifica las pantallas de guardar/cargar para 
 * mostrar más información del juego, además de proporcionar opciones de 
 * personalización adicionales.
 * ----------------------------------------------------------------------------
 * Documentación:
 * -------------------------Guardar Imágenes-----------------------------------
 * Si usas una imagen guardada:
 * Las dimensiones de la imagen (resolución predeterminada) son 582 x 160
 * La altura de la imagen se puede cambiar en los parámetros a continuación
 * Las imágenes deben colocarse en la carpeta "imágenes"
 * ------------------------Etiquetas de nota-----------------------------------
 * Para establecer la ubicación, use la etiqueta de nota  <cgmzname:tuNombre>
 * Para configurar la imagen guardada para cada mapa, use la etiqueta de nota
 * <cgmzsaveimg:tuImagen>
 * Estas etiquetas de notas van en el cuadro de notas de propiedades del mapa.
 * --------------------Guardar datos personalizados----------------------------
 * Los datos guardados personalizados son variables del juego. Para asignar una 
 * variable de juego para almacenar texto, utilice la llamada de secuencia de
 * comandos:
 * $gameVariables.setValue(13, "El Heroe regresa");
 * donde 13 sería el id de la variable y la cadena sería lo que aparece en
 * la pantalla de guardar.
 * --------------------------Integrations--------------------------------------
 * This plugin can display New Game Plus text from [CGMZ] New Game Plus. To 
 * set this up, add New Game Plus as a line item in the Display Info parameter.
 * This option in the Display Info parameter will have no effect if not using
 * [CGMZ] New Game Plus.
 * --------------------------Latest Version------------------------------------
 * Hi all, this latest version adds support for drawing the save file [CGMZ]
 * Hardcore status on the info window, which you can add to the Display Info
 * parameter now. This will draw the hardcore text set up in [CGMZ] Hardcore
 * if the save file is hardcore, or be skipped if it is not hardcore.
 *
 * This version also fixes a bug that could cause a crash when trying to load
 * old save files from before this plugin was added, specifically with actor
 * names above characters or faces if set to draw those.
 *
 * Version 1.6.0
 * - Add support for drawing [CGMZ] Hardcore status
 * - Fix bug with old save files from before plugin added crashing
 *
 * @command Save File
 * @desc Saves the current game in the current save slot
 *
 * @arg Slot
 * @type number
 * @min -1
 * @desc Slot id to save over. If -1, will automatically pick slot to save in
 * @default -1
 *
 * @param Autosave Options
 * @text Opciones de autoguardado
 * 
 * @param Show Autosave in Save Mode
 * @text Mostrar guardado automático en el modo Guardar
 * @parent Autosave Options
 * @type boolean
 * @desc Mostrar guardado automático en la pantalla de guardar.
 * @default false
 * 
 * @param Show Autosave in Load Mode
 * @parent Autosave Options
 * @type boolean
 * @desc Show autosave in the load screen?
 * @default false
 *
 * @param File Options
 * @text Opciones de archivo
 *
 * @param Max Save Files
 * @text Máximo de archivos guardados
 * @parent File Options
 * @type number
 * @min 1
 * @desc Cambiar la cantidad de archivos que el jugador puede seleccionar.
 * @default 20
 *
 * @param Max Filename Length
 * @parent File Options
 * @type number
 * @min 1
 * @desc The number of characters possible to use in a file name
 * @default 20
 *
 * @param File Icon Unused
 * @text Icono de archivo sin usar
 * @parent File Options
 * @type icon
 * @desc ID de icono para mostrar junto a un archivo que no tiene datos guardados. Establecer en 0 para ningún icono.
 * @default 229
 *
 * @param File Icon Used
 * @text Icono de archivo utilizado
 * @parent File Options
 * @type icon
 * @desc ID de icono para mostrar junto a un archivo que tiene datos guardados. Establecer en 0 para ningún icono.
 * @default 230
 *
 * @param Image Options
 * @text Opciones de Imagen
 *
 * @param Default Image
 * @text Imagen por defecto
 * @parent Image Options
 * @type file
 * @dir img
 * @desc Imagen predeterminada para mostrar (si se muestran imágenes) si no existe otra imagen.
 *
 * @param Image Height
 * @text Altura de Imagen
 * @parent Image Options
 * @type number
 * @desc Altura de la imagen a mostrar. 0 = height of image file
 * @default 0
 *
 * @param Stretch Image
 * @parent Image Options
 * @type boolean
 * @desc If true, will stretch the width to fit the window width
 * @default false
 *
 * @param Center Image
 * @parent Image Options
 * @type boolean
 * @desc If true, will center the image on the window
 * @default false
 *
 * @param Location Fade Opacity
 * @text Ubicación Desvanecimiento de Opacidad
 * @parent Image Options
 * @type number
 * @desc Opacidad (0-255) del rectángulo negro detrás de la ubicación.
 * @default 120
 * 
 * @param Custom Options
 * @text Opciones personalizadas
 * 
 * @param Custom Save Variables
 * @text Variables de guardado personalizadas
 * @parent Custom Options
 * @type struct<CustomSaveInfo>[]
 * @desc Configure las variables de guardado personalizadas aquí.
 * @default []
 * 
 * @param Window Options
 *
 * @param Show Delete Option
 * @parent Window Options
 * @type boolean
 * @desc If true, will allow the player to delete save files
 * @default false
 *
 * @param Show Rename Option
 * @parent Window Options
 * @type boolean
 * @desc If true, will allow the player to rename save files
 * @default false
 *
 * @param Show Confirm Window
 * @parent Window Options
 * @type boolean
 * @desc If true, will ask the player to confirm they want to save over that file
 * @default false
 *
 * @param Show Delete Confirm Window
 * @parent Window Options
 * @type boolean
 * @desc If true, will ask the player to confirm they want to delete that file
 * @default true
 * 
 * @param Display Options
 * @text Opciones de pantalla
 *
 * @param Label Color
 * @text Color del encabezado
 * @parent Display Options
 * @type color
 * @default 16
 * @desc El color que se usará para los encabezados de información (ejemplo: ubicación: u oro:).
 *
 * @param Disable Touch UI Space
 * @parent Display Options
 * @type boolean
 * @desc If true, will not leave space for Touch UI buttons if Touch UI is disabled
 * @default false
 *
 * @param List Window Width
 * @parent Display Options
 * @type number
 * @min 0
 * @max 100
 * @desc The width (as a percentage of screen width) to make the List Window
 * @default 25
 *
 * @param Scroll Speed
 * @parent Display Options
 * @type number
 * @min 0
 * @desc Speed at which the save display window scrolls (if needed)
 * @default 1
 *
 * @param Scroll Wait
 * @parent Display Options
 * @type number
 * @min 0
 * @desc Amount of time (in frames) to wait before beginning to scroll
 * @default 300
 *
 * @param Scroll Deceleration
 * @parent Display Options
 * @type number
 * @min 0.01
 * @max 0.99
 * @decimals 2
 * @desc Rate of deceleration after letting go of touch
 * @default 0.92
 *
 * @param Auto Scroll
 * @parent Display Options
 * @type boolean
 * @desc Determine if the display window should automatically scroll after so long of no user input
 * @default true
 *
 * @param Show Step Anim
 * @parent Display Options
 * @type boolean
 * @desc If true, will show step animation for characters display item
 * @default true
 *
 * @param Character Height
 * @parent Display Options
 * @type number
 * @min 0
 * @desc The vertical space to give the characters on the save file window
 * @default 72
 *
 * @param Display Info
 * @parent Display Options
 * @type select[]
 * @option Image
 * @option Location
 * @option Image And Location
 * @option Date
 * @option Playtime
 * @option Gold
 * @option Custom Info
 * @option Custom Text
 * @option Custom Headers
 * @option Faces
 * @option Characters
 * @option Achievements
 * @option Achievement Points
 * @option Blank Line
 * @option Custom Space
 * @option New Game Plus
 * @option Hardcore
 * @desc Determines the order and what info the display window shows.
 * @default ["Date","Playtime","Faces"]
 *
 * @param Custom Space
 * @parent Display Options
 * @type number
 * @min 0
 * @desc Amount of blank vertical space to leave when Custom Space is used in Display Info param
 * @default 6
 *
 * @param Custom Texts
 * @parent Display Options
 * @type multiline_string[]
 * @desc Custom text elements to draw on the save file display window, in order listed here
 * @default []
 *
 * @param Custom Header Options
 * @parent Display Options
 *
 * @param Custom Headers
 * @parent Custom Header Options
 * @type text[]
 * @desc Custom header elements to draw on the save file display window, in order listed here
 * @default []
 *
 * @param Header Color 1
 * @parent Custom Header Options
 * @default 0
 * @type color
 * @desc The first color to use for dividing header element line gradients
 *
 * @param Header Color 2
 * @parent Custom Header Options
 * @default 1
 * @type color
 * @desc The second color to use for dividing header element line gradients
 *
 * @param Header Padding
 * @parent Custom Header Options
 * @default 12
 * @type number
 * @min 0
 * @desc The padding of header dividing lines
 *
 * @param Draw Header Dividers
 * @parent Custom Header Options
 * @default true
 * @type boolean
 * @desc If true, will draw dividing lines on each side of the header element
 *
 * @param Text Options
 * @text Opciones de texto
 * 
 * @param Hide Levels
 * @parent Text Options
 * @type boolean
 * @desc If true, actor level and associated text will not be displayed on face/character images
 * @default false
 * 
 * @param Empty Text
 * @text Texto Vacío
 * @parent Text Options
 * @type multiline_string
 * @desc Texto para mostrar cuando no existe información de archivo guardado.
 * @default No save data exists.
 * 
 * @param Location Text
 * @text Texto de Ubicación
 * @parent Text Options
 * @desc Texto para describir la cadena de "ubicación".
 * @default Location: 
 * 
 * @param Playtime Text
 * @text Texto de tiempo de juego
 * @parent Text Options
 * @desc Texto para describir la cadena "tiempo de juego".
 * @default Playtime: 
 * 
 * @param Gold Text
 * @text Texto Oro
 * @parent Text Options
 * @desc Texto para describir la cadena "oro".
 * @default Gold: 
 * 
 * @param Achievement Text
 * @parent Text Options
 * @desc Text to describe the "achievement" string. Requires CGMZ Achievements
 * @default Achievements: 
 * 
 * @param Achievement Point Text
 * @parent Text Options
 * @desc Text to describe the "achievement points" string. Requires CGMZ Achievements
 * @default Achievement Points: 
 * 
 * @param Level Text
 * @text Texto de nivel
 * @parent Text Options
 * @desc Texto para describir la cadena de "nivel".
 * @default Lv. 
 * 
 * @param Confirm Window Text
 * @parent Text Options
 * @desc Text at the top of the confirm window
 * @default Are You Sure?
 * 
 * @param Confirm Option Text
 * @parent Text Options
 * @desc Text to describe the "Confirm" string
 * @default Confirm
 * 
 * @param Cancel Option Text
 * @parent Text Options
 * @desc Text to describe the "Cancel" string
 * @default Cancel
 * 
 * @param Delete Option Text
 * @parent Text Options
 * @desc Text to describe the "Delete" string
 * @default Delete
 * 
 * @param Rename Option Text
 * @parent Text Options
 * @desc Text to describe the "Rename" string
 * @default Rename
 * 
 * @param Save Text
 * @parent Text Options
 * @desc Text to describe the "Save" string
 * @default Save
 * 
 * @param Load Text
 * @parent Text Options
 * @desc Text to describe the "Load" string
 * @default Load
 *
 * @param Integrations
 *
 * @param Load Background
 * @parent Integrations
 * @desc The [CGMZ] Scene Backgrounds preset id to use in the load screen
 *
 * @param Save Background
 * @parent Integrations
 * @desc The [CGMZ] Scene Backgrounds preset id to use in the save screen
 *
 * @param List Window Settings
 * @parent Integrations
 * @desc The [CGMZ] Window Settings preset id to use in the file screen - list window
 *
 * @param Display Window Settings
 * @parent Integrations
 * @desc The [CGMZ] Window Settings preset id to use in the file screen - display window
 *
 * @param Help Window Settings
 * @parent Integrations
 * @desc The [CGMZ] Window Settings preset id to use in the file screen - help window
 *
 * @param Option Window Settings
 * @parent Integrations
 * @desc The [CGMZ] Window Settings preset id to use in the file screen - option window
 *
 * @param Confirm Window Settings
 * @parent Integrations
 * @desc The [CGMZ] Window Settings preset id to use in the file screen - confirm window
 *
 * @param Delete Confirm Window Settings
 * @parent Integrations
 * @desc The [CGMZ] Window Settings preset id to use in the file screen - delete confirm window
 *
 * @param Name Window Settings
 * @parent Integrations
 * @desc The [CGMZ] Window Settings preset id to use in the file screen - name window
 *
 * @param Input Window Settings
 * @parent Integrations
 * @desc The [CGMZ] Window Settings preset id to use in the file screen - input window
 *
 * @param List Window Background
 * @parent Integrations
 * @desc The [CGMZ] Window Backgrounds preset id to use in the file screen - list window
 *
 * @param Display Window Background
 * @parent Integrations
 * @desc The [CGMZ] Window Backgrounds preset id to use in the file screen - display window
 *
 * @param Help Window Background
 * @parent Integrations
 * @desc The [CGMZ] Window Backgrounds preset id to use in the file screen - help window
 *
 * @param Option Window Background
 * @parent Integrations
 * @desc The [CGMZ] Window Backgrounds preset id to use in the file screen - option window
 *
 * @param Confirm Window Background
 * @parent Integrations
 * @desc The [CGMZ] Window Backgrounds preset id to use in the file screen - confirm window
 *
 * @param Delete Confirm Window Background
 * @parent Integrations
 * @desc The [CGMZ] Window Backgrounds preset id to use in the file screen - delete confirm window
 *
 * @param Name Window Background
 * @parent Integrations
 * @desc The [CGMZ] Window Backgrounds preset id to use in the file screen - name window
 *
 * @param Input Window Background
 * @parent Integrations
 * @desc The [CGMZ] Window Backgrounds preset id to use in the file screen - input window
*/
/*~struct~CustomSaveInfo:es
 * @param Variable
 * @text Variable
 * @type variable
 * @desc El valor de la variable para mostrar como una opción personalizada en la pantalla de guardar.
 *
 * @param Description
 * @text Descripción
 * @desc El texto para describir la variable (ej: Capítulo: ).
 *
 * @param Trailing Text
 * @text Texto final
 * @desc Texto a mostrar después de la variable (ej: %).
 *
 * @param Icon
 * @text Icono
 * @type number
 * @min -1
 * @desc El icono que se muestra junto a la variable. Establecer en -1 para no usar.
 * @default -1
*/
Imported.CGMZ_SaveFile = true;
CGMZ.Versions["Save File"] = "1.6.0";
CGMZ.SaveFile = {};
CGMZ.SaveFile.parameters = PluginManager.parameters('CGMZ_SaveFile');
CGMZ.SaveFile.DefaultImage = CGMZ.SaveFile.parameters["Default Image"];
CGMZ.SaveFile.LoadBackground = CGMZ.SaveFile.parameters["Load Background"];
CGMZ.SaveFile.SaveBackground = CGMZ.SaveFile.parameters["Save Background"];
CGMZ.SaveFile.ListWindowSettings = CGMZ.SaveFile.parameters["List Window Settings"];
CGMZ.SaveFile.DisplayWindowSettings = CGMZ.SaveFile.parameters["Display Window Settings"];
CGMZ.SaveFile.HelpWindowSettings = CGMZ.SaveFile.parameters["Help Window Settings"];
CGMZ.SaveFile.ConfirmWindowSettings = CGMZ.SaveFile.parameters["Confirm Window Settings"];
CGMZ.SaveFile.DeleteConfirmWindowSettings = CGMZ.SaveFile.parameters["Delete Confirm Window Settings"];
CGMZ.SaveFile.OptionWindowSettings = CGMZ.SaveFile.parameters["Option Window Settings"];
CGMZ.SaveFile.NameWindowSettings = CGMZ.SaveFile.parameters["Name Window Settings"];
CGMZ.SaveFile.InputWindowSettings = CGMZ.SaveFile.parameters["Input Window Settings"];
CGMZ.SaveFile.ListWindowBackground = CGMZ.SaveFile.parameters["List Window Background"];
CGMZ.SaveFile.DisplayWindowBackground = CGMZ.SaveFile.parameters["Display Window Background"];
CGMZ.SaveFile.HelpWindowBackground = CGMZ.SaveFile.parameters["Help Window Background"];
CGMZ.SaveFile.ConfirmWindowBackground = CGMZ.SaveFile.parameters["Confirm Window Background"];
CGMZ.SaveFile.DeleteConfirmWindowBackground = CGMZ.SaveFile.parameters["Delete Confirm Window Background"];
CGMZ.SaveFile.OptionWindowBackground = CGMZ.SaveFile.parameters["Option Window Background"];
CGMZ.SaveFile.NameWindowBackground = CGMZ.SaveFile.parameters["Name Window Background"];
CGMZ.SaveFile.InputWindowBackground = CGMZ.SaveFile.parameters["Input Window Background"];
CGMZ.SaveFile.EmptyText = CGMZ.SaveFile.parameters["Empty Text"];
CGMZ.SaveFile.LocationText = CGMZ.SaveFile.parameters["Location Text"];
CGMZ.SaveFile.PlaytimeText = CGMZ.SaveFile.parameters["Playtime Text"];
CGMZ.SaveFile.GoldText = CGMZ.SaveFile.parameters["Gold Text"];
CGMZ.SaveFile.AchievementText = CGMZ.SaveFile.parameters["Achievement Text"];
CGMZ.SaveFile.AchievementPointText = CGMZ.SaveFile.parameters["Achievement Point Text"];
CGMZ.SaveFile.LevelText = CGMZ.SaveFile.parameters["Level Text"];
CGMZ.SaveFile.ConfirmWindowText = CGMZ.SaveFile.parameters["Confirm Window Text"];
CGMZ.SaveFile.ConfirmText = CGMZ.SaveFile.parameters["Confirm Option Text"];
CGMZ.SaveFile.CancelText = CGMZ.SaveFile.parameters["Cancel Option Text"];
CGMZ.SaveFile.DeleteText = CGMZ.SaveFile.parameters["Delete Option Text"];
CGMZ.SaveFile.RenameText = CGMZ.SaveFile.parameters["Rename Option Text"];
CGMZ.SaveFile.SaveText = CGMZ.SaveFile.parameters["Save Text"];
CGMZ.SaveFile.LoadText = CGMZ.SaveFile.parameters["Load Text"];
CGMZ.SaveFile.MaxSaveFiles = Number(CGMZ.SaveFile.parameters["Max Save Files"]);
CGMZ.SaveFile.MaxFilenameLength = Number(CGMZ.SaveFile.parameters["Max Filename Length"]);
CGMZ.SaveFile.SaveIconUsed = Number(CGMZ.SaveFile.parameters["File Icon Used"]);
CGMZ.SaveFile.SaveIconUnused = Number(CGMZ.SaveFile.parameters["File Icon Unused"]);
CGMZ.SaveFile.ImageHeight = Number(CGMZ.SaveFile.parameters["Image Height"]);
CGMZ.SaveFile.FadeSpriteOpacity = Number(CGMZ.SaveFile.parameters["Location Fade Opacity"]);
CGMZ.SaveFile.LabelColor = Number(CGMZ.SaveFile.parameters["Label Color"]);
CGMZ.SaveFile.CharHeight = Number(CGMZ.SaveFile.parameters["Character Height"]);
CGMZ.SaveFile.ListWindowWidth = Number(CGMZ.SaveFile.parameters["List Window Width"]);
CGMZ.SaveFile.ScrollSpeed = Number(CGMZ.SaveFile.parameters["Scroll Speed"]);
CGMZ.SaveFile.ScrollWait = Number(CGMZ.SaveFile.parameters["Scroll Wait"]);
CGMZ.SaveFile.CustomSpace = Number(CGMZ.SaveFile.parameters["Custom Space"]);
CGMZ.SaveFile.HeaderColor1 = Number(CGMZ.SaveFile.parameters["Header Color 1"]);
CGMZ.SaveFile.HeaderColor2 = Number(CGMZ.SaveFile.parameters["Header Color 2"]);
CGMZ.SaveFile.HeaderPadding = Number(CGMZ.SaveFile.parameters["Header Padding"]);
CGMZ.SaveFile.ScrollDeceleration = parseFloat(CGMZ.SaveFile.parameters["Scroll Deceleration"]);
CGMZ.SaveFile.ShowAutosaveInSaveMode = (CGMZ.SaveFile.parameters["Show Autosave in Save Mode"] === "true");
CGMZ.SaveFile.ShowAutosaveInLoadMode = (CGMZ.SaveFile.parameters["Show Autosave in Load Mode"] === "true");
CGMZ.SaveFile.DisableTouchUISpace = (CGMZ.SaveFile.parameters["Disable Touch UI Space"] === "true");
CGMZ.SaveFile.ShowStepAnim = (CGMZ.SaveFile.parameters["Show Step Anim"] === "true");
CGMZ.SaveFile.ShowConfirmWindow = (CGMZ.SaveFile.parameters["Show Confirm Window"] === "true");
CGMZ.SaveFile.ShowDeleteConfirmWindow = (CGMZ.SaveFile.parameters["Show Delete Confirm Window"] === "true");
CGMZ.SaveFile.ShowDeleteOption = (CGMZ.SaveFile.parameters["Show Delete Option"] === "true");
CGMZ.SaveFile.ShowRenameOption = (CGMZ.SaveFile.parameters["Show Rename Option"] === "true");
CGMZ.SaveFile.AutoScroll = (CGMZ.SaveFile.parameters["Auto Scroll"] === "true");
CGMZ.SaveFile.StretchImage = (CGMZ.SaveFile.parameters["Stretch Image"] === "true");
CGMZ.SaveFile.CenterImage = (CGMZ.SaveFile.parameters["Center Image"] === "true");
CGMZ.SaveFile.HideLevels = (CGMZ.SaveFile.parameters["Hide Levels"] === "true");
CGMZ.SaveFile.DrawHeaderDividers = (CGMZ.SaveFile.parameters["Draw Header Dividers"] === "true");
CGMZ.SaveFile.CustomSaveInfo = CGMZ_Utils.parseJSON(CGMZ.SaveFile.parameters["Custom Save Variables"], [], "[CGMZ] Save File", "Your Custom Save Variables parameter had invalid JSON and could not be loaded.");
CGMZ.SaveFile.DisplayInfo = CGMZ_Utils.parseJSON(CGMZ.SaveFile.parameters["Display Info"], [], "[CGMZ] Save File", "Your Display Info parameter's JSON was invalid and could not be loaded.");
CGMZ.SaveFile.CustomHeaders = CGMZ_Utils.parseJSON(CGMZ.SaveFile.parameters["Custom Headers"], [], "[CGMZ] Save File", "Your Custom Headers parameter's JSON was invalid and could not be loaded.");
CGMZ.SaveFile.CustomTexts = CGMZ_Utils.parseJSON(CGMZ.SaveFile.parameters["Custom Texts"], [], "[CGMZ] Save File", "Your Custom Texts parameter's JSON was invalid and could not be loaded.");
//=============================================================================
// CGMZ Temp
//-----------------------------------------------------------------------------
// Create plugin data. Not saved.
//=============================================================================
//-----------------------------------------------------------------------------
// Create (unsaved) plugin data
//-----------------------------------------------------------------------------
const alias_CGMZ_SaveFile_CGMZTemp_createPluginData = CGMZ_Temp.prototype.createPluginData;
CGMZ_Temp.prototype.createPluginData = function() {
	alias_CGMZ_SaveFile_CGMZTemp_createPluginData.call(this);
	this._saveFileTempImageURL = "";
	this._saveFileTempMapName = "Unknown";
	this.saveFileAutosaveFlag = false;
};
//-----------------------------------------------------------------------------
// Get the save file's filename
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.getSaveFileFilename = function() {
	if(this.saveFileAutosaveFlag) {
		const info = DataManager._globalInfo[0];
		if(info && info.cgmz_savefileName) return info.cgmz_savefileName;
		return "";
	}
	const id = $gameSystem.savefileId();
	if(id) {
		const info = DataManager._globalInfo[id];
		if(info && info.cgmz_savefileName) return info.cgmz_savefileName;
	}
	return "";
};
//-----------------------------------------------------------------------------
// Register Save File Plugin Commands
//-----------------------------------------------------------------------------
const alias_CGMZSaveFile_CGMZTemp_registerPluginCommands = CGMZ_Temp.prototype.registerPluginCommands;
CGMZ_Temp.prototype.registerPluginCommands = function() {
	alias_CGMZSaveFile_CGMZTemp_registerPluginCommands.call(this);
	PluginManager.registerCommand("CGMZ_SaveFile", "Save File", this.pluginCommandSaveFileSaveFile);
};
//-----------------------------------------------------------------------------
// Plugin Command - Save File
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandSaveFileSaveFile = function(args) {
	const slot = Number(args.Slot);
	let id = (slot >= 0) ? slot : $gameSystem.savefileId();
	if(!id && slot < 0) id = DataManager.emptySavefileId();
	if(id < 0) id = DataManager.earliestSavefileId();
	if(id >= 0) {
		$gameSystem.setSavefileId(id);
		$gameSystem.onBeforeSave();
		DataManager.saveGame(id).catch(() => console.log('[CGMZ] Save File - Save File Plugin Command - save failed'));
	}
};
//=============================================================================
// Game_Map
//-----------------------------------------------------------------------------
// Add function for getting map save file image
//=============================================================================
//-----------------------------------------------------------------------------
// Get CGMZ Save File Image url
//-----------------------------------------------------------------------------
Game_Map.prototype.CGMZ_SaveFile_getImage = function() {
	let url = "";
	if($dataMap && $dataMap.meta && $dataMap.meta.cgmzsaveimg) url = $dataMap.meta.cgmzsaveimg;
	if(url === "" && $cgmzTemp._saveFileTempImageURL) url = $cgmzTemp._saveFileTempImageURL;
	return url;
};
//-----------------------------------------------------------------------------
// Temporarily store cgmz meta for save file on map load. Needed because
// map data is not yet loaded when autosave occurs after battle, so dataMap
// meta tags cannot be read after battle during autosave
//-----------------------------------------------------------------------------
const alias_CGMZ_SaveFile_GameMap_setup = Game_Map.prototype.setup;
Game_Map.prototype.setup = function(mapId) {
	alias_CGMZ_SaveFile_GameMap_setup.call(this, mapId);
	$cgmzTemp._saveFileTempImageURL = this.CGMZ_SaveFile_getImage();
	$cgmzTemp._saveFileTempMapName = this.CGMZ_getMapName();
};
//-----------------------------------------------------------------------------
// Check if data map is null, if so return temp saved map name
//-----------------------------------------------------------------------------
const alias_CGMZ_SaveFile_GameMap_CGMZ_getMapName = Game_Map.prototype.CGMZ_getMapName;
Game_Map.prototype.CGMZ_getMapName = function() {
	if(!$dataMap) return $cgmzTemp._saveFileTempMapName;
	return alias_CGMZ_SaveFile_GameMap_CGMZ_getMapName.call(this);
};
//=============================================================================
// Game_Party
//-----------------------------------------------------------------------------
// Add more to each actor's save info
//=============================================================================
//-----------------------------------------------------------------------------
// Get CGMZ Save File actor info
//-----------------------------------------------------------------------------
Game_Party.prototype.CGMZ_SaveFile_actorInfoForSavefile = function() {
	return this.battleMembers().map(actor => [
		actor.name(),
		actor._level
	]);
};
//=============================================================================
// Scene_Base
//-----------------------------------------------------------------------------
// Add tracking for autosaves
//=============================================================================
//-----------------------------------------------------------------------------
// Set CGMZ flag before autosave
//-----------------------------------------------------------------------------
const alias_CGMZ_SaveFile_SceneBase_executeAutosave = Scene_Base.prototype.executeAutosave;
Scene_Base.prototype.executeAutosave = function() {
	$cgmzTemp.saveFileAutosaveFlag = true;
    alias_CGMZ_SaveFile_SceneBase_executeAutosave.call(this);
};
//-----------------------------------------------------------------------------
// Clear CGMZ flag after autosave
//-----------------------------------------------------------------------------
const alias_CGMZ_SaveFile_SceneBase_onAutosaveSuccess = Scene_Base.prototype.onAutosaveSuccess;
Scene_Base.prototype.onAutosaveSuccess = function() {
	alias_CGMZ_SaveFile_SceneBase_onAutosaveSuccess.call(this);
    $cgmzTemp.saveFileAutosaveFlag = false;
};
//-----------------------------------------------------------------------------
// Clear CGMZ flag after autosave
//-----------------------------------------------------------------------------
const alias_CGMZ_SaveFile_SceneBase_onAutosaveFailure = Scene_Base.prototype.onAutosaveFailure;
Scene_Base.prototype.onAutosaveFailure = function() {
	alias_CGMZ_SaveFile_SceneBase_onAutosaveFailure.call(this);
    $cgmzTemp.saveFileAutosaveFlag = false;
};
//=============================================================================
// DataManager
//-----------------------------------------------------------------------------
// Change max save file count and save file info
//=============================================================================
//-----------------------------------------------------------------------------
// Change max save files
//-----------------------------------------------------------------------------
DataManager.maxSavefiles = function() {
	return CGMZ.SaveFile.MaxSaveFiles;
};
//-----------------------------------------------------------------------------
// Add additional save file info
//-----------------------------------------------------------------------------
const alias_CGMZ_SaveFile_DataManager_makeSavefileInfo = DataManager.makeSavefileInfo;
DataManager.makeSavefileInfo = function() {
	const info = alias_CGMZ_SaveFile_DataManager_makeSavefileInfo.call(this);
	info.cgmz_savefileName = $cgmzTemp.getSaveFileFilename();
	info.cgmz_mapName = $gameMap.CGMZ_getMapName();
	info.cgmz_gold = $gameParty.gold();
	info.cgmz_image = $gameMap.CGMZ_SaveFile_getImage();
	info.cgmz_actor = $gameParty.CGMZ_SaveFile_actorInfoForSavefile();
	if(Imported.CGMZ_Achievements) {
		info.cgmz_achievements = $cgmz.countEarnedAchievements();
		info.cgmz_achievementPts = $cgmz.countEarnedAchievementPoints();
	}
	info.cgmz_custom = {};
	for(const customInfo of CGMZ.SaveFile.CustomSaveInfo) {
		const data = CGMZ_Utils.parseJSON(customInfo, null, "CGMZ Save File", "A custom save info property could not be read due to invalid JSON");
		if(!data) continue;
		const variable = Number(data.Variable);
		info.cgmz_custom[variable] = $gameVariables.value(variable);
	}
	return info;
};
//=============================================================================
// Window_SavefileList
//-----------------------------------------------------------------------------
// Do not include autosave if save mode and option disabled
//=============================================================================
//-----------------------------------------------------------------------------
// Set window transparency
//-----------------------------------------------------------------------------
const alias_CGMZ_SaveFile_WindowSavefileList_initialize = Window_SavefileList.prototype.initialize;
Window_SavefileList.prototype.initialize = function(rect) {
	alias_CGMZ_SaveFile_WindowSavefileList_initialize.call(this, rect);
	if(Imported.CGMZ_WindowBackgrounds && CGMZ.SaveFile.ListWindowBackground) this.CGMZ_setWindowBackground(CGMZ.SaveFile.ListWindowBackground);
	if(Imported.CGMZ_WindowSettings && CGMZ.SaveFile.ListWindowSettings) this.CGMZ_setWindowSettings(CGMZ.SaveFile.ListWindowSettings);
};
//-----------------------------------------------------------------------------
// Do not include autosave if in save mode (optional)
//-----------------------------------------------------------------------------
const alias_CGMZ_SaveFile_WindowSavefileList_setMode = Window_SavefileList.prototype.setMode;
Window_SavefileList.prototype.setMode = function(mode, autosave) {
	if(mode === "save" && !CGMZ.SaveFile.ShowAutosaveInSaveMode) autosave = false;
	if(mode === "load" && !CGMZ.SaveFile.ShowAutosaveInLoadMode) autosave = false;
	alias_CGMZ_SaveFile_WindowSavefileList_setMode.call(this, mode, autosave);
};
//-----------------------------------------------------------------------------
// Draw the item
//-----------------------------------------------------------------------------
Window_SavefileList.prototype.drawItem = function(index) {
	const savefileId = this.indexToSavefileId(index);
	const rect = this.itemRectWithPadding(index);
	this.resetTextColor();
	this.changePaintOpacity(this.isEnabled(savefileId));
	if(!!DataManager.savefileInfo(savefileId) && CGMZ.SaveFile.SaveIconUsed > 0) {
		this.drawIcon(CGMZ.SaveFile.SaveIconUsed, rect.x, rect.y + 4);
		rect.x += ImageManager.iconWidth + 4;
	} else if(!DataManager.savefileInfo(savefileId) && CGMZ.SaveFile.SaveIconUnused > 0) {
		this.drawIcon(CGMZ.SaveFile.SaveIconUnused, rect.x, rect.y + 4);
		rect.x += ImageManager.iconWidth + 4;
	}
	this.drawTitle(savefileId, rect.x, rect.y + 4);
};
//-----------------------------------------------------------------------------
// Check for custom title first. Otherwise, draw the default title
//-----------------------------------------------------------------------------
const alias_CGMZ_SaveFile_WindowSavefileList_drawTitle = Window_SavefileList.prototype.drawTitle;
Window_SavefileList.prototype.drawTitle = function(savefileId, x, y) {
	const info = DataManager.savefileInfo(savefileId);
	if(info && info.cgmz_savefileName) {
		this.drawText(info.cgmz_savefileName, x, y, this.contents.width - x - this.padding);
	} else {
		alias_CGMZ_SaveFile_WindowSavefileList_drawTitle.apply(this, arguments);
	}
};
//-----------------------------------------------------------------------------
// Change item height back to default
//-----------------------------------------------------------------------------
Window_SavefileList.prototype.itemHeight = function() {
	return Window_Selectable.prototype.itemHeight.call(this);
};
//-----------------------------------------------------------------------------
// Set Help Window
//-----------------------------------------------------------------------------
Window_SavefileList.prototype.CGMZ_SaveFile_setDisplayWindow = function(displayWindow) {
	this._CGMZ_SaveFile_displayWindow = displayWindow;
	this.callUpdateHelp();
};
//-----------------------------------------------------------------------------
// Set Help Window
//-----------------------------------------------------------------------------
Window_SavefileList.prototype.CGMZ_SaveFile_setDeleteRenameWindow = function(deleteRenameWindow) {
	this._CGMZ_SaveFile_deleteRenameWindow = deleteRenameWindow;
	this.callUpdateHelp();
};
//-----------------------------------------------------------------------------
// Update display window if exists
//-----------------------------------------------------------------------------
Window_SavefileList.prototype.callUpdateHelp = function() {
	if(this._CGMZ_SaveFile_displayWindow) {
		const savefileId = this.indexToSavefileId(this.index());
		const info = DataManager.savefileInfo(savefileId);
		this._CGMZ_SaveFile_displayWindow.setInfo(info);
	}
	if(this._CGMZ_SaveFile_deleteRenameWindow) {
		const savefileId = this.indexToSavefileId(this.index());
		this._CGMZ_SaveFile_deleteRenameWindow.setSavefileIndex(savefileId);
	}
};
//=============================================================================
// Scene_File
//-----------------------------------------------------------------------------
// Shrink list window and display new display window
//=============================================================================
const alias_CGMZ_SaveFile_SceneFile_initialize = Scene_File.prototype.initialize;
Scene_File.prototype.initialize = function() {
	alias_CGMZ_SaveFile_SceneFile_initialize.call(this);
	this._cgmz_needsRefresh = false;
};
//-----------------------------------------------------------------------------
// Also create save file display window
//-----------------------------------------------------------------------------
const alias_CGMZ_SaveFile_SceneFile_create = Scene_File.prototype.create;
Scene_File.prototype.create = function() {
	alias_CGMZ_SaveFile_SceneFile_create.call(this);
	this.CGMZ_createDisplayWindow();
	this.CGMZ_createConfirmWindow();
	this.CGMZ_createDeleteConfirmWindow();
	this.CGMZ_createDeleteRenameOptionWindow();
	this.CGMZ_createFilenameEditWindow();
	this.CGMZ_createFilenameInputWindow();
};
//-----------------------------------------------------------------------------
// Change to CGMZ Help Window
//-----------------------------------------------------------------------------
Scene_File.prototype.createHelpWindow = function() {
    const rect = this.helpWindowRect();
    this._helpWindow = new CGMZ_SaveFile_Window_Help(rect);
    this.addWindow(this._helpWindow);
};
//-----------------------------------------------------------------------------
// Change help window y if no touch ui
//-----------------------------------------------------------------------------
const alias_CGMZ_SaveFile_SceneFile_helpWindowRect = Scene_File.prototype.helpWindowRect;
Scene_File.prototype.helpWindowRect = function() {
	const rect = alias_CGMZ_SaveFile_SceneFile_helpWindowRect.apply(this, arguments);
	if(CGMZ.SaveFile.DisableTouchUISpace && !ConfigManager.touchUI) rect.y = 0;
	if(this.CGMZ_needsDeleteRenameOptionWindow()) rect.height = this.calcWindowHeight(1, true);
	return rect;
};
//-----------------------------------------------------------------------------
// Change width of save file select window
//-----------------------------------------------------------------------------
const alias_CGMZ_SaveFile_SceneFile_listWindowRect = Scene_File.prototype.listWindowRect;
Scene_File.prototype.listWindowRect = function() {
	const rect = alias_CGMZ_SaveFile_SceneFile_listWindowRect.call(this);
	rect.width = Graphics.boxWidth * (CGMZ.SaveFile.ListWindowWidth / 100.0);
	rect.y = this._helpWindow.height + this._helpWindow.y;
	rect.height = Graphics.boxHeight - rect.y;
	return rect;
};
//-----------------------------------------------------------------------------
// VS compatibility. Seems their core doesn't call above default function ¯\_(ツ)_/¯
//-----------------------------------------------------------------------------
const alias_CGMZ_SaveFile_SceneFile_createListWindow = Scene_File.prototype.createListWindow;
Scene_File.prototype.createListWindow = function() {
	alias_CGMZ_SaveFile_SceneFile_createListWindow.call(this);
	this._listWindow.width = Graphics.boxWidth * (CGMZ.SaveFile.ListWindowWidth / 100.0);
};
//-----------------------------------------------------------------------------
// Create display window
//-----------------------------------------------------------------------------
Scene_File.prototype.CGMZ_createDisplayWindow = function() {
	const rect = this.CGMZ_displayWindowRect();
	this._cgmz_displayWindow = new CGMZ_Window_SaveFileDisplay(rect);
	this.addWindow(this._cgmz_displayWindow);
	this._listWindow.CGMZ_SaveFile_setDisplayWindow(this._cgmz_displayWindow);
};
//-----------------------------------------------------------------------------
// Display window rect
//-----------------------------------------------------------------------------
Scene_File.prototype.CGMZ_displayWindowRect = function() {
	const x = this._listWindow.width;
	const y = this._listWindow.y;
	const width = Graphics.boxWidth - this._listWindow.width;
	const height = this._listWindow.height;
	return new Rectangle(x, y, width, height);
};
//-----------------------------------------------------------------------------
// Create confirm window
//-----------------------------------------------------------------------------
Scene_File.prototype.CGMZ_createConfirmWindow = function() {
	const rect = this.CGMZ_confirmWindowRect();
	this._cgmz_confirmWindow = new CGMZ_Window_SaveFileConfirm(rect);
	this._cgmz_confirmWindow.setHandler("confirm", this.CGMZ_onSaveConfirmConfirm.bind(this));
	this._cgmz_confirmWindow.setHandler("cancel", this.CGMZ_onSaveConfirmCancel.bind(this));
	this._cgmz_confirmWindow.deactivate();
	this._cgmz_confirmWindow.hide();
	this.addWindow(this._cgmz_confirmWindow);
};
//-----------------------------------------------------------------------------
// Confirm window rect
//-----------------------------------------------------------------------------
Scene_File.prototype.CGMZ_confirmWindowRect = function() {
	const width = Graphics.boxWidth * (50 / 100.0);
	const height = this.calcWindowHeight(2, true);
	const x = Graphics.boxWidth / 2 - width / 2;
	const y = Graphics.boxHeight / 2 - height / 2;
	return new Rectangle(x, y, width, height);
};
//-----------------------------------------------------------------------------
// Create delete confirm window
//-----------------------------------------------------------------------------
Scene_File.prototype.CGMZ_createDeleteConfirmWindow = function() {
	const rect = this.CGMZ_deleteConfirmWindowRect();
	this._cgmz_deleteConfirmWindow = new CGMZ_Window_SaveFileConfirmDelete(rect);
	this._cgmz_deleteConfirmWindow.setHandler("confirm", this.CGMZ_onDeleteConfirm.bind(this));
	this._cgmz_deleteConfirmWindow.setHandler("cancel", this.CGMZ_onDeleteCancel.bind(this));
	this._cgmz_deleteConfirmWindow.deactivate();
	this._cgmz_deleteConfirmWindow.hide();
	this.addWindow(this._cgmz_deleteConfirmWindow);
};
//-----------------------------------------------------------------------------
// Delete confirm window rect
//-----------------------------------------------------------------------------
Scene_File.prototype.CGMZ_deleteConfirmWindowRect = function() {
	const width = this._cgmz_confirmWindow.width;
	const height = this._cgmz_confirmWindow.height;
	const x = this._cgmz_confirmWindow.x;
	const y = this._cgmz_confirmWindow.y;
	return new Rectangle(x, y, width, height);
};
//-----------------------------------------------------------------------------
// Create delete/rename option window
//-----------------------------------------------------------------------------
Scene_File.prototype.CGMZ_createDeleteRenameOptionWindow = function() {
	const rect = this.CGMZ_deleteRenameWindowRect();
	this._cgmz_deleteRenameWindow = new CGMZ_Window_SaveFileDeleteRename(rect, this.mode());
	this._cgmz_deleteRenameWindow.setHandler("main", this.CGMZ_postDROnSaveFileOk.bind(this));
	this._cgmz_deleteRenameWindow.setHandler("delete", this.CGMZ_onDeleteOption.bind(this));
	this._cgmz_deleteRenameWindow.setHandler("rename", this.CGMZ_onRenameOption.bind(this));
	this._cgmz_deleteRenameWindow.setHandler("cancel", this.CGMZ_onDRCancel.bind(this));
	this._listWindow.CGMZ_SaveFile_setDeleteRenameWindow(this._cgmz_deleteRenameWindow);
	this._cgmz_deleteRenameWindow.deactivate();
	this._cgmz_deleteRenameWindow.hide();
	this.addWindow(this._cgmz_deleteRenameWindow);
};
//-----------------------------------------------------------------------------
// Copy help window rect for the delete/rename window rect
// Need to re-set the height to prevent scrolling because VS changes it back
// for the actual help window
//-----------------------------------------------------------------------------
Scene_File.prototype.CGMZ_deleteRenameWindowRect = function() {
	const rect = this.helpWindowRect();
	rect.height = this.calcWindowHeight(1, true);
	return rect;
};
//-----------------------------------------------------------------------------
// Create name edit window
//-----------------------------------------------------------------------------
Scene_File.prototype.CGMZ_createFilenameEditWindow = function() {
	const rect = this.CGMZ_filenameEditWindowRect();
	this._cgmz_filenameEditWindow = new CGMZ_SaveFile_Window_NameEdit(rect);
	this._cgmz_filenameEditWindow.setup(CGMZ.SaveFile.MaxFilenameLength);
	this._cgmz_filenameEditWindow.hide();
	this.addWindow(this._cgmz_filenameEditWindow);
};
//-----------------------------------------------------------------------------
// Get the name edit window rect
//-----------------------------------------------------------------------------
Scene_File.prototype.CGMZ_filenameEditWindowRect = function() {
	const inputWindowHeight = this.calcWindowHeight(9, true);
	const ww = 600;
	const wh = this.calcWindowHeight(1, true);
	const wx = (Graphics.boxWidth - ww) / 2;
	const wy = (Graphics.boxHeight - (wh + inputWindowHeight + 4)) / 2;
	return new Rectangle(wx, wy, ww, wh);
};
//-----------------------------------------------------------------------------
// Create name input window
//-----------------------------------------------------------------------------
Scene_File.prototype.CGMZ_createFilenameInputWindow = function() {
	const rect = this.CGMZ_filenameInputWindowRect();
	this._cgmz_filenameInputWindow = new CGMZ_SaveFile_Window_NameInput(rect);
	this._cgmz_filenameInputWindow.setEditWindow(this._cgmz_filenameEditWindow);
	this._cgmz_filenameInputWindow.setHandler("ok", this.CGMZ_onFilenameInputOk.bind(this));
	this._cgmz_filenameInputWindow.hide();
	this._cgmz_filenameInputWindow.deactivate();
	this.addWindow(this._cgmz_filenameInputWindow);
};
//-----------------------------------------------------------------------------
// Get the name input window rect
//-----------------------------------------------------------------------------
Scene_File.prototype.CGMZ_filenameInputWindowRect = function() {
	const wx = this._cgmz_filenameEditWindow.x;
	const wy = this._cgmz_filenameEditWindow.y + this._cgmz_filenameEditWindow.height + 8;
	const ww = this._cgmz_filenameEditWindow.width;
	const wh = this.calcWindowHeight(9, true);
	return new Rectangle(wx, wy, ww, wh);
};
//-----------------------------------------------------------------------------
// Check if this scene needs the delete/rename option window
//-----------------------------------------------------------------------------
Scene_File.prototype.CGMZ_needsDeleteRenameOptionWindow = function() {
	return CGMZ.SaveFile.ShowDeleteOption || CGMZ.SaveFile.ShowRenameOption;
};
//-----------------------------------------------------------------------------
// Handling for confirm window confirm
//-----------------------------------------------------------------------------
Scene_File.prototype.CGMZ_onSaveConfirmConfirm = function() {
	this.CGMZ_postConfirmOnSaveFileOk();
};
//-----------------------------------------------------------------------------
// Implemented by children
//-----------------------------------------------------------------------------
Scene_File.prototype.CGMZ_postConfirmOnSaveFileOk = function() {
	//
};
//-----------------------------------------------------------------------------
// Handling for confirm window cancel
//-----------------------------------------------------------------------------
Scene_File.prototype.CGMZ_onSaveConfirmCancel = function() {
	this._cgmz_confirmWindow.deactivate();
	this._cgmz_confirmWindow.hide();
	this._listWindow.activate();
};
//-----------------------------------------------------------------------------
// Also deactivate and hide the confirm window
//-----------------------------------------------------------------------------
const alias_CGMZ_SaveFile_SceneFile_activateListWindow = Scene_File.prototype.activateListWindow;
Scene_File.prototype.activateListWindow = function() {
	if(this._cgmz_confirmWindow) {
		this._cgmz_confirmWindow.deactivate();
		this._cgmz_confirmWindow.hide();
	}
	alias_CGMZ_SaveFile_SceneFile_activateListWindow.call(this);
};
//-----------------------------------------------------------------------------
// Handling for delete/rename window cancel
//-----------------------------------------------------------------------------
Scene_File.prototype.CGMZ_onDRCancel = function() {
	this._cgmz_deleteRenameWindow.deactivate();
	this._cgmz_deleteRenameWindow.hide();
	this._helpWindow.show();
	this._listWindow.activate();
};
//-----------------------------------------------------------------------------
// Implemented by children
//-----------------------------------------------------------------------------
Scene_File.prototype.CGMZ_postDROnSaveFileOk = function() {
	//
};
//-----------------------------------------------------------------------------
// Handle when delete option is selected
//-----------------------------------------------------------------------------
Scene_File.prototype.CGMZ_onDeleteOption = function() {
	if(CGMZ.SaveFile.ShowDeleteConfirmWindow) {
		this._cgmz_deleteRenameWindow.deactivate();
		this._cgmz_deleteConfirmWindow.activate();
		this._cgmz_deleteConfirmWindow.show();
	} else {
		this.CGMZ_deleteFile();
	}
};
//-----------------------------------------------------------------------------
// Handle when delete is canceled
//-----------------------------------------------------------------------------
Scene_File.prototype.CGMZ_onDeleteCancel = function() {
	this._cgmz_deleteConfirmWindow.deactivate();
	this._cgmz_deleteConfirmWindow.hide();
	this._cgmz_deleteRenameWindow.activate();
};
//-----------------------------------------------------------------------------
// Handle when delete is confirmed
//-----------------------------------------------------------------------------
Scene_File.prototype.CGMZ_onDeleteConfirm = function() {
	this.CGMZ_deleteFile();
};
//-----------------------------------------------------------------------------
// Handle deleting the save file
//-----------------------------------------------------------------------------
Scene_File.prototype.CGMZ_deleteFile = function() {
	if(DataManager.savefileExists(this.savefileId())) {
		const saveName = DataManager.makeSavename(this.savefileId());
		StorageManager.remove(saveName);
		DataManager.removeInvalidGlobalInfo();
		const savefileId = this.savefileId();
		const info = DataManager.savefileInfo(savefileId);
		this._listWindow.refresh();
		this._cgmz_displayWindow.setInfo(info);
	}
	this._cgmz_deleteRenameWindow.deactivate();
	this._cgmz_deleteRenameWindow.hide();
	this._cgmz_deleteConfirmWindow.deactivate();
	this._cgmz_deleteConfirmWindow.hide();
	this._helpWindow.show();
	this._listWindow.activate();
};
//-----------------------------------------------------------------------------
// Handle rename option
//-----------------------------------------------------------------------------
Scene_File.prototype.CGMZ_onRenameOption = function() {
	this._listWindow.hide();
	this._cgmz_displayWindow.hide();
	this._cgmz_deleteRenameWindow.hide();
	this._cgmz_deleteRenameWindow.deactivate();
	this._cgmz_filenameEditWindow.setSaveFileId(this.savefileId());
	this._cgmz_filenameEditWindow.refresh();
	this._cgmz_filenameEditWindow.show();
	this._cgmz_filenameInputWindow.show();
	this._cgmz_filenameInputWindow.activate();
};
//-----------------------------------------------------------------------------
// Handle filename input accept
//-----------------------------------------------------------------------------
Scene_File.prototype.CGMZ_onFilenameInputOk = function() {
	const savefileId = this.savefileId();
	const info = DataManager.savefileInfo(savefileId);
	const name = this._cgmz_filenameEditWindow.name();
	if(name && info) {
		info.cgmz_savefileName = name;
		DataManager.saveGlobalInfo();
	}
	this._cgmz_filenameEditWindow.hide();
	this._cgmz_filenameInputWindow.hide();
	this._cgmz_filenameInputWindow.deactivate();
	this._listWindow.refresh();
	this._listWindow.show();
	this._cgmz_displayWindow.show();
	this._cgmz_deleteRenameWindow.show();
	this._cgmz_deleteRenameWindow.activate();
};
//=============================================================================
// Scene_Save
//-----------------------------------------------------------------------------
// Different handling to show confirm window if configured
//=============================================================================
//-----------------------------------------------------------------------------
// Handle save file ok in case of confirm window
//-----------------------------------------------------------------------------
const alias_CGMZ_SaveFile_SceneSave_onSavefileOk = Scene_Save.prototype.onSavefileOk;
Scene_Save.prototype.onSavefileOk = function() {
	const savefileId = this.savefileId();
	if(this.CGMZ_needsDeleteRenameOptionWindow()) {
		if(this.isSavefileEnabled(savefileId)) {
			SoundManager.playOk();
			this._helpWindow.hide();
			this._cgmz_deleteRenameWindow.activate();
			this._cgmz_deleteRenameWindow.show();
		} else {
			SoundManager.playBuzzer();
			this._listWindow.activate();
		}
	} else if(CGMZ.SaveFile.ShowConfirmWindow) {
		if(this.isSavefileEnabled(savefileId)) {
			SoundManager.playOk();
			this._listWindow.deactivate();
			this._cgmz_confirmWindow.activate();
			this._cgmz_confirmWindow.show();
		} else {
			SoundManager.playBuzzer();
			this._listWindow.activate();
		}
	} else {
		alias_CGMZ_SaveFile_SceneSave_onSavefileOk.call(this);
	}
};
//-----------------------------------------------------------------------------
// Same save file processing for after save confirmation
//-----------------------------------------------------------------------------
Scene_Save.prototype.CGMZ_postConfirmOnSaveFileOk = function() {
	alias_CGMZ_SaveFile_SceneSave_onSavefileOk.call(this);
};
//-----------------------------------------------------------------------------
// Call original load function
//-----------------------------------------------------------------------------
Scene_Save.prototype.CGMZ_postDROnSaveFileOk = function() {
	if(CGMZ.SaveFile.ShowConfirmWindow) {
		const savefileId = this.savefileId();
		if(this.isSavefileEnabled(savefileId)) {
			SoundManager.playOk();
			this._listWindow.deactivate();
			this._cgmz_confirmWindow.activate();
			this._cgmz_confirmWindow.show();
		} else {
			SoundManager.playBuzzer();
			this._listWindow.activate();
		}
	} else {
		alias_CGMZ_SaveFile_SceneSave_onSavefileOk.call(this);
	}
};
//-----------------------------------------------------------------------------
// Get the scene's custom scene background
// No need to check if Scene Backgrounds is installed because this custom func
// is only called by that plugin
//-----------------------------------------------------------------------------
Scene_Save.prototype.CGMZ_getCustomSceneBackground = function() {
	return $cgmzTemp.sceneBackgroundPresets[CGMZ.SaveFile.SaveBackground];
};
//=============================================================================
// Scene_Load
//-----------------------------------------------------------------------------
// Different handling to show delete/rename option window if needed
//=============================================================================
//-----------------------------------------------------------------------------
// Handle save file ok in case of delete/rename option window
//-----------------------------------------------------------------------------
const alias_CGMZ_SaveFile_SceneLoad_onSavefileOk = Scene_Load.prototype.onSavefileOk;
Scene_Load.prototype.onSavefileOk = function() {
	if(this.CGMZ_needsDeleteRenameOptionWindow()) {
		const savefileId = this.savefileId();
		if(this.isSavefileEnabled(savefileId)) {
			SoundManager.playOk();
			this._helpWindow.hide();
			this._cgmz_deleteRenameWindow.activate();
			this._cgmz_deleteRenameWindow.show();
		} else {
			SoundManager.playBuzzer();
			this._listWindow.activate();
		}
	} else {
		alias_CGMZ_SaveFile_SceneLoad_onSavefileOk.call(this);
	}
};
//-----------------------------------------------------------------------------
// Call original load function
//-----------------------------------------------------------------------------
Scene_Load.prototype.CGMZ_postDROnSaveFileOk = function() {
	alias_CGMZ_SaveFile_SceneLoad_onSavefileOk.call(this);
};
//-----------------------------------------------------------------------------
// Get the scene's custom scene background
// No need to check if Scene Backgrounds is installed because this custom func
// is only called by that plugin
//-----------------------------------------------------------------------------
Scene_Load.prototype.CGMZ_getCustomSceneBackground = function() {
	return $cgmzTemp.sceneBackgroundPresets[CGMZ.SaveFile.LoadBackground];
};
//=============================================================================
// CGMZ_Window_SaveFileDisplay
//-----------------------------------------------------------------------------
// Display save file info
//=============================================================================
function CGMZ_Window_SaveFileDisplay(rect) {
	this.initialize.apply(this, arguments);
}
CGMZ_Window_SaveFileDisplay.prototype = Object.create(CGMZ_Window_Scrollable.prototype);
CGMZ_Window_SaveFileDisplay.prototype.constructor = CGMZ_Window_SaveFileDisplay;
//-----------------------------------------------------------------------------
// Initialize
//-----------------------------------------------------------------------------
CGMZ_Window_SaveFileDisplay.prototype.initialize = function(rect) {
	const heightMultiplier = 10;
	CGMZ_Window_Scrollable.prototype.initialize.call(this, rect, heightMultiplier, CGMZ.SaveFile.ScrollWait, CGMZ.SaveFile.ScrollSpeed, CGMZ.SaveFile.AutoScroll, CGMZ.SaveFile.ScrollDeceleration);
	if(Imported.CGMZ_WindowBackgrounds && CGMZ.SaveFile.DisplayWindowBackground) this.CGMZ_setWindowBackground(CGMZ.SaveFile.DisplayWindowBackground);
	if(Imported.CGMZ_WindowSettings && CGMZ.SaveFile.DisplayWindowSettings) this.CGMZ_setWindowSettings(CGMZ.SaveFile.DisplayWindowSettings);
	this._info = null;
	this._characterXOffset = 0;
	this._characterHeight = -1;
	this._isNegativeCharacter = false;
	this._charStepAnimTimer = 0;
	this.refresh();
};
//-----------------------------------------------------------------------------
// Refresh
//-----------------------------------------------------------------------------
CGMZ_Window_SaveFileDisplay.prototype.refresh = function() {
	this.contents.clear();
	this.contentsBack.clear();
	if(this._info) {
		this.loadSaveImage();
	} else {
		this.drawEmptyInfo();
	}
};
//-----------------------------------------------------------------------------
// Set save file info object (might also be null if save file not exists)
//-----------------------------------------------------------------------------
CGMZ_Window_SaveFileDisplay.prototype.setInfo = function(info) {
	this._info = info;
	this.refresh();
};
//-----------------------------------------------------------------------------
// Draw empty save file information
//-----------------------------------------------------------------------------
CGMZ_Window_SaveFileDisplay.prototype.drawEmptyInfo = function() {
	this.setupWindowForNewEntry();
	this._neededHeight = this.CGMZ_drawText(CGMZ.SaveFile.EmptyText, 0, 0, 0, this.contents.width, 'center');
	this._neededHeight += this.padding * 2;
	this.checkForScroll();
};
//-----------------------------------------------------------------------------
// Refresh
//-----------------------------------------------------------------------------
CGMZ_Window_SaveFileDisplay.prototype.loadSaveImage = function() {
	let url = this._info.cgmz_image || CGMZ.SaveFile.DefaultImage;
	if(!url) {
		this.drawSaveFileInfo();
		return;
	}
	if(!url.includes("/")) url = "pictures/" + url; // backwards compatibility with pre-1.1.0 where pictures folder was mandatory; if no folder detected chances are it's the pictures folder
	const imgData = CGMZ_Utils.getImageData(url, "img");
	const bitmap = ImageManager.loadBitmap(imgData.folder, imgData.filename);
	bitmap.addLoadListener(this.drawSaveFileInfo.bind(this, bitmap));
};
//-----------------------------------------------------------------------------
// Draw save file information
//-----------------------------------------------------------------------------
CGMZ_Window_SaveFileDisplay.prototype.drawSaveFileInfo = function(bitmap) {
	this.setupWindowForNewEntry();
	let customInfoIndex = 0;
	let customHeaderIndex = 0;
	let customTextIndex = 0;
	for(const infoType of CGMZ.SaveFile.DisplayInfo) {
		this.resetFontSettings();
		switch(infoType) {
			case "Image":
				this.drawSaveBitmap(bitmap);
				this._neededHeight += CGMZ.SaveFile.ImageHeight || bitmap.height;
				break;
			case "Image And Location":
				this.drawSaveBitmap(bitmap);
				this._neededHeight += (CGMZ.SaveFile.ImageHeight || bitmap.height) - this.lineHeight();
				this.drawLocationInfo(4, true);
				break;
			case "Date": this.drawTimestamp(); break;
			case "Location": this.drawLocationInfo(0, false); break;
			case "Playtime": this.drawPlaytime(); break;
			case "Gold": this.drawGoldInfo(); break;
			case "Achievements": this.drawAchievements(); break;
			case "Achievement Points": this.drawAchievementPoints(); break;
			case "Custom Info": this.drawCustomInfo(customInfoIndex++); break;
			case "Custom Header": this.drawCustomHeader(customHeaderIndex++); break;
			case "Custom Text": this.drawCustomText(customTextIndex++); break;
			case "Faces":
				this.drawFaces();
				this._neededHeight += ImageManager.faceHeight;
				break;
			case "Characters":
				this.drawChars();
				this._characterHeight = this._neededHeight;
				this._neededHeight += CGMZ.SaveFile.CharHeight;
				break;
			case "New Game Plus": this.drawNewGamePlus(); break;
			case "Hardcore": this.drawHardcore(); break;
			case "Blank Line": this._neededHeight += this.lineHeight(); break;
			case "Custom Space": this._neededHeight += CGMZ.SaveFile.CustomSpace; break;
		}
	}
	this._neededHeight += this.padding * 2;
	this.checkForScroll();
};
//-----------------------------------------------------------------------------
// Draw save bitmap
//-----------------------------------------------------------------------------
CGMZ_Window_SaveFileDisplay.prototype.drawSaveBitmap = function(bitmap) {
	const sw = bitmap.width;
	const sh = bitmap.height;
	const sx = sy = 0;
	const dw = (CGMZ.SaveFile.StretchImage) ? this.contents.width : bitmap.width;
	const dh = CGMZ.SaveFile.ImageHeight || bitmap.height;
	const dx = (CGMZ.SaveFile.CenterImage) ? (this.contents.width - dw) / 2 : 0;
	const dy = this._neededHeight;
	this.contentsBack.blt(bitmap, sx, sy, sw, sh, dx, dy, dw, dh);
};
//-----------------------------------------------------------------------------
// Draw location info
//-----------------------------------------------------------------------------
CGMZ_Window_SaveFileDisplay.prototype.drawLocationInfo = function(x, drawFadeSprite) {
	const y = this._neededHeight;
	const string = `\\c[${CGMZ.SaveFile.LabelColor}]${CGMZ.SaveFile.LocationText}\\c[0]${this._info.cgmz_mapName}`;
	this._neededHeight += this.CGMZ_drawTextLine(string, x, this._neededHeight, this.contents.width);
	if(drawFadeSprite) {
		const width = this.textSizeEx(string).width;
		const fadeBitmap = new Bitmap(width + x + 4, this.lineHeight());
		fadeBitmap.paintOpacity = CGMZ.SaveFile.FadeSpriteOpacity;
		fadeBitmap.fillRect(0, 0, fadeBitmap.width, fadeBitmap.height, "#000000");
		const sx = sy = 0;
		const sw = fadeBitmap.width;
		const sh = fadeBitmap.height;
		const dx = 0;
		const dy = y;
		this.contentsBack.blt(fadeBitmap, sx, sy, sw, sh, dx, dy, sw, sh);
	}
};
//-----------------------------------------------------------------------------
// Draw timestamp. Does not increase height because it is drawn on the right.
//-----------------------------------------------------------------------------
CGMZ_Window_SaveFileDisplay.prototype.drawTimestamp = function() {
	const date = new Date(this._info.timestamp);
	this.CGMZ_drawTextLine(date.toLocaleDateString(), 0, this._neededHeight, this.contents.width, 'right');
};
//-----------------------------------------------------------------------------
// Draw playtime
//-----------------------------------------------------------------------------
CGMZ_Window_SaveFileDisplay.prototype.drawPlaytime = function() {
	const string = `\\c[${CGMZ.SaveFile.LabelColor}]${CGMZ.SaveFile.PlaytimeText}\\c[0]${this._info.playtime}`;
	this._neededHeight += this.CGMZ_drawTextLine(string, 0, this._neededHeight, this.contents.width);
};
//-----------------------------------------------------------------------------
// Draw gold info
//-----------------------------------------------------------------------------
CGMZ_Window_SaveFileDisplay.prototype.drawGoldInfo = function() {
	const string = (this._info.cgmz_gold) ? `\\c[${CGMZ.SaveFile.LabelColor}]${CGMZ.SaveFile.GoldText}\\c[0]${this._info.cgmz_gold.toLocaleString()}` : `\\c[${CGMZ.SaveFile.LabelColor}]${CGMZ.SaveFile.GoldText}\\c[0]${this._info.cgmz_gold}`;
	this._neededHeight += this.CGMZ_drawTextLine(string, 0, this._neededHeight, this.contents.width);
};
//-----------------------------------------------------------------------------
// Draw achievements
//-----------------------------------------------------------------------------
CGMZ_Window_SaveFileDisplay.prototype.drawAchievements = function() {
	if(!this._info.cgmz_achievements) return;
	const string = `\\c[${CGMZ.SaveFile.LabelColor}]${CGMZ.SaveFile.AchievementText}\\c[0]${this._info.cgmz_achievements.toLocaleString()}`;
	this._neededHeight += this.CGMZ_drawTextLine(string, 0, this._neededHeight, this.contents.width);
};
//-----------------------------------------------------------------------------
// Draw achievement points
//-----------------------------------------------------------------------------
CGMZ_Window_SaveFileDisplay.prototype.drawAchievementPoints = function() {
	if(!this._info.cgmz_achievementPts) return;
	const string = `\\c[${CGMZ.SaveFile.LabelColor}]${CGMZ.SaveFile.AchievementPointText}\\c[0]${this._info.cgmz_achievementPts.toLocaleString()}`;
	this._neededHeight += this.CGMZ_drawTextLine(string, 0, this._neededHeight, this.contents.width);
};
//-----------------------------------------------------------------------------
// Draw custom save info
//-----------------------------------------------------------------------------
CGMZ_Window_SaveFileDisplay.prototype.drawCustomInfo = function(index) {
	if(!this._info.cgmz_custom) return;
	const customInfo = CGMZ.SaveFile.CustomSaveInfo[index];
	const info = CGMZ_Utils.parseJSON(customInfo, null, "[CGMZ] Save File", "A custom save info line could not be loaded due to invalid JSON.");
	if(!info) return;
	const variable = Number(info.Variable);
	if(typeof(this._info.cgmz_custom[variable]) === 'undefined') return;
	let string = `\\c[${CGMZ.SaveFile.LabelColor}]${info.Description}\\c[0]`;
	if(Number(info.Icon) > 0) string += `\\i[${Number(info.Icon)}]`;
	string += this._info.cgmz_custom[variable].toString() + info["Trailing Text"];
	this._neededHeight += this.CGMZ_drawTextLine(string, 0, this._neededHeight, this.contents.width);
};
//-----------------------------------------------------------------------------
// Draw custom header
//-----------------------------------------------------------------------------
CGMZ_Window_SaveFileDisplay.prototype.drawCustomHeader = function(index) {
	const header = CGMZ.SaveFile.CustomHeaders[index];
	if(!header) return;
	const opts = {drawDividers: CGMZ.SaveFile.DrawHeaderDividers, padding: CGMZ.SaveFile.HeaderPadding}
	this._neededHeight += this.CGMZ_drawHeader(header, this._neededHeight, CGMZ.SaveFile.HeaderColor1, CGMZ.SaveFile.HeaderColor2, opts);
};
//-----------------------------------------------------------------------------
// Draw custom text
//-----------------------------------------------------------------------------
CGMZ_Window_SaveFileDisplay.prototype.drawCustomText = function(index) {
	const txt = CGMZ.SaveFile.CustomTexts[index];
	if(!txt) return;
	this._neededHeight += this.CGMZ_drawText(txt, 0, 0, this._neededHeight, this.contents.width, 'left');
};
//-----------------------------------------------------------------------------
// Draw new game plus (if using [CGMZ] New Game Plus)
//-----------------------------------------------------------------------------
CGMZ_Window_SaveFileDisplay.prototype.drawNewGamePlus = function() {
	if(!Imported.CGMZ_NewGamePlus) return;
	if(!this._info.cgmz_newGamePlus) return;
	this._neededHeight += this.CGMZ_drawTextLine(CGMZ.NewGamePlus.NewGamePlusText, 0, this._neededHeight, this.contents.width, "left");
};
//-----------------------------------------------------------------------------
// Draw hardcore (if using [CGMZ] Hardcore)
//-----------------------------------------------------------------------------
CGMZ_Window_SaveFileDisplay.prototype.drawHardcore = function() {
	if(!Imported.CGMZ_Hardcore || !this._info.cgmz_hardcore) return;
	this._neededHeight += this.CGMZ_drawTextLine(CGMZ.Hardcore.SaveFileText, 0, this._neededHeight, this.contents.width, "left");
};
//-----------------------------------------------------------------------------
// Load face and info
//-----------------------------------------------------------------------------
CGMZ_Window_SaveFileDisplay.prototype.drawFaces = function() {
	let x = 0;
	const width = this.contents.width / 4;
	for(let i = 0; i < this._info.faces.length; i++) {
		const faceName = this._info.faces[i][0];
		const faceIndex = this._info.faces[i][1];
		const bitmap = ImageManager.loadFace(faceName);
		const args = {"name": faceName, "index": faceIndex, "x": x, "y": this._neededHeight, "width": width};
		if(this._info.cgmz_actor && this._info.cgmz_actor[i]) {
			args.actorName = this._info.cgmz_actor[i][0];
			args.actorLevel = this._info.cgmz_actor[i][1];
		}
		bitmap.addLoadListener(this.onFaceLoaded.bind(this, args));
		x += width;
	}
};
//-----------------------------------------------------------------------------
// Draw face & info after loaded
//-----------------------------------------------------------------------------
CGMZ_Window_SaveFileDisplay.prototype.onFaceLoaded = function(args) {
	this.drawFace(args.name, args.index, args.x, args.y, args.width);
	const y = args.y + ImageManager.faceHeight - this.lineHeight();
	this.contents.fontBold = true;
	this.contents.outlineWidth = 6;
	if(args.actorName) this.CGMZ_drawTextLine(args.actorName, args.x, args.y, args.width, "center");
	if(!CGMZ.SaveFile.HideLevels) this.CGMZ_drawTextLine(CGMZ.SaveFile.LevelText + args.actorLevel, args.x, y, args.width, "right");
	this.contents.fontBold = false;
	this.contents.outlineWidth = 3;
};
//-----------------------------------------------------------------------------
// Load characters and info
//-----------------------------------------------------------------------------
CGMZ_Window_SaveFileDisplay.prototype.drawChars = function(y = -1) {
	let x = 0;
	const width = this.contents.width / 4;
	for(let i = 0; i < this._info.characters.length; i++) {
		const charName = this._info.characters[i][0];
		const charIndex = this._info.characters[i][1];
		const bitmap = ImageManager.loadCharacter(charName);
		const args = {"name": charName, "index": charIndex, "x": x, "y": (y >= 0) ? y : this._neededHeight, "width": width};
		if(this._info.cgmz_actor && this._info.cgmz_actor[i]) {
			args.actorName = this._info.cgmz_actor[i][0];
			args.actorLevel = this._info.cgmz_actor[i][1];
		}
		bitmap.addLoadListener(this.onCharLoaded.bind(this, args));
		x += width;
	}
};
//-----------------------------------------------------------------------------
// Draw character & info after loaded
//-----------------------------------------------------------------------------
CGMZ_Window_SaveFileDisplay.prototype.onCharLoaded = function(args) {
	this.CGMZ_drawCharacter(args.name, args.index, args.x + args.width / 2, args.y + CGMZ.SaveFile.CharHeight, this._characterXOffset);
	this.makeFontSmaller();
	this.contents.outlineWidth = 4;
	if(args.actorName) this.CGMZ_drawTextLine(args.actorName, args.x, args.y, args.width, "center");
	if(!CGMZ.SaveFile.HideLevels) this.CGMZ_drawTextLine(CGMZ.SaveFile.LevelText + args.actorLevel, args.x, args.y + CGMZ.SaveFile.CharHeight - 18, args.width, "center");
	this.contents.outlineWidth = 3;
	this.makeFontBigger();
};
//-----------------------------------------------------------------------------
// Update window if needed
//-----------------------------------------------------------------------------
CGMZ_Window_SaveFileDisplay.prototype.update = function() {
	CGMZ_Window_Scrollable.prototype.update.call(this);
	if(CGMZ.SaveFile.ShowStepAnim && this._characterHeight > 0 && this._info) {
		this.updateCharacterSprite();
	}
};
//-----------------------------------------------------------------------------
// Update character sprites for step animation
//-----------------------------------------------------------------------------
CGMZ_Window_SaveFileDisplay.prototype.updateCharacterSprite = function() {
	this._charStepAnimTimer++;
	if(this._charStepAnimTimer > 15) {
		this.clearCharacterSpriteRect();
		this._charStepAnimTimer = 0;
		const modifier = (this._isNegativeCharacter) ? -1 : 1;
		this._characterXOffset += modifier;
		if(this._characterXOffset >= 1) {
			this._isNegativeCharacter = true;
		}
		else if(this._characterXOffset <= -1) {
			this._isNegativeCharacter = false;
		}
		this.drawChars(this._characterHeight);
	}
};
//-----------------------------------------------------------------------------
// Update character sprites for step animation
//-----------------------------------------------------------------------------
CGMZ_Window_SaveFileDisplay.prototype.clearCharacterSpriteRect = function() {
	const x = 0;
	const width = this.contents.width;
	const y = this._characterHeight;
	const height = CGMZ.SaveFile.CharHeight;
	this.contents.clearRect(x, y, width, height);
};
//=============================================================================
// CGMZ_Window_SaveFileConfirm
//-----------------------------------------------------------------------------
// Display save file confirm
//=============================================================================
function CGMZ_Window_SaveFileConfirm() {
	this.initialize(...arguments);
}
CGMZ_Window_SaveFileConfirm.prototype = Object.create(Window_HorzCommand.prototype);
CGMZ_Window_SaveFileConfirm.prototype.constructor = CGMZ_Window_SaveFileConfirm;
//-----------------------------------------------------------------------------
// Initialize
//-----------------------------------------------------------------------------
CGMZ_Window_SaveFileConfirm.prototype.initialize = function(rect) {
	Window_HorzCommand.prototype.initialize.call(this, rect);
	if(Imported.CGMZ_WindowBackgrounds && CGMZ.SaveFile.ConfirmWindowBackground) this.CGMZ_setWindowBackground(CGMZ.SaveFile.ConfirmWindowBackground);
	if(Imported.CGMZ_WindowSettings && CGMZ.SaveFile.ConfirmWindowSettings) this.CGMZ_setWindowSettings(CGMZ.SaveFile.ConfirmWindowSettings);
};
//-----------------------------------------------------------------------------
// Get max columns
//-----------------------------------------------------------------------------
CGMZ_Window_SaveFileConfirm.prototype.maxCols = function() {
	return 2;
};
//-----------------------------------------------------------------------------
// Make the command list
//-----------------------------------------------------------------------------
CGMZ_Window_SaveFileConfirm.prototype.makeCommandList = function() {
	this.addCommand(CGMZ.SaveFile.ConfirmText, "confirm");
	this.addCommand(CGMZ.SaveFile.CancelText, "cancel");
};
//-----------------------------------------------------------------------------
// Adjust the item rect down 1 line
//-----------------------------------------------------------------------------
CGMZ_Window_SaveFileConfirm.prototype.itemRect = function(index) {
	const rect = Window_HorzCommand.prototype.itemRect.call(this, index);
	rect.y += this.lineHeight() + 8;
	return rect;
};
//-----------------------------------------------------------------------------
// Refresh the window
//-----------------------------------------------------------------------------
CGMZ_Window_SaveFileConfirm.prototype.refresh = function() {
	Window_HorzCommand.prototype.refresh.call(this);
	this.CGMZ_drawTextLine(CGMZ.SaveFile.ConfirmWindowText, 0, 0, this.contents.width, "center");
};
//-----------------------------------------------------------------------------
// Refresh the window
//-----------------------------------------------------------------------------
CGMZ_Window_SaveFileConfirm.prototype.drawItem = function(index) {
	const rect = this.itemLineRect(index);
	const align = this.itemTextAlign();
	this.resetTextColor();
	this.changePaintOpacity(this.isCommandEnabled(index));
	this.CGMZ_drawTextLine(this.commandName(index), rect.x, rect.y, rect.width, align);
};
//=============================================================================
// CGMZ_Window_SaveFileConfirmDelete
//-----------------------------------------------------------------------------
// Display save file confirm for deletion
//=============================================================================
function CGMZ_Window_SaveFileConfirmDelete() {
	this.initialize(...arguments);
}
CGMZ_Window_SaveFileConfirmDelete.prototype = Object.create(CGMZ_Window_SaveFileConfirm.prototype);
CGMZ_Window_SaveFileConfirmDelete.prototype.constructor = CGMZ_Window_SaveFileConfirmDelete;
//-----------------------------------------------------------------------------
// Initialize the window
//-----------------------------------------------------------------------------
CGMZ_Window_SaveFileConfirmDelete.prototype.initialize = function(rect) {
	CGMZ_Window_SaveFileConfirm.prototype.initialize.call(this, rect);
	if(Imported.CGMZ_WindowBackgrounds && CGMZ.SaveFile.DeleteConfirmWindowBackground) this.CGMZ_setWindowBackground(CGMZ.SaveFile.DeleteConfirmWindowBackground);
	if(Imported.CGMZ_WindowSettings && CGMZ.SaveFile.DeleteConfirmWindowSettings) this.CGMZ_setWindowSettings(CGMZ.SaveFile.DeleteConfirmWindowSettings);
};
//=============================================================================
// CGMZ_Window_SaveFileDeleteRename
//-----------------------------------------------------------------------------
// Display save file load/save, delete, and rename options
//=============================================================================
function CGMZ_Window_SaveFileDeleteRename() {
	this.initialize(...arguments);
}
CGMZ_Window_SaveFileDeleteRename.prototype = Object.create(Window_HorzCommand.prototype);
CGMZ_Window_SaveFileDeleteRename.prototype.constructor = CGMZ_Window_SaveFileDeleteRename;
//-----------------------------------------------------------------------------
// Initialize the window
//-----------------------------------------------------------------------------
CGMZ_Window_SaveFileDeleteRename.prototype.initialize = function(rect, mode) {
	Window_HorzCommand.prototype.initialize.call(this, rect);
	if(Imported.CGMZ_WindowBackgrounds && CGMZ.SaveFile.OptionWindowBackground) this.CGMZ_setWindowBackground(CGMZ.SaveFile.OptionWindowBackground);
	if(Imported.CGMZ_WindowSettings && CGMZ.SaveFile.OptionWindowSettings) this.CGMZ_setWindowSettings(CGMZ.SaveFile.OptionWindowSettings);
	this._saveMode = mode;
	this._savefileIndex = 0;
	this.refresh();
};
//-----------------------------------------------------------------------------
// Get max columns
//-----------------------------------------------------------------------------
CGMZ_Window_SaveFileDeleteRename.prototype.maxCols = function() {
	return 2 + 1 * (CGMZ.SaveFile.ShowDeleteOption) + 1 * (CGMZ.SaveFile.ShowRenameOption);
};
//-----------------------------------------------------------------------------
// Get max columns
//-----------------------------------------------------------------------------
CGMZ_Window_SaveFileDeleteRename.prototype.setSavefileIndex = function(index) {
	this._savefileIndex = index;
	this.refresh();
};
//-----------------------------------------------------------------------------
// Make the command list
//-----------------------------------------------------------------------------
CGMZ_Window_SaveFileDeleteRename.prototype.makeCommandList = function() {
	this.addCommand(this.getConfirmOptionText(), "main");
	if(CGMZ.SaveFile.ShowDeleteOption) this.addCommand(CGMZ.SaveFile.DeleteText, "delete", this.isDeleteEnabled());
	if(CGMZ.SaveFile.ShowRenameOption) this.addCommand(CGMZ.SaveFile.RenameText, "rename", this.isRenameEnabled());
	this.addCommand(CGMZ.SaveFile.CancelText, "cancel");
};
//-----------------------------------------------------------------------------
// Check if delete is enabled
//-----------------------------------------------------------------------------
CGMZ_Window_SaveFileDeleteRename.prototype.isDeleteEnabled = function() {
	return DataManager.savefileExists(this._savefileIndex);
};
//-----------------------------------------------------------------------------
// Check if rename is enabled
//-----------------------------------------------------------------------------
CGMZ_Window_SaveFileDeleteRename.prototype.isRenameEnabled = function() {
	return DataManager.savefileExists(this._savefileIndex);
};
//-----------------------------------------------------------------------------
// Get the confirm option text depending on mode
//-----------------------------------------------------------------------------
CGMZ_Window_SaveFileDeleteRename.prototype.getConfirmOptionText = function() {
	if(this._saveMode === "save") {
		return CGMZ.SaveFile.SaveText;
	}
	return CGMZ.SaveFile.LoadText;
};
//-----------------------------------------------------------------------------
// Refresh the window
//-----------------------------------------------------------------------------
CGMZ_Window_SaveFileDeleteRename.prototype.drawItem = function(index) {
	const rect = this.itemLineRect(index);
	const align = this.itemTextAlign();
	this.resetTextColor();
	this.changePaintOpacity(this.isCommandEnabled(index));
	this.CGMZ_drawTextLine(this.commandName(index), rect.x, rect.y, rect.width, align);
};
//=============================================================================
// CGMZ_SaveFile_Window_NameEdit
//-----------------------------------------------------------------------------
// Display save file name
//=============================================================================
function CGMZ_SaveFile_Window_NameEdit() {
	this.initialize(...arguments);
}
CGMZ_SaveFile_Window_NameEdit.prototype = Object.create(Window_NameEdit.prototype);
CGMZ_SaveFile_Window_NameEdit.prototype.constructor = CGMZ_SaveFile_Window_NameEdit;
//-----------------------------------------------------------------------------
// Do not need to set up actor
//-----------------------------------------------------------------------------
CGMZ_SaveFile_Window_NameEdit.prototype.initialize = function(rect) {
	Window_NameEdit.prototype.initialize.call(this, rect);
	if(Imported.CGMZ_WindowBackgrounds && CGMZ.SaveFile.NameWindowBackground) this.CGMZ_setWindowBackground(CGMZ.SaveFile.NameWindowBackground);
	if(Imported.CGMZ_WindowSettings && CGMZ.SaveFile.NameWindowSettings) this.CGMZ_setWindowSettings(CGMZ.SaveFile.NameWindowSettings);
};
//-----------------------------------------------------------------------------
// Do not need to set up actor
//-----------------------------------------------------------------------------
CGMZ_SaveFile_Window_NameEdit.prototype.setup = function(maxLength) {
	this._saveFileId = 0;
	this._maxLength = maxLength;
	this._name = "";
	this._index = this._name.length;
	this._defaultName = "";
};
//-----------------------------------------------------------------------------
// Set the save file for rename
//-----------------------------------------------------------------------------
CGMZ_SaveFile_Window_NameEdit.prototype.setSaveFileId = function(savefileId) {
	this._saveFileId = savefileId;
	const info = DataManager.savefileInfo(this._saveFileId);
	if(info && info.cgmz_savefileName) {
		this._name = info.cgmz_savefileName;
		this._defaultName = this._name;
		this._index = this._name.length;
	} else {
		this.makeDefaultName();
	}
};
//-----------------------------------------------------------------------------
// Make the default filename
//-----------------------------------------------------------------------------
CGMZ_SaveFile_Window_NameEdit.prototype.makeDefaultName = function() {
	if(this._saveFileId === 0) {
		this._name = TextManager.autosave;
	} else {
		this._name = TextManager.file + " " + this._saveFileId;
	}
	this._defaultName = this._name;
	this._index = this._name.length;
};
//-----------------------------------------------------------------------------
// Get the left most item
//-----------------------------------------------------------------------------
CGMZ_SaveFile_Window_NameEdit.prototype.left = function() {
	const nameCenter = (this.innerWidth) / 2;
	const nameWidth = (this._maxLength + 1) * this.charWidth();
	return Math.min(nameCenter - nameWidth / 2, this.innerWidth - nameWidth);
};
//-----------------------------------------------------------------------------
// Get the item rect
//-----------------------------------------------------------------------------
CGMZ_SaveFile_Window_NameEdit.prototype.itemRect = function(index) {
	const x = this.left() + index * this.charWidth();
	const y = 0;
	const width = this.charWidth();
	const height = this.lineHeight();
	return new Rectangle(x, y, width, height);
};
//-----------------------------------------------------------------------------
// Refresh the window
//-----------------------------------------------------------------------------
CGMZ_SaveFile_Window_NameEdit.prototype.refresh = function() {
	this.contents.clear();
	for (let i = 0; i < this._maxLength; i++) {
		this.drawUnderline(i);
	}
	for (let j = 0; j < this._name.length; j++) {
		this.drawChar(j);
	}
	const rect = this.itemRect(this._index);
	this.setCursorRect(rect.x, rect.y, rect.width, rect.height);
};
//=============================================================================
// CGMZ_SaveFile_Window_NameInput
//-----------------------------------------------------------------------------
// Display save file name input
//=============================================================================
function CGMZ_SaveFile_Window_NameInput() {
	this.initialize(...arguments);
}
CGMZ_SaveFile_Window_NameInput.prototype = Object.create(Window_NameInput.prototype);
CGMZ_SaveFile_Window_NameInput.prototype.constructor = CGMZ_SaveFile_Window_NameInput;
//-----------------------------------------------------------------------------
// Initialize window
//-----------------------------------------------------------------------------
CGMZ_SaveFile_Window_NameInput.prototype.initialize = function(rect) {
	Window_NameInput.prototype.initialize.call(this, rect);
	if(Imported.CGMZ_WindowBackgrounds && CGMZ.SaveFile.InputWindowBackground) this.CGMZ_setWindowBackground(CGMZ.SaveFile.InputWindowBackground);
	if(Imported.CGMZ_WindowSettings && CGMZ.SaveFile.InputWindowSettings) this.CGMZ_setWindowSettings(CGMZ.SaveFile.InputWindowSettings);
};
//=============================================================================
// CGMZ_SaveFile_Window_Help
//-----------------------------------------------------------------------------
// Display save file help window
//=============================================================================
function CGMZ_SaveFile_Window_Help() {
	this.initialize(...arguments);
}
CGMZ_SaveFile_Window_Help.prototype = Object.create(Window_Help.prototype);
CGMZ_SaveFile_Window_Help.prototype.constructor = CGMZ_SaveFile_Window_Help;
//-----------------------------------------------------------------------------
// Initialize window
//-----------------------------------------------------------------------------
CGMZ_SaveFile_Window_Help.prototype.initialize = function(rect) {
	Window_Help.prototype.initialize.call(this, rect);
	if(Imported.CGMZ_WindowBackgrounds && CGMZ.SaveFile.HelpWindowBackground) this.CGMZ_setWindowBackground(CGMZ.SaveFile.HelpWindowBackground);
	if(Imported.CGMZ_WindowSettings && CGMZ.SaveFile.HelpWindowSettings) this.CGMZ_setWindowSettings(CGMZ.SaveFile.HelpWindowSettings);
};