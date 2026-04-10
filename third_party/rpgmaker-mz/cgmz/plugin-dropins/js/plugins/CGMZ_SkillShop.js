/*:
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/skillshop/
 * @target MZ
 * @base CGMZ_Core
 * @orderAfter CGMZ_Core
 * @plugindesc Creates a shop scene where you can buy skills
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
 * Made for RPG Maker MZ 1.9.0
 * ----------------------------------------------------------------------------
 * Description: Creates a shop scene where you can buy skills for actors.
 * It includes selecting which actor can learn the skill, and showing which
 * actors can learn the skill or those that already learned the skill.
 * ----------------------------------------------------------------------------
 * Documentation:
 * ----------------------------Default Prices----------------------------------
 * You can set up default prices for skills, which will be used when the skill
 * price in the shop is set to -1. In case you set a skill price to -1 in the
 * shop and no default price was set up in the plugin parameters, this plugin
 * will fall back and use the Missing Default Price parameter.
 *
 * Default prices allow you to offer the same spells in many different shops
 * without needing to remember how much the skill should cost.
 *
 * Default prices are optional, and can always be overridden by providing a 
 * non-negative number in the shop goods parameter when calling the Skill Shop.
 * -------------------------------Note Tags------------------------------------
 * To restrict certain actors from purchasing certain skills, add the following
 * notetag to the actor in the database:
 * <cgmzssrestriction:skillId1,skillId2>
 * Example restricting skill 1 and 2 from purchase:
 * <cgmzssrestriction:1,2>
 * Note: This notetag also works for classes.
 *
 * To ensure a skill cannot be purchased before a prerequisite skill is known,
 * use the following notetag in the skill notebox in the database:
 * <cgmzssprereq:skillId1,skillId2>
 * Example preventing a skill from being purchased if the actor does not
 * already know skill 1 and 2:
 * <cgmzssprereq:1,2>
 *
 * To ensure a skill cannot be purchased before a level requirement is met,
 * use the following notetag in the skill notebox in the database:
 * <cgmzsslevel:level>
 * Example preventing a skill from being purchased if the actor is not at
 * least level 10:
 * <cgmzsslevel:10>
 * ---------------------------Default Commands---------------------------------
 * When setting up your skill shop commands, the following symbols can be used
 * to refer to the default commands:
 *
 * buy - Will handle like the default buy command
 * cancel - Will handle like the default cancel command 
 * ----------------------------Plugin Commands---------------------------------
 * • Call Scene
 * Calls the skill shop scene with the provided shop goods (skills) for sale.
 * ------------------------------Saved Games-----------------------------------
 * This plugin is fully compatible with saved games. This means you can:
 *
 * ✓ Add this plugin to a saved game and it will work as expected
 * ✓ Change any plugin params and changes will be reflected in saved games
 * ✓ Remove the plugin with no issue to save data
 * -------------------------Latest Version-------------------------------------
 * Hi all, this update makes the skill shop compatible with [CGMZ] Currency
 * System. You can now use alternate currencies in the skill shop and have it
 * correctly remove the price from the currency when buying a skill.
 *
 * Version 1.3.0
 * - Skill Shop now compatible with [CGMZ] Currency System
 *
 * @command Call Scene
 * @desc Calls the Skill Shop scene
 *
 * @arg goods
 * @type struct<Skill>[]
 * @text Shop Goods
 * @desc Set up shop goods here
 * @default []
 *
 * @arg Background
 * @desc [CGMZ] Scene Backgrounds preset id to use for the shop
 *
 * @param Price Options
 *
 * @param Default Skill Prices
 * @parent Price Options
 * @type struct<SkillDefault>[]
 * @desc Set up default skill prices here
 * @default []
 *
 * @param Missing Default Price
 * @parent Price Options
 * @type number
 * @min 0
 * @desc Price to use if default price is not set, but default price access attempted from shop
 * @default 0
 *
 * @param Window Options
 *
 * @param Disable Touch UI Space
 * @parent Window Options
 * @type boolean
 * @desc If true, will not leave space for Touch UI buttons if Touch UI is disabled
 * @default false
 *
 * @param Command Window
 * @parent Window Options
 *
 * @param Skill Shop Commands
 * @parent Command Window
 * @type struct<Handler>[]
 * @desc Command Name and associated js commands
 * @default ["{\"Command Name\":\"Buy\",\"Icon\":\"208\",\"Command Symbol\":\"buy\",\"JS Command\":\"\",\"JS Enable Condition\":\"return true;\",\"JS Show Condition\":\"return true;\",\"Background Image\":\"\",\"Background Image X\":\"0\",\"Background Image Y\":\"0\"}","{\"Command Name\":\"Cancel\",\"Icon\":\"82\",\"Command Symbol\":\"cancel\",\"JS Command\":\"\",\"JS Enable Condition\":\"return true;\",\"JS Show Condition\":\"return true;\",\"Background Image\":\"\",\"Background Image X\":\"0\",\"Background Image Y\":\"0\"}"]
 *
 * @param Command Columns
 * @parent Command Window
 * @type number
 * @desc Number of columns to display in the command window
 * @default 2
 *
 * @param Command Alignment
 * @parent Command Window
 * @type select
 * @option left
 * @option center
 * @option right
 * @default center
 * @desc The alignment of the command text in the command window
 *
 * @param Command Icon Alignment
 * @parent Command Window
 * @type select
 * @option left
 * @option right
 * @default left
 * @desc The alignment of the icon parameter in the command window
 *
 * @param Confirm Window
 * @parent Window Options
 *
 * @param Show Confirm Window
 * @parent Confirm Window
 * @type boolean
 * @desc If true, will ask the player to confirm they want to purchase a spell before the purchase actually occurs
 * @default false
 *
 * @param Confirm As Default
 * @parent Confirm Window
 * @type boolean
 * @desc If true, the confirm window will start with Confirm selected. Otherwise, it will start with Cancel selected.
 * @default true
 *
 * @param Confirm Window Width
 * @parent Confirm Window
 * @type number
 * @max 100
 * @desc Width of the confirm window (as percentage of the screen)
 * @default 50
 *
 * @param Confirm Text
 * @parent Confirm Window
 * @desc Text to show for the confirm option
 * @default Purchase
 *
 * @param No Confirm Text
 * @parent Confirm Window
 * @desc Text to show for the cancel confirm option
 * @default Cancel
 *
 * @param Confirm Title Text
 * @parent Confirm Window
 * @desc Text to show for the title text in the confirm window
 * @default Confirm Purchase
 *
 * @param Actor Window
 * @parent Window Options
 *
 * @param Available Text
 * @parent Actor Window
 * @desc Text to show when the actor can learn a skill
 * @default Available
 *
 * @param Already Known Text
 * @parent Actor Window
 * @desc Text to show when the actor has already learned the skill
 * @default Already Known
 *
 * @param Cannot Learn Text
 * @parent Actor Window
 * @desc Text to show when the actor cannot learn a skill (due to skill type or some other reason)
 * @default Cannot Learn
 *
 * @param Restricted Text
 * @parent Actor Window
 * @desc Text to show when the actor cannot learn a skill due to notetag restriction
 * @default Restricted
 *
 * @param Level Issue Text
 * @parent Actor Window
 * @desc Text to show when the actor cannot learn a skill due to being too low level
 * @default Level Too Low
 *
 * @param Show All Missing Prerequisites
 * @parent Actor Window
 * @type boolean
 * @desc If true, will display all missing prerequisites
 * @default false
 *
 * @param Missing Prerequisites Text
 * @parent Actor Window
 * @desc Text to show when the actor cannot learn a skill due to missing prereq skills
 * @default Missing Prerequisites
 *
 * @param Learned Skills Only
 * @parent Actor Window
 * @type boolean
 * @desc If prerequisites only look at learned skills (not temp skills via equips)
 * @default true
 *
 * @param Draw Character
 * @parent Actor Window
 * @type boolean
 * @desc If the window should draw character sprites. Cannot be used with Draw Face.
 * @default true
 *
 * @param Draw Face
 * @parent Actor Window
 * @type boolean
 * @desc If the window should draw face sprites. Cannot be used with Draw Character
 * @default true
 *
 * @param Face Height
 * @parent Actor Window
 * @type number
 * @min 0
 * @desc The height (and width) to draw face images. 0 = default
 * @default 0
 *
 * @param Integrations
 *
 * @param Shop Background
 * @parent Integrations
 * @desc The [CGMZ] Scene Backgrounds preset id for the skill shop scene
 *
 * @param Controls Window
 * @parent Integrations
 * @desc The [CGMZ] Controls Window preset id for the skill shop scene
 *
 * @param Command Window Settings
 * @parent Integrations
 * @desc The [CGMZ] Window Settings preset id for the command window
 *
 * @param Confirm Window Settings
 * @parent Integrations
 * @desc The [CGMZ] Window Settings preset id for the confirm window
 *
 * @param Buy Window Settings
 * @parent Integrations
 * @desc The [CGMZ] Window Settings preset id for the buy window
 *
 * @param Actor Window Settings
 * @parent Integrations
 * @desc The [CGMZ] Window Settings preset id for the actor window
 *
 * @param Gold Window Settings
 * @parent Integrations
 * @desc The [CGMZ] Window Settings preset id for the gold window
 *
 * @param Help Window Settings
 * @parent Integrations
 * @desc The [CGMZ] Window Settings preset id for the help window
 *
 * @param Command Window Background
 * @parent Integrations
 * @desc The [CGMZ] Window Backgrounds preset id for the command window
 *
 * @param Confirm Window Background
 * @parent Integrations
 * @desc The [CGMZ] Window Backgrounds preset id for the confirm window
 *
 * @param Buy Window Background
 * @parent Integrations
 * @desc The [CGMZ] Window Backgrounds preset id for the buy window
 *
 * @param Actor Window Background
 * @parent Integrations
 * @desc The [CGMZ] Window Backgrounds preset id for the actor window
 *
 * @param Gold Window Background
 * @parent Integrations
 * @desc The [CGMZ] Window Backgrounds preset id for the gold window
 *
 * @param Help Window Background
 * @parent Integrations
 * @desc The [CGMZ] Window Backgrounds preset id for the help window
*/
/*~struct~Skill:
 * @param skill
 * @type skill
 * @default 0
 * @desc The skill to offer in the skill shop
 *
 * @param price
 * @type number
 * @min -1
 * @default -1
 * @desc The price of the skill. -1 = default price
*/
/*~struct~SkillDefault:
 * @param skill
 * @type skill
 * @default 0
 * @desc The skill to set a default price for
 *
 * @param price
 * @type number
 * @min 0
 * @default 0
 * @desc The default price of the skill
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
 * @param JS Enable Condition
 * @type multiline_string
 * @default return true;
 * @desc JavaScript to run to determine if the command is enabled
 *
 * @param JS Show Condition
 * @type multiline_string
 * @default return true;
 * @desc JavaScript to run to determine if the command is shown
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
*/
/*:zh-CN
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/skillshop/
 * @target MZ
 * @base CGMZ_Core
 * @orderAfter CGMZ_Core
 * @plugindesc 技能商店系统（在商店内直接学习技能）
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
 * 【插件版本】 V 1.3.0
 * ----------------------------------------------------------------------------
 * 【兼容性】仅测试作者所制作的插件
 * 【RM版本】RPG Maker MZ 1.9.0
 * ----------------------------------------------------------------------------
 * 【插件描述】
 * 1、创建一个技能商店，使角色可以直接学习技能。
 * 2、可以设置专属的技能，使某些角色不能学习。
 * 3、可以设置先决条件，需要先学习技能1才能学习技能2。
 * ----------------------------------------------------------------------------
 * 【使用说明】
 *
 * 一、备注命令
 * 如果你需要限制某个角色不能从技能商店中学习某个技能。
 * 请在角色设置界面的"备注"栏内输入以下命令。N1、N2对应技能ID
 * <cgmzssrestriction:N1,N2>
 * 本命令同样适用于职业设置。被限制的技能可以是1个或者多个。
 *
 * 如果你想设置学习某个技能的先决条件，
 * 请在某个技能的设置界面的"备注"里输入以下命令。N1、N2对应先决技能的ID。
 * <cgmzssprereq:N1,N2>
 * 先决技能可以是1个或者是多个。
 *
 * To ensure a skill cannot be purchased before a level requirement is met,
 * use the following notetag in the skill notebox in the database:
 * <cgmzsslevel:level>
 * Example preventing a skill from being purchased if the actor is not at
 * least level 10:
 * <cgmzsslevel:10>
 * 
 * 二、插件指令
 * 打开技能商店：打开技能商店，并通过设置来指定本商店可以出售的技能和价格。
 * 通过不同的事件来设置不同的商店指令来出售不同的技能。
 * 
 * 三、作者注
 * This plugin is fully compatible with saved games.
 * 
 * ----------------------------Default Prices----------------------------------
 * You can set up default prices for skills, which will be used when the skill
 * price in the shop is set to -1. In case you set a skill price to -1 in the
 * shop and no default price was set up in the plugin parameters, this plugin
 * will fall back and use the Missing Default Price parameter.
 *
 * Default prices allow you to offer the same spells in many different shops
 * without needing to remember how much the skill should cost.
 *
 * Default prices are optional, and can always be overridden by providing a 
 * non-negative number in the shop goods parameter when calling the Skill Shop.
 * ---------------------------Default Commands---------------------------------
 * When setting up your skill shop commands, the following symbols can be used
 * to refer to the default commands:
 *
 * buy - Will handle like the default buy command
 * cancel - Will handle like the default cancel command 
 * -------------------------Latest Version-------------------------------------
 * Hi all, this update makes the skill shop compatible with [CGMZ] Currency
 * System. You can now use alternate currencies in the skill shop and have it
 * correctly remove the price from the currency when buying a skill.
 *
 * Version 1.3.0
 * - Skill Shop now compatible with [CGMZ] Currency System
 *
 * @command Call Scene
 * @text 打开技能商店
 * @desc 打开技能商店
 *
 * @arg goods
 * @text 技能种类
 * @type struct<Skill>[]
 * @desc 本商店出售的技能。
 * @default []
 *
 * @arg Background
 * @desc [CGMZ] Scene Backgrounds preset id to use for the shop
 *
 * @param Price Options
 *
 * @param Default Skill Prices
 * @parent Price Options
 * @type struct<SkillDefault>[]
 * @desc Set up default skill prices here
 * @default []
 *
 * @param Missing Default Price
 * @parent Price Options
 * @type number
 * @min 0
 * @desc Price to use if default price is not set, but default price access attempted from shop
 * @default 0
 *
 * @param Window Options
 * @text 文本描述设置
 *
 * @param Command Window
 * @parent Window Options
 *
 * @param Disable Touch UI Space
 * @parent Window Options
 * @type boolean
 * @desc If true, will not leave space for Touch UI buttons if Touch UI is disabled
 * @default false
 *
 * @param Skill Shop Commands
 * @parent Command Window
 * @type struct<Handler>[]
 * @desc Command Name and associated js commands
 * @default ["{\"Command Name\":\"Buy\",\"Icon\":\"208\",\"Command Symbol\":\"buy\",\"JS Command\":\"\",\"JS Enable Condition\":\"return true;\",\"JS Show Condition\":\"return true;\",\"Background Image\":\"\",\"Background Image X\":\"0\",\"Background Image Y\":\"0\"}","{\"Command Name\":\"Cancel\",\"Icon\":\"82\",\"Command Symbol\":\"cancel\",\"JS Command\":\"\",\"JS Enable Condition\":\"return true;\",\"JS Show Condition\":\"return true;\",\"Background Image\":\"\",\"Background Image X\":\"0\",\"Background Image Y\":\"0\"}"]
 *
 * @param Command Columns
 * @parent Command Window
 * @type number
 * @desc Number of columns to display in the command window
 * @default 2
 *
 * @param Command Alignment
 * @parent Command Window
 * @type select
 * @option left
 * @option center
 * @option right
 * @default center
 * @desc The alignment of the command text in the command window
 *
 * @param Command Icon Alignment
 * @parent Command Window
 * @type select
 * @option left
 * @option right
 * @default left
 * @desc The alignment of the icon parameter in the command window
 *
 * @param Confirm Window
 * @parent Window Options
 *
 * @param Show Confirm Window
 * @parent Confirm Window
 * @type boolean
 * @desc If true, will ask the player to confirm they want to purchase a spell before the purchase actually occurs
 * @default false
 *
 * @param Confirm As Default
 * @parent Confirm Window
 * @type boolean
 * @desc If true, the confirm window will start with Confirm selected. Otherwise, it will start with Cancel selected.
 * @default true
 *
 * @param Confirm Window Width
 * @parent Confirm Window
 * @type number
 * @max 100
 * @desc Width of the confirm window (as percentage of the screen)
 * @default 50
 *
 * @param Confirm Text
 * @parent Confirm Window
 * @desc Text to show for the confirm option
 * @default Purchase
 *
 * @param No Confirm Text
 * @parent Confirm Window
 * @desc Text to show for the cancel confirm option
 * @default Cancel
 *
 * @param Confirm Title Text
 * @parent Confirm Window
 * @desc Text to show for the title text in the confirm window
 * @default Confirm Purchase
 *
 * @param Actor Window
 * @parent Window Options
 *
 * @param Available Text
 * @text 可以学习的描述
 * @parent Actor Window
 * @desc 显示这个技能是可以学习的。
 * @default 可以学习
 *
 * @param Already Known Text
 * @text 已经学会的描述
 * @parent Actor Window
 * @desc 显示这个技能是已经学会了的。
 * @default 已经学会
 *
 * @param Cannot Learn Text
 * @text 无法学习的描绘
 * @parent Actor Window
 * @desc 显示这个技能因为技能类型或其他原因无法学习。
 * @default 无法学习
 *
 * @param Restricted Text
 * @text 被限制学习的描述
 * @parent Actor Window
 * @desc 显示这个技能因为被限制而无法学习的。（通过"备注"输入指令来限制）
 * @default 被限制的
 *
 * @param Level Issue Text
 * @parent Actor Window
 * @desc Text to show when the actor cannot learn a skill due to being too low level
 * @default Level Too Low
 *
 * @param Show All Missing Prerequisites
 * @parent Actor Window
 * @type boolean
 * @desc If true, will display all missing prerequisites
 * @default false
 *
 * @param Missing Prerequisites Text
 * @text 条件不足的描述
 * @parent Actor Window
 * @desc 显示这个技能需要先学会某些先决技能才能学习。
 * @default 条件不足
 *
 * @param Learned Skills Only
 * @parent Actor Window
 * @type boolean
 * @desc If prerequisites only look at learned skills (not temp skills via equips)
 * @default true
 *
 * @param Draw Character
 * @parent Actor Window
 * @type boolean
 * @desc If the window should draw character sprites. Cannot be used with Draw Face.
 * @default true
 *
 * @param Draw Face
 * @parent Actor Window
 * @type boolean
 * @desc If the window should draw face sprites. Cannot be used with Draw Character
 * @default true
 *
 * @param Face Height
 * @parent Actor Window
 * @type number
 * @min 0
 * @desc The height (and width) to draw face images. 0 = default
 * @default 0
 *
 * @param Integrations
 *
 * @param Shop Background
 * @parent Integrations
 * @desc The [CGMZ] Scene Backgrounds preset id for the skill shop scene
 *
 * @param Controls Window
 * @parent Integrations
 * @desc The [CGMZ] Controls Window preset id for the skill shop scene
 *
 * @param Command Window Settings
 * @parent Integrations
 * @desc The [CGMZ] Window Settings preset id for the command window
 *
 * @param Confirm Window Settings
 * @parent Integrations
 * @desc The [CGMZ] Window Settings preset id for the confirm window
 *
 * @param Buy Window Settings
 * @parent Integrations
 * @desc The [CGMZ] Window Settings preset id for the buy window
 *
 * @param Actor Window Settings
 * @parent Integrations
 * @desc The [CGMZ] Window Settings preset id for the actor window
 *
 * @param Gold Window Settings
 * @parent Integrations
 * @desc The [CGMZ] Window Settings preset id for the gold window
 *
 * @param Help Window Settings
 * @parent Integrations
 * @desc The [CGMZ] Window Settings preset id for the help window
 *
 * @param Command Window Background
 * @parent Integrations
 * @desc The [CGMZ] Window Backgrounds preset id for the command window
 *
 * @param Confirm Window Background
 * @parent Integrations
 * @desc The [CGMZ] Window Backgrounds preset id for the confirm window
 *
 * @param Buy Window Background
 * @parent Integrations
 * @desc The [CGMZ] Window Backgrounds preset id for the buy window
 *
 * @param Actor Window Background
 * @parent Integrations
 * @desc The [CGMZ] Window Backgrounds preset id for the actor window
 *
 * @param Gold Window Background
 * @parent Integrations
 * @desc The [CGMZ] Window Backgrounds preset id for the gold window
 *
 * @param Help Window Background
 * @parent Integrations
 * @desc The [CGMZ] Window Backgrounds preset id for the help window
*/
/*~struct~Skill:zh-CN
 * @param skill
 * @text 技能
 * @type skill
 * @default 0
 * @desc 选择商店出售的技能的ID。
 *
 * @param price
 * @text 价格
 * @type number
 * @min -1
 * @default -1
 * @desc 该技能的售价。 -1 = default price
*/
/*~struct~SkillDefault:zh-CN
 * @param skill
 * @type skill
 * @default 0
 * @desc The skill to set a default price for
 *
 * @param price
 * @type number
 * @min 0
 * @default 0
 * @desc The default price of the skill
*/
/*~struct~Handler:zh-CN
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
 * @param JS Enable Condition
 * @type multiline_string
 * @default return true;
 * @desc JavaScript to run to determine if the command is enabled
 *
 * @param JS Show Condition
 * @type multiline_string
 * @default return true;
 * @desc JavaScript to run to determine if the command is shown
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
*/
/*:es
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/skillshop/
 * @target MZ
 * @base CGMZ_Core
 * @orderAfter CGMZ_Core
 * @plugindesc Crea una escena de tienda donde puedes comprar habilidades.
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
 * Versión: 1.3.0
 * ----------------------------------------------------------------------------
 * Compatibilidad: Sólo probado con mis CGMZ plugins.
 * Hecho para RPG Maker MZ 1.9.0
 * ----------------------------------------------------------------------------
 * Descripción: Crea una escena de tienda donde puedes comprar habilidades para
 * actores. Incluye seleccionar qué actor puede aprender la habilidad y mostrar
 * qué actores pueden aprender la habilidad o aquellos que ya aprendieron la
 * habilidad.
 * ----------------------------------------------------------------------------
 * Documentación:
 * ----------------------------Default Prices----------------------------------
 * You can set up default prices for skills, which will be used when the skill
 * price in the shop is set to -1. In case you set a skill price to -1 in the
 * shop and no default price was set up in the plugin parameters, this plugin
 * will fall back and use the Missing Default Price parameter.
 *
 * Default prices allow you to offer the same spells in many different shops
 * without needing to remember how much the skill should cost.
 *
 * Default prices are optional, and can always be overridden by providing a 
 * non-negative number in the shop goods parameter when calling the Skill Shop.
 * ---------------------------Default Commands---------------------------------
 * When setting up your skill shop commands, the following symbols can be used
 * to refer to the default commands:
 *
 * buy - Will handle like the default buy command
 * cancel - Will handle like the default cancel command 
 * ---------------------------Etiquetas de nota--------------------------------
 * Para restringir que ciertos actores compren ciertas habilidades, agregue la 
 * siguiente etiqueta de nota al actor en la base de datos:
 * <cgmzssrestriction:skillId1,skillId2>
 * Ejemplo de restricción de habilidad 1 y 2 desde la compra:
 * <cgmzssrestriction:1,2>
 * Nota: Esta etiqueta de notas también funciona para las clases.
 *
 * Para garantizar que no se pueda comprar una habilidad antes de que se
 * conozca una habilidad previa, use la siguiente etiqueta de nota en el cuadro
 * de notas de habilidades en la base de datos:
 * <cgmzssprereq:skillId1,skillId2>
 * Ejemplo que impide que se compre una habilidad si el actor aún no conoce la
 * habilidad 1 y 2:
 * <cgmzssprereq:1,2>
 *
 * To ensure a skill cannot be purchased before a level requirement is met,
 * use the following notetag in the skill notebox in the database:
 * <cgmzsslevel:level>
 * Example preventing a skill from being purchased if the actor is not at
 * least level 10:
 * <cgmzsslevel:10>
 * ---------------------------Comandos de Plugin-------------------------------
 * • Escena de llamada
 * Apertura de la tienda de habilidades con los productos provistos de la
 * tienda (habilidades) en venta.
 * ----------------------------Juegos Guardados--------------------------------
 * Este plugin es totalmente compatible con los juegos guardados.
 * -------------------------Latest Version-------------------------------------
 * Hi all, this update makes the skill shop compatible with [CGMZ] Currency
 * System. You can now use alternate currencies in the skill shop and have it
 * correctly remove the price from the currency when buying a skill.
 *
 * Version 1.3.0
 * - Skill Shop now compatible with [CGMZ] Currency System
 *
 * @command Call Scene
 * @text Abrir la tienda de habilidades
 * @desc Llamar/Abrir la tienda de habilidades.
 *
 * @arg goods
 * @text Productos
 * @type struct<Skill>[]
 * @text Shop Goods
 * @desc Configure los productos de la tienda aquí.
 * @default []
 *
 * @arg Background
 * @desc [CGMZ] Scene Backgrounds preset id to use for the shop
 *
 * @param Price Options
 *
 * @param Default Skill Prices
 * @parent Price Options
 * @type struct<SkillDefault>[]
 * @desc Set up default skill prices here
 * @default []
 *
 * @param Missing Default Price
 * @parent Price Options
 * @type number
 * @min 0
 * @desc Price to use if default price is not set, but default price access attempted from shop
 * @default 0
 *
 * @param Window Options
 * @text Opciones de Ventana
 *
 * @param Disable Touch UI Space
 * @parent Window Options
 * @type boolean
 * @desc If true, will not leave space for Touch UI buttons if Touch UI is disabled
 * @default false
 *
 * @param Command Window
 * @parent Window Options
 *
 * @param Skill Shop Commands
 * @parent Command Window
 * @type struct<Handler>[]
 * @desc Command Name and associated js commands
 * @default ["{\"Command Name\":\"Buy\",\"Icon\":\"208\",\"Command Symbol\":\"buy\",\"JS Command\":\"\",\"JS Enable Condition\":\"return true;\",\"JS Show Condition\":\"return true;\",\"Background Image\":\"\",\"Background Image X\":\"0\",\"Background Image Y\":\"0\"}","{\"Command Name\":\"Cancel\",\"Icon\":\"82\",\"Command Symbol\":\"cancel\",\"JS Command\":\"\",\"JS Enable Condition\":\"return true;\",\"JS Show Condition\":\"return true;\",\"Background Image\":\"\",\"Background Image X\":\"0\",\"Background Image Y\":\"0\"}"]
 *
 * @param Command Columns
 * @parent Command Window
 * @type number
 * @desc Number of columns to display in the command window
 * @default 2
 *
 * @param Command Alignment
 * @parent Command Window
 * @type select
 * @option left
 * @option center
 * @option right
 * @default center
 * @desc The alignment of the command text in the command window
 *
 * @param Command Icon Alignment
 * @parent Command Window
 * @type select
 * @option left
 * @option right
 * @default left
 * @desc The alignment of the icon parameter in the command window
 *
 * @param Confirm Window
 * @parent Window Options
 *
 * @param Show Confirm Window
 * @parent Confirm Window
 * @type boolean
 * @desc If true, will ask the player to confirm they want to purchase a spell before the purchase actually occurs
 * @default false
 *
 * @param Confirm As Default
 * @parent Confirm Window
 * @type boolean
 * @desc If true, the confirm window will start with Confirm selected. Otherwise, it will start with Cancel selected.
 * @default true
 *
 * @param Confirm Window Width
 * @parent Confirm Window
 * @type number
 * @max 100
 * @desc Width of the confirm window (as percentage of the screen)
 * @default 50
 *
 * @param Confirm Text
 * @parent Confirm Window
 * @desc Text to show for the confirm option
 * @default Purchase
 *
 * @param No Confirm Text
 * @parent Confirm Window
 * @desc Text to show for the cancel confirm option
 * @default Cancel
 *
 * @param Confirm Title Text
 * @parent Confirm Window
 * @desc Text to show for the title text in the confirm window
 * @default Confirm Purchase
 *
 * @param Actor Window
 * @parent Window Options
 *
 * @param Available Text
 * @text Texto Disponible
 * @parent Actor Window
 * @desc Texto para mostrar cuando el actor puede aprender una habilidad.
 * @default Available
 *
 * @param Already Known Text
 * @text Texto Ya conocido
 * @parent Actor Window
 * @desc Texto para mostrar cuando el actor ya ha aprendido la habilidad.
 * @default Already Known
 *
 * @param Cannot Learn Text
 * @text Texto No puede aprender
 * @parent Actor Window
 * @desc Texto para mostrar cuando el actor no puede aprender una habilidad (debido al tipo de habilidad o alguna otra razón)
 * @default Cannot Learn
 *
 * @param Restricted Text
 * @text Texto Restringido
 * @parent Actor Window
 * @desc Texto para mostrar cuando el actor no puede aprender una habilidad debido a la restricción de etiquetas de notas
 * @default Restricted
 *
 * @param Level Issue Text
 * @parent Actor Window
 * @desc Text to show when the actor cannot learn a skill due to being too low level
 * @default Level Too Low
 *
 * @param Show All Missing Prerequisites
 * @parent Actor Window
 * @type boolean
 * @desc If true, will display all missing prerequisites
 * @default false
 *
 * @param Missing Prerequisites Text
 * @text Texto Requisitos previos que faltan
 * @parent Actor Window
 * @desc Texto para mostrar cuando el actor no puede aprender una habilidad debido a la falta de habilidades previas.
 * @default Missing Prerequisites
 *
 * @param Learned Skills Only
 * @parent Actor Window
 * @type boolean
 * @desc If prerequisites only look at learned skills (not temp skills via equips)
 * @default true
 *
 * @param Draw Character
 * @parent Actor Window
 * @type boolean
 * @desc If the window should draw character sprites. Cannot be used with Draw Face.
 * @default true
 *
 * @param Draw Face
 * @parent Actor Window
 * @type boolean
 * @desc If the window should draw face sprites. Cannot be used with Draw Character
 * @default true
 *
 * @param Face Height
 * @parent Actor Window
 * @type number
 * @min 0
 * @desc The height (and width) to draw face images. 0 = default
 * @default 0
 *
 * @param Integrations
 *
 * @param Shop Background
 * @parent Integrations
 * @desc The [CGMZ] Scene Backgrounds preset id for the skill shop scene
 *
 * @param Controls Window
 * @parent Integrations
 * @desc The [CGMZ] Controls Window preset id for the skill shop scene
 *
 * @param Command Window Settings
 * @parent Integrations
 * @desc The [CGMZ] Window Settings preset id for the command window
 *
 * @param Confirm Window Settings
 * @parent Integrations
 * @desc The [CGMZ] Window Settings preset id for the confirm window
 *
 * @param Buy Window Settings
 * @parent Integrations
 * @desc The [CGMZ] Window Settings preset id for the buy window
 *
 * @param Actor Window Settings
 * @parent Integrations
 * @desc The [CGMZ] Window Settings preset id for the actor window
 *
 * @param Gold Window Settings
 * @parent Integrations
 * @desc The [CGMZ] Window Settings preset id for the gold window
 *
 * @param Help Window Settings
 * @parent Integrations
 * @desc The [CGMZ] Window Settings preset id for the help window
 *
 * @param Command Window Background
 * @parent Integrations
 * @desc The [CGMZ] Window Backgrounds preset id for the command window
 *
 * @param Confirm Window Background
 * @parent Integrations
 * @desc The [CGMZ] Window Backgrounds preset id for the confirm window
 *
 * @param Buy Window Background
 * @parent Integrations
 * @desc The [CGMZ] Window Backgrounds preset id for the buy window
 *
 * @param Actor Window Background
 * @parent Integrations
 * @desc The [CGMZ] Window Backgrounds preset id for the actor window
 *
 * @param Gold Window Background
 * @parent Integrations
 * @desc The [CGMZ] Window Backgrounds preset id for the gold window
 *
 * @param Help Window Background
 * @parent Integrations
 * @desc The [CGMZ] Window Backgrounds preset id for the help window
*/
/*~struct~Skill:es
 * @param skill
 * @text Habilidad
 * @type skill
 * @default 0
 * @desc La habilidad para ofrecer en la tienda de habilidades.
 *
 * @param price
 * @text Precio
 * @type number
 * @min -1
 * @default -1
 * @desc Precio de la habilidad. -1 = default price
*/
/*~struct~SkillDefault:es
 * @param skill
 * @type skill
 * @default 0
 * @desc The skill to set a default price for
 *
 * @param price
 * @type number
 * @min 0
 * @default 0
 * @desc The default price of the skill
*/
/*~struct~Handler:es
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
 * @param JS Enable Condition
 * @type multiline_string
 * @default return true;
 * @desc JavaScript to run to determine if the command is enabled
 *
 * @param JS Show Condition
 * @type multiline_string
 * @default return true;
 * @desc JavaScript to run to determine if the command is shown
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
*/
Imported.CGMZ_SkillShop = true;
CGMZ.Versions["Skill Shop"] = "1.3.0";
CGMZ.SkillShop = {};
CGMZ.SkillShop.parameters = PluginManager.parameters('CGMZ_SkillShop');
CGMZ.SkillShop.AvailableText = CGMZ.SkillShop.parameters["Available Text"];
CGMZ.SkillShop.AlreadyKnownText = CGMZ.SkillShop.parameters["Already Known Text"];
CGMZ.SkillShop.CannotLearnText = CGMZ.SkillShop.parameters["Cannot Learn Text"];
CGMZ.SkillShop.RestrictedText = CGMZ.SkillShop.parameters["Restricted Text"];
CGMZ.SkillShop.MissingPrerequisitesText = CGMZ.SkillShop.parameters["Missing Prerequisites Text"];
CGMZ.SkillShop.LevelIssueText = CGMZ.SkillShop.parameters["Level Issue Text"];
CGMZ.SkillShop.ConfirmText = CGMZ.SkillShop.parameters["Confirm Text"];
CGMZ.SkillShop.NoConfirmText = CGMZ.SkillShop.parameters["No Confirm Text"];
CGMZ.SkillShop.ConfirmTitleText = CGMZ.SkillShop.parameters["Confirm Title Text"];
CGMZ.SkillShop.CommandAlignment = CGMZ.SkillShop.parameters["Command Alignment"];
CGMZ.SkillShop.CommandIconAlignment = CGMZ.SkillShop.parameters["Command Icon Alignment"];
CGMZ.SkillShop.ShopBackground = CGMZ.SkillShop.parameters["Shop Background"];
CGMZ.SkillShop.ControlsWindow = CGMZ.SkillShop.parameters["Controls Window"];
CGMZ.SkillShop.CommandWindowSettings = CGMZ.SkillShop.parameters["Command Window Settings"];
CGMZ.SkillShop.ConfirmWindowSettings = CGMZ.SkillShop.parameters["Confirm Window Settings"];
CGMZ.SkillShop.BuyWindowSettings = CGMZ.SkillShop.parameters["Buy Window Settings"];
CGMZ.SkillShop.ActorWindowSettings = CGMZ.SkillShop.parameters["Actor Window Settings"];
CGMZ.SkillShop.GoldWindowSettings = CGMZ.SkillShop.parameters["Gold Window Settings"];
CGMZ.SkillShop.HelpWindowSettings = CGMZ.SkillShop.parameters["Help Window Settings"];
CGMZ.SkillShop.CommandWindowBackground = CGMZ.SkillShop.parameters["Command Window Background"];
CGMZ.SkillShop.ConfirmWindowBackground = CGMZ.SkillShop.parameters["Confirm Window Background"];
CGMZ.SkillShop.BuyWindowBackground = CGMZ.SkillShop.parameters["Buy Window Background"];
CGMZ.SkillShop.ActorWindowBackground = CGMZ.SkillShop.parameters["Actor Window Background"];
CGMZ.SkillShop.GoldWindowBackground = CGMZ.SkillShop.parameters["Gold Window Background"];
CGMZ.SkillShop.HelpWindowBackground = CGMZ.SkillShop.parameters["Help Window Background"];
CGMZ.SkillShop.MissingDefaultPrice = Number(CGMZ.SkillShop.parameters["Missing Default Price"]);
CGMZ.SkillShop.FaceHeight = Number(CGMZ.SkillShop.parameters["Face Height"]);
CGMZ.SkillShop.CommandColumns = Number(CGMZ.SkillShop.parameters["Command Columns"]);
CGMZ.SkillShop.ConfirmWindowWidth = Number(CGMZ.SkillShop.parameters["Confirm Window Width"]);
CGMZ.SkillShop.DisableTouchUISpace = (CGMZ.SkillShop.parameters["Disable Touch UI Space"] === 'true');
CGMZ.SkillShop.LearnedSkillsOnly = (CGMZ.SkillShop.parameters["Learned Skills Only"] === 'true');
CGMZ.SkillShop.DrawCharacter = (CGMZ.SkillShop.parameters["Draw Character"] === 'true');
CGMZ.SkillShop.DrawFace = (CGMZ.SkillShop.parameters["Draw Face"] === 'true');
CGMZ.SkillShop.ShowConfirmWindow = (CGMZ.SkillShop.parameters["Show Confirm Window"] === 'true');
CGMZ.SkillShop.ConfirmAsDefault = (CGMZ.SkillShop.parameters["Confirm As Default"] === 'true');
CGMZ.SkillShop.ShowAllMissingPrerequisites = (CGMZ.SkillShop.parameters["Show All Missing Prerequisites"] === 'true');
CGMZ.SkillShop.DefaultSkillPrices = CGMZ_Utils.parseJSON(CGMZ.SkillShop.parameters["Default Skill Prices"], [], "[CGMZ] Skill Shop", "Your Default Skill Prices parameter could not be parsed, default skill prices will not work correctly.");
CGMZ.SkillShop.Commands = CGMZ_Utils.parseJSON(CGMZ.SkillShop.parameters["Skill Shop Commands"], [], "[CGMZ] Skill Shop", "Your Skill Shop Commands parameter had invalid JSON and could not be read.").map((command) => {
	const cmdP = CGMZ_Utils.parseJSON(command, null, "[CGMZ] Skill Shop", "One of your skill shop commands had invalid JSON and could not be read.");
	if(!cmdP) return null;
	const cmd = {};
	cmd.icon = Number(cmdP.Icon);
	cmd.backgroundImage = cmdP["Background Image"];
	cmd.backImgX = Number(cmdP["Background Image X"]);
	cmd.backImgY = Number(cmdP["Background Image Y"]);
	cmd.symbol = cmdP["Command Symbol"] || Math.random().toString(36);
	cmd.name = cmdP["Command Name"];
	cmd.js = cmdP["JS Command"];
	cmd.jsShow = cmdP["JS Show Condition"];
	cmd.jsEnable = cmdP["JS Enable Condition"];
	return cmd;
}).filter(x => !!x);
//=============================================================================
// CGMZ_Temp
//-----------------------------------------------------------------------------
// Register and handling for plugin commands, default price data
//=============================================================================
//-----------------------------------------------------------------------------
// Set up default skill price data
//-----------------------------------------------------------------------------
const CGMZ_SkillShop_CGMZTemp_createPluginData = CGMZ_Temp.prototype.createPluginData;
CGMZ_Temp.prototype.createPluginData = function() {
	CGMZ_SkillShop_CGMZTemp_createPluginData.call(this);
	this._defaultSkillPrices = {};
	for(const skillJSON of CGMZ.SkillShop.DefaultSkillPrices) {
		const settings = CGMZ_Utils.parseJSON(skillJSON, null, "[CGMZ] Skill Shop", "One of your default skill prices could not be parsed due to invalid JSON. Skipping.");
		if(!settings) continue;
		this._defaultSkillPrices[Number(settings.skill)] = Number(settings.price);
	}
};
//-----------------------------------------------------------------------------
// Get a skill default price, or 0 if not exists
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.getDefaultSkillShopPrice = function(skillId) {
	const defaultPrice = this._defaultSkillPrices[skillId];
	return (defaultPrice) ? defaultPrice : CGMZ.SkillShop.MissingDefaultPrice;
};
//-----------------------------------------------------------------------------
// Register Plugin Commands
//-----------------------------------------------------------------------------
const alias_CGMZ_SkillShop_registerPluginCommands = CGMZ_Temp.prototype.registerPluginCommands;
CGMZ_Temp.prototype.registerPluginCommands = function() {
	alias_CGMZ_SkillShop_registerPluginCommands.call(this);
	PluginManager.registerCommand("CGMZ_SkillShop", "Call Scene", this.pluginCommandSkillShopCallScene);
};
//-----------------------------------------------------------------------------
// Call skill shop scene
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandSkillShopCallScene = function(args) {
	SceneManager.push(CGMZ_Scene_SkillShop);
	SceneManager.prepareNextScene(JSON.parse(args.goods), args.Background);
};
//=============================================================================
// CGMZ_Scene_SkillShop
//-----------------------------------------------------------------------------
// Handle the skill shop scene
//=============================================================================
function CGMZ_Scene_SkillShop() {
	this.initialize.apply(this, arguments);
}
CGMZ_Scene_SkillShop.prototype = Object.create(Scene_MenuBase.prototype);
CGMZ_Scene_SkillShop.prototype.constructor = CGMZ_Scene_SkillShop;
//-----------------------------------------------------------------------------
// Initialize
//-----------------------------------------------------------------------------
CGMZ_Scene_SkillShop.prototype.initialize = function() {
	Scene_MenuBase.prototype.initialize.call(this);
};
//-----------------------------------------------------------------------------
// Prepare the scene
//-----------------------------------------------------------------------------
CGMZ_Scene_SkillShop.prototype.prepare = function(goods, bg) {
	this._bg = bg;
	this._goods = [];
	goods.forEach(good => {
		const skillObj = JSON.parse(good);
		this._goods.push(skillObj);
	});
	this._item = null;
};
//-----------------------------------------------------------------------------
// Check if should make room for Touch UI
//-----------------------------------------------------------------------------
CGMZ_Scene_SkillShop.prototype.hasTouchUI = function() {
	return !CGMZ.SkillShop.DisableTouchUISpace || ConfigManager.touchUI;
};
//-----------------------------------------------------------------------------
// Create skill shop windows
//-----------------------------------------------------------------------------
CGMZ_Scene_SkillShop.prototype.create = function() {
	Scene_MenuBase.prototype.create.call(this);
	this.loadActorImages();
	this.createHelpWindow();
    this.createGoldWindow();
	this.createCommandWindow();
	this.createBuyWindow();
	this.createActorWindow();
	this.createConfirmWindow();
};
//-----------------------------------------------------------------------------
// Load Actor Images
//-----------------------------------------------------------------------------
CGMZ_Scene_SkillShop.prototype.loadActorImages = function() {
	const partyMembers = $gameParty.members();
	for(const actor of partyMembers) {
		const faceBitmap = ImageManager.loadFace(actor.faceName());
		const charBitmap = ImageManager.loadCharacter(actor.characterName());
	}
};
//-----------------------------------------------------------------------------
// Create help window
//-----------------------------------------------------------------------------
CGMZ_Scene_SkillShop.prototype.createHelpWindow = function() {
    const rect = this.helpWindowRect();
    this._helpWindow = new CGMZ_Window_SkillShopHelp(rect);
    this.addWindow(this._helpWindow);
};
//-----------------------------------------------------------------------------
// Create gold window
//-----------------------------------------------------------------------------
CGMZ_Scene_SkillShop.prototype.createGoldWindow = function() {
	const rect = this.goldWindowRect();
	this._goldWindow = new CGMZ_Window_SkillShopGold(rect);
	this.addWindow(this._goldWindow);
};
//-----------------------------------------------------------------------------
// Get gold window rect
//-----------------------------------------------------------------------------
CGMZ_Scene_SkillShop.prototype.goldWindowRect = function() {
    const ww = this.mainCommandWidth();
    const wh = this.calcWindowHeight(1, true);
    const wx = Graphics.boxWidth - ww;
    const wy = this.hasTouchUI() ? this.mainAreaTop() : 0;
    return new Rectangle(wx, wy, ww, wh);
};
//-----------------------------------------------------------------------------
// Create command window
//-----------------------------------------------------------------------------
CGMZ_Scene_SkillShop.prototype.createCommandWindow = function() {
    const rect = this.commandWindowRect();
    this._commandWindow = new CGMZ_Window_SkillShopCommand(rect);
    this._commandWindow.setHandler("buy", this.commandBuy.bind(this));
    this._commandWindow.setHandler("cancel", this.popScene.bind(this));
	for(const cmd of CGMZ.SkillShop.Commands) {
		if(this.isCustomCommand(cmd.symbol)) {
			this._commandWindow.setHandler(cmd.symbol, this.commandCustom.bind(this));
		}
	}
    this.addWindow(this._commandWindow);
};
//-----------------------------------------------------------------------------
// Get command window rect
//-----------------------------------------------------------------------------
CGMZ_Scene_SkillShop.prototype.commandWindowRect = function() {
    const wx = 0;
    const wy = this._goldWindow.y;
    const ww = this._goldWindow.x;
    const wh = this._goldWindow.height;
    return new Rectangle(wx, wy, ww, wh);
};
//-----------------------------------------------------------------------------
// Create list (buy) window
//-----------------------------------------------------------------------------
CGMZ_Scene_SkillShop.prototype.createBuyWindow = function() {
    const rect = this.buyWindowRect();
    this._buyWindow = new CGMZ_Window_SkillShopBuy(rect);
    this._buyWindow.setHandler("ok", this.onBuyOk.bind(this));
    this._buyWindow.setHandler("cancel", this.onBuyCancel.bind(this));
	this._buyWindow.setupGoods(this._goods);
	this._buyWindow.setHelpWindow(this._helpWindow);
    this.addWindow(this._buyWindow);
};
//-----------------------------------------------------------------------------
// Get list (buy) window rect
//-----------------------------------------------------------------------------
CGMZ_Scene_SkillShop.prototype.buyWindowRect = function() {
    const wx = 0;
    const wy = this._goldWindow.y + this._goldWindow.height;
    const ww = Graphics.boxWidth * 6 / 10;
    const wh = Graphics.boxHeight - this._goldWindow.y - this._goldWindow.height - this._helpWindow.height;
    return new Rectangle(wx, wy, ww, wh);
};
//-----------------------------------------------------------------------------
// Create list (actor) window
//-----------------------------------------------------------------------------
CGMZ_Scene_SkillShop.prototype.createActorWindow = function() {
    const rect = this.actorWindowRect();
    this._actorWindow = new CGMZ_Window_SkillShopActor(rect);
    this._actorWindow.setHandler("ok", this.onActorOk.bind(this));
    this._actorWindow.setHandler("cancel", this.onActorCancel.bind(this));
	this._buyWindow.setActorWindow(this._actorWindow);
    this.addWindow(this._actorWindow);
};
//-----------------------------------------------------------------------------
// Get list (actor) window rect
//-----------------------------------------------------------------------------
CGMZ_Scene_SkillShop.prototype.actorWindowRect = function() {
    const wx = this._buyWindow.width;
    const wy = this._buyWindow.y;
    const ww = Graphics.boxWidth - wx;
    const wh = this._buyWindow.height;
    return new Rectangle(wx, wy, ww, wh);
};
//-----------------------------------------------------------------------------
// Create confirm window
//-----------------------------------------------------------------------------
CGMZ_Scene_SkillShop.prototype.createConfirmWindow = function() {
    const rect = this.confirmWindowRect();
    this._confirmWindow = new CGMZ_Window_SkillShopConfirm(rect);
    this._confirmWindow.setHandler("ok", this.onConfirmOk.bind(this));
    this._confirmWindow.setHandler("cancel", this.onConfirmCancel.bind(this));
    this.addWindow(this._confirmWindow);
};
//-----------------------------------------------------------------------------
// Get confirm window rect
//-----------------------------------------------------------------------------
CGMZ_Scene_SkillShop.prototype.confirmWindowRect = function() {
    const ww = Graphics.boxWidth * (CGMZ.SkillShop.ConfirmWindowWidth / 100.0)
    const wh = this.calcMixedHeight(1, 1);
	const wx = Graphics.boxWidth / 2 - ww / 2;
    const wy = Graphics.boxHeight / 2 - wh / 2;
    return new Rectangle(wx, wy, ww, wh);
};
//-----------------------------------------------------------------------------
// Determine if command is a custom command in need of custom handler
//-----------------------------------------------------------------------------
CGMZ_Scene_SkillShop.prototype.isCustomCommand = function(symbol) {
	return (symbol !== 'cancel' && symbol !== 'buy');
};
//-----------------------------------------------------------------------------
// Handling for custom Commands added through the plugin
//-----------------------------------------------------------------------------
CGMZ_Scene_SkillShop.prototype.commandCustom = function() {
	for(const cmd of CGMZ.SkillShop.Commands) {
		if(this._commandWindow.currentSymbol() === cmd.symbol) {
			try {
				const hookFunc = new Function(cmd.js);
				hookFunc.call(this);
			}
			catch (e) {
				const origin = "[CGMZ] Skill Shop";
				const suggestion = "Check your JavaScript command";
				CGMZ_Utils.reportError(e.message, origin, suggestion);
			}
			break;
		}
	}
};
//-----------------------------------------------------------------------------
// Handling for the buy command
//-----------------------------------------------------------------------------
CGMZ_Scene_SkillShop.prototype.commandBuy = function() {
    this._buyWindow.setMoney(this.money());
    this._buyWindow.activate();
	this._buyWindow.select(0);
};
//-----------------------------------------------------------------------------
// Handling for buy window OK (skill selected)
//-----------------------------------------------------------------------------
CGMZ_Scene_SkillShop.prototype.onBuyOk = function() {
    this._actorWindow.activate();
	this._actorWindow.select(0);
	this._actorWindow.ensureCursorVisible(true);
};
//-----------------------------------------------------------------------------
// Handling for buy window cancel (back out to command window)
//-----------------------------------------------------------------------------
CGMZ_Scene_SkillShop.prototype.onBuyCancel = function() {
    this._commandWindow.activate();
	this._buyWindow.deselect();
    this._actorWindow.setItem(null);
    this._helpWindow.clear();
};
//-----------------------------------------------------------------------------
// Handling for actor window OK (purchase)
//-----------------------------------------------------------------------------
CGMZ_Scene_SkillShop.prototype.onActorOk = function() {
	if(CGMZ.SkillShop.ShowConfirmWindow) {
		this._actorWindow.deactivate();
		this._confirmWindow.show();
		this._confirmWindow.activate();
		(CGMZ.SkillShop.ConfirmAsDefault) ? this._confirmWindow.select(0) : this._confirmWindow.select(1);
	} else {
		this.confirmPurchase();
	}
};
//-----------------------------------------------------------------------------
// Handling for confirm window cancel
//-----------------------------------------------------------------------------
CGMZ_Scene_SkillShop.prototype.onConfirmCancel = function() {
	this._confirmWindow.deactivate();
	this._confirmWindow.hide();
	this._actorWindow.activate();
};
//-----------------------------------------------------------------------------
// Handling for confirm window cancel
//-----------------------------------------------------------------------------
CGMZ_Scene_SkillShop.prototype.onConfirmOk = function() {
	const index = this._confirmWindow.index();
	this._confirmWindow.deactivate();
	this._confirmWindow.hide();
	this.confirmPurchase();
};
//-----------------------------------------------------------------------------
// Handling for when a purchase is confirmed (either through actor or confirm window)
//-----------------------------------------------------------------------------
CGMZ_Scene_SkillShop.prototype.confirmPurchase = function() {
	const skill = this._buyWindow.item();
	const price = this._buyWindow.price(skill);
	const actor = this._actorWindow.item();
	SoundManager.playShop();
	this.loseGold(price);
	actor.learnSkill(skill.id);
	this._goldWindow.refresh();
	this._buyWindow.setMoney(this.money());
    this._actorWindow.refresh();
	this._actorWindow.deselect();
	this._buyWindow.activate();
};
//-----------------------------------------------------------------------------
// Handling for buy window cancel (back out to command window)
//-----------------------------------------------------------------------------
CGMZ_Scene_SkillShop.prototype.onActorCancel = function() {
	this._buyWindow.activate();
	this._actorWindow.deselect();
};
//-----------------------------------------------------------------------------
// Lose gold, account for currency system currency if imported
//-----------------------------------------------------------------------------
CGMZ_Scene_SkillShop.prototype.loseGold = function(price) {
	if(Imported.CGMZ_CurrencySystem) {
		const mainCurrency = $cgmz.mainCurrency();
		const currency = $cgmz.getCurrency(mainCurrency);
		if(mainCurrency !== "default" && currency) {
			currency.loseCurrency(price);
		} else {
			$gameParty.loseGold(price);
		}
	} else {
		$gameParty.loseGold(price);
	}
};
//-----------------------------------------------------------------------------
// Get the money value
//-----------------------------------------------------------------------------
CGMZ_Scene_SkillShop.prototype.money = function() {
    return this._goldWindow.value();
};
//-----------------------------------------------------------------------------
// Calculation for mixed window height
//-----------------------------------------------------------------------------
CGMZ_Scene_SkillShop.prototype.calcMixedHeight = function(selectableLines, staticLines) {
	return this.calcWindowHeight(selectableLines, true) + this.calcWindowHeight(staticLines, false) - $gameSystem.windowPadding() * 2;
};
//-----------------------------------------------------------------------------
// Get the shop scene's custom scene background
// No need to check if Scene Backgrounds is installed because this custom func
// is only called by that plugin
//-----------------------------------------------------------------------------
CGMZ_Scene_SkillShop.prototype.CGMZ_getCustomSceneBackground = function() {
	const bg = (this._bg) ? this._bg : CGMZ.SkillShop.ShopBackground;
	return $cgmzTemp.sceneBackgroundPresets[bg];
};
//-----------------------------------------------------------------------------
// Get controls window preset for [CGMZ] Controls Window
// No need to check if plugin is installed because this custom func is only called by that plugin
//-----------------------------------------------------------------------------
CGMZ_Scene_SkillShop.prototype.CGMZ_getControlsWindowOtherPreset = function() {
	return $cgmzTemp.getControlWindowPresetOther(CGMZ.SkillShop.ControlsWindow);
};
//=============================================================================
// CGMZ_Window_SkillShopCommand
//-----------------------------------------------------------------------------
// Skill Shop Command Window
//=============================================================================
function CGMZ_Window_SkillShopCommand() {
    this.initialize(...arguments);
}
CGMZ_Window_SkillShopCommand.prototype = Object.create(Window_HorzCommand.prototype);
CGMZ_Window_SkillShopCommand.prototype.constructor = CGMZ_Window_SkillShopCommand;
//-----------------------------------------------------------------------------
// Initialize
//-----------------------------------------------------------------------------
CGMZ_Window_SkillShopCommand.prototype.initialize = function(rect) {
    Window_HorzCommand.prototype.initialize.call(this, rect);
	if(Imported.CGMZ_WindowSettings && CGMZ.SkillShop.CommandWindowSettings) this.CGMZ_setWindowSettings(CGMZ.SkillShop.CommandWindowSettings);
	if(Imported.CGMZ_WindowBackgrounds && CGMZ.SkillShop.CommandWindowBackground) this.CGMZ_setWindowBackground(CGMZ.SkillShop.CommandWindowBackground);
	this.refresh();
};
//-----------------------------------------------------------------------------
// Create the command list
//-----------------------------------------------------------------------------
CGMZ_Window_SkillShopCommand.prototype.makeCommandList = function() {
    for(const cmd of CGMZ.SkillShop.Commands) {
		const showFunc = new Function(cmd.jsShow);
		if(showFunc.call(this)) {
			const enabledFunc = new Function(cmd.jsEnable);
			const enabled = enabledFunc.call(this);
			this.addCommand(cmd.name, cmd.symbol, enabled, {icon: cmd.icon, img: cmd.backgroundImage, imgX: cmd.backImgX, imgY: cmd.backImgY});
		}
	}
};
//-----------------------------------------------------------------------------
// Change columns
//-----------------------------------------------------------------------------
CGMZ_Window_SkillShopCommand.prototype.maxCols = function() {
    return CGMZ.SkillShop.CommandColumns;
};
//-----------------------------------------------------------------------------
// Change alignment of command text
//-----------------------------------------------------------------------------
CGMZ_Window_SkillShopCommand.prototype.itemTextAlign = function() {
    return CGMZ.SkillShop.CommandAlignment;
};
//-----------------------------------------------------------------------------
// Get the command icon
//-----------------------------------------------------------------------------
CGMZ_Window_SkillShopCommand.prototype.CGMZ_icon = function(index) {
	return this._list[index].ext?.icon;
};
//-----------------------------------------------------------------------------
// Get selectable cgmz options
//-----------------------------------------------------------------------------
CGMZ_Window_SkillShopCommand.prototype.CGMZ_getSelectableCGMZOptions = function(index) {
	const ext = this._list[index].ext;
	if(ext && ext.img) {
		const bg = {
			img: ext.img,
			imgX: ext.imgX,
			imgY: ext.imgY
		}
		return {bg: bg};
	}
	return Window_HorzCommand.prototype.CGMZ_getSelectableCGMZOptions.call(this, index);
};
//-----------------------------------------------------------------------------
// Draw new command item with potential icon and text codes
//-----------------------------------------------------------------------------
CGMZ_Window_SkillShopCommand.prototype.drawItem = function(index) {
	const rect = this.itemLineRect(index);
	const align = this.itemTextAlign();
	const icon = this.CGMZ_icon(index);
	this.resetTextColor();
	this.changePaintOpacity(this.isCommandEnabled(index));
	if(icon) {
		const iconX = (CGMZ.SkillShop.CommandIconAlignment === 'left') ? rect.x : rect.x + rect.width - ImageManager.iconWidth;
		this.drawIcon(icon, iconX, rect.y + 2);
		rect.x += (ImageManager.iconWidth + 2) * (CGMZ.SkillShop.CommandIconAlignment === 'left');
		rect.width -= ImageManager.iconWidth + 2;
	}
	this.CGMZ_drawTextLine(this.commandName(index), rect.x, rect.y, rect.width, align);
};
//=============================================================================
// CGMZ_Window_SkillShopBuy
//-----------------------------------------------------------------------------
// Skill Shop Buy Window
//=============================================================================
function CGMZ_Window_SkillShopBuy() {
    this.initialize(...arguments);
}
CGMZ_Window_SkillShopBuy.prototype = Object.create(Window_Selectable.prototype);
CGMZ_Window_SkillShopBuy.prototype.constructor = CGMZ_Window_SkillShopBuy;
//-----------------------------------------------------------------------------
// Initialize the window
//-----------------------------------------------------------------------------
CGMZ_Window_SkillShopBuy.prototype.initialize = function(rect) {
    Window_Selectable.prototype.initialize.call(this, rect);
	if(Imported.CGMZ_WindowSettings && CGMZ.SkillShop.BuyWindowSettings) this.CGMZ_setWindowSettings(CGMZ.SkillShop.BuyWindowSettings);
	if(Imported.CGMZ_WindowBackgrounds && CGMZ.SkillShop.BuyWindowBackground) this.CGMZ_setWindowBackground(CGMZ.SkillShop.BuyWindowBackground);
	this._money = $gameParty.gold();
	if(Imported.CGMZ_CurrencySystem) {
		const mainCurrency = $cgmz.mainCurrency();
		const currency = $cgmz.getCurrency(mainCurrency);
		if(mainCurrency !== "default" && currency) {
			this._money = currency.amount();
		}
	}
};
//-----------------------------------------------------------------------------
// Set up the skills for sale
//-----------------------------------------------------------------------------
CGMZ_Window_SkillShopBuy.prototype.setupGoods = function(shopGoods) {
    this._shopGoods = shopGoods;
    this.refresh();
};
//-----------------------------------------------------------------------------
// Get max item count
//-----------------------------------------------------------------------------
CGMZ_Window_SkillShopBuy.prototype.maxItems = function() {
    return this._data ? this._data.length : 1;
};
//-----------------------------------------------------------------------------
// Get current item
//-----------------------------------------------------------------------------
CGMZ_Window_SkillShopBuy.prototype.item = function() {
    return this.itemAt(this.index());
};
//-----------------------------------------------------------------------------
// Get item at index
//-----------------------------------------------------------------------------
CGMZ_Window_SkillShopBuy.prototype.itemAt = function(index) {
    return this._data && index >= 0 ? this._data[index] : null;
};
//-----------------------------------------------------------------------------
// Set the money
//-----------------------------------------------------------------------------
CGMZ_Window_SkillShopBuy.prototype.setMoney = function(money) {
    this._money = money;
    this.refresh();
};
//-----------------------------------------------------------------------------
// Determine if the current item is enabled
//-----------------------------------------------------------------------------
CGMZ_Window_SkillShopBuy.prototype.isCurrentItemEnabled = function() {
    return this.isEnabled(this._data[this.index()]);
};
//-----------------------------------------------------------------------------
// Get the price
//-----------------------------------------------------------------------------
CGMZ_Window_SkillShopBuy.prototype.price = function(item) {
    return this._price[this._data.indexOf(item)] || 0;
};
//-----------------------------------------------------------------------------
// Determine if item is enabled
//-----------------------------------------------------------------------------
CGMZ_Window_SkillShopBuy.prototype.isEnabled = function(item) {
	return (item && this.price(item) <= this._money);
};
//-----------------------------------------------------------------------------
// Refresh window
//-----------------------------------------------------------------------------
CGMZ_Window_SkillShopBuy.prototype.refresh = function() {
    this.makeItemList();
    Window_Selectable.prototype.refresh.call(this);
};
//-----------------------------------------------------------------------------
// Make the list of all skills for sale and their prices
//-----------------------------------------------------------------------------
CGMZ_Window_SkillShopBuy.prototype.makeItemList = function() {
    this._data = [];
    this._price = [];
    this._shopGoods.forEach(skillObj => {
		const skillId = Number(skillObj.skill);
        const skill = $dataSkills[skillId];
        if (skill) {
            this._data.push(skill);
			const price = Number(skillObj.price);
            this._price.push((price >= 0) ? price : this.getDefaultPrice(skillId));
        }
    });
};
//-----------------------------------------------------------------------------
// Get the default price of a skill, or 0 if not found
//-----------------------------------------------------------------------------
CGMZ_Window_SkillShopBuy.prototype.getDefaultPrice = function(skillId) {
    return $cgmzTemp.getDefaultSkillShopPrice(skillId);
};
//-----------------------------------------------------------------------------
// Draw the skill and price
//-----------------------------------------------------------------------------
CGMZ_Window_SkillShopBuy.prototype.drawItem = function(index) {
    const item = this.itemAt(index);
    const price = this.price(item);
    const rect = this.itemLineRect(index);
    const priceWidth = this.priceWidth();
    const priceX = rect.x + rect.width - priceWidth;
    const nameWidth = rect.width - priceWidth;
    this.changePaintOpacity(this.isEnabled(item));
    this.drawItemName(item, rect.x, rect.y, nameWidth);
    this.drawText(CGMZ_Utils.numberSplit(price), priceX, rect.y, priceWidth, "right");
    this.changePaintOpacity(true);
};
//-----------------------------------------------------------------------------
// Width of the price
//-----------------------------------------------------------------------------
CGMZ_Window_SkillShopBuy.prototype.priceWidth = function() {
    return 96;
};
//-----------------------------------------------------------------------------
// Set the actor window
//-----------------------------------------------------------------------------
CGMZ_Window_SkillShopBuy.prototype.setActorWindow = function(actorWindow) {
    this._actorWindow = actorWindow;
    this.callUpdateHelp();
};
//-----------------------------------------------------------------------------
// Update the help and actor windows
//-----------------------------------------------------------------------------
CGMZ_Window_SkillShopBuy.prototype.updateHelp = function() {
    this.setHelpWindowItem(this.item());
    if (this._actorWindow) {
        this._actorWindow.setItem(this.item());
    }
};
//=============================================================================
// CGMZ_Window_SkillShopActor
//-----------------------------------------------------------------------------
// Skill Shop Actor Window
//=============================================================================
function CGMZ_Window_SkillShopActor() {
    this.initialize(...arguments);
}
CGMZ_Window_SkillShopActor.prototype = Object.create(CGMZ_Window_Selectable.prototype);
CGMZ_Window_SkillShopActor.prototype.constructor = CGMZ_Window_SkillShopActor;
//-----------------------------------------------------------------------------
// Initialize the window
//-----------------------------------------------------------------------------
CGMZ_Window_SkillShopActor.prototype.initialize = function(rect) {
    CGMZ_Window_Selectable.prototype.initialize.call(this, rect);
	if(Imported.CGMZ_WindowSettings && CGMZ.SkillShop.ActorWindowSettings) this.CGMZ_setWindowSettings(CGMZ.SkillShop.ActorWindowSettings);
	if(Imported.CGMZ_WindowBackgrounds && CGMZ.SkillShop.ActorWindowBackground) this.CGMZ_setWindowBackground(CGMZ.SkillShop.ActorWindowBackground);
    this._skill = null;
	this._characterTimer = 0;
	this._characterXOffset = 0;
	this._isNegativeCharacter = false;
};
//-----------------------------------------------------------------------------
// Set the skill to buy
//-----------------------------------------------------------------------------
CGMZ_Window_SkillShopActor.prototype.setItem = function(skill) {
	if(this._skill && skill && this._skill.id === skill.id) return;
    this._skill = skill;
	this.scrollTo(0, 0);
	this.refresh();
};
//-----------------------------------------------------------------------------
// Get the height of an item
//-----------------------------------------------------------------------------
CGMZ_Window_SkillShopActor.prototype.itemHeight = function(index) {
	const textHeight = this.CGMZ_calcTextHeight(index);
	const characterHeight = (CGMZ.SkillShop.DrawCharacter) ? this.getMaxCharacterHeight() : 0;
	const faceHeight = (CGMZ.SkillShop.DrawFace) ? (CGMZ.SkillShop.FaceHeight > 0) ? CGMZ.SkillShop.FaceHeight + 8 : ImageManager.faceHeight + 8 : 0;
	return Math.max(textHeight, characterHeight, faceHeight);
};
//-----------------------------------------------------------------------------
// Get max character height
//-----------------------------------------------------------------------------
CGMZ_Window_SkillShopActor.prototype.getMaxCharacterHeight = function() {
    let height = 0;
	for(const actor of $gameParty.members()) {
		const characterName = actor.characterName();
		const characterIndex = actor.characterIndex();
		const isBigCharacter = ImageManager.isBigCharacter(characterName);
		const bitmap = ImageManager.loadCharacter(this._characterName);
		const charHeight = (isBigCharacter) ?  Math.floor(bitmap.height / 4) : Math.floor(bitmap.height / 8);
		height = Math.max(height, charHeight);
	}
	return height;
};
//-----------------------------------------------------------------------------
// Calculate the text height
//-----------------------------------------------------------------------------
CGMZ_Window_SkillShopActor.prototype.CGMZ_calcTextHeight = function(index) {
    let height = Window_Selectable.prototype.itemHeight.call(this) + Window_Scrollable.prototype.itemHeight.call(this);
	const actor = this.itemAt(index);
	if(index < 0 || !actor || !this._skill) return height;
	if(actor.hasSkill(this._skill.id)) return height;
	if(!actor.skillTypes().includes(this._skill.stypeId)) return height;
	if(actor.isCGMZSkillShopRestricted(this._skill.id) || actor.isCGMZSkillShopRestrictedByClass(this._skill.id)) return height;
	if(this.levelRequirementNotMet(actor)) return height;
	if(this.missingSkillPrerequisites(actor)) {
		const missingPrereqString = this.getMissingPrerequisiteString(actor);
		return height + this.lineHeight() * Math.ceil(this.textSizeEx(missingPrereqString).width / (Window_Selectable.prototype.itemWidth.call(this) - this.itemPadding() * 2));
	}
	return height;
};
//-----------------------------------------------------------------------------
// Get missing prereq string for an actor
//-----------------------------------------------------------------------------
CGMZ_Window_SkillShopActor.prototype.getMissingPrerequisiteString = function(actor) {
	let string = "";
	if(!CGMZ.SkillShop.ShowAllMissingPrerequisites) return "";
	let needSpace = false;
	for(const idS of this._skill.meta.cgmzssprereq.split(",")) {
		if(needSpace) string += " ";
		const id = Number(idS);
		const skillObj = $dataSkills[id];
		if(CGMZ.SkillShop.LearnedSkillsOnly) {
			if(!actor.isLearnedSkill(id)) {
				string += `\\i[${skillObj.iconIndex}]${skillObj.name}`;
				needSpace = true;
			}
		} else {
			if(!actor.hasSkill(id)) {
				string += `\\i[${skillObj.iconIndex}]${skillObj.name}`;
				needSpace = true;
			}
		}
	}
	return string;
};
//-----------------------------------------------------------------------------
// Get max item count
//-----------------------------------------------------------------------------
CGMZ_Window_SkillShopActor.prototype.maxItems = function() {
    return this._data ? this._data.length : 1;
};
//-----------------------------------------------------------------------------
// Get current item
//-----------------------------------------------------------------------------
CGMZ_Window_SkillShopActor.prototype.item = function() {
    return this.itemAt(this.index());
};
//-----------------------------------------------------------------------------
// Get item at index
//-----------------------------------------------------------------------------
CGMZ_Window_SkillShopActor.prototype.itemAt = function(index) {
    return this._data && index >= 0 ? this._data[index] : null;
};
//-----------------------------------------------------------------------------
// Determine if the current item is enabled
//-----------------------------------------------------------------------------
CGMZ_Window_SkillShopActor.prototype.isCurrentItemEnabled = function() {
    return this.isEnabled(this._data[this.index()]);
};
//-----------------------------------------------------------------------------
// Determine if item is enabled
//-----------------------------------------------------------------------------
CGMZ_Window_SkillShopActor.prototype.isEnabled = function(actor) {
	if(!this._skill || !actor) return false;
	if(!actor.skillTypes().includes(this._skill.stypeId)) return false;
	if(actor.hasSkill(this._skill.id)) return false;
	if(actor.isCGMZSkillShopRestricted(this._skill.id)) return false;
	if(this.missingSkillPrerequisites(actor)) return false;
	if(this.levelRequirementNotMet(actor)) return false;
	if(actor.isCGMZSkillShopRestrictedByClass(this._skill.id)) return false;
	return true;
};
//-----------------------------------------------------------------------------
// Check skill prereqs
//-----------------------------------------------------------------------------
CGMZ_Window_SkillShopActor.prototype.missingSkillPrerequisites = function(actor) {
    if(this._skill.meta?.cgmzssprereq) {
		for(const id of this._skill.meta.cgmzssprereq.split(",")) {
			if(CGMZ.SkillShop.LearnedSkillsOnly) {
				if(!actor.isLearnedSkill(Number(id))) return true;
			} else {
				if(!actor.hasSkill(Number(id))) return true;
			}
		}
	}
	return false;
};
//-----------------------------------------------------------------------------
// Check skill level req
//-----------------------------------------------------------------------------
CGMZ_Window_SkillShopActor.prototype.levelRequirementNotMet = function(actor) {
	const meta = this._skill.meta?.cgmzsslevel;
    if(meta) {
		const level = Number(meta);
		if(actor.level < level) return true;
	}
	return false;
};
//-----------------------------------------------------------------------------
// Refresh window
//-----------------------------------------------------------------------------
CGMZ_Window_SkillShopActor.prototype.refresh = function() {
	if(!this._skill) {
		this.contents.clear();
		this.contentsBack.clear();
	} else {
		this.makeItemList();
		CGMZ_Window_Selectable.prototype.refresh.call(this);
	}
};
//-----------------------------------------------------------------------------
// Make the list of all actors
//-----------------------------------------------------------------------------
CGMZ_Window_SkillShopActor.prototype.makeItemList = function() {
    this._data = $gameParty.members();
};
//-----------------------------------------------------------------------------
// Draw the actor info
//-----------------------------------------------------------------------------
CGMZ_Window_SkillShopActor.prototype.drawItem = function(index) {
    const actor = this.itemAt(index);
    const rect = this.itemRectWithPadding(index);
	if(!actor || !this._skill) return;
    this.changePaintOpacity(this.isEnabled(actor));
	if(CGMZ.SkillShop.DrawCharacter) {
		const frameOffset = (this.index() === index) ? this._characterXOffset : 0;
		this.CGMZ_drawCharacter(actor.characterName(), actor.characterIndex(), rect.x + rect.width - 50, rect.y + rect.height - 4, frameOffset);
	} else if(CGMZ.SkillShop.DrawFace) {
		const height = (CGMZ.SkillShop.FaceHeight > 0) ? CGMZ.SkillShop.FaceHeight : ImageManager.faceHeight;
		const width = (CGMZ.SkillShop.FaceHeight > 0) ? CGMZ.SkillShop.FaceHeight : ImageManager.faceWidth;
		this.drawFace(actor.faceName(), actor.faceIndex(), rect.x + rect.width - width, rect.y + 4, width, height);
	}
    this.CGMZ_drawTextLine(actor._name, rect.x, rect.y, rect.width);
	if(actor.hasSkill(this._skill.id)) {
		this.CGMZ_drawTextLine(CGMZ.SkillShop.AlreadyKnownText, rect.x, rect.y + this.lineHeight(), rect.width);
	} else if(!actor.skillTypes().includes(this._skill.stypeId)) {
		this.CGMZ_drawTextLine(CGMZ.SkillShop.CannotLearnText, rect.x, rect.y + this.lineHeight(), rect.width);
	} else if(actor.isCGMZSkillShopRestricted(this._skill.id) || actor.isCGMZSkillShopRestrictedByClass(this._skill.id)) {
		this.CGMZ_drawTextLine(CGMZ.SkillShop.RestrictedText, rect.x, rect.y + this.lineHeight(), rect.width);
	} else if(this.levelRequirementNotMet(actor)) {
		this.CGMZ_drawTextLine(CGMZ.SkillShop.LevelIssueText, rect.x, rect.y + this.lineHeight(), rect.width);
	} else if(this.missingSkillPrerequisites(actor)) {
		this.CGMZ_drawTextLine(CGMZ.SkillShop.MissingPrerequisitesText, rect.x, rect.y + this.lineHeight(), rect.width);
		if(CGMZ.SkillShop.ShowAllMissingPrerequisites) {
			const missingPrereqString = this.getMissingPrerequisiteString(actor);
			this.CGMZ_drawText(missingPrereqString, rect.x, rect.x, rect.y + this.lineHeight() * 2, rect.width);
		}
	} else {
		this.CGMZ_drawTextLine(CGMZ.SkillShop.AvailableText, rect.x, rect.y + this.lineHeight(), rect.width);
	}
    this.changePaintOpacity(true);
};
//-----------------------------------------------------------------------------
// Draw the face  image
//-----------------------------------------------------------------------------
CGMZ_Window_SkillShopActor.prototype.drawFace = function(faceName, faceIndex, x, y, width, height) {
	const bitmap = ImageManager.loadFace(faceName);
	const sw = ImageManager.faceWidth;
	const sh = ImageManager.faceHeight;
	const dx = x;
	const dy = y;
	const sx = Math.floor((faceIndex % 4) * sw);
	const sy = Math.floor(Math.floor(faceIndex / 4) * sh);
	const dw = width;
	const dh = height;
	this.contents.blt(bitmap, sx, sy, sw, sh, dx, dy, dw, dh);
};
//-----------------------------------------------------------------------------
// Update window
//-----------------------------------------------------------------------------
CGMZ_Window_SkillShopActor.prototype.update = function() {
	CGMZ_Window_Selectable.prototype.update.call(this);
	if(CGMZ.SkillShop.DrawCharacter) {
		this.updateCharacterSprite();
	}
};
//-----------------------------------------------------------------------------
// Update window character frame
//-----------------------------------------------------------------------------
CGMZ_Window_SkillShopActor.prototype.updateCharacterSprite = function() {
	if(!this.active || this.index() < 0) return;
	this._characterTimer++;
	if(this._characterTimer > 15) {
		this._characterTimer = 0;
		const modifier = (this._isNegativeCharacter) ? -1 : 1;
		this._characterXOffset += modifier;
		if(this._characterXOffset >= 1) {
			this._isNegativeCharacter = true;
		}
		else if(this._characterXOffset <= -1) {
			this._isNegativeCharacter = false;
		}
		this.redrawCurrentItem();
	}
};
//=============================================================================
// CGMZ_Window_SkillShopConfirm
//-----------------------------------------------------------------------------
// Skill Shop Confirm Window
//=============================================================================
function CGMZ_Window_SkillShopConfirm() {
    this.initialize(...arguments);
}
CGMZ_Window_SkillShopConfirm.prototype = Object.create(Window_Selectable.prototype);
CGMZ_Window_SkillShopConfirm.prototype.constructor = CGMZ_Window_SkillShopConfirm;
//-----------------------------------------------------------------------------
// Initialize the window
//-----------------------------------------------------------------------------
CGMZ_Window_SkillShopConfirm.prototype.initialize = function(rect) {
    Window_Selectable.prototype.initialize.call(this, rect);
	if(Imported.CGMZ_WindowSettings && CGMZ.SkillShop.ConfirmWindowSettings) this.CGMZ_setWindowSettings(CGMZ.SkillShop.ConfirmWindowSettings);
	if(Imported.CGMZ_WindowBackgrounds && CGMZ.SkillShop.ConfirmWindowBackground) this.CGMZ_setWindowBackground(CGMZ.SkillShop.ConfirmWindowBackground);
	this.hide();
	this.deactivate();
	this.refresh();
};
//-----------------------------------------------------------------------------
// Get max item count
//-----------------------------------------------------------------------------
CGMZ_Window_SkillShopConfirm.prototype.maxItems = function() {
    return 2;
};
//-----------------------------------------------------------------------------
// Get max item count
//-----------------------------------------------------------------------------
CGMZ_Window_SkillShopConfirm.prototype.maxCols = function() {
    return 2;
};
//-----------------------------------------------------------------------------
// Adjust the rect down 1 line
//-----------------------------------------------------------------------------
CGMZ_Window_SkillShopConfirm.prototype.itemRect = function(index) {
    const rect = Window_Selectable.prototype.itemRect.call(this, index);
	rect.y += this.lineHeight();
	return rect;
};
//-----------------------------------------------------------------------------
// Route ok processing to cancel if index is cancel
//-----------------------------------------------------------------------------
CGMZ_Window_SkillShopConfirm.prototype.processOk = function() {
	if(this.index() === 1) {
		this.processCancel();
	} else {
        this.updateInputData();
        this.deactivate();
        this.callOkHandler();
	}
};
//-----------------------------------------------------------------------------
// Refresh the window
//-----------------------------------------------------------------------------
CGMZ_Window_SkillShopConfirm.prototype.refresh = function() {
    Window_Selectable.prototype.refresh.call(this);
	this.CGMZ_drawTextLine(CGMZ.SkillShop.ConfirmTitleText, 0, 0, this.contents.width, 'center');
};
//-----------------------------------------------------------------------------
// Draw the actor info
//-----------------------------------------------------------------------------
CGMZ_Window_SkillShopConfirm.prototype.drawItem = function(index) {
    const rect = this.itemRectWithPadding(index);
	const string = (index === 0) ? CGMZ.SkillShop.ConfirmText : CGMZ.SkillShop.NoConfirmText;
	this.CGMZ_drawTextLine(string, rect.x, rect.y, rect.width, 'center');
};
//=============================================================================
// CGMZ_Window_SkillShopGold
//-----------------------------------------------------------------------------
// Skill Shop Gold Window
//=============================================================================
function CGMZ_Window_SkillShopGold() {
    this.initialize(...arguments);
}
CGMZ_Window_SkillShopGold.prototype = Object.create(Window_Gold.prototype);
CGMZ_Window_SkillShopGold.prototype.constructor = CGMZ_Window_SkillShopGold;
//-----------------------------------------------------------------------------
// Initialize the window
//-----------------------------------------------------------------------------
CGMZ_Window_SkillShopGold.prototype.initialize = function(rect) {
    Window_Gold.prototype.initialize.call(this, rect);
	if(Imported.CGMZ_WindowSettings && CGMZ.SkillShop.GoldWindowSettings) this.CGMZ_setWindowSettings(CGMZ.SkillShop.GoldWindowSettings);
	if(Imported.CGMZ_WindowBackgrounds && CGMZ.SkillShop.GoldWindowBackground) this.CGMZ_setWindowBackground(CGMZ.SkillShop.GoldWindowBackground);
};
//=============================================================================
// CGMZ_Window_SkillShopHelp
//-----------------------------------------------------------------------------
// Skill Shop Help Window
//=============================================================================
function CGMZ_Window_SkillShopHelp() {
    this.initialize(...arguments);
}
CGMZ_Window_SkillShopHelp.prototype = Object.create(Window_Help.prototype);
CGMZ_Window_SkillShopHelp.prototype.constructor = CGMZ_Window_SkillShopHelp;
//-----------------------------------------------------------------------------
// Initialize the window
//-----------------------------------------------------------------------------
CGMZ_Window_SkillShopHelp.prototype.initialize = function(rect) {
    Window_Help.prototype.initialize.call(this, rect);
	if(Imported.CGMZ_WindowSettings && CGMZ.SkillShop.HelpWindowSettings) this.CGMZ_setWindowSettings(CGMZ.SkillShop.HelpWindowSettings);
	if(Imported.CGMZ_WindowBackgrounds && CGMZ.SkillShop.HelpWindowBackground) this.CGMZ_setWindowBackground(CGMZ.SkillShop.HelpWindowBackground);
};
//=============================================================================
// Game_Actor
//-----------------------------------------------------------------------------
// Check if actor has restriction for certain skills in skill shop
//=============================================================================
//-----------------------------------------------------------------------------
// Check if actor has restriction note tag
//-----------------------------------------------------------------------------
Game_Actor.prototype.isCGMZSkillShopRestricted = function(skillId) {
	const meta = $dataActors[this._actorId].meta;
	if(!meta || !meta.cgmzssrestriction) return false;
	for(const id of meta.cgmzssrestriction.split(",")) {
		if(Number(id) === skillId) return true;
	}
	return false;
};
//-----------------------------------------------------------------------------
// Check if actor class has restriction note tag
//-----------------------------------------------------------------------------
Game_Actor.prototype.isCGMZSkillShopRestrictedByClass = function(skillId) {
	const meta = $dataClasses[this._classId].meta;
	if(!meta || !meta.cgmzssrestriction) return false;
	for(const id of meta.cgmzssrestriction.split(",")) {
		if(Number(id) === skillId) return true;
	}
	return false;
};