/*:
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/toastmanager/
 * @target MZ
 * @base CGMZ_Core
 * @orderAfter CGMZ_Core
 * @plugindesc Manages CGMZ toast (or pop up) windows for your game
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
 * Made for RPG Maker MZ 1.9.0
 * ----------------------------------------------------------------------------
 * Description: This plugin creates up to 3 toast windows in each scene (some
 * exceptions) which will display brief information or images to the player
 * for a short amount of time. Windows will be shown as they are freed in the
 * case of a queue of more than 3 toasts.
 * ----------------------------------------------------------------------------
 * Documentation:
 * ----------------------------Plugin Commands---------------------------------
 * This plugin supports the following plugin commands:
 * • Create Text Toast
 * Creates a new text toast with options, adds it to the toast queue.
 * 
 * • Create Image Toast
 * Creates a new image toast with options, adds it to the toast queue.
 * Image dimensions should be 336x72 by default (if changing width,
 * subtract 24 from width value).
 * 
 * • Create Paragraph Toast
 * Creates a new paragraph toast with options, adds it to the toast queue.
 *
 * • Create Toast Preset
 * Creates a new toast from the given preset id.
 * ------------------------------Saved Games-----------------------------------
 * This plugin is fully compatible with saved games. This means you can:
 *
 * ✓ Add this plugin to a saved game and it will work as expected
 * ✓ Change any plugin params and changes will be reflected in saved games
 * ✓ Remove the plugin with no issue to save data
 * -----------------------------Filename---------------------------------------
 * The filename for this plugin MUST remain CGMZ_ToastManager.js
 * This is what it comes as when downloaded. The filename is used to load
 * parameters and execute plugin commands. If you change it, things will begin
 * behaving incorrectly and your game will probably crash. Please do not
 * rename the js file.
 * -----------------------------Latest Version---------------------------------
 * Hi all, this latest version adds the paragraph toast type. You can either
 * create one-off paragraph toasts as needed, or set up paragraph presets
 * that can then be called by using the preset id. This toast consists of a
 * header line and then a paragraph, and it will automatically resize the
 * height to fit the paragraph.
 * 
 * Version 1.6.0
 * - Added paragraph toast type
 *
 * @command Create Toast Preset
 * @desc Creates a toast based on a preset id.
 *
 * @arg Id
 * @desc The preset id to make the toast from.
 *
 * @command Create Text Toast
 * @desc Creates a text-based toast window
 *
 * @arg lineOne
 * @text Line 1
 * @desc Text to display in Line 1 of the toast.
 *
 * @arg lineOneAlignment
 * @type combo
 * @text Line 1 Alignment
 * @option left
 * @option center
 * @option right
 * @desc Align the text in line 1 left, center, or right
 * @default center
 *
 * @arg lineOneColor
 * @type color
 * @text Line One Color
 * @desc The color of line 1's text (same as in show text event command)
 * @default 0
 *
 * @arg lineTwo
 * @text Line 2
 * @desc Text to display in Line 2 of the toast.
 *
 * @arg lineTwoAlignment
 * @type combo
 * @text Line 2 Alignment
 * @option left
 * @option center
 * @option right
 * @desc Align the text in line 2 left, center, or right
 * @default center
 *
 * @arg lineTwoColor
 * @type color
 * @text Line Two Color
 * @desc The color of line 2's text (same as in show text event command)
 * @default 0
 *
 * @arg height
 * @type number
 * @min 1
 * @desc Height (in text lines) to make the toast window if not using fixed height parameter
 * @default 2
 *
 * @arg width
 * @type number
 * @min 1
 * @text Width
 * @desc Width (in pixels) to make the toast window if not using fixed width parameter
 * @default 360
 *
 * @arg SE
 * @type struct<SoundEffect>
 * @default {"File":"","Volume":"90","Pitch":"100","Pan":"0"}
 * @text Sound Effect
 * @desc Sound effect to play when the toast appears
 *
 * @arg Display Time
 * @type number
 * @min 0
 * @desc Display time to show the toast (in frames). If 0, falls back to global Display Time param
 * @default 0
 *
 * @arg backgroundStyle
 * @type select
 * @option Window
 * @option Dim
 * @option Transparent
 * @text Background Style
 * @desc Window background style. Same as Show Text event command.
 * @default Window
 *
 * @arg windowskinTone
 * @type struct<Tone>
 * @text Windowskin Tone
 * @desc Windowskin tone (color)
 *
 * @arg windowskin
 * @type file
 * @dir img/
 * @desc The windowskin file to use for the toast window.
 *
 * @command Create Image Toast
 * @desc Creates an image-based toast window
 *
 * @arg image
 * @type file
 * @dir img/pictures
 * @text Image
 * @desc Image to display in the toast
 *
 * @arg showBackground
 * @type boolean
 * @text Show Background
 * @desc Whether to show the window background or not
 * @default false
 *
 * @arg width
 * @type number
 * @min 1
 * @text Width
 * @desc Width (in pixels) to make the toast window if not using fixed width plugin parameter
 * @default 360
 *
 * @arg height
 * @type number
 * @min 1
 * @desc Height (in text lines) to make the toast window if not using fixed height parameter
 * @default 2
 *
 * @arg SE
 * @type struct<SoundEffect>
 * @default {"Name":"","Volume":"90","Pitch":"100","Pan":"0"}
 * @text Sound Effect
 * @desc Sound effect to play when the toast appears
 *
 * @arg Display Time
 * @type number
 * @min 0
 * @desc Display time to show the toast (in frames). If 0, falls back to global Display Time param
 * @default 0
 *
 * @arg windowskinTone
 * @type struct<Tone>
 * @text Windowskin Tone
 * @desc Windowskin tone (color)
 *
 * @arg windowskin
 * @type file
 * @dir img/
 * @desc The windowskin file to use for the toast window.
 *
 * @command Create Paragraph Toast
 * @desc Creates a text-based toast window that shows a variable height paragraph
 *
 * @arg Header Line
 * @desc The first line shown in the toast, aligned center
 *
 * @arg Header Dividers
 * @type boolean
 * @default true
 * @desc Draw paragraph divider gradient lines?
 *
 * @arg Header Divider Color 1
 * @type color
 * @default 1
 * @desc Color 1 of the header line gradient
 *
 * @arg Header Divider Color 2
 * @type color
 * @default 0
 * @desc Color 2 of the header line gradient
 *
 * @arg Paragraph
 * @type multiline_string
 * @desc The paragraph to show
 *
 * @arg Paragraph Alignment
 * @type select
 * @option left
 * @option center
 * @option right
 * @default center
 * @desc The paragraph alignment
 *
 * @arg width
 * @type number
 * @min 1
 * @text Width
 * @desc Width (in pixels) to make the toast window if not using fixed width parameter
 * @default 360
 *
 * @arg SE
 * @type struct<SoundEffect>
 * @default {"File":"","Volume":"90","Pitch":"100","Pan":"0"}
 * @text Sound Effect
 * @desc Sound effect to play when the toast appears
 *
 * @arg Display Time
 * @type number
 * @min 0
 * @desc Display time to show the toast (in frames). If 0, falls back to global Display Time param
 * @default 0
 *
 * @arg backgroundStyle
 * @type select
 * @option Window
 * @option Dim
 * @option Transparent
 * @text Background Style
 * @desc Window background style. Same as Show Text event command.
 * @default Window
 *
 * @arg windowskinTone
 * @type struct<Tone>
 * @text Windowskin Tone
 * @desc Windowskin tone (color)
 *
 * @arg windowskin
 * @type file
 * @dir img/
 * @desc The windowskin file to use for the toast window.
 * 
 * @param Max Window Count
 * @type number
 * @min 1
 * @max 3
 * @desc Determines max amount of toast windows shown at once
 * @default 1
 *
 * @param Spacing
 * @type number
 * @min 0
 * @desc Determines pixels between each toast window if showing multiple.
 * @default 12
 *
 * @param Width
 * @type number
 * @min 0
 * @desc Determines default width (in pixels) of the toast windows
 * @default 360
 *
 * @param Fixed Width
 * @type boolean
 * @desc Determines if toasts should adjust width or not. If true, toasts always use the above width parameter.
 * @default false
 *
 * @param Height
 * @type number
 * @min 1
 * @desc Determines default height (in text lines) of the toast windows
 * @default 2
 *
 * @param Fixed Height
 * @type boolean
 * @desc Determines if toasts should adjust height or not. If true, toasts always use the above height parameter.
 * @default true
 *
 * @param Display Time
 * @type number
 * @min 0
 * @desc Length of time toast is displayed for (in frames)
 * @default 240
 *
 * @param Display From Bottom
 * @type boolean
 * @desc Determines if toasts should display from the bottom of the screen up, or from top down.
 * @default true
 *
 * @param Display Alignment
 * @type select
 * @option left
 * @option center
 * @option right
 * @desc Should toasts appear on the left, center, or right of the screen?
 * @default center
 *
 * @param X Offset
 * @type number
 * @min -9999
 * @desc The X offset of the window, larger number = further right on screen.
 * @default 0
 *
 * @param Can Display Switch
 * @type switch
 * @desc If set, this switch must be ON before toasts will be able to display (they can still be created and queued up).
 * @default 0
 *
 * @param Can Create Switch
 * @type switch
 * @desc If set, this switch must be ON before toasts will be able to get created.
 * @default 0
 *
 * @param Presets
 * 
 * @param Toast Presets
 * @parent Presets
 * @type struct<Preset>[]
 * @desc Set up toast presets to use for other plugins here, to use them enter the id in the other plugin.
 * @default []
*/
/*~struct~Tone:
 * @param Red
 * @type number
 * @min -255
 * @max 255
 * @desc Amount of red in the tone
 * @default 0
 *
 * @param Green
 * @type number
 * @min -255
 * @max 255
 * @desc Amount of green in the tone
 * @default 0
 *
 * @param Blue
 * @type number
 * @min -255
 * @max 255
 * @desc Amount of blue in the tone
 * @default 0
*/
/*~struct~SoundEffect:
 * @param Name
 * @type file
 * @dir audio/se
 * @desc The sound effect file to play
 *
 * @param Volume
 * @type number
 * @min 0
 * @max 100
 * @default 90
 * @desc The volume of the sound effect
 *
 * @param Pitch
 * @type number
 * @min 50
 * @max 150
 * @default 100
 * @desc The pitch of the sound effect
 *
 * @param Pan
 * @type number
 * @min -100
 * @max 100
 * @default 0
 * @desc The pitch of the sound effect
*/
/*~struct~Preset:
 * @param Id
 * @desc The id associated with this preset, used to refer to this preset later.
 *
 * @param Type
 * @type select
 * @option Text
 * @option Paragraph
 * @option Image
 * @desc The type of toast this is. Determines which settings are used below.
 * @default Text
 * 
 * @param Common Properties
 * 
 * @param Note
 * @parent Common Properties
 * @type multiline_string
 * @desc Not used, only to help your notes or possibly as extra data by other plugins.
 *
 * @param Width
 * @parent Common Properties
 * @type number
 * @default 0
 * @desc The width of the toast in pixels. Set to 0 to use default width.
 *
 * @param Height
 * @parent Common Properties
 * @type number
 * @default 0
 * @desc The height of the toast in line count. Set to 0 to use default height.
 *
 * @param Windowskin Tone
 * @parent Common Properties
 * @type struct<Tone>
 * @desc Windowskin tone (color). Leave blank to use default.
 *
 * @param Windowskin
 * @parent Common Properties
 * @type file
 * @dir img/
 * @desc The windowskin file to use for the toast window. Leave blank to use default.
 *
 * @param Sound Effect
 * @parent Common Properties
 * @type struct<SoundEffect>
 * @default {"Name":"","Volume":"90","Pitch":"100","Pan":"0"}
 * @desc Sound effect to play when the toast appears
 *
 * @param Display Time
 * @parent Common Properties
 * @type number
 * @min 0
 * @desc Display time to show the toast (in frames). If 0, falls back to global Display Time param
 * @default 0
 *
 * @param Text Properties
 *
 * @param Text Line 1
 * @parent Text Properties
 * @desc The first text line to show in the toast.
 *
 * @param Text Line 1 Color
 * @parent Text Properties
 * @type color
 * @default 0
 * @desc The first text line's color.
 *
 * @param Text Line 1 Alignment
 * @parent Text Properties
 * @type select
 * @option left
 * @option center
 * @option right
 * @default center
 * @desc The first text line's alignment.
 *
 * @param Text Line 2
 * @parent Text Properties
 * @desc The first text line to show in the toast.
 *
 * @param Text Line 2 Color
 * @parent Text Properties
 * @type color
 * @default 0
 * @desc The first text line's color.
 *
 * @param Text Line 2 Alignment
 * @parent Text Properties
 * @type select
 * @option left
 * @option center
 * @option right
 * @default center
 * @desc The first text line's alignment.
 *
 * @param Background Style
 * @parent Text Properties
 * @type select
 * @option Window
 * @option Dim
 * @option Transparent
 * @desc Window background style. Same as Show Text event command.
 * @default Window
 *
 * @param Image Properties
 *
 * @param Image
 * @parent Image Properties
 * @type file
 * @dir img/pictures
 * @desc Image to display in the toast
 *
 * @param Show Background
 * @parent Image Properties
 * @type boolean
 * @desc Whether to show the window background or not
 * @default false
 *
 * @param Paragraph Properties
 *
 * @param Header Line
 * @parent Paragraph Properties
 * @desc The first line shown in the toast, aligned center
 *
 * @param Header Dividers
 * @parent Paragraph Properties
 * @type boolean
 * @default true
 * @desc Draw paragraph divider gradient lines?
 *
 * @param Header Divider Color 1
 * @parent Paragraph Properties
 * @type color
 * @default 1
 * @desc Color 1 of the header line gradient
 *
 * @param Header Divider Color 2
 * @parent Paragraph Properties
 * @type color
 * @default 0
 * @desc Color 2 of the header line gradient
 *
 * @param Paragraph
 * @parent Paragraph Properties
 * @type multiline_string
 * @desc The paragraph to show
 *
 * @param Paragraph Alignment
 * @parent Paragraph Properties
 * @type select
 * @option left
 * @option center
 * @option right
 * @default center
 * @desc The paragraph alignment
*/
/*:zh-CN
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/toastmanager/
 * @target MZ
 * @base CGMZ_Core
 * @orderAfter CGMZ_Core
 * @plugindesc 信息弹窗系统（配合作者各类插件进行提示）
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
 * 【插件版本】V 1.6.0
 * ----------------------------------------------------------------------------
 * 【兼容性】仅测试作者所制作的插件
 * 【RM版本】RPG Maker MZ 1.9.0
 * ----------------------------------------------------------------------------
 * 【插件描述】
 * 在游戏中可以跳出弹窗，来提示玩家获得成就、专业升级、习得配方等。
 * 玩家需要配合使用作者对应的其他插件来获得相关的提示效果。
 * 可以设置同一时间显示最多3个弹窗、弹窗尺寸、显示时间和新旧弹窗切换效果等。
 * 本插件不是传统意义上的物品得失自动提示插件，但可以用插件指令手动制作文字或图片提示。
 *
 * 【搭配插件】
 * CGMZ Core:核心插件，运行作者插件的必须插件。
 * CGMZ Professions:专业技能插件。可提示专业技能升级等。
 * CGMZ Crafting:手工艺合成插件。可提示获得配方等。
 * CGMZ Achievements:成就系统插件。可提示成就达成等。
 * 以及其他该作者支持弹窗提示功能的插件。
 * 注：本插件在插件列表中必须置于"核心插件Core"之下，并置于其他可提示插件之上。
 * ----------------------------------------------------------------------------
 * 【使用说明】
 * 一、插件指令：
 * 1. 制作文字弹窗（Create Text Toast）
 * 2. 制作图片弹窗（Create Image Toast）
 * 注：图片弹窗尺寸为336×72像素，调整宽度请以24的倍数加减调整。
 *
 * 二、其他插件的弹窗设置
 * 作者其他插件均有不同效果的弹窗设置，具体请在其他插件的参数设置中调整。
 * 
 * • Create Paragraph Toast
 * Creates a new paragraph toast with options, adds it to the toast queue.
 *
 * • Create Toast Preset
 * Creates a new toast from the given preset id.
 * ---------------------------------------------------------------------------
 *【版本更新历史】
 * Hi all, this latest version adds the paragraph toast type. You can either
 * create one-off paragraph toasts as needed, or set up paragraph presets
 * that can then be called by using the preset id. This toast consists of a
 * header line and then a paragraph, and it will automatically resize the
 * height to fit the paragraph.
 * 
 * Version 1.6.0
 * - Added paragraph toast type
 *
 * @command Create Toast Preset
 * @desc Creates a toast based on a preset id.
 *
 * @arg Id
 * @desc The preset id to make the toast from.
 *
 * @command Create Text Toast
 * @text 制作文字弹窗
 * @desc 制作一个只有文字描述的弹窗。
 *
 * @arg lineOne
 * @text 第一行描述
 * @desc 设置弹窗第一行文字的内容。支持变量\v，角色名\n，队友序号\p和货币单位\g的文本指令。
 *
 * @arg lineOneAlignment
 * @type combo
 * @text 第一行文字位置
 * @option left
 * @option center
 * @option right
 * @desc 设置弹窗第一行文字位置。Left-靠左，Center-居中，Right-靠右。
 * @default center
 *
 * @arg lineOneColor
 * @type color
 * @text 第一行文字颜色
 * @desc 设置弹窗第一行文字颜色。数值参照显示文本事件的颜色序号设置。
 * @default 0
 *
 * @arg lineTwo
 * @text 第二行描述
 * @desc 设置弹窗第二行文字的内容。支持变量\v，角色名\n，队友序号\p和货币单位\g的文本指令。
 *
 * @arg lineTwoAlignment
 * @type combo
 * @text 第二行文字位置
 * @option left
 * @option center
 * @option right
 * @desc 设置弹窗第二行文字位置。Left-靠左，Center-居中，Right-靠右。
 * @default center
 *
 * @arg lineTwoColor
 * @type color
 * @text 第二行文字颜色
 * @desc 设置弹窗第二行文字颜色。数值参照显示文本事件的颜色序号设置。
 * @default 0
 *
 * @arg width
 * @type number
 * @min 1
 * @text 宽度
 * @desc 设置提示弹窗的宽度，默认为360像素。
 * @default 360
 *
 * @arg height
 * @type number
 * @min 1
 * @desc Height (in text lines) to make the toast window if not using fixed height parameter
 * @default 2
 *
 * @arg SE
 * @type struct<SoundEffect>
 * @default {"Name":"","Volume":"90","Pitch":"100","Pan":"0"}
 * @text 音效
 * @desc 设置一个弹窗提示时的音效。
 *
 * @arg Display Time
 * @type number
 * @min 0
 * @desc Display time to show the toast (in frames). If 0, falls back to global Display Time param
 * @default 0
 *
 * @arg backgroundStyle
 * @type select
 * @option Window
 * @option Dim
 * @option Transparent
 * @text Background Style
 * @desc Window background style. Same as Show Text event command.
 * @default Window
 *
 * @arg windowskinTone
 * @type struct<Tone>
 * @text Windowskin Tone
 * @desc Windowskin tone (color)
 *
 * @arg windowskin
 * @text windowskin
 * @type file
 * @dir img/
 * @desc The windowskin file to use for the toast window.
 *
 * @command Create Image Toast
 * @text 制作图片弹窗
 * @desc 制作一个只有图片的弹窗。（默认尺寸建议：336×72像素）
 *
 * @arg image
 * @type file
 * @dir img/pictures
 * @text 图片
 * @desc 选择一张作为提示用的图片。（默认尺寸建议：336×72像素
 *
 * @arg showBackground
 * @type boolean
 * @text 显示边框
 * @desc 显示图片时是否显示默认UI的边框？
 * @default false
 *
 * @arg width
 * @type number
 * @min 1
 * @text 宽度
 * @desc 设置提示弹窗的宽度，默认为360像素。
 * @default 360
 *
 * @arg height
 * @type number
 * @min 1
 * @desc Height (in text lines) to make the toast window if not using fixed height parameter
 * @default 2
 *
 * @arg SE
 * @type struct<SoundEffect>
 * @default {"Name":"","Volume":"90","Pitch":"100","Pan":"0"}
 * @text 音效
 * @desc 选择一个弹窗提示时的音效。
 *
 * @arg Display Time
 * @type number
 * @min 0
 * @desc Display time to show the toast (in frames). If 0, falls back to global Display Time param
 * @default 0
 *
 * @arg windowskinTone
 * @type struct<Tone>
 * @text Windowskin Tone
 * @desc Windowskin tone (color)
 *
 * @arg windowskin
 * @text windowskin
 * @type file
 * @dir img/
 * @desc The windowskin file to use for the toast window.
 *
 * @command Create Paragraph Toast
 * @desc Creates a text-based toast window that shows a variable height paragraph
 *
 * @arg Header Line
 * @desc The first line shown in the toast, aligned center
 *
 * @arg Header Dividers
 * @type boolean
 * @default true
 * @desc Draw paragraph divider gradient lines?
 *
 * @arg Header Divider Color 1
 * @type color
 * @default 1
 * @desc Color 1 of the header line gradient
 *
 * @arg Header Divider Color 2
 * @type color
 * @default 0
 * @desc Color 2 of the header line gradient
 *
 * @arg Paragraph
 * @type multiline_string
 * @desc The paragraph to show
 *
 * @arg Paragraph Alignment
 * @type select
 * @option left
 * @option center
 * @option right
 * @default center
 * @desc The paragraph alignment
 *
 * @arg width
 * @type number
 * @min 1
 * @text Width
 * @desc Width (in pixels) to make the toast window if not using fixed width parameter
 * @default 360
 *
 * @arg SE
 * @type struct<SoundEffect>
 * @default {"File":"","Volume":"90","Pitch":"100","Pan":"0"}
 * @text Sound Effect
 * @desc Sound effect to play when the toast appears
 *
 * @arg Display Time
 * @type number
 * @min 0
 * @desc Display time to show the toast (in frames). If 0, falls back to global Display Time param
 * @default 0
 *
 * @arg backgroundStyle
 * @type select
 * @option Window
 * @option Dim
 * @option Transparent
 * @text Background Style
 * @desc Window background style. Same as Show Text event command.
 * @default Window
 *
 * @arg windowskinTone
 * @type struct<Tone>
 * @text Windowskin Tone
 * @desc Windowskin tone (color)
 *
 * @arg windowskin
 * @type file
 * @dir img/
 * @desc The windowskin file to use for the toast window.
 * 
 * @param Max Window Count
 * @type number
 * @min 1
 * @max 3
 * @text 弹窗数
 * @desc 设置同一时间显示的最大弹窗数。（数量1~3）
 * @default 1
 *
 * @param Spacing
 * @type number
 * @min 0
 * @text 弹窗间隔
 * @desc 同时显示多个弹窗时，设置弹窗之间的间隔。
 * @default 12
 *
 * @param Width
 * @type number
 * @min 0
 * @text 弹窗宽度
 * @desc 设置提示弹窗的宽度。
 * @default 360
 *
 * @param Fixed Width
 * @type boolean
 * @text 统一宽度
 * @desc 是否统一调整弹窗宽度？Ture-统一使用"弹窗宽度"设置的宽度。
 * @default false
 *
 * @param Height
 * @type number
 * @min 1
 * @desc Determines default height (in text lines) of the toast windows
 * @default 2
 *
 * @param Fixed Height
 * @type boolean
 * @desc Determines if toasts should adjust height or not. If true, toasts always use the above height parameter.
 * @default true
 *
 * @param Display Time
 * @type number
 * @min 0
 * @text 显示时间
 * @desc 弹窗在画面上显示的时长。（单位:帧，60帧=1秒）
 * @default 240
 *
 * @param Display From Bottom
 * @type boolean
 * @text 弹窗位置
 * @desc Ture-底部显示，False-顶部显示。
 * @default true
 *
 * @param Display Alignment
 * @type select
 * @option left
 * @option center
 * @option right
 * @desc Should toasts appear on the left, center, or right of the screen?
 * @default center
 *
 * @param X Offset
 * @type number
 * @min -9999
 * @desc The X offset of the window, larger number = further right on screen.
 * @default 0
 *
 * @param Can Display Switch
 * @type switch
 * @desc If set, this switch must be ON before toasts will be able to display (they can still be created and queued up).
 * @default 0
 *
 * @param Can Create Switch
 * @type switch
 * @desc If set, this switch must be ON before toasts will be able to get created.
 * @default 0
 *
 * @param Presets
 * 
 * @param Toast Presets
 * @parent Presets
 * @type struct<Preset>[]
 * @desc Set up toast presets to use for other plugins here, to use them enter the id in the other plugin.
 * @default []
*/
/*~struct~Tone:zh-CN
 * @param Red
 * @type number
 * @min -255
 * @max 255
 * @desc Amount of red in the tone
 * @default 0
 *
 * @param Green
 * @type number
 * @min -255
 * @max 255
 * @desc Amount of green in the tone
 * @default 0
 *
 * @param Blue
 * @type number
 * @min -255
 * @max 255
 * @desc Amount of blue in the tone
 * @default 0
*/
/*~struct~SoundEffect:zh-CN
 * @param Name
 * @type file
 * @dir audio/se
 * @desc The sound effect file to play
 *
 * @param Volume
 * @type number
 * @min 0
 * @max 100
 * @default 90
 * @desc The volume of the sound effect
 *
 * @param Pitch
 * @type number
 * @min 50
 * @max 150
 * @default 100
 * @desc The pitch of the sound effect
 *
 * @param Pan
 * @type number
 * @min -100
 * @max 100
 * @default 0
 * @desc The pitch of the sound effect
*/
/*~struct~Preset:zh-CN
 * @param Id
 * @desc The id associated with this preset, used to refer to this preset later.
 *
 * @param Type
 * @type select
 * @option Text
 * @option Paragraph
 * @option Image
 * @desc The type of toast this is. Determines which settings are used below.
 * @default Text
 * 
 * @param Common Properties
 * 
 * @param Note
 * @parent Common Properties
 * @type multiline_string
 * @desc Not used, only to help your notes or possibly as extra data by other plugins.
 *
 * @param Width
 * @parent Common Properties
 * @type number
 * @default 0
 * @desc The width of the toast in pixels. Set to 0 to use default width.
 *
 * @param Height
 * @parent Common Properties
 * @type number
 * @default 0
 * @desc The height of the toast in line count. Set to 0 to use default height.
 *
 * @param Windowskin Tone
 * @parent Common Properties
 * @type struct<Tone>
 * @desc Windowskin tone (color). Leave blank to use default.
 *
 * @param Windowskin
 * @parent Common Properties
 * @type file
 * @dir img/
 * @desc The windowskin file to use for the toast window. Leave blank to use default.
 *
 * @param Sound Effect
 * @parent Common Properties
 * @type struct<SoundEffect>
 * @default {"Name":"","Volume":"90","Pitch":"100","Pan":"0"}
 * @desc Sound effect to play when the toast appears
 *
 * @param Display Time
 * @parent Common Properties
 * @type number
 * @min 0
 * @desc Display time to show the toast (in frames). If 0, falls back to global Display Time param
 * @default 0
 *
 * @param Text Properties
 *
 * @param Text Line 1
 * @parent Text Properties
 * @desc The first text line to show in the toast.
 *
 * @param Text Line 1 Color
 * @parent Text Properties
 * @type color
 * @default 0
 * @desc The first text line's color.
 *
 * @param Text Line 1 Alignment
 * @parent Text Properties
 * @type select
 * @option left
 * @option center
 * @option right
 * @default center
 * @desc The first text line's alignment.
 *
 * @param Text Line 2
 * @parent Text Properties
 * @desc The first text line to show in the toast.
 *
 * @param Text Line 2 Color
 * @parent Text Properties
 * @type color
 * @default 0
 * @desc The first text line's color.
 *
 * @param Text Line 2 Alignment
 * @parent Text Properties
 * @type select
 * @option left
 * @option center
 * @option right
 * @default center
 * @desc The first text line's alignment.
 *
 * @param Background Style
 * @parent Text Properties
 * @type select
 * @option Window
 * @option Dim
 * @option Transparent
 * @desc Window background style. Same as Show Text event command.
 * @default Window
 *
 * @param Image Properties
 *
 * @param Image
 * @parent Image Properties
 * @type file
 * @dir img/pictures
 * @desc Image to display in the toast
 *
 * @param Show Background
 * @parent Image Properties
 * @type boolean
 * @desc Whether to show the window background or not
 * @default false
 *
 * @param Paragraph Properties
 *
 * @param Header Line
 * @parent Paragraph Properties
 * @desc The first line shown in the toast, aligned center
 *
 * @param Header Dividers
 * @parent Paragraph Properties
 * @type boolean
 * @default true
 * @desc Draw paragraph divider gradient lines?
 *
 * @param Header Divider Color 1
 * @parent Paragraph Properties
 * @type color
 * @default 1
 * @desc Color 1 of the header line gradient
 *
 * @param Header Divider Color 2
 * @parent Paragraph Properties
 * @type color
 * @default 0
 * @desc Color 2 of the header line gradient
 *
 * @param Paragraph
 * @parent Paragraph Properties
 * @type multiline_string
 * @desc The paragraph to show
 *
 * @param Paragraph Alignment
 * @parent Paragraph Properties
 * @type select
 * @option left
 * @option center
 * @option right
 * @default center
 * @desc The paragraph alignment
*/
/*:es
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/toastmanager/
 * @target MZ
 * @base CGMZ_Core
 * @orderAfter CGMZ_Core
 * @plugindesc Administra las ventanas emergentes de CGMZ para tu juego
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
 * Hecho para RPG Maker MZ 1.9.0
 * ----------------------------------------------------------------------------
 * Descripción: Este plugin crea hasta 3 ventanas emergentes en cada escena
 * (hay algunas excepciones) que mostrará información breve o imágenes al
 * jugador por un corto período de tiempo. Las ventanas se mostrarán a medida
 * que se liberan en el caso de una cola de mas de 3 mensajes/avisos. 
 * ----------------------------------------------------------------------------
 * Documentación:
 * ---------------------------Comandos de Plugin-------------------------------
 * • Crear notificación de texto
 * Crea una nueva notificación de texto con opciones, que se agrega a la cola
 * 
 * • Crear notificación de imagen
 * Crea una nueva notificación de imagen con opciones, que se agrega a la cola
 *
 * Las dimensiones de la imagen deben ser 336x72 por defecto (si cambia el 
 * ancho, restar 24 del valor del ancho).
 * 
 * • Create Paragraph Toast
 * Creates a new paragraph toast with options, adds it to the toast queue.
 *
 * • Create Toast Preset
 * Creates a new toast from the given preset id.
 * ------------------------------Saved Games-----------------------------------
 * This plugin is fully compatible with saved games. This means you can:
 *
 * ✓ Add this plugin to a saved game and it will work as expected
 * ✓ Change any plugin params and changes will be reflected in saved games
 * ✓ Remove the plugin with no issue to save data
 * -----------------------------Filename---------------------------------------
 * The filename for this plugin MUST remain CGMZ_ToastManager.js
 * This is what it comes as when downloaded. The filename is used to load
 * parameters and execute plugin commands. If you change it, things will begin
 * behaving incorrectly and your game will probably crash. Please do not
 * rename the js file.
 * -----------------------------Latest Version---------------------------------
 * Hi all, this latest version adds the paragraph toast type. You can either
 * create one-off paragraph toasts as needed, or set up paragraph presets
 * that can then be called by using the preset id. This toast consists of a
 * header line and then a paragraph, and it will automatically resize the
 * height to fit the paragraph.
 * 
 * Version 1.6.0
 * - Added paragraph toast type
 *
 * @command Create Toast Preset
 * @desc Creates a toast based on a preset id.
 *
 * @arg Id
 * @desc The preset id to make the toast from.
 *
 * @command Create Text Toast
 * @text Crear Mensaje de respuesta
 * @desc Crea una ventana de mensaje basada en texto
 *
 * @arg lineOne
 * @text Línea 1
 * @desc Texto para mostrar en la Línea 1 del mensaje.
 *
 * @arg lineOneAlignment
 * @type combo
 * @text Alineación de la línea 1
 * @option left
 * @option center
 * @option right
 * @desc Alinear el texto en la línea 1 a la izquierda, al centro o a la derecha.
 * @default center
 *
 * @arg lineOneColor
 * @type color
 * @text Color del texto de la primera línea
 * @desc El color del texto de la línea 1 (igual que en el comando Mostrar evento de texto)
 * @default 0
 *
 * @arg lineTwo
 * @text Línea 2
 * @desc Texto para mostrar en la Línea 2 del mensaje.
 *
 * @arg lineTwoAlignment
 * @type combo
 * @text Alineación de la línea 2
 * @option left
 * @option center
 * @option right
 * @desc Alinear el texto en la línea 2 a la izquierda, al centro o a la derecha.
 * @default center
 *
 * @arg lineTwoColor
 * @type color
 * @text Color del texto de la segunda línea
 * @desc El color del texto de la línea 2 (igual que en el comando Mostrar evento de texto).
 * @default 0
 *
 * @arg width
 * @type number
 * @min 1
 * @text Ancho
 * @desc Ancho (en píxeles) para hacer la ventana del mensaje si no se usa el parámetro de plugin de ancho fijo.
 * @default 360
 *
 * @arg height
 * @type number
 * @min 1
 * @desc Height (in text lines) to make the toast window if not using fixed height parameter
 * @default 2
 *
 * @arg SE
 * @type struct<SoundEffect>
 * @default {"Name":"","Volume":"90","Pitch":"100","Pan":"0"}
 * @text Efecto de sonido
 * @desc Efecto de sonido para reproducir cuando aparece el mensaje.
 *
 * @arg Display Time
 * @type number
 * @min 0
 * @text Tiempo de visualización
 * @desc Tiempo de visualización para mostrar el mensaje (en fotogramas). Si es 0, vuelve al parámetro de tiempo de visualización global.
 * @default 0
 *
 * @arg backgroundStyle
 * @type select
 * @option Window
 * @option Dim
 * @option Transparent
 * @text Estilo de fondo
 * @desc Estilo de fondo de ventana. Igual que el comando de evento Mostrar texto.
 * @default Window
 *
 * @arg windowskinTone
 * @type struct<Tone>
 * @text Tono de piel de ventana
 * @desc Tono de piel de ventana (color).
 *
 * @arg windowskin
 * @type file
 * @dir img/
 * @desc The windowskin file to use for the toast window.
 *
 * @command Create Image Toast
 * @text Crear mensaje de imagen
 * @desc Crea una ventana de mensaje basada en imágenes.
 *
 * @arg image
 * @type file
 * @dir img/pictures
 * @text Imagen
 * @desc Imagen para mostrar en el mensaje.
 *
 * @arg showBackground
 * @type boolean
 * @text Mostrar fondo
 * @desc Mostrar o no el fondo de la ventana.
 * @default false
 *
 * @arg width
 * @type number
 * @min 1
 * @text Ancho
 * @desc Ancho (en píxeles) para hacer la ventana del mensaje si no se usa el parámetro de plugin de ancho fijo.
 * @default 360
 *
 * @arg height
 * @type number
 * @min 1
 * @desc Height (in text lines) to make the toast window if not using fixed height parameter
 * @default 2
 *
 * @arg SE
 * @type struct<SoundEffect>
 * @default {"Name":"","Volume":"90","Pitch":"100","Pan":"0"}
 * @text Efecto de sonido
 * @desc Efecto de sonido para reproducir cuando aparece el mensaje.
 *
 * @arg Display Time
 * @type number
 * @min 0
 * @text Tiempo de visualización
 * @desc Tiempo de visualización para mostrar el mensaje (en fotogramas). Si es 0, vuelve al parámetro de tiempo de visualización global.
 * @default 0
 *
 * @arg windowskinTone
 * @type struct<Tone>
 * @text Tono de piel de ventana
 * @desc Tono de piel de ventana (color).
 *
 * @arg windowskin
 * @type file
 * @dir img/
 * @desc The windowskin file to use for the toast window.
 *
 * @command Create Paragraph Toast
 * @desc Creates a text-based toast window that shows a variable height paragraph
 *
 * @arg Header Line
 * @desc The first line shown in the toast, aligned center
 *
 * @arg Header Dividers
 * @type boolean
 * @default true
 * @desc Draw paragraph divider gradient lines?
 *
 * @arg Header Divider Color 1
 * @type color
 * @default 1
 * @desc Color 1 of the header line gradient
 *
 * @arg Header Divider Color 2
 * @type color
 * @default 0
 * @desc Color 2 of the header line gradient
 *
 * @arg Paragraph
 * @type multiline_string
 * @desc The paragraph to show
 *
 * @arg Paragraph Alignment
 * @type select
 * @option left
 * @option center
 * @option right
 * @default center
 * @desc The paragraph alignment
 *
 * @arg width
 * @type number
 * @min 1
 * @text Width
 * @desc Width (in pixels) to make the toast window if not using fixed width parameter
 * @default 360
 *
 * @arg SE
 * @type struct<SoundEffect>
 * @default {"File":"","Volume":"90","Pitch":"100","Pan":"0"}
 * @text Sound Effect
 * @desc Sound effect to play when the toast appears
 *
 * @arg Display Time
 * @type number
 * @min 0
 * @desc Display time to show the toast (in frames). If 0, falls back to global Display Time param
 * @default 0
 *
 * @arg backgroundStyle
 * @type select
 * @option Window
 * @option Dim
 * @option Transparent
 * @text Background Style
 * @desc Window background style. Same as Show Text event command.
 * @default Window
 *
 * @arg windowskinTone
 * @type struct<Tone>
 * @text Windowskin Tone
 * @desc Windowskin tone (color)
 *
 * @arg windowskin
 * @type file
 * @dir img/
 * @desc The windowskin file to use for the toast window.
 * 
 * @param Max Window Count
 * @type number
 * @min 1
 * @max 3
 * @text Recuento máximo de ventanas
 * @desc Determina la cantidad máxima de ventanas de mensaje que se muestran a la vez.
 * @default 1
 *
 * @param Spacing
 * @type number
 * @min 0
 * @text Espaciado
 * @desc Determina los píxeles entre cada ventana de mensaje si se muestran varias.
 * @default 12
 *
 * @param Width
 * @type number
 * @min 0
 * @text Ancho
 * @desc Determina el ancho predeterminado (en píxeles) de las ventanas de mensaje.
 * @default 360
 *
 * @param Fixed Width
 * @type boolean
 * @text Ancho fijo
 * @desc Determina si los mensajes deben ajustar el ancho o no. Si es verdadero, los mensajes siempre usan el parámetro de ancho anterior.
 * @default false
 *
 * @param Height
 * @type number
 * @min 1
 * @desc Determines default height (in text lines) of the toast windows
 * @default 2
 *
 * @param Fixed Height
 * @type boolean
 * @desc Determines if toasts should adjust height or not. If true, toasts always use the above height parameter.
 * @default true
 *
 * @param Display Time
 * @type number
 * @min 0
 * @text Tiempo de visualización
 * @desc Duración del tiempo que se muestra el mensaje (en fotogramas)
 * @default 240
 *
 * @param Display From Bottom
 * @type boolean
 * @text Mostrar desde abajo
 * @desc Determina si los mensajes deben mostrarse desde la parte inferior de la pantalla hacia arriba o desde arriba hacia abajo.
 * @default true
 *
 * @param Display Alignment
 * @type select
 * @option left
 * @option center
 * @option right
 * @desc Should toasts appear on the left, center, or right of the screen?
 * @default center
 *
 * @param X Offset
 * @type number
 * @min -9999
 * @desc The X offset of the window, larger number = further right on screen.
 * @default 0
 *
 * @param Can Display Switch
 * @type switch
 * @desc If set, this switch must be ON before toasts will be able to display (they can still be created and queued up).
 * @default 0
 *
 * @param Can Create Switch
 * @type switch
 * @desc If set, this switch must be ON before toasts will be able to get created.
 * @default 0
 *
 * @param Presets
 * 
 * @param Toast Presets
 * @parent Presets
 * @type struct<Preset>[]
 * @desc Set up toast presets to use for other plugins here, to use them enter the id in the other plugin.
 * @default []
*/
/*~struct~Tone:es
 * @param Red
 * @text Rojo
 * @type number
 * @min -255
 * @max 255
 * @desc Cantidad de rojo en el tono.
 * @default 0
 *
 * @param Green
 * @text Verde
 * @type number
 * @min -255
 * @max 255
 * @desc Cantidad de verde en el tono.
 * @default 0
 *
 * @param Blue
 * @text Azul
 * @type number
 * @min -255
 * @max 255
 * @desc Cantidad de azul en el tono.
 * @default 0
*/
/*~struct~SoundEffect:es
 * @param Name
 * @type file
 * @dir audio/se
 * @desc The sound effect file to play
 *
 * @param Volume
 * @type number
 * @min 0
 * @max 100
 * @default 90
 * @desc The volume of the sound effect
 *
 * @param Pitch
 * @type number
 * @min 50
 * @max 150
 * @default 100
 * @desc The pitch of the sound effect
 *
 * @param Pan
 * @type number
 * @min -100
 * @max 100
 * @default 0
 * @desc The pitch of the sound effect
*/
/*~struct~Preset:es
 * @param Id
 * @desc The id associated with this preset, used to refer to this preset later.
 *
 * @param Type
 * @type select
 * @option Text
 * @option Paragraph
 * @option Image
 * @desc The type of toast this is. Determines which settings are used below.
 * @default Text
 * 
 * @param Common Properties
 * 
 * @param Note
 * @parent Common Properties
 * @type multiline_string
 * @desc Not used, only to help your notes or possibly as extra data by other plugins.
 *
 * @param Width
 * @parent Common Properties
 * @type number
 * @default 0
 * @desc The width of the toast in pixels. Set to 0 to use default width.
 *
 * @param Height
 * @parent Common Properties
 * @type number
 * @default 0
 * @desc The height of the toast in line count. Set to 0 to use default height.
 *
 * @param Windowskin Tone
 * @parent Common Properties
 * @type struct<Tone>
 * @desc Windowskin tone (color). Leave blank to use default.
 *
 * @param Windowskin
 * @parent Common Properties
 * @type file
 * @dir img/
 * @desc The windowskin file to use for the toast window. Leave blank to use default.
 *
 * @param Sound Effect
 * @parent Common Properties
 * @type struct<SoundEffect>
 * @default {"Name":"","Volume":"90","Pitch":"100","Pan":"0"}
 * @desc Sound effect to play when the toast appears
 *
 * @param Display Time
 * @parent Common Properties
 * @type number
 * @min 0
 * @desc Display time to show the toast (in frames). If 0, falls back to global Display Time param
 * @default 0
 *
 * @param Text Properties
 *
 * @param Text Line 1
 * @parent Text Properties
 * @desc The first text line to show in the toast.
 *
 * @param Text Line 1 Color
 * @parent Text Properties
 * @type color
 * @default 0
 * @desc The first text line's color.
 *
 * @param Text Line 1 Alignment
 * @parent Text Properties
 * @type select
 * @option left
 * @option center
 * @option right
 * @default center
 * @desc The first text line's alignment.
 *
 * @param Text Line 2
 * @parent Text Properties
 * @desc The first text line to show in the toast.
 *
 * @param Text Line 2 Color
 * @parent Text Properties
 * @type color
 * @default 0
 * @desc The first text line's color.
 *
 * @param Text Line 2 Alignment
 * @parent Text Properties
 * @type select
 * @option left
 * @option center
 * @option right
 * @default center
 * @desc The first text line's alignment.
 *
 * @param Background Style
 * @parent Text Properties
 * @type select
 * @option Window
 * @option Dim
 * @option Transparent
 * @desc Window background style. Same as Show Text event command.
 * @default Window
 *
 * @param Image Properties
 *
 * @param Image
 * @parent Image Properties
 * @type file
 * @dir img/pictures
 * @desc Image to display in the toast
 *
 * @param Show Background
 * @parent Image Properties
 * @type boolean
 * @desc Whether to show the window background or not
 * @default false
 *
 * @param Paragraph Properties
 *
 * @param Header Line
 * @parent Paragraph Properties
 * @desc The first line shown in the toast, aligned center
 *
 * @param Header Dividers
 * @parent Paragraph Properties
 * @type boolean
 * @default true
 * @desc Draw paragraph divider gradient lines?
 *
 * @param Header Divider Color 1
 * @parent Paragraph Properties
 * @type color
 * @default 1
 * @desc Color 1 of the header line gradient
 *
 * @param Header Divider Color 2
 * @parent Paragraph Properties
 * @type color
 * @default 0
 * @desc Color 2 of the header line gradient
 *
 * @param Paragraph
 * @parent Paragraph Properties
 * @type multiline_string
 * @desc The paragraph to show
 *
 * @param Paragraph Alignment
 * @parent Paragraph Properties
 * @type select
 * @option left
 * @option center
 * @option right
 * @default center
 * @desc The paragraph alignment
*/
Imported.CGMZ_ToastManager = true;
CGMZ.Versions["Toast Manager"] = "1.6.0";
CGMZ.ToastManager = {};
CGMZ.ToastManager.parameters = PluginManager.parameters('CGMZ_ToastManager');
CGMZ.ToastManager.DisplayAlignment = CGMZ.ToastManager.parameters["Display Alignment"];
CGMZ.ToastManager.MaxWindowCount = Number(CGMZ.ToastManager.parameters["Max Window Count"]);
CGMZ.ToastManager.Spacing = Number(CGMZ.ToastManager.parameters["Spacing"]);
CGMZ.ToastManager.Width = Number(CGMZ.ToastManager.parameters["Width"]);
CGMZ.ToastManager.Height = Number(CGMZ.ToastManager.parameters["Height"]);
CGMZ.ToastManager.DisplayTime = Number(CGMZ.ToastManager.parameters["Display Time"]);
CGMZ.ToastManager.XOffset = Number(CGMZ.ToastManager.parameters["X Offset"]);
CGMZ.ToastManager.CanDisplaySwitch = Number(CGMZ.ToastManager.parameters["Can Display Switch"]);
CGMZ.ToastManager.CanCreateSwitch = Number(CGMZ.ToastManager.parameters["Can Create Switch"]);
CGMZ.ToastManager.FixedWidth = (CGMZ.ToastManager.parameters["Fixed Width"] === "true");
CGMZ.ToastManager.FixedHeight = (CGMZ.ToastManager.parameters["Fixed Height"] === "true");
CGMZ.ToastManager.DisplayFromBottom = (CGMZ.ToastManager.parameters["Display From Bottom"] === "true");
CGMZ.ToastManager.ToastPresets = CGMZ_Utils.parseJSON(CGMZ.ToastManager.parameters["Toast Presets"], [], '[CGMZ] Toast Manager', 'Your Toast Presets parameter was invalid JSON and could not be read. Check that it is not blank.');
//=============================================================================
// CGMZ_ToastPreset
//-----------------------------------------------------------------------------
// Toast Preset object class. Not part of save data.
//=============================================================================
function CGMZ_ToastPreset() {
	this.initialize(...arguments);
}
//-----------------------------------------------------------------------------
// Initialize
//-----------------------------------------------------------------------------
CGMZ_ToastPreset.prototype.initialize = function(data) {
	this.id = data.Id;
	this.backgroundStyle = data["Background Style"];
	this.windowskin = data.Windowskin;
	this.type = data.Type;
	this.img = data.Image;
	this.note = data.Note;
	this.textLine1 = data["Text Line 1"];
	this.textLine2 = data["Text Line 2"];
	this.textLine1Align = data["Text Line 1 Alignment"];
	this.textLine2Align = data["Text Line 2 Alignment"];
	this.tone = data["Windowskin Tone"];
	this.se = data["Sound Effect"];
	this.width = Number(data.Width);
	this.height = Number(data.Height);
	this.displayTime = Number(data["Display Time"]);
	this.textLine1Color = Number(data["Text Line 1 Color"]);
	this.textLine2Color = Number(data["Text Line 2 Color"]);
	this.showBackground = (data["Show Background"] === 'true');
	this.headerLine = data["Header Line"];
	this.headerDividers = (data["Header Dividers"] === 'true');
	this.headerColor1 = Number(data["Header Divider Color 1"]);
	this.headerColor2 = Number(data["Header Divider Color 2"]);
	this.paragraph = data.Paragraph;
	this.paragraphAlignment = data["Paragraph Alignment"];
};
//-----------------------------------------------------------------------------
// Creates and returns the actual toast object
//-----------------------------------------------------------------------------
CGMZ_ToastPreset.prototype.makeToastObject = function() {
	const obj = {};
	switch(this.type) {
		case 'Text': obj.isText = true; break;
		case 'Image': obj.isImage = true; break;
		case 'Paragraph': obj.isParagraph = true; break;
	}
	if(this.se) obj.SE = CGMZ_Utils.parseSoundEffectJSON(this.se, "[CGMZ] Toast Manager");
	if(this.windowskin) obj.windowskin = CGMZ_Utils.getImageData(this.windowskin, "img");
	if(this.tone) obj.windowskinTone = this.tone;
	obj.width = this.width;
	obj.height = this.height;
	obj.note = this.note;
	obj.displayTime = this.displayTime;
	obj.backgroundStyle = this.backgroundStyle;
	obj.type = this.type;
	obj.id = this.id;
	if(obj.isImage) {
		obj.showBackground = this.showBackground;
		obj.picture = this.img;
	} else if(obj.isText) {
		obj.lineOne = this.textLine1;
		obj.lineOneColor = this.textLine1Color;
		obj.lineOneAlignment = this.textLine1Align;
		obj.lineTwo = this.textLine2;
		obj.lineTwoColor = this.textLine2Color;
		obj.lineTwoAlignment = this.textLine2Align;
	} else if(obj.isParagraph) {
		obj.header = this.headerLine;
		obj.opts = {drawDividers: this.headerDividers};
		obj.color1 = this.headerColor1;
		obj.color2 = this.headerColor2;
		obj.paragraph = this.paragraph;
		obj.paragraphAlign = this.paragraphAlignment;
	}
	return obj;
};
//=============================================================================
// CGMZ_Temp
//-----------------------------------------------------------------------------
// Store data to be used for toast windows.
//=============================================================================
//-----------------------------------------------------------------------------
// Create toast window array.
//-----------------------------------------------------------------------------
const alias_CGMZ_ToastManager_createPluginData = CGMZ_Temp.prototype.createPluginData;
CGMZ_Temp.prototype.createPluginData = function() {
	alias_CGMZ_ToastManager_createPluginData.call(this);
	this._toastWindows = [];
	this._toastPresets = {};
	for(const presetJSON of CGMZ.ToastManager.ToastPresets) {
		const data = CGMZ_Utils.parseJSON(presetJSON, null, '[CGMZ] Toast Manager', 'A toast preset had invalid JSON and could not be read. It will be skipped.');
		if(!data) continue;
		const preset = new CGMZ_ToastPreset(data);
		this._toastPresets[preset.id] = preset;
	}
};
//-----------------------------------------------------------------------------
// Get toast preset
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.getToastPreset = function(id) {
	return this._toastPresets[id];
};
//-----------------------------------------------------------------------------
// Get toast object from preset. Returns null if preset doesn't exist
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.getToastObjectFromPreset = function(id) {
	const preset = this.getToastPreset(id);
	if(!preset) return null;
	return preset.makeToastObject();
};
//-----------------------------------------------------------------------------
// Create toast window array.
//-----------------------------------------------------------------------------
const alias_CGMZ_ToastManager_registerPluginCommands = CGMZ_Temp.prototype.registerPluginCommands;
CGMZ_Temp.prototype.registerPluginCommands = function() {
	alias_CGMZ_ToastManager_registerPluginCommands.call(this);
	PluginManager.registerCommand("CGMZ_ToastManager", "Create Text Toast", this.pluginCommandCreateTextToast);
	PluginManager.registerCommand("CGMZ_ToastManager", "Create Image Toast", this.pluginCommandCreateImageToast);
	PluginManager.registerCommand("CGMZ_ToastManager", "Create Paragraph Toast", this.pluginCommandCreateParagraphToast);
	PluginManager.registerCommand("CGMZ_ToastManager", "Create Toast Preset", this.pluginCommandCreateToastPreset);
};
//-----------------------------------------------------------------------------
// Plugin Command - Create Text Toast
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandCreateTextToast = function(args) {
	const toast = {};
	toast.isText = true;
	toast.width = Number(args.width);
	if(args.height) toast.height = Number(args.height);
	if(args.SE.includes('{')) {
		toast.SE = CGMZ_Utils.parseSoundEffectJSON(args.SE, "[CGMZ] Toast Manager");
	} else { // legacy SE toast commands
		toast.SE = {name: args.SE, pan: 0, pitch: 100, volume: 90};
	}
	toast.lineOne = args.lineOne;
	toast.lineOneColor = Number(args.lineOneColor);
	toast.lineOneAlignment = args.lineOneAlignment;
	toast.lineTwo = args.lineTwo;
	toast.lineTwoColor = Number(args.lineTwoColor);
	toast.lineTwoAlignment = args.lineTwoAlignment;
	toast.displayTime = Number(args["Display Time"]);
	toast.backgroundStyle = args.backgroundStyle;
	toast.windowskinTone = args.windowskinTone;
	if(args.windowskin) toast.windowskin = CGMZ_Utils.getImageData(args.windowskin, "img");
	$cgmzTemp.createNewToast(toast);
};
//-----------------------------------------------------------------------------
// Plugin Command - Create Image Toast
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandCreateImageToast = function(args) {
	const toast = {};
	toast.isImage = true;
	toast.width = Number(args.width);
	if(args.height) toast.height = Number(args.height);
	if(args.SE.includes('{')) {
		toast.SE = CGMZ_Utils.parseSoundEffectJSON(args.SE, "[CGMZ] Toast Manager");
	} else { // legacy SE toast commands
		toast.SE = {name: args.SE, pan: 0, pitch: 100, volume: 90};
	}
	toast.showBackground = (args.showBackground === "true");
	toast.picture = args.image
	toast.displayTime = Number(args["Display Time"]);
	toast.windowskinTone = args.windowskinTone;
	if(args.windowskin) toast.windowskin = CGMZ_Utils.getImageData(args.windowskin, "img");
	$cgmzTemp.createNewToast(toast);
};
//-----------------------------------------------------------------------------
// Plugin Command - Create Paragraph Toast
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandCreateParagraphToast = function(args) {
	const toast = {};
	toast.isParagraph = true;
	toast.width = Number(args.width);
	if(args.SE) {
		toast.SE = CGMZ_Utils.parseSoundEffectJSON(args.SE, "[CGMZ] Toast Manager");
	}
	toast.header = args["Header Line"];
	toast.opts = {drawDividers: (args["Header Dividers"] === 'true')};
	toast.color1 = Number(args["Header Divider Color 1"]);
	toast.color2 = Number(args["Header Divider Color 2"]);
	toast.paragraph = args.Paragraph;
	toast.paragraphAlign = args["Paragraph Alignment"];
	toast.displayTime = Number(args["Display Time"]);
	toast.backgroundStyle = args.backgroundStyle;
	toast.windowskinTone = args.windowskinTone;
	if(args.windowskin) toast.windowskin = CGMZ_Utils.getImageData(args.windowskin, "img");
	$cgmzTemp.createNewToast(toast);
};
//-----------------------------------------------------------------------------
// Plugin Command - Create Toast Preset
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandCreateToastPreset = function(args) {
	const toast = $cgmzTemp.getToastObjectFromPreset(args.Id);
	if(toast) $cgmzTemp.createNewToast(toast);
};
//-----------------------------------------------------------------------------
// Add new toast object to toast queue
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.createNewToast = function(toastObject) {
	if(this.canCreateToast()) this._toastWindows.push(toastObject);
};
//-----------------------------------------------------------------------------
// Get first toast from queue
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.getToast = function() {
	return this._toastWindows.shift();
};
//-----------------------------------------------------------------------------
// Look at the first toast object without removing from queue
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.peekToast = function() {
	return this._toastWindows[0];
};
//-----------------------------------------------------------------------------
// Determine if toast waiting for display exists
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.hasToast = function() {
	return this._toastWindows.length > 0;
};
//-----------------------------------------------------------------------------
// Check if toasts can display
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.canCreateToast = function() {
	if(CGMZ.ToastManager.CanCreateSwitch && !$gameSwitches.value(CGMZ.ToastManager.CanCreateSwitch)) return false;
	return true;
};
//-----------------------------------------------------------------------------
// Check if toasts can display
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.canDisplayToast = function() {
	if(CGMZ.ToastManager.CanDisplaySwitch && !$gameSwitches.value(CGMZ.ToastManager.CanDisplaySwitch)) return false;
	return this.hasToast();
};
//=============================================================================
// SceneManager
//-----------------------------------------------------------------------------
// Determine if the scene should have toast windows made
//=============================================================================
//-----------------------------------------------------------------------------
// Determine if the scene should make toast windows
//-----------------------------------------------------------------------------
SceneManager.CGMZ_ToastManager_canCreateToasts = function() {
	if(this._scene.constructor === Scene_File || this._scene.constructor === Scene_Boot || this._scene.constructor === Scene_Gameover ||
	   this._scene.constructor === Scene_Save || this._scene.constructor === Scene_Load || this._scene.constructor === Scene_Options ||
	   this._scene.constructor === Scene_Title) {
		return false;
	}
	return true;
};
//=============================================================================
// Scene_Base
//-----------------------------------------------------------------------------
// Modify the base scene to add handling for the toast windows.
//=============================================================================
//-----------------------------------------------------------------------------
// Alias. Initialize whether the scene has toasts to false
//-----------------------------------------------------------------------------
const alias_CGMZ_ToastManager_SceneBase_initialize = Scene_Base.prototype.initialize;
Scene_Base.prototype.initialize = function() {
    alias_CGMZ_ToastManager_SceneBase_initialize.call(this);
	this._cgmz_hasToastWindows = false;
};
//-----------------------------------------------------------------------------
// Alias. Create toast windows after scene makes the window layer (if needed)
//-----------------------------------------------------------------------------
const alias_CGMZ_ToastManager_SceneBase_createWindowLayer = Scene_Base.prototype.createWindowLayer;
Scene_Base.prototype.createWindowLayer = function() {
    alias_CGMZ_ToastManager_SceneBase_createWindowLayer.call(this);
	if(SceneManager.CGMZ_ToastManager_canCreateToasts()) {
		this._cgmz_toastLayer = new WindowLayer();
		this._cgmz_toastLayer.x = (Graphics.width - Graphics.boxWidth) / 2;
		this._cgmz_toastLayer.y = (Graphics.height - Graphics.boxHeight) / 2;
		this.addChild(this._cgmz_toastLayer);
		this.CGMZ_ToastManager_createToastWindows();
	}
};
//-----------------------------------------------------------------------------
// Alias. Remove toast window layer.
//-----------------------------------------------------------------------------
const alias_CGMZ_ToastManager_SceneBase_terminate = Scene_Base.prototype.terminate;
Scene_Base.prototype.terminate = function() {
	alias_CGMZ_ToastManager_SceneBase_terminate.call(this);
	this.removeChild(this._cgmz_toastLayer);
};
//-----------------------------------------------------------------------------
// Add toast window
//-----------------------------------------------------------------------------
Scene_Base.prototype.CGMZ_ToastManager_addToast = function(window) {
    this._cgmz_toastLayer.addChild(window);
};
//-----------------------------------------------------------------------------
// Alias. Update toast windows
//-----------------------------------------------------------------------------
const alias_CGMZ_ToastManager_SceneBase_update = Scene_Base.prototype.update;
Scene_Base.prototype.update = function() {
	alias_CGMZ_ToastManager_SceneBase_update.call(this);
	if(this._cgmz_hasToastWindows && this.isActive() && !this.isBusy()) {
        this.CGMZ_ToastManager_updateToastWindows();
    }
};
//-----------------------------------------------------------------------------
// Create amount of toast windows depending on need.
//-----------------------------------------------------------------------------
Scene_Base.prototype.CGMZ_ToastManager_createToastWindows = function() {
	const rect1 = this.CGMZ_ToastManager_toastWindowRect();
    this._cgmz_toastWindow1 = new CGMZ_Window_Toast(rect1);
    this.CGMZ_ToastManager_addToast(this._cgmz_toastWindow1);
	if(CGMZ.ToastManager.MaxWindowCount > 1) {
		const rect2 = this.CGMZ_ToastManager_toastWindowRect();
		this._cgmz_toastWindow2 = new CGMZ_Window_Toast(rect2);
		this.CGMZ_ToastManager_addToast(this._cgmz_toastWindow2);
	}
	if(CGMZ.ToastManager.MaxWindowCount > 2) {
		const rect3 = this.CGMZ_ToastManager_toastWindowRect();
		this._cgmz_toastWindow3 = new CGMZ_Window_Toast(rect3);
		this.CGMZ_ToastManager_addToast(this._cgmz_toastWindow3);
	}
	this._cgmz_hasToastWindows = true;
};
//-----------------------------------------------------------------------------
// Get the toast window's starting rect. All values set to 0 because toast
// window properties are set for each toast window.
//-----------------------------------------------------------------------------
Scene_Base.prototype.CGMZ_ToastManager_toastWindowRect = function() {
	return new Rectangle(0, 0, 0, 0);
};
//-----------------------------------------------------------------------------
// Update toast windows. Determine which ones can open and their y value.
//-----------------------------------------------------------------------------
Scene_Base.prototype.CGMZ_ToastManager_updateToastWindows = function() {
	if($cgmzTemp.canDisplayToast()) {
		if(!this._cgmz_toastWindow1.isDisplaying() && this.CGMZ_ToastManager_canDisplayToast(1, $cgmzTemp.peekToast())) {
			const y = (CGMZ.ToastManager.DisplayFromBottom) ? Graphics.boxHeight : 0;
			this._cgmz_toastWindow1.y = y;
			this._cgmz_toastWindow1.open($cgmzTemp.getToast());
		}
		else if(CGMZ.ToastManager.MaxWindowCount > 1 && !this._cgmz_toastWindow2.isDisplaying() && this.CGMZ_ToastManager_canDisplayToast(2, $cgmzTemp.peekToast())) {
			if(CGMZ.ToastManager.DisplayFromBottom) {
				this._cgmz_toastWindow2.y = this._cgmz_toastWindow1.y - CGMZ.ToastManager.Spacing;
			} else {
				this._cgmz_toastWindow2.y = this._cgmz_toastWindow1.y + this._cgmz_toastWindow1.height + CGMZ.ToastManager.Spacing;
			}
			this._cgmz_toastWindow2.open($cgmzTemp.getToast());
		}
		else if(CGMZ.ToastManager.MaxWindowCount > 2 && !this._cgmz_toastWindow3.isDisplaying()) {
			if(CGMZ.ToastManager.DisplayFromBottom) {
				this._cgmz_toastWindow3.y = this._cgmz_toastWindow2.y - CGMZ.ToastManager.Spacing;
			} else {
				this._cgmz_toastWindow3.y = this._cgmz_toastWindow2.y + this._cgmz_toastWindow2.height + CGMZ.ToastManager.Spacing;
			}
			this._cgmz_toastWindow3.open($cgmzTemp.getToast());
		}
	}
};
//-----------------------------------------------------------------------------
// Determine if there is enough space for the toast to display
// Always true if fixed height, always true if window is 1 and max is 1,
// always true if window is 2 and max is 2. Method not called for third window
// as third window can always display.
//-----------------------------------------------------------------------------
Scene_Base.prototype.CGMZ_ToastManager_canDisplayToast = function(toastNum, toastObject) {
	if(CGMZ.ToastManager.FixedHeight) return true;
	if(toastNum === 1 && CGMZ.ToastManager.MaxWindowCount === 1) return true;
	if(toastNum === 2 && (CGMZ.ToastManager.MaxWindowCount === 2 || !this._cgmz_toastWindow3._isDisplaying)) return true;
	if(!this._cgmz_toastWindow2._isDisplaying && !this._cgmz_toastWindow3._isDisplaying) return true;
	const neededHeight = (toastObject.height) ? this.calcWindowHeight(toastObject.height, false) : this.calcWindowHeight(CGMZ.ToastManager.Height, false);
	let y = (CGMZ.ToastManager.DisplayFromBottom) ? Graphics.boxHeight : 0;
	if(toastNum === 2 && CGMZ.ToastManager.DisplayFromBottom) y = this._cgmz_toastWindow1.y - CGMZ.ToastManager.Spacing;
	if(toastNum === 2 && !CGMZ.ToastManager.DisplayFromBottom) y = this._cgmz_toastWindow1.y + this._cgmz_toastWindow1.height + CGMZ.ToastManager.Spacing;
	const line2 = (CGMZ.ToastManager.MaxWindowCount > 1) ? {y:this._cgmz_toastWindow2.y,height:this._cgmz_toastWindow2.height} : {y:0,height:0};
	const line3 = (CGMZ.ToastManager.MaxWindowCount > 2) ? {y:this._cgmz_toastWindow3.y,height:this._cgmz_toastWindow3.height} : {y:0,height:0};
	if(toastNum === 1) {
		if(CGMZ.ToastManager.MaxWindowCount > 2 && this._cgmz_toastWindow3._isDisplaying) {
			if((CGMZ.ToastManager.DisplayFromBottom)) {
				if(y > (line3.y + line3.height) && (y - neededHeight) < (line3.y + line3.height)) return false;
			} else {
				if(y < (line3.y) && (y + neededHeight) > (line3.y)) return false;
			}
		}
		if(CGMZ.ToastManager.MaxWindowCount > 1) {
			if((CGMZ.ToastManager.DisplayFromBottom)) {
				if(y > (line2.y + line2.height) && (y - neededHeight) < (line2.y + line2.height)) return false;
			} else {
				if(y < (line2.y) && (y + neededHeight) > (line2.y)) return false;
			}
		}
	}
	if(toastNum === 2 && CGMZ.ToastManager.MaxWindowCount > 2 && this._cgmz_toastWindow3._isDisplaying) {
		if(!this._cgmz_toastWindow1._isDisplaying) return false;
		if((CGMZ.ToastManager.DisplayFromBottom)) {
			if(y > (line3.y + line3.height) && (y - neededHeight) < (line3.y + line3.height)) return false;
		} else {
			if(y < (line3.y) && (y + neededHeight) > (line3.y)) return false;
		}
	}
	return true;
};
//=============================================================================
// CGMZ_Window_Toast
//-----------------------------------------------------------------------------
// The toast window, handles displaying the toast information
//=============================================================================
function CGMZ_Window_Toast() {
    this.initialize.apply(this, arguments);
}
CGMZ_Window_Toast.prototype = Object.create(Window_Base.prototype);
CGMZ_Window_Toast.prototype.constructor = CGMZ_Window_Toast;
//-----------------------------------------------------------------------------
// Initialize
//-----------------------------------------------------------------------------
CGMZ_Window_Toast.prototype.initialize = function(rect) {
    Window_Base.prototype.initialize.call(this, rect);
    this.opacity = 0;
    this.contentsOpacity = 0;
    this._showCount = 0;
	this._isDisplaying = false;
	this._showBG = true;
	this._bgType = 2;
	this._tone = null;
	this._bitmap = ImageManager._emptyBitmap;
	this._bitmapLoading = false;
    this.contents.clear();
	this.createDimmerSprite();
	this._dimmerSprite.opacity = 0;
};
//-----------------------------------------------------------------------------
// Update for fade in/out
//-----------------------------------------------------------------------------
CGMZ_Window_Toast.prototype.update = function() {
	if(this.isDisplaying()) {
		Window_Base.prototype.update.call(this);
		if (this._showCount > 0 && this.canDisplay()) {
			this.updateFadeIn();
			this._showCount--;
		} else {
			this.updateFadeOut();
		}
		if(this.contentsOpacity <= 0 && this._showCount <= 0) {
			this.y = 0;
			this.height = 0;
			this._isDisplaying = false;
		}
	}
};
//-----------------------------------------------------------------------------
// Check if everything is ready for displaying the toast.
//-----------------------------------------------------------------------------
CGMZ_Window_Toast.prototype.canDisplay = function() {
	return !this._bitmapLoading;
};
//-----------------------------------------------------------------------------
// Check if this is currently displaying
//-----------------------------------------------------------------------------
CGMZ_Window_Toast.prototype.isDisplaying = function() {
	return this._isDisplaying
};
//-----------------------------------------------------------------------------
// Load listener for after bitmap is done loading.
//-----------------------------------------------------------------------------
CGMZ_Window_Toast.prototype.onBitmapLoaded = function() {
	this._bitmapLoading = false;
	const sx = 0;
	const sy = 0;
	const sw = this._bitmap.width;
	const sh = this._bitmap.height;
	const dx = 0;
	const dy = 0;
	const dw = this.contents.width;
	const dh = this.contents.height;
	this.contents.blt(this._bitmap, sx, sy, sw, sh, dx, dy, dw, dh);
	this._bitmap = ImageManager._emptyBitmap;
};
//-----------------------------------------------------------------------------
// Fade in
//-----------------------------------------------------------------------------
CGMZ_Window_Toast.prototype.updateFadeIn = function() {
	this.opacity += 16 * (this._showBG);
	this._dimmerSprite.opacity += 16 * (this._bgType === 1);
    this.contentsOpacity += 16;
};
//-----------------------------------------------------------------------------
// Fade out
//-----------------------------------------------------------------------------
CGMZ_Window_Toast.prototype.updateFadeOut = function() {
	this.opacity -= 16 * (this._showBG);
	this._dimmerSprite.opacity -= 16 * (this._bgType === 1);
    this.contentsOpacity -= 16;
};
//-----------------------------------------------------------------------------
// Open the window
//-----------------------------------------------------------------------------
CGMZ_Window_Toast.prototype.open = function(toastObject) {
	this._isDisplaying = true;
    this.refresh(toastObject);
    this._showCount = (toastObject && toastObject.displayTime) ? toastObject.displayTime : CGMZ.ToastManager.DisplayTime;
};
//-----------------------------------------------------------------------------
// Close the window
//-----------------------------------------------------------------------------
CGMZ_Window_Toast.prototype.close = function() {
    this._showCount = 0;
};
//-----------------------------------------------------------------------------
// Refresh the window
//-----------------------------------------------------------------------------
CGMZ_Window_Toast.prototype.refresh = function(toastObject) {
	if(!toastObject) return;
	this._showBG = true;
	this._bgType = 2;
	this._tone = null;
	this.doCommonEffects(toastObject);
	this.refreshDimmerBitmap();
	this.contents.clear();
	this.resetTextColor();
	//Default text toast
	if(toastObject.isText) {
		if(!toastObject.lineOneColor) toastObject.lineOneColor = 0;
		if(!toastObject.lineTwoColor) toastObject.lineTwoColor = 0;
		if(!toastObject.lineOneAlignment) toastObject.lineOneAlignment = 'center';
		if(!toastObject.lineTwoAlignment) toastObject.lineTwoAlignment = 'center';
		const line1 = `\\c[${toastObject.lineOneColor}]${toastObject.lineOne}\\c[0]`;
		const line2 = `\\c[${toastObject.lineTwoColor}]${toastObject.lineTwo}\\c[0]`;
		this.CGMZ_drawTextLine(line1, 0, 0, this.contents.width, toastObject.lineOneAlignment);
		this.CGMZ_drawTextLine(line2, 0, this.lineHeight(), this.contents.width, toastObject.lineTwoAlignment);
	}
	//Default image toast
	if(toastObject.isImage) {
		this._bitmapLoading = true;
		this._showBG = toastObject.showBackground;
		this._bitmap = ImageManager.loadPicture(toastObject.picture);
		this._bitmap.addLoadListener(this.onBitmapLoaded.bind(this));
	}
	// Default paragraph toast
	if(toastObject.isParagraph) {
		let paraY = 0;
		if(toastObject.header) {
			paraY = this.CGMZ_drawHeader(toastObject.header, 0, toastObject.color1, toastObject.color2, toastObject.opts);
		}
		this.CGMZ_drawText(toastObject.paragraph, 0, 0, paraY, this.contents.width, toastObject.paragraphAlign);
	}
	this.processCustomToast(toastObject);
};
//-----------------------------------------------------------------------------
// Perform common effects for all toast types
// 1. Resize width
// 2. Resize height
// 3. Play se
// 4. Change background style
// 5. Change windowskin Tone
// 6. Change windowskin
//-----------------------------------------------------------------------------
CGMZ_Window_Toast.prototype.doCommonEffects = function(toastObject) {
	if(toastObject.width && !CGMZ.ToastManager.FixedWidth) {
		this.updateWidth(toastObject.width);
	} else {
		this.updateWidth(CGMZ.ToastManager.Width);
	}
	if(toastObject.height && !CGMZ.ToastManager.FixedHeight) {
		this.updateHeight(toastObject.height);
	} else {
		if(toastObject.isParagraph) {
			let neededHeight = (toastObject.header) ? this.textSizeEx(toastObject.header).height : 0;
			neededHeight += this.CGMZ_drawText(toastObject.paragraph, 0, 0, 0, this.width, toastObject.paragraphAlign);
			this.updateHeightExact(neededHeight + this.padding * 2);
		} else {
			this.updateHeight(CGMZ.ToastManager.Height);
		}
	}
	this.createContents();
	if(toastObject.hasOwnProperty('SE')) {
		AudioManager.playSe(toastObject.SE);
	}
	if(toastObject.hasOwnProperty('backgroundStyle') && toastObject.backgroundStyle) {
		const types = ['Transparent','Dim','Window'];
		this._bgType = types.indexOf(toastObject.backgroundStyle);
		this._showBG = (this._bgType === 2);
	}
	if(toastObject.hasOwnProperty('windowskinTone') && toastObject.windowskinTone) {
		const tone = CGMZ_Utils.parseJSON(toastObject.windowskinTone, {Red: 0, Blue: 0, Green: 0}, "[CGMZ] Toast Manager", "Your windowskinTone argument had invalid JSON and could not be read.");
		this._tone = [Number(tone.Red), Number(tone.Green), Number(tone.Blue)];
	}
	if(toastObject.hasOwnProperty('windowskin')) {
		this.windowskin = ImageManager.loadBitmap(toastObject.windowskin.folder, toastObject.windowskin.filename);
	} else {
		this.windowskin = ImageManager.loadSystem("Window");
	}
};
//-----------------------------------------------------------------------------
// Change window width. Also changes x value to re-center based on custom width
//-----------------------------------------------------------------------------
CGMZ_Window_Toast.prototype.updateWidth = function(width) {
	this.width = width;
	switch(CGMZ.ToastManager.DisplayAlignment) {
		case 'left':this.x = 0; break;
		case 'center': this.x = Graphics.boxWidth/2 - width/2; break;
		case 'right': this.x = Graphics.boxWidth - width; break;
	}
	this.x += CGMZ.ToastManager.XOffset;
};
//-----------------------------------------------------------------------------
// Change window height. Also changes y value based on height
//-----------------------------------------------------------------------------
CGMZ_Window_Toast.prototype.updateHeight = function(height) {
	const neededHeight = this.fittingHeight(height);
	this.height = neededHeight;
	this.y -= neededHeight * CGMZ.ToastManager.DisplayFromBottom;
};
//-----------------------------------------------------------------------------
// Change window height from exact pixel value. Also changes y value based on height
//-----------------------------------------------------------------------------
CGMZ_Window_Toast.prototype.updateHeightExact = function(height) {
	this.height = height;
	this.y -= height * CGMZ.ToastManager.DisplayFromBottom;
};
//-----------------------------------------------------------------------------
// Update the tone of the windowskin. If tone property no exist, use default
//-----------------------------------------------------------------------------
CGMZ_Window_Toast.prototype.updateTone = function() {
	if(!this._tone) {
		Window_Base.prototype.updateTone.call(this);
		return;
	}
    this.setTone(this._tone[0], this._tone[1], this._tone[2]);
};
//-----------------------------------------------------------------------------
// Do nothing, update this in opacity update function instead
//-----------------------------------------------------------------------------
CGMZ_Window_Toast.prototype.updateBackgroundDimmer = function() {
};
//-----------------------------------------------------------------------------
// Processing for custom toasts. To be used by other plugins for toast behavior
//-----------------------------------------------------------------------------
CGMZ_Window_Toast.prototype.processCustomToast = function(toastObject) {
	// Put toast behavior here
};