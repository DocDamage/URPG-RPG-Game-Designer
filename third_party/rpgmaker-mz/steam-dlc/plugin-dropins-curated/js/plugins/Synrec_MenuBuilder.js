/*:
 * @author Synrec/Kylestclr
 * @plugindesc v3.4 Allows for creation of custom menus
 * @target MZ
 * @url https://synrec.itch.io/
 *
 * @help
 * Use the plugin parameters to setup custom menus.
 * Ensure menu identifier names are unique.
 *
 * You can open the live menu editor by pressing the default "F9" key
 * (Or otherwise set in the plugin parameters)whilst in a custom menu
 * scene and this is the only way to open the live editor. Attempting
 * to open the live editor otherwise will not work.
 *
 * You must also close the RPG Maker Project Editor if you want to
 * save any changes from the live editor otherwise all menu changes
 * will be temporary.
 *
 * The live editor allows for real time customization of your custom
 * menu scenes and real time setting of scene overrides.
 *
 * You may visit the Youtube playlist to see examples on how to setup
 * custom menus here:
 * https://www.youtube.com/playlist?list=PLLNw1pfxQDSluUMNnzdeEnlEMF-OYn0OE
 *
 * Some parameters may be substituted for game variables/switches. The
 * live editor may not be truly capable of handling these RPG Maker
 * specific calls just yet and as such, it will auto set any number
 * requiring parameter to zero (0).
 *
 * The live editor is made from html, css and javascript and does NOT
 * link to any external server/client. If you find it linking to any
 * non-local service, delete the editor and contact the developer
 * with information as to where you obtained the plugin and source from
 * as it is likely you have a maliciously modified version.
 *
 * The live menu editor is not designed for deployment projects and
 * as such, it must be deleted from projects that are to be deployed.
 *
 * Menus can be "nested"/lead further into more menus.
 *
 * You can play video on menu by using script calls.
 * - this.playVideo(name, label, loop, x, y, width, height);
 * -- name = video file name (without extension)
 * -- label = video identifier in scene so that you can specifically end it
 * -- loop = video will continuously loop
 * -- x = horizontal position of video
 * -- y = vertical position of video
 * -- width/height = size of video. Set to zero (0) for video default
 *
 * - this.stopVideo(label)
 * -- label = video identifier in scene
 *
 * You can call a scene by using SceneManager.push(Menu_Class_Name)
 * Where "Menu_Class_Name" can be the following:
 * - Scene_Item
 * - Scene_Skill
 * - Scene_Equip
 * - Scene_Status
 * - Scene_Options
 * - Scene_Save
 * - Scene_Load
 * - Scene_Debug
 *
 * You can see the "Menu_Class_Name" options from rmmz_scenes.js (if using MZ)
 * or rpg_scenes.js (if using MV)
 *
 * Convenient Script Calls:
 * > DataManager.menuBuilderGameReset()
 * - Resets all game objects and sets up project as for new game
 *
 * > $gameTemp.openMenu("name")
 * - Opens custom menu scene  by identifier, "name"
 *
 * > SceneManager._scene.closeMenu()
 * - If in custom menu scene, closes the menu properly.
 * - Do not use $gameTemp.closeMenu()
 *
 * > $gameTemp.openedMenu()
 * - Returns data of opened menu
 *
 * @param Editor Access Button Name
 * @desc Name of the button to access editor
 * Default: debug
 * @type text
 * @default debug
 *
 * @param Menu Configurations
 * @desc Setup custom menus
 * @type struct<custMenu>[]
 * @default []
 *
 * @param Scene Overrides
 * @desc Override scene class
 * @type struct<sceneOverride>[]
 * @default []
 *
 */
/*~struct~sceneOverride:
 *
 * @param Name
 * @desc No function
 * @type text
 * @default OVERRIDE
 *
 * @param Scene Class
 * @desc The scene class to override
 * @type text
 * @default Scene_Menu
 *
 * @param Menu Identifier
 * @desc The identifier of the menu to load
 * @type text
 * @default menu
 *
 */
/*~struct~staticGfx:
 *
 * @param File
 * @desc Graphic file
 * @type file
 * @dir img/pictures/
 *
 * @param X
 * @desc Position Setting
 * @type text
 * @default  0
 *
 * @param Y
 * @desc Position Setting
 * @type text
 * @default  0
 *
 * @param Scrolling X
 * @desc Scroll image consistently on origin.
 * @type number
 * @default 0
 *
 * @param Scrolling Y
 * @desc Scroll image consistently on origin.
 * @type number
 * @default 0
 *
 * @param Anchor X
 * @desc Modify image pivot point.
 * @type number
 * @decimals 3
 * @min -999999
 * @default 0.000
 *
 * @param Anchor Y
 * @desc Modify image pivot point.
 * @type number
 * @decimals 3
 * @min -999999
 * @default 0.000
 *
 * @param Rotation
 * @desc Apply rotation to image.
 * @type number
 * @decimals 3
 * @min -999999
 * @default 0.000
 *
 * @param Constant Rotation
 * @desc Repeatedly apply rotation
 * @type boolean
 * @default false
 *
 */
/*~struct~animGfx:
 *
 * @param File
 * @desc Graphic file
 * @type file
 * @dir img/pictures/
 *
 * @param X
 * @desc Position Setting
 * @type number
 * @default  0
 *
 * @param Y
 * @desc Position Setting
 * @type number
 * @default  0
 *
 * @param Max Frames
 * @desc Number of frames the graphic uses
 * @type number
 * @min 1
 * @default 1
 *
 * @param Frame Rate
 * @desc Speed at which frames update
 * @type number
 * @default 0
 *
 */
/*~struct~winText:
 *
 * @param Text
 * @desc Text to draw
 * @type note
 * @default ""
 *
 * @param X
 * @desc Starting position of text
 * @type text
 * @default 0
 *
 * @param Y
 * @desc Starting position of text
 * @type text
 * @default 0
 *
 */
/*~struct~gfx:
 *
 * @param Picture
 * @desc Picture file to use
 * @type file
 * @dir img/pictures/
 *
 * @param X
 * @desc Starting position of picture
 * @type text
 * @default 0
 *
 * @param Y
 * @desc Starting position of picture
 * @type text
 * @default 0
 *
 * @param Width
 * @desc Size of the picture
 * @type text
 * @default 1
 *
 * @param Height
 * @desc Size of the picture
 * @type text
 * @default 1
 *
 */
/*~struct~locSize:
 *
 * @param X
 * @desc Position setting.
 * @type number
 * @default 0
 *
 * @param Y
 * @desc Position setting.
 * @type number
 * @default 0
 *
 * @param Width
 * @desc Size setting.
 * @type number
 * @default 1
 *
 * @param Height
 * @desc Size setting.
 * @type number
 * @default 1
 *
 */
/*~struct~windowStyle:
 *
 * @param Font Settings
 * @desc Setup child parameters
 *
 * @param Font Size
 * @parent Font Settings
 * @desc Size of font.
 * @type number
 * @default 16
 *
 * @param Font Face
 * @parent Font Settings
 * @desc Font face used for the window.
 * @type text
 * @default sans-serif
 *
 * @param Base Font Color
 * @parent Font Settings
 * @desc Default font color for window
 * @type text
 * @default #ffffff
 *
 * @param Font Outline Color
 * @parent Font Settings
 * @desc Default font color for window
 * @type text
 * @default rgba(0, 0, 0, 0.5)
 *
 * @param Font Outline Thickness
 * @parent Font Settings
 * @desc The thickness of the text outline
 * @type number
 * @default 3
 *
 * @param Window Skin
 * @desc Image file used for the window skin.
 * @type file
 * @dir img/system/
 * @default Window
 *
 * @param Window Opacity
 * @desc 0 = Fully transparent, 255 = Fully opaque.
 * @type number
 * @default 255
 *
 * @param Show Window Dimmer
 * @desc Hides window skin
 * @type boolean
 * @default false
 *
 */
/*~struct~displayRequirements:
 *
 * @param Game Switch
 * @desc This switch must be ON
 * @type switch
 * @default 0
 *
 * @param Game Variable
 * @desc The variable to consider
 * @type variable
 * @default 0
 *
 * @param Variable Minimum
 * @parent Game Variable
 * @desc Lowest value of the variable
 * @type text
 * @default 0
 *
 * @param Variable Maximum
 * @parent Game Variable
 * @desc Highest value of the variable
 * @type text
 * @default 0
 *
 * @param Code
 * @desc Code must evaluate to a "true" value
 * @type note
 * @default ""
 *
 */
/*~struct~gaugeDraw:
 *
 * @param Label
 * @desc Label text for gauge
 * @type text
 * @default gauge
 *
 * @param Label X
 * @desc Position of the label text in window
 * @type text
 * @default 0
 *
 * @param Label Y
 * @desc Position of the label text in window
 * @type text
 * @default 0
 *
 * @param Gauge Current Value
 * @desc How to set gauge current value
 * Evaluated value.
 * @type text
 * @default
 *
 * @param Gauge Max Value
 * @desc How to set gauge max value
 * Evaluated value.
 * @type text
 * @default
 *
 * @param Gauge X
 * @desc Position of the gauge in window
 * @type text
 * @default 0
 *
 * @param Gauge Y
 * @desc Position of the gauge in window
 * @type text
 * @default 0
 *
 * @param Gauge Width
 * @desc Size of the gauge
 * @type text
 * @default 1
 *
 * @param Gauge Height
 * @desc Size of the gauge
 * @type text
 * @default 1
 * @default 1
 *
 * @param Gauge Border
 * @desc Border size indent of the gauge
 * @type text
 * @default 2
 *
 * @param Gauge Border Color
 * @desc Color for gauge border
 * @type text
 * @default #000000
 *
 * @param Gauge Background Color
 * @desc Color for gauge background
 * @type text
 * @default #666666
 *
 * @param Gauge Color
 * @desc Color for gauge background
 * @type text
 * @default #aaffaa
 *
 */
/*~struct~basicWin:
 *
 * @param Name
 * @desc No function
 * @type text
 * @default Window
 *
 * @param Dimension Configuration
 * @desc Setup position and width of the window
 * @type struct<locSize>
 * @default {"X":"0","Y":"0","Width":"1","Height":"1"}
 *
 * @param Window Font and Style Configuration
 * @desc Custom style the window
 * @type struct<windowStyle>
 * @default {"Font Settings":"","Font Size":"16","Font Face":"sans-serif","Base Font Color":"#ffffff","Font Outline Color":"rgba(0, 0, 0, 0.5)","Font Outline Thickness":"3","Window Skin":"Window","Window Opacity":"255","Show Window Dimmer":"false"}
 *
 * @param Display Requirements
 * @desc Conditions to display the window
 * @type struct<displayRequirements>
 *
 * @param Draw Texts
 * @desc Draw various text
 * @type struct<winText>[]
 * @default []
 *
 * @param Text References
 * @parent Draw Texts
 * @desc Set code references for draw text
 * %1 = first, %2 = second, etc...
 * @type text[]
 * @default []
 *
 * @param Draw Pictures
 * @desc Draw various pictures
 * @type struct<gfx>[]
 * @default []
 *
 * @param Gauges
 * @desc Draw gauges on the window
 * @type struct<gaugeDraw>[]
 * @default []
 *
 */
/*~struct~gaugeDraw:
 *
 * @param Label
 * @desc Label text for gauge
 * @type text
 * @default gauge
 *
 * @param Label X
 * @desc Position of the label text in window
 * @type text
 * @default 0
 *
 * @param Label Y
 * @desc Position of the label text in window
 * @type text
 * @default 0
 *
 * @param Gauge Current Value
 * @desc How to set gauge current value
 * @type text
 * @default
 *
 * @param Gauge Max Value
 * @desc How to set gauge max value
 * @type text
 * @default
 *
 * @param Gauge X
 * @desc Position of the gauge in window
 * @type text
 * @default 0
 *
 * @param Gauge Y
 * @desc Position of the gauge in window
 * @type text
 * @default 0
 *
 * @param Gauge Width
 * @desc Size of the gauge
 * @type text
 * @default 1
 *
 * @param Gauge Height
 * @desc Size of the gauge
 * @type text
 * @default 1
 * @default 1
 *
 * @param Gauge Border
 * @desc Border size indent of the gauge
 * @type text
 * @default 2
 *
 * @param Gauge Border Color
 * @desc Color for gauge border
 * @type text
 * @default #000000
 *
 * @param Gauge Background Color
 * @desc Color for gauge background
 * @type text
 * @default #666666
 *
 * @param Gauge Color
 * @desc Color for gauge background
 * @type text
 * @default #aaffaa
 *
 */
/*~struct~actorBaseParamWindow:
 *
 * @param Name
 * @desc No function.
 * @type text
 * @default Window
 *
 * @param Base Param
 * @desc The base param to draw
 * @type select
 * @option mhp
 * @value 0
 * @option mmp
 * @value 1
 * @option atk
 * @value 2
 * @option def
 * @value 3
 * @option mat
 * @value 4
 * @option mdf
 * @value 5
 * @option agi
 * @value 6
 * @option luk
 * @value 7
 * @default 0
 *
 * @param Param Text
 * @desc How to draw param text
 * %1 = param value
 * @type text
 * @default %1
 *
 * @param X
 * @desc Position in window
 * @type text
 * @default 0
 *
 * @param Y
 * @desc Position in window
 * @type text
 * @default 0
 *
 */
/*~struct~actorExParamWindow:
 *
 * @param Name
 * @desc No function.
 * @type text
 * @default Window
 *
 * @param Ex Param
 * @desc The ex param to draw. Converted to percentage.
 * @type select
 * @option Hit Rate
 * @value 0
 * @option Evasion Rate
 * @value 1
 * @option Critical Rate
 * @value 2
 * @option Critical Evasion Rate
 * @value 3
 * @option Magic Evasion Rate
 * @value 4
 * @option Magic Reflection Rate
 * @value 5
 * @option Counter Attack Rate
 * @value 6
 * @option HP Regeneration Rate
 * @value 7
 * @option MP Regeneration Rate
 * @value 8
 * @option TP Regeneration Rate
 * @value 9
 * @default 0
 *
 * @param Param Text
 * @desc How to draw param text
 * %1 = param value
 * @type text
 * @default %1
 *
 * @param X
 * @desc Position in window
 * @type text
 * @default 0
 *
 * @param Y
 * @desc Position in window
 * @type text
 * @default 0
 *
 */
/*~struct~actorSPParamWindow:
 *
 * @param Name
 * @desc No function.
 * @type text
 * @default Window
 *
 * @param Sp Param
 * @desc The sp param to draw. Converted to percentage.
 * @type select
 * @option Target Rate
 * @value 0
 * @option Guard Effect Rate
 * @value 1
 * @option Recovery Effect Rate
 * @value 2
 * @option Pharmacology
 * @value 3
 * @option MP Cost Rate
 * @value 4
 * @option TP Charge Rate
 * @value 5
 * @option Physical Damage Rate
 * @value 6
 * @option Magical Damage Rate
 * @value 7
 * @option Floor Damage Rate
 * @value 8
 * @option Experience Rate
 * @value 9
 * @default 0
 *
 * @param Param Text
 * @desc How to draw param text
 * %1 = param value
 * @type text
 * @default %1
 *
 * @param X
 * @desc Position in window
 * @type text
 * @default 0
 *
 * @param Y
 * @desc Position in window
 * @type text
 * @default 0
 *
 */
/*~struct~actorDataWindow:
 *
 * @param Name
 * @desc No function.
 * @type text
 * @default Window
 *
 * @param Dimension Configuration
 * @desc Setup position and width of the window
 * @type struct<locSize>
 * @default {"X":"0","Y":"0","Width":"1","Height":"1"}
 *
 * @param Window Font and Style Configuration
 * @desc Custom style the window
 * @type struct<windowStyle>
 * @default {"Font Settings":"","Font Size":"16","Font Face":"sans-serif","Base Font Color":"#ffffff","Font Outline Color":"rgba(0, 0, 0, 0.5)","Font Outline Thickness":"3","Window Skin":"Window","Window Opacity":"255","Show Window Dimmer":"false"}
 *
 * @param Display Requirements
 * @desc Conditions to display the window
 * @type struct<displayRequirements>
 *
 * @param Gauges
 * @desc Setup gauges for the window
 * @type struct<gaugeDraw>[]
 * @default []
 *
 * @param Draw Actor Name
 * @desc Draw actor name
 * @type boolean
 * @default false
 *
 * @param Name Text
 * @parent Draw Actor Name
 * @desc Text used for the name
 * %1 = Name, %2 = Nickname
 * @type text
 * @default %1
 *
 * @param Name X
 * @parent Draw Actor Name
 * @desc Position of name in window
 * @type text
 * @default 0
 *
 * @param Name Y
 * @parent Draw Actor Name
 * @desc Position of name in window
 * @type text
 * @default 0
 *
 * @param Draw Actor Profile
 * @desc Draw actor profile
 * @type boolean
 * @default false
 *
 * @param Profile X
 * @parent Draw Actor Profile
 * @desc Position of profile in window
 * @type text
 * @default 0
 *
 * @param Profile Y
 * @parent Draw Actor Profile
 * @desc Position of profile in window
 * @type text
 * @default 0
 *
 * @param Draw Class Level
 * @desc Draw actor class name and level
 * @type boolean
 * @default false
 *
 * @param Class Level Text
 * @parent Draw Class Level
 * @desc Draw class name and level
 * %1 = class name, %2 = level
 * @type text
 * @default Class: %1 <%2>
 *
 * @param Class Level X
 * @parent Draw Class Level
 * @desc Position of class level in window.
 * @type text
 * @default 0
 *
 * @param Class Level Y
 * @parent Draw Class Level
 * @desc Position of class level in window.
 * @type text
 * @default 0
 *
 * @param Draw HP Resource
 * @desc Draw actor current and max HP
 * @type boolean
 * @default false
 *
 * @param HP Text
 * @parent Draw HP Resource
 * @desc Text for HP (Escape chars allowed)
 * %1 = Current, %2 = Max
 * @type text
 * @default \I[84]%1 / %2
 *
 * @param HP X
 * @parent Draw HP Resource
 * @desc Position of text in window
 * @type text
 * @default 0
 *
 * @param HP Y
 * @parent Draw HP Resource
 * @desc Position of text in window
 * @type text
 * @default 0
 *
 * @param Draw MP Resource
 * @desc Draw actor current and max MP
 * @type boolean
 * @default false
 *
 * @param MP Text
 * @parent Draw MP Resource
 * @desc Text for MP (Escape chars allowed)
 * %1 = Current, %2 = Max
 * @type text
 * @default \I[79]%1 / %2
 *
 * @param MP X
 * @parent Draw MP Resource
 * @desc Position of text in window
 * @type text
 * @default 0
 *
 * @param MP Y
 * @parent Draw MP Resource
 * @desc Position of text in window
 * @type text
 * @default 0
 *
 * @param Draw TP Resource
 * @desc Draw actor current and max TP
 * @type boolean
 * @default false
 *
 * @param TP Text
 * @parent Draw TP Resource
 * @desc Text for TP (Escape chars allowed)
 * %1 = Current, %2 = Max
 * @type text
 * @default \I[79]%1 / %2
 *
 * @param TP X
 * @parent Draw TP Resource
 * @desc Position of text in window
 * @type text
 * @default 0
 *
 * @param TP Y
 * @parent Draw TP Resource
 * @desc Position of text in window
 * @type text
 * @default 0
 *
 * @param Draw Base Params
 * @desc Draw actor base params
 * @type struct<actorBaseParamWindow>[]
 * @default []
 *
 * @param Draw Ex Params
 * @desc Draw actor extra params
 * @type struct<actorExParamWindow>[]
 * @default []
 *
 * @param Draw Sp Params
 * @desc Draw actor special params
 * @type struct<actorSpParamWindow>[]
 * @default []
 *
 * @param Display Map Character
 * @desc Display actor map character
 * @type boolean
 * @default false
 *
 * @param Character Direction
 * @parent Display Map Character
 * @desc Facing direction of the character.
 * @type select
 * @option down
 * @value 2
 * @option left
 * @value 4
 * @option right
 * @value 6
 * @option up
 * @value 8
 * @default 2
 *
 * @param Character X
 * @parent Display Map Character
 * @desc Position relative to window
 * @type text
 * @default 0
 *
 * @param Character Y
 * @parent Display Map Character
 * @desc Position relative to window
 * @type text
 * @default 0
 *
 * @param Character Scale X
 * @parent Display Map Character
 * @desc Size of the character
 * @type text
 * @default 1
 *
 * @param Character Scale Y
 * @parent Display Map Character
 * @desc Size of the character
 * @type text
 * @default 1
 *
 * @param Display Battler
 * @desc Display actor battler
 * @type boolean
 * @default false
 *
 * @param Battler Motion
 * @parent Display Battler
 * @desc Battler motion to refresh to
 * @type select
 * @option walk
 * @option wait
 * @option chant
 * @option guard
 * @option damage
 * @option evade
 * @option thrust
 * @option swing
 * @option missile
 * @option skill
 * @option spell
 * @option item
 * @option escape
 * @option victory
 * @option dying
 * @option abnormal
 * @option sleep
 * @option dead
 * @default wait
 *
 * @param Battler X
 * @parent Display Battler
 * @desc Position relative to window
 * @type text
 * @default 0
 *
 * @param Battler Y
 * @parent Display Battler
 * @desc Position relative to window
 * @type text
 * @default 0
 *
 * @param Battler Scale X
 * @parent Display Battler
 * @desc Size of the battler
 * @type text
 * @default 1
 *
 * @param Battler Scale Y
 * @parent Display Battler
 * @desc Size of the battler
 * @type text
 * @default 1
 *
 */
/*~struct~actorSelcWindow:
 *
 * @param Name
 * @desc No function.
 * @type text
 * @default Window
 *
 * @param Dimension Configuration
 * @desc Setup position and width of the window
 * @type struct<locSize>
 * @default {"X":"0","Y":"0","Width":"1","Height":"1"}
 *
 * @param Window Font and Style Configuration
 * @desc Custom style the window
 * @type struct<windowStyle>
 * @default {"Font Settings":"","Font Size":"16","Font Face":"sans-serif","Base Font Color":"#ffffff","Font Outline Color":"rgba(0, 0, 0, 0.5)","Font Outline Thickness":"3","Window Skin":"Window","Window Opacity":"255","Show Window Dimmer":"false"}
 *
 * @param Max Columns
 * @desc Max columns the window will use
 * @type number
 * @default 1
 *
 * @param Item Width
 * @desc Max width of window items. 0 = Default
 * @type number
 * @default 0
 *
 * @param Item Height
 * @desc Max Item height of window items. 0 = Default
 * @type number
 * @default 0
 *
 * @param Gauges
 * @desc Setup gauges for the window
 * @type struct<gaugeDraw>[]
 * @default []
 *
 * @param Draw Actor Name
 * @desc Draw actor name
 * @type boolean
 * @default false
 *
 * @param Name Text
 * @parent Draw Actor Name
 * @desc Text used for the name
 * %1 = Name, %2 = Nickname
 * @type text
 * @default %1
 *
 * @param Name X
 * @parent Draw Actor Name
 * @desc Position of name in window
 * @type text
 * @default 0
 *
 * @param Name Y
 * @parent Draw Actor Name
 * @desc Position of name in window
 * @type text
 * @default 0
 *
 * @param Draw Actor Profile
 * @desc Draw actor profile
 * @type boolean
 * @default false
 *
 * @param Profile X
 * @parent Draw Actor Profile
 * @desc Position of profile in window
 * @type text
 * @default 0
 *
 * @param Profile Y
 * @parent Draw Actor Profile
 * @desc Position of profile in window
 * @type text
 * @default 0
 *
 * @param Draw Class Level
 * @desc Draw actor class name and level
 * @type boolean
 * @default false
 *
 * @param Class Level Text
 * @parent Draw Class Level
 * @desc Draw class name and level
 * %1 = class name, %2 = level
 * @type text
 * @default Class: %1 <%2>
 *
 * @param Class Level X
 * @parent Draw Class Level
 * @desc Position of class level in window.
 * @type text
 * @default 0
 *
 * @param Class Level Y
 * @parent Draw Class Level
 * @desc Position of class level in window.
 * @type text
 * @default 0
 *
 * @param Draw HP Resource
 * @desc Draw actor current and max HP
 * @type boolean
 * @default false
 *
 * @param HP Text
 * @parent Draw HP Resource
 * @desc Text for HP (Escape chars allowed)
 * %1 = Current, %2 = Max
 * @type text
 * @default \I[84]%1 / %2
 *
 * @param HP X
 * @parent Draw HP Resource
 * @desc Position of text in window
 * @type text
 * @default 0
 *
 * @param HP Y
 * @parent Draw HP Resource
 * @desc Position of text in window
 * @type text
 * @default 0
 *
 * @param Draw MP Resource
 * @desc Draw actor current and max MP
 * @type boolean
 * @default false
 *
 * @param MP Text
 * @parent Draw MP Resource
 * @desc Text for MP (Escape chars allowed)
 * %1 = Current, %2 = Max
 * @type text
 * @default \I[79]%1 / %2
 *
 * @param MP X
 * @parent Draw MP Resource
 * @desc Position of text in window
 * @type text
 * @default 0
 *
 * @param MP Y
 * @parent Draw MP Resource
 * @desc Position of text in window
 * @type text
 * @default 0
 *
 * @param Draw TP Resource
 * @desc Draw actor current and max TP
 * @type boolean
 * @default false
 *
 * @param TP Text
 * @parent Draw TP Resource
 * @desc Text for TP (Escape chars allowed)
 * %1 = Current, %2 = Max
 * @type text
 * @default \I[79]%1 / %2
 *
 * @param TP X
 * @parent Draw TP Resource
 * @desc Position of text in window
 * @type text
 * @default 0
 *
 * @param TP Y
 * @parent Draw TP Resource
 * @desc Position of text in window
 * @type text
 * @default 0
 *
 * @param Draw Base Params
 * @desc Draw actor base params
 * @type struct<actorBaseParamWindow>[]
 * @default []
 *
 * @param Draw Ex Params
 * @desc Draw actor extra params
 * @type struct<actorExParamWindow>[]
 * @default []
 *
 * @param Draw Sp Params
 * @desc Draw actor special params
 * @type struct<actorSpParamWindow>[]
 * @default []
 *
 * @param Display Map Character
 * @desc Display actor map character
 * @type boolean
 * @default false
 *
 * @param Character Direction
 * @parent Display Map Character
 * @desc Facing direction of the character.
 * @type select
 * @option down
 * @value 2
 * @option left
 * @value 4
 * @option right
 * @value 6
 * @option up
 * @value 8
 * @default 2
 *
 * @param Character X
 * @parent Display Map Character
 * @desc Position relative to window
 * @type text
 * @default 0
 *
 * @param Character Y
 * @parent Display Map Character
 * @desc Position relative to window
 * @type text
 * @default 0
 *
 * @param Character Scale X
 * @parent Display Map Character
 * @desc Size of the character
 * @type text
 * @default 1
 *
 * @param Character Scale Y
 * @parent Display Map Character
 * @desc Size of the character
 * @type text
 * @default 1
 *
 * @param Display Battler
 * @desc Display actor battler
 * @type boolean
 * @default false
 *
 * @param Battler Motion
 * @parent Display Battler
 * @desc Battler motion to refresh to
 * @type select
 * @option walk
 * @option wait
 * @option chant
 * @option guard
 * @option damage
 * @option evade
 * @option thrust
 * @option swing
 * @option missile
 * @option skill
 * @option spell
 * @option item
 * @option escape
 * @option victory
 * @option dying
 * @option abnormal
 * @option sleep
 * @option dead
 * @default wait
 *
 * @param Battler X
 * @parent Display Battler
 * @desc Position relative to window
 * @type text
 * @default 0
 *
 * @param Battler Y
 * @parent Display Battler
 * @desc Position relative to window
 * @type text
 * @default 0
 *
 * @param Battler Scale X
 * @parent Display Battler
 * @desc Size of the battler
 * @type text
 * @default 1
 *
 * @param Battler Scale Y
 * @parent Display Battler
 * @desc Size of the battler
 * @type text
 * @default 1
 *
 */
/*~struct~stockVarReq:
 *
 * @param Name
 * @desc No function
 * @type text
 * @default Variable
 *
 * @param Variable
 * @desc The game variable considered
 * @type variable
 * @default 0
 *
 * @param Min Value
 * @desc Smallest value of the variable
 * @type text
 * @default 0
 *
 * @param Max Value
 * @desc Largest value of the variable
 * @type text
 * @default 0
 *
 */
/*~struct~stockItmReq:
 *
 * @param Name
 * @desc No function
 * @type text
 * @default Variable
 *
 * @param Item
 * @desc The game item considered
 * @type item
 * @default 0
 *
 * @param Min Value
 * @desc Smallest amount of the item.
 * @type text
 * @default 0
 *
 * @param Max Value
 * @desc Largest amount of the item.
 * @type text
 * @default 0
 *
 */
/*~struct~stockWepReq:
 *
 * @param Name
 * @desc No function
 * @type text
 * @default Variable
 *
 * @param Weapon
 * @desc The game weapon considered
 * @type weapon
 * @default 0
 *
 * @param Min Value
 * @desc Smallest amount of the weapon.
 * @type text
 * @default 0
 *
 * @param Max Value
 * @desc Largest amount of the weapon.
 * @type text
 * @default 0
 *
 */
/*~struct~stockArmReq:
 *
 * @param Name
 * @desc No function
 * @type text
 * @default Variable
 *
 * @param Armor
 * @desc The game weapon considered
 * @type armor
 * @default 0
 *
 * @param Min Value
 * @desc Smallest amount of the armor.
 * @type text
 * @default 0
 *
 * @param Max Value
 * @desc Largest amount of the armor.
 * @type text
 * @default 0
 *
 */
/*~struct~optReq:
 *
 * @param Variable Requirements
 * @desc Setup multiple variable requirements
 * @type struct<stockVarReq>[]
 * @default []
 *
 * @param Switch Requirements
 * @desc The listed switches must be enabled/ON.
 * @type switch[]
 * @default []
 *
 * @param Item Requirements
 * @desc Setup multiple item requirements.
 * @type struct<stockItmReq>[]
 * @default []
 *
 * @param Weapon Requirements
 * @desc Setup multiple weapon requirements.
 * @type struct<stockWepReq>[]
 * @default []
 *
 * @param Armor Requirements
 * @desc Setup multiple armor requirements.
 * @type struct<stockArmReq>[]
 * @default []
 *
 */
/*~struct~selcOpt:
 *
 * @param Name
 * @desc Name of the option
 * @type text
 * @default Option
 *
 * @param Alternative Name
 * @desc Alternative name of the option for display use
 * @type text
 * @default Alternative Name
 *
 * @param Display Requirements
 * @desc Requirements to list option in menu
 * @type struct<optReq>
 *
 * @param Select Requirements
 * @desc Requirements to list option in menu
 * @type struct<optReq>
 *
 * @param Description
 * @desc How to describe the selection
 * @type note
 * @default ""
 *
 * @param Pictures
 * @desc Picture images used to represent selection
 * @type file[]
 * @dir img/pictures/
 * @default []
 *
 * @param Static Graphic
 * @desc Image layered over scene background and just below back graphics
 * @type struct<staticGfx>
 *
 * @param Animated Graphic
 * @desc Animated graphic to represent selection
 * @type struct<animGfx>
 *
 * @param Video
 * @desc Play video with selection
 * Video is played below video layer
 * @type file
 * @dir movies/
 *
 * @param Video X
 * @parent Video
 * @desc Position video
 * @type text
 * @default 0
 *
 * @param Video Y
 * @parent Video
 * @desc Position video
 * @type text
 * @default 0
 *
 * @param Video Width
 * @parent Video
 * @desc Set video size
 * Use 0 for default
 * @type text
 * @default 0
 *
 * @param Video Height
 * @parent Video
 * @desc Set video size
 * Use 0 for default
 * @type text
 * @default 0
 *
 * @param Scene Button
 * @desc Add button to scene to link to option
 * @type struct<menuButton>
 *
 * @param Event Execution
 * @desc Event to run on selecting the option
 * Takes priority over code execution.
 * @type common_event
 * @default 0
 *
 * @param Code Execution
 * @desc Code to run on selecting the option
 * @type note
 * @default ""
 *
 * @param Require Actor Select
 * @desc If enabled, opens actor selection window
 * @type boolean
 * @default false
 *
 */
/*~struct~selcWindow:
 *
 * @param Dimension Configuration
 * @desc Setup position and width of the window
 * @type struct<locSize>
 * @default {"X":"0","Y":"0","Width":"1","Height":"1"}
 *
 * @param Window Font and Style Configuration
 * @desc Custom style the window
 * @type struct<windowStyle>
 * @default {"Font Settings":"","Font Size":"16","Font Face":"sans-serif","Base Font Color":"#ffffff","Font Outline Color":"rgba(0, 0, 0, 0.5)","Font Outline Thickness":"3","Window Skin":"Window","Window Opacity":"255","Show Window Dimmer":"false"}
 *
 * @param Item Width
 * @desc Size of the selection area
 * @type text
 * @default 1
 *
 * @param Item Height
 * @desc Size of the selection area
 * @type text
 * @default 1
 *
 * @param Max Columns
 * @desc Number of horizontal columns
 * @type text
 * @default 1
 *
 * @param Selection Options
 * @desc A list of options for the window
 * @type struct<selcOpt>[]
 * @default []
 *
 * @param Gauges
 * @desc Setup gauges for the window
 * @type struct<gaugeDraw>[]
 * @default []
 *
 * @param Draw Name
 * @desc Draw Option Name?
 * @type boolean
 * @default false
 *
 * @param Name Text
 * @parent Draw Name
 * @desc Text for name. %1 = Name.
 * @type text
 * @default %1
 *
 * @param Name X
 * @parent Draw Name
 * @desc Position of text
 * @type text
 * @default 0
 *
 * @param Name Y
 * @parent Draw Name
 * @desc Position of text
 * @type text
 * @default 0
 *
 * @param Draw Alternative Name
 * @desc Draw Option Alternative Name?
 * @type boolean
 * @default false
 *
 * @param Alternative Name Text
 * @parent Draw Alternative Name
 * @desc Text for name. %1 = Alternative Name.
 * @type text
 * @default %1
 *
 * @param Alternative Name X
 * @parent Draw Alternative Name
 * @desc Position of text
 * @type text
 * @default 0
 *
 * @param Alternative Name Y
 * @parent Draw Alternative Name
 * @desc Position of text
 * @type text
 * @default 0
 *
 * @param Draw Description
 * @desc Draw Option Description?
 * @type boolean
 * @default false
 *
 * @param Description X
 * @parent Draw Description
 * @desc Position of text
 * @type text
 * @default 0
 *
 * @param Description Y
 * @parent Draw Description
 * @desc Position of text
 * @type text
 * @default 0
 *
 * @param Draw Picture
 * @desc Draw Option Picture?
 * @type boolean
 * @default false
 *
 * @param Picture Index
 * @parent Draw Picture
 * @desc Index of picture to draw (start at zero (0))
 * @type text
 * @default $gameVariables.value(1)
 *
 * @param Picture X
 * @parent Draw Picture
 * @desc Position of picture
 * @type text
 * @default 0
 *
 * @param Picture Y
 * @parent Draw Picture
 * @desc Position of picture
 * @type text
 * @default 0
 *
 * @param Picture Width
 * @parent Draw Picture
 * @desc Size of picture
 * @type text
 * @default 0
 *
 * @param Picture Height
 * @parent Draw Picture
 * @desc Size of picture
 * @type text
 * @default 0
 *
 */
/*~struct~selcDataWindow:
 *
 * @param Name
 * @desc No function
 * @type text
 * @default Data Window
 *
 * @param Dimension Configuration
 * @desc Setup position and width of the window
 * @type struct<locSize>
 * @default {"X":"0","Y":"0","Width":"1","Height":"1"}
 *
 * @param Window Font and Style Configuration
 * @desc Custom style the window
 * @type struct<windowStyle>
 * @default {"Font Settings":"","Font Size":"16","Font Face":"sans-serif","Base Font Color":"#ffffff","Font Outline Color":"rgba(0, 0, 0, 0.5)","Font Outline Thickness":"3","Window Skin":"Window","Window Opacity":"255","Show Window Dimmer":"false"}
 *
 * @param Display Requirements
 * @desc Conditions to display the window
 * @type struct<displayRequirements>
 *
 * @param Gauges
 * @desc Setup gauges for the window
 * @type struct<gaugeDraw>[]
 * @default []
 *
 * @param Draw Option Name
 * @desc Draw Name of option?
 * @type boolean
 * @default false
 *
 * @param Name Text
 * @parent Draw Option Name
 * @desc Text for name. %1 = Name.
 * @type text
 * @default %1
 *
 * @param Name X
 * @parent Draw Option Name
 * @desc Position of name
 * @type text
 * @default 0
 *
 * @param Name Y
 * @parent Draw Option Name
 * @desc Position of name
 * @type text
 * @default 0
 *
 * @param Draw Alternative Option Name
 * @desc Draw Name of option?
 * @type boolean
 * @default false
 *
 * @param Alternative Name Text
 * @parent Draw Alternative Option Name
 * @desc Text for name. %1 = Alternative Name.
 * @type text
 * @default %1
 *
 * @param Alternative Name X
 * @parent Draw Alternative Option Name
 * @desc Position of name
 * @type text
 * @default 0
 *
 * @param Alternative Name Y
 * @parent Draw Alternative Option Name
 * @desc Position of name
 * @type text
 * @default 0
 *
 * @param Draw Description
 * @desc Draw Description of option?
 * @type boolean
 * @default false
 *
 * @param Description X
 * @parent Draw Description
 * @desc Position of name
 * @type text
 * @default 0
 *
 * @param Description Y
 * @parent Draw Description
 * @desc Position of name
 * @type text
 * @default 0
 *
 * @param Draw Picture
 * @desc Draw Description of option?
 * @type boolean
 * @default false
 *
 * @param Picture Index
 * @parent Draw Picture
 * @desc Index of picture used.
 * @type text
 * @default 1
 *
 * @param Picture X
 * @parent Draw Picture
 * @desc Position of picture
 * @type text
 * @default 0
 *
 * @param Picture Y
 * @parent Draw Picture
 * @desc Position of picture
 * @type text
 * @default 0
 *
 * @param Picture Width
 * @parent Draw Picture
 * @desc Size of picture
 * @type text
 * @default 0
 *
 * @param Picture Height
 * @parent Draw Picture
 * @desc Size of picture
 * @type text
 * @default 0
 *
 */
/*~struct~menuButton:
 *
 * @param Name
 * @desc No function
 * @type text
 * @default Button
 *
 * @param X
 * @desc Position on screen.
 * @type text
 * @default 0
 *
 * @param Y
 * @desc Position on screen.
 * @type text
 * @default 0
 *
 * @param Cold Graphic
 * @desc Graphic when not mouse over.
 * @type struct<animGfx>
 *
 * @param Hot Graphic
 * @desc Graphic when mouse over.
 * @type struct<animGfx>
 *
 */
/*~struct~custMenu:
 *
 * @param Identifier Name
 * @desc Name used to identify menu data.
 * @type text
 * @default menu
 *
 * @param Preload Backgrounds
 * @desc Picture images used for scene preload background.
 * @type struct<gfx>[]
 * @default []
 *
 * @param On Load Script Calls
 * @desc Script calls to run at start of menu scene
 * @type note[]
 * @default []
 *
 * @param Backgrounds
 * @desc Picture images used for scene background.
 * @type struct<staticGfx>[]
 * @default []
 *
 * @param Back Graphics
 * @desc Animated pictures layered just above scene background.
 * @type struct<animGfx>[]
 * @default []
 *
 * @param Selection Window
 * @desc Window used for selection options
 * @type struct<selcWindow>
 *
 * @param Open Effect
 * @parent Selection Window
 * @desc Selection window has an open effect at start
 * @type boolean
 * @default false
 *
 * @param Disable Cancel Exit
 * @parent Selection Window
 * @desc Prevents cancel button from exiting menu.
 * You will need to set exit option
 * @type boolean
 * @default false
 *
 * @param Update Codes
 * @desc The code here will run every frame in
 * sequence as they are listed.
 * @type note[]
 * @default []
 *
 * @param Actor Selection Window
 * @parent Selection Window
 * @desc Window draws all party members and allow selection
 * Hidden unless selection condition met
 * @type struct<actorSelcWindow>
 *
 * @param Always Show Actor Select
 * @parent Actor Selection Window
 * @desc Always show actor select on scene
 * @type boolean
 * @default false
 *
 * @param Actor Data Windows
 * @parent Actor Selection Window
 * @desc Window to display selected actor data
 * @type struct<actorDataWindow>[]
 * @default []
 *
 * @param Selection Data Windows
 * @parent Selection Window
 * @desc Display data based on window selection
 * @type struct<selcDataWindow>[]
 * @default []
 *
 * @param Basic Windows
 * @desc Windows to display basic data. (Always shown)
 * @type struct<basicWin>[]
 * @default []
 *
 * @param Fore Graphics
 * @desc Animated pictures layered just below scene foreground.
 * @type struct<animGfx>[]
 * @default []
 *
 * @param Foregrounds
 * @desc Picture images used for scene foreground.
 * @type struct<staticGfx>[]
 * @default []
 *
 */
/*:ja
 * @author Synrec/Kylestclr
 * @plugindesc v3.4 カスタムメニューの作成が可能
 * @target MZ
 * @url https://synrec.itch.io/
 *
 * @help
 * カスタムメニューを設定するには、プラグインパラメータを使用します。
 * メニュー識別子の名前がユニークであることを確認します。
 *
 * カスタムメニューシーン中に、デフォルトでは「F9」キー（またはプラグイン
 * パラメータで設定されたキー）を押すことで、ライブメニューエディタを開く
 * ことができます。これはライブエディタを開く唯一の方法です。
 * その他の方法でライブエディタを開こうとしても、動作しません。
 *
 * ライブエディタでの変更を保存するには、RPGツクールのプロジェクトエディタを
 * 閉じておく必要があります。プロジェクトエディタを開いたままだと、
 * メニューの変更は一時的なものとなります。
 *
 * ライブエディタでは、カスタムメニューシーンのリアルタイムカスタマイズ
 * およびシーンオーバーライドのリアルタイム設定が可能です。
 *
 * カスタムメニューの設定方法については、以下のYouTubeプレイリストで
 * 例を見ることができます：
 * https://www.youtube.com/playlist?list=PLLNw1pfxQDSluUMNnzdeEnlEMF-OYn0OE
 *
 * 一部のパラメータには、ゲーム内の変数やスイッチを代用することができます。
 * ただし、ライブエディタは現時点でこれらのRPGツクール特有の呼び出しを完全には
 * 処理できないため、数値が必要なパラメータには自動的に「0」が設定されます。
 *
 *
 * ライブエディタはHTML、CSS、JavaScriptで作成されており、外部のサーバーや
 * クライアントには一切接続しません。もし外部のサービスに接続しようとしている
 * のを発見した場合は、そのエディタを削除し、プラグインおよびソースの入手元の
 * 情報を添えて開発者に連絡してください。悪意のある改造版である可能性があります。
 *
 *
 * ライブメニューエディタは配布用プロジェクト向けには設計されていません。
 * したがって、配布予定のプロジェクトからは削除する必要があります。
 *
 * メニューは 「ネスト」（入れ子）することができ、さらに他のメニューに導くことができる。
 *
 * スクリプトコールを使用することで、メニュー上でビデオを再生することができます。
 * - this.playVideo(name, label, loop, x, y, width, height);
 * -- name = ビデオファイル名（拡張子なし）。
 * -- label = シーン内のビデオの識別子。
 * -- loop = 動画を連続的にループさせる
 * -- x = 動画の水平位置。
 * -- y = 動画の垂直位置。
 * -- width/height = 動画のサイズ。デフォルトでは0に設定する。
 *
 * - this.stopVideo(label)
 * -- label = シーン内の動画の識別子。
 *
 * シーンを呼び出すには、 SceneManager.push(Menu_Class_Name) を使用します。
 * ここで、"Menu_Class_Name"は以下のようになります：
 * - Scene_Item
 * - Scene_Skill
 * - Scene_Equip
 * - Scene_Status
 * - Scene_Options
 * - Scene_Save
 * - Scene_Load
 * - Scene_Debug
 *
 * "Menu_Class_Name」オプションは、rmmz_scenes.js（MZを使用している場合）
 * またはrpg_scenes.js（MVを使用している場合）から見ることができます。
 *
 * 便利なスクリプトコール:
 * > DataManager.menuBuilderGameReset()
 * - すべてのゲームオブジェクトをリセットし、新しいゲームとしてプロジェクトをセットアップします。
 *
 * > $gameTemp.openMenu("name")
 * - 識別子 "name" によって指定されたカスタムメニューシーンを開きます。
 *
 * > SceneManager._scene.closeMenu()
 * - カスタムメニューシーン内で使用され、メニューを正しく閉じます。
 * - $gameTemp.closeMenu() は使用しないでください。
 *
 * > $gameTemp.openedMenu()
 * - 現在開かれているメニューのデータを返します。
 *
 * @param Editor Access Button Name
 * @desc エディタを開くためのボタン名
 * Default: debug
 * @type text
 * @default debug
 *
 * @param Menu Configurations
 * @desc カスタムメニューの設定
 * @type struct<custMenu>[]
 * @default []
 *
 * @param Scene Overrides
 * @desc シーンクラスのオーバーライド
 * @type struct<sceneOverride>[]
 * @default []
 *
 */
/*~struct~sceneOverride:ja
 *
 * @param Name
 * @desc 機能なし
 * @type text
 * @default OVERRIDE
 *
 * @param Scene Class
 * @desc 上書きするシーンクラス
 * @type text
 * @default Scene_Menu
 *
 * @param Menu Identifier
 * @desc ロードするメニューの識別子
 * @type text
 * @default menu
 *
*/
/*~struct~staticGfx:ja
 *
 * @param File
 * @desc 画像ファイル
 * @type file
 * @dir img/pictures/
 *
 * @param X
 * @desc X位置設定
 * @type text
 * @default  0
 *
 * @param Y
 * @desc Y位置設定
 * @type text
 * @default  0
 *
 * @param Scrolling X
 * @desc 画像をX原点上で一貫して画像をスクロールする。
 * @type number
 * @default 0
 *
 * @param Scrolling Y
 * @desc 画像をY原点上で一貫して画像をスクロールする。
 * @type number
 * @default 0
 *
 * @param Anchor X
 * @desc 画像のXピボットポイントを変更する。
 * @type number
 * @decimals 3
 * @min -999999
 * @default 0.000
 *
 * @param Anchor Y
 * @desc 画像のYピボットポイントを変更する。
 * @type number
 * @decimals 3
 * @min -999999
 * @default 0.000
 *
 * @param Rotation
 * @desc 画像に回転をかける。
 * @type number
 * @decimals 3
 * @min -999999
 * @default 0.000
 *
 * @param Constant Rotation
 * @desc 回転の繰り返し
 * @type boolean
 * @default false
 *
 */
/*~struct~animGfx:ja
 *
 * @param File
 * @desc 画像ファイル
 * @type file
 * @dir img/pictures/
 *
 * @param X
 * @desc X位置設定
 * @type number
 * @default  0
 *
 * @param Y
 * @desc Y位置設定
 * @type number
 * @default  0
 *
 * @param Max Frames
 * @desc グラフィックが使用するフレーム数
 * @type number
 * @min 1
 * @default 1
 *
 * @param Frame Rate
 * @desc フレームの更新速度
 * @type number
 * @default 0
 *
 */
/*~struct~winText:ja
 *
 * @param Text
 * @desc 描くテキスト
 * @type note
 * @default ""
 *
 * @param X
 * @desc テキストのX開始位置
 * @type text
 * @default 0
 *
 * @param Y
 * @desc テキストのY開始位置
 * @type text
 * @default 0
 *
 */
/*~struct~gfx:ja
 *
 * @param Picture
 * @desc 使用する画像ファイル
 * @type file
 * @dir img/pictures/
 *
 * @param X
 * @desc 画像のX開始位置
 * @type text
 * @default 0
 *
 * @param Y
 * @desc 画像のY開始位置
 * @type text
 * @default 0
 *
 * @param Width
 * @desc 画像の幅
 * @type text
 * @default 1
 *
 * @param Height
 * @desc 画像の高さ
 * @type text
 * @default 1
 *
 */
/*~struct~locSize:ja
 *
 * @param X
 * @desc X位置設定
 * @type number
 * @default 0
 *
 * @param Y
 * @desc Y位置設定.
 * @type number
 * @default 0
 *
 * @param Width
 * @desc 幅の設定
 * @type number
 * @default 1
 *
 * @param Height
 * @desc 高さの設定
 * @type number
 * @default 1
 *
 */
/*~struct~windowStyle:ja
 *
 * @param Font Settings
 * @desc 子パラメーターの設定
 *
 * @param Font Size
 * @parent Font Settings
 * @desc フォントサイズ
 * @type number
 * @default 16
 *
 * @param Font Face
 * @parent Font Settings
 * @desc ウィンドウに使用されるフォント
 * @type text
 * @default sans-serif
 *
 * @param Base Font Color
 * @parent Font Settings
 * @desc ウィンドウのデフォルトフォントの色
 * @type text
 * @default #ffffff
 *
 * @param Font Outline Color
 * @parent Font Settings
 * @desc ウィンドウのデフォルトフォントの輪郭色
 * @type text
 * @default rgba(0, 0, 0, 0.5)
 *
 * @param Font Outline Thickness
 * @parent Font Settings
 * @desc テキストの輪郭の太さ
 * @type number
 * @default 3
 *
 * @param Window Skin
 * @desc ウィンドウスキンに使用される画像ファイル
 * @type file
 * @dir img/system/
 * @default Window
 *
 * @param Window Opacity
 * @desc 0 = 完全に透明、255 = 完全に不透明
 * @type number
 * @default 255
 *
 * @param Show Window Dimmer
 * @desc ウィンドウのスキンを隠す
 * @type boolean
 * @default false
 *
 */
/*~struct~displayRequirements:ja
 *
 * @param Game Switch
 * @desc このスイッチは ON（オン） にする必要があります。
 * @type switch
 * @default 0
 *
 * @param Game Variable
 * @desc 対象となる変数
 * @type variable
 * @default 0
 *
 * @param Variable Minimum
 * @parent Game Variable
 * @desc 変数の最小値
 * @type text
 * @default 0
 *
 * @param Variable Maximum
 * @parent Game Variable
 * @desc変数の最大値
 * @type text
 * @default 0
 *
 * @param Code
 * @desc コードの評価結果が「真（true）」でなければなりません。
 * @type note
 * @default ""
 *
 */
/*~struct~gaugeDraw:ja
 *
 * @param Label
 * @desc ゲージのラベルテキスト
 * @type text
 * @default gauge
 *
 * @param Label X
 * @desc ウィンドウ内のラベルテキストのX位置
 * @type text
 * @default 0
 *
 * @param Label Y
 * @desc ウィンドウ内のラベルテキストのY位置
 * @type text
 * @default 0
 *
 * @param Gauge Current Value
 * @desc ゲージ電流値の設定方法
 * Evaluated value.
 * @type text
 * @default
 *
 * @param Gauge Max Value
 * @descゲージの最大値の設定方法
 * Evaluated value.
 * @type text
 * @default
 *
 * @param Gauge X
 * @desc ウィンドウ内のゲージのX位置
 * @type text
 * @default 0
 *
 * @param Gauge Y
 * @desc ウィンドウ内のゲージのY位置
 * @type text
 * @default 0
 *
 * @param Gauge Width
 * @desc ゲージの幅
 * @type text
 * @default 1
 *
 * @param Gauge Height
 * @desc ゲージの高さ
 * @type text
 * @default 1
 * @default 1
 *
 * @param Gauge Border
 * @desc ゲージの枠の大きさ
 * @type text
 * @default 2
 *
 * @param Gauge Border Color
 * @desc ゲージの縁取りの色
 * @type text
 * @default #000000
 *
 * @param Gauge Background Color
 * @desc ゲージ背景の色
 * @type text
 * @default #666666
 *
 * @param Gauge Color
 * @desc ゲージ背景の色
 * @type text
 * @default #aaffaa
 *
 */
/*~struct~basicWin:ja
 *
 * @param Name
 * @desc 機能なし
 * @type text
 * @default Window
 *
 * @param Dimension Configuration
 * @desc ウィンドウの位置と幅の設定
 * @type struct<locSize>
 * @default {"X":"0","Y":"0","Width":"1","Height":"1"}
 *
 * @param Window Font and Style Configuration
 * @desc ウィンドウのカスタムスタイル
 * @type struct<windowStyle>
 * @default {"Font Settings":"","Font Size":"16","Font Face":"sans-serif","Base Font Color":"#ffffff","Font Outline Color":"rgba(0, 0, 0, 0.5)","Font Outline Thickness":"3","Window Skin":"Window","Window Opacity":"255","Show Window Dimmer":"false"}
 *
 * @param Display Requirements
 * @desc ウィンドウを表示するための条件
 * @type struct<displayRequirements>
 *
 * @param Draw Texts
 * @desc 様々なテキストを描画
 * @type struct<winText>[]
 * @default []
 *
 * @param Text References
 * @parent テキストを描画
 * @desc Set code references for draw text
 * %1 = first, %2 = second, etc...
 * @type text []
 * @default []
 *
 * @param Draw Pictures
 * @desc様々な絵を描画
 * @type struct<gfx>[]
 * @default []
 *
 * @param Draw Gauges
 * @desc ウィンドウにゲージを描画
 * @type struct<gaugeDraw>[]
 * @default []
 *
 */
/*~struct~gaugeDraw:ja
 *
 * @param Label
 * @desc ゲージのラベルテキスト
 * @type text
 * @default gauge
 *
 * @param Label X
 * @desc ウィンドウ内のラベルテキストのX位置
 * @type text
 * @default 0
 *
 * @param Label Y
 * @desc ウィンドウ内のラベルテキストのY位置
 * @type text
 * @default 0
 *
 * @param Gauge Current Value
 * @desc ゲージ電流値の設定方法
 * @type text
 * @default
 *
 * @param Gauge Max Value
 * @desc Hゲージ最大値の設定方法
 * @type text
 * @default
 *
 * @param Gauge X
 * @desc ウィンドウ内のゲージのX位置
 * @type text
 * @default 0
 *
 * @param Gauge Y
 * @desc ウィンドウ内のゲージのY位置
 * @type text
 * @default 0
 *
 * @param Gauge Width
 * @desc ゲージの幅
 * @type text
 * @default 1
 *
 * @param Gauge Height
 * @desc ゲージの高さ
 * @type text
 * @default 1
 * @default 1
 *
 * @param Gauge Border
 * @desc ゲージの枠の大きさ
 * @type text
 * @default 2
 *
 * @param Gauge Border Color
 * @desc ゲージの縁取りの色
 * @type text
 * @default #000000
 *
 * @param Gauge Background Color
 * @desc ゲージ背景の色
 * @type text
 * @default #666666
 *
 * @param Gauge Color
 * @desc ゲージ背景の色
 * @type text
 * @default #aaffaa
 *
 */
/*~struct~actorBaseParamWindow:ja
 *
 * @param Name
 * @desc 処理なし
 * @type text
 * @default ウィンドウ
 *
 * @param Base Param
 * @desc 描画に使用する基本パラメータ
 * @type select
 * @option mhp
 * @value 0
 * @option mmp
 * @value 1
 * @option atk
 * @value 2
 * @option def
 * @value 3
 * @option mat
 * @value 4
 * @option mdf
 * @value 5
 * @option agi
 * @value 6
 * @option luk
 * @value 7
 * @default 0
 *
 * @param Param Text
 * @desc パラメータの表示方法（テキストとして）
 * %1 = param value
 * @type text
 * @default %1
 *
 * @param X
 * @desc ウィンドウ内のX位置
 * @type text
 * @default 0
 *
 * @param Y
 * @desc ウィンドウ内のY位置
 * @type text
 * @default 0
 *
 */
/*~struct~actorExParamWindow:ja
 *
 * @param Name
 * @desc 処理なし
 * @type text
 * @default ウィンドウ
 *
 * @param Ex Param
 * @desc 描画する追加パラメータ（EXパラメータ）。パーセンテージに変換されます。
 * @type select
 * @option 命中率（Hit Rate）
 * @value 0
 * @option 回避率（Evasion Rate）
 * @value 1
 * @option クリティカル率（Critical Rate）
 * @value 2
 * @option クリティカル回避率（Critical Evasion Rate）
 * @value 3
 * @option 魔法回避率（Magic Evasion Rate）
 * @value 4
 * @option 魔法反射率（Magic Reflection Rate）
 * @value 5
 * @option 反撃率（Counter Attack Rate）
 * @value 6
 * @option HP再生率（HP Regeneration Rate）
 * @value 7
 * @option MP再生率（MP Regeneration Rate）
 * @value 8
 * @option TP再生率（TP Regeneration Rate）
 * @value 9
 * @default 0
 *
 * @param Param Text
 * @desc パラメータの表示方法（テキストとして）
 * %1 = param value
 * @type text
 * @default %1
 *
 * @param X
 * @desc ウィンドウ内のX位置
 * @type text
 * @default 0
 *
 * @param Y
 * @desc ウィンドウ内のY位置
 * @type text
 * @default 0
 *
 */
/*~struct~actorSPParamWindow:ja
 *
 * @param Name
 * @desc 処理なし
 * @type text
 * @default ウィンドウ
 *
 * @param Sp Param
 * @desc 描画する追加パラメータ（SPパラメータ）。パーセンテージに変換されます。
 * @type select
 * @option 狙われ率（Target Rate）
 * @value 0
 * @option 防御効果率（Guard Effect Rate）
 * @value 1
 * @option 回復効果率（Recovery Effect Rate）
 * @value 2
 * @option 薬の知識（Pharmacology）
 * @value 3
 * @option MP消費率（MP Cost Rate）
 * @value 4
 * @option TPチャージ率（TP Charge Rate）
 * @value 5
 * @option 物理ダメージ率（Physical Damage Rate）
 * @value 6
 * @option 魔法ダメージ率（Magical Damage Rate）
 * @value 7
 * @option 床ダメージ率（Floor Damage Rate）
 * @value 8
 * @option 経験値獲得率（Experience Rate）
 * @value 9
 * @default 0
 *
 * @param Param Text
 * @desc パラメータの表示方法（テキストとして）
 * %1 = param value
 * @type text
 * @default %1
 *
 * @param X
 * @desc ウィンドウ内のX位置
 * @type text
 * @default 0
 *
 * @param Y
 * @desc ウィンドウ内のY位置
 * @type text
 * @default 0
 *
 */
/*~struct~actorDataWindow:ja
 *
 * @param Name
 * @desc 機能なし
 * @type text
 * @default Window
 *
 * @param Dimension Configuration
 * @desc ウィンドウの位置と幅の設定
 * @type struct<locSize>
 * @default {"X":"0","Y":"0","Width":"1","Height":"1"}
*
 * @param Window Font and Style Configuration
 * @desc ウィンドウのカスタムスタイル
 * @type struct<windowStyle>
 * @default {"Font Settings":"","Font Size":"16","Font Face":"sans-serif","Base Font Color":"#ffffff","Font Outline Color":"rgba(0, 0, 0, 0.5)","Font Outline Thickness":"3","Window Skin":"Window","Window Opacity":"255","Show Window Dimmer":"false"}
 *
 * @param Display Requirements
 * @desc ウィンドウを表示するための条件
 * @type struct<displayRequirements>
 *
 * @param Gauges
 * @desc ウィンドウのゲージを設定する
 * @type struct<gaugeDraw>[]
 * @default []
 *
 * @param Draw Actor Name
 * @desc アクター名描画
 * @type boolean
 * @default false
 *
 * @param Name Text
 * @parent Draw Actor Name
 * @desc 名前に使用されるテキスト
 * %1 = Name, %2 = Nickname
 * @type text
 * @default %1
 *
 * @param Name X
 * @parent Draw Actor Name
 * @desc ウィンドウ内の名前のX位置
 * @type text
 * @default 0
 *
 * @param Name Y
 * @parent Draw Actor Name
 * @desc ウィンドウ内の名前のY位置
 * @type text
 * @default 0
 *
 * @param Draw Actor Profile
 * @desc アクタープロフィール描画
 * @type boolean
 * @default false
 *
 * @param Profile X
 * @parent Draw Actor Profile
 * @desc ウィンドウ内のプロフィールのX位置
 * @type text
 * @default 0
 *
 * @param Profile Y
 * @parent Draw Actor Profile
 * @desc ウィンドウ内のプロフィールのY位置
 * @type text
 * @default 0
 *
 * @param Draw Class Level
 * @desc アクターのクラス名とレベル描画
 * @type boolean
 * @default false
 *　
 * @param Class Level Text
 * @parent Draw Class Level
 * @desc アクターのクラス名とレベルを描画
 * %1 = class name, %2 = level
 * @type text
 * @default Class: %1 <%2>
 *
 * @param Class Level X
 * @parent Draw Class Level
 * @desc ウィンドウ内のクラスレベルのX位置
 * @type text
 * @default 0
 *
 * @param Class Level Y
 * @parent Draw Class Level
 * @desc ウィンドウ内のクラスレベルのY位置
 * @type text
 * @default 0
 *
 * @param Draw HP Resource
 * @desc アクターの現在HPと最大HPを描画
 * @type boolean
 * @default false
 *
 * @param HP Text
 * @parent Draw HP Resource
 * @desc HP用テキスト（エスケープ文字可）
 * %1 = Current, %2 = Max
 * @type text
 * @default \I[84]%1 / %2
 *
 * @param HP X
 * @parent Draw HP Resource
 * @desc ウィンドウ内のHPテキストのX位置
 * @type text
 * @default 0
 *
 * @param HP Y
 * @parent Draw HP Resource
 * @desc ウィンドウ内のHPテキストのY位置
 * @type text
 * @default 0
 *
 * @param Draw MP Resource
 * @desc アクターの現在MPと最大MPを描画
 * @type boolean
 * @default false
 *
 * @param MP Text
 * @parent Draw MP Resource
 * @desc MP用テキスト（エスケープ文字可）
 * %1 = Current, %2 = Max
 * @type text
 * @default \I[79]%1 / %2
 *
 * @param MP X
 * @parent Draw MP Resource
 * @desc ウィンドウ内のMPテキストのX位置
 * @type text
 * @default 0
 *
 * @param MP Y
 * @parent Draw MP Resource
 * @desc ウィンドウ内のMPテキストのY位置
 * @type text
 * @default 0
 *
 * @param Draw TP Resource
 * @desc アクターの現在TPと最大TPを描画
 * @type boolean
 * @default false
 *
 * @param TP Text
 * @parent Draw TP Resource
 * @desc TP用テキスト（エスケープ文字可）
 * %1 = Current, %2 = Max
 * @type text
 * @default \I[79]%1 / %2
 *
 * @param TP X
 * @parent Draw TP Resource
 * @desc ウィンドウ内のTPテキストのX位置
 * @type text
 * @default 0
 *
 * @param TP Y
 * @parent Draw TP Resource
 * @desc ウィンドウ内のTPテキストのY位置
 * @type text
 * @default 0
 *
 * @param Draw Base Params
 * @desc アクターの基本パラメータを描画
 * @type struct<actorBaseParamWindow>[]
 * @default []
 *
 * @param Draw Ex Params
 * @desc アクターの追加パラメータを描画
 * @type struct<actorExParamWindow>[]
 * @default []
 *
 * @param Draw Sp Params
 * @desc アクターの特殊パラメータを描画
 * @type struct<actorSpParamWindow>[]
 * @default []
 *
 * @param Display Map Character
 * @desc アクターの歩行グラフィックを表示
 * @type boolean
 * @default false
 *
 * @param Character Direction
 * @parent Display Map Character
 * @desc キャラクターの顔の向き
 * @type select
 * @option down
 * @value 2
 * @option left
 * @value 4
 * @option right
 * @value 6
 * @option up
 * @value 8
 * @default 2
 *
 * @param Character X
 * @parent Display Map Character
 * @desc ウィンドウに対する歩行グラフィックのX相対位置
 * @type text
 * @default 0
 *
 * @param Character Y
 * @parent Display Map Character
 * @desc ウィンドウに対する歩行グラフィックのY相対位置
 * @type text
 * @default 0
 *
 * @param Character Scale X
 * @parent Display Map Character
 * @desc 歩行グラフィックのXサイズ
 * @type text
 * @default 1
 *
 * @param Character Scale Y
 * @parent Display Map Character
 * @desc 歩行グラフィックのYサイズ
 * @type text
 * @default 1
 *
 * @param Display Battler
 * @desc アクターのバトラー表示
 * @type boolean
 * @default false
 *
 * @param Battler Motion
 * @parent Display Battler
 * @desc バトラーのリフレッシュ動議
 * @type select
 * @option walk
 * @option wait
 * @option chant
 * @option guard
 * @option damage
 * @option evade
 * @option thrust
 * @option swing
 * @option missile
 * @option skill
 * @option spell
 * @option item
 * @option escape
 * @option victory
 * @option dying
 * @option abnormal
 * @option sleep
 * @option dead
 * @default wait
 *
 * @param Battler X
 * @parent Display Battler
 * @desc ウィンドウに対するバトラーのX相対位置
 * @type text
 * @default 0
 *
 * @param Battler Y
 * @parent Display Battler
 * @desc ウィンドウに対するバトラーのY相対位置
 * @type text
 * @default 0
 *
 * @param Battler Scale X
 * @parent Display Battler
 * @desc バトラーのXサイズ
 * @type text
 * @default 1
 *
 * @param Battler Scale Y
 * @parent Display Battler
 * @desc バトラーのYサイズ
 * @type text
 * @default 1
 *
 */
/*~struct~actorSelcWindow:ja
 *
 * @param Name
 * @desc 機能なし
 * @type text
 * @default Window
 *
 * @param Dimension Configuration
 * @desc ウィンドウの位置と幅の設定
 * @type struct<locSize>
 * @default {"X":"0","Y":"0","Width":"1","Height":"1"}
 *
 * @param Window Font and Style Configuration
 * @desc ウィンドウのカスタムスタイル
 * @type struct<windowStyle>
 * @default {"Font Settings":"","Font Size":"16","Font Face":"sans-serif","Base Font Color":"#ffffff","Font Outline Color":"rgba(0, 0, 0, 0.5)","Font Outline Thickness":"3","Window Skin":"Window","Window Opacity":"255","Show Window Dimmer":"false"}
 *
 * @param Max Columns
 * @desc ウィンドウが使用する最大列数
 * @type number
 * @default 1
 *
 * @param Item Width
 * @desc ウィンドウ項目の最大幅。0 = デフォルト
 * @type number
 * @default 0
 *
 * @param Item Height
 * @desc ウィンドウアイテムの高さの最大値。0 = デフォルト
 * @type number
 * @default 0
 *
 * @param Gauges
 * @desc ウィンドウのゲージを設定
 * @type struct<gaugeDraw>[]
 * @default []
 *
 * @param Draw Actor Name
 * @desc アクター名を描画
 * @type boolean
 * @default false
 *
 * @param Name Text
 * @parent Draw Actor Name
 * @desc 名前に使用されるテキスト
 * %1 = Name, %2 = Nickname
 * @type text
 * @default %1
 *
 * @param Name X
 * @parent Draw Actor Name
 * @desc ウィンドウ内の名前のX位置
 * @type text
 * @default 0
 *
 * @param Name Y
 * @parent Draw Actor Name
 * @desc ウィンドウ内の名前のY位置
 * @type text
 * @default 0
 *
 * @param Draw Actor Profile
 * @desc アクターのプロフィールを描画
 * @type boolean
 * @default false
 *
 * @param Profile X
 * @parent Draw Actor Profile
 * @desc ウィンドウ内のプロフィールのX位置
 * @type text
 * @default 0
 *
 * @param Profile Y
 * @parent Draw Actor Profile
 * @desc ウィンドウ内のプロフィールのX位置
 * @type text
 * @default 0
 *
 * @param Draw Class Level
 * @desc アクターのクラス名とレベルを描画
 * @type boolean
 * @default false
 *
 * @param Class Level Text
 * @parent Draw Class Level
 * @desc クラス名とレベルを描画
 * %1 = class name, %2 = level
 * @type text
 * @default Class: %1 <%2>
 *
 * @param Class Level X
 * @parent Draw Class Level
 * @desc ウィンドウ内のクラスレベルのX位置
 * @type text
 * @default 0
 *
 * @param Class Level Y
 * @parent Draw Class Level
 * @desc ウィンドウ内のクラスレベルのY位置
 * @type text
 * @default 0
 *
 * @param Draw HP Resource
 * @desc アクターの現在HPと最大HPを描画
 * @type boolean
 * @default false
 *
 * @param HP Text
 * @parent Draw HP Resource
 * @desc HP用テキスト（エスケープ文字可）
 * %1 = Current, %2 = Max
 * @type text
 * @default \I[84]%1 / %2
 *
 * @param HP X
 * @parent Draw HP Resource
 * @desc ウィンドウ内のテキストのX位置
 * @type text
 * @default 0
 *
 * @param HP Y
 * @parent Draw HP Resource
 * @desc ウィンドウ内のテキストのY位置
 * @type text
 * @default 0
 *
 * @param Draw MP Resource
 * @desc アクターの現在MPと最大MPを描画
 * @type boolean
 * @default false
 *
 * @param MP Text
 * @parent Draw MP Resource
 * @desc MP用テキスト（エスケープ文字可）
 * %1 = Current, %2 = Max
 * @type text
 * @default \I[79]%1 / %2
 *
 * @param MP X
 * @parent Draw MP Resource
 * @desc ウィンドウ内のテキストのX位置
 * @type text
 * @default 0
 *
 * @param MP Y
 * @parent Draw MP Resource
 * @desc ウィンドウ内のテキストのY位置
 * @type text
 * @default 0
 *
 * @param Draw TP Resource
 * @desc アクターの現在TPと最大TPを描画
 * @type boolean
 * @default false
 *
 * @param TP Text
 * @parent Draw TP Resource
 * @desc TP用テキスト（エスケープ文字可）
 * %1 = Current, %2 = Max
 * @type text
 * @default \I[79]%1 / %2
 *
 * @param TP X
 * @parent Draw TP Resource
 * @desc ウィンドウ内のテキストのX位置
 * @type text
 * @default 0
 *
 * @param TP Y
 * @parent Draw TP Resource
 * @desc ウィンドウ内のテキストのY位置
 * @type text
 * @default 0
 *
 * @param Draw Base Params
 * @desc アクターの基本パラメータを描画
 * @type struct<actorBaseParamWindow>[]
 * @default []
 *
 * @param Draw Ex Params
 * @desc アクターの追加パラメータを描画
 * @type struct<actorExParamWindow>[]
 * @default []
 *
 * @param Draw Sp Params
 * @desc アクターの特殊パラメータを描画
 * @type struct<actorSpParamWindow>[]
 * @default []
 *
 * @param Display Map Character
 * @desc アクターの歩行グラフィックを表示
 * @type boolean
 * @default false
 *
 * @param Character Direction
 * @parent Display Map Character
 * @desc キャラクターの顔の向き
 * @type select
 * @option down
 * @value 2
 * @option left
 * @value 4
 * @option right
 * @value 6
 * @option up
 * @value 8
 * @default 2
 *
 * @param Character X
 * @parent Display Map Character
 * @desc ウィンドウに対するX相対位置
 * @type text
 * @default 0
 *
 * @param Character Y
 * @parent Display Map Character
 * @desc ウィンドウに対するY相対位置
 * @type text
 * @default 0
 *
 * @param Character Scale X
 * @parent Display Map Character
 * @desc キャラクターのXサイズ
 * @type text
 * @default 1
 *
 * @param Character Scale Y
 * @parent Display Map Character
 * @desc キャラクターのYサイズ
 * @type text
 * @default 1
 *
 * @param Display Battler
 * @desc アクターのバトラーを描画
 * @type boolean
 * @default false
 *
 * @param Battler Motion
 * @parent Display Battler
 * @desc  バトラーのリフレッシュ動議
 * @type select
 * @option walk
 * @option wait
 * @option chant
 * @option guard
 * @option damage
 * @option evade
 * @option thrust
 * @option swing
 * @option missile
 * @option skill
 * @option spell
 * @option item
 * @option escape
 * @option victory
 * @option dying
 * @option abnormal
 * @option sleep
 * @option dead
 * @default wait
 *
 * @param Battler X
 * @parent Display Battler
 * @desc ウィンドウからのX相対位置
 * @type text
 * @default 0
 *
 * @param Battler Y
 * @parent Display Battler
 * @desc ウィンドウからのY相対位置
 * @type text
 * @default 0
 *
 * @param Battler Scale X
 * @parent Display Battler
 * @desc バトラーのXサイズ
 * @type text
 * @default 1
 *
 * @param Battler Scale Y
 * @parent Display Battler
 * @desc バトラーのYサイズ
 * @type text
 * @default 1
 *
*/
/*~struct~stockVarReq:ja
 *
 * @param Name
 * @desc 機能なし
 * @type text
 * @default Variable
 *
 * @param Variable
 * @desc ゲーム変数
 * @type variable
 * @default 0
 *
 * @param Min Value
 * @desc 変数の最小値
 * @type text
 * @default 0
 *
 * @param Max Value
 * @desc 変数の最大値
 * @type text
 * @default 0
 *
 */
/*~struct~stockItmReq:ja
 *
 * @param Name
 * @desc 機能なし
 * @type text
 * @default Variable
 *
 * @param Item
 * @desc ゲームアイテム
 * @type item
 * @default 0
 *
 * @param Min Value
 * @desc アイテムの最小量
 * @type text
 * @default 0
 *
 * @param Max Value
 * @desc アイテムの最大量
 * @type text
 * @default 0
 *
 */
/*~struct~stockWepReq:ja
 *
 * @param Name
 * @desc 機能なし
 * @type text
 * @default Variable
 *
 * @param Weapon
 * @desc ゲームの武器
 * @type weapon
 * @default 0
 *
 * @param Min Value
 * @desc 武器の最小量
 * @type text
 * @default 0
 *
 * @param Max Value
 * @desc 武器の最大量
 * @type text
 * @default 0
 *
 */
/*~struct~stockArmReq:ja
 *
 * @param Name
 * @desc 機能なし
 * @type text
 * @default Variable
 *
 * @param Armor
 * @desc ゲームの鎧
 * @type armor
 * @default 0
 *
 * @param Min Value
 * @desc 鎧の最小値
 * @type text
 * @default 0
 *
 * @param Max Value
 * @desc 鎧の最大値
 * @type text
 * @default 0
 *
 */
/*~struct~optReq:ja
 *
 * @param Variable Requirements
 * @desc 複数の可変要件を設定する
 * @type struct<stockVarReq>[]
 * @default []
 *
 * @param Switch Requirements
 * @desc 記載されているスイッチは有効/ONでなければならない。
 * @type switch[]
 * @default []
 *
 * @param Item Requirements
 * @desc 複数のアイテムの必要要件を設定します。
 * @type struct<stockItmReq>[]
 * @default []
 *
 * @param Weapon Requirements
 * @desc 複数の武器の必要要件を設定します。
 * @type struct<stockWepReq>[]
 * @default []
 *
 * @param Armor Requirements
 * @desc 複数の鎧の必要要件を設定します。
 * @type struct<stockArmReq>[]
 * @default []
 *
 */
/*~struct~selcOpt:ja
 *
 * @param Name
 * @desc オプション名
 * @type text
 * @default Option
 *
 * @param Alternative Name
 * @desc 表示用のオプションの別名
 * @type text
 * @default Alternative Name
 *
 * @param Display Requirements
 * @desc メニューにオプションを表示するための条件
 * @type struct<optReq>
 *
 * @param Select Requirements
 * @desc メニューにオプションを表示するための条件
 * @type struct<optReq>
 *
 * @param Description
 * @desc セレクションの説明方法
 * @type note
 * @default ""
 *
 * @param Pictures
 * @desc セレクションを表す画像
 * @type file[]
 * @dir img/pictures/
 * @default []
 *
 * @param Static Graphic
 * @desc 画像は背景の上にレイヤーされ、バックグラフィックのすぐ下にある。
 * @type struct<staticGfx>
 *
 * @param Animated Graphic
 * @desc 選択を表すアニメーショングラフィック
 * @type struct<animGfx>
 *
 * @param Video
 * @desc 選択した動画を再生する
 * Video is played below video layer
 * @type text
 *
 * @param Video X
 * @parent Video
 * @desc 動画の座標X
 * @type text
 * @default 0
 *
 * @param Video Y
 * @parent Video
 * @desc 動画の座標Y
 * @type text
 * @default 0
 *
 * @param Video Width
 * @parent Video
 * @desc 動画サイズの設定 幅
 * Use 0 for default
 * @type text
 * @default 0
 *
 * @param Video Height
 * @parent Video
 * @desc 動画サイズの設定 高さ
 * Use 0 for default
 * @type text
 * @default 0
 *
 * @param Scene Button
 * @descオプションにリンクするボタンをシーンに追加する
 * @type struct<menuButton>
 *
 * @param Event Execution
 * @desc オプションを選択すると実行されるイベント
 * Takes priority over code execution.
 * @type common_event
 * @default 0
 *
 * @param Code Execution
 * @desc オプション選択時に実行されるコード
 * @type note
 * @default ""
 *
 * @param Require Actor Select
 * @desc 有効な場合、アクター選択ウィンドウを開く
 * @type boolean
 * @default false
 *
 */
/*~struct~selcWindow:ja
 *
 * @param Dimension Configuration
 * @desc ウィンドウの位置と幅の設定
 * @type struct<locSize>
 * @default {"X":"0","Y":"0","Width":"1","Height":"1"}
 *
 * @param Window Font and Style Configuration
 * @desc ウィンドウのカスタムスタイル
 * @type struct<windowStyle>
 * @default {"Font Settings":"","Font Size":"16","Font Face":"sans-serif","Base Font Color":"#ffffff","Font Outline Color":"rgba(0, 0, 0, 0.5)","Font Outline Thickness":"3","Window Skin":"Window","Window Opacity":"255","Show Window Dimmer":"false"}
 *
 * @param Item Width
 * @desc 選択領域の幅
 * @type text
 * @default 1
 *
 * @param Item Height
 * @desc 選択領域の高さ
 * @type text
 * @default 1
 *
 * @param Max Columns
 * @desc 横列の数
 * @type text
 * @default 1
 *
 * @param Selection Options
 * @desc ウィンドウのオプションのリスト
 * @type struct<selcOpt>[]
 * @default []
 *
 * @param Draw Gauges
 * @descウィンドウのゲージを設定する
 * @type struct<gaugeDraw>[]
 * @default []
 *
 * @param Draw Name
 * @desc ドローオプション名は？
 * @type boolean
 * @default false
 *
 * @param Name Text
 * @parent Draw Name
 * @desc 名前のテキスト。%1 = 名前。
 * @type text
 * @default %1
 *
 * @param Name X
 * @parent Draw Name
 * @desc テキストのX位置
 * @type text
 * @default 0
 *
 * @param Name Y
 * @parent Draw Name
 * @desc テキストのY位置
 * @type text
 * @default 0
 *
 * @param Draw Alternative Name
 * @desc ドローオプションの別名は？
 * @type boolean
 * @default false
 *
 * @param Alternative Name Text
 * @parent Draw Alternative Name
 * @desc 代替名のテキスト。%1 = 代替名。
 * @type text
 * @default %1
 *
 * @param Alternative Name X
 * @parent Draw Alternative Name
 * @desc テキストのX位置
 * @type text
 * @default 0
 *
 * @param Alternative Name Y
 * @parent Draw Alternative Name
 * @desc テキストのY位置
 * @type text
 * @default 0
 *
 * @param Draw Description
 * @desc ドローオプションの説明
 * @type boolean
 * @default false
 *
 * @param Description X
 * @parent Draw Description
 * @desc テキストのX位置
 * @type text
 * @default 0
 *
 * @param Description Y
 * @parent Draw Description
 * @desc テキストのY位置
 * @type text
 * @default 0
 *
 * @param Draw Picture
 * @desc オプション画像を描く？
 * @type boolean
 * @default false
 *
 * @param Picture Index
 * @parent Draw Picture
 * @desc 描画する画像のインデックス（0から始まる）
 * @type text
 * @default $gameVariables.value(1)
 *
 * @param Picture X
 * @parent Draw Picture
 * @desc 画像のX位置
 * @type text
 * @default 0
 *
 * @param Picture Y
 * @parent Draw Picture
 * @desc 画像のY位置
 * @type text
 * @default 0
 *
 * @param Picture Width
 * @parent Draw Picture
 * @desc 画像の幅
 * @type text
 * @default 0
 *
 * @param Picture Height
 * @parent Draw Picture
 * @desc 画像の高さ
 * @type text
 * @default 0
 *
 */
/*~struct~selcDataWindow:ja
 *
 * @param Name
 * @desc 機能なし
 * @type text
 * @default Data Window
 *
 * @param Dimension Configuration
 * @desc ウィンドウの位置と幅の設定
 * @type struct<locSize>
 * @default {"X":"0","Y":"0","Width":"1","Height":"1"}
 *
 * @param Window Font and Style Configuration
 * @desc ウィンドウのカスタムスタイル
 * @type struct<windowStyle>
 * @default {"Font Settings":"","Font Size":"16","Font Face":"sans-serif","Base Font Color":"#ffffff","Font Outline Color":"rgba(0, 0, 0, 0.5)","Font Outline Thickness":"3","Window Skin":"Window","Window Opacity":"255","Show Window Dimmer":"false"}
 *
 * @param Display Requirements
 * @desc ウィンドウを表示するための条件
 * @type struct<displayRequirements>
 *
 * @param Gauges
 * @desc ウィンドウのゲージを設定する
 * @type struct<gaugeDraw>[]
 * @default []
 *
 * @param Draw Option Name
 * @desc オプション名は？
 * @type boolean
 * @default false
 *
 * @param Name Text
 * @parent Draw Option Name
 * @desc 名前のテキスト。%1 = 名前。
 * @type text
 * @default %1
 *
 * @param Name X
 * @parent Draw Option Name
 * @desc 名前のX位置
 * @type text
 * @default 0
 *
 * @param Name Y
 * @parent Draw Option Name
 * @desc 名前のY位置
 * @type text
 * @default 0
 *
 * @param Draw Alternative Option Name
 * @desc オプションの代替名は？
 * @type boolean
 * @default false
 *
 * @param Alternative Name Text
 * @parent Draw Alternative Option Name
 * @desc 名前のテキスト。%1 = 代替名。
 * @type text
 * @default %1
 *
 * @param Alternative Name X
 * @parent Draw Alternative Option Name
 * @desc 代替名のX位置
 * @type text
 * @default 0
 *
 * @param Alternative Name Y
 * @parent Draw Alternative Option Name
 * @desc 代替名のY位置
 * @type text
 * @default 0
 *
 * @param Draw Description
 * @desc オプションの説明は？
 * @type boolean
 * @default false
 *
 * @param Description X
 * @parent Draw Description
 * @desc 説明のX位置
 * @type text
 * @default 0
 *
 * @param Description Y
 * @parent Draw Description
 * @desc 説明のY位置
 * @type text
 * @default 0
 *
 * @param Draw Picture
 * @desc オプションの説明を描く？
 * @type boolean
 * @default false
 *
 * @param Picture Index
 * @parent Draw Picture
 * @desc 使用画像のインデックス
 * @type text
 * @default 1
 *
 * @param Picture X
 * @parent Draw Picture
 * @desc 画像のX位置
 * @type text
 * @default 0
 *
 * @param Picture Y
 * @parent Draw Picture
 * @desc 画像のY位置
 * @type text
 * @default 0
 *
 * @param Picture Width
 * @parent Draw Picture
 * @desc 画像の幅
 * @type text
 * @default 0
 *
 * @param Picture Height
 * @parent Draw Picture
 * @desc 画像の高さ
 * @type text
 * @default 0
 *
 */
/*~struct~menuButton:ja
 *
 * @param Name
 * @desc 機能なし
 * @type text
 * @default Button
 *
 * @param X
 * @desc 画面上のX位置
 * @type text
 * @default 0
 *
 * @param Y
 * @desc 画面上のY位置
 * @type text
 * @default 0
 *
 * @param Cold Graphic
 * @desc マウスオーバーしていない時のグラフィック
 * @type struct<animGfx>
 *
 * @param Hot Graphic
 * @desc マウスオーバー時のグラフィック
 * @type struct<animGfx>
 *
 */
/*~struct~custMenu:ja
 *
 * @param Identifier Name
 * @desc メニューデータを識別するための名前。
 * @type text
 * @default menu
 *
 * @param Preload Backgrounds
 * @desc シーンのプリロードの背景に使用される画像。
 * @type struct<gfx>[]
 * @default []
 *
 * @param On Load Script Calls
 * @desc メニューシーンの開始時に実行されるスクリプト呼び出し
 * @type note[]
 * @default []
 *
 * @param Backgrounds
 * @desc シーンの背景に使用される画像。
 * @type struct<staticGfx>[]
 * @default []
 *
 * @param Back Graphics
 * @desc シーンの背景のすぐ上にレイヤーされたアニメーション画像。
 * @type struct<animGfx>[]
 * @default []
 *
 * @param Selection Window
 * @desc 選択オプションに使用されるウィンドウ
 * @type struct<selcWindow>
 *
 * @param Open Effect
 * @parent Selection Window
 * @desc 選択ウィンドウは開始時にオープンエフェクトがあります
 * @type boolean
 * @default false
 *
 * @param Disable Cancel Exit
 * @parent Selection Window
 * @desc キャンセルボタンによってメニューが終了しないようにします。
 * 終了させるには、終了オプションを設定する必要があります。
 * @type boolean
 * @default false
 *
 * @param Update Codes
 * @desc ここに記述されたコードは、一覧にある順番で
 * 毎フレーム実行されます。
 * @type note[]
 * @default []
 *
 * @param Actor Selection Window
 * @parent Selection Window
 * @desc このウィンドウはすべてのパーティメンバーを描画し、選択を可能にします。
 * 選択条件が満たされない限り非表示になります。
 * @type struct<actorSelcWindow>
 *
 * @param Always Show Actor Select
 * @parent Actor Selection Window
 * @desc 常にアクターセレクトシーンを表示
 * @type boolean
 * @default false
 *
 * @param Actor Data Windows
 * @parent Actor Selection Window
 * @desc 選択したアクターのデータを表示するウィンドウ
 * @type struct<actorDataWindow>[]
 * @default []
 *
 * @param Selection Data Windows
 * @parent Selection Window
 * @desc ウィンドウの選択に基づいてデータを表示する
 * @type struct<selcDataWindow>[]
 * @default []
 *
 * @param Basic Windows
 * @desc 基本データを表示するウィンドウ。(常時表示）
 * @type struct<basicWin>[]
 * @default []
 *
 * @param Fore Graphics
 * @desc シーンの前景のすぐ下にレイヤーされたアニメーション画像
 * @type struct<animGfx>[]
 * @default []
 *
 * @param Foregrounds
 * @desc シーンの前景に使用される画像。
 * @type struct<staticGfx>[]
 * @default []
 *
 */


const Syn_MenuBuildr = {};

function LOAD_MENU_BUILDER(){
    Syn_MenuBuildr.Plugin = PluginManager.parameters(`Synrec_MenuBuilder`);
    Syn_MenuBuildr.EDITOR_ACCESS_BUTTON = Syn_MenuBuildr.Plugin['Editor Access Button Name'] || 'debug';

    function WINDOW_TEXT_PARSER_MNUBLD(obj){
        try{
            obj = JSON.parse(obj);
            try{
                obj['Text'] = JSON.parse(obj['Text']);
            }catch(e){
                obj['Text'] = "";
            }
            return obj
        }catch(e){
            return;
        }
    }

    function GRAPHIC_PARSER_MNUBLD(obj){
        try{
            obj = JSON.parse(obj);
            return obj
        }catch(e){
            return;
        }
    }

    function STATIC_GRAPHIC_PARSER_MNUBLD(obj){
        try{
            obj = JSON.parse(obj);
            return obj
        }catch(e){
            return;
        }
    }

    function ANIMATED_GRAPHIC_PARSER_MNUBLD(obj){
        try{
            obj = JSON.parse(obj);
            return obj
        }catch(e){
            return;
        }
    }

    function DIMENSION_CONFIGURATION_PARSER_MNUBLD(obj){
        try{
            obj = JSON.parse(obj);
            obj['X'] = eval(obj['X']);
            obj['Y'] = eval(obj['Y']);
            obj['Width'] = eval(obj['Width']);
            obj['Height'] = eval(obj['Height']);
        }catch(e){
            console.warn(`Failed to parse dimension configuration. ${e}`);
            const obj = {};
            obj['X'] = 0;
            obj['Y'] = 0;
            obj['Width'] = 1;
            obj['Height'] = 1;
        }
        return obj;
    }

    function WINDOW_STYLE_PARSER_MNUBLD(obj){
        try{
            obj = JSON.parse(obj)
            obj['Font Size'] = eval(obj['Font Size']);
            obj['Font Outline Thickness'] = eval(obj['Font Outline Thickness']);
            obj['Window Opacity'] = eval(obj['Window Opacity']);
            obj['Show Window Dimmer'] = eval(obj['Show Window Dimmer']);
        }catch(e){
            console.warn(`Failed to parse window style. ${e}`);
            const obj = {};
            obj['Font Size'] = 16;
            obj['Font Face'] = 'sans-serif';
            obj['Base Font Color'] = '#ffffff';
            obj['Font Outline Color'] = 'rgba(0, 0, 0, 0.5)';
            obj['Font Outline Thickness'] = 3;
            obj['Window Skin'] = 'Window';
            obj['Window Opacity'] = 255;
            obj['Show Window Dimmer'] = false;
        }
        return obj;
    }

    function WINDOW_DISPLAY_REQUIREMENTS_PARSER_MNUBLD(obj){
        try{
            obj = JSON.parse(obj);
            try{
                obj['Code'] = JSON.parse(obj['Code']);
            }catch(e){
                obj['Code'] = "";
            }
            return obj;
        }catch(e){
            return;
        }
    }

    function GAUGE_DRAW_PARSER_MNUBLD(obj){
        try{
            obj = JSON.parse(obj);
            return obj;
        }catch(e){
            return;
        }
    }

    function BASIC_WINDOW_PARSER_MNUBLD(obj){
        try{
            obj = JSON.parse(obj);
            obj['Dimension Configuration'] = DIMENSION_CONFIGURATION_PARSER_MNUBLD(obj['Dimension Configuration']);
            obj['Window Font and Style Configuration'] = WINDOW_STYLE_PARSER_MNUBLD(obj['Window Font and Style Configuration']);
            try{
                const texts = JSON.parse(obj['Draw Texts']).map((text_config)=>{
                    return WINDOW_TEXT_PARSER_MNUBLD(text_config);
                }).filter(Boolean);
                obj['Draw Texts'] = texts;
            }catch(e){
                obj['Draw Texts'] = [];
            }
            try{
                const codes = JSON.parse(obj['Text References'])
                obj['Text References'] = codes;
            }catch(e){
                obj['Text References'] = [];
            }
            try{
                const pictures = JSON.parse(obj['Draw Pictures']).map((pic_config)=>{
                    return GRAPHIC_PARSER_MNUBLD(pic_config);
                }).filter(Boolean);
                obj['Draw Pictures'] = pictures;
            }catch(e){
                obj['Draw Pictures'] = [];
            }
            try{
                let i = 0;
                obj['Gauges'] = JSON.parse(obj['Gauges']).map((gauge_draw_config)=>{
                    const data = GAUGE_DRAW_PARSER_MNUBLD(gauge_draw_config);
                    if(data){
                        data['ID'] = i;
                        i++;
                        return data;
                    }
                }).filter(Boolean)
            }catch(e){
                obj['Gauges'] = [];
            }
            return obj;
        }catch(e){
            return;
        }
    }

    function ACTOR_BASE_PARAM_WINDOW_PARSER_MNUBLD(obj){
        try{
            obj = JSON.parse(obj);
            return obj;
        }catch(e){
            return;
        }
    }

    function ACTOR_EX_PARAM_WINDOW_PARSER_MNUBLD(obj){
        try{
            obj = JSON.parse(obj);
            return obj;
        }catch(e){
            return;
        }
    }

    function ACTOR_SP_PARAM_WINDOW_PARSER_MNUBLD(obj){
        try{
            obj = JSON.parse(obj);
            return obj;
        }catch(e){
            return;
        }
    }

    function ACTOR_DATA_WINDOW_PARSER_MNUBLD(obj){
        try{
            obj = JSON.parse(obj);
            obj['Dimension Configuration'] = DIMENSION_CONFIGURATION_PARSER_MNUBLD(obj['Dimension Configuration']);
            obj['Window Font and Style Configuration'] = WINDOW_STYLE_PARSER_MNUBLD(obj['Window Font and Style Configuration']);
            try{
                let i = 0;
                obj['Gauges'] = JSON.parse(obj['Gauges']).map((gauge_draw_config)=>{
                    const data = GAUGE_DRAW_PARSER_MNUBLD(gauge_draw_config);
                    if(data){
                        data['ID'] = i;
                        i++;
                        return data;
                    }
                }).filter(Boolean)
            }catch(e){
                obj['Gauges'] = [];
            }
            try{
                obj['Draw Base Params'] = JSON.parse(obj['Draw Base Params']).map((data)=>{
                    return ACTOR_BASE_PARAM_WINDOW_PARSER_MNUBLD(data);
                }).filter(Boolean);
            }catch(e){
                obj['Draw Base Params'] = [];
            }
            try{
                obj['Draw Ex Params'] = JSON.parse(obj['Draw Ex Params']).map((data)=>{
                    return ACTOR_EX_PARAM_WINDOW_PARSER_MNUBLD(data);
                }).filter(Boolean);
            }catch(e){
                obj['Draw Ex Params'] = [];
            }
            try{
                obj['Draw Sp Params'] = JSON.parse(obj['Draw Sp Params']).map((data)=>{
                    return ACTOR_SP_PARAM_WINDOW_PARSER_MNUBLD(data);
                }).filter(Boolean);
            }catch(e){
                obj['Draw Sp Params'] = [];
            }
            return obj;
        }catch(e){
            return;
        }
    }

    function ACTOR_SELECT_WINDOW_PARSER_MNUBLD(obj){
        try{
            obj = JSON.parse(obj);
            obj['Dimension Configuration'] = DIMENSION_CONFIGURATION_PARSER_MNUBLD(obj['Dimension Configuration']);
            obj['Window Font and Style Configuration'] = WINDOW_STYLE_PARSER_MNUBLD(obj['Window Font and Style Configuration']);
            try{
                let i = 0;
                obj['Gauges'] = JSON.parse(obj['Gauges']).map((gauge_draw_config)=>{
                    const data = GAUGE_DRAW_PARSER_MNUBLD(gauge_draw_config);
                    if(data){
                        data['ID'] = i;
                        i++;
                        return data;
                    }
                }).filter(Boolean)
            }catch(e){
                obj['Gauges'] = [];
            }
            try{
                obj['Draw Base Params'] = JSON.parse(obj['Draw Base Params']).map((data)=>{
                    return ACTOR_BASE_PARAM_WINDOW_PARSER_MNUBLD(data);
                }).filter(Boolean);
            }catch(e){
                obj['Draw Base Params'] = [];
            }
            try{
                obj['Draw Ex Params'] = JSON.parse(obj['Draw Ex Params']).map((data)=>{
                    return ACTOR_EX_PARAM_WINDOW_PARSER_MNUBLD(data);
                }).filter(Boolean);
            }catch(e){
                obj['Draw Ex Params'] = [];
            }
            try{
                obj['Draw Sp Params'] = JSON.parse(obj['Draw Sp Params']).map((data)=>{
                    return ACTOR_SP_PARAM_WINDOW_PARSER_MNUBLD(data);
                }).filter(Boolean);
            }catch(e){
                obj['Draw Sp Params'] = [];
            }
            return obj;
        }catch(e){
            return;
        }
    }

    function VAR_REQ_PARSER_MNUBLD(obj){
        try{
            obj = JSON.parse(obj);
            return obj;
        }catch(e){
            const obj = {};
            obj['Name'] = "";
            obj['Variable'] = 0;
            obj['Min Value'] = 0;
            obj['Max Value'] = 0;
            return;
        }
    }

    function ITM_REQ_PARSER_MNUBLD(obj){
        try{
            obj = JSON.parse(obj);
            return obj;
        }catch(e){
            return;
        }
    }

    function WEP_REQ_PARSER_MNUBLD(obj){
        try{
            obj = JSON.parse(obj);
            return obj;
        }catch(e){
            return;
        }
    }

    function ARM_REQ_PARSER_MNUBLD(obj){
        try{
            obj = JSON.parse(obj);
            return obj;
        }catch(e){
            return;
        }
    }

    function SELC_OPT_REQ_PARSER_MNUBLD(obj){
        try{
            obj = JSON.parse(obj);
            try{
                obj['Variable Requirements'] = JSON.parse(obj['Variable Requirements']).map((config)=>{
                    return VAR_REQ_PARSER_MNUBLD(config);
                }).filter(Boolean)
            }catch(e){
                obj['Variable Requirements'] = [];
            }
            try{
                obj['Switch Requirements'] = JSON.parse(obj['Switch Requirements']);
            }catch(e){
                obj['Switch Requirements'] = [];
            }
            try{
                obj['Item Requirements'] = JSON.parse(obj['Item Requirements']).map((config)=>{
                    return ITM_REQ_PARSER_MNUBLD(config);
                }).filter(Boolean)
            }catch(e){
                obj['Item Requirements'] = [];
            }
            try{
                obj['Weapon Requirements'] = JSON.parse(obj['Weapon Requirements']).map((config)=>{
                    return WEP_REQ_PARSER_MNUBLD(config);
                }).filter(Boolean)
            }catch(e){
                obj['Weapon Requirements'] = [];
            }
            try{
                obj['Armor Requirements'] = JSON.parse(obj['Armor Requirements']).map((config)=>{
                    return ARM_REQ_PARSER_MNUBLD(config);
                }).filter(Boolean)
            }catch(e){
                obj['Armor Requirements'] = [];
            }
            return obj;
        }catch(e){
            return;
        }
    }

    function SELC_OPT_BUTTON_PARSER_MNUBLD(obj){
        try{
            obj = JSON.parse(obj);
            obj['Cold Graphic'] = ANIMATED_GRAPHIC_PARSER_MNUBLD(obj['Cold Graphic']);
            obj['Hot Graphic'] = ANIMATED_GRAPHIC_PARSER_MNUBLD(obj['Hot Graphic']);
            return obj;
        }catch(e){
            return;
        }
    }

    function SELC_OPT_PARSER_MNUBLD(obj){
        try{
            obj = JSON.parse(obj);
            obj['Display Requirements'] = SELC_OPT_REQ_PARSER_MNUBLD(obj['Display Requirements']);
            obj['Select Requirements'] = SELC_OPT_REQ_PARSER_MNUBLD(obj['Select Requirements']);
            obj['Static Graphic'] = STATIC_GRAPHIC_PARSER_MNUBLD(obj['Static Graphic']);
            obj['Animated Graphic'] = ANIMATED_GRAPHIC_PARSER_MNUBLD(obj['Animated Graphic']);
            obj['Scene Button'] = SELC_OPT_BUTTON_PARSER_MNUBLD(obj['Scene Button']);
            try{
                obj['Pictures'] = JSON.parse(obj['Pictures']);
            }catch(e){
                obj['Pictures'] = [];
            }
            try{
                obj['Description'] = JSON.parse(obj['Description']);
            }catch(e){
                obj['Description'] = "";
            }
            try{
                obj['Code Execution'] = JSON.parse(obj['Code Execution']);
            }catch(e){
                obj['Code Execution'] = "";
            }
            return obj;
        }catch(e){
            return;
        }
    }

    function SELC_WINDOW_PARSER_MNUBLD(obj){
        try{
            obj = JSON.parse(obj);
            obj['Dimension Configuration'] = DIMENSION_CONFIGURATION_PARSER_MNUBLD(obj['Dimension Configuration']);
            obj['Window Font and Style Configuration'] = WINDOW_STYLE_PARSER_MNUBLD(obj['Window Font and Style Configuration']);
            try{
                let index = 0;
                obj['Selection Options'] = JSON.parse(obj['Selection Options']).map((opt_config)=>{
                    const data = SELC_OPT_PARSER_MNUBLD(opt_config);
                    if(data){
                        data['ID'] = index;
                        index++;
                        return data;
                    }
                }).filter(Boolean)
            }catch(e){
                obj['Selection Options'] = [];
            }
            try{
                let i = 0;
                obj['Gauges'] = JSON.parse(obj['Gauges']).map((gauge_draw_config)=>{
                    const data = GAUGE_DRAW_PARSER_MNUBLD(gauge_draw_config);
                    if(data){
                        data['ID'] = i;
                        i++;
                        return data;
                    }
                }).filter(Boolean)
            }catch(e){
                obj['Gauges'] = [];
            }
            return obj;
        }catch(e){
            return console.error(e);
        }
    }

    function SELC_DATA_WINDOW_PARSER_MNUBLD(obj){
        try{
            obj = JSON.parse(obj);
            obj['Dimension Configuration'] = DIMENSION_CONFIGURATION_PARSER_MNUBLD(obj['Dimension Configuration']);
            obj['Window Font and Style Configuration'] = WINDOW_STYLE_PARSER_MNUBLD(obj['Window Font and Style Configuration']);
            try{
                let i = 0;
                obj['Gauges'] = JSON.parse(obj['Gauges']).map((gauge_draw_config)=>{
                    const data = GAUGE_DRAW_PARSER_MNUBLD(gauge_draw_config);
                    if(data){
                        data['ID'] = i;
                        i++;
                        return data;
                    }
                }).filter(Boolean)
            }catch(e){
                obj['Gauges'] = [];
            }
            return obj;
        }catch(e){
            return;
        }
    }

    function MENU_SCENE_BUILD_PARSER_MNUBLD(obj){
        try{
            obj = JSON.parse(obj);
            if(!obj['Identifier Name']){
                console.error(`No identifier name detected. Parse aborted.`);
                return;
            }
            try{
                obj['Preload Backgrounds'] = JSON.parse(obj['Preload Backgrounds']);
            }catch(e){
                obj['Preload Backgrounds'] = [];
            }
            try{
                obj['On Load Script Calls'] = JSON.parse(obj['On Load Script Calls']).map((script_json)=>{
                    try{
                        script_json = JSON.parse(script_json);
                        return script_json;
                    }catch(e){
                        return ''
                    }
                }).filter(Boolean);
            }catch(e){
                obj['On Load Script Calls'] = [];
            }
            try{
                obj['Backgrounds'] = JSON.parse(obj['Backgrounds']).map((bg_config)=>{
                    return STATIC_GRAPHIC_PARSER_MNUBLD(bg_config);
                }).filter(Boolean);
            }catch(e){
                obj['Backgrounds'] = [];
            }
            try{
                obj['Back Graphics'] = JSON.parse(obj['Back Graphics']).map((bg_config)=>{
                    return ANIMATED_GRAPHIC_PARSER_MNUBLD(bg_config);
                }).filter(Boolean);
            }catch(e){
                obj['Back Graphics'] = [];
            }
            try{
                obj['Update Codes'] = JSON.parse(obj['Update Codes']).map((code_json)=>{
                    try{
                        code_json = JSON.parse(code_json);
                        return code_json;
                    }catch(e){
                        return "";
                    }
                }).filter(Boolean)
            }catch(e){
                obj['Update Codes'] = [];
            }
            obj['Selection Window'] = SELC_WINDOW_PARSER_MNUBLD(obj['Selection Window']);
            obj['Actor Selection Window'] = ACTOR_SELECT_WINDOW_PARSER_MNUBLD(obj['Actor Selection Window']);
            try{
                obj['Actor Data Windows'] = JSON.parse(obj['Actor Data Windows']).map((config)=>{
                    return ACTOR_DATA_WINDOW_PARSER_MNUBLD(config);
                }).filter(Boolean);
            }catch(e){
                obj['Actor Data Windows'] = [];
            }
            try{
                obj['Selection Data Windows'] = JSON.parse(obj['Selection Data Windows']).map((config)=>{
                    return SELC_DATA_WINDOW_PARSER_MNUBLD(config);
                }).filter(Boolean);
            }catch(e){
                obj['Selection Data Windows'] = [];
            }
            try{
                const basic_windows = JSON.parse(obj['Basic Windows']).map((window_config)=>{
                    return BASIC_WINDOW_PARSER_MNUBLD(window_config);
                }).filter(Boolean)
                obj['Basic Windows'] = basic_windows;
            }catch(e){
                obj['Basic Windows'] = [];
            }
            try{
                obj['Foregrounds'] = JSON.parse(obj['Foregrounds']).map((fg_config)=>{
                    return STATIC_GRAPHIC_PARSER_MNUBLD(fg_config);
                }).filter(Boolean);
            }catch(e){
                obj['Foregrounds'] = [];
            }
            try{
                obj['Fore Graphics'] = JSON.parse(obj['Fore Graphics']).map((fg_config)=>{
                    return ANIMATED_GRAPHIC_PARSER_MNUBLD(fg_config);
                }).filter(Boolean);
            }catch(e){
                obj['Fore Graphics'] = [];
            }
            return obj;
        }catch(e){
            return;
        }
    }

    try{
        const menus = JSON.parse(Syn_MenuBuildr.Plugin['Menu Configurations']).map((config)=>{
            return MENU_SCENE_BUILD_PARSER_MNUBLD(config);
        }).filter(Boolean);
        Syn_MenuBuildr.MENU_CONFIGURATIONS = menus;
    }catch(e){
        Syn_MenuBuildr.MENU_CONFIGURATIONS = [];
    }

    function SCENE_OVERRIDE_PARSER_MNUBLD(obj){
        try{
            obj = JSON.parse(obj);
            return obj;
        }catch(e){
            return;
        }
    }

    try{
        Syn_MenuBuildr.SCENE_OVERRIDES = JSON.parse(Syn_MenuBuildr.Plugin['Scene Overrides']).map((config)=>{
            return SCENE_OVERRIDE_PARSER_MNUBLD(config);
        }).filter(Boolean)
    }catch(e){
        Syn_MenuBuildr.SCENE_OVERRIDES = [];
    }
}

LOAD_MENU_BUILDER();

function RESET_MENU_BUILDER(data, scene_data){
    const backup = `var $plugins = ${JSON.stringify($plugins)}`;
    const fs = require('fs');
    $plugins.forEach((plugin)=>{
        if(plugin.name == data.name){
            plugin.parameters = data.parameters;
        }
    })
    const path_backup = `./js/plugin_backup.js`;
    const execute_backup = function(){
        const backup_exists = fs.existsSync(path_backup);
        if(backup_exists){
            console.warn(`Will not create new backup unless old is deleted.`);
            return;
        }
        fs.writeFileSync(path_backup, backup, (e)=>{
            console.error(`Failed to overwrite backup file.`)
        });
    }
    execute_backup();
    const path = `./js/plugins.js`;
    const file_string = `//Generated by RPG Maker (Synrec Menu Builder).\n//Do not edit this file directly.\nvar $plugins = \n${JSON.stringify($plugins)}`;
    fs.writeFileSync(path, file_string, (e)=>{
        console.error(`Failed to overwrite backup file.`)
    });
    PluginManager.setup($plugins);
    LOAD_MENU_BUILDER();
    const scene = SceneManager._scene;
    if(scene && scene instanceof Scene_SynrecMenu){
        scene.reloadScene(scene_data);
    }
    console.log(scene_data)
}

DataManager.menuBuilderGameReset = function(){
    const menu_data = JsonEx.makeDeepCopy($gameTemp.openedMenu());
    if(menu_data){
        const name = menu_data['Identifier Name'];
        $gameTemp.openMenu(name)
    }
    this.createGameObjects();
    this.setupNewGame();
}

SceneManager.snapForSynMenuBackground = function() {
    this._synMenuBackgroundBitmap = this.snap();
}

SceneManager.synMenuBackgroundBitmap = function() {
    return this._synMenuBackgroundBitmap;
}

Syn_MenuBuildr_ScnMngr_Goto = SceneManager.goto;
SceneManager.goto = function(sceneClass) {
    const overrides = Syn_MenuBuildr.SCENE_OVERRIDES;
    const override = overrides.find((config)=>{
        try{
            const class_id = eval(config['Scene Class']);
            const has_override = class_id == sceneClass && !!config['Menu Identifier'];
            if(has_override){
                return true;
            }
        }catch(e){
            return false;
        }
    })
    if(override){
        const identifier = override['Menu Identifier'];
        $gameTemp.openMenu(identifier);
        return
    }
    Syn_MenuBuildr_ScnMngr_Goto.call(this, ...arguments);
}

Syn_MenuBuildr_ScnMngr_push = SceneManager.push;
SceneManager.push = function(sceneClass) {
    const overrides = Syn_MenuBuildr.SCENE_OVERRIDES;
    const override = overrides.find((config)=>{
        try{
            const class_id = eval(config['Scene Class']);
            const has_override = class_id == sceneClass && !!config['Menu Identifier'];
            if(has_override){
                return true;
            }
        }catch(e){
            return false;
        }
    })
    if(override){
        const identifier = override['Menu Identifier'];
        $gameTemp.openMenu(identifier);
        return
    }
    Syn_MenuBuildr_ScnMngr_push.call(this, ...arguments);
}

Syn_MenuBuildr_ScnMngr_IsGmActv = SceneManager.isGameActive;
SceneManager.isGameActive = function() {
    const scene = this._scene;
    if(scene instanceof Scene_SynrecMenu){
        if(scene._editor_window){
            return true;
        }
    }
    return Syn_MenuBuildr_ScnMngr_IsGmActv.call(this, ...arguments);
}

Game_Temp.prototype.openedMenu = function(){
    return this._current_menu;
}

Game_Temp.prototype.openMenu = function(menu_name){
    if(!Array.isArray(this._last_menu)){
        this._last_menu = [];
    }
    const new_menu = Syn_MenuBuildr.MENU_CONFIGURATIONS.find((menu_config)=>{
        return menu_config['Identifier Name'] == menu_name;
    })
    if(!new_menu)return;
    const current_menu = this.openedMenu();
    this._last_menu.push(current_menu);
    if(!current_menu){
        SceneManager.snapForSynMenuBackground();
    }
    this._current_menu = JsonEx.makeDeepCopy(new_menu);
    SceneManager.push(Scene_SynrecMenuPreload);
}

Game_Temp.prototype.closeMenu = function(){
    if(!Array.isArray(this._last_menu)){
        this._last_menu = [];
    }
    const old_menu = this._last_menu.pop();
    if(old_menu){
        this._current_menu = old_menu;
    }else{
        this._current_menu = null;
    }
}

function Game_MenuCharacter(){
    this.initialize(...arguments);
}

Game_MenuCharacter.prototype = Object.create(Game_Character.prototype);
Game_MenuCharacter.prototype.constructor = Game_MenuCharacter;

Game_MenuCharacter.prototype.allowPixelMove = function(){
    //DO NOTHING!
}

Game_MenuCharacter.prototype.screenX = function() {
    return this._screenX || 0;
}

Game_MenuCharacter.prototype.screenY = function() {
    return this._screenY || 0;
}

Game_MenuCharacter.prototype.screenZ = function() {
    return 1;
}

Game_MenuCharacter.prototype.setActor = function(actor){
    if(actor instanceof (Game_Actor)){
        this._actor = actor;
        const char_name = actor.characterName();
        const char_index = actor.characterIndex();
        this.setImage(char_name, char_index);
        this.setStepAnime(true);
    }else{
        this.setImage("", 0);
    }
}

Game_MenuCharacter.prototype.setScreenX = function(num){
    isNaN(num) ? num = 0 : num;
    this._screenX = num;
}

Game_MenuCharacter.prototype.setScreenY = function(num){
    isNaN(num) ? num = 0 : num;
    this._screenY = num;
}

Syn_MenuBuildr_GmIntrpr_SetWaitMode = Game_Interpreter.prototype.setWaitMode
Game_Interpreter.prototype.setWaitMode = function(waitMode) {
    Syn_MenuBuildr_GmIntrpr_SetWaitMode.call(this, ...arguments);
    if(waitMode == 'transfer'){
        const scene = SceneManager._scene;
        if(scene instanceof Scene_SynrecMenu){
            scene.closeMenu();
        }
    }
}

function Sprite_SynMenuStaticGfx(){
    this.initialize(...arguments);
}

Sprite_SynMenuStaticGfx.prototype = Object.create(TilingSprite.prototype);
Sprite_SynMenuStaticGfx.prototype.constructor = Sprite_SynMenuStaticGfx;

Sprite_SynMenuStaticGfx.prototype.initialize = function(data){
    TilingSprite.prototype.initialize.call(this);
    this._gfx_data = data;
    this.setupGfx();
}

Sprite_SynMenuStaticGfx.prototype.setupGfx = function(gfx_data){
    const gfx_config = gfx_data || this._gfx_data;
    if(!gfx_config)return;
    const file_name = gfx_config['File'];
    if(!file_name)return;
    const bitmap = ImageManager.loadPicture(file_name);
    this._scroll_x = eval(gfx_config['Scrolling X']) || 0;
    this._scroll_y = eval(gfx_config['Scrolling Y']) || 0;
    this.anchor.x = eval(gfx_config['Anchor X']);
    this.anchor.y = eval(gfx_config['Anchor Y']);
    this._rotation = eval(gfx_config['Rotation']) || 0;
    this._constant_rot = !!eval(gfx_config['Constant Rotation']);
    this.bitmap = bitmap;
    const mx = eval(gfx_config['X']) || 0;
    const my = eval(gfx_config['Y']) || 0;
    const mw = bitmap.width;
    const mh = bitmap.height;
    this.move(mx,my,mw,mh);
    this.rotation = this._rotation;
}

Sprite_SynMenuStaticGfx.prototype.update = function(){
    TilingSprite.prototype.update.call(this);
    this.updateScrolling();
    this.updateRotation();
}

Sprite_SynMenuStaticGfx.prototype.updateScrolling = function(){
    this.origin.x += this._scroll_x || 0;
    this.origin.y += this._scroll_y || 0;
}

Sprite_SynMenuStaticGfx.prototype.updateRotation = function(){
    if(this._constant_rot){
        this.rotation += this._rotation || 0;
    }
}

function Sprite_SynMenuAnimGfx(){
    this.initialize(...arguments);
}

Sprite_SynMenuAnimGfx.prototype = Object.create(Sprite.prototype);
Sprite_SynMenuAnimGfx.prototype.constructor = Sprite_SynMenuAnimGfx;

Sprite_SynMenuAnimGfx.prototype.initialize = function(data){
    Sprite.prototype.initialize.call(this);
    this._gfx_data = data;
    this.setupGfx();
}

Sprite_SynMenuAnimGfx.prototype.setupGfx = function(gfx_data){
    const gfx_config = gfx_data || this._gfx_data;
    if(!gfx_config)return;
    const file_name = gfx_config['File'];
    if(!file_name)return;
    const bitmap = ImageManager.loadPicture(file_name);
    this._cur_frame = 0;
    this._max_frames = eval(gfx_config['Max Frames']);
    this._frame_rate = eval(gfx_config['Frame Rate']);
    this._frame_time = eval(gfx_config['Frame Rate']);
    this.bitmap = bitmap;
    const mx = eval(gfx_config['X']);
    const my = eval(gfx_config['Y']);
    this.move(mx,my);
    this.updateFrames();
}

Sprite_SynMenuAnimGfx.prototype.update = function(){
    Sprite.prototype.update.call(this);
    this.updateFrames();
}

Sprite_SynMenuAnimGfx.prototype.updateFrames = function(){
    const bitmap = this.bitmap;
    if(!bitmap)return;
    if(isNaN(this._frame_time) || this._frame_time >= this._frame_rate){
        this._frame_time = 0;
        const frames = this._max_frames;
        const w = bitmap.width / frames;
        const h = bitmap.height;
        const x = w * this._cur_frame;
        const y = 0;
        this.setFrame(x,y,w,h);
        this._cur_frame++;
        if(this._cur_frame >= frames){
            this._cur_frame = 0;
        }
    }else{
        this._frame_time++;
    }
}

function Sprite_SynMenuButton(){
    this.initialize(...arguments);
}

Sprite_SynMenuButton.prototype = Object.create(Sprite.prototype);
Sprite_SynMenuButton.prototype.constructor = Sprite_SynMenuButton;

Sprite_SynMenuButton.prototype.initialize = function(data, selc_window, index){
    Sprite.prototype.initialize.call(this);
    this._button_data = data;
    this._select_window = selc_window;
    this._selc_index = index;
    this.createColdGraphic();
    this.createHotGraphic();
    this.setPosition();
}

Sprite_SynMenuButton.prototype.createColdGraphic = function(){
    const sprite = new Sprite_SynMenuAnimGfx();
    const btn_data = this._button_data;
    if(btn_data){
        const cold_graphic_data = btn_data['Cold Graphic'];
        sprite.setupGfx(cold_graphic_data);
    }
    this.addChild(sprite);
    this._cold_gfx = sprite;
}

Sprite_SynMenuButton.prototype.createHotGraphic = function(){
    const sprite = new Sprite_SynMenuAnimGfx();
    sprite.visible = false;
    const btn_data = this._button_data;
    if(btn_data){
        const hot_graphic_data = btn_data['Hot Graphic'];
        sprite.setupGfx(hot_graphic_data);
    }
    this.addChild(sprite);
    this._hot_gfx = sprite;
}

Sprite_SynMenuButton.prototype.setPosition = function(){
    const btn_data = this._button_data;
    if(btn_data){
        const x = eval(btn_data['X']);
        const y = eval(btn_data['Y']);
        this.move(x,y);
    }
}

Sprite_SynMenuButton.prototype.update = function(){
    Sprite.prototype.update.call(this);
    this.updateSelected();
    this.updateGraphic();
    this.updateOnClick();
}

Sprite_SynMenuButton.prototype.updateSelected = function(){
    const selc_window = this._select_window;
    if(!selc_window)return;
    const selc_index = this._selc_index;
    const tx = TouchInput.x;
    const ty = TouchInput.y;
    const gfx = this._cold_gfx;
    const x = this.x;
    const y = this.y;
    const w = gfx.width;
    const h = gfx.height;
    if(
        tx >= x &&
        tx <= x + w &&
        ty >= y &&
        ty <= y + h
    ){
        this._hover = true;
        if(selc_window.index() != selc_index){
            selc_window.select(selc_index);
            SoundManager.playCursor();
        }
    }else{
        this._hover = false;
    }
    const win_indx = selc_window.index();
    this._selected = win_indx == selc_index;
}

Sprite_SynMenuButton.prototype.updateGraphic = function(){
    if(this._selected){
        this._cold_gfx.visible = false;
        this._hot_gfx.visible = true;
    }else{
        this._cold_gfx.visible = true;
        this._hot_gfx.visible = false;
    }
}

Sprite_SynMenuButton.prototype.updateOnClick = function(){
    if(this._hover && TouchInput.isTriggered()){
        const scene = SceneManager._scene;
        SoundManager.playOk();
        scene.doSelectAction();
    }
}

function SpriteMenu_Character(){
    this.initialize(...arguments);
}

SpriteMenu_Character.prototype = Object.create(Sprite_Character.prototype);
SpriteMenu_Character.prototype.constructor = SpriteMenu_Character;

SpriteMenu_Character.prototype.update = function(){
    this.updateChara();
    Sprite_Character.prototype.update.call(this);
}

SpriteMenu_Character.prototype.updateChara = function(){
    if(this._character){
        if(this._character.update){
            this._character.update();
        }
    }
}

function SpriteMenu_Battler(){
    this.initialize(...arguments);
}

SpriteMenu_Battler.prototype = Object.create(Sprite_Actor.prototype);
SpriteMenu_Battler.prototype.constructor = SpriteMenu_Battler;

SpriteMenu_Battler.prototype.updateMain = function() {
    this.updateBitmap();
    this.updateFrame();
    this.updateMove();
    this.updatePosition();
}

SpriteMenu_Battler.prototype.updateVisibility = function() {
    const isMV = Utils.RPGMAKER_NAME == 'MV';
    if(isMV){
        Sprite_Base.prototype.updateVisibility.call(this);
    }else{
        Sprite_Clickable.prototype.updateVisibility.call(this);
    }
    if (!this._battler) {
        this.visible = false;
    }
}

SpriteMenu_Battler.prototype.moveToStartPosition = function() {
    //No do move.
}

SpriteMenu_Battler.prototype.setActorHome = function(index) {
    //No do this.
}

SpriteMenu_Battler.prototype.setMotion = function(motion_name){
    this._setMotion = motion_name;
}

SpriteMenu_Battler.prototype.refreshMotion = function(){
    if(!this._setMotion)this._setMotion = 'walk';
    this.startMotion(this._setMotion);
}

function SpriteSynrec_VideoLayer(){
    this.initialize(...arguments);
}

SpriteSynrec_VideoLayer.prototype = Object.create(Sprite.prototype);
SpriteSynrec_VideoLayer.prototype.constructor = SpriteSynrec_VideoLayer;

SpriteSynrec_VideoLayer.prototype.initialize = function(){
    Sprite.prototype.initialize.call(this);
    this._videos = [];
    this._mz_mode = Utils.RPGMAKER_NAME == "MZ";
}

SpriteSynrec_VideoLayer.prototype.startVideo = function(name, label, loop, rect){
    if(!name)return;
    const mz_mode = this._mz_mode;
    const sprite = new PIXI.Sprite();
    const src = `movies/${name}.webm`;
    const videoTexture = mz_mode ? new PIXI.Texture.from(src) : new PIXI.Texture.fromVideo(src);
    const source = mz_mode ? videoTexture.baseTexture.resource.source : videoTexture.baseTexture.source;
    source.currentTime = 0;
    source.muted = false;
    source.loop = loop;
    source.preload = 'auto';
    source.autoload = true;
    source.autoplay = true;
    sprite.texture = videoTexture;
    sprite.x = rect.x || 0;
    sprite.y = rect.y || 0;
    sprite.width = rect.width || videoTexture.width || Graphics.boxWidth;
    sprite.height = rect.height || videoTexture.height || Graphics.boxHeight;
    sprite._label = label;
    sprite.alpha = 1;
    if(mz_mode)source.play();
    this.addChild(sprite);
    if(
        !rect.width ||
        !rect.height ||
        (
            sprite.width >= Infinity ||
            sprite.height >= Infinity
        )
    ){
    }
    this._videos.push(sprite)
}

SpriteSynrec_VideoLayer.prototype.endVideo = function(video){
    const mz_mode = this._mz_mode;
    const videos = this._videos;
    const index = videos.indexOf(video);
    if(index >= 0){
        videos.splice(index, 1);
        if(video){
            const texture = video.texture;
            const source = mz_mode ? texture.baseTexture.resource.source : texture.baseTexture.source;
            source.loop = false;
            source.muted = true;
            source.autoplay = false;
            source.currentTime = JsonEx.makeDeepCopy(source.duration);
            source.pause();
            this.removeChild(video);
        }
    }
}

SpriteSynrec_VideoLayer.prototype.endVideoByLabel = function(label){
    const videos = this._videos;
    const video = videos.find((video)=>{
        return video._label == label;
    })
    if(video){
        this.endVideo(video);
    }
}

SpriteSynrec_VideoLayer.prototype.endAllVideos = function(){
    const mz_mode = this._mz_mode;
    const videos = this._videos;
    videos.forEach((video)=>{
        const texture = video.texture;
        const source = mz_mode ? texture.baseTexture.resource.source : texture.baseTexture.source;
        source.loop = false;
        source.muted = true;
        source.autoplay = false;
        source.currentTime = JsonEx.makeDeepCopy(source.duration);
        source.pause();
        this.removeChild(video);
    })
    this._videos = [];
}

SpriteSynrec_VideoLayer.prototype.update = function(){
    Sprite.prototype.update.call(this);
    this.updateVideos()
}

SpriteSynrec_VideoLayer.prototype.updateVideos = function(){
    const mz_mode = this._mz_mode;
    const videos = this._videos;
    for(let i = 0; i < videos.length; i++){
        const video = videos[i];
        const texture = video.texture;
        texture.update();
        const source = mz_mode ? texture.baseTexture.resource.source : texture.baseTexture.source;
        if(
            video._need_resize > 0 &&
            !isNaN(video._need_resize) ||
            (
                video.width >= Infinity ||
                video.height >= Infinity
            )
        ){
            video.width = texture.width;
            video.height = texture.height;
            if(video._need_resize > 0)video._need_resize--;
        }
        const is_loop = source.loop;
        const is_end = source.currentTime >= source.duration;
        if(is_end){
            if(is_loop){
                source.currentTime = 0;
                source.play();
            }else{
                this.endVideo(video)
            }
        }
    }
}

function Window_SynMenuBasic(){
    this.initialize(...arguments);
}

Window_SynMenuBasic.prototype = Object.create(Window_Base.prototype);
Window_SynMenuBasic.prototype.constructor = Window_SynMenuBasic;

Window_SynMenuBasic.prototype.initialize = function(data){
    const mz_mode = Utils.RPGMAKER_NAME == "MZ";
    const rect = this.createRect(data);
    this._window_data = data;
    this._style_data = data['Window Font and Style Configuration'];
    if(mz_mode){
        Window_Base.prototype.initialize.call(this, rect);
    }else{
        const x = rect.x;
        const y = rect.y;
        const w = rect.width;
        const h = rect.height;
        Window_Base.prototype.initialize.call(this,x,y,w,h);
    }
    this.setOpacityAndDimmer();
    this.drawData();
}

Window_SynMenuBasic.prototype.createRect = function(data){
    const dimension_config = data['Dimension Configuration'];
    const x = dimension_config['X'];
    const y = dimension_config['Y'];
    const w = dimension_config['Width'];
    const h = dimension_config['Height'];
    return new Rectangle(x,y,w,h);
}

Window_SynMenuBasic.prototype.standardPadding = function() {
    return 8;
}

Window_SynMenuBasic.prototype.loadWindowskin = function(){
    const base = Window_Base.prototype.loadWindowskin.call(this);
    const custom_config = this._style_data;
    if(!custom_config)return base;
    const skin_name = custom_config['Window Skin'];
    if(!skin_name)return base;
    this.windowskin = ImageManager.loadSystem(skin_name);
}

Window_SynMenuBasic.prototype.resetFontSettings = function() {
    const base = Window_Base.prototype.resetFontSettings;
    const custom_config = this._style_data;
    if(!custom_config)return base.call(this);
    const font_face = custom_config['Font Face'] || "sans-serif";
    const font_size = custom_config['Font Size'] || 16;
    const font_outline_size = custom_config['Font Outline Thickness'] || 3;
    this.contents.fontFace = font_face;
    this.contents.fontSize = font_size;
    this.contents.outlineWidth = font_outline_size;
    this.resetTextColor();
}

Window_SynMenuBasic.prototype.resetTextColor = function() {
    const base = Window_Base.prototype.resetTextColor;
    const custom_config = this._style_data;
    if(!custom_config)return base.call(this);
    const text_color = custom_config['Base Font Color'] || "#ffffff";
    const outline_color = custom_config['Font Outline Color'] || "rgba(0, 0, 0, 0.5)";
    this.changeTextColor(text_color);
    this.contents.outlineColor = outline_color;
}

Window_SynMenuBasic.prototype.setOpacityAndDimmer = function(){
    const custom_config = this._style_data;
    if(!custom_config)return;
    const show_dimmer = custom_config['Show Window Dimmer'] || false;
    const win_opacity = custom_config['Window Opacity'] || 0;
    this.opacity = win_opacity;
    show_dimmer ? this.showBackgroundDimmer() : this.hideBackgroundDimmer();
}

Window_SynMenuBasic.prototype.drawData = function(){
    this.drawPictures();
    this.drawGauges();
    this.drawTexts();
}

Window_SynMenuBasic.prototype.drawPictures = function(){
    const data = this._window_data;
    const pictures = data['Draw Pictures'];
    if(Array.isArray(pictures)){
        const win = this;
        pictures.forEach((config)=>{
            try{
                const image_name = config['Picture'];
                if(image_name){
                    const bitmap = ImageManager.loadPicture(image_name);
                    const dx = eval(config['X']);
                    const dy = eval(config['Y']);
                    const dw = eval(config['Width']);
                    const dh = eval(config['Height']);
                    const bx = 0;
                    const by = 0;
                    const bw = bitmap.width || dw;
                    const bh = bitmap.height || dh;
                    win.contents.blt(bitmap, bx, by, bw, bh, dx, dy, dw, dh);
                }
            }catch(e){
                console.error(`Failed to draw picture: ${e}`);
            }
        })
    }
}

Window_SynMenuBasic.prototype.drawGauges = function(){
    const window = this;
    const data = this._window_data;
    const gauge_draw_configs = data['Gauges'];
    gauge_draw_configs.forEach((config)=>{
        try{
            const label = config['Label'];
            const lx = eval(config['Label X']);
            const ly = eval(config['Label Y']);
            window.drawTextEx(label, lx, ly);
            const cur_val = eval(config['Gauge Current Value']) || 0;
            const max_val = eval(config['Gauge Max Value']) || 1;
            const ratio = Math.max(0, Math.min(1, cur_val / max_val));
            const gx = eval(config['Gauge X']);
            const gy = eval(config['Gauge Y']);
            const gw = eval(config['Gauge Width']);
            const gh = eval(config['Gauge Height']);
            const gb = eval(config['Gauge Border']);
            const border_color = config['Gauge Border Color'];
            const background_color = config['Gauge Background Color'];
            const fill_color = config['Gauge Color'];
            window.contents.fillRect(gx,gy,gw,gh,border_color);
            window.contents.fillRect(gx + gb, gy + gb, gw - (gb * 2), gh - (gb * 2), background_color);
            window.contents.fillRect(gx + gb, gy + gb, (gw - (gb * 2)) * ratio, gh - (gb * 2), fill_color);
        }catch(e){
            console.error(`Failed to draw gauge: ${e}`);
        }
    })
}

Window_SynMenuBasic.prototype.drawTexts = function(){
    const data = this._window_data;
    const references = (data['Text References'] || []).map((code)=>{
        try{
            code = eval(code);
            return code;
        }catch(e){
            return '';
        }
    })
    const texts = data['Draw Texts'];
    if(Array.isArray(texts)){
        const win = this;
        texts.forEach((config)=>{
            try{
                const text = (config['Text'] || "").format(...references);
                const tx = eval(config['X']);
                const ty = eval(config['Y']);
                win.drawTextEx(text, tx, ty);
            }catch(e){
                console.error(`Failed to draw text: ${e}`);
            }
        })
    }
}

Window_SynMenuBasic.prototype.update = function(){
    Window_Base.prototype.update.call(this);
    this.updateDisplay();
}

Window_SynMenuBasic.prototype.updateDisplay = function(){
    try{
        const window_data = this._window_data;
        const display_requirements = window_data['Display Requirements'];
        if(!display_requirements){
            this.visible = true;
            return;
        }
        const switch_id = eval(display_requirements['Game Switch']);
        if(switch_id){
            if(!$gameSwitches.value(switch_id)){
                this.visible = false;
                return;
            }
        }
        const var_id = eval(display_requirements['Game Variable']);
        if(var_id){
            const value = $gameVariables.value(var_id);
            const min_var = eval(display_requirements['Variable Minimum']);
            const max_var = eval(display_requirements['Variable Maximum']);
            if(
                value < min_var ||
                value > max_var
            ){
                this.visible = false;
                return;
            }
        }
        if(display_requirements['Code']){
            const bool_code = !!eval(display_requirements['Code']);
            if(!bool_code){
                this.visible = false;
                return;
            }
        }
        this.visible = true;
    }catch(e){
        console.error(`Failed to parse window requirements: ${e}`);
        this.visible = false;
    }
}

function Window_SynMenuSelc(){
    this.initialize(...arguments);
}

Window_SynMenuSelc.prototype = Object.create(Window_Selectable.prototype);
Window_SynMenuSelc.prototype.constructor = Window_SynMenuSelc;

Window_SynMenuSelc.prototype.initialize = function(data){
    this._list = [];
    const mz_mode = Utils.RPGMAKER_NAME == "MZ";
    const rect = this.createRect(data);
    this._window_data = data;
    this._style_data = data['Window Font and Style Configuration'];
    if(mz_mode){
        Window_Selectable.prototype.initialize.call(this, rect);
    }else{
        const x = rect.x;
        const y = rect.y;
        const w = rect.width;
        const h = rect.height;
        Window_Selectable.prototype.initialize.call(this,x,y,w,h);
    }
    this.setOpacityAndDimmer();
    this.generateList();
    this.refresh();
    this.select(0);
    this.activate();
}

Window_SynMenuSelc.prototype.generateList = function(){
    const scene = SceneManager._scene;
    scene.clearButtons();
    const win = this;
    const window_data = this._window_data;
    const selc_options = (window_data['Selection Options'] || []).filter((option)=>{
        const display_req = option['Display Requirements'];
        if(win.checkRequirements(display_req)){
            return true;
        }
    });
    let index = 0;
    selc_options.forEach((option)=>{
        const button_data = option['Scene Button'];
        if(button_data){
            const button = new Sprite_SynMenuButton(button_data, win, index);
            scene.addChild(button);
            scene._scene_buttons.push(button);
        }
        index++;
    })
    this._list = selc_options;
}

Window_SynMenuSelc.prototype.createRect = function(data){
    const dimension_config = data['Dimension Configuration'];
    const x = dimension_config['X'];
    const y = dimension_config['Y'];
    const w = dimension_config['Width'];
    const h = dimension_config['Height'];
    return new Rectangle(x,y,w,h);
}

Window_SynMenuSelc.prototype.standardPadding = function() {
    return 8;
}

Window_SynMenuSelc.prototype.loadWindowskin = function(){
    const base = Window_Selectable.prototype.loadWindowskin.call(this);
    const custom_config = this._style_data;
    if(!custom_config)return base;
    const skin_name = custom_config['Window Skin'];
    if(!skin_name)return base;
    this.windowskin = ImageManager.loadSystem(skin_name);
}

Window_SynMenuSelc.prototype.resetFontSettings = function() {
    const base = Window_Selectable.prototype.resetFontSettings;
    const custom_config = this._style_data;
    if(!custom_config)return base.call(this);
    const font_face = custom_config['Font Face'] || "sans-serif";
    const font_size = custom_config['Font Size'] || 16;
    const font_outline_size = custom_config['Font Outline Thickness'] || 3;
    this.contents.fontFace = font_face;
    this.contents.fontSize = font_size;
    this.contents.outlineWidth = font_outline_size;
    this.resetTextColor();
}

Window_SynMenuSelc.prototype.resetTextColor = function() {
    const base = Window_Selectable.prototype.resetTextColor;
    const custom_config = this._style_data;
    if(!custom_config)return base.call(this);
    const text_color = custom_config['Base Font Color'] || "#ffffff";
    const outline_color = custom_config['Font Outline Color'] || "rgba(0, 0, 0, 0.5)";
    this.changeTextColor(text_color);
    this.contents.outlineColor = outline_color;
}

Window_SynMenuSelc.prototype.setOpacityAndDimmer = function(){
    const custom_config = this._style_data;
    if(!custom_config)return;
    const show_dimmer = custom_config['Show Window Dimmer'] || false;
    const win_opacity = custom_config['Window Opacity'] || 0;
    this.opacity = win_opacity;
    show_dimmer ? this.showBackgroundDimmer() : this.hideBackgroundDimmer();
}

Window_SynMenuSelc.prototype.maxItems = function(){
    return this._list.length;
}

Window_SynMenuSelc.prototype.maxCols = function(){
    const window_data = this._window_data;
    const cols = eval(window_data['Max Columns']) || 1;
    return Math.max(1, cols);
}

Window_SynMenuSelc.prototype.itemWidth = function(){
    const window_data = this._window_data;
    return eval(window_data['Item Width']) || 1;
}

Window_SynMenuSelc.prototype.itemHeight = function(){
    const window_data = this._window_data;
    return eval(window_data['Item Height']) || 1;
}

Window_SynMenuSelc.prototype.drawItem = function(i){
    const rect = this.itemRect(i);
    const data = this._list[i];
    if(data){
        this.drawPicture(rect, data);
        this.drawGauges(rect, data);
        this.drawName(rect, data);
        this.drawAltName(rect, data);
        this.drawDesc(rect, data);
    }
}

Window_SynMenuSelc.prototype.drawPicture = function(rect, data){
    const rx = rect.x;
    const ry = rect.y;
    const window_data = this._window_data;
    const draw_pic = eval(window_data['Draw Picture']);
    if(!draw_pic)return;
    const pic_names = JSON.parse(data['Pictures'] || []);
    const pic_index = eval(window_data['Picture Index']);
    const pic_name = pic_names[pic_index];
    if(!pic_name)return;
    const bitmap = ImageManager.loadPicture(pic_name);
    const bx = 0;
    const by = 0;
    const bw = bitmap.width;
    const bh = bitmap.height;
    const dx = rx + eval(window_data['Picture X']);
    const dy = ry + eval(window_data['Picture Y']);
    const dw = eval(window_data['Picture Width']);
    const dh = eval(window_data['Picture Height']);
    this.contents.blt(bitmap,bx,by,bw,bh,dx,dy,dw,dh);
}

Window_SynMenuSelc.prototype.drawGauges = function(rect, data){
    const rx = rect.x;
    const ry = rect.y;
    const window = this;
    const window_data = this._window_data;
    const gauge_draw_configs = window_data['Gauges'];
    gauge_draw_configs.forEach((config)=>{
        try{
            const label = config['Label'];
            const lx = rx + eval(config['Label X']);
            const ly = ry + eval(config['Label Y']);
            window.drawTextEx(label, lx, ly);
            const cur_val = eval(config['Gauge Current Value']) || 0;
            const max_val = eval(config['Gauge Max Value']) || 1;
            const ratio = Math.max(0, Math.min(1, cur_val / max_val));
            const gx = rx + eval(config['Gauge X']);
            const gy = ry + eval(config['Gauge Y']);
            const gw = eval(config['Gauge Width']);
            const gh = eval(config['Gauge Height']);
            const gb = eval(config['Gauge Border']);
            const border_color = config['Gauge Border Color'];
            const background_color = config['Gauge Background Color'];
            const fill_color = config['Gauge Color'];
            window.contents.fillRect(gx,gy,gw,gh,border_color);
            window.contents.fillRect(gx + gb, gy + gb, gw - (gb * 2), gh - (gb * 2), background_color);
            window.contents.fillRect(gx + gb, gy + gb, (gw - (gb * 2)) * ratio, gh - (gb * 2), fill_color);
        }catch(e){
            console.error(`Failed to draw gauge: ${e}`);
        }
    })
}

Window_SynMenuSelc.prototype.drawName = function(rect, data){
    const rx = rect.x;
    const ry = rect.y;
    const window_data = this._window_data;
    const draw_name = eval(window_data['Draw Name']);
    if(!draw_name)return;
    const name = data['Name'];
    const tx = rx + eval(window_data['Name X']);
    const ty = ry + eval(window_data['Name Y']);
    const text = (window_data['Name Text'] || "").format(name);
    this.drawTextEx(text,tx,ty);
}

Window_SynMenuSelc.prototype.drawAltName = function(rect, data){
    const rx = rect.x;
    const ry = rect.y;
    const window_data = this._window_data;
    const draw_alt_name = eval(window_data['Draw Alternative Name']);
    if(!draw_alt_name)return;
    const alt_name = data['Alternative Name'];
    const tx = rx + eval(window_data['Alternative Name X']);
    const ty = ry + eval(window_data['Alternative Name Y']);
    const text = (window_data['Alternative Name Text'] || "").format(alt_name);
    this.drawTextEx(text,tx,ty);
}

Window_SynMenuSelc.prototype.drawDesc = function(rect, data){
    const rx = rect.x;
    const ry = rect.y;
    const window_data = this._window_data;
    const draw_desc = eval(window_data['Draw Description']);
    if(!draw_desc)return;
    const desc = data['Description'];
    const tx = rx + eval(window_data['Description X']);
    const ty = ry + eval(window_data['Description Y']);
    this.drawTextEx(desc,tx,ty);
}

Window_SynMenuSelc.prototype.currentSelection = function(){
    const index = this.index();
    return this._list[index];
}

Window_SynMenuSelc.prototype.meetSelectRequirements = function(){
    const option = this.currentSelection();
    if(!option)return false;
    const requirements = option['Select Requirements'];
    const valid_reqs = this.checkRequirements(requirements);
    return valid_reqs;
}

Window_SynMenuSelc.prototype.checkRequirements = function(requirements){
    if(!requirements)return true;
    try{
        const var_reqs = requirements['Variable Requirements'].filter((var_data)=>{
            const id = eval(var_data['Variable']);
            return !isNaN(id) && id;
        });
        if(Array.isArray(var_reqs)){
            if(var_reqs.some((req)=>{
                const id = eval(req['Variable']);
                const value = $gameVariables.value(id);
                const min = eval(req['Min Value']);
                const max = eval(req['Max Value']);
                if(
                    value < min ||
                    value > max
                ){
                    return true;
                }
            })){
                return false;
            }
        }
        const sw_reqs = requirements['Switch Requirements'].filter((sw_id)=>{
            const id = eval(sw_id);
            return !isNaN(id) && id;
        });
        if(Array.isArray(sw_reqs)){
            if(sw_reqs.some((sw_id)=>{
                const switch_value = $gameSwitches.value(sw_id);
                return !switch_value;
            })){
                return false;
            }
        }
        const itm_reqs = requirements['Item Requirements'].filter((itm_data)=>{
            const id = eval(itm_data['Item']);
            return !isNaN(id) && id;
        });
        if(Array.isArray(itm_reqs)){
            if(itm_reqs.some((req)=>{
                const id = eval(req['Item']);
                const data = $dataItems[id];
                if(!data)throw new Error(`${id} is not a valid item ID.`);
                const min = eval(req['Min Value']);
                const max = eval(req['Max Value']);
                const num_bag = $gameParty.numItems(data);
                if(
                    num_bag < min ||
                    num_bag > max
                ){
                    return true;
                }
            })){
                return false;
            }
        }
        const wep_reqs = requirements['Weapon Requirements'].filter((wep_data)=>{
            const id = eval(wep_data['Weapon']);
            return !isNaN(id) && id;
        });
        if(Array.isArray(wep_reqs)){
            if(wep_reqs.some((req)=>{
                const id = eval(req['Weapon']);
                const data = $dataWeapons[id];
                if(!data)throw new Error(`${id} is not a valid weapon ID.`);
                const min = eval(req['Min Value']);
                const max = eval(req['Max Value']);
                const num_bag = $gameParty.numItems(data);
                if(
                    num_bag < min ||
                    num_bag > max
                ){
                    return true;
                }
            })){
                return false;
            }
        }
        const arm_reqs = requirements['Armor Requirements'].filter((arm_data)=>{
            const id = eval(arm_data['Armor']);
            return !isNaN(id) && id;
        });
        if(Array.isArray(arm_reqs)){
            if(arm_reqs.some((req)=>{
                const id = eval(req['Armor']);
                const data = $dataArmors[id];
                if(!data)throw new Error(`${id} is not a valid armor ID.`);
                const min = eval(req['Min Value']);
                const max = eval(req['Max Value']);
                const num_bag = $gameParty.numItems(data);
                if(
                    num_bag < min ||
                    num_bag > max
                ){
                    return true;
                }
            })){
                return false;
            }
        }
        return true;
    }catch(e){
        console.error(`Failed to parse window requirements: ${e}`);
        return false;
    }
}

function Window_SynMenuSelcData(){
    this.initialize(...arguments);
}

Window_SynMenuSelcData.prototype = Object.create(Window_Base.prototype);
Window_SynMenuSelcData.prototype.constructor = Window_SynMenuSelcData;

Window_SynMenuSelcData.prototype.initialize = function(data){
    const mz_mode = Utils.RPGMAKER_NAME == "MZ";
    const rect = this.createRect(data);
    this._window_data = data;
    this._style_data = data['Window Font and Style Configuration'];
    if(mz_mode){
        Window_Base.prototype.initialize.call(this, rect);
    }else{
        const x = rect.x;
        const y = rect.y;
        const w = rect.width;
        const h = rect.height;
        Window_Base.prototype.initialize.call(this,x,y,w,h);
    }
    this.setOpacityAndDimmer();
}

Window_SynMenuSelcData.prototype.createRect = function(data){
    const dimension_config = data['Dimension Configuration'];
    const x = dimension_config['X'];
    const y = dimension_config['Y'];
    const w = dimension_config['Width'];
    const h = dimension_config['Height'];
    return new Rectangle(x,y,w,h);
}

Window_SynMenuSelcData.prototype.standardPadding = function() {
    return 8;
}

Window_SynMenuSelcData.prototype.loadWindowskin = function(){
    const base = Window_Base.prototype.loadWindowskin.call(this);
    const custom_config = this._style_data;
    if(!custom_config)return base;
    const skin_name = custom_config['Window Skin'];
    if(!skin_name)return base;
    this.windowskin = ImageManager.loadSystem(skin_name);
}

Window_SynMenuSelcData.prototype.resetFontSettings = function() {
    const base = Window_Base.prototype.resetFontSettings;
    const custom_config = this._style_data;
    if(!custom_config)return base.call(this);
    const font_face = custom_config['Font Face'] || "sans-serif";
    const font_size = custom_config['Font Size'] || 16;
    const font_outline_size = custom_config['Font Outline Thickness'] || 3;
    this.contents.fontFace = font_face;
    this.contents.fontSize = font_size;
    this.contents.outlineWidth = font_outline_size;
    this.resetTextColor();
}

Window_SynMenuSelcData.prototype.resetTextColor = function() {
    const base = Window_Base.prototype.resetTextColor;
    const custom_config = this._style_data;
    if(!custom_config)return base.call(this);
    const text_color = custom_config['Base Font Color'] || "#ffffff";
    const outline_color = custom_config['Font Outline Color'] || "rgba(0, 0, 0, 0.5)";
    this.changeTextColor(text_color);
    this.contents.outlineColor = outline_color;
}

Window_SynMenuSelcData.prototype.setOpacityAndDimmer = function(){
    const custom_config = this._style_data;
    if(!custom_config)return;
    const show_dimmer = custom_config['Show Window Dimmer'] || false;
    const win_opacity = custom_config['Window Opacity'] || 0;
    this.opacity = win_opacity;
    show_dimmer ? this.showBackgroundDimmer() : this.hideBackgroundDimmer();
}

Window_SynMenuSelcData.prototype.update = function(){
    Window_Base.prototype.update.call(this);
    this.updateSelected();
    this.updateDisplay();
}

Window_SynMenuSelcData.prototype.updateSelected = function(){
    const selc_window = this._select_window;
    if(!selc_window)return;
    const selc_index = selc_window.index();
    if(this._selc_index != selc_index){
        this._selc_index = selc_index;
        this._data = selc_window.currentSelection();
        this.drawData();
    }
}

Window_SynMenuSelcData.prototype.updateDisplay = function(){
    const window_data = this._window_data;
    try{
        const display_requirements = window_data['Display Requirements'];
        if(!display_requirements){
            this.visible = true;
            return;
        }
        const switch_id = eval(display_requirements['Game Switch']);
        if(switch_id){
            if(!$gameSwitches.value(switch_id)){
                this.visible = false;
                return;
            }
        }
        const var_id = eval(display_requirements['Game Variable']);
        if(var_id){
            const value = $gameVariables.value(var_id);
            const min_var = eval(display_requirements['Variable Minimum']);
            const max_var = eval(display_requirements['Variable Maximum']);
            if(
                value < min_var ||
                value > max_var
            ){
                this.visible = false;
                return;
            }
        }
        if(display_requirements['Code']){
            const bool_code = !!eval(display_requirements['Code']);
            if(!bool_code){
                this.visible = false;
                return;
            }
        }
        this.visible = true;
    }catch(e){
        console.error(`Failed to parse display requirements: ${e}`);
        this.visible = false;
    }
}

Window_SynMenuSelcData.prototype.drawData = function(){
    this.contents.clear();
    const data = this._data;
    console.log(data)
    console.log(this._window_data)
    if(data){
        this.drawPicture(data);
        this.drawGauges(data);
        this.drawName(data);
        this.drawAltName(data);
        this.drawDesc(data);
    }
}

Window_SynMenuSelcData.prototype.drawPicture = function(data){
    const window_data = this._window_data;
    const draw_pic = eval(window_data['Draw Picture']);
    if(!draw_pic)return;
    const pic_names = JSON.parse(data['Pictures'] || [])
    const pic_index = eval(window_data['Picture Index']);
    const pic_name = pic_names[pic_index];
    if(!pic_name)return;
    const bitmap = ImageManager.loadPicture(pic_name);
    const bx = 0;
    const by = 0;
    const bw = bitmap.width;
    const bh = bitmap.height;
    const dx = eval(window_data['Picture X']);
    const dy = eval(window_data['Picture Y']);
    const dw = eval(window_data['Picture Width']);
    const dh = eval(window_data['Picture Height']);
    this.contents.blt(bitmap,bx,by,bw,bh,dx,dy,dw,dh);
}

Window_SynMenuSelcData.prototype.drawGauges = function(data){
    const window = this;
    const window_data = this._window_data;
    const gauge_draw_configs = window_data['Gauges'];
    gauge_draw_configs.forEach((config)=>{
        try{
            const label = config['Label'];
            const lx = eval(config['Label X']);
            const ly = eval(config['Label Y']);
            window.drawTextEx(label, lx, ly);
            const cur_val = eval(config['Gauge Current Value']) || 0;
            const max_val = eval(config['Gauge Max Value']) || 1;
            const ratio = Math.max(0, Math.min(1, cur_val / max_val));
            const gx = eval(config['Gauge X']);
            const gy = eval(config['Gauge Y']);
            const gw = eval(config['Gauge Width']);
            const gh = eval(config['Gauge Height']);
            const gb = eval(config['Gauge Border']);
            const border_color = config['Gauge Border Color'];
            const background_color = config['Gauge Background Color'];
            const fill_color = config['Gauge Color'];
            window.contents.fillRect(gx,gy,gw,gh,border_color);
            window.contents.fillRect(gx + gb, gy + gb, gw - (gb * 2), gh - (gb * 2), background_color);
            window.contents.fillRect(gx + gb, gy + gb, (gw - (gb * 2)) * ratio, gh - (gb * 2), fill_color);
        }catch(e){
            console.error(`Failed to draw gauge: ${e}`);
        }
    })
}

Window_SynMenuSelcData.prototype.drawName = function(data){
    const window_data = this._window_data;
    const draw_name = eval(window_data['Draw Option Name']);
    if(!draw_name)return;
    const name = data['Name'];
    const tx = eval(window_data['Name X']);
    const ty = eval(window_data['Name Y']);
    const text = (window_data['Name Text'] || "").format(name);
    this.drawTextEx(text,tx,ty);
}

Window_SynMenuSelcData.prototype.drawAltName = function(data){
    const window_data = this._window_data;
    const draw_alt_name = eval(window_data['Draw Alternative Option Name']);
    if(!draw_alt_name)return;
    const alt_name = data['Alternative Name'];
    const tx = eval(window_data['Alternative Name X']);
    const ty = eval(window_data['Alternative Name Y']);
    const text = (window_data['Alternative Name Text'] || "").format(alt_name);
    this.drawTextEx(text,tx,ty);
}

Window_SynMenuSelcData.prototype.drawDesc = function(data){
    const window_data = this._window_data;
    const draw_desc = eval(window_data['Draw Description']);
    if(!draw_desc)return;
    const desc = data['Description'];
    const tx = eval(window_data['Description X']);
    const ty = eval(window_data['Description Y']);
    this.drawTextEx(desc,tx,ty);
}
function WindowMenu_ActorData(){
    this.initialize(...arguments);
}

WindowMenu_ActorData.prototype = Object.create(Window_Base.prototype);
WindowMenu_ActorData.prototype.constructor = WindowMenu_ActorData;

WindowMenu_ActorData.prototype.initialize = function(data){
    const mz_mode = Utils.RPGMAKER_NAME == "MZ";
    const rect = this.createRect(data);
    this._window_data = data;
    this._style_data = data['Window Font and Style Configuration'];
    if(mz_mode){
        Window_Base.prototype.initialize.call(this, rect);
    }else{
        const x = rect.x;
        const y = rect.y;
        const w = rect.width;
        const h = rect.height;
        Window_Base.prototype.initialize.call(this,x,y,w,h);
    }
    this.setOpacityAndDimmer();
    this.createCharacterSprite();
    this.createBattlerSprite();
}

WindowMenu_ActorData.prototype.createCharacterSprite = function(){
    const chara = new Game_MenuCharacter();
    chara.setStepAnime(true);
    const sprite = new SpriteMenu_Character(chara);
    sprite.visible = false;
    this.addChild(sprite);
    this._chara = chara;
    this._character_sprite = sprite;
}

WindowMenu_ActorData.prototype.createBattlerSprite = function(){
    const sprite = new SpriteMenu_Battler();
    sprite.visible = false;
    this.addChild(sprite);
    this._battler_sprite = sprite;
}

WindowMenu_ActorData.prototype.createRect = function(data){
    const dimension_config = data['Dimension Configuration'];
    const x = dimension_config['X'];
    const y = dimension_config['Y'];
    const w = dimension_config['Width'];
    const h = dimension_config['Height'];
    return new Rectangle(x,y,w,h);
}

WindowMenu_ActorData.prototype.standardPadding = function() {
    return 8;
}

WindowMenu_ActorData.prototype.loadWindowskin = function(){
    const base = Window_Base.prototype.loadWindowskin.call(this);
    const custom_config = this._style_data;
    if(!custom_config)return base;
    const skin_name = custom_config['Window Skin'];
    if(!skin_name)return base;
    this.windowskin = ImageManager.loadSystem(skin_name);
}

WindowMenu_ActorData.prototype.resetFontSettings = function() {
    const base = Window_Base.prototype.resetFontSettings;
    const custom_config = this._style_data;
    if(!custom_config)return base.call(this);
    const font_face = custom_config['Font Face'] || "sans-serif";
    const font_size = custom_config['Font Size'] || 16;
    const font_outline_size = custom_config['Font Outline Thickness'] || 3;
    this.contents.fontFace = font_face;
    this.contents.fontSize = font_size;
    this.contents.outlineWidth = font_outline_size;
    this.resetTextColor();
}

WindowMenu_ActorData.prototype.resetTextColor = function() {
    const base = Window_Base.prototype.resetTextColor;
    const custom_config = this._style_data;
    if(!custom_config)return base.call(this);
    const text_color = custom_config['Base Font Color'] || "#ffffff";
    const outline_color = custom_config['Font Outline Color'] || "rgba(0, 0, 0, 0.5)";
    this.changeTextColor(text_color);
    this.contents.outlineColor = outline_color;
}

WindowMenu_ActorData.prototype.setOpacityAndDimmer = function(){
    const custom_config = this._style_data;
    if(!custom_config)return;
    const show_dimmer = custom_config['Show Window Dimmer'] || false;
    const win_opacity = custom_config['Window Opacity'] || 0;
    this.opacity = win_opacity;
    show_dimmer ? this.showBackgroundDimmer() : this.hideBackgroundDimmer();
}

WindowMenu_ActorData.prototype.update = function(){
    Window_Base.prototype.update.call(this);
    this.updateActor();
    this.updateVisible();
}

WindowMenu_ActorData.prototype.updateActor = function(){
    if(this._actor){
        const batt_sprite = this._battler_sprite;
        if(batt_sprite._motionCD <= 0){
            batt_sprite.startMotion(batt_sprite._motionLoaded);
            batt_sprite._motionCD = batt_sprite.motionSpeed() * 4;
        }else{
            batt_sprite._motionCD--;
        }
    }
}

WindowMenu_ActorData.prototype.updateVisible = function(){
    if(!this._selc_window){
        this.hide();
        return;
    }
    const window_data = this._window_data;
    try{
        const display_requirements = window_data['Display Requirements'];
        if(!display_requirements){
            this.visible = this._selc_window.openness >= 255;
            return;
        }
        const switch_id = eval(display_requirements['Game Switch']);
        if(switch_id){
            if(!$gameSwitches.value(switch_id)){
                this.visible = false;
                return;
            }
        }
        const var_id = eval(display_requirements['Game Variable']);
        if(var_id){
            const value = $gameVariables.value(var_id);
            const min_var = eval(display_requirements['Variable Minimum']);
            const max_var = eval(display_requirements['Variable Maximum']);
            if(
                value < min_var ||
                value > max_var
            ){
                this.visible = false;
                return;
            }
        }
        if(display_requirements['Code']){
            const bool_code = !!eval(display_requirements['Code']);
            if(!bool_code){
                this.visible = false;
                return;
            }
        }
        this.visible = this._selc_window.openness >= 255;
    }catch(e){
        console.error(`Failed to parse requirements: ${e}`);
        this.visible = false;
    }
}

WindowMenu_ActorData.prototype.setActor = function(actor){
    this.contents.clear();
    this._actor = actor;
    if(actor){
        this.show();
        this.drawData();
    }else if(this._blank_hide){
        this.hide();
    }else{
        this._battler_sprite.setBattler();
        this._chara.setOpacity(0);
    }
}

WindowMenu_ActorData.prototype.drawData = function(){
    this.drawGauges();
    this.drawName();
    this.drawProfile();
    this.drawClassLevel();
    this.drawResHP();
    this.drawResMP();
    this.drawResTP();
    this.drawBaseParams();
    this.drawExParams();
    this.drawSpParams();
    this.displayMapCharacter();
    this.displayBattler();
}

WindowMenu_ActorData.prototype.drawGauges = function(){
    const window = this;
    const actor = this._actor;
    const window_data = this._window_data;
    const gauges = window_data['Gauges'];
    gauges.forEach((config)=>{
        try{
            const label = config['Label'];
            const lx = eval(config['Label X']);
            const ly = eval(config['Label Y']);
            window.drawTextEx(label, lx, ly);
            const cur_val = eval(config['Gauge Current Value']) || 0;
            const max_val = eval(config['Gauge Max Value']) || 1;
            const ratio = Math.max(0, Math.min(1, cur_val / max_val));
            const gx = eval(config['Gauge X']);
            const gy = eval(config['Gauge Y']);
            const gw = eval(config['Gauge Width']);
            const gh = eval(config['Gauge Height']);
            const gb = eval(config['Gauge Border']);
            const border_color = config['Gauge Border Color'];
            const background_color = config['Gauge Background Color'];
            const fill_color = config['Gauge Color'];
            window.contents.fillRect(gx,gy,gw,gh,border_color);
            window.contents.fillRect(gx + gb, gy + gb, gw - (gb * 2), gh - (gb * 2), background_color);
            window.contents.fillRect(gx + gb, gy + gb, (gw - (gb * 2)) * ratio, gh - (gb * 2), fill_color);
        }catch(e){
            console.error(`Failed to draw gauge: ${e}`);
        }
    })
}

WindowMenu_ActorData.prototype.drawName = function(){
    const actor = this._actor;
    const window_data = this._window_data;
    if(!eval(window_data['Draw Actor Name']))return;
    const name = actor.name();
    const nickname = actor.nickname();
    const text = (window_data['Name Text'] || "").format(name, nickname);
    const tx = eval(window_data['Name X']) || 0;
    const ty = eval(window_data['Name Y']) || 0;
    this.drawTextEx(text, tx, ty);
}

WindowMenu_ActorData.prototype.drawProfile = function(){
    const actor = this._actor;
    const window_data = this._window_data;
    if(!eval(window_data['Draw Actor Profile']))return;
    const text = actor.profile();
    const tx = eval(window_data['Profile X']) || 0;
    const ty = eval(window_data['Profile Y']) || 0;
    this.drawTextEx(text, tx, ty);
}

WindowMenu_ActorData.prototype.drawClassLevel = function(){
    const actor = this._actor;
    const window_data = this._window_data;
    if(!eval(window_data['Draw Class Level']))return;
    const class_id = actor._classId;
    const class_data = $dataClasses[class_id] || {};
    const class_name = class_data ? class_data.name : "";
    const level = actor.level;
    const text = (window_data['Class Level Text'] || "").format(class_name, level);
    const tx = eval(window_data['Class Level X']) || 0;
    const ty = eval(window_data['Class Level Y']) || 0;
    this.drawTextEx(text, tx, ty);
}

WindowMenu_ActorData.prototype.drawResHP = function(){
    const actor = this._actor;
    const window_data = this._window_data;
    if(!eval(window_data['Draw HP Resource']))return;
    const cur = actor.hp;
    const max = actor.mhp;
    const text = (window_data['HP Text'] || "").format(cur, max);
    const tx = eval(window_data['HP X']) || 0;
    const ty = eval(window_data['HP Y']) || 0;
    this.drawTextEx(text, tx, ty);
}

WindowMenu_ActorData.prototype.drawResMP = function(){
    const actor = this._actor;
    const window_data = this._window_data;
    if(!eval(window_data['Draw MP Resource']))return;
    const cur = actor.mp;
    const max = actor.mmp;
    const text = (window_data['MP Text'] || "").format(cur, max);
    const tx = eval(window_data['MP X']) || 0;
    const ty = eval(window_data['MP Y']) || 0;
    this.drawTextEx(text, tx, ty);
}

WindowMenu_ActorData.prototype.drawResTP = function(){
    const actor = this._actor;
    const window_data = this._window_data;
    if(!eval(window_data['Draw TP Resource']))return;
    const cur = actor.tp;
    const max = actor.maxTp();
    const text = (window_data['TP Text'] || "").format(cur, max);
    const tx = eval(window_data['TP X']) || 0;
    const ty = eval(window_data['TP Y']) || 0;
    this.drawTextEx(text, tx, ty);
}

WindowMenu_ActorData.prototype.drawBaseParams = function(){
    const window = this;
    const actor = this._actor;
    const window_data = this._window_data;
    const draw_params = window_data['Draw Base Params'] || [];
    draw_params.forEach((param_draw)=>{
        try{
            const param_id = eval(param_draw['Base Param']);
            const param_value = actor.param(param_id) || 0;
            const text = (param_draw['Param Text'] || "").format(param_value);
            const tx = eval(param_draw['X']) || 0;
            const ty = eval(param_draw['Y']) || 0;
            window.drawTextEx(text, tx, ty);
        }catch(e){
            console.error(`Failed to draw base param: ${e}`);
        }
    })
}

WindowMenu_ActorData.prototype.drawExParams = function(){
    const window = this;
    const actor = this._actor;
    const window_data = this._window_data;
    const draw_params = window_data['Draw Ex Params'] || [];
    draw_params.forEach((param_draw)=>{
        try{
            const param_id = eval(param_draw['Ex Param']);
            const param_value = (actor.xparam(param_id) || 0) * 100;
            const text = (param_draw['Param Text'] || "").format(param_value);
            const tx = eval(param_draw['X']) || 0;
            const ty = eval(param_draw['Y']) || 0;
            window.drawTextEx(text, tx, ty);
        }catch(e){
            console.error(`Failed to draw EX param: ${e}`);
        }
    })
}

WindowMenu_ActorData.prototype.drawSpParams = function(){
    const window = this;
    const actor = this._actor;
    const window_data = this._window_data;
    const draw_params = window_data['Draw Sp Params'] || [];
    draw_params.forEach((param_draw)=>{
        try{
            const param_id = eval(param_draw['SP Param']);
            const param_value = (actor.sparam(param_id) || 0) * 100;
            const text = (param_draw['Param Text'] || "").format(param_value);
            const tx = eval(param_draw['X']) || 0;
            const ty = eval(param_draw['Y']) || 0;
            window.drawTextEx(text, tx, ty);
        }catch(e){
            console.error(`Failed to draw SP param: ${e}`);
        }
    })
}

WindowMenu_ActorData.prototype.displayMapCharacter = function(){
    const actor = this._actor;
    const window_data = this._window_data;
    if(!eval(window_data['Display Map Character'])){
        this._chara.setOpacity(0);
        return;
    }else if(actor){
        this._chara.setOpacity(255);
        const char_name = actor.characterName();
        const char_indx = actor.characterIndex();
        this._chara.setImage(char_name, char_indx);
        this._chara.setDirection(eval(window_data['Character Direction']) || 2);
        this._chara._screenX = eval(window_data['Character X']) || 0;
        this._chara._screenY = eval(window_data['Character Y']) || 0;
        this._character_sprite.scale.x = eval(window_data['Character Scale X']) || 0;
        this._character_sprite.scale.y = eval(window_data['Character Scale Y']) || 0;
    }else{
        this._chara.setOpacity(0);
    }
}

WindowMenu_ActorData.prototype.displayBattler = function(){
    const actor = this._actor;
    const window_data = this._window_data;
    if(!eval(window_data['Display Battler'])){
        this._battler_sprite.setBattler();
        return;
    }else if(actor){
        const hx = eval(window_data['Battler X']);
        const hy = eval(window_data['Battler Y']);
        this._battler_sprite._motionLoaded = window_data['Battler Motion'];
        this._battler_sprite.startMotion(this._battler_sprite._motionLoaded);
        this._battler_sprite.setHome(hx, hy);
        this._battler_sprite.setBattler(actor);
        this._battler_sprite.scale.x = eval(window_data['Battler Scale X']);
        this._battler_sprite.scale.y = eval(window_data['Battler Scale Y']);
        this._battler_sprite._motionCD = 0;
    }else{
        this._battler_sprite.setBattler();
    }
}

function WindowMenu_ActorSelector(){
    this.initialize(...arguments);
}

WindowMenu_ActorSelector.prototype = Object.create(Window_Selectable.prototype);
WindowMenu_ActorSelector.prototype.constructor = WindowMenu_ActorSelector;

WindowMenu_ActorSelector.prototype.initialize = function(data, list){
    const mz_mode = Utils.RPGMAKER_NAME == "MZ";
    const rect = this.createRect(data);
    this._window_data = data;
    this._style_data = data['Window Font and Style Configuration'];
    this._list = list;
    if(mz_mode){
        Window_Selectable.prototype.initialize.call(this, rect);
    }else{
        const x = rect.x;
        const y = rect.y;
        const w = rect.width;
        const h = rect.height;
        Window_Selectable.prototype.initialize.call(this,x,y,w,h);
    }
    const UI = $gameTemp.openedMenu();
    this.openness = eval(UI['Always Show Actor Select']) ? 255 : 0;
    this.setOpacityAndDimmer();
    this.createCharacterSprites();
    this.createBattlerSprites();
}

WindowMenu_ActorSelector.prototype.clearSprites = function(){
    this._character_sprites.forEach((sprite)=>{
        if(sprite.parent)sprite.parent.removeChild(sprite);
        if(sprite.destroy)sprite.destroy();
    })
    this._character_sprites = [];
    this._battler_sprites.forEach((sprite)=>{
        if(sprite.parent)sprite.parent.removeChild(sprite);
        if(sprite.destroy)sprite.destroy();
    })
    this._battler_sprites = [];
}

WindowMenu_ActorSelector.prototype.createCharacterSprites = function(){
    this._character_sprites = [];
}

WindowMenu_ActorSelector.prototype.createBattlerSprites = function(){
    this._battler_sprites = [];
}

WindowMenu_ActorSelector.prototype.createCharacterSprite = function(i){
    const rect = this.itemRect(i);
    const chara = new Game_MenuCharacter();
    chara.setStepAnime(true);
    chara.setOpacity(0);
    const sprite = new SpriteMenu_Character(chara);
    sprite.visible = false;
    this.addChild(sprite);
    this._chara = chara;
    this._character_sprites[i] = sprite;
}

WindowMenu_ActorSelector.prototype.createBattlerSprite = function(i){
    const rect = this.itemRect(i);
    const sprite = new SpriteMenu_Battler();
    sprite.visible = false;
    this.addChild(sprite);
    this._battler_sprites[i] = sprite;
}

WindowMenu_ActorSelector.prototype.maxItems = function(){
    if(this._forceMaxItems)return this._forceMaxItems;
    return this._list ? this._list.length : 0;
}

WindowMenu_ActorSelector.prototype.maxCols = function(){
    const window_data = this._window_data;
    return eval(window_data['Max Columns']) || 1;
}

WindowMenu_ActorSelector.prototype.itemWidth = function(){
    const base = Window_Selectable.prototype.itemWidth.call(this);
    const window_data = this._window_data;
    return eval(window_data['Item Width']) || base;
}

WindowMenu_ActorSelector.prototype.itemHeight = function(){
    const base = Window_Selectable.prototype.itemHeight.call(this);
    const window_data = this._window_data;
    return eval(window_data['Item Height']) || base;
}

WindowMenu_ActorSelector.prototype.createRect = function(data){
    const dimension_config = data['Dimension Configuration'];
    const x = dimension_config['X'];
    const y = dimension_config['Y'];
    const w = dimension_config['Width'];
    const h = dimension_config['Height'];
    return new Rectangle(x,y,w,h);
}

WindowMenu_ActorSelector.prototype.standardPadding = function() {
    return 8;
}

WindowMenu_ActorSelector.prototype.loadWindowskin = function(){
    const base = Window_Base.prototype.loadWindowskin.call(this);
    const custom_config = this._style_data;
    if(!custom_config)return base;
    const skin_name = custom_config['Window Skin'];
    if(!skin_name)return base;
    this.windowskin = ImageManager.loadSystem(skin_name);
}

WindowMenu_ActorSelector.prototype.resetFontSettings = function() {
    const base = Window_Base.prototype.resetFontSettings;
    const custom_config = this._style_data;
    if(!custom_config)return base.call(this);
    const font_face = custom_config['Font Face'] || "sans-serif";
    const font_size = custom_config['Font Size'] || 16;
    const font_outline_size = custom_config['Font Outline Thickness'] || 3;
    this.contents.fontFace = font_face;
    this.contents.fontSize = font_size;
    this.contents.outlineWidth = font_outline_size;
    this.resetTextColor();
}

WindowMenu_ActorSelector.prototype.resetTextColor = function() {
    const base = Window_Base.prototype.resetTextColor;
    const custom_config = this._style_data;
    if(!custom_config)return base.call(this);
    const text_color = custom_config['Base Font Color'] || "#ffffff";
    const outline_color = custom_config['Font Outline Color'] || "rgba(0, 0, 0, 0.5)";
    this.changeTextColor(text_color);
    this.contents.outlineColor = outline_color;
}

WindowMenu_ActorSelector.prototype.setOpacityAndDimmer = function(){
    const custom_config = this._style_data;
    if(!custom_config)return;
    const show_dimmer = custom_config['Show Window Dimmer'] || false;
    const win_opacity = custom_config['Window Opacity'] || 0;
    this.opacity = win_opacity;
    show_dimmer ? this.showBackgroundDimmer() : this.hideBackgroundDimmer();
}

WindowMenu_ActorSelector.prototype.setList = function(list){
    this._list = list;
    this.clearSprites();
    this.refresh();
}

WindowMenu_ActorSelector.prototype.actor = function(i){
    const index = isNaN(i) ? this.index() : i;
    const actor = this._list[index];
    return actor;
}

WindowMenu_ActorSelector.prototype.update = function(){
    Window_Selectable.prototype.update.call(this);
    this.updateSprites();
}

WindowMenu_ActorSelector.prototype.updateSprites = function(){
    const chara_sprites = this._character_sprites;
    for(let i = 0; i < chara_sprites.length; i++){
        const rect = this.itemRect(i);
        const sprite = chara_sprites[i];
        if(sprite){
            if(sprite.visible){
                const chara = sprite._character;
                const rx = rect.x;
                const ry = rect.y;
                const sx = -this._scrollX || 0;
                const sy = -this._scrollY || 0;
                const ox = chara._off_screenX || 0;
                const oy = chara._off_screenY || 0;
                const x = rx + sx + ox;
                const y = ry + sy + oy;
                chara._screenX = x;
                chara._screenY = y;
                if(sprite._visibility){
                    chara.setOpacity(255);
                }else{
                    chara.setOpacity(0);
                }
            }
        }
    }
    const batt_sprites = this._battler_sprites;
    for(let i = 0; i < batt_sprites.length; i++){
        const rect = this.itemRect(i);
        const sprite = batt_sprites[i];
        if(sprite){
            const rx = rect.x;
            const ry = rect.y;
            const sx = -this._scrollX || 0;
            const sy = -this._scrollY || 0;
            const ox = sprite._offset_x || 0;
            const oy = sprite._offset_y || 0;
            const x = rx + sx + ox;
            const y = ry + sy + oy;
            sprite.setHome(x, y);
            if(sprite._visibility){
                const actor = this.actor(i);
                if(sprite._battler != actor){
                    sprite.setBattler(actor);
                }else{
                    if(sprite._motionCD <= 0){
                        sprite.startMotion(sprite._motionLoaded);
                        sprite._motionCD = sprite.motionSpeed() * 4;
                    }else{
                        sprite._motionCD--;
                    }
                }
            }else{
                sprite.setBattler(null);
            }
        }
    }
}

WindowMenu_ActorSelector.prototype.drawItem = function(i){
    if(!this._list)return;
    const rect = this.itemRect(i);
    const actor = this._list[i];
    if(actor){
        this.drawGauges(rect, actor);
        this.drawName(rect, actor);
        this.drawProfile(rect, actor);
        this.drawClassLevel(rect, actor);
        this.drawResHP(rect, actor);
        this.drawResMP(rect, actor);
        this.drawResTP(rect, actor);
        this.drawBaseParams(rect, actor);
        this.drawExParams(rect, actor);
        this.drawSpParams(rect, actor);
        this.displayMapCharacter(rect, i, actor);
        this.displayBattler(rect, i, actor);
    }else{
        const text = '-';
        const x = rect.x;
        const y = rect.y + (rect.height * 0.5);
        this.drawText(text, x, y, rect.width, 'center');
    }
}

WindowMenu_ActorSelector.prototype.drawGauges = function(rect, actor){
    const rx = rect.x;
    const ry = rect.y;
    const window = this;
    const window_data = this._window_data;
    const gauges = window_data['Gauges'] || [];
    gauges.forEach((config)=>{
        try{
            const label = config['Label'];
            const lx = rx + eval(config['Label X']);
            const ly = ry + eval(config['Label Y']);
            window.drawTextEx(label, lx, ly);
            const cur_val = eval(config['Gauge Current Value']) || 0;
            const max_val = eval(config['Gauge Max Value']) || 1;
            const ratio = Math.max(0, Math.min(1, cur_val / max_val));
            const gx = rx + eval(config['Gauge X']);
            const gy = ry + eval(config['Gauge Y']);
            const gw = eval(config['Gauge Width']);
            const gh = eval(config['Gauge Height']);
            const gb = eval(config['Gauge Border']);
            const border_color = config['Gauge Border Color'];
            const background_color = config['Gauge Background Color'];
            const fill_color = config['Gauge Color'];
            window.contents.fillRect(gx,gy,gw,gh,border_color);
            window.contents.fillRect(gx + gb, gy + gb, gw - (gb * 2), gh - (gb * 2), background_color);
            window.contents.fillRect(gx + gb, gy + gb, (gw - (gb * 2)) * ratio, gh - (gb * 2), fill_color);
        }catch(e){
            console.error(`Failed to draw gauge: ${e}`);
        }
    })
}

WindowMenu_ActorSelector.prototype.drawName = function(rect, actor){
    const rx = rect.x;
    const ry = rect.y;
    const window_data = this._window_data;
    if(!eval(window_data['Draw Actor Name']))return;
    const name = actor.name();
    const nickname = actor.nickname();
    const text = (window_data['Name Text'] || "").format(name, nickname);
    const tx = rx + eval(window_data['Name X']) || 0;
    const ty = ry + eval(window_data['Name Y']) || 0;
    this.drawTextEx(text, tx, ty);
}

WindowMenu_ActorSelector.prototype.drawProfile = function(rect, actor){
    const rx = rect.x;
    const ry = rect.y;
    const window_data = this._window_data;
    if(!eval(window_data['Draw Actor Profile']))return;
    const text = actor.profile();
    const tx = rx + eval(window_data['Profile X']) || 0;
    const ty = ry + eval(window_data['Profile Y']) || 0;
    this.drawTextEx(text, tx, ty);
}

WindowMenu_ActorSelector.prototype.drawClassLevel = function(rect, actor){
    const rx = rect.x;
    const ry = rect.y;
    const window_data = this._window_data;
    if(!eval(window_data['Draw Class Level']))return;
    const class_id = actor._classId;
    const class_data = $dataClasses[class_id] || {};
    const class_name = class_data ? class_data.name : "";
    const level = actor.level;
    const text = (window_data['Class Level Text'] || "").format(class_name, level);
    const tx = rx + eval(window_data['Class Level X']) || 0;
    const ty = ry + eval(window_data['Class Level Y']) || 0;
    this.drawTextEx(text, tx, ty);
}

WindowMenu_ActorSelector.prototype.drawResHP = function(rect, actor){
    const rx = rect.x;
    const ry = rect.y;
    const window_data = this._window_data;
    if(!eval(window_data['Draw HP Resource']))return;
    const cur = actor.hp;
    const max = actor.mhp;
    const text = (window_data['HP Text'] || "").format(cur, max);
    const tx = rx + eval(window_data['HP X']) || 0;
    const ty = ry + eval(window_data['HP Y']) || 0;
    this.drawTextEx(text, tx, ty);
}

WindowMenu_ActorSelector.prototype.drawResMP = function(rect, actor){
    const rx = rect.x;
    const ry = rect.y;
    const window_data = this._window_data;
    if(!eval(window_data['Draw MP Resource']))return;
    const cur = actor.mp;
    const max = actor.mmp;
    const text = (window_data['MP Text'] || "").format(cur, max);
    const tx = rx + eval(window_data['MP X']) || 0;
    const ty = ry + eval(window_data['MP Y']) || 0;
    this.drawTextEx(text, tx, ty);
}

WindowMenu_ActorSelector.prototype.drawResTP = function(rect, actor){
    const rx = rect.x;
    const ry = rect.y;
    const window_data = this._window_data;
    if(!eval(window_data['Draw TP Resource']))return;
    const cur = actor.tp;
    const max = actor.maxTp();
    const text = (window_data['TP Text'] || "").format(cur, max);
    const tx = rx + eval(window_data['TP X']) || 0;
    const ty = ry + eval(window_data['TP Y']) || 0;
    this.drawTextEx(text, tx, ty);
}

WindowMenu_ActorSelector.prototype.drawBaseParams = function(rect, actor){
    const rx = rect.x;
    const ry = rect.y;
    const window = this;
    const window_data = this._window_data;
    const draw_params = window_data['Draw Base Params'] || [];
    draw_params.forEach((param_draw)=>{
        try{
            const param_id = eval(param_draw['Base Param']);
            const param_value = actor.param(param_id) || 0;
            const text = (param_draw['Param Text'] || "").format(param_value);
            const tx = rx + eval(param_draw['X']) || 0;
            const ty = ry + eval(param_draw['Y']) || 0;
            window.drawTextEx(text, tx, ty);
        }catch(e){
            console.error(`Failed to draw base param: ${e}`);
        }
    })
}

WindowMenu_ActorSelector.prototype.drawExParams = function(rect, actor){
    const rx = rect.x;
    const ry = rect.y;
    const window = this;
    const window_data = this._window_data;
    const draw_params = window_data['Draw Ex Params'] || [];
    draw_params.forEach((param_draw)=>{
        try{
            const param_id = eval(param_draw['Ex Param']);
            const param_value = (actor.xparam(param_id) || 0) * 100;
            const text = (param_draw['Param Text'] || "").format(param_value);
            const tx = rx + eval(param_draw['X']) || 0;
            const ty = ry + eval(param_draw['Y']) || 0;
            window.drawTextEx(text, tx, ty);
        }catch(e){
            console.error(`Failed to draw EX param: ${e}`);
        }
    })
}

WindowMenu_ActorSelector.prototype.drawSpParams = function(rect, actor){
    const rx = rect.x;
    const ry = rect.y;
    const window = this;
    const window_data = this._window_data;
    const draw_params = window_data['Draw Sp Params'] || [];
    draw_params.forEach((param_draw)=>{
        try{
            const param_id = eval(param_draw['Sp Param']);
            const param_value = (actor.sparam(param_id) || 0) * 100;
            const text = (param_draw['Param Text'] || "").format(param_value);
            const tx = rx + eval(param_draw['X']) || 0;
            const ty = ry + eval(param_draw['Y']) || 0;
            window.drawTextEx(text, tx, ty);
        }catch(e){
            console.error(`Failed to draw SP param: ${e}`);
        }
    })
}

WindowMenu_ActorSelector.prototype.displayMapCharacter = function(rect, index, actor){
    const window_data = this._window_data;
    if(!this._character_sprites[index])this.createCharacterSprite(index);
    const character_sprite = this._character_sprites[index];
    if(!eval(window_data['Display Map Character'])){
        character_sprite._visibility = false;
        character_sprite._character.setOpacity(0);
        return;
    }else{
        this._chara.setActor(actor);
        this._chara.setDirection(eval(window_data['Character Direction']) || 2);
        this._chara._screenX = rect.x + (eval(window_data['Character X']) || 0);
        this._chara._screenY = rect.y + (eval(window_data['Character X']) || 0);
        this._chara._off_screenX = eval(window_data['Character X']) || 0;
        this._chara._off_screenY = eval(window_data['Character Y']) || 0;
        character_sprite.scale.x = eval(window_data['Character Scale X']) || 0;
        character_sprite.scale.y = eval(window_data['Character Scale Y']) || 0;
        character_sprite._visibility = true;
        character_sprite._character.setOpacity(255);
    }
}

WindowMenu_ActorSelector.prototype.displayBattler = function(rect, index, actor){
    const window_data = this._window_data;
    if(!this._battler_sprites[index])this.createBattlerSprite(index);
    const battler_sprite = this._battler_sprites[index];
    if(!eval(window_data['Display Battler'])){
        battler_sprite._visibility = false;
        return;
    }else{
        const hx = eval(window_data['Battler X']) || 0;
        const hy = eval(window_data['Battler Y']) || 0;
        battler_sprite.setHome(rect.x + hx, rect.y + hy);
        battler_sprite._motionLoaded = window_data['Battler Motion'];
        battler_sprite.startMotion(battler_sprite._motionLoaded);
        battler_sprite._offset_x = hx;
        battler_sprite._offset_y = hy;
        battler_sprite.scale.x = eval(window_data['Battler Scale X']);
        battler_sprite.scale.y = eval(window_data['Battler Scale Y']);
        battler_sprite._visibility = true;
        battler_sprite._motionCD = 0;
    }
}

WindowMenu_ActorSelector.prototype.refreshSprites = function(){
    this.clearSprites();
    this.createCharacterSprites();
    this.createBattlerSprites();
}

WindowMenu_ActorSelector.prototype.refresh = function(){
    this.refreshSprites();
    Window_Selectable.prototype.refresh.call(this, ...arguments);
}

function Scene_SynrecMenuPreload(){
    this.initialize(...arguments);
}

Scene_SynrecMenuPreload.prototype = Object.create(Scene_Base.prototype);
Scene_SynrecMenuPreload.prototype.constructor = Scene_SynrecMenuPreload;

Scene_SynrecMenuPreload.prototype.initialize = function(){
    Scene_Base.prototype.initialize.call(this);
    if(this.checkMenu()){
        this._rsvp_exit = true;
        this.clearSceneStack();
        return;
    }
    this._menu_data = $gameTemp.openedMenu();
    this.setupPreloadGfx();
}

Scene_SynrecMenuPreload.prototype.checkMenu = function(){
    const current_menu = $gameTemp.openedMenu();
    return !current_menu;
}

Scene_SynrecMenuPreload.prototype.clearSceneStack = function(){
    const scene_stack = SceneManager._stack;
    for(let i = 0; i < scene_stack.length; i++){
        const func = scene_stack[i];
        if(
            func == Scene_SynrecMenuPreload ||
            func == Scene_SynrecMenu
        ){
            scene_stack.splice(i, 1);
            i--;
        }
    }
}

Scene_SynrecMenuPreload.prototype.setupPreloadGfx = function(){
    const preload_pictures = [];
    const UI = $gameTemp.openedMenu();
    const backgrounds = UI['Backgrounds'];
    if(Array.isArray(backgrounds)){
        backgrounds.forEach((background)=>{
            const bitmap_name = background['File'];
            if(bitmap_name){
                preload_pictures.push(bitmap_name);
            }
        })
    }
    const back_gfxs = UI['Back Graphics'];
    if(Array.isArray(back_gfxs)){
        back_gfxs.forEach((background)=>{
            const bitmap_name = background['File'];
            if(bitmap_name){
                preload_pictures.push(bitmap_name);
            }
        })
    }
    const foregrounds = UI['Foregrounds'];
    if(Array.isArray(foregrounds)){
        foregrounds.forEach((foreground)=>{
            const bitmap_name = foreground['File'];
            if(bitmap_name){
                preload_pictures.push(bitmap_name);
            }
        })
    }
    const fore_gfxs = UI['Fore Graphics'];
    if(Array.isArray(fore_gfxs)){
        fore_gfxs.forEach((foreground)=>{
            const bitmap_name = foreground['File'];
            if(bitmap_name){
                preload_pictures.push(bitmap_name);
            }
        })
    }
    const basic_windows = UI['Basic Windows'];
    if(Array.isArray(basic_windows)){
        basic_windows.forEach((window_config)=>{
            const picture_draws = window_config['Draw Pictures'];
            if(Array.isArray(picture_draws)){
                picture_draws.forEach((pic_config)=>{
                    const pic_name = pic_config['Picture'];
                    preload_pictures.push(pic_name);
                })
            }
        })
    }
    const selc_window = UI['Selection Window'];
    if(!selc_window){
        throw new Error(`No selection window setup.`);
    }
    const selc_opts = selc_window['Selection Options']
    if(Array.isArray(selc_opts)){
        selc_opts.forEach((option)=>{
            const pics = option['Pictures'] || [];
            if(Array.isArray(pics)){
                pics.forEach((pic)=>{
                    if(pic){
                        preload_pictures.push(pic);
                    }
                })
            }
            const static_gfx = option['Static Graphic'];
            if(static_gfx){
                const pic_name = static_gfx['File'];
                if(pic_name){
                    preload_pictures.push(pic_name);
                }
            }
            const anim_gfx = option['Animated Graphic'];
            if(anim_gfx){
                const pic_name = anim_gfx['File'];
                if(pic_name){
                    preload_pictures.push(pic_name);
                }
            }
            const button = option['Scene Button'];
            if(button){
                const cold_gfx = button['Cold Graphic'];
                if(cold_gfx){
                    const pic_name = cold_gfx['File'];
                    if(pic_name){
                        preload_pictures.push(pic_name);
                    }
                }
                const hot_gfx = button['Hot Graphic'];
                if(hot_gfx){
                    const pic_name = hot_gfx['File'];
                    if(pic_name){
                        preload_pictures.push(pic_name);
                    }
                }
            }
        })
    }
    this._preload_pictures = preload_pictures;
    this._load_index = 0;
}

Scene_SynrecMenuPreload.prototype.create = function(){
    Scene_Base.prototype.create.call(this);
    if(!this._rsvp_exit){
        this.createBackgrounds();
    }else{
        this.createBasicBackground();
    }
}

Scene_SynrecMenuPreload.prototype.createBackgrounds = function(){
    const scene = this;
    const UI = $gameTemp.openedMenu();
    const backgrounds = UI['Preload Backgrounds'];
    if(!Array.isArray(backgrounds)){
        return this._backgrounds = [];
    }
    const sprites = [];
    const base_bg = new Sprite();
    base_bg.bitmap = SceneManager.synMenuBackgroundBitmap();
    this.addChild(base_bg);
    sprites.push(base_bg);
    backgrounds.forEach((bg_config)=>{
        const sprite = new Sprite();
        sprite._blt_nme = bg_config['Picture'];
        sprite._px = eval(bg_config['X']);
        sprite._py = eval(bg_config['Y']);
        sprite._sw = eval(bg_config['Width']);
        sprite._sh = eval(bg_config['Height']);
        sprite.aliasUpdt = sprite.update;
        sprite.update = function(){
            this.aliasUpdt.call(this);
            if(this._blt_nme){
                this.bitmap = ImageManager.loadPicture(this._blt_nme);
            }
            const x = this._px || 0;
            const y = this._py || 0;
            const w = this._sw || 1;
            const h = this._sh || 1;
            this.setFrame(x,y,w,h);
        }
        scene.addChild(sprite);
        sprites.push(sprite);
    })
    this._backgrounds = sprites;
}

Scene_SynrecMenuPreload.prototype.createBasicBackground = function(){
    const base_bg = new Sprite();
    base_bg.bitmap = SceneManager.synMenuBackgroundBitmap();
    this.addChild(base_bg);
}

Scene_SynrecMenuPreload.prototype.update = function(){
    if(this._rsvp_exit && !SceneManager.isSceneChanging()){
        SceneManager.pop();
        return;
    }
    if(SceneManager.isSceneChanging())return;
    Scene_Base.prototype.update.call(this);
    this.updateLoadGraphics();
    this.updateLoadMenu();
}

Scene_SynrecMenuPreload.prototype.updateLoadGraphics = function(){
    if(this._rsvp_exit || SceneManager.isSceneChanging())return;
    if(!ImageManager.isReady())return;
    for(let i = 0; i < 5; i++){
        const bitmap_name = this._preload_pictures[this._load_index];
        if(!bitmap_name)break;
        ImageManager.loadPicture(bitmap_name);
        this._load_index++;
    }
}

Scene_SynrecMenuPreload.prototype.updateLoadMenu = function(){
    if(this._rsvp_exit || SceneManager.isSceneChanging())return;
    if(!ImageManager.isReady())return;
    if(this._preload_pictures.length <= 0){
        return SceneManager.push(Scene_SynrecMenu);
    }
    const ratio = this._load_index / this._preload_pictures.length;
    if(ratio >= 1){
        SceneManager.push(Scene_SynrecMenu);
    }
}

function Scene_SynrecMenu(){
    this.initialize(...arguments);
}

Scene_SynrecMenu.prototype = Object.create(Scene_Base.prototype);
Scene_SynrecMenu.prototype.constructor = Scene_SynrecMenu;

Scene_SynrecMenu.prototype.initialize = function(){
    const preload_scene = Scene_SynrecMenuPreload;
    let check = true;
    while(check){
        const index = SceneManager._stack.indexOf(preload_scene);
        if(index >= 0){
            SceneManager._stack.splice(index, 1);
        }
        check = false;
    }
    Scene_Base.prototype.initialize.call(this);
}

Scene_SynrecMenu.prototype.reloadScene = function(data){
    this._static_selc = -1;
    this._anim_selc = -1;
    this._video_selc = -1;
    this._saved_actor_index = -1;
    const UI = $gameTemp.openedMenu();
    if(!data)return;
    if(data['Identifier Name'] != UI['Identifier Name'])return false;
    this.children.forEach((child)=>{
        if(child.parent){
            child.parent.removeChild(child);
        }
    })
    const menu_name = data['Identifier Name'];
    const new_menu = Syn_MenuBuildr.MENU_CONFIGURATIONS.find((menu_config)=>{
        return menu_config['Identifier Name'] == menu_name;
    })
    $gameTemp._current_menu = JsonEx.makeDeepCopy(new_menu);
    this.initialize();
    this.create();
    return true;
}

Scene_SynrecMenu.prototype.playVideo = function(name, label, loop, x, y, w, h){
    const video_layer = this._video_layer;
    if(video_layer){
        const rect = new Rectangle(x, y, w, h);
        video_layer.startVideo(name, label, loop, rect);
    }
}

Scene_SynrecMenu.prototype.stopVideo = function(label){
    if(!label)return;
    const video_layer = this._video_layer;
    if(video_layer){
        video_layer.endVideoByLabel(label);
    }
}

Scene_SynrecMenu.prototype.create = function(){
    this.createBlackScreen();
    this.executeScripts();
    this.createBackgrounds();
    this.createBackgfx();
    this.createSelectionVideo();
    this.createVideoLayer();
    this.createStaticGfx();
    this.createWindowLayer();
    this.createWindows();
    this.createEventHandlers();
    this.createAnimGfx();
    this.createForegfx();
    this.createForegrounds();
}

Scene_SynrecMenu.prototype.createBlackScreen = function(){
    const sprite = new Sprite();
    sprite.bitmap = new Bitmap(Graphics.width, Graphics.height);
    sprite.bitmap.fillRect(0, 0, Graphics.width, Graphics.height, "#000000");
    this.addChild(sprite);
    this._blackScreen = sprite;
}

Scene_SynrecMenu.prototype.executeScripts = function(){
    const scene = this;
    const UI = $gameTemp.openedMenu();
    const scripts = UI['On Load Script Calls'];
    scripts.forEach((script)=>{
        try{
            eval(script);
        }catch(e){
            console.warn(`Failed to execute script: ${script}`);
            console.error(e);
        }
    })
}

Scene_SynrecMenu.prototype.clearButtons = function(){
    if(!Array.isArray(this._scene_buttons)){
        this._scene_buttons = [];
        return;
    }
    for(let i = 0; i < this._scene_buttons.length; i++){
        const btn = this._scene_buttons[i];
        if(btn){
            if(btn.parent){
                btn.parent.removeChild(btn);
            }
            if(btn.destroy){
                btn.destroy();
            }
            this._scene_buttons.splice(i, 1);
            i--;
        }
    }
    this._scene_buttons = this._scene_buttons.filter(Boolean);
}

Scene_SynrecMenu.prototype.createBackgrounds = function(){
    const scene = this;
    const UI = $gameTemp.openedMenu();
    const data = UI['Backgrounds'];
    const arr = [];
    const base_bg = new Sprite();
    base_bg.bitmap = SceneManager.synMenuBackgroundBitmap();
    this.addChild(base_bg);
    arr.push(base_bg);
    data.forEach((gfx)=>{
        const sprite = new Sprite_SynMenuStaticGfx(gfx);
        scene.addChild(sprite);
        arr.push(sprite);
    })
    this._backgrounds = arr;
}

Scene_SynrecMenu.prototype.createBackgfx = function(){
    const scene = this;
    const UI = $gameTemp.openedMenu();
    const data = UI['Back Graphics'];
    const arr = [];
    data.forEach((gfx)=>{
        const sprite = new Sprite_SynMenuAnimGfx(gfx);
        scene.addChild(sprite);
        arr.push(sprite);
    })
    this._back_gfxs = arr;
}

Scene_SynrecMenu.prototype.createSelectionVideo = function(){
    const video_sprite = new PIXI.Sprite();
    this.addChild(video_sprite);
    this._selection_video = video_sprite;
}

Scene_SynrecMenu.prototype.createVideoLayer = function(){
    const video_layer = new SpriteSynrec_VideoLayer();
    this.addChild(video_layer);
    this._video_layer = video_layer;
}

Scene_SynrecMenu.prototype.createStaticGfx = function(){
    const sprite = new Sprite_SynMenuStaticGfx();
    this.addChild(sprite);
    this._select_static_gfx = sprite;
}

Scene_SynrecMenu.prototype.createWindows = function(){
    this.createSelectionWindow();
    this.createSelectionDataWindows();
    this.createActorSelectionWindow();
    this.createActorDataWindows();
    this.createBasicWindows();
}

Scene_SynrecMenu.prototype.createSelectionWindow = function(){
    this._scene_buttons = [];
    const UI = $gameTemp.openedMenu();
    const data = UI['Selection Window'];
    if(!data){
        throw new Error(`No selection window setup. Check menu setup.`);
    }
    const window = new Window_SynMenuSelc(data);
    window.setHandler(`ok`, this.doSelectAction.bind(this));
    window.setHandler(`cancel`, this.inputCloseMenu.bind(this));
    if(eval(UI['Open Effect'])){
        window.openness = 0;
        window.open();
    }
    this.addWindow(window);
    this._selection_window = window;
}

Scene_SynrecMenu.prototype.createSelectionDataWindows = function(){
    const UI = $gameTemp.openedMenu();
    const data = UI['Selection Data Windows'];
    if(!Array.isArray(data))return;
    const scene = this;
    const windows = [];
    data.forEach((config)=>{
        const window = new Window_SynMenuSelcData(config);
        window._select_window = scene._selection_window;
        scene.addWindow(window);
        windows.push(window);
    })
    this._data_windows = windows;
}

Scene_SynrecMenu.prototype.createActorSelectionWindow = function(){
    const UI = $gameTemp.openedMenu();
    const data = UI['Actor Selection Window'];
    const list = $gameParty.allMembers();
    const window = new WindowMenu_ActorSelector(data, list);
    window.refresh();
    window.deactivate();
    window.select(0);
    window.setHandler('ok', this.confirmSelectActor.bind(this));
    window.setHandler('cancel', this.cancelSelectActor.bind(this));
    this.addWindow(window);
    this._actor_selection_window = window;
}

Scene_SynrecMenu.prototype.createActorDataWindows = function(){
    const scene = this;
    const UI = $gameTemp.openedMenu();
    const selc_window = this._actor_selection_window;
    const actor_winodws = UI['Actor Data Windows'];
    const windows = [];
    actor_winodws.forEach((config)=>{
        const window = new WindowMenu_ActorData(config);
        window._selc_window = selc_window;
        scene.addWindow(window);
        windows.push(window);
    })
    this._actor_data_windows = windows;
}

Scene_SynrecMenu.prototype.createBasicWindows = function(){
    const UI = $gameTemp.openedMenu();
    const data = UI['Basic Windows'];
    if(!Array.isArray(data))return;
    const scene = this;
    const windows = [];
    data.forEach((config)=>{
        const window = new Window_SynMenuBasic(config);
        scene.addWindow(window);
        windows.push(window);
    })
    this._basic_windows = windows;
}

Scene_SynrecMenu.prototype.createEventHandlers = function(){
    const scene = this;
    scene.createPictures();
    this._event_handler = new Game_Interpreter();
    const msg_w = Graphics.boxWidth;
    const msg_h = Window_Base.prototype.fittingHeight(4);
    const msg_x = (Graphics.boxWidth * 0.5) - (msg_w * 0.5);
    const msg_y = 0;
    const rect_text = new Rectangle(msg_x, msg_y, msg_w, msg_h);
    this._messageWindow = new Window_Message(rect_text);
    if(Utils.RPGMAKER_NAME == 'MZ'){
        this._messageWindow.subWindows = function(){
            return [
                this._goldWindow,
                this._choiceListWindow,
                this._numberInputWindow,
                this._eventItemWindow,
                this._nameBoxWindow
            ];
        }
        this._messageWindow.createSubWindows = function(){
            const gold_rect = Scene_Message.prototype.goldWindowRect();
            this._goldWindow = new Window_Gold(gold_rect);
            this._goldWindow.openness = 0;
            this._choiceListWindow = new Window_ChoiceList();
            this._numberInputWindow = new Window_NumberInput();
            const item_rect = Scene_Message.prototype.eventItemWindowRect();
            this._eventItemWindow = new Window_EventItem(item_rect);
            this._nameBoxWindow = new Window_NameBox();
            this._nameBoxWindow.setMessageWindow(this);
            this._choiceListWindow.setMessageWindow(this);
            this._numberInputWindow.setMessageWindow(this);
            this._eventItemWindow.setMessageWindow(this);
        }
        this._messageWindow.createSubWindows();
        this._messageWindow.close();
    }
    this.addWindow(this._messageWindow);
    if(this._messageWindow.subWindows){
        this._messageWindow.subWindows().forEach((window)=>{
            scene.addWindow(window);
        });
    }
    const rect_scroll = new Rectangle(0, 0, Graphics.width, Graphics.height);
    this._scrollTextWindow = new Window_ScrollText(rect_scroll);
    this.addWindow(this._scrollTextWindow);
}

Scene_SynrecMenu.prototype.createPictures = function(){
    if(Utils.RPGMAKER_NAME == 'MZ'){
        const rect = new Rectangle(0, 0, Graphics.width, Graphics.height);
        this._pictureContainer = new Sprite();
        this._pictureContainer.setFrame(rect.x, rect.y, rect.width, rect.height);
        for (let i = 1; i <= $gameScreen.maxPictures(); i++) {
            this._pictureContainer.addChild(new Sprite_Picture(i));
        }
        this.addChild(this._pictureContainer);
    }else{
        const width = Graphics.boxWidth;
        const height = Graphics.boxHeight;
        const x = (Graphics.width - width) / 2;
        const y = (Graphics.height - height) / 2;
        this._pictureContainer = new Sprite();
        this._pictureContainer.setFrame(x, y, width, height);
        for (let i = 1; i <= $gameScreen.maxPictures(); i++) {
            this._pictureContainer.addChild(new Sprite_Picture(i));
        }
        this.addChild(this._pictureContainer);
    }
}

Scene_SynrecMenu.prototype.createAnimGfx = function(){
    const sprite = new Sprite_SynMenuAnimGfx();
    this.addChild(sprite);
    this._select_anim_gfx = sprite;
}

Scene_SynrecMenu.prototype.createForegfx = function(){
    const scene = this;
    const UI = $gameTemp.openedMenu();
    const data = UI['Fore Graphics'];
    const arr = [];
    data.forEach((gfx)=>{
        const sprite = new Sprite_SynMenuAnimGfx(gfx);
        scene.addChild(sprite);
        arr.push(sprite);
    })
    this._fore_gfxs = arr;
}

Scene_SynrecMenu.prototype.createForegrounds = function(){
    const scene = this;
    const UI = $gameTemp.openedMenu();
    const data = UI['Foregrounds'];
    const arr = [];
    data.forEach((gfx)=>{
        const sprite = new Sprite_SynMenuStaticGfx(gfx);
        scene.addChild(sprite);
        arr.push(sprite);
    })
    this._foregrounds = arr;
}

Scene_SynrecMenu.prototype.startActorSelect = function(){
    this._actor_selection_window.open();
    this._actor_selection_window.activate();
    this._selection_window.deactivate();
}

Scene_SynrecMenu.prototype.doSelectAction = function(){
    const actor = this._scene_actor;
    const selc_window = this._selection_window;
    const option_data = selc_window.currentSelection();
    if(!option_data){
        SoundManager.playBuzzer();
        selc_window.activate();
        return;
    }
    if(!selc_window.meetSelectRequirements()){
        SoundManager.playBuzzer();
        selc_window.activate();
        return;
    }
    if(eval(option_data['Require Actor Select'])){
        if(!actor){
            this.startActorSelect();
            return;
        }
    }
    try{
        const event = eval(option_data['Event Execution']) || 0;
        if(event){
            this._rsvp_evnt = event;
            return;
        }
    }catch(e){
        selc_window.activate();
        console.error(e);
    }
    const code = option_data['Code Execution'];
    if(code){
        eval(code);
        selc_window.activate();
    }else{
        SoundManager.playBuzzer();
        selc_window.activate();
    }
    this._scene_actor = null;
}

Scene_SynrecMenu.prototype.inputCloseMenu = function(){
    const UI = $gameTemp.openedMenu();
    if(eval(UI['Disable Cancel Exit'])){
        this._selection_window.activate();
        return;
    }
    this.closeMenu();
}

Scene_SynrecMenu.prototype.closeMenu = function(){
    const mz_mode = Utils.RPGMAKER_NAME == 'MZ';
    const video_layer = this._video_layer;
    video_layer.endAllVideos();
    const selection_video = this._selection_video;
    if(selection_video){
        const texture = selection_video.texture;
        if(texture){
            if(mz_mode){
                if(texture.baseTexture.resource){
                    const source = texture.baseTexture.resource.source;
                    if(source){
                        source.loop = false;
                        source.muted = true;
                        source.autoplay = false;
                        source.currentTime = JsonEx.makeDeepCopy(source.duration);
                        if(mz_mode)source.pause();
                    }
                }
            }else{
                if(texture.baseTexture){
                    const source = texture.baseTexture.source;
                    if(source){
                        source.loop = false;
                        source.muted = true;
                        source.autoplay = false;
                        source.currentTime = JsonEx.makeDeepCopy(source.duration);
                        if(mz_mode)source.pause();
                    }
                }
            }
            selection_video.texture = null;
            texture.destroy();
        }
    }
    this._scene_actor = null;
    $gameTemp.closeMenu();
    SceneManager.pop();
}

Scene_SynrecMenu.prototype.confirmSelectActor = function(){
    const UI = $gameTemp.openedMenu();
    const actor_selc_window = this._actor_selection_window;
    const actor = actor_selc_window.actor();
    this._scene_actor = actor;
    $gameParty.setMenuActor(actor);
    if(!eval(UI['Always Show Actor Select'])){
        actor_selc_window.close();
    }
    actor_selc_window.deactivate();
    this._selection_window.open();
    this._selection_window.activate();
    this.doSelectAction();
}

Scene_SynrecMenu.prototype.cancelSelectActor = function(){
    const UI = $gameTemp.openedMenu();
    const actor_selc_window = this._actor_selection_window;
    if(!eval(UI['Always Show Actor Select'])){
        actor_selc_window.close();
    }
    actor_selc_window.deactivate();
    this._selection_window.open();
    this._selection_window.activate();
}

Scene_SynrecMenu.prototype.update = function(){
    Scene_Base.prototype.update.call(this);
    this.updateActorWindows();
    this.updateStaitcGfx();
    this.updateAnimGfx();
    this.updateVideo();
    if(this.updateEvent())return;
    this.updateDebugModifier();
    this.updateCodes();
}

Scene_SynrecMenu.prototype.updateActorWindows = function(){
    const selc_window = this._actor_selection_window;
    const selc_index = selc_window.index();
    const data_windows = this._actor_data_windows;
    if(this._saved_actor_index != selc_index){
        const actor = selc_window.actor();
        data_windows.forEach((window)=>{
            window.setActor(actor);
        })
        this._saved_actor_index = selc_index;
    }
}

Scene_SynrecMenu.prototype.updateStaitcGfx = function(){
    const selc_window = this._selection_window;
    const static_gfx = this._select_static_gfx
    if(!static_gfx)return;
    const selc_index = selc_window.index();
    if(this._static_selc === selc_index)return;
    this._static_selc = selc_index;
    if(!selc_window){
        static_gfx.visible = false;
        return;
    }
    const option_data = selc_window.currentSelection();
    if(!option_data){
        static_gfx.visible = false;
        return;
    }
    const gfx_data = option_data['Static Graphic'];
    console.log(gfx_data)
    if(gfx_data){
        static_gfx.setupGfx(gfx_data);
        static_gfx.visible = !!gfx_data['File'];
    }else{
        static_gfx.visible = false;
    }
}

Scene_SynrecMenu.prototype.updateAnimGfx = function(){
    const selc_window = this._selection_window;
    const anim_gfx = this._select_anim_gfx
    if(!anim_gfx)return;
    const selc_index = selc_window.index();
    if(this._anim_selc === selc_index)return;
    this._anim_selc = selc_index;
    if(!selc_window){
        anim_gfx.visible = false;
        return;
    }
    const option_data = selc_window.currentSelection();
    if(!option_data){
        anim_gfx.visible = false;
        return;
    }
    const gfx_data = option_data['Animated Graphic'];
    if(gfx_data){
        anim_gfx.setupGfx(gfx_data);
        anim_gfx.visible = !!gfx_data['File'];
    }else{
        anim_gfx.visible = false;
    }
}

Scene_SynrecMenu.prototype.updateVideo = function(){
    const mz_mode = Utils.RPGMAKER_NAME == 'MZ';
    const sprite = this._selection_video;
    const selc_window = this._selection_window;
    if(!sprite)return;
    const selc_index = selc_window.index();
    if(this._video_selc === selc_index){
        const texture = sprite.texture;
        if(texture){
            let check = true;
            if(mz_mode){
                if(!texture.baseTexture.resource){
                    check = false;
                }else if(texture.baseTexture.resource){
                    if(!texture.baseTexture.resource.source)check = false;
                }
            }else{
                if(!texture.baseTexture){
                    check = false;
                }else if(texture.baseTexture){
                    if(!texture.baseTexture.source)check = false;
                }
            }
            if(check){
                texture.update();
                if(sprite._need_resize > 0 && !isNaN(sprite._need_resize)){
                    sprite.width = texture.width;
                    sprite.height = texture.height;
                    sprite._need_resize--;
                }
                const source = mz_mode ? texture.baseTexture.resource.source : texture.baseTexture.source;
                if(source){
                    sprite.alpha = 1;
                    const is_end = source.currentTime >= source.duration;
                    if(is_end){
                        source.currentTime = 0;
                        source.play();
                    }
                }
            }
        }
        return
    }else{
        const texture = sprite.texture;
        if(texture){
            let check = true;
            if(mz_mode){
                if(!texture.baseTexture.resource)check = false;
            }else{
                if(!texture.baseTexture)check = false;
            }
            if(check){
                const source = mz_mode ? texture.baseTexture.resource.source : texture.baseTexture.source;
                if(source){
                    source.loop = false;
                    source.muted = true;
                    source.autoload = false;
                    source.autoplay = false;
                    source.currentTime = JsonEx.makeDeepCopy(source.duration);
                    if(mz_mode)source.pause();
                }
            }
            sprite.texture = null;
            texture.destroy();
        }
    }
    if(!selc_window){
        sprite.alpha = 0;
        return;
    }
    const option_data = selc_window.currentSelection();
    if(!option_data){
        sprite.alpha = 0;
        return;
    }
    this._video_selc = selc_index;
    const video_name = option_data['Video'];
    if(!video_name){
        sprite.alpha = 0;
        return;
    }else{
        sprite.alpha = 1;
    }
    const src = `movies/${video_name}.webm`;
    const videoTexture = mz_mode ? new PIXI.Texture.from(src) : new PIXI.Texture.fromVideo(src);
    const source = mz_mode ? videoTexture.baseTexture.resource.source : videoTexture.baseTexture.source;
    source.autoload = true;
    source.autoplay = true;
    source.preload = 'auto';
    source.currentTime = 0;
    source.muted = false;
    source.loop = true;
    sprite.texture = videoTexture;
    const vx = eval(option_data['Video X']);
    const vy = eval(option_data['Video Y']);
    const vw = eval(option_data['Video Width']);
    const vh = eval(option_data['Video Height']);
    sprite.x = vx || 0;
    sprite.y = vy || 0;
    sprite.width = vw || videoTexture.width || Graphics.boxWidth;
    sprite.height = vh || videoTexture.height || Graphics.boxHeight;
    if(
        !vw ||
        !vh
    ){
        sprite._need_resize = 60;
    }
    if(mz_mode)source.play();
    sprite.alpha = 1;
}

Scene_SynrecMenu.prototype.updateEvent = function(){
    if(this._rsvp_evnt){
        const event = $dataCommonEvents[this._rsvp_evnt];
        if(event){
            this._event_handler.setup(event.list);
            if(event.list.length <= 0){
                this.activateSelcWindow();
            }
            this._reactivate_on_false = true;
        }
        this._rsvp_evnt = undefined;
    }
    this._event_handler.update();
    if(
        this._event_handler.isRunning() ||
        $gameMessage.isBusy() ||
        this._messageWindow.isOpen() ||
        this._scrollTextWindow.visible
    ){
        delete this._reactivate_on_false;
        this._event_busy = true;
        this.deactivateSelcWindow();
        return true;
    }else if(this._event_busy || this._reactivate_on_false){
        this._event_busy = false;
        this.activateSelcWindow();
    }
    return false;
}

Scene_SynrecMenu.prototype.updateDebugModifier = function(){
    if(Input.isTriggered(Syn_MenuBuildr.EDITOR_ACCESS_BUTTON) && $gameTemp.isPlaytest()){
        const w = Graphics.boxWidth;
        const h = Graphics.boxHeight;
        const editor_window = window.open(`/menu_builder/Synrec_MenuEditor.html`, "_blank", `width=${w},height=${h}`);
        this._editor_window = editor_window;
    }
}

Scene_SynrecMenu.prototype.deactivateSelcWindow = function(){
    this._selection_window.deactivate();
    this._selection_window.hide();
    this._data_windows.forEach((window)=>{
        window.hide();
    })
    this._basic_windows.forEach((window)=>{
        window.hide();
    })
    this._select_static_gfx.visible = false;
    this._select_anim_gfx.visible = false;
}

Scene_SynrecMenu.prototype.activateSelcWindow = function(){
    this._selection_window.activate();
    this._selection_window.show();
    this._data_windows.forEach((window)=>{
        window.show();
    })
    this._basic_windows.forEach((window)=>{
        window.show();
    })
    this._select_static_gfx.visible = true;
    this._select_anim_gfx.visible = true;
}

Scene_SynrecMenu.prototype.updateCodes = function(){
    const menu = $gameTemp.openedMenu();
    if(!menu)return;
    const codes = menu['Update Codes'] || [];
    codes.forEach((code)=>{
        try{
            eval(code);
        }catch(e){
            console.error("FAILED TO EXECUTE CODE!");
            console.error(e);
        }
    })
}
