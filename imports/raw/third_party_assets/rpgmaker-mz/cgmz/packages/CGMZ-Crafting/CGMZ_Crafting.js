/*:
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/crafting/
 * @target MZ
 * @base CGMZ_Core
 * @orderAfter CGMZ_Core
 * @plugindesc Adds a crafting system to your game
 * @help
 * ============================================================================
 * For terms and conditions using this plugin in your game please visit:
 * https://www.caspergaming.com/terms-of-use/
 * ============================================================================
 * Become a Patron to get access to beta/alpha plugins plus other goodies!
 * https://www.patreon.com/CasperGamingRPGM
 * ============================================================================
 * Version: 1.12.1
 * ----------------------------------------------------------------------------
 * Compatibility: Only tested with my CGMZ plugins.
 * Made for RPG Maker MZ 1.10.0
 * ----------------------------------------------------------------------------
 * Description: Adds a crafting system to your game that works well with CGMZ
 * Professions. It can handle item requirements (consumed on craft), tool
 * requirements (not consumed on craft), products (produced on craft success)
 * and fail products (produced on craft fail). Recipes can be discovered by 
 * using an item or via plugin command.
 * ----------------------------------------------------------------------------
 * Documentation:
 * --------------------------Crafting In/Output--------------------------------
 * Recipes have 4 categories of items associated with them: "Products", "Fail
 * Products", "Tools", and "Ingredients".
 * - Products are items that are received by the player directly into the
 *   inventory on a successful craft.
 * - Fail Products are items that are received by the player directly into the
 *   inventory on a failed craft. This is an optional parameter.
 * - Tools are items that are required to craft the recipe, but which are NOT
 *   consumed on craft.
 * - Ingredients are items that are required to craft the recipe, and are
 *   consumed on craft.
 * --------------------------High Quality Crafts-------------------------------
 * Recipes can also have high quality craft results if high quality products
 * are set up. These are special products that occur only in the case the
 * player both succeeds the craft (did not fail the craft) and rolled a second
 * high quality check to see if the craft outcome is a high quality craft.
 *
 * High quality craft items functions similar to regular product items.
 * --------------------------Generic Item Types--------------------------------
 * Recipes can use "generic item types" for their ingredients or tools. What
 * this means is that if you need a "skillet" tool to craft an "omelet", you
 * could designate your "iron skillet" and "steel skillet" as a "skillet" type
 * item, and then either could be used to craft the "omelet".
 *
 * See note tag section for setting an item's type.
 *
 * Caution: Care should be taken if an item belongs to multiple generic
 * categories and the recipe uses multiple generic categories, as the item
 * could be double counted when calculating the amount of items the player has.
 * ----------------------------Unique Products---------------------------------
 * Unique products are products which can only be crafted up until the player
 * has the Product Unique Amount of that item in their inventory. For example,
 * you could make the Ultimate Sword which has a Product Unique Amount of 1,
 * which would mean players can only craft 1 of these swords (at least, until 
 * the first sword is no longer in their party's inventory).
 * 
 * Unique products only support item/weapon/armors. You cannot use unique
 * products to restrict currency or generic item products. Unique Products are
 * only for products, not fail products.
 * -------------------------------Note Tags------------------------------------
 * You can make items that, when used, will cause a recipe to be learned.
 * To do so, put the following tag in its notebox:
 * <cgmzrecipe:RecipeName>
 * And replace RecipeName with the name of the recipe.
 *
 * You can make weapons or armors that, when equipped, will add to a recipe
 * success chance. To do so, put the following note tags in the equip notebox:
 * <cgmzrecipetype:ProfessionName>
 * <cgmzrecipebonus:BonusAmount>
 *
 * You can make "generic" types of items/weapons/armors to be used as either
 * ingredients or tools. To do so, enter the following note tag into any
 * item/armor/weapon notebox:
 * <cgmzcraftinggeneric:GenericType,GenericType2,etc>
 * Caution: Please be aware that generics can ONLY be used for ingredients or
 *          tools!
 *
 * Note: When typing a note tag, it is CASE SENSITIVE which means that
 *       "Cooking" is not the same as "cOOking"
 * -----------------------------Toast Manager----------------------------------
 * In your toast presets, you can use the text code %name to show the recipe
 * name in your toast.
 * --------------------------------Filters-------------------------------------
 * This plugin has the option to allow your player to filter the current list
 * of crafting recipes they are looking at. The following filter types are
 * available:
 *
 * Category Filter
 * This filter is only available if not already using the Category window. To
 * use it, type the categories you want to be available to filter by and 
 * ensure they match the recipe's Profession parameters.
 *
 * Subcategory Filter
 * This filter is always available if enabled. To use it, you need to set up
 * which category (or all) it should show up for as well as the specific
 * recipe Subcategory it should match. To do this, use a colon ":" to separate
 * the category from the subcategory.
 *
 * For example, if you had a recipe of category (profession) "Cooking" and you
 * wanted to be able to filter that category by the subcategory "Spices", you
 * would make your filter entry: Cooking:Spices
 *
 * This supports "All" for the category where the filter subcategory will
 * always appear no matter which category is selected. For example: All:Spices
 *
 * Quality Filter
 * This filter is always available if enabled. To use it, you need to set up
 * which category (or all) it should show up for as well as the specific
 * recipe Quality it should match. To do this, use a colon ":" to separate
 * the category from the quality.
 *
 * For example, if you had a recipe of category (profession) "Cooking" and you
 * wanted to be able to filter that category by the quality "Common", you
 * would make your filter entry: Cooking:Common
 *
 * This supports "All" for the category where the filter quality will
 * always appear no matter which category is selected. For example: All:Common
 * -------------------------------Scene Keys-----------------------------------
 * The plugin command that includes scene keys help differentiate crafting
 * scenes. By default, once something is crafting, it cannot have another
 * craft of the same item start. However, if the scene key is different
 * between the two scenes, it will be recognized as a different item and you
 * can craft multiple of the same craft at once.
 *
 * Scene keys can also come in handy when checking if a specific crafting
 * station is currently crafting something. Note that it is important your
 * scene key is something unique, and should not be "crafting" or match a
 * recipe's name.
 * ----------------------------Plugin Commands---------------------------------
 * This plugin supports the following commands:
 * • Discover
 * Discovers (or undiscovers) the given recipe by name
 *
 * • Discover Batch
 * Discovers (or undiscovers) the given recipes by name
 * 
 * • Call Scene
 * Calls the crafting scene. Specify a profession type (case sensitive) to only
 * include certain recipes in the scene.
 *
 * • Is Scene Crafting
 * Check if the scene with the provided scene key is currently crafting a
 * recipe. Sets a game switch to TRUE/FALSE based on result of the check, for
 * use in eventing.
 *
 * • Is Recipe Discovered
 * Check if the recipe is discovered, and sets a switch to on/off depending
 * on the result. On = discovered, Off = undiscovered
 * 
 * • Set Description
 * Changes the given recipe's description
 * 
 * • Reinitialize
 * Reinitializes crafting data as if you had started a new game.
 * ------------------------Calling the Scene-----------------------------------
 * The JS to call the scene is: SceneManager.push(CGMZ_Scene_Crafting);
 * You can also prepare the crafting scene to show only certain recipes with:
 * SceneManager.prepareNextScene(["Type", "Type2"]);
 * -----------------------------Saved Games------------------------------------
 * This plugin partially supports saved games. Adding new recipes is supported,
 * but removing or modifying existing recipes is not supported. This is
 * because everything is saved when the game is saved. Any issues with
 * removed/modified recipes will only occur in previously saved games.
 *
 * Generic Item Info supports saved games completely, you can add/edit/remove
 * generic item information in saved games. However, the generic item type in
 * the recipe parameters does not support saved games.
 * -----------------------------Filename---------------------------------------
 * The filename for this plugin MUST remain CGMZ_Crafting.js
 * This is what it comes as when downloaded. The filename is used to load
 * parameters and execute plugin commands. If you change it, things will begin
 * behaving incorrectly and your game will probably crash. Please do not
 * rename the js file.
 * --------------------------Latest Version------------------------------------
 * Hi all, this latest version fixes a small bug that could get a player stuck
 * in the craft scene if the plugin is set to wait for crafting to finish and
 * your recipe auto-discovers a new recipe above it in the list window.
 *
 * Version 1.12.1
 * - Fix bug with getting stuck when wait for craft finish enabled
 *
 * @command discover
 * @text Discover
 * @desc Discover (or undiscover) a recipe
 *
 * @arg name
 * @text Recipe Name
 * @desc The name of the recipe to discover
 *
 * @arg discover
 * @type boolean
 * @text Discover
 * @desc Whether to discover or undiscover the recipe
 * @default true
 *
 * @command Discover Batch
 * @desc Discover (or undiscover) multiple recipes
 *
 * @arg Recipes
 * @type text[]
 * @default []
 * @desc Enter names of recipes to discover / undiscover here
 *
 * @arg Discover
 * @type boolean
 * @desc Whether to discover or undiscover the recipes
 * @default true
 *
 * @command Call Scene
 * @desc Calls the crafting scene
 *
 * @arg type
 * @type text[]
 * @text Type
 * @desc The type of recipes to include. Leave this blank to include all discovered recipes
 * @default []
 *
 * @arg View Only
 * @type boolean
 * @desc If true, the player will not be able to craft, only view recipe info
 * @default false
 *
 * @arg Scene Key
 * @desc Key used to differentiate from recipes crafting in other scenes
 *
 * @arg Background Preset
 * @desc [CGMZ] Scene Backgrounds preset id to use for the scene (leave blank to use default)
 *
 * @command Is Scene Crafting
 * @desc Check if a scene currently has recipes crafting
 *
 * @arg Scene Key
 * @desc The scene's key to check, this must be filled out
 *
 * @arg Game Switch
 * @default 0
 * @type switch
 * @desc The game switch to turn on or off based on the result
 *
 * @command Is Recipe Discovered
 * @desc Check if a recipe is discovered
 *
 * @arg Recipe Name
 * @desc The name of the recipe to check
 *
 * @arg Switch
 * @type switch
 * @desc The switch to set to true if discovered
 * @default 0
 *
 * @command Set Description
 * @desc Set a recipe's description
 *
 * @arg name
 * @text Recipe Name
 * @desc The name of the recipe to change description (case sensitive)
 *
 * @arg description
 * @text Description
 * @type multiline_string
 * @desc The new description
 *
 * @command Reinitialize
 * @desc Resets all crafting data. Use for saved games to recognize changed data
 *
 * @param Recipes
 * @type struct<Recipe>[]
 * @default []
 * @desc Set up recipes here
 *
 * @param Generic Items
 * @type struct<GenericItem>[]
 * @default []
 * @desc Set up generic items here
 *
 * @param Categories
 * @type struct<CategoryData>[]
 * @default []
 * @desc Set up crafting categories here
 *
 * @param Crafting Options
 *
 * @param Instant Crafting
 * @parent Crafting Options
 * @type boolean
 * @desc If true, will not show the progress window and will instantly craft everything
 * @default false
 *
 * @param Automatic Learn
 * @parent Crafting Options
 * @type boolean
 * @desc If true, recipes will automatically be discovered when the player gets the items for a recipe
 * @default false
 *
 * @param Allow Multicraft
 * @parent Crafting Options
 * @type boolean
 * @desc If true, the scene will ask the player how many they want to craft
 * @default false
 *
 * @param Wait For Finish
 * @parent Crafting Options
 * @type boolean
 * @desc If true, the user will not be able to do anything until their current craft finishes
 * @default false
 *
 * @param Collect Items At Station
 * @parent Crafting Options
 * @type boolean
 * @desc If true, items crafted will need to be collected at the crafting station they are made at
 * @default true
 *
 * @param Window Options
 *
 * @param Allow Filter
 * @parent Window Options
 * @type boolean
 * @desc If true, will allow the player to set filters for the recipes shown
 * @default true
 *
 * @param Transparent Windows
 * @parent Window Options
 * @type boolean
 * @desc Whether the crafting windows are transparent or not
 * @default false
 *
 * @param Disable Touch UI Space
 * @parent Window Options
 * @type boolean
 * @desc If true, will not leave space for Touch UI buttons if Touch UI is disabled
 * @default false
 *
 * @param ScrollSpeed
 * @parent Window Options
 * @type number
 * @min 0
 * @desc speed at which the recipe window display scrolls (if needed)
 * @default 1
 *
 * @param ScrollWait
 * @parent Window Options
 * @type number
 * @min 0
 * @desc amount of time (in frames) to wait before beginning to scroll
 * @default 300
 *
 * @param Scroll Deceleration
 * @parent Window Options
 * @type number
 * @min 0.01
 * @max 0.99
 * @decimals 2
 * @desc Rate of deceleration after letting go of touch
 * @default 0.92
 *
 * @param Auto Scroll
 * @parent Window Options
 * @type boolean
 * @desc Determine if the display window should automatically scroll after so long of no user input
 * @default true
 *
 * @param Category Sort Order
 * @parent Window Options
 * @type text[]
 * @desc The order categories appear in when all are showing
 * @default []
 *
 * @param List Window Width
 * @parent Window Options
 * @type number
 * @min 0
 * @max 100
 * @desc % of the screen width the list window takes up
 * @default 33
 *
 * @param Multicraft Window Width
 * @parent Window Options
 * @type number
 * @min 0
 * @max 100
 * @desc % of the screen width the multicraft window takes up
 * @default 50
 *
 * @param List Window On Right
 * @parent Window Options
 * @type boolean
 * @default false
 * @desc If true, the list window will be on the right side of the screen
 *
 * @param Show Ingredient Amount
 * @parent Window Options
 * @type boolean
 * @default true
 * @desc Show the current amount of ingredients the player has?
 *
 * @param Show Tool Amount
 * @parent Window Options
 * @type boolean
 * @default false
 * @desc Show the current amount of tools the player has?
 *
 * @param Show Product Amount
 * @parent Window Options
 * @type boolean
 * @default false
 * @desc Show the current amount of the product the player has?
 *
 * @param Show Unique Product Amount
 * @parent Window Options
 * @type boolean
 * @default false
 * @desc Show the maximum amount of unique product a player can make?
 *
 * @param Show Fail Product Amount
 * @parent Window Options
 * @type boolean
 * @default false
 * @desc Show the current amount of the product the player has?
 *
 * @param Show High Quality Product Amount
 * @parent Window Options
 * @type boolean
 * @default false
 * @desc Show the current amount of the product the player has?
 *
 * @param Hide Uncraftable Recipes
 * @parent Window Options
 * @type boolean
 * @default false
 * @desc If true, recipes the player cannot craft will not show even if they learned them
 *
 * @param Show Confirm Window
 * @parent Window Options
 * @type boolean
 * @desc Determine if there should be an additional confirmation window before starting to craft
 * @default false
 *
 * @param Show Progress Percentage
 * @parent Window Options
 * @desc Whether to show the progress % text or not
 * @type boolean
 * @default true
 *
 * @param Show Progress Time Remaining
 * @parent Window Options
 * @desc If true, the progress window will show time remaining.
 * @type boolean
 * @default false
 *
 * @param Progress Decimals
 * @parent Window Options
 * @desc Number of decimals to show on the progress percentage, if displayed
 * @type number
 * @default 2
 *
 * @param Show Category Window
 * @parent Window Options
 * @desc Whether to use the category window or not
 * @type boolean
 * @default true
 *
 * @param Display Window Info
 * @parent Window Options
 * @type select[]
 * @option Name
 * @option Image
 * @option Times Crafted
 * @option Success Rate
 * @option High Quality Rate
 * @option Quality
 * @option Subcategory
 * @option Exp
 * @option Level Required
 * @option Current Level
 * @option Description
 * @option Products
 * @option Tools
 * @option Ingredients
 * @option Fail Products
 * @option High Quality Products
 * @option Info Header
 * @option Description Header
 * @option Product Header
 * @option Tool Header
 * @option Ingredient Header
 * @option Fail Product Header
 * @option High Quality Product Header
 * @option Blank Line
 * @option Custom Space
 * @desc Determines the order and what info the display window shows.
 * @default ["Name","Image","Times Crafted","Success Rate","Quality","Exp","Level Required","Current Level","Description","Product Header","Products","Tool Header","Tools","Ingredient Header","Ingredients"]
 *
 * @param Custom Space
 * @parent Window Options
 * @desc Amount of blank vertical space to leave when Custom Space display parameter is used
 * @type number
 * @default 6
 * 
 * @param Category Filter Options
 * @parent Window Options
 * @desc Category filter options. Must match recipe profession exactly.
 * @type text[]
 * @default []
 *
 * @param Subcategory Filter Options
 * @parent Window Options
 * @desc Subcategory filter options. Must match recipe subcategory exactly.
 * @type text[]
 * @default []
 *
 * @param Quality Filter Options
 * @parent Window Options
 * @desc Quality filter options. Must match recipe quality exactly.
 * @type text[]
 * @default []
 *
 * @param Filter Button Width
 * @parent Window Options
 * @desc Width of the filter touch UI button (in multiples of 48)
 * @type number
 * @default 1
 *
 * @param Filter Button Offset
 * @parent Window Options
 * @desc Filter Button index on the button sheet
 * @type number
 * @default 4
 *
 * @param Filter Key
 * @parent Window Options
 * @desc Key that when pressed will start the filter flow
 * @default f
 *
 * @param Filter Gamepad
 * @parent Window Options
 * @desc Gamepad button that when pressed will start the filter flow
 * @type select
 * @option A
 * @value 0
 * @option B
 * @value 1
 * @option X
 * @value 2
 * @option Y
 * @value 3
 * @option LB
 * @value 4
 * @option RB
 * @value 5
 * @option LT
 * @value 6
 * @option RT
 * @value 7
 * @option Back / Select
 * @value 8
 * @option Start
 * @value 9
 * @option Left Stick
 * @value 10
 * @option Right Stick
 * @value 11
 * @option Dpad Up
 * @value 12
 * @option Dpad Down
 * @value 13
 * @option Dpad Left
 * @value 14
 * @option Dpad Right
 * @value 15
 * @default 5
 * 
 * @param Text Options
 *
 * @param Description Alignment
 * @parent Text Options
 * @type select
 * @option left
 * @option center
 * @option right
 * @desc The alignment of the description text.
 * @default left
 *
 * @param Success Rate Text
 * @parent Text Options
 * @desc Text to show to describe the success rate of a recipe
 * @default Success Rate: 
 *
 * @param High Quality Rate Text
 * @parent Text Options
 * @desc Text to show to describe the high quality rate of a recipe
 * @default High Quality Rate: 
 *
 * @param Quality Label Text
 * @parent Text Options
 * @desc Text to show to describe the quality of a recipe
 * @default Quality: 
 *
 * @param Subcategory Label Text
 * @parent Text Options
 * @desc Text to show to describe the subcategory of a recipe
 * @default Specialization: 
 *
 * @param Times Crafted Text
 * @parent Text Options
 * @desc Text to show to describe the amount of times a recipe has been crafted
 * @default Times Crafted: 
 *
 * @param Experience Text
 * @parent Text Options
 * @desc Text to show to describe the experience gained for crafting the recipe (Requires CGMZ Professions)
 * @default Exp Gain: 
 *
 * @param Level Requirement Text
 * @parent Text Options
 * @desc Text to show to describe the level required to craft the recipe (Requires CGMZ Professions)
 * @default Level Req: 
 *
 * @param Current Level Text
 * @parent Text Options
 * @desc Text to show to describe the current level of the required profession (Requires CGMZ Professions)
 * @default Level: 
 *
 * @param Level Abbreviation Text
 * @parent Text Options
 * @desc Text to abbreviate level requirement to (Requires CGMZ Professions)
 * @default Lv.
 *
 * @param Amount X Text
 * @parent Text Options
 * @desc Text to show to describe the amount x
 * @default x
 *
 * @param Craft Confirm Text
 * @parent Text Options
 * @desc Text to show to describe the command for crafting
 * @default Craft
 *
 * @param Craft Cancel Text
 * @parent Text Options
 * @desc Text to show to describe the command for cancelling a craft
 * @default Cancel
 *
 * @param Progress Text
 * @parent Text Options
 * @desc Text to show to describe the progress of the currently crafting recipe
 * @default Progress: 
 *
 * @param Success Text
 * @parent Text Options
 * @desc Text to show to describe a successful craft
 * @default Craft Success!
 *
 * @param Failure Text
 * @parent Text Options
 * @desc Text to show to describe a failed craft
 * @default Craft Failed!
 *
 * @param High Quality Text
 * @parent Text Options
 * @desc Text to show to describe a high quality craft
 * @default High Quality Craft!
 *
 * @param Craft Quantity Text
 * @parent Text Options
 * @desc Text to show to describe the amount the player wants to craft
 * @default Amount to Craft:
 *
 * @param Label Text Color
 * @parent Text Options
 * @desc The color of the text labels in the crafting scene
 * @type color
 * @default 16
 *
 * @param Progress Color1
 * @parent Text Options
 * @desc First color of the progress bar using Windowskin colors
 * @type color
 * @default 28
 *
 * @param Progress Color2
 * @parent Text Options
 * @desc Second color of the progress bar using Windowskin colors
 * @type color
 * @default 29
 *
 * @param Success Color
 * @parent Text Options
 * @desc Color of the Successful Craft Text using Windowskin colors
 * @type color
 * @default 29
 *
 * @param High Quality Color
 * @parent Text Options
 * @desc Color of the High Quality Craft Text
 * @type color
 * @default 29
 *
 * @param Failure Color
 * @parent Text Options
 * @desc Color of the Failure Craft Text using Windowskin colors
 * @type color
 * @default 18
 *
 * @param Header Color 1
 * @parent Text Options
 * @desc Color 1 of the gradient lines in headers
 * @type color
 * @default 1
 *
 * @param Header Color 2
 * @parent Text Options
 * @desc Color 2 of the gradient lines in headers
 * @type color
 * @default 0
 *
 * @param Info Header Text
 * @parent Text Options
 * @desc Text in the Info Header of display window
 * @default Info
 *
 * @param Desc Header Text
 * @parent Text Options
 * @desc Text in the Description Header of display window
 * @default Description
 *
 * @param Ingredient Header Text
 * @parent Text Options
 * @desc Text in the Ingredient Header of display window
 * @default Ingredients
 *
 * @param Tool Header Text
 * @parent Text Options
 * @desc Text in the Tool Header of display window
 * @default Tools
 *
 * @param Product Header Text
 * @parent Text Options
 * @desc Text in the Product Header of display window
 * @default Products
 *
 * @param Fail Product Header Text
 * @parent Text Options
 * @desc Text in the Fail Product Header of display window
 * @default Fail Products
 *
 * @param High Quality Product Header Text
 * @parent Text Options
 * @desc Text in the High Quality Product Header of display window
 * @default High Quality Products
 *
 * @param Unique Text
 * @parent Text Options
 * @desc Text to show for unique products
 * @default Unique (%unique)
 *
 * @param No Recipe Text
 * @parent Text Options
 * @desc Text to show when the scene has no recipes
 * @default Keep playing the game to learn more recipes!
 *
 * @param Percent Text
 * @parent Text Options
 * @desc Text to show after progress percentage
 * @default  %
 *
 * @param Sec Text
 * @parent Text Options
 * @desc Text to show when there are seconds remaining on progress bar
 * @default sec
 *
 * @param Min Text
 * @parent Text Options
 * @desc Text to show when there are minutes remaining on progress bar
 * @default min
 *
 * @param Hr Text
 * @parent Text Options
 * @desc Text to show when there are hours remaining on progress bar
 * @default hr
 *
 * @param Remove Filter Text
 * @parent Text Options
 * @desc Text to show for the Remove Filter option
 * @default Remove Filter
 *
 * @param Category Filter Text
 * @parent Text Options
 * @desc Text to show for the Category Filter option
 * @default Category
 *
 * @param Subcategory Filter Text
 * @parent Text Options
 * @desc Text to show for the Subcategory Filter option
 * @default Subcategory
 *
 * @param Quality Filter Text
 * @parent Text Options
 * @desc Text to show for the Quality Filter option
 * @default Quality
 *
 * @param Collect Item Confirm Text
 * @parent Text Options
 * @desc Text for the confirm option in the collected items window
 * @default Collect
 *
 * @param Collect Item Item Text
 * @parent Text Options
 * @desc Text for an item that was collected in the crafting scene
 * @default %amountx %icon%name
 *
 * @param Collect Item Weapon Text
 * @parent Text Options
 * @desc Text for a weapon that was collected in the crafting scene
 * @default %amountx %icon%name
 *
 * @param Collect Item Armor Text
 * @parent Text Options
 * @desc Text for an armor that was collected in the crafting scene
 * @default %amountx %icon%name
 *
 * @param Collect Item Currency Text
 * @parent Text Options
 * @desc Text for an armor that was collected in the crafting scene
 * @default %icon%amount %color%unit
 *
 * @param Collected Items Text
 * @parent Text Options
 * @desc Text to display at the top of the collected items window
 * @default Collected Items:
 *
 * @param Integrations
 *
 * @param Learn Toast
 * @parent Integrations
 * @desc [CGMZ] Toast Manager preset id to show when learning a recipe
 *
 * @param Complete Toast
 * @parent Integrations
 * @desc [CGMZ] Toast Manager preset id to show when a craft is completed
 *
 * @param Always Award Exp
 * @parent Integrations
 * @type boolean
 * @default true
 * @desc Award exp even on recipe failure? (requires CGMZ Professions)
 *
 * @param Hide High Level Recipes
 * @parent Integrations
 * @type boolean
 * @default true
 * @desc Hide recipes the player does not meet level requirements for?
 *
 * @param Close On Level Up
 * @parent Integrations
 * @type boolean
 * @desc Close the crafting scene on profession level up?
 * @default false
 *
 * @param Fail Rumble
 * @parent Integrations
 * @type struct<Rumble>
 * @default {"Duration":"0","Weak Magnitude":"1.00","Strong Magnitude":"1.00","Start Delay":"0"}
 * @desc [CGMZ] Rumble to play when successful craft finishes
 *
 * @param Success Rumble
 * @parent Integrations
 * @type struct<Rumble>
 * @default {"Duration":"0","Weak Magnitude":"1.00","Strong Magnitude":"1.00","Start Delay":"0"}
 * @desc [CGMZ] Rumble to play when successful craft finishes
 *
 * @param Scene Background
 * @parent Integrations
 * @desc [CGMZ] Scene Backgrounds preset id to use for the crafting scene
 *
 * @param Category Window Settings
 * @parent Integrations
 * @desc [CGMZ] Window Settings preset to use for the category window
 *
 * @param List Window Settings
 * @parent Integrations
 * @desc [CGMZ] Window Settings preset to use for the list window
 *
 * @param Display Window Settings
 * @parent Integrations
 * @desc [CGMZ] Window Settings preset to use for the display window
 *
 * @param Progress Window Settings
 * @parent Integrations
 * @desc [CGMZ] Window Settings preset to use for the progress window
 *
 * @param Confirm Window Settings
 * @parent Integrations
 * @desc [CGMZ] Window Settings preset to use for the confirm window
 *
 * @param Multicraft Window Settings
 * @parent Integrations
 * @desc [CGMZ] Window Settings preset to use for the multicraft window
 *
 * @param Filter Type Window Settings
 * @parent Integrations
 * @desc [CGMZ] Window Settings preset to use for the filter type window
 *
 * @param Filter Option Window Settings
 * @parent Integrations
 * @desc [CGMZ] Window Settings preset to use for the filter option window
 *
 * @param Collected Items Window Settings
 * @parent Integrations
 * @desc [CGMZ] Window Settings preset to use for the collected items window
 *
 * @param Collect Items Confirm Window Settings
 * @parent Integrations
 * @desc [CGMZ] Window Settings preset to use for the collect items window
 *
 * @param Category Window Background
 * @parent Integrations
 * @desc [CGMZ] Window Backgrounds preset to use for the category window
 *
 * @param List Window Background
 * @parent Integrations
 * @desc [CGMZ] Window Backgrounds preset to use for the list window
 *
 * @param Display Window Background
 * @parent Integrations
 * @desc [CGMZ] Window Backgrounds preset to use for the display window
 *
 * @param Progress Window Background
 * @parent Integrations
 * @desc [CGMZ] Window Backgrounds preset to use for the progress window
 *
 * @param Confirm Window Background
 * @parent Integrations
 * @desc [CGMZ] Window Backgrounds preset to use for the confirm window
 *
 * @param Multicraft Window Background
 * @parent Integrations
 * @desc [CGMZ] Window Backgrounds preset to use for the multicraft window
 *
 * @param Filter Type Window Background
 * @parent Integrations
 * @desc [CGMZ] Window Backgrounds preset to use for the filter type window
 *
 * @param Filter Option Window Background
 * @parent Integrations
 * @desc [CGMZ] Window Backgrounds preset to use for the filter option window
 *
 * @param Collected Items Window Background
 * @parent Integrations
 * @desc [CGMZ] Window Backgrounds preset to use for the collected items window
 *
 * @param Collect Items Confirm Window Background
 * @parent Integrations
 * @desc [CGMZ] Window Backgrounds preset to use for the collect items confirm window
*/
/*~struct~Recipe:
 * @param Name
 * @desc The name of the recipe.
 *
 * @param Display Name
 * @desc The displayed name of the recipe (if different than name).
 *
 * @param Products
 * @type struct<Item>[]
 * @default []
 * @desc The items produced by crafting the recipe
 *
 * @param Fail Products
 * @type struct<Item>[]
 * @default []
 * @desc The items produced by a failed craft of this recipe
 *
 * @param Ingredients
 * @type struct<Item>[]
 * @default []
 * @desc The items required to craft the recipe that are consumed on craft
 *
 * @param Tools
 * @type struct<Item>[]
 * @default []
 * @desc The items required to craft the recipe that are not consumed on craft
 * 
 * @param Discovered
 * @type boolean
 * @default false
 * @desc Determine whether the recipe is discovered at the beginning of the game.
 *
 * @param Picture
 * @type file
 * @dir img/
 * @desc The image to use for the recipe in place of the big icon (recommended size: 64x64). Leave blank to not use.
 * 
 * @param Icon
 * @type icon
 * @default 0
 * @desc Icon index to use for the recipe
 *
 * @param Description
 * @type multiline_string
 * @desc Recipe description
 *
 * @param Success Rate
 * @type number
 * @min -99999
 * @max 100
 * @default 100
 * @desc The % chance that the craft will succeed.
 *
 * @param Time
 * @type number
 * @min 1
 * @default 120
 * @desc The time (in frames, 60f = 1sec) it takes to craft the recipe
 *
 * @param Success Rate Per Level
 * @type number
 * @min -99999
 * @max 100
 * @default 0
 * @desc The change (additive) in % chance that the craft will succeed per level beyond the level requirement. (Requires CGMZ Professions)
 *
 * @param Experience
 * @type number
 * @min 1
 * @default 1
 * @desc The amount of experience to award for crafting this recipe (Requires CGMZ Professions)
 *
 * @param Profession
 * @desc The profession name which the recipe belongs to if using CGMZ Professions. The type of recipe if not using CGMZ Professions.
 *
 * @param Subcategory
 * @desc If you want, you can add a subcategory to the recipe. This does not affect how the recipe functions.
 *
 * @param Quality
 * @desc If you want, you can add a quality to the recipe. This does not affect how the recipe functions.
 *
 * @param Level Requirement
 * @type number
 * @default 1
 * @min 0
 * @desc Profession level required to craft (Requires CGMZ Professions)
 *
 * @param Craft Sound Effect
 * @type struct<SoundEffect>
 * @default {"Name":"","Volume":"90","Pitch":"100","Pan":"0"}
 * @desc The sound effect to play when crafting the recipe
 *
 * @param Fail Sound Effect
 * @type struct<SoundEffect>
 * @default {"Name":"","Volume":"90","Pitch":"100","Pan":"0"}
 * @desc The sound effect to play when crafting fails
 *
 * @param Success Sound Effect
 * @type struct<SoundEffect>
 * @default {"Name":"","Volume":"90","Pitch":"100","Pan":"0"}
 * @desc The sound effect to play when crafting succeeds
 *
 * @param Learn Toast
 * @desc [CGMZ] Toast Manager preset id to use for learning this specific recipe only
 *
 * @param Complete Toast
 * @desc [CGMZ] Toast Manager preset id to use for crafting this specific recipe only
 *
 * @param Disable Learn Toast
 * @type boolean
 * @default false
 * @desc Do not show the learn toast for only this recipe?
 *
 * @param Disable Complete Toast
 * @type boolean
 * @default false
 * @desc Do not show the complete toast for only this recipe?
 *
 * @param Disable Autodiscover
 * @type boolean
 * @default false
 * @desc If true, this recipe must be manually discovered.
 *
 * @param Learn On Craft
 * @type struct<OnCraftRecipe>[]
 * @default []
 * @desc Recipe(s) to learn when this recipe is crafted
 *
 * @param Unlearn On Craft
 * @type struct<OnCraftRecipe>[]
 * @default []
 * @desc Recipe(s) to unlearn when this recipe is crafted
 *
 * @param Success Variable
 * @type variable
 * @default 0
 * @desc Variable ID which, on successful craft, will increase by 1
 *
 * @param Product Unique Amount
 * @type number
 * @default 0
 * @desc Amount of the Product items in inventory before the recipe can no longer be crafted
 *
 * @param High Quality Products
 * @type struct<Item>[]
 * @default []
 * @desc The items produced by crafting the recipe when a high quality craft occurs
 *
 * @param High Quality Chance
 * @type number
 * @min 0
 * @max 100
 * @default 0
 * @desc The chance a craft will instead be high quality
 *
 * @param High Quality Chance Per Level
 * @type number
 * @min -100
 * @max 100
 * @default 0
 * @desc The change (additive) in % chance that the craft will be high quality per level beyond the level requirement. (Requires CGMZ Professions)
 */
/*~struct~OnCraftRecipe:
 * @param Recipe Name
 * @desc The recipe name to learn/unlearn
 *
 * @param Chance
 * @type number
 * @default 100
 * @desc The chance to learn/unlearn the recipe.
*/
/*~struct~Item:
 * @param Item
 * @type item
 * @default 0
 * @desc The item to use. If you set this, do not set armor/weapon.
 *
 * @param Weapon
 * @type weapon
 * @default 0
 * @desc The weapon to use. If you set this, do not set armor/item.
 *
 * @param Armor
 * @type armor
 * @default 0
 * @desc The item to use. If you set this, do not set item/weapon.
 *
 * @param Gold
 * @type boolean
 * @default false
 * @desc True to set a gold amount.
 *
 * @param Generic
 * @desc A generic type to use. Only for use in Ingredients/Tools! Ignored for Products.
 * 
 * @param Amount
 * @type number
 * @default 1
 * @desc The amount of this item needed
 *
 * @param Currency Id
 * @desc [CGMZ] Currency System currency id if Gold is set
*/
/*~struct~GenericItem:
 * @param Type
 * @desc The id/type of the generic item. Used internally to refer to this generic item.
 *
 * @param Display Name
 * @desc The displayed name of the generic item
 *
 * @param Icon Index
 * @type icon
 * @default 0
 * @desc The icon to use to represent the generic item
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
/*~struct~CategoryData:
 * @param Id
 * @desc The id (profession) of the category
 *
 * @param Command Text
 * @desc The text to show for this category in the category window
*/
/*~struct~Rumble:
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
 * @url https://www.caspergaming.com/plugins/cgmz/crafting/
 * @target MZ
 * @base CGMZ_Core
 * @orderAfter CGMZ_Core
 * @plugindesc 手工艺系统（物品合成制作）
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
 * 【插件版本】V 1.12.1
 * ----------------------------------------------------------------------------
 * 【兼容性】仅测试作者所制作的插件
 * 【RM版本】RPG Maker MZ 1.10.0
 * ----------------------------------------------------------------------------
 * 【插件描述】
 * 功能全面的传统物品合成插件。
 * 通过设置配方的材料和工具，来合成获得产物，或得到废品。
 * 可以通过合成来领悟新的配方。
 * 可以设定和使用相同属性工具或材料作为合成替代品。
 * 可以使用物品或触发事件来获取配方。
 *
 * 【搭配插件】
 * CGMZ Core:核心插件，运行作者插件的必须插件。
 * CGMZ Toast Manager:提示插件，提示配方得失，专业技能升级等。
 * CGMZ Profession:专业插件，用专业等级作为合成条件，获得经验升级和提高成功率等。 
 * 注：本插件在插件列表中必须置于"核心插件Core"和"提示插件Toast Manager"之下。
 * ----------------------------------------------------------------------------
 * 【使用说明】
 *
 * 一、配方设置（4个主要项目）
 *   1.产物：合成成功的产物，可以设置多种产物和不同数量。
 *   2.废品：合成失败的产物，可以不设置。
 *   3.工具：合成所需工具，需持有但不会消耗。可以不设置。
 *   4.材料：合成所需的物品，合成时会被消耗。
 *       注：以上项目可以设置为：物品/武器/防具/金钱。
 *   5.专业：定义配方的类型，在打开合成界面时使用。
 *       注：如果配合专业插件，可以拓展以专业等级作为合成条件。
 *   6.领悟：在合成时，有几率获得新的配方。
 *
 * 二、物品属性和合成次数（1.4.0新增功能）
 *   1.可以定义作为材料或工具的物品/武器/防具的属性。
 *     拥有相同属性的物品可以在配方合成时作为替代品。
 *     尽量避免一个配方中的一个物品有多种属性的设置。（详细设置见备注指令的说明）
 *     举例1：制作皮革需要剥皮刀工具，铁刀、银刀、金刀均为剥皮刀属性的物品，
 *            拥有其中之一即可制作皮革配方。
 *     举例2：制作沙拉需要5个蔬菜属性的物品，
 *            你可以使用同为蔬菜属性的白菜和番茄来制作，满足材料需求数量即可。
 *            同类物品会按照物品ID顺序从小到大消耗。
 *   2.可以获取某个配方的成功合成次数作为变量在游戏中使用。
 *
 * 三、备注指令（在物品/武器/防具设置界面备注栏输入）
 *   1.制作使用后可获得配方的物品：
 *     <cgmzrecipe:配方名称>
 *   2.制作装备后提高合成成功率百分比的武器或防具：
 *     <cgmzrecipetype:配方名称>
 *     <cgmzrecipebonus:提升数值>
 *   3.定义工具或材料的属性
 *     <cgmzcraftinggeneric:属性1, 属性2, 属性3等>
 *     注：只能定义工具或材料，产物和废品无效。
 *
 * 四、插件指令（事件中使用插件指令）
 *   1.打开菜单：打开合成菜单显示已获得的配方。
 *               设置一种或多种专业，则可以同时打开所选专业对应的配方。
 *               不设置则打开所有已获得配方。
 *   2.获得配方：设置获得或失去配方。
 *   3.修改描述：修改一个配方的备注描述。
 *   4.重置数据：调试用指令，重置合成数据到新游戏开始状态。
 * 
 * 五、脚本指令（事件中运行脚本指令）
 *   1.打开合成界面：SceneManager.push(CGMZ_Scene_Crafting);
 *   2.打开指定专业的合成界面：SceneManager.prepareNextScene(["专业1", "专业2"]);
 * 
 * 六、关于所有设置中的配方名称、物品属性、专业名称：
 *     字符必须一致，并区分大小写。如Cooking和cooKING会定义为不同的配方或专业。
 *
 * 七、关于插件对已保存游戏的支持。
 *     大概意思是支持加入新配方，不支持配方修改或删除。
 *     不太明白，请各位参考原文。
 *     This plugin partially supports saved games. Adding new recipes is supported,
 *     but removing or modifying existing recipes is not supported. This is
 * 	   because everything is saved when the game is saved. Any issues with
 *     removed/modified recipes will only occur in previously saved games.
 * --------------------------High Quality Crafts-------------------------------
 * Recipes can also have high quality craft results if high quality products
 * are set up. These are special products that occur only in the case the
 * player both succeeds the craft (did not fail the craft) and rolled a second
 * high quality check to see if the craft outcome is a high quality craft.
 *
 * High quality craft items functions similar to regular product items.
 * ----------------------------Unique Products---------------------------------
 * Unique products are products which can only be crafted up until the player
 * has the Product Unique Amount of that item in their inventory. For example,
 * you could make the Ultimate Sword which has a Product Unique Amount of 1,
 * which would mean players can only craft 1 of these swords (at least, until 
 * the first sword is no longer in their party's inventory).
 * 
 * Unique products only support item/weapon/armors. You cannot use unique
 * products to restrict currency or generic item products. Unique Products are
 * only for products, not fail products.
 * -------------------------------Scene Keys-----------------------------------
 * The plugin command that includes scene keys help differentiate crafting
 * scenes. By default, once something is crafting, it cannot have another
 * craft of the same item start. However, if the scene key is different
 * between the two scenes, it will be recognized as a different item and you
 * can craft multiple of the same craft at once.
 *
 * Scene keys can also come in handy when checking if a specific crafting
 * station is currently crafting something. Note that it is important your
 * scene key is something unique, and should not be "crafting" or match a
 * recipe's name.
 * --------------------------------Filters-------------------------------------
 * This plugin has the option to allow your player to filter the current list
 * of crafting recipes they are looking at. The following filter types are
 * available:
 *
 * Category Filter
 * This filter is only available if not already using the Category window. To
 * use it, type the categories you want to be available to filter by and 
 * ensure they match the recipe's Profession parameters.
 *
 * Subcategory Filter
 * This filter is always available if enabled. To use it, you need to set up
 * which category (or all) it should show up for as well as the specific
 * recipe Subcategory it should match. To do this, use a colon ":" to separate
 * the category from the subcategory.
 *
 * For example, if you had a recipe of category (profession) "Cooking" and you
 * wanted to be able to filter that category by the subcategory "Spices", you
 * would make your filter entry: Cooking:Spices
 *
 * This supports "All" for the category where the filter subcategory will
 * always appear no matter which category is selected. For example: All:Spices
 *
 * Quality Filter
 * This filter is always available if enabled. To use it, you need to set up
 * which category (or all) it should show up for as well as the specific
 * recipe Quality it should match. To do this, use a colon ":" to separate
 * the category from the quality.
 *
 * For example, if you had a recipe of category (profession) "Cooking" and you
 * wanted to be able to filter that category by the quality "Common", you
 * would make your filter entry: Cooking:Common
 *
 * This supports "All" for the category where the filter quality will
 * always appear no matter which category is selected. For example: All:Common
 * -----------------------------Toast Manager----------------------------------
 * In your toast presets, you can use the text code %name to show the recipe
 * name in your toast.
 * -----------------------------Filename---------------------------------------
 * The filename for this plugin MUST remain CGMZ_Crafting.js
 * This is what it comes as when downloaded. The filename is used to load
 * parameters and execute plugin commands. If you change it, things will begin
 * behaving incorrectly and your game will probably crash. Please do not
 * rename the js file.
 * ---------------------------------------------------------------------------
 *【版本更新历史】
 * Hi all, this latest version fixes a small bug that could get a player stuck
 * in the craft scene if the plugin is set to wait for crafting to finish and
 * your recipe auto-discovers a new recipe above it in the list window.
 *
 * Version 1.12.1
 * - Fix bug with getting stuck when wait for craft finish enabled
 *
 * @command discover
 * @text 获得配方
 * @desc 获得或失去配方。
 *
 * @arg name
 * @text 配方名称
 * @desc 获得或失去的配方的名称。
 *
 * @arg discover
 * @type boolean
 * @text 获得/失去
 * @desc Ture获得/False失去配方。
 * @default true
 *
 * @command Discover Batch
 * @desc Discover (or undiscover) multiple recipes
 *
 * @arg Recipes
 * @type text[]
 * @default []
 * @desc Enter names of recipes to discover / undiscover here
 *
 * @arg Discover
 * @type boolean
 * @desc Whether to discover or undiscover the recipes
 * @default true
 *
 * @command Call Scene
 * @text 打开合成菜单
 * @desc 打开合成菜单，如果不设置专业则显示所有已获得的配方。
 *
 * @arg type
 * @type text[]
 * @text 专业
 * @desc 设置专业，打开菜单时会显示所有属于该专业的已获得的配方。
 * @default []
 *
 * @arg View Only
 * @type boolean
 * @desc If true, the player will not be able to craft, only view recipe info
 * @default false
 *
 * @arg Scene Key
 * @desc Key used to differentiate from recipes crafting in other scenes
 *
 * @arg Background Preset
 * @desc [CGMZ] Scene Backgrounds preset id to use for the scene (leave blank to use default)
 *
 * @command Is Scene Crafting
 * @desc Check if a scene currently has recipes crafting
 *
 * @arg Scene Key
 * @desc The scene's key to check, this must be filled out
 *
 * @arg Game Switch
 * @default 0
 * @type switch
 * @desc The game switch to turn on or off based on the result
 *
 * @command Is Recipe Discovered
 * @desc Check if a recipe is discovered
 *
 * @arg Recipe Name
 * @desc The name of the recipe to check
 *
 * @arg Switch
 * @type switch
 * @desc The switch to set to true if discovered
 * @default 0
 *
 * @command Set Description
 * @text 修改配方描述
 * @desc 修改一个配方的备注描述。
 *
 * @arg name
 * @text 配方名称
 * @desc 需要修改描述的配方名称。
 *
 * @arg description
 * @text 新的描述
 * @type multiline_string
 * @desc 输入新的配方描述。
 *
 * @command Reinitialize
 * @text 重置数据
 * @desc 调试用指令，重置合成数据到新游戏开始状态。
 *
 * @param Recipes
 * @text 配方设置
 * @type struct<Recipe>[]
 * @default []
 * @desc 设置你需要的配方。
 *
 * @param Generic Items
 * @text 物品属性设置
 * @type struct<GenericItem>[]
 * @default []
 * @desc 设置物品/武器/防具的属性，使其可以作为配方中通用的工具或材料。
 *
 * @param Categories
 * @type struct<CategoryData>[]
 * @default []
 * @desc Set up crafting categories here
 *
 * @param Crafting Options
 *
 * @param Automatic Learn
 * @parent Crafting Options
 * @type boolean
 * @desc If true, recipes will automatically be discovered when the player gets the items for a recipe
 * @default false
 *
 * @param Allow Multicraft
 * @parent Crafting Options
 * @type boolean
 * @desc If true, the scene will ask the player how many they want to craft
 * @default false
 *
 * @param Wait For Finish
 * @parent Crafting Options
 * @type boolean
 * @desc If true, the user will not be able to do anything until their current craft finishes
 * @default false
 *
 * @param Collect Items At Station
 * @parent Crafting Options
 * @type boolean
 * @desc If true, items crafted will need to be collected at the crafting station they are made at
 * @default true
 *
 * @param Window Options
 * @text 窗口设置
 *
 * @param Instant Crafting
 * @parent Window Options
 * @type boolean
 * @desc If true, will not show the progress window and will instantly craft everything
 * @default false
 *
 * @param Allow Filter
 * @parent Window Options
 * @type boolean
 * @desc If true, will allow the player to set filters for the recipes shown
 * @default true
 *
 * @param Transparent Windows
 * @text 窗口透明
 * @parent Window Options
 * @type boolean
 * @desc 设置合成界面的边框是否透明。（默认：不透明）
 * @default false
 *
 * @param Disable Touch UI Space
 * @parent Window Options
 * @type boolean
 * @desc If true, will not leave space for Touch UI buttons if Touch UI is disabled
 * @default false
 *
 * @param ScrollSpeed
 * @text 滚动速度
 * @parent Window Options
 * @type number
 * @min 0
 * @desc speed at which the recipe window display scrolls (if needed)
 * @default 1
 *
 * @param ScrollWait
 * @text 滚动等待
 * @parent Window Options
 * @type number
 * @min 0
 * @desc amount of time (in frames) to wait before beginning to scroll
 * @default 300
 *
 * @param Scroll Deceleration
 * @text 滚动减速
 * @parent Window Options
 * @type number
 * @min 0.01
 * @max 0.99
 * @decimals 2
 * @desc Rate of deceleration after letting go of touch
 * @default 0.92
 *
 * @param Auto Scroll
 * @text 自动滚动
 * @parent Window Options
 * @type boolean
 * @desc Determine if the display window should automatically scroll after so long of no user input
 * @default true
 *
 * @param Category Sort Order
 * @parent Window Options
 * @type text[]
 * @desc The order categories appear in when all are showing
 * @default []
 *
 * @param List Window Width
 * @parent Window Options
 * @type number
 * @min 0
 * @max 100
 * @desc % of the screen width the list window takes up
 * @default 33
 *
 * @param Multicraft Window Width
 * @parent Window Options
 * @type number
 * @min 0
 * @max 100
 * @desc % of the screen width the multicraft window takes up
 * @default 50
 *
 * @param List Window On Right
 * @parent Window Options
 * @type boolean
 * @default false
 * @desc If true, the list window will be on the right side of the screen
 *
 * @param Show Ingredient Amount
 * @text 显示材料库存数量
 * @parent Window Options
 * @type boolean
 * @default true
 * @desc 显示玩家持有的合成材料的数量。
 *
 * @param Show Tool Amount
 * @parent Window Options
 * @type boolean
 * @default false
 * @desc Show the current amount of tools the player has?
 *
 * @param Show Product Amount
 * @parent Window Options
 * @type boolean
 * @default false
 * @desc Show the current amount of the product the player has?
 *
 * @param Show Unique Product Amount
 * @parent Window Options
 * @type boolean
 * @default false
 * @desc Show the maximum amount of unique product a player can make?
 *
 * @param Show Fail Product Amount
 * @parent Window Options
 * @type boolean
 * @default false
 * @desc Show the current amount of the product the player has?
 *
 * @param Show High Quality Product Amount
 * @parent Window Options
 * @type boolean
 * @default false
 * @desc Show the current amount of the product the player has?
 *
 * @param Hide Uncraftable Recipes
 * @parent Window Options
 * @type boolean
 * @default false
 * @desc If true, recipes the player cannot craft will not show even if they learned them
 *
 * @param Show Confirm Window
 * @parent Window Options
 * @text 显示确定窗口
 * @type boolean
 * @desc 是否在制作时显示一个确认窗口。
 * @default false
 *
 * @param Show Progress Percentage
 * @text 显示进度
 * @parent Window Options
 * @desc 是否显示合成进度条中的百分比。
 * @type boolean
 * @default true
 *
 * @param Show Progress Time Remaining
 * @parent Window Options
 * @desc If true, the progress window will show time remaining.
 * @type boolean
 * @default false
 *
 * @param Progress Decimals
 * @parent Window Options
 * @desc Number of decimals to show on the progress percentage, if displayed
 * @type number
 * @default 2
 *
 * @param Show Category Window
 * @parent Window Options
 * @desc Whether to use the category window or not
 * @type boolean
 * @default true
 *
 * @param Display Window Info
 * @text 设置界面显示标签
 * @parent Window Options
 * @type select[]
 * @option Name
 * @option Image
 * @option Times Crafted
 * @option Success Rate
 * @option High Quality Rate
 * @option Quality
 * @option Subcategory
 * @option Exp
 * @option Level Required
 * @option Current Level
 * @option Description
 * @option Products
 * @option Tools
 * @option Ingredients
 * @option Fail Products
 * @option High Quality Products
 * @option Info Header
 * @option Description Header
 * @option Product Header
 * @option Tool Header
 * @option Ingredient Header
 * @option Fail Product Header
 * @option High Quality Product Header
 * @option Blank Line
 * @option Custom Space
 * @desc 设置合成界面中你想要显示的项目、标签和内容。
 * @default ["Name","Image","Times Crafted","Success Rate","Quality","Exp","Level Required","Current Level","Description","Product Header","Products","Tool Header","Tools","Ingredient Header","Ingredients"]
 *
 * @param Custom Space
 * @parent Window Options
 * @desc Amount of blank vertical space to leave when Custom Space display parameter is used
 * @type number
 * @default 6
 *
 * @param Category Filter Options
 * @parent Window Options
 * @desc Category filter options (only shown if the scene does not already filter by category with category window). Must match recipe profession exactly.
 * @type text[]
 * @default []
 *
 * @param Subcategory Filter Options
 * @parent Window Options
 * @desc Subcategory filter options (always shown if filter is enabled). Must match recipe subcategory exactly.
 * @type text[]
 * @default []
 *
 * @param Quality Filter Options
 * @parent Window Options
 * @desc Quality filter options (always shown if filter is enabled). Must match recipe quality exactly.
 * @type text[]
 * @default []
 *
 * @param Filter Button Width
 * @parent Window Options
 * @desc Width of the filter touch UI button (in multiples of 48)
 * @type number
 * @default 1
 *
 * @param Filter Button Offset
 * @parent Window Options
 * @desc Filter Button index on the button sheet
 * @type number
 * @default 4
 *
 * @param Filter Key
 * @parent Window Options
 * @desc Key that when pressed will start the filter flow
 * @default f
 *
 * @param Filter Gamepad
 * @parent Window Options
 * @desc Gamepad button that when pressed will start the filter flow
 * @type select
 * @option A
 * @value 0
 * @option B
 * @value 1
 * @option X
 * @value 2
 * @option Y
 * @value 3
 * @option LB
 * @value 4
 * @option RB
 * @value 5
 * @option LT
 * @value 6
 * @option RT
 * @value 7
 * @option Back / Select
 * @value 8
 * @option Start
 * @value 9
 * @option Left Stick
 * @value 10
 * @option Right Stick
 * @value 11
 * @option Dpad Up
 * @value 12
 * @option Dpad Down
 * @value 13
 * @option Dpad Left
 * @value 14
 * @option Dpad Right
 * @value 15
 * @default 5
 *
 * @param Text Options
 * @text 文本描述设置
 *
 * @param Description Alignment
 * @text 描述位置
 * @parent Text Options
 * @type select
 * @option left
 * @option center
 * @option right
 * @desc 描述位置：Left-靠左，Center-居中，Right-靠右。
 * @default left
 *
 * @param Success Rate Text
 * @text 成功率的描述
 * @parent Text Options
 * @desc 关于配方制作成功率的文本描述。
 * @default 成功率: 
 *
 * @param High Quality Rate Text
 * @parent Text Options
 * @desc Text to show to describe the high quality rate of a recipe
 * @default High Quality Rate: 
 *
 * @param Quality Label Text
 * @parent Text Options
 * @desc Text to show to describe the quality of a recipe
 * @default Quality: 
 *
 * @param Subcategory Label Text
 * @parent Text Options
 * @desc Text to show to describe the subcategory of a recipe
 * @default Specialization: 
 *
 * @param Times Crafted Text
 * @text 制作次数的描述
 * @parent Text Options
 * @desc 关于配方制作成功的次数统计的文本描述。
 * @default 制作次数: 
 *
 * @param Experience Text
 * @text 经验值的描述
 * @parent Text Options
 * @desc 关于获得经验值的文本描述(需要CGMZ Professions插件)
 * @default 获得经验: 
 *
 * @param Level Requirement Text
 * @text 专业等级要求的描述
 * @parent Text Options
 * @desc 关于制作需要的专业技能等级要求的文本描述(需要CGMZ Professions插件)
 * @default 专业等级要求: 
 *
 * @param Current Level Text
 * @text 当前专业等级的描述
 * @parent Text Options
 * @desc 关于当前的专业技能等级的文本描述(需要CGMZ Professions插件)
 * @default 专业等级: 
 *
 * @param Level Abbreviation Text
 * @text 专业等级缩写的描述
 * @parent Text Options
 * @desc 关于等级的缩写文本描述(需要CGMZ Professions插件)
 * @default Lv.
 *
 * @param Amount X Text
 * @parent Text Options
 * @desc Text to show to describe the amount x
 * @default x
 *
 * @param Craft Confirm Text
 * @text 确认合成的描述
 * @parent Text Options
 * @desc 关于确认并开始合成的文本描述。
 * @default 合成
 *
 * @param Craft Cancel Text
 * @text 取消合成的描述
 * @parent Text Options
 * @desc 关于取消这次合成的文本描述。
 * @default 取消
 *
 * @param Progress Text
 * @text 进度的描述
 * @parent Text Options
 * @desc 关于合成时进度的文本描述。
 * @default 进度 
 *
 * @param Success Text
 * @text 合成成功的描述
 * @parent Text Options
 * @desc 关于合成成功时显示的文字描述。
 * @default 合成成功！
 *
 * @param Failure Text
 * @text 合成失败的描述
 * @parent Text Options
 * @desc 关于合成失败时显示的文字描述。
 * @default 合成失败！
 *
 * @param High Quality Text
 * @parent Text Options
 * @desc Text to show to describe a high quality craft
 * @default High Quality Craft!
 *
 * @param Craft Quantity Text
 * @parent Text Options
 * @desc Text to show to describe the amount the player wants to craft
 * @default Amount to Craft:
 *
 * @param Label Text Color
 * @text 标签颜色
 * @parent Text Options
 * @desc 合成界面中各种标签文字的颜色。（如制作次数、成功率等标签）
 * @type color
 * @default 16
 *
 * @param Progress Color1
 * @text 进度条的颜色1
 * @parent Text Options
 * @desc 合成进度条的第一种颜色。
 * @type color
 * @default 28
 *
 * @param Progress Color2
 * @text 进度条的颜色2
 * @parent Text Options
 * @desc 合成进度条的第二种颜色。
 * @type color
 * @default 29
 *
 * @param Success Color
 * @text 合成成功的文本颜色
 * @parent Text Options
 * @desc 合成成功时显示的文本的颜色。
 * @type color
 * @default 29
 *
 * @param High Quality Color
 * @parent Text Options
 * @desc Color of the High Quality Craft Text
 * @type color
 * @default 29
 *
 * @param Failure Color
 * @text 合成失败的文本颜色
 * @parent Text Options
 * @desc 合成失败时显示的文本的颜色。
 * @type color
 * @default 18
 *
 * @param Header Color 1
 * @parent Text Options
 * @desc Color 1 of the gradient lines in headers
 * @type color
 * @default 1
 *
 * @param Header Color 2
 * @parent Text Options
 * @desc Color 2 of the gradient lines in headers
 * @type color
 * @default 0
 *
 * @param Info Header Text
 * @text 信息的标签
 * @parent Text Options
 * @desc 合成界面中关于配方信息标签的描述。（须在"设置界面显示标签"内增加）
 * @default 配方内容
 *
 * @param Desc Header Text
 * @text 配方描述的标签
 * @parent Text Options
 * @desc 合成界面中关于配方内容信息标签的描述。（须在"设置界面显示标签"内增加）
 * @default 配方描述
 *
 * @param Ingredient Header Text
 * @text 配方材料的标签
 * @parent Text Options
 * @desc 合成界面中关于配方材料标签的描述。
 * @default 材料
 *
 * @param Tool Header Text
 * @text 制作工具的标签
 * @parent Text Options
 * @desc 合成界面中关于制作工具标签的描述。
 * @default 工具
 *
 * @param Product Header Text
 * @text 配方产物的标签
 * @parent Text Options
 * @desc 合成界面中关于成功产物标签的描述。
 * @default 产品
 *
 * @param Fail Product Header Text
 * @text 失败产物的标签
 * @parent Text Options
 * @desc 合成界面中关于失败产物标签的描述。
 * @default 废品
 *
 * @param High Quality Product Header Text
 * @parent Text Options
 * @desc Text in the High Quality Product Header of display window
 * @default High Quality Products
 *
 * @param Unique Text
 * @parent Text Options
 * @desc Text to show for unique products
 * @default Unique (%unique)
 *
 * @param No Recipe Text
 * @parent Text Options
 * @desc Text to show when the scene has no recipes
 * @default Keep playing the game to learn more recipes!
 *
 * @param Percent Text
 * @parent Text Options
 * @desc Text to show after progress percentage
 * @default  %
 *
 * @param Sec Text
 * @parent Text Options
 * @desc Text to show when there are seconds remaining on progress bar
 * @default sec
 *
 * @param Min Text
 * @parent Text Options
 * @desc Text to show when there are minutes remaining on progress bar
 * @default min
 *
 * @param Hr Text
 * @parent Text Options
 * @desc Text to show when there are hours remaining on progress bar
 * @default hr
 *
 * @param Remove Filter Text
 * @parent Text Options
 * @desc Text to show for the Remove Filter option
 * @default Remove Filter
 *
 * @param Category Filter Text
 * @parent Text Options
 * @desc Text to show for the Category Filter option
 * @default Category
 *
 * @param Subcategory Filter Text
 * @parent Text Options
 * @desc Text to show for the Subcategory Filter option
 * @default Subcategory
 *
 * @param Quality Filter Text
 * @parent Text Options
 * @desc Text to show for the Quality Filter option
 * @default Quality
 *
 * @param Collect Item Confirm Text
 * @parent Text Options
 * @desc Text for the confirm option in the collected items window
 * @default Collect
 *
 * @param Collect Item Item Text
 * @parent Text Options
 * @desc Text for an item that was collected in the crafting scene
 * @default %amountx %icon%name
 *
 * @param Collect Item Weapon Text
 * @parent Text Options
 * @desc Text for a weapon that was collected in the crafting scene
 * @default %amountx %icon%name
 *
 * @param Collect Item Armor Text
 * @parent Text Options
 * @desc Text for an armor that was collected in the crafting scene
 * @default %amountx %icon%name
 *
 * @param Collect Item Currency Text
 * @parent Text Options
 * @desc Text for an armor that was collected in the crafting scene
 * @default %icon%amount %color%unit
 *
 * @param Collected Items Text
 * @parent Text Options
 * @desc Text to display at the top of the collected items window
 * @default Collected Items:
 *
 * @param Integrations
 * @text 其他关联插件的设置
 *
 * @param Learn Toast
 * @parent Integrations
 * @desc [CGMZ] Toast Manager preset id to show when learning a recipe
 *
 * @param Complete Toast
 * @parent Integrations
 * @desc [CGMZ] Toast Manager preset id to show when a craft is completed
 *
 * @param Always Award Exp
 * @text 失败经验
 * @parent Integrations
 * @type boolean
 * @default true
 * @desc 合成失败时是否也依然获得经验？ (需要CGMZ Professions插件)
 *
 * @param Hide High Level Recipes
 * @parent Integrations
 * @type boolean
 * @default true
 * @desc Hide recipes the player does not meet level requirements for?
 *
 * @param Close On Level Up
 * @text 专业升级时关闭合成界面
 * @parent Integrations
 * @type boolean
 * @desc 当专业技能等级提升时是否关闭合成界面。(需要CGMZ Professions插件)
 * @default false
 *
 * @param Fail Rumble
 * @parent Integrations
 * @type struct<Rumble>
 * @default {"Duration":"0","Weak Magnitude":"1.00","Strong Magnitude":"1.00","Start Delay":"0"}
 * @desc [CGMZ] Rumble to play when successful craft finishes
 *
 * @param Success Rumble
 * @parent Integrations
 * @type struct<Rumble>
 * @default {"Duration":"0","Weak Magnitude":"1.00","Strong Magnitude":"1.00","Start Delay":"0"}
 * @desc [CGMZ] Rumble to play when successful craft finishes
 *
 * @param Scene Background
 * @parent Integrations
 * @desc [CGMZ] Scene Backgrounds preset id to use for the crafting scene
 *
 * @param Category Window Settings
 * @parent Integrations
 * @desc [CGMZ] Window Settings preset to use for the category window
 *
 * @param List Window Settings
 * @parent Integrations
 * @desc [CGMZ] Window Settings preset to use for the list window
 *
 * @param Display Window Settings
 * @parent Integrations
 * @desc [CGMZ] Window Settings preset to use for the display window
 *
 * @param Progress Window Settings
 * @parent Integrations
 * @desc [CGMZ] Window Settings preset to use for the progress window
 *
 * @param Confirm Window Settings
 * @parent Integrations
 * @desc [CGMZ] Window Settings preset to use for the confirm window
 *
 * @param Multicraft Window Settings
 * @parent Integrations
 * @desc [CGMZ] Window Settings preset to use for the multicraft window
 *
 * @param Filter Type Window Settings
 * @parent Integrations
 * @desc [CGMZ] Window Settings preset to use for the filter type window
 *
 * @param Filter Option Window Settings
 * @parent Integrations
 * @desc [CGMZ] Window Settings preset to use for the filter option window
 *
 * @param Collected Items Window Settings
 * @parent Integrations
 * @desc [CGMZ] Window Settings preset to use for the collected items window
 *
 * @param Collect Items Confirm Window Settings
 * @parent Integrations
 * @desc [CGMZ] Window Settings preset to use for the collect items window
 *
 * @param Category Window Background
 * @parent Integrations
 * @desc [CGMZ] Window Backgrounds preset to use for the category window
 *
 * @param List Window Background
 * @parent Integrations
 * @desc [CGMZ] Window Backgrounds preset to use for the list window
 *
 * @param Display Window Background
 * @parent Integrations
 * @desc [CGMZ] Window Backgrounds preset to use for the display window
 *
 * @param Progress Window Background
 * @parent Integrations
 * @desc [CGMZ] Window Backgrounds preset to use for the progress window
 *
 * @param Confirm Window Background
 * @parent Integrations
 * @desc [CGMZ] Window Backgrounds preset to use for the confirm window
 *
 * @param Multicraft Window Background
 * @parent Integrations
 * @desc [CGMZ] Window Backgrounds preset to use for the multicraft window
 *
 * @param Filter Type Window Background
 * @parent Integrations
 * @desc [CGMZ] Window Backgrounds preset to use for the filter type window
 *
 * @param Filter Option Window Background
 * @parent Integrations
 * @desc [CGMZ] Window Backgrounds preset to use for the filter option window
 *
 * @param Collected Items Window Background
 * @parent Integrations
 * @desc [CGMZ] Window Backgrounds preset to use for the collected items window
 *
 * @param Collect Items Confirm Window Background
 * @parent Integrations
 * @desc [CGMZ] Window Backgrounds preset to use for the collect items confirm window
*/
/*~struct~Recipe:zh-CN
 * @param Name
 * @text 配方名称
 * @desc 设置一个配方的名称。
 *
 * @param Display Name
 * @desc The displayed name of the recipe (if different than name).
 *
 * @param Products
 * @text 产品
 * @type struct<Item>[]
 * @default []
 * @desc 合成成功时获得的物品。
 *
 * @param Fail Products
 * @text 废品
 * @type struct<Item>[]
 * @default []
 * @desc 合成失败时获得的物品。可选设置。
 *
 * @param Ingredients
 * @text 材料
 * @type struct<Item>[]
 * @default []
 * @desc 合成所需的材料。
 *
 * @param Tools
 * @text 工具
 * @type struct<Item>[]
 * @default []
 * @desc 合成所需的工具，不会被消耗。可选设置。
 * 
 * @param Discovered
 * @text 是否学会
 * @type boolean
 * @default false
 * @desc 新游戏开始时是否已学会该配方。
 *
 * @param Picture
 * @text 大图标
 * @type file
 * @dir img/
 * @desc 设置一张64*64图片作为配方内显示的大图标。不设置为无图标。
 * 
 * @param Icon
 * @text 小图标
 * @type icon
 * @default 0
 * @desc 从IconSet内选择一个图标作为配方列表内的配方图标。
 *
 * @param Description
 * @text 配方描述
 * @type multiline_string
 * @desc 设置配方的描述。
 *
 * @param Success Rate
 * @text 成功率
 * @type number
 * @min -99999
 * @max 100
 * @default 100
 * @desc 设置配方的成功率百分比值。
 *
 * @param Time
 * @text 合成时间
 * @type number
 * @min 1
 * @default 120
 * @desc 设置合成该配方所需的时间。（单位：帧，60帧=1秒）
 *
 * @param Success Rate Per Level
 * @text 专业成功率加成
 * @type number
 * @min -99999
 * @max 100
 * @default 0
 * @desc 设置该配方关于专业技能等级的等级加成。(需要CGMZ Professions插件)
 *
 * @param Experience
 * @text 获得经验
 * @type number
 * @min 1
 * @default 1
 * @desc 设置合成该配方后可获得的专业技能经验值。(需要CGMZ Professions插件)
 *
 * @param Profession
 * @text 专业
 * @desc 设置该配方的专业类型，如锻造、炼金术等。（当使用CGMZ Professions插件后，会指向到专业插件设定的专业技能）
 *
 * @param Subcategory
 * @desc If you want, you can add a subcategory to the recipe. This does not affect how the recipe functions.
 *
 * @param Quality
 * @desc If you want, you can add a quality to the recipe. This does not affect how the recipe functions.
 *
 * @param Level Requirement
 * @text 专业等级要求
 * @type number
 * @default 1
 * @min 0
 * @desc 设置合成该配方所需要的专业技能等级。(需要CGMZ Professions插件)
 *
 * @param Craft Sound Effect
 * @text 合成音效
 * @type struct<SoundEffect>
 * @default {"Name":"","Volume":"90","Pitch":"100","Pan":"0"}
 * @desc 设置合成该配方时播放的音效。
 *
 * @param Fail Sound Effect
 * @text 失败音效
 * @type struct<SoundEffect>
 * @default {"Name":"","Volume":"90","Pitch":"100","Pan":"0"}
 * @desc 设置合成失败时播放的音效。
 *
 * @param Success Sound Effect
 * @text 成功音效
 * @type struct<SoundEffect>
 * @default {"Name":"","Volume":"90","Pitch":"100","Pan":"0"}
 * @desc 设置合成成功时播放的音效。
 *
 * @param Learn Toast
 * @desc [CGMZ] Toast Manager preset id to use for learning this specific recipe only
 *
 * @param Complete Toast
 * @desc [CGMZ] Toast Manager preset id to use for crafting this specific recipe only
 *
 * @param Disable Learn Toast
 * @type boolean
 * @default false
 * @desc Do not show the learn toast for only this recipe?
 *
 * @param Disable Complete Toast
 * @type boolean
 * @default false
 * @desc Do not show the complete toast for only this recipe?
 *
 * @param Disable Autodiscover
 * @type boolean
 * @default false
 * @desc If true, this recipe must be manually discovered.
 *
 * @param Learn On Craft
 * @text 领悟新配方
 * @type struct<OnCraftRecipe>[]
 * @default []
 * @desc 设置当合成时会领悟的新配方。领悟新配方的序号对应领悟几率的序号。
 *
 * @param Unlearn On Craft
 * @type struct<OnCraftRecipe>[]
 * @default []
 * @desc Recipe name(s) to unlearn when this recipe is crafted
 *
 * @param Success Variable
 * @text 获取制作次数变量
 * @type variable
 * @default 0
 * @desc 指定一个变量来获取某个配方的合成次数。
 *
 * @param Product Unique Amount
 * @type number
 * @default 0
 * @desc Amount of the Product items in inventory before the recipe can no longer be crafted
 *
 * @param High Quality Products
 * @type struct<Item>[]
 * @default []
 * @desc The items produced by crafting the recipe when a high quality craft occurs
 *
 * @param High Quality Chance
 * @type number
 * @min 0
 * @max 100
 * @default 0
 * @desc The chance a craft will instead be high quality
 *
 * @param High Quality Chance Per Level
 * @type number
 * @min -100
 * @max 100
 * @default 0
 * @desc The change (additive) in % chance that the craft will be high quality per level beyond the level requirement. (Requires CGMZ Professions)
*/
/*~struct~OnCraftRecipe:zh-CN
 * @param Recipe Name
 * @desc The recipe name to learn/unlearn
 *
 * @param Chance
 * @type number
 * @default 100
 * @desc The chance to learn/unlearn the recipe.
*/
/*~struct~Item:zh-CN
 * @param Item
 * @text 物品
 * @type item
 * @default 0
 * @desc 当选择物品作为项目参数时，请勿同时设置武器、防具、金钱或物品属性。
 *
 * @param Weapon
 * @text 武器
 * @type weapon
 * @default 0
 * @desc 当选择武器作为项目参数时，请勿同时设置物品、防具、金钱或物品属性。
 *
 * @param Armor
 * @text 防具
 * @type armor
 * @default 0
 * @desc 当选择防具作为项目参数时，请勿同时设置物品、武器、金钱或物品属性。
 *
 * @param Gold
 * @text 金钱
 * @type boolean
 * @default false
 * @desc 设置True时，请勿同时设置物品、武器、防具或物品属性。[数量]的设置也将变为金钱数而不是物品数。
 *
 * @param Generic
 * @text 物品属性
 * @desc 设当选择物品属性作为项目参数时，请勿同时设置物品、武器、防具或金钱。该设置只适用于材料或工具。
 * 
 * @param Amount
 * @text 数量
 * @type number
 * @default 1
 * @desc 设置项目所需物品或金钱的数量。
 *
 * @param Currency Id
 * @desc [CGMZ] Currency System currency id if Gold is set
*/
/*~struct~GenericItem:zh-CN
 * @param Type
 * @text 属性名称（参数）
 * @desc 指定一个物品属性的名称，该名称会作为插件相关设置的参数使用。如在备注命令、配方设置中。
 *
 * @param Display Name
 * @text 属性名称（显示）
 * @desc 设置你想显示的该物品属性的名称。如：炼金器材、采矿工具等。
 *
 * @param Icon Index
 * @text 属性图标
 * @type icon
 * @default 0
 * @min 0
 * @desc 设置一个用来表示该类物品属性的图标。
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
/*~struct~CategoryData:zh-CN
 * @param Id
 * @desc The id (profession) of the category
 *
 * @param Command Text
 * @desc The text to show for this category in the category window
*/
/*~struct~Rumble:zh-CN
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
 * @url https://www.caspergaming.com/plugins/cgmz/crafting/
 * @target MZ
 * @base CGMZ_Core
 * @orderAfter CGMZ_Core
 * @plugindesc Añade un sistema de creación a tu juego.
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
 * Versión: 1.12.1
 * ----------------------------------------------------------------------------
 * Compatibilidad: Sólo probado con mis CGMZ plugins.
 * Hecho para RPG Maker MZ 1.10.0
 * ----------------------------------------------------------------------------
 * Descripción: Agrega un sistema de creación a tu juego que funciona bien con 
 * CGMZ Professions. Puede manejar los requisitos de artículos (consumidos en 
 * la elaboración), los requisitos de herramientas (no consumidos en la
 * elaboración), los productos (producidos con éxito en la elaboración) y los 
 * productos  defectuosos (producidos en la elaboración fallida). Las recetas 
 * se pueden  descubrir usando un elemento o mediante un comando de plugin.
 * ----------------------------------------------------------------------------
 * Documentación:
 * -----------------------Partidas guardadas-----------------------------------
 * Este plugin admite parcialmente juegos guardados. Se admite la adición de 
 * nuevas recetas, pero no se admite la eliminación o modificación de recetas
 * existentes. Esto se debe a que todo se guarda cuando se guarda el juego. 
 * Cualquier problema con recetas eliminadas/modificadas solo ocurrirá en  
 * juegos guardados previamente. 
 *
 * La información de elementos genéricos admite juegos guardados por completo,
 * puede agregar/editar/eliminar información de elementos genéricos en los 
 * juegos guardados. Sin embargo, el tipo de artículo genérico en los 
 * parámetros de la receta no admite partidas guardadas.
 * --------------------Elaboración de entrada/salida---------------------------
 * Las recetas tienen 4 categorías de elementos asociados a ellas: "Productos",
 * "Productos fallidos", "Herramientas" e "Ingredientes".
 * - Los productos son artículos que el jugador recibe directamente en el 
 *   inventario en una elaboración exitosa.
 * - Los productos fallidos son elementos que el jugador recibe directamente
 *   en el inventario de una nave fallida. Este es un parámetro opcional.
 * - Las herramientas son elementos necesarios para elaborar la receta, pero
 *   que NO se consumen en la elaboración.
 * - Los ingredientes son elementos necesarios para elaborar la receta y se 
 *   consumen en la elaboración.
 * --------------------------High Quality Crafts-------------------------------
 * Recipes can also have high quality craft results if high quality products
 * are set up. These are special products that occur only in the case the
 * player both succeeds the craft (did not fail the craft) and rolled a second
 * high quality check to see if the craft outcome is a high quality craft.
 *
 * High quality craft items functions similar to regular product items.
 * ----------------------Tipos de artículos genéricos--------------------------
 * Las recetas pueden usar "tipos de elementos genéricos" para sus ingredientes
 * o herramientas. Lo que esto significa es que si necesita una herramienta de 
 * "sartén" para elaborar una "tortilla", puede designar su "sartén de hierro"
 * y su "sartén de acero" como un elemento de tipo "sartén", y luego cualquiera
 * podría usarse para elaborar la "tortilla".
 *
 * Consulte la sección de etiquetas de notas para configurar el tipo de
 * elemento.
 *
 * Precaución: Se debe tener cuidado si un artículo pertenece a varias
 * categorías genéricas y la receta usa varias categorías genéricas, ya que el
 * artículo podría contarse dos veces al calcular la cantidad de artículos que
 * tiene el jugador.
 * --------------------------Etiquetas de notas--------------------------------
 * Puedes hacer artículos que, cuando se usan, harán que se aprenda una receta.
 * Para hacerlo, coloque la siguiente etiqueta en su notebox:
 * <cgmzrecipe:RecipeName>
 * Y reemplace RecipeName con el nombre de la receta.
 *
 * Puedes hacer armas o armaduras que, cuando estén equipadas, aumentarán la 
 * probabilidad de éxito de una receta. Para hacerlo, coloque las siguientes 
 * etiquetas de notas en el cuadro de notas del equipo:
 * <cgmzrecipetype:ProfessionName>
 * <cgmzrecipebonus:BonusAmount> 
 *
 * Puedes hacer tipos "genéricos" de artículos/armas/armaduras para usar como 
 * ingredientes o herramientas. Para hacerlo, ingrese la siguiente etiqueta de
 * nota en cualquier caja de notas de artículo/armadura/arma:
 * <cgmzcraftinggeneric:GenericType,GenericType2,etc>
 * Precaución: ¡Tenga en cuenta que los genéricos SOLO pueden usarse para 
 * ingredientes o herramientas!
 *
 * Nota: Al escribir una etiqueta de nota, se distingue entre mayúsculas y 
 * minúsculas, lo que significa que "Cocinar" no es lo mismo que "cOOcinar".
 * --------------------------------Filters-------------------------------------
 * This plugin has the option to allow your player to filter the current list
 * of crafting recipes they are looking at. The following filter types are
 * available:
 *
 * Category Filter
 * This filter is only available if not already using the Category window. To
 * use it, type the categories you want to be available to filter by and 
 * ensure they match the recipe's Profession parameters.
 *
 * Subcategory Filter
 * This filter is always available if enabled. To use it, you need to set up
 * which category (or all) it should show up for as well as the specific
 * recipe Subcategory it should match. To do this, use a colon ":" to separate
 * the category from the subcategory.
 *
 * For example, if you had a recipe of category (profession) "Cooking" and you
 * wanted to be able to filter that category by the subcategory "Spices", you
 * would make your filter entry: Cooking:Spices
 *
 * This supports "All" for the category where the filter subcategory will
 * always appear no matter which category is selected. For example: All:Spices
 *
 * Quality Filter
 * This filter is always available if enabled. To use it, you need to set up
 * which category (or all) it should show up for as well as the specific
 * recipe Quality it should match. To do this, use a colon ":" to separate
 * the category from the quality.
 *
 * For example, if you had a recipe of category (profession) "Cooking" and you
 * wanted to be able to filter that category by the quality "Common", you
 * would make your filter entry: Cooking:Common
 *
 * This supports "All" for the category where the filter quality will
 * always appear no matter which category is selected. For example: All:Common
 * -------------------------------Scene Keys-----------------------------------
 * The plugin command that includes scene keys help differentiate crafting
 * scenes. By default, once something is crafting, it cannot have another
 * craft of the same item start. However, if the scene key is different
 * between the two scenes, it will be recognized as a different item and you
 * can craft multiple of the same craft at once.
 *
 * Scene keys can also come in handy when checking if a specific crafting
 * station is currently crafting something. Note that it is important your
 * scene key is something unique, and should not be "crafting" or match a
 * recipe's name.
 * ------------------------Comandos de Plugin----------------------------------
 * Este plugin admite los siguientes comandos:
 * • Descubrir
 * Descubre (o no descubre) la receta dada por su nombre
 *
 * • Discover Batch
 * Discovers (or undiscovers) the given recipes by name
 *
 * • Escena de llamada
 * Llama a la escena de elaboración. Especifique un tipo de profesión
 * (distingue entre mayúsculas y minúsculas) para incluir solo ciertas 
 * recetas en la escena.
 *
 * • Is Scene Crafting
 * Check if the scene with the provided scene key is currently crafting a
 * recipe. Sets a game switch to TRUE/FALSE based on result of the check, for
 * use in eventing.
 *
 * • Is Recipe Discovered
 * Check if the recipe is discovered, and sets a switch to on/off depending
 * on the result. On = discovered, Off = undiscovered
 *
 * • Descripción del conjunto
 * Cambia la descripción de la receta dada.
 *
 * • Reinicializar
 * Reinicializa los datos de elaboración como si hubieras comenzado un nuevo
 * juego.
 * ------------------------Llamando a la escena--------------------------------
 * El JS para llamar a la escena es: SceneManager.push(CGMZ_Scene_Crafting);
 * También puedes preparar la escena de elaboración para mostrar solo ciertas 
 * recetas con:
 * SceneManager.prepareNextScene(["Type", "Type2"]);
 * ----------------------------Unique Products---------------------------------
 * Unique products are products which can only be crafted up until the player
 * has the Product Unique Amount of that item in their inventory. For example,
 * you could make the Ultimate Sword which has a Product Unique Amount of 1,
 * which would mean players can only craft 1 of these swords (at least, until 
 * the first sword is no longer in their party's inventory).
 * 
 * Unique products only support item/weapon/armors. You cannot use unique
 * products to restrict currency or generic item products. Unique Products are
 * only for products, not fail products.
 * -----------------------------Toast Manager----------------------------------
 * In your toast presets, you can use the text code %name to show the recipe
 * name in your toast.
 * -----------------------------Filename---------------------------------------
 * The filename for this plugin MUST remain CGMZ_Crafting.js
 * This is what it comes as when downloaded. The filename is used to load
 * parameters and execute plugin commands. If you change it, things will begin
 * behaving incorrectly and your game will probably crash. Please do not
 * rename the js file.
 * --------------------------Latest Version------------------------------------
 * Hi all, this latest version fixes a small bug that could get a player stuck
 * in the craft scene if the plugin is set to wait for crafting to finish and
 * your recipe auto-discovers a new recipe above it in the list window.
 *
 * Version 1.12.1
 * - Fix bug with getting stuck when wait for craft finish enabled
 *
 * @command discover
 * @text Descubrir
 * @desc Descubrir (o no descubrir) una receta.
 *
 * @arg name
 * @text Nombre de Receta
 * @desc El nombre de la receta a descubrir.
 *
 * @arg discover
 * @type boolean
 * @text Descubrir
 * @desc Ya sea para descubrir o no descubrir la receta.
 * @default true
 *
 * @command Discover Batch
 * @desc Discover (or undiscover) multiple recipes
 *
 * @arg Recipes
 * @type text[]
 * @default []
 * @desc Enter names of recipes to discover / undiscover here
 *
 * @arg Discover
 * @type boolean
 * @desc Whether to discover or undiscover the recipes
 * @default true
 *
 * @command Call Scene
 * @text Escena de llamada 
 * @desc Llama a la escena de la elaboración.
 *
 * @arg type
 * @type text[]
 * @text Tipo
 * @desc El tipo de recetas a incluir. Deje esto en blanco para incluir todas las recetas descubiertas.
 * @default []
 *
 * @arg View Only
 * @type boolean
 * @desc If true, the player will not be able to craft, only view recipe info
 * @default false
 *
 * @arg Scene Key
 * @desc Key used to differentiate from recipes crafting in other scenes
 *
 * @arg Background Preset
 * @desc [CGMZ] Scene Backgrounds preset id to use for the scene (leave blank to use default)
 *
 * @command Is Scene Crafting
 * @desc Check if a scene currently has recipes crafting
 *
 * @arg Scene Key
 * @desc The scene's key to check, this must be filled out
 *
 * @arg Game Switch
 * @default 0
 * @type switch
 * @desc The game switch to turn on or off based on the result
 *
 * @command Is Recipe Discovered
 * @desc Check if a recipe is discovered
 *
 * @arg Recipe Name
 * @desc The name of the recipe to check
 *
 * @arg Switch
 * @type switch
 * @desc The switch to set to true if discovered
 * @default 0
 *
 * @command Set Description
 * @text Configurar Descripción
 * @desc Establecer la descripción de una receta.
 *
 * @arg name
 * @text Nombre de receta
 * @desc El nombre de la receta para cambiar la descripción (distingue entre mayúsculas y minúsculas).
 *
 * @arg description
 * @text Descripción
 * @type multiline_string
 * @desc La nueva descripción.
 *
 * @command Reinitialize
 * @text Reinicializar
 * @desc Restablece todos los datos de elaboración. Úselo para juegos guardados para reconocer datos modificados.
 *
 * @param Recipes
 * @text Recetas
 * @type struct<Recipe>[]
 * @default []
 * @desc Configurar recetas aquí.
 *
 * @param Generic Items
 * @text Artículos genéricos
 * @type struct<GenericItem>[]
 * @default []
 * @desc Configurar artículos genéricos aquí.
 *
 * @param Categories
 * @type struct<CategoryData>[]
 * @default []
 * @desc Set up crafting categories here
 *
 * @param Crafting Options
 *
 * @param Automatic Learn
 * @parent Crafting Options
 * @type boolean
 * @desc If true, recipes will automatically be discovered when the player gets the items for a recipe
 * @default false
 *
 * @param Allow Multicraft
 * @parent Crafting Options
 * @type boolean
 * @desc If true, the scene will ask the player how many they want to craft
 * @default false
 *
 * @param Wait For Finish
 * @parent Crafting Options
 * @type boolean
 * @desc If true, the user will not be able to do anything until their current craft finishes
 * @default false
 *
 * @param Collect Items At Station
 * @parent Crafting Options
 * @type boolean
 * @desc If true, items crafted will need to be collected at the crafting station they are made at
 * @default true
 *
 * @param Window Options
 * @text Opciones de ventana
 *
 * @param Instant Crafting
 * @parent Window Options
 * @type boolean
 * @desc If true, will not show the progress window and will instantly craft everything
 * @default false
 *
 * @param Allow Filter
 * @parent Window Options
 * @type boolean
 * @desc If true, will allow the player to set filters for the recipes shown
 * @default true
 *
 * @param Transparent Windows
 * @text Ventana transparente
 * @parent Window Options
 * @type boolean
 * @desc Si las ventanas de elaboración son transparentes o no. 
 * @default false
 *
 * @param Disable Touch UI Space
 * @parent Window Options
 * @type boolean
 * @desc If true, will not leave space for Touch UI buttons if Touch UI is disabled
 * @default false
 *
 * @param ScrollSpeed
 * @text Velocidad de desplazamiento
 * @parent Window Options
 * @type number
 * @min 0
 * @desc Velocidad a la que se desplaza la ventana de recetas (si es necesario)
 * @default 1
 *
 * @param ScrollWait
 * @text Espera de desplazamiento
 * @parent Window Options
 * @type number
 * @min 0
 * @desc Cantidad de tiempo (en fotogramas) a esperar antes de comenzar a desplazarse
 * @default 300
 *
 * @param Scroll Deceleration
 * @text Desaceleración de desplazamiento
 * @parent Window Options
 * @type number
 * @min 0.01
 * @max 0.99
 * @decimals 2
 * @desc Tasa de desaceleración después de soltar el toque.
 * @default 0.92
 *
 * @param Auto Scroll
 * @text Desplazamiento automático
 * @parent Window Options
 * @type boolean
 * @desc Determinar si la ventana de visualización debe desplazarse automáticamente después de tanto tiempo sin intervención del usuario.
 * @default true
 *
 * @param Category Sort Order
 * @parent Window Options
 * @type text[]
 * @desc The order categories appear in when all are showing
 * @default []
 *
 * @param List Window Width
 * @parent Window Options
 * @type number
 * @min 0
 * @max 100
 * @desc % of the screen width the list window takes up
 * @default 33
 *
 * @param Multicraft Window Width
 * @parent Window Options
 * @type number
 * @min 0
 * @max 100
 * @desc % of the screen width the multicraft window takes up
 * @default 50
 *
 * @param List Window On Right
 * @parent Window Options
 * @type boolean
 * @default false
 * @desc If true, the list window will be on the right side of the screen
 *
 * @param Show Ingredient Amount
 * @text Mostrar cantidad de ingrediente
 * @parent Window Options
 * @type boolean
 * @default true
 * @desc ¿Mostrar la cantidad actual de ingredientes que tiene el jugador?
 *
 * @param Show Tool Amount
 * @parent Window Options
 * @type boolean
 * @default false
 * @desc Show the current amount of tools the player has?
 *
 * @param Show Product Amount
 * @parent Window Options
 * @type boolean
 * @default false
 * @desc Show the current amount of the product the player has?
 *
 * @param Show Unique Product Amount
 * @parent Window Options
 * @type boolean
 * @default false
 * @desc Show the maximum amount of unique product a player can make?
 *
 * @param Show Fail Product Amount
 * @parent Window Options
 * @type boolean
 * @default false
 * @desc Show the current amount of the product the player has?
 *
 * @param Show High Quality Product Amount
 * @parent Window Options
 * @type boolean
 * @default false
 * @desc Show the current amount of the product the player has?
 *
 * @param Hide Uncraftable Recipes
 * @parent Window Options
 * @type boolean
 * @default false
 * @desc If true, recipes the player cannot craft will not show even if they learned them
 *
 * @param Show Confirm Window
 * @text Mostrar ventana de confirmación
 * @parent Window Options
 * @type boolean
 * @desc Determine si debe haber una ventana de confirmación adicional antes de comenzar a elaborar.
 * @default false
 *
 * @param Show Progress Percentage
 * @text Mostrar porcentaje de progreso
 * @parent Window Options
 * @desc Mostrar o no el texto de porcentaje de progreso.
 * @type boolean
 * @default true
 *
 * @param Show Progress Time Remaining
 * @parent Window Options
 * @desc If true, the progress window will show time remaining.
 * @type boolean
 * @default false
 *
 * @param Progress Decimals
 * @parent Window Options
 * @desc Number of decimals to show on the progress percentage, if displayed
 * @type number
 * @default 2
 *
 * @param Show Category Window
 * @parent Window Options
 * @desc Whether to use the category window or not
 * @type boolean
 * @default true
 *
 * @param Display Window Info
 * @text Información de la ventana de visualización
 * @parent Window Options
 * @type select[]
 * @option Name
 * @option Image
 * @option Times Crafted
 * @option Success Rate
 * @option High Quality Rate
 * @option Quality
 * @option Subcategory
 * @option Exp
 * @option Level Required
 * @option Current Level
 * @option Description
 * @option Products
 * @option Tools
 * @option Ingredients
 * @option Fail Products
 * @option High Quality Products
 * @option Info Header
 * @option Description Header
 * @option Product Header
 * @option Tool Header
 * @option Ingredient Header
 * @option Fail Product Header
 * @option High Quality Product Header
 * @option Blank Line
 * @option Custom Space
 * @desc Determina el orden y qué información muestra la ventana de visualización. 
 * @default ["Name","Image","Times Crafted","Success Rate","Quality","Exp","Level Required","Current Level","Description","Product Header","Products","Tool Header","Tools","Ingredient Header","Ingredients"]
 *
 * @param Custom Space
 * @parent Window Options
 * @desc Amount of blank vertical space to leave when Custom Space display parameter is used
 * @type number
 * @default 6
 *
 * @param Category Filter Options
 * @parent Window Options
 * @desc Category filter options (only shown if the scene does not already filter by category with category window). Must match recipe profession exactly.
 * @type text[]
 * @default []
 *
 * @param Subcategory Filter Options
 * @parent Window Options
 * @desc Subcategory filter options (always shown if filter is enabled). Must match recipe subcategory exactly.
 * @type text[]
 * @default []
 *
 * @param Quality Filter Options
 * @parent Window Options
 * @desc Quality filter options (always shown if filter is enabled). Must match recipe quality exactly.
 * @type text[]
 * @default []
 *
 * @param Filter Button Width
 * @parent Window Options
 * @desc Width of the filter touch UI button (in multiples of 48)
 * @type number
 * @default 1
 *
 * @param Filter Button Offset
 * @parent Window Options
 * @desc Filter Button index on the button sheet
 * @type number
 * @default 4
 *
 * @param Filter Key
 * @parent Window Options
 * @desc Key that when pressed will start the filter flow
 * @default f
 *
 * @param Filter Gamepad
 * @parent Window Options
 * @desc Gamepad button that when pressed will start the filter flow
 * @type select
 * @option A
 * @value 0
 * @option B
 * @value 1
 * @option X
 * @value 2
 * @option Y
 * @value 3
 * @option LB
 * @value 4
 * @option RB
 * @value 5
 * @option LT
 * @value 6
 * @option RT
 * @value 7
 * @option Back / Select
 * @value 8
 * @option Start
 * @value 9
 * @option Left Stick
 * @value 10
 * @option Right Stick
 * @value 11
 * @option Dpad Up
 * @value 12
 * @option Dpad Down
 * @value 13
 * @option Dpad Left
 * @value 14
 * @option Dpad Right
 * @value 15
 * @default 5
 *
 * @param Text Options
 * @text Opciones de Texto
 *
 * @param Description Alignment
 * @text Alineación de Descripción 
 * @parent Text Options
 * @type select
 * @option left
 * @option center
 * @option right
 * @desc La alineación del texto de la descripción.
 * @default left
 *
 * @param Success Rate Text
 * @text Texto de tasa de éxito
 * @parent Text Options
 * @desc Texto a mostrar para describir la tasa de éxito de una receta.
 * @default Success Rate: 
 *
 * @param High Quality Rate Text
 * @parent Text Options
 * @desc Text to show to describe the high quality rate of a recipe
 * @default High Quality Rate: 
 *
 * @param Quality Label Text
 * @parent Text Options
 * @desc Text to show to describe the quality of a recipe
 * @default Quality: 
 *
 * @param Subcategory Label Text
 * @parent Text Options
 * @desc Text to show to describe the subcategory of a recipe
 * @default Specialization: 
 *
 * @param Times Crafted Text
 * @text Texto de tiempo de elaboración 
 * @parent Text Options
 * @desc Texto a mostrar para describir la cantidad de veces que se ha elaborado una receta.
 * @default Times Crafted: 
 *
 * @param Experience Text
 * @text Texto Experiencia
 * @parent Text Options
 * @desc Texto a mostrar para describir la experiencia obtenida al elaborar la receta (Requiere profesiones CGMZ).
 * @default Exp Gain: 
 *
 * @param Level Requirement Text
 * @text Texto de requisito de nivel
 * @parent Text Options
 * @desc Texto a mostrar para describir el nivel requerido para elaborar la receta (Requiere Profesiones CGMZ).
 * @default Level Req: 
 *
 * @param Current Level Text
 * @text Texto de nivel actual
 * @parent Text Options
 * @desc Texto a mostrar para describir el nivel actual de la profesión requerida (Requiere CGMZ Professions).
 * @default Level: 
 *
 * @param Level Abbreviation Text
 * @text Texto de abreviatura de nivel
 * @parent Text Options
 * @desc Texto al cual abreviar el requisito de nivel (Requiere profesiones CGMZ).
 * @default Lv.
 *
 * @param Amount X Text
 * @parent Text Options
 * @desc Text to show to describe the amount x
 * @default x
 *
 * @param Craft Confirm Text
 * @text Crear texto de confirmación
 * @parent Text Options
 * @desc Texto para mostrar para describir el comando para la elaboración.
 * @default Craft
 *
 * @param Craft Cancel Text
 * @text Texto de cancelación de elaboración
 * @parent Text Options
 * @desc Texto a mostrar para describir el comando para cancelar una elaboración.
 * @default Cancel
 *
 * @param Progress Text
 * @text Texto de progreso
 * @parent Text Options
 * @desc Texto a mostrar para describir el progreso de la receta que se está elaborando actualmente.
 * @default Progress: 
 *
 * @param Success Text
 * @text Texto de éxito.
 * @parent Text Options
 * @desc Texto a mostrar para describir una artesanía exitosa.
 * @default Craft Success!
 *
 * @param Failure Text
 * @text Texto de falla
 * @parent Text Options
 * @desc Texto a mostrar para describir una elaboración fallida.
 * @default Craft Failed!
 *
 * @param High Quality Text
 * @parent Text Options
 * @desc Text to show to describe a high quality craft
 * @default High Quality Craft!
 *
 * @param Craft Quantity Text
 * @parent Text Options
 * @desc Text to show to describe the amount the player wants to craft
 * @default Amount to Craft:
 *
 * @param Label Text Color
 * @text Color del texto de la etiqueta
 * @parent Text Options
 * @desc El color de las etiquetas de texto en la escena de elaboración.
 * @type color
 * @default 16
 *
 * @param Progress Color1
 * @text Color de progreso1
 * @parent Text Options
 * @desc Primer color de la barra de progreso usando colores Windowskin.
 * @type color
 * @default 28
 *
 * @param Progress Color2
 * @text Color de progreso2
 * @parent Text Options
 * @desc Segundo color de la barra de progreso usando colores Windowskin.
 * @type color
 * @default 29
 *
 * @param Success Color
 * @text Color de éxito
 * @parent Text Options
 * @desc Color del texto Elaboración exitosa usando colores Windowskin.
 * @type color
 * @default 29
 *
 * @param High Quality Color
 * @parent Text Options
 * @desc Color of the High Quality Craft Text
 * @type color
 * @default 29
 *
 * @param Failure Color
 * @text Color de falla
 * @parent Text Options
 * @desc Color del texto Elaboración fallida usando colores Windowskin.
 * @type color
 * @default 18
 *
 * @param Header Color 1
 * @parent Text Options
 * @desc Color 1 of the gradient lines in headers
 * @type color
 * @default 1
 *
 * @param Header Color 2
 * @parent Text Options
 * @desc Color 2 of the gradient lines in headers
 * @type color
 * @default 0
 *
 * @param Info Header Text
 * @text Texto del encabezado de información
 * @parent Text Options
 * @desc Texto en el encabezado de información de la ventana de visualización.
 * @default Info
 *
 * @param Desc Header Text
 * @text Texto de encabezado de descripción
 * @parent Text Options
 * @desc Texto en el encabezado de descripción de la ventana de visualización.
 * @default Description
 *
 * @param Ingredient Header Text
 * @text Texto de Encabezado de ingredientes
 * @parent Text Options
 * @desc Texto en el encabezado de ingrediente de la ventana de visualización.
 * @default Ingredients
 *
 * @param Tool Header Text
 * @text Texto del encabezado de la herramienta
 * @parent Text Options
 * @desc Texto en el encabezado de herramienta de la ventana de visualización.
 * @default Tools
 *
 * @param Product Header Text
 * @text Texto del encabezado del producto
 * @parent Text Options
 * @desc Texto en el encabezado del producto de la ventana de visualización.
 * @default Products
 *
 * @param Fail Product Header Text
 * @text Texto de encabezado de producto fallido
 * @parent Text Options
 * @desc Texto en el encabezado del producto fallido de la ventana de visualización.
 * @default Fail Products
 *
 * @param High Quality Product Header Text
 * @parent Text Options
 * @desc Text in the High Quality Product Header of display window
 * @default High Quality Products
 *
 * @param Unique Text
 * @parent Text Options
 * @desc Text to show for unique products
 * @default Unique (%unique)
 *
 * @param No Recipe Text
 * @parent Text Options
 * @desc Text to show when the scene has no recipes
 * @default Keep playing the game to learn more recipes!
 *
 * @param Percent Text
 * @parent Text Options
 * @desc Text to show after progress percentage
 * @default  %
 *
 * @param Sec Text
 * @parent Text Options
 * @desc Text to show when there are seconds remaining on progress bar
 * @default sec
 *
 * @param Min Text
 * @parent Text Options
 * @desc Text to show when there are minutes remaining on progress bar
 * @default min
 *
 * @param Hr Text
 * @parent Text Options
 * @desc Text to show when there are hours remaining on progress bar
 * @default hr
 *
 * @param Remove Filter Text
 * @parent Text Options
 * @desc Text to show for the Remove Filter option
 * @default Remove Filter
 *
 * @param Category Filter Text
 * @parent Text Options
 * @desc Text to show for the Category Filter option
 * @default Category
 *
 * @param Subcategory Filter Text
 * @parent Text Options
 * @desc Text to show for the Subcategory Filter option
 * @default Subcategory
 *
 * @param Quality Filter Text
 * @parent Text Options
 * @desc Text to show for the Quality Filter option
 * @default Quality
 *
 * @param Collect Item Confirm Text
 * @parent Text Options
 * @desc Text for the confirm option in the collected items window
 * @default Collect
 *
 * @param Collect Item Item Text
 * @parent Text Options
 * @desc Text for an item that was collected in the crafting scene
 * @default %amountx %icon%name
 *
 * @param Collect Item Weapon Text
 * @parent Text Options
 * @desc Text for a weapon that was collected in the crafting scene
 * @default %amountx %icon%name
 *
 * @param Collect Item Armor Text
 * @parent Text Options
 * @desc Text for an armor that was collected in the crafting scene
 * @default %amountx %icon%name
 *
 * @param Collect Item Currency Text
 * @parent Text Options
 * @desc Text for an armor that was collected in the crafting scene
 * @default %icon%amount %color%unit
 *
 * @param Collected Items Text
 * @parent Text Options
 * @desc Text to display at the top of the collected items window
 * @default Collected Items:
 *
 * @param Integrations
 * @text Otras opciones del plugin CGMZ
 *
 * @param Learn Toast
 * @parent Integrations
 * @desc [CGMZ] Toast Manager preset id to show when learning a recipe
 *
 * @param Complete Toast
 * @parent Integrations
 * @desc [CGMZ] Toast Manager preset id to show when a craft is completed
 *
 * @param Always Award Exp
 * @text Experiencia de premio
 * @parent Integrations
 * @type boolean
 * @default true
 * @desc ¿Experiencia de premio incluso en caso de falla de receta? (requiere profesiones CGMZ).
 *
 * @param Hide High Level Recipes
 * @parent Integrations
 * @type boolean
 * @default true
 * @desc Hide recipes the player does not meet level requirements for?
 *
 * @param Close On Level Up
 * @text Cerrar al subir de nivel
 * @parent Integrations
 * @type boolean
 * @desc ¿Cerrar la escena de creación al subir de nivel de profesión?.
 * @default false
 *
 * @param Fail Rumble
 * @parent Integrations
 * @type struct<Rumble>
 * @default {"Duration":"0","Weak Magnitude":"1.00","Strong Magnitude":"1.00","Start Delay":"0"}
 * @desc [CGMZ] Rumble to play when successful craft finishes
 *
 * @param Success Rumble
 * @parent Integrations
 * @type struct<Rumble>
 * @default {"Duration":"0","Weak Magnitude":"1.00","Strong Magnitude":"1.00","Start Delay":"0"}
 * @desc [CGMZ] Rumble to play when successful craft finishes
 *
 * @param Scene Background
 * @parent Integrations
 * @desc [CGMZ] Scene Backgrounds preset id to use for the crafting scene
 *
 * @param Category Window Settings
 * @parent Integrations
 * @desc [CGMZ] Window Settings preset to use for the category window
 *
 * @param List Window Settings
 * @parent Integrations
 * @desc [CGMZ] Window Settings preset to use for the list window
 *
 * @param Display Window Settings
 * @parent Integrations
 * @desc [CGMZ] Window Settings preset to use for the display window
 *
 * @param Progress Window Settings
 * @parent Integrations
 * @desc [CGMZ] Window Settings preset to use for the progress window
 *
 * @param Confirm Window Settings
 * @parent Integrations
 * @desc [CGMZ] Window Settings preset to use for the confirm window
 *
 * @param Multicraft Window Settings
 * @parent Integrations
 * @desc [CGMZ] Window Settings preset to use for the multicraft window
 *
 * @param Filter Type Window Settings
 * @parent Integrations
 * @desc [CGMZ] Window Settings preset to use for the filter type window
 *
 * @param Filter Option Window Settings
 * @parent Integrations
 * @desc [CGMZ] Window Settings preset to use for the filter option window
 *
 * @param Collected Items Window Settings
 * @parent Integrations
 * @desc [CGMZ] Window Settings preset to use for the collected items window
 *
 * @param Collect Items Confirm Window Settings
 * @parent Integrations
 * @desc [CGMZ] Window Settings preset to use for the collect items window
 *
 * @param Category Window Background
 * @parent Integrations
 * @desc [CGMZ] Window Backgrounds preset to use for the category window
 *
 * @param List Window Background
 * @parent Integrations
 * @desc [CGMZ] Window Backgrounds preset to use for the list window
 *
 * @param Display Window Background
 * @parent Integrations
 * @desc [CGMZ] Window Backgrounds preset to use for the display window
 *
 * @param Progress Window Background
 * @parent Integrations
 * @desc [CGMZ] Window Backgrounds preset to use for the progress window
 *
 * @param Confirm Window Background
 * @parent Integrations
 * @desc [CGMZ] Window Backgrounds preset to use for the confirm window
 *
 * @param Multicraft Window Background
 * @parent Integrations
 * @desc [CGMZ] Window Backgrounds preset to use for the multicraft window
 *
 * @param Filter Type Window Background
 * @parent Integrations
 * @desc [CGMZ] Window Backgrounds preset to use for the filter type window
 *
 * @param Filter Option Window Background
 * @parent Integrations
 * @desc [CGMZ] Window Backgrounds preset to use for the filter option window
 *
 * @param Collected Items Window Background
 * @parent Integrations
 * @desc [CGMZ] Window Backgrounds preset to use for the collected items window
 *
 * @param Collect Items Confirm Window Background
 * @parent Integrations
 * @desc [CGMZ] Window Backgrounds preset to use for the collect items confirm window
*/
/*~struct~Recipe:es
 * @param Name
 * @text Nombre
 * @desc El nombre de la receta.
 *
 * @param Display Name
 * @desc The displayed name of the recipe (if different than name).
 *
 * @param Products
 * @text Productos
 * @type struct<Item>[]
 * @default []
 * @desc Los artículos producidos al elaborar la receta.
 *
 * @param Fail Products
 * @text Productos fallidos
 * @type struct<Item>[]
 * @default []
 * @desc Los artículos producidos por una elaboración fallida de esta receta.
 *
 * @param Ingredients
 * @text Ingredientes
 * @type struct<Item>[]
 * @default []
 * @desc Los elementos necesarios para elaborar la receta que se consumen en la artesanía.
 *
 * @param Tools
 * @text Herramientas
 * @type struct<Item>[]
 * @default []
 * @desc Los elementos necesarios para elaborar la receta que no se consumen en la elaboración.
 * 
 * @param Discovered
 * @text Descubierto
 * @type boolean
 * @default false
 * @desc Determine si la receta se descubre al comienzo del juego.
 *
 * @param Picture
 * @text Imagen
 * @type file
 * @dir img/
 * @desc La imagen que se usará para la receta en lugar del ícono grande (tamaño recomendado: 64x64). Dejar en blanco para no usar.
 * 
 * @param Icon
 * @text Ícono
 * @type icon
 * @default 0
 * @desc Índice de iconos a utilizar para la receta.
 *
 * @param Description
 * @text Descripción
 * @type multiline_string
 * @desc Descripción de la receta.
 *
 * @param Success Rate
 * @text Tasa de éxito
 * @type number
 * @min -99999
 * @max 100
 * @default 100
 * @desc El % de probabilidad de que la elaboración tenga éxito.
 *
 * @param Time
 * @text Tiempo
 * @type number
 * @min 1
 * @default 120
 * @desc El tiempo (en fotogramas, 60f = 1 s) que se tarda en elaborar la receta.
 *
 * @param Success Rate Per Level
 * @text Tasa de éxito por nivel
 * @type number
 * @min -99999
 * @max 100
 * @default 0
 * @desc El cambio (aditivo) en el % de probabilidad de que la elaboración tenga éxito por nivel más allá del requisito de nivel. (Requiere Profesiones CGMZ)
 *
 * @param Experience
 * @text Experiencia
 * @type number
 * @min 1
 * @default 1
 * @desc La cantidad de experiencia a otorgar por elaborar esta receta (Requiere Profesiones CGMZ).
 *
 * @param Profession
 * @text Profesión
 * @desc El nombre de la profesión a la que pertenece la receta si se utilizan Profesiones CGMZ. El tipo de receta si no usa Profesiones CGMZ.
 *
 * @param Subcategory
 * @desc If you want, you can add a subcategory to the recipe. This does not affect how the recipe functions.
 *
 * @param Quality
 * @desc If you want, you can add a quality to the recipe. This does not affect how the recipe functions.
 *
 * @param Level Requirement
 * @text Requerimiento de nivel
 * @type number
 * @default 1
 * @min 0
 * @desc Nivel de profesión requerido para fabricar (Requiere Profesiones CGMZ).
 *
 * @param Craft Sound Effect
 * @text Efecto de sonido de elaboración
 * @type struct<SoundEffect>
 * @default {"Name":"","Volume":"90","Pitch":"100","Pan":"0"}
 * @desc El efecto de sonido para reproducir al elaborar la receta.
 *
 * @param Fail Sound Effect
 * @text Efecto de sonido de falla
 * @type struct<SoundEffect>
 * @default {"Name":"","Volume":"90","Pitch":"100","Pan":"0"}
 * @desc El efecto de sonido que se reproduce cuando falla la elaboración.
 *
 * @param Success Sound Effect
 * @text Efecto de sonido de éxito
 * @type struct<SoundEffect>
 * @default {"Name":"","Volume":"90","Pitch":"100","Pan":"0"}
 * @desc El efecto de sonido para jugar cuando la elaboración tiene éxito.
 *
 * @param Learn Toast
 * @desc [CGMZ] Toast Manager preset id to use for learning this specific recipe only
 *
 * @param Complete Toast
 * @desc [CGMZ] Toast Manager preset id to use for crafting this specific recipe only
 *
 * @param Disable Learn Toast
 * @type boolean
 * @default false
 * @desc Do not show the learn toast for only this recipe?
 *
 * @param Disable Complete Toast
 * @type boolean
 * @default false
 * @desc Do not show the complete toast for only this recipe?
 *
 * @param Disable Autodiscover
 * @type boolean
 * @default false
 * @desc If true, this recipe must be manually discovered.
 *
 * @param Learn On Craft
 * @text Aprender sobre manualidade
 * @type struct<OnCraftRecipe>[]
 * @default []
 * @desc Nombre(s) de receta para saber cuándo se elabora esta receta.
 *
 * @param Unlearn On Craft
 * @type struct<OnCraftRecipe>[]
 * @default []
 * @desc Recipe name(s) to unlearn when this recipe is crafted
 *
 * @param Success Variable
 * @text Variable de éxito
 * @type variable
 * @default 0
 * @desc ID variable que, en una elaboración exitosa, aumentará en 1
 *
 * @param Product Unique Amount
 * @type number
 * @default 0
 * @desc Amount of the Product items in inventory before the recipe can no longer be crafted
 *
 * @param High Quality Products
 * @type struct<Item>[]
 * @default []
 * @desc The items produced by crafting the recipe when a high quality craft occurs
 *
 * @param High Quality Chance
 * @type number
 * @min 0
 * @max 100
 * @default 0
 * @desc The chance a craft will instead be high quality
 *
 * @param High Quality Chance Per Level
 * @type number
 * @min -100
 * @max 100
 * @default 0
 * @desc The change (additive) in % chance that the craft will be high quality per level beyond the level requirement. (Requires CGMZ Professions)
*/
/*~struct~OnCraftRecipe:es
 * @param Recipe Name
 * @desc The recipe name to learn/unlearn
 *
 * @param Chance
 * @type number
 * @default 100
 * @desc The chance to learn/unlearn the recipe.
*/
/*~struct~Item:es
 * @param Item
 * @text Artículo
 * @type item
 * @default 0
 * @desc El artículo a utilizar. Si configuras esto, no configures armadura/arma.
 *
 * @param Weapon
 * @text Arma
 * @type weapon
 * @default 0
 * @desc El arma a utilizar. Si configuras esto, no configures armadura/objeto.
 *
 * @param Armor
 * @text Armadura
 * @type armor
 * @default 0
 * @desc El artículo a utilizar. Si configuras esto, no configures un objeto/arma.
 *
 * @param Gold
 * @text Oro
 * @type boolean
 * @default false
 * @desc True para establecer una cantidad de oro.
 *
 * @param Generic
 * @text Genérico
 * @desc Un tipo genérico para usar. ¡Solo para uso en ingredientes/herramientas! Ignorado para Productos.
 * 
 * @param Amount
 * @text Cantidad
 * @type number
 * @default 1
 * @desc La cantidad necesaria de este artículo.
 *
 * @param Currency Id
 * @desc [CGMZ] Currency System currency id if Gold is set
*/
/*~struct~GenericItem:es
 * @param Type
 * @text Tipo
 * @desc El id/tipo del artículo genérico. Se usa internamente para referirse a este artículo genérico.
 *
 * @param Display Name
 * @text Nombre para mostrar
 * @desc El nombre mostrado del artículo genérico.
 *
 * @param Icon Index
 * @text Índice de iconos
 * @type icon
 * @default 0
 * @desc El icono a utilizar para representar el artículo genérico.
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
/*~struct~CategoryData:es
 * @param Id
 * @desc The id (profession) of the category
 *
 * @param Command Text
 * @desc The text to show for this category in the category window
*/
/*~struct~Rumble:es
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
Imported.CGMZ_Crafting = true;
CGMZ.Versions["Crafting"] = "1.12.1";
CGMZ.Crafting = {};
CGMZ.Crafting.parameters = PluginManager.parameters('CGMZ_Crafting');
CGMZ.Crafting.SceneBackground = CGMZ.Crafting.parameters["Scene Background"];
CGMZ.Crafting.SuccessRateText = CGMZ.Crafting.parameters["Success Rate Text"];
CGMZ.Crafting.HighQualityRateText = CGMZ.Crafting.parameters["High Quality Rate Text"];
CGMZ.Crafting.TimesCraftedText = CGMZ.Crafting.parameters["Times Crafted Text"];
CGMZ.Crafting.ExpText = CGMZ.Crafting.parameters["Experience Text"];
CGMZ.Crafting.LevelReqText = CGMZ.Crafting.parameters["Level Requirement Text"];
CGMZ.Crafting.CurrentLevelText = CGMZ.Crafting.parameters["Current Level Text"];
CGMZ.Crafting.LevelAbbrText = CGMZ.Crafting.parameters["Level Abbreviation Text"];
CGMZ.Crafting.ProgressText = CGMZ.Crafting.parameters["Progress Text"];
CGMZ.Crafting.SuccessText = CGMZ.Crafting.parameters["Success Text"];
CGMZ.Crafting.HighQualityText = CGMZ.Crafting.parameters["High Quality Text"];
CGMZ.Crafting.FailureText = CGMZ.Crafting.parameters["Failure Text"];
CGMZ.Crafting.InfoHeaderText = CGMZ.Crafting.parameters["Info Header Text"];
CGMZ.Crafting.DescHeaderText = CGMZ.Crafting.parameters["Desc Header Text"];
CGMZ.Crafting.IngredientHeaderText = CGMZ.Crafting.parameters["Ingredient Header Text"];
CGMZ.Crafting.ToolHeaderText = CGMZ.Crafting.parameters["Tool Header Text"];
CGMZ.Crafting.ProductHeaderText = CGMZ.Crafting.parameters["Product Header Text"];
CGMZ.Crafting.FailProductHeaderText = CGMZ.Crafting.parameters["Fail Product Header Text"];
CGMZ.Crafting.HighQualityProductHeaderText = CGMZ.Crafting.parameters["High Quality Product Header Text"];
CGMZ.Crafting.DescriptionAlignment = CGMZ.Crafting.parameters["Description Alignment"];
CGMZ.Crafting.CraftConfirmText = CGMZ.Crafting.parameters["Craft Confirm Text"];
CGMZ.Crafting.CraftCancelText = CGMZ.Crafting.parameters["Craft Cancel Text"];
CGMZ.Crafting.QualityLabelText = CGMZ.Crafting.parameters["Quality Label Text"];
CGMZ.Crafting.SubcategoryLabelText = CGMZ.Crafting.parameters["Subcategory Label Text"];
CGMZ.Crafting.ListWindowSettings = CGMZ.Crafting.parameters["List Window Settings"];
CGMZ.Crafting.DisplayWindowSettings = CGMZ.Crafting.parameters["Display Window Settings"];
CGMZ.Crafting.ProgressWindowSettings = CGMZ.Crafting.parameters["Progress Window Settings"];
CGMZ.Crafting.ConfirmWindowSettings = CGMZ.Crafting.parameters["Confirm Window Settings"];
CGMZ.Crafting.MulticraftWindowSettings = CGMZ.Crafting.parameters["Multicraft Window Settings"];
CGMZ.Crafting.CategoryWindowSettings = CGMZ.Crafting.parameters["Category Window Settings"];
CGMZ.Crafting.FilterOptionWindowSettings = CGMZ.Crafting.parameters["Filter Option Window Settings"];
CGMZ.Crafting.FilterTypeWindowSettings = CGMZ.Crafting.parameters["Filter Type Window Settings"];
CGMZ.Crafting.CollectedItemsWindowSettings = CGMZ.Crafting.parameters["Collected Items Window Settings"];
CGMZ.Crafting.CollectItemsConfirmWindowSettings = CGMZ.Crafting.parameters["Collect items Confirm Window Settings"];
CGMZ.Crafting.ListWindowBackground = CGMZ.Crafting.parameters["List Window Background"];
CGMZ.Crafting.DisplayWindowBackground = CGMZ.Crafting.parameters["Display Window Background"];
CGMZ.Crafting.ProgressWindowBackground = CGMZ.Crafting.parameters["Progress Window Background"];
CGMZ.Crafting.ConfirmWindowBackground = CGMZ.Crafting.parameters["Confirm Window Background"];
CGMZ.Crafting.MulticraftWindowBackground = CGMZ.Crafting.parameters["Multicraft Window Background"];
CGMZ.Crafting.CategoryWindowBackground = CGMZ.Crafting.parameters["Category Window Background"];
CGMZ.Crafting.FilterOptionWindowBackground = CGMZ.Crafting.parameters["Filter Option Window Background"];
CGMZ.Crafting.FilterTypeWindowBackground = CGMZ.Crafting.parameters["Filter Type Window Background"];
CGMZ.Crafting.CollectedItemsWindowBackground = CGMZ.Crafting.parameters["Collected Items Window Background"];
CGMZ.Crafting.CollectItemsConfirmWindowBackground = CGMZ.Crafting.parameters["Collect items Confirm Window Background"];
CGMZ.Crafting.UniqueText = CGMZ.Crafting.parameters["Unique Text"];
CGMZ.Crafting.NoRecipeText = CGMZ.Crafting.parameters["No Recipe Text"];
CGMZ.Crafting.AmountXText = CGMZ.Crafting.parameters["Amount X Text"];
CGMZ.Crafting.CraftQuantityText = CGMZ.Crafting.parameters["Craft Quantity Text"];
CGMZ.Crafting.SecText = CGMZ.Crafting.parameters["Sec Text"];
CGMZ.Crafting.MinText = CGMZ.Crafting.parameters["Min Text"];
CGMZ.Crafting.HrText = CGMZ.Crafting.parameters["Hr Text"];
CGMZ.Crafting.PercentText = CGMZ.Crafting.parameters["Percent Text"];
CGMZ.Crafting.RemoveFilterText = CGMZ.Crafting.parameters["Remove Filter Text"];
CGMZ.Crafting.CategoryFilterText = CGMZ.Crafting.parameters["Category Filter Text"];
CGMZ.Crafting.SubcategoryFilterText = CGMZ.Crafting.parameters["Subcategory Filter Text"];
CGMZ.Crafting.QualityFilterText = CGMZ.Crafting.parameters["Quality Filter Text"];
CGMZ.Crafting.FilterKey = CGMZ.Crafting.parameters["Filter Key"];
CGMZ.Crafting.LearnToast = CGMZ.Crafting.parameters["Learn Toast"];
CGMZ.Crafting.CompleteToast = CGMZ.Crafting.parameters["Complete Toast"];
CGMZ.Crafting.CollectItemConfirmText = CGMZ.Crafting.parameters["Collect Item Confirm Text"];
CGMZ.Crafting.CollectItemItemText = CGMZ.Crafting.parameters["Collect Item Item Text"];
CGMZ.Crafting.CollectItemWeaponText = CGMZ.Crafting.parameters["Collect Item Weapon Text"];
CGMZ.Crafting.CollectItemArmorText = CGMZ.Crafting.parameters["Collect Item Armor Text"];
CGMZ.Crafting.CollectItemCurrencyText = CGMZ.Crafting.parameters["Collect Item Currency Text"];
CGMZ.Crafting.CollectedItemsText = CGMZ.Crafting.parameters["Collected Items Text"];
CGMZ.Crafting.ScrollDeceleration = parseFloat(CGMZ.Crafting.parameters["Scroll Deceleration"]);
CGMZ.Crafting.LabelColor = Number(CGMZ.Crafting.parameters["Label Text Color"]);
CGMZ.Crafting.ScrollSpeed = Number(CGMZ.Crafting.parameters["ScrollSpeed"]);
CGMZ.Crafting.ScrollWait = Number(CGMZ.Crafting.parameters["ScrollWait"]);
CGMZ.Crafting.ProgressColor1 = Number(CGMZ.Crafting.parameters["Progress Color1"]);
CGMZ.Crafting.ProgressColor2 = Number(CGMZ.Crafting.parameters["Progress Color2"]);
CGMZ.Crafting.FailureColor = Number(CGMZ.Crafting.parameters["Failure Color"]);
CGMZ.Crafting.SuccessColor = Number(CGMZ.Crafting.parameters["Success Color"]);
CGMZ.Crafting.HighQualityColor = Number(CGMZ.Crafting.parameters["High Quality Color"]);
CGMZ.Crafting.ListWindowWidth = Number(CGMZ.Crafting.parameters["List Window Width"]);
CGMZ.Crafting.MulticraftWindowWidth = Number(CGMZ.Crafting.parameters["Multicraft Window Width"]);
CGMZ.Crafting.HeaderColor1 = Number(CGMZ.Crafting.parameters["Header Color 1"]);
CGMZ.Crafting.HeaderColor2 = Number(CGMZ.Crafting.parameters["Header Color 2"]);
CGMZ.Crafting.ProgressDecimals = Number(CGMZ.Crafting.parameters["Progress Decimals"]);
CGMZ.Crafting.FilterButtonOffset = Number(CGMZ.Crafting.parameters["Filter Button Offset"]);
CGMZ.Crafting.FilterButtonWidth = Number(CGMZ.Crafting.parameters["Filter Button Width"]);
CGMZ.Crafting.FilterGamepad = Number(CGMZ.Crafting.parameters["Filter Gamepad"]);
CGMZ.Crafting.CustomSpace = Number(CGMZ.Crafting.parameters["Custom Space"]);
CGMZ.Crafting.AutoScroll = (CGMZ.Crafting.parameters["Auto Scroll"] === "true");
CGMZ.Crafting.AlwaysAwardExp = (CGMZ.Crafting.parameters["Always Award Exp"] === "true");
CGMZ.Crafting.HideHighLevelRecipes = (CGMZ.Crafting.parameters["Hide High Level Recipes"] === "true");
CGMZ.Crafting.ShowProgressPercentage = (CGMZ.Crafting.parameters["Show Progress Percentage"] === "true");
CGMZ.Crafting.ShowIngredientAmount = (CGMZ.Crafting.parameters["Show Ingredient Amount"] === "true");
CGMZ.Crafting.ShowToolAmount = (CGMZ.Crafting.parameters["Show Tool Amount"] === "true");
CGMZ.Crafting.ShowProductAmount = (CGMZ.Crafting.parameters["Show Product Amount"] === "true");
CGMZ.Crafting.ShowFailProductAmount = (CGMZ.Crafting.parameters["Show Fail Product Amount"] === "true");
CGMZ.Crafting.ShowHighQualityProductAmount = (CGMZ.Crafting.parameters["Show High Quality Product Amount"] === "true");
CGMZ.Crafting.WindowTransparency = (CGMZ.Crafting.parameters["Transparent Windows"] === "true");
CGMZ.Crafting.PopSceneOnLevel = (CGMZ.Crafting.parameters["Close On Level Up"] === "true");
CGMZ.Crafting.ListWindowRight = (CGMZ.Crafting.parameters["List Window On Right"] === "true");
CGMZ.Crafting.ShowConfirmationWindow = (CGMZ.Crafting.parameters["Show Confirm Window"] === "true");
CGMZ.Crafting.DisableTouchUISpace = (CGMZ.Crafting.parameters["Disable Touch UI Space"] === "true");
CGMZ.Crafting.InstantCrafting = (CGMZ.Crafting.parameters["Instant Crafting"] === "true");
CGMZ.Crafting.ShowUniqueProductAmount = (CGMZ.Crafting.parameters["Show Unique Product Amount"] === "true");
CGMZ.Crafting.HideUncraftableRecipes = (CGMZ.Crafting.parameters["Hide Uncraftable Recipes"] === "true");
CGMZ.Crafting.ShowCategoryWindow = (CGMZ.Crafting.parameters["Show Category Window"] === "true");
CGMZ.Crafting.AutomaticLearn = (CGMZ.Crafting.parameters["Automatic Learn"] === "true");
CGMZ.Crafting.AllowMulticraft = (CGMZ.Crafting.parameters["Allow Multicraft"] === "true");
CGMZ.Crafting.ShowProgressTimeRemaining = (CGMZ.Crafting.parameters["Show Progress Time Remaining"] === "true");
CGMZ.Crafting.AllowFilter = (CGMZ.Crafting.parameters["Allow Filter"] === "true");
CGMZ.Crafting.WaitForFinish = (CGMZ.Crafting.parameters["Wait For Finish"] === "true");
CGMZ.Crafting.CollectItemsAtStation = (CGMZ.Crafting.parameters["Collect Items At Station"] === "true");
CGMZ.Crafting.SuccessRumble = CGMZ_Utils.parseRumbleJSON(CGMZ.Crafting.parameters["Success Rumble"], "[CGMZ] Crafting");
CGMZ.Crafting.FailRumble = CGMZ_Utils.parseRumbleJSON(CGMZ.Crafting.parameters["Fail Rumble"], "[CGMZ] Crafting");
CGMZ.Crafting.Recipes = CGMZ_Utils.parseJSON(CGMZ.Crafting.parameters["Recipes"], [], "[CGMZ] Crafting", "Your Recipes parameter had invalid JSON and could not be read.");
CGMZ.Crafting.Generics = CGMZ_Utils.parseJSON(CGMZ.Crafting.parameters["Generic Items"], [], "[CGMZ] Crafting", "Your Generic Items parameter had invalid JSON and could not be read.");
CGMZ.Crafting.DisplayWindowInfo = CGMZ_Utils.parseJSON(CGMZ.Crafting.parameters["Display Window Info"], [], "[CGMZ] Crafting", "Your Display Window Info parameter had invalid JSON and could not be read.");
CGMZ.Crafting.Categories = CGMZ_Utils.parseJSON(CGMZ.Crafting.parameters["Categories"], [], "[CGMZ] Crafting", "Your Categories parameter had invalid JSON and could not be read.");
CGMZ.Crafting.CategorySortOrder = CGMZ_Utils.parseJSON(CGMZ.Crafting.parameters["Category Sort Order"], [], "[CGMZ] Crafting", "Your Category Sort Order parameter had invalid JSON and could not be read.").reverse();
CGMZ.Crafting.CategoryFilterOptions = CGMZ_Utils.parseJSON(CGMZ.Crafting.parameters["Category Filter Options"], [], "[CGMZ] Crafting", "Your Category Filter Options parameter had invalid JSON and could not be read.");
CGMZ.Crafting.SubcategoryFilterOptions = CGMZ_Utils.parseJSON(CGMZ.Crafting.parameters["Subcategory Filter Options"], [], "[CGMZ] Crafting", "Your Subcategory Filter Options parameter had invalid JSON and could not be read.");
CGMZ.Crafting.QualityFilterOptions = CGMZ_Utils.parseJSON(CGMZ.Crafting.parameters["Quality Filter Options"], [], "[CGMZ] Crafting", "Your Quality Filter Options parameter had invalid JSON and could not be read.");
//=============================================================================
// CGMZ_Recipe
//-----------------------------------------------------------------------------
// Store and manage recipe data
//=============================================================================
function CGMZ_Recipe() {
	this.initialize.apply(this, arguments);
}
//-----------------------------------------------------------------------------
// Initialize Recipe
//-----------------------------------------------------------------------------
CGMZ_Recipe.prototype.initialize = function(recipe) {
	this._name = recipe.Name;
	this._discovered = (recipe.Discovered === 'true');
	this._profession = recipe.Profession;
	this._description = recipe.Description;
	this._timesCrafted = 0;
};
//-----------------------------------------------------------------------------
// Change discovered status of a recipe
//-----------------------------------------------------------------------------
CGMZ_Recipe.prototype.changeDiscoveredStatus = function(discovered) {
	this._discovered = discovered;
};
//-----------------------------------------------------------------------------
// Set new description
//-----------------------------------------------------------------------------
CGMZ_Recipe.prototype.setDescription = function(description) {
	this._description = description;
};
//-----------------------------------------------------------------------------
// Set up learn toast (Requires CGMZ ToastManager)
//-----------------------------------------------------------------------------
CGMZ_Recipe.prototype.setupLearnToast = function() {
	if(!Imported.CGMZ_ToastManager) return;
	const recipeTemp = $cgmzTemp.getRecipeTempData(this._name);
	const toastPreset = recipeTemp.learnToast || CGMZ.Crafting.LearnToast;
	const toast = $cgmzTemp.getToastObjectFromPreset(toastPreset);
	if(!recipeTemp || recipeTemp.disableLearnToast || !toast) return;
	toast.lineOne = toast.lineOne.replace('%name', recipeTemp.getName());
	toast.lineTwo = toast.lineTwo.replace('%name', recipeTemp.getName());
	$cgmzTemp.createNewToast(toast);
};
//-----------------------------------------------------------------------------
// Set up complete toast (Requires CGMZ ToastManager)
//-----------------------------------------------------------------------------
CGMZ_Recipe.prototype.setupCompleteToast = function() {
	if(!Imported.CGMZ_ToastManager) return;
	const recipeTemp = $cgmzTemp.getRecipeTempData(this._name);
	const toastPreset = recipeTemp.completeToast || CGMZ.Crafting.CompleteToast;
	const toast = $cgmzTemp.getToastObjectFromPreset(toastPreset);
	if(!recipeTemp || recipeTemp.disableCompleteToast || !toast) return;
	toast.lineOne = toast.lineOne.replace('%name', recipeTemp.getName());
	toast.lineTwo = toast.lineTwo.replace('%name', recipeTemp.getName());
	$cgmzTemp.createNewToast(toast);
};
//-----------------------------------------------------------------------------
// Perform the craft
//-----------------------------------------------------------------------------
CGMZ_Recipe.prototype.doCraft = function(success, hq = false) {
	const tempData = $cgmzTemp.getRecipeTempData(this._name);
	this.takeItems(tempData.ingredients);
	if(Imported.CGMZ_Professions) this.awardExp(success);
	if(success) {
		this._timesCrafted++;
		this.learnRecipesOnCraft();
		this.unlearnRecipesOnCraft();
		const products = (hq) ? tempData.hqproducts : tempData.products;
		this.giveItems(products);
		const recipeTemp = $cgmzTemp.getRecipeTempData(this._name);
		if(recipeTemp && recipeTemp.successVariable) {
			const val = $gameVariables.value(recipeTemp.successVariable);
			$gameVariables.setValue(recipeTemp.successVariable, val + 1);
		}
	}
	else {
		this.giveItems(tempData.failProducts);
	}
};
//-----------------------------------------------------------------------------
// Handling when craft starts
//-----------------------------------------------------------------------------
CGMZ_Recipe.prototype.startCraft = function() {
	const tempData = $cgmzTemp.getRecipeTempData(this._name);
	this.takeItems(tempData.ingredients);
};
//-----------------------------------------------------------------------------
// Handling when craft finishes
//-----------------------------------------------------------------------------
CGMZ_Recipe.prototype.finishCraft = function(success, hq = false, sceneKey = "") {
	const tempData = $cgmzTemp.getRecipeTempData(this._name);
	if(Imported.CGMZ_Professions) this.awardExp(success);
	let isInStation = false;
	if(SceneManager.CGMZ_isCurrentScene('CGMZ_Scene_Crafting')) isInStation = (!sceneKey || SceneManager._scene.getSceneKey() === sceneKey);
	if(!CGMZ.Crafting.CollectItemsAtStation) isInStation = true;
	if(success) {
		this._timesCrafted++;
		this.learnRecipesOnCraft();
		this.unlearnRecipesOnCraft();
		const products = (hq) ? tempData.hqproducts : tempData.products;
		(isInStation) ? this.giveItems(products) : $cgmz.addCraftItemsToScene(products, sceneKey);
		const recipeTemp = $cgmzTemp.getRecipeTempData(this._name);
		if(recipeTemp?.successVariable) {
			const val = $gameVariables.value(recipeTemp.successVariable);
			$gameVariables.setValue(recipeTemp.successVariable, val + 1);
		}
	} else {
		(isInStation) ? this.giveItems(tempData.failProducts) : $cgmz.addCraftItemsToScene(tempData.failProducts, sceneKey);
	}
};
//-----------------------------------------------------------------------------
// Check if recipe can be crafted
//-----------------------------------------------------------------------------
CGMZ_Recipe.prototype.canCraft = function() {
	if(!this.meetsLevelRequirements()) return false;
	if(!this.hasItemsNeeded()) return false;
	if(!this.meetsUniqueItemCheck()) return false;
	return this._discovered;
};
//-----------------------------------------------------------------------------
// Check if profession level requirements are met
//-----------------------------------------------------------------------------
CGMZ_Recipe.prototype.meetsLevelRequirements = function() {
	if(!Imported.CGMZ_Professions) return true;
	const profession = $cgmz.getProfession(this._profession);
	const tempData = $cgmzTemp.getRecipeTempData(this._name);
	if(profession) {
		return tempData.levelRequirement <= profession.getBuffedLevel();
	}
	return true;
};
//-----------------------------------------------------------------------------
// Check if player has the items required to craft
//-----------------------------------------------------------------------------
CGMZ_Recipe.prototype.hasItemsNeeded = function() {
	const tempData = $cgmzTemp.getRecipeTempData(this._name);
	for(const tool of tempData.tools) {
		if(tool.Type === 'currency') {
			if(Imported.CGMZ_CurrencySystem && tool.ID) {
				const currency = $cgmz.getCurrency(tool.ID);
				if(currency && currency.amount() < tool.Amount) return false;
			} else {
				if($gameParty.gold() < tool.Amount) return false;
			}
		} else if(tool.Type === 'generic') {
			const numItems = $cgmzTemp.calculateTotalNumItemsOfGenericType(tool.GenericCategory);
			if(numItems < tool.Amount) return false;
		} else {
			const item = CGMZ_Utils.lookupItem(tool.Type, tool.ID);
			if(item) {
				if($gameParty.numItems(item) < tool.Amount) return false;
			}
		}
	}
	for(const ingredient of tempData.ingredients) {
		if(ingredient.Type === 'currency') {
			if(Imported.CGMZ_CurrencySystem && ingredient.ID) {
				const currency = $cgmz.getCurrency(ingredient.ID);
				if(currency && currency.amount() < ingredient.Amount) return false;
			} else {
				if($gameParty.gold() < ingredient.Amount) return false;
			}
		} else if(ingredient.Type === 'generic') {
			const numItems = $cgmzTemp.calculateTotalNumItemsOfGenericType(ingredient.GenericCategory);
			if(numItems < ingredient.Amount) return false;
		} else {
			const item = CGMZ_Utils.lookupItem(ingredient.Type, ingredient.ID);
			if(item) {
				if($gameParty.numItems(item) < ingredient.Amount) return false;
			}
		}
	}
	return true;
};
//-----------------------------------------------------------------------------
// Check if player already has the unique items
//-----------------------------------------------------------------------------
CGMZ_Recipe.prototype.meetsUniqueItemCheck = function() {
	const recipeTemp = $cgmzTemp.getRecipeTempData(this._name);
	if(!recipeTemp || !recipeTemp.productUniqueAmount) return true;
	for(const product of recipeTemp.products) {
		if(product.Type === 'currency' || product.Type === 'generic') continue; // currency and generic cannot be unique
		const item = CGMZ_Utils.lookupItem(product.Type, product.ID);
		if(item) {
			if($gameParty.numItems(item) >= recipeTemp.productUniqueAmount) return false;
		}
	}
	return true;
};
//-----------------------------------------------------------------------------
// Take away ingredients needed to craft
//-----------------------------------------------------------------------------
CGMZ_Recipe.prototype.takeItems = function(itemArray) {
	for(const itemObj of itemArray) {
		if(itemObj.Type === 'currency') {
			if(Imported.CGMZ_CurrencySystem && itemObj.ID) {
				const currency = $cgmz.getCurrency(itemObj.ID);
				if(currency) {
					currency.loseCurrency(itemObj.Amount);
				}
			} else {
				$gameParty.loseGold(itemObj.Amount);
			}
		} else if(itemObj.Type === 'generic') {
			let amtToLose = itemObj.Amount;
			for(const item of $gameParty.allItems()) {
				if(amtToLose <= 0) break;
				const genericType = itemObj.GenericCategory;
				if(item.meta && item.meta.cgmzcraftinggeneric && item.meta.cgmzcraftinggeneric.split(",").includes(genericType)) {
					const itemAmt = $gameParty.numItems(item);
					(amtToLose > itemAmt) ? $gameParty.loseItem(item, itemAmt, true) : $gameParty.loseItem(item, amtToLose, true);
					amtToLose -= itemAmt;
				}
			}
		} else {
			const item = CGMZ_Utils.lookupItem(itemObj.Type, itemObj.ID);
			if(item) {
				$gameParty.loseItem(item, itemObj.Amount, false);
			}
		}
	}
};
//-----------------------------------------------------------------------------
// Give item products generated from craft
//-----------------------------------------------------------------------------
CGMZ_Recipe.prototype.giveItems = function(itemArray) {
	for(const itemObj of itemArray) {
		if(itemObj.Type === 'currency') {
			if(Imported.CGMZ_CurrencySystem && itemObj.ID) {
				const currency = $cgmz.getCurrency(itemObj.ID);
				if(currency) currency.gainCurrency(itemObj.Amount);
			} else {
				$gameParty.gainGold(itemObj.Amount);
			}
		} else {
			const item = CGMZ_Utils.lookupItem(itemObj.Type, itemObj.ID);
			if(item) {
				$gameParty.gainItem(item, itemObj.Amount, false);
			}
		}
	}
};
//-----------------------------------------------------------------------------
// Learn a recipe and show toast if applicable
//-----------------------------------------------------------------------------
CGMZ_Recipe.prototype.learn = function() {
	if(!this._discovered) {
		this.changeDiscoveredStatus(true);
		this.setupLearnToast();
	}
};
//-----------------------------------------------------------------------------
// Unlearn a recipe
//-----------------------------------------------------------------------------
CGMZ_Recipe.prototype.unlearn = function() {
	this.changeDiscoveredStatus(false);
};
//-----------------------------------------------------------------------------
// Award profession Exp if applicable
//-----------------------------------------------------------------------------
CGMZ_Recipe.prototype.awardExp = function(success) {
	if(success || CGMZ.Crafting.AlwaysAwardExp) {
		const tempData = $cgmzTemp.getRecipeTempData(this._name);
		if(tempData) $cgmz.changeProfessionExp(this._profession, "+", tempData.experience);
	}
};
//-----------------------------------------------------------------------------
// Try to learn applicable recipes on craft
//-----------------------------------------------------------------------------
CGMZ_Recipe.prototype.learnRecipesOnCraft = function() {
	const recipeTemp = $cgmzTemp.getRecipeTempData(this._name);
	if(!recipeTemp) return;
	for(const obj of recipeTemp.learnOnCraft) {
		const recipe = $cgmz.getRecipe(obj.recipe);
		const roll = Math.random()*100;
		if(recipe && !recipe._discovered && roll <= obj.odds) {
			recipe.learn();
		}
	}
};
//-----------------------------------------------------------------------------
// Try to unlearn applicable recipes on craft
//-----------------------------------------------------------------------------
CGMZ_Recipe.prototype.unlearnRecipesOnCraft = function() {
	const recipeTemp = $cgmzTemp.getRecipeTempData(this._name);
	if(!recipeTemp) return;
	for(const obj of recipeTemp.unlearnOnCraft) {
		const recipe = $cgmz.getRecipe(obj.recipe);
		const roll = Math.random()*100;
		if(recipe && recipe._discovered && roll <= obj.odds) {
			recipe.unlearn();
		}
	}
};
//-----------------------------------------------------------------------------
// Calculate the success rate of the recipe
//-----------------------------------------------------------------------------
CGMZ_Recipe.prototype.calculateSuccessRate = function() {
	const tempData = $cgmzTemp.getRecipeTempData(this._name);
	let successRate = tempData.successRate;
	if(Imported.CGMZ_Professions) {
		const profession = $cgmz.getProfession(this._profession);
		if(profession) {
			successRate += tempData.addedSuccessPerLevel * (profession.getBuffedLevel() - tempData.levelRequirement);
		}
		for(const actor of $gameParty.members()) {
			for(const equip of actor.equips()) {
				if(equip && equip.meta && equip.meta.cgmzrecipetype === this._profession) {
					successRate += Number(equip.meta.cgmzrecipebonus) || 0;
				}
			}
		}
	}
	return Math.max(0.0, Math.min(successRate, 100.0));
};
//-----------------------------------------------------------------------------
// Calculate the high quality rate of the recipe
//-----------------------------------------------------------------------------
CGMZ_Recipe.prototype.calculateHighQualityRate = function() {
	const tempData = $cgmzTemp.getRecipeTempData(this._name);
	let successRate = tempData.hqChance;
	if(Imported.CGMZ_Professions) {
		const profession = $cgmz.getProfession(this._profession);
		if(profession) {
			successRate += tempData.hqChancePerLevel * (profession.getBuffedLevel() - tempData.levelRequirement);
		}
	}
	return Math.max(0.0, Math.min(successRate, 100.0));
};
//-----------------------------------------------------------------------------
// Check if this recipe can be autodiscovered
//-----------------------------------------------------------------------------
CGMZ_Recipe.prototype.canAutodiscover = function() {
	const recipeTemp = $cgmzTemp.getRecipeTempData(this._name);
	if(this._discovered) return false;
	if(recipeTemp.disableAutodiscover) return false;
	if(!this.hasItemsNeeded()) return false;
	return true;
};
//=============================================================================
// CGMZ_RecipeTemp
//-----------------------------------------------------------------------------
// Store and manage unsaved recipe data
//=============================================================================
function CGMZ_RecipeTemp() {
	this.initialize.apply(this, arguments);
}
//-----------------------------------------------------------------------------
// Initialize Recipe
//-----------------------------------------------------------------------------
CGMZ_RecipeTemp.prototype.initialize = function(recipe) {
	this._name = recipe.Name;
	this._displayName = recipe["Display Name"];
	this.subcategory = recipe.Subcategory;
	this.quality = recipe.Quality;
	this.image = recipe.Picture;
	this.learnToast = recipe["Learn Toast"];
	this.completeToast = recipe["Complete Toast"];
	this.disableLearnToast = (recipe["Disable Learn Toast"] === 'true');
	this.disableCompleteToast = (recipe["Disable Complete Toast"] === 'true');
	this.disableAutodiscover = (recipe["Disable Autodiscover"] === 'true');
	this.time = Number(recipe.Time);
	this.successRate = Number(recipe["Success Rate"]);
	this.experience = Number(recipe.Experience);
	this.productUniqueAmount = Number(recipe["Product Unique Amount"]);
	this.successVariable = Number(recipe["Success Variable"]);
	this.iconIndex = Number(recipe.Icon);
	this.addedSuccessPerLevel = Number(recipe["Success Rate Per Level"]);
	this.hqChance = Number(recipe["High Quality Chance"]);
	this.hqChancePerLevel = Number(recipe["High Quality Chance Per Level"]);
	this.levelRequirement = Number(recipe["Level Requirement"]);
	this.craftSE = CGMZ_Utils.parseSoundEffectJSON(recipe["Craft Sound Effect"], "[CGMZ] Crafting");
	this.failSE = CGMZ_Utils.parseSoundEffectJSON(recipe["Fail Sound Effect"], "[CGMZ] Crafting");
	this.successSE = CGMZ_Utils.parseSoundEffectJSON(recipe["Success Sound Effect"], "[CGMZ] Crafting");
	this.learnOnCraft = [];
	this.unlearnOnCraft = [];
	this.setupRecipeOnCraft(recipe["Learn On Craft"], 'Learn On Craft');
	this.setupRecipeOnCraft(recipe["Unlearn On Craft"], 'Unlearn On Craft');
	this.products = [];
	this.hqproducts = [];
	this.failProducts = [];
	this.ingredients = [];
	this.tools = [];
	this.setupArray(this.products, recipe.Products, false);
	this.setupArray(this.ingredients, recipe.Ingredients, true);
	this.setupArray(this.tools, recipe.Tools, true);
	this.setupArray(this.failProducts, recipe["Fail Products"], false);
	this.setupArray(this.hqproducts, recipe["High Quality Products"], false);
};
//-----------------------------------------------------------------------------
// Set up the array for learn / unlearn on craft chance parameters
//-----------------------------------------------------------------------------
CGMZ_RecipeTemp.prototype.setupRecipeOnCraft = function(recipesJSON, type) {
	const recipesA = CGMZ_Utils.parseJSON(recipesJSON, [], "[CGMZ] Crafting", `Your recipe '${this._name}' had invalid JSON in a ${type} parameter.`);
	for(const recipeJ of recipesA) {
		const parsed = CGMZ_Utils.parseJSON(recipeJ, null, "[CGMZ] Crafting", `Your recipe '${this._name}' had invalid JSON in a ${type} parameter.`);
		if(!parsed) continue;
		const obj = {recipe: parsed["Recipe Name"], odds: Number(parsed.Chance)};
		if(type === 'Learn On Craft') {
			this.learnOnCraft.push(obj);
		} else {
			this.unlearnOnCraft.push(obj);
		}
	}
};
//-----------------------------------------------------------------------------
// Set up the arrays for items associated with the recipe.
//-----------------------------------------------------------------------------
CGMZ_RecipeTemp.prototype.setupArray = function(array, recipeJSONArray, allowGenerics) {
	const recipeArray = CGMZ_Utils.parseJSON(recipeJSONArray, [], "[CGMZ] Crafting", "Your recipe '" + this._name + "' had invalid JSON in parameter: product, ingredient, tool, high quality product, or fail product. Skipping");
	for(const recipeJSON of recipeArray) {
		const item = CGMZ_Utils.parseJSON(recipeJSON, null, "[CGMZ] Crafting", "Could not parse a product, ingredient, tool, high quality product, or fail product parameter in recipe '" + this._name + "'");
		if(!item) continue;
		if(Number(item.Item) !== 0) {
			item.ID = Number(item.Item);
			item.Type = "item";
		} else if(Number(item.Weapon) !== 0) {
			item.ID = Number(item.Weapon);
			item.Type = "weapon";
		} else if(Number(item.Armor) !== 0) {
			item.ID = Number(item.Armor);
			item.Type = "armor";
		} else if(allowGenerics && item.Generic) {
			item.ID = 0;
			item.Type = "generic";
			item.GenericCategory = item.Generic;
		} else {
			item.ID = item["Currency Id"];
			item.Type = "currency";
		}
		item.Amount = Number(item.Amount);
		array.push(item);
	}
};
//-----------------------------------------------------------------------------
// Set up the array for possible crafting recipes learned on successful craft
//-----------------------------------------------------------------------------
CGMZ_RecipeTemp.prototype.getName = function() {
	return (this._displayName) ? this._displayName : this._name;
};
//=============================================================================
// CGMZ_CraftingGenericItem
//-----------------------------------------------------------------------------
// Object representing a generic item, not saved
//=============================================================================
function CGMZ_CraftingGenericItem() {
	this.initialize.apply(this, arguments);
}
//-----------------------------------------------------------------------------
// Initialize generic item data
//-----------------------------------------------------------------------------
CGMZ_CraftingGenericItem.prototype.initialize = function(generic) {
	this._type = generic.Type;
	this._name = generic["Display Name"];
	this._iconIndex = Number(generic["Icon Index"]);
};
//=============================================================================
// CGMZ_CraftingCategoryData
//-----------------------------------------------------------------------------
// Object representing a crafting category, not saved
//=============================================================================
function CGMZ_CraftingCategoryData() {
	this.initialize.apply(this, arguments);
}
//-----------------------------------------------------------------------------
// Initialize category data
//-----------------------------------------------------------------------------
CGMZ_CraftingCategoryData.prototype.initialize = function(category) {
	this.id = category.Id;
	this.commandText = category["Command Text"];
};
//=============================================================================
// CGMZ
//-----------------------------------------------------------------------------
// Manage recipe data
//=============================================================================
//-----------------------------------------------------------------------------
// Also initialize recipe data
//-----------------------------------------------------------------------------
const alias_CGMZ_Crafting_createPluginData = CGMZ_Core.prototype.createPluginData;
CGMZ_Core.prototype.createPluginData = function() {
	alias_CGMZ_Crafting_createPluginData.call(this);
	this.initializeRecipeData();
};
//-----------------------------------------------------------------------------
// Initialize recipe data
//-----------------------------------------------------------------------------
CGMZ_Core.prototype.initializeRecipeData = function(reinitialize = false) {
	if(!this._recipes || reinitialize) {
		this._recipes = [];
	}
	if(!this._recipeSceneData || reinitialize) {
		this._recipeSceneData = {};
	}
	for(const recipeJSON of CGMZ.Crafting.Recipes) {
		const recipeParsed = CGMZ_Utils.parseJSON(recipeJSON, null, "[CGMZ] Crafting", "One of your recipes had invalid JSON and could not be read, skipping.");
		if(!recipeParsed) continue;
		const recipe = new CGMZ_Recipe(recipeParsed);
		const existingRecipe = this.getRecipe(recipe._name);
		if(!existingRecipe) {
			this._recipes.push(recipe);
		}
	}
};
//-----------------------------------------------------------------------------
// Alias. Check for new recipes after loading saved game
//-----------------------------------------------------------------------------
const alias_CGMZ_Crafting_onAfterLoad = CGMZ_Core.prototype.onAfterLoad;
CGMZ_Core.prototype.onAfterLoad = function() {
	alias_CGMZ_Crafting_onAfterLoad.call(this);
	this.initializeRecipeData();
};
//-----------------------------------------------------------------------------
// Add craft result products to a scene key
//-----------------------------------------------------------------------------
CGMZ_Core.prototype.addCraftItemsToScene = function(itemArray, sceneKey) {
	if(!this._recipeSceneData[sceneKey]) this._recipeSceneData[sceneKey] = [];
	for(const itemObj of itemArray) {
		let arrayId = "";
		if(itemObj.Type === 'currency') {
			if(Imported.CGMZ_CurrencySystem && itemObj.ID) {
				arrayId = `currency-${itemObj.ID}`;
			} else {
				arrayId = `currency-default`;
			}
		} else {
			arrayId = `${itemObj.Type}-${itemObj.ID}`;
		}
		const index = this._recipeSceneData[sceneKey].findIndex(obj => obj.internalId === arrayId);
		if(index >= 0) {
			this._recipeSceneData[sceneKey][index].Amount += itemObj.Amount;
		} else {
			this._recipeSceneData[sceneKey].push({
				internalId: arrayId,
				ID: itemObj.ID,
				Type: itemObj.Type,
				Amount: itemObj.Amount
			});
		}
	}
};
//-----------------------------------------------------------------------------
// Get crafted items from a specific station
//-----------------------------------------------------------------------------
CGMZ_Core.prototype.getCraftedItemsForScene = function(sceneKey) {
	if(!this._recipeSceneData[sceneKey]) this._recipeSceneData[sceneKey] = [];
	return this._recipeSceneData[sceneKey];
};
//-----------------------------------------------------------------------------
// Check if a crafting station has items
//-----------------------------------------------------------------------------
CGMZ_Core.prototype.doesCraftingStationHaveItems = function(sceneKey) {
	if(!this._recipeSceneData[sceneKey]) this._recipeSceneData[sceneKey] = [];
	return this._recipeSceneData[sceneKey].length > 0;
};
//-----------------------------------------------------------------------------
// Clear items from a crafting station
//-----------------------------------------------------------------------------
CGMZ_Core.prototype.clearCraftingStationItems = function(sceneKey) {
	this._recipeSceneData[sceneKey] = [];
};
//-----------------------------------------------------------------------------
// Returns array of all recipes
//-----------------------------------------------------------------------------
CGMZ_Core.prototype.getAllRecipes = function() {
	return this._recipes;
};
//-----------------------------------------------------------------------------
// Returns array of all discovered recipes
//-----------------------------------------------------------------------------
CGMZ_Core.prototype.getAllDiscoveredRecipes = function() {
	const discoveredRecipes = [];
	for(const recipe of this._recipes) {
		if(recipe._discovered) discoveredRecipes.push(recipe);
	}
	return discoveredRecipes;
};
//-----------------------------------------------------------------------------
// Returns array of all discovered recipes of certain type (profession)
//-----------------------------------------------------------------------------
CGMZ_Core.prototype.getDiscoveredRecipesOfType = function(type) {
	const discoveredRecipes = [];
	for(const recipe of this._recipes) {
		if(recipe._discovered && recipe._profession === type) discoveredRecipes.push(recipe);
	}
	return discoveredRecipes;
};
//-----------------------------------------------------------------------------
// Get recipe by name. Returns null if unsuccessful
//-----------------------------------------------------------------------------
CGMZ_Core.prototype.getRecipe = function(name) {
	for(const recipe of this._recipes) {
		if(name === recipe._name) return recipe;
	}
	return null;
};
//-----------------------------------------------------------------------------
// Alters the discovered property of a recipe
//-----------------------------------------------------------------------------
CGMZ_Core.prototype.discoverRecipe = function(name, discovered) {
	const recipe = this.getRecipe(name);
	if(recipe) {
		(discovered) ? recipe.learn() : recipe.changeDiscoveredStatus(discovered);
	}
};
//-----------------------------------------------------------------------------
// Check if Item has a recipe learn property attached to it
//-----------------------------------------------------------------------------
CGMZ_Core.prototype.checkItemForRecipe = function(item) {
	if(item?.meta?.cgmzrecipe) {
		this.discoverRecipe(item.meta.cgmzrecipe, true);
	}
};
//-----------------------------------------------------------------------------
// Returns array of all discovered recipes
//-----------------------------------------------------------------------------
CGMZ_Core.prototype.checkCraftingRecipesForLearn = function() {
	for(const recipe of this._recipes) {
		if(recipe.canAutodiscover()) this.discoverRecipe(recipe._name, true);
	}
};
//=============================================================================
// CGMZ_Temp
//-----------------------------------------------------------------------------
// Adds plugin commands and initializes unsaved crafting data like generics
// and unsaved recipe data
//=============================================================================
//-----------------------------------------------------------------------------
// Set up generic item data and temp recipe data
//-----------------------------------------------------------------------------
const CGMZ_Crafting_CGMZTemp_createPluginData = CGMZ_Temp.prototype.createPluginData;
CGMZ_Temp.prototype.createPluginData = function() {
	CGMZ_Crafting_CGMZTemp_createPluginData.call(this);
	this.isCraftFilterKeyPressed = false;
	this._craftingGenericItems = {};
	this._recipes = {};
	this._craftingCategories = {};
	this.craftingSceneRequest = null;
	for(const itemJSON of CGMZ.Crafting.Generics) {
		const item = CGMZ_Utils.parseJSON(itemJSON, null, "[CGMZ] Crafting", "One of your generic items had invalid JSON and could not be read. Skipping.");
		if(!item) continue;
		const id = item.Type;
		this._craftingGenericItems[id] = new CGMZ_CraftingGenericItem(item);
	}
	for(const recipeJSON of CGMZ.Crafting.Recipes) {
		const recipeParse = CGMZ_Utils.parseJSON(recipeJSON, null, "[CGMZ] Crafting", "One of your recipes had invalid JSON and could not be read. Skipping.");
		if(!recipeParse) continue;
		const recipe = new CGMZ_RecipeTemp(recipeParse);
		this._recipes[recipe._name] = recipe;
	}
	for(const categoryJSON of CGMZ.Crafting.Categories) {
		const categoryParse = CGMZ_Utils.parseJSON(categoryJSON, null, "[CGMZ] Crafting", "One of your crafting categories had invalid JSON and could not be read. Skipping.");
		if(!categoryParse) continue;
		const category = new CGMZ_CraftingCategoryData(categoryParse);
		this._craftingCategories[category.id] = category;
	}
};
//-----------------------------------------------------------------------------
// Get crafting category
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.getCraftingCategoryData = function(id) {
	return this._craftingCategories[id];
};
//-----------------------------------------------------------------------------
// Get crafting generic item
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.getGenericCraftingItem = function(type) {
	return this._craftingGenericItems[type];
};
//-----------------------------------------------------------------------------
// Get recipe temp data
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.getRecipeTempData = function(name) {
	return this._recipes[name];
};
//-----------------------------------------------------------------------------
// Calculate the maximum amount of items that can be crafted
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.calculateMaxRecipeCraftTimes = function(name) {
	const recipeTemp = this.getRecipeTempData(name);
	const recipe = $cgmz.getRecipe(name);
	if(!recipeTemp || !recipe) return 0;
	const maxPossibleCrafts = 99;
	let max = maxPossibleCrafts;
	let tempMax = 0;
	for(const ingredient of recipeTemp.ingredients) {
		switch(ingredient.Type) {
			case 'currency':
				if(Imported.CGMZ_CurrencySystem && ingredient.ID) {
					tempMax = 9999;
					const currency = $cgmz.getCurrency(ingredient.ID);
					if(currency) {
						tempMax = Math.min(currency.amount() / ingredient.Amount, maxPossibleCrafts);
					}
				} else {
					tempMax = Math.min($gameParty.gold() / ingredient.Amount, maxPossibleCrafts);
				}
				if(tempMax < max) max = tempMax;
				break;
			case 'item':
			case 'weapon':
			case 'armor':
				const item = CGMZ_Utils.lookupItem(ingredient.Type, ingredient.ID);
				const numItems = $gameParty.numItems(item);
				tempMax = Math.min(numItems / ingredient.Amount, maxPossibleCrafts);
				if(tempMax < max) max = tempMax;
				break;
			case 'generic':
				const genItems = this.calculateTotalNumItemsOfGenericType(ingredient.GenericCategory);
				tempMax = Math.min(genItems / ingredient.Amount, maxPossibleCrafts);
				if(tempMax < max) max = tempMax;
				break;
		}
	}
	if(recipeTemp.productUniqueAmount > 0) {
		for(const product of recipeTemp.products) {
			if(product.Type === 'currency' || product.Type === 'generic') continue; // currency and generic cannot be unique
			const item = CGMZ_Utils.lookupItem(product.Type, product.ID);
			if(item) {
				const numItemsLeft = recipeTemp.productUniqueAmount - $gameParty.numItems(item);
				if(numItemsLeft < max) max = numItemsLeft;
			}
		}
	}
	if(max < 1) max = 0;
	return Math.floor(max);
};
//-----------------------------------------------------------------------------
// Calculate number of items/weapons/armors belonging to generic category
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.calculateTotalNumItemsOfGenericType = function(genericType) {
	let itemCount = 0;
	for(const item of $gameParty.allItems()) {
		if(item.meta && item.meta.cgmzcraftinggeneric && item.meta.cgmzcraftinggeneric.split(",").includes(genericType)) {
			itemCount += $gameParty.numItems(item);
		}
	}
	return itemCount;
};
//-----------------------------------------------------------------------------
// Give item products generated from craft
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.giveCraftedItems = function(itemArray) {
	for(const itemObj of itemArray) {
		if(itemObj.Type === 'currency') {
			if(Imported.CGMZ_CurrencySystem && itemObj.ID) {
				const currency = $cgmz.getCurrency(itemObj.ID);
				if(currency) currency.gainCurrency(itemObj.Amount);
			} else {
				$gameParty.gainGold(itemObj.Amount);
			}
		} else {
			const item = CGMZ_Utils.lookupItem(itemObj.Type, itemObj.ID);
			if(item) {
				$gameParty.gainItem(item, itemObj.Amount, false);
			}
		}
	}
};
//-----------------------------------------------------------------------------
// Update gamepad input
//-----------------------------------------------------------------------------
const alias_CGMZ_Crafting_updateLastGamepad = CGMZ_Temp.prototype.updateLastGamepad;
CGMZ_Temp.prototype.updateLastGamepad = function(gamepad) {
	alias_CGMZ_Crafting_updateLastGamepad.call(this, gamepad);
	if(gamepad?.buttons[CGMZ.Crafting.FilterGamepad].pressed) {
		this.isCraftFilterKeyPressed = true;
	}
};
//-----------------------------------------------------------------------------
// Create mapped functions
//-----------------------------------------------------------------------------
const alias_CGMZCrafting_createMappedFunctions = CGMZ_Temp.prototype.createMappedFunctions;
CGMZ_Temp.prototype.createMappedFunctions = function() {
	alias_CGMZCrafting_createMappedFunctions.call(this);
	this._mappedFunctions["onCraftTimerFinish"] = this.onCraftTimerFinish.bind(this);
};
//-----------------------------------------------------------------------------
// Processing when a craft timer finishes
// args is in the format:
// recipe: (string) recipe's name
// success: (boolean) craft was a success?
// prof: (object: {lv: profLevel, id: profId}) contains prof id (string) and prof level at time of craft start (number). Will be null if not using professions or not needed to pop scene
// hq: (boolean) if craft was high quality or not
// amount: (number) amount to craft if multicraft
// timeNeeded: (number) frame count of the craft
// currentTime: (number) frame count at the time the craft started
// timerKey: (string) the timer key
// sceneKey: (string) the scene key that the craft belongs to
// fullRecipeCraftTime: (number) the full amount of time the craft takes to create whether if successful
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.onCraftTimerFinish = function(args) {
	const recipe = $cgmz.getRecipe(args.recipe);
	const recipeTemp = $cgmzTemp.getRecipeTempData(args.recipe);
	if(!recipe) return;
	const isInCraftScene = SceneManager.CGMZ_isCurrentScene('CGMZ_Scene_Crafting');
	if(isInCraftScene) {
		let pop = false;
		if(args.prof) {
			const profession = $cgmz.getProfession(prof.id);
			if(profession && prof.lv < profession._level) pop = true;
		}
		$cgmzTemp.createCraftSceneRequest(args.recipe, args.success, args.hq, pop, args.amount - 1, args.timerKey, args.sceneKey);
	}
	recipe.finishCraft(args.success, args.hq, args.sceneKey);
	const newAmount = args.amount - 1;
	if(newAmount > 0 && recipe._discovered && recipe.canCraft()) {
		recipe.startCraft();
		const ext = $cgmzTemp.makeCraftTimerInfo(args.recipe, args.timerKey, args.sceneKey, newAmount);
		const timer = new CGMZ_Timer(
			ext.timeNeeded,
			ext.timerKey,
			"onCraftTimerFinish",
			ext
		);
		$cgmz.requestNewTimer(timer);
	} else {
		recipe.setupCompleteToast();
	}
};
//-----------------------------------------------------------------------------
// Make the craft timer info
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.makeCraftTimerInfo = function(recipeId, timerKey, sceneKey, amount) {
	const recipe = $cgmz.getRecipe(recipeId);
	const recipeTemp = $cgmzTemp.getRecipeTempData(recipeId);
	let success, timeNeeded, profLevel, profId, hq;
	if(recipe.calculateSuccessRate()/100.0 > Math.random()) {
		success = true;
		timeNeeded = recipeTemp.time;
	} else {
		success = false;
		timeNeeded = Math.max(Math.random()*recipeTemp.time, recipeTemp.time/2);
	}
	if(Imported.CGMZ_Professions && CGMZ.Crafting.PopSceneOnLevel) {
		const profession = $cgmz.getProfession(recipe._profession);
		if(profession) {
			profLevel = profession._level;
			profId = profession.getId();
		}
	}
	hq = (success) ? (recipe.calculateHighQualityRate()/100.0 > Math.random()) : false;
	return {
		recipe: recipeId,
		success: success,
		prof: (profId) ? {lv: profLevel, id: profId} : null,
		hq: hq,
		amount: amount,
		timeNeeded: timeNeeded,
		currentTime: Graphics.frameCount,
		timerKey: timerKey,
		sceneKey: sceneKey,
		fullRecipeCraftTime: recipeTemp.time
	};
};
//-----------------------------------------------------------------------------
// Create a request for the craft scene
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.createCraftSceneRequest = function(recipeId, success, hq, pop, newAmount, key, sceneKey) {
	this.craftingSceneRequest = {
		recipe: recipeId,
		success: success,
		hq: hq,
		pop: pop,
		amount: newAmount,
		key: key,
		sceneKey: sceneKey
	}
};
//-----------------------------------------------------------------------------
// Create a timer key for a recipe
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.createCraftingTimerKey = function(recipeName, sceneKey = "") {
	if(sceneKey) return `crafting-${recipeName}-${sceneKey}`;
	return `crafting-${recipeName}`;
};
//-----------------------------------------------------------------------------
// Register Plugin Commands
//-----------------------------------------------------------------------------
const alias_CGMZ_Crafting_registerPluginCommands = CGMZ_Temp.prototype.registerPluginCommands;
CGMZ_Temp.prototype.registerPluginCommands = function() {
	alias_CGMZ_Crafting_registerPluginCommands.call(this);
	PluginManager.registerCommand("CGMZ_Crafting", "discover", this.pluginCommandCraftingDiscover);
	PluginManager.registerCommand("CGMZ_Crafting", "Discover Batch", this.pluginCommandCraftingDiscoverBatch);
	PluginManager.registerCommand("CGMZ_Crafting", "Call Scene", this.pluginCommandCraftingCallScene);
	PluginManager.registerCommand("CGMZ_Crafting", "Set Description", this.pluginCommandCraftingSetDescription);
	PluginManager.registerCommand("CGMZ_Crafting", "Is Scene Crafting", this.pluginCommandCraftingIsSceneCrafting);
	PluginManager.registerCommand("CGMZ_Crafting", "Is Recipe Discovered", this.pluginCommandCraftingIsRecipeDiscovered);
	PluginManager.registerCommand("CGMZ_Crafting", "Reinitialize", this.pluginCommandCraftingReinitialize);
};
//-----------------------------------------------------------------------------
// Plugin Command - Reinitialize
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandCraftingReinitialize = function() {
	$cgmz.initializeRecipeData(true);
};
//-----------------------------------------------------------------------------
// Plugin Command - Discover
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandCraftingDiscover = function(args) {
	$cgmz.discoverRecipe(args.name, (args.discover === 'true'));
};
//-----------------------------------------------------------------------------
// Plugin Command - Discover Batch
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandCraftingDiscoverBatch = function(args) {
	const recipes = CGMZ_Utils.parseJSON(args.Recipes, [], '[CGMZ] Crafting', 'Your Discover Batch plugin command had invalid JSON and could not be read. It will do nothing.');
	for(const recipe of recipes) {
		$cgmz.discoverRecipe(recipe, (args.Discover === 'true'));
	}
};
//-----------------------------------------------------------------------------
// Plugin Command - Call Scene
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandCraftingCallScene = function(args) {
	SceneManager.push(CGMZ_Scene_Crafting);
	const types = CGMZ_Utils.parseJSON(args.type, [], "[CGMZ] Crafting", "Your Call Scene Plugin Command had invalid JSON for the 'type' argument.");
	const viewOnly = (args["View Only"] === 'true');
	SceneManager.prepareNextScene(types, viewOnly, args["Background Preset"], args["Scene Key"]);
};
//-----------------------------------------------------------------------------
// Plugin Command - Set Description
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandCraftingSetDescription = function(args) {
	const recipe = $cgmz.getRecipe(args.name);
	if(!recipe) return;
	recipe.setDescription(args.description);
};
//-----------------------------------------------------------------------------
// Plugin Command - Is Scene Crafting
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandCraftingIsSceneCrafting = function(args) {
	const key = args["Scene Key"];
	const s = Number(args["Game Switch"]);
	if(key && s) {
		const timers = $cgmz.getAllTimers();
		const isCrafting = timers.some(timer => timer._id.includes(key));
		$gameSwitches.setValue(s, isCrafting);
	}
};
//-----------------------------------------------------------------------------
// Plugin Command - Is Recipe Discovered
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandCraftingIsRecipeDiscovered = function(args) {
	const recipe = $cgmz.getRecipe(args["Recipe Name"]);
	$gameSwitches.setValue(Number(args.Switch), recipe._discovered);
};
//=============================================================================
// CGMZ_Scene_Crafting
//-----------------------------------------------------------------------------
// Handle the crafting scene
//=============================================================================
function CGMZ_Scene_Crafting(craftType) {
	this.initialize.apply(this, arguments);
}
CGMZ_Scene_Crafting.prototype = Object.create(Scene_MenuBase.prototype);
CGMZ_Scene_Crafting.prototype.constructor = CGMZ_Scene_Crafting;
//-----------------------------------------------------------------------------
// Initialize
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.initialize = function(craftType = null) {
	this._craftType = (craftType) ? craftType : [];
	this._viewOnly = false;
	this._bgPresetId = "";
	this._sceneKey = "";
	$cgmzTemp.isCraftFilterKeyPressed = false;
	this.clearFilters();
	Scene_MenuBase.prototype.initialize.call(this);
};
//-----------------------------------------------------------------------------
// Prepare
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.prepare = function(craftType = null, viewOnly = false, bgPresetId = "", sceneKey = "") {
	this._craftType = (craftType) ? craftType : [];
	this._viewOnly = viewOnly;
	this._bgPresetId = bgPresetId;
	this._sceneKey = sceneKey;
};
//-----------------------------------------------------------------------------
// Get the scene key of the scene
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.getSceneKey = function() {
	return this._sceneKey;
};
//-----------------------------------------------------------------------------
// Create crafting windows
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.create = function() {
	Scene_MenuBase.prototype.create.call(this);
	this.createCategoryWindow();
	this.createListWindow();
	this.createDisplayWindow();
	this.createProgressWindow();
	this.createConfirmationWindow();
	this.createMulticraftWindow();
	this.createFilterTypeWindow();
	this.createFilterOptionWindow();
	this.createCollectedItemsWindow();
	this.createCollectItemsCommandWindow();
	this.afterWindowCreate();
};
//-----------------------------------------------------------------------------
// Update scene - check for pop on level up, also check for recipe disagreement
// between list and display window, and update windows as needed
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.update = function() {
	Scene_MenuBase.prototype.update.call(this);
	const request = $cgmzTemp.craftingSceneRequest;
	if(this._listWindow && this._displayWindow) {
		const litem = this._listWindow.item();
		const ditem = this._displayWindow._recipe;
		if(litem && ditem && litem._name !== ditem._name) {
			this._listWindow.callUpdateHelp();
		}
		if(request && litem) {
			this._listWindow.requestRefresh();
			const keySplit = request.key.split('-');
			const key = (keySplit.length === 3) ? keySplit[2] : "";
			if((litem._name === request.recipe && key === this._sceneKey) || CGMZ.Crafting.WaitForFinish) {
				this.handleRequestForCurrentlySelectedRecipe();
			}
		}
	}
	if(request?.pop) this.popScene();
	$cgmzTemp.craftingSceneRequest = null;
	if(this._listWindow?.active) {
		this.updateFilterInput();
	}
};
//-----------------------------------------------------------------------------
// Handle a request for the currently selected recipe
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.handleRequestForCurrentlySelectedRecipe = function() {
	const request = $cgmzTemp.craftingSceneRequest;
	this._displayWindow.requestRefresh();
	const recipeTemp = $cgmzTemp.getRecipeTempData(request.recipe);
	if(request.amount) {
		this._progressWindow.resetForNextCraft();
	} else {
		this._progressWindow.startDisplayCraftResult(request.success, request.hq);
	}
	if(recipeTemp) {
		const se = (request.success) ? recipeTemp.successSE : recipeTemp.failSE;
		AudioManager.playSe(se);
	}
	if(Imported.CGMZ_Rumble) {
		request.success ? $cgmzTemp.startRumble(CGMZ.Crafting.SuccessRumble) : $cgmzTemp.startRumble(CGMZ.Crafting.FailRumble)
	}
	if(CGMZ.Crafting.WaitForFinish) {
		this._listWindow.selectRecipeByName(request.recipe);
		const timer = $cgmz.getTimerById(request.key);
		if(!timer) this.activateListWindow();
	}
};
//-----------------------------------------------------------------------------
// Update to check for filter input. Called when the list window is active.
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.updateFilterInput = function() {
	if(!CGMZ.Crafting.AllowFilter) return;
	if($cgmzTemp.isKeyPressed(CGMZ.Crafting.FilterKey)) {
		this.startFilterFlow();
	} else if($cgmzTemp.isCraftFilterKeyPressed) {
		this.startFilterFlow();
	}
};
//-----------------------------------------------------------------------------
// Create category window
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.createCategoryWindow = function() {
	if(!CGMZ.Crafting.ShowCategoryWindow) return;
	const rect = this.categoryWindowRect();
	this._categoryWindow = new CGMZ_Window_RecipeCategory(rect, this._craftType);
	this._categoryWindow.setHandler('cancel', this.popScene.bind(this));
	this._categoryWindow.setHandler('ok', this.onCategoryOk.bind(this));
	this.addWindow(this._categoryWindow);
};
//-----------------------------------------------------------------------------
// Get the category window rect
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.categoryWindowRect = function() {
	const x = 0;
	const y = this.hasTouchUI() ? this.buttonAreaHeight() : 0;
	const width = Graphics.boxWidth;
	const height = this.calcWindowHeight(1, true);
	return new Rectangle(x, y, width, height);
};
//-----------------------------------------------------------------------------
// Create list window
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.createListWindow = function() {
	const rect = this.listWindowRect();
	this._listWindow = new CGMZ_Window_RecipeList(rect, this._craftType);
	this._listWindow.setHandler('cancel', this.onListCancel.bind(this));
	this._listWindow.setHandler('ok', this.onListOk.bind(this));
	this.addWindow(this._listWindow);
};
//-----------------------------------------------------------------------------
// Get the list window rect
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.listWindowRect = function() {
	const y = this._categoryWindow ? this._categoryWindow.y + this._categoryWindow.height : this.hasTouchUI() ? this.buttonAreaHeight() : 0;
	const height = Graphics.boxHeight - y;
	const width = Graphics.boxWidth * (CGMZ.Crafting.ListWindowWidth / 100.0);
	const x = CGMZ.Crafting.ListWindowRight ? Graphics.boxWidth - width : 0;
	return new Rectangle(x, y, width, height);
};
//-----------------------------------------------------------------------------
// Create display window
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.createDisplayWindow = function() {
	const rect = this.displayWindowRect();
	this._displayWindow = new CGMZ_Window_RecipeDisplay(rect);
	this.addWindow(this._displayWindow);
};
//-----------------------------------------------------------------------------
// Get the display window rect
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.displayWindowRect = function() {
	const x = CGMZ.Crafting.ListWindowRight ? 0 : this._listWindow.width;
	const y = this._listWindow.y;
	const height = this._listWindow.height - this.calcWindowHeight(1, false) * !CGMZ.Crafting.InstantCrafting;
	const width = Graphics.boxWidth - this._listWindow.width;
	return new Rectangle(x, y, width, height);
};
//-----------------------------------------------------------------------------
// Create progress window
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.createProgressWindow = function() {
	const rect = this.progressWindowRect();
	this._progressWindow = new CGMZ_Window_RecipeProgress(rect, this._sceneKey);
	this.addWindow(this._progressWindow);
};
//-----------------------------------------------------------------------------
// Get rectangle for progress window
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.progressWindowRect = function() {
	const x = this._displayWindow.x;
	const y = this._displayWindow.y + this._displayWindow.height;
	const height = this.calcWindowHeight(1, false);
	const width = this._displayWindow.width;
	const rect = (CGMZ.Crafting.InstantCrafting) ? new Rectangle(0, 0, 0, 0) : new Rectangle(x, y, width, height);
	return rect;
};
//-----------------------------------------------------------------------------
// Create confirmation window
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.createConfirmationWindow = function() {
	const rect = this.confirmationWindowRect();
	this._confirmationWindow = new CGMZ_Window_CraftConfirmation(rect);
	this._confirmationWindow.setHandler('craft', this.onCraftConfirm.bind(this));
	this._confirmationWindow.setHandler('cancel', this.onCraftCancel.bind(this));
	this.addWindow(this._confirmationWindow);
};
//-----------------------------------------------------------------------------
// Get rectangle for confirmation window
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.confirmationWindowRect = function() {
	const height = this.calcWindowHeight(2, true);
	const width = Graphics.boxWidth / 4;
	const x = Graphics.boxWidth / 2 - width / 2;
	const y = Graphics.boxHeight / 2 - height / 2;
	return new Rectangle(x, y, width, height);
};
//-----------------------------------------------------------------------------
// Create multicraft window
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.createMulticraftWindow = function() {
	const rect = this.multicraftWindowRect();
	this._multicraftWindow = new CGMZ_Window_Multicraft(rect);
	this._multicraftWindow.setHandler('ok', this.onMulticraftConfirm.bind(this));
	this._multicraftWindow.setHandler('cancel', this.onMulticraftCancel.bind(this));
	this.addWindow(this._multicraftWindow);
};
//-----------------------------------------------------------------------------
// Get rectangle for multicraft window
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.multicraftWindowRect = function() {
	const height = this.calcWindowHeight(1 + 1 * ConfigManager.touchUI, true);
	const width = Graphics.boxWidth * (CGMZ.Crafting.MulticraftWindowWidth / 100.0);
	const x = Graphics.boxWidth / 2 - width / 2;
	const y = Graphics.boxHeight / 2 - height / 2;
	return new Rectangle(x, y, width, height);
};
//-----------------------------------------------------------------------------
// Create filter type window
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.createFilterTypeWindow = function() {
	const rect = this.filterTypeWindowRect();
	this._filterTypeWindow = new CGMZ_Window_FilterType(rect);
	this._filterTypeWindow.setHandler('ok', this.onFilterTypeConfirm.bind(this));
	this._filterTypeWindow.setHandler('cancel', this.onFilterTypeCancel.bind(this));
	this.addWindow(this._filterTypeWindow);
};
//-----------------------------------------------------------------------------
// Get initial rectangle for filter type window
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.filterTypeWindowRect = function() {
	const height = this.calcWindowHeight(this.getFilterTypeOptionCount(), true);
	const width = Graphics.boxWidth * (0.5);
	const x = Graphics.boxWidth / 2 - width / 2;
	const y = Graphics.boxHeight / 2 - height / 2;
	return new Rectangle(x, y, width, height);
};
//-----------------------------------------------------------------------------
// Get count of filter types
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.getFilterTypeOptionCount = function() {
	let count = 1; // There is always the Remove Filter option
	if(this._categoryFilters.length > 0) count++;
	if(this._subcategoryFilters.length > 0) count++;
	if(this._qualityFilters.length > 0) count++;
	return count;
};
//-----------------------------------------------------------------------------
// Create filter option window
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.createFilterOptionWindow = function() {
	const rect = this.filterOptionWindowRect();
	this._filterOptionWindow = new CGMZ_Window_FilterOption(rect);
	this._filterOptionWindow.setHandler('ok', this.onFilterOptionConfirm.bind(this));
	this._filterOptionWindow.setHandler('cancel', this.onFilterOptionCancel.bind(this));
	this.addWindow(this._filterOptionWindow);
};
//-----------------------------------------------------------------------------
// Get initial rectangle for filter option window
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.filterOptionWindowRect = function() {
	const height = this.calcWindowHeight(this.getFilterOptionOptionCount(), true);
	const width = Graphics.boxWidth * (0.5);
	const x = Graphics.boxWidth / 2 - width / 2;
	const y = Graphics.boxHeight / 2 - height / 2;
	return new Rectangle(x, y, width, height);
};
//-----------------------------------------------------------------------------
// Get count of filter types
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.getFilterOptionOptionCount = function() {
	const type = this._filterTypeWindow.item();
	switch(type) {
		case 'quality': return this._qualityFilters.length;
		case 'category': return this._categoryFilters.length;
		case 'subcategory': return this._subcategoryFilters.length;
	}
	return 0;
};
//-----------------------------------------------------------------------------
// Create collected items window
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.createCollectedItemsWindow = function() {
	const rect = this.collectedItemsWindowRect();
	this._collectedItemsWindow = new CGMZ_Window_CraftingCollectedItems(rect, this._sceneKey);
	this.addChild(this._collectedItemsWindow);
};
//-----------------------------------------------------------------------------
// Get the collected items window rect
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.collectedItemsWindowRect = function() {
	const width = Graphics.boxWidth * 3 / 4;
	const height = Graphics.boxHeight / 2;
	const x = (Graphics.boxWidth - width) / 2;
	const y = (Graphics.boxHeight - height) / 2;
	return new Rectangle(x, y, width, height);
};
//-----------------------------------------------------------------------------
// Create collect items ok window
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.createCollectItemsCommandWindow = function() {
	const rect = this.collectItemsCommandWindowRect();
	this._collectItemsCommandWindow = new CGMZ_Window_CraftingCollectItemsCommand(rect);
	this._collectItemsCommandWindow.setHandler('ok', this.onCollectConfirm.bind(this));
	this._collectItemsCommandWindow.setHandler('cancel', this.onCollectConfirm.bind(this));
	this.addChild(this._collectItemsCommandWindow);
};
//-----------------------------------------------------------------------------
// Get the collect items ok window rect
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.collectItemsCommandWindowRect = function() {
	const width = this._collectedItemsWindow.width;
	const height = this.calcWindowHeight(1, true);
	const x = this._collectedItemsWindow.x;
	const y = this._collectedItemsWindow.y + this._collectedItemsWindow.height;
	return new Rectangle(x, y, width, height);
};
//-----------------------------------------------------------------------------
// Processing after windows have been created
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.afterWindowCreate = function() {
	this._listWindow.setDisplayWindow(this._displayWindow);
	this._listWindow.setProgressWindow(this._progressWindow);
	if($cgmz.doesCraftingStationHaveItems(this._sceneKey)) {
		this._collectedItemsWindow.show();
		this._collectItemsCommandWindow.show();
		this._collectItemsCommandWindow.activate();
		this._collectItemsCommandWindow.select(0);
		const collectedItems = $cgmz.getCraftedItemsForScene(this._sceneKey);
		$cgmzTemp.giveCraftedItems(collectedItems);
		$cgmz.clearCraftingStationItems(this._sceneKey);
	} else {
		if(this._categoryWindow) {
			this._categoryWindow.setListWindow(this._listWindow);
			this._categoryWindow.activate();
		} else {
			this._listWindow.activate();
		}
		this.setFilters();
	}
	if(this._listWindow.active && this.hasFilter()) this._filterButton?.show();
};
//-----------------------------------------------------------------------------
// Check if should make room for Touch UI
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.hasTouchUI = function() {
	return !CGMZ.Crafting.DisableTouchUISpace || ConfigManager.touchUI;
};
//-----------------------------------------------------------------------------
// On collect item confirm
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.onCollectConfirm = function() {
	this._collectedItemsWindow.hide();
	this._collectItemsCommandWindow.deactivate();
	this._collectItemsCommandWindow.hide();
	if(this._categoryWindow) {
		this._categoryWindow.setListWindow(this._listWindow);
		this._categoryWindow.activate();
	} else {
		this._listWindow.activate();
	}
	this.setFilters();
};
//-----------------------------------------------------------------------------
// On Category Ok
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.onCategoryOk = function() {
	this.activateListWindow();
	this.setFilters();
	this._listWindow.select(0);
	this._categoryWindow.deactivate();
};
//-----------------------------------------------------------------------------
// On List Cancel
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.onListCancel = function() {
	if(this._categoryWindow) {
		this._listWindow.setFilterMode(null, null);
		this._listWindow.select(0);
		this._listWindow.ensureCursorVisible();
		this._listWindow.deselect();
		this._categoryWindow.activate();
		this._listWindow.deactivate();
		this._displayWindow.setItem(0);
		this._displayWindow.contents.clear();
		this._filterButton?.hide();
		this._progressWindow.setItem(null);
	} else {
		this.popScene();
	}
};
//-----------------------------------------------------------------------------
// On List Ok
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.onListOk = function() {
	if(!this._viewOnly) {
		if(this._displayWindow.canCraft() && this._progressWindow.canCraft()) {
			this._listWindow.deactivate();
			this._filterButton?.hide();
			if(CGMZ.Crafting.ShowConfirmationWindow) {
				this._confirmationWindow.activate();
				this._confirmationWindow.show();
			} else {
				if(CGMZ.Crafting.AllowMulticraft) {
					this.startMulticraft();
				} else {
					this.startCraft();
				}
			}
		} else {
			this._listWindow.activate();
		}
	} else {
		this._listWindow.activate();
	}
};
//-----------------------------------------------------------------------------
// On Craft Confirmation
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.onCraftConfirm = function() {
	this._confirmationWindow.deactivate();
	this._confirmationWindow.hide();
	if(CGMZ.Crafting.AllowMulticraft) {
		this.startMulticraft();
	} else {
		this.startCraft();
	}
};
//-----------------------------------------------------------------------------
// On Craft Confirmation Cancel
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.onCraftCancel = function() {
	this.activateListWindow();
	this._confirmationWindow.deactivate();
	this._confirmationWindow.hide();
};
//-----------------------------------------------------------------------------
// On Multicraft Confirmation
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.onMulticraftConfirm = function() {
	this._multicraftWindow.deactivate();
	this._multicraftWindow.hide();
	this.startCraft();
};
//-----------------------------------------------------------------------------
// On Multicraft Cancel
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.onMulticraftCancel = function() {
	this.activateListWindow();
	this._multicraftWindow.deactivate();
	this._multicraftWindow.hide();
};
//-----------------------------------------------------------------------------
// Start Multicraft Select
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.startMulticraft = function() {
	const maxCrafts = $cgmzTemp.calculateMaxRecipeCraftTimes(this._displayWindow._recipe?._name);
	this._multicraftWindow.setup(maxCrafts);
	this._multicraftWindow.activate();
	this._multicraftWindow.show();
};
//-----------------------------------------------------------------------------
// Start Crafting
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.startCraft = function() {
	if(CGMZ.Crafting.InstantCrafting) {
		this._progressWindow.doInstantCraft(this._multicraftWindow.number());
		this._listWindow.refresh();
		this._displayWindow.refresh();
		this.activateListWindow();
	} else {
		this._progressWindow.startCraft(this._multicraftWindow.number());
		this._displayWindow.refresh();
	}
	if(!CGMZ.Crafting.WaitForFinish) this.activateListWindow();
};
//-----------------------------------------------------------------------------
// Processing when activating the list window
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.activateListWindow = function() {
	$cgmzTemp.isCraftFilterKeyPressed = false;
	this._listWindow.activate();
	if(this.hasFilter()) this._filterButton?.show();
};
//-----------------------------------------------------------------------------
// Check if should provide filter option
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.hasFilter = function() {
	return this._hasFilters;
};
//-----------------------------------------------------------------------------
// Set up filter info
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.setFilters = function() {
	if(!CGMZ.Crafting.AllowFilter) return;
	if(!this._listWindow) return;
	const category = this._listWindow._category;
	this.clearFilters();
	this._categoryFilters = this.getCategoryFilters();
	this._qualityFilters = this.getQualityFilters(category);
	this._subcategoryFilters = this.getSubcategoryFilters(category);
	this._hasFilters = (this._categoryFilters.length > 0 || this._qualityFilters.length > 0 || this._subcategoryFilters.length > 0);
};
//-----------------------------------------------------------------------------
// Get possible category filters
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.getCategoryFilters = function() {
	if(this._categoryWindow) return [];
	return CGMZ.Crafting.CategoryFilterOptions;
};
//-----------------------------------------------------------------------------
// Get possible subcategory filters
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.getSubcategoryFilters = function(category) {
	const possibleFilters = CGMZ.Crafting.SubcategoryFilterOptions.map((opt) => {
		const split = opt.split(":");
		return (split[0] === category || split[0].toLowerCase() === 'all') ? split[1] : null;
	});
	return possibleFilters.filter(x => !!x);
};
//-----------------------------------------------------------------------------
// Get possible quality filters
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.getQualityFilters = function(category) {
	const possibleFilters = CGMZ.Crafting.QualityFilterOptions.map((opt) => {
		const split = opt.split(":");
		return (split[0] === category || split[0].toLowerCase() === 'all') ? split[1] : null;
	});
	return possibleFilters.filter(x => !!x);
};
//-----------------------------------------------------------------------------
// Clear Filters
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.clearFilters = function() {
	this._categoryFilters = [];
	this._qualityFilters = [];
	this._subcategoryFilters = [];
	this._hasFilters = false;
};
//-----------------------------------------------------------------------------
// Get Filters
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.getFilters = function() {
	return {
		category: this._categoryFilters,
		quality: this._qualityFilters,
		subcategory: this._subcategoryFilters
	};
};
//-----------------------------------------------------------------------------
// Start the filter flow, if possible
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.startFilterFlow = function() {
	if(!this.canFilter()) return;
	this._listWindow.deactivate();
	const rect = this.filterTypeWindowRect();
	this._filterTypeWindow.move(rect.x, rect.y, rect.width, rect.height);
	this._filterTypeWindow.createContents();
	this._filterTypeWindow.refresh(this.getFilters());
	this._filterTypeWindow.activate();
	this._filterTypeWindow.show();
};
//-----------------------------------------------------------------------------
// Check if filtering is currently possible
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.canFilter = function() {
	return this._listWindow.active;
};
//-----------------------------------------------------------------------------
// On Filter Type Cancel
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.onFilterTypeCancel = function() {
	this._filterTypeWindow.deactivate();
	this._filterTypeWindow.hide();
	this.activateListWindow();
};
//-----------------------------------------------------------------------------
// On Filter Type Confirm
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.onFilterTypeConfirm = function() {
	const type = this._filterTypeWindow.item();
	if(type === 'remove') {
		this._filterTypeWindow.deactivate();
		this._filterTypeWindow.hide();
		this._listWindow.setFilterMode(null, null);
		this.activateListWindow();
	} else {
		const rect = this.filterOptionWindowRect();
		this._filterOptionWindow.move(rect.x, rect.y, rect.width, rect.height);
		this._filterOptionWindow.createContents();
		this._filterOptionWindow.setFilterType(type);
		this._filterOptionWindow.refresh(this.getFilters());
		this._filterTypeWindow.deactivate();
		this._filterTypeWindow.hide();
		this._filterOptionWindow.activate();
		this._filterOptionWindow.show();
	}
};
//-----------------------------------------------------------------------------
// On Filter Option Cancel
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.onFilterOptionCancel = function() {
	this._filterOptionWindow.deactivate();
	this._filterOptionWindow.hide();
	this._filterTypeWindow.activate();
	this._filterTypeWindow.show();
};
//-----------------------------------------------------------------------------
// On Filter Option Confirm
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.onFilterOptionConfirm = function() {
	const type = this._filterTypeWindow.item();
	const value = this._filterOptionWindow.item();
	this._filterOptionWindow.deactivate();
	this._filterOptionWindow.hide();
	this._listWindow.setFilterMode(type, value);
	this.activateListWindow();
};
//-----------------------------------------------------------------------------
// Create filter button if needed
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.createButtons = function() {
	Scene_MenuBase.prototype.createButtons.call(this);
	this.createFilterButton();
};
//-----------------------------------------------------------------------------
// Create filter Button
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.createFilterButton = function() {
	this._filterButton = new Sprite_Button("cgmzFilter");
	this._filterButton.x = Graphics.boxWidth - this._filterButton.width - 4 - this._cancelButton?.width - 4;
	this._filterButton.y = this.buttonY();
	this._filterButton.setClickHandler(this.startFilterFlow.bind(this));
	this._filterButton.hide();
	this.addWindow(this._filterButton);
};
//-----------------------------------------------------------------------------
// Get the scene's custom scene background
// No need to check if Scene Backgrounds is installed because this custom func
// is only called by that plugin
//-----------------------------------------------------------------------------
CGMZ_Scene_Crafting.prototype.CGMZ_getCustomSceneBackground = function() {
	const presetId = this._bgPresetId || CGMZ.Crafting.SceneBackground;
	return $cgmzTemp.sceneBackgroundPresets[presetId];
};
//=============================================================================
// Sprite_Button
//-----------------------------------------------------------------------------
// Add filter button
//=============================================================================
//-----------------------------------------------------------------------------
// Alias. If undefined, check if filter button and return expected results
//-----------------------------------------------------------------------------
const alias_CGMZ_Crafting_SpriteButton_buttonData = Sprite_Button.prototype.buttonData;
Sprite_Button.prototype.buttonData = function() {
	data = alias_CGMZ_Crafting_SpriteButton_buttonData.call(this);
	if(data) return data;
	const CGMZCraftingButtonTable = {
		cgmzFilter: {x: CGMZ.Crafting.FilterButtonOffset, w: CGMZ.Crafting.FilterButtonWidth}
	};
	return CGMZCraftingButtonTable[this._buttonType];
};
//=============================================================================
// CGMZ_Window_RecipeCategory
//-----------------------------------------------------------------------------
// Selectable window for choosing a recipe type
//=============================================================================
function CGMZ_Window_RecipeCategory(rect) {
	this.initialize.apply(this, arguments);
}
CGMZ_Window_RecipeCategory.prototype = Object.create(Window_Selectable.prototype);
CGMZ_Window_RecipeCategory.prototype.constructor = CGMZ_Window_RecipeCategory;
//-----------------------------------------------------------------------------
// Initialize
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeCategory.prototype.initialize = function(rect, craftType) {
	Window_Selectable.prototype.initialize.call(this, rect);
	if(Imported.CGMZ_WindowBackgrounds && CGMZ.Crafting.CategoryWindowBackground) this.CGMZ_setWindowBackground(CGMZ.Crafting.CategoryWindowBackground);
	if(Imported.CGMZ_WindowSettings && CGMZ.Crafting.CategoryWindowSettings) this.CGMZ_setWindowSettings(CGMZ.Crafting.CategoryWindowSettings);
	this._craftType = craftType;
	this._data = [];
	this.refresh();
	this.select(0);
};
//-----------------------------------------------------------------------------
// Max items
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeCategory.prototype.maxItems = function() {
	return this._data ? this._data.length : 1;
};
//-----------------------------------------------------------------------------
// Max columns
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeCategory.prototype.maxCols = function() {
	return 4;
};
//-----------------------------------------------------------------------------
// Current item
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeCategory.prototype.item = function() {
	return this._data[this.index()];
};
//-----------------------------------------------------------------------------
// Refresh
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeCategory.prototype.refresh = function() {
	this.makeItemList();
	this.createContents();
	this.drawAllItems();
};
//-----------------------------------------------------------------------------
// Make item list
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeCategory.prototype.makeItemList = function() {
	this._data = [];
	if(this._craftType.length === 0) {
		const recipes = $cgmz.getAllDiscoveredRecipes().filter(this.includeRecipeInList);
		for(const recipe of recipes) {
			const recipeCategory = recipe._profession;
			if(!this._data.includes(recipeCategory)) this._data.push(recipeCategory);
		}
		this._data.sort((a, b) => {
			const ia = CGMZ.Crafting.CategorySortOrder.indexOf(a);
			const ib = CGMZ.Crafting.CategorySortOrder.indexOf(b);
			return ib - ia;
		});
	}
	else {
		this._data = this._craftType;
	}
};
//-----------------------------------------------------------------------------
// Determine if recipe should be included in list
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeCategory.prototype.includeRecipeInList = function(recipe) {
	if(CGMZ.Crafting.HideUncraftableRecipes && !recipe.canCraft()) return false;
	if(CGMZ.Crafting.HideHighLevelRecipes && !recipe.meetsLevelRequirements()) return false;
	return true;
};
//-----------------------------------------------------------------------------
// Draw item in list
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeCategory.prototype.drawItem = function(index) {
	const item = this._data[index];
	const category = $cgmzTemp.getCraftingCategoryData(item) || {commandText: item};
	const rect = this.itemRectWithPadding(index);
	this.CGMZ_drawTextLine(category.commandText, rect.x, rect.y, rect.width, 'center');
};
//-----------------------------------------------------------------------------
// Set list window
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeCategory.prototype.setListWindow = function(listWindow) {
	this._listWindow = listWindow;
	this.callUpdateHelp();
};
//-----------------------------------------------------------------------------
// See if can update list window
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeCategory.prototype.callUpdateHelp = function() {
	if(this.active && this._listWindow) {
		this._listWindow.setItem(this.item());
	}
};
//-----------------------------------------------------------------------------
// If refresh is requested from other window
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeCategory.prototype.requestRefresh = function() {
	this.refresh();
};
//=============================================================================
// CGMZ_Window_RecipeList
//-----------------------------------------------------------------------------
// Selectable window for choosing a recipe in a list.
//=============================================================================
function CGMZ_Window_RecipeList(rect) {
	this.initialize.apply(this, arguments);
}
CGMZ_Window_RecipeList.prototype = Object.create(Window_Selectable.prototype);
CGMZ_Window_RecipeList.prototype.constructor = CGMZ_Window_RecipeList;
//-----------------------------------------------------------------------------
// Initialize
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeList.prototype.initialize = function(rect, craftType) {
	Window_Selectable.prototype.initialize.call(this, rect);
	if(Imported.CGMZ_WindowBackgrounds && CGMZ.Crafting.ListWindowBackground) this.CGMZ_setWindowBackground(CGMZ.Crafting.ListWindowBackground);
	if(Imported.CGMZ_WindowSettings && CGMZ.Crafting.ListWindowSettings) this.CGMZ_setWindowSettings(CGMZ.Crafting.ListWindowSettings);
	this._craftType = craftType;
	this._filterMode = null;
	this._filterValue = null;
	this._category = null;
	this._data = [];
	this.refresh();
	this.select(0);
};
//-----------------------------------------------------------------------------
// Update
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeList.prototype.update = function() {
	Window_Selectable.prototype.update.call(this);
	if(Imported.CGMZ_Professions && $cgmzTemp._professionBuffRemoved) {
		$cgmzTemp._professionBuffRemoved = false;
		this.refresh();
		if(this.index() > this.topIndex()) this.select(this.topIndex());
	}
};
//-----------------------------------------------------------------------------
// Process ok
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeList.prototype.processOk = function() {
	const item = this._data[this.index()];
	if(item && item.canCraft()) {
		this.playOkSound();
		this.updateInputData();
		this.callOkHandler();
	} else {
		SoundManager.playBuzzer();
	}
};
//-----------------------------------------------------------------------------
// Max items
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeList.prototype.maxItems = function() {
	return this._data ? this._data.length : 1;
};
//-----------------------------------------------------------------------------
// Current item
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeList.prototype.item = function() {
	return this._data[this.index()];
};
//-----------------------------------------------------------------------------
// Set the category for the list window
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeList.prototype.setItem = function(category) {
	if(category && this._category && category === this._category) return;
	this._category = category;
	this.refresh();
};
//-----------------------------------------------------------------------------
// Select a recipe in the list by name
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeList.prototype.selectRecipeByName = function(name) {
	if(!this._data) return;
	const index = this._data.findIndex((item) => item._name === name);
	if(index >= 0) this.select(index);
};
//-----------------------------------------------------------------------------
// Set the filter mode for the window
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeList.prototype.setFilterMode = function(mode, value) {
	if(mode === this._filterMode && value === this._filterValue) return;
	this._filterMode = mode;
	this._filterValue = value;
	this.refresh();
	this.smoothSelect(0);
};
//-----------------------------------------------------------------------------
// Refresh
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeList.prototype.refresh = function() {
	this.makeItemList();
	Window_Selectable.prototype.refresh.call(this);
};
//-----------------------------------------------------------------------------
// Make item list
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeList.prototype.makeItemList = function() {
	this._data = this.getRecipeList();
};
//-----------------------------------------------------------------------------
// Get a list of recipes
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeList.prototype.getRecipeList = function() {
	let list = [];
	if(this._craftType.length === 0) {
		list = $cgmz.getAllDiscoveredRecipes();
	} else {
		for(const craftType of this._craftType) {
			list = list.concat($cgmz.getDiscoveredRecipesOfType(craftType));
		}
	}
	return list.filter(this.includeRecipeInList.bind(this));
};
//-----------------------------------------------------------------------------
// Determine if recipe should be included in list
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeList.prototype.includeRecipeInList = function(recipe) {
	if(this._category) {
		if(recipe._profession !== this._category) return false;
	}
	if(CGMZ.Crafting.HideUncraftableRecipes && !recipe.canCraft()) return false;
	if(CGMZ.Crafting.HideHighLevelRecipes && !recipe.meetsLevelRequirements()) return false;
	if(!this.meetsFilterConditions(recipe)) return false;
	return true;
};
//-----------------------------------------------------------------------------
// Check any active filters
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeList.prototype.meetsFilterConditions = function(recipe) {
	const tempRecipeData = $cgmzTemp.getRecipeTempData(recipe._name);
	if(!tempRecipeData) return true;
	switch(this._filterMode) {
		case 'category': return recipe._profession === this._filterValue;
		case 'quality': return tempRecipeData.quality === this._filterValue;
		case 'subcategory': return tempRecipeData.subcategory === this._filterValue;
	}
	return true;
};
//-----------------------------------------------------------------------------
// Draw item in list
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeList.prototype.drawItem = function(index) {
	const item = this._data[index];
	const tempItem = $cgmzTemp.getRecipeTempData(item._name);
	if(!tempItem) return;
	const rect = this.itemRectWithPadding(index);
	const string = (tempItem.iconIndex) ? `\\i[${tempItem.iconIndex}]${tempItem.getName()}` : tempItem.getName();
	this.changePaintOpacity(item.canCraft());
	this.CGMZ_drawTextLine(string, rect.x, rect.y, rect.width, 'left');
	this.changePaintOpacity(true);
};
//-----------------------------------------------------------------------------
// Set display window
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeList.prototype.setDisplayWindow = function(displayWindow) {
	this._displayWindow = displayWindow;
	this.callUpdateHelp();
};
//-----------------------------------------------------------------------------
// Set progress window
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeList.prototype.setProgressWindow = function(progressWindow) {
	this._progressWindow = progressWindow;
	this.callUpdateHelp();
};
//-----------------------------------------------------------------------------
// See if can update display window
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeList.prototype.callUpdateHelp = function() {
	if(this.active && this._displayWindow) {
		this._displayWindow.setItem(this.item());
	}
	if(this.active && this._progressWindow) {
		this._progressWindow.setItem(this.item());
	}
};
//-----------------------------------------------------------------------------
// If refresh is requested from other window
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeList.prototype.requestRefresh = function() {
	this.refresh();
};
//=============================================================================
// CGMZ_Window_RecipeDisplay
//-----------------------------------------------------------------------------
// Window displaying recipe information
//=============================================================================
function CGMZ_Window_RecipeDisplay() {
	this.initialize.apply(this, arguments);
}
CGMZ_Window_RecipeDisplay.prototype = Object.create(CGMZ_Window_Scrollable.prototype);
CGMZ_Window_RecipeDisplay.prototype.constructor = CGMZ_Window_RecipeDisplay;
//-----------------------------------------------------------------------------
// Initialize
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeDisplay.prototype.initialize = function(rect) {
	const heightMultiplier = 10; // maximum of 10 windows tall of data to scroll
	CGMZ_Window_Scrollable.prototype.initialize.call(this, rect, heightMultiplier, CGMZ.Crafting.ScrollWait, CGMZ.Crafting.ScrollSpeed, CGMZ.Crafting.AutoScroll, CGMZ.Crafting.ScrollDeceleration);
	if(Imported.CGMZ_WindowBackgrounds && CGMZ.Crafting.DisplayWindowBackground) this.CGMZ_setWindowBackground(CGMZ.Crafting.DisplayWindowBackground);
	if(Imported.CGMZ_WindowSettings && CGMZ.Crafting.DisplayWindowSettings) this.CGMZ_setWindowSettings(CGMZ.Crafting.DisplayWindowSettings);
	this._recipe = null;
	this._largeIconWidth = ImageManager.iconWidth*2.2;
	this._largeIconHeight = ImageManager.iconHeight*2.2;
	this._iconBitmap = ImageManager.loadSystem('IconSet'); //only load this once
	this.deactivate();
	this._iconSprite = new Sprite();
	this.addInnerChild(this._iconSprite);
};
//-----------------------------------------------------------------------------
// Update
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeDisplay.prototype.update = function() {
	CGMZ_Window_Scrollable.prototype.update.call(this);
	if(Imported.CGMZ_Professions && this._recipe) {
		const profession = $cgmz.getProfession(this._recipe._profession);
		if(profession && profession._needRefreshForBuff) {
			this.refresh();
		}
	}
};
//-----------------------------------------------------------------------------
// Determine if the recipe shown can be crafted
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeDisplay.prototype.canCraft = function() {
	if(this._recipe) return this._recipe.canCraft();
	return false;
};
//-----------------------------------------------------------------------------
// Set the recipe to be displayed (do nothing if already being displayed)
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeDisplay.prototype.setItem = function(recipe) {
	if(this._recipe && recipe && this._recipe._name === recipe._name) return;
	this._recipe = recipe;
	this.setupWindowForNewEntry();
	this.refresh();
};
//-----------------------------------------------------------------------------
// Refresh
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeDisplay.prototype.refresh = function() {
	this.contents.clear();
	this._iconSprite.hide();
	if(!this._recipe) {
		this.drawNoRecipeText();
		return;
	}
	const tempData = $cgmzTemp.getRecipeTempData(this._recipe._name);
	this._iconDisplacement = {yStart:0,yEnd:0,xStart:0,xEnd:0,isDisplaced:false};
	this._neededHeight = 0;
	if(tempData.image) {
		this.loadRecipeImage(tempData.image);
		return; // Draw the rest of the stuff after sprite is done loading
	}
	this.drawRecipeInfo();
};
//-----------------------------------------------------------------------------
// Draw text if there is no recipe selected
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeDisplay.prototype.drawNoRecipeText = function() {
	this.CGMZ_drawText(CGMZ.Crafting.NoRecipeText, 0, 0, 0, this.contents.width, 'center');
	this._neededHeight += this.padding * 2;
	this.checkForScroll();
};
//-----------------------------------------------------------------------------
// Load recipe custom image
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeDisplay.prototype.loadRecipeImage = function(img) {
	const imgData = CGMZ_Utils.getImageData(img, "img");
	this._iconSprite.bitmap = ImageManager.loadBitmap(imgData.folder, imgData.filename);
	this._iconSprite.bitmap.addLoadListener(this.drawRecipeInfo.bind(this));
};
//-----------------------------------------------------------------------------
// Display recipe custom image after load
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeDisplay.prototype.displayPictureBitmap = function() {
	this._iconSprite.y = this._neededHeight;
	this._iconSprite.x = 0;
	this._iconSprite.show();
	this._iconDisplacement.yStart = this._neededHeight;
	this._iconDisplacement.yEnd = this._neededHeight + this._iconSprite.height;
	this._iconDisplacement.xEnd = this._iconSprite.width + 4;
	this._iconDisplacement.isDisplaced = true;
};
//-----------------------------------------------------------------------------
// Adjust available width for icon displacement (if any)
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeDisplay.prototype.adjustWidthForIconDisplacement = function(width = this.contents.width) {
	if(!this._iconDisplacement.isDisplaced) return width;
	if(this._neededHeight >= this._iconDisplacement.yStart && this._neededHeight <= this._iconDisplacement.yEnd) {
		return width - this._iconDisplacement.xEnd;
	}
	return width;
};
//-----------------------------------------------------------------------------
// Adjust starting x for icon displacement (if any)
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeDisplay.prototype.adjustXForIconDisplacement = function(x = 0) {
	if(!this._iconDisplacement.isDisplaced) return x;
	if(this._neededHeight >= this._iconDisplacement.yStart && this._neededHeight <= this._iconDisplacement.yEnd) {
		return x + this._iconDisplacement.xEnd;
	}
	return x;
};
//-----------------------------------------------------------------------------
// Draw Recipe info after loading everything
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeDisplay.prototype.drawRecipeInfo = function() {
	const recipe = this._recipe;
	const tempRecipeData = $cgmzTemp.getRecipeTempData(recipe._name);
	for(const section of CGMZ.Crafting.DisplayWindowInfo) {
		switch(section) {
			case "Name": this.drawRecipeName(tempRecipeData.getName()); break;
			case "Image":
				if(tempRecipeData.image) {
					this.displayPictureBitmap();
				} else if(tempRecipeData.iconIndex > 0) {
					this.drawLargeIcon(tempRecipeData.iconIndex);
				}
				break;
			case "Times Crafted": this.drawRecipeStandardLine(CGMZ.Crafting.TimesCraftedText, recipe._timesCrafted); break;
			case "Success Rate": this.drawRecipeStandardLine(CGMZ.Crafting.SuccessRateText, recipe.calculateSuccessRate() + "%"); break;
			case "High Quality Rate": this.drawRecipeStandardLine(CGMZ.Crafting.HighQualityRateText, recipe.calculateHighQualityRate() + "%"); break;
			case "Quality": this.drawRecipeStandardLine(CGMZ.Crafting.QualityLabelText, tempRecipeData.quality); break;
			case "Subcategory": this.drawRecipeStandardLine(CGMZ.Crafting.SubcategoryLabelText, tempRecipeData.subcategory); break;
			case "Exp":
				if(Imported.CGMZ_Professions && $cgmz.getProfession(recipe._profession)) {
					this.drawRecipeStandardLine(CGMZ.Crafting.ExpText, tempRecipeData.experience);
				}
				break;
			case "Level Required":
				if(Imported.CGMZ_Professions && $cgmz.getProfession(recipe._profession)) {
					this.drawRecipeStandardLine(CGMZ.Crafting.LevelReqText, recipe._profession + " " + CGMZ.Crafting.LevelAbbrText + " " + tempRecipeData.levelRequirement);
				}
				break;
			case "Current Level":
				if(Imported.CGMZ_Professions && $cgmz.getProfession(recipe._profession)) {
					this.drawRecipeStandardLine(recipe._profession + " " + CGMZ.Crafting.CurrentLevelText, $cgmz.getProfession(recipe._profession).getBuffedLevel());
				}
				break;
			case "Description": this.drawRecipeDescription(recipe._description); break;
			case "Products":
				this.drawRecipeUniqueAmount(tempRecipeData.productUniqueAmount);
				this.drawRecipeItems(tempRecipeData.products, true, CGMZ.Crafting.ShowProductAmount);
				break;
			case "Tools": this.drawRecipeItems(tempRecipeData.tools, false, CGMZ.Crafting.ShowToolAmount); break;
			case "Ingredients": this.drawRecipeItems(tempRecipeData.ingredients, false, CGMZ.Crafting.ShowIngredientAmount); break;
			case "Fail Products": this.drawRecipeItems(tempRecipeData.failProducts, true, CGMZ.Crafting.ShowFailProductAmount); break;
			case "High Quality Products": this.drawRecipeItems(tempRecipeData.hqproducts, true, CGMZ.Crafting.ShowHighQualityProductAmount); break;
			case "Info Header": this._neededHeight += this.CGMZ_drawHeader(CGMZ.Crafting.InfoHeaderText, this._neededHeight, CGMZ.Crafting.HeaderColor1, CGMZ.Crafting.HeaderColor2); break;
			case "Description Header": this._neededHeight += this.CGMZ_drawHeader(CGMZ.Crafting.DescHeaderText, this._neededHeight, CGMZ.Crafting.HeaderColor1, CGMZ.Crafting.HeaderColor2); break;
			case "Product Header":
				if(tempRecipeData.products.length > 0) {
					this._neededHeight += this.CGMZ_drawHeader(CGMZ.Crafting.ProductHeaderText, this._neededHeight, CGMZ.Crafting.HeaderColor1, CGMZ.Crafting.HeaderColor2);
				}
				break;
			case "Tool Header":
				if(tempRecipeData.tools.length > 0) {
					this._neededHeight += this.CGMZ_drawHeader(CGMZ.Crafting.ToolHeaderText, this._neededHeight, CGMZ.Crafting.HeaderColor1, CGMZ.Crafting.HeaderColor2);
				}
				break;
			case "Ingredient Header":
				if(tempRecipeData.ingredients.length > 0) {
					this._neededHeight += this.CGMZ_drawHeader(CGMZ.Crafting.IngredientHeaderText, this._neededHeight, CGMZ.Crafting.HeaderColor1, CGMZ.Crafting.HeaderColor2);
				}
				break;
			case "Fail Product Header":
				if(tempRecipeData.failProducts.length > 0) {
					this._neededHeight += this.CGMZ_drawHeader(CGMZ.Crafting.FailProductHeaderText, this._neededHeight, CGMZ.Crafting.HeaderColor1, CGMZ.Crafting.HeaderColor2);
				}
				break;
			case "High Quality Product Header":
				if(tempRecipeData.hqproducts.length > 0) {
					this._neededHeight += this.CGMZ_drawHeader(CGMZ.Crafting.HighQualityProductHeaderText, this._neededHeight, CGMZ.Crafting.HeaderColor1, CGMZ.Crafting.HeaderColor2);
				}
				break;
			case "Blank Line": this._neededHeight += this.lineHeight(); break;
			case "Custom Space": this._neededHeight += CGMZ.Crafting.CustomSpace; break;
		}
	}
	this._neededHeight += this.padding * 2;
	this.checkForScroll();
};
//-----------------------------------------------------------------------------
// Draw Name of recipe
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeDisplay.prototype.drawRecipeName = function(name) {
	this.contents.fontBold = true;
	this._neededHeight += this.CGMZ_drawTextLine(name, 0, this._neededHeight, this.contents.width, 'center');
	this.contents.fontBold = false;
};
//-----------------------------------------------------------------------------
// Draw recipe description - returns height needed for description
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeDisplay.prototype.drawRecipeDescription = function(description) {
	this._neededHeight += this.CGMZ_drawText(description, 0, 0, this._neededHeight, this.contents.width, CGMZ.Crafting.DescriptionAlignment);
};
//-----------------------------------------------------------------------------
// Draws a standard line
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeDisplay.prototype.drawRecipeStandardLine = function(descriptor1, descriptor2) {
	const x = this.adjustXForIconDisplacement();
	const width = this.adjustWidthForIconDisplacement();
	const totalString = '\\c[' + CGMZ.Crafting.LabelColor + ']' + descriptor1 + '\\c[0]' + descriptor2;
	this._neededHeight += this.CGMZ_drawTextLine(totalString, x, this._neededHeight, width, 'left');
};
//-----------------------------------------------------------------------------
// Draw large icon
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeDisplay.prototype.drawLargeIcon = function(iconIndex) {
	const bitmap = this._iconBitmap;
	const pw = ImageManager.iconWidth;
	const ph = ImageManager.iconHeight;
	const sx = iconIndex % 16 * pw;
	const sy = Math.floor(iconIndex / 16) * ph;
	const dw = this._largeIconWidth;
	const dh = this._largeIconHeight;
	const x = 0;
	const y = this._neededHeight;
	this.contents.blt(bitmap, sx, sy, pw, ph, x, y, dw, dh);
	this._iconDisplacement.yStart = this._neededHeight;
	this._iconDisplacement.yEnd = this._neededHeight + this._largeIconHeight;
	this._iconDisplacement.xEnd = this._largeIconWidth + 4;
	this._iconDisplacement.isDisplaced = true;
};
//-----------------------------------------------------------------------------
// Draw regular icon
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeDisplay.prototype.drawIcon = function(iconIndex, x, y) {
	const bitmap = this._iconBitmap;
	const pw = ImageManager.iconWidth;
	const ph = ImageManager.iconHeight;
	const sx = iconIndex % 16 * pw;
	const sy = Math.floor(iconIndex / 16) * ph;
	this.contents.blt(bitmap, sx, sy, pw, ph, x, y);
};
//-----------------------------------------------------------------------------
// Draw Item Lists
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeDisplay.prototype.drawRecipeUniqueAmount = function(amount) {
	if(!amount || !CGMZ.Crafting.ShowUniqueProductAmount) return;
	const string = CGMZ.Crafting.UniqueText.replace(/%unique/gi, String(amount));
	this._neededHeight += this.CGMZ_drawTextLine(string, 0, this._neededHeight, this.contents.width, 'center');
};
//-----------------------------------------------------------------------------
// Draw Item Lists
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeDisplay.prototype.drawRecipeItems = function(itemArray, product, showAmount) {
	if(itemArray.length === 0) return;
	for(const itemObj of itemArray) {
		if(itemObj.Type === 'currency') {
			let currentSupply = $gameParty.gold();
			let unit = TextManager.currencyUnit;
			if(Imported.CGMZ_CurrencySystem && itemObj.ID) {
				const CScurrency = $cgmz.getCurrency(itemObj.ID);
				const CScurrencyTemp = $cgmzTemp.getCurrency(itemObj.ID);
				if(CScurrency && CScurrencyTemp) {
					currentSupply = CScurrency.amount();
					unit = `\\c[${CScurrencyTemp._color}]${CScurrencyTemp._unit}\\c[0]`;
				}
			}
			const amount = itemObj.Amount + unit;
			this.changePaintOpacity(product || itemObj.Amount <= currentSupply);
			const currentAmount = (showAmount) ? ` (${currentSupply})` : "";
			const string = amount + currentAmount;
			this._neededHeight += this.CGMZ_drawText(string, 0, 0, this._neededHeight, this.contents.width, 'center');
		} else if(itemObj.Type === 'generic') {
			const currentSupply = $cgmzTemp.calculateTotalNumItemsOfGenericType(itemObj.GenericCategory);
			this.changePaintOpacity(itemObj.Amount <= currentSupply);
			const amount = itemObj.Amount + CGMZ.Crafting.AmountXText;
			const currentAmount = (showAmount) ? ` (${currentSupply})` : "";
			const itemData = $cgmzTemp.getGenericCraftingItem(itemObj.GenericCategory);
			let string = amount;
			string += (itemData._iconIndex) ? '\\i[' + itemData._iconIndex + '] ' : "";
			string += (itemData._name + currentAmount);
			this._neededHeight += this.CGMZ_drawText(string, 0, 0, this._neededHeight, this.contents.width, 'center');
		} else {
			const item = CGMZ_Utils.lookupItem(itemObj.Type, itemObj.ID);
			if(item) {
				const currentSupply = $gameParty.numItems(item);
				const amount = itemObj.Amount + CGMZ.Crafting.AmountXText;
				const name = item.name;
				const iconIndex = item.iconIndex;
				this.changePaintOpacity(product || itemObj.Amount <= currentSupply);
				const currentAmount = (showAmount) ? ` (${currentSupply})` : "";
				const string = amount + "\\i[" + iconIndex + "]" + name + currentAmount;
				this._neededHeight += this.CGMZ_drawText(string, 0, 0, this._neededHeight, this.contents.width, 'center');
			}
		}
	}
	this.changePaintOpacity(true);
};
//=============================================================================
// CGMZ_Window_RecipeProgress
//-----------------------------------------------------------------------------
// Window displaying crafting progress
//=============================================================================
function CGMZ_Window_RecipeProgress() {
	this.initialize.apply(this, arguments);
}
CGMZ_Window_RecipeProgress.prototype = Object.create(Window_Base.prototype);
CGMZ_Window_RecipeProgress.prototype.constructor = CGMZ_Window_RecipeProgress;
//-----------------------------------------------------------------------------
// Initialize
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeProgress.prototype.initialize = function(rect, sceneKey) {
	Window_Base.prototype.initialize.call(this, rect);
	if(Imported.CGMZ_WindowBackgrounds && CGMZ.Crafting.ProgressWindowBackground) this.CGMZ_setWindowBackground(CGMZ.Crafting.ProgressWindowBackground);
	if(Imported.CGMZ_WindowSettings && CGMZ.Crafting.ProgressWindowSettings) this.CGMZ_setWindowSettings(CGMZ.Crafting.ProgressWindowSettings);
	this._sceneKey = sceneKey;
	this.resetWindow();
};
//-----------------------------------------------------------------------------
// Reset the window
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeProgress.prototype.resetWindow = function() {
	this._recipe = null;
	this._timerKey = "";
	this._timer = null;
	this._resultTimer = 0;
	this._craftResult = "";
	this._needsUpdate = false;
	this.refresh();
};
//-----------------------------------------------------------------------------
// Determine if progress window can craft
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeProgress.prototype.canCraft = function() {
	return !this._needsUpdate;
};
//-----------------------------------------------------------------------------
// Start the crafting process
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeProgress.prototype.startCraft = function(amount) {
	if(this._recipe) {
		const tempData = $cgmzTemp.getRecipeTempData(this._recipe._name);
		AudioManager.playSe(tempData.craftSE);
		this._recipe.startCraft();
		const ext = $cgmzTemp.makeCraftTimerInfo(this._recipe._name, this._timerKey, this._sceneKey, amount);
		const timer = new CGMZ_Timer(
			ext.timeNeeded,
			this._timerKey,
			"onCraftTimerFinish",
			ext
		);
		$cgmz.requestNewTimer(timer);
		this._needsUpdate = true;
		this._timer = $cgmz.getTimerById(this._timerKey);
	}
};
//-----------------------------------------------------------------------------
// Do an instant craft
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeProgress.prototype.doInstantCraft = function(amount) {
	if(this._recipe) {
		let success = true;
		for(let i = 0; i < amount; i++) {
			const ext = $cgmzTemp.makeCraftTimerInfo(this._recipe._name, this._timerKey, this._sceneKey, amount);
			this._recipe.doCraft(ext.success, ext.hq);
			if(!ext.success) success = false;
		}
		const recipeTemp = $cgmzTemp.getRecipeTempData(this._recipe._name);
		if(recipeTemp) {
			const se = success ? recipeTemp.successSE : recipeTemp.failSE;
			AudioManager.playSe(se);
		}
		if(Imported.CGMZ_Rumble) {
			success ? $cgmzTemp.startRumble(CGMZ.Crafting.SuccessRumble) : $cgmzTemp.startRumble(CGMZ.Crafting.FailRumble)
		}
	}
};
//-----------------------------------------------------------------------------
// Update the crafting process
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeProgress.prototype.update = function() {
	Window_Base.prototype.update.call(this);
	if(this._needsUpdate) this.refresh();
};
//-----------------------------------------------------------------------------
// Set the recipe to be displayed
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeProgress.prototype.setItem = function(recipe) {
	this._recipe = recipe;
	if(recipe) {
		this._timerKey = $cgmzTemp.createCraftingTimerKey(recipe._name, this._sceneKey);
		this._timer = $cgmz.getTimerById(this._timerKey);
		if(this._timer) {
			const info = this._timer._args;
			this._needsUpdate = true;
			this._resultTimer = 0;
			this._craftResult = "";
		}
	} else {
		this._timer = null;
		this._timerKey = "";
		this._resultTimer = 0;
		this._craftResult = "";
		this._needsUpdate = false;
	}
	this.refresh();
};
//-----------------------------------------------------------------------------
// Reset for a new recipe
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeProgress.prototype.resetForNextCraft = function() {
	if(this._recipe) {
		this._timerKey = $cgmzTemp.createCraftingTimerKey(this._recipe._name, this._sceneKey);
		this._timer = $cgmz.getTimerById(this._timerKey);
		if(this._timer) {
			const info = this._timer._args;
			this._resultTimer = 0;
			this._craftResult = "";
			this._needsUpdate = true;
		}
	}
	this.refresh();
};
//-----------------------------------------------------------------------------
// Reset for a new recipe
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeProgress.prototype.startDisplayCraftResult = function(success, hq) {
	this._resultTimer = 0;
	if(success && hq) {
		this._craftResult = `\\c[${CGMZ.Crafting.HighQualityColor}]${CGMZ.Crafting.HighQualityText}\\c[0]`;
	} else if(success) {
		this._craftResult = `\\c[${CGMZ.Crafting.SuccessColor}]${CGMZ.Crafting.SuccessText}\\c[0]`;
	} else {
		this._craftResult = `\\c[${CGMZ.Crafting.FailureColor}]${CGMZ.Crafting.FailureText}\\c[0]`;
	}
	this._needsUpdate = true;
};
//-----------------------------------------------------------------------------
// Refresh the window contents
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeProgress.prototype.refresh = function() {
	this.contents.clear();
	if(this._timer) {
		const currentlyCrafting = true;
		this.drawCraftProgress(currentlyCrafting);
	} else {
		this.drawCraftResult();
	}
};
//-----------------------------------------------------------------------------
// Draw the craft progress
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeProgress.prototype.drawCraftProgress = function(currentlyCrafting = false) {
	this.CGMZ_drawTextLine(CGMZ.Crafting.ProgressText, 0, 0, this.contents.width, 'left');
	const color1 = ColorManager.textColor(CGMZ.Crafting.ProgressColor1);
	const color2 = ColorManager.textColor(CGMZ.Crafting.ProgressColor2);
	const x = this.CGMZ_textSizeEx(CGMZ.Crafting.ProgressText).width;
	const rect = new Rectangle(x, 0, this.contents.width - x, this.lineHeight());
	const rate = this.makeProgressRate(currentlyCrafting);
	this.CGMZ_drawGauge(rect, rate, color1, color2);
	if(currentlyCrafting) {
		if(CGMZ.Crafting.ShowProgressPercentage) {
			const string = (CGMZ.Crafting.ProgressDecimals) ? (rate*100.0).toFixed(CGMZ.Crafting.ProgressDecimals) : Math.floor(rate*100.0);
			this.CGMZ_drawTextLine(`${string}${CGMZ.Crafting.PercentText}`, x, 0, this.contents.width - x, 'center');
		} else if(CGMZ.Crafting.ShowProgressTimeRemaining) {
			const timeRemaining = this.makeTimeRemaining();
			this.CGMZ_drawTextLine(timeRemaining, x, 0, this.contents.width - x, 'center');
		}
		const info = this._timer._args;
		const startFrame = info.currentTime;
		const currentFrame = Graphics.frameCount - startFrame;
		const endFrame = info.timeNeeded;
		if(currentFrame >= endFrame) {
			this._timer = null;
			this._timerKey = "";
		}
	}
};
//-----------------------------------------------------------------------------
// Calculate the rate of the progress bar (returns 0 if not currently crafting)
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeProgress.prototype.makeProgressRate = function(currentlyCrafting) {
	if(!currentlyCrafting) return 0.0;
	const info = this._timer._args;
	const startFrame = info.currentTime;
	const currentFrame = Graphics.frameCount - startFrame;
	const endFrame = info.fullRecipeCraftTime;
	return currentFrame / endFrame;
};
//-----------------------------------------------------------------------------
// Calculate the time remaining, return string for display
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeProgress.prototype.makeTimeRemaining = function() {
	const info = this._timer._args;
	const currentFrame = Graphics.frameCount;
	const endFrame = info.fullRecipeCraftTime;
	const time = endFrame - currentFrame;
	const seconds = Math.ceil(time / 60);
	if(seconds < 60) return seconds + CGMZ.Crafting.SecText;
	const minutes = Math.ceil(seconds / 60);
	if(minutes < 60) return minutes + CGMZ.Crafting.MinText;
	const hr = Math.ceil(minutes / 60);
	return hr + CGMZ.Crafting.HrText;
};
//-----------------------------------------------------------------------------
// Draw craft result
//-----------------------------------------------------------------------------
CGMZ_Window_RecipeProgress.prototype.drawCraftResult = function() {
	if(this._craftResult) {
		this.CGMZ_drawTextLine(this._craftResult, 0, 0, this.contents.width, 'center');
		this._resultTimer++;
		if(this._resultTimer > 60) {
			this._resultTimer = 0;
			this._craftResult = "";
		}
	} else {
		this._needsUpdate = false;
		const currentlyCrafting = false;
		this.drawCraftProgress(currentlyCrafting);
	}
};
//=============================================================================
// CGMZ_Window_CraftConfirmation
//-----------------------------------------------------------------------------
// Handle CGMZ craft confirmation / cancel
//=============================================================================
function CGMZ_Window_CraftConfirmation() {
	this.initialize(...arguments);
}
CGMZ_Window_CraftConfirmation.prototype = Object.create(Window_Command.prototype);
CGMZ_Window_CraftConfirmation.prototype.constructor = CGMZ_Window_CraftConfirmation;
//-----------------------------------------------------------------------------
// Initialize
//-----------------------------------------------------------------------------
CGMZ_Window_CraftConfirmation.prototype.initialize = function(rect) {
	Window_Command.prototype.initialize.call(this, rect);
	if(Imported.CGMZ_WindowBackgrounds && CGMZ.Crafting.ConfirmWindowBackground) this.CGMZ_setWindowBackground(CGMZ.Crafting.ConfirmWindowBackground);
	if(Imported.CGMZ_WindowSettings && CGMZ.Crafting.ConfirmWindowSettings) this.CGMZ_setWindowSettings(CGMZ.Crafting.ConfirmWindowSettings);
	this._waitTime = 10;
	this.deactivate();
	this.hide();
};
//-----------------------------------------------------------------------------
// Add commands
//-----------------------------------------------------------------------------
CGMZ_Window_CraftConfirmation.prototype.makeCommandList = function() {
	this.addCommand(CGMZ.Crafting.CraftConfirmText, "craft", true);
	this.addCommand(CGMZ.Crafting.CraftCancelText, "cancel", true);
};
//-----------------------------------------------------------------------------
// Add a wait period before being active after show
//-----------------------------------------------------------------------------
CGMZ_Window_CraftConfirmation.prototype.show = function() {
	Window_Command.prototype.show.call(this);
	this._waitTime = 5;
};
//-----------------------------------------------------------------------------
// Only allow OK trigger after wait time is over to prevent instant selection
//-----------------------------------------------------------------------------
CGMZ_Window_CraftConfirmation.prototype.isOkTriggered = function() {
	const triggered = Window_Command.prototype.isOkTriggered.call(this);
	return triggered && this._waitTime <= 0;
};
//-----------------------------------------------------------------------------
// Determine if OK is actually triggered
//-----------------------------------------------------------------------------
CGMZ_Window_CraftConfirmation.prototype.update = function() {
	Window_Command.prototype.update.call(this);
	this._waitTime--;
	if(this._waitTime < 0) this._waitTime = 0;
};
//=============================================================================
// CGMZ_Window_Multicraft
//-----------------------------------------------------------------------------
// Handle multicraft select
//=============================================================================
function CGMZ_Window_Multicraft() {
	this.initialize(...arguments);
}
CGMZ_Window_Multicraft.prototype = Object.create(Window_Selectable.prototype);
CGMZ_Window_Multicraft.prototype.constructor = CGMZ_Window_Multicraft;
//-----------------------------------------------------------------------------
// Initialize
//-----------------------------------------------------------------------------
CGMZ_Window_Multicraft.prototype.initialize = function(rect) {
	Window_Selectable.prototype.initialize.call(this, rect);
	if(Imported.CGMZ_WindowBackgrounds && CGMZ.Crafting.MulticraftWindowBackground) this.CGMZ_setWindowBackground(CGMZ.Crafting.MulticraftWindowBackground);
	if(Imported.CGMZ_WindowSettings && CGMZ.Crafting.MulticraftWindowSettings) this.CGMZ_setWindowSettings(CGMZ.Crafting.MulticraftWindowSettings);
	this._item = null;
	this._max = 1;
	this._number = 1;
	this.createButtons();
	this.placeButtons();
	this.deactivate();
	this.hide();
	this._canRepeat = false;
};
//-----------------------------------------------------------------------------
// Disable scroll
//-----------------------------------------------------------------------------
CGMZ_Window_Multicraft.prototype.isScrollEnabled = function() {
	return false;
};
//-----------------------------------------------------------------------------
// Get the number to craft
//-----------------------------------------------------------------------------
CGMZ_Window_Multicraft.prototype.number = function() {
	return this._number;
};
//-----------------------------------------------------------------------------
// Setup the window with max amount to craft
//-----------------------------------------------------------------------------
CGMZ_Window_Multicraft.prototype.setup = function(max) {
	this._max = max;
	this._number = 1;
	this.refresh();
};
//-----------------------------------------------------------------------------
// Create the buttons
//-----------------------------------------------------------------------------
CGMZ_Window_Multicraft.prototype.createButtons = function() {
	this._buttons = [];
	if (ConfigManager.touchUI) {
		for (const type of ["down2", "down", "up", "up2", "ok"]) {
			const button = new Sprite_Button(type);
			this._buttons.push(button);
			this.addInnerChild(button);
		}
		this._buttons[0].setClickHandler(this.onButtonDown2.bind(this));
		this._buttons[1].setClickHandler(this.onButtonDown.bind(this));
		this._buttons[2].setClickHandler(this.onButtonUp.bind(this));
		this._buttons[3].setClickHandler(this.onButtonUp2.bind(this));
		this._buttons[4].setClickHandler(this.onButtonOk.bind(this));
	}
};
//-----------------------------------------------------------------------------
// Place the buttons
//-----------------------------------------------------------------------------
CGMZ_Window_Multicraft.prototype.placeButtons = function() {
	const sp = this.buttonSpacing();
	const totalWidth = this.totalButtonWidth();
	let x = (this.innerWidth - totalWidth) / 2;
	for (const button of this._buttons) {
		button.x = x;
		button.y = this.buttonY();
		x += button.width + sp;
	}
};
//-----------------------------------------------------------------------------
// Get total button width
//-----------------------------------------------------------------------------
CGMZ_Window_Multicraft.prototype.totalButtonWidth = function() {
	const sp = this.buttonSpacing();
	return this._buttons.reduce((r, button) => r + button.width + sp, -sp);
};
//-----------------------------------------------------------------------------
// Get button spacing
//-----------------------------------------------------------------------------
CGMZ_Window_Multicraft.prototype.buttonSpacing = function() {
	return 8;
};
//-----------------------------------------------------------------------------
// Refresh the window
//-----------------------------------------------------------------------------
CGMZ_Window_Multicraft.prototype.refresh = function() {
	Window_Selectable.prototype.refresh.call(this);
	this.drawQuantityText();
	this.drawNumber();
};
//-----------------------------------------------------------------------------
// Draw Quantity Text
//-----------------------------------------------------------------------------
CGMZ_Window_Multicraft.prototype.drawQuantityText = function() {
	this.CGMZ_drawTextLine(CGMZ.Crafting.CraftQuantityText, 0, 0, this.contents.width, 'left');
};
//-----------------------------------------------------------------------------
// Draw current quantity
//-----------------------------------------------------------------------------
CGMZ_Window_Multicraft.prototype.drawNumber = function() {
	this.CGMZ_drawTextLine("" + this._number, 0, 0, this.contents.width, "right");
};
//-----------------------------------------------------------------------------
// Get button y coordinate
//-----------------------------------------------------------------------------
CGMZ_Window_Multicraft.prototype.buttonY = function() {
	return this.lineHeight();
};
//-----------------------------------------------------------------------------
// Update window
//-----------------------------------------------------------------------------
CGMZ_Window_Multicraft.prototype.update = function() {
	Window_Selectable.prototype.update.call(this);
	this.processNumberChange();
};
//-----------------------------------------------------------------------------
// Do not play ok sound
//-----------------------------------------------------------------------------
CGMZ_Window_Multicraft.prototype.playOkSound = function() {
};
//-----------------------------------------------------------------------------
// Process a number change
//-----------------------------------------------------------------------------
CGMZ_Window_Multicraft.prototype.processNumberChange = function() {
	if (this.isOpenAndActive()) {
		if(Input.isRepeated("right")) {
			this.changeNumber(1);
		}
		if(Input.isRepeated("left")) {
			this.changeNumber(-1);
		}
		if(Input.isRepeated("up")) {
			this.changeNumber(10);
		}
		if(Input.isRepeated("down")) {
			this.changeNumber(-10);
		}
	}
};
//-----------------------------------------------------------------------------
// Change the number
//-----------------------------------------------------------------------------
CGMZ_Window_Multicraft.prototype.changeNumber = function(amount) {
	const lastNumber = this._number;
	this._number = (this._number + amount).clamp(1, this._max);
	if(this._number !== lastNumber) {
		this.playCursorSound();
		this.refresh();
	}
};
//-----------------------------------------------------------------------------
// Disable touch
//-----------------------------------------------------------------------------
CGMZ_Window_Multicraft.prototype.isTouchOkEnabled = function() {
	return false;
};
//-----------------------------------------------------------------------------
// Processing for button up
//-----------------------------------------------------------------------------
CGMZ_Window_Multicraft.prototype.onButtonUp = function() {
	this.changeNumber(1);
};
//-----------------------------------------------------------------------------
// Processing for button up 2
//-----------------------------------------------------------------------------
CGMZ_Window_Multicraft.prototype.onButtonUp2 = function() {
	this.changeNumber(10);
};
//-----------------------------------------------------------------------------
// Processing for button down
//-----------------------------------------------------------------------------
CGMZ_Window_Multicraft.prototype.onButtonDown = function() {
	this.changeNumber(-1);
};
//-----------------------------------------------------------------------------
// Processing for button down 2
//-----------------------------------------------------------------------------
CGMZ_Window_Multicraft.prototype.onButtonDown2 = function() {
	this.changeNumber(-10);
};
//-----------------------------------------------------------------------------
// Processing for button ok
//-----------------------------------------------------------------------------
CGMZ_Window_Multicraft.prototype.onButtonOk = function() {
	this.processOk();
};
//=============================================================================
// CGMZ_Window_FilterType
//-----------------------------------------------------------------------------
// Selectable window for choosing a filter type
//=============================================================================
function CGMZ_Window_FilterType(rect) {
	this.initialize.apply(this, arguments);
}
CGMZ_Window_FilterType.prototype = Object.create(Window_Selectable.prototype);
CGMZ_Window_FilterType.prototype.constructor = CGMZ_Window_FilterType;
//-----------------------------------------------------------------------------
// Initialize
//-----------------------------------------------------------------------------
CGMZ_Window_FilterType.prototype.initialize = function(rect) {
	Window_Selectable.prototype.initialize.call(this, rect);
	if(Imported.CGMZ_WindowBackgrounds && CGMZ.Crafting.FilterTypeWindowBackground) this.CGMZ_setWindowBackground(CGMZ.Crafting.FilterTypeWindowBackground);
	if(Imported.CGMZ_WindowSettings && CGMZ.Crafting.FilterTypeWindowSettings) this.CGMZ_setWindowSettings(CGMZ.Crafting.FilterTypeWindowSettings);
	this._data = [];
	this.refresh();
	this.deactivate();
	this.hide();
};
//-----------------------------------------------------------------------------
// Max items
//-----------------------------------------------------------------------------
CGMZ_Window_FilterType.prototype.maxItems = function() {
	return this._data ? this._data.length : 1;
};
//-----------------------------------------------------------------------------
// Current item
//-----------------------------------------------------------------------------
CGMZ_Window_FilterType.prototype.item = function() {
	return this._data[this.index()];
};
//-----------------------------------------------------------------------------
// Refresh
//-----------------------------------------------------------------------------
CGMZ_Window_FilterType.prototype.refresh = function(filters) {
	this.makeItemList(filters);
	Window_Selectable.prototype.refresh.call(this);
};
//-----------------------------------------------------------------------------
// Make item list
//-----------------------------------------------------------------------------
CGMZ_Window_FilterType.prototype.makeItemList = function(filters) {
	this._data = [];
	this._data.push("remove");
	if(filters?.category?.length) this._data.push("category");
	if(filters?.subcategory?.length) this._data.push("subcategory");
	if(filters?.quality?.length) this._data.push("quality");
};
//-----------------------------------------------------------------------------
// Draw item in list
//-----------------------------------------------------------------------------
CGMZ_Window_FilterType.prototype.drawItem = function(index) {
	const table = {
		remove: CGMZ.Crafting.RemoveFilterText,
		category: CGMZ.Crafting.CategoryFilterText,
		subcategory: CGMZ.Crafting.SubcategoryFilterText,
		quality: CGMZ.Crafting.QualityFilterText
	}
	const item = table[this._data[index]];
	const rect = this.itemRectWithPadding(index);
	this.CGMZ_drawTextLine(item, rect.x, rect.y, rect.width, 'center');
};
//=============================================================================
// CGMZ_Window_FilterOption
//-----------------------------------------------------------------------------
// Selectable window for choosing a filter option
//=============================================================================
function CGMZ_Window_FilterOption(rect) {
	this.initialize.apply(this, arguments);
}
CGMZ_Window_FilterOption.prototype = Object.create(Window_Selectable.prototype);
CGMZ_Window_FilterOption.prototype.constructor = CGMZ_Window_FilterOption;
//-----------------------------------------------------------------------------
// Initialize
//-----------------------------------------------------------------------------
CGMZ_Window_FilterOption.prototype.initialize = function(rect) {
	Window_Selectable.prototype.initialize.call(this, rect);
	if(Imported.CGMZ_WindowBackgrounds && CGMZ.Crafting.FilterOptionWindowBackground) this.CGMZ_setWindowBackground(CGMZ.Crafting.FilterOptionWindowBackground);
	if(Imported.CGMZ_WindowSettings && CGMZ.Crafting.FilterOptionWindowSettings) this.CGMZ_setWindowSettings(CGMZ.Crafting.FilterOptionWindowSettings);
	this._data = [];
	this._filterType = null;
	this.refresh();
	this.deactivate();
	this.hide();
};
//-----------------------------------------------------------------------------
// Max items
//-----------------------------------------------------------------------------
CGMZ_Window_FilterOption.prototype.maxItems = function() {
	return this._data ? this._data.length : 1;
};
//-----------------------------------------------------------------------------
// Current item
//-----------------------------------------------------------------------------
CGMZ_Window_FilterOption.prototype.item = function() {
	return this._data[this.index()];
};
//-----------------------------------------------------------------------------
// Refresh
//-----------------------------------------------------------------------------
CGMZ_Window_FilterOption.prototype.setFilterType = function(type) {
	this._filterType = type;
};
//-----------------------------------------------------------------------------
// Refresh
//-----------------------------------------------------------------------------
CGMZ_Window_FilterOption.prototype.refresh = function(filters) {
	this.makeItemList(filters);
	Window_Selectable.prototype.refresh.call(this);
};
//-----------------------------------------------------------------------------
// Make item list
//-----------------------------------------------------------------------------
CGMZ_Window_FilterOption.prototype.makeItemList = function(filters) {
	this._data = [];
	if(!this._filterType) return;
	if(!filters[this._filterType]) return;
	this._data = JSON.parse(JSON.stringify(filters[this._filterType]));
};
//-----------------------------------------------------------------------------
// Draw item in list
//-----------------------------------------------------------------------------
CGMZ_Window_FilterOption.prototype.drawItem = function(index) {
	const item = this._data[index];
	const rect = this.itemRectWithPadding(index);
	this.CGMZ_drawTextLine(item, rect.x, rect.y, rect.width, 'center');
};
//=============================================================================
// CGMZ_Window_CraftingCollectedItems
//-----------------------------------------------------------------------------
// Window displaying recipe information
//=============================================================================
function CGMZ_Window_CraftingCollectedItems() {
	this.initialize.apply(this, arguments);
}
CGMZ_Window_CraftingCollectedItems.prototype = Object.create(CGMZ_Window_Scrollable.prototype);
CGMZ_Window_CraftingCollectedItems.prototype.constructor = CGMZ_Window_CraftingCollectedItems;
//-----------------------------------------------------------------------------
// Initialize
//-----------------------------------------------------------------------------
CGMZ_Window_CraftingCollectedItems.prototype.initialize = function(rect, sceneKey) {
	const heightMultiplier = 100; // maximum of 100 windows tall of data to scroll
	CGMZ_Window_Scrollable.prototype.initialize.call(this, rect, heightMultiplier, CGMZ.Crafting.ScrollWait, CGMZ.Crafting.ScrollSpeed, CGMZ.Crafting.AutoScroll, CGMZ.Crafting.ScrollDeceleration);
	if(Imported.CGMZ_WindowBackgrounds && CGMZ.Crafting.CollectedItemsWindowBackground) this.CGMZ_setWindowBackground(CGMZ.Crafting.CollectedItemsWindowBackground);
	if(Imported.CGMZ_WindowSettings && CGMZ.Crafting.CollectedItemsWindowSettings) this.CGMZ_setWindowSettings(CGMZ.Crafting.CollectedItemsWindowSettings);
	this._sceneKey = sceneKey;
	this.deactivate();
	this.hide();
	this.requestRefresh();
};
//-----------------------------------------------------------------------------
// Refresh
//-----------------------------------------------------------------------------
CGMZ_Window_CraftingCollectedItems.prototype.refresh = function() {
	this.setupWindowForNewEntry();
	this.contents.clear();
	const collectedItems = JSON.parse(JSON.stringify($cgmz.getCraftedItemsForScene(this._sceneKey)));
	this._neededHeight += this.CGMZ_drawTextLine(CGMZ.Crafting.CollectedItemsText, 0, 0, this.contents.width, 'center');
	for(const item of collectedItems) {
		switch(item.Type) {
			case 'item': this.drawTypeItem(item); break;
			case 'currency': this.drawTypeCurrency(item); break;
			case 'weapon': this.drawTypeWeapon(item); break;
			case 'armor': this.drawTypeArmor(item); break;
		}
	}
};
//-----------------------------------------------------------------------------
// Draw a item type item
//-----------------------------------------------------------------------------
CGMZ_Window_CraftingCollectedItems.prototype.drawTypeItem = function(itemObj) {
	const item = $dataItems[itemObj.ID];
	const string = CGMZ.Crafting.CollectItemItemText.replace('%name', item.name).replace('%icon', (item.iconIndex > 0) ? `\\i[${item.iconIndex}]` : '').replace('%amount', itemObj.Amount);
	this._neededHeight += this.CGMZ_drawTextLine(string, 0, this._neededHeight, this.contents.width, 'left');
};
//-----------------------------------------------------------------------------
// Draw a weapon type item
//-----------------------------------------------------------------------------
CGMZ_Window_CraftingCollectedItems.prototype.drawTypeWeapon = function(itemObj) {
	const weapon = $dataWeapons[itemObj.ID];
	const string = CGMZ.Crafting.CollectItemWeaponText.replace('%name', weapon.name).replace('%icon', (weapon.iconIndex > 0) ? `\\i[${weapon.iconIndex}]` : '').replace('%amount', itemObj.Amount);
	this._neededHeight += this.CGMZ_drawTextLine(string, 0, this._neededHeight, this.contents.width, 'left');
};
//-----------------------------------------------------------------------------
// Draw an armor type item
//-----------------------------------------------------------------------------
CGMZ_Window_CraftingCollectedItems.prototype.drawTypeArmor = function(itemObj) {
	const armor = $dataArmors[itemObj.ID];
	const string = CGMZ.Crafting.CollectItemArmorText.replace('%name', armor.name).replace('%icon', (armor.iconIndex > 0) ? `\\i[${armor.iconIndex}]` : '').replace('%amount', itemObj.Amount);
	this._neededHeight += this.CGMZ_drawTextLine(string, 0, this._neededHeight, this.contents.width, 'left');
};
//-----------------------------------------------------------------------------
// Draw a currency type item
//-----------------------------------------------------------------------------
CGMZ_Window_CraftingCollectedItems.prototype.drawTypeCurrency = function(itemObj) {
	if(Imported.CGMZ_CurrencySystem) {
		const currency = $cgmzTemp.getCurrency(itemObj.ID) || $cgmzTemp.getCurrency('default');
		const string = CGMZ.Crafting.CollectItemCurrencyText.replace('%icon', (currency._iconIndex > 0) ? `\\i[${currency._iconIndex}]` : '')
			.replace('%color', `\\c[${currency._color}]`).replace('%unit', currency._unit).replace('%name', currency._name).replace('%amount', CGMZ_Utils.numberSplit(itemObj.Amount));
		this._neededHeight += this.CGMZ_drawTextLine(string, 0, this._neededHeight, this.contents.width, 'left');
	} else {
		const string = CGMZ.Crafting.CollectItemCurrencyText.replace('%icon', '').replace('%color', '').replace('%unit', TextManager.currencyUnit).replace('%name', '').replace('%amount', CGMZ_Utils.numberSplit(itemObj.Amount));
		this._neededHeight += this.CGMZ_drawTextLine(string, 0, this._neededHeight, this.contents.width, 'left');
	}
};
//=============================================================================
// CGMZ_Window_CraftingCollectItemsCommand
//-----------------------------------------------------------------------------
// Window for confirming the player sees the items they collected
//=============================================================================
function CGMZ_Window_CraftingCollectItemsCommand(rect) {
	this.initialize.apply(this, arguments);
}
CGMZ_Window_CraftingCollectItemsCommand.prototype = Object.create(Window_Selectable.prototype);
CGMZ_Window_CraftingCollectItemsCommand.prototype.constructor = CGMZ_Window_CraftingCollectItemsCommand;
//-----------------------------------------------------------------------------
// Initialize
//-----------------------------------------------------------------------------
CGMZ_Window_CraftingCollectItemsCommand.prototype.initialize = function(rect) {
	Window_Selectable.prototype.initialize.call(this, rect);
	if(Imported.CGMZ_WindowBackgrounds && CGMZ.Crafting.CollectItemsConfirmWindowBackground) this.CGMZ_setWindowBackground(CGMZ.Crafting.CollectItemsConfirmWindowBackground);
	if(Imported.CGMZ_WindowSettings && CGMZ.Crafting.CollectItemsConfirmWindowSettings) this.CGMZ_setWindowSettings(CGMZ.Crafting.CollectItemsConfirmWindowSettings);
	this.refresh();
	this.deactivate();
	this.hide();
};
//-----------------------------------------------------------------------------
// Max items
//-----------------------------------------------------------------------------
CGMZ_Window_CraftingCollectItemsCommand.prototype.maxItems = function() {
	return 1;
};
//-----------------------------------------------------------------------------
// Draw item in list
//-----------------------------------------------------------------------------
CGMZ_Window_CraftingCollectItemsCommand.prototype.drawItem = function(index) {
	const rect = this.itemRectWithPadding(index);
	this.CGMZ_drawTextLine(CGMZ.Crafting.CollectItemConfirmText, rect.x, rect.y, rect.width, 'center');
};
//=============================================================================
// Game_Battler
//-----------------------------------------------------------------------------
// Use recipe item processing
//=============================================================================
//-----------------------------------------------------------------------------
// Item use may cause learning of a recipe
//-----------------------------------------------------------------------------
const alias_CGMZ_Crafting_useItem = Game_Battler.prototype.useItem;
Game_Battler.prototype.useItem = function(item) {
	alias_CGMZ_Crafting_useItem.call(this, item);
	if(DataManager.isItem(item)) $cgmz.checkItemForRecipe(item);
};
//=============================================================================
// Game_Party
//-----------------------------------------------------------------------------
// Use recipe item processing
//=============================================================================
//-----------------------------------------------------------------------------
// Item use may cause learning of a recipe
//-----------------------------------------------------------------------------
const alias_CGMZ_Crafting_GameParty_gainItem = Game_Party.prototype.gainItem;
Game_Party.prototype.gainItem = function(item, amount, includeEquip) {
	alias_CGMZ_Crafting_GameParty_gainItem.apply(this, arguments);
	if(CGMZ.Crafting.AutomaticLearn) $cgmz.checkCraftingRecipesForLearn();
};
//-----------------------------------------------------------------------------
// Gold changes may cause learning of recipe
//-----------------------------------------------------------------------------
const alias_CGMZ_Crafting_GameParty_gainGold = Game_Party.prototype.gainGold;
Game_Party.prototype.gainGold = function(amount) {
	alias_CGMZ_Crafting_GameParty_gainGold.apply(this, arguments);
	if(CGMZ.Crafting.AutomaticLearn) $cgmz.checkCraftingRecipesForLearn();
};