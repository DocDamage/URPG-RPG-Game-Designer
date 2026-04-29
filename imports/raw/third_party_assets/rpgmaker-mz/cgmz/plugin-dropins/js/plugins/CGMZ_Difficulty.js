/*:
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/difficulty/
 * @target MZ
 * @base CGMZ_Core
 * @orderAfter CGMZ_Core
 * @orderAfter CGMZ_VehicleEncounters
 * @plugindesc Add a difficulty system to your game
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
 * Description: Use this plugin to add a difficulty select with many different
 * customization settings created by you. The difficulty select scene can be
 * accessed from anywhere and the difficulty setting is available via script
 * call for eventing custom difficulty differences on maps.
 * ----------------------------------------------------------------------------
 * Documentation:
 * ------------------------------Images----------------------------------------
 * Using default resolution, the image size which fits best is 446px wide.
 * Images will be automatically scaled if they are wider than the window,
 * and images will be automatically centered if they are not as wide as the
 * window.
 * ------------------------Rating Parameter------------------------------------
 * The difficulty's rating parameter is a number meant to represent the
 * difficulty in number form. It is not displayed anywhere. It is used in the
 * Get Difficulty Plugin Command, for use in eventing custom difficulty
 * differences in your game.
 * -----------------------Default Difficulty-----------------------------------
 * The default difficulty must be the name of a difficulty you have set up.
 * If you mistype the name or the difficulty name does not exist it could
 * cause issues.
 * ---------------------------Saved Games--------------------------------------
 * This plugin partially supports saved games. Difficulty settings other than
 * the difficulty name can be changed and are supported in saved games. The
 * name is saved and changes to difficulty name may behave weird in saved
 * games.
 * -------------------------Plugin Commands------------------------------------
 * This plugin supports the following plugin commands:
 * • Set Difficulty
 * Sets the difficulty of the game to the given value
 *
 * • Get Difficulty
 * Stores the difficulty rating of the current difficulty in a variable
 *
 * • Discover Difficulty
 * Sets the discovered status of the difficulty
 *
 * • Enable Difficulty
 * Sets the enabled status of the difficulty
 *
 * • Reset Difficulty
 * Resets the difficulty to the default difficulty
 *
 * • Call Scene
 * Calls the difficulty select scene
 *
 * • Check Playthrough Difficulty
 * Stores the hardest/easiest difficulty's rating parameter into a variable,
 * for use in eventing.
 * ---------------------------JavaScript---------------------------------------
 * To call the difficulty scene with JavaScript, use the following:
 * SceneManager.push(CGMZ_Scene_Difficulty);
 * ----------------------------Required Plugin---------------------------------
 * Please note that all [CGMZ] plugins require [CGMZ] Core to be installed
 * above them in the plugin manager. You can download it from my website:
 * https://www.caspergaming.com/plugins/cgmz/core/
 * -----------------------------Filename---------------------------------------
 * The filename of this plugin's JavaScript file MUST be CGMZ_Difficulty.js
 * This is what it comes as when downloaded. The filename is used to load
 * parameters and execute plugin commands. If you change it, things will begin
 * behaving incorrectly and your game will probably crash. Please do not
 * rename the js file.
 * --------------------------Latest Version------------------------------------
 * Hi all, this latest version adds support in this plugin to respect the
 * parameter Add Space After Label from [CGMZ] Core. This means you no longer
 * need to add the space after labels manually yourself if your language uses
 * spaces after label text (such as for English).
 *
 * Version 1.5.1
 * - Added support for automatic space after label text from [CGMZ] Core
 *
 * @command Set Difficulty
 * @desc Forcibly sets the difficulty
 *
 * @arg difficulty
 * @text Difficulty
 * @desc Name of the difficulty (exact capitalization required) to set
 *
 * @command Get Difficulty
 * @desc Get the difficulty rating in a variable - for use in eventing
 *
 * @arg variableId
 * @text Variable ID
 * @default 0
 * @type variable
 * @desc Variable to store the difficulty rating in
 *
 * @command Call Scene
 * @desc Calls the Difficulty scene
 *
 * @command Discover Difficulty
 * @desc Set discover status of difficulty
 *
 * @arg Difficulty
 * @desc Name of the difficulty (exact capitalization required) to change
 *
 * @arg Discover
 * @default true
 * @type boolean
 * @desc Set if it is discovered or not
 *
 * @command Enable Difficulty
 * @desc Set enable status of difficulty
 *
 * @arg Difficulty
 * @desc Name of the difficulty (exact capitalization required) to change
 *
 * @arg Enable
 * @default true
 * @type boolean
 * @desc Set if it is enabled or disabled
 *
 * @command Reset Difficulty
 * @desc Resets the difficulty to the default
 *
 * @command Check Playthrough Difficulty
 * @desc Check the rating of the easiest/hardest difficulty that was selected in the playthrough
 *
 * @arg Type
 * @type select
 * @option Easiest
 * @option Hardest
 * @default Easiest
 * @desc Check for the hardest or easiest difficulty?
 *
 * @arg Variable
 * @type variable
 * @default 0
 * @desc The game variable to store the easiest/hardest selected difficulty's rating in
 *
 * @param Difficulty Options
 *
 * @param Difficulties
 * @parent Difficulty Options
 * @type struct<Difficulties>[]
 * @desc Set up difficulties here
 * @default []
 *
 * @param Default Difficulty
 * @parent Difficulty Options
 * @desc The name (case sensitive) of the difficulty to use at game start.
 *
 * @param Rating Variable
 * @type variable
 * @default 0
 * @desc A game variable that will be set to the current difficulty's rating value when changing difficulties
 *
 * @param Window Options
 *
 * @param Display Window Info
 * @parent Window Options
 * @type select[]
 * @option Name
 * @option Image
 * @option Enemy Stats
 * @option Enemy Gold
 * @option Enemy EXP
 * @option Encounter Rate
 * @option Escape Rate
 * @option Preemptive Rate
 * @option Surprise Rate
 * @option Buy Rate
 * @option Sell Rate
 * @option Description
 * @option Custom Line
 * @option Blank Line
 * @option Custom Space
 * @option Custom Header
 * @desc The information displayed and order it is in on display window
 * @default ["Name","Image","Enemy Stats","Enemy Gold","Enemy EXP","Encounter Rate","Escape Rate","Preemptive Rate","Surprise Rate","Description"]
 * 
 * @param Custom Space
 * @parent Window Options
 * @type number
 * @min 1
 * @default 6
 * @desc A custom amount of blank vertical space to show in the display window for the Custom Space display item.
 * 
 * @param Headers
 * @parent Window Options
 * @type text[]
 * @default []
 * @desc Custom headers to display in the window
 * 
 * @param Visible Commands
 * @parent Window Options
 * @type number
 * @min 1
 * @default 3
 * @desc This is the number of commands that will be visible in the difficulty select window without scrolling
 *
 * @param Auto Close Scene On Select
 * @parent Window Options
 * @type boolean
 * @default false
 * @desc If true, the difficulty select scene will close when the player selects a difficulty.
 *
 * @param List Window Right
 * @parent Window Options
 * @type boolean
 * @default false
 * @desc If true, difficulty select window will be on the right side of screen
 * 
 * @param Show Change
 * @parent Window Options
 * @type boolean
 * @default true
 * @desc If true, difficulty scene will show +/- changes. If false, will show raw percentages
 *
 * @param Text Options
 *
 * @param Label Text Color
 * @parent Text Options
 * @type color
 * @default 1
 * @desc The color of the label text
 *
 * @param Easy Color
 * @parent Text Options
 * @type color
 * @default 3
 * @desc The color of changes if easier
 *
 * @param Hard Color
 * @parent Text Options
 * @type color
 * @default 2
 * @desc The color of changes if harder
 *
 * @param Header Color 1
 * @parent Text Options
 * @type color
 * @default 1
 * @desc The color 1 in the header gradient
 *
 * @param Header Color 2
 * @parent Text Options
 * @type color
 * @default 0
 * @desc The color 2 in the header gradient
 *
 * @param Current Difficulty
 * @parent Text Options
 * @desc Text to describe current difficulty in difficulty select scene
 * @default Current:
 *
 * @param No Change Text
 * @parent Text Options
 * @desc Text to describe when there is no change
 * @default No Change
 *
 * @param Enemy Stats
 * @parent Text Options
 * @desc Text to describe enemy stat modifier in difficulty select scene
 * @default Enemy Stats: 
 *
 * @param Enemy Gold
 * @parent Text Options
 * @desc Text to describe enemy gold modifier in difficulty select scene
 * @default Enemy Gold: 
 *
 * @param Enemy Exp
 * @parent Text Options
 * @desc Text to describe enemy exp modifier in difficulty select scene
 * @default Enemy Exp: 
 *
 * @param Encounter Rate
 * @parent Text Options
 * @desc Text to describe encounter rate modifier in difficulty select scene
 * @default Encounter Rate: 
 *
 * @param Escape Rate
 * @parent Text Options
 * @desc Text to describe escape rate modifier in difficulty select scene
 * @default Escape Rate: 
 *
 * @param Preemptive Rate
 * @parent Text Options
 * @desc Text to describe preemptive rate modifier in difficulty select scene
 * @default Preemptive Rate: 
 *
 * @param Surprise Rate
 * @parent Text Options
 * @desc Text to describe surprise rate modifier in difficulty select scene
 * @default Surprise Rate: 
 *
 * @param Buy Rate
 * @parent Text Options
 * @desc Text to describe buy rate modifier in difficulty select scene
 * @default Item Buy Rate: 
 *
 * @param Sell Rate
 * @parent Text Options
 * @desc Text to describe sell rate modifier in difficulty select scene
 * @default Item Sell Rate: 
 *
 * @param Integrations
 *
 * @param Background Image
 * @parent Integrations
 * @desc [CGMZ] Scene Background preset id to use for background image
 *
 * @param Controls Window
 * @parent Integrations
 * @desc [CGMZ] Controls Window preset id to use for the difficulty scene
 *
 * @param Difficulty Window Settings
 * @parent Integrations
 * @desc [CGMZ] Window Settings preset id to use for the difficulty window
 *
 * @param List Window Settings
 * @parent Integrations
 * @desc [CGMZ] Window Settings preset id to use for the list window
 *
 * @param Current Difficulty Window Settings
 * @parent Integrations
 * @desc [CGMZ] Window Settings preset id to use for the current difficulty window
 *
 * @param Difficulty Window Background
 * @parent Integrations
 * @desc [CGMZ] Window Backgrounds preset id to use for the difficulty window
 *
 * @param List Window Background
 * @parent Integrations
 * @desc [CGMZ] Window Backgrounds preset id to use for the list window
 *
 * @param Current Difficulty Window Background
 * @parent Integrations
 * @desc [CGMZ] Window Backgrounds preset id to use for the current difficulty window
*/
/*~struct~Difficulties:
 * @param Name
 * @desc The name of the difficulty. Must be unique.
 *
 * @param Description
 * @type multiline_string
 * @desc The description of the difficulty.
 *
 * @param Rating
 * @type number
 * @min 0
 * @default 0
 * @desc A number rating for the difficulty, used to store difficulty in game variable (see documentation)
 *
 * @param Discovered
 * @type boolean
 * @default true
 * @desc Determines if new games start with this difficulty discovered
 *
 * @param Enabled
 * @type boolean
 * @default true
 * @desc Determines if new games start with this difficulty enabled
 *
 * @param Image
 * @type file
 * @dir img
 * @desc The description of the difficulty.
 *
 * @param Custom Info
 * @type struct<CustomInfo>[]
 * @default []
 * @desc The custom info lines to display in the difficulty scene
 *
 * @param Switch
 * @type switch
 * @default 0
 * @desc A switch that is automatically turned ON when the difficulty is selected and turned OFF when not selected
 *
 * @param Enemy Stat Modifier
 * @type number
 * @default 100
 * @min 0
 * @desc The percentage change (multiplicative) of base stats the enemy has (higher = harder)
 *
 * @param Enemy Gold Modifier
 * @type number
 * @default 100
 * @min 0
 * @desc The percentage change (multiplicative) of gold the enemy awards (higher = easier)
 *
 * @param Enemy Exp Modifier
 * @type number
 * @default 100
 * @min 0
 * @desc The percentage change (multiplicative) of experience the enemy awards (higher = easier)
 *
 * @param Encounter Rate Modifier
 * @type number
 * @default 100
 * @min 0
 * @desc The percentage change (multiplicative) of encounter rate (higher = harder)
 *
 * @param Escape Ratio Modifier
 * @type number
 * @default 100
 * @min 0
 * @desc The percentage change (multiplicative) of escape success chance (higher = easier)
 *
 * @param Preemptive Modifier
 * @type number
 * @default 100
 * @min 0
 * @desc The percentage change (multiplicative) of preemptive battle chance (higher = easier)
 *
 * @param Surprise Modifier
 * @type number
 * @default 100
 * @min 0
 * @desc The percentage change (multiplicative) of surprise battle chance (higher = harder)
 *
 * @param Buy Modifier
 * @type number
 * @default 100
 * @min 0
 * @desc The percentage change (multiplicative) of item prices in shops when buying (higher = harder)
 *
 * @param Sell Modifier
 * @type number
 * @default 100
 * @min 0
 * @desc The percentage change (multiplicative) of item prices in shops when selling (lower = harder)
*/
/*~struct~CustomInfo:
 * @param Label
 * @desc The label for the custom info
 *
 * @param Info
 * @desc The info part to display after the label
*/
/*:zh-CN
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/difficulty/
 * @target MZ
 * @base CGMZ_Core
 * @orderAfter CGMZ_Core
 * @orderAfter CGMZ_VehicleEncounters
 * @plugindesc 难度系统
 * @help
 * ============================================================================
 * 【使用条款】
 * 1、本插件可作商用或非商用。
 * 2、须注明插件作者"Casper Gaming"。
 * 3、须提供该插件的作者网站链接。
 * 4、最终使用条款以作者官网公告为准。https://www.caspergaming.com/terms-of-use/
 * ============================================================================
 * 【赞助支持】
 * 您可以登陆以下网站并对作者进行支持和赞助。
 * 然后获得作者和其插件的最新资讯，以及测试版插件的试用。
 * https://www.patreon.com/CasperGamingRPGM
 * ============================================================================
 * 【插件版本】V 1.5.1
 * ----------------------------------------------------------------------------
 * 【兼容性】仅测试作者所制作的插件
 * 【RM版本】RPG Maker MZ 1.10.0
 * ----------------------------------------------------------------------------
 * 【插件描述】
 *  创建多种你想要的自定义难度等级，调整敌人战斗力和战利品等参数。
 *  可以使用指令在游戏中随时切换难度。
 * ----------------------------------------------------------------------------
 * 【使用说明】
 * 一、图片
 * 设置显示难度等级的图片，建议宽度为446像素。
 * 如果图片太宽会自动缩放，如果图片宽度不够则会自动居中图片。
 *
 * 二、难度等级的相关设置。
 * 1、难度等级的参数是一个数值，用来获取插件中对应难度的设置，这个数值不会在任何地方显示。
 * 2、在部分插件指令中是使用"难度名称"来调用难度，设置和指令中使用的名称必须字符、大小写一致。
 *    如 Easy 和 EASY 因大小写不一致，会被视为不同的难度，使用没有相关设置的错误名称会使游戏出错。
 *
 * 三、支持已保存游戏：除了难度名称，支持其他所有的难度设置更改。
 *
 * 四、插件指令
 * 1. 指定难度（Set Difficulty）：将游戏直接设置为指定的难度。
 * 2. 获取难度变量（Get Difficulty）：获取当前难度等级的参数作为变量使用。
 *    如：根据难度不同，用变量作为条件，而获得不同品质的宝箱战利品。
 * 3. 打开难度界面（Call Scene）：打开难度界面，可以选择不同难度和查看难度信息。
 *    打开界面的JS命令为：SceneManager.push(CGMZ_Scene_Difficulty);
 * • Discover Difficulty
 * Sets the discovered status of the difficulty
 * • Enable Difficulty
 * Sets the enabled status of the difficulty
 * • Reset Difficulty
 * Resets the difficulty to the default difficulty
 * • Check Playthrough Difficulty
 * Stores the hardest/easiest difficulty's rating parameter into a variable,
 * for use in eventing.
 *
 * 五、文件名注意事项：
 *     本插件文件名为CGMZ_Difficulty.js. 因为引用了其他文件的参数，
 *     所以请勿修改本插件的文件名，否则会因读取数据失败而使游戏报错。
 * ----------------------------Required Plugin---------------------------------
 * Please note that all [CGMZ] plugins require [CGMZ] Core to be installed
 * above them in the plugin manager. You can download it from my website:
 * https://www.caspergaming.com/plugins/cgmz/core/
 * ---------------------------------------------------------------------------
 *【版本更新历史】
 * Hi all, this latest version adds support in this plugin to respect the
 * parameter Add Space After Label from [CGMZ] Core. This means you no longer
 * need to add the space after labels manually yourself if your language uses
 * spaces after label text (such as for English).
 *
 * Version 1.5.1
 * - Added support for automatic space after label text from [CGMZ] Core
 *
 * @command Set Difficulty
 * @text 设置难度
 * @desc 直接将游戏设置为指定的难度。
 *
 * @arg difficulty
 * @text 难度名称
 * @desc 输入需要设置的难度名称。难度名称要求与插件设置中名称的字符一致并区分大小写。
 *
 * @command Get Difficulty
 * @text 获取难度变量
 * @desc 获取难度等级的参数作为变量在游戏事件设置中使用。
 *
 * @arg variableId
 * @text 变量ID
 * @default 0
 * @type variable
 * @desc 指定一个变量ID，来获取难度等级的参数。
 *
 * @command Call Scene
 * @text 打开难度界面
 * @desc 打开一个可以选择难度和查看相关信息的界面。
 *
 * @command Discover Difficulty
 * @desc Set discover status of difficulty
 *
 * @arg Difficulty
 * @desc Name of the difficulty (exact capitalization required) to change
 *
 * @arg Discover
 * @default true
 * @type boolean
 * @desc Set if it is discovered or not
 *
 * @command Enable Difficulty
 * @desc Set enable status of difficulty
 *
 * @arg Difficulty
 * @desc Name of the difficulty (exact capitalization required) to change
 *
 * @arg Enable
 * @default true
 * @type boolean
 * @desc Set if it is enabled or disabled
 *
 * @command Reset Difficulty
 * @desc Resets the difficulty to the default
 *
 * @command Check Playthrough Difficulty
 * @desc Check the rating of the easiest/hardest difficulty that was selected in the playthrough
 *
 * @arg Type
 * @type select
 * @option Easiest
 * @option Hardest
 * @default Easiest
 * @desc Check for the hardest or easiest difficulty?
 *
 * @arg Variable
 * @type variable
 * @default 0
 * @desc The game variable to store the easiest/hardest selected difficulty's rating in
 *
 * @param Difficulty Options
 * @text 难度设置
 *
 * @param Difficulties
 * @text 难度
 * @parent Difficulty Options
 * @type struct<Difficulties>[]
 * @desc 设置你想要的难度等级。
 * @default []
 *
 * @param Default Difficulty
 * @text 默认难度
 * @parent Difficulty Options
 * @desc 输入作为游戏开始时默认难度的名称。
 *
 * @param Rating Variable
 * @type variable
 * @default 0
 * @desc A game variable that will be set to the current difficulty's rating value when changing difficulties
 *
 * @param Window Options
 * @text 界面设置
 *
 * @param Display Window Info
 * @text 界面显示的信息
 * @parent Window Options
 * @type select[]
 * @option Name
 * @option Image
 * @option Enemy Stats
 * @option Enemy Gold
 * @option Enemy EXP
 * @option Encounter Rate
 * @option Escape Rate
 * @option Preemptive Rate
 * @option Surprise Rate
 * @option Buy Rate
 * @option Sell Rate
 * @option Description
 * @option Custom Line
 * @option Blank Line
 * @option Custom Space
 * @option Custom Header
 * @desc 设置难度界面内显示的难度信息的种类。
 * @default ["Name","Image","Enemy Stats","Enemy Gold","Enemy EXP","Encounter Rate","Escape Rate","Preemptive Rate","Surprise Rate","Description"]
 * 
 * @param Custom Space
 * @parent Window Options
 * @type number
 * @min 1
 * @default 6
 * @desc A custom amount of blank vertical space to show in the display window for the Custom Space display item.
 * 
 * @param Headers
 * @parent Window Options
 * @type text[]
 * @default []
 * @desc Custom headers to display in the window
 * 
 * @param Visible Commands
 * @text 难度选项显示数
 * @parent Window Options
 * @type number
 * @min 1
 * @default 3
 * @desc 设置在难度选项框中显示多少个选项，超过的需要按上下滚动显示和选择。
 *
 * @param Auto Close Scene On Select
 * @text 选择后关闭难度界面
 * @parent Window Options
 * @type boolean
 * @default false
 * @desc Ture-选择难度之后，自动关闭难度选择界面。
 *
 * @param List Window Right
 * @parent Window Options
 * @type boolean
 * @default false
 * @desc If true, difficulty select window will be on the right side of screen
 * 
 * @param Show Change
 * @parent Window Options
 * @type boolean
 * @default true
 * @desc If true, difficulty scene will show +/- changes. If false, will show raw percentages
 *
 * @param Text Options
 * @text 文本设置
 *
 * @param Label Text Color
 * @text 标签文本的颜色
 * @parent Text Options
 * @type color
 * @default 1
 * @desc 设置标签文本的颜色。如：敌人属性倍率、经验获得赔率等描述。
 *
 * @param Easy Color
 * @parent Text Options
 * @type color
 * @default 3
 * @desc The color of changes if easier
 *
 * @param Hard Color
 * @parent Text Options
 * @type color
 * @default 2
 * @desc The color of changes if harder
 *
 * @param Header Color 1
 * @parent Text Options
 * @type color
 * @default 1
 * @desc The color 1 in the header gradient
 *
 * @param Header Color 2
 * @parent Text Options
 * @type color
 * @default 0
 * @desc The color 2 in the header gradient
 *
 * @param Current Difficulty
 * @text 当前难度的描述
 * @parent Text Options
 * @desc 设置关于当前难度等级标签的描述文本。
 * @default 当前难度:
 *
 * @param No Change Text
 * @parent Text Options
 * @desc Text to describe when there is no change
 * @default No Change
 *
 * @param Enemy Stats
 * @text 敌人属性倍率的描述
 * @parent Text Options
 * @desc 设置关于敌人属性倍率标签的描述。
 * @default 敌人属性倍率:
 *
 * @param Enemy Gold
 * @text 金币掉率的描述
 * @parent Text Options
 * @desc 设置关于击败敌人后获得的金币倍率标签的描述。
 * @default 金币掉率:
 *
 * @param Enemy Exp
 * @text 经验倍率的描述
 * @parent Text Options
 * @desc 设置关于击败敌人后获得的经验倍率标签的描述。
 * @default 经验倍率:
 *
 * @param Encounter Rate
 * @text 遇敌几率的描述
 * @parent Text Options
 * @desc 设置关于地图上遇敌几率标签的描述。
 * @default 遇敌几率: 
 *
 * @param Escape Rate
 * @text 逃跑成功率的描述
 * @parent Text Options
 * @desc 设置关于队伍在战斗中逃跑成功率标签的描述。
 * @default 逃跑成功率: 
 *
 * @param Preemptive Rate
 * @text 先发制人的描述
 * @parent Text Options
 * @desc 设置关于队伍遇敌时先发制人几率标签的描述。
 * @default 先发制人几率: 
 *
 * @param Surprise Rate
 * @text 被偷袭几率的描述
 * @parent Text Options
 * @desc 设置关于队伍遇敌时被突袭几率标签的描述。
 * @default 被偷袭几率: 
 *
 * @param Buy Rate
 * @parent Text Options
 * @desc Text to describe buy rate modifier in difficulty select scene
 * @default Item Buy Rate: 
 *
 * @param Sell Rate
 * @parent Text Options
 * @desc Text to describe sell rate modifier in difficulty select scene
 * @default Item Sell Rate: 
 *
 * @param Integrations
 *
 * @param Background Image
 * @parent Integrations
 * @desc [CGMZ] Scene Background preset id to use for background image
 *
 * @param Controls Window
 * @parent Integrations
 * @desc [CGMZ] Controls Window preset id to use for the difficulty scene
 *
 * @param Difficulty Window Settings
 * @parent Integrations
 * @desc [CGMZ] Window Settings preset id to use for the difficulty window
 *
 * @param List Window Settings
 * @parent Integrations
 * @desc [CGMZ] Window Settings preset id to use for the list window
 *
 * @param Current Difficulty Window Settings
 * @parent Integrations
 * @desc [CGMZ] Window Settings preset id to use for the current difficulty window
 *
 * @param Difficulty Window Background
 * @parent Integrations
 * @desc [CGMZ] Window Backgrounds preset id to use for the difficulty window
 *
 * @param List Window Background
 * @parent Integrations
 * @desc [CGMZ] Window Backgrounds preset id to use for the list window
 *
 * @param Current Difficulty Window Background
 * @parent Integrations
 * @desc [CGMZ] Window Backgrounds preset id to use for the current difficulty window
*/
/*~struct~Difficulties:zh-CN
 * @param Name
 * @text 难度名称
 * @type text
 * @desc 设置一个难度等级的名称，名称需要唯一，不能多个难度使用相同难度名称。
 *
 * @param Description
 * @text 难度描述
 * @type multiline_string
 * @desc 关于这个难度等级的描述。
 *
 * @param Rating
 * @text 难度参数
 * @type number
 * @min 0
 * @default 0
 * @desc 设置一个数值作为难度参数，当使用插件命令时将获取该数值作为变量在游戏中使用。
 *
 * @param Discovered
 * @type boolean
 * @default true
 * @desc Determines if new games start with this difficulty discovered
 *
 * @param Enabled
 * @type boolean
 * @default true
 * @desc Determines if new games start with this difficulty enabled
 *
 * @param Image
 * @text 图片
 * @type file
 * @dir img
 * @desc 选择作为难度内容标题的图片。
 *
 * @param Custom Info
 * @type struct<CustomInfo>[]
 * @default []
 * @desc The custom info lines to display in the difficulty scene
 *
 * @param Switch
 * @type switch
 * @default 0
 * @desc A switch that is automatically turned ON when the difficulty is selected and turned OFF when not selected
 *
 * @param Enemy Stat Modifier
 * @text 敌人属性设置
 * @type number
 * @default 100
 * @min 0
 * @desc 设置敌人整体属性的倍率。设置数值越高，难度越大。（越高越难）
 *
 * @param Enemy Gold Modifier
 * @text 金币掉落设置
 * @type number
 * @default 100
 * @min 0
 * @desc 设置战斗胜利后获得金币的倍率。设置数值越大，获得金币越多。（越高越容易）
 *
 * @param Enemy Exp Modifier
 * @text 经验倍率设置
 * @type number
 * @default 100
 * @min 0
 * @desc 设置战斗胜利后获得经验值的赔率。设置数值越大，获得经验越多。（越高越容易）
 *
 * @param Encounter Rate Modifier
 * @text 遇敌几率设置
 * @type number
 * @default 100
 * @min 0
 * @desc 设置在地图上遇到敌人（暗雷）的几率。设置数值越高，越容易遇到敌人。（越高越难）
 *
 * @param Escape Ratio Modifier
 * @text 逃跑成功率设置
 * @type number
 * @default 100
 * @min 0
 * @desc 战斗中逃跑成功率的设置。设置数值越高，越容易逃跑成功。（越高越容易）
 *
 * @param Preemptive Modifier
 * @text 先发制人几率设置
 * @type number
 * @default 100
 * @min 0
 * @desc 遭遇敌人时先发制人几率的设置。设置数值越高，越容易先发制人。（越高越容易）
 *
 * @param Surprise Modifier
 * @text 被偷袭几率设置
 * @type number
 * @default 100
 * @min 0
 * @desc 遭遇敌人时被偷袭几率的设置。设置数值越高，越容易被偷袭。（越高越难）
 *
 * @param Buy Modifier
 * @type number
 * @default 100
 * @min 0
 * @desc The percentage change (multiplicative) of item prices in shops when buying (higher = harder)
 *
 * @param Sell Modifier
 * @type number
 * @default 100
 * @min 0
 * @desc The percentage change (multiplicative) of item prices in shops when selling (lower = harder)
*/
/*~struct~CustomInfo:zh-CN
 * @param Label
 * @desc The label for the custom info
 *
 * @param Info
 * @desc The info part to display after the label
*/
/*:es
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/difficulty/
 * @target MZ
 * @base CGMZ_Core
 * @orderAfter CGMZ_Core
 * @orderAfter CGMZ_VehicleEncounters
 * @plugindesc Añade un sistema de dificultad a tu juego.
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
 * Compatibilidad: solo probado con mis complementos CGMZ.
 * Hecho para RPG Maker MZ 1.10.0
 * ----------------------------------------------------------------------------
 * Descripción: Usa este plugin para agregar una selección de dificultad con 
 * muchas configuraciones de personalización diferentes creadas por ti. Se 
 * puede acceder a la escena de selección de dificultad desde cualquier lugar y 
 * la configuración de dificultad está disponible a través de una llamada de 
 * script para eventos de diferencias de dificultad personalizadas en los
 * mapas.
 * ----------------------------------------------------------------------------
 * Documentación:
 * ----------------------------Imágenes----------------------------------------
 * Usando la resolución predeterminada, el tamaño de imagen que mejor se adapta 
 * es 446px de ancho. Las imágenes se escalarán automáticamente si son más 
 * anchas que la ventana y las imágenes se centrarán automáticamente si no son 
 * tan anchas como la ventana.
 * -------------------Parámetro de calificación--------------------------------
 * El parámetro de clasificación de dificultad es un número que representa la 
 * dificultad en forma de número. No se muestra en ninguna parte. Se usa en el 
 * comando de plugin Obtener dificultad, para usar en eventos de diferencias 
 * de dificultad personalizadas en tu juego.
 * ----------------------Dificultad predeterminada-----------------------------
 * La dificultad predeterminada debe ser el nombre de una dificultad que hayas 
 * configurado. Si escribes mal el nombre o el nombre de la dificultad no 
 * existe, podría causar problemas.
 * -------------------------Juegos Guardados-----------------------------------
 * Este complemento es compatible parcialmente con los juegos guardados. Los 
 * ajustes de dificultad que no sean el nombre de la dificultad se pueden 
 * cambiar y son compatibles con los juegos guardados. El nombre se guarda y 
 * los cambios en el nombre de la dificultad pueden comportarse de manera 
 * extraña en los juegos guardados.
 * ------------------------Comandos de Plugin----------------------------------
 * Este plugin admite los siguientes comandos de complemento:
 * • Establecer dificultad
 * Esto establecerá la dificultad del juego en el valor dado
 *
 * • Obtener dificultad
 * Obtendrá la calificación de dificultad de la dificultad actual y la
 * almacenará en una variable
 *
 * • Escena de llamada
 * Esto llamará a la escena de selección de dificultad
 *
 * • Discover Difficulty
 * Sets the discovered status of the difficulty
 *
 * • Enable Difficulty
 * Sets the enabled status of the difficulty
 *
 * • Reset Difficulty
 * Resets the difficulty to the default difficulty
 *
 * • Check Playthrough Difficulty
 * Stores the hardest/easiest difficulty's rating parameter into a variable,
 * for use in eventing.
 * ---------------------------JavaScript---------------------------------------
 * Para llamar a la escena de dificultad con JavaScript, usa lo siguiente:
 * SceneManager.push(CGMZ_Scene_Difficulty);
 * ----------------------------Required Plugin---------------------------------
 * Please note that all [CGMZ] plugins require [CGMZ] Core to be installed
 * above them in the plugin manager. You can download it from my website:
 * https://www.caspergaming.com/plugins/cgmz/core/
 * ------------------------Nombre del archivo----------------------------------
 * El nombre del archivo JavaScript de este plugin DEBE ser CGMZ_Difficulty.js
 * Esto es lo que aparece cuando se descarga. El nombre de archivo se utiliza 
 * para cargar parámetros y ejecutar comandos de plugins. Si lo cambias, las 
 * cosas comenzarán a comportarse incorrectamente y tu juego probablemente se 
 * bloquee. No cambies el nombre del archivo js.
 * --------------------------Latest Version------------------------------------
 * Hi all, this latest version adds support in this plugin to respect the
 * parameter Add Space After Label from [CGMZ] Core. This means you no longer
 * need to add the space after labels manually yourself if your language uses
 * spaces after label text (such as for English).
 *
 * Version 1.5.1
 * - Added support for automatic space after label text from [CGMZ] Core
 *
 * @command Set Difficulty
 * @text Establecer dificultad
 * @desc Establece la dificultad a la fuerza.
 *
 * @arg difficulty
 * @text Dificultad
 * @desc Nombre de la dificultad (se requiere mayúsculas exactas) para establecer.
 *
 * @command Get Difficulty
 * @text Obtener dificultad
 * @desc Obtenga la calificación de dificultad en una variable - para usar en eventos completos.
 *
 * @arg variableId
 * @text ID de Variable
 * @default 0
 * @type variable
 * @desc Variable en la cual almacenar la calificación de dificultad .
 *
 * @command Call Scene
 * @text Escena de llamada
 * @desc Llama a la escena de dificultad
 *
 * @command Discover Difficulty
 * @desc Set discover status of difficulty
 *
 * @arg Difficulty
 * @desc Name of the difficulty (exact capitalization required) to change
 *
 * @arg Discover
 * @default true
 * @type boolean
 * @desc Set if it is discovered or not
 *
 * @command Enable Difficulty
 * @desc Set enable status of difficulty
 *
 * @arg Difficulty
 * @desc Name of the difficulty (exact capitalization required) to change
 *
 * @arg Enable
 * @default true
 * @type boolean
 * @desc Set if it is enabled or disabled
 *
 * @command Reset Difficulty
 * @desc Resets the difficulty to the default
 *
 * @command Check Playthrough Difficulty
 * @desc Check the rating of the easiest/hardest difficulty that was selected in the playthrough
 *
 * @arg Type
 * @type select
 * @option Easiest
 * @option Hardest
 * @default Easiest
 * @desc Check for the hardest or easiest difficulty?
 *
 * @arg Variable
 * @type variable
 * @default 0
 * @desc The game variable to store the easiest/hardest selected difficulty's rating in
 *
 * @param Difficulty Options
 * @text Opciones de Dificultad
 *
 * @param Difficulties
 * @text Dificultades
 * @parent Difficulty Options
 * @type struct<Difficulties>[]
 * @desc Establecer dificultades aquí.
 * @default []
 *
 * @param Default Difficulty
 * @text Dificultad predeterminada
 * @parent Difficulty Options
 * @desc El nombre (distingue entre mayúsculas y minúsculas) de la dificultad que se usará al inicio del juego.
 *
 * @param Rating Variable
 * @type variable
 * @default 0
 * @desc A game variable that will be set to the current difficulty's rating value when changing difficulties
 *
 * @param Window Options
 * @text Opciones de ventana
 *
 * @param Display Window Info
 * @text Información de la ventana de visualización
 * @parent Window Options
 * @type select[]
 * @option Name
 * @option Image
 * @option Enemy Stats
 * @option Enemy Gold
 * @option Enemy EXP
 * @option Encounter Rate
 * @option Escape Rate
 * @option Preemptive Rate
 * @option Surprise Rate
 * @option Buy Rate
 * @option Sell Rate
 * @option Description
 * @option Custom Line
 * @option Blank Line
 * @option Custom Space
 * @option Custom Header
 * @desc La información que se muestra y el orden en que se encuentra en la ventana de visualización.
 * @default ["Name","Image","Enemy Stats","Enemy Gold","Enemy EXP","Encounter Rate","Escape Rate","Preemptive Rate","Surprise Rate","Description"]
 * 
 * @param Custom Space
 * @parent Window Options
 * @type number
 * @min 1
 * @default 6
 * @desc A custom amount of blank vertical space to show in the display window for the Custom Space display item.
 * 
 * @param Headers
 * @parent Window Options
 * @type text[]
 * @default []
 * @desc Custom headers to display in the window
 * 
 * @param Visible Commands
 * @text Comandos visibles
 * @parent Window Options
 * @type number
 * @min 1
 * @default 3
 * @desc Este es el número de comandos que serán visibles en la ventana de selección de dificultad sin desplazarse.
 *
 * @param Auto Close Scene On Select
 * @text Cerrar escena automáticamente al seleccionar
 * @parent Window Options
 * @type boolean
 * @default false
 * @desc Si es cierto, la escena de selección de dificultad se cerrará cuando el jugador seleccione una dificultad.
 *
 * @param List Window Right
 * @parent Window Options
 * @type boolean
 * @default false
 * @desc If true, difficulty select window will be on the right side of screen
 * 
 * @param Show Change
 * @parent Window Options
 * @type boolean
 * @default true
 * @desc If true, difficulty scene will show +/- changes. If false, will show raw percentages
 *
 * @param Text Options
 * @text Opciones de texto 
 *
 * @param Label Text Color
 * @text Color del texto de la etiqueta
 * @parent Text Options
 * @type color
 * @default 1
 * @desc El color del texto de la etiqueta.
 *
 * @param Easy Color
 * @parent Text Options
 * @type color
 * @default 3
 * @desc The color of changes if easier
 *
 * @param Hard Color
 * @parent Text Options
 * @type color
 * @default 2
 * @desc The color of changes if harder
 *
 * @param Header Color 1
 * @parent Text Options
 * @type color
 * @default 1
 * @desc The color 1 in the header gradient
 *
 * @param Header Color 2
 * @parent Text Options
 * @type color
 * @default 0
 * @desc The color 2 in the header gradient
 *
 * @param Current Difficulty
 * @text Dificultad actual
 * @parent Text Options
 * @desc Texto para describir la dificultad actual en dificultad seleccionar escena.
 * @default Current:
 *
 * @param No Change Text
 * @parent Text Options
 * @desc Text to describe when there is no change
 * @default No Change
 *
 * @param Enemy Stats
 * @text Estadísticas del enemigo
 * @parent Text Options
 * @desc Texto para describir el modificador de estadísticas del enemigo en la escena de selección de dificultad.
 * @default Enemy Stats: 
 *
 * @param Enemy Gold
 * @text Oro enemigo
 * @parent Text Options
 * @desc Texto para describir el modificador de oro enemigo en la escena de selección de dificultad.
 * @default Enemy Gold: 
 *
 * @param Enemy Exp
 * @text Experiencia de Enemigo
 * @parent Text Options
 * @desc Texto para describir el modificador de experiencia del enemigo en la escena de selección de dificultad.
 * @default Enemy Exp: 
 *
 * @param Encounter Rate
 * @text Tasa de Encuentro 
 * @parent Text Options
 * @desc Texto para describir el modificador de tasa de encuentro en dificultad seleccionar escena.
 * @default Encounter Rate: 
 *
 * @param Escape Rate
 * @text Tasa de Escape
 * @parent Text Options
 * @desc Texto para describir el modificador de tasa de escape en dificultad seleccionar escena.
 * @default Escape Rate: 
 *
 * @param Preemptive Rate
 * @text Tasa preferencial
 * @parent Text Options
 * @desc Texto para describir el modificador de tasa preventiva en la escena de selección de dificultad.
 * @default Preemptive Rate: 
 *
 * @param Surprise Rate
 * @text Tasa Sorpresa
 * @parent Text Options
 * @desc Texto para describir el modificador de tasa de sorpresa en la escena de selección de dificultad.
 * @default Surprise Rate: 
 *
 * @param Buy Rate
 * @parent Text Options
 * @desc Text to describe buy rate modifier in difficulty select scene
 * @default Item Buy Rate: 
 *
 * @param Sell Rate
 * @parent Text Options
 * @desc Text to describe sell rate modifier in difficulty select scene
 * @default Item Sell Rate: 
 *
 * @param Integrations
 *
 * @param Background Image
 * @parent Integrations
 * @desc [CGMZ] Scene Background preset id to use for background image
 *
 * @param Controls Window
 * @parent Integrations
 * @desc [CGMZ] Controls Window preset id to use for the difficulty scene
 *
 * @param Difficulty Window Settings
 * @parent Integrations
 * @desc [CGMZ] Window Settings preset id to use for the difficulty window
 *
 * @param List Window Settings
 * @parent Integrations
 * @desc [CGMZ] Window Settings preset id to use for the list window
 *
 * @param Current Difficulty Window Settings
 * @parent Integrations
 * @desc [CGMZ] Window Settings preset id to use for the current difficulty window
 *
 * @param Difficulty Window Background
 * @parent Integrations
 * @desc [CGMZ] Window Backgrounds preset id to use for the difficulty window
 *
 * @param List Window Background
 * @parent Integrations
 * @desc [CGMZ] Window Backgrounds preset id to use for the list window
 *
 * @param Current Difficulty Window Background
 * @parent Integrations
 * @desc [CGMZ] Window Backgrounds preset id to use for the current difficulty window
*/
/*~struct~Difficulties:es
 * @param Name
 * @text Nombre de Dificultad
 * @type text
 * @desc El nombre de la dificultad. Debe ser único.
 *
 * @param Description
 * @text Descripción
 * @type multiline_string
 * @desc La descripción de la dificultad.
 *
 * @param Rating
 * @text Clasificación
 * @type number
 * @min 0
 * @default 0
 * @desc Una calificación numérica para la dificultad, utilizada para almacenar la dificultad en la variable del juego (ver documentación).
 *
 * @param Discovered
 * @type boolean
 * @default true
 * @desc Determines if new games start with this difficulty discovered
 *
 * @param Enabled
 * @type boolean
 * @default true
 * @desc Determines if new games start with this difficulty enabled
 *
 * @param Image
 * @text Imagen
 * @type file
 * @dir img
 * @desc Selecciona la imagen que será el título del contenido de dificultad.
 *
 * @param Custom Info
 * @type struct<CustomInfo>[]
 * @default []
 * @desc The custom info lines to display in the difficulty scene
 *
 * @param Switch
 * @type switch
 * @default 0
 * @desc A switch that is automatically turned ON when the difficulty is selected and turned OFF when not selected
 *
 * @param Enemy Stat Modifier
 * @text Modificador de estadísticas del enemigo
 * @type number
 * @default 100
 * @min 0
 * @desc El cambio porcentual (multiplicativo) de las estadísticas base que tiene el enemigo (más alto = más difícil).
 *
 * @param Enemy Gold Modifier
 * @text Modificador de oro de enemigo
 * @type number
 * @default 100
 * @min 0
 * @desc El cambio porcentual (multiplicativo) de oro que otorga el enemigo (más alto = más fácil).
 *
 * @param Enemy Exp Modifier
 * @text Modificador de experiencia del enemigo
 * @type number
 * @default 100
 * @min 0
 * @desc El cambio porcentual (multiplicativo) de experiencia que otorga el enemigo (mayor = más fácil).
 *
 * @param Encounter Rate Modifier
 * @text Modificador de tasa de encuentro
 * @type number
 * @default 100
 * @min 0
 * @desc El cambio porcentual (multiplicativo) de la tasa de encuentro (mayor = más difícil).
 *
 * @param Escape Ratio Modifier
 * @text Modificador de relación de escape
 * @type number
 * @default 100
 * @min 0
 * @desc El cambio porcentual (multiplicativo) de la probabilidad de éxito de escape (mayor = más fácil).
 *
 * @param Preemptive Modifier
 * @text Modificador preventivo
 * @type number
 * @default 100
 * @min 0
 * @desc El cambio porcentual (multiplicativo) de probabilidad de batalla preventiva (mayor = más fácil).
 *
 * @param Surprise Modifier
 * @text Modificador Sorpresa
 * @type number
 * @default 100
 * @min 0
 * @desc El cambio porcentual (multiplicativo) de la probabilidad de batalla sorpresa (mayor = más difícil).
 *
 * @param Buy Modifier
 * @type number
 * @default 100
 * @min 0
 * @desc The percentage change (multiplicative) of item prices in shops when buying (higher = harder)
 *
 * @param Sell Modifier
 * @type number
 * @default 100
 * @min 0
 * @desc The percentage change (multiplicative) of item prices in shops when selling (lower = harder)
*/
/*~struct~CustomInfo:es
 * @param Label
 * @desc The label for the custom info
 *
 * @param Info
 * @desc The info part to display after the label
*/
Imported.CGMZ_Difficulty = true;
CGMZ.Versions["Difficulty"] = "1.5.1";
CGMZ.Difficulty = {};
CGMZ.Difficulty.parameters = PluginManager.parameters('CGMZ_Difficulty');
CGMZ.Difficulty.DefaultDifficulty = CGMZ.Difficulty.parameters["Default Difficulty"];
CGMZ.Difficulty.BackgroundImage = CGMZ.Difficulty.parameters["Background Image"];
CGMZ.Difficulty.ControlsWindow = CGMZ.Difficulty.parameters["Controls Window"];
CGMZ.Difficulty.CurrentDifficulty = CGMZ.Difficulty.parameters["Current Difficulty"];
CGMZ.Difficulty.EnemyStatsText = CGMZ.Difficulty.parameters["Enemy Stats"];
CGMZ.Difficulty.EnemyGoldText = CGMZ.Difficulty.parameters["Enemy Gold"];
CGMZ.Difficulty.EnemyExpText = CGMZ.Difficulty.parameters["Enemy Exp"];
CGMZ.Difficulty.EncounterRateText = CGMZ.Difficulty.parameters["Encounter Rate"];
CGMZ.Difficulty.EscapeRateText = CGMZ.Difficulty.parameters["Escape Rate"];
CGMZ.Difficulty.SurpriseRateText = CGMZ.Difficulty.parameters["Surprise Rate"];
CGMZ.Difficulty.PreemptiveRateText = CGMZ.Difficulty.parameters["Preemptive Rate"];
CGMZ.Difficulty.BuyRateText = CGMZ.Difficulty.parameters["Buy Rate"];
CGMZ.Difficulty.SellRateText = CGMZ.Difficulty.parameters["Sell Rate"];
CGMZ.Difficulty.NoChangeText = CGMZ.Difficulty.parameters["No Change Text"];
CGMZ.Difficulty.ListWindowSettings = CGMZ.Difficulty.parameters["List Window Settings"];
CGMZ.Difficulty.DifficultyWindowSettings = CGMZ.Difficulty.parameters["Difficulty Window Settings"];
CGMZ.Difficulty.CurrentDifficultyWindowSettings = CGMZ.Difficulty.parameters["Current Difficulty Window Settings"];
CGMZ.Difficulty.ListWindowBackground = CGMZ.Difficulty.parameters["List Window Background"];
CGMZ.Difficulty.DifficultyWindowBackground = CGMZ.Difficulty.parameters["Difficulty Window Background"];
CGMZ.Difficulty.CurrentDifficultyWindowBackground = CGMZ.Difficulty.parameters["Current Difficulty Window Background"];
CGMZ.Difficulty.VisibleCommands = Number(CGMZ.Difficulty.parameters["Visible Commands"]);
CGMZ.Difficulty.LabelTextColor = Number(CGMZ.Difficulty.parameters["Label Text Color"]);
CGMZ.Difficulty.EasyColor = Number(CGMZ.Difficulty.parameters["Easy Color"]);
CGMZ.Difficulty.HardColor = Number(CGMZ.Difficulty.parameters["Hard Color"]);
CGMZ.Difficulty.CustomSpace = Number(CGMZ.Difficulty.parameters["Custom Space"]);
CGMZ.Difficulty.HeaderColor1 = Number(CGMZ.Difficulty.parameters["Header Color 1"]);
CGMZ.Difficulty.HeaderColor2 = Number(CGMZ.Difficulty.parameters["Header Color 2"]);
CGMZ.Difficulty.RatingVariable = Number(CGMZ.Difficulty.parameters["Rating Variable"]);
CGMZ.Difficulty.AutoCloseSceneOnSelect = (CGMZ.Difficulty.parameters["Auto Close Scene On Select"] === "true");
CGMZ.Difficulty.ListWindowRight = (CGMZ.Difficulty.parameters["List Window Right"] === "true");
CGMZ.Difficulty.ShowChange = (CGMZ.Difficulty.parameters["Show Change"] === "true");
CGMZ.Difficulty.Difficulties = CGMZ_Utils.parseJSON(CGMZ.Difficulty.parameters["Difficulties"], [], "[CGMZ] Difficulty", "Could not parse difficulty parameter JSON, please configure your Difficulties");
CGMZ.Difficulty.DisplayWindowInfo = CGMZ_Utils.parseJSON(CGMZ.Difficulty.parameters["Display Window Info"], [], "[CGMZ] Difficulty", "Could not parse difficulty window info JSON, please configure the Difficulty Window Info parameter");
CGMZ.Difficulty.Headers = CGMZ_Utils.parseJSON(CGMZ.Difficulty.parameters["Headers"], [], "[CGMZ] Difficulty", "Could not parse Headers parameter, please configure the Headers parameter");
//=============================================================================
// CGMZ_Difficulty
//-----------------------------------------------------------------------------
// Data class used to store difficulty data. Not saved
//=============================================================================
function CGMZ_Difficulty() {
    this.initialize(...arguments);
}
//-----------------------------------------------------------------------------
// Initialize
//-----------------------------------------------------------------------------
CGMZ_Difficulty.prototype.initialize = function(difficulty) {
	this._name = difficulty.Name;
	this._description = difficulty.Description;
	this._rating = Number(difficulty.Rating);
	this._switch = Number(difficulty.Switch);
	this._image = difficulty.Image;
	this._enemyStatModifier = parseFloat(difficulty["Enemy Stat Modifier"]) / 100.0;
	this._enemyGoldModifier = parseFloat(difficulty["Enemy Gold Modifier"]) / 100.0;
	this._enemyExpModifier = parseFloat(difficulty["Enemy Exp Modifier"]) / 100.0;
	this._encounterRateModifier = parseFloat(difficulty["Encounter Rate Modifier"]) / 100.0;
	this._escapeRatioModifier = parseFloat(difficulty["Escape Ratio Modifier"]) / 100.0;
	this._preemptiveModifier = parseFloat(difficulty["Preemptive Modifier"]) / 100.0;
	this._surpriseModifier = parseFloat(difficulty["Surprise Modifier"]) / 100.0;
	this._buyModifier = parseFloat(difficulty["Buy Modifier"]) / 100.0;
	this._sellModifier = parseFloat(difficulty["Sell Modifier"]) / 100.0;
	const discovered = (difficulty.Discovered === 'true');
	const enabled = (difficulty.Enabled === 'true');
	$cgmz.discoverDifficulty(this._name, discovered);
	$cgmz.enableDifficulty(this._name, enabled);
	this.setupCustomInfo(difficulty["Custom Info"]);
};
//-----------------------------------------------------------------------------
// Initialize custom info lines
//-----------------------------------------------------------------------------
CGMZ_Difficulty.prototype.setupCustomInfo = function(infosJSON) {
	this._customInfo = [];
	const infos = CGMZ_Utils.parseJSON(infosJSON, [], "[CGMZ] Difficulty", `Could not parse difficulty custom info JSON for difficulty: ${this._name}`);
	for(const infoJSON of infos) {
		const info = CGMZ_Utils.parseJSON(infoJSON, null, "[CGMZ] Difficulty", `Could not parse difficulty custom info JSON for difficulty: ${this._name}`);
		if(info) {
			this._customInfo.push({label:info.Label,info:info.Info});
		}
	}
};
//=============================================================================
// CGMZ_Core
//-----------------------------------------------------------------------------
// Save the difficulty name
//=============================================================================
//-----------------------------------------------------------------------------
// Add difficulty name to save data
//-----------------------------------------------------------------------------
const alias_CGMZ_Difficulty_CGMZ_Core_createPluginData = CGMZ_Core.prototype.createPluginData;
CGMZ_Core.prototype.createPluginData = function() {
	alias_CGMZ_Difficulty_CGMZ_Core_createPluginData.call(this);
	this._difficulty = CGMZ.Difficulty.DefaultDifficulty;
	this._discoveredDifficulties = {};
	this._enabledDifficulties = {};
	this._playthroughDifficulties = [];
};
//-----------------------------------------------------------------------------
// Create new difficulty data after load
//-----------------------------------------------------------------------------
const alias_CGMZ_Difficulty_CGMZ_Core_createAfterLoad = CGMZ_Core.prototype.createAfterLoad;
CGMZ_Core.prototype.createAfterLoad = function() {
	alias_CGMZ_Difficulty_CGMZ_Core_createAfterLoad.call(this);
	if(!this._difficulty) {
		this._difficulty = CGMZ.Difficulty.DefaultDifficulty;
	}
	if(!this._discoveredDifficulties) {
		this._discoveredDifficulties = {};
		for(const difficultyKey of Object.keys($cgmzTemp.getDifficulties())) {
			this._discoveredDifficulties[difficultyKey] = true;
		}
	}
	if(!this._enabledDifficulties) {
		this._enabledDifficulties = {};
		for(const difficultyKey of Object.keys($cgmzTemp.getDifficulties())) {
			this._enabledDifficulties[difficultyKey] = true;
		}
	}
	if(!this._playthroughDifficulties) {
		this._playthroughDifficulties = [this._difficulty];
	};
	this.setDifficulty(this._difficulty);
};
//-----------------------------------------------------------------------------
// Get difficulty
//-----------------------------------------------------------------------------
CGMZ_Core.prototype.getDifficulty = function() {
	return this._difficulty;
};
//-----------------------------------------------------------------------------
// Get discovered difficulties
//-----------------------------------------------------------------------------
CGMZ_Core.prototype.getDiscoveredDifficultyNames = function() {
	return Object.keys(this._discoveredDifficulties).filter(key => this._discoveredDifficulties[key]);
};
//-----------------------------------------------------------------------------
// Set difficulty
//-----------------------------------------------------------------------------
CGMZ_Core.prototype.setDifficulty = function(difficulty) {
	const oldDifficulty = this._difficulty;
	this._difficulty = difficulty;
	this.addPlaythroughDifficulty(difficulty);
	const tempDifficulty = $cgmzTemp?.getDifficulty(difficulty);
	if(tempDifficulty) {
		if(CGMZ.Difficulty.RatingVariable) $gameVariables.setValue(CGMZ.Difficulty.RatingVariable, tempDifficulty._rating);
		if(oldDifficulty !== this._difficulty) {
			const oldTempDifficulty = $cgmzTemp.getDifficulty(oldDifficulty);
			if(oldTempDifficulty && oldTempDifficulty._switch) {
				$gameSwitches.setValue(oldTempDifficulty._switch, false);
			}
		}
		if(tempDifficulty._switch) $gameSwitches.setValue(tempDifficulty._switch, true);
	}
};
//-----------------------------------------------------------------------------
// Discover (or undiscover) a difficulty
//-----------------------------------------------------------------------------
CGMZ_Core.prototype.discoverDifficulty = function(difficulty, discoverStatus) {
	this._discoveredDifficulties[difficulty] = discoverStatus;
};
//-----------------------------------------------------------------------------
// Enable (or disable) a difficulty
//-----------------------------------------------------------------------------
CGMZ_Core.prototype.enableDifficulty = function(difficulty, enableStatus) {
	this._enabledDifficulties[difficulty] = enableStatus;
};
//-----------------------------------------------------------------------------
// Check if difficulty is enabled
//-----------------------------------------------------------------------------
CGMZ_Core.prototype.isDifficultyEnabled = function(name) {
	return this._enabledDifficulties[name];
};
//-----------------------------------------------------------------------------
// Add a difficulty to the playthrough difficulties
//-----------------------------------------------------------------------------
CGMZ_Core.prototype.addPlaythroughDifficulty = function(difficulty) {
	if(!this._playthroughDifficulties.includes(difficulty)) {
		this._playthroughDifficulties.push(difficulty);
	}
};
//-----------------------------------------------------------------------------
// Get the easiest difficulty selected throughout the playthrough
// If getRating is true, will return rating, otherwise return name
//-----------------------------------------------------------------------------
CGMZ_Core.prototype.getEasiestSelectedDifficulty = function(getRating = false) {
	let rating = 999999;
	let easy = "";
	for(const difficulty of this._playthroughDifficulties) {
		const data = $cgmzTemp.getDifficulty(difficulty);
		if(!data) continue;
		if(data._rating < rating) {
			rating = data._rating;
			easy = difficulty;
		}
	}
	if(getRating) {
		if(rating < 999999) {
			return rating;
		} else {
			const def = $cgmzTemp.getDifficulty(CGMZ.Difficulty.DefaultDifficulty);
			return (def) ? def._rating : 999999;
		}
	} else {
		return (easy) ? easy : CGMZ.Difficulty.DefaultDifficulty;
	}
};
//-----------------------------------------------------------------------------
// Get the hardest difficulty selected throughout the playthrough
// If getRating is true, will return rating, otherwise return name
//-----------------------------------------------------------------------------
CGMZ_Core.prototype.getHardestSelectedDifficulty = function(getRating = false) {
	let rating = -1;
	let hard = "";
	for(const difficulty of this._playthroughDifficulties) {
		const data = $cgmzTemp.getDifficulty(difficulty);
		if(!data) continue;
		if(data._rating > rating) {
			rating = data._rating;
			hard = difficulty;
		}
	}
	if(getRating) {
		if(rating > -1) {
			return rating;
		} else {
			const def = $cgmzTemp.getDifficulty(CGMZ.Difficulty.DefaultDifficulty);
			return (def) ? def._rating : -1;
		}
	} else {
		return (hard) ? hard : CGMZ.Difficulty.DefaultDifficulty;
	}
};
//=============================================================================
// CGMZ_Temp
//-----------------------------------------------------------------------------
// Unsaved Difficulty Data handling
//=============================================================================
//-----------------------------------------------------------------------------
// Add difficulty data
//-----------------------------------------------------------------------------
const alias_CGMZ_Difficulty_CGMZ_Temp_createPluginData = CGMZ_Temp.prototype.createPluginData;
CGMZ_Temp.prototype.createPluginData = function() {
	alias_CGMZ_Difficulty_CGMZ_Temp_createPluginData.call(this);
	this.initializeDifficulties();
};
//-----------------------------------------------------------------------------
// Initialize Difficulties
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.initializeDifficulties = function() {
	this._difficulties = {};
	for(const json of CGMZ.Difficulty.Difficulties) {
		const parsedDifficulty = CGMZ_Utils.parseJSON(json, null, "[CGMZ] Difficulty", "Could not parse difficulty JSON. Please configure your Difficulties parameter");
		if(!parsedDifficulty) continue;
		const difficulty = new CGMZ_Difficulty(parsedDifficulty);
		this._difficulties[difficulty._name] = difficulty;
	}
	$cgmz.setDifficulty($cgmz.getDifficulty());
};
//-----------------------------------------------------------------------------
// Register Difficulty Plugin Commands
//-----------------------------------------------------------------------------
const alias_CGMZ_Difficulty_CGMZ_Temp_registerPluginCommands = CGMZ_Temp.prototype.registerPluginCommands;
CGMZ_Temp.prototype.registerPluginCommands = function() {
	alias_CGMZ_Difficulty_CGMZ_Temp_registerPluginCommands.call(this);
	PluginManager.registerCommand("CGMZ_Difficulty", "Call Scene", this.pluginCommandDifficultyCallScene);
	PluginManager.registerCommand("CGMZ_Difficulty", "Set Difficulty", this.pluginCommandDifficultySetDifficulty);
	PluginManager.registerCommand("CGMZ_Difficulty", "Get Difficulty", this.pluginCommandDifficultyGetDifficulty);
	PluginManager.registerCommand("CGMZ_Difficulty", "Discover Difficulty", this.pluginCommandDifficultyDiscoverDifficulty);
	PluginManager.registerCommand("CGMZ_Difficulty", "Enable Difficulty", this.pluginCommandDifficultyEnableDifficulty);
	PluginManager.registerCommand("CGMZ_Difficulty", "Reset Difficulty", this.pluginCommandDifficultyResetDifficulty);
	PluginManager.registerCommand("CGMZ_Difficulty", "Check Playthrough Difficulty", this.pluginCommandDifficultyCheckPlaythroughDifficulty);
};
//-----------------------------------------------------------------------------
// Plugin Command - Call Scene
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandDifficultyCallScene = function() {
	SceneManager.push(CGMZ_Scene_Difficulty);
};
//-----------------------------------------------------------------------------
// Plugin Command - Set difficulty
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandDifficultySetDifficulty = function(args) {
	$cgmz.setDifficulty(args.difficulty);
};
//-----------------------------------------------------------------------------
// Plugin Command - Get difficulty
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandDifficultyGetDifficulty = function(args) {
	const difficulty = $cgmzTemp.getDifficulty($cgmz.getDifficulty());
	$gameVariables.setValue(Number(args.variableId), difficulty._rating);
};
//-----------------------------------------------------------------------------
// Plugin Command - Discover Difficulty
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandDifficultyDiscoverDifficulty = function(args) {
	const difficulty = $cgmzTemp.getDifficulty(args.Difficulty);
	if(difficulty) {
		$cgmz.discoverDifficulty(args.Difficulty, args.Discover === 'true');
	}
};
//-----------------------------------------------------------------------------
// Plugin Command - Enable Difficulty
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandDifficultyEnableDifficulty = function(args) {
	const difficulty = $cgmzTemp.getDifficulty(args.Difficulty);
	if(difficulty) {
		$cgmz.enableDifficulty(args.Difficulty, args.Enable === 'true');
	}
};
//-----------------------------------------------------------------------------
// Plugin Command - Reset difficulty
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandDifficultyResetDifficulty = function() {
	$cgmz.setDifficulty(CGMZ.Difficulty.DefaultDifficulty);
};
//-----------------------------------------------------------------------------
// Plugin Command - Check Playthrough Difficulty
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandDifficultyCheckPlaythroughDifficulty = function(args) {
	const variableId = Number(args.Variable);
	if(args.Type === 'Hardest') {
		$gameVariables.setValue(variableId, $cgmz.getHardestSelectedDifficulty(true));
	} else {
		$gameVariables.setValue(variableId, $cgmz.getEasiestSelectedDifficulty(true));
	}
};
//-----------------------------------------------------------------------------
// Get a specific difficulty object by name
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.getDifficulty = function(name) {
	return this._difficulties[name];
};
//-----------------------------------------------------------------------------
// Get all difficulties
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.getDifficulties = function() {
	return this._difficulties;
};
//-----------------------------------------------------------------------------
// Get discovered difficulties
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.getDiscoveredDifficulties = function() {
	const discoveredNames = $cgmz.getDiscoveredDifficultyNames();
	return Object.keys(this._difficulties).filter(difficulty => discoveredNames.includes(difficulty));
};
//=============================================================================
// Game Enemy
//-----------------------------------------------------------------------------
// Change gold and exp by difficulty modifier. Also change base params
//=============================================================================
//-----------------------------------------------------------------------------
// Change the enemy exp
//-----------------------------------------------------------------------------
const alias_CGMZ_Difficulty_GameEnemy_exp = Game_Enemy.prototype.exp;
Game_Enemy.prototype.exp = function() {
	const difficulty = $cgmzTemp.getDifficulty($cgmz.getDifficulty());
    return alias_CGMZ_Difficulty_GameEnemy_exp.call(this) * difficulty._enemyExpModifier;
};
//-----------------------------------------------------------------------------
// Change the enemy gold
//-----------------------------------------------------------------------------
const alias_CGMZ_Difficulty_GameEnemy_gold = Game_Enemy.prototype.gold;
Game_Enemy.prototype.gold = function() {
	const difficulty = $cgmzTemp.getDifficulty($cgmz.getDifficulty());
    return alias_CGMZ_Difficulty_GameEnemy_gold.call(this) * difficulty._enemyGoldModifier;
};
//-----------------------------------------------------------------------------
// Change the enemy base params
//-----------------------------------------------------------------------------
const alias_CGMZ_Difficulty_GameEnemy_paramBase = Game_Enemy.prototype.paramBase;
Game_Enemy.prototype.paramBase = function(paramId) {
	const difficulty = $cgmzTemp.getDifficulty($cgmz.getDifficulty());
    return alias_CGMZ_Difficulty_GameEnemy_paramBase.call(this, paramId) * difficulty._enemyStatModifier;
};
//=============================================================================
// Game Party
//-----------------------------------------------------------------------------
// Change surprise and preemptive rates by modifiers
//=============================================================================
//-----------------------------------------------------------------------------
// Change the preemptive rate by difficulty modifier
//-----------------------------------------------------------------------------
const alias_CGMZ_Difficulty_GameParty_ratePreemptive = Game_Party.prototype.ratePreemptive;
Game_Party.prototype.ratePreemptive = function(troopAgi) {
    const difficulty = $cgmzTemp.getDifficulty($cgmz.getDifficulty());
    return alias_CGMZ_Difficulty_GameParty_ratePreemptive.call(this, troopAgi) * difficulty._preemptiveModifier;
};
//-----------------------------------------------------------------------------
// Change the surprise rate by difficulty modifier
//-----------------------------------------------------------------------------
const alias_CGMZ_Difficulty_GameParty_rateSurprise = Game_Party.prototype.rateSurprise;
Game_Party.prototype.rateSurprise = function(troopAgi) {
    const difficulty = $cgmzTemp.getDifficulty($cgmz.getDifficulty());
    return alias_CGMZ_Difficulty_GameParty_rateSurprise.call(this, troopAgi) * difficulty._surpriseModifier;
};
//=============================================================================
// BattleManager
//-----------------------------------------------------------------------------
// Change escape ratio by difficulty modifier
//=============================================================================
//-----------------------------------------------------------------------------
// Change escape ratio by difficulty modifier
//-----------------------------------------------------------------------------
const alias_CGMZ_Difficulty_BattleManager_makeEscapeRatio = BattleManager.makeEscapeRatio;
BattleManager.makeEscapeRatio = function() {
	alias_CGMZ_Difficulty_BattleManager_makeEscapeRatio.call(this);
	const difficulty = $cgmzTemp.getDifficulty($cgmz.getDifficulty());
    this._escapeRatio *= difficulty._escapeRatioModifier;
};
//=============================================================================
// Game Map
//-----------------------------------------------------------------------------
// Change encounter step by difficulty modifier
//=============================================================================
//-----------------------------------------------------------------------------
// Change encounter step by difficulty modifier
//-----------------------------------------------------------------------------
const alias_CGMZ_Difficulty_GameMap_encounterStep = Game_Map.prototype.encounterStep;
Game_Map.prototype.encounterStep = function() {
	const difficulty = $cgmzTemp.getDifficulty($cgmz.getDifficulty());
    return Number(alias_CGMZ_Difficulty_GameMap_encounterStep.call(this) * difficulty._encounterRateModifier);
};
//=============================================================================
// Scene_Shop
//-----------------------------------------------------------------------------
// Change sell price by modifier
//=============================================================================
//-----------------------------------------------------------------------------
// Change selling price by difficulty modifier
//-----------------------------------------------------------------------------
const alias_CGMZDifficulty_SceneShop_sellingPrice = Scene_Shop.prototype.sellingPrice;
Scene_Shop.prototype.sellingPrice = function() {
	const difficulty = $cgmzTemp.getDifficulty($cgmz.getDifficulty());
    const originalPrice = alias_CGMZDifficulty_SceneShop_sellingPrice.call(this);
	return Math.round(originalPrice * difficulty._sellModifier);
};
//=============================================================================
// Window_ShopBuy
//-----------------------------------------------------------------------------
// Change item buy price by modifier
//=============================================================================
//-----------------------------------------------------------------------------
// Change buying price by difficulty modifier
//-----------------------------------------------------------------------------
const alias_CGMZDifficulty_WindowShopBuy_price = Window_ShopBuy.prototype.price;
Window_ShopBuy.prototype.price = function(item) {
    const difficulty = $cgmzTemp.getDifficulty($cgmz.getDifficulty());
	const originalPrice = alias_CGMZDifficulty_WindowShopBuy_price.apply(this, arguments);
    return Math.round(originalPrice * difficulty._buyModifier);
};
//=============================================================================
// CGMZ_Scene_Difficulty
//-----------------------------------------------------------------------------
// Handle the difficulty select scene
//=============================================================================
function CGMZ_Scene_Difficulty() {
    this.initialize.apply(this, arguments);
}
CGMZ_Scene_Difficulty.prototype = Object.create(Scene_MenuBase.prototype);
CGMZ_Scene_Difficulty.prototype.constructor = CGMZ_Scene_Difficulty;
//-----------------------------------------------------------------------------
// Create difficulty windows
//-----------------------------------------------------------------------------
CGMZ_Scene_Difficulty.prototype.create = function() {
    Scene_MenuBase.prototype.create.call(this);
	this.createCurrentDifficultyWindow();
	this.createListWindow();
	this.createDisplayWindow();
};
//-----------------------------------------------------------------------------
// Create current difficulty window
//-----------------------------------------------------------------------------
CGMZ_Scene_Difficulty.prototype.createCurrentDifficultyWindow = function() {
	const rect = this.currentDifficultyWindowRect();
    this._currentDifficultyWindow = new CGMZ_Window_Difficulty_CurrentDifficulty(rect);
    this.addWindow(this._currentDifficultyWindow);
};
//-----------------------------------------------------------------------------
// Get current difficulty window rect
//-----------------------------------------------------------------------------
CGMZ_Scene_Difficulty.prototype.currentDifficultyWindowRect = function() {
	const x = CGMZ.Difficulty.ListWindowRight ? Graphics.boxWidth * 8 / 12 : Graphics.boxWidth / 12;
	const width = Graphics.boxWidth / 4;
	const height = this.calcWindowHeight(2, false);
	const y = Graphics.boxHeight / 12;
	return new Rectangle(x, y, width, height);
};
//-----------------------------------------------------------------------------
// Create list window
//-----------------------------------------------------------------------------
CGMZ_Scene_Difficulty.prototype.createListWindow = function() {
	const rect = this.listWindowRect();
    this._listWindow = new CGMZ_Window_Difficulty_DifficultyList(rect);
	this._listWindow.setHandler('cancel', this.popScene.bind(this));
	this._listWindow.setHandler('ok', this.onListOk.bind(this));
    this.addWindow(this._listWindow);
};
//-----------------------------------------------------------------------------
// Get list window rect
//-----------------------------------------------------------------------------
CGMZ_Scene_Difficulty.prototype.listWindowRect = function() {
	const x = this._currentDifficultyWindow.x;
	const y = this._currentDifficultyWindow.y + this._currentDifficultyWindow.height;
	const width = this._currentDifficultyWindow.width;
	const height = this.calcWindowHeight(CGMZ.Difficulty.VisibleCommands, true);
	return new Rectangle(x, y, width, height);
};
//-----------------------------------------------------------------------------
// Create display window
//-----------------------------------------------------------------------------
CGMZ_Scene_Difficulty.prototype.createDisplayWindow = function() {
	const rect = this.displayWindowRect()
    this._displayWindow = new CGMZ_Window_Difficulty_Display(rect);
	this._listWindow.setDisplayWindow(this._displayWindow);
    this.addWindow(this._displayWindow);
};
//-----------------------------------------------------------------------------
// Get display window rect
//-----------------------------------------------------------------------------
CGMZ_Scene_Difficulty.prototype.displayWindowRect = function() {
	const x = CGMZ.Difficulty.ListWindowRight ? Graphics.boxWidth / 12 : this._listWindow.x + this._listWindow.width;
	const y = this._currentDifficultyWindow.y;
	const width = Graphics.boxWidth * 7 / 12;
	const height = Graphics.boxHeight - Graphics.boxHeight / 6;
	return new Rectangle(x, y, width, height);
};
//-----------------------------------------------------------------------------
// On List Ok
//-----------------------------------------------------------------------------
CGMZ_Scene_Difficulty.prototype.onListOk = function() {
	$cgmz.setDifficulty(this._listWindow.item()._name);
	this._currentDifficultyWindow.refresh();
	this._listWindow.activate();
	if(CGMZ.Difficulty.AutoCloseSceneOnSelect) this.popScene();
};
//-----------------------------------------------------------------------------
// Get the scene's custom scene background for [CGMZ] Scene Backgrounds
// No need to check if Scene Backgrounds is installed because this custom func is only called by that plugin
//-----------------------------------------------------------------------------
CGMZ_Scene_Difficulty.prototype.CGMZ_getCustomSceneBackground = function() {
	return $cgmzTemp.sceneBackgroundPresets[CGMZ.Difficulty.BackgroundImage];
};
//-----------------------------------------------------------------------------
// Get controls window preset for [CGMZ] Controls Window
// No need to check if plugin is installed because this custom func is only called by that plugin
//-----------------------------------------------------------------------------
CGMZ_Scene_Difficulty.prototype.CGMZ_getControlsWindowOtherPreset = function() {
	return $cgmzTemp.getControlWindowPresetOther(CGMZ.Difficulty.ControlsWindow);
};
//=============================================================================
// CGMZ_Window_Difficulty_CurrentDifficulty
//-----------------------------------------------------------------------------
// Shows current difficulty
//=============================================================================
function CGMZ_Window_Difficulty_CurrentDifficulty(rect) {
    this.initialize.apply(this, arguments);
}
CGMZ_Window_Difficulty_CurrentDifficulty.prototype = Object.create(Window_Base.prototype);
CGMZ_Window_Difficulty_CurrentDifficulty.prototype.constructor = CGMZ_Window_Difficulty_CurrentDifficulty;
//-----------------------------------------------------------------------------
// Initialize
//-----------------------------------------------------------------------------
CGMZ_Window_Difficulty_CurrentDifficulty.prototype.initialize = function(rect) {
    Window_Base.prototype.initialize.call(this, rect);
	if(Imported.CGMZ_WindowSettings && CGMZ.Difficulty.CurrentDifficultyWindowSettings) this.CGMZ_setWindowSettings(CGMZ.Difficulty.CurrentDifficultyWindowSettings);
	if(Imported.CGMZ_WindowBackgrounds && CGMZ.Difficulty.CurrentDifficultyWindowBackground) this.CGMZ_setWindowBackground(CGMZ.Difficulty.CurrentDifficultyWindowBackground);
	this.refresh();
};
//-----------------------------------------------------------------------------
// Refresh
//-----------------------------------------------------------------------------
CGMZ_Window_Difficulty_CurrentDifficulty.prototype.refresh = function() {
	this.contents.clear();
	this.drawCurrentDifficulty();
};
//-----------------------------------------------------------------------------
// Draw current difficulty
//-----------------------------------------------------------------------------
CGMZ_Window_Difficulty_CurrentDifficulty.prototype.drawCurrentDifficulty = function() {
	this.CGMZ_drawTextLine(`\\c[${CGMZ.Difficulty.LabelTextColor}]${CGMZ.Difficulty.CurrentDifficulty}\\c[0]`, 0, 0, this.contents.width, 'center');
	this.CGMZ_drawTextLine($cgmz.getDifficulty(), 0, this.lineHeight(), this.contents.width, 'center');
};
//=============================================================================
// CGMZ_Window_Difficulty_DifficultyList
//-----------------------------------------------------------------------------
// Selectable window for choosing a profession in a list.
//=============================================================================
function CGMZ_Window_Difficulty_DifficultyList(rect) {
    this.initialize.apply(this, arguments);
}
CGMZ_Window_Difficulty_DifficultyList.prototype = Object.create(Window_Selectable.prototype);
CGMZ_Window_Difficulty_DifficultyList.prototype.constructor = CGMZ_Window_Difficulty_DifficultyList;
//-----------------------------------------------------------------------------
// Initialize
//-----------------------------------------------------------------------------
CGMZ_Window_Difficulty_DifficultyList.prototype.initialize = function(rect) {
    Window_Selectable.prototype.initialize.call(this, rect);
	if(Imported.CGMZ_WindowSettings && CGMZ.Difficulty.ListWindowSettings) this.CGMZ_setWindowSettings(CGMZ.Difficulty.ListWindowSettings);
	if(Imported.CGMZ_WindowBackgrounds && CGMZ.Difficulty.ListWindowBackground) this.CGMZ_setWindowBackground(CGMZ.Difficulty.ListWindowBackground);
	this.refresh();
	this.activate();
	this.select(0);
};
//-----------------------------------------------------------------------------
// Max items
//-----------------------------------------------------------------------------
CGMZ_Window_Difficulty_DifficultyList.prototype.maxItems = function() {
    return this._data ? this._data.length : 1;
};
//-----------------------------------------------------------------------------
// Current item
//-----------------------------------------------------------------------------
CGMZ_Window_Difficulty_DifficultyList.prototype.item = function() {
    return this._data[this.index()];
};
//-----------------------------------------------------------------------------
// Refresh
//-----------------------------------------------------------------------------
CGMZ_Window_Difficulty_DifficultyList.prototype.refresh = function() {
    this.makeItemList();
    Window_Selectable.prototype.refresh.call(this);
	this.selectCurrentDifficulty();
};
//-----------------------------------------------------------------------------
// Make item list
//-----------------------------------------------------------------------------
CGMZ_Window_Difficulty_DifficultyList.prototype.makeItemList = function() {
	const discoveredDifficulties = $cgmzTemp.getDiscoveredDifficulties();
	this._data = discoveredDifficulties.map(name => $cgmzTemp.getDifficulty(name));
};
//-----------------------------------------------------------------------------
// Make item list
//-----------------------------------------------------------------------------
CGMZ_Window_Difficulty_DifficultyList.prototype.selectCurrentDifficulty = function() {
	let index = this._data.length > 0 ? 0 : -1;
	const currentDifficulty = $cgmz.getDifficulty();
	for(let i = 0; i < this._data.length; i++) {
		if(this._data[i]._name === currentDifficulty) {
			index = i;
			break;
		}
	}
	this.select(index);
};
//-----------------------------------------------------------------------------
// Draw item in list
//-----------------------------------------------------------------------------
CGMZ_Window_Difficulty_DifficultyList.prototype.drawItem = function(index) {
    const item = this._data[index];
    const rect = this.itemRectWithPadding(index);
	this.changePaintOpacity(this.isEnabled(item));
    this.CGMZ_drawTextLine(item._name, rect.x, rect.y, rect.width, 'left');
};
//-----------------------------------------------------------------------------
// Determine if item is enabled
//-----------------------------------------------------------------------------
CGMZ_Window_Difficulty_DifficultyList.prototype.isEnabled = function(item) {
    return $cgmz.isDifficultyEnabled(item._name);
};
//-----------------------------------------------------------------------------
// Check if current item is enabled
//-----------------------------------------------------------------------------
CGMZ_Window_Difficulty_DifficultyList.prototype.isCurrentItemEnabled = function() {
	return this.isEnabled(this.item());
};
//-----------------------------------------------------------------------------
// Set display window
//-----------------------------------------------------------------------------
CGMZ_Window_Difficulty_DifficultyList.prototype.setDisplayWindow = function(displayWindow) {
    this._displayWindow = displayWindow;
    this.callUpdateHelp();
};
//-----------------------------------------------------------------------------
// See if can update display window
//-----------------------------------------------------------------------------
CGMZ_Window_Difficulty_DifficultyList.prototype.callUpdateHelp = function() {
    if(this.active && this._displayWindow) {
		this._displayWindow.setItem(this.item());
	}
};
//=============================================================================
// CGMZ_Window_Difficulty_Display
//-----------------------------------------------------------------------------
// Shows current difficulty modifiers
//=============================================================================
function CGMZ_Window_Difficulty_Display(rect) {
    this.initialize.apply(this, arguments);
}
CGMZ_Window_Difficulty_Display.prototype = Object.create(CGMZ_Window_Scrollable.prototype);
CGMZ_Window_Difficulty_Display.prototype.constructor = CGMZ_Window_Difficulty_Display;
//-----------------------------------------------------------------------------
// Initialize
//-----------------------------------------------------------------------------
CGMZ_Window_Difficulty_Display.prototype.initialize = function(rect) {
	const heightMultipleir = 20;
    CGMZ_Window_Scrollable.prototype.initialize.call(this, rect, heightMultipleir);
	if(Imported.CGMZ_WindowSettings && CGMZ.Difficulty.DifficultyWindowSettings) this.CGMZ_setWindowSettings(CGMZ.Difficulty.DifficultyWindowSettings);
	if(Imported.CGMZ_WindowBackgrounds && CGMZ.Difficulty.DifficultyWindowBackground) this.CGMZ_setWindowBackground(CGMZ.Difficulty.DifficultyWindowBackground);
	this.deactivate();
	this._neededHeight = 0;
	this._difficulty = null;
	this._difficultyImage = new Sprite();
	this.addInnerChild(this._difficultyImage);
	this.refresh();
};
//-----------------------------------------------------------------------------
// Set the currently displayed difficulty
//-----------------------------------------------------------------------------
CGMZ_Window_Difficulty_Display.prototype.setItem = function(difficulty) {
	if(this._difficulty === difficulty) return;
	this._difficulty = difficulty;
	this.refresh();
};
//-----------------------------------------------------------------------------
// Refresh
//-----------------------------------------------------------------------------
CGMZ_Window_Difficulty_Display.prototype.refresh = function() {
	this.contents.clear();
	this.setupWindowForNewEntry();
	if(!this._difficulty) return;
	this.loadDifficultyImage(); // calls the drawing function after image load
};
//-----------------------------------------------------------------------------
// Load the difficulty image
//-----------------------------------------------------------------------------
CGMZ_Window_Difficulty_Display.prototype.loadDifficultyImage = function() {
	const imageData = CGMZ_Utils.getImageData(this._difficulty._image, "img");
	this._difficultyImage.bitmap = ImageManager.loadBitmap(imageData.folder, imageData.filename);
	this._difficultyImage.bitmap.addLoadListener(this.drawCurrentDifficulty.bind(this));
};
//-----------------------------------------------------------------------------
// Draw difficulty info
//-----------------------------------------------------------------------------
CGMZ_Window_Difficulty_Display.prototype.drawCurrentDifficulty = function() {
	this._neededHeight = 0;
	let customLineCount = 0;
	let customHeader = 0;
	for(const section of CGMZ.Difficulty.DisplayWindowInfo) {
		switch(section) {
			case "Name": this._neededHeight += this.CGMZ_drawTextLine(this._difficulty._name, 0, this._neededHeight, this.contents.width, 'center'); break;
			case "Image": this.showImage(); break;
			case "Enemy Stats": this.drawStandardLine(CGMZ.Difficulty.EnemyStatsText, this.getPercentageString(this._difficulty._enemyStatModifier, true)); break;
			case "Enemy Gold": this.drawStandardLine(CGMZ.Difficulty.EnemyGoldText, this.getPercentageString(this._difficulty._enemyGoldModifier, false)); break;
			case "Enemy EXP": this.drawStandardLine(CGMZ.Difficulty.EnemyExpText, this.getPercentageString(this._difficulty._enemyExpModifier, false)); break;
			case "Encounter Rate": this.drawStandardLine(CGMZ.Difficulty.EncounterRateText, this.getPercentageString(this._difficulty._encounterRateModifier, true)); break;
			case "Escape Rate": this.drawStandardLine(CGMZ.Difficulty.EscapeRateText, this.getPercentageString(this._difficulty._escapeRatioModifier, false)); break;
			case "Preemptive Rate": this.drawStandardLine(CGMZ.Difficulty.PreemptiveRateText, this.getPercentageString(this._difficulty._preemptiveModifier, false)); break;
			case "Surprise Rate": this.drawStandardLine(CGMZ.Difficulty.SurpriseRateText, this.getPercentageString(this._difficulty._surpriseModifier, true)); break;
			case "Buy Rate": this.drawStandardLine(CGMZ.Difficulty.BuyRateText, this.getPercentageString(this._difficulty._buyModifier, true)); break;
			case "Sell Rate": this.drawStandardLine(CGMZ.Difficulty.SellRateText, this.getPercentageString(this._difficulty._sellModifier, false)); break;
			case "Description": this._neededHeight += this.CGMZ_drawText(this._difficulty._description, 0, 0, this._neededHeight, this.contents.width, 'left'); break;
			case "Custom Line": this.drawCustomInfo(customLineCount++); break;
			case "Blank Line": this._neededHeight += this.lineHeight(); break;
			case "Custom Space": this._neededHeight += CGMZ.Difficulty.CustomSpace; break;
			case "Custom Header": this._neededHeight += this.CGMZ_drawHeader(CGMZ.Difficulty.Headers[customHeader++], this._neededHeight, CGMZ.Difficulty.HeaderColor1, CGMZ.Difficulty.HeaderColor2); break;
		}
	}
	this._neededHeight += this.padding * 2;
	this.checkForScroll();
};
//-----------------------------------------------------------------------------
// Show difficulty image (only scale x value, don't care about y scaling here)
//-----------------------------------------------------------------------------
CGMZ_Window_Difficulty_Display.prototype.showImage = function() {
	const scaleX = Math.min(1.0, this.contents.width/this._difficultyImage.width);
	this._difficultyImage.scale.x = scaleX;
	this._difficultyImage.x = (this.contents.width - this._difficultyImage.width * scaleX) / 2;
	this._difficultyImage.y = this._neededHeight;
	this._difficulty._image ? this._difficultyImage.show() : this._difficultyImage.hide();
	this._neededHeight += this._difficultyImage.height;
};
//-----------------------------------------------------------------------------
// Get the percentage string
//-----------------------------------------------------------------------------
CGMZ_Window_Difficulty_Display.prototype.getPercentageString = function(basePercent, lowerIsBetter) {
	const raw = Math.round(Number(basePercent*100));
	if(!CGMZ.Difficulty.ShowChange) return `${raw}%`;
	const percent = raw - 100;
	if(percent === 0) return CGMZ.Difficulty.NoChangeText;
	if(percent < 0) return (lowerIsBetter) ? `\\c[${CGMZ.Difficulty.EasyColor}]${percent}%\\c[0]` : `\\c[${CGMZ.Difficulty.HardColor}]${percent}%\\c[0]`;
	return (lowerIsBetter) ? `\\c[${CGMZ.Difficulty.HardColor}]+${percent}%\\c[0]` : `\\c[${CGMZ.Difficulty.EasyColor}]+${percent}%\\c[0]`;
};
//-----------------------------------------------------------------------------
// Draw standard difficulty line
//-----------------------------------------------------------------------------
CGMZ_Window_Difficulty_Display.prototype.drawStandardLine = function(label, descriptor) {
	const labelString = CGMZ_Utils.makeLabelText(label, CGMZ.Difficulty.LabelTextColor);
	const totalString = `${labelString}${descriptor}`;
	this._neededHeight += this.CGMZ_drawTextLine(totalString, 0, this._neededHeight, this.contents.width, 'left');
};
//-----------------------------------------------------------------------------
// Draw a custom info line
//-----------------------------------------------------------------------------
CGMZ_Window_Difficulty_Display.prototype.drawCustomInfo = function(customLineCount) {
	const customLineObject = this._difficulty._customInfo[customLineCount];
	if(customLineObject) {
		this.drawStandardLine(customLineObject.label, customLineObject.info, this._neededHeight);
	}
};