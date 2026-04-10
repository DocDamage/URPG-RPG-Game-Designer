/*:
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/titlecommandwindow/
 * @target MZ
 * @base CGMZ_Core
 * @orderAfter CGMZ_Core
 * @orderAfter CGMZ_ExitToDesktop
 * @plugindesc Manage the menu command window
 * @help
 * ============================================================================
 * For terms and conditions using this plugin in your game please visit:
 * https://www.caspergaming.com/terms-of-use/
 * ============================================================================
 * Become a Patron to get access to beta/alpha plugins plus other goodies!
 * https://www.patreon.com/CasperGamingRPGM
 * ============================================================================
 * Version: 1.5.0
 * ----------------------------------------------------------------------------
 * Compatibility: Only tested with my CGMZ plugins.
 * Made for RPG Maker MZ 1.10.0
 * ----------------------------------------------------------------------------
 * Description: Use this plugin to easily manage the command window in the
 * menu scene. It allows you to re-arrange commands or use JavaScript to 
 * add custom commands which are capable of calling custom plugin scenes or
 * functions.
 * ----------------------------------------------------------------------------
 * Documentation:
 * This plugin will overwrite the default title window if keep originals is
 * off. It is best to place this below any other plugins that add commands to
 * the title window if this option is used.
 *
 * The command symbol should be unique and not blank for every command. This
 * symbol is how the plugin knows internally which JS code to run.
 *
 * Some Command Symbols can have special meanings, mainly when they represent
 * the original commands.
 * The following symbols represent the original commands (case sensitive):
 * newGame - Will handle like the original new game command
 * continue - Will handle like the original continue command
 * options - Will handle like the original options command
 * 
 * It is important that you do not use these strings as the Command Symbol
 * property unless you mean to refer to the original commands.
 * -------------------------Command Pictures-----------------------------------
 * To use pictures as commands, set the Visible Commands parameter to 0 so the
 * command window is hidden, and this plugin will then display your command
 * pictures in its place.
 * --------------------------Latest Version------------------------------------
 * Hi all, this latest version adds pictures you can use in place of the
 * command window. You still move through pictures using the gamepad or
 * keyboard (or touch ui mouse over them) as normal. They can have both an
 * active and inactive state, and function similarly to how [CGMZ] Game End
 * Window pictures work so if you use that plugin you will already be familiar
 * with set up for this plugin.
 *
 * To use them, set the Visible Commands to 0 for your command window in this
 * plugin.
 *
 * Version 1.5.0
 * - Added pictures for commands
 *
 * @param Visible Commands
 * @type number
 * @min 0
 * @default 3
 * @desc This is the number of commands that will be visible in the window without scrolling
 *
 * @param Columns
 * @type number
 * @min 1
 * @default 1
 * @desc The amount of columns in the title command window
 *
 * @param Window Width
 * @type number
 * @min -1
 * @default 30
 * @desc Percentage of the screen to use for title command window width. Set to -1 for default width.
 *
 * @param Alignment
 * @type select
 * @option left
 * @option center
 * @option right
 * @default center
 * @desc The alignment of the command text in the window
 *
 * @param Icon Alignment
 * @type select
 * @option left
 * @option right
 * @default left
 * @desc The alignment of the command icons in the window
 *
 * @param Keep Original Commands
 * @type boolean
 * @default true
 * @desc Determine whether to show the original commands in their original order.
 *
 * @param Commands
 * @type struct<Handler>[]
 * @desc Command Name and associated js commands
 * @default []
 *
 * @param Picture Setup
 *
 * @param Pictures
 * @parent Picture Setup
 * @type struct<Picture>[]
 * @desc Selectable images that can be used in place of the command window
 * @default []
 *
 * @param Picture Base X
 * @parent Picture Setup
 * @type number
 * @default 0
 * @min -9999
 * @desc The base X of all pictures
 *
 * @param Picture Base Y
 * @parent Picture Setup
 * @type number
 * @default 0
 * @min -9999
 * @desc The base Y of all pictures
 *
 * @param Picture Add X
 * @parent Picture Setup
 * @type number
 * @default 0
 * @min -9999
 * @desc Amount added to picture X after each image
 *
 * @param Picture Add Y
 * @parent Picture Setup
 * @type number
 * @default 0
 * @min -9999
 * @desc Amount added to picture Y after each image
 *
 * @param Integrations
 *
 * @param Window Background
 * @parent Integrations
 * @desc The [CGMZ] Window Backgrounds preset id to use for the title command window.
 *
 * @param Window Settings
 * @parent Integrations
 * @desc The [CGMZ] Window Settings preset id to use for the title command window.
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
 * @desc This symbol is used internally to recognize the command. Special meaning for original commands (see documentation).
 *
 * @param JS Command
 * @type multiline_string
 * @desc JavaScript to run when command is selected.
 *
 * @param Show JS
 * @type multiline_string
 * @default return true;
 * @desc JavaScript to run to determine if the command is shown
 *
 * @param Enable JS
 * @type multiline_string
 * @default return true;
 * @desc JavaScript to run to determine if the command is selectable
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
 *
 * @param Selected Info
 *
 * @param Selected Text
 * @parent Selected Info
 * @desc Text to display when selected. Blank = same as default command name
 *
 * @param Selected Icon
 * @parent Selected Info
 * @type icon
 * @default 0
 * @desc An icon to show aligned separately from the command text. 0 = same as default icon
 *
 * @param Selected Back Img
 * @parent Selected Info
 * @type file
 * @dir img/
 * @desc A background image to use for the command. Blank = default back image
 *
 * @param Selected Back Img X
 * @parent Selected Info
 * @type number
 * @default 0
 * @min 0
 * @desc The x coordinate to start the background image from the source image (upper left corner) while selected
 *
 * @param Selected Back Img Y
 * @parent Selected Info
 * @type number
 * @default 0
 * @min 0
 * @desc The y coordinate to start the background image from the source image (upper left corner) while selected
*/
/*~struct~Picture:
 * @param Command Symbol
 * @desc The command symbol this picture will activate
 *
 * @param Inactive Image
 * @type file
 * @dir img
 * @desc Image file to use when not active
 *
 * @param Active Image
 * @type file
 * @dir img
 * @desc Image file to use when active
 *
 * @param X Offset
 * @type number
 * @default 0
 * @min -9999
 * @desc Amount of pixels to offset the x for this picture
 *
 * @param Y Offset
 * @type number
 * @default 0
 * @min -9999
 * @desc Amount of pixels to offset the y for this picture
*/
/*:zh-CN
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/titlecommandwindow/
 * @target MZ
 * @base CGMZ_Core
 * @orderAfter CGMZ_Core
 * @orderAfter CGMZ_ExitToDesktop
 * @plugindesc 标题选项拓展系统（为标题画面增加新的选项和美化）
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
 * 【插件版本】V 1.5.0
 * ----------------------------------------------------------------------------
 * 【兼容性】仅测试作者所制作的插件
 * 【RM版本】RPG Maker MZ 1.10.0
 * ----------------------------------------------------------------------------
 * 【插件描述】
 * 本插件可以轻松管理标题画面选项。
 * 可以使用JS语言命令来制作新的自定义选项。
 * 用于制作类似“鸣谢”、“制作名单”、“图鉴”等需要其他插件辅助的新选项。
 * 可以设置选项文字颜色，增加图标等。
 *
 * 【搭配插件】
 * CGMZ Core:核心插件，运行作者插件的必须插件!!!
 * CGMZ Changelog:版本记录插件，制作一个版本更新记录的界面。
 * CGMZ Credits:制作名单插件，制作一个游戏插件、素材等资源来源和资源作者等信息的界面。
 * CGMZ Exit To Desktop:退出游戏插件，制作一个选项，用于结束和退出游戏。
 * 注：本插件在插件列表中必须置于上述插件之下。
 * -----------------------------------------------------------------------------
 * 【使用说明】
 * 一、本插件支持关闭原标题选项的功能。
 *     如果你使用了其他添加标题画面选项的插件。
 *     请把本插件在插件列表里置于其他插件下方。
 * 二、每个自定义选项的命令字符或JS命令只能选择使用其中一种，且不能取空值。
 * 三、以下命令字符具有特殊含义，是用于默认指令的，请勿用于其他用途。字符区分大小写。
 *     如：newGame - 开始新游戏，continue - 继续游戏， options - 打开游戏设置。
 *
 * 举例：继续游戏的选项设置
 *     1、选项名称：继续游戏 （游戏标题画面显示的描述）
 *     2、命令字符：continue
 *     3、JS命令：（留空）
 * 文本命令则是：{"Command Name":"继续游戏","Command Symbol":"continue","JS Command":"\"\""}
 *
 * 举例2：自定义制作名单选项的设置（举例使用了作者的CGMZ Credits插件和JS脚本命令）
 *     1、选项名称：制作名单
 *     2、命令字符：（留空）
 *     3、JS命令：SceneManager.push(CGMZ_Scene_Credits);
 *
 * 四、通过设置，可以支持标题选项添加图标\I[n]或使用颜色\C[n]等文本指令。
 * 
 * -------------------------Command Pictures-----------------------------------
 * To use pictures as commands, set the Visible Commands parameter to 0 so the
 * command window is hidden, and this plugin will then display your command
 * pictures in its place.
 * ---------------------------------------------------------------------------
 *【版本更新历史】
 * Hi all, this latest version adds pictures you can use in place of the
 * command window. You still move through pictures using the gamepad or
 * keyboard (or touch ui mouse over them) as normal. They can have both an
 * active and inactive state, and function similarly to how [CGMZ] Game End
 * Window pictures work so if you use that plugin you will already be familiar
 * with set up for this plugin.
 *
 * To use them, set the Visible Commands to 0 for your command window in this
 * plugin.
 *
 * Version 1.5.0
 * - Added pictures for commands
 * 
 * @param Visible Commands
 * @text 显示选项数
 * @type number
 * @min 0
 * @default 3
 * @desc 标题画面里显示的选项数，实际选项多于显示数会以滚动形式显示。显示选项数过多会超出画面和覆盖标题，须设置分辨率。
 *
 * @param Columns
 * @type number
 * @min 1
 * @default 1
 * @desc The amount of columns in the title command window
 *
 * @param Window Width
 * @type number
 * @min -1
 * @default 30
 * @desc Percentage of the screen to use for title command window width. Set to -1 for default width.
 *
 * @param Alignment
 * @text 选项中文字位置
 * @type select
 * @option left
 * @option center
 * @option right
 * @default center
 * @desc 设置选项框中文字的位置。Left-靠左，Center-居中，Right-靠右。
 *
 * @param Icon Alignment
 * @type select
 * @option left
 * @option right
 * @default left
 * @desc The alignment of the command icons in the window
 *
 * @param Keep Original Commands
 * @text 保留默认选项
 * @type boolean
 * @default true
 * @desc 是否保留游戏默认的选项。如：重新开始、继续游戏和游戏设置。Ture-保留，False-不保留。
 *
 * @param Commands
 * @text 自定义标题选项
 * @type struct<Handler>[]
 * @desc 设置你想要的标题画面选项。
 * @default []
 *
 * @param Picture Setup
 *
 * @param Pictures
 * @parent Picture Setup
 * @type struct<Picture>[]
 * @desc Selectable images that can be used in place of the command window
 * @default []
 *
 * @param Picture Base X
 * @parent Picture Setup
 * @type number
 * @default 0
 * @min -9999
 * @desc The base X of all pictures
 *
 * @param Picture Base Y
 * @parent Picture Setup
 * @type number
 * @default 0
 * @min -9999
 * @desc The base Y of all pictures
 *
 * @param Picture Add X
 * @parent Picture Setup
 * @type number
 * @default 0
 * @min -9999
 * @desc Amount added to picture X after each image
 *
 * @param Picture Add Y
 * @parent Picture Setup
 * @type number
 * @default 0
 * @min -9999
 * @desc Amount added to picture Y after each image
 *
 * @param Integrations
 *
 * @param Window Background
 * @parent Integrations
 * @desc The [CGMZ] Window Backgrounds preset id to use for the title command window.
 *
 * @param Window Settings
 * @parent Integrations
 * @desc The [CGMZ] Window Settings preset id to use for the title command window.
*/
/*~struct~Handler:zh-CN
 * @param Command Name
 * @text 选项名字（显示）
 * @desc 在标题画面显示的选项名字。支持使用文本指令。如 \I[n]图标、\C[n]颜色等。
 *
 * @param Icon
 * @type icon
 * @default 0
 * @desc An icon to show for the command, if 0 will not show any icon
 *
 * @param Command Symbol
 * @text 命令字符
 * @desc 系统默认选项的指令，如：newGame、continue、options等。
 *
 * @param JS Command
 * @text JS命令
 * @type multiline_string
 * @desc 设置自定义选项用的JS命令，取决于你所使用的插件的脚本指令。
 *
 * @param Show JS
 * @type multiline_string
 * @default return true;
 * @desc JavaScript to run to determine if the command is shown
 *
 * @param Enable JS
 * @type multiline_string
 * @default return true;
 * @desc JavaScript to run to determine if the command is selectable
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
 *
 * @param Selected Info
 *
 * @param Selected Text
 * @parent Selected Info
 * @desc Text to display when selected. Blank = same as default command name
 *
 * @param Selected Icon
 * @parent Selected Info
 * @type icon
 * @default 0
 * @desc An icon to show aligned separately from the command text. 0 = same as default icon
 *
 * @param Selected Back Img
 * @parent Selected Info
 * @type file
 * @dir img/
 * @desc A background image to use for the command. Blank = default back image
 *
 * @param Selected Back Img X
 * @parent Selected Info
 * @type number
 * @default 0
 * @min 0
 * @desc The x coordinate to start the background image from the source image (upper left corner) while selected
 *
 * @param Selected Back Img Y
 * @parent Selected Info
 * @type number
 * @default 0
 * @min 0
 * @desc The y coordinate to start the background image from the source image (upper left corner) while selected
*/
/*~struct~Picture:zh-CN
 * @param Command Symbol
 * @desc The command symbol this picture will activate
 *
 * @param Inactive Image
 * @type file
 * @dir img
 * @desc Image file to use when not active
 *
 * @param Active Image
 * @type file
 * @dir img
 * @desc Image file to use when active
 *
 * @param X Offset
 * @type number
 * @default 0
 * @min -9999
 * @desc Amount of pixels to offset the x for this picture
 *
 * @param Y Offset
 * @type number
 * @default 0
 * @min -9999
 * @desc Amount of pixels to offset the y for this picture
*/
/*:es 
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/titlecommandwindow/
 * @target MZ
 * @base CGMZ_Core
 * @orderAfter CGMZ_Core
 * @orderAfter CGMZ_ExitToDesktop
 * @plugindesc Administrar la ventana de comandos del menú
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
 * Versión: 1.5.0
 * ----------------------------------------------------------------------------
 * Compatibilidad: Sólo probado con mis CGMZ plugins.
 * Hecho para RPG Maker MZ 1.10.0
 * ----------------------------------------------------------------------------
 * Descripción: Usa este plugin para administrar fácilmente la ventana de 
 * comandos en la escena del menú. Te permite reorganizar los comandos o usar 
 * JavaScript para agregar comandos personalizados que sean capaces de llamar 
 * escenas de plugin personalizados o funciones.
 * ----------------------------------------------------------------------------
 * Documentación:
 * Este plugin sobrescribirá la ventana de título predeterminada si conservar
 * originales está desactivado. Es mejor colocar esto debajo de cualquier otro
 * complemento que agregue comandos a la ventana de título si se utiliza esta
 * opción.
 *
 * El símbolo de comando debe ser único y no estar en blanco para cada comando.
 * Este símbolo es cómo el plugin sabe internamente qué código JS ejecutar.
 *
 * Algunos Símbolos de Comando pueden tener significados especiales, 
 * principalmente cuando representan los comandos originales.
 * Los siguientes símbolos representan los comandos originales (se distingue
 * entre mayúsculas y minúsculas):
 * newGame - Manejará como el nuevo comando original del juego
 * continue - Manejará como el comando de continuación original
 * options - Manejará como el comando de opciones original
 * 
 * Es importante que no utilicee estas cadenas como la propiedad de símbolo
 * de comando a menos que desee hacer referencia a los comandos originales.
 * -------------------------Command Pictures-----------------------------------
 * To use pictures as commands, set the Visible Commands parameter to 0 so the
 * command window is hidden, and this plugin will then display your command
 * pictures in its place.
 * --------------------------Latest Version------------------------------------
 * Hi all, this latest version adds pictures you can use in place of the
 * command window. You still move through pictures using the gamepad or
 * keyboard (or touch ui mouse over them) as normal. They can have both an
 * active and inactive state, and function similarly to how [CGMZ] Game End
 * Window pictures work so if you use that plugin you will already be familiar
 * with set up for this plugin.
 *
 * To use them, set the Visible Commands to 0 for your command window in this
 * plugin.
 *
 * Version 1.5.0
 * - Added pictures for commands
 *
 * @param Visible Commands
 * @text Comandos visibles
 * @type number
 * @min 0
 * @default 3
 * @desc Este es el número de comandos que serán visibles en la ventana sin necesidad de desplazarse.
 *
 * @param Columns
 * @type number
 * @min 1
 * @default 1
 * @desc The amount of columns in the title command window
 *
 * @param Window Width
 * @type number
 * @min -1
 * @default 30
 * @desc Percentage of the screen to use for title command window width. Set to -1 for default width.
 *
 * @param Alignment
 * @text Alineación
 * @type select
 * @option left
 * @option center
 * @option right
 * @default center
 * @desc La alineación del texto del comando en la ventana.
 *
 * @param Icon Alignment
 * @type select
 * @option left
 * @option right
 * @default left
 * @desc The alignment of the command icons in the window
 *
 * @param Keep Original Commands
 * @text Mantener comandos originales
 * @type boolean
 * @default true
 * @desc Determine si desea mostrar los comandos originales en su orden original.
 *
 * @param Commands
 * @text Comandos 
 * @type struct<Handler>[]
 * @desc Nombre del comando y comandos js asociados.
 * @default []
 *
 * @param Picture Setup
 *
 * @param Pictures
 * @parent Picture Setup
 * @type struct<Picture>[]
 * @desc Selectable images that can be used in place of the command window
 * @default []
 *
 * @param Picture Base X
 * @parent Picture Setup
 * @type number
 * @default 0
 * @min -9999
 * @desc The base X of all pictures
 *
 * @param Picture Base Y
 * @parent Picture Setup
 * @type number
 * @default 0
 * @min -9999
 * @desc The base Y of all pictures
 *
 * @param Picture Add X
 * @parent Picture Setup
 * @type number
 * @default 0
 * @min -9999
 * @desc Amount added to picture X after each image
 *
 * @param Picture Add Y
 * @parent Picture Setup
 * @type number
 * @default 0
 * @min -9999
 * @desc Amount added to picture Y after each image
 *
 * @param Integrations
 *
 * @param Window Background
 * @parent Integrations
 * @desc The [CGMZ] Window Backgrounds preset id to use for the title command window.
 *
 * @param Window Settings
 * @parent Integrations
 * @desc The [CGMZ] Window Settings preset id to use for the title command window.
*/
/*~struct~Handler:es
 * @param Command Name
 * @text Nombre de comando
 * @desc Nombre del comando que se mostrará en la ventana de comandos.
 *
 * @param Icon
 * @type icon
 * @default 0
 * @desc An icon to show for the command, if 0 will not show any icon
 *
 * @param Command Symbol
 * @text Símbolo de comando
 * @desc Este símbolo se usa internamente para reconocer el comando. Significado especial para comandos originales (ver documentación).
 *
 * @param JS Command
 * @text Comando JS
 * @type multiline_string
 * @desc JavaScript para ejecutar cuando se selecciona el comando.
 *
 * @param Show JS
 * @type multiline_string
 * @default return true;
 * @desc JavaScript to run to determine if the command is shown
 *
 * @param Enable JS
 * @type multiline_string
 * @default return true;
 * @desc JavaScript to run to determine if the command is selectable
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
 *
 * @param Selected Info
 *
 * @param Selected Text
 * @parent Selected Info
 * @desc Text to display when selected. Blank = same as default command name
 *
 * @param Selected Icon
 * @parent Selected Info
 * @type icon
 * @default 0
 * @desc An icon to show aligned separately from the command text. 0 = same as default icon
 *
 * @param Selected Back Img
 * @parent Selected Info
 * @type file
 * @dir img/
 * @desc A background image to use for the command. Blank = default back image
 *
 * @param Selected Back Img X
 * @parent Selected Info
 * @type number
 * @default 0
 * @min 0
 * @desc The x coordinate to start the background image from the source image (upper left corner) while selected
 *
 * @param Selected Back Img Y
 * @parent Selected Info
 * @type number
 * @default 0
 * @min 0
 * @desc The y coordinate to start the background image from the source image (upper left corner) while selected
*/
/*~struct~Picture:es
 * @param Command Symbol
 * @desc The command symbol this picture will activate
 *
 * @param Inactive Image
 * @type file
 * @dir img
 * @desc Image file to use when not active
 *
 * @param Active Image
 * @type file
 * @dir img
 * @desc Image file to use when active
 *
 * @param X Offset
 * @type number
 * @default 0
 * @min -9999
 * @desc Amount of pixels to offset the x for this picture
 *
 * @param Y Offset
 * @type number
 * @default 0
 * @min -9999
 * @desc Amount of pixels to offset the y for this picture
*/
Imported.CGMZ_Title_CommandWindow = true;
CGMZ.Versions["Title Command Window"] = "1.5.0";
CGMZ.Title_CommandWindow = {};
CGMZ.Title_CommandWindow.parameters = PluginManager.parameters('CGMZ_TitleCommandWindow');
CGMZ.Title_CommandWindow.Alignment = CGMZ.Title_CommandWindow.parameters["Alignment"];
CGMZ.Title_CommandWindow.IconAlignment = CGMZ.Title_CommandWindow.parameters["Icon Alignment"];
CGMZ.Title_CommandWindow.WindowSettings = CGMZ.Title_CommandWindow.parameters["Window Settings"];
CGMZ.Title_CommandWindow.WindowBackground = CGMZ.Title_CommandWindow.parameters["Window Background"];
CGMZ.Title_CommandWindow.VisibleCommands = Number(CGMZ.Title_CommandWindow.parameters["Visible Commands"]);
CGMZ.Title_CommandWindow.WindowWidth = Number(CGMZ.Title_CommandWindow.parameters["Window Width"]);
CGMZ.Title_CommandWindow.Columns = Number(CGMZ.Title_CommandWindow.parameters["Columns"]);
CGMZ.Title_CommandWindow.PictureBaseX = Number(CGMZ.Title_CommandWindow.parameters["Picture Base X"]);
CGMZ.Title_CommandWindow.PictureBaseY = Number(CGMZ.Title_CommandWindow.parameters["Picture Base Y"]);
CGMZ.Title_CommandWindow.PictureAddX = Number(CGMZ.Title_CommandWindow.parameters["Picture Add X"]);
CGMZ.Title_CommandWindow.PictureAddY = Number(CGMZ.Title_CommandWindow.parameters["Picture Add Y"]);
CGMZ.Title_CommandWindow.KeepOriginals = (CGMZ.Title_CommandWindow.parameters["Keep Original Commands"] === "true");
CGMZ.Title_CommandWindow.Commands = CGMZ_Utils.parseJSON(CGMZ.Title_CommandWindow.parameters["Commands"], [], "[CGMZ] Title Command Window", "Your Commands parameter was set up incorrectly and could not be read.").map(cmdJSON => {
	const cmd = CGMZ_Utils.parseJSON(cmdJSON, null, "[CGMZ] Title Command Window", "One of your title commands was set up incorrectly and could not be read.");
	if(!cmd) return null;
	return {
		icon: Number(cmd.Icon),
		symbol: cmd["Command Symbol"] || Math.random().toString(36),
		name: cmd["Command Name"],
		js: cmd["JS Command"],
		showJS: cmd["Show JS"],
		enableJS: cmd["Enable JS"],
		backImg: {
			img: cmd["Background Image"],
			x: Number(cmd["Background Image X"]),
			y: Number(cmd["Background Image Y"])
		},
		selectedInfo: {
			icon: Number(cmd["Selected Icon"]) || Number(cmd["Icon"]),
			name: cmd["Selected Text"] || cmd["Command Name"],
			backImg: {
				img: cmd["Selected Back Img"] || cmd["Background Image"],
				x: Number(cmd["Selected Back Img X"]) || Number(cmd["Background Image X"]),
				y: Number(cmd["Selected Back Img Y"]) || Number(cmd["Background Image Y"])
			}
		}
	};
}).filter(x => !!x);
CGMZ.Title_CommandWindow.Pictures = CGMZ_Utils.parseJSON(CGMZ.Title_CommandWindow.parameters["Pictures"], [], "[CGMZ] Title Command Window", "Your Pictures parameter had invalid JSON and could not be read.").map(picJSON => {;
	const pic = CGMZ_Utils.parseJSON(picJSON, null, "[CGMZ] Title Command Window", "One of your title command pictures had invalid JSON and could not be read.")
	if(!pic) return null;
	return {
		symbol: pic["Command Symbol"],
		xOffset: Number(pic["X Offset"]),
		yOffset: Number(pic["Y Offset"]),
		imgActive: CGMZ_Utils.getImageData(pic["Active Image"], "img"),
		imgInactive: CGMZ_Utils.getImageData(pic["Inactive Image"], "img")
	};
}).filter(x => !!x);
//=============================================================================
// Scene Title
//-----------------------------------------------------------------------------
// Handling for command window entries
//=============================================================================
//-----------------------------------------------------------------------------
// Initialize command select
//-----------------------------------------------------------------------------
const alias_CGMZTitleCommandWindow_SceneTitle_initialize = Scene_Title.prototype.initialize;
Scene_Title.prototype.initialize = function() {
	alias_CGMZTitleCommandWindow_SceneTitle_initialize.call(this);
	this._cgmz_currentCommand = "";
	this._cgmz_commandPictures = [];
};
//-----------------------------------------------------------------------------
// Handling for custom Commands added through the plugin
//-----------------------------------------------------------------------------
Scene_Title.prototype.CGMZ_TitleCommand_commandCustom = function() {
	for(const cmd of CGMZ.Title_CommandWindow.Commands) {
		if(this._commandWindow.currentSymbol() === cmd.symbol) {
			try {
				const hookFunc = new Function(cmd.js);
				hookFunc.call(this);
			}
			catch (e) {
				const origin = "[CGMZ] Title Command Window";
				const suggestion = "Check your JavaScript command";
				CGMZ_Utils.reportError(e.message, origin, suggestion);
			}
			break;
		}
	}
};
//-----------------------------------------------------------------------------
// Add additional commands.
//-----------------------------------------------------------------------------
const alias_CGMZ_TitleCommandWindow_createCommandWindow = Scene_Title.prototype.createCommandWindow;
Scene_Title.prototype.createCommandWindow = function() {
	alias_CGMZ_TitleCommandWindow_createCommandWindow.call(this);
	for(const cmd of CGMZ.Title_CommandWindow.Commands) {
		if(this.CGMZ_TitleCommandWindow_isCustomCommand(cmd.symbol)) {
			this._commandWindow.setHandler(cmd.symbol, this.CGMZ_TitleCommand_commandCustom.bind(this));
		}
	}
	if(!CGMZ.Title_CommandWindow.VisibleCommands) {
		this.CGMZ_createCommandPictures();
	}
};
//-----------------------------------------------------------------------------
// Add command pictures
//-----------------------------------------------------------------------------
Scene_Title.prototype.CGMZ_createCommandPictures = function() {
	let order = 0;
	for(const picture of CGMZ.Title_CommandWindow.Pictures) {
		const symbol = picture.symbol;
		for(const cmd of this._commandWindow._list) {
			if(cmd.symbol === symbol) {
				const sprite = new CGMZ_Sprite_TitlePicture(picture, order++, this._commandWindow);
				this._cgmz_commandPictures.push(sprite);
				this.addChild(sprite);
			}
		}
	}
};
//-----------------------------------------------------------------------------
// Determine if command is a custom command in need of custom handler
//-----------------------------------------------------------------------------
Scene_Title.prototype.CGMZ_TitleCommandWindow_isCustomCommand = function(symbol) {
	return (symbol !== 'options' && symbol !== 'continue' && symbol !== 'newGame');
};
//-----------------------------------------------------------------------------
// Change the main command width if not set to use default
//-----------------------------------------------------------------------------
const alias_CGMZ_TitleCommandWindow_mainCommandWidth = Scene_Title.prototype.mainCommandWidth;
Scene_Title.prototype.mainCommandWidth = function() {
	if(CGMZ.Title_CommandWindow.WindowWidth < 0) return alias_CGMZ_TitleCommandWindow_mainCommandWidth.call(this);
	return Graphics.boxWidth * (CGMZ.Title_CommandWindow.WindowWidth / 100.0);
};
//-----------------------------------------------------------------------------
// Change the rectangle height based on number of visible commands
//-----------------------------------------------------------------------------
const alias_CGMZ_TitleCommandWindow_commandWindowRect = Scene_Title.prototype.commandWindowRect;
Scene_Title.prototype.commandWindowRect = function() {
	const rect = alias_CGMZ_TitleCommandWindow_commandWindowRect.call(this);
	rect.height = this.calcWindowHeight(CGMZ.Title_CommandWindow.VisibleCommands, true);
	if(!CGMZ.Title_CommandWindow.VisibleCommands) {
		rect.x = Graphics.width + 100;
	}
	return rect;
};
//-----------------------------------------------------------------------------
// Update command pictures if necessary
//-----------------------------------------------------------------------------
const alias_CGMZTitleCommandWindow_SceneTitle_update = Scene_Title.prototype.update;
Scene_Title.prototype.update = function() {
	alias_CGMZTitleCommandWindow_SceneTitle_update.call(this);
	if(!CGMZ.Title_CommandWindow.VisibleCommands) {
		this.CGMZ_updateCommandPictures();
	}
};
//-----------------------------------------------------------------------------
// Update command pictures
//-----------------------------------------------------------------------------
Scene_Title.prototype.CGMZ_updateCommandPictures = function() {
	const currentCommand = this._commandWindow.currentSymbol();
	if(currentCommand && currentCommand !== this._cgmz_currentCommand) {
		this.CGMZ_unselectPicture(this._cgmz_currentCommand);
		this._cgmz_currentCommand = currentCommand;
		this.CGMZ_selectPicture(this._cgmz_currentCommand);
	}
};
//-----------------------------------------------------------------------------
// Unselect a picture by symbol
//-----------------------------------------------------------------------------
Scene_Title.prototype.CGMZ_unselectPicture = function(symbol) {
	for(const picture of this._cgmz_commandPictures) {
		if(picture._symbol === symbol) {
			picture.setBitmapToInactive();
			break;
		}
	}
};
//-----------------------------------------------------------------------------
// Select a picture by symbol
//-----------------------------------------------------------------------------
Scene_Title.prototype.CGMZ_selectPicture = function(symbol) {
	for(const picture of this._cgmz_commandPictures) {
		if(picture._symbol === symbol) {
			picture.setBitmapToActive();
			break;
		}
	}
};
//=============================================================================
// Window TitleCommand
//-----------------------------------------------------------------------------
// Change commands in the command window
//=============================================================================
//-----------------------------------------------------------------------------
// Add window background preset
//-----------------------------------------------------------------------------
const alias_CGMZ_TitleCommandWindow_initialize = Window_TitleCommand.prototype.initialize;
Window_TitleCommand.prototype.initialize = function(rect) {
	alias_CGMZ_TitleCommandWindow_initialize.call(this, rect);
	if(Imported.CGMZ_WindowSettings && CGMZ.Title_CommandWindow.WindowSettings) this.CGMZ_setWindowSettings(CGMZ.Title_CommandWindow.WindowSettings);
	if(Imported.CGMZ_WindowBackgrounds && CGMZ.Title_CommandWindow.WindowBackground) this.CGMZ_setWindowBackground(CGMZ.Title_CommandWindow.WindowBackground);
};
//-----------------------------------------------------------------------------
// Add original commands in original order if user wishes
//-----------------------------------------------------------------------------
const alias_CGMZ_TitleCommandWindow_makeCommandList = Window_TitleCommand.prototype.makeCommandList;
Window_TitleCommand.prototype.makeCommandList = function() {
	if(CGMZ.Title_CommandWindow.KeepOriginals) {
		alias_CGMZ_TitleCommandWindow_makeCommandList.call(this);
	}
	for(const cmd of CGMZ.Title_CommandWindow.Commands) {
		if(cmd.showJS) {
			const showFunc = new Function(cmd.showJS);
			if(!showFunc.call(this)) continue;
		}
		this.addCommand(cmd.name, cmd.symbol, this.CGMZ_TitleCommandWindow_isCommandEnabled(cmd), {icon: cmd.icon, img: cmd.backImg, selectedInfo: cmd.selectedInfo});
	}
};
//-----------------------------------------------------------------------------
// Check if command should be enabled
//-----------------------------------------------------------------------------
Window_TitleCommand.prototype.CGMZ_TitleCommandWindow_isCommandEnabled = function(command) {
	if(command.enableJS) {
		const func = new Function(command.enableJS);
		if(!func.call(this)) return false;
	}
	if(command.symbol === "continue") {
		return this.isContinueEnabled();
	}
	return true;
};
//-----------------------------------------------------------------------------
// Change number of columns in the window
//-----------------------------------------------------------------------------
Window_TitleCommand.prototype.maxCols = function() {
	return CGMZ.Title_CommandWindow.Columns;
};
//-----------------------------------------------------------------------------
// Change alignment of command text
//-----------------------------------------------------------------------------
Window_TitleCommand.prototype.itemTextAlign = function() {
	return CGMZ.Title_CommandWindow.Alignment;
};
//-----------------------------------------------------------------------------
// Get the command icon
//-----------------------------------------------------------------------------
Window_TitleCommand.prototype.CGMZ_icon = function(index) {
	const ext = this._list[index]?.ext;
	if(!ext) return 0;
	const selected = (this._cgmz_lastIndex === index);
	let icon = ext.icon;
	if(selected && ext.selectedInfo?.icon) icon = ext.selectedInfo.icon;
	return icon;
};
//-----------------------------------------------------------------------------
// Get the command name
//-----------------------------------------------------------------------------
const alias_CGMZ_TitleCommandWindow_TitleCommand_commandName = Window_TitleCommand.prototype.commandName;
Window_TitleCommand.prototype.commandName = function(index) {
	const selected = (this._cgmz_lastIndex === index);
	const ext = this._list[index]?.ext;
	const originalName = alias_CGMZ_TitleCommandWindow_TitleCommand_commandName.call(this, index);
	if(!selected || !ext) return originalName;
    return ext.selectedInfo?.name || originalName;
};
//-----------------------------------------------------------------------------
// Allow use of text codes in command
//-----------------------------------------------------------------------------
Window_TitleCommand.prototype.drawItem = function(index) {
	const rect = this.itemLineRect(index);
	const align = this.itemTextAlign();
	const icon = this.CGMZ_icon(index);
	this.resetTextColor();
	this.changePaintOpacity(this.isCommandEnabled(index));
	if(icon) {
		const iconX = (CGMZ.Title_CommandWindow.IconAlignment === 'left') ? rect.x : rect.x + rect.width - ImageManager.iconWidth;
		this.drawIcon(icon, iconX, rect.y + 2);
		rect.x += (ImageManager.iconWidth + 2) * (CGMZ.Title_CommandWindow.IconAlignment === 'left');
		rect.width -= ImageManager.iconWidth + 2;
	}
	this.CGMZ_drawTextLine(this.commandName(index), rect.x, rect.y, rect.width, align);
};
//-----------------------------------------------------------------------------
// Get selectable cgmz options
//-----------------------------------------------------------------------------
Window_TitleCommand.prototype.CGMZ_getSelectableCGMZOptions = function(index) {
	const ext = this._list[index].ext;
	const selected = (this._cgmz_lastIndex === index);
	if(selected && ext?.selectedInfo?.backImg?.img) {
		const selectBg = {
			img: ext.selectedInfo.backImg.img,
			imgX: ext.selectedInfo.backImg.x,
			imgY: ext.selectedInfo.backImg.y
		}
		return {bg: selectBg};
	}
	if(ext && ext.img && ext.img.img) {
		const bg = {
			img: ext.img.img,
			imgX: ext.img.x,
			imgY: ext.img.y
		}
		return {bg: bg};
	}
	return Window_Command.prototype.CGMZ_getSelectableCGMZOptions.call(this, index);
};
//-----------------------------------------------------------------------------
// Redraw old index
//-----------------------------------------------------------------------------
Window_TitleCommand.prototype.CGMZ_handleSelectionChangePrevious = function(index) {
	if(this._list?.[index]) this.redrawItem(index);
};
//-----------------------------------------------------------------------------
// Redraw new index
//-----------------------------------------------------------------------------
Window_TitleCommand.prototype.CGMZ_handleSelectionChangeNext = function(index) {
	if(this._list?.[index]) this.redrawItem(index);
};
//=============================================================================
// CGMZ_Sprite_TitlePicture
//-----------------------------------------------------------------------------
// Pictures for commands
//=============================================================================
function CGMZ_Sprite_TitlePicture() {
    this.initialize(...arguments);
}
CGMZ_Sprite_TitlePicture.prototype = Object.create(Sprite_Clickable.prototype);
CGMZ_Sprite_TitlePicture.prototype.constructor = CGMZ_Sprite_TitlePicture;
//-----------------------------------------------------------------------------
// Initialize
//-----------------------------------------------------------------------------
CGMZ_Sprite_TitlePicture.prototype.initialize = function(data, order, cmdWindow) {
	Sprite_Clickable.prototype.initialize.call(this);
	this._commandWindow = cmdWindow;
	this._symbol = data.symbol;
	this.anchor.x = 0;
	this.anchor.y = 0;
	const x = this.calculateX(data.xOffset, order);
	const y = this.calculateY(data.yOffset, order);
	this.move(x, y);
	this._inactiveBitmap = ImageManager.loadBitmap(data.imgInactive.folder, data.imgInactive.filename);
	this._activeBitmap = ImageManager.loadBitmap(data.imgActive.folder, data.imgActive.filename);
	this.bitmap = this._inactiveBitmap;
	this.CGMZ_setExact(true);
	this.show();
};
//-----------------------------------------------------------------------------
// Get picture x coordinate
//-----------------------------------------------------------------------------
CGMZ_Sprite_TitlePicture.prototype.calculateX = function(offset, order) {
	const base = CGMZ.Title_CommandWindow.PictureBaseX + (CGMZ.Title_CommandWindow.PictureAddX * order);
	return base + offset;
};
//-----------------------------------------------------------------------------
// Get picture y coordinate
//-----------------------------------------------------------------------------
CGMZ_Sprite_TitlePicture.prototype.calculateY = function(offset, order) {
	const base = CGMZ.Title_CommandWindow.PictureBaseY + (CGMZ.Title_CommandWindow.PictureAddY * order);
	return base + offset;
};
//-----------------------------------------------------------------------------
// Set the bitmap to the active bitmap
//-----------------------------------------------------------------------------
CGMZ_Sprite_TitlePicture.prototype.setBitmapToActive = function() {
    this.bitmap = this._activeBitmap;
};
//-----------------------------------------------------------------------------
// Set the bitmap to the inactive bitmap
//-----------------------------------------------------------------------------
CGMZ_Sprite_TitlePicture.prototype.setBitmapToInactive = function() {
    this.bitmap = this._inactiveBitmap;
};
//-----------------------------------------------------------------------------
// If mouse over, select the pic symbol
//-----------------------------------------------------------------------------
CGMZ_Sprite_TitlePicture.prototype.onMouseEnter = function() {
    this._commandWindow.selectSymbol(this._symbol);
};
//-----------------------------------------------------------------------------
// Call the command associated with the picture when clicked
//-----------------------------------------------------------------------------
CGMZ_Sprite_TitlePicture.prototype.onClick = function() {
	this._commandWindow.selectSymbol(this._symbol);
	this._commandWindow.processOk();
};