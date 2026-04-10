/*:
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/pixifilters/
 * @target MZ
 * @base CGMZ_Core
 * @orderAfter CGMZ_Core
 * @plugindesc Adds additional PIXI filters to RPG Maker
 * @help
 * ============================================================================
 * For terms and conditions using this plugin in your game please visit:
 * https://www.caspergaming.com/terms-of-use/
 * ============================================================================
 * Become a Patron to get access to beta/alpha plugins plus other goodies!
 * https://www.patreon.com/CasperGamingRPGM
 * ============================================================================
 * Version: Alpha R6
 * ----------------------------------------------------------------------------
 * Compatibility: Only tested with my CGMZ plugins.
 * Made for RPG Maker MZ 1.9.0
 * ----------------------------------------------------------------------------
 * Description: This plugin adds additional PIXI filters to your game, such as
 * godrays, crt, glitch, and many more. Plugin commands are provided to use
 * these filters as is, or they may be used by other plugins.
 * ----------------------------------------------------------------------------
 * Documentation:
 * You can find documentation and a demo on the additional filters at:
 * https://filters.pixijs.download/main/docs/index.html
 *
 * These filters are licensed under MIT license and not made by Casper Gaming.
 * [CGMZ] Pixi Filters only provides a user friendly way to access these 
 * filters within RPG Maker since they are not built-in for RPG Maker.
 *
 * To install the filters, please also import the included pixi-filters.js
 * into your plugin manager. The filters included are v5.3.0. If you run into
 * issues with a filter, you can try downgrading to an older version of the
 * filters.
 * --------------------------Finding Filter------------------------------------
 * Because there are a lot of filters added by this plugin, they have been
 * organized in alphabetical order where possible. If you want to add a Godray
 * Filter, look for the plugin command in the filters starting with G.
 * --------------------------Filter Target-------------------------------------
 * The filter targets are explained below:
 * 
 * Scene - This target will affect the entire scene, which means it should
 * cover everything on screen
 *
 * Spriteset - This target affects the spriteset (map/battle scenes). This
 * will target everything added to the spriteset such as pictures or the timer.
 * This will not affect windows.
 *
 * Base Sprite - This only targets the spriteset's base sprite, which is most
 * things but will not affect pictures, the timer, windows, etc.
 *
 * Performance Note: Using too many filters may lead to drops in FPS.
 * ------------------------------Scenes----------------------------------------
 * If using "All" it will affect all scenes. You should not add other scenes
 * manually to this parameter when using All.
 *
 * If not using All, you can select which scenes you want the filter to affect.
 * Please be careful because some scenes do not have things like a spriteset,
 * in which case if you select to target the spriteset the filter will not
 * have any effect.
 *
 * To select custom scenes, you can switch to the Text tab at the top of the
 * parameter and manually type in a custom scene's constructor. You would need
 * to get that from the custom scene's plugin author or by reading the plugin
 * code. Alternatively, some plugins may integrate with this one to add a
 * filter using a preset setting by calling the id. Filter presets can be
 * retrieved using the following JS:
 * 
 * $cgmzTemp.getPixiFilterSettings(id);
 * -----------------------------Filters----------------------------------------
 * Currently this plugin supports the following filters:
 * - Adjustment Filter
 * - Advanced Bloom Filter
 * - Alpha Filter
 * - Ascii Filter
 * - Bloom Filter
 * - Blur Filter
 * - Bulge Pinch Filter
 * - Color Replace Filter
 * - CrossHatch Filter
 * - CRT Filter
 * - Displacement Filter
 * - Dot Filter
 * - Emboss Filter
 * - Glitch Filter
 * - Godray Filter
 * - Grayscale Filter
 * - Kawase Blur Filter
 * - Noise Filter
 * - Old Film Filter
 * - Pixelate Filter
 *
 * Additional filters will be added over time
 * ---------------------Displacement Filter Note-------------------------------
 * This plugin does support multiple displacement filters, however, you should
 * use a unique sprite for the displacement map image for each filter. You also
 * should not edit the displacement map of the filter in the Edit Filter
 * plugin command.
 * ---------------------------Saved Games--------------------------------------
 * This plugin fully supports saved games.
 * ✓ You should be able to add this  plugin to a saved game and add new mail
 * ✓ You can modify parameters and it will reflect accurately in game
 * ✓ You can remove this plugin with no issue to save data
 * -----------------------------Filename---------------------------------------
 * The filename of this plugin's JS file MUST be CGMZ_PixiFilters.js
 * This is what it comes as when downloaded. The filename is used to load
 * parameters and execute plugin commands. If you change it, things will begin
 * behaving incorrectly and your game will probably crash. Please do not
 * rename the js file.
 * --------------------------Latest Version------------------------------------
 * Hi all, this latest version adds a new filter: the displacement filter!
 *
 * Displacement filter - This filter uses the pixel values from a displacement
 * map sprite to displace its filter target. You can use this for a lot of
 * warping effects since it depends on your dispalcement map what effect you
 * will get. Included is a difference cloud texture that can cause a watery
 * effect.
 *
 * This update also added a plugin command to remove all active filters. This
 * is a convenience plugin command in case you use a lot of filters and have a
 * need to just remove all of them at once.
 *
 * Some bugs were fixed. The glitch filter now has its seed setting back, which
 * was causing the non-animated glitch filter to not work right. The filters
 * will now be automatically removed when returning to title from the game end
 * screen.
 * 
 * Version Alpha R6
 * - Added Displacement Filter
 * - Added Remove All Filters plugin command
 * - Fix bug with glitch filter not having a seed plugin parameter
 * - Fix bug with filters staying after returning to title
 *
 * @command Remove Filter
 * @desc Remove an existing filter (used for all filter types)
 *
 * @arg Filter Id
 * @desc The id of the filter to remove
 *
 * @command Remove All Filters
 * @desc Removes all existing filter
 *
 * @command Create Adjustment Filter
 * @desc Creates a new Adjustment filter
 *
 * @arg Filter Id
 * @desc The id used to refer to this filter later, for editing/removal
 *
 * @arg Settings
 * @type struct<Adjustment>
 * @desc Set up filter settings here. If using preset, fill in settings id from plugin params.
 *
 * @arg Targeting Options
 * @type struct<Targeting>
 * @desc Set up what the filter affects here.
 *
 * @command Edit Adjustment Filter
 * @desc Edits an existing Adjustment filter
 *
 * @arg Filter Id
 * @desc The id used to refer to this filter later, for editing/removal
 *
 * @arg Settings
 * @type struct<Adjustment>
 * @desc Set up filter settings here. If using preset, fill in settings id from plugin params.
 *
 * @command Create Advanced Bloom Filter
 * @desc Creates a new Advanced Bloom filter
 *
 * @arg Filter Id
 * @desc The id used to refer to this filter later, for editing/removal
 *
 * @arg Settings
 * @type struct<AdvancedBloom>
 * @desc Set up filter settings here. If using preset, fill in settings id from plugin params.
 *
 * @arg Targeting Options
 * @type struct<Targeting>
 * @desc Set up what the filter affects here.
 *
 * @command Edit Advanced Bloom Filter
 * @desc Edits an existing Advanced Bloom filter
 *
 * @arg Filter Id
 * @desc The id used to refer to this filter later, for editing/removal
 *
 * @arg Settings
 * @type struct<AdvancedBloom>
 * @desc Set up filter settings here. If using preset, fill in settings id from plugin params.
 *
 * @command Create Alpha Filter
 * @desc Creates a new Alpha filter
 *
 * @arg Filter Id
 * @desc The id used to refer to this filter later, for editing/removal
 *
 * @arg Settings
 * @type struct<Alpha>
 * @desc Set up filter settings here. If using preset, fill in settings id from plugin params.
 *
 * @arg Targeting Options
 * @type struct<Targeting>
 * @desc Set up what the filter affects here.
 *
 * @command Edit Alpha Filter
 * @desc Edits an existing Alpha filter
 *
 * @arg Filter Id
 * @desc The id used to refer to this filter later, for editing/removal
 *
 * @arg Settings
 * @type struct<Alpha>
 * @desc Set up filter settings here. If using preset, fill in settings id from plugin params.
 *
 * @command Create Ascii Filter
 * @desc Creates a new Ascii filter
 *
 * @arg Filter Id
 * @desc The id used to refer to this filter later, for editing/removal
 *
 * @arg Settings
 * @type struct<Ascii>
 * @desc Set up filter settings here. If using preset, fill in settings id from plugin params.
 *
 * @arg Targeting Options
 * @type struct<Targeting>
 * @desc Set up what the filter affects here.
 *
 * @command Edit Ascii Filter
 * @desc Edits an existing Ascii filter
 *
 * @arg Filter Id
 * @desc The id used to refer to this filter later, for editing/removal
 *
 * @arg Settings
 * @type struct<Ascii>
 * @desc Set up filter settings here. If using preset, fill in settings id from plugin params.
 *
 * @command Create Bloom Filter
 * @desc Creates a new Bloom filter
 *
 * @arg Filter Id
 * @desc The id used to refer to this filter later, for editing/removal
 *
 * @arg Settings
 * @type struct<Bloom>
 * @desc Set up filter settings here. If using preset, fill in settings id from plugin params.
 *
 * @arg Targeting Options
 * @type struct<Targeting>
 * @desc Set up what the filter affects here.
 *
 * @command Edit Bloom Filter
 * @desc Edits an existing Bloom filter
 *
 * @arg Filter Id
 * @desc The id used to refer to this filter later, for editing/removal
 *
 * @arg Settings
 * @type struct<Bloom>
 * @desc Set up filter settings here. If using preset, fill in settings id from plugin params.
 *
 * @command Create Blur Filter
 * @desc Creates a new Blur filter
 *
 * @arg Filter Id
 * @desc The id used to refer to this filter later, for editing/removal
 *
 * @arg Settings
 * @type struct<Blur>
 * @desc Set up filter settings here. If using preset, fill in settings id from plugin params.
 *
 * @arg Targeting Options
 * @type struct<Targeting>
 * @desc Set up what the filter affects here.
 *
 * @command Edit Blur Filter
 * @desc Edits an existing Blur filter
 *
 * @arg Filter Id
 * @desc The id used to refer to this filter later, for editing/removal
 *
 * @arg Settings
 * @type struct<Blur>
 * @desc Set up filter settings here. If using preset, fill in settings id from plugin params.
 *
 * @command Create Bulge Pinch Filter
 * @desc Creates a new Bulge Pinch filter
 *
 * @arg Filter Id
 * @desc The id used to refer to this filter later, for editing/removal
 *
 * @arg Settings
 * @type struct<BulgePinch>
 * @desc Set up filter settings here. If using preset, fill in settings id from plugin params.
 *
 * @arg Targeting Options
 * @type struct<Targeting>
 * @desc Set up what the filter affects here.
 *
 * @command Edit Bulge Pinch Filter
 * @desc Edits an existing Bulge Pinch filter
 *
 * @arg Filter Id
 * @desc The id used to refer to this filter later, for editing/removal
 *
 * @arg Settings
 * @type struct<BulgePinch>
 * @desc Set up filter settings here. If using preset, fill in settings id from plugin params.
 *
 * @command Create Color Replace Filter
 * @desc Creates a new Color Replace filter
 *
 * @arg Filter Id
 * @desc The id used to refer to this filter later, for editing/removal
 *
 * @arg Settings
 * @type struct<ColorReplace>
 * @desc Set up filter settings here. If using preset, fill in settings id from plugin params.
 *
 * @arg Targeting Options
 * @type struct<Targeting>
 * @desc Set up what the filter affects here.
 *
 * @command Edit Color Replace Filter
 * @desc Edits an existing Color Replace filter
 *
 * @arg Filter Id
 * @desc The id used to refer to this filter later, for editing/removal
 *
 * @arg Settings
 * @type struct<ColorReplace>
 * @desc Set up filter settings here. If using preset, fill in settings id from plugin params.
 *
 * @command Create CrossHatch Filter
 * @desc Creates a new CrossHatch filter
 *
 * @arg Filter Id
 * @desc The id used to refer to this filter later, for editing/removal
 *
 * @arg Targeting Options
 * @type struct<Targeting>
 * @desc Set up what the filter affects here.
 * 
 * @command Create CRT Filter
 * @desc Creates a new CRT filter
 *
 * @arg Filter Id
 * @desc The id used to refer to this filter later, for editing/removal
 *
 * @arg Settings
 * @type struct<CRT>
 * @desc Set up filter settings here. If using preset, fill in settings id from plugin params.
 *
 * @arg Targeting Options
 * @type struct<Targeting>
 * @desc Set up what the filter affects here.
 *
 * @command Edit CRT Filter
 * @desc Edit an existing CRT filter by identifier
 *
 * @arg Filter Id
 * @desc The id of the filter to edit
 *
 * @arg Settings
 * @type struct<CRT>
 * @desc Set up filter settings here. If using preset, fill in settings id from plugin params.
 *
 * @command Create Displacement Filter
 * @desc Creates a new Displacement filter
 *
 * @arg Filter Id
 * @desc The id used to refer to this filter later, for editing/removal
 *
 * @arg Settings
 * @type struct<Displacement>
 * @desc Set up filter settings here. If using preset, fill in settings id from plugin params.
 *
 * @arg Targeting Options
 * @type struct<Targeting>
 * @desc Set up what the filter affects here.
 *
 * @command Edit Displacement Filter
 * @desc Edits an existing Displacement filter
 *
 * @arg Filter Id
 * @desc The id used to refer to this filter later, for editing/removal
 *
 * @arg Settings
 * @type struct<Displacement>
 * @desc Set up filter settings here. If using preset, fill in settings id from plugin params.
 *
 * @command Create Dot Filter
 * @desc Creates a new Dot filter
 *
 * @arg Filter Id
 * @desc The id used to refer to this filter later, for editing/removal
 *
 * @arg Settings
 * @type struct<Dot>
 * @desc Set up filter settings here. If using preset, fill in settings id from plugin params.
 *
 * @arg Targeting Options
 * @type struct<Targeting>
 * @desc Set up what the filter affects here.
 *
 * @command Edit Dot Filter
 * @desc Edits an existing Dot filter
 *
 * @arg Filter Id
 * @desc The id used to refer to this filter later, for editing/removal
 *
 * @arg Settings
 * @type struct<Dot>
 * @desc Set up filter settings here. If using preset, fill in settings id from plugin params.
 *
 * @command Create Emboss Filter
 * @desc Creates a new Emboss filter
 *
 * @arg Filter Id
 * @desc The id used to refer to this filter later, for editing/removal
 *
 * @arg Settings
 * @type struct<Emboss>
 * @desc Set up filter settings here. If using preset, fill in settings id from plugin params.
 *
 * @arg Targeting Options
 * @type struct<Targeting>
 * @desc Set up what the filter affects here.
 *
 * @command Edit Emboss Filter
 * @desc Edits an existing Emboss filter
 *
 * @arg Filter Id
 * @desc The id used to refer to this filter later, for editing/removal
 *
 * @arg Settings
 * @type struct<Emboss>
 * @desc Set up filter settings here. If using preset, fill in settings id from plugin params.
 *
 * @command Create Glitch Filter
 * @desc Creates a new Glitch filter
 *
 * @arg Filter Id
 * @desc The id used to refer to this filter later, for editing/removal
 *
 * @arg Settings
 * @type struct<Glitch>
 * @desc Set up filter settings here. If using preset, fill in settings id from plugin params.
 *
 * @arg Targeting Options
 * @type struct<Targeting>
 * @desc Set up what the filter affects here.
 *
 * @command Edit Glitch Filter
 * @desc Edit an existing Glitch filter
 *
 * @arg Filter Id
 * @desc The id used to refer to this filter later, for editing/removal
 *
 * @arg Settings
 * @type struct<Glitch>
 * @desc Set up filter settings here. If using preset, fill in settings id from plugin params.
 *
 * @command Create Godray Filter
 * @desc Creates a new godray filter
 *
 * @arg Filter Id
 * @desc The id used to refer to this filter later, for editing/removal
 *
 * @arg Settings
 * @type struct<Godray>
 * @desc Set up filter settings here. If using preset, fill in settings id from plugin params.
 *
 * @arg Targeting Options
 * @type struct<Targeting>
 * @desc Set up what the filter affects here.
 *
 * @command Edit Godray Filter
 * @desc Edit an existing godray filter
 *
 * @arg Filter Id
 * @desc The id of the filter to edit
 *
 * @arg Settings
 * @type struct<Godray>
 * @desc Set up filter settings here. If using preset, fill in settings id from plugin params.
 *
 * @command Create Grayscale Filter
 * @desc Creates a new Grayscale filter
 *
 * @arg Filter Id
 * @desc The id used to refer to this filter later, for editing/removal
 *
 * @arg Targeting Options
 * @type struct<Targeting>
 * @desc Set up what the filter affects here.
 *
 * @command Create Kawase Blur Filter
 * @desc Creates a new Kawase Blur filter
 *
 * @arg Filter Id
 * @desc The id used to refer to this filter later, for editing/removal
 *
 * @arg Settings
 * @type struct<KawaseBlur>
 * @desc Set up filter settings here. If using preset, fill in settings id from plugin params.
 *
 * @arg Targeting Options
 * @type struct<Targeting>
 * @desc Set up what the filter affects here.
 *
 * @command Edit Kawase Blur Filter
 * @desc Edits an existing Kawase Blur filter
 *
 * @arg Filter Id
 * @desc The id used to refer to this filter later, for editing/removal
 *
 * @arg Settings
 * @type struct<KawaseBlur>
 * @desc Set up filter settings here. If using preset, fill in settings id from plugin params.
 *
 * @command Create Noise Filter
 * @desc Creates a new Noise filter
 *
 * @arg Filter Id
 * @desc The id used to refer to this filter later, for editing/removal
 *
 * @arg Settings
 * @type struct<Noise>
 * @desc Set up filter settings here. If using preset, fill in settings id from plugin params.
 *
 * @arg Targeting Options
 * @type struct<Targeting>
 * @desc Set up what the filter affects here.
 *
 * @command Edit Noise Filter
 * @desc Edits an existing Noise filter
 *
 * @arg Filter Id
 * @desc The id used to refer to this filter later, for editing/removal
 *
 * @arg Settings
 * @type struct<Noise>
 * @desc Set up filter settings here. If using preset, fill in settings id from plugin params.
 *
 * @command Create Old Film Filter
 * @desc Creates a new Old Film filter
 *
 * @arg Filter Id
 * @desc The id used to refer to this filter later, for editing/removal
 *
 * @arg Settings
 * @type struct<OldFilm>
 * @desc Set up filter settings here. If using preset, fill in settings id from plugin params.
 *
 * @arg Targeting Options
 * @type struct<Targeting>
 * @desc Set up what the filter affects here.
 *
 * @command Edit Old Film Filter
 * @desc Edits an existing Old Film filter by identifier
 *
 * @arg Filter Id
 * @desc The id of the filter to edit
 *
 * @arg Settings
 * @type struct<OldFilm>
 * @desc Set up filter settings here. If using preset, fill in settings id from plugin params.
 *
 * @command Create Pixelate Filter
 * @desc Creates a new Pixelate filter
 *
 * @arg Filter Id
 * @desc The id used to refer to this filter later, for editing/removal
 *
 * @arg Settings
 * @type struct<Pixelate>
 * @desc Set up filter settings here. If using preset, fill in settings id from plugin params.
 *
 * @arg Targeting Options
 * @type struct<Targeting>
 * @desc Set up what the filter affects here
 *
 * @arg Targeting Options
 * @type struct<Targeting>
 * @desc Set up what the filter affects here.
 *
 * @command Edit Pixelate Filter
 * @desc Edits an existing Pixelate filter
 *
 * @arg Filter Id
 * @desc The id used to refer to this filter later, for editing/removal
 *
 * @arg Settings
 * @type struct<Pixelate>
 * @desc Set up filter settings here. If using preset, fill in settings id from plugin params.
 *
 * @param Preset Setup
 *
 * @param Adjustment Settings
 * @parent Preset Setup
 * @type struct<Adjustment>[]
 * @default []
 * @desc Set up adjustment filter settings here
 *
 * @param Advanced Bloom Settings
 * @parent Preset Setup
 * @type struct<AdvancedBloom>[]
 * @default []
 * @desc Set up advanced bloom filter settings here
 *
 * @param Alpha Settings
 * @parent Preset Setup
 * @type struct<Alpha>[]
 * @default []
 * @desc Set up alpha filter settings here
 *
 * @param Ascii Settings
 * @parent Preset Setup
 * @type struct<Ascii>[]
 * @default []
 * @desc Set up ascii filter settings here
 *
 * @param Bloom Settings
 * @parent Preset Setup
 * @type struct<Bloom>[]
 * @default []
 * @desc Set up bloom filter settings here
 *
 * @param Blur Settings
 * @parent Preset Setup
 * @type struct<Blur>[]
 * @default []
 * @desc Set up blur filter settings here
 *
 * @param Bulge Pinch Settings
 * @parent Preset Setup
 * @type struct<BulgePinch>[]
 * @default []
 * @desc Set up bulge pinch filter settings here
 *
 * @param Color Replace Settings
 * @parent Preset Setup
 * @type struct<ColorReplace>[]
 * @default []
 * @desc Set up color replace filter settings here
 *
 * @param CRT Settings
 * @parent Preset Setup
 * @type struct<CRT>[]
 * @default []
 * @desc Set up CRT filter settings here
 *
 * @param Displacement Settings
 * @parent Preset Setup
 * @type struct<Displacement>[]
 * @default []
 * @desc Set up displacement filter settings here
 *
 * @param Dot Settings
 * @parent Preset Setup
 * @type struct<Dot>[]
 * @default []
 * @desc Set up dot filter settings here
 *
 * @param Emboss Settings
 * @parent Preset Setup
 * @type struct<Emboss>[]
 * @default []
 * @desc Set up Emboss filter settings here
 *
 * @param Glitch Settings
 * @parent Preset Setup
 * @type struct<Glitch>[]
 * @default []
 * @desc Set up glitch filter settings here
 *
 * @param Godray Settings
 * @parent Preset Setup
 * @type struct<Godray>[]
 * @default []
 * @desc Set up godray filter settings here
 *
 * @param Kawase Blur Settings
 * @parent Preset Setup
 * @type struct<KawaseBlur>[]
 * @default []
 * @desc Set up kawase blur filter settings here
 *
 * @param Noise Settings
 * @parent Preset Setup
 * @type struct<Noise>[]
 * @default []
 * @desc Set up noise filter settings here
 *
 * @param Old Film Settings
 * @parent Preset Setup
 * @type struct<OldFilm>[]
 * @default []
 * @desc Set up old film filter settings here
 *
 * @param Pixelate Settings
 * @parent Preset Setup
 * @type struct<Pixelate>[]
 * @default []
 * @desc Set up pixelate filter settings here
*/
/*~struct~Targeting:
 * @param Scene
 * @type select[]
 * @option All
 * @option Scene_Battle
 * @option Scene_Equip
 * @option Scene_GameEnd
 * @option Scene_Gameover
 * @option Scene_Item
 * @option Scene_Load
 * @option Scene_Map
 * @option Scene_Menu
 * @option Scene_Name
 * @option Scene_Options
 * @option Scene_Save
 * @option Scene_Shop
 * @option Scene_Skill
 * @option Scene_Status
 * @default ["Scene_Map"]
 * @desc The scene during which the filter is active. If using "All", do not include any other scenes
 *
 * @param Target
 * @type select
 * @option Scene
 * @option Spriteset
 * @option Base Sprite
 * @default Scene
 * @desc The target within the scene to add the filter to
*/
/*~struct~Adjustment:
 * @param Settings Id
 * @desc The id of these filter settings
 *
 * @param Gamma
 * @type number
 * @decimals 3
 * @min 0
 * @max 5
 * @default 1.000
 * @desc The gamma of the filter
 *
 * @param Saturation
 * @type number
 * @decimals 3
 * @min 0
 * @max 5
 * @default 1.000
 * @desc The saturation of the filter
 *
 * @param Contrast
 * @type number
 * @decimals 3
 * @min 0
 * @max 5
 * @default 1.000
 * @desc The contrast of the filter
 *
 * @param Brightness
 * @type number
 * @decimals 3
 * @min 0
 * @max 5
 * @default 1.000
 * @desc The brightness of the filter
 *
 * @param Red
 * @type number
 * @decimals 3
 * @min 0
 * @max 5
 * @default 1.000
 * @desc The red of the filter
 *
 * @param Green
 * @type number
 * @decimals 3
 * @min 0
 * @max 5
 * @default 1.000
 * @desc The green of the filter
 *
 * @param Blue
 * @type number
 * @decimals 3
 * @min 0
 * @max 5
 * @default 1.000
 * @desc The blue of the filter
 *
 * @param Alpha
 * @type number
 * @decimals 3
 * @min 0
 * @max 1
 * @default 1.000
 * @desc The alpha of the filter
*/
/*~struct~AdvancedBloom:
 * @param Settings Id
 * @desc The id of these filter settings
 *
 * @param Threshold
 * @type number
 * @decimals 4
 * @min 0.1
 * @max 0.9
 * @default 0.5
 * @desc The threshold of the filter
 *
 * @param Bloom Scale
 * @type number
 * @decimals 3
 * @min 0.5
 * @max 1.5
 * @default 1.0
 * @desc The bloom scale of the filter
 *
 * @param Brightness
 * @type number
 * @decimals 3
 * @min 0.5
 * @max 1.5
 * @default 1.0
 * @desc The brightness of the filter
 *
 * @param Blur
 * @type number
 * @decimals 3
 * @min 0
 * @max 20
 * @default 8
 * @desc The blur of the filter
 *
 * @param Quality
 * @type number
 * @min 1
 * @max 20
 * @default 4
 * @desc The blur of the filter
*/
/*~struct~Alpha:
 * @param Settings Id
 * @desc The id of these filter settings
 *
 * @param Alpha
 * @type number
 * @min 0
 * @max 1
 * @decimals 3
 * @default 1
 * @desc The alpha to use for the filter
*/
/*~struct~Ascii:
 * @param Settings Id
 * @desc The id of these filter settings
 *
 * @param Size
 * @type number
 * @min 1
 * @max 24
 * @default 8
 * @desc The pixel size used by the filter.
*/
/*~struct~Bloom:
 * @param Settings Id
 * @desc The id of these filter settings
 *
 * @param Blur
 * @type number
 * @min 0
 * @max 20
 * @decimals 2
 * @default 2
 * @desc The strength of both the blurX and blurY properties simultaneously
 *
 * @param Blur X
 * @type number
 * @min 0
 * @max 20
 * @decimals 2
 * @default 2
 * @desc The strength of the blurX property
 *
 * @param Blur Y
 * @type number
 * @min 0
 * @max 20
 * @decimals 2
 * @default 2
 * @desc The strength of the blurX property
*/
/*~struct~Blur:
 * @param Settings Id
 * @desc The id of these filter settings
 *
 * @param Blur
 * @type number
 * @min 0
 * @max 100
 * @decimals 1
 * @default 8
 * @desc The amount of blur
 *
 * @param Quality
 * @type number
 * @min 1
 * @max 10
 * @decimals 3
 * @default 4.000
 * @desc The quality of the blur
*/
/*~struct~BulgePinch:
 * @param Settings Id
 * @desc The id of these filter settings
 *
 * @param Radius
 * @type number
 * @min 0
 * @max 1000
 * @default 300
 * @desc The radius of the bulge pinch effect
 *
 * @param Strength
 * @type number
 * @min -1
 * @max 1
 * @decimals 3
 * @default 1
 * @desc The strength of the effect
 *
 * @param Center X
 * @type number
 * @min 0
 * @max 1
 * @decimals 3
 * @default 0.500
 * @desc The x of the effect (as a percentage of screen width)
 *
 * @param Center Y
 * @type number
 * @min 0
 * @max 1
 * @decimals 3
 * @default 0.500
 * @desc The y of the effect (as a percentage of screen height)
*/
/*~struct~ColorReplace:
 * @param Settings Id
 * @desc The id of these filter settings
 *
 * @param Epsilon
 * @type number
 * @min 0
 * @max 1
 * @decimals 3
 * @default 0.400
 * @desc Tolerance/sensitivity of the floating-point comparison between colors (lower = more exact, higher = more inclusive)
 *
 * @param Original Red
 * @type number
 * @min 0
 * @max 255
 * @default 0
 * @desc Amount of red in the original color RGB value
 *
 * @param Original Blue
 * @type number
 * @min 0
 * @max 255
 * @default 0
 * @desc Amount of blue in the original color RGB value
 *
 * @param Original Green
 * @type number
 * @min 0
 * @max 255
 * @default 0
 * @desc Amount of green in the original color RGB value
 *
 * @param New Red
 * @type number
 * @min 0
 * @max 255
 * @default 0
 * @desc Amount of red in the new color RGB value
 *
 * @param New Blue
 * @type number
 * @min 0
 * @max 255
 * @default 0
 * @desc Amount of blue in the new color RGB value
 *
 * @param New Green
 * @type number
 * @min 0
 * @max 255
 * @default 0
 * @desc Amount of green in the new color RGB value
*/
/*~struct~CRT:
 * @param Settings Id
 * @desc The id of these filter settings
 * 
 * @param Animating
 * @type boolean
 * @default true
 * @desc Determine whether the crt should animate or not
 *
 * @param Animation Speed
 * @type number
 * @min 0.001
 * @max 1.000
 * @decimals 3
 * @default 0.3
 * @desc Speed at which the animation will play
 *
 * @param Time
 * @type number
 * @min 0
 * @default 0
 * @desc The time position. This is not used if animating.
 *
 * @param Curvature
 * @type number
 * @min 0
 * @decimals 1
 * @default 2.0
 * @desc The amount of curvature to give the lines.
 *
 * @param Line Width
 * @type number
 * @min 0
 * @decimals 1
 * @default 4.0
 * @desc The width of the lines
 *
 * @param Line Contrast
 * @type number
 * @min 0
 * @max 1
 * @decimals 2
 * @default 0.5
 * @desc The contrast of the lines
 *
 * @param Noise
 * @type number
 * @min 0
 * @max 1
 * @decimals 2
 * @default 0.3
 * @desc The opacity/intensity of the noise pixels
 *
 * @param Noise Size
 * @type number
 * @min 0
 * @max 10
 * @decimals 1
 * @default 1
 * @desc The size of the noise pixels
 *
 * @param Vertical Line
 * @type boolean
 * @default false
 * @desc Whether lines should be vertical (true) or horizontal (false)
 *
 * @param Seed
 * @type number
 * @min 0
 * @max 1
 * @decimals 2
 * @default 0
 * @desc The seed to use for random pixel generation
 *
 * @param Vignetting
 * @type number
 * @min 0
 * @max 1
 * @decimals 2
 * @default 0.4
 * @desc The radius of vignette effect. Smaller value = smaller effect
 *
 * @param Vignetting Alpha
 * @type number
 * @min 0
 * @max 1
 * @decimals 2
 * @default 1
 * @desc The opacity of the vignette effect
 *
 * @param Vignetting Blur
 * @type number
 * @min 0
 * @max 1
 * @decimals 2
 * @default 0.3
 * @desc The intensity of the blur effect on the vignette
*/
/*~struct~Displacement:
 * @param Settings Id
 * @desc The id of these filter settings
 *
 * @param Scale X
 * @type number
 * @min 1
 * @max 200
 * @decimals 3
 * @default 50.0
 * @desc The scale X of the effect
 *
 * @param Scale Y
 * @type number
 * @min 1
 * @max 200
 * @decimals 3
 * @default 50.0
 * @desc The scale Y of the effect
 *
 * @param Map
 * @type file
 * @dir img/
 * @desc The displacement map texture to use for the effect
 *
 * @param Animating
 * @type boolean
 * @default true
 * @desc If the displacement scrolls (true) or static (false)
 *
 * @param X Scroll
 * @type number
 * @min -9999
 * @default 1
 * @desc The scroll x of the effect
 *
 * @param Y Scroll
 * @type number
 * @min -9999
 * @default 1
 * @desc The scroll y of the effect
*/
/*~struct~Dot:
 * @param Settings Id
 * @desc The id of these filter settings
 *
 * @param Scale
 * @type number
 * @min 0
 * @max 1
 * @decimals 1
 * @default 1.0
 * @desc The scale of the effect
 *
 * @param Grayscale
 * @type boolean
 * @default true
 * @desc If the dot effect is grayscale
 *
 * @param Angle
 * @type number
 * @min 0
 * @max 5
 * @decimals 1
 * @default 5.0
 * @desc The angle of the effect
*/
/*~struct~Emboss:
 * @param Settings Id
 * @desc The id of these filter settings
 * 
 * @param Strength
 * @type number
 * @decimals 2
 * @min 0
 * @max 20
 * @default 5.00
 * @desc Strength of the filter
*/
/*~struct~Glitch:
 * @param Settings Id
 * @desc The id of these filter settings
 *
 * @param Animating
 * @type boolean
 * @default true
 * @desc Determine whether the glitch filter should animate or not
 *
 * @param Average
 * @type boolean
 * @default false
 * @desc True divides the bands roughly equally, false makes them more random looking
 *
 * @param Seed
 * @type number
 * @min 0
 * @max 1
 * @decimals 3
 * @default 0.500
 * @desc Glitch seed
 *
 * @param Slices
 * @type number
 * @min 2
 * @max 20
 * @default 5
 * @desc Number of glitch slices
 *
 * @param Offset
 * @type number
 * @min -400
 * @max 400
 * @default 100
 * @desc The max offset of the slices
 *
 * @param Direction
 * @type number
 * @min -180
 * @max 180
 * @default 0
 * @desc Angle of offset of the slices
 *
 * @param Fill Mode
 * @type select
 * @option Transparent
 * @option Original
 * @option Loop
 * @option Clamp
 * @option Mirror
 * @default Loop
 * @desc The fill mode of the glitch slices
 *
 * @param Min Size
 * @type number
 * @min 1
 * @max 512
 * @default 8
 * @desc The minimum size of individual slice, as a segment of sample size
 *
 * @param Sample Size
 * @type number
 * @min 1
 * @max 2048
 * @default 512
 * @desc The resolution of the displacement map texture
 *
 * @param Red X
 * @type number
 * @min -200
 * @max 200
 * @default 0
 * @desc The offset of the color red (x axis)
 *
 * @param Red Y
 * @type number
 * @min -200
 * @max 200
 * @default 0
 * @desc The offset of the color red (y axis)
 *
 * @param Blue X
 * @type number
 * @min -200
 * @max 200
 * @default 0
 * @desc The offset of the color blue (x axis)
 *
 * @param Blue Y
 * @type number
 * @min -200
 * @max 200
 * @default 0
 * @desc The offset of the color blue (y axis)
 *
 * @param Green X
 * @type number
 * @min -200
 * @max 200
 * @default 0
 * @desc The offset of the color green (x axis)
 *
 * @param Green Y
 * @type number
 * @min -200
 * @max 200
 * @default 0
 * @desc The offset of the color green (y axis)
*/
/*~struct~Godray:
 * @param Settings Id
 * @desc The id of these filter settings
 * 
 * @param Animating
 * @type boolean
 * @default true
 * @desc Determine whether the godray should animate or not
 *
 * @param Animation Speed
 * @type number
 * @min 0.001
 * @max 1.000
 * @decimals 3
 * @default 0.01
 * @desc Speed at which the animation will play
 *
 * @param Alpha
 * @type number
 * @min 0
 * @max 1.000
 * @decimals 3
 * @default 1.000
 * @desc Alpha of the godray (1 = full opaque, 0 = full transparent)
 *
 * @param Time
 * @type number
 * @min 0
 * @default 0
 * @desc The time position. This is not used if animating.
 *
 * @param Gain
 * @type number
 * @min 0.00
 * @max 1.00
 * @decimals 2
 * @default 0.50
 * @desc General intensity of the effect
 *
 * @param Lacunarity
 * @type number
 * @min 0.0
 * @max 5.0
 * @decimals 1
 * @default 2.5
 * @desc Density of fractal noise
 *
 * @param Parallel
 * @type boolean
 * @default true
 * @desc True to use angle, False to use center
 *
 * @param Angle
 * @type number
 * @min -60
 * @max 60
 * @default 30
 * @decimals 0
 * @desc Angle / Light source of rays (if using parallel)
 *
 * @param X
 * @type number
 * @default 0
 * @desc x origin of light rays (if not using parallel)
 *
 * @param Y
 * @type number
 * @default 0
 * @desc y origin of light rays (if not using parallel)
*/
/*~struct~KawaseBlur:
 * @param Settings Id
 * @desc The id of these filter settings
 *
 * @param Blur
 * @type number
 * @min 0
 * @max 20
 * @decimals 1
 * @default 4.0
 * @desc The intensity of the blur effect
 *
 * @param Quality
 * @type number
 * @min 1
 * @max 20
 * @decimals 1
 * @default 4.0
 * @desc The quality of the blur effect
 *
 * @param Pixel Size X
 * @type number
 * @min 0
 * @max 10
 * @decimals 1
 * @default 1.0
 * @desc The pixel size of the filter (x-axis)
 *
 * @param Pixel Size Y
 * @type number
 * @min 0
 * @max 10
 * @decimals 1
 * @default 1.0
 * @desc The pixel size of the filter (y-axis)
 *
 * @param Clamp
 * @type boolean
 * @default false
 * @desc Clamp edges, useful for removing dark edges from fullscreen filters or bleeding to the edge of filterArea.
*/
/*~struct~Noise:
 * @param Settings Id
 * @desc The id of these filter settings
 *
 * @param Animating
 * @type boolean
 * @default true
 * @desc Determine whether the noise filter should animate or not
 *
 * @param Noise
 * @type number
 * @min 0
 * @max 1
 * @decimals 2
 * @default 0.5
 * @desc The opacity/intensity of the noise pixels
 *
 * @param Seed
 * @type number
 * @min 0
 * @max 0.99
 * @decimals 2
 * @default 0.3
 * @desc The seed to use for random pixel generation
*/
/*~struct~OldFilm:
 * @param Settings Id
 * @desc The id of these filter settings
 *
 * @param Animating
 * @type boolean
 * @default true
 * @desc Determine whether the old film filter should animate or not
 *
 * @param Sepia
 * @type number
 * @min 0
 * @max 1
 * @decimals 2
 * @default 0.30
 * @desc The amount of saturation on the sepia effect.
 *
 * @param Scratch
 * @type number
 * @min -1
 * @max 1
 * @decimals 2
 * @default 0.50
 * @desc How often scratches appear
 *
 * @param Scratch Density
 * @type number
 * @min 0
 * @max 1
 * @decimals 2
 * @default 0.30
 * @desc The density of scratch lines, higher number = more lines
 *
 * @param Scratch Width
 * @type number
 * @min 0
 * @max 20
 * @decimals 1
 * @default 1.0
 * @desc The width of the scratch lines
 *
 * @param Noise
 * @type number
 * @min 0
 * @max 1
 * @decimals 2
 * @default 0
 * @desc The opacity/intensity of the noise pixels
 *
 * @param Noise Size
 * @type number
 * @min 0
 * @max 10
 * @decimals 1
 * @default 1
 * @desc The size of the noise pixels
 *
 * @param Seed
 * @type number
 * @min 0
 * @max 1
 * @decimals 2
 * @default 0
 * @desc The seed to use for random pixel generation
 *
 * @param Vignetting
 * @type number
 * @min 0
 * @max 1
 * @decimals 2
 * @default 0
 * @desc The radius of vignette effect. Smaller value = smaller effect
 *
 * @param Vignetting Alpha
 * @type number
 * @min 0
 * @max 1
 * @decimals 2
 * @default 0
 * @desc The opacity of the vignette effect
 *
 * @param Vignetting Blur
 * @type number
 * @min 0
 * @max 1
 * @decimals 2
 * @default 0
 * @desc The intensity of the blur effect on the vignette
*/
/*~struct~Pixelate:
 * @param Settings Id
 * @desc The id of these filter settings
 *
 * @param Size X
 * @type number
 * @min 1
 * @max 40
 * @default 4
 * @desc The size of the pixel blocks (x-axis)
 *
 * @param Size Y
 * @type number
 * @min 1
 * @max 40
 * @default 4
 * @desc The size of the pixel blocks (y-axis)
*/
Imported.CGMZ_PixiFilters = true;
CGMZ.Versions["Pixi Filters"] = "Alpha R6";
CGMZ.PixiFilters = {};
CGMZ.PixiFilters.parameters = PluginManager.parameters('CGMZ_PixiFilters');
CGMZ.PixiFilters.AdjustmentSettings = CGMZ_Utils.parseJSON(CGMZ.PixiFilters.parameters["Adjustment Settings"], [], "[CGMZ] Pixi Filters", "Your Adjustment Settings parameter was set up incorrectly and could not be read.");
CGMZ.PixiFilters.AdvancedBloomSettings = CGMZ_Utils.parseJSON(CGMZ.PixiFilters.parameters["Advanced Bloom Settings"], [], "[CGMZ] Pixi Filters", "Your Advanced Bloom Settings parameter was set up incorrectly and could not be read.");
CGMZ.PixiFilters.AlphaSettings = CGMZ_Utils.parseJSON(CGMZ.PixiFilters.parameters["Alpha Settings"], [], "[CGMZ] Pixi Filters", "Your Alpha Settings parameter was set up incorrectly and could not be read.");
CGMZ.PixiFilters.AsciiSettings = CGMZ_Utils.parseJSON(CGMZ.PixiFilters.parameters["Ascii Settings"], [], "[CGMZ] Pixi Filters", "Your Ascii Settings parameter was set up incorrectly and could not be read.");
CGMZ.PixiFilters.BloomSettings = CGMZ_Utils.parseJSON(CGMZ.PixiFilters.parameters["Bloom Settings"], [], "[CGMZ] Pixi Filters", "Your Bloom Settings parameter was set up incorrectly and could not be read.");
CGMZ.PixiFilters.BlurSettings = CGMZ_Utils.parseJSON(CGMZ.PixiFilters.parameters["Blur Settings"], [], "[CGMZ] Pixi Filters", "Your Blur Settings parameter was set up incorrectly and could not be read.");
CGMZ.PixiFilters.BulgePinchSettings = CGMZ_Utils.parseJSON(CGMZ.PixiFilters.parameters["Bulge Pinch Settings"], [], "[CGMZ] Pixi Filters", "Your Bulge Pinch Settings parameter was set up incorrectly and could not be read.");
CGMZ.PixiFilters.ColorReplaceSettings = CGMZ_Utils.parseJSON(CGMZ.PixiFilters.parameters["Color Replace Settings"], [], "[CGMZ] Pixi Filters", "Your Color Replace Settings parameter was set up incorrectly and could not be read.");
CGMZ.PixiFilters.CRTSettings = CGMZ_Utils.parseJSON(CGMZ.PixiFilters.parameters["CRT Settings"], [], "[CGMZ] Pixi Filters", "Your CRT Settings parameter was set up incorrectly and could not be read.");
CGMZ.PixiFilters.DisplacementSettings = CGMZ_Utils.parseJSON(CGMZ.PixiFilters.parameters["Displacement Settings"], [], "[CGMZ] Pixi Filters", "Your Displacement Settings parameter was set up incorrectly and could not be read.");
CGMZ.PixiFilters.DotSettings = CGMZ_Utils.parseJSON(CGMZ.PixiFilters.parameters["Dot Settings"], [], "[CGMZ] Pixi Filters", "Your Dot Settings parameter was set up incorrectly and could not be read.");
CGMZ.PixiFilters.EmbossSettings = CGMZ_Utils.parseJSON(CGMZ.PixiFilters.parameters["Emboss Settings"], [], "[CGMZ] Pixi Filters", "Your Emboss Settings parameter was set up incorrectly and could not be read.");
CGMZ.PixiFilters.GlitchSettings = CGMZ_Utils.parseJSON(CGMZ.PixiFilters.parameters["Glitch Settings"], [], "[CGMZ] Pixi Filters", "Your Glitch Settings parameter was set up incorrectly and could not be read.");
CGMZ.PixiFilters.GodraySettings = CGMZ_Utils.parseJSON(CGMZ.PixiFilters.parameters["Godray Settings"], [], "[CGMZ] Pixi Filters", "Your Godray Settings parameter was set up incorrectly and could not be read.");
CGMZ.PixiFilters.KawaseBlurSettings = CGMZ_Utils.parseJSON(CGMZ.PixiFilters.parameters["Kawase Blur Settings"], [], "[CGMZ] Pixi Filters", "Your Kawase Blur Settings parameter was set up incorrectly and could not be read.");
CGMZ.PixiFilters.OldFilmSettings = CGMZ_Utils.parseJSON(CGMZ.PixiFilters.parameters["Old Film Settings"], [], "[CGMZ] Pixi Filters", "Your Old Film Settings parameter was set up incorrectly and could not be read.");
CGMZ.PixiFilters.NoiseSettings = CGMZ_Utils.parseJSON(CGMZ.PixiFilters.parameters["Noise Settings"], [], "[CGMZ] Pixi Filters", "Your Noise Settings parameter was set up incorrectly and could not be read.");
CGMZ.PixiFilters.PixelateSettings = CGMZ_Utils.parseJSON(CGMZ.PixiFilters.parameters["Pixelate Settings"], [], "[CGMZ] Pixi Filters", "Your Pixelate Settings parameter was set up incorrectly and could not be read.");
//=============================================================================
// CGMZ_AdjustmentFilter
//-----------------------------------------------------------------------------
// Store data for Adjustment filters
//=============================================================================
function CGMZ_AdjustmentFilter() {
	this.initialize(...arguments);
}
//-----------------------------------------------------------------------------
// Initialize filter
//-----------------------------------------------------------------------------
CGMZ_AdjustmentFilter.prototype.initialize = function(id, target, scenes, settings) {
	this.cgmzType = 'adjustment';
	this.target = target;
	this.id = id;
	this.scenes = scenes;
	this.updateFilterSettings(settings);
};
//-----------------------------------------------------------------------------
// Update filter settings
//-----------------------------------------------------------------------------
CGMZ_AdjustmentFilter.prototype.updateFilterSettings = function(settings) {
	this.gamma = settings.gamma;
	this.saturation = settings.saturation;
	this.contrast = settings.contrast;
	this.brightness = settings.brightness;
	this.red = settings.red;
	this.green = settings.green;
	this.blue = settings.blue;
	this.alpha = settings.alpha;
};
//-----------------------------------------------------------------------------
// Get settings to edit for filter
//-----------------------------------------------------------------------------
CGMZ_AdjustmentFilter.prototype.getEditSettings = function() {
	return {gamma: this.gamma, saturation: this.saturation, contrast: this.contrast, brightness: this.brightness, red: this.red, green: this.green, blue: this.blue, alpha: this.alpha};
};
//=============================================================================
// CGMZ_AdvancedBloomFilter
//-----------------------------------------------------------------------------
// Store data for Advanced Bloom filters
//=============================================================================
function CGMZ_AdvancedBloomFilter() {
	this.initialize(...arguments);
}
//-----------------------------------------------------------------------------
// Initialize filter
//-----------------------------------------------------------------------------
CGMZ_AdvancedBloomFilter.prototype.initialize = function(id, target, scenes, settings) {
	this.cgmzType = 'advancedbloom';
	this.target = target;
	this.id = id;
	this.scenes = scenes;
	this.updateFilterSettings(settings);
};
//-----------------------------------------------------------------------------
// Update filter settings
//-----------------------------------------------------------------------------
CGMZ_AdvancedBloomFilter.prototype.updateFilterSettings = function(settings) {
	this.threshold = settings.threshold;
	this.bloomScale = settings.bloomScale;
	this.brightness = settings.brightness;
	this.blur = settings.blur;
	this.quality = settings.quality;
};
//-----------------------------------------------------------------------------
// Get settings to edit for filter
//-----------------------------------------------------------------------------
CGMZ_AdvancedBloomFilter.prototype.getEditSettings = function() {
	return {threshold: this.threshold, bloomScale: this.bloomScale, brightness: this.brightness, blur: this.blur, quality: this.quality};
};
//=============================================================================
// CGMZ_AlphaFilter
//-----------------------------------------------------------------------------
// Store data for Alpha filters
//=============================================================================
function CGMZ_AlphaFilter() {
	this.initialize(...arguments);
}
//-----------------------------------------------------------------------------
// Initialize filter
//-----------------------------------------------------------------------------
CGMZ_AlphaFilter.prototype.initialize = function(id, target, scenes, settings) {
	this.cgmzType = 'alpha';
	this.target = target;
	this.id = id;
	this.scenes = scenes;
	this.updateFilterSettings(settings);
};
//-----------------------------------------------------------------------------
// Update the filter settings
//-----------------------------------------------------------------------------
CGMZ_AlphaFilter.prototype.updateFilterSettings = function(settings) {
	this.alpha = settings.alpha;
};
//-----------------------------------------------------------------------------
// Get settings to edit for filter
//-----------------------------------------------------------------------------
CGMZ_AlphaFilter.prototype.getEditSettings = function() {
	return {alpha: this.alpha};
};
//=============================================================================
// CGMZ_AsciiFilter
//-----------------------------------------------------------------------------
// Store data for Ascii filters
//=============================================================================
function CGMZ_AsciiFilter() {
	this.initialize(...arguments);
}
//-----------------------------------------------------------------------------
// Initialize filter
//-----------------------------------------------------------------------------
CGMZ_AsciiFilter.prototype.initialize = function(id, target, scenes, settings) {
	this.cgmzType = 'ascii';
	this.target = target;
	this.id = id;
	this.scenes = scenes;
	this.updateFilterSettings(settings);
};
//-----------------------------------------------------------------------------
// Update the filter settings
//-----------------------------------------------------------------------------
CGMZ_AsciiFilter.prototype.updateFilterSettings = function(settings) {
	this.size = settings.size;
};
//-----------------------------------------------------------------------------
// Get settings to edit for filter
//-----------------------------------------------------------------------------
CGMZ_AsciiFilter.prototype.getEditSettings = function() {
	return {size: this.size};
};
//=============================================================================
// CGMZ_BloomFilter
//-----------------------------------------------------------------------------
// Store data for Bloom filters
//=============================================================================
function CGMZ_BloomFilter() {
	this.initialize(...arguments);
}
//-----------------------------------------------------------------------------
// Initialize filter
//-----------------------------------------------------------------------------
CGMZ_BloomFilter.prototype.initialize = function(id, target, scenes, settings) {
	this.cgmzType = 'bloom';
	this.target = target;
	this.id = id;
	this.scenes = scenes;
	this.updateFilterSettings(settings);
};
//-----------------------------------------------------------------------------
// Update the filter settings
//-----------------------------------------------------------------------------
CGMZ_BloomFilter.prototype.updateFilterSettings = function(settings) {
	this.blur = settings.blur;
	this.blurX = settings.blurX;
	this.blurY = settings.blurY;
};
//-----------------------------------------------------------------------------
// Get settings to edit for filter
//-----------------------------------------------------------------------------
CGMZ_BloomFilter.prototype.getEditSettings = function() {
	return {blur: this.blur, blurX: this.blurX, blurY: this.blurY};
};
//=============================================================================
// CGMZ_BlurFilter
//-----------------------------------------------------------------------------
// Store data for Blur filters
//=============================================================================
function CGMZ_BlurFilter() {
	this.initialize(...arguments);
}
//-----------------------------------------------------------------------------
// Initialize filter
//-----------------------------------------------------------------------------
CGMZ_BlurFilter.prototype.initialize = function(id, target, scenes, settings) {
	this.cgmzType = 'blur';
	this.target = target;
	this.id = id;
	this.scenes = scenes;
	this.updateFilterSettings(settings);
};
//-----------------------------------------------------------------------------
// Update the filter settings
//-----------------------------------------------------------------------------
CGMZ_BlurFilter.prototype.updateFilterSettings = function(settings) {
	this.blur = settings.blur;
	this.quality = settings.quality;
};
//-----------------------------------------------------------------------------
// Get settings to edit for filter
//-----------------------------------------------------------------------------
CGMZ_BlurFilter.prototype.getEditSettings = function() {
	return {blur: this.blur, quality: this.quality};
};
//=============================================================================
// CGMZ_BulgePinchFilter
//-----------------------------------------------------------------------------
// Store data for Bulge Pinch filters
//=============================================================================
function CGMZ_BulgePinchFilter() {
	this.initialize(...arguments);
}
//-----------------------------------------------------------------------------
// Initialize filter
//-----------------------------------------------------------------------------
CGMZ_BulgePinchFilter.prototype.initialize = function(id, target, scenes, settings) {
	this.cgmzType = 'bulgepinch';
	this.target = target;
	this.id = id;
	this.scenes = scenes;
	this.updateFilterSettings(settings);
};
//-----------------------------------------------------------------------------
// Update the filter settings
//-----------------------------------------------------------------------------
CGMZ_BulgePinchFilter.prototype.updateFilterSettings = function(settings) {
	this.radius = settings.radius;
	this.strength = settings.strength;
	this.center = new Point(settings.center.x, settings.center.y);
};
//-----------------------------------------------------------------------------
// Get settings to edit for filter
//-----------------------------------------------------------------------------
CGMZ_BulgePinchFilter.prototype.getEditSettings = function() {
	return {radius: this.radius, strength: this.strength, center: JSON.parse(JSON.stringify(this.center))};
};
//=============================================================================
// CGMZ_ColorReplaceFilter
//-----------------------------------------------------------------------------
// Store data for Alpha filters
//=============================================================================
function CGMZ_ColorReplaceFilter() {
	this.initialize(...arguments);
}
//-----------------------------------------------------------------------------
// Initialize filter
//-----------------------------------------------------------------------------
CGMZ_ColorReplaceFilter.prototype.initialize = function(id, target, scenes, settings) {
	this.cgmzType = 'colorreplace';
	this.target = target;
	this.id = id;
	this.scenes = scenes;
	this.updateFilterSettings(settings);
};
//-----------------------------------------------------------------------------
// Update the filter settings
//-----------------------------------------------------------------------------
CGMZ_ColorReplaceFilter.prototype.updateFilterSettings = function(settings) {
	this.epsilon = settings.epsilon;
	this.originalColor = JSON.parse(JSON.stringify(settings.originalColor));
	this.newColor = JSON.parse(JSON.stringify(settings.newColor));
};
//-----------------------------------------------------------------------------
// Get settings to edit for filter
//-----------------------------------------------------------------------------
CGMZ_ColorReplaceFilter.prototype.getEditSettings = function() {
	return {epsilon: this.epsilon, originalColor: JSON.parse(JSON.stringify(this.originalColor)), newColor: JSON.parse(JSON.stringify(this.newColor))};
};
//=============================================================================
// CGMZ_CrossHatchFilter
//-----------------------------------------------------------------------------
// Store data for CrossHatch filters
//=============================================================================
function CGMZ_CrossHatchFilter() {
	this.initialize(...arguments);
}
//-----------------------------------------------------------------------------
// Initialize filter
//-----------------------------------------------------------------------------
CGMZ_CrossHatchFilter.prototype.initialize = function(id, target, scenes) {
	this.cgmzType = 'crosshatch';
	this.target = target;
	this.id = id;
	this.scenes = scenes;
};
//=============================================================================
// CGMZ_CRTFilter
//-----------------------------------------------------------------------------
// Store data for CRT filters
//=============================================================================
function CGMZ_CRTFilter() {
	this.initialize(...arguments);
}
//-----------------------------------------------------------------------------
// Initialize filter
//-----------------------------------------------------------------------------
CGMZ_CRTFilter.prototype.initialize = function(id, target, scenes, settings) {
	this.cgmzType = 'crt';
	this.target = target;
	this.id = id;
	this.scenes = scenes;
	this.updateFilterSettings(settings);
};
//-----------------------------------------------------------------------------
// Update the filter settings
//-----------------------------------------------------------------------------
CGMZ_CRTFilter.prototype.updateFilterSettings = function(settings) {
	this.animating = settings.animating;
	this.animationSpeed = settings.animationSpeed;
	this.seed = settings.seed;
	this.time = settings.time;
	this.curvature = settings.curvature;
	this.lineContrast = settings.lineContrast;
	this.lineWidth =settings.lineWidth;
	this.verticalLine = settings.verticalLine;
	this.noise = settings.noise;
	this.noiseSize = settings.noiseSize;
	this.vignetting = settings.vignetting;
	this.vignettingAlpha = settings.vignettingAlpha;
	this.vignettingBlur = settings.vignettingBlur;
};
//-----------------------------------------------------------------------------
// Get settings to edit for filter
//-----------------------------------------------------------------------------
CGMZ_CRTFilter.prototype.getEditSettings = function() {
	return {
		seed: this.seed, time: this.time, curvature: this.curvature, lineContrast: this.lineContrast, lineWidth: this.lineWidth, verticalLine: this.verticalLine, noise: this.noise,
		noiseSize: this.noiseSize, vignetting: this.vignetting, vignettingAlpha: this.vignettingAlpha, vignettingBlur: this.vignettingBlur, animating: this.animating, animationSpeed: this.animationSpeed
	};
};
//=============================================================================
// CGMZ_DisplacementFilter
//-----------------------------------------------------------------------------
// Store data for Displacement filters
//=============================================================================
function CGMZ_DisplacementFilter() {
	this.initialize(...arguments);
}
//-----------------------------------------------------------------------------
// Initialize filter
//-----------------------------------------------------------------------------
CGMZ_DisplacementFilter.prototype.initialize = function(id, target, scenes, settings) {
	this.cgmzType = 'displacement';
	this.target = target;
	this.id = id;
	this.scenes = scenes;
	this.updateFilterSettings(settings);
};
//-----------------------------------------------------------------------------
// Update filter settings
//-----------------------------------------------------------------------------
CGMZ_DisplacementFilter.prototype.updateFilterSettings = function(settings) {
	this.map = settings.map;
	this.scale = new Point(settings.scale.x, settings.scale.y);
	this.animating = settings.animating;
	this.xScroll = settings.xScroll;
	this.yScroll = settings.yScroll;
};
//-----------------------------------------------------------------------------
// Get displacement sprite
//-----------------------------------------------------------------------------
CGMZ_DisplacementFilter.prototype.getDisplacementTexture = function() {
	const sprite = new Sprite();
	const imgData = CGMZ_Utils.getImageData(this.map, "img");
	sprite.bitmap = ImageManager.loadBitmap(imgData.folder, imgData.filename);
	sprite.texture.baseTexture.wrapMore = PIXI.WRAP_MODES.REPEAT;
	return sprite;
};
//-----------------------------------------------------------------------------
// Get settings to edit for filter
//-----------------------------------------------------------------------------
CGMZ_DisplacementFilter.prototype.getEditSettings = function() {
	return {scale: new Point(this.scale.x, this.scale.y), xScroll: this.xScroll, yScroll: this.yScroll, animating: this.animating};
};
//=============================================================================
// CGMZ_DotFilter
//-----------------------------------------------------------------------------
// Store data for Dot filters
//=============================================================================
function CGMZ_DotFilter() {
	this.initialize(...arguments);
}
//-----------------------------------------------------------------------------
// Initialize filter
//-----------------------------------------------------------------------------
CGMZ_DotFilter.prototype.initialize = function(id, target, scenes, settings) {
	this.cgmzType = 'dot';
	this.target = target;
	this.id = id;
	this.scenes = scenes;
	this.updateFilterSettings(settings);
};
//-----------------------------------------------------------------------------
// Update filter settings
//-----------------------------------------------------------------------------
CGMZ_DotFilter.prototype.updateFilterSettings = function(settings) {
	this.scale = settings.scale;
	this.angle = settings.angle;
	this.grayscale = settings.grayscale;
};
//-----------------------------------------------------------------------------
// Get settings to edit for filter
//-----------------------------------------------------------------------------
CGMZ_DotFilter.prototype.getEditSettings = function() {
	return {scale: this.scale, angle: this.angle, grayscale: this.grayscale};
};
//=============================================================================
// CGMZ_EmbossFilter
//-----------------------------------------------------------------------------
// Store data for Emboss filters
//=============================================================================
function CGMZ_EmbossFilter() {
	this.initialize(...arguments);
}
//-----------------------------------------------------------------------------
// Initialize filter
//-----------------------------------------------------------------------------
CGMZ_EmbossFilter.prototype.initialize = function(id, target, scenes, settings) {
	this.cgmzType = 'emboss';
	this.target = target;
	this.id = id;
	this.scenes = scenes;
	this.updateFilterSettings(settings);
};
//-----------------------------------------------------------------------------
// Update filter settings
//-----------------------------------------------------------------------------
CGMZ_EmbossFilter.prototype.updateFilterSettings = function(settings) {
	this.strength = settings.strength;
};
//-----------------------------------------------------------------------------
// Get settings to edit for filter
//-----------------------------------------------------------------------------
CGMZ_EmbossFilter.prototype.getEditSettings = function() {
	return {strength: this.strength};
};
//=============================================================================
// CGMZ_GlitchFilter
//-----------------------------------------------------------------------------
// Store data for Glitch filters
//=============================================================================
function CGMZ_GlitchFilter() {
	this.initialize(...arguments);
}
//-----------------------------------------------------------------------------
// Initialize filter
//-----------------------------------------------------------------------------
CGMZ_GlitchFilter.prototype.initialize = function(id, target, scenes, settings) {
	this.cgmzType = 'glitch';
	this.target = target;
	this.id = id;
	this.scenes = scenes;
	this.updateFilterSettings(settings);
};
//-----------------------------------------------------------------------------
// Update the filter settings
//-----------------------------------------------------------------------------
CGMZ_GlitchFilter.prototype.updateFilterSettings = function(settings) {
	this.animating = settings.animating;
	this.seed = settings.seed;
	this.slices = settings.slices;
	this.offset = settings.offset;
	this.direction = settings.direction;
	this.fillMode = settings.fillMode;
	this.average = settings.average;
	this.minSize = settings.minSize;
	this.sampleSize = settings.sampleSize;
	this.red = [settings.red[0], settings.red[1]];
	this.blue = [settings.blue[0], settings.blue[1]];
	this.green = [settings.green[0], settings.green[1]];
};
//-----------------------------------------------------------------------------
// Get settings to edit for filter
//-----------------------------------------------------------------------------
CGMZ_GlitchFilter.prototype.getEditSettings = function() {
	return {
		animating: this.animating, seed: this.seed, slices: this.slices, offset: this.offset, direction: this.direction, fillMode: this.fillMode, average: this.average, minSize: this.minSize,
		sampleSize: this.sampleSize, red: JSON.parse(JSON.stringify(this.red)), blue: JSON.parse(JSON.stringify(this.blue)), green: JSON.parse(JSON.stringify(this.green))
	};
};
//=============================================================================
// CGMZ_GodrayFilter
//-----------------------------------------------------------------------------
// Store data for godray filters
//=============================================================================
function CGMZ_GodrayFilter() {
	this.initialize(...arguments);
}
//-----------------------------------------------------------------------------
// Initialize filter
//-----------------------------------------------------------------------------
CGMZ_GodrayFilter.prototype.initialize = function(id, target, scenes, settings) {
	this.cgmzType = 'godray';
	this.target = target;
	this.id = id;
	this.scenes = scenes;
	this.updateFilterSettings(settings);
};
//-----------------------------------------------------------------------------
// Update the filter settings
//-----------------------------------------------------------------------------
CGMZ_GodrayFilter.prototype.updateFilterSettings = function(settings) {
	this.animating = settings.animating;
	this.animationSpeed = settings.animationSpeed;
	this.alpha = settings.alpha;
	this.gain = settings.gain;
	this.lacunarity = settings.lacunarity;
	this.angle = settings.angle;
	this.parallel = settings.parallel;
	this.time = settings.time;
	this.center = JsonEx.parse(JsonEx.stringify(settings.center));
};
//-----------------------------------------------------------------------------
// Get settings to edit for filter
//-----------------------------------------------------------------------------
CGMZ_GodrayFilter.prototype.getEditSettings = function() {
	return {
		alpha: this.alpha, gain: this.gain, lacunarity: this.lacunarity, angle: this.angle, parallel: this.parallel, time: this.time, center: this.center,
		animating: this.animating, animationSpeed: this.animationSpeed
	};
};
//=============================================================================
// CGMZ_GrayscaleFilter
//-----------------------------------------------------------------------------
// Store data for Grayscale filters
//=============================================================================
function CGMZ_GrayscaleFilter() {
	this.initialize(...arguments);
}
//-----------------------------------------------------------------------------
// Initialize filter
//-----------------------------------------------------------------------------
CGMZ_GrayscaleFilter.prototype.initialize = function(id, target, scenes) {
	this.cgmzType = 'grayscale';
	this.target = target;
	this.id = id;
	this.scenes = scenes;
};
//=============================================================================
// CGMZ_KawaseBlurFilter
//-----------------------------------------------------------------------------
// Store data for Kawase Blur filters
//=============================================================================
function CGMZ_KawaseBlurFilter() {
	this.initialize(...arguments);
}
//-----------------------------------------------------------------------------
// Initialize filter
//-----------------------------------------------------------------------------
CGMZ_KawaseBlurFilter.prototype.initialize = function(id, target, scenes, settings) {
	this.cgmzType = 'kawaseblur';
	this.target = target;
	this.id = id;
	this.scenes = scenes;
	this.updateFilterSettings(settings);
};
//-----------------------------------------------------------------------------
// Update filter settings
//-----------------------------------------------------------------------------
CGMZ_KawaseBlurFilter.prototype.updateFilterSettings = function(settings) {
	this.blur = settings.blur;
	this.quality = settings.quality;
	this.clamp = settings.clamp; // readonly
	this.pixelSize = JSON.parse(JSON.stringify(settings.pixelSize));
};
//-----------------------------------------------------------------------------
// Get settings to edit for filter
//-----------------------------------------------------------------------------
CGMZ_KawaseBlurFilter.prototype.getEditSettings = function() {
	return {blur: this.blur, quality: this.quality, pixelSize: JSON.parse(JSON.stringify(this.pixelSize))};
};
//=============================================================================
// CGMZ_OldFilmFilter
//-----------------------------------------------------------------------------
// Store data for Old Film filters
//=============================================================================
function CGMZ_OldFilmFilter() {
	this.initialize(...arguments);
}
//-----------------------------------------------------------------------------
// Initialize filter
//-----------------------------------------------------------------------------
CGMZ_OldFilmFilter.prototype.initialize = function(id, target, scenes, settings) {
	this.cgmzType = 'oldfilm';
	this.target = target;
	this.id = id;
	this.scenes = scenes;
	this.updateFilterSettings(settings);
};
//-----------------------------------------------------------------------------
// Update the filter settings
//-----------------------------------------------------------------------------
CGMZ_OldFilmFilter.prototype.updateFilterSettings = function(settings) {
	this.animating = settings.animating;
	this.seed = settings.seed;
	this.sepia = settings.sepia;
	this.noise = settings.noise;
	this.noiseSize = settings.noiseSize;
	this.scratch = settings.scratch;
	this.scratchDensity = settings.scratchDensity;
	this.scratchWidth = settings.scratchWidth;
	this.vignetting = settings.vignetting;
	this.vignettingAlpha = settings.vignettingAlpha;
	this.vignettingBlur = settings.vignettingBlur;
};
//-----------------------------------------------------------------------------
// Get settings to edit for filter
//-----------------------------------------------------------------------------
CGMZ_OldFilmFilter.prototype.getEditSettings = function() {
	return {
		animating: this.animating, animationSpeed: this.animationSpeed, seed: this.seed, sepia: this.sepia, noise: this.noise, noiseSize: this.noiseSize,
		scratch: this.scratch, scratchDensity: this.scratchDensity, scratchWidth: this.scratchWidth, vignetting: this.vignetting, vignettingAlpha: this.vignettingAlpha,
		vignettingBlur: this.vignettingBlur
	};
};
//=============================================================================
// CGMZ_NoiseFilter
//-----------------------------------------------------------------------------
// Store data for Noise filters
//=============================================================================
function CGMZ_NoiseFilter() {
	this.initialize(...arguments);
}
//-----------------------------------------------------------------------------
// Initialize filter
//-----------------------------------------------------------------------------
CGMZ_NoiseFilter.prototype.initialize = function(id, target, scenes, settings) {
	this.cgmzType = 'noise';
	this.target = target;
	this.id = id;
	this.scenes = scenes;
	this.updateFilterSettings(settings);
};
//-----------------------------------------------------------------------------
// Update the filter settings
//-----------------------------------------------------------------------------
CGMZ_NoiseFilter.prototype.updateFilterSettings = function(settings) {
	this.animating = settings.animating;
	this.seed = settings.seed;
	this.noise = settings.noise;
};
//-----------------------------------------------------------------------------
// Get settings to edit for filter
//-----------------------------------------------------------------------------
CGMZ_NoiseFilter.prototype.getEditSettings = function() {
	return {animating: this.animating, seed: this.seed, noise: this.noise};
};
//=============================================================================
// CGMZ_PixelateFilter
//-----------------------------------------------------------------------------
// Store data for Pixelate filters
//=============================================================================
function CGMZ_PixelateFilter() {
	this.initialize(...arguments);
}
//-----------------------------------------------------------------------------
// Initialize filter
//-----------------------------------------------------------------------------
CGMZ_PixelateFilter.prototype.initialize = function(id, target, scenes, settings) {
	this.cgmzType = 'pixelate';
	this.target = target;
	this.id = id;
	this.scenes = scenes;
	this.updateFilterSettings(settings);
};
//-----------------------------------------------------------------------------
// Update filter settings
//-----------------------------------------------------------------------------
CGMZ_PixelateFilter.prototype.updateFilterSettings = function(settings) {
	this.size = JsonEx.parse(JsonEx.stringify(settings.size));
};
//-----------------------------------------------------------------------------
// Get settings to edit for filter
//-----------------------------------------------------------------------------
CGMZ_PixelateFilter.prototype.getEditSettings = function() {
	return {size: JsonEx.parse(JsonEx.stringify(this.size))};
};
//=============================================================================
// CGMZ_PixiFilterSettings
//-----------------------------------------------------------------------------
// Store filter settings
//=============================================================================
function CGMZ_PixiFilterSettings() {
	this.initialize(...arguments);
}
//-----------------------------------------------------------------------------
// Initialize settings
//-----------------------------------------------------------------------------
CGMZ_PixiFilterSettings.prototype.initialize = function(id, type, settings) {
	this.id = id;
	this.type = type;
	this.settings = this.createSettings(type, settings);
};
//-----------------------------------------------------------------------------
// Create the settings from parsed json object
//-----------------------------------------------------------------------------
CGMZ_PixiFilterSettings.prototype.createSettings = function(type, s) {
	const glitchFilterModeMap = {
		"Transparent": 0,
		"Original": 1,
		"Loop": 2,
		"Clamp": 3,
		"Mirror": 4
	};
	switch(type) {
		case 'Adjustment Filter': return {
			alpha: parseFloat(s.Alpha),
			blue: parseFloat(s.Blue),
			brightness: parseFloat(s.Brightness),
			contrast: parseFloat(s.Contrast),
			gamma: parseFloat(s.Gamma),
			green: parseFloat(s.Green),
			red: parseFloat(s.Red),
			saturation: parseFloat(s.Saturation)
		};
		case 'Advanced Bloom Filter': return {
			bloomScale: parseFloat(s["Bloom Scale"]),
			blur: parseFloat(s.Blur),
			brightness: parseFloat(s.Brightness),
			quality: parseFloat(s.Quality),
			threshold: parseFloat(s.Threshold)
		};
		case 'Alpha Filter': return {alpha: parseFloat(s.Alpha)};
		case 'Ascii Filter': return {size: parseFloat(s.Size)};
		case 'Bloom Filter': return {
			blur: parseFloat(s.Blur),
			blurX: parseFloat(s["Blur X"]),
			blurY: parseFloat(s["Blur Y"])
		};
		case 'Blur Filter': return {
			blur: parseFloat(s.Blur),
			quality: parseFloat(s.Quality)
		};
		case 'Bulge Pinch Filter': return {
			radius: Number(s.Radius),
			strength: Number(s.Strength),
			center: new Point(parseFloat(s["Center X"]), parseFloat(s["Center Y"]))
		};
		case 'Color Replace Filter': return {
			epsilon: parseFloat(s.Epsilon),
			originalColor: [parseFloat(s["Original Red"])/255.0, parseFloat(s["Original Green"])/255.0, parseFloat(s["Original Blue"])/255.0],
			newColor: [parseFloat(s["New Red"])/255.0, parseFloat(s["New Green"])/255.0, parseFloat(s["New Blue"])/255.0]
		};
		case 'CRT Filter': return {
			animating: s.Animating === 'true',
			animationSpeed: parseFloat(s["Animation Speed"]),
			curvature: parseFloat(s.Curvature),
			lineContrast: parseFloat(s["Line Contrast"]),
			lineWidth: parseFloat(s["Line Width"]),
			noise: parseFloat(s.Noise),
			noiseSize: parseFloat(s["Noise Size"]),
			seed: parseFloat(s.Seed),
			time: parseFloat(s.Time),
			verticalLine: s["Vertical Line"] === 'true',
			vignetting: parseFloat(s.Vignetting),
			vignettingAlpha: parseFloat(s["Vignetting Alpha"]),
			vignettingBlur: parseFloat(s["Vignetting Blur"])
		};
		case 'Displacement Filter': return {
			map: s.Map,
			scale: new Point(parseFloat(s["Scale X"]), parseFloat(s["Scale Y"])),
			animating: (s["Animating"] === 'true'),
			xScroll: Number(s["X Scroll"]),
			yScroll: Number(s["Y Scroll"])
		};
		case 'Dot Filter': return {
			angle: parseFloat(s.Angle),
			grayscale: (s.Grayscale === 'true'),
			scale: parseFloat(s.Scale)
		};
		case 'Emboss Filter': return {strength: parseFloat(s.Strength)};
		case 'Glitch Filter': return {
			animating: s.Animating === 'true',
			average: s.Average === 'true',
			blue: [parseFloat(s["Blue X"]), parseFloat(s["Blue Y"])],
			direction: parseFloat(s.Direction),
			fillMode: glitchFilterModeMap[s["Fill Mode"]],
			green: [parseFloat(s["Green X"]), parseFloat(s["Green Y"])],
			minSize: parseFloat(s["Min Size"]),
			offset: parseFloat(s.Offset),
			red: [parseFloat(s["Red X"]), parseFloat(s["Red Y"])],
			sampleSize: parseFloat(s["Sample Size"]),
			seed: parseFloat(s["Seed"]),
			slices: parseFloat(s.Slices)
		};
		case 'Godray Filter': return {
			angle: parseFloat(s.Angle),
			alpha: parseFloat(s.Alpha),
			animating: s.Animating === 'true',
			animationSpeed: parseFloat(s["Animation Speed"]),
			center: new Point(parseFloat(s.X), parseFloat(s.Y)),
			gain: parseFloat(s.Gain),
			lacunarity: parseFloat(s.Lacunarity),
			parallel: s.Parallel === 'true',
			time: parseFloat(s.Time)
		};
		case 'Kawase Blur Filter': return {
			blur: parseFloat(s.Blur),
			clamp: s.Clamp === 'true',
			pixelSize: [parseFloat(s["Pixel Size X"]), parseFloat(s["Pixel Size Y"])],
			quality: parseFloat(s.Quality)
		};
		case 'Old Film Filter': return {
			animating: (s.Animating === 'true'),
			noise: parseFloat(s.Noise),
			noiseSize: parseFloat(s["Noise Size"]),
			scratch: parseFloat(s.Scratch),
			scratchDensity: parseFloat(s["Scratch Density"]),
			scratchWidth: parseFloat(s["Scratch Width"]),
			seed: parseFloat(s.Seed),
			sepia: parseFloat(s.Sepia),
			vignetting: parseFloat(s.Vignetting),
			vignettingAlpha: parseFloat(s["Vignetting Alpha"]),
			vignettingBlur: parseFloat(s["Vignetting Blur"])
		};
		case 'Noise Filter': return {
			animating: (s.Animating === 'true'),
			noise: parseFloat(s.Noise),
			seed: parseFloat(s.Seed)
		};
		case 'Pixelate Filter': return {size: new Point(parseFloat(s["Size X"]), parseFloat(s["Size Y"]))}
	}
	return {};
};
//=============================================================================
// CGMZ_Temp
//-----------------------------------------------------------------------------
// Add temp filter data, plugin Commands for filters
//=============================================================================
//-----------------------------------------------------------------------------
// Also add check for if pixi filter has changed
//-----------------------------------------------------------------------------
const alias_CGMZ_PixiFilters_CGMZ_Temp_createPluginData = CGMZ_Temp.prototype.createPluginData;
CGMZ_Temp.prototype.createPluginData = function() {
	alias_CGMZ_PixiFilters_CGMZ_Temp_createPluginData.call(this);
	this.initializePixiFilterData();
	this.clearPixiFilterRequests();
};
//-----------------------------------------------------------------------------
// Clear pixi filter requests
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.clearPixiFilterRequests = function() {
	this._pixiFiltersAdded = [];
	this._pixiFiltersChanged = [];
	this._pixiFiltersRemoved = [];
};
//-----------------------------------------------------------------------------
// Initialize any pixi filter data needed
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.initializePixiFilterData = function() {
	this._filterSettings = {};
	this.initializePixiFilterSettingArray(CGMZ.PixiFilters.AdjustmentSettings, "Adjustment Filter");
	this.initializePixiFilterSettingArray(CGMZ.PixiFilters.AdvancedBloomSettings, "Advanced Bloom Filter");
	this.initializePixiFilterSettingArray(CGMZ.PixiFilters.AlphaSettings, "Alpha Filter");
	this.initializePixiFilterSettingArray(CGMZ.PixiFilters.AsciiSettings, "Ascii Filter");
	this.initializePixiFilterSettingArray(CGMZ.PixiFilters.BloomSettings, "Bloom Filter");
	this.initializePixiFilterSettingArray(CGMZ.PixiFilters.BlurSettings, "Blur Filter");
	this.initializePixiFilterSettingArray(CGMZ.PixiFilters.BulgePinchSettings, "Bulge Pinch Filter");
	this.initializePixiFilterSettingArray(CGMZ.PixiFilters.ColorReplaceSettings, "Color Replace Filter");
	this.initializePixiFilterSettingArray(CGMZ.PixiFilters.CRTSettings, "CRT Filter");
	this.initializePixiFilterSettingArray(CGMZ.PixiFilters.DisplacementSettings, "Displacement Filter");
	this.initializePixiFilterSettingArray(CGMZ.PixiFilters.DotSettings, "Dot Filter");
	this.initializePixiFilterSettingArray(CGMZ.PixiFilters.EmbossSettings, "Emboss Filter");
	this.initializePixiFilterSettingArray(CGMZ.PixiFilters.GlitchSettings, "Glitch Filter");
	this.initializePixiFilterSettingArray(CGMZ.PixiFilters.GodraySettings, "Godray Filter");
	this.initializePixiFilterSettingArray(CGMZ.PixiFilters.KawaseBlurSettings, "Kawase Blur Filter");
	this.initializePixiFilterSettingArray(CGMZ.PixiFilters.OldFilmSettings, "Old Film Filter");
	this.initializePixiFilterSettingArray(CGMZ.PixiFilters.NoiseSettings, "Noise Filter");
	this.initializePixiFilterSettingArray(CGMZ.PixiFilters.PixelateSettings, "Pixelate Filter");
};
//-----------------------------------------------------------------------------
// Initialize array of pixi filter's settings objects from a json array
// This is mostly only used for convenience in the above function initializePixiFilterData
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.initializePixiFilterSettingArray = function(jsonArray, type) {
	for(const json of jsonArray) {
		const s = CGMZ_Utils.parseJSON(json, null, '[CGMZ] Pixi Filters', `One of your ${type} settings was invalid and could not be read.`);
		if(!s) continue;
		const id = s["Settings Id"];
		this._filterSettings[id] = new CGMZ_PixiFilterSettings(id, type, s);
	}
};
//-----------------------------------------------------------------------------
// Creates a filter object from json
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.createFilterSettingsFromJSON = function(json, type) {
	const s = CGMZ_Utils.parseJSON(json, null, '[CGMZ] Pixi Filters', `Failed to create filter settings for filter type: ${type}`);
	if(!s) return null;
	const id = s["Settings Id"];
	return new CGMZ_PixiFilterSettings(id, type, s);
};
//-----------------------------------------------------------------------------
// Creates an actual pixi filter object, or null if not possible
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.createPixiFilterObject = function(filter) {
	let f = null;
	switch(filter.cgmzType) {
		case 'adjustment': f = new PIXI.filters.AdjustmentFilter(filter.getEditSettings()); break;
		case 'advancedbloom': f = new PIXI.filters.AdvancedBloomFilter(filter.getEditSettings()); break;
		case 'alpha': f = new PIXI.filters.AlphaFilter(filter.alpha); break;
		case 'ascii': f = new PIXI.filters.AsciiFilter(filter.size); break;
		case 'blur': f = new PIXI.filters.BlurFilter();
			f.blur = filter.blur;
			f.quality = filter.quality;
			break;
		case 'bulgepinch': f = new PIXI.filters.BulgePinchFilter(filter.getEditSettings()); break;
		case 'bloom': f = new PIXI.filters.BloomFilter(filter.getEditSettings()); break;
		case 'colorreplace': f = new PIXI.filters.ColorReplaceFilter(); f.epsilon = filter.epsilon; f.originalColor = filter.originalColor; f.newColor = filter.newColor; break;
		case 'crosshatch': f = new PIXI.filters.CrossHatchFilter(); break;
		case 'crt': f = new PIXI.filters.CRTFilter(filter.getEditSettings());
			f.animating = filter.animating;
			f.animationSpeed = filter.animationSpeed;
			break;
		case 'displacement': 
			const sprite = filter.getDisplacementTexture();
			sprite.filterCGMZId = filter.id;
			SceneManager._scene.CGMZ_addDisplacementMapFilterSprite(sprite);
			f = new PIXI.filters.DisplacementFilter(sprite);
			f.scale = filter.scale;
			f.xScroll = filter.xScroll;
			f.yScroll = filter.yScroll;
			f.animating = filter.animating;
			break;
		case 'dot': f = new PIXI.filters.DotFilter(filter.scale, filter.angle, filter.grayscale); break;
		case 'emboss': f = new PIXI.filters.EmbossFilter(filter.strength); break;
		case 'glitch': f = new PIXI.filters.GlitchFilter(filter.getEditSettings());
			f.animating = filter.animating;
			break;
		case 'godray': f = new PIXI.filters.GodrayFilter(filter.getEditSettings());
			f.animating = filter.animating;
			f.animationSpeed = filter.animationSpeed;
			break;
		case 'grayscale': f = new PIXI.filters.GrayscaleFilter(); break;
		case 'kawaseblur': f = new PIXI.filters.KawaseBlurFilter(filter.blur, filter.quality, filter.clamp);
			f.pixelSize = JSON.parse(JSON.stringify(filter.pixelSize));
			break;
		case 'oldfilm': f = new PIXI.filters.OldFilmFilter(filter.getEditSettings()); break;
		case 'noise': f = new PIXI.filters.NoiseFilter(filter.noise, filter.seed);
			f.animating = filter.animating;
			break;
		case 'pixelate': f = new PIXI.filters.PixelateFilter(JsonEx.parse(JsonEx.stringify(filter.size))); break;
	}
	f.cgmzType = filter.cgmzType;
	return f;
};
//-----------------------------------------------------------------------------
// Creates a new cgmz pixi filter data object from setting id or filtertype (if filter has no settings)
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.createCGMZPixiFilterFromPreset = function(settingsId, filterType = "") {
	let filterClass = null;
	const preset = $cgmzTemp.getPixiFilterSettings(settingsId);
	const noSettings = ["CrossHatch", "Grayscale"];
	if(!preset && !noSettings.includes(filterType)) return null;
	if(!filterType) {
		switch(preset.type) {
			case 'Adjustment Filter': filterClass = CGMZ_AdjustmentFilter; break;
			case 'Advanced Bloom Filter': filterClass = CGMZ_AdvancedBloomFilter; break;
			case 'Alpha Filter': filterClass = CGMZ_AlphaFilter; break;
			case 'Ascii Filter': filterClass = CGMZ_AsciiFilter; break;
			case 'Bloom Filter': filterClass = CGMZ_BloomFilter; break;
			case 'Blur Filter': filterClass = CGMZ_BlurFilter; break;
			case 'Bulge Pinch Filter': filterClass = CGMZ_BulgePinchFilter; break;
			case 'Color Replace Filter': filterClass = CGMZ_ColorReplaceFilter; break;
			case 'CRT Filter': filterClass = CGMZ_CRTFilter; break;
			case 'Displacement Filter': filterClass = CGMZ_DisplacementFilter; break;
			case 'Dot Filter': filterClass = CGMZ_DotFilter; break;
			case 'Emboss Filter': filterClass = CGMZ_EmbossFilter; break;
			case 'Glitch Filter': filterClass = CGMZ_GlitchFilter; break;
			case 'Godray Filter': filterClass = CGMZ_GodrayFilter; break;
			case 'Kawase Blur Filter': filterClass = CGMZ_KawaseBlurFilter; break;
			case 'Old Film Filter': filterClass = CGMZ_OldFilmFilter; break;
			case 'Noise Filter': filterClass = CGMZ_NoiseFilter; break;
			case 'Pixelate Filter': filterClass = CGMZ_PixelateFilter; break;
		}
	} else {
		switch(filterType) {
			case 'CrossHatch': filterClass = CGMZ_CrossHatchFilter; break;
			case 'Grayscale': filterClass = CGMZ_GrayscaleFilter; break;
		}
	}
	if(!filterClass) return null;
	return new filterClass("cgmzInternalFilter", null, [], preset?.settings);
};
//-----------------------------------------------------------------------------
// Get a pixi filter's settings
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.getPixiFilterSettings = function(id) {
	return this._filterSettings[id];
};
//-----------------------------------------------------------------------------
// Request to add a pixi filter
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.requestAddPixiFilter = function(filter) {
	this._pixiFiltersAdded.push(filter);
};
//-----------------------------------------------------------------------------
// Get a pixi filter add request and remove it from the queue
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.getRequestedPixiFilterAdd = function() {
	return this._pixiFiltersAdded.pop();
};
//-----------------------------------------------------------------------------
// Request to edit a pixi filter
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.requestEditPixiFilter = function(id) {
	this._pixiFiltersChanged.push(id);
};
//-----------------------------------------------------------------------------
// Get a pixi filter edit request and remove it from the queue
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.getRequestedPixiFilterEdit = function() {
	return this._pixiFiltersChanged.pop();
};
//-----------------------------------------------------------------------------
// Request to remove a pixi filter
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.requestRemovePixiFilter = function(id) {
	this._pixiFiltersRemoved.push(id);
};
//-----------------------------------------------------------------------------
// Get a pixi filter removal request and remove it from the queue
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.getRequestedPixiFilterRemoval = function() {
	return this._pixiFiltersRemoved.pop();
};
//-----------------------------------------------------------------------------
// Also register Filter Plugin Commands
//-----------------------------------------------------------------------------
const alias_CGMZ_PixiFilters_CGMZ_Temp_registerPluginCommands = CGMZ_Temp.prototype.registerPluginCommands;
CGMZ_Temp.prototype.registerPluginCommands = function() {
	alias_CGMZ_PixiFilters_CGMZ_Temp_registerPluginCommands.call(this);
	PluginManager.registerCommand("CGMZ_PixiFilters", "Create Adjustment Filter", this.pluginCommandPixiFiltersCreateAdjustmentFilter);
	PluginManager.registerCommand("CGMZ_PixiFilters", "Create Advanced Bloom Filter", this.pluginCommandPixiFiltersCreateAdvancedBloomFilter);
	PluginManager.registerCommand("CGMZ_PixiFilters", "Create Alpha Filter", this.pluginCommandPixiFiltersCreateAlphaFilter);
	PluginManager.registerCommand("CGMZ_PixiFilters", "Create Ascii Filter", this.pluginCommandPixiFiltersCreateAsciiFilter);
	PluginManager.registerCommand("CGMZ_PixiFilters", "Create Bloom Filter", this.pluginCommandPixiFiltersCreateBloomFilter);
	PluginManager.registerCommand("CGMZ_PixiFilters", "Create Blur Filter", this.pluginCommandPixiFiltersCreateBlurFilter);
	PluginManager.registerCommand("CGMZ_PixiFilters", "Create Bulge Pinch Filter", this.pluginCommandPixiFiltersCreateBulgePinchFilter);
	PluginManager.registerCommand("CGMZ_PixiFilters", "Create Color Replace Filter", this.pluginCommandPixiFiltersCreateColorReplaceFilter);
	PluginManager.registerCommand("CGMZ_PixiFilters", "Create CrossHatch Filter", this.pluginCommandPixiFiltersCreateCrossHatchFilter);
	PluginManager.registerCommand("CGMZ_PixiFilters", "Create CRT Filter", this.pluginCommandPixiFiltersCreateCRTFilter);
	PluginManager.registerCommand("CGMZ_PixiFilters", "Create Displacement Filter", this.pluginCommandPixiFiltersCreateDisplacementFilter);
	PluginManager.registerCommand("CGMZ_PixiFilters", "Create Dot Filter", this.pluginCommandPixiFiltersCreateDotFilter);
	PluginManager.registerCommand("CGMZ_PixiFilters", "Create Emboss Filter", this.pluginCommandPixiFiltersCreateEmbossFilter);
	PluginManager.registerCommand("CGMZ_PixiFilters", "Create Glitch Filter", this.pluginCommandPixiFiltersCreateGlitchFilter);
	PluginManager.registerCommand("CGMZ_PixiFilters", "Create Godray Filter", this.pluginCommandPixiFiltersCreateGodrayFilter);
	PluginManager.registerCommand("CGMZ_PixiFilters", "Create Grayscale Filter", this.pluginCommandPixiFiltersCreateGrayscaleFilter);
	PluginManager.registerCommand("CGMZ_PixiFilters", "Create Kawase Blur Filter", this.pluginCommandPixiFiltersCreateKawaseBlurFilter);
	PluginManager.registerCommand("CGMZ_PixiFilters", "Create Noise Filter", this.pluginCommandPixiFiltersCreateNoiseFilter);
	PluginManager.registerCommand("CGMZ_PixiFilters", "Create Old Film Filter", this.pluginCommandPixiFiltersCreateOldFilmFilter);
	PluginManager.registerCommand("CGMZ_PixiFilters", "Create Pixelate Filter", this.pluginCommandPixiFiltersCreatePixelateFilter);
	PluginManager.registerCommand("CGMZ_PixiFilters", "Edit Adjustment Filter", this.pluginCommandPixiFiltersEditAdjustmentFilter);
	PluginManager.registerCommand("CGMZ_PixiFilters", "Edit Advanced Bloom Filter", this.pluginCommandPixiFiltersEditAdvancedBloomFilter);
	PluginManager.registerCommand("CGMZ_PixiFilters", "Edit Alpha Filter", this.pluginCommandPixiFiltersEditAlphaFilter);
	PluginManager.registerCommand("CGMZ_PixiFilters", "Edit Ascii Filter", this.pluginCommandPixiFiltersEditAsciiFilter);
	PluginManager.registerCommand("CGMZ_PixiFilters", "Edit Bloom Filter", this.pluginCommandPixiFiltersEditBloomFilter);
	PluginManager.registerCommand("CGMZ_PixiFilters", "Edit Blur Filter", this.pluginCommandPixiFiltersEditBlurFilter);
	PluginManager.registerCommand("CGMZ_PixiFilters", "Edit Bulge Pinch Filter", this.pluginCommandPixiFiltersEditBulgePinchFilter);
	PluginManager.registerCommand("CGMZ_PixiFilters", "Edit Color Replace Filter", this.pluginCommandPixiFiltersEditColorReplaceFilter);
	PluginManager.registerCommand("CGMZ_PixiFilters", "Edit CRT Filter", this.pluginCommandPixiFiltersEditCRTFilter);
	PluginManager.registerCommand("CGMZ_PixiFilters", "Edit Dot Filter", this.pluginCommandPixiFiltersEditDotFilter);
	PluginManager.registerCommand("CGMZ_PixiFilters", "Edit Displacement Filter", this.pluginCommandPixiFiltersEditDisplacementFilter);
	PluginManager.registerCommand("CGMZ_PixiFilters", "Edit Emboss Filter", this.pluginCommandPixiFiltersEditEmbossFilter);
	PluginManager.registerCommand("CGMZ_PixiFilters", "Edit Glitch Filter", this.pluginCommandPixiFiltersEditGlitchFilter);
	PluginManager.registerCommand("CGMZ_PixiFilters", "Edit Godray Filter", this.pluginCommandPixiFiltersEditGodrayFilter);
	PluginManager.registerCommand("CGMZ_PixiFilters", "Edit Kawase Blur Filter", this.pluginCommandPixiFiltersEditKawaseBlurFilter);
	PluginManager.registerCommand("CGMZ_PixiFilters", "Edit Noise Filter", this.pluginCommandPixiFiltersEditNoiseFilter);
	PluginManager.registerCommand("CGMZ_PixiFilters", "Edit Old Film Filter", this.pluginCommandPixiFiltersEditOldFilmFilter);
	PluginManager.registerCommand("CGMZ_PixiFilters", "Edit Pixelate Filter", this.pluginCommandPixiFiltersEditPixelateFilter);
	PluginManager.registerCommand("CGMZ_PixiFilters", "Remove Filter", this.pluginCommandPixiFiltersRemoveFilter);
	PluginManager.registerCommand("CGMZ_PixiFilters", "Remove All Filters", this.pluginCommandPixiFiltersRemoveAllFilters);
};
//-----------------------------------------------------------------------------
// Plugin Command - Remove filter
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPixiFiltersRemoveFilter = function(args) {
	$cgmz.removePixiFilterById(args["Filter Id"]);
};
//-----------------------------------------------------------------------------
// Plugin Command - Remove all filters
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPixiFiltersRemoveAllFilters = function() {
	$cgmz.removeAllPixiFilters();
};
//-----------------------------------------------------------------------------
// Plugin Command - Create adjustment filter
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPixiFiltersCreateAdjustmentFilter = function(args) {
	$cgmzTemp.pluginCommandPixiFiltersHelperCreate("Adjustment Filter", CGMZ_AdjustmentFilter, args);
};
//-----------------------------------------------------------------------------
// Plugin Command - Edit adjustment filter
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPixiFiltersEditAdjustmentFilter = function(args) {
	$cgmzTemp.pluginCommandPixiFiltersHelperEdit("Adjustment Filter", args);
};
//-----------------------------------------------------------------------------
// Plugin Command - Create advanced bloom filter
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPixiFiltersCreateAdvancedBloomFilter = function(args) {
	$cgmzTemp.pluginCommandPixiFiltersHelperCreate("Advanced Bloom Filter", CGMZ_AdvancedBloomFilter, args);
};
//-----------------------------------------------------------------------------
// Plugin Command - Edit advanced bloom filter
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPixiFiltersEditAdvancedBloomFilter = function(args) {
	$cgmzTemp.pluginCommandPixiFiltersHelperEdit("Advanced Bloom Filter", args);
};
//-----------------------------------------------------------------------------
// Plugin Command - Create alpha filter
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPixiFiltersCreateAlphaFilter = function(args) {
	$cgmzTemp.pluginCommandPixiFiltersHelperCreate("Alpha Filter", CGMZ_AlphaFilter, args);
};
//-----------------------------------------------------------------------------
// Plugin Command - Edit alpha filter
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPixiFiltersEditAlphaFilter = function(args) {
	$cgmzTemp.pluginCommandPixiFiltersHelperEdit("Alpha Filter", args);
};
//-----------------------------------------------------------------------------
// Plugin Command - Create ascii filter
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPixiFiltersCreateAsciiFilter = function(args) {
	$cgmzTemp.pluginCommandPixiFiltersHelperCreate("Ascii Filter", CGMZ_AsciiFilter, args);
};
//-----------------------------------------------------------------------------
// Plugin Command - Edit ascii filter
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPixiFiltersEditAsciiFilter = function(args) {
	$cgmzTemp.pluginCommandPixiFiltersHelperEdit("Ascii Filter", args);
};
//-----------------------------------------------------------------------------
// Plugin Command - Create bloom filter
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPixiFiltersCreateBloomFilter = function(args) {
	$cgmzTemp.pluginCommandPixiFiltersHelperCreate("Bloom Filter", CGMZ_BloomFilter, args);
};
//-----------------------------------------------------------------------------
// Plugin Command - Edit bloom filter
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPixiFiltersEditBloomFilter = function(args) {
	$cgmzTemp.pluginCommandPixiFiltersHelperEdit("Bloom Filter", args);
};
//-----------------------------------------------------------------------------
// Plugin Command - Create blur filter
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPixiFiltersCreateBlurFilter = function(args) {
	$cgmzTemp.pluginCommandPixiFiltersHelperCreate("Blur Filter", CGMZ_BlurFilter, args);
};
//-----------------------------------------------------------------------------
// Plugin Command - Edit blur filter
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPixiFiltersEditBlurFilter = function(args) {
	$cgmzTemp.pluginCommandPixiFiltersHelperEdit("Blur Filter", args);
};
//-----------------------------------------------------------------------------
// Plugin Command - Create bulge pinch filter
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPixiFiltersCreateBulgePinchFilter = function(args) {
	$cgmzTemp.pluginCommandPixiFiltersHelperCreate("Bulge Pinch Filter", CGMZ_BulgePinchFilter, args);
};
//-----------------------------------------------------------------------------
// Plugin Command - Edit bulge pinch filter
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPixiFiltersEditBulgePinchFilter = function(args) {
	$cgmzTemp.pluginCommandPixiFiltersHelperEdit("Bulge Pinch Filter", args);
};
//-----------------------------------------------------------------------------
// Plugin Command - Create color replace filter
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPixiFiltersCreateColorReplaceFilter = function(args) {
	$cgmzTemp.pluginCommandPixiFiltersHelperCreate("Color Replace Filter", CGMZ_ColorReplaceFilter, args);
};
//-----------------------------------------------------------------------------
// Plugin Command - Edit color replace filter
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPixiFiltersEditColorReplaceFilter = function(args) {
	$cgmzTemp.pluginCommandPixiFiltersHelperEdit("Color Replace Filter", args);
};
//-----------------------------------------------------------------------------
// Plugin Command - Create crosshatch filter
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPixiFiltersCreateCrossHatchFilter = function(args) {
	$cgmzTemp.pluginCommandPixiFiltersHelperCreateNoSettings("CrossHatch Filter", CGMZ_CrossHatchFilter, args);
};
//-----------------------------------------------------------------------------
// Plugin Command - Create CRT filter
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPixiFiltersCreateCRTFilter = function(args) {
	$cgmzTemp.pluginCommandPixiFiltersHelperCreate("CRT Filter", CGMZ_CRTFilter, args);
};
//-----------------------------------------------------------------------------
// Plugin Command - Edit CRT filter
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPixiFiltersEditCRTFilter = function(args) {
	$cgmzTemp.pluginCommandPixiFiltersHelperEdit("CRT Filter", args);
};
//-----------------------------------------------------------------------------
// Plugin Command - Create displacement filter
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPixiFiltersCreateDisplacementFilter = function(args) {
	$cgmzTemp.pluginCommandPixiFiltersHelperCreate("Displacement Filter", CGMZ_DisplacementFilter, args);
};
//-----------------------------------------------------------------------------
// Plugin Command - Edit displacement filter
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPixiFiltersEditDisplacementFilter = function(args) {
	$cgmzTemp.pluginCommandPixiFiltersHelperEdit("Displacement Filter", args);
};
//-----------------------------------------------------------------------------
// Plugin Command - Create dot filter
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPixiFiltersCreateDotFilter = function(args) {
	$cgmzTemp.pluginCommandPixiFiltersHelperCreate("Dot Filter", CGMZ_DotFilter, args);
};
//-----------------------------------------------------------------------------
// Plugin Command - Edit dot filter
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPixiFiltersEditDotFilter = function(args) {
	$cgmzTemp.pluginCommandPixiFiltersHelperEdit("Dot Filter", args);
};
//-----------------------------------------------------------------------------
// Plugin Command - Create emboss filter
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPixiFiltersCreateEmbossFilter = function(args) {
	$cgmzTemp.pluginCommandPixiFiltersHelperCreate("Emboss Filter", CGMZ_EmbossFilter, args);
};
//-----------------------------------------------------------------------------
// Plugin Command - Edit emboss filter
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPixiFiltersEditEmbossFilter = function(args) {
	$cgmzTemp.pluginCommandPixiFiltersHelperEdit("Emboss Filter", args);
};
//-----------------------------------------------------------------------------
// Plugin Command - Create Glitch filter
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPixiFiltersCreateGlitchFilter = function(args) {
	$cgmzTemp.pluginCommandPixiFiltersHelperCreate("Glitch Filter", CGMZ_GlitchFilter, args);
};
//-----------------------------------------------------------------------------
// Plugin Command - Edit Glitch filter
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPixiFiltersEditGlitchFilter = function(args) {
	$cgmzTemp.pluginCommandPixiFiltersHelperEdit("Glitch Filter", args);
};
//-----------------------------------------------------------------------------
// Plugin Command - Create godray filter
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPixiFiltersCreateGodrayFilter = function(args) {
	$cgmzTemp.pluginCommandPixiFiltersHelperCreate("Godray Filter", CGMZ_GodrayFilter, args);
};
//-----------------------------------------------------------------------------
// Plugin Command - Edit godray filter
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPixiFiltersEditGodrayFilter = function(args) {
	$cgmzTemp.pluginCommandPixiFiltersHelperEdit("Godray Filter", args);
};
//-----------------------------------------------------------------------------
// Plugin Command - Create grayscale filter
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPixiFiltersCreateGrayscaleFilter = function(args) {
	$cgmzTemp.pluginCommandPixiFiltersHelperCreateNoSettings("Grayscale Filter", CGMZ_GrayscaleFilter, args);
};
//-----------------------------------------------------------------------------
// Plugin Command - Create kawase blur filter
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPixiFiltersCreateKawaseBlurFilter = function(args) {
	$cgmzTemp.pluginCommandPixiFiltersHelperCreate("Kawase Blur Filter", CGMZ_KawaseBlurFilter, args);
};
//-----------------------------------------------------------------------------
// Plugin Command - Edit kawase blur filter
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPixiFiltersEditKawaseBlurFilter = function(args) {
	$cgmzTemp.pluginCommandPixiFiltersHelperEdit("Kawase Blur Filter", args);
};
//-----------------------------------------------------------------------------
// Plugin Command - Create Noise filter
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPixiFiltersCreateNoiseFilter = function(args) {
	$cgmzTemp.pluginCommandPixiFiltersHelperCreate("Noise Filter", CGMZ_NoiseFilter, args);
};
//-----------------------------------------------------------------------------
// Plugin Command - Edit Noise filter
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPixiFiltersEditNoiseFilter = function(args) {
	$cgmzTemp.pluginCommandPixiFiltersHelperEdit("Noise Filter", args);
};
//-----------------------------------------------------------------------------
// Plugin Command - Create Old Film filter
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPixiFiltersCreateOldFilmFilter = function(args) {
	$cgmzTemp.pluginCommandPixiFiltersHelperCreate("Old Film Filter", CGMZ_OldFilmFilter, args);
};
//-----------------------------------------------------------------------------
// Plugin Command - Edit Old Film filter
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPixiFiltersEditOldFilmFilter = function(args) {
	$cgmzTemp.pluginCommandPixiFiltersHelperEdit("Old Film Filter", args);
};
//-----------------------------------------------------------------------------
// Plugin Command - Create pixelate filter
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPixiFiltersCreatePixelateFilter = function(args) {
	$cgmzTemp.pluginCommandPixiFiltersHelperCreate("Pixelate Filter", CGMZ_PixelateFilter, args);
};
//-----------------------------------------------------------------------------
// Plugin Command - Edit pixelate filter
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPixiFiltersEditPixelateFilter = function(args) {
	$cgmzTemp.pluginCommandPixiFiltersHelperEdit("Pixelate Filter", args);
};
//-----------------------------------------------------------------------------
// Plugin Command Helper - Create filter
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPixiFiltersHelperCreate = function(filterType, filterClass, args) {
	const id = args["Filter Id"];
	const targetingOptions = CGMZ_Utils.parseJSON(args["Targeting Options"], null, "[CGMZ] Pixi Filters", `Your Create ${filterType} plugin command had invalid targeting options and could not be created`);
	if(!targetingOptions) return;
	const target = targetingOptions.Target;
	const scenes = CGMZ_Utils.parseJSON(targetingOptions.Scene, [], "[CGMZ] Pixi Filters", `Your Create ${filterType} plugin command had invalid scenes and could not be created`);
	if(scenes.length === 0) return;
	const settings = $cgmzTemp.createFilterSettingsFromJSON(args.Settings, filterType);
	const preset = $cgmzTemp.getPixiFilterSettings(settings.id);
	const filter = new filterClass(id, target, scenes, (preset) ? preset.settings : settings.settings);
	$cgmz.addPixiFilter(filter);
};
//-----------------------------------------------------------------------------
// Plugin Command Helper - Create filter (no settings needed)
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPixiFiltersHelperCreateNoSettings = function(filterType, filterClass, args) {
	const id = args["Filter Id"];
	const targetingOptions = CGMZ_Utils.parseJSON(args["Targeting Options"], null, "[CGMZ] Pixi Filters", `Your Create ${filterType} plugin command had invalid targeting options and could not be created`);
	if(!targetingOptions) return;
	const target = targetingOptions.Target;
	const scenes = CGMZ_Utils.parseJSON(targetingOptions.Scene, [], "[CGMZ] Pixi Filters", `Your Create ${filterType} plugin command had invalid scenes and could not be created`);
	if(scenes.length === 0) return;
	const filter = new filterClass(id, target, scenes);
	$cgmz.addPixiFilter(filter);
};
//-----------------------------------------------------------------------------
// Plugin Command Helper - Edit filter
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandPixiFiltersHelperEdit = function(filterType, args) {
	const id = args["Filter Id"];
	const settings = $cgmzTemp.createFilterSettingsFromJSON(args.Settings, filterType);
	const preset = $cgmzTemp.getPixiFilterSettings(settings.id);
	const s = (preset) ? preset.settings : settings.settings;
	const filter = $cgmz.getPixiFilterById(id);
	if(filter) {
		filter.updateFilterSettings(s);
		$cgmzTemp.requestEditPixiFilter(id);
	}
};
//=============================================================================
// CGMZ_Core
//-----------------------------------------------------------------------------
// Store filter data
//=============================================================================
//-----------------------------------------------------------------------------
// Also create filter data
//-----------------------------------------------------------------------------
const alias_CGMZ_PixiFilters_CGMZ_Core_createPluginData = CGMZ_Core.prototype.createPluginData;
CGMZ_Core.prototype.createPluginData = function() {
	alias_CGMZ_PixiFilters_CGMZ_Core_createPluginData.call(this);
	this.initializePixiFilters();
};
//-----------------------------------------------------------------------------
// Initialize Pixi Filter Data
//-----------------------------------------------------------------------------
CGMZ_Core.prototype.initializePixiFilters = function() {
	this.pixifilters = {};
};
//-----------------------------------------------------------------------------
// Check if new filter arrays need to be created
//-----------------------------------------------------------------------------
const alias_CGMZ_PixiFilters_CGMZ_Core_onAfterLoad = CGMZ_Core.prototype.onAfterLoad;
CGMZ_Core.prototype.onAfterLoad = function() {
	alias_CGMZ_PixiFilters_CGMZ_Core_onAfterLoad.call(this);
	if(!this.pixifilters) this.pixifilters = {};
};
//-----------------------------------------------------------------------------
// Get all pixi filters
//-----------------------------------------------------------------------------
CGMZ_Core.prototype.getAllPixiFilters = function() {
	return Object.values(this.pixifilters);
};
//-----------------------------------------------------------------------------
// Determine if filter is valid & unique id is not already in use
//-----------------------------------------------------------------------------
CGMZ_Core.prototype.canAddPixiFilter = function(filter) {
	if(!filter.id) {
		console.warn("[CGMZ] Pixi Filters - Could not create PIXI Filter: Filter missing id!");
		return false;
	}
	if(this.pixifilters[filter.id]) {
		console.warn("[CGMZ] Pixi Filters - Could not create PIXI Filter: Filter already exists!");
		return false;
	}
	return true;
};
//-----------------------------------------------------------------------------
// Add a Pixi Filter
//-----------------------------------------------------------------------------
CGMZ_Core.prototype.addPixiFilter = function(filter) {
	if(this.canAddPixiFilter(filter)) {
		this.pixifilters[filter.id] = filter;
		$cgmzTemp.requestAddPixiFilter(filter);
	}
};
//-----------------------------------------------------------------------------
// Get a Pixi Filter by Id within a filter type
//-----------------------------------------------------------------------------
CGMZ_Core.prototype.getPixiFilterById = function(id) {
	return this.pixifilters[id];
};
//-----------------------------------------------------------------------------
// Remove a filter by id
//-----------------------------------------------------------------------------
CGMZ_Core.prototype.removePixiFilterById = function(id) {
	delete this.pixifilters[id];
	$cgmzTemp.requestRemovePixiFilter(id);
};
//-----------------------------------------------------------------------------
// Remove all filters
//-----------------------------------------------------------------------------
CGMZ_Core.prototype.removeAllPixiFilters = function() {
	for(const filterId in this.pixifilters) {
		this.removePixiFilterById(filterId);
	}
};
//=============================================================================
// Scene_Base
//-----------------------------------------------------------------------------
// Apply new filters
//=============================================================================
//-----------------------------------------------------------------------------
// Initialize the cgmz filter array
//-----------------------------------------------------------------------------
const alias_CGMZPixiFilters_SceneBase_initialize = Scene_Base.prototype.initialize;
Scene_Base.prototype.initialize = function() {
	alias_CGMZPixiFilters_SceneBase_initialize.call(this);
	this._cgmz_filters = [];
	this.cgmzDisplacementFilterSprites = [];
};
//-----------------------------------------------------------------------------
// Also create CGMZ Filters after scene start
//-----------------------------------------------------------------------------
const alias_CGMZPixiFilters_SceneBase_start = Scene_Base.prototype.start;
Scene_Base.prototype.start = function() {
	alias_CGMZPixiFilters_SceneBase_start.call(this);
	this.CGMZ_createPixiFilters();
};
//-----------------------------------------------------------------------------
// Create any [CGMZ] pixi filters needed
//-----------------------------------------------------------------------------
Scene_Base.prototype.CGMZ_createPixiFilters = function() {
	if(!$cgmz || !$cgmz.pixifilters || !$cgmzTemp) return;
	$cgmzTemp.clearPixiFilterRequests();
	const filters = $cgmz.getAllPixiFilters();
	for(const filter of filters) {
		if(!this.CGMZ_shouldAddPixiFilter(filter)) continue;
		this.CGMZ_handlePixiFilterCreate(filter);
	}
};
//-----------------------------------------------------------------------------
// Create any [CGMZ] pixi filters needed
//-----------------------------------------------------------------------------
Scene_Base.prototype.CGMZ_shouldAddPixiFilter = function(filter) {
	if(!filter || !filter.scenes) return false;
	const sceneName = this.constructor.name;
	if(!filter.scenes.includes(sceneName) && !filter.scenes.includes('All')) return false;
	if((filter.target === 'Spriteset' || filter.target === 'Base Sprite') && !this._spriteset) return false;
	return true;
};
//-----------------------------------------------------------------------------
// Create a specific [CGMZ] pixi filter and adds it to the requested target
//-----------------------------------------------------------------------------
Scene_Base.prototype.CGMZ_handlePixiFilterCreate = function(filter) {
	const f = $cgmzTemp.createPixiFilterObject(filter);
	if(!f) return;
	f.id = filter.id;
	f.target = filter.target;
	f.cgmzType = filter.cgmzType;
	switch(filter.target) {
		case 'Scene': this.filters.push(f); break;
		case 'Spriteset': this._spriteset.filters.push(f); break;
		case 'Base Sprite': this._spriteset._baseSprite.filters.push(f); break;
	}
	this._cgmz_filters.push(f);
};
//-----------------------------------------------------------------------------
// Also update CGMZ Filters
//-----------------------------------------------------------------------------
const alias_CGMZPixiFilters_SceneBase_update = Scene_Base.prototype.update;
Scene_Base.prototype.update = function() {
	alias_CGMZPixiFilters_SceneBase_update.call(this);
	this.CGMZ_updatePixiFilters();
};
//-----------------------------------------------------------------------------
// Update any [CGMZ] pixi filters
//-----------------------------------------------------------------------------
Scene_Base.prototype.CGMZ_updatePixiFilters = function() {
	for(let i = 0; i < this._cgmz_filters.length; i++) {
		const f = this._cgmz_filters[i];
		if(!f.animating) continue;
		switch(f.cgmzType) {
			case 'godray': f.time += f.animationSpeed; break;
			case 'crt': f.time += f.animationSpeed; f.seed = Math.random(); break;
			case 'noise': f.seed = Math.random(); break;
			case 'oldfilm': f.seed = Math.random(); break;
			case 'glitch': f.seed = Math.random(); break;
			case 'displacement':
				f.maskSprite.x += f.xScroll;
				f.maskSprite.y += f.yScroll;
				f.maskSprite.texture.baseTexture.wrapMode = PIXI.WRAP_MODES.REPEAT;
				break;
		}
	}
	this.CGMZ_checkForFilterAddRequests();
	this.CGMZ_checkForFilterEditRequests();
	this.CGMZ_checkForFilterRemoveRequests();
};
//-----------------------------------------------------------------------------
// Check if need to add any [CGMZ] pixi filters
//-----------------------------------------------------------------------------
Scene_Base.prototype.CGMZ_checkForFilterAddRequests = function() {
	for(;;) {
		const filter = $cgmzTemp?.getRequestedPixiFilterAdd();
		if(filter) {
			if(this.CGMZ_shouldAddPixiFilter(filter)) this.CGMZ_handlePixiFilterCreate(filter);
		} else {
			break;
		}
	}
};
//-----------------------------------------------------------------------------
// Check if need to edit any [CGMZ] pixi filters
//-----------------------------------------------------------------------------
Scene_Base.prototype.CGMZ_checkForFilterEditRequests = function() {
	for(;;) {
		const id = $cgmzTemp?.getRequestedPixiFilterEdit();
		if(id) {
			const filter = $cgmz.getPixiFilterById(id);
			if(filter) this.CGMZ_handlePixiFilterEdit(filter);
		} else {
			break;
		}
	}
};
//-----------------------------------------------------------------------------
// Edit a pixi filter (first checks if the filter is possible to exist in the scene
//-----------------------------------------------------------------------------
Scene_Base.prototype.CGMZ_handlePixiFilterEdit = function(filter) {
	for(let existingFilter of this._cgmz_filters) {
		if(existingFilter.id !== filter.id) continue;
		existingFilter = Object.assign(existingFilter, filter.getEditSettings());
		break;
	}
};
//-----------------------------------------------------------------------------
// Check if need to add any [CGMZ] pixi filters
//-----------------------------------------------------------------------------
Scene_Base.prototype.CGMZ_checkForFilterRemoveRequests = function() {
	for(;;) {
		const id = $cgmzTemp?.getRequestedPixiFilterRemoval();
		if(id) {
			this.CGMZ_handlePixiFilterRemoval(id);
		} else {
			break;
		}
	}
};
//-----------------------------------------------------------------------------
// Remove a pixi filter from the scene
//-----------------------------------------------------------------------------
Scene_Base.prototype.CGMZ_handlePixiFilterRemoval = function(filterId) {
	const index = this._cgmz_filters.findIndex((obj) => {
		return obj.id === filterId;
	});
	if(index < 0) return;
	const cgmzFilter = this._cgmz_filters[index];
	let filterArray = [];
	switch(cgmzFilter.target) {
		case 'Scene': filterArray = this.filters; break;
		case 'Spriteset': filterArray = this._spriteset.filters; break;
		case 'Base Sprite': filterArray = this._spriteset._baseSprite.filters; break;
	}
	const index2 = filterArray.findIndex((obj) => {
		return obj.id === filterId;
	});
	if(index2 < 0) return;
	const filter = filterArray[index2];
	if(filter && filter.cgmzType === 'displacement') {
		for(let i = 0; i < this.cgmzDisplacementFilterSprites.length; i++) {
			const sprite = this.cgmzDisplacementFilterSprites[i];
			if(sprite.filterCGMZId === filterId) {
				this.cgmzDisplacementFilterSprites.splice(i, 1);
				this.removeChild(sprite);
				break;
			}
		}
	}
	filterArray.splice(index2, 1);
	this._cgmz_filters.splice(index, 1);
};
//-----------------------------------------------------------------------------
// Add a displacement map filter sprite to the scene
//-----------------------------------------------------------------------------
Scene_Base.prototype.CGMZ_addDisplacementMapFilterSprite = function(sprite) {
	this.cgmzDisplacementFilterSprites.push(sprite);
	this.addChild(sprite);
};
//=============================================================================
// Scene_GameEnd
//-----------------------------------------------------------------------------
// Remove all filters when going back to title
//=============================================================================
//-----------------------------------------------------------------------------
// Also remove all filters when going back to title
//-----------------------------------------------------------------------------
const alias_CGMZPixiFilters_SceneGameEnd_commandToTitle = Scene_GameEnd.prototype.commandToTitle;
Scene_GameEnd.prototype.commandToTitle = function() {
    alias_CGMZPixiFilters_SceneGameEnd_commandToTitle.call(this);
	$cgmz.removeAllPixiFilters();
};