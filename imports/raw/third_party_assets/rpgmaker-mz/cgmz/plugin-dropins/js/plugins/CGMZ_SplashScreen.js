/*:
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/splashscreen/
 * @target MZ
 * @base CGMZ_Core
 * @orderAfter CGMZ_Core
 * @plugindesc Creates a splash screen before the title screen
 * @help
 * ============================================================================
 * For terms and conditions using this plugin in your game please visit:
 * https://www.caspergaming.com/terms-of-use/
 * ============================================================================
 * Become a Patron to get access to beta/alpha plugins plus other goodies!
 * https://www.patreon.com/CasperGamingRPGM
 * ============================================================================
 * Version: 1.4.0
 * ----------------------------------------------------------------------------
 * Compatibility: Only tested with my CGMZ plugins.
 * Made for RPG Maker MZ 1.10.0
 * ----------------------------------------------------------------------------
 * Description: Creates a splash screen before the title screen. It can handle
 * multiple splashes, and splashes can be skipped with input.
 * ----------------------------------------------------------------------------
 * Documentation:
 * Fade speed is how much is added/subtracted from opacity each frame during
 * fade.
 * 
 * Images that do not fill the screen completely will be centered.
 *
 * For the sound delay and display time parameters, 60f = 1 second
 *
 * When setting Stop BGM After Splash to false, BGM will continue to play
 * until either splash scene end or another splash is encountered with a
 * different BGM.
 * ---------------------------Integrations-------------------------------------
 * This plugin includes additional functionality when paired with the following
 * [CGMZ] plugins:
 * 
 * [CGMZ] Rumble
 * Rumble the player's controller (if they are using one) during your splash 
 * screens. This can also work on a delay to time it with a sound effect.
 * ---------------------------Saved Games--------------------------------------
 * This plugin fully supports saved games.
 * -------------------------Plugin Commands------------------------------------
 * This plugin does not use any plugin commands.
 * ----------------------------Required Plugin---------------------------------
 * Please note that all [CGMZ] plugins require [CGMZ] Core to be installed
 * above them in the plugin manager. You can download it from my website:
 * https://www.caspergaming.com/plugins/cgmz/core/
 * -----------------------------Filename---------------------------------------
 * The filename of this plugin's JavaScript file MUST be CGMZ_SplashScreen.js
 * This is what it comes as when downloaded. The filename is used to load
 * parameters and execute plugin commands. If you change it, things will begin
 * behaving incorrectly and your game will probably crash. Please do not
 * rename the js file.
 * --------------------------Latest Version------------------------------------
 * Hi all, this latest version adds a [CGMZ] Rumble integration so you can now
 * rumble the player's controller during your splash screens. This can work
 * with a delay, in case you also want to time it with a delayed sound effect.
 *
 * Version 1.4.0
 * - Added [CGMZ] Rumble integration
 *
 * @param Skip In Playtest
 * @type boolean
 * @desc Skip splash screens during test plays?
 * @default false
 * 
 * @param Stop BGM After Splash
 * @type boolean
 * @desc If set to false, BGM will not stop after splash
 * @default true
 *
 * @param Splashes
 * @type struct<Splash>[]
 * @default []
 * @desc Set up splash image/sound properties
*/
/*~struct~Splash:
 * @param Image
 * @type file
 * @dir img/
 * @desc The image to show on the splash screen
 *
 * @param Center Image
 * @type boolean
 * @default true
 * @desc Center the image?
 *
 * @param Scale Image
 * @type boolean
 * @default true
 * @desc Scale the image to fit the screen?
 *
 * @param Video
 * @type file
 * @dir movies/
 * @desc The video to show on the splash screen
 *
 * @param Display Time
 * @type number
 * @min 1
 * @desc Determines amount of time (in frames) splash is shown for
 * @default 360
 * 
 * @param Fade In Time
 * @type number
 * @desc Determines how many frames fade in takes
 * @default 30
 * 
 * @param Fade Out Time
 * @type number
 * @desc Determines how many frames fade out takes
 * @default 30
 * 
 * @param Sound Effect
 * @type struct<SE>
 * @desc SE to play when the splash is shown
 * 
 * @param BGM
 * @type struct<BGM>
 * @desc BGM to play when the splash is shown
 *
 * @param Sound Delay
 * @type number
 * @min 0
 * @default 0
 * @desc The amount of time (in frames) to wait before playing the sound effect
 *
 * @param Unskippable
 * @type boolean
 * @default false
 * @desc If true, the player will not be able to skip this splash with input
 *
 * @param Integrations
 *
 * @param Rumble
 * @parent Integrations
 * @type struct<RumbleSettings>
 * @default {"Duration":"0","Weak Magnitude":"1.00","Strong Magnitude":"1.00","Start Delay":"0"}
 * @desc If set, this splash will rumble the player's controller (requires [CGMZ] Rumble)
 *
 * @param Rumble Delay
 * @parent Integrations
 * @type number
 * @min 0
 * @default 0
 * @desc The amount of time (in frames) to wait before playing the rumble effect
*/
/*~struct~SE:
 * @param Name
 * @type file
 * @dir audio/se
 * @desc The audio file to play
 * 
 * @param Volume
 * @type number
 * @min 0
 * @max 100
 * @default 90
 * @desc The volume of the sound
 * 
 * @param Pitch
 * @type number
 * @min 50
 * @max 150
 * @default 100
 * @desc The pitch of the sound
 * 
 * @param Pan
 * @type number
 * @min -100
 * @max 100
 * @default 0
 * @desc The pan of the sound
*/
/*~struct~BGM:
 * @param Name
 * @type file
 * @dir audio/bgm
 * @desc The audio file to play
 * 
 * @param Volume
 * @type number
 * @min 0
 * @max 100
 * @default 90
 * @desc The volume of the sound
 * 
 * @param Pitch
 * @type number
 * @min 50
 * @max 150
 * @default 100
 * @desc The pitch of the sound
 * 
 * @param Pan
 * @type number
 * @min -100
 * @max 100
 * @default 0
 * @desc The pan of the sound
*/
/*~struct~RumbleSettings:
 * @param Duration
 * @type number
 * @min 0
 * @max 5000
 * @default 0
 * @desc The duration (in ms) of the rumble
 *
 * @param Weak Magnitude
 * @type number
 * @decimals 2
 * @min 0.00
 * @max 1.00
 * @default 1.00
 * @desc The weak magnitude of the rumble
 *
 * @param Strong Magnitude
 * @type number
 * @decimals 2
 * @min 0.00
 * @max 1.00
 * @default 1.00
 * @desc The strong magnitude of the rumble
 *
 * @param Start Delay
 * @type number
 * @min 0
 * @max 4800
 * @default 0
 * @desc The delay (in ms) before the rumble starts
*/
/*:zh-CN
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/splashscreen/
 * @target MZ
 * @base CGMZ_Core
 * @orderAfter CGMZ_Core
 * @plugindesc OP播放系统（在标题画面之前播放图片、音效或音乐）
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
 * 【插件版本】 V 1.4.0
 * ----------------------------------------------------------------------------
 * 【兼容性】仅测试作者所制作的插件
 * 【RM版本】RPG Maker MZ 1.10.0
 * ----------------------------------------------------------------------------
 * 【插件描述】在进入标题画面前播放图片、音效或音乐，可以通过按键跳过。
 * ----------------------------------------------------------------------------
 * 【使用说明】
 * 1、本插件不支持事件的插件指令。
 * 2、切换速度：指图片切换时的淡入淡出效果的速度。
 * 3、少于屏幕尺寸的图片将会居中。
 * 4、声音延迟和图片播放参数, 60帧 = 1秒
 * 5、9/19/2022新增播放BGM功能:
 *    每张图片可以设置播放BGM。图片切换时会自动播放下一组的BGM。
 *    如果需要整个OP过程只播放一首BGM，则设置第一组图片的BGM，其余图片组的BGM留空。
 *    OP结束会自动切换并播放标题画面设置的BGM，如不设置标题BGM则变为无声。
 *
 * ---------------------------Integrations-------------------------------------
 * This plugin includes additional functionality when paired with the following
 * [CGMZ] plugins:
 * 
 * [CGMZ] Rumble
 * Rumble the player's controller (if they are using one) during your splash 
 * screens. This can also work on a delay to time it with a sound effect.
 * ----------------------------------------------------------------------------
 * 【版本历史】
 * Hi all, this latest version adds a [CGMZ] Rumble integration so you can now
 * rumble the player's controller during your splash screens. This can work
 * with a delay, in case you also want to time it with a delayed sound effect.
 *
 * Version 1.4.0
 * - Added [CGMZ] Rumble integration
 *
 * @param Skip In Playtest
 * @type boolean
 * @desc Skip splash screens during test plays?
 * @default false
 * 
 * @param Stop BGM After Splash
 * @text 切换时关闭BGM
 * @type boolean
 * @desc OP播放中切换图片时，自动关闭或继续播放BGM。（如果下一张图片带有BGM，则会自动切换播放新的BGM）
 * @default true
 *
 * @param Splashes
 * @text 图片组
 * @type struct<Splash>[]
 * @default []
 * @desc 设置播放的图片、音效和背景音乐。
*/
/*~struct~Splash:zh-CN
 * @param Image
 * @text 图片
 * @type file
 * @dir img/
 * @desc 设置需要播放的图片。
 *
 * @param Center Image
 * @type boolean
 * @default true
 * @desc Center the image?
 *
 * @param Scale Image
 * @type boolean
 * @default true
 * @desc Scale the image to fit the screen?
 *
 * @param Video
 * @type file
 * @dir movies/
 * @desc The video to show on the splash screen
 *
 * @param Display Time
 * @text 播放时间
 * @type number
 * @min 1
 * @desc 每组图片的播放时间。（60帧=1秒）
 * @default 360
 * 
 * @param Fade In Time
 * @type number
 * @desc Determines how many frames fade in takes
 * @default 30
 * 
 * @param Fade Out Time
 * @type number
 * @desc Determines how many frames fade out takes
 * @default 30
 * 
 * @param Sound Effect
 * @text 音效
 * @type struct<SE>
 * @desc 设置需要播放的音效。
 * 
 * @param BGM
 * @text 背景音乐
 * @type struct<BGM>
 * @desc 设置需要播放的BGM背景音乐。
 *
 * @param Sound Delay
 * @text 声音延迟
 * @type number
 * @min 0
 * @default 0
 * @desc 设置该组图片播放后延迟多少秒播放音效。（60帧=1秒）
 *
 * @param Unskippable
 * @type boolean
 * @default false
 * @desc If true, the player will not be able to skip this splash with input
 *
 * @param Integrations
 *
 * @param Rumble
 * @parent Integrations
 * @type struct<RumbleSettings>
 * @default {"Duration":"0","Weak Magnitude":"1.00","Strong Magnitude":"1.00","Start Delay":"0"}
 * @desc If set, this splash will rumble the player's controller (requires [CGMZ] Rumble)
 *
 * @param Rumble Delay
 * @parent Integrations
 * @type number
 * @min 0
 * @default 0
 * @desc The amount of time (in frames) to wait before playing the rumble effect
*/
/*~struct~SE:zh-CN
 * @param Name
 * @type file
 * @dir audio/se
 * @desc The audio file to play
 * 
 * @param Volume
 * @type number
 * @min 0
 * @max 100
 * @default 90
 * @desc The volume of the sound
 * 
 * @param Pitch
 * @type number
 * @min 50
 * @max 150
 * @default 100
 * @desc The pitch of the sound
 * 
 * @param Pan
 * @type number
 * @min -100
 * @max 100
 * @default 0
 * @desc The pan of the sound
*/
/*~struct~BGM:zh-CN
 * @param Name
 * @type file
 * @dir audio/bgm
 * @desc The audio file to play
 * 
 * @param Volume
 * @type number
 * @min 0
 * @max 100
 * @default 90
 * @desc The volume of the sound
 * 
 * @param Pitch
 * @type number
 * @min 50
 * @max 150
 * @default 100
 * @desc The pitch of the sound
 * 
 * @param Pan
 * @type number
 * @min -100
 * @max 100
 * @default 0
 * @desc The pan of the sound
*/
/*~struct~RumbleSettings:zh-CN
 * @param Duration
 * @type number
 * @min 0
 * @max 5000
 * @default 0
 * @desc The duration (in ms) of the rumble
 *
 * @param Weak Magnitude
 * @type number
 * @decimals 2
 * @min 0.00
 * @max 1.00
 * @default 1.00
 * @desc The weak magnitude of the rumble
 *
 * @param Strong Magnitude
 * @type number
 * @decimals 2
 * @min 0.00
 * @max 1.00
 * @default 1.00
 * @desc The strong magnitude of the rumble
 *
 * @param Start Delay
 * @type number
 * @min 0
 * @max 4800
 * @default 0
 * @desc The delay (in ms) before the rumble starts
*/
/*:es
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/splashscreen/
 * @target MZ
 * @base CGMZ_Core
 * @orderAfter CGMZ_Core
 * @plugindesc Crea una pantalla de inicio antes de la pantalla de título
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
 * Versión: 1.4.0
 * ----------------------------------------------------------------------------
 * Compatibilidad: Sólo probado con mis CGMZ plugins.
 * Hecho para RPG Maker MZ 1.10.0
 * ----------------------------------------------------------------------------
 * Descripción: Crea una pantalla de inicio antes de la pantalla de título. 
 * Puede manejar multiple pantallas de bienvenida, y estas se pueden omitir
 * con la entrada.
 * ----------------------------------------------------------------------------
 * Documentacion:
 * La velocidad de desvanecimiento es cuánto se suma/resta de la opacidad de 
 * cada cuadro durante el desvanecimiento.
 * 
 * Las imágenes que no llenen completamente la pantalla se centrarán.
 *
 * Para los parámetros de retraso de sonido y tiempo de visualización, 
 * 60f = 1 segundo
 *
 * Al configurar un paro de sonido de fondo después de un splash/grupo de 
 * imágenes en falso, el sonido continuará reproduciéndose hasta que finalice
 * la escena del splash o grupo de imágenes o se encuentre otro splash con
 * música de fondo diferente.
 * ---------------------------Integrations-------------------------------------
 * This plugin includes additional functionality when paired with the following
 * [CGMZ] plugins:
 * 
 * [CGMZ] Rumble
 * Rumble the player's controller (if they are using one) during your splash 
 * screens. This can also work on a delay to time it with a sound effect.
 * --------------------------Juegos guardados----------------------------------
 * Este plugin es totalmente compatible con los juegos guardados.
 * --------------------------Comandos de Plugin--------------------------------
 * Este plugin no admite comandos de complemento.
 * -------------------------Nombre del archivo---------------------------------
 * El nombre de archivo JavaScript de este plugin DEBE ser CGMZ_SplashScreen.js
 * Esto es lo que se obtiene cuando se descarga. El nombre de archivo se 
 * utiliza para cargar parámetros y ejecutar comandos de plugin. Si lo cambias,
 * las cosas comenzarán comportarse incorrectamente y tu juego probablemente
 * falle. Por favor no renombrar este archivo js.
 * --------------------------Latest Version------------------------------------
 * Hi all, this latest version adds a [CGMZ] Rumble integration so you can now
 * rumble the player's controller during your splash screens. This can work
 * with a delay, in case you also want to time it with a delayed sound effect.
 *
 * Version 1.4.0
 * - Added [CGMZ] Rumble integration
 *
 * @param Skip In Playtest
 * @type boolean
 * @desc Skip splash screens during test plays?
 * @default false
 * 
 * @param Stop BGM After Splash
 * @text Detener BGM después de Splash/Imágenes
 * @type boolean
 * @desc Si se establece en falso, el BGM no se detendrá después del splash/grupo de imágenes.
 * @default true
 *
 * @param Splashes
 * @text Grupos de Imágenes
 * @type struct<Splash>[]
 * @default []
 * @desc Configurar propiedades de imagen/sonido de bienvenida.
*/
/*~struct~Splash:es
 * @param Image
 * @text Imagen
 * @type file
 * @dir img/
 * @desc La imagen que se mostrará en la pantalla de inicio.
 *
 * @param Center Image
 * @type boolean
 * @default true
 * @desc Center the image?
 *
 * @param Scale Image
 * @type boolean
 * @default true
 * @desc Scale the image to fit the screen?
 *
 * @param Video
 * @type file
 * @dir movies/
 * @desc The video to show on the splash screen
 *
 * @param Display Time
 * @text Tiempo de visualización
 * @type number
 * @min 1
 * @desc Determina la cantidad de tiempo (en fotogramas) durante la que se muestra cada splash/grupo de imágenes.
 * @default 360
 * 
 * @param Fade In Time
 * @type number
 * @desc Determines how many frames fade in takes
 * @default 30
 * 
 * @param Fade Out Time
 * @type number
 * @desc Determines how many frames fade out takes
 * @default 30
 * 
 * @param Sound Effect
 * @text Efecto de sonido
 * @type struct<SE>
 * @desc Sonido para reproducir cuando se muestra el splash/grupo de imágenes.
 * 
 * @param BGM
 * @text Background music/Sonido de fondo
 * @type struct<BGM>
 * @desc Sonido para reproducir cuando se muestra el splash/grupo de imágenes.
 *
 * @param Sound Delay
 * @text Retraso de sonido
 * @type number
 * @min 0
 * @default 0
 * @desc La cantidad de tiempo a esperar antes de reproducir el efecto de sonido
 *
 * @param Unskippable
 * @type boolean
 * @default false
 * @desc If true, the player will not be able to skip this splash with input
 *
 * @param Integrations
 *
 * @param Rumble
 * @parent Integrations
 * @type struct<RumbleSettings>
 * @default {"Duration":"0","Weak Magnitude":"1.00","Strong Magnitude":"1.00","Start Delay":"0"}
 * @desc If set, this splash will rumble the player's controller (requires [CGMZ] Rumble)
 *
 * @param Rumble Delay
 * @parent Integrations
 * @type number
 * @min 0
 * @default 0
 * @desc The amount of time (in frames) to wait before playing the rumble effect
*/
/*~struct~SE:es
 * @param Name
 * @type file
 * @dir audio/se
 * @desc The audio file to play
 * 
 * @param Volume
 * @type number
 * @min 0
 * @max 100
 * @default 90
 * @desc The volume of the sound
 * 
 * @param Pitch
 * @type number
 * @min 50
 * @max 150
 * @default 100
 * @desc The pitch of the sound
 * 
 * @param Pan
 * @type number
 * @min -100
 * @max 100
 * @default 0
 * @desc The pan of the sound
*/
/*~struct~BGM:es
 * @param Name
 * @type file
 * @dir audio/bgm
 * @desc The audio file to play
 * 
 * @param Volume
 * @type number
 * @min 0
 * @max 100
 * @default 90
 * @desc The volume of the sound
 * 
 * @param Pitch
 * @type number
 * @min 50
 * @max 150
 * @default 100
 * @desc The pitch of the sound
 * 
 * @param Pan
 * @type number
 * @min -100
 * @max 100
 * @default 0
 * @desc The pan of the sound
*/
/*~struct~RumbleSettings:es
 * @param Duration
 * @type number
 * @min 0
 * @max 5000
 * @default 0
 * @desc The duration (in ms) of the rumble
 *
 * @param Weak Magnitude
 * @type number
 * @decimals 2
 * @min 0.00
 * @max 1.00
 * @default 1.00
 * @desc The weak magnitude of the rumble
 *
 * @param Strong Magnitude
 * @type number
 * @decimals 2
 * @min 0.00
 * @max 1.00
 * @default 1.00
 * @desc The strong magnitude of the rumble
 *
 * @param Start Delay
 * @type number
 * @min 0
 * @max 4800
 * @default 0
 * @desc The delay (in ms) before the rumble starts
*/
Imported.CGMZ_SplashScreen = true;
CGMZ.Versions["Splash Screen"] = "1.4.0";
CGMZ.SplashScreen = {};
CGMZ.SplashScreen.parameters = PluginManager.parameters('CGMZ_SplashScreen');
CGMZ.SplashScreen.StopBGMs = (CGMZ.SplashScreen.parameters["Stop BGM After Splash"] === 'true');
CGMZ.SplashScreen.SkipInPlaytest = (CGMZ.SplashScreen.parameters["Skip In Playtest"] === 'true');
CGMZ.SplashScreen.Splashes = CGMZ_Utils.parseJSON(CGMZ.SplashScreen.parameters["Splashes"], [], "[CGMZ] Splash Screen", "Your Splashes parameter was set up incorrectly and could not be read.");
//=============================================================================
// CGMZ_Splash
//-----------------------------------------------------------------------------
// Object which stores splash data
//=============================================================================
function CGMZ_Splash() {
	this.initialize(...arguments);
}
//-----------------------------------------------------------------------------
// Initialize
//-----------------------------------------------------------------------------
CGMZ_Splash.prototype.initialize = function(splash) {
	this.img = splash.Image;
	this.video = splash.Video;
	if(splash["Sound Effect"]) this.se = CGMZ_Utils.parseSoundEffectJSON(splash["Sound Effect"], "[CGMZ] Splash Screen");
	if(splash.BGM) this.bgm = CGMZ_Utils.parseSoundEffectJSON(splash.BGM, "[CGMZ] Splash Screen");
	this.centerImage = (splash["Center Image"] === 'true');
	this.scaleImage = (splash["Scale Image"] === 'true');
	this.fadeInDuration = Number(splash["Fade In Time"]);
	this.fadeOutDuration = Number(splash["Fade Out Time"]);
	this.displayTime = Number(splash["Display Time"]);
	this.soundDelay = Number(splash["Sound Delay"]);
	this.unskippable = (splash.Unskippable === 'true');
	this.rumble = CGMZ_Utils.parseRumbleJSON(splash.Rumble, '[CGMZ] Splash Screen');
	this.rumble.forceRumble = true;
	this.rumbleDelay = Number(splash["Rumble Delay"]);
};
//-----------------------------------------------------------------------------
// Determine if this splash has an se
//-----------------------------------------------------------------------------
CGMZ_Splash.prototype.hasSound = function() {
	return !!(this.se?.name);
};
//-----------------------------------------------------------------------------
// Determine if this splash has a bgm
//-----------------------------------------------------------------------------
CGMZ_Splash.prototype.hasBGM = function() {
	return !!(this.bgm?.name);
};
//-----------------------------------------------------------------------------
// Determine if this splash is an image splash
//-----------------------------------------------------------------------------
CGMZ_Splash.prototype.isImageSplash = function() {
	return !!this.img;
};
//=============================================================================
// Scene_Splash
//-----------------------------------------------------------------------------
// Take over default splash scene
//=============================================================================
//-----------------------------------------------------------------------------
// Initialize
//-----------------------------------------------------------------------------
const alias_CGMZSplashScreen_SceneSplash_initialize = Scene_Splash.prototype.initialize;
Scene_Splash.prototype.initialize = function() {
	alias_CGMZSplashScreen_SceneSplash_initialize.call(this);
	this._phase = 'loading';
	this._index = 0;
	this._soundPlayed = false;
	this._rumblePlayed = false;
	this._splashes = this.CGMZ_initSplashes();
};
//-----------------------------------------------------------------------------
// Initialize splash objects
//-----------------------------------------------------------------------------
Scene_Splash.prototype.CGMZ_initSplashes = function() {
	const splashes = [];
	for(const splashJSON of CGMZ.SplashScreen.Splashes) {
		const splash = CGMZ_Utils.parseJSON(splashJSON, null, "[CGMZ] Splash Screen", "One of your splashes was set up incorrectly and could not be read.");
		if(!splash) return;
		splashes.push(new CGMZ_Splash(splash));
	}
	return splashes;
};
//-----------------------------------------------------------------------------
// Overwrite this to not try loading default bitmap - other default behavior is kept
//-----------------------------------------------------------------------------
Scene_Splash.prototype.createBackground = function() {
	this._backSprite = new Sprite();
	this.addChild(this._backSprite);
};
//-----------------------------------------------------------------------------
// Overwrite this since we need to wait for splash to load
//-----------------------------------------------------------------------------
Scene_Splash.prototype.start = function() {
	Scene_Base.prototype.start.call(this);
	if((CGMZ.SplashScreen.SkipInPlaytest && $gameTemp.isPlaytest()) || this._splashes.length < 1) {
		this.gotoTitle();
	} else {
		this.CGMZ_loadSplash();
	}
};
//-----------------------------------------------------------------------------
// Overwrite this to prevent the second fade out
//-----------------------------------------------------------------------------
Scene_Splash.prototype.stop = function() {
	Scene_Base.prototype.stop.call(this);
};
//-----------------------------------------------------------------------------
// If using this plugin, assume the user always has a splash screen
//-----------------------------------------------------------------------------
Scene_Splash.prototype.isEnabled = function() {
	return true;
};
//-----------------------------------------------------------------------------
// Get the splash at the current index
//-----------------------------------------------------------------------------
Scene_Splash.prototype.CGMZ_getSplash = function() {
	return this._splashes[this._index];
};
//-----------------------------------------------------------------------------
// Load the splash
//-----------------------------------------------------------------------------
Scene_Splash.prototype.CGMZ_loadSplash = function() {
	const splash = this.CGMZ_getSplash();
	this._backSprite.scale.x = 1;
	this._backSprite.scale.y = 1;
	this._backSprite.anchor.x = 0;
	this._backSprite.anchor.y = 0;
	this._backSprite.x = 0;
	this._backSprite.y = 0;
	if(splash.isImageSplash()) {
		const imgData = CGMZ_Utils.getImageData(splash.img, "img");
		const bitmap = ImageManager.loadBitmap(imgData.folder, imgData.filename);
		bitmap.addLoadListener(this.CGMZ_onSplashLoaded.bind(this, bitmap, splash.centerImage, splash.scaleImage));
	} else {
		const ext = this.videoFileExt();
		CGMZ_Video.play(splash.video);
		this._backSprite.bitmap = new Bitmap(Graphics.width, Graphics.height);
		this.adjustBackground();
	}
	this._phase = 'loading';
};
//-----------------------------------------------------------------------------
// Get video file ext
//-----------------------------------------------------------------------------
Scene_Splash.prototype.videoFileExt = function() {
	return (Utils.canPlayWebm()) ? ".webm" : ".mp4";
};
//-----------------------------------------------------------------------------
// When splash has been loaded
//-----------------------------------------------------------------------------
Scene_Splash.prototype.CGMZ_onSplashLoaded = function(bitmap, center, scale) {
	this._backSprite.bitmap = bitmap;
	if(center) this.centerSprite(this._backSprite);
	if(scale) this.scaleSprite(this._backSprite);
	this._phase = 'fadeIn';
	this.CGMZ_startFadeIn();
};
//-----------------------------------------------------------------------------
// Start fade in
//-----------------------------------------------------------------------------
Scene_Splash.prototype.CGMZ_startFadeIn = function() {
	const splash = this.CGMZ_getSplash();
	this.startFadeIn(splash.fadeInDuration, false);
};
//-----------------------------------------------------------------------------
// Start fade out
//-----------------------------------------------------------------------------
Scene_Splash.prototype.CGMZ_startFadeOut = function() {
	const splash = this.CGMZ_getSplash();
	this.startFadeOut(splash.fadeOutDuration, false);
};
//-----------------------------------------------------------------------------
// Change image bitmap to next image
//-----------------------------------------------------------------------------
Scene_Splash.prototype.CGMZ_nextSplash = function() {
	const splash = this.CGMZ_getSplash();
	if(!splash.isImageSplash()) CGMZ_Video.pause();
	this._index++;
	this.CGMZ_setNewSplash();
};
//-----------------------------------------------------------------------------
// Set the next splash
//-----------------------------------------------------------------------------
Scene_Splash.prototype.CGMZ_setNewSplash = function() {
	if(this._index >= this._splashes.length) {
		this._phase = 'exit';
		this.gotoTitle();
	} else {
		const splash = this.CGMZ_getSplash();
		this._soundPlayed = false;
		this._rumblePlayed = false;
		AudioManager.stopSe();
		if(CGMZ.SplashScreen.StopBGMs) AudioManager.stopBgm();
		this.CGMZ_loadSplash();
	}
};
//-----------------------------------------------------------------------------
// Update
//-----------------------------------------------------------------------------
const alias_CGMZSplashScreen_SceneSplash_update = Scene_Splash.prototype.update;
Scene_Splash.prototype.update = function() {
	alias_CGMZSplashScreen_SceneSplash_update.call(this);
	switch(this._phase) {
		case 'loading': this.CGMZ_updateLoad(); break;
		case 'fadeIn': this.CGMZ_updateFadeIn(); break;
		case 'display': this.CGMZ_updateDisplay(); break;
		case 'fadeOut': this.CGMZ_updateFadeOut();  break;
	}
};
//-----------------------------------------------------------------------------
// Handle checking skip
//-----------------------------------------------------------------------------
const alias_CGMZSplashScreen_SceneSplash_checkSkip = Scene_Splash.prototype.checkSkip;
Scene_Splash.prototype.checkSkip = function() {
	const splash = this.CGMZ_getSplash();
	if(!splash) return;
	if((Input.isTriggered('ok') || TouchInput.isPressed()) && !splash.unskippable) {
		alias_CGMZSplashScreen_SceneSplash_checkSkip.call(this);
		this.phase = 'fadeOut';
	}
};
//-----------------------------------------------------------------------------
// Always return true to make wait count no longer move to next scene
//-----------------------------------------------------------------------------
const alias_CGMZSplashScreen_SceneSplash_updateWaitCount = Scene_Splash.prototype.updateWaitCount;
Scene_Splash.prototype.updateWaitCount = function() {
	alias_CGMZSplashScreen_SceneSplash_updateWaitCount.call(this);
	return true;
};
//-----------------------------------------------------------------------------
// Update during load phase
//-----------------------------------------------------------------------------
Scene_Splash.prototype.CGMZ_updateLoad = function() {
	const splash = this.CGMZ_getSplash();
	if(!splash.isImageSplash()) {
		if(!CGMZ_Video._loading) {
			this._phase = 'fadeIn';
			this.CGMZ_startFadeIn();
		}
	}
};
//-----------------------------------------------------------------------------
// Update during load phase
//-----------------------------------------------------------------------------
Scene_Splash.prototype.CGMZ_updateFadeIn = function() {
	const splash = this.CGMZ_getSplash();
	if(!splash.isImageSplash()) {
		const video = CGMZ_Video._element;
		this._backSprite.bitmap.clear();
		this._backSprite.bitmap.context.drawImage(video, 0, 0, Graphics.width, Graphics.height);
		this._backSprite.bitmap._baseTexture.update();
	}
	if(this._fadeDuration <= 0) {
		this._waitCount = splash.displayTime;
		this._phase = 'display';
	}
};
//-----------------------------------------------------------------------------
// Update during display phase
//-----------------------------------------------------------------------------
Scene_Splash.prototype.CGMZ_updateDisplay = function() {
	this.CGMZ_updateAudio();
	if(Imported.CGMZ_Rumble) this.CGMZ_updateRumble();
	const splash = this.CGMZ_getSplash();
	if(!splash.isImageSplash()) {
		const video = CGMZ_Video._element;
		this._backSprite.bitmap.clear();
		this._backSprite.bitmap.context.drawImage(video, 0, 0, Graphics.width, Graphics.height);
		this._backSprite.bitmap._baseTexture.update();
	}
	if(this._waitCount <= 0) {
		this.CGMZ_startFadeOut();
		this._phase = 'fadeOut';
		this._waitCount = 0;
	}
};
//-----------------------------------------------------------------------------
// Update during fade out phase
//-----------------------------------------------------------------------------
Scene_Splash.prototype.CGMZ_updateFadeOut = function() {
	if(this._fadeDuration <= 0) {
		this.CGMZ_nextSplash();
		this._phase = 'loading';
	}
};
//-----------------------------------------------------------------------------
// Update audio playing
//-----------------------------------------------------------------------------
Scene_Splash.prototype.CGMZ_updateAudio = function() {
	const splash = this.CGMZ_getSplash();
	if(splash && !this._soundPlayed && splash.displayTime - this._waitCount > splash.soundDelay) {
		this._soundPlayed = true;
		if(splash.hasSound()) AudioManager.playSe(splash.se);
		if(splash.hasBGM()) AudioManager.playBgm(splash.bgm, 0);
	}
};
//-----------------------------------------------------------------------------
// Update rumble effect
//-----------------------------------------------------------------------------
Scene_Splash.prototype.CGMZ_updateRumble = function() {
	const splash = this.CGMZ_getSplash();
	if(splash && !this._rumblePlayed && splash.displayTime - this._waitCount > splash.rumbleDelay) {
		this._rumblePlayed = true;
		$cgmzTemp.startRumble(splash.rumble);
	}
};