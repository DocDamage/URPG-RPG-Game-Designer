//=============================================================================
// VisuStella MZ - Events & Movement Core
// VisuMZ_1_EventsMoveCore.js
//=============================================================================

var Imported = Imported || {};
Imported.VisuMZ_1_EventsMoveCore = true;

var VisuMZ = VisuMZ || {};
VisuMZ.EventsMoveCore = VisuMZ.EventsMoveCore || {};
VisuMZ.EventsMoveCore.version = 1.64;

//=============================================================================
 /*:
 * @target MZ
 * @plugindesc [RPG Maker MZ] [Tier 1] [Version 1.64] [EventsMoveCore]
 * @author VisuStella
 * @url http://www.yanfly.moe/wiki/Events_and_Movement_Core_VisuStella_MZ
 * @orderAfter VisuMZ_0_CoreEngine
 *
 * @help
 * ============================================================================
 * Introduction
 * ============================================================================
 *
 * The Events & Movement Core plugin adds a lot of new functionality in terms
 * of event flexibility and movement options to RPG Maker MZ. These range from
 * adding in old capabilities from previous iterations of RPG Maker to more
 * mainstream techniques found in other game engines. Movement options are also
 * expanded to support 8-directional movement as well as sprite sheets provided
 * that the VisuStella 8 format is used.
 *
 * Features include all (but not limited to) the following:
 * 
 * * Event commands expanded upon to include old and new functions.
 * * Event templates for Copying Events, Morphing Events, and Spawning Events.
 * * 8-directional movement option available and sprite sheet support.
 * * Aesthetics for tilting the sprite when dashing and having shadows below.
 * * Pathfinding support for event movement through custom Move Route commands.
 * * Advanced switches and variable support to run code automatically.
 * * Turn regular Switches and Variables into Self Switches and Self Variables.
 * * Put labels and icons over events.
 * * Allow numerous ways to trigger events, through clicking, proximity, or by
 *   usage of Regions.
 * * Change the hitbox sizes of events to larger in any direction.
 * * Synchronize event movement options to move when player/other events move.
 * * The ability for the player to turn in place.
 *
 * ============================================================================
 * Requirements
 * ============================================================================
 *
 * This plugin is made for RPG Maker MZ. This will not work in other iterations
 * of RPG Maker.
 *
 * ------ Tier 1 ------
 *
 * This plugin is a Tier 1 plugin. Place it under other plugins of lower tier
 * value on your Plugin Manager list (ie: 0, 1, 2, 3, 4, 5). This is to ensure
 * that your plugins will have the best compatibility with the rest of the
 * VisuStella MZ library.
 *
 * ============================================================================
 * Features: Advanced Switches and Variables
 * ============================================================================
 *
 * Switches and variables can now run JavaScript code and return values
 * instantly. While at first glance, this may seem no different from using
 * the Control Variables event command's Script option, this can be used to
 * instantly set up Switch and/or Variable conditions for Parallel Common
 * Events, Event Page Conditions, Enemy Skill Conditions, and Troop Page
 * Conditions instantly without needing to make an event command to do so.
 *
 * ---
 *
 * <JS> code </JS>
 * - Used for: Switch and Variable names
 * - Replace 'code' with JavaScript code on what value to return.
 *
 * ---
 *
 * NOTE: Tagged Switches/Variables are mutually exclusive from one another.
 * You cannot tag them with <JS>, <Self>, <Map>, or <Global> simultaneously.
 *
 * ============================================================================
 * Features: Self Switches and Variables
 * ============================================================================
 *
 * RPG Maker MZ by default has 4 Self Switches: A, B, C, D. For some types of
 * games, this isn't enough. This plugin gives you the ability convert regular
 * Switches into Self Switches so you could have more.
 *
 * Self Variables also do not exist in RPG Maker MZ by default. Just like with
 * Switches, you can turn regular Variables into Self Variables.
 *
 * ---
 *
 * <Self>
 * - Used for: Switch and Variable names
 * - Converts the Switch/Variable into a Self Switch/Variable.
 *
 * ---
 *
 * After, just use them like you would for normal Switches and Variables in an
 * event's page conditions. If the <Self> tag is present inside the Switch or
 * Variable's name, then it will use data unique to only that event.
 *
 * NOTE: Tagged Switches/Variables are mutually exclusive from one another.
 * You cannot tag them with <JS>, <Self>, <Map>, or <Global> simultaneously.
 * 
 * ---
 * 
 * If you need to use a script call to get a Self Switch or Self Variable's
 * value, you can use the following script calls.
 * 
 *   ---
 * 
 *   Get Self Switch Values:
 * 
 *   getSelfSwitchValue(mapID, eventID, switchID)
 *   - Replace 'mapID' with the map ID the target event is located on.
 *   - Replace 'eventID' with the ID of the target event.
 *   - Replace 'switchID' with the ID number if it is a Self Switch made with
 *     <Self> or a capital letter surrounded by quotes if it's A, B, C, or D.
 *   - This will return the true/false value of the Self Switch.
 *   - Example: getSelfSwitchValue(12, 34, 56)
 *   - Example: getSelfSwitchValue(12, 34, 'B')
 * 
 *   ---
 * 
 *   Get Self Variable Values:
 * 
 *   getSelfVariableValue(mapID, eventID, variableID)
 *   - Replace 'mapID' with the map ID the target event is located on.
 *   - Replace 'eventID' with the ID of the target event.
 *   - Replace 'variableID' with the ID number of the Self Variable.
 *   - This will return whatever stored value is found in the Self Variable.
 *   - Example: getSelfVariableValue(12, 34, 56)
 * 
 *   ---
 * 
 *   Set Self Switch Values:
 * 
 *   setSelfSwitchValue(mapID, eventID, switchID, value)
 *   - Replace 'mapID' with the map ID the target event is located on.
 *   - Replace 'eventID' with the ID of the target event.
 *   - Replace 'switchID' with the ID number if it is a Self Switch made with
 *     <Self> or a capital letter surrounded by quotes if it's A, B, C, or D.
 *   - Replace 'value' with either 'true' or 'false' for ON/OFF respectively.
 *     Do not use quotes.
 *   - This will change the Self Switch's value to true/false.
 *     - Example: setSelfSwitchValue(12, 34, 56, false)
 *     - Example: setSelfSwitchValue(12, 34, 'B', true)
 * 
 *   ---
 * 
 *   Set Self Variable Values:
 * 
 *   setSelfVariableValue(mapID, eventID, variableID, value)
 *   - Replace 'mapID' with the map ID the target event is located on.
 *   - Replace 'eventID' with the ID of the target event.
 *   - Replace 'variableID' with the ID number of the Self Variable.
 *   - Replace 'value' with the value you want to set the Self Variable to.
 *   - Example: setSelfVariableValue(12, 34, 56, 88888)
 * 
 *   ---
 * 
 * ---
 *
 * ============================================================================
 * Features: Map Switches and Variables
 * ============================================================================
 * 
 * Similar to Self Switches and Self Variables, Map Switches and Map Variables
 * are switches and variables that retain data based on the map the player is
 * currently located in. In other words, they're self switches and variables
 * but for maps instead!
 * 
 * These features do not exist in RPG Maker MZ by default. Just like with the
 * Self Switches and Self Variables, you can turn regular Switches or Variables
 * into Map Switches and Map Variables using the following name tag:
 * 
 * ---
 * 
 * <Map>
 * - Used for: Switch and Variable names
 * - Converts the Switch/Variable into a Map Switch/Variable.
 * 
 * ---
 *
 * After, just use them like you would for normal Switches and Variables in an
 * event's page conditions. If the <Map> tag is present inside the Switch or
 * Variable's name, then it will use data unique to only that map.
 *
 * NOTE: Tagged Switches/Variables are mutually exclusive from one another.
 * You cannot tag them with <JS>, <Self>, <Map>, or <Global> simultaneously.
 * 
 * ---
 * 
 * If you need to use a script call to get a Map Switch or Map Variable's
 * value, you can use the following script calls:
 * 
 *   ---
 * 
 *   Get Map Switch Values:
 * 
 *   getMapSwitchValue(mapID, switchID)
 *   - Replace 'mapID' with the map ID the switch is located on.
 *   - Replace 'switchID' with the ID number of the switch to get data.
 *   - Example: getMapSwitchValue(4, 20)
 * 
 *   ---
 * 
 *   Get Variable Switch Values:
 * 
 *   getMapVariableValue(mapID, variableID)
 *   - Replace 'mapID' with the map ID the switch is located on.
 *   - Replace 'variableID' with the ID number of the variable to get data.
 *   - Example: getMapVariableValue(6, 9)
 * 
 *   ---
 * 
 *   Set Map Switch Values:
 * 
 *   setMapSwitchValue(mapID, switchID, value)
 *   - Replace 'mapID' with the map ID the switch is located on.
 *   - Replace 'switchID' with the ID number of the switch to get data.
 *   - Replace 'value' with either 'true' or 'false' for ON/OFF respectively.
 *     Do not use quotes.
 *   - Example: setMapSwitchValue(4, 20, true)
 *   - Example: setMapSwitchValue(6, 9, false)
 * 
 *   ---
 * 
 *   Set Map Variable Values:
 * 
 *   setMapVariableValue(mapID, variableID, value)
 *   - Replace 'mapID' with the map ID the switch is located on.
 *   - Replace 'variableID' with the ID number of the variable to get data.
 *   - Replace 'value' with the value you want to set the Map Variable to.
 *   - Example: setMapVariableValue(6, 9, 420)
 * 
 *   ---
 * 
 * ---
 *
 * ============================================================================
 * Features: Reference Switches and Reference Variables
 * ============================================================================
 * 
 * Reference Switches and Reference Variables are added in version 1.62 of this
 * plugin. These switches and variables allow you to reference them through
 * strings when using script calls.
 * 
 * By simply naming your switch or variable ((Reference Name)), you can use
 * that 'reference name' in a string to call them for script calls. This is
 * just so you don't need to remember the ID's of every other Switch/Variable.
 * 
 * When referencing the strings in the script calls, case does not matter,
 * which means you can use all capitals or all lower case and they'll still
 * reference the same switch or variable.
 * 
 * ---
 * 
 * For example:
 * 
 *   ---
 * 
 *   Switch 10 Name: ((Priscilla Joined))
 * 
 *   Script Call: $gameSwitches.value('Priscilla Joined')
 *                $gameSwitches.setValue('Priscilla Joined', true)
 * 
 *   ---
 * 
 *   Variable 20 Name: Total ((Goblins Slain))
 * 
 *   Script Call: $gameVariables.value('Goblins Slain')
 *                $gameVariables.setValue('Goblins Slain', 50)
 * 
 *   ---
 * 
 * Remember to put quotes around the name for the script call!
 * 
 * This only applies for the $gameSwitches and $gameVariables functions of
 * value(id) and setValue(id, value). They do not apply to the other
 * $gameSwitches and $gameVariables functions.
 * 
 * ---
 *
 * ============================================================================
 * Features: Character Sprite Filename Tags
 * ============================================================================
 * 
 * For the files located inside of your project's /img/characters/ folder, if
 * the filenames themselves have specific "tags" in them, special properties
 * will be applied to them. These tags can be combined together with a few
 * exceptions.
 * 
 * Some of these are new to VisuStella MZ, while others are default to MZ.
 * 
 * ---
 * 
 *   !filename.png
 *   - Tag: !
 *   - Causes this character's sprite to align with the tile grid instead of
 *     being lifted a few pixels higher.
 *   - This is primarily used for things like doors, chests, and floor plates.
 *   - Default to RPG Maker MZ.
 * 
 * ---
 * 
 *   $filename.png
 *   - Tag: $
 *   - Causes this character's sprite to use the "big character" format.
 *   - Primarily used for sprites like the big monsters and such which only
 *     have 3x4 cells as opposed to 12x8 cells that regular sprite sheets have.
 *   - Cannot be combined with the [VS8] tag.
 *   - Default to RPG Maker MZ.
 * 
 * ---
 * 
 *   filename[Invisible].png
 *   - Tag: [Invisible] or [Inv]
 *   - This character's sprite will become invisible on the map screen in-game
 *     while almost everything else about it is visible.
 *   - This is used for those who wish to use sprite labels for things such as
 *     autorun and parallel events.
 * 
 * ---
 * 
 *   filename[VS8].png
 *   - Tag: [VS8]
 *   - Converts this sprite into a VisuStella-Style 8-Direction Sprite Sheet.
 *   - Refer to the section below.
 *   - Cannot be combined with the $ tag.
 * 
 * ---
 *
 * ============================================================================
 * Features: VisuStella-Style 8-Directional Sprite Sheets
 * ============================================================================
 *
 * This plugin provides support for the VisuStella-Style 8-Directional Sprite
 * Sheets, also know as VS8. VS8 sprite sheets offer support for walking
 * frames, dashing frames, carrying frames, and emotes.
 *
 * ---
 *
 * To designate a sprite sheet as VS8, simply add [VS8] to the filename.
 * Something like Actor1.png would become Actor1_[VS8].png.
 *
 * ---
 *
 * VS8 sprites are formatted as such. Each block below is a set of 3 frames.
 *
 * Walk Down    Walk DL     Dash Down   Dash DL
 * Walk Left    Walk DR     Dash Left   Dash DR
 * Walk Right   Walk UL     Dash Right  Dash UL
 * Walk Up      Walk UR     Dash Up     Dash UR
 *
 * Carry Down   Carry DL    Ladder      Emotes 3
 * Carry Left   Carry DR    Rope        Emotes 4
 * Carry Right  Carry UL    Emotes 1    Emotes 5
 * Carry Up     Carry UR    Emotes 2    Emotes 6
 *
 * ---
 *
 * Here are how each of the emote sets are grouped from left to right.
 *
 * Emotes 1: Item, Hmph, Victory
 * Emotes 2: Hurt, Kneel, Collapse
 * Emotes 3: !, ?, Music Note
 * Emotes 4: Heart, Anger, Sweat
 * Emotes 5: Cobweb, ..., Light Bulb
 * Emotes 6: Sleep0, Sleep1, Sleep2
 *
 * ---
 *
 * ============================================================================
 * Features: Weighted Random Movement
 * ============================================================================
 * 
 * When creating events to place on the map, you can determine what type of
 * autonomous movement the event will have. When selecting "Random", the event
 * will move randomly across the map.
 * 
 * However, with the way "Random" movement works with the RPG Maker MZ default
 * code, the event is more likely to hit a wall and then hug the said wall as
 * it maps laps around the map's outer borders making it feel very unnatural
 * for any player who's been on the map long enough.
 * 
 * This is where "Weighted Random Movement" comes in. It changes up the random
 * movement behavior to function where the farther the event is, the more
 * likely the event is to step back towards its "home" position (aka where it
 * spawned upon loading the map). This is so that a housewife NPC doesn't
 * suddenly wander off into the middle of an army's training grounds on the
 * same town map.
 * 
 * The event will stay closer to its home value depending on how high the
 * weight's value is. There are a number of ways to adjust the weighted value.
 * 
 * ---
 * 
 * Plugin Parameters > Movement > Event Movement > Random Move Weight
 * 
 * This Plugin Parameter setting allows you to set the default weight for all
 * events with "Random" autonomous movement. It is set at a default value of
 * 0.10 to give the event an understandable degree of freedom.
 * 
 * Lower numbers give events more freedom to move. Larger numbers will make the
 * events stick closer to home.
 * 
 * Change this value to 0 to disable it.
 * 
 * ---
 * 
 * You can customize this individually per event by using Notetags and/or
 * Comment Tags for the events.
 * 
 * <Random Move Weight: x>
 * 
 * - Used for: Event Notetags and Event Page Comment Tags
 * - If this tag is used on an event with random-type autonomous movement, then
 *   the event will stick closer to their home location (where they are located
 *   upon spawning on the map). How close they stick to their home location
 *   will depend on the weighted 'x' value.
 * - Replace 'x' with a number between 0 and 1. Numbers closer to 0 give the
 *   event more freedom when moving randomly while numbers closer to 1 cause
 *   the event to stick closer to their home position.
 * 
 * <True Random Move>
 * 
 * - Used for: Event Notetags and Event Page Comment Tags
 * - If this tag is used on an event with random-type autonomous movement, then
 *   that event will ignore the effects of weighted randomized movement.
 * 
 * ---
 *
 * ============================================================================
 * Notetags and Comment Tags
 * ============================================================================
 *
 * The following are notetags that have been added through this plugin. These
 * notetags will not work with your game if this plugin is OFF or not present.
 *
 * Some of these are comment tags. Comment tags are used for events to mark and
 * affect individual event pages rather than the whole event.
 *
 * === Map Notetags ===
 *
 * The following notetags are used for maps only. While some of these options
 * are also available in the Plugin Parameters, some of these notetags extend
 * usage to specific maps marked by these notetags as well.
 *
 * ---
 *
 * <Diagonal Movement: On>
 * <Diagonal Movement: Off>
 *
 * - Used for: Map Notetags
 * - Turns on/off diagonal movement for those maps.
 * - If notetag isn't present, use Plugin Parameter setting.
 *
 * ---
 *
 * <type Allow Region: x>
 * <type Allow Region: x, x, x>
 *
 * <type Forbid Region: x>
 * <type Forbid Region: x, x, x>
 *
 * <type Dock Region: x>
 * <type Dock Region: x, x, x>
 *
 * - Used for: Map Notetags
 * - Replace 'type' with 'All', 'Walk', 'Player', 'Event', 'Vehicle', 'Boat',
 *   'Ship', or 'Airship'.
 * - 'Allow' notetag variants allow that type to pass through them no matter
 *   what other passability settings are in place.
 * - 'Forbid' notetag variants forbid that type from passing through at all.
 * - 'Dock' notetag variants allow vehicles to dock there. Boats and ships must
 *   face the region direction while airships must land directly on top.
 *
 * ---
 * 
 * <Map Load Common Event: x>
 * <Map Load Common Events: x, x, x>
 * 
 * - Used for: Map Notetags
 * - When this map is loaded, run the specified Common Events once available.
 *   - Does NOT trigger if you transfer to a different part of the same map.
 * - Replace 'x' with a number representing the ID of the Common Event you wish
 *   to reserve and run once ready.
 * 
 * ---
 *
 * <Save Event Locations>
 *
 * - Used for: Maps Notetags
 * - Saves the locations of all events on the map so that when you return to
 *   that map at a later point, the events will be in the position they were
 *   last in.
 *
 * ---
 * 
 * <Hide Player>
 * <Show Player>
 * 
 * - Used for: Map Notetags
 * - Forcefully hides or shows the player sprite. This is so you don't need to
 *   manually turn the setting on/off each time you enter a specific map.
 * - These settings will take priority over the event commands.
 * - If the player sprite is hidden, so are the player's followers.
 * - If the player sprite is visible, the player's followers will still depend
 *   on their settings.
 * - These notetags are mutually exclusive from each other.
 * 
 * ---
 * 
 * <Hide Followers>
 * <Show Followers>
 * 
 * - Used for: Map Notetags
 * - Forcefully hides or shows the player's followers. This is so you don't
 *   need to manually turn them on/off each time you enter a specific map.
 * - These settings will take priority over the event commands.
 * - These notetags are mutually exclusive from each other.
 * 
 * ---
 * 
 * === Page Comment Tags ===
 * 
 * The following comment tags are to be put inside of the pages of events,
 * troops, and common events for them to work!
 * 
 * ---
 * 
 * <Page Conditions>
 *   conditions
 *   conditions
 *   conditions
 * </Page Conditions>
 * 
 * - Used for: Map Event Page, Troop Page, and Common Event Page Comment Tags
 * - This allows you to create custom page conditions that utilize the
 *   Conditional Branch event command to see if the additional page conditions
 *   are met.
 * 
 * ---
 * 
 * <Conditions Met>
 * - Used for: Map Event Page, Troop Page, and Common Event Page Comment Tags
 * - If used between the <Page Conditions> and </Page Conditions> comment tag,
 *   upon reaching this part of event command list, the custom page conditions
 *   will be considered met.
 * 
 * ---
 * 
 * Example:
 * 
 * ◆Comment：<Page Conditions>
 * ◆If：Reid has equipped Potion Sword
 *   ◆Comment：If Reid has equipped the Potion Sword
 * ：       ：<Condition Met>
 *   ◆
 * ：End
 * ◆Comment：</Page Conditions>
 * 
 * If Reid has the "Potion Sword" weapon equipped, then the additional custom
 * page conditions are met and the event page will be present/active.
 * 
 * If this is a troop condition, the troop page event will activate.
 * 
 * If this is a common event, there will be a parallel common event active.
 * 
 * ---
 *
 * === Event and Event Page Notetags ===
 *
 * The following notetags have comment tag variants (with a few exceptions).
 * If a notetag is used for an event, it will affect the event constantly.
 * If a comment tag is used, it will only affect the page the comment tag is
 * on and only that page.
 *
 * ---
 *
 * <Activation Region: x>
 * <Activation Regions: x,x,x>
 *
 * - Used for: Event Notetags and Event Page Comment Tags
 * - Allows this event to be remotely activated as long as the player is
 *   standing within a tile marked by a designated region.
 * - Replace 'x' with the regions you wish to remotely activate this event in.
 *   - Action Button: Player must press OK while being in the region.
 *   - Player/Event Touch: Player must step onto the region.
 *   - Autorun/Parallel: Player be in the region.
 * - If this is placed in a notetag, the effect will be present across
 *   all event pages used.
 * - If this is placed inside a page's comment, the effect will only occur
 *   if that event page is currently active.
 * - NOTE: This cannot be used with any other activation tags.
 *
 * ---
 *
 * <Activation Square: x>
 * <Activation Circle: x>
 * <Activation Delta: x>
 * <Activation Row: x>
 * <Activation Column: x>
 *
 * - Used for: Event Notetags and Event Page Comment Tags
 * - Allows this event to be remotely activated as long as the player is
 *   within range of its activation type.
 * - Replace 'x' with a number stating the range in tiles.
 *   - Square: A square-shaped range with the event at the center.
 *   - Circle: A circle-shaped range with the event at the center.
 *   - Delta: A diamond-shaped range with the event at the center.
 *   - Row: Spans horizontally across the map. 'x' expands up and down.
 *   - Column: Spans vertically across the map. 'x' expands left and right.
 * - If this is placed in a notetag, the effect will be present across
 *   all event pages used.
 * - If this is placed inside a page's comment, the effect will only occur
 *   if that event page is currently active.
 * - NOTE: This cannot be used with any other activation tags.
 *
 * ---
 *
 * <Always Update Movement>
 *
 * - Used for: Event Notetags and Event Page Comment Tags
 * - Events normally have to be within screen range for them to update their
 *   self movement. If this tag is present, the event is always updating.
 * - If this is placed in a notetag, the effect will be present across
 *   all event pages used.
 * - If this is placed inside a page's comment, the effect will only occur
 *   if that event page is currently active.
 *
 * ---
 *
 * <Click Trigger>
 *
 * - Used for: Event Notetags and Event Page Comment Tags
 * - Allows this event to activate upon being clicked on with the mouse.
 * - If this is placed in a notetag, the effect will be present across
 *   all event pages used.
 * - If this is placed inside a page's comment, the effect will only occur
 *   if that event page is currently active.
 *
 * ---
 *
 * <Copy Event: Map x, Event y>
 * <Copy Event: x, y>
 *
 * <Copy Event: template>
 *
 * - Used for: Event Notetags ONLY
 * - Makes this event copy all of the event settings from a different event
 *   that can be found on a different map (as long as that map is registered
 *   inside of Plugin Parameters => Event Template Settings => Preloaded Maps).
 * - Replace 'x' with a number representing the copied event's Map ID.
 *   - If '0' is used for the Map ID, reference the current map.
 * - Replace 'y' with a number representing the copied event's Event ID.
 * - For the 'template' variant, replace 'template' with the name of the
 *   template made in Plugin Parameters => Event Template Settings =>
 *   Event Template List.
 * - If this is placed in a notetag, the effect will be present across
 *   all event pages used.
 *
 * ---
 * 
 * <Custom Z: x>
 * 
 * - Used for: Event Notetags and Event Page Comment Tags
 * - Replace 'x' with a number value to determine the event sprite's Z value
 *   relative to the tilemap.
 * - For reference from rmmz_core.js:
 *   - 0 : Lower tiles
 *   - 1 : Lower characters
 *   - 3 : Normal characters
 *   - 4 : Upper tiles
 *   - 5 : Upper characters
 *   - 6 : Airship shadow
 *   - 7 : Balloon
 *   - 8 : Animation
 *   - 9 : Destination
 * - You can use numbers below 0 and above 9.
 *   - Values under 0 go below the tilemap.
 *   - Values above 9 go above everything else on the tilemap.
 *   - These values do NOT go below or above other screen objects that are
 *     NOT attached to the tilemap layer such as parallaxes or weather or
 *     windows because that's simply not how z-axis work with sprite layers.
 * 
 * ---
 * 
 * <Encounter Half Square: x>
 * <Encounter Half Circle: x>
 * <Encounter Half Delta: x>
 * <Encounter Half Row: x>
 * <Encounter Half Column: x>
 * 
 * - Used for: Event Notetags and Event Page Comment Tags
 * - If the player is within the 'x' area effect of this event, the random
 *   encounter rate will be halved.
 * - Replace 'x' with a number stating the range in tiles.
 *   - Square: A square-shaped range with the event at the center.
 *   - Circle: A circle-shaped range with the event at the center.
 *   - Delta: A diamond-shaped range with the event at the center.
 *   - Row: Spans horizontally across the map. 'x' expands up and down.
 *   - Column: Spans vertically across the map. 'x' expands left and right.
 * - If this is placed in a notetag, the effect will be present across
 *   all event pages used.
 * - If this is placed inside a page's comment, the effect will only occur
 *   if that event page is currently active.
 * 
 * Script Call Check:
 * 
 *   $isTileEncounterHalf(x, y)
 * 
 * - This can be used to check if a certain map tile (x, y) has an encounter
 *   rate halving effect on it.
 * - Returns a boolean (true or false) when used.
 * 
 * ---
 * 
 * <Encounter None Square: x>
 * <Encounter None Circle: x>
 * <Encounter None Delta: x>
 * <Encounter None Row: x>
 * <Encounter None Column: x>
 * 
 * - Used for: Event Notetags and Event Page Comment Tags
 * - If the player is within the 'x' area effect of this event, the random
 *   encounter rate will be suppressed completely.
 * - Replace 'x' with a number stating the range in tiles.
 *   - Square: A square-shaped range with the event at the center.
 *   - Circle: A circle-shaped range with the event at the center.
 *   - Delta: A diamond-shaped range with the event at the center.
 *   - Row: Spans horizontally across the map. 'x' expands up and down.
 *   - Column: Spans vertically across the map. 'x' expands left and right.
 * - If this is placed in a notetag, the effect will be present across
 *   all event pages used.
 * - If this is placed inside a page's comment, the effect will only occur
 *   if that event page is currently active.
 * 
 * Script Call Check:
 * 
 *   $isTileEncounterNone(x, y)
 * 
 * - This can be used to check if a certain map tile (x, y) has an encounter
 *   rate suppression effect on it.
 * - Returns a boolean (true or false) when used.
 * 
 * ---
 * 
 * <Erase if Encounter Half>
 * <Erase if Encounter None>
 * 
 * - Used for: Event Notetags ONLY
 * - Automatically erase this event if the player's party has an encounter half
 *   or encounter none effect, or if the event has spawned in an encounter half
 *   or encounter none area.
 * - This check only occurs in two situations: when the map is first loaded
 *   after being teleported into or when the player leaves a menu and returns
 *   back to the map.
 * - Events that have been erased due to this effect will NOT return even if
 *   the encounter half/none effect is removed while the player is still on the
 *   map. The event will return if the player exits the map and comes back.
 * 
 * ---
 * 
 * <Exit Reset Self Data>
 * 
 * - Used for: Event Notetags ONLY
 * - When the player leaves the current map, all Self Switches and Self
 *   Variables related to this event will be reset.
 * 
 * ---
 *
 * <Hitbox Left: x>
 * <Hitbox Right: x>
 * <Hitbox Up: x>
 * <Hitbox Down: x>
 *
 * - Used for: Event Notetags and Event Page Comment Tags
 * - Replace 'x' with a number to extend the hitbox of the event by that many
 *   tiles towards the listed direction.
 * - Use multiples of this notetag to extend them to different directions.
 * - If this is placed in a notetag, the effect will be present across
 *   all event pages used.
 * - If this is placed inside a page's comment, the effect will only occur
 *   if that event page is currently active.
 *
 * ---
 *
 * <Icon: x>
 *
 * - Used for: Event Notetags and Event Page Comment Tags
 * - Replace 'x' with the Icon ID you wish to put above this event.
 * - This will not override any Icons designated to the ID through a
 *   Plugin Command.
 * - If this is placed in a notetag, the effect will be present across
 *   all event pages used.
 * - If this is placed inside a page's comment, the effect will only occur
 *   if that event page is currently active.
 *
 * ---
 *
 * <Icon Buffer X: +x>
 * <Icon Buffer X: -x>
 *
 * <Icon Buffer Y: +x>
 * <Icon Buffer Y: -x>
 *
 * <Icon Buffer: +x, +y>
 * <Icon Buffer: -x, -y>
 *
 * - Used for: Event Notetags and Event Page Comment Tags
 * - Allows you to adjust the positions of the icon on the envent by buffers.
 * - Replace 'x' and 'y' with the values to adjust the position buffers by.
 * - If this is placed in a notetag, the effect will be present across
 *   all event pages used.
 * - If this is placed inside a page's comment, the effect will only occur
 *   if that event page is currently active.
 *
 * ---
 *
 * <Label: text>
 *
 * - Used for: Event Notetags and Event Page Comment Tags
 * - Puts a label over the event's head displaying 'text'.
 * - Text codes can be used.
 *   - If text codes are used, avoid text codes that use < and > wrappers.
 * - If this is placed in a notetag, the effect will be present across
 *   all event pages used.
 * - If this is placed inside a page's comment, the effect will only occur
 *   if that event page is currently active.
 *
 * ---
 *
 * <Label>
 * text
 * text
 * </Label>
 *
 * - Used for: Event Notetags and Event Page Comment Tags
 * - Puts a label over the event's head displaying 'text'.
 * - This can display multiple lines.
 * - Text codes can be used.
 *   - You can use text codes with < and > wrappers.
 * - If this is placed in a notetag, the effect will be present across
 *   all event pages used.
 * - If this is placed inside a page's comment, the effect will only occur
 *   if that event page is currently active.
 *
 * ---
 *
 * <Label Range: x>
 *
 * - Used for: Event Notetags and Event Page Comment Tags
 * - Sets a range requirement for the player to be in order for the event's
 *   label to appear.
 * - Replace 'x' with a number value depicting the range in tiles.
 * - If this is placed in a notetag, the effect will be present across
 *   all event pages used.
 * - If this is placed inside a page's comment, the effect will only occur
 *   if that event page is currently active.
 * - If this tag is not used, refer to the default plugin parameter settings.
 *
 * ---
 * 
 * <Label Range Type: Square>
 * <Label Range Type: Circle>
 * <Label Range Type: Diamond>
 * 
 * - Used for: Event Notetags and Event Page Comment Tags
 * - Sets a range type for the label to appear visible for.
 *   - Square: A square-shaped range with the event at the center.
 *   - Circle: A circle-shaped range with the event at the center.
 *   - Diamond: A diamond-shaped range with the event at the center.
 * - If this is placed in a notetag, the effect will be present across
 *   all event pages used.
 * - If this is placed inside a page's comment, the effect will only occur
 *   if that event page is currently active.
 * - If this tag is not used, refer to the default plugin parameter settings.
 * 
 * ---
 *
 * <Label Offset X: +x>
 * <Label Offset X: -x>
 *
 * <Label Offset Y: +x>
 * <Label Offset Y: -x>
 *
 * <Label Offset: +x, +y>
 * <Label Offset: -x, -y>
 *
 * - Used for: Event Notetags and Event Page Comment Tags
 * - Allows you to adjust the positions of the label on the envent by offsets.
 * - Replace 'x' and 'y' with the values to adjust the position offsets by.
 * - If this is placed in a notetag, the effect will be present across
 *   all event pages used.
 * - If this is placed inside a page's comment, the effect will only occur
 *   if that event page is currently active.
 *
 * ---
 * 
 * <Label Hue Shift: +x>
 * <Label Hue Shift: -x>
 * 
 * - Used for: Event Notetags and Event Page Comment Tags
 * - Changes the hue of the event label by +x or -x every frame.
 *   - Keep in mind that since this is changing hue, this will appear to have
 *     no effect if you are using black and white labels.
 *   - Use labels with text codes that add color to them like '\C[4]text'
 * - This only works with the sprite version of event labels and does not work
 *   with the legacy version.
 * 
 * ---
 * 
 * <Location X: +x>
 * <Location X: -x>
 * 
 * <Location Y: +x>
 * <Location Y: -x>
 * 
 * <Location: +x, +y>
 * <Location: +x, -y>
 * <Location: -x, +y>
 * <Location: -x, -y>
 * 
 * - Used for: Event Notetags and Event Page Comment Tags
 * - Adjusts the initial location of this event by +x and +y (or -x and -y).
 * - This allows you to stack events on top of each other or even move them to
 *   various places of the map.
 * - Replace 'x' with a number that represents the horizontal tiles to adjust
 *   the initial starting location by.
 * - Replace 'y' with a number that represents the vertical tiles to adjust
 *   the initial starting location by.
 * 
 * ---
 * 
 * <Mirror Sprite>
 * 
 * - Used for: Event Notetags and Event Page Comment Tags
 * - The event sprite's visual appearance is mirrored.
 * 
 * ---
 * 
 * <Move Only Region: x>
 * <Move Only Regions: x,x,x>
 * 
 * - Used for: Event Notetags and Event Page Comment Tags
 * - Sets the move range of this event to only the region(s) marked by the
 *   notetag(s) or comment tag(s).
 * - This will bypass terrain passability.
 * - This will not bypass event collision.
 * - If this is placed in a notetag, the effect will be present across
 *   all event pages used.
 * - If this is placed inside a page's comment, the effect will only occur
 *   if that event page is currently active.
 * 
 * ---
 *
 * <Move Synch Target: Player>
 *
 * <Move Synch Target: Event x>
 *
 * - Used for: Event Notetags and Event Page Comment Tags
 * - Synchronizes the movement of this event with a target (either the player
 *   or another event). This event will only move whenever the synchronized
 *   target moves.
 * - For 'Event x' variant, replace 'x' with the ID of the event to synch to.
 * - If this is placed in a notetag, the effect will be present across
 *   all event pages used.
 * - If this is placed inside a page's comment, the effect will only occur
 *   if that event page is currently active.
 *
 * ---
 *
 * <Move Synch Type: Random>
 * <Move Synch Type: Approach>
 * <Move Synch Type: Away>
 * <Move Synch Type: Custom>
 *
 * <Move Synch Type: Mimic>
 * <Move Synch Type: Reverse Mimic>
 *
 * <Move Synch Type: Mirror Horizontal>
 * <Move Synch Type: Mirror Vertical>
 *
 * - Used for: Event Notetags and Event Page Comment Tags
 * - Choose the type of movement the event will have if it is synchronized to
 *   a target.
 *   - Random: Move to a random position.
 *   - Approach: Approaches target.
 *   - Away: Flees from target.
 *   - Custom: Follows a custom move route.
 *   - Mimic: Imitates the target's movement style.
 *   - Reverse Mimic: Does the opposite of the target's movement.
 *   - Mirror Horizontal: Moves as if a mirror is placed horizontally.
 *   - Mirror Vertical: Moves as if a mirror is placed vertically.
 * - If this is placed in a notetag, the effect will be present across
 *   all event pages used.
 * - If this is placed inside a page's comment, the effect will only occur
 *   if that event page is currently active.
 *
 * ---
 *
 * <Move Synch Delay: x>
 *
 * - Used for: Event Notetags and Event Page Comment Tags
 * - If this tag is present, the event will wait a bit after each move before
 *   moving again.
 * - Replace 'x' with the number of movement instances in between.
 * - If this is placed in a notetag, the effect will be present across
 *   all event pages used.
 * - If this is placed inside a page's comment, the effect will only occur
 *   if that event page is currently active.
 *
 * ---
 * 
 * <Move Synch Distance Opacity: x>
 * 
 * - Used for: Event Notetags and Event Page Comment Tags
 * - Changes the opacity of the event based on the distance between it and its
 *   move synched target. Closer means more opaque. Further away means more
 *   transparent.
 * - Replace 'x' with a number representing the opacity change per pixel
 *   distance away. 'x' can use decimal values like 1.05 and 1.5.
 * 
 * ---
 * 
 * <Picture Filename: filename>
 * 
 * - Used for: Event Notetags and Event Page Comment Tags
 * - Applies a picture graphic from the /img/pictures/ folder of your project.
 * - This graphic will be on top of the character sprite but below the event
 *   icon sprite.
 *   - The picture priority will be the same as the event's priority.
 *   - If it is "below characters", the player can walk on top of it.
 *   - If it is "above characters", the player will behind it.
 *   - If it is "same as characters", the priority will be based on the
 *     current relative Y position. This also means, if the picture is big
 *     enough, it can clip into the top of tree tiles and such.
 * - Replace 'filename' with a filename from the game project's /img/pictures/
 *   folder. This is case sensitive. Do NOT include the file extension.
 * - If this is placed in a notetag, the effect will be present across
 *   all event pages used.
 * - If this is placed inside a page's comment, the effect will only occur
 *   if that event page is currently active.
 * 
 * ---
 * 
 * <Picture Type: Enemy>
 * <Picture Type: SV Enemy>
 * 
 * - Used for: Event Notetags and Event Page Comment Tags
 * - Used with <Picture Filename: filename> notetag.
 * - Will use /img/enemies/ or /img/sv_enemies/ instead of /img/pictures/ to
 *   grab a picture graphic from.
 * - Other picture graphic sprite related notetags will apply as normal.
 * 
 * ---
 * 
 * <Picture Max Size: x>
 * <Picture Scale: y%>
 * 
 * - Used for: Event Notetags and Event Page Comment Tags
 * - Used with <Picture Filename: filename> notetag.
 * - If the "Max Size" or "Scale" supplementary notetags are used, the picture
 *   graphic will be scaled proportionally to fit either the exact pixel size
 *   for "Max Size" or the "Scale" ratio.
 *   - Both the "Max Size" and "Scale" notetags require the "Filename" notetag.
 * - Replace 'x' with a number value representing the exact pixel size for the
 *   "Max Size" notetag.
 * - Replace 'y' with a number value representing the scale on which to shrink
 *   or enlarge the picture. 100% is normal size. 50% is half size. 200% is
 *   double size.
 * - If this is placed in a notetag, the effect will be present across
 *   all event pages used.
 * - If this is placed inside a page's comment, the effect will only occur
 *   if that event page is currently active.
 * 
 * ---
 *
 * <Picture Offset X: +x>
 * <Picture Offset X: -x>
 *
 * <Picture Offset Y: +x>
 * <Picture Offset Y: -x>
 *
 * <Picture Offset: +x, +y>
 * <Picture Offset: -x, -y>
 * 
 * - Used for: Event Notetags and Event Page Comment Tags
 * - Used with <Picture Filename: filename> notetag.
 * - Offsets the X and Y position of the event picture relative to the event
 *   sprite's own position.
 * - Replace 'x' and 'y' with numbers indicating the offset in pixels.
 * - If this is placed in a notetag, the effect will be present across
 *   all event pages used.
 * - If this is placed inside a page's comment, the effect will only occur
 *   if that event page is currently active.
 * 
 * ---
 * 
 * <Picture Wait Frames: x>
 * 
 * - Used for: Event Notetags and Event Page Comment Tags
 * - Used with <Picture Filename: filename> notetag.
 * - Requires VisuMZ_4_AnimatedPictures!
 * - "Wait Frames" is used with VisuMZ's Animated Pictures plugin. This
 *   determines the delay inbetween frame changes.
 * - Replace 'x' with a number representing the amount of frames to wait
 *   inbetween frame changes.
 * - If this is placed in a notetag, the effect will be present across
 *   all event pages used.
 * - If this is placed inside a page's comment, the effect will only occur
 *   if that event page is currently active.
 * 
 * ---
 * 
 * <Playtest>
 * 
 * - Used for: Event Notetags.
 * - This does NOT work when it's in the Event Page Comment Tags.
 * - If this notetag is found in the event's notebox (NOT comments), then the
 *   event will only appear during a playtest session. It will not appear in a
 *   deployed game where the playtest flag is not on.
 * 
 * ---
 * 
 * <Random Move Weight: x>
 * 
 * - Used for: Event Notetags and Event Page Comment Tags
 * - If this tag is used on an event with random-type autonomous movement, then
 *   the event will stick closer to their home location (where they are located
 *   upon spawning on the map). How close they stick to their home location
 *   will depend on the weighted 'x' value.
 * - Replace 'x' with a number between 0 and 1. Numbers closer to 0 give the
 *   event more freedom when moving randomly while numbers closer to 1 cause
 *   the event to stick closer to their home position.
 * 
 * ---
 * 
 * <True Random Move>
 * 
 * - Used for: Event Notetags and Event Page Comment Tags
 * - If this tag is used on an event with random-type autonomous movement, then
 *   that event will ignore the effects of weighted randomized movement.
 * 
 * ---
 *
 * <Save Event Location>
 *
 * - Used for: Event Notetags ONLY
 * - Saves the locations of the event on the map so that when you return to
 *   that map at a later point, the event will be in the position it was
 *   last in.
 *
 * ---
 *
 * <Hide Shadow>
 * - Used for: Event Notetags and Event Page Comment Tags
 * - Hides the shadow for the event.
 * - If this is placed in a notetag, the effect will be present across
 *   all event pages used.
 * - If this is placed inside a page's comment, the effect will only occur
 *   if that event page is currently active.
 *
 * ---
 * 
 * <Scale: x%>
 * 
 * <Scale X: x%>
 * <Scale Y: y%>
 * 
 * - Used for: Event Notetags and Event Page Comment Tags
 * - Changes the scale of the sprite to the designated size.
 * - For <Scale: x%> variant: replace 'x' with a number representing the
 *   scaling overall percentage to be used.
 * - For <Scale X: x%> variant, replace 'x' with a number representing the x
 *   factor for the horizontal scaling percentage to be used.
 * - For <Scale Y: y%> variant, replace 'y' with a number representing the y
 *   factor for the vertical scaling percentage to be used.
 * - If this is placed in a notetag, the effect will be present across
 *   all event pages used.
 * - If this is placed inside a page's comment, the effect will only occur
 *   if that event page is currently active.
 * 
 * ---
 *
 * <Shadow Filename: filename>
 *
 * - Used for: Event Notetags and Event Page Comment Tags
 * - Replaces the shadow graphic used with 'filename' found in the
 *   img/system/ project folder.
 * - If this is placed in a notetag, the effect will be present across
 *   all event pages used.
 * - If this is placed inside a page's comment, the effect will only occur
 *   if that event page is currently active.
 *
 * ---
 *
 * <Sprite Offset X: +x>
 * <Sprite Offset X: -x>
 *
 * <Sprite Offset Y: +x>
 * <Sprite Offset Y: -x>
 *
 * <Sprite Offset: +x, +y>
 * <Sprite Offset: -x, -y>
 *
 * - Used for: Event Notetags and Event Page Comment Tags
 * - Changes how much the event's sprite is visibly offset by.
 * - Replace 'x' and 'y' with numbers indicating the offset in pixels.
 * - If this is placed in a notetag, the effect will be present across
 *   all event pages used.
 * - If this is placed inside a page's comment, the effect will only occur
 *   if that event page is currently active.
 *
 * ---
 *
 * <Step Pattern: Left to Right>
 * <Step Pattern: Right to Left>
 *
 * <Step Pattern: Spin Clockwise>
 * <Step Pattern: Spin CW>
 *
 * <Step Pattern: Spin CounterClockwise>
 * <Step Pattern: Spin CCW>
 * <Step Pattern: Spin AntiClockwise>
 * <Step Pattern: Spin ACW>
 *
 * - Used for: Event Notetags and Event Page Comment Tags
 * - Changes the way the event animates if a tag is present.
 *   - Left to Right: Makes the event sprite's step behavior go from frame 0 to
 *     1 to 2, then back to 0 instead of looping backward.
 *   - Right to Left: Makes the event sprite's step behavior go from frame 2 to
 *     1 to 0, then back to 2 instead of looping forward.
 *   - Spin Clockwise: Makes the event sprite's step behavior spin CW.
 *   - Spin CounterClockwise: Makes the event sprite's step behavior spin CCW.
 * - If this is placed in a notetag, the effect will be present across
 *   all event pages used.
 * - If this is placed inside a page's comment, the effect will only occur
 *   if that event page is currently active.
 *
 * ---
 * 
 * <Tile Expand Up: x>
 * <Tile Expand Down: x>
 * <Tile Expand Left: x>
 * <Tile Expand Right: x>
 * 
 * - Used for: Event Notetags and Event Page Comment Tags
 * - Used for events with tile graphics. Expands the graphic up, down, left, or
 *   right from the spritesheet.
 *   - This does NOT expand the hitbox.
 * - The graphic will be anchored to the tile it's expanded from. This means
 *   even if you expanded downward, the actual event's position will still be
 *   the current event's X/Y coordinates. It's just grown more vertically and
 *   is still centered horizontally.
 * - This is primarily used to save on having to use too many events for tiles
 *   that expanded past 1x1 tile sizes.
 * 
 * ---
 *
 * ============================================================================
 * Plugin Commands
 * ============================================================================
 *
 * The following are Plugin Commands that come with this plugin. They can be
 * accessed through the Plugin Command event command.
 *
 * ---
 * 
 * === Auto Movement Plugin Commands ===
 * 
 * ---
 *
 * Auto Movement: Events
 * - Allow/stop events from auto movement.
 *
 *   Value:
 *   - Allow events to move automatically?
 *
 * ---
 * 
 * === Call Event Plugin Commands ===
 * 
 * ---
 *
 * Call Event: Remote Read
 * - Runs the page of a different event remotely.
 * - This will run the page of the target event on the CURRENT event.
 * - This means that any "This Event" commands will be applied to the event
 *   using this Plugin Command and NOT the target event that page data is being
 *   retrieved from.
 * - Think of it as the current event using the target called event as a
 *   Common Event ala how RPG Maker 2003 works (for those familiar with it).
 *
 *   Map ID:
 *   - Target event's map. Use 0 for current map.
 *   - You may use JavaScript code.
 *
 *   Event ID:
 *   - The ID of the event to remotely run.
 *   - Use 0 for current event.
 *   - You may use JavaScript code.
 *
 *   Page ID:
 *   - The page of the remote event to run.
 *   - You may use JavaScript code.
 *
 * ---
 * 
 * === Dash Plugin Commands ===
 * 
 * ---
 *
 * Dash Enable: Toggle
 * - Enable/Disable Dashing on maps.
 *
 *   Value:
 *   - What do you wish to change dashing to?
 *
 * ---
 * 
 * === Event Icon Plugin Commands ===
 * 
 * ---
 *
 * Event Icon: Change (Temporary)
 * - Change the icon that appears on an event.
 * - This change is temporary and resets upon new events.
 *
 *   Map ID:
 *   - The map the target map. Use 0 for current map.
 *   - You may use JavaScript code.
 *
 *   Event ID:
 *   - The ID of the target event.
 *   - Use 0 for current event.
 *   - You may use JavaScript code.
 *
 *   Icon Index:
 *   - Icon index used for the icon.
 *   - You may use JavaScript code.
 *
 *   Buffer X:
 *   - How much to shift the X position by?
 *   - You may use JavaScript code.
 *
 *   Buffer Y:
 *   - How much to shift the Y position by?
 *   - You may use JavaScript code.
 *
 *   Blend Mode:
 *   - What kind of blend mode do you wish to apply to the icon sprite?
 *
 * ---
 *
 * Event Icon: Change (Forced)
 * - Change the icon that appears on an event.
 * - This change is forced and needs to be restored.
 *
 *   Map ID:
 *   - The map the target map. Use 0 for current map.
 *   - You may use JavaScript code.
 *
 *   Event ID:
 *   - The ID of the target event.
 *   - Use 0 for current event.
 *   - You may use JavaScript code.
 *
 *   Icon Index:
 *   - Icon index used for the icon.
 *   - You may use JavaScript code.
 *
 *   Buffer X:
 *   - How much to shift the X position by?
 *   - You may use JavaScript code.
 *
 *   Buffer Y:
 *   - How much to shift the Y position by?
 *   - You may use JavaScript code.
 *
 *   Blend Mode:
 *   - What kind of blend mode do you wish to apply to the icon sprite?
 *
 * ---
 *
 * Event Icon: Delete
 * - Delete the icon that appears on an event.
 * - This will remain deleted and invisible for events.
 *
 *   Map ID:
 *   - The map the target map. Use 0 for current map.
 *   - You may use JavaScript code.
 *
 *   Event ID:
 *   - The ID of the target event.
 *   - Use 0 for current event.
 *   - You may use JavaScript code.
 *
 * ---
 * 
 * Event Icon: Restore
 * - Restores a deleted or forced icon that appears on an event.
 * 
 *   Map ID: 
 *   - The map the target map. Use 0 for current map.
 *   - You may use JavaScript code.
 *
 *   Event ID:
 *   - The ID of the target event.
 *   - Use 0 for current event.
 *   - You may use JavaScript code.
 * 
 * ---
 * 
 * === Event Label Plugin Commands ===
 * 
 * ---
 *
 * Event Label: Refresh
 * - Refresh all Event Labels on screen.
 * - This is used to refresh page conditions for map changes that don't
 *   force a refresh.
 *
 * ---
 *
 * Event Label: Visible
 * - Change the visibility of Event Labels.
 *
 *   Visibility:
 *   - What do you wish to change visibility to?
 *
 * ---
 * 
 * === Event Location Plugin Commands ===
 * 
 * ---
 *
 * Event Location: Save
 * - Memorize an event's map location so it reappears there the next time the
 *   map is loaded.
 *
 *   Event ID:
 *   - The ID of the target event.
 *   - Use 0 for current event.
 *   - You may use JavaScript code.
 *
 * ---
 *
 * Event Location: Delete
 * - Deletes an event's saved map location.
 * - The event will reappear at its default location.
 *
 *   Map ID:
 *   - The map the target map. Use 0 for current map.
 *   - You may use JavaScript code.
 *   
 *   Event ID:
 *   - The ID of the target event.
 *   - Use 0 for current event.
 *   - You may use JavaScript code.
 *
 * ---
 *
 * Event Location: Create
 * - Creates a custom spawn location for a specific map's event so it appears
 *   there the next time the map is loaded.
 *
 *   Map ID:
 *   - The map the target map. Use 0 for current map.
 *   - You may use JavaScript code.
 *
 *   Event ID:
 *   - The ID of the target event.
 *   - Use 0 for current event.
 *   - You may use JavaScript code.
 *
 *   X Coordinate:
 *   - The X coordinate of the event.
 *   - You may use JavaScript code.
 *
 *   Y Coordinate:
 *   - The Y coordinate of the event.
 *   - You may use JavaScript code.
 *
 *   Direction:
 *   - The direction the event will be facing.
 *
 *   Optional:
 *
 *     Page ID:
 *     - The page of the event to set the move route to.
 *     - You may use JavaScript code.
 *
 *     Move Route Index:
 *     - The point in the move route for this event to be at if the page ID
 *       matches the rest of the page conditions.
 *
 * ---
 * 
 * === Event Popup Plugin Commands ===
 * 
 * ---
 * 
 * Event Popup: Player
 * - Makes a centered event popup on the player sprite.
 * - Requires VisuMZ_1_MessageCore!
 * - Cannot be used in battle!
 * 
 *   Message Text:
 *   - Insert the text to be displayed.
 *   - Text codes can be used.
 * 
 *   Message Duration:
 *   - What is the frame duration of the event popup?
 *   - 60 frames = 1 second. You may use code.
 * 
 *   Popup Settings:
 *   - These settings let you adjust how the popup animates.
 *   - See "Popup Settings" section below.
 * 
 * ---
 * 
 * Event Popup: Follower
 * - Makes a centered event popup on target follower sprite.
 * - Requires VisuMZ_1_MessageCore!
 * - Cannot be used in battle!
 * 
 *   Follower Index:
 *   - Which follower index to play popup?
 *   - Index starts at 0.
 *   - You may use JavaScript code.
 * 
 *   Message Text:
 *   - Insert the text to be displayed.
 *   - Text codes can be used.
 * 
 *   Message Duration:
 *   - What is the frame duration of the event popup?
 *   - 60 frames = 1 second.
 *   - You may use code.
 * 
 *   Popup Settings:
 *   - These settings let you adjust how the popup animates.
 *   - See "Popup Settings" section below.
 * 
 * ---
 * 
 * Event Popup: Event
 * - Makes a centered event popup on target event sprite.
 * - Requires VisuMZ_1_MessageCore!
 * - Cannot be used in battle!
 * 
 *   Event ID:
 *   - The ID of the event to play popup on.
 *   - Use 0 for current event.
 *   - You may use JavaScript code.
 * 
 *   Message Text:
 *   - Insert the text to be displayed.
 *   - Text codes can be used.
 * 
 *   Message Duration:
 *   - What is the frame duration of the event popup?
 *   - 60 frames = 1 second. You may use code.
 * 
 *   Popup Settings:
 *   - These settings let you adjust how the popup animates.
 *   - See "Popup Settings" section below.
 * 
 * ---
 * 
 * Event Popup: Target Tile
 * - Makes a centered event popup on target tile.
 * - Requires VisuMZ_1_MessageCore!
 * - Cannot be used in battle!
 * 
 *   Map Tile X:
 *   Map Tile Y:
 *   - The x/y coordinate of the map tile.
 *   - You may use JavaScript code.
 * 
 *   Message Text:
 *   - Insert the text to be displayed.
 *   - Text codes can be used.
 * 
 *   Message Duration:
 *   - What is the frame duration of the event popup?
 *   - 60 frames = 1 second. You may use code.
 * 
 *   Popup Settings:
 *   - These settings let you adjust how the popup animates.
 *   - See "Popup Settings" section below.
 * 
 * ---
 * 
 * Popup Settings
 * 
 *   Fade Settings:
 * 
 *     Fade In Duration:
 *     - How many frames does it take to fade in?
 *     - 60 frames = 1 second.
 * 
 *     Fade Out Duration:
 *     - How many frames does it take to fade out?
 *     - 60 frames = 1 second.
 * 
 *   Offset Settings:
 * 
 *     Starting Offset X:
 *     - Offsets the starting x position.
 *     - Negative: left. Positive: right.
 *     - You may use code.
 * 
 *     Starting Offset Y:
 *     - Offsets the starting y position. 
 *     - Negative: up. Positive: down.
 *     - You may use code.
 * 
 *     Ending Offset X:
 *     - Offsets the ending x position. 
 *     - Negative: left. Positive: right.
 *     - You may use code.
 * 
 *     Ending Offset Y:
 *     - Offsets the ending y position. 
 *     - Negative: up. Positive: down.
 *     - You may use code.
 * 
 *   Scaling Settings:
 * 
 *     Starting Scale X:
 *     - What is the starting scale x?
 *     - 0.0 = 0%, 0.5 = 50%, 1.0 = 100%
 *     - You may use code.
 * 
 *     Starting Scale Y:
 *     - What is the starting scale y?
 *     - 0.0 = 0%, 0.5 = 50%, 1.0 = 100%
 *     - You may use code.
 * 
 *     Ending Scale X:
 *     - What is the ending scale x?
 *     - 0.0 = 0%, 0.5 = 50%, 1.0 = 100%
 *     - You may use code.
 * 
 *     Ending Scale Y:
 *     - What is the ending scale y?
 *     - 0.0 = 0%, 0.5 = 50%, 1.0 = 100%
 *     - You may use code.
 * 
 *   Angle Settings:
 * 
 *     Starting Offset Angle:
 *     - What is the starting angle offset?
 *     - Use numbers between 0 and 360.
 *     - You may use code.
 * 
 *     Ending Offset Angle:
 *     - What is the ending angle offset?
 *     - Use numbers between 0 and 360.
 *     - You may use code.
 * 
 *   Misc Settings:
 * 
 *     Arc Peak:
 *     - This is the height of the popup's trajectory arc in pixels.
 *     - Positive: up. Negative: down.
 *     - You may use code.
 * 
 * ---
 * 
 * === Event Timer Plugin Commands ===
 * 
 * ---
 *
 * Event Timer: Change Speed
 * - Changes the timer frame decrease (or increase) speed.
 *
 *   Speed:
 *   - How many 1/60ths of a second does each frame increase or decrease by?
 *   - Negative decreases.
 *   - Positive increases.
 *   - JavaScript allowed.
 *
 * ---
 *
 * Event Timer: Expire Event Assign
 * - Sets a Common Event to run upon expiration.
 * - Bypasses the default code if one is set.
 *
 *   Common Event ID:
 *   - Select the Common Event to run upon the timer's expiration.
 *
 * ---
 *
 * Event Timer: Expire Event Clear
 * - Clears any set to expire Common Event and instead, run the default
 *   Game_Timer expiration code.
 *
 * ---
 *
 * Event Timer: Frames Gain
 * - Chooses how many frames, seconds, minutes, or hours are gained or lost for
 *   the event timer.
 *
 *   Frames:
 *   - How many 1/60ths of a second are gained/lost?
 *   - Positive for gain.
 *   - Negative for lost.
 *   - JavaScript allowed.
 *
 *   Seconds:
 *   - How many seconds are gained/lost?
 *   - Positive for gain.
 *   - Negative for lost.
 *   - JavaScript allowed.
 *
 *   Minutes:
 *   - How many minutes are gained/lost?
 *   - Positive for gain.
 *   - Negative for lost.
 *   - JavaScript allowed.
 *
 *   Hours:
 *   - How many hours are gained/lost?
 *   - Positive for gain.
 *   - Negative for lost.
 *   - JavaScript allowed.
 *
 * ---
 *
 * Event Timer: Frames Set
 * - Chooses how many frames, seconds, minutes, or hours are set for the event
 *   timer.
 *
 *   Frames:
 *   - Set frame count to this value.
 *   - Each frame is 1/60th of a second.
 *   - JavaScript allowed.
 *
 *   Seconds:
 *   - Set seconds to this value.
 *   - JavaScript allowed.
 *
 *   Minutes:
 *   - Set minutes to this value.
 *   - Each minute is 60 seconds.
 *   - JavaScript allowed.
 *
 *   Hours:
 *   - Set hours to this value.
 *   - Each hour is 60 minutes.
 *   - JavaScript allowed.
 *
 * ---
 *
 * Event Timer: Pause
 * - Pauses the current event timer, but does not stop it.
 *
 * ---
 *
 * Event Timer: Resume
 * - Resumes the current event timer from the paused state.
 *
 * ---
 * 
 * === Follower Control Plugin Commands ===
 * 
 * ---
 *
 * Follower: Set Global Chase
 * - Disables all followers from chasing the player or reenables it.
 *
 *   Chase:
 *   - Sets all followers to chase the player or not.
 *
 * ---
 *
 * Follower: Set Target Chase
 * - Disables target follower from chasing the player or reenables it.
 *
 *   Follower ID:
 *   - Select which follower ID to disable/reenable chasing for.
 *
 *   Chase:
 *   - Sets target follower to chase its target or not.
 *
 * ---
 *
 * Follower: Set Control
 * - Sets the event commands to target a follower when "Player" is selected as
 *   the target.
 *
 *   Follower ID:
 *   - Select which follower ID to control.
 *   - 0 is the player.
 *
 * ---
 *
 * Follower: Reset
 * - Resets all follower controls. Event Commands that target the "Player"
 *   return to normal and followers chase again.
 *
 * ---
 * 
 * === Global Switch Plugin Commands ===
 * 
 * ---
 * 
 * Global Switch: Get Self Switch A B C D
 * - Gets the current ON/OFF value from a Self Switch and stores it onto a
 *   Global Switch.
 * 
 *   Map ID:
 *   - The map the source map. Use 0 for current map.
 *   - You may use JavaScript code.
 * 
 *   Event ID:
 *   - The ID of the source event.
 *   - Use 0 for current event.
 *   - You may use JavaScript code.
 * 
 *   Letter:
 *   - Letter of the target event's Self Switch to obtain data from.
 * 
 *   -
 * 
 *   Target Switch ID:
 *   - The ID of the target switch.
 * 
 * ---
 * 
 * Global Switch: Get Self Switch ID
 * - Gets the current ON/OFF value from a Self Switch and stores it onto a
 *   Global Switch.
 * 
 *   Map ID:
 *   - The map the source map. Use 0 for current map.
 *   - You may use JavaScript code.
 * 
 *   Event ID:
 *   - The ID of the source event.
 *   - Use 0 for current event.
 *   - You may use JavaScript code.
 * 
 *   Switch ID:
 *   - The ID of the source switch.
 * 
 *   -
 * 
 *   Target Switch ID:
 *   - The ID of the target switch.
 * 
 * ---
 * 
 * === Global Variable Plugin Commands ===
 * 
 * ---
 * 
 * Global Variable: Get Self Variable ID
 * - Gets the current stored value from a Self Variable and stores it onto a
 *   Global Variable.
 * 
 *   Map ID:
 *   - The map the source map. Use 0 for current map.
 *   - You may use JavaScript code.
 * 
 *   Event ID:
 *   - The ID of the source event.
 *   - Use 0 for current event.
 *   - You may use JavaScript code.
 * 
 *   Variable ID:
 *   - The ID of the source variable.
 * 
 *   -
 * 
 *   Target Variable ID:
 *   - The ID of the target variable.
 * 
 * ---
 * 
 * === Morph Event Plugin Commands ===
 * 
 * ---
 *
 * Morph Event: Change
 * - Runs the page of a different event remotely.
 *
 *   Step 1:
 *
 *     Map ID:
 *     - Target event's map. Use 0 for current map.
 *     - You may use JavaScript code.
 *
 *     Event ID:
 *     - The ID of the target event.
 *     - Use 0 for current event.
 *     - You may use JavaScript code.
 *
 *   Step 2:
 *
 *     Template Name:
 *     - Name of the target event template to morph into.
 *     - Ignored if this is called "Untitled".
 *
 *     Map ID:
 *     - Target event's map. Use 0 for current map.
 *     - You may use JavaScript code.
 *
 *     Event ID:
 *     - The ID of the target event.
 *     - Use 0 for current event.
 *     - You may use JavaScript code.
 *
 *     Preserve Morph:
 *     - Is the morph effect preserved?
 *     - Or does it expire upon leaving the map?
 *
 * ---
 *
 * Morph Event: Remove
 * - Remove the morph status of an event.
 *
 *   Map ID:
 *   - Target event's map. Use 0 for current map.
 *   - You may use JavaScript code.
 *
 *   Event ID:
 *   - The ID of the event to remotely run.
 *   - Use 0 for current event.
 *   - You may use JavaScript code.
 *
 *   Remove Preservation:
 *   - Also remove the preservation effect?
 *
 * ---
 * 
 * === Player Icon Plugin Commands ===
 * 
 * ---
 *
 * Player Icon: Change
 * - Change the icon that appears on on the player.
 *
 *   Icon Index:
 *   - Icon index used for the icon.
 *   - You may use JavaScript code.
 *
 *   Buffer X:
 *   - How much to shift the X position by?
 *   - You may use JavaScript code.
 *
 *   Buffer Y:
 *   - How much to shift the Y position by?
 *   - You may use JavaScript code.
 *
 *   Blend Mode:
 *   - What kind of blend mode do you wish to apply to the icon sprite?
 *
 * ---
 *
 * Player Icon: Delete
 * - Delete the icon that appears on the player.
 *
 * ---
 * 
 * === Player Movement Plugin Commands ===
 * 
 * ---
 * 
 * Player Movement: Control
 * - Enable or disable player control over the player character's movement.
 * 
 *   Enable?:
 *   - Let the player control where the player character moves?
 * 
 * ---
 * 
 * Player Movement: Diagonal
 * - Override settings to for player diagonal movement.
 * 
 *   Setting:
 *   - How do you want to change diagonal movement?
 *   - Default: Whatever the Map Uses
 *   - Forcefully Disable Diagonal Movement
 *   - Forcefully Enable Diagonal Movement
 * 
 * ---
 * 
 * === Self Data Plugin Commands ===
 * 
 * ---
 * 
 * Self Data: Reset All
 * - Reset the Self Switch and Self Variable data of all events within the
 *   specified map.
 * 
 *   Map ID:
 *   - The map the target map. Use 0 for current map.
 *   - You may use JavaScript code.
 * 
 * ---
 * 
 * === Self Switch Plugin Commands ===
 * 
 * ---
 *
 * Self Switch: A B C D
 * - Change the Self Switch of a different event.
 *
 *   Map ID:
 *   - The map the target map. Use 0 for current map.
 *   - You may use JavaScript code.
 *
 *   Event ID:
 *   - The ID of the target event.
 *   - Use 0 for current event.
 *   - You may use JavaScript code.
 *
 *   Letter:
 *   - Letter of the target event's Self Switch to change.
 *
 *   Value:
 *   - What value do you want to set the Self Switch to?
 *
 * ---
 *
 * Self Switch: Switch ID
 * - Change the Self Switch of a different event.
 *
 *   Map ID:
 *   - The map the target map. Use 0 for current map.
 *   - You may use JavaScript code.
 *
 *   Event ID:
 *   - The ID of the target event.
 *   - Use 0 for current event.
 *   - You may use JavaScript code.
 *
 *   Switch ID:
 *   - The ID of the target switch.
 *
 *   Value:
 *   - What value do you want to set the Self Switch to?
 *
 * ---
 * 
 * === Self Variable Plugin Commands ===
 * 
 * ---
 *
 * Self Variable: Variable ID
 * - Change the Self Variable of a different event.
 *
 *   Map ID:
 *   - The map the target map. Use 0 for current map.
 *   - You may use JavaScript code.
 *
 *   Event ID:
 *   - The ID of the target event.
 *   - Use 0 for current event.
 *   - You may use JavaScript code.
 *
 *   Variable ID:
 *   - The ID of the target variable.
 *
 *   Value:
 *   - What value do you want to set the Self Switch to?
 *
 * ---
 * 
 * === Shadow Visibility Plugin Commands ===
 * 
 * ---
 * 
 * Shadow Hide: Player
 * - Hides the visibility of the player sprite shadow.
 * 
 * ---
 * 
 * Shadow Hide: Followers
 * - Hides the visibility of follower sprite shadows.
 * 
 * ---
 * 
 * Shadow Hide: All Events
 * - Hides the visibility of all event sprite shadows.
 * 
 * ---
 * 
 * Shadow Show: Player
 * - Returns the visibility of the player sprite shadow.
 * - Does NOT override Plugin Parameter "Shadows > Show" if off.
 * 
 * ---
 * 
 * Shadow Show: Followers
 * - Returns the visibility of follower sprite shadows.
 * - Does NOT override Plugin Parameter "Shadows > Show" if off.
 * 
 * ---
 * 
 * Shadow Show: All Events
 * - Returns the visibility of all event sprite shadows.
 * - Does NOT override Plugin Parameter or <Hide Shadow> notetag.
 * 
 * ---
 * 
 * === Spawn Event Plugin Commands ===
 * 
 * ---
 *
 * Spawn Event: Spawn At X, Y
 * - Spawns desired event at X, Y location on the current map.
 *
 *   Step 1:
 *
 *     Template Name:
 *     - Name of the target event template to spawn as.
 *     - Ignored if this is called "Untitled".
 *
 *     Map ID:
 *     - Target event's map to be used as reference.
 *     - You may use JavaScript code.
 *
 *     Event ID:
 *     - The ID of the target event to be used as reference.
 *     - You may use JavaScript code.
 *
 *   Step 2:
 *
 *     X Coordinate:
 *     - Target Location to spawn at.
 *     - You may use JavaScript code.
 *
 *     Y Coordinate:
 *     - Target Location to spawn at.
 *     - You may use JavaScript code.
 *
 *     Check Event Collision:
 *     - Check collision with any other events and player?
 *
 *     Check Passability:
 *     - Check passability of the target location.
 *
 *     Preserve Spawn:
 *     - Is the spawned event preserved?
 *     - Or does it expire upon leaving the map?
 *
 *   Step 3:
 *
 *     Success Switch ID:
 *     - Target switch ID to record spawning success.
 *     - Ignore if ID is 0. OFF means failed. ON means success.
 *
 * ---
 *
 * Spawn Event: Spawn At Region
 * - Spawns desired event at a random region-marked location on the
 *   current map.
 *
 *   Step 1:
 *
 *     Template Name:
 *     - Name of the target event template to spawn as.
 *     - Ignored if this is called "Untitled".
 *
 *     Map ID:
 *     - Target event's map to be used as reference.
 *     - You may use JavaScript code.
 *
 *     Event ID:
 *     - The ID of the target event to be used as reference.
 *     - You may use JavaScript code.
 *
 *   Step 2:
 *
 *     Region ID(s):
 *     - Pick region(s) to spawn this event at.
 *
 *     Check Event Collision:
 *     - Check collision with any other events and player?
 *
 *     Check Passability:
 *     - Check passability of the target location.
 *
 *     Preserve Spawn:
 *     - Is the spawned event preserved?
 *     - Or does it expire upon leaving the map?
 *
 *   Step 3:
 *
 *     Success Switch ID:
 *     - Target switch ID to record spawning success.
 *     - Ignore if ID is 0. OFF means failed. ON means success.
 *
 * ---
 *
 * Spawn Event: Spawn At Terrain Tag
 * - Spawns desired event at a random terrain tag-marked location on the
 *   current map.
 *
 *   Step 1:
 *
 *     Template Name:
 *     - Name of the target event template to spawn as.
 *     - Ignored if this is called "Untitled".
 *
 *     Map ID:
 *     - Target event's map to be used as reference.
 *     - You may use JavaScript code.
 *
 *     Event ID:
 *     - The ID of the target event to be used as reference.
 *     - You may use JavaScript code.
 *
 *   Step 2:
 *
 *     Terrain Tag(s):
 *     - Pick terrain tag(s) to spawn this event at.
 *     - Insert numbers between 0 and 7.
 *
 *     Check Event Collision:
 *     - Check collision with any other events and player?
 *
 *     Check Passability:
 *     - Check passability of the target location.
 *
 *     Preserve Spawn:
 *     - Is the spawned event preserved?
 *     - Or does it expire upon leaving the map?
 *
 *   Step 3:
 *
 *     Success Switch ID:
 *     - Target switch ID to record spawning success.
 *     - Ignore if ID is 0. OFF means failed. ON means success.
 *
 * ---
 *
 * Spawn Event: Despawn Event ID
 * - Despawns the selected Event ID on the current map.
 *
 *   Event ID
 *   - The ID of the target event.
 *   - You may use JavaScript code.
 *
 * ---
 *
 * Spawn Event: Despawn At X, Y
 * - Despawns any spawned event(s) at X, Y location on the current map.
 *
 *   X Coordinate:
 *   - Target Location to despawn at.
 *   - You may use JavaScript code.
 *
 *   Y Coordinate:
 *   - Target Location to despawn at.
 *   - You may use JavaScript code.
 *
 * ---
 *
 * Spawn Event: Despawn Region(s)
 * - Despawns the selected Region(s) on the current map.
 *
 *   Region ID(s):
 *   - Pick region(s) and despawn everything inside it.
 *
 * ---
 *
 * Spawn Event: Despawn Terrain Tag(s)
 * - Despawns the selected Terrain Tags(s) on the current map.
 *
 *   Terrain Tag(s):
 *   - Pick terrain tag(s) and despawn everything inside it.
 *   - Insert numbers between 0 and 7.
 *
 * ---
 *
 * Spawn Event: Despawn Everything
 * - Despawns all spawned events on the current map.
 *
 * ---
 *
 * ============================================================================
 * Move Route Custom Commands
 * ============================================================================
 *
 * Some custom commands have been added to the "Set Movement Route" event
 * command. These can be accessed by pressing the "Script..." command and
 * typing in the following, which don't need to be in code form.
 *
 * Keep in mind that since these are custom additions and RPG Maker MZ does not
 * allow plugins to modify the editor, the "Preview" button will not factor in
 * the effects of these commands.
 * 
 * If you wish to use a value from a variable, insert $gameVariables.value(x)
 * or \V[x] in place of the x in any of the below.
 * 
 * If you wish to use a value from a self variable, insert \SelfVar[x] in place
 * of the x in any of the below. This will only draw from the current event. If
 * you wish to draw data from outside event self variables, we recommend you
 * use the \V[x] variant after using the Plugin Commands to draw data from them
 * for the best accuracy.
 *
 * ---
 * 
 * Animation: x
 * - Replace 'x' with the ID of the animation to play on moving unit.
 *
 * ---
 * 
 * Balloon: name
 * - Replace 'name' with any of the following to play a balloon on that the
 *   target moving unit.
 * - '!', '?', 'Music Note', 'Heart', 'Anger', 'Sweat', 'Cobweb', 'Silence',
 *   'Light Bulb', 'Sleep', 'User-Defined 1', 'User-Defined 2',
 *   'User-Defined 3', 'User-Defined 4', 'User-Defined 5'
 *    - Do NOT insert quotes.
 * - Examples:
 *   - Balloon: !
 *   - Balloon: Sleep
 *   - Balloon: Heart
 *
 * ---
 * 
 * Fade In: x
 * Fade Out: x
 * - Fades in/out the sprite's opacity.
 * - Fade In will continuously raise the opacity level until it reaches 255.
 * - Fade Out will continuously lower the opacity level until it reaches 0.
 * - Replace 'x' with the speed to fade in/out the sprite.
 * 
 * ---
 * 
 * Force Carry: On
 * Force Carry: Off
 * - For usage with the VS8 sprite sheet.
 * - Use ON to turn force carrying on.
 * - Use OFF to turn force carrying off.
 * - Sprites using the VS8 sprite sheet will also show the VS8 Carry frames.
 * 
 * ---
 * 
 * Force Dash: On
 * Force Dash: Off
 * - Use ON to turn force dashing on.
 * - Use OFF to turn force dashing off.
 * - Forces dashing will prompt the player or event to be in the dashing state.
 * - Sprites using the VS8 sprite sheet will also show the VS8 Dashing frames.
 * 
 * ---
 * 
 * Hug: Left
 * Hug: Right
 * - Causes the moving unit to hug the left/right side of the wall.
 *
 * ---
 * 
 * Index: x
 * - Replace 'x' with a number depicting the character index to change the
 *   moving unit's sprite to.
 *
 * ---
 * 
 * Index: +x
 * Index: -x
 * - Replace 'x' with the value to change the character index of the moving
 *   unit's sprite by.
 *
 * ---
 * 
 * Jump Forward: x
 * - Replace 'x' with the number of tiles for the unit to jump forward by.
 *
 * ---
 * 
 * Jump To: x, y
 * - Replace 'x' and 'y' with the coordinates for the unit to jump to.
 *
 * ---
 * 
 * Jump to Event: x
 * - Replace 'x' with the ID of the event for the unit to jump to.
 *
 * ---
 * 
 * Jump to Player
 * - Causes the moving unit to jump to the player.
 *
 * ---
 * 
 * Jump To Home
 * - Causes the event to jump to its home position.
 * - This only works on events, not player characters or followers.
 * 
 * ---
 * 
 * Move Lower Left Until Stop
 * Move Down Until Stop
 * Move Lower Right Until Stop
 * Move Left Until Stop
 * Move Right Until Stop
 * Move Upper Left Until Stop
 * Move Up Until Stop
 * Move Upper Right Until Stop
 * - Causes the moving unit to move that direction until it hits a stop.
 * - Events will stop moving before they make contact with the player.
 *
 * ---
 * 
 * Crash Move Lower Left Until Stop
 * Crash Move Down Until Stop
 * Crash Move Lower Right Until Stop
 * Crash Move Left Until Stop
 * Crash Move Right Until Stop
 * Crash Move Upper Left Until Stop
 * Crash Move Up Until Stop
 * Crash Move Upper Right Until Stop
 * - Causes the moving unit to move that direction until it hits a stop.
 * - Events can crash into the player and trigger an event.
 *
 * ---
 * 
 * Move To: x, y
 * - Replace 'x' and 'y' with the map coordinates to move the unit to through
 *   pathfinding.
 * - This uses RPG Maker MZ's pathfinding algorithm. It is not perfect so do
 *   not expect the most optimal results.
 * - Events will go around the player.
 *
 * ---
 * 
 * Crash Move To: x, y
 * - Replace 'x' and 'y' with the map coordinates to move the unit to through
 *   pathfinding.
 * - This uses RPG Maker MZ's pathfinding algorithm. It is not perfect so do
 *   not expect the most optimal results.
 * - Events can crash into the player and trigger an event.
 *
 * ---
 * 
 * Move to Event: x
 * - Replace 'x' with the ID of the event to move the unit to.
 * - This uses RPG Maker MZ's pathfinding algorithm. It is not perfect so do
 *   not expect the most optimal results.
 * - Events will go around the player.
 *
 * ---
 * 
 * Crash Move to Event: x
 * - Replace 'x' with the ID of the event to move the unit to.
 * - This uses RPG Maker MZ's pathfinding algorithm. It is not perfect so do
 *   not expect the most optimal results.
 * - Events can crash into the player and trigger an event.
 *
 * ---
 * 
 * Move to Player
 * - Moves the unit to the player.
 * - This uses RPG Maker MZ's pathfinding algorithm. It is not perfect so do
 *   not expect the most optimal results.
 *
 * ---
 * 
 * Move to Home
 * - Moves the unit towards their home position on the map.
 * - This only works on events, not player characters or followers.
 * - This uses RPG Maker MZ's pathfinding algorithm. It is not perfect so do
 *   not expect the most optimal results.
 * 
 * ---
 * 
 * Crash Move to Home
 * - Moves the unit towards their home position on the map.
 * - This only works on events, not player characters or followers.
 * - This uses RPG Maker MZ's pathfinding algorithm. It is not perfect so do
 *   not expect the most optimal results.
 * - Events can crash into the player and trigger an event.
 * 
 * ---
 * 
 * Move Lower Left: x
 * Move Down: x
 * Move Lower Right: x
 * Move Left: x
 * Move Right: x
 * Move Upper Left: x
 * Move Up: x
 * Move Upper Right: x
 * - Replace 'x' with the number of times to move the unit by in the designated
 *   direction on the map.
 * - Events can crash into the player and trigger an event.
 *
 * ---
 * 
 * Opacity: x%
 * - Replace 'x' with the percentage to change the unit's sprite opacity to.
 *
 * ---
 * 
 * Opacity: +x
 * Opacity: -x
 * - Replace 'x' with the increment to change the unit's sprite opacity by.
 *
 * ---
 *
 * Pattern Lock: x
 * - Replace 'x' with the step pattern to lock the unit's sprite to.
 *
 * ---
 *
 * Pattern Unlock
 * - Removes pattern lock effect.
 *
 * ---
 * 
 * Pose: name
 * - If using a VS8 sprite, this will cause the unit to strike a pose.
 * - Replace 'name' with any the following:
 * - 'Item', 'Hmph', 'Victory', 'Hurt', 'Kneel', 'Collapse',
 *   '!', '?', 'Music Note', 'Heart', 'Anger', 'Sweat', 'Cobweb', 'Silence',
 *   'Light Bulb', 'Sleep'
 *    - Do NOT insert quotes.
 * - Examples:
 *   - Balloon: Item
 *   - Balloon: Victory
 *   - Balloon: ?
 *
 * ---
 * 
 * Step Toward: x, y
 * - Replace 'x' and 'y' for the desired coordinates for the unit to take one
 *   step towards.
 * - This uses RPG Maker MZ's pathfinding algorithm. It is not perfect so do
 *   not expect the most optimal results.
 *
 * ---
 * 
 * Step Toward Event: x
 * - Replace 'x' with the ID of the event for the unit to take one step to.
 * - This uses RPG Maker MZ's pathfinding algorithm. It is not perfect so do
 *   not expect the most optimal results.
 *
 * ---
 * 
 * Step Toward Player
 * - Causes event to take one step towards the player.
 * - This uses RPG Maker MZ's pathfinding algorithm. It is not perfect so do
 *   not expect the most optimal results.
 *
 * ---
 * 
 * Step Toward Home
 * - Causes the event to take one step towards its home position.
 * - This only works on events, not player characters or followers.
 * - This uses RPG Maker MZ's pathfinding algorithm. It is not perfect so do
 *   not expect the most optimal results.
 * 
 * ---
 * 
 * Step Away From: x, y
 * - Replace 'x' and 'y' for the desired coordinates for the unit to take one
 *   step away from.
 * - This uses RPG Maker MZ's pathfinding algorithm. It is not perfect so do
 *   not expect the most optimal results.
 *
 * ---
 * 
 * Step Away From Event: x
 * - Replace 'x' with the ID of the event for the unit to take one step from.
 * - This uses RPG Maker MZ's pathfinding algorithm. It is not perfect so do
 *   not expect the most optimal results.
 *
 * ---
 * 
 * Step Away From Player
 * - Causes event to take one step away from the player.
 * - This uses RPG Maker MZ's pathfinding algorithm. It is not perfect so do
 *   not expect the most optimal results.
 *
 * ---
 * 
 * Step Away From Home
 * - Causes the event to take one step away from its home position.
 * - This only works on events, not player characters or followers.
 * - This uses RPG Maker MZ's pathfinding algorithm. It is not perfect so do
 *   not expect the most optimal results.
 * 
 * ---
 * 
 * Turn To: x, y
 * - Replace 'x' and 'y' for the coordinates to make the unit face towards.
 * - This supports 8 directional turning.
 *
 * ---
 * 
 * Turn to Event: x
 * - Replace 'x' with the ID of the event to turn the unit towards.
 * - This supports 8 directional turning.
 *
 * ---
 * 
 * Turn to Player
 * - Causes the unit to turn towards the player.
 * - This supports 8 directional turning.
 *
 * ---
 * 
 * Turn to Home
 * - Causes the event to turn towards its home position.
 * - This refers to the original position's X/Y on the map.
 * - The event will turn and face the tile that is its original X/Y location.
 * - This only works on events, not player characters or followers.
 * 
 * ---
 * 
 * Turn Away From: x, y
 * - Replace 'x' and 'y' for the coordinates to make the unit face away from.
 * - This supports 8 directional turning.
 *
 * ---
 * 
 * Turn Away From Event: x
 * - Replace 'x' with the ID of the event to turn the unit away from.
 * - This supports 8 directional turning.
 *
 * ---
 * 
 * Turn Away From Player
 * - Causes the unit to turn away from the player.
 * - This supports 8 directional turning.
 *
 * ---
 * 
 * Turn Away From Home
 * - Causes the event to turn away from its home position.
 * - This only works on events, not player characters or followers.
 * 
 * ---
 * 
 * Turn Lower Left
 * Turn Lower Right
 * Turn Upper Left
 * Turn Upper Right
 * - Causes the unit to turn to one of the diagonal directions.
 *
 * ---
 * 
 * Self Switch x: On
 * Self Switch x: Off
 * Self Switch x: Toggle
 * - Replace 'x' with 'A', 'B', 'C', 'D', or a <Self> Switch ID to adjust the
 *   unit's Self Switch.
 *
 * ---
 * 
 * Self Variable x: y
 * - Replace 'x' with a <Self> Variable ID to adjust the unit's Self Variable.
 * - Replace 'y' with a number value to set the Self Variable to.
 *
 * ---
 * 
 * Teleport To: x, y
 * - Replace 'x' and 'y' with the coordinates to instantly move the unit to.
 *
 * ---
 * 
 * Teleport to Event: x
 * - Replace 'x' with the ID of the event to instantly move the unit to.
 *
 * ---
 * 
 * Teleport to Player
 * - Instantly moves the unit to the player's location.
 *
 * ---
 * 
 * Teleport to Home
 * - Instantly teleports an event to its home position on the map.
 * - This only works on events, not player characters or followers.
 * 
 * ---
 * 
 * If none of the commands are detected above, then a script call will be ran.
 *
 * ---
 *
 * ============================================================================
 * Plugin Parameters: Event Label Settings
 * ============================================================================
 *
 * Event Labels are small windows created to display text over an event's head.
 * They're set up using the <Label> notetags and/or comment tags. Event Labels
 * are a great way to instantly relay information about the event's role to
 * the player.
 *
 * ---
 *
 * Event Labels
 * 
 *   Sprite Based?:
 *   - Use sprite-based labels instead of legacy-window version.
 *   - Legacy-window version will not be supported in future.
 *   - Sprite-based labels are more memory efficient and work better
 *     compatibility-wise.
 * 
 *   Mobile-Enabled?:
 *   - Enable event labels for mobile devices?
 * 
 *   Font Size:
 *   - The font size used for the Event Labels.
 * 
 *   Icon Size:
 *   - The size of the icons used in the Event Labels.
 * 
 *   Line Height:
 *   - The line height used for the Event Labels.
 * 
 *   Offset X:
 *   - Globally offset all labels horizontally by this amount.
 * 
 *   Offset Y:
 *   - Globally offset all labels vertically by this amount.
 * 
 *   Fade Speed:
 *   - Fade speed for labels.
 * 
 *   Visible Range:
 *   - Range the player has to be within the event to make its label visible.
 * 
 *     Range Type:
 *     - What do you want the default label visible range type?
 *       - Square
 *       - Diamond
 *       - Circle
 *
 * ---
 *
 * ============================================================================
 * Plugin Parameters: Event Icon Settings
 * ============================================================================
 *
 * Icons can be displayed over an event's head through the <Icon> notetags
 * and/or comment tags. These can be used for a variety of things such as
 * making them look like they're carrying an item or to indicate they have a
 * specific role.
 *
 * ---
 *
 * Event Icon
 * 
 *   Buffer X:
 *   - Default X position buffer for event icons.
 * 
 *   Buffer Y:
 *   - Default Y position buffer for event icons.
 * 
 *   Blend Mode:
 *   - Default blend mode for even icons.
 *     - 0 - Normal
 *     - 1 - Additive
 *     - 2 - Multiply
 *     - 3 - Screen
 *
 * ---
 *
 * ============================================================================
 * Plugin Parameters: Event Template Settings
 * ============================================================================
 *
 * Event Templates allow you to store specific maps and/or event data to bring
 * out on need while having a premade set base. They're similar to prefabs but
 * aren't things that can be altered individually as one setting for an event
 * template will serve as a blueprint for all of them that use them.
 *
 * Event Templates are used for the <Copy Event> notetags, the Morph Event and
 * Spawn Event Plugin Commands.
 *
 * ---
 *
 * Settings
 * 
 *   Preloaded Maps:
 *   - A list of all the ID's of the maps that will be preloaded to serve as
 *     template maps for this plugin.
 *
 * ---
 *
 * Templates
 * - A list of all the Event Templates used by this project. Used for notetags
 *   and Plugin Commands.
 * 
 *     Name:
 *     - Name of the template. It'll be used as anchor points for notetags and
 *       Plugin Commands.
 * 
 *     Map ID:
 *     - ID of the map the template event is stored on.
 *     - This will automatically add this ID to preloaded list.
 * 
 *     Event ID:
 *     - ID of the event the template event is based on.
 * 
 *     JavaScript:
 *       JS: Pre-Copy:
 *       JS: Post-Copy:
 *       JS: Pre-Morph:
 *       JS: Post-Morph:
 *       JS: Pre-Spawn:
 *       JS: Post-Spawn:
 *       - Code that's ran during certain circumstances.
 *       - The code will occur at the same time as the ones listed in the main
 *         Event Template Settings Plugin Parameters. However, the ones listed
 *         in these individual entries will only occur for these specific
 *         templates and only if the templates are used.
 *
 * ---
 *
 * JavaScript
 * 
 *   JS: Pre-Copy:
 *   JS: Post-Copy:
 *   JS: Pre-Morph:
 *   JS: Post-Morph:
 *   JS: Pre-Spawn:
 *   JS: Post-Spawn:
 *   - Code that's ran during certain circumstances.
 *   - These are global and are ran for all copies, morphs, and/or spawns.
 *
 * ---
 *
 * ============================================================================
 * Plugin Parameters: Movement Settings
 * ============================================================================
 *
 * These plugin parameters allow you to control how movement works in your
 * game, toggling it from 4-directional to 8-directional, setting up rules to
 * stop self-movement from events while an event or message is present, and
 * other aesthetics such as tilting the sprite while dashing, setting shadows
 * beneath the sprites, and allow for turning in place.
 * 
 * Shadows do NOT appear for sprites using a "!" as their leading filename
 * marker. These sprites are environmental and are considered "object"
 * characters by the RPG Maker MZ core scripts. They do not utilize character
 * shadows due.
 *
 * ---
 *
 * 8 Directional Movement
 * 
 *   Enable:
 *   - Allow 8-directional movement by default? Players can move diagonally.
 * 
 *   Strict Collision:
 *   - Enforce strict collission rules where the player must be able to pass
 *     both cardinal directions?
 * 
 *   Favor Horizontal:
 *   - Favor horizontal if cannot pass diagonally but can pass both
 *     horizontally and vertically?
 * 
 *   Slower Diagonals?
 *   - Enforce a slower movement speed when moving diagonally?
 * 
 *     Speed Multiplier
 *     - What's the multiplier to adjust movement speed when moving diagonally?
 *
 * ---
 *
 * Automatic Movement
 * 
 *   Stop During Events:
 *   - Stop automatic event movement while events are running.
 * 
 *   Stop During Messages:
 *   - Stop automatic event movement while a message is running.
 *
 * ---
 * 
 * Bitmap
 * 
 *   Smoothing:
 *   - Do you want to smooth or pixelate the map sprites?
 *   - Pixelating them is better for zooming and tilting.
 * 
 * ---
 *
 * Dash
 * 
 *   Dash Modifier:
 *   - Alters the dash speed modifier.
 * 
 *   Dash on Ladder?
 *   - Allow dashing while on a ladder or rope?
 * 
 *   Enable Dash Tilt?:
 *   - Tilt any sprites that are currently dashing?
 * 
 *     Tilt Left Amount:
 *     - Amount in radians when moving left (upper left, left, lower left).
 * 
 *     Tilt Right Amount:
 *     - Amount in radians when moving right (upper right, right, lower right).
 * 
 *     Tilt Vertical Amount:
 *     - Amount in radians when moving vertical (up, down).
 *
 * ---
 * 
 * Event Movement
 * 
 *   Random Move Weight:
 *   - Use numbers between 0 and 1.
 *   - Numbers closer to 1 stay closer to their home position.
 *   - 0 to disable it.
 * 
 *   Shift Y:
 *   - How many pixels should non-tile characters be shifted by?
 *   - Negative: up. Positive: down.
 * 
 * ---
 * 
 * Path Finding
 * 
 *   Mobile-Enabled?:
 *   - Enable diagonal pathfinding for mobile devices?
 * 
 * ---
 *
 * Shadows
 * 
 *   Show:
 *   - Show shadows on all events and player-related sprites.
 * 
 *   Default Filename:
 *   - Default filename used for shadows found in img/system/ folder.
 * 
 *   Shadow Z Layer:
 *   - What is the sprite Z layer used for the shadow sprites?
 *     - In-game layers are as follows:
 *     - 0 : Lower tiles
 *     - 1 : Lower characters
 *     - 3 : Normal characters
 *     - 4 : Upper tiles
 *     - 5 : Upper characters
 *     - 6 : Airship shadow
 *     - 7 : Balloon
 *     - 8 : Animation
 *     - 9 : Destination
 *
 * ---
 *
 * Turn in Place
 * 
 *   Enable:
 *   - When not dashing, player will turn in place before moving.
 *   - This only applies with keyboard inputs.
 * 
 *   Delay in Frames:
 *   - The number of frames to wait before moving.
 *
 * ---
 * 
 * Vehicle Speeds
 * 
 *   Boat Speed:
 *   - Allows you to adjust the base speed of the boat vehicle.
 * 
 *   Ship Speed:
 *   - Allows you to adjust the base speed of the ship vehicle.
 * 
 *   Airship Speed:
 *   - Allows you to adjust the base speed of the airship vehicle.
 * 
 * ---
 * 
 * Wall Bump
 * 
 *   Enable?:
 *   - Enable the sound effect to be played when bumping into a wall?
 * 
 * ---
 *
 * ============================================================================
 * Plugin Parameters: VisuStella 8-Dir Settings
 * ============================================================================
 *
 * These are settings for sprite sheets using the VS8 format.
 * For more information on the VS8 format, look in the help section above.
 *
 * ---
 *
 * Balloon Icon Settings
 * 
 *   Auto-Balloon Poses:
 *   - Automatically pose VS8 sprites when using balloon icons.
 * 
 *   Balloon Offset X:
 *   - Offset balloon icons on VS8 sprites by x pixels.
 * 
 *   Balloon Offset Y:
 *   - Offset balloon icons on VS8 sprites by y pixels.
 *
 * ---
 *
 * Icons
 * 
 *   Auto Buffer:
 *   - Automatically buffer the X and Y coordinates of VS8 sprites?
 * 
 *   Use Carry Pose:
 *   - Use the carry pose when moving with an icon overhead.
 *
 * ---
 *
 * ============================================================================
 * Plugin Parameters: Region Rulings
 * ============================================================================
 *
 * These settings allow you to decide the passability of the player, events,
 * and various vehicles through the usage of Regions.
 *
 * ---
 *
 * Allow Regions
 * 
 *   All Allow:
 *   Walk Allow:
 *   Player Allow:
 *   Event Allow:
 *   Vehicle Allow:
 *   Boat Allow:
 *   Ship Allow:
 *   Airship Allow:
 *   - Insert Region ID's where the affected unit type can enter.
 *   - Region ID's range from 0 to 255.
 *
 * ---
 *
 * Forbid Regions
 * 
 *   All Forbid:
 *   Walk Forbid:
 *   Player Forbid:
 *   Event Forbid:
 *   Vehicle Forbid:
 *   Boat Forbid:
 *   Ship Forbid:
 *   Airship Forbid:
 *   - Insert Region ID's where the affected unit type cannot enter.
 *   - Region ID's range from 0 to 255.
 *
 * ---
 *
 * Dock Regions
 * 
 *   Vehicle Dock:
 *   Boat Dock:
 *   Ship Dock:
 *   Airship Dock:
 *   - Insert Region ID's where the affected vehicle can dock
 *   - Region ID's range from 0 to 255.
 * 
 *   Only Region Dockable:
 *   - Vehicles are only able to dock at designated regions.
 *
 * ---
 *
 * ============================================================================
 * Plugin Parameters: Common Event on OK Button
 * ============================================================================
 *
 * These Plugin Parameters allow you to setup Common Events that activate using
 * Regions when pressing the OK button while standing on top of them or in
 * front of them. These let you create near universally interactable objects
 * using Regions, such as rivers to start up fishing events or locations to
 * places items on.
 *
 * ---
 *
 * Regions
 * 
 *   Regions 1 - 255:
 *   - Which Common Event does this region activate?
 *   - Use 0 to not activate any Common Events.
 *
 * ---
 *
 * Target Tile
 * 
 *   Target Tile:
 *   - Which tile should be checked for Common Event on OK Button?
 *     - Tile in front of player.
 *     - Tile player is standing on top of.
 *
 * ---
 *
 * ============================================================================
 * Plugin Parameters: Common Event on Touch
 * ============================================================================
 *
 * These Plugin Parameters allow you to setup Common Events that trigger when
 * stepping onto Region-marked tiles. These let you create custom effects that
 * will occur such as customized damage floors, traps, and/or events.
 * 
 * Areas marked with these regions will not allow random encounters to occur.
 * This is how RPG Maker works. Assuming you are not using plugins at all, by
 * putting on touch events all over the map, tiles with those on touch events
 * will not let random encounters trigger.
 *
 * ---
 *
 * Regions
 * 
 *   Regions 1 - 255:
 *   - Which Common Event does this region activate?
 *   - Use 0 to not activate any Common Events.
 *
 * ---
 *
 * ============================================================================
 * Plugin Parameters: Terrain Tag Settings
 * ============================================================================
 *
 * Terrain Tags are used in Database => Tilesets to mark certain tiles and
 * give them unique properties through terrain tags.
 *
 * ---
 *
 * Terrain Tag ID's
 * 
 *   Rope:
 *   - Which terrain tag number to use for ropes?
 *
 * ---
 *
 * ============================================================================
 * Terms of Use
 * ============================================================================
 *
 * 1. These plugins may be used in free or commercial games provided that they
 * have been acquired through legitimate means at VisuStella.com and/or any
 * other official approved VisuStella sources. Exceptions and special
 * circumstances that may prohibit usage will be listed on VisuStella.com.
 * 
 * 2. All of the listed coders found in the Credits section of this plugin must
 * be given credit in your games or credited as a collective under the name:
 * "VisuStella".
 * 
 * 3. You may edit the source code to suit your needs, so long as you do not
 * claim the source code belongs to you. VisuStella also does not take
 * responsibility for the plugin if any changes have been made to the plugin's
 * code, nor does VisuStella take responsibility for user-provided custom code
 * used for custom control effects including advanced JavaScript notetags
 * and/or plugin parameters that allow custom JavaScript code.
 * 
 * 4. You may NOT redistribute these plugins nor take code from this plugin to
 * use as your own. These plugins and their code are only to be downloaded from
 * VisuStella.com and other official/approved VisuStella sources. A list of
 * official/approved sources can also be found on VisuStella.com.
 *
 * 5. VisuStella is not responsible for problems found in your game due to
 * unintended usage, incompatibility problems with plugins outside of the
 * VisuStella MZ library, plugin versions that aren't up to date, nor
 * responsible for the proper working of compatibility patches made by any
 * third parties. VisuStella is not responsible for errors caused by any
 * user-provided custom code used for custom control effects including advanced
 * JavaScript notetags and/or plugin parameters that allow JavaScript code.
 *
 * 6. If a compatibility patch needs to be made through a third party that is
 * unaffiliated with VisuStella that involves using code from the VisuStella MZ
 * library, contact must be made with a member from VisuStella and have it
 * approved. The patch would be placed on VisuStella.com as a free download
 * to the public. Such patches cannot be sold for monetary gain, including
 * commissions, crowdfunding, and/or donations.
 *
 * ============================================================================
 * Credits
 * ============================================================================
 * 
 * If you are using this plugin, credit the following people in your game:
 * 
 * Team VisuStella
 * * Yanfly
 * * Arisu
 * * Olivia
 * * Irina
 *
 * ============================================================================
 * Changelog
 * ============================================================================
 * 
 * Version 1.64: March 16, 2026
 * * Bug Fixes!
 * ** Click + hold movement no longer crashes when moving over events without
 *    any active pages. Fix made by Arisu.
 * 
 * Version 1.63: February 16, 2026
 * * Feature Update!
 * ** When moving by clicking and holding down the mouse button, the touch
 *    input will no longer lock onto any events with empty event lists or
 *    populated by just comments. Update made by Arisu.
 * 
 * Version 1.62: January 19, 2026
 * * Documentation Update!
 * ** Help file updated for new features.
 * ** New section added: Reference Switches and Reference Variables
 * *** Reference Switches and Reference Variables are added in version 1.62 of
 *     this plugin. These switches and variables allow you to reference them
 *     through strings when using script calls.
 * *** By simply naming your switch or variable ((Reference Name)), you can use
 *     that 'reference name' in a string to call them for script calls.
 * *** When referencing the strings in the script calls, case does not matter,
 *     which means you can use all capitals or all lower case and they'll still
 *     reference the same switch or variable.
 * **** Example: Switch 10 Name: ((Priscilla Joined))
 * ***** Script Call: $gameSwitches.value('Priscilla Joined')
 * ***** $gameSwitches.setValue('Priscilla Joined', true)
 * **** Variable 20 Name: Total ((Goblins Slain))
 * ***** $gameVariables.value('Goblins Slain')
 * ***** $gameVariables.setValue('Goblins Slain', 50)
 * *** Remember to put quotes around the name for the script call!
 * *** This only applies for the $gameSwitches and $gameVariables functions of
 *     value(id) and setValue(id, value). They do not apply to the other
 *     $gameSwitches and $gameVariables functions.
 * * New Features!
 * ** Added Reference Switches and Reference Variables
 * *** See Help section for more info about Reference Switches and Variables
 * ** New Plugin Commands added:
 * *** Shadow Hide: Player
 * *** Shadow Hide: Followers
 * *** Shadow Hide: All Events
 * **** Hides the visibility of the target sprite shadow.
 * *** Shadow Show: Player
 * *** Shadow Show: Followers
 * *** Shadow Show: All Events
 * **** Returns the visibility of target sprite shadow.
 * **** Does NOT override Plugin Parameter "Shadows > Show" if off.
 * **** Does NOT override <Hide Shadow> notetag.
 * 
 * Version 1.61: December 15, 2025
 * * Bug Fixes!
 * ** Fixed a bug where shadows would appear under lower-priority event sprites
 *    making usage of certain tiles awkward looking. This is corrected by the
 *    new Plugin Parameter. Fix made by Arisu.
 * * Documentation Update!
 * ** Help file updated for new features.
 * * New Feature!
 * ** New Plugin Parameter added by Arisu:
 * *** Parameters > Movement Settings > Shadows > Shadow Z Layer
 * **** What is the sprite Z layer used for the shadow sprites?
 * **** By default, this layer will now be 0.5 instead of 0.
 * * Feature Update!
 * ** If a event is made whose priority is "Below characters" and is a tile
 *    object (ie taking a sprite from the map tileset or a character sprite
 *    with "!" in front of the name), it will be automatically regulated to
 *    a custom Z layer of 0.
 * 
 * Version 1.60: August 29, 2024
 * * Bug Fixes!
 * ** Fixed a bug where events with large hitboxes do not work with crash move.
 *    Fix made by Arisu.
 * ** Fixed a bug where single-mode save games by Save Core would freeze after
 *    executed event movements. Fix made by Arisu.
 * * Documentation Update!
 * ** Help file updated for new features.
 * * Feature Update!
 * ** Event Labels will adjust their vertical position to the picture of any
 *    attached event picture if one is present. Update by Arisu.
 * * New Features!
 * ** New Notetags added by Arisu:
 * *** <Picture Type: Enemy>
 * *** <Picture Type: SV Enemy>
 * **** Will use /img/enemies/ or /img/sv_enemies/ instead of /img/pictures/ to
 *      grab a picture graphic from.
 * **** Other picture graphic sprite related notetags will apply as normal.
 * *** <Label Range Type: Square>
 * *** <Label Range Type: Circle>
 * *** <Label Range Type: Diamond>
 * **** Sets a range type for the label to appear visible for.
 * ** New Plugin Parameters added by Arisu:
 * *** Parameters > Event Label Settings > Visible Range > Range Type:
 * **** What do you want the default label visible range type?
 * 
 * Version 1.59: June 13, 2024
 * * Bug Fixes!
 * ** Added a cache check for character sprite tag names to reduce frame drops.
 *    Fix made by Arisu.
 * * Documentation Update!
 * ** Help file updated for new features.
 * * New Features!
 * ** New notetags added by Arisu:
 * *** <Location X: +x>, <Location X: -x>
 * *** <Location Y: +y>, <Location Y: -y>
 * *** <Location: +x, +y>, <Location: +x, -y>
 * *** <Location: -x, +y>, <Location: -x, -y>
 * **** Adjusts the initial location of this event by +x and +y (or -x and -y).
 * **** This allows you to stack events on top of each other or even move them
 *      to various places of the map.
 * *** <Tile Expand Up: x>
 * *** <Tile Expand Down: x>
 * *** <Tile Expand Left: x>
 * *** <Tile Expand Right: x>
 * **** Used for events with tile graphics. Expands the graphic up, down, left,
 *      or right from the spritesheet.
 * **** This does NOT expand the hitbox.
 * **** The graphic will be anchored to the tile it's expanded from. This means
 *      even if you expanded downward, the actual event's position will still
 *      be the current event's X/Y coordinates. It's just grown more vertically
 *      and is still centered horizontally.
 * **** This is primarily used to save on having to use too many events for
 *      tiles that expanded past 1x1 tile sizes.
 * 
 * Version 1.58: May 16, 2024
 * * Documentation Update!
 * ** Added "Features: Character Sprite Filename Tags" section.
 * * New Features!
 * ** [Invisible] tag added to character sprite filenames.
 * *** If a character sprite's filename has [invisible] in it, it will become
 *     invisible on the map screen in-game while almost everything else about
 *     it is visible. This is used for those who wish to use sprite labels for
 *     things such as autorun and parallel events.
 * 
 * Version 1.57: March 14, 2024
 * * Documentation Update!
 * ** Help file updated for new features.
 * * Feature Update!
 * ** Plugin Command: "Event Icon: Delete" will now keep an event icon cleared
 *    until the newly added Plugin Command: "Event Icon: Restore" is used.
 *    Update made by Arisu.
 * ** Plugin Command: "Event Icon: Change" is now renamed to have "(Temporary)"
 *    after its name in order to clarify the temporary changes made to it.
 * * New Features!
 * ** New Plugin Command added by Arisu:
 * *** Event Icon: Event Icon: Change (Forced)
 * **** Change the icon that appears on an event.
 * **** This change is forced and needs to be restored.
 * *** Event Icon: Restore
 * **** Restores a deleted or forced icon that appears on an event.
 * 
 * Version 1.56: February 15, 2024
 * * Documentation Update!
 * ** Help file updated for new features.
 * * Feature Update!
 * ** Added fail safes for activation proximity notetags when loaded from past
 *    save files without Events and Movement Core installed. Added by Arisu.
 * * New Features!
 * ** New notetags added by Arisu:
 * *** <Encounter Half Square: x>
 * *** <Encounter Half Circle: x>
 * *** <Encounter Half Delta: x>
 * *** <Encounter Half Row: x>
 * *** <Encounter Half Column: x>
 * *** <Encounter None Square: x>
 * *** <Encounter None Circle: x>
 * *** <Encounter None Delta: x>
 * *** <Encounter None Row: x>
 * *** <Encounter None Column: x>
 * **** If the player is within the 'x' area effect of this event, the random
 *      encounter rate will be halved or suppressed completely depending on the
 *      notetag used.
 * **** These include script call checks.
 * *** <Erase if Encounter Half>
 * *** <Erase if Encounter None>
 * **** Automatically erase this event if the player's party has an encounter
 *      half or encounter none effect, or if the event has spawned in an
 *      encounter half or encounter none area.
 * **** This check only occurs in two situations: when the map is first loaded
 *      after being teleported into or when the player leaves a menu and
 *      returns back to the map.
 * **** Events that have been erased due to this effect will NOT return even if
 *      the encounter half/none effect is removed while the player is still on
 *      the map. The event will return if the player exits the map and comes
 *      back.
 * 
 * Version 1.55: December 14, 2023
 * * Documentation Update!
 * ** Help file updated for new features.
 * * New Features!
 * ** New Plugin Commands added by Arisu:
 * *** Event Popup: Player
 * *** Event Popup: Follower
 * *** Event Popup: Event
 * *** Event Popup: Target Tile
 * **** Makes a centered event popup on the player sprite, target follower
 *      sprite, target event sprite, or target tile.
 * **** All of these new Plugin Commands require VisuMZ_1_MessageCore and
 *      cannot be used in battle.
 * 
 * Version 1.54: October 12, 2023
 * * Documentation Update!
 * ** Help file updated for new features.
 * ** Help file updated to reduce confusion:
 * *** Call Event: Remote Read
 * **** This will run the page of the target event on the current event.
 * **** This means that any "This Event" commands will be applied to the event
 *      using this Plugin Command and NOT the target event that page data is
 *      being retrieved from.
 * **** Think of it as the current event using the target called event as a
 *      Common Event ala how RPG Maker 2003 works (for those familiar with it).
 * * Feature Update!
 * ** Renamed "Call Event: Remote Activation" to "Call Event: Remote Read" to
 *    reduce confusion.
 * * Feature Update!
 * ** <Activation Radius: x> notetag is now defined as <Activation Delta: x>
 * *** 'Radius' variant will still work and function as 'Delta' but will no
 *     longer be listed in the help file as 'Radius'
 * *** This is changed to avoid confusion with the new notetag.
 * * New Features!
 * ** New notetag added by Arisu and sponsored by AndyL:
 * *** <Activation Circle: x>
 * **** A circle-shaped range with the event at the center.
 * **** 'x' represents the distance from the center.
 * 
 * Version 1.53: August 17, 2023
 * * Documentation Update!
 * ** Help file updated for new features.
 * * New Features!
 * ** <Map Load Common Event: x>
 * ** <Map Load Common Events: x, x, x>
 * *** When this map is loaded, run the specified Common Events once available.
 * **** Does NOT trigger if you transfer to a different part of the same map.
 * 
 * Version 1.52: July 13, 2023
 * * Documentation Update!
 * ** Help file updated for new features.
 * ** Updated help file for <Label: text> notetags:
 * *** If text codes are used, avoid text codes that use < and > wrappers.
 * ** Updated help file for <Label> sandwich notetags:
 * *** You can use text codes with < and > wrappers.
 * * Feature Update!
 * ** Event labels now work properly with scaling sprites.
 * * New Features!
 * ** New notetag added by Arisu and sponsored by Anon:
 * *** <Label Hue Shift: +x>
 * *** <Label Hue Shift: -x>
 * **** Changes the hue of the event label by +x or -x every frame.
 * **** Keep in mind that since this is changing hue, this will appear to have
 *      no effect if you are using black and white labels.
 * **** Use labels with text codes that add color to them like '\C[4]text'
 * **** This only works with the sprite version of event labels and does not
 *      work with the legacy version.
 * 
 * Version 1.51: June 15, 2023
 * * Bug Fixes!
 * ** Provided a fail safe for plugins using the scaling options from this
 *    plugin but do not have scaling parameters identified. The scaling ratio
 *    should now default to 1.0. Fix made by Olivia.
 * * Feature Update!
 * ** Diagonal pathfinding is now improved as to not get stuck on tight corners
 *    on the map. Feature update made by Arisu.
 * 
 * Version 1.50: April 13, 2023
 * * Bug Fixes!
 * ** <Icon: x> should now update correctly when changing pages through self
 *    switches or other event conditions. Fix made by Arisu.
 * * Documentation Update!
 * ** Help file updated for new features.
 * * New Features!
 * ** New Plugin Parameters added by Arisu:
 * *** Plugin Parameters > Event Labels > Mobile-Enabled?
 * *** Plugin Parameters > Movement > Pathfinding > Mobile-Enabled?
 * **** These settings allow you to enable or disable certain features when
 *      played on mobile devices for better performance.
 * 
 * Version 1.49: March 16, 2023
 * * Documentation Update!
 * ** Help file updated for new features.
 * * New Features!
 * ** New Event Notetag and Comment Tags added by Arisu:
 * *** <Scale: x%>
 * *** <Scale X: x%>
 * *** <Scale Y: y%>
 * **** Changes the scale of the sprite to the designated size.
 * 
 * Version 1.48: January 20, 2023
 * * Feature Update!
 * ** <Move Synch> for certain types will also copy facing directions even if
 *    there are no tile movements (ie changing directions when pressed up
 *    against and obstacle). Update made by Arisu.
 * 
 * Version 1.47: November 10, 2022
 * * Feature Update!
 * ** If "Follower: Set Global Chase" is set to false, followers will no longer
 *    jump towards the player location when the player jumps. This does NOT
 *    apply to gather or location changing players. Followers will still have
 *    to synchronize their positions there regardless in order to maintain
 *    consistency. Update made by Olivia.
 * 
 * Version 1.46: September 29, 2022
 * * Bug Fixes!
 * ** Altered the self switch auto-reset timing to reduce errors. Fix by Arisu.
 * * Feature Update!
 * ** Added self-movement prevention whenever scenes are deactivated. Update
 *    made by Arisu.
 * 
 * Version 1.45: August 18, 2022
 * * Bug Fixes!
 * ** Fixed a bug that caused event labels with variables from refreshing
 *    properly. Fix made by Arisu.
 * 
 * Version 1.44: July 21, 2022
 * * Bug Fixes!
 * ** Fixed a problem that caused <Exit Reset Self Data> notetag to not work.
 *    Fix made by Arisu.
 * * Feature Update!
 * ** Diagonal pathfinding is now disabled when there are too many events on a
 *    map, causing extra collission checks. This value is set to 100 for the
 *    time being until we can figure out a better way to calculate diagonal
 *    pathfinding. Update made by Irina.
 * 
 * Version 1.43: July 14, 2022
 * * Bug Fixes!
 * ** Move to Player for events should no longer cause hang ups. Fix by Olivia.
 * * Documentation Update!
 * ** Help file updated for new features.
 * * Feature Update!
 * ** Added caching function for pathfinding when using touch movement for a
 *    smoother experience. When touch movement is held down, pathfinding will
 *    utilize the non-diagonal function for less resource consumption to
 *    prevent FPS frame drops. Update made by Arisu.
 * * New Features!
 * ** New notetag added by Arisu:
 * *** <Playtest>
 * **** If this notetag is found in the event's notebox (NOT comments), then
 *      the event will only appear during a playtest session. It will not
 *      appear in a deployed game where the playtest flag is not on.
 * 
 * Version 1.42: June 23, 2022
 * * Documentation Update!
 * ** Help file updated for new features.
 * ** Added to <Copy Event: x, y> notetag help:
 * *** - If '0' is used for the Map ID, reference the current map.
 * * Feature Update!
 * ** Default MZ behavior would have "below characters" trigger events with
 *    only comments lock out facing "same as characters" trigger events. This
 *    is now bypassed. Update made by Arisu.
 * ** The <Copy Event: mapID, eventID> notetags now allow usage of '0' for the
 *    mapID to reference the current map. Update made by Arisu.
 * ** <Save Event Location> should now work more efficiently. Update by Arisu.
 * ** Dashing animations for followers will no longer look weird after having
 *    gathered up and then proceeding to dash. Update made by Irina.
 * * New Features!
 * ** New event notetag added by Arisu:
 * *** <Exit Reset Self Data>
 * **** When the player leaves the current map, all Self Switches and Self
 *      Variables related to this event will be reset.
 * ** New Plugin Command added by Arisu and sponsored by Anon:
 * *** Self Data: Reset All
 * **** Reset the Self Switch and Self Variable data of all events within the
 *      specified map.
 * ** New Plugin Parameter added by Arisu and sponsored by Anon:
 * *** Plugin Params > Movement Settings > Dash > Dash on Ladder?
 * **** Allow dashing while on a ladder or rope?
 * 
 * Version 1.41: June 1, 2022
 * * Bug Fixes!
 * ** Parallel Process Common Events above 1000 should no longer crash the
 *    game. Bug fixed by Irina.
 * 
 * Version 1.40: May 19, 2022
 * * Bug Fixes!
 * ** Sprite Event Labels with distance properties will now work properly
 *    when changing from a non-met page condition to a met page condition.
 *    Fix made by Arisu.
 * 
 * Version 1.39: May 5, 2022
 * * Bug Fixes!
 * ** Save event location should now work properly with Set Event Location
 *    command. Fix made by Arisu.
 * ** Sprite Event Labels with distance properties will no longer be visible
 *    when constantly entering/exiting the Main Menu. Fix made by Arisu.
 * 
 * Version 1.38: April 28, 2022
 * * Documentation Update!
 * ** Help file updated for new features.
 * * New Features!
 * ** New Plugin Parameter added by Arisu and sponsored by Archeia:
 * *** Plugin Parameters > Movement Settings > Event Movement > Shift Y
 * **** How many pixels should non-tile characters be shifted by?
 * ** New Notetags added by Arisu and sponsored by Archeia:
 * *** <Picture Filename: filename>
 * **** applies a picture graphic from the /img/pictures/ folder of your
 *      game project.
 * **** This graphic will be on top of the character sprite but below the event
 *      icon sprite.
 * **** The picture priority will be the same as the event's priority. If it is
 *      "below characters", the player can walk on top of it. If it is "above
 *      characters", the player will behind it. If it is "same as characters",
 *      the priority will be based on the current relative Y position.
 * *** <Picture Max Size: x>
 * *** <Picture Scale: y%>
 * **** If the "Max Size" or "Scale" supplementary notetags are used, the
 *      picture graphic will be scaled proportionally to fit either the exact
 *      pixel size for "Max Size" or the "Scale" ratio.
 * *** <Picture Offset: +x, +y>
 * *** <Picture Offset: -x, -y>
 * **** Offsets the X and Y position of the event picture relative to the event
 *      sprite's own position.
 * *** <Picture Wait Frames: x>
 * **** Requires VisuMZ_4_AnimatedPictures! "Wait Frames" is used with VisuMZ's
 *      Animated Pictures plugin. This determines the delay inbetween
 *      frame changes.
 * 
 * Version 1.37: March 24, 2022
 * * Documentation Update!
 * ** Added extra clarity to "Turn to Home" Movement Command.
 * *** This refers to the original position's X/Y on the map.
 * *** The event will turn and face the tile that is its original X/Y location.
 * 
 * Version 1.36: March 17, 2022
 * * Bug Fixes!
 * ** "Turn To Home" movement command now properly faces the home position.
 *    Fix made by Irina.
 * * Feature Update!
 * ** Plugin Commands now have separators for easier selection.
 * 
 * Version 1.35: March 3, 2022
 * * IMPORTANT! Compatibility Update!
 * ** Compatibility Update with RPG Maker MZ 1.4.4.
 * *** For some reason this update broke any saves made before 1.4.4 was
 *     updated and they cannot be loaded. The only way saves would load is if
 *     you made a safe after 1.4.4 was done. This should be fixed and saves
 *     made with 1.4.3 and before should now be working. Update made by Irina.
 * 
 * Version 1.34: February 17, 2022
 * * Documentation Update!
 * ** Help file updated for new features.
 * * Optimization Update!
 * ** Plugin should run more optimized.
 * * New Features!
 * ** Arisu has created new event notetag/comment tags:
 * *** <Custom Z: x>
 * **** Replace 'x' with a number value to determine the event sprite's Z value
 *      relative to the tilemap.
 * **** View the helpfile for more information.
 * *** <Mirror Sprite>
 * **** The event sprite's visual appearance is mirrored.
 * *** <Move Synch Distance Opacity: x>
 * **** Changes the opacity of the event based on the distance between it and
 *      its move synched target. Closer means more opaque. Further away means
 *      more transparent.
 * ** Irina has created a more memory efficient version of Event Labels.
 * *** Plugin Parameters > Event Label Settings > Sprite Based?
 * **** Use sprite-based labels instead of legacy-window version.
 * **** Legacy-window version will not be supported in future.
 * 
 * Version 1.33: February 3, 2022
 * * Documentation Update!
 * ** Help file updated for new features.
 * * New Features!
 * ** New Notetags added by Arisu!
 * *** <Hide Player>
 * *** <Show Player>
 * **** Map Notetag. Forcefully hides or shows the player sprite. This is so
 *      you don't need to manually turn the setting on/off each time you enter
 *      a specific map.
 * *** <Hide Followers>
 * *** <Show Followers>
 * **** Map Notetag. Forcefully hides or shows the player's followers. This is
 *      so you don't need to manually turn them on/off each time you enter a
 *      specific map.
 * 
 * Version 1.32: January 20, 2022
 * * Bug Fixes!
 * ** Self Variable changes from custom move routes should no longer cause
 *    crashes. Fix made by Arisu.
 * ** Self Switch custom move route toggles should now work properly. Fix made
 *    by Arisu.
 * * Feature Update!
 * ** Better shadow tracking algorithm to remove any shadow twitching.
 *    Update made by Yanfly.
 * 
 * Version 1.31: January 6, 2022
 * * Optimization Update!
 * ** Plugin should run more optimized.
 * 
 * Version 1.30: November 25, 2021
 * * Documentation Update!
 * ** Help file updated for new features.
 * * New Features!
 * ** Map Switches and Map Variables added by Arisu:
 * *** Map Switches are self-switches for maps. Instead of using <Self>, use
 *     <Map> in the Switch name to designate it as a Map Switch. The ON/OFF
 *     data for that Switch will vary depending on the map the player is
 *     currently on.
 * *** Map Variables are self-variables for maps. Instead of using <Self>, use
 *     <Map> in the Variable name to designate it as a Map Switch. The number
 *     data for that Variable will vary depending on the map the player is
 *     currently on.
 * *** Script Calls have been added for these features as well.
 * **** See help file for them.
 * 
 * Version 1.29: October 7, 2021
 * * Bug Fixes!
 * ** Same map event spawning should now work properly without the need to add
 *    the current map ID to the preloaded map array. Update made by Arisu.
 * 
 * Version 1.28: September 30, 2021
 * * Documentation Update!
 * ** Help file updated for new features.
 * * New Features!
 * ** New move route commands added by Arisu:
 * *** Jump to Home
 * *** Move to Home
 * *** Crash Move to Home
 * *** Step Toward Home
 * *** Step Away From Home
 * *** Turn to Home
 * *** Turn Away From Home
 * *** Teleport to Home
 * **** These only work on events. Their actions should be reflective of what
 *      their command names suggest.
 * 
 * Version 1.27: September 17, 2021
 * * Bug Fixes!
 * ** Fixed event spawn templates so that they can work properly with Common
 *    Events. Fix made by Arisu.
 * 
 * Version 1.26: September 3, 2021
 * * Bug Fixes!
 * ** "Step Towards Player" custom command should now work properly. Fix made
 *    by Arisu.
 * ** Having multiple region restriction notetags for a map will no longer
 *    cause others to lock out. Fix made by Arisu.
 * 
 * Version 1.25: July 30, 2021
 * * Bug Fixes!
 * ** Fixed a problem that caused the 'setSelfSwitchValue' and
 *    'setSelfVariableValue' functions to not work properly. Fix made by Irina.
 * 
 * Version 1.24: June 4, 2021
 * * Documentation Update!
 * ** Help file updated for new features.
 * ** Added extra clarification on which commands will go around the player
 *    character and which ones won't.
 * * New Move Route Custom Commands added by Arisu:
 * ** Crash Move (direction) Until Stop
 * ** Crash Move To: x, y
 * ** Crash Move To Event: x
 * *** These allow events to collide with the player character and trigger
 *     Event Touch events.
 * 
 * Version 1.23: May 21, 2021
 * * Bug Fixes!
 * ** Morphing by templates should no longer cause a crash. Fix made by Arisu.
 * 
 * Version 1.22: May 7, 2021
 * * Bug Fixes!
 * ** Plugin Commands for Event Label Visibility should now update without
 *    needing to take steps as per distance detection. Fix made by Arisu.
 * * Documentation Update!
 * ** Added clarity to "Common Event on Touch" Plugin Parameters.
 * *** Areas marked with these regions will not allow random encounters to
 *     occur. This is how RPG Maker works. Assuming you are not using plugins
 *     at all, by putting on touch events all over the map, tiles with those on
 *     touch events will not let random encounters trigger.
 * 
 * Version 1.21: March 12, 2021
 * * Bug Fixes!
 * ** Move until stop custom move routes should no longer cause crashes.
 *    Fix made by Arisu.
 * 
 * Version 1.20: February 26, 2021
 * * Bug Fixes!
 * ** Region Restrictions regarding Player Allow will no longer affect vehicle
 *    passability. Update made by Arisu.
 * 
 * Version 1.19: February 12, 2021
 * * Bug Fixes!
 * ** "Self Variable: Variable ID" plugin command's Map ID should now be able
 *    to use "0" to self reference the current map. Fix made by Olivia.
 * 
 * Version 1.18: February 5, 2021
 * * Bug Fixes!
 * ** Event icon plugin commands should now work properly. Fix made by Arisu.
 * * Documentation Update!
 * ** Added new "Features: Weighted Random Movement" section.
 * ** Help file updated for new features.
 * * New Features!
 * ** New Notetags added by Arisu:
 * *** <Random Move Weight: x>
 * **** If this tag is used on an event with random-type autonomous movement,
 *      then the event will stick closer to their home location (where they are
 *      located upon spawning on the map). How close they stick to their home
 *      location will depend on the weighted 'x' value.
 * *** <True Random Move>
 * **** If this tag is used on an event with random-type autonomous movement,
 *      then that event will ignore the effects of weighted randomized
 *      movement.
 * ** New Plugin Commands added by Arisu and sponsored by AndyL:
 * *** Event Timer: Change Speed
 * *** Event Timer: Expire Event Assign
 * *** Event Timer: Expire Event Clear
 * *** Event Timer: Frames Gain
 * *** Event Timer: Frames Set
 * *** Event Timer: Pause
 * *** Event Timer: Resume
 * **** The above Plugin Commands allow you to control the game timer better.
 * ** New Plugin Parameters added by Arisu:
 * *** Plugin Parameters > Movement > Event Movement > Random Move Weight
 * **** Use numbers between 0 and 1. Numbers closer to 1 stay closer to their
 *      home position.
 * 
 * Version 1.17: January 29, 2021
 * * Documentation Update!
 * ** Added "Do NOT insert quotes" to "Balloon: name" and "Pose: name".
 * ** Added Examples for extra clarification.
 * * Optimization Update!
 * ** When touch clicking an event on a map with multiple events, pathfinding
 *    will utilize the non-diagonal function for less resource consumption to
 *    prevent FPS frame drops. Fix made by Arisu.
 * 
 * Version 1.16: January 22, 2021
 * * Optimization Update!
 * ** When touch clicking multiple times on an impassable tile, pathfinding
 *    will utilize the non-diagonal function for less resource consumption to
 *    prevent FPS frame drops. Fix made by Arisu.
 * 
 * Version 1.15: January 1, 2021
 * * Bug Fixes!
 * ** Spawned events should now resume their automated self movement after
 *    being interacted with. Fix made by Yanfly.
 * * Documentation Update!
 * ** Help file updated for new features.
 * ** Help file updated for updated features.
 * * Feature Updates!
 * ** Collission checks for the Spawn Event Plugin Commands now account for
 *    the spawning event's Hitbox, too. Update made by Yanfly.
 * ** Spawn Event Plugin Commands adds a new parameter "Success Switch ID" to
 *    check if the spawning has been successful or not.
 * * New Features!
 * ** New Plugin Commands added by Yanfly!
 * *** Spawn Event: Spawn At Terrain Tag
 * *** Spawn Event: Despawn Terrain Tag(s)
 * **** These function similar to their region counterparts except they target
 *      terrain tags instead.
 * 
 * Version 1.14: December 18, 2020
 * * Bug Fixes!
 * ** Caching for event label positions now account for page index.
 *    Fix made by Yanfly.
 * * Documentation Update!
 * ** Added documentation for the new features!
 * * New Features!
 * ** New Plugin Commands added by Irina.
 * *** Follower: Set Global Chase
 * *** Follower: Set Target Chase
 * *** Follower: Set Control
 * *** Follower: Reset
 * **** These plugin commands allow you to change whether or not the followers
 *      will chase their intended targets and/or shift control over their
 *      movement route from the "Player" to the target follower.
 * 
 * Version 1.13: December 4, 2020
 * * Bug Fixes!
 * ** Caching for event label positions now account for one-screen maps.
 *    Fix made by Arisu.
 * 
 * Version 1.12: November 29, 2020
 * * Bug Fixes!
 * ** Click Triggers no longer work on erased events. Fix made by Arisu.
 * ** Erased events no longer have icons appear above their heads.
 *    Fix made by Arisu.
 * * Feature Update!
 * ** Initialization of the plugin's effects no only occur if the event's
 *    current page settings have been altered. Change made by Arisu.
 * * Optimization Update!
 * ** Plugin should run more optimized.
 * 
 * Version 1.11: November 15, 2020
 * * Bug Fixes!
 * ** Morph plugin command should no longer cause crashes. Fix made by Yanfly.
 * * Documentation Update!
 * ** Added documentation for the updated features!
 * * Feature Updates!
 * ** Updates to these Plugin Commands made by Yanfly:
 * *** Call Event: Remote Activation
 * *** Event Icon: Change
 * *** Event Icon: Delete
 * *** Event Location: Create
 * *** Event Location: Delete
 * *** Global Switch: Get Self Switch A B C D
 * *** Global Switch: Get Self Switch ID
 * *** Global Variable: Get Self Variable ID
 * *** Morph Event: Change
 * *** Morph Event: Remove
 * *** Self Switch: A B C D
 * *** Self Switch: Switch ID
 * *** Self Variable: Variable ID
 * **** All of the above Plugin Commands can now use 0 for their Event ID's in
 *      order to refer to the running event's ID value.
 * 
 * Version 1.10: November 1, 2020
 * * Bug Fixes!
 * ** Spawned Event preserve function now works properly. Fix made by Arisu.
 * 
 * Version 1.09: October 25, 2020
 * * Documentation Update
 * ** Added clarity on the notetags and comment tags on when their effects
 *    are present.
 * * Feature Update!
 * ** Event icons now have an unsmoothing property to them to make them
 *    look better. Update made by Irina.
 * 
 * Version 1.08: October 11, 2020
 * * Compatibility Update
 * ** Added failsafes for better compatibility.
 * 
 * Version 1.07: October 4, 2020
 * * Documentation Update!
 * ** Updated for the new features!
 * * Feature Update!
 * ** Data from deleted events will now be cleared and removed from maps if the
 *    events do not exist to prevent conflict with plugins from the VisuStella
 *    MZ library and other plugins. Feature added by Irina.
 * ** Move Route Custom Commands now support self variable values! If you wish
 *    to use a value from a self variable, insert \SelfVar[x] in place of the x
 *    in any of the below. This will only draw from the current event. If you 
 *    wish to draw data from outside event self variables, we recommend you
 *    use the \V[x] variant after using the Plugin Commands to draw data from
 *    them for the best accuracy.
 * * New Features!
 * ** New Plugin Parameter added by Yanfly!
 * *** Movement > Bitmap > Smoothing
 * **** Do you want to smooth or pixelate the map sprites? Pixelating them is
 *      better for zooming and tilting.
 * 
 * Version 1.06: September 27, 2020
 * * Bug Fixes!
 * ** Events & Movement Core no longer disables the Core Engine's Smart Event
 *    Collision plugin parameter. Fix made by Yanfly.
 * * Documentation Update!
 * ** Move Route Custom Commands updated with the new feature for inserting
 *    variable values.
 * * Feature Update!
 * ** Move Route Custom Commands now support $gameVariable.value(x) values.
 *    You can also just use \V[x] for variable values, too. Added by Irina.
 * 
 * Version 1.05: September 20, 2020
 * * Bug Fixes!
 * ** If player movement is disabled, mouse movement is disabled, too.
 *    Fix made by Arisu.
 * ** The region restriction notetags should be fixed and work again.
 *    Fix made by Arisu.
 * 
 * Version 1.04: September 13, 2020
 * * Feature Update!
 * * Some Move Route Custom Commands are updated to ignore spaces:
 * ** Jump To: x, y
 * ** Move To: x, y
 * ** Step Toward: x, y
 * ** Step Away From: x, y
 * ** Turn To: x, y
 * ** Turn Away From: x, y
 * ** Teleport To: x, y
 * *** These can now be written as x,y. There still needs to be a space between
 *     the : and x for parsing clarity, however.
 * *** Feature updated by Arisu with help from BlueMoon and Zeriab.
 * * New Features!
 * ** New 'Move Route Custom Commands' added by Arisu.
 * *** Fade In: x
 * *** Fade Out: x
 * *** Force Carry: On
 * *** Force Carry: Off
 * *** Force Dash: On
 * *** Force Dash: Off
 * ** New Plugin Commands added by Arisu.
 * *** Player Movement: Control
 * **** Enable or disable player control over the player character's movement.
 * *** Player Movement: Diagonal
 * **** Override settings to for player diagonal movement.
 * 
 * Version 1.03: September 6, 2020
 * * Bug Fixes!
 * ** Sleeping pose is now fixed and working! Fix made by Yanfly.
 * * Documentation Update!
 * ** Extended "Features: Self Switches and Variables" to explain how to use
 *    script calls to grab self switch information.
 * * New Features!
 * ** New Plugin Commands added by Yanfly:
 * *** Global Switch: Get Self Switch A B C D
 * *** Global Switch: Get Self Switch ID
 * *** Global Variable: Get Self Variable ID
 * **** These plugin commands allow you to transfer data stored in a self
 *      switch or Self Variable into a global switch or global variable.
 * 
 * Version 1.02: August 30, 2020
 * * Bug Fixes!
 * ** <Diagonal Movement: Off> notetag now works properly. Fix made by Yanfly.
 * ** Plugin Command "Event Label: Visible" now works properly. Fix made by
 *    Shaz.
 * ** Custom Move Route commands should now be working properly. Fix made by
 *    Shaz.
 * 
 * Version 1.01: August 23, 2020
 * * Bug Fixes!
 * ** Event Cache issues fixed upon loading a saved game. Fix made by Yanfly.
 *
 * Version 1.00: August 20, 2020
 * * Finished Plugin!
 *
 * ============================================================================
 * End of Helpfile
 * ============================================================================
 *
 * @ --------------------------------------------------------------------------
 *
 * @command Separator_AutoMove
 * @text -
 * @desc -
 *
 * @ --------------------------------------------------------------------------
 *
 * @command AutoMoveEvents
 * @text Auto Movement: Events
 * @desc Allow/stop events from auto movement.
 *
 * @arg Value:str
 * @text Value
 * @type select
 * @option Allow
 * @value Allow
 * @option Stop
 * @value Stop
 * @option Toggle
 * @value Toggle
 * @desc Allow events to move automatically?
 * @default Allow
 *
 * @ --------------------------------------------------------------------------
 *
 * @command Separator_CallEvent
 * @text -
 * @desc -
 *
 * @ --------------------------------------------------------------------------
 *
 * @command CallEvent
 * @text Call Event: Remote Read
 * @desc Runs the page of a different event remotely. This will run
 * the page of the target event on the CURRENT event.
 *
 * @arg MapId:eval
 * @text Map ID
 * @desc Target event's map. Use 0 for current map.
 * You may use JavaScript code.
 * @default 0
 *
 * @arg EventId:eval
 * @text Event ID
 * @desc The ID of the event to remotely run. Use 0 for current event. You may use JavaScript code.
 * @default 0
 *
 * @arg PageId:eval
 * @text Page ID
 * @desc The page of the remote event to run.
 * You may use JavaScript code.
 * @default 1
 *
 * @ --------------------------------------------------------------------------
 *
 * @command Separator_DashEnable
 * @text -
 * @desc -
 *
 * @ --------------------------------------------------------------------------
 *
 * @command DashEnableToggle
 * @text Dash Enable: Toggle
 * @desc Enable/Disable Dashing on maps.
 *
 * @arg Value:str
 * @text Value
 * @type select
 * @option Enable
 * @value Enable
 * @option Disable
 * @value Disable
 * @option Toggle
 * @value Toggle
 * @desc What do you wish to change dashing to?
 * @default Enable
 *
 * @ --------------------------------------------------------------------------
 *
 * @command Separator_EventIcon
 * @text -
 * @desc -
 *
 * @ --------------------------------------------------------------------------
 *
 * @command EventIconChange
 * @text Event Icon: Change (Temporary)
 * @desc Change the icon that appears on an event.
 * This change is temporary and resets upon new events.
 *
 * @arg MapId:eval
 * @text Map ID
 * @desc The map the target map. Use 0 for current map.
 * You may use JavaScript code.
 * @default 0
 *
 * @arg EventId:eval
 * @text Event ID
 * @parent MapId:eval
 * @desc The ID of the target event.  Use 0 for current event.
 * You may use JavaScript code.
 * @default 0
 *
 * @arg IconIndex:eval
 * @text Icon Index
 * @desc Icon index used for the icon.
 * You may use JavaScript code.
 * @default 1
 *
 * @arg IconBufferX:eval
 * @text Buffer X
 * @parent IconIndex:eval
 * @desc How much to shift the X position by?
 * You may use JavaScript code.
 * @default 0
 *
 * @arg IconBufferY:eval
 * @text Buffer Y
 * @parent IconIndex:eval
 * @desc How much to shift the Y position by?
 * You may use JavaScript code.
 * @default 12
 *
 * @arg IconBlendMode:num
 * @text Blend Mode
 * @parent IconIndex:eval
 * @type select
 * @option 0 - Normal
 * @value 0
 * @option 1 - Additive
 * @value 1
 * @option 2 - Multiply
 * @value 2
 * @option 3 - Screen
 * @value 3
 * @desc What kind of blend mode do you wish to apply to the icon sprite?
 * @default 0
 *
 * @ --------------------------------------------------------------------------
 *
 * @command EventIconChangeForced
 * @text Event Icon: Change (Forced)
 * @desc Change the icon that appears on an event.
 * This change is forced and needs to be restored.
 *
 * @arg MapId:eval
 * @text Map ID
 * @desc The map the target map. Use 0 for current map.
 * You may use JavaScript code.
 * @default 0
 *
 * @arg EventId:eval
 * @text Event ID
 * @parent MapId:eval
 * @desc The ID of the target event.  Use 0 for current event.
 * You may use JavaScript code.
 * @default 0
 *
 * @arg IconIndex:eval
 * @text Icon Index
 * @desc Icon index used for the icon.
 * You may use JavaScript code.
 * @default 1
 *
 * @arg IconBufferX:eval
 * @text Buffer X
 * @parent IconIndex:eval
 * @desc How much to shift the X position by?
 * You may use JavaScript code.
 * @default 0
 *
 * @arg IconBufferY:eval
 * @text Buffer Y
 * @parent IconIndex:eval
 * @desc How much to shift the Y position by?
 * You may use JavaScript code.
 * @default 12
 *
 * @arg IconBlendMode:num
 * @text Blend Mode
 * @parent IconIndex:eval
 * @type select
 * @option 0 - Normal
 * @value 0
 * @option 1 - Additive
 * @value 1
 * @option 2 - Multiply
 * @value 2
 * @option 3 - Screen
 * @value 3
 * @desc What kind of blend mode do you wish to apply to the icon sprite?
 * @default 0
 *
 * @ --------------------------------------------------------------------------
 *
 * @command EventIconDelete
 * @text Event Icon: Delete
 * @desc Delete the icon that appears on an event.
 * This will remain deleted and invisible for events.
 *
 * @arg MapId:eval
 * @text Map ID
 * @desc The map the target map. Use 0 for current map.
 * You may use JavaScript code.
 * @default 0
 *
 * @arg EventId:eval
 * @text Event ID
 * @parent MapId:eval
 * @desc The ID of the target event. Use 0 for current event.
 * You may use JavaScript code.
 * @default 0
 *
 * @ --------------------------------------------------------------------------
 *
 * @command EventIconRestore
 * @text Event Icon: Restore
 * @desc Restores a deleted or forced icon that appears on an event.
 *
 * @arg MapId:eval
 * @text Map ID
 * @desc The map the target map. Use 0 for current map.
 * You may use JavaScript code.
 * @default 0
 *
 * @arg EventId:eval
 * @text Event ID
 * @parent MapId:eval
 * @desc The ID of the target event. Use 0 for current event.
 * You may use JavaScript code.
 * @default 0
 *
 * @ --------------------------------------------------------------------------
 *
 * @command Separator_EventLabel
 * @text -
 * @desc -
 *
 * @ --------------------------------------------------------------------------
 *
 * @command EventLabelRefresh
 * @text Event Label: Refresh
 * @desc Refresh all Event Labels on screen.
 *
 * @ --------------------------------------------------------------------------
 *
 * @command EventLabelVisible
 * @text Event Label: Visible
 * @desc Change the visibility of Event Labels.
 *
 * @arg Visibility:str
 * @text Visibility
 * @type select
 * @option Visible
 * @value Visible
 * @option Hidden
 * @value Hidden
 * @option Toggle
 * @value Toggle
 * @desc What do you wish to change visibility to?
 * @default Visible
 *
 * @ --------------------------------------------------------------------------
 *
 * @command Separator_EventLocation
 * @text -
 * @desc -
 *
 * @ --------------------------------------------------------------------------
 *
 * @command EventLocationSave
 * @text Event Location: Save
 * @desc Memorize an event's map location so it reappears there
 * the next time the map is loaded.
 *
 * @arg EventId:eval
 * @text Event ID
 * @desc The ID of the target event.
 * You may use JavaScript code.
 * @default 1
 *
 * @ --------------------------------------------------------------------------
 *
 * @command EventLocationCreate
 * @text Event Location: Create
 * @desc Creates a custom spawn location for a specific map's event
 * so it appears there the next time the map is loaded.
 *
 * @arg MapId:eval
 * @text Map ID
 * @desc The map the target map. Use 0 for current map.
 * You may use JavaScript code.
 * @default 0
 *
 * @arg EventId:eval
 * @text Event ID
 * @parent MapId:eval
 * @desc The ID of the target event. Use 0 for current event.
 * You may use JavaScript code.
 * @default 0
 *
 * @arg PosX:eval
 * @text X Coordinate
 * @parent MapId:eval
 * @desc The X coordinate of the event.
 * You may use JavaScript code.
 * @default 0
 *
 * @arg PosY:eval
 * @text Y Coordinate
 * @parent MapId:eval
 * @desc The Y coordinate of the event.
 * You may use JavaScript code.
 * @default 0
 *
 * @arg Direction:num
 * @text Direction
 * @parent MapId:eval
 * @type select
 * @option 1 - Lower Left
 * @value 1
 * @option 2 - Down
 * @value 2
 * @option 3 - Lower Right
 * @value 3
 * @option 4 - Left
 * @value 4
 * @option 6 - Right
 * @value 6
 * @option 7 - Upper Left
 * @value 7
 * @option 8 - Up
 * @value 8
 * @option 9 - Upper Right
 * @value 9
 * @desc The direction the event will be facing.
 * @default 2
 *
 * @arg Optional
 *
 * @arg PageId:eval
 * @text Page ID
 * @parent Optional
 * @desc The page of the event to set the move route to.
 * You may use JavaScript code.
 * @default 1
 *
 * @arg MoveRouteIndex:eval
 * @text Move Route Index
 * @parent Optional
 * @desc The point in the move route for this event to be at
 * if the page ID matches the rest of the page conditions.
 * @default 0
 *
 * @ --------------------------------------------------------------------------
 *
 * @command EventLocationDelete
 * @text Event Location: Delete
 * @desc Deletes an event's saved map location.
 * The event will reappear at its default location.
 *
 * @arg MapId:eval
 * @text Map ID
 * @desc The map the target map. Use 0 for current map.
 * You may use JavaScript code.
 * @default 0
 *
 * @arg EventId:eval
 * @text Event ID
 * @desc The ID of the target event. Use 0 for current event.
 * You may use JavaScript code.
 * @default 0
 *
 * @ --------------------------------------------------------------------------
 *
 * @command Separator_EventPopup
 * @text -
 * @desc -
 *
 * @ --------------------------------------------------------------------------
 *
 * @command MsgPopupPlayer
 * @text Event Popup: Player
 * @desc Makes a centered event popup on the player sprite.
 * Requires VisuMZ_1_MessageCore! Cannot be used in battle!
 *
 * @arg MessageText:json
 * @text Message Text
 * @type note
 * @desc Insert the text to be displayed.
 * Text codes can be used.
 * @default "+\\LastGainObjQuantity\\LastGainObj"
 * 
 * @arg MsgDuration:eval
 * @text Message Duration
 * @parent MessageText:json
 * @desc What is the frame duration of the event popup?
 * 60 frames = 1 second. You may use code.
 * @default 60
 *
 * @arg PopupExtra:struct
 * @text Popup Settings
 * @type struct<PopupExtra>
 * @desc These settings let you adjust how the popup animates.
 * @default {"Fade":"","fadeInDuration:eval":"8","fadeOutDuration:eval":"8","Offset":"","startOffsetX:eval":"+0","startOffsetY:eval":"-48","endOffsetX:eval":"+0","endOffsetY:eval":"-96","Scale":"","startScaleX:eval":"0.8","startScaleY:eval":"0.8","endScaleX:eval":"0.8","endScaleY:eval":"0.8","Angle":"","startAngle:eval":"+0","endAngle:eval":"+0","Misc":"","Arc:eval":"+0"}
 *
 * @ --------------------------------------------------------------------------
 *
 * @command MsgPopupFollower
 * @text Event Popup: Follower
 * @desc Makes a centered event popup on target follower sprite.
 * Requires VisuMZ_1_MessageCore! Cannot be used in battle!
 *
 * @arg FollowerIndex:eval
 * @text Follower Index
 * @desc Which follower index to play popup?
 * Index starts at 0. You may use JavaScript code.
 * @default 0
 *
 * @arg MessageText:json
 * @text Message Text
 * @type note
 * @desc Insert the text to be displayed.
 * Text codes can be used.
 * @default "\\I[23]"
 * 
 * @arg MsgDuration:eval
 * @text Message Duration
 * @parent MessageText:json
 * @desc What is the frame duration of the event popup?
 * 60 frames = 1 second. You may use code.
 * @default 60
 *
 * @arg PopupExtra:struct
 * @text Popup Settings
 * @type struct<PopupExtra>
 * @desc These settings let you adjust how the popup animates.
 * @default {"Fade":"","fadeInDuration:eval":"8","fadeOutDuration:eval":"8","Offset":"","startOffsetX:eval":"+0","startOffsetY:eval":"-48","endOffsetX:eval":"+0","endOffsetY:eval":"-96","Scale":"","startScaleX:eval":"0.8","startScaleY:eval":"0.8","endScaleX:eval":"0.8","endScaleY:eval":"0.8","Angle":"","startAngle:eval":"+0","endAngle:eval":"+0","Misc":"","Arc:eval":"+0"}
 *
 * @ --------------------------------------------------------------------------
 *
 * @command MsgPopupEvent
 * @text Event Popup: Event
 * @desc Makes a centered event popup on target event sprite.
 * Requires VisuMZ_1_MessageCore! Cannot be used in battle!
 *
 * @arg EventId:eval
 * @text Event ID
 * @desc The ID of the event to play popup on.
 * Use 0 for current event. You may use JavaScript code.
 * @default 0
 *
 * @arg MessageText:json
 * @text Message Text
 * @type note
 * @desc Insert the text to be displayed.
 * Text codes can be used.
 * @default "Line1\nLine2"
 * 
 * @arg MsgDuration:eval
 * @text Message Duration
 * @parent MessageText:json
 * @desc What is the frame duration of the event popup?
 * 60 frames = 1 second. You may use code.
 * @default 60
 *
 * @arg PopupExtra:struct
 * @text Popup Settings
 * @type struct<PopupExtra>
 * @desc These settings let you adjust how the popup animates.
 * @default {"Fade":"","fadeInDuration:eval":"8","fadeOutDuration:eval":"8","Offset":"","startOffsetX:eval":"+0","startOffsetY:eval":"-48","endOffsetX:eval":"+0","endOffsetY:eval":"-96","Scale":"","startScaleX:eval":"0.8","startScaleY:eval":"0.8","endScaleX:eval":"0.8","endScaleY:eval":"0.8","Angle":"","startAngle:eval":"+0","endAngle:eval":"+0","Misc":"","Arc:eval":"+0"}
 *
 * @ --------------------------------------------------------------------------
 *
 * @command MsgPopupTargetTile
 * @text Event Popup: Target Tile
 * @desc Makes a centered event popup on target tile.
 * Requires VisuMZ_1_MessageCore! Cannot be used in battle!
 *
 * @arg TileX:eval
 * @text Map Tile X
 * @desc The x coordinate of the map tile.
 * You may use JavaScript code.
 * @default $gameMap.width() / 2
 *
 * @arg TileY:eval
 * @text Map Tile Y
 * @desc The y coordinate of the map tile.
 * You may use JavaScript code.
 * @default $gameMap.height() / 2
 *
 * @arg MessageText:json
 * @text Message Text
 * @type note
 * @desc Insert the text to be displayed.
 * Text codes can be used.
 * @default "\\I[87]"
 * 
 * @arg MsgDuration:eval
 * @text Message Duration
 * @parent MessageText:json
 * @desc What is the frame duration of the event popup?
 * 60 frames = 1 second. You may use code.
 * @default 60
 *
 * @arg PopupExtra:struct
 * @text Popup Settings
 * @type struct<PopupExtra>
 * @desc These settings let you adjust how the popup animates.
 * @default {"Fade":"","fadeInDuration:eval":"8","fadeOutDuration:eval":"8","Offset":"","startOffsetX:eval":"+0","startOffsetY:eval":"-24","endOffsetX:eval":"+0","endOffsetY:eval":"-24","Scale":"","startScaleX:eval":"0.8","startScaleY:eval":"0.8","endScaleX:eval":"0.8","endScaleY:eval":"0.8","Angle":"","startAngle:eval":"+0","endAngle:eval":"+0","Misc":"","Arc:eval":"+0"}
 *
 * @ --------------------------------------------------------------------------
 *
 * @command Separator_EventTimer
 * @text -
 * @desc -
 *
 * @ --------------------------------------------------------------------------
 *
 * @command EventTimerExpireEvent
 * @text Event Timer: Expire Event Assign
 * @desc Sets a Common Event to run upon expiration.
 * Bypasses the default code if one is set.
 *
 * @arg CommonEventID:num
 * @text Common Event ID
 * @type common_event
 * @desc Select the Common Event to run upon the timer's expiration.
 * @default 1
 *
 * @ --------------------------------------------------------------------------
 *
 * @command EventTimerSpeed
 * @text Event Timer: Change Speed
 * @desc Changes the timer frame decrease (or increase) speed.
 *
 * @arg Speed:eval
 * @text Speed
 * @desc How many 1/60ths of a second does each frame increase or
 * decrease by? Negative decreases. Positive increases.
 * @default -1
 *
 * @ --------------------------------------------------------------------------
 *
 * @command EventTimerExpireClear
 * @text Event Timer: Expire Event Clear
 * @desc Clears any set to expire Common Event and instead,
 * run the default Game_Timer expiration code.
 *
 * @ --------------------------------------------------------------------------
 *
 * @command EventTimerFramesGain
 * @text Event Timer: Frames Gain
 * @desc Chooses how many frames, seconds, minutes, or hours
 * are gained or lost for the event timer.
 *
 * @arg Frames:eval
 * @text Frames
 * @desc How many 1/60ths of a second are gained/lost?
 * Positive for gain. Negative for lost. JavaScript allowed.
 * @default +0
 *
 * @arg Seconds:eval
 * @text Seconds
 * @desc How many seconds are gained/lost?
 * Positive for gain. Negative for lost. JavaScript allowed.
 * @default +0
 *
 * @arg Minutes:eval
 * @text Minutes
 * @desc How many minutes are gained/lost?
 * Positive for gain. Negative for lost. JavaScript allowed.
 * @default +0
 *
 * @arg Hours:eval
 * @text Hours
 * @desc How many hours are gained/lost?
 * Positive for gain. Negative for lost. JavaScript allowed.
 * @default +0
 *
 * @ --------------------------------------------------------------------------
 *
 * @command EventTimerFramesSet
 * @text Event Timer: Frames Set
 * @desc Chooses how many frames, seconds, minutes, or hours
 * are set for the event timer.
 *
 * @arg Frames:eval
 * @text Frames
 * @desc Set frame count to this value.
 * Each frame is 1/60th of a second. JavaScript allowed.
 * @default 0
 *
 * @arg Seconds:eval
 * @text Seconds
 * @desc Set seconds to this value.
 * JavaScript allowed.
 * @default 0
 *
 * @arg Minutes:eval
 * @text Minutes
 * @desc Set minutes to this value.
 * Each minute is 60 seconds. JavaScript allowed.
 * @default 0
 *
 * @arg Hours:eval
 * @text Hours
 * @desc Set hours to this value.
 * Each hour is 60 minutes. JavaScript allowed.
 * @default 0
 *
 * @ --------------------------------------------------------------------------
 *
 * @command EventTimerPause
 * @text Event Timer: Pause
 * @desc Pauses the current event timer, but does not stop it.
 *
 * @ --------------------------------------------------------------------------
 *
 * @command EventTimerResume
 * @text Event Timer: Resume
 * @desc Resumes the current event timer from the paused state.
 *
 * @ --------------------------------------------------------------------------
 *
 * @command Separator_Follower
 * @text -
 * @desc -
 *
 * @ --------------------------------------------------------------------------
 *
 * @command FollowerSetGlobalChase
 * @text Follower: Set Global Chase
 * @desc Disables all followers from chasing the player
 * or reenables it.
 *
 * @arg Chase:eval
 * @text Chase
 * @type boolean
 * @on Chase
 * @off Don't Chase
 * @desc Sets all followers to chase the player or not.
 * @default false
 *
 * @ --------------------------------------------------------------------------
 *
 * @command FollowerSetTargetChase
 * @text Follower: Set Target Chase
 * @desc Disables target follower from chasing the player
 * or reenables it.
 *
 * @arg FollowerID:eval
 * @text Follower ID
 * @desc Select which follower ID to disable/reenable chasing for.
 * @default 1
 *
 * @arg Chase:eval
 * @text Chase
 * @type boolean
 * @on Chase
 * @off Don't Chase
 * @desc Sets target follower to chase its target or not.
 * @default false
 *
 * @ --------------------------------------------------------------------------
 *
 * @command FollowerSetControl
 * @text Follower: Set Control
 * @desc Sets the event commands to target a follower when "Player"
 * is selected as the target.
 *
 * @arg FollowerID:eval
 * @text Follower ID
 * @desc Select which follower ID to control.
 * 0 is the player.
 * @default 1
 *
 * @ --------------------------------------------------------------------------
 *
 * @command FollowerReset
 * @text Follower: Reset
 * @desc Resets all follower controls. Event Commands that target
 * the "Player" return to normal and followers chase again.
 *
 * @ --------------------------------------------------------------------------
 *
 * @command Separator_GlobalSwitch
 * @text -
 * @desc -
 *
 * @ --------------------------------------------------------------------------
 *
 * @command SwitchGetSelfSwitchABCD
 * @text Global Switch: Get Self Switch A B C D
 * @desc Gets the current ON/OFF value from a Self Switch and
 * stores it onto a Global Switch.
 *
 * @arg MapId:eval
 * @text Map ID
 * @desc The map the source map. Use 0 for current map.
 * You may use JavaScript code.
 * @default 0
 *
 * @arg EventId:eval
 * @text Event ID
 * @desc The ID of the source event. Use 0 for current event.
 * You may use JavaScript code.
 * @default 0
 *
 * @arg Letter:str
 * @text Letter
 * @type select
 * @option A
 * @value A
 * @option B
 * @value B
 * @option C
 * @value C
 * @option D
 * @value D
 * @desc Letter of the target event's Self Switch to obtain data from.
 * @default A
 *
 * @arg Break
 * @text -
 *
 * @arg TargetSwitchId:num
 * @text Target Switch ID
 * @type switch
 * @desc The ID of the target switch.
 * @default 1
 *
 * @ --------------------------------------------------------------------------
 *
 * @command SwitchGetSelfSwitchID
 * @text Global Switch: Get Self Switch ID
 * @desc Gets the current ON/OFF value from a Self Switch and
 * stores it onto a Global Switch.
 *
 * @arg MapId:eval
 * @text Map ID
 * @desc The map the source map. Use 0 for current map.
 * You may use JavaScript code.
 * @default 0
 *
 * @arg EventId:eval
 * @text Event ID
 * @desc The ID of the source event. Use 0 for current event.
 * You may use JavaScript code.
 * @default 0
 *
 * @arg SwitchId:num
 * @text Switch ID
 * @type switch
 * @desc The ID of the source switch.
 * @default 1
 *
 * @arg Break
 * @text -
 *
 * @arg TargetSwitchId:num
 * @text Target Switch ID
 * @type switch
 * @desc The ID of the target switch.
 * @default 1
 *
 * @ --------------------------------------------------------------------------
 *
 * @command Separator_GlobalVar
 * @text -
 * @desc -
 *
 * @ --------------------------------------------------------------------------
 *
 * @command VariableGetSelfVariableID
 * @text Global Variable: Get Self Variable ID
 * @desc Gets the current stored value from a Self Variable and
 * stores it onto a Global Variable.
 *
 * @arg MapId:eval
 * @text Map ID
 * @desc The map the source map. Use 0 for current map.
 * You may use JavaScript code.
 * @default 0
 *
 * @arg EventId:eval
 * @text Event ID
 * @desc The ID of the source event. Use 0 for current event.
 * You may use JavaScript code.
 * @default 0
 *
 * @arg VariableId:num
 * @text Variable ID
 * @type variable
 * @desc The ID of the source variable.
 * @default 1
 *
 * @arg Break
 * @text -
 *
 * @arg TargetVariableId:num
 * @text Target Variable ID
 * @type variable
 * @desc The ID of the target variable.
 * @default 1
 *
 * @ --------------------------------------------------------------------------
 *
 * @command Separator_MorphEvent
 * @text -
 * @desc -
 *
 * @ --------------------------------------------------------------------------
 *
 * @command MorphEventTo
 * @text Morph Event: Change
 * @desc Runs the page of a different event remotely.
 *
 * @arg Step1
 * @text Step 1: To Be Changed
 *
 * @arg Step1MapId:eval
 * @text Map ID
 * @parent Step1
 * @desc Target event's map. Use 0 for current map.
 * You may use JavaScript code.
 * @default 0
 *
 * @arg Step1EventId:eval
 * @text Event ID
 * @parent Step1
 * @desc The ID of the target event. Use 0 for current event.
 * You may use JavaScript code.
 * @default 0
 *
 * @arg Step2
 * @text Step 2: Change Into
 *
 * @arg TemplateName:str
 * @text Template Name
 * @parent Step2
 * @desc Name of the target event template to morph into.
 * Ignored if this is called "Untitled".
 * @default Untitled
 *
 * @arg Step2MapId:eval
 * @text Map ID
 * @parent Step2
 * @desc Target event's map. Use 0 for current map.
 * You may use JavaScript code.
 * @default 1
 *
 * @arg Step2EventId:eval
 * @text Event ID
 * @parent Step2
 * @desc The ID of the target event. Use 0 for current event.
 * You may use JavaScript code.
 * @default 0
 *
 * @arg Step2Preserve:eval
 * @text Preserve Morph
 * @parent Step2
 * @type boolean
 * @on Preserve
 * @off Expires
 * @desc Is the morph effect preserved?
 * Or does it expire upon leaving the map?
 * @default true
 *
 * @ --------------------------------------------------------------------------
 *
 * @command MorphEventRemove
 * @text Morph Event: Remove
 * @desc Remove the morph status of an event.
 *
 * @arg MapId:eval
 * @text Map ID
 * @parent Step1
 * @desc Target event's map. Use 0 for current map.
 * You may use JavaScript code.
 * @default 0
 *
 * @arg EventId:eval
 * @text Event ID
 * @parent Step1
 * @desc The ID of the event to remove morph from. Use 0 for current event. You may use JavaScript code.
 * @default 0
 *
 * @arg RemovePreserve:eval
 * @text Remove Preservation
 * @parent Step2
 * @type boolean
 * @on Remove
 * @off Contain
 * @desc Also remove the preservation effect?
 * @default true
 *
 * @ --------------------------------------------------------------------------
 *
 * @command Separator_PlayerIcon
 * @text -
 * @desc -
 *
 * @ --------------------------------------------------------------------------
 *
 * @command PlayerIconChange
 * @text Player Icon: Change
 * @desc Change the icon that appears on on the player.
 *
 * @arg IconIndex:eval
 * @text Icon Index
 * @desc Icon index used for the icon.
 * You may use JavaScript code.
 * @default 1
 *
 * @arg IconBufferX:eval
 * @text Buffer X
 * @parent IconIndex:eval
 * @desc How much to shift the X position by?
 * You may use JavaScript code.
 * @default 0
 *
 * @arg IconBufferY:eval
 * @text Buffer Y
 * @parent IconIndex:eval
 * @desc How much to shift the Y position by?
 * You may use JavaScript code.
 * @default 12
 *
 * @arg IconBlendMode:num
 * @text Blend Mode
 * @parent IconIndex:eval
 * @type select
 * @option 0 - Normal
 * @value 0
 * @option 1 - Additive
 * @value 1
 * @option 2 - Multiply
 * @value 2
 * @option 3 - Screen
 * @value 3
 * @desc What kind of blend mode do you wish to apply to the icon sprite?
 * @default 0
 *
 * @ --------------------------------------------------------------------------
 *
 * @command PlayerIconDelete
 * @text Player Icon: Delete
 * @desc Delete the icon that appears on the player.
 *
 * @ --------------------------------------------------------------------------
 *
 * @command Separator_PlayerMovement
 * @text -
 * @desc -
 *
 * @ --------------------------------------------------------------------------
 *
 * @command PlayerMovementChange
 * @text Player Movement: Control
 * @desc Enable or disable player control over the player character's movement.
 *
 * @arg Enable:eval
 * @text Enable?
 * @type boolean
 * @on Enable
 * @off Disable
 * @desc Let the player control where the player character moves?
 * @default true
 *
 * @ --------------------------------------------------------------------------
 *
 * @command PlayerMovementDiagonal
 * @text Player Movement: Diagonal
 * @desc Override settings to for player diagonal movement.
 *
 * @arg Setting:str
 * @text Setting
 * @type select
 * @option Default: Whatever the Map Uses
 * @value default
 * @option Forcefully Disable Diagonal Movement
 * @value disable
 * @option Forcefully Enable Diagonal Movement
 * @value enable
 * @desc How do you want to change diagonal movement?
 * @default default
 *
 * @ --------------------------------------------------------------------------
 *
 * @command Separator_SelfData
 * @text -
 * @desc -
 *
 * @ --------------------------------------------------------------------------
 *
 * @command SelfDataResetAll
 * @text Self Data: Reset All
 * @desc Reset the Self Switch and Self Variable data of all events
 * within the specified map.
 *
 * @arg MapId:eval
 * @text Map ID
 * @desc The map the target map. Use 0 for current map.
 * You may use JavaScript code.
 * @default 0
 *
 * @ --------------------------------------------------------------------------
 *
 * @command Separator_SelfSwitch
 * @text -
 * @desc -
 *
 * @ --------------------------------------------------------------------------
 *
 * @command SelfSwitchABCD
 * @text Self Switch: A B C D
 * @desc Change the Self Switch of a different event.
 *
 * @arg MapId:eval
 * @text Map ID
 * @desc The map the target map. Use 0 for current map.
 * You may use JavaScript code.
 * @default 0
 *
 * @arg EventId:eval
 * @text Event ID
 * @desc The ID of the target event. Use 0 for current event.
 * You may use JavaScript code.
 * @default 0
 *
 * @arg Letter:str
 * @text Letter
 * @type select
 * @option A
 * @value A
 * @option B
 * @value B
 * @option C
 * @value C
 * @option D
 * @value D
 * @desc Letter of the target event's Self Switch to change.
 * @default A
 *
 * @arg Break
 * @text -
 *
 * @arg Value:str
 * @text Value
 * @type select
 * @option ON
 * @value ON
 * @option OFF
 * @value OFF
 * @option Toggle
 * @value Toggle
 * @desc What value do you want to set the Self Switch to?
 * @default ON
 *
 * @ --------------------------------------------------------------------------
 *
 * @command SelfSwitchID
 * @text Self Switch: Switch ID
 * @desc Change the Self Switch of a different event.
 *
 * @arg MapId:eval
 * @text Map ID
 * @desc The map the target map. Use 0 for current map.
 * You may use JavaScript code.
 * @default 0
 *
 * @arg EventId:eval
 * @text Event ID
 * @desc The ID of the target event. Use 0 for current event.
 * You may use JavaScript code.
 * @default 0
 *
 * @arg SwitchId:num
 * @text Switch ID
 * @type switch
 * @desc The ID of the target switch.
 * @default 1
 *
 * @arg Break
 * @text -
 *
 * @arg Value:str
 * @text Value
 * @type select
 * @option ON
 * @value ON
 * @option OFF
 * @value OFF
 * @option Toggle
 * @value Toggle
 * @desc What value do you want to set the Self Switch to?
 * @default ON
 *
 * @ --------------------------------------------------------------------------
 *
 * @command Separator_SelfVar
 * @text -
 * @desc -
 *
 * @ --------------------------------------------------------------------------
 *
 * @command SelfVariableID
 * @text Self Variable: Variable ID
 * @desc Change the Self Variable of a different event.
 *
 * @arg MapId:eval
 * @text Map ID
 * @desc The map the target map. Use 0 for current map.
 * You may use JavaScript code.
 * @default 0
 *
 * @arg EventId:eval
 * @text Event ID
 * @desc The ID of the target event. Use 0 for current event.
 * You may use JavaScript code.
 * @default 0
 *
 * @arg VariableId:num
 * @text Variable ID
 * @type variable
 * @desc The ID of the target variable.
 * @default 1
 *
 * @arg Break
 * @text -
 *
 * @arg Operation:str
 * @text Operation
 * @type select
 * @option = Set
 * @value =
 * @option + Add
 * @value +
 * @option - Subtract
 * @value -
 * @option * Multiply
 * @value *
 * @option / Divide
 * @value /
 * @option % Modulus
 * @value %
 * @desc Set the operation used.
 * @default =
 *
 * @arg Break2
 * @text -
 *
 * @arg Value:eval
 * @text Value
 * @desc Insert the value to modify the Self Variable by.
 * You may use JavaScript code.
 * @default 0
 *
 * @ --------------------------------------------------------------------------
 *
 * @command Separator_Shadow
 * @text -
 * @desc -
 *
 * @ --------------------------------------------------------------------------
 *
 * @command ShadowHidePlayer
 * @text Shadow Hide: Player
 * @desc Hides the visibility of the player sprite shadow.
 * 
 * @ --------------------------------------------------------------------------
 *
 * @command ShadowHideFollowers
 * @text Shadow Hide: Followers
 * @desc Hides the visibility of follower sprite shadows.
 * 
 * @ --------------------------------------------------------------------------
 *
 * @command ShadowHideAllEvents
 * @text Shadow Hide: All Events
 * @desc Hides the visibility of all event sprite shadows.
 * 
 * @ --------------------------------------------------------------------------
 *
 * @command ShadowShowPlayer
 * @text Shadow Show: Player
 * @desc Returns the visibility of the player sprite shadow.
 * Does NOT override Plugin Parameter "Shadows > Show" if off.
 * 
 * @ --------------------------------------------------------------------------
 *
 * @command ShadowShowFollowers
 * @text Shadow Show: Followers
 * @desc Returns the visibility of follower sprite shadows.
 * Does NOT override Plugin Parameter "Shadows > Show" if off.
 * 
 * @ --------------------------------------------------------------------------
 *
 * @command ShadowShowAllEvents
 * @text Shadow Show: All Events
 * @desc Returns the visibility of all event sprite shadows.
 * Does NOT override Plugin Parameter or <Hide Shadow> notetag.
 * 
 * @ --------------------------------------------------------------------------
 *
 * @command Separator_SpawnEvent
 * @text -
 * @desc -
 *
 * @ --------------------------------------------------------------------------
 *
 * @command SpawnEventAtXY
 * @text Spawn Event: Spawn At X, Y
 * @desc Spawns desired event at X, Y location on the current map.
 *
 * @arg Step1
 * @text Step 1: Spawned Event
 *
 * @arg TemplateName:str
 * @text Template Name
 * @parent Step1
 * @desc Name of the target event template to spawn as.
 * Ignored if this is called "Untitled".
 * @default Untitled
 *
 * @arg MapId:eval
 * @text Map ID
 * @parent Step1
 * @desc Target event's map to be used as reference.
 * You may use JavaScript code.
 * @default 1
 *
 * @arg EventId:eval
 * @text Event ID
 * @parent Step1
 * @desc The ID of the target event to be used as reference.
 * You may use JavaScript code.
 * @default 1
 *
 * @arg Step2
 * @text Step 2: Location
 *
 * @arg PosX:eval
 * @text X Coordinate
 * @parent Step2
 * @type combo
 * @option $gamePlayer.frontX()
 * @option $gamePlayer.backX()
 * @option Math.randomInt($gameMap.width())
 * @option 0
 * @desc Target Location to spawn at.
 * You may use JavaScript code.
 * @default $gamePlayer.frontX()
 *
 * @arg PosY:eval
 * @text Y Coordinate
 * @parent Step2
 * @type combo
 * @option $gamePlayer.frontY()
 * @option $gamePlayer.backY()
 * @option Math.randomInt($gameMap.height())
 * @option 0
 * @desc Target Location to spawn at.
 * You may use JavaScript code.
 * @default $gamePlayer.frontY()
 *
 * @arg Collision:eval
 * @text Check Event Collision
 * @parent Step2
 * @type boolean
 * @on Check
 * @off Ignore
 * @desc Check collision with any other events and player?
 * @default true
 *
 * @arg Passability:eval
 * @text Check Passability
 * @parent Step2
 * @type boolean
 * @on Check
 * @off Ignore
 * @desc Check passability of the target location.
 * @default true
 *
 * @arg Preserve:eval
 * @text Preserve Spawn
 * @parent Step2
 * @type boolean
 * @on Preserve
 * @off Expires
 * @desc Is the spawned event preserved?
 * Or does it expire upon leaving the map?
 * @default true
 *
 * @arg Step3
 * @text Step 3: Success Check
 *
 * @arg SuccessSwitchId:num
 * @text Success Switch ID
 * @parent Step3
 * @type switch
 * @desc Target switch ID to record spawning success.
 * Ignore if ID is 0. OFF means failed. ON means success.
 * @default 0
 *
 * @ --------------------------------------------------------------------------
 *
 * @command SpawnEventAtRegion
 * @text Spawn Event: Spawn At Region
 * @desc Spawns desired event at a random region-marked location on the current map.
 *
 * @arg Step1
 * @text Step 1: Spawned Event
 *
 * @arg TemplateName:str
 * @text Template Name
 * @parent Step1
 * @desc Name of the target event template to spawn as.
 * Ignored if this is called "Untitled".
 * @default Untitled
 *
 * @arg MapId:eval
 * @text Map ID
 * @parent Step1
 * @desc Target event's map.
 * You may use JavaScript code.
 * @default 1
 *
 * @arg EventId:eval
 * @text Event ID
 * @parent Step1
 * @desc The ID of the target event.
 * You may use JavaScript code.
 * @default 1
 *
 * @arg Step2
 * @text Step 2: Location
 *
 * @arg Region:arraynum
 * @text Region ID(s)
 * @parent Step2
 * @type number[]
 * @min 0
 * @max 255
 * @desc Pick region(s) to spawn this event at.
 * @default ["1"]
 *
 * @arg Collision:eval
 * @text Check Event Collision
 * @parent Step2
 * @type boolean
 * @on Check
 * @off Ignore
 * @desc Check collision with any other events and player?
 * @default true
 *
 * @arg Passability:eval
 * @text Check Passability
 * @parent Step2
 * @type boolean
 * @on Check
 * @off Ignore
 * @desc Check passability of the target location.
 * @default true
 *
 * @arg Preserve:eval
 * @text Preserve Spawn
 * @parent Step2
 * @type boolean
 * @on Preserve
 * @off Expires
 * @desc Is the spawned event preserved?
 * Or does it expire upon leaving the map?
 * @default true
 *
 * @arg Step3
 * @text Step 3: Success Check
 *
 * @arg SuccessSwitchId:num
 * @text Success Switch ID
 * @parent Step3
 * @type switch
 * @desc Target switch ID to record spawning success.
 * Ignore if ID is 0. OFF means failed. ON means success.
 * @default 0
 *
 * @ --------------------------------------------------------------------------
 *
 * @command SpawnEventAtTerrainTag
 * @text Spawn Event: Spawn At Terrain Tag
 * @desc Spawns desired event at a random terrain tag-marked location on the current map.
 *
 * @arg Step1
 * @text Step 1: Spawned Event
 *
 * @arg TemplateName:str
 * @text Template Name
 * @parent Step1
 * @desc Name of the target event template to spawn as.
 * Ignored if this is called "Untitled".
 * @default Untitled
 *
 * @arg MapId:eval
 * @text Map ID
 * @parent Step1
 * @desc Target event's map.
 * You may use JavaScript code.
 * @default 1
 *
 * @arg EventId:eval
 * @text Event ID
 * @parent Step1
 * @desc The ID of the target event.
 * You may use JavaScript code.
 * @default 1
 *
 * @arg Step2
 * @text Step 2: Location
 *
 * @arg TerrainTags:arraynum
 * @text Terrain Tag(s)
 * @parent Step2
 * @type number[]
 * @min 0
 * @max 7
 * @desc Pick terrain tag(s) to spawn this event at.
 * Insert numbers between 0 and 7.
 * @default ["1"]
 *
 * @arg Collision:eval
 * @text Check Event Collision
 * @parent Step2
 * @type boolean
 * @on Check
 * @off Ignore
 * @desc Check collision with any other events and player?
 * @default true
 *
 * @arg Passability:eval
 * @text Check Passability
 * @parent Step2
 * @type boolean
 * @on Check
 * @off Ignore
 * @desc Check passability of the target location.
 * @default true
 *
 * @arg Preserve:eval
 * @text Preserve Spawn
 * @parent Step2
 * @type boolean
 * @on Preserve
 * @off Expires
 * @desc Is the spawned event preserved?
 * Or does it expire upon leaving the map?
 * @default true
 *
 * @arg Step3
 * @text Step 3: Success Check
 *
 * @arg SuccessSwitchId:num
 * @text Success Switch ID
 * @parent Step3
 * @type switch
 * @desc Target switch ID to record spawning success.
 * Ignore if ID is 0. OFF means failed. ON means success.
 * @default 0
 *
 * @ --------------------------------------------------------------------------
 *
 * @command SpawnEventDespawnEventID
 * @text Spawn Event: Despawn Event ID
 * @desc Despawns the selected Event ID on the current map.
 *
 * @arg EventID:eval
 * @text Event ID
 * @type combo
 * @option $gameMap.firstSpawnedEventID()
 * @option $gameMap.lastSpawnedEventID()
 * @option 1001
 * @desc The ID of the target event.
 * You may use JavaScript code.
 * @default $gameMap.lastSpawnedEventID()
 *
 * @ --------------------------------------------------------------------------
 *
 * @command SpawnEventDespawnAtXY
 * @text Spawn Event: Despawn At X, Y
 * @desc Despawns any spawned event(s) at X, Y location on the current map.
 *
 * @arg PosX:eval
 * @text X Coordinate
 * @parent Step2
 * @type combo
 * @option $gamePlayer.frontX()
 * @option $gamePlayer.backX()
 * @option Math.randomInt($gameMap.width())
 * @option 0
 * @desc Target Location to despawn at.
 * You may use JavaScript code.
 * @default $gamePlayer.frontX()
 *
 * @arg PosY:eval
 * @text Y Coordinate
 * @parent Step2
 * @type combo
 * @option $gamePlayer.frontY()
 * @option $gamePlayer.backY()
 * @option Math.randomInt($gameMap.height())
 * @option 0
 * @desc Target Location to despawn at.
 * You may use JavaScript code.
 * @default $gamePlayer.frontY()
 *
 * @ --------------------------------------------------------------------------
 *
 * @command SpawnEventDespawnRegions
 * @text Spawn Event: Despawn Region(s)
 * @desc Despawns the selected Region(s) on the current map.
 *
 * @arg Region:arraynum
 * @text Region ID(s)
 * @parent Step2
 * @type number[]
 * @min 0
 * @max 255
 * @desc Pick region(s) and despawn everything inside it.
 * @default ["1"]
 *
 * @ --------------------------------------------------------------------------
 *
 * @command SpawnEventDespawnTerrainTags
 * @text Spawn Event: Despawn Terrain Tag(s)
 * @desc Despawns the selected Terrain Tags(s) on the current map.
 *
 * @arg TerrainTags:arraynum
 * @text Terrain Tag(s)
 * @parent Step2
 * @type number[]
 * @min 0
 * @max 7
 * @desc Pick terrain tag(s) and despawn everything inside it.
 * Insert numbers between 0 and 7.
 * @default ["1"]
 *
 * @ --------------------------------------------------------------------------
 *
 * @command SpawnEventDespawnEverything
 * @text Spawn Event: Despawn Everything
 * @desc Despawns all spawned events on the current map.
 *
 * @ --------------------------------------------------------------------------
 *
 * @command Separator_End
 * @text -
 * @desc -
 *
 * @ --------------------------------------------------------------------------
 *
 * @ ==========================================================================
 * @ Plugin Parameters
 * @ ==========================================================================
 *
 * @param BreakHead
 * @text --------------------------
 * @default ----------------------------------
 *
 * @param EventsMoveCore
 * @default Plugin Parameters
 *
 * @param ATTENTION
 * @default READ THE HELP FILE
 *
 * @param BreakSettings
 * @text --------------------------
 * @default ----------------------------------
 *
 * @param Label:struct
 * @text Event Label Settings
 * @type struct<Label>
 * @desc Choose settings regarding the Event Labels.
 * @default {"FontSize:num":"22","IconSize:num":"26","LineHeight:num":"30","OffsetX:num":"0","OffsetY:num":"12","OpacitySpeed:num":"16","VisibleRange:num":"30"}
 *
 * @param Icon:struct
 * @text Event Icon Settings
 * @type struct<Icon>
 * @desc Choose settings regarding the Event Icons.
 * @default {"BufferX:num":"0","BufferY:num":"12","BlendMode:num":"0"}
 *
 * @param Template:struct
 * @text Event Template Settings
 * @type struct<Template>
 * @desc Choose settings regarding Event Templates.
 * @default {"Settings":"","PreloadMaps:arraynum":"[\"1\"]","Prefabs":"","List:arraystruct":"[]","JavaScript":"","PreCopyJS:func":"\"// Declare Constants\\nconst mapId = arguments[1];\\nconst eventId = arguments[2];\\nconst target = arguments[3];\\nconst player = $gamePlayer;\\n\\n// Perform Actions\\n\"","PostCopyJS:func":"\"// Declare Constants\\nconst mapId = arguments[1];\\nconst eventId = arguments[2];\\nconst target = arguments[3];\\nconst player = $gamePlayer;\\n\\n// Perform Actions\\n\"","PreMorphJS:func":"\"// Declare Constants\\nconst mapId = arguments[1];\\nconst eventId = arguments[2];\\nconst target = arguments[3];\\nconst player = $gamePlayer;\\n\\n// Perform Actions\\n\"","PostMorphJS:func":"\"// Declare Constants\\nconst mapId = arguments[1];\\nconst eventId = arguments[2];\\nconst target = arguments[3];\\nconst player = $gamePlayer;\\n\\n// Perform Actions\\n\"","PreSpawnJS:func":"\"// Declare Constants\\nconst mapId = arguments[1];\\nconst eventId = arguments[2];\\nconst target = arguments[3];\\nconst player = $gamePlayer;\\n\\n// Perform Actions\\n\"","PostSpawnJS:func":"\"// Declare Constants\\nconst mapId = arguments[1];\\nconst eventId = arguments[2];\\nconst target = arguments[3];\\nconst player = $gamePlayer;\\n\\n// Perform Actions\\n\""}
 *
 * @param EventBreak
 * @text --------------------------
 * @default ----------------------------------
 *
 * @param Movement:struct
 * @text Movement Settings
 * @type struct<Movement>
 * @desc Change the rules regarding movement in the game.
 * @default {"Dir8":"","EnableDir8:eval":"true","StrictCollision:eval":"true","FavorHorz:eval":"true","SlowerSpeed:eval":"false","DiagonalSpeedMultiplier:num":"0.85","AutoMove":"","StopAutoMoveEvents:eval":"true","StopAutoMoveMessages:eval":"true","Bitmap":"","BitmapSmoothing:eval":"false","Dash":"","DashModifier:num":"+1.0","EnableDashTilt:eval":"true","TiltLeft:num":"-0.15","TiltRight:num":"0.15","TiltVert:num":"0.05","EventMove":"","RandomMoveWeight:num":"0.10","Shadows":"","ShowShadows:eval":"true","DefaultShadow:str":"Shadow1","TurnInPlace":"","EnableTurnInPlace:eval":"false","TurnInPlaceDelay:num":"10","Vehicle":"","BoatSpeed:num":"4.0","ShipSpeed:num":"5.0","AirshipSpeed:num":"6.0"}
 *
 * @param VS8:struct
 * @text VisuStella 8-Dir Settings
 * @type struct<VS8>
 * @desc Choose settings regarding VisuStella 8-Directional Sprites.
 * @default {"Balloons":"","AutoBalloon:eval":"true","BalloonOffsetX:num":"0","BalloonOffsetY:num":"12","Icons":"","AutoBuffer:eval":"true","CarryPose:eval":"true"}
 *
 * @param MovementBreak
 * @text --------------------------
 * @default ----------------------------------
 *
 * @param Region:struct
 * @text Region Rulings
 * @type struct<Region>
 * @desc Choose settings regarding regions.
 * @default {"Allow":"","AllAllow:arraynum":"[]","WalkAllow:arraynum":"[]","PlayerAllow:arraynum":"[]","EventAllow:arraynum":"[]","VehicleAllow:arraynum":"[]","BoatAllow:arraynum":"[]","ShipAllow:arraynum":"[]","AirshipAllow:arraynum":"[]","Forbid":"","AllForbid:arraynum":"[]","WalkForbid:arraynum":"[]","PlayerForbid:arraynum":"[]","EventForbid:arraynum":"[]","VehicleForbid:arraynum":"[]","BoatForbid:arraynum":"[]","ShipForbid:arraynum":"[]","AirshipForbid:arraynum":"[]","Dock":"","VehicleDock:arraynum":"[]","BoatDock:arraynum":"[]","BoatDockRegionOnly:eval":"false","ShipDock:arraynum":"[]","ShipDockRegionOnly:eval":"false","AirshipDock:arraynum":"[]","AirshipDockRegionOnly:eval":"false"}
 *
 * @param RegionOk:struct
 * @text Common Event on OK Button
 * @parent Region:struct
 * @type struct<RegionCommonEvent>
 * @desc Set Common Events that activate upon pressing the
 * OK button while standing on top of designated regions.
 * @default {"Region1:num":"0","Region2:num":"0","Region3:num":"0","Region4:num":"0","Region5:num":"0","Region6:num":"0","Region7:num":"0","Region8:num":"0","Region9:num":"0","Region10:num":"0","Region11:num":"0","Region12:num":"0","Region13:num":"0","Region14:num":"0","Region15:num":"0","Region16:num":"0","Region17:num":"0","Region18:num":"0","Region19:num":"0","Region20:num":"0","Region21:num":"0","Region22:num":"0","Region23:num":"0","Region24:num":"0","Region25:num":"0","Region26:num":"0","Region27:num":"0","Region28:num":"0","Region29:num":"0","Region30:num":"0","Region31:num":"0","Region32:num":"0","Region33:num":"0","Region34:num":"0","Region35:num":"0","Region36:num":"0","Region37:num":"0","Region38:num":"0","Region39:num":"0","Region40:num":"0","Region41:num":"0","Region42:num":"0","Region43:num":"0","Region44:num":"0","Region45:num":"0","Region46:num":"0","Region47:num":"0","Region48:num":"0","Region49:num":"0","Region50:num":"0","Region51:num":"0","Region52:num":"0","Region53:num":"0","Region54:num":"0","Region55:num":"0","Region56:num":"0","Region57:num":"0","Region58:num":"0","Region59:num":"0","Region60:num":"0","Region61:num":"0","Region62:num":"0","Region63:num":"0","Region64:num":"0","Region65:num":"0","Region66:num":"0","Region67:num":"0","Region68:num":"0","Region69:num":"0","Region70:num":"0","Region71:num":"0","Region72:num":"0","Region73:num":"0","Region74:num":"0","Region75:num":"0","Region76:num":"0","Region77:num":"0","Region78:num":"0","Region79:num":"0","Region80:num":"0","Region81:num":"0","Region82:num":"0","Region83:num":"0","Region84:num":"0","Region85:num":"0","Region86:num":"0","Region87:num":"0","Region88:num":"0","Region89:num":"0","Region90:num":"0","Region91:num":"0","Region92:num":"0","Region93:num":"0","Region94:num":"0","Region95:num":"0","Region96:num":"0","Region97:num":"0","Region98:num":"0","Region99:num":"0","Region100:num":"0","Region101:num":"0","Region102:num":"0","Region103:num":"0","Region104:num":"0","Region105:num":"0","Region106:num":"0","Region107:num":"0","Region108:num":"0","Region109:num":"0","Region110:num":"0","Region111:num":"0","Region112:num":"0","Region113:num":"0","Region114:num":"0","Region115:num":"0","Region116:num":"0","Region117:num":"0","Region118:num":"0","Region119:num":"0","Region120:num":"0","Region121:num":"0","Region122:num":"0","Region123:num":"0","Region124:num":"0","Region125:num":"0","Region126:num":"0","Region127:num":"0","Region128:num":"0","Region129:num":"0","Region130:num":"0","Region131:num":"0","Region132:num":"0","Region133:num":"0","Region134:num":"0","Region135:num":"0","Region136:num":"0","Region137:num":"0","Region138:num":"0","Region139:num":"0","Region140:num":"0","Region141:num":"0","Region142:num":"0","Region143:num":"0","Region144:num":"0","Region145:num":"0","Region146:num":"0","Region147:num":"0","Region148:num":"0","Region149:num":"0","Region150:num":"0","Region151:num":"0","Region152:num":"0","Region153:num":"0","Region154:num":"0","Region155:num":"0","Region156:num":"0","Region157:num":"0","Region158:num":"0","Region159:num":"0","Region160:num":"0","Region161:num":"0","Region162:num":"0","Region163:num":"0","Region164:num":"0","Region165:num":"0","Region166:num":"0","Region167:num":"0","Region168:num":"0","Region169:num":"0","Region170:num":"0","Region171:num":"0","Region172:num":"0","Region173:num":"0","Region174:num":"0","Region175:num":"0","Region176:num":"0","Region177:num":"0","Region178:num":"0","Region179:num":"0","Region180:num":"0","Region181:num":"0","Region182:num":"0","Region183:num":"0","Region184:num":"0","Region185:num":"0","Region186:num":"0","Region187:num":"0","Region188:num":"0","Region189:num":"0","Region190:num":"0","Region191:num":"0","Region192:num":"0","Region193:num":"0","Region194:num":"0","Region195:num":"0","Region196:num":"0","Region197:num":"0","Region198:num":"0","Region199:num":"0","Region200:num":"0","Region201:num":"0","Region202:num":"0","Region203:num":"0","Region204:num":"0","Region205:num":"0","Region206:num":"0","Region207:num":"0","Region208:num":"0","Region209:num":"0","Region210:num":"0","Region211:num":"0","Region212:num":"0","Region213:num":"0","Region214:num":"0","Region215:num":"0","Region216:num":"0","Region217:num":"0","Region218:num":"0","Region219:num":"0","Region220:num":"0","Region221:num":"0","Region222:num":"0","Region223:num":"0","Region224:num":"0","Region225:num":"0","Region226:num":"0","Region227:num":"0","Region228:num":"0","Region229:num":"0","Region230:num":"0","Region231:num":"0","Region232:num":"0","Region233:num":"0","Region234:num":"0","Region235:num":"0","Region236:num":"0","Region237:num":"0","Region238:num":"0","Region239:num":"0","Region240:num":"0","Region241:num":"0","Region242:num":"0","Region243:num":"0","Region244:num":"0","Region245:num":"0","Region246:num":"0","Region247:num":"0","Region248:num":"0","Region249:num":"0","Region250:num":"0","Region251:num":"0","Region252:num":"0","Region253:num":"0","Region254:num":"0","Region255:num":"0"}
 *
 * @param RegionOkTarget:str
 * @text Target Tile
 * @parent RegionOk:struct
 * @type select
 * @option Tile in front of player.
 * @value front
 * @option Tile player is standing on top of.
 * @value standing
 * @desc Which tile should be checked for
 * Common Event on OK Button?
 * @default front
 *
 * @param RegionTouch:struct
 * @text Common Event on Touch
 * @parent Region:struct
 * @type struct<RegionCommonEvent>
 * @desc Set Common Events that activate upon stepping the tiles
 * marked by the designated regions.
 * @default {"Region1:num":"0","Region2:num":"0","Region3:num":"0","Region4:num":"0","Region5:num":"0","Region6:num":"0","Region7:num":"0","Region8:num":"0","Region9:num":"0","Region10:num":"0","Region11:num":"0","Region12:num":"0","Region13:num":"0","Region14:num":"0","Region15:num":"0","Region16:num":"0","Region17:num":"0","Region18:num":"0","Region19:num":"0","Region20:num":"0","Region21:num":"0","Region22:num":"0","Region23:num":"0","Region24:num":"0","Region25:num":"0","Region26:num":"0","Region27:num":"0","Region28:num":"0","Region29:num":"0","Region30:num":"0","Region31:num":"0","Region32:num":"0","Region33:num":"0","Region34:num":"0","Region35:num":"0","Region36:num":"0","Region37:num":"0","Region38:num":"0","Region39:num":"0","Region40:num":"0","Region41:num":"0","Region42:num":"0","Region43:num":"0","Region44:num":"0","Region45:num":"0","Region46:num":"0","Region47:num":"0","Region48:num":"0","Region49:num":"0","Region50:num":"0","Region51:num":"0","Region52:num":"0","Region53:num":"0","Region54:num":"0","Region55:num":"0","Region56:num":"0","Region57:num":"0","Region58:num":"0","Region59:num":"0","Region60:num":"0","Region61:num":"0","Region62:num":"0","Region63:num":"0","Region64:num":"0","Region65:num":"0","Region66:num":"0","Region67:num":"0","Region68:num":"0","Region69:num":"0","Region70:num":"0","Region71:num":"0","Region72:num":"0","Region73:num":"0","Region74:num":"0","Region75:num":"0","Region76:num":"0","Region77:num":"0","Region78:num":"0","Region79:num":"0","Region80:num":"0","Region81:num":"0","Region82:num":"0","Region83:num":"0","Region84:num":"0","Region85:num":"0","Region86:num":"0","Region87:num":"0","Region88:num":"0","Region89:num":"0","Region90:num":"0","Region91:num":"0","Region92:num":"0","Region93:num":"0","Region94:num":"0","Region95:num":"0","Region96:num":"0","Region97:num":"0","Region98:num":"0","Region99:num":"0","Region100:num":"0","Region101:num":"0","Region102:num":"0","Region103:num":"0","Region104:num":"0","Region105:num":"0","Region106:num":"0","Region107:num":"0","Region108:num":"0","Region109:num":"0","Region110:num":"0","Region111:num":"0","Region112:num":"0","Region113:num":"0","Region114:num":"0","Region115:num":"0","Region116:num":"0","Region117:num":"0","Region118:num":"0","Region119:num":"0","Region120:num":"0","Region121:num":"0","Region122:num":"0","Region123:num":"0","Region124:num":"0","Region125:num":"0","Region126:num":"0","Region127:num":"0","Region128:num":"0","Region129:num":"0","Region130:num":"0","Region131:num":"0","Region132:num":"0","Region133:num":"0","Region134:num":"0","Region135:num":"0","Region136:num":"0","Region137:num":"0","Region138:num":"0","Region139:num":"0","Region140:num":"0","Region141:num":"0","Region142:num":"0","Region143:num":"0","Region144:num":"0","Region145:num":"0","Region146:num":"0","Region147:num":"0","Region148:num":"0","Region149:num":"0","Region150:num":"0","Region151:num":"0","Region152:num":"0","Region153:num":"0","Region154:num":"0","Region155:num":"0","Region156:num":"0","Region157:num":"0","Region158:num":"0","Region159:num":"0","Region160:num":"0","Region161:num":"0","Region162:num":"0","Region163:num":"0","Region164:num":"0","Region165:num":"0","Region166:num":"0","Region167:num":"0","Region168:num":"0","Region169:num":"0","Region170:num":"0","Region171:num":"0","Region172:num":"0","Region173:num":"0","Region174:num":"0","Region175:num":"0","Region176:num":"0","Region177:num":"0","Region178:num":"0","Region179:num":"0","Region180:num":"0","Region181:num":"0","Region182:num":"0","Region183:num":"0","Region184:num":"0","Region185:num":"0","Region186:num":"0","Region187:num":"0","Region188:num":"0","Region189:num":"0","Region190:num":"0","Region191:num":"0","Region192:num":"0","Region193:num":"0","Region194:num":"0","Region195:num":"0","Region196:num":"0","Region197:num":"0","Region198:num":"0","Region199:num":"0","Region200:num":"0","Region201:num":"0","Region202:num":"0","Region203:num":"0","Region204:num":"0","Region205:num":"0","Region206:num":"0","Region207:num":"0","Region208:num":"0","Region209:num":"0","Region210:num":"0","Region211:num":"0","Region212:num":"0","Region213:num":"0","Region214:num":"0","Region215:num":"0","Region216:num":"0","Region217:num":"0","Region218:num":"0","Region219:num":"0","Region220:num":"0","Region221:num":"0","Region222:num":"0","Region223:num":"0","Region224:num":"0","Region225:num":"0","Region226:num":"0","Region227:num":"0","Region228:num":"0","Region229:num":"0","Region230:num":"0","Region231:num":"0","Region232:num":"0","Region233:num":"0","Region234:num":"0","Region235:num":"0","Region236:num":"0","Region237:num":"0","Region238:num":"0","Region239:num":"0","Region240:num":"0","Region241:num":"0","Region242:num":"0","Region243:num":"0","Region244:num":"0","Region245:num":"0","Region246:num":"0","Region247:num":"0","Region248:num":"0","Region249:num":"0","Region250:num":"0","Region251:num":"0","Region252:num":"0","Region253:num":"0","Region254:num":"0","Region255:num":"0"}
 *
 * @param TerrainTag:struct
 * @text Terrain Tag Settings
 * @type struct<TerrainTag>
 * @desc Choose settings regarding terrain tags.
 * @default {"TerrainTag":"","Rope:num":"1"}
 *
 * @param BreakEnd1
 * @text --------------------------
 * @default ----------------------------------
 *
 * @param End Of
 * @default Plugin Parameters
 *
 * @param BreakEnd2
 * @text --------------------------
 * @default ----------------------------------
 *
 */
/* ----------------------------------------------------------------------------
 * Label Settings
 * ----------------------------------------------------------------------------
 */
/*~struct~Label:
 *
 * @param SpriteBased:eval
 * @text Sprite Based?
 * @type boolean
 * @on Sprite-Based
 * @off Legacy-Window
 * @desc Use sprite-based labels instead of legacy-window version.
 * Legacy-window version will not be supported in future.
 * @default true
 *
 * @param MobileEnabled:eval
 * @text Mobile-Enabled?
 * @type boolean
 * @on Enabled
 * @off Disabled
 * @desc Enable event labels for mobile devices?
 * @default true
 *
 * @param FontSize:num
 * @text Font Size
 * @type number
 * @min 1
 * @desc The font size used for the Event Labels.
 * @default 22
 *
 * @param IconSize:num
 * @text Icon Size
 * @type number
 * @min 1
 * @desc The size of the icons used in the Event Labels.
 * @default 26
 *
 * @param LineHeight:num
 * @text Line Height
 * @type number
 * @min 1
 * @desc The line height used for the Event Labels.
 * @default 26
 *
 * @param OffsetX:num
 * @text Offset X
 * @type number
 * @min 0
 * @desc Globally offset all labels horizontally by this amount.
 * @default 0
 *
 * @param OffsetY:num
 * @text Offset Y
 * @type number
 * @min 0
 * @desc Globally offset all labels vertically by this amount.
 * @default 12
 *
 * @param OpacitySpeed:num
 * @text Fade Speed
 * @type number
 * @min 1
 * @desc Fade speed for labels.
 * @default 16
 *
 * @param VisibleRange:num
 * @text Visible Range
 * @type number
 * @min 1
 * @desc Range the player has to be within the event to make its label visible.
 * @default 30
 *
 * @param RangeType:str
 * @text Range Type
 * @parent VisibleRange:num
 * @type select
 * @option square
 * @option circle
 * @option diamond
 * @desc What do you want the default label visible range type?
 * @default square
 *
 */
/* ----------------------------------------------------------------------------
 * Icon Settings
 * ----------------------------------------------------------------------------
 */
/*~struct~Icon:
 *
 * @param BufferX:num
 * @text Buffer X
 * @desc Default X position buffer for event icons.
 * @default 0
 *
 * @param BufferY:num
 * @text Buffer Y
 * @desc Default Y position buffer for event icons.
 * @default 12
 *
 * @param BlendMode:num
 * @text Blend Mode
 * @type select
 * @option 0 - Normal
 * @value 0
 * @option 1 - Additive
 * @value 1
 * @option 2 - Multiply
 * @value 2
 * @option 3 - Screen
 * @value 3
 * @desc Default blend mode for even icons.
 * @default 0
 *
 */
/* ----------------------------------------------------------------------------
 * Template Settings
 * ----------------------------------------------------------------------------
 */
/*~struct~Template:
 *
 * @param Settings
 *
 * @param PreloadMaps:arraynum
 * @text Preloaded Maps
 * @parent Settings
 * @type number[]
 * @desc A list of all the ID's of the maps that will be preloaded
 * to serve as template maps for this plugin.
 * @default ["1"]
 *
 * @param Templates
 *
 * @param List:arraystruct
 * @text Event Template List
 * @parent Templates
 * @type struct<EventTemplate>[]
 * @desc A list of all the Event Templates used by this project.
 * Used for notetags and Plugin Commands.
 * @default []
 *
 * @param JavaScript
 *
 * @param PreCopyJS:func
 * @text JS: Pre-Copy
 * @parent JavaScript
 * @type note
 * @desc Code that's ran before an event is copied.
 * This is global and is ran for all copied events.
 * @default "// Declare Constants\nconst mapId = arguments[1];\nconst eventId = arguments[2];\nconst target = arguments[3];\nconst player = $gamePlayer;\n\n// Perform Actions\n"
 *
 * @param PostCopyJS:func
 * @text JS: Post-Copy
 * @parent JavaScript
 * @type note
 * @desc Code that's ran after an event is copied.
 * This is global and is ran for all copied events.
 * @default "// Declare Constants\nconst mapId = arguments[1];\nconst eventId = arguments[2];\nconst target = arguments[3];\nconst player = $gamePlayer;\n\n// Perform Actions\n"
 *
 * @param PreMorphJS:func
 * @text JS: Pre-Morph
 * @parent JavaScript
 * @type note
 * @desc Code that's ran before an event is morphed.
 * This is global and is ran for all morphed events.
 * @default "// Declare Constants\nconst mapId = arguments[1];\nconst eventId = arguments[2];\nconst target = arguments[3];\nconst player = $gamePlayer;\n\n// Perform Actions\n"
 *
 * @param PostMorphJS:func
 * @text JS: Post-Morph
 * @parent JavaScript
 * @type note
 * @desc Code that's ran after an event is morphed.
 * This is global and is ran for all morphed events.
 * @default "// Declare Constants\nconst mapId = arguments[1];\nconst eventId = arguments[2];\nconst target = arguments[3];\nconst player = $gamePlayer;\n\n// Perform Actions\n"
 *
 * @param PreSpawnJS:func
 * @text JS: Pre-Spawn
 * @parent JavaScript
 * @type note
 * @desc Code that's ran before an event is spawned.
 * This is global and is ran for all spawned events.
 * @default "// Declare Constants\nconst mapId = arguments[1];\nconst eventId = arguments[2];\nconst target = arguments[3];\nconst player = $gamePlayer;\n\n// Perform Actions\n"
 *
 * @param PostSpawnJS:func
 * @text JS: Post-Spawn
 * @parent JavaScript
 * @type note
 * @desc Code that's ran after an event is spawned.
 * This is global and is ran for all spawned events.
 * @default "// Declare Constants\nconst mapId = arguments[1];\nconst eventId = arguments[2];\nconst target = arguments[3];\nconst player = $gamePlayer;\n\n// Perform Actions\n"
 *
 */
/* ----------------------------------------------------------------------------
 * Event Template
 * ----------------------------------------------------------------------------
 */
/*~struct~EventTemplate:
 *
 * @param Name:str
 * @text Name
 * @desc Name of the template. It'll be used as anchor points for
 * notetags and Plugin Commands.
 * @default Untitled
 *
 * @param MapID:num
 * @text Map ID
 * @parent Name:str
 * @type number
 * @min 1
 * @max 999
 * @desc ID of the map the template event is stored on.
 * This will automatically add this ID to preloaded list.
 * @default 1
 *
 * @param EventID:num
 * @text Event ID
 * @parent Name:str
 * @type number
 * @min 1
 * @max 999
 * @desc ID of the event the template event is based on.
 * @default 1
 *
 * @param JavaScript
 *
 * @param PreCopyJS:func
 * @text JS: Pre-Copy
 * @parent JavaScript
 * @type note
 * @desc Code that's ran before an event is copied.
 * This is local only for this template.
 * @default "// Declare Constants\nconst mapId = arguments[1];\nconst eventId = arguments[2];\nconst target = arguments[3];\nconst player = $gamePlayer;\n\n// Perform Actions\n"
 *
 * @param PostCopyJS:func
 * @text JS: Post-Copy
 * @parent JavaScript
 * @type note
 * @desc Code that's ran after an event is copied.
 * This is local only for this template.
 * @default "// Declare Constants\nconst mapId = arguments[1];\nconst eventId = arguments[2];\nconst target = arguments[3];\nconst player = $gamePlayer;\n\n// Perform Actions\n"
 *
 * @param PreMorphJS:func
 * @text JS: Pre-Morph
 * @parent JavaScript
 * @type note
 * @desc Code that's ran before an event is morphed.
 * This is local only for this template.
 * @default "// Declare Constants\nconst mapId = arguments[1];\nconst eventId = arguments[2];\nconst target = arguments[3];\nconst player = $gamePlayer;\n\n// Perform Actions\n"
 *
 * @param PostMorphJS:func
 * @text JS: Post-Morph
 * @parent JavaScript
 * @type note
 * @desc Code that's ran after an event is morphed.
 * This is local only for this template.
 * @default "// Declare Constants\nconst mapId = arguments[1];\nconst eventId = arguments[2];\nconst target = arguments[3];\nconst player = $gamePlayer;\n\n// Perform Actions\n"
 *
 * @param PreSpawnJS:func
 * @text JS: Pre-Spawn
 * @parent JavaScript
 * @type note
 * @desc Code that's ran before an event is spawned.
 * This is global and is ran for all spawned events.
 * @default "// Declare Constants\nconst mapId = arguments[1];\nconst eventId = arguments[2];\nconst target = arguments[3];\nconst player = $gamePlayer;\n\n// Perform Actions\n"
 *
 * @param PostSpawnJS:func
 * @text JS: Post-Spawn
 * @parent JavaScript
 * @type note
 * @desc Code that's ran after an event is spawned.
 * This is global and is ran for all spawned events.
 * @default "// Declare Constants\nconst mapId = arguments[1];\nconst eventId = arguments[2];\nconst target = arguments[3];\nconst player = $gamePlayer;\n\n// Perform Actions\n"
 *
 */
/* ----------------------------------------------------------------------------
 * Movement Settings
 * ----------------------------------------------------------------------------
 */
/*~struct~Movement:
 *
 * @param Dir8
 * @text 8 Directional Movement
 *
 * @param EnableDir8:eval
 * @text Enable
 * @parent Dir8
 * @type boolean
 * @on Enable
 * @off Disabled
 * @desc Allow 8-directional movement by default? Players can move diagonally.
 * @default true
 *
 * @param StrictCollision:eval
 * @text Strict Collision
 * @parent Dir8
 * @type boolean
 * @on Strict
 * @off Flexible
 * @desc Enforce strict collission rules where the player must be able to pass both cardinal directions?
 * @default true
 *
 * @param FavorHorz:eval
 * @text Favor Horizontal
 * @parent StrictCollision:eval
 * @type boolean
 * @on Horizontal
 * @off Vertical
 * @desc Favor horizontal if cannot pass diagonally but can pass both horizontally and vertically?
 * @default true
 *
 * @param SlowerSpeed:eval
 * @text Slower Diagonals?
 * @parent Dir8
 * @type boolean
 * @on Slower
 * @off Normal
 * @desc Enforce a slower movement speed when moving diagonally?
 * @default false
 *
 * @param DiagonalSpeedMultiplier:num
 * @text Speed Multiplier
 * @parent SlowerSpeed:eval
 * @desc What's the multiplier to adjust movement speed when moving diagonally?
 * @default 0.85
 *
 * @param AutoMove
 * @text Automatic Movement
 *
 * @param StopAutoMoveEvents:eval
 * @text Stop During Events
 * @parent AutoMove
 * @type boolean
 * @on Stop
 * @off Wander
 * @desc Stop automatic event movement while events are running.
 * @default true
 *
 * @param StopAutoMoveMessages:eval
 * @text Stop During Messages
 * @parent AutoMove
 * @type boolean
 * @on Stop
 * @off Wander
 * @desc Stop automatic event movement while a message is running.
 * @default true
 *
 * @param Bitmap
 *
 * @param BitmapSmoothing:eval
 * @text Smoothing
 * @parent Bitmap
 * @type boolean
 * @on Smooth
 * @off Pixelated
 * @desc Do you want to smooth or pixelate the map sprites?
 * Pixelating them is better for zooming and tilting.
 * @default false
 *
 * @param Dash
 * @text Dash
 *
 * @param DashModifier:num
 * @text Dash Modifier
 * @parent Dash
 * @desc Alters the dash speed modifier.
 * @default +1.0
 *
 * @param DashOnLadder:eval
 * @text Dash On Ladder?
 * @parent Dash
 * @type boolean
 * @on Allow
 * @off Disallow
 * @desc Allow dashing while on a ladder or rope?
 * @default false
 *
 * @param EnableDashTilt:eval
 * @text Enable Dash Tilt?
 * @parent Dash
 * @type boolean
 * @on Enable
 * @off Disabled
 * @desc Tilt any sprites that are currently dashing?
 * @default true
 *
 * @param TiltLeft:num
 * @text Tilt Left Amount
 * @parent EnableDashTilt:eval
 * @desc Amount in radians when moving left (upper left, left, lower left).
 * @default -0.15
 *
 * @param TiltRight:num
 * @text Tilt Right Amount
 * @parent EnableDashTilt:eval
 * @desc Amount in radians when moving right (upper right, right, lower right).
 * @default 0.15
 *
 * @param TiltVert:num
 * @text Tilt Vertical Amount
 * @parent EnableDashTilt:eval
 * @desc Amount in radians when moving vertical (up, down).
 * @default 0.05
 * 
 * @param EventMove
 * @text Event Movement
 *
 * @param RandomMoveWeight:num
 * @text Random Move Weight
 * @parent EventMove
 * @desc Use numbers between 0 and 1. Numbers closer to 1 stay
 * closer to their home position. 0 to disable it.
 * @default 0.10
 *
 * @param ShiftY:num
 * @text Shift Y
 * @parent EventMove
 * @desc How many pixels should non-tile characters be shifted by?
 * Negative: up. Positive: down.
 * @default -6
 *
 * @param PathFind
 * @text Path Finding
 *
 * @param PathfindMobileEnabled:eval
 * @text Mobile-Enabled?
 * @parent PathFind
 * @type boolean
 * @on Enabled
 * @off Disabled
 * @desc Enable diagonal pathfinding for mobile devices?
 * @default false
 *
 * @param Shadows
 *
 * @param ShowShadows:eval
 * @text Show
 * @parent Shadows
 * @type boolean
 * @on Show
 * @off Don't Show
 * @desc Show shadows on all events and player-related sprites.
 * @default true
 *
 * @param DefaultShadow:str
 * @text Default Filename
 * @parent Shadows
 * @type file
 * @dir img/system/
 * @desc Default filename used for shadows found in img/system/ folder.
 * @default Shadow1
 *
 * @param ShadowLayer:num
 * @text Shadow Z Layer
 * @parent Shadows
 * @desc What is the sprite Z layer used for the shadow sprites?
 * @default 0.5
 *
 * @param TurnInPlace
 * @text Turn in Place
 *
 * @param EnableTurnInPlace:eval
 * @text Enable
 * @parent TurnInPlace
 * @type boolean
 * @on Turn in Place
 * @off Skip
 * @desc When not dashing, player will turn in place before moving.
 * This only applies with keyboard inputs.
 * @default false
 *
 * @param TurnInPlaceDelay:num
 * @text Delay in Frames
 * @parent TurnInPlace
 * @type number
 * @min 0
 * @desc The number of frames to wait before moving.
 * @default 10
 *
 * @param Vehicle
 * @text Vehicle Speeds
 *
 * @param BoatSpeed:num
 * @text Boat Speed
 * @parent Vehicle
 * @desc Allows you to adjust the base speed of the boat vehicle.
 * @default 4.0
 *
 * @param ShipSpeed:num
 * @text Ship Speed
 * @parent Vehicle
 * @desc Allows you to adjust the base speed of the ship vehicle.
 * @default 5.0
 *
 * @param AirshipSpeed:num
 * @text Airship Speed
 * @parent Vehicle
 * @desc Allows you to adjust the base speed of the airship vehicle.
 * @default 6.0
 *
 */
/* ----------------------------------------------------------------------------
 * Region Rulings
 * ----------------------------------------------------------------------------
 */
/*~struct~Region:
 *
 * @param Allow
 * @text Allow Regions
 *
 * @param AllAllow:arraynum
 * @text All Allow
 * @parent Allow
 * @type number[]
 * @min 0
 * @max 255
 * @desc Insert Region ID's where the player can enter.
 * Region ID's range from 0 to 255.
 * @default []
 *
 * @param WalkAllow:arraynum
 * @text Walk Allow
 * @parent Allow
 * @type number[]
 * @min 0
 * @max 255
 * @desc Insert Region ID's where walking units can enter.
 * Region ID's range from 0 to 255.
 * @default []
 *
 * @param PlayerAllow:arraynum
 * @text Player Allow
 * @parent WalkAllow:arraynum
 * @type number[]
 * @min 0
 * @max 255
 * @desc Insert Region ID's where the player can enter.
 * Region ID's range from 0 to 255.
 * @default []
 *
 * @param EventAllow:arraynum
 * @text Event Allow
 * @parent WalkAllow:arraynum
 * @type number[]
 * @min 0
 * @max 255
 * @desc Insert Region ID's where events can enter.
 * Region ID's range from 0 to 255.
 * @default []
 *
 * @param VehicleAllow:arraynum
 * @text Vehicle Allow
 * @parent Allow
 * @type number[]
 * @min 0
 * @max 255
 * @desc Insert Region ID's where any vehicle can enter.
 * Region ID's range from 0 to 255.
 * @default []
 *
 * @param BoatAllow:arraynum
 * @text Boat Allow
 * @parent VehicleAllow:arraynum
 * @type number[]
 * @min 0
 * @max 255
 * @desc Insert Region ID's where boats can enter.
 * Region ID's range from 0 to 255.
 * @default []
 *
 * @param ShipAllow:arraynum
 * @text Ship Allow
 * @parent VehicleAllow:arraynum
 * @type number[]
 * @min 0
 * @max 255
 * @desc Insert Region ID's where ships can enter.
 * Region ID's range from 0 to 255.
 * @default []
 *
 * @param AirshipAllow:arraynum
 * @text Airship Allow
 * @parent VehicleAllow:arraynum
 * @type number[]
 * @min 0
 * @max 255
 * @desc Insert Region ID's where airships can enter.
 * Region ID's range from 0 to 255.
 * @default []
 *
 * @param Forbid
 * @text Forbid Regions
 *
 * @param AllForbid:arraynum
 * @text All Forbid
 * @parent Forbid
 * @type number[]
 * @min 0
 * @max 255
 * @desc Insert Region ID's where the player cannot enter.
 * Region ID's range from 0 to 255.
 * @default []
 *
 * @param WalkForbid:arraynum
 * @text Walk Forbid
 * @parent Forbid
 * @type number[]
 * @min 0
 * @max 255
 * @desc Insert Region ID's where walking units cannot enter.
 * Region ID's range from 0 to 255.
 * @default []
 *
 * @param PlayerForbid:arraynum
 * @text Player Forbid
 * @parent WalkForbid:arraynum
 * @type number[]
 * @min 0
 * @max 255
 * @desc Insert Region ID's where the player cannot enter.
 * Region ID's range from 0 to 255.
 * @default []
 *
 * @param EventForbid:arraynum
 * @text Event Forbid
 * @parent WalkForbid:arraynum
 * @type number[]
 * @min 0
 * @max 255
 * @desc Insert Region ID's where events cannot enter.
 * Region ID's range from 0 to 255.
 * @default []
 *
 * @param VehicleForbid:arraynum
 * @text Vehicle Forbid
 * @parent Forbid
 * @type number[]
 * @min 0
 * @max 255
 * @desc Insert Region ID's where vehicles cannot enter.
 * Region ID's range from 0 to 255.
 * @default []
 *
 * @param BoatForbid:arraynum
 * @text Boat Forbid
 * @parent VehicleForbid:arraynum
 * @type number[]
 * @min 0
 * @max 255
 * @desc Insert Region ID's where ships cannot enter.
 * Region ID's range from 0 to 255.
 * @default []
 *
 * @param ShipForbid:arraynum
 * @text Ship Forbid
 * @parent VehicleForbid:arraynum
 * @type number[]
 * @min 0
 * @max 255
 * @desc Insert Region ID's where ships cannot enter.
 * Region ID's range from 0 to 255.
 * @default []
 *
 * @param AirshipForbid:arraynum
 * @text Airship Forbid
 * @parent VehicleForbid:arraynum
 * @type number[]
 * @min 0
 * @max 255
 * @desc Insert Region ID's where airships cannot enter.
 * Region ID's range from 0 to 255.
 * @default []
 *
 * @param Dock
 * @text Dock Regions
 *
 * @param VehicleDock:arraynum
 * @text Vehicle Dock
 * @parent Dock
 * @type number[]
 * @min 0
 * @max 255
 * @desc Insert Region ID's where any vehicle can dock.
 * Region ID's range from 0 to 255.
 * @default []
 *
 * @param BoatDock:arraynum
 * @text Boat Dock
 * @parent Dock
 * @type number[]
 * @min 0
 * @max 255
 * @desc Insert Region ID's where boats can dock.
 * Region ID's range from 0 to 255.
 * @default []
 *
 * @param BoatDockRegionOnly:eval
 * @text Only Region Dockable
 * @parent BoatDock:arraynum
 * @type boolean
 * @on At Regions Only
 * @off Default
 * @desc Boats can only dock at designated regions.
 * @default false
 *
 * @param ShipDock:arraynum
 * @text Ship Dock
 * @parent Dock
 * @type number[]
 * @min 0
 * @max 255
 * @desc Insert Region ID's where ships can dock.
 * Region ID's range from 0 to 255.
 * @default []
 *
 * @param ShipDockRegionOnly:eval
 * @text Only Region Dockable
 * @parent ShipDock:arraynum
 * @type boolean
 * @on At Regions Only
 * @off Default
 * @desc Ships can only dock at designated regions.
 * @default false
 *
 * @param AirshipDock:arraynum
 * @text Airship Dock
 * @parent Dock
 * @type number[]
 * @min 0
 * @max 255
 * @desc Insert Region ID's where airships can dock.
 * Region ID's range from 0 to 255.
 * @default []
 *
 * @param AirshipDockRegionOnly:eval
 * @text Only Region Dockable
 * @parent AirshipDock:arraynum
 * @type boolean
 * @on At Regions Only
 * @off Default
 * @desc Airships can only dock at designated regions.
 * @default false
 *
 */
/* ----------------------------------------------------------------------------
 * Region Common Events
 * ----------------------------------------------------------------------------
 */
/*~struct~RegionCommonEvent:
 *
 * @param Region1:num
 * @text Region 1
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region2:num
 * @text Region 2
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region3:num
 * @text Region 3
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region4:num
 * @text Region 4
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region5:num
 * @text Region 5
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region6:num
 * @text Region 6
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region7:num
 * @text Region 7
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region8:num
 * @text Region 8
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region9:num
 * @text Region 9
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region10:num
 * @text Region 10
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region11:num
 * @text Region 11
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region12:num
 * @text Region 12
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region13:num
 * @text Region 13
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region14:num
 * @text Region 14
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region15:num
 * @text Region 15
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region16:num
 * @text Region 16
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region17:num
 * @text Region 17
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region18:num
 * @text Region 18
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region19:num
 * @text Region 19
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region20:num
 * @text Region 20
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region21:num
 * @text Region 21
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region22:num
 * @text Region 22
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region23:num
 * @text Region 23
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region24:num
 * @text Region 24
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region25:num
 * @text Region 25
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region26:num
 * @text Region 26
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region27:num
 * @text Region 27
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region28:num
 * @text Region 28
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region29:num
 * @text Region 29
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region30:num
 * @text Region 30
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region31:num
 * @text Region 31
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region32:num
 * @text Region 32
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region33:num
 * @text Region 33
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region34:num
 * @text Region 34
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region35:num
 * @text Region 35
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region36:num
 * @text Region 36
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region37:num
 * @text Region 37
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region38:num
 * @text Region 38
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region39:num
 * @text Region 39
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region40:num
 * @text Region 40
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region41:num
 * @text Region 41
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region42:num
 * @text Region 42
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region43:num
 * @text Region 43
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region44:num
 * @text Region 44
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region45:num
 * @text Region 45
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region46:num
 * @text Region 46
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region47:num
 * @text Region 47
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region48:num
 * @text Region 48
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region49:num
 * @text Region 49
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region50:num
 * @text Region 50
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region51:num
 * @text Region 51
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region52:num
 * @text Region 52
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region53:num
 * @text Region 53
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region54:num
 * @text Region 54
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region55:num
 * @text Region 55
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region56:num
 * @text Region 56
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region57:num
 * @text Region 57
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region58:num
 * @text Region 58
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region59:num
 * @text Region 59
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region60:num
 * @text Region 60
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region61:num
 * @text Region 61
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region62:num
 * @text Region 62
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region63:num
 * @text Region 63
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region64:num
 * @text Region 64
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region65:num
 * @text Region 65
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region66:num
 * @text Region 66
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region67:num
 * @text Region 67
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region68:num
 * @text Region 68
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region69:num
 * @text Region 69
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region70:num
 * @text Region 70
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region71:num
 * @text Region 71
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region72:num
 * @text Region 72
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region73:num
 * @text Region 73
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region74:num
 * @text Region 74
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region75:num
 * @text Region 75
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region76:num
 * @text Region 76
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region77:num
 * @text Region 77
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region78:num
 * @text Region 78
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region79:num
 * @text Region 79
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region80:num
 * @text Region 70
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region81:num
 * @text Region 71
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region82:num
 * @text Region 72
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region83:num
 * @text Region 73
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region84:num
 * @text Region 74
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region85:num
 * @text Region 75
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region86:num
 * @text Region 76
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region87:num
 * @text Region 77
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region88:num
 * @text Region 78
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region89:num
 * @text Region 79
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region80:num
 * @text Region 80
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region81:num
 * @text Region 81
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region82:num
 * @text Region 82
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region83:num
 * @text Region 83
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region84:num
 * @text Region 84
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region85:num
 * @text Region 85
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region86:num
 * @text Region 86
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region87:num
 * @text Region 87
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region88:num
 * @text Region 88
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region89:num
 * @text Region 89
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region90:num
 * @text Region 80
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region91:num
 * @text Region 81
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region92:num
 * @text Region 82
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region93:num
 * @text Region 83
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region94:num
 * @text Region 84
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region95:num
 * @text Region 85
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region96:num
 * @text Region 86
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region97:num
 * @text Region 87
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region98:num
 * @text Region 88
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region99:num
 * @text Region 89
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region90:num
 * @text Region 90
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region91:num
 * @text Region 91
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region92:num
 * @text Region 92
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region93:num
 * @text Region 93
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region94:num
 * @text Region 94
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region95:num
 * @text Region 95
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region96:num
 * @text Region 96
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region97:num
 * @text Region 97
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region98:num
 * @text Region 98
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region99:num
 * @text Region 99
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region100:num
 * @text Region 100
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region101:num
 * @text Region 101
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region102:num
 * @text Region 102
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region103:num
 * @text Region 103
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region104:num
 * @text Region 104
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region105:num
 * @text Region 105
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region106:num
 * @text Region 106
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region107:num
 * @text Region 107
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region108:num
 * @text Region 108
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region109:num
 * @text Region 109
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region110:num
 * @text Region 110
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region111:num
 * @text Region 111
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region112:num
 * @text Region 112
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region113:num
 * @text Region 113
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region114:num
 * @text Region 114
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region115:num
 * @text Region 115
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region116:num
 * @text Region 116
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region117:num
 * @text Region 117
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region118:num
 * @text Region 118
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region119:num
 * @text Region 119
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region120:num
 * @text Region 120
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region121:num
 * @text Region 121
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region122:num
 * @text Region 122
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region123:num
 * @text Region 123
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region124:num
 * @text Region 124
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region125:num
 * @text Region 125
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region126:num
 * @text Region 126
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region127:num
 * @text Region 127
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region128:num
 * @text Region 128
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region129:num
 * @text Region 129
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region130:num
 * @text Region 130
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region131:num
 * @text Region 131
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region132:num
 * @text Region 132
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region133:num
 * @text Region 133
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region134:num
 * @text Region 134
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region135:num
 * @text Region 135
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region136:num
 * @text Region 136
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region137:num
 * @text Region 137
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region138:num
 * @text Region 138
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region139:num
 * @text Region 139
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region140:num
 * @text Region 140
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region141:num
 * @text Region 141
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region142:num
 * @text Region 142
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region143:num
 * @text Region 143
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region144:num
 * @text Region 144
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region145:num
 * @text Region 145
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region146:num
 * @text Region 146
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region147:num
 * @text Region 147
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region148:num
 * @text Region 148
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region149:num
 * @text Region 149
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region150:num
 * @text Region 150
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region151:num
 * @text Region 151
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region152:num
 * @text Region 152
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region153:num
 * @text Region 153
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region154:num
 * @text Region 154
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region155:num
 * @text Region 155
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region156:num
 * @text Region 156
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region157:num
 * @text Region 157
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region158:num
 * @text Region 158
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region159:num
 * @text Region 159
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region160:num
 * @text Region 160
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region161:num
 * @text Region 161
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region162:num
 * @text Region 162
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region163:num
 * @text Region 163
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region164:num
 * @text Region 164
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region165:num
 * @text Region 165
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region166:num
 * @text Region 166
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region167:num
 * @text Region 167
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region168:num
 * @text Region 168
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region169:num
 * @text Region 169
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region170:num
 * @text Region 170
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region171:num
 * @text Region 171
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region172:num
 * @text Region 172
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region173:num
 * @text Region 173
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region174:num
 * @text Region 174
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region175:num
 * @text Region 175
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region176:num
 * @text Region 176
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region177:num
 * @text Region 177
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region178:num
 * @text Region 178
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region179:num
 * @text Region 179
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region180:num
 * @text Region 170
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region181:num
 * @text Region 171
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region182:num
 * @text Region 172
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region183:num
 * @text Region 173
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region184:num
 * @text Region 174
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region185:num
 * @text Region 175
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region186:num
 * @text Region 176
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region187:num
 * @text Region 177
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region188:num
 * @text Region 178
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region189:num
 * @text Region 179
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region180:num
 * @text Region 180
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region181:num
 * @text Region 181
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region182:num
 * @text Region 182
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region183:num
 * @text Region 183
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region184:num
 * @text Region 184
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region185:num
 * @text Region 185
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region186:num
 * @text Region 186
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region187:num
 * @text Region 187
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region188:num
 * @text Region 188
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region189:num
 * @text Region 189
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region190:num
 * @text Region 180
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region191:num
 * @text Region 181
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region192:num
 * @text Region 182
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region193:num
 * @text Region 183
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region194:num
 * @text Region 184
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region195:num
 * @text Region 185
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region196:num
 * @text Region 186
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region197:num
 * @text Region 187
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region198:num
 * @text Region 188
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region199:num
 * @text Region 189
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region190:num
 * @text Region 190
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region191:num
 * @text Region 191
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region192:num
 * @text Region 192
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region193:num
 * @text Region 193
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region194:num
 * @text Region 194
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region195:num
 * @text Region 195
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region196:num
 * @text Region 196
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region197:num
 * @text Region 197
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region198:num
 * @text Region 198
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region199:num
 * @text Region 199
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region200:num
 * @text Region 200
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region201:num
 * @text Region 201
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region202:num
 * @text Region 202
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region203:num
 * @text Region 203
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region204:num
 * @text Region 204
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region205:num
 * @text Region 205
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region206:num
 * @text Region 206
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region207:num
 * @text Region 207
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region208:num
 * @text Region 208
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region209:num
 * @text Region 209
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region210:num
 * @text Region 210
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region211:num
 * @text Region 211
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region212:num
 * @text Region 212
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region213:num
 * @text Region 213
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region214:num
 * @text Region 214
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region215:num
 * @text Region 215
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region216:num
 * @text Region 216
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region217:num
 * @text Region 217
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region218:num
 * @text Region 218
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region219:num
 * @text Region 219
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region220:num
 * @text Region 220
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region221:num
 * @text Region 221
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region222:num
 * @text Region 222
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region223:num
 * @text Region 223
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region224:num
 * @text Region 224
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region225:num
 * @text Region 225
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region226:num
 * @text Region 226
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region227:num
 * @text Region 227
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region228:num
 * @text Region 228
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region229:num
 * @text Region 229
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region230:num
 * @text Region 230
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region231:num
 * @text Region 231
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region232:num
 * @text Region 232
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region233:num
 * @text Region 233
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region234:num
 * @text Region 234
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region235:num
 * @text Region 235
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region236:num
 * @text Region 236
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region237:num
 * @text Region 237
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region238:num
 * @text Region 238
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region239:num
 * @text Region 239
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region240:num
 * @text Region 240
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region241:num
 * @text Region 241
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region242:num
 * @text Region 242
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region243:num
 * @text Region 243
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region244:num
 * @text Region 244
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region245:num
 * @text Region 245
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region246:num
 * @text Region 246
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region247:num
 * @text Region 247
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region248:num
 * @text Region 248
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region249:num
 * @text Region 249
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region250:num
 * @text Region 250
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region251:num
 * @text Region 251
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region252:num
 * @text Region 252
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region253:num
 * @text Region 253
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region254:num
 * @text Region 254
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 * @param Region255:num
 * @text Region 255
 * @type common_event
 * @desc Which Common Event does this region activate?
 * Use 0 to not activate any Common Events.
 * @default 0
 *
 */
/* ----------------------------------------------------------------------------
 * Terrain Tag Settings
 * ----------------------------------------------------------------------------
 */
/*~struct~TerrainTag:
 *
 * @param TerrainTag
 * @text Terrain Tag ID's
 *
 * @param Rope:num
 * @text Rope
 * @parent TerrainTag
 * @type number
 * @min 0
 * @max 7
 * @desc Which terrain tag number to use for ropes?
 * @default 1
 *
 */
/* ----------------------------------------------------------------------------
 * VisuStella 8-Dir Settings
 * ----------------------------------------------------------------------------
 */
/*~struct~VS8:
 *
 * @param Balloons
 * @text Balloon Icon Settings
 *
 * @param AutoBalloon:eval
 * @text Auto-Balloon Poses
 * @parent Balloons
 * @type boolean
 * @on Auto
 * @off Manual
 * @desc Automatically pose VS8 sprites when using balloon icons.
 * @default true
 *
 * @param BalloonOffsetX:num
 * @text Balloon Offset X
 * @parent Balloons
 * @desc Offset balloon icons on VS8 sprites by x pixels.
 * @default 0
 *
 * @param BalloonOffsetY:num
 * @text Balloon Offset Y
 * @parent Balloons
 * @desc Offset balloon icons on VS8 sprites by y pixels.
 * @default 10
 *
 * @param Icons
 * 
 * @param AutoBuffer:eval
 * @text Auto Buffer
 * @parent Icons
 * @type boolean
 * @on Auto
 * @off Manual
 * @desc Automatically buffer the X and Y coordinates of
 * VS8 sprites?
 * @default true
 * 
 * @param CarryPose:eval
 * @text Use Carry Pose
 * @parent Icons
 * @type boolean
 * @on Carry Pose
 * @off Normal
 * @desc Use the carry pose when moving with an icon overhead.
 * @default true
 *
 */
/* ----------------------------------------------------------------------------
 * Popup Extra Settings
 * ----------------------------------------------------------------------------
 */
/*~struct~PopupExtra:
 *
 * @param Fade
 * @text Fade Settings
 *
 * @param fadeInDuration:eval
 * @text Fade In Duration
 * @parent Fade
 * @desc How many frames does it take to fade in?
 * 60 frames = 1 second.
 * @default 8
 *
 * @param fadeOutDuration:eval
 * @text Fade Out Duration
 * @parent Fade
 * @desc How many frames does it take to fade out?
 * 60 frames = 1 second.
 * @default 8
 *
 * @param Offset
 * @text Offset Settings
 *
 * @param startOffsetX:eval
 * @text Starting Offset X
 * @parent Offset
 * @desc Offsets the starting x position. You may use code.
 * Negative: left. Positive: right.
 * @default +0
 *
 * @param startOffsetY:eval
 * @text Starting Offset Y
 * @parent Offset
 * @desc Offsets the starting y position. You may use code.
 * Negative: up. Positive: down.
 * @default -48
 *
 * @param endOffsetX:eval
 * @text Ending Offset X
 * @parent Offset
 * @desc Offsets the ending x position. You may use code.
 * Negative: left. Positive: right.
 * @default +0
 *
 * @param endOffsetY:eval
 * @text Ending Offset Y
 * @parent Offset
 * @desc Offsets the ending y position. You may use code.
 * Negative: up. Positive: down.
 * @default -96
 *
 * @param Scale
 * @text Scaling Settings
 *
 * @param startScaleX:eval
 * @text Starting Scale X
 * @parent Scale
 * @desc What is the starting scale x? You may use code.
 * 0.0 = 0%, 0.5 = 50%, 1.0 = 100%
 * @default 0.8
 *
 * @param startScaleY:eval
 * @text Starting Scale Y
 * @parent Scale
 * @desc What is the starting scale y? You may use code.
 * 0.0 = 0%, 0.5 = 50%, 1.0 = 100%
 * @default 0.8
 *
 * @param endScaleX:eval
 * @text Ending Scale X
 * @parent Scale
 * @desc What is the ending scale x? You may use code.
 * 0.0 = 0%, 0.5 = 50%, 1.0 = 100%
 * @default 0.8
 *
 * @param endScaleY:eval
 * @text Ending Scale Y
 * @parent Scale
 * @desc What is the ending scale y? You may use code.
 * 0.0 = 0%, 0.5 = 50%, 1.0 = 100%
 * @default 0.8
 *
 * @param Angle
 * @text Angle Settings
 *
 * @param startAngle:eval
 * @text Starting Offset Angle
 * @parent Angle
 * @desc What is the starting angle offset?
 * Use numbers between 0 and 360. You may use code.
 * @default +0
 *
 * @param endAngle:eval
 * @text Ending Offset Angle
 * @parent Angle
 * @desc What is the ending angle offset?
 * Use numbers between 0 and 360. You may use code.
 * @default +0
 * 
 * @param Misc
 * @text Misc Settings
 * 
 * @param Arc:eval
 * @text Arc Peak
 * @parent Misc
 * @desc This is the height of the popup's trajectory arc
 * in pixels. Positive: up. Negative: down. Code allowed.
 * @default +0
 *
 */
//=============================================================================

const _0x442592=_0x2440;(function(_0x41b27a,_0x4c2531){const _0x2abd50=_0x2440,_0x59bdab=_0x41b27a();while(!![]){try{const _0xe7f286=-parseInt(_0x2abd50(0x27d))/0x1+parseInt(_0x2abd50(0x385))/0x2*(-parseInt(_0x2abd50(0x2bc))/0x3)+-parseInt(_0x2abd50(0x130))/0x4*(parseInt(_0x2abd50(0x274))/0x5)+-parseInt(_0x2abd50(0x476))/0x6+-parseInt(_0x2abd50(0x537))/0x7+-parseInt(_0x2abd50(0x48c))/0x8+-parseInt(_0x2abd50(0x12e))/0x9*(-parseInt(_0x2abd50(0x238))/0xa);if(_0xe7f286===_0x4c2531)break;else _0x59bdab['push'](_0x59bdab['shift']());}catch(_0x3ed0c3){_0x59bdab['push'](_0x59bdab['shift']());}}}(_0x3f84,0x25f1e));var label=_0x442592(0x3dc),tier=tier||0x0,dependencies=[],pluginData=$plugins['filter'](function(_0x3aca8a){const _0x33b5ab=_0x442592;return _0x3aca8a[_0x33b5ab(0x1db)]&&_0x3aca8a['description']['includes']('['+label+']');})[0x0];VisuMZ[label][_0x442592(0x240)]=VisuMZ[label][_0x442592(0x240)]||{},VisuMZ[_0x442592(0x30c)]=function(_0x2e9d33,_0x508ce7){const _0x32895c=_0x442592;for(const _0x456375 in _0x508ce7){if(_0x456375['match'](/(.*):(.*)/i)){const _0x17834f=String(RegExp['$1']),_0x501683=String(RegExp['$2'])[_0x32895c(0x22e)]()[_0x32895c(0x36f)]();let _0x2ade37,_0x5a7ceb,_0xa5b6d8;switch(_0x501683){case'NUM':_0x2ade37=_0x508ce7[_0x456375]!==''?Number(_0x508ce7[_0x456375]):0x0;break;case'ARRAYNUM':_0x5a7ceb=_0x508ce7[_0x456375]!==''?JSON[_0x32895c(0x2ab)](_0x508ce7[_0x456375]):[],_0x2ade37=_0x5a7ceb[_0x32895c(0x510)](_0x216c09=>Number(_0x216c09));break;case'EVAL':_0x2ade37=_0x508ce7[_0x456375]!==''?eval(_0x508ce7[_0x456375]):null;break;case _0x32895c(0x284):_0x5a7ceb=_0x508ce7[_0x456375]!==''?JSON['parse'](_0x508ce7[_0x456375]):[],_0x2ade37=_0x5a7ceb[_0x32895c(0x510)](_0x864725=>eval(_0x864725));break;case _0x32895c(0x103):_0x2ade37=_0x508ce7[_0x456375]!==''?JSON[_0x32895c(0x2ab)](_0x508ce7[_0x456375]):'';break;case _0x32895c(0x1ae):_0x5a7ceb=_0x508ce7[_0x456375]!==''?JSON[_0x32895c(0x2ab)](_0x508ce7[_0x456375]):[],_0x2ade37=_0x5a7ceb[_0x32895c(0x510)](_0x3daa65=>JSON[_0x32895c(0x2ab)](_0x3daa65));break;case _0x32895c(0x498):_0x2ade37=_0x508ce7[_0x456375]!==''?new Function(JSON[_0x32895c(0x2ab)](_0x508ce7[_0x456375])):new Function(_0x32895c(0x39f));break;case _0x32895c(0x75):_0x5a7ceb=_0x508ce7[_0x456375]!==''?JSON['parse'](_0x508ce7[_0x456375]):[],_0x2ade37=_0x5a7ceb[_0x32895c(0x510)](_0x12ee31=>new Function(JSON[_0x32895c(0x2ab)](_0x12ee31)));break;case _0x32895c(0x22c):_0x2ade37=_0x508ce7[_0x456375]!==''?String(_0x508ce7[_0x456375]):'';break;case _0x32895c(0x527):_0x5a7ceb=_0x508ce7[_0x456375]!==''?JSON[_0x32895c(0x2ab)](_0x508ce7[_0x456375]):[],_0x2ade37=_0x5a7ceb[_0x32895c(0x510)](_0x346d69=>String(_0x346d69));break;case _0x32895c(0x3f0):_0xa5b6d8=_0x508ce7[_0x456375]!==''?JSON['parse'](_0x508ce7[_0x456375]):{},_0x2e9d33[_0x17834f]={},VisuMZ[_0x32895c(0x30c)](_0x2e9d33[_0x17834f],_0xa5b6d8);continue;case'ARRAYSTRUCT':_0x5a7ceb=_0x508ce7[_0x456375]!==''?JSON[_0x32895c(0x2ab)](_0x508ce7[_0x456375]):[],_0x2ade37=_0x5a7ceb[_0x32895c(0x510)](_0x4b7309=>VisuMZ[_0x32895c(0x30c)]({},JSON[_0x32895c(0x2ab)](_0x4b7309)));break;default:continue;}_0x2e9d33[_0x17834f]=_0x2ade37;}}return _0x2e9d33;},(_0x2444a3=>{const _0x1f0b13=_0x442592,_0x2ea005=_0x2444a3[_0x1f0b13(0x332)];for(const _0x1f03bd of dependencies){if(!Imported[_0x1f03bd]){alert(_0x1f0b13(0x2d2)[_0x1f0b13(0x2e6)](_0x2ea005,_0x1f03bd)),SceneManager[_0x1f0b13(0x2a1)]();break;}}const _0x2bdaa1=_0x2444a3[_0x1f0b13(0x3a7)];if(_0x2bdaa1['match'](/\[Version[ ](.*?)\]/i)){const _0x1de62d=Number(RegExp['$1']);_0x1de62d!==VisuMZ[label]['version']&&(alert(_0x1f0b13(0x1c2)[_0x1f0b13(0x2e6)](_0x2ea005,_0x1de62d)),SceneManager[_0x1f0b13(0x2a1)]());}if(_0x2bdaa1[_0x1f0b13(0x110)](/\[Tier[ ](\d+)\]/i)){const _0x34c667=Number(RegExp['$1']);_0x34c667<tier?(alert(_0x1f0b13(0x193)[_0x1f0b13(0x2e6)](_0x2ea005,_0x34c667,tier)),SceneManager[_0x1f0b13(0x2a1)]()):tier=Math[_0x1f0b13(0xcc)](_0x34c667,tier);}VisuMZ[_0x1f0b13(0x30c)](VisuMZ[label][_0x1f0b13(0x240)],_0x2444a3['parameters']);})(pluginData),VisuMZ[_0x442592(0x235)]=function(_0x53e507,_0x1c1639,_0x216c10){switch(_0x216c10){case'=':return _0x1c1639;break;case'+':return _0x53e507+_0x1c1639;break;case'-':return _0x53e507-_0x1c1639;break;case'*':return _0x53e507*_0x1c1639;break;case'/':return _0x53e507/_0x1c1639;break;case'%':return _0x53e507%_0x1c1639;break;}return _0x53e507;},PluginManager['registerCommand'](pluginData['name'],'AutoMoveEvents',_0x2ab2ba=>{const _0x316ea8=_0x442592;VisuMZ[_0x316ea8(0x30c)](_0x2ab2ba,_0x2ab2ba);switch(_0x2ab2ba[_0x316ea8(0x153)]){case'Allow':$gameSystem[_0x316ea8(0xbd)](!![]);break;case _0x316ea8(0x2c8):$gameSystem[_0x316ea8(0xbd)](![]);break;case _0x316ea8(0x3b8):$gameSystem[_0x316ea8(0xbd)](!$gameSystem['isAllowEventAutoMovement']());break;}}),PluginManager[_0x442592(0x487)](pluginData[_0x442592(0x332)],_0x442592(0x27c),_0x2b9239=>{const _0x3210f6=_0x442592;VisuMZ[_0x3210f6(0x30c)](_0x2b9239,_0x2b9239);const _0x2a96bc=$gameTemp['getLastPluginCommandInterpreter'](),_0x4ac3dc={'mapId':_0x2b9239['MapId'],'eventId':_0x2b9239['EventId']||_0x2a96bc[_0x3210f6(0xb6)](),'pageId':_0x2b9239['PageId']};if(_0x4ac3dc[_0x3210f6(0x321)]<=0x0)_0x4ac3dc[_0x3210f6(0x321)]=$gameMap?$gameMap[_0x3210f6(0x321)]():0x1;$gameTemp['getLastPluginCommandInterpreter']()[_0x3210f6(0x3a5)](_0x4ac3dc);}),PluginManager[_0x442592(0x487)](pluginData[_0x442592(0x332)],'DashEnableToggle',_0x420511=>{const _0x5a18d2=_0x442592;VisuMZ[_0x5a18d2(0x30c)](_0x420511,_0x420511);switch(_0x420511['Value']){case _0x5a18d2(0x2af):$gameSystem[_0x5a18d2(0x2d3)](!![]);break;case _0x5a18d2(0xec):$gameSystem[_0x5a18d2(0x2d3)](![]);break;case'Toggle':$gameSystem[_0x5a18d2(0x2d3)](!$gameSystem[_0x5a18d2(0x39c)]());break;}}),PluginManager['registerCommand'](pluginData[_0x442592(0x332)],'EventIconChange',_0x3e3137=>{const _0x59b331=_0x442592;VisuMZ[_0x59b331(0x30c)](_0x3e3137,_0x3e3137);const _0x23ad25=$gameTemp[_0x59b331(0x330)]();_0x3e3137[_0x59b331(0x3a0)]=_0x3e3137[_0x59b331(0x3a0)]||$gameMap[_0x59b331(0x321)](),$gameSystem[_0x59b331(0x303)](_0x3e3137[_0x59b331(0x3a0)],_0x3e3137[_0x59b331(0x2a7)]||_0x23ad25[_0x59b331(0xb6)](),_0x3e3137[_0x59b331(0x37f)],_0x3e3137[_0x59b331(0x489)],_0x3e3137[_0x59b331(0x1e1)],_0x3e3137['IconBlendMode'],![]);}),PluginManager[_0x442592(0x487)](pluginData[_0x442592(0x332)],_0x442592(0x23d),_0x184c53=>{const _0x484707=_0x442592;VisuMZ[_0x484707(0x30c)](_0x184c53,_0x184c53);const _0x5a4407=$gameTemp[_0x484707(0x330)]();_0x184c53[_0x484707(0x3a0)]=_0x184c53[_0x484707(0x3a0)]||$gameMap[_0x484707(0x321)](),$gameSystem[_0x484707(0x303)](_0x184c53[_0x484707(0x3a0)],_0x184c53['EventId']||_0x5a4407[_0x484707(0xb6)](),_0x184c53[_0x484707(0x37f)],_0x184c53[_0x484707(0x489)],_0x184c53[_0x484707(0x1e1)],_0x184c53[_0x484707(0x21a)],!![]);}),PluginManager[_0x442592(0x487)](pluginData[_0x442592(0x332)],_0x442592(0x409),_0x5ef0f3=>{const _0x57443a=_0x442592;VisuMZ[_0x57443a(0x30c)](_0x5ef0f3,_0x5ef0f3);const _0x56d858=$gameTemp['getLastPluginCommandInterpreter']();_0x5ef0f3[_0x57443a(0x3a0)]=_0x5ef0f3[_0x57443a(0x3a0)]||$gameMap['mapId'](),$gameSystem['deleteIconsOnEventsDataKey'](_0x5ef0f3[_0x57443a(0x3a0)],_0x5ef0f3[_0x57443a(0x2a7)]||_0x56d858['eventId']());}),PluginManager['registerCommand'](pluginData[_0x442592(0x332)],'EventIconRestore',_0x2b9b32=>{const _0x5b2c32=_0x442592;VisuMZ[_0x5b2c32(0x30c)](_0x2b9b32,_0x2b9b32);const _0x401dac=$gameTemp['getLastPluginCommandInterpreter']();_0x2b9b32[_0x5b2c32(0x3a0)]=_0x2b9b32[_0x5b2c32(0x3a0)]||$gameMap[_0x5b2c32(0x321)](),$gameSystem[_0x5b2c32(0x2ff)](_0x2b9b32['MapId'],_0x2b9b32['EventId']||_0x401dac[_0x5b2c32(0xb6)]());}),PluginManager[_0x442592(0x487)](pluginData[_0x442592(0x332)],_0x442592(0x370),_0x5bba49=>{const _0x3f281b=_0x442592;if($gameMap)for(const _0x50a1c3 of $gameMap[_0x3f281b(0x35b)]()){_0x50a1c3[_0x3f281b(0x29c)](),_0x50a1c3['updateEventLabelText']();}if(SceneManager[_0x3f281b(0x479)]()){const _0x18213b=SceneManager['_scene']['_spriteset'];if(_0x18213b)_0x18213b['refreshEventLabels']();}}),PluginManager['registerCommand'](pluginData[_0x442592(0x332)],_0x442592(0x1e7),_0x3bde1c=>{const _0x1831ab=_0x442592;VisuMZ[_0x1831ab(0x30c)](_0x3bde1c,_0x3bde1c);switch(_0x3bde1c['Visibility']){case _0x1831ab(0x3f6):$gameSystem[_0x1831ab(0x376)](!![]);break;case _0x1831ab(0x289):$gameSystem['setEventLabelsVisible'](![]);break;case _0x1831ab(0x3b8):$gameSystem['setEventLabelsVisible'](!$gameSystem[_0x1831ab(0x1ad)]());break;}}),PluginManager[_0x442592(0x487)](pluginData[_0x442592(0x332)],_0x442592(0x1b6),_0x29c14a=>{const _0x12e224=_0x442592;VisuMZ['ConvertParams'](_0x29c14a,_0x29c14a);const _0x5bc921=$gameTemp[_0x12e224(0x330)]();if(!$gameMap)return;const _0x26f2ce=$gameMap[_0x12e224(0x2f2)](_0x29c14a[_0x12e224(0x2a7)]||_0x5bc921['eventId']());if(_0x26f2ce)_0x26f2ce[_0x12e224(0x181)]();}),PluginManager[_0x442592(0x487)](pluginData[_0x442592(0x332)],_0x442592(0x38a),_0x2ba798=>{const _0x3b790e=_0x442592;VisuMZ[_0x3b790e(0x30c)](_0x2ba798,_0x2ba798);const _0x45a52d=$gameTemp[_0x3b790e(0x330)](),_0x401126=_0x2ba798['MapId']||$gameMap[_0x3b790e(0x321)](),_0x1105c6=_0x2ba798[_0x3b790e(0x2a7)]||_0x45a52d[_0x3b790e(0xb6)](),_0x5c9fbf=_0x2ba798[_0x3b790e(0x40c)]||0x0,_0x36c7cc=_0x2ba798[_0x3b790e(0x4ee)]||0x0,_0x28d584=_0x2ba798[_0x3b790e(0x416)]||0x2,_0x54f772=((_0x2ba798[_0x3b790e(0x290)]||0x1)-0x1)[_0x3b790e(0x551)](0x0,0x13),_0x3a457e=_0x2ba798[_0x3b790e(0x166)]||0x0;$gameSystem[_0x3b790e(0x4a0)](_0x401126,_0x1105c6,_0x5c9fbf,_0x36c7cc,_0x28d584,_0x54f772,_0x3a457e);}),PluginManager[_0x442592(0x487)](pluginData['name'],'EventLocationDelete',_0x41cc94=>{const _0x2e93cb=_0x442592;VisuMZ['ConvertParams'](_0x41cc94,_0x41cc94);const _0x503e7d=$gameTemp[_0x2e93cb(0x330)](),_0x6e722b=_0x41cc94[_0x2e93cb(0x3a0)]||$gameMap[_0x2e93cb(0x321)](),_0x105051=_0x41cc94[_0x2e93cb(0x2a7)]||_0x503e7d[_0x2e93cb(0xb6)]();$gameSystem[_0x2e93cb(0x226)](_0x6e722b,_0x105051);}),VisuMZ[_0x442592(0x3dc)][_0x442592(0x286)]=function(_0x1222da,_0x3a639e){const _0x39b832=_0x442592;_0x3a639e=_0x3a639e||{},_0x1222da[_0x39b832(0x4f9)]={'fadeIn':_0x3a639e[_0x39b832(0x21b)]||0x0,'fadeOut':_0x3a639e[_0x39b832(0x239)]||0x0},_0x1222da[_0x39b832(0x311)]={'x':_0x3a639e[_0x39b832(0xe2)]||0x0,'y':_0x3a639e['startOffsetY']||0x0},_0x1222da[_0x39b832(0x214)]={'x':_0x3a639e[_0x39b832(0x50f)]||0x0,'y':_0x3a639e[_0x39b832(0x35f)]||0x0},_0x1222da[_0x39b832(0x524)]={'x':_0x3a639e[_0x39b832(0x2dd)]||0x0,'y':_0x3a639e[_0x39b832(0x6f)]||0x0},_0x1222da[_0x39b832(0x3bb)]={'x':_0x3a639e[_0x39b832(0x347)]||0x0,'y':_0x3a639e[_0x39b832(0x4f4)]||0x0},_0x1222da[_0x39b832(0x52b)]={'start':_0x3a639e['startAngle']||0x0,'end':_0x3a639e[_0x39b832(0x72)]||0x0},_0x1222da[_0x39b832(0x1dc)]={'arc':_0x3a639e[_0x39b832(0x244)]||0x0};},PluginManager[_0x442592(0x487)](pluginData['name'],'MsgPopupPlayer',_0x5522c5=>{const _0x1373fd=_0x442592;if(!SceneManager[_0x1373fd(0x508)]())return;if(!Imported['VisuMZ_1_MessageCore']){$gameTemp[_0x1373fd(0x255)]()&&alert(_0x1373fd(0x4b5)+_0x1373fd(0x25a));return;}VisuMZ[_0x1373fd(0x30c)](_0x5522c5,_0x5522c5);const _0x579df6={'text':_0x5522c5['MessageText']||'','duration':Math[_0x1373fd(0xcc)](_0x5522c5[_0x1373fd(0x363)]||0x3c,0xc)},_0xe36a65=_0x5522c5[_0x1373fd(0x2f7)]||{};VisuMZ[_0x1373fd(0x3dc)][_0x1373fd(0x286)](_0x579df6,_0xe36a65);const _0x1ea5b4=SceneManager['_scene'][_0x1373fd(0x54c)];if(_0x1ea5b4){const _0x50332d=$gamePlayer;_0x1ea5b4[_0x1373fd(0x2f0)](_0x50332d,_0x579df6);}}),PluginManager[_0x442592(0x487)](pluginData[_0x442592(0x332)],_0x442592(0x4f5),_0x5e0fd4=>{const _0x427760=_0x442592;if(!SceneManager[_0x427760(0x508)]())return;if(!Imported['VisuMZ_1_MessageCore']){$gameTemp[_0x427760(0x255)]()&&alert(_0x427760(0x4b5)+'\x22Event\x20Popup:\x20Player\x22\x20plugin\x20command!');return;}VisuMZ['ConvertParams'](_0x5e0fd4,_0x5e0fd4);const _0x1e69bc=_0x5e0fd4[_0x427760(0x3c3)]||0x0,_0x19f524={'text':_0x5e0fd4[_0x427760(0x1ff)]||'','duration':Math['max'](_0x5e0fd4[_0x427760(0x363)]||0x3c,0xc)},_0x3423f3=_0x5e0fd4[_0x427760(0x2f7)]||{};VisuMZ['EventsMoveCore'][_0x427760(0x286)](_0x19f524,_0x3423f3);const _0x5f4d1e=SceneManager[_0x427760(0x29e)][_0x427760(0x54c)];if(_0x5f4d1e){const _0x12ab1e=$gamePlayer[_0x427760(0x14a)]()[_0x427760(0x177)](_0x1e69bc);_0x5f4d1e[_0x427760(0x2f0)](_0x12ab1e,_0x19f524);}}),PluginManager[_0x442592(0x487)](pluginData[_0x442592(0x332)],_0x442592(0x467),_0x249297=>{const _0x48daa1=_0x442592;if(!SceneManager[_0x48daa1(0x508)]())return;if(!Imported[_0x48daa1(0x201)]){$gameTemp[_0x48daa1(0x255)]()&&alert('VisuMZ_1_MessageCore\x20is\x20required\x20to\x20run\x20'+_0x48daa1(0x25a));return;}VisuMZ[_0x48daa1(0x30c)](_0x249297,_0x249297);const _0x4a4c06=$gameTemp['getLastPluginCommandInterpreter'](),_0x43eda8=_0x249297['EventId']||(_0x4a4c06?_0x4a4c06['eventId']():0x1),_0x198b56={'text':_0x249297['MessageText']||'','duration':Math[_0x48daa1(0xcc)](_0x249297[_0x48daa1(0x363)]||0x3c,0xc)},_0x5bce41=_0x249297[_0x48daa1(0x2f7)]||{};VisuMZ[_0x48daa1(0x3dc)][_0x48daa1(0x286)](_0x198b56,_0x5bce41);const _0x136baf=SceneManager[_0x48daa1(0x29e)]['_spriteset'];if(_0x136baf){const _0x3ff863=$gameMap[_0x48daa1(0x2f2)](_0x43eda8);_0x136baf[_0x48daa1(0x2f0)](_0x3ff863,_0x198b56);}}),PluginManager['registerCommand'](pluginData[_0x442592(0x332)],_0x442592(0x23e),_0x554b8b=>{const _0x1d7aa3=_0x442592;if(!SceneManager[_0x1d7aa3(0x508)]())return;if(!Imported[_0x1d7aa3(0x201)]){$gameTemp[_0x1d7aa3(0x255)]()&&alert(_0x1d7aa3(0x4b5)+'\x22Event\x20Popup:\x20Player\x22\x20plugin\x20command!');return;}VisuMZ[_0x1d7aa3(0x30c)](_0x554b8b,_0x554b8b);const _0x4b0adb={'text':_0x554b8b['MessageText']||'','duration':Math[_0x1d7aa3(0xcc)](_0x554b8b['MsgDuration']||0x3c,0xc),'tileCoordinates':{'x':Math[_0x1d7aa3(0x1e4)](_0x554b8b[_0x1d7aa3(0x3dd)]||0x0),'y':Math[_0x1d7aa3(0x1e4)](_0x554b8b[_0x1d7aa3(0x3b4)]||0x0)}},_0x61630d=_0x554b8b['PopupExtra']||{};VisuMZ['EventsMoveCore'][_0x1d7aa3(0x286)](_0x4b0adb,_0x61630d);const _0x256cb2=SceneManager[_0x1d7aa3(0x29e)]['_spriteset'];_0x256cb2&&_0x256cb2[_0x1d7aa3(0x14b)](_0x4b0adb);}),PluginManager['registerCommand'](pluginData[_0x442592(0x332)],_0x442592(0x45c),_0x42c9cc=>{const _0xef2282=_0x442592;VisuMZ['ConvertParams'](_0x42c9cc,_0x42c9cc);const _0x110de1=_0x42c9cc[_0xef2282(0x26e)];$gameTimer[_0xef2282(0x4c5)](_0x110de1);}),PluginManager[_0x442592(0x487)](pluginData[_0x442592(0x332)],_0x442592(0x1b7),_0x3bd672=>{const _0x51fb2=_0x442592;$gameTimer[_0x51fb2(0x4c5)](0x0);}),PluginManager[_0x442592(0x487)](pluginData[_0x442592(0x332)],_0x442592(0xe7),_0x40ff65=>{const _0x3dfa57=_0x442592;if(!$gameTimer[_0x3dfa57(0x50e)]())return;VisuMZ[_0x3dfa57(0x30c)](_0x40ff65,_0x40ff65);let _0x9dc077=0x0;_0x9dc077+=_0x40ff65[_0x3dfa57(0x2b4)],_0x9dc077+=_0x40ff65[_0x3dfa57(0x425)]*0x3c,_0x9dc077+=_0x40ff65['Minutes']*0x3c*0x3c,_0x9dc077+=_0x40ff65['Hours']*0x3c*0x3c*0x3c,$gameTimer[_0x3dfa57(0x1e5)](_0x9dc077);}),PluginManager[_0x442592(0x487)](pluginData[_0x442592(0x332)],_0x442592(0x459),_0x409fe2=>{const _0x2787dd=_0x442592;if(!$gameTimer[_0x2787dd(0x50e)]())return;VisuMZ[_0x2787dd(0x30c)](_0x409fe2,_0x409fe2);let _0x1b87af=0x0;_0x1b87af+=_0x409fe2[_0x2787dd(0x2b4)],_0x1b87af+=_0x409fe2['Seconds']*0x3c,_0x1b87af+=_0x409fe2['Minutes']*0x3c*0x3c,_0x1b87af+=_0x409fe2[_0x2787dd(0x3e7)]*0x3c*0x3c*0x3c,$gameTimer[_0x2787dd(0x48b)](_0x1b87af);}),PluginManager[_0x442592(0x487)](pluginData['name'],_0x442592(0x28d),_0x3055b2=>{const _0x58cf0d=_0x442592;if(!$gameTimer[_0x58cf0d(0x50e)]())return;$gameTimer[_0x58cf0d(0x31d)]();}),PluginManager[_0x442592(0x487)](pluginData['name'],_0x442592(0x37a),_0x13a390=>{if(!$gameTimer['isWorking']())return;$gameTimer['resume']();}),PluginManager[_0x442592(0x487)](pluginData[_0x442592(0x332)],'EventTimerSpeed',_0x303f1b=>{const _0x516567=_0x442592;VisuMZ['ConvertParams'](_0x303f1b,_0x303f1b);const _0x3a35d2=_0x303f1b['Speed']||0x0;$gameTimer[_0x516567(0x85)](_0x3a35d2);}),PluginManager['registerCommand'](pluginData['name'],_0x442592(0x46e),_0x381c3b=>{const _0x5bcf14=_0x442592;VisuMZ[_0x5bcf14(0x30c)](_0x381c3b,_0x381c3b);const _0xdcb19=!_0x381c3b[_0x5bcf14(0x1ac)];$gameSystem[_0x5bcf14(0x88)](_0xdcb19);}),PluginManager[_0x442592(0x487)](pluginData[_0x442592(0x332)],_0x442592(0x257),_0x14322=>{const _0x32e81b=_0x442592;VisuMZ[_0x32e81b(0x30c)](_0x14322,_0x14322);const _0x5017f0=(_0x14322[_0x32e81b(0x1df)]||0x0)-0x1,_0x5a0fbc=!_0x14322[_0x32e81b(0x1ac)],_0x472b6e=$gamePlayer['followers']()[_0x32e81b(0x177)](_0x5017f0);if(_0x472b6e)_0x472b6e[_0x32e81b(0xed)](_0x5a0fbc);}),PluginManager['registerCommand'](pluginData[_0x442592(0x332)],_0x442592(0x3d1),_0x4627e7=>{const _0x46df8c=_0x442592;VisuMZ[_0x46df8c(0x30c)](_0x4627e7,_0x4627e7);const _0x567158=_0x4627e7[_0x46df8c(0x1df)];$gameSystem['setControlledFollowerID'](_0x567158);}),PluginManager[_0x442592(0x487)](pluginData['name'],_0x442592(0x495),_0x144223=>{const _0x2f3469=_0x442592;VisuMZ['ConvertParams'](_0x144223,_0x144223),$gameSystem[_0x2f3469(0x4ac)](0x0),$gameSystem['setStopFollowerChasing'](![]);for(const _0x46f7ed of $gamePlayer[_0x2f3469(0x14a)]()[_0x2f3469(0x3ed)]){if(_0x46f7ed)_0x46f7ed[_0x2f3469(0xed)](![]);}}),PluginManager['registerCommand'](pluginData[_0x442592(0x332)],_0x442592(0x3d2),_0x587a5e=>{const _0x626042=_0x442592;VisuMZ[_0x626042(0x30c)](_0x587a5e,_0x587a5e);const _0x4113a5=$gameTemp[_0x626042(0x330)]();_0x587a5e['MapId']=_0x587a5e['MapId']||$gameMap[_0x626042(0x321)]();const _0x2feff0=[_0x587a5e[_0x626042(0x3a0)],_0x587a5e[_0x626042(0x2a7)]||_0x4113a5[_0x626042(0xb6)](),_0x587a5e[_0x626042(0x301)]],_0x6c4820=_0x587a5e[_0x626042(0x51e)],_0x399f81=$gameSelfSwitches[_0x626042(0x161)](_0x2feff0)||![];$gameSwitches[_0x626042(0x3c5)](_0x6c4820,_0x399f81);}),PluginManager['registerCommand'](pluginData[_0x442592(0x332)],_0x442592(0x2b6),_0x513f78=>{const _0x5ab50f=_0x442592;VisuMZ[_0x5ab50f(0x30c)](_0x513f78,_0x513f78);const _0x52ea10=$gameTemp[_0x5ab50f(0x330)]();_0x513f78[_0x5ab50f(0x3a0)]=_0x513f78[_0x5ab50f(0x3a0)]||$gameMap[_0x5ab50f(0x321)]();const _0x24066f=[_0x513f78[_0x5ab50f(0x3a0)],_0x513f78[_0x5ab50f(0x2a7)]||_0x52ea10[_0x5ab50f(0xb6)](),_0x5ab50f(0x4a4)[_0x5ab50f(0x2e6)](_0x513f78[_0x5ab50f(0x362)])],_0x4fddd1=_0x513f78[_0x5ab50f(0x51e)],_0x27c800=$gameSelfSwitches['value'](_0x24066f)||![];$gameSwitches[_0x5ab50f(0x3c5)](_0x4fddd1,_0x27c800);}),PluginManager['registerCommand'](pluginData[_0x442592(0x332)],_0x442592(0x167),_0x131c9d=>{const _0x2c3928=_0x442592;VisuMZ[_0x2c3928(0x30c)](_0x131c9d,_0x131c9d);const _0x23076d=$gameTemp['getLastPluginCommandInterpreter']();_0x131c9d[_0x2c3928(0x3a0)]=_0x131c9d[_0x2c3928(0x3a0)]||$gameMap[_0x2c3928(0x321)]();const _0x49545e=[_0x131c9d[_0x2c3928(0x3a0)],_0x131c9d['EventId']||_0x23076d[_0x2c3928(0xb6)](),_0x2c3928(0x39a)[_0x2c3928(0x2e6)](_0x131c9d['VariableId'])],_0x597ba4=_0x131c9d[_0x2c3928(0x26a)],_0x403d35=$gameSelfSwitches[_0x2c3928(0x161)](_0x49545e)||![];$gameVariables[_0x2c3928(0x3c5)](_0x597ba4,_0x403d35);}),PluginManager[_0x442592(0x487)](pluginData[_0x442592(0x332)],'MorphEventTo',_0x458ad7=>{const _0x19c2eb=_0x442592;VisuMZ[_0x19c2eb(0x30c)](_0x458ad7,_0x458ad7);if(!$gameMap)return;const _0x8e445=$gameTemp[_0x19c2eb(0x330)](),_0x5a0ae1=_0x458ad7[_0x19c2eb(0x1b8)];_0x458ad7[_0x19c2eb(0x159)]=_0x458ad7[_0x19c2eb(0x159)]||$gameMap[_0x19c2eb(0x321)](),_0x458ad7['Step2MapId']=_0x458ad7[_0x19c2eb(0x548)]||$gameMap[_0x19c2eb(0x321)](),_0x458ad7['TemplateName']=_0x458ad7[_0x19c2eb(0x4f1)][_0x19c2eb(0x22e)]()[_0x19c2eb(0x36f)]();if(!_0x5a0ae1&&_0x458ad7['Step1MapId']!==$gameMap[_0x19c2eb(0x321)]())return;if($gameMap[_0x19c2eb(0x321)]()===_0x458ad7[_0x19c2eb(0x159)]){const _0x2f6e89=$gameMap['event'](_0x458ad7[_0x19c2eb(0x2cd)]||_0x8e445[_0x19c2eb(0xb6)]());if(!_0x2f6e89)return;_0x458ad7[_0x19c2eb(0x4f1)]!==_0x19c2eb(0x2b8)?_0x2f6e89[_0x19c2eb(0x42c)](_0x458ad7[_0x19c2eb(0x4f1)]):_0x2f6e89[_0x19c2eb(0x35a)](_0x458ad7[_0x19c2eb(0x548)],_0x458ad7[_0x19c2eb(0x217)]||_0x8e445['eventId']());}_0x5a0ae1&&$gameSystem['savePreservedMorphEventDataKey'](_0x458ad7[_0x19c2eb(0x159)],_0x458ad7[_0x19c2eb(0x2cd)],_0x458ad7[_0x19c2eb(0x4f1)],_0x458ad7[_0x19c2eb(0x548)],_0x458ad7[_0x19c2eb(0x217)]);}),PluginManager['registerCommand'](pluginData[_0x442592(0x332)],_0x442592(0x1d7),_0x2e402f=>{const _0x272a11=_0x442592;VisuMZ[_0x272a11(0x30c)](_0x2e402f,_0x2e402f);if(!$gameMap)return;const _0x2a96ef=$gameTemp[_0x272a11(0x330)]();_0x2e402f[_0x272a11(0x3a0)]=_0x2e402f[_0x272a11(0x3a0)]||$gameMap[_0x272a11(0x321)]();if($gameMap[_0x272a11(0x321)]()===_0x2e402f['MapId']){const _0x56777c=$gameMap[_0x272a11(0x2f2)](_0x2e402f['EventId']||_0x2a96ef['eventId']());_0x56777c[_0x272a11(0x4d2)]();}_0x2e402f['RemovePreserve']&&$gameSystem[_0x272a11(0x43a)](_0x2e402f[_0x272a11(0x3a0)],_0x2e402f[_0x272a11(0x2a7)]||_0x2a96ef['eventId']());}),PluginManager['registerCommand'](pluginData[_0x442592(0x332)],'PlayerIconChange',_0xf2f14f=>{const _0xad02a5=_0x442592;VisuMZ[_0xad02a5(0x30c)](_0xf2f14f,_0xf2f14f),$gameSystem[_0xad02a5(0x288)]($gamePlayer,_0xf2f14f[_0xad02a5(0x37f)],_0xf2f14f['IconBufferX'],_0xf2f14f['IconBufferY'],_0xf2f14f[_0xad02a5(0x21a)]);}),PluginManager[_0x442592(0x487)](pluginData[_0x442592(0x332)],_0x442592(0x234),_0xd448cd=>{const _0x1df04d=_0x442592;VisuMZ['ConvertParams'](_0xd448cd,_0xd448cd),$gameSystem[_0x1df04d(0x203)]($gamePlayer);}),PluginManager[_0x442592(0x487)](pluginData['name'],_0x442592(0x2cf),_0x3f6cec=>{const _0x3f8b76=_0x442592;VisuMZ[_0x3f8b76(0x30c)](_0x3f6cec,_0x3f6cec),$gameSystem[_0x3f8b76(0x48d)](!_0x3f6cec[_0x3f8b76(0x2af)]);}),PluginManager[_0x442592(0x487)](pluginData[_0x442592(0x332)],_0x442592(0x2a2),_0x7deac5=>{const _0x34fe28=_0x442592;VisuMZ[_0x34fe28(0x30c)](_0x7deac5,_0x7deac5),$gameSystem['setPlayerDiagonalSetting'](_0x7deac5[_0x34fe28(0x346)]);}),PluginManager[_0x442592(0x487)](pluginData[_0x442592(0x332)],'SelfDataResetAll',_0x44a294=>{const _0x303ccc=_0x442592;VisuMZ['ConvertParams'](_0x44a294,_0x44a294);const _0x1d6bde=_0x44a294[_0x303ccc(0x3a0)]||$gameMap[_0x303ccc(0x321)]();$gameSelfSwitches[_0x303ccc(0x200)](_0x1d6bde);}),PluginManager['registerCommand'](pluginData[_0x442592(0x332)],_0x442592(0x1c0),_0x2ad769=>{const _0x4a30bf=_0x442592;VisuMZ[_0x4a30bf(0x30c)](_0x2ad769,_0x2ad769);const _0x48c10a=$gameTemp[_0x4a30bf(0x330)]();_0x2ad769['MapId']=_0x2ad769['MapId']||$gameMap['mapId']();const _0x2346c2=[_0x2ad769[_0x4a30bf(0x3a0)],_0x2ad769[_0x4a30bf(0x2a7)]||_0x48c10a[_0x4a30bf(0xb6)](),_0x2ad769[_0x4a30bf(0x301)]];switch(_0x2ad769[_0x4a30bf(0x153)]){case'ON':$gameSelfSwitches[_0x4a30bf(0x3c5)](_0x2346c2,!![]);break;case _0x4a30bf(0x1f3):$gameSelfSwitches['setValue'](_0x2346c2,![]);break;case _0x4a30bf(0x3b8):$gameSelfSwitches[_0x4a30bf(0x3c5)](_0x2346c2,!$gameSelfSwitches[_0x4a30bf(0x161)](_0x2346c2));break;}}),PluginManager[_0x442592(0x487)](pluginData[_0x442592(0x332)],_0x442592(0x420),_0x1b186c=>{const _0x387aad=_0x442592;VisuMZ['ConvertParams'](_0x1b186c,_0x1b186c);const _0x2f7555=$gameTemp[_0x387aad(0x330)]();_0x1b186c[_0x387aad(0x3a0)]=_0x1b186c['MapId']||$gameMap[_0x387aad(0x321)]();const _0x5d5dd8=[_0x1b186c[_0x387aad(0x3a0)],_0x1b186c[_0x387aad(0x2a7)]||_0x2f7555['eventId'](),_0x387aad(0x4a4)[_0x387aad(0x2e6)](_0x1b186c[_0x387aad(0x362)])];switch(_0x1b186c['Value']){case'ON':$gameSelfSwitches[_0x387aad(0x3c5)](_0x5d5dd8,!![]);break;case _0x387aad(0x1f3):$gameSelfSwitches[_0x387aad(0x3c5)](_0x5d5dd8,![]);break;case'Toggle':$gameSelfSwitches['setValue'](_0x5d5dd8,!$gameSelfSwitches[_0x387aad(0x161)](_0x5d5dd8));break;}}),PluginManager[_0x442592(0x487)](pluginData[_0x442592(0x332)],'SelfVariableID',_0x1ac0a8=>{const _0x30d9a4=_0x442592;VisuMZ[_0x30d9a4(0x30c)](_0x1ac0a8,_0x1ac0a8);const _0x270fc3=$gameTemp[_0x30d9a4(0x330)]();_0x1ac0a8['MapId']=_0x1ac0a8['MapId']||$gameMap[_0x30d9a4(0x321)]();const _0x34c434=[_0x1ac0a8[_0x30d9a4(0x3a0)],_0x1ac0a8[_0x30d9a4(0x2a7)]||_0x270fc3[_0x30d9a4(0xb6)](),_0x30d9a4(0x39a)[_0x30d9a4(0x2e6)](_0x1ac0a8[_0x30d9a4(0x44a)])],_0x456777=VisuMZ[_0x30d9a4(0x235)]($gameSelfSwitches[_0x30d9a4(0x161)](_0x34c434),_0x1ac0a8['Value'],_0x1ac0a8[_0x30d9a4(0x4c4)]);$gameSelfSwitches[_0x30d9a4(0x3c5)](_0x34c434,_0x456777);}),PluginManager['registerCommand'](pluginData['name'],_0x442592(0x79),_0x21b4ab=>{const _0x17d0ac=_0x442592;$gamePlayer[_0x17d0ac(0x517)]=!![];}),PluginManager[_0x442592(0x487)](pluginData[_0x442592(0x332)],'ShadowHideFollowers',_0x2ee3cc=>{const _0x5c59de=_0x442592;$gamePlayer[_0x5c59de(0x4d3)]=!![];}),PluginManager[_0x442592(0x487)](pluginData[_0x442592(0x332)],'ShadowHideAllEvents',_0x57e2fa=>{const _0x484cbb=_0x442592;$gamePlayer[_0x484cbb(0x2f9)]=!![];}),PluginManager[_0x442592(0x487)](pluginData['name'],_0x442592(0x47a),_0x3e973b=>{const _0x44999b=_0x442592;$gamePlayer[_0x44999b(0x517)]=![];}),PluginManager['registerCommand'](pluginData['name'],'ShadowShowFollowers',_0x519131=>{$gamePlayer['_noFollowerMovementShadow']=![];}),PluginManager[_0x442592(0x487)](pluginData[_0x442592(0x332)],_0x442592(0x1b1),_0x1be250=>{const _0x50efc8=_0x442592;$gamePlayer[_0x50efc8(0x2f9)]=!![];}),PluginManager[_0x442592(0x487)](pluginData[_0x442592(0x332)],_0x442592(0x119),_0x1535a6=>{const _0x3ff7e0=_0x442592;VisuMZ[_0x3ff7e0(0x30c)](_0x1535a6,_0x1535a6);const _0x2f3ad2=$gameTemp[_0x3ff7e0(0x330)](),_0x486a93={'template':_0x1535a6[_0x3ff7e0(0x4f1)],'mapId':_0x1535a6[_0x3ff7e0(0x3a0)]||$gameMap[_0x3ff7e0(0x321)](),'eventId':_0x1535a6[_0x3ff7e0(0x2a7)]||_0x2f3ad2['eventId'](),'x':_0x1535a6[_0x3ff7e0(0x40c)],'y':_0x1535a6[_0x3ff7e0(0x4ee)],'spawnPreserved':_0x1535a6[_0x3ff7e0(0x46b)],'spawnEventId':$gameMap[_0x3ff7e0(0x292)][_0x3ff7e0(0x2fe)]+0x3e8},_0xc42077=_0x1535a6['SuccessSwitchId']||0x0;if(!VisuMZ[_0x3ff7e0(0xb1)][_0x486a93['mapId']]&&_0x486a93[_0x3ff7e0(0x321)]!==$gameMap[_0x3ff7e0(0x321)]()){let _0x3f1992=_0x3ff7e0(0x36a)['format'](_0x486a93['mapId']);_0x3f1992+=_0x3ff7e0(0x480),_0x3f1992+=_0x3ff7e0(0x13b),_0x3f1992+=_0x3ff7e0(0x41d),_0x3f1992+=_0x3ff7e0(0xea)[_0x3ff7e0(0x2e6)](_0x486a93['mapId']),alert(_0x3f1992);return;}const _0x45f05e=$gameMap['prepareSpawnedEventAtXY'](_0x486a93,_0x1535a6[_0x3ff7e0(0x434)],_0x1535a6[_0x3ff7e0(0x53b)]);_0xc42077&&$gameSwitches['setValue'](_0xc42077,!!_0x45f05e);}),PluginManager[_0x442592(0x487)](pluginData[_0x442592(0x332)],'SpawnEventAtRegion',_0x3c05f2=>{const _0x4fa99e=_0x442592;VisuMZ['ConvertParams'](_0x3c05f2,_0x3c05f2);const _0x265675=$gameTemp[_0x4fa99e(0x330)](),_0x2f5aed={'template':_0x3c05f2['TemplateName'],'mapId':_0x3c05f2[_0x4fa99e(0x3a0)]||$gameMap['mapId'](),'eventId':_0x3c05f2[_0x4fa99e(0x2a7)]||_0x265675[_0x4fa99e(0xb6)](),'x':-0x1,'y':-0x1,'spawnPreserved':_0x3c05f2['Preserve'],'spawnEventId':$gameMap[_0x4fa99e(0x292)][_0x4fa99e(0x2fe)]+0x3e8},_0x25d30f=_0x3c05f2[_0x4fa99e(0x266)]||0x0;if(!VisuMZ[_0x4fa99e(0xb1)][_0x2f5aed[_0x4fa99e(0x321)]]&&_0x2f5aed[_0x4fa99e(0x321)]!==$gameMap['mapId']()){let _0x5d4137='You\x20do\x20not\x20have\x20Map\x20%1\x20added\x20to\x20the\x20list\x0a'[_0x4fa99e(0x2e6)](_0x2f5aed[_0x4fa99e(0x321)]);_0x5d4137+=_0x4fa99e(0x480),_0x5d4137+=_0x4fa99e(0x13b),_0x5d4137+=_0x4fa99e(0x41d),_0x5d4137+=_0x4fa99e(0xea)['format'](_0x2f5aed[_0x4fa99e(0x321)]),alert(_0x5d4137);return;}const _0x23c578=$gameMap[_0x4fa99e(0x3d4)](_0x2f5aed,_0x3c05f2[_0x4fa99e(0x4ba)],_0x3c05f2[_0x4fa99e(0x434)],_0x3c05f2[_0x4fa99e(0x53b)]);_0x25d30f&&$gameSwitches[_0x4fa99e(0x3c5)](_0x25d30f,!!_0x23c578);}),PluginManager[_0x442592(0x487)](pluginData[_0x442592(0x332)],'SpawnEventAtTerrainTag',_0x5f15f5=>{const _0x168c9e=_0x442592;VisuMZ['ConvertParams'](_0x5f15f5,_0x5f15f5);const _0x5e8cce=$gameTemp['getLastPluginCommandInterpreter'](),_0x1d18e4={'template':_0x5f15f5[_0x168c9e(0x4f1)],'mapId':_0x5f15f5[_0x168c9e(0x3a0)]||$gameMap[_0x168c9e(0x321)](),'eventId':_0x5f15f5['EventId']||_0x5e8cce[_0x168c9e(0xb6)](),'x':-0x1,'y':-0x1,'spawnPreserved':_0x5f15f5[_0x168c9e(0x46b)],'spawnEventId':$gameMap[_0x168c9e(0x292)][_0x168c9e(0x2fe)]+0x3e8},_0x14f114=_0x5f15f5[_0x168c9e(0x266)]||0x0;if(!VisuMZ[_0x168c9e(0xb1)][_0x1d18e4[_0x168c9e(0x321)]]&&_0x1d18e4[_0x168c9e(0x321)]!==$gameMap[_0x168c9e(0x321)]()){let _0x146b57=_0x168c9e(0x36a)[_0x168c9e(0x2e6)](_0x1d18e4[_0x168c9e(0x321)]);_0x146b57+=_0x168c9e(0x480),_0x146b57+=_0x168c9e(0x13b),_0x146b57+=_0x168c9e(0x41d),_0x146b57+=_0x168c9e(0xea)[_0x168c9e(0x2e6)](_0x1d18e4[_0x168c9e(0x321)]),alert(_0x146b57);return;}const _0x30b6b4=$gameMap['prepareSpawnedEventAtTerrainTag'](_0x1d18e4,_0x5f15f5[_0x168c9e(0x3a3)],_0x5f15f5[_0x168c9e(0x434)],_0x5f15f5[_0x168c9e(0x53b)]);_0x14f114&&$gameSwitches['setValue'](_0x14f114,!!_0x30b6b4);}),PluginManager['registerCommand'](pluginData[_0x442592(0x332)],_0x442592(0x2e4),_0x8e2258=>{const _0x484cfe=_0x442592;VisuMZ['ConvertParams'](_0x8e2258,_0x8e2258);const _0x2c1636=$gameTemp[_0x484cfe(0x330)]();$gameMap[_0x484cfe(0xa6)](_0x8e2258[_0x484cfe(0x28b)]||_0x2c1636[_0x484cfe(0xb6)]());}),PluginManager[_0x442592(0x487)](pluginData[_0x442592(0x332)],_0x442592(0x213),_0x105a7c=>{const _0x16a1af=_0x442592;VisuMZ['ConvertParams'](_0x105a7c,_0x105a7c);const _0x1563d3=_0x105a7c['PosX'],_0x509416=_0x105a7c[_0x16a1af(0x4ee)];$gameMap['despawnAtXY'](_0x1563d3,_0x509416);}),PluginManager['registerCommand'](pluginData[_0x442592(0x332)],_0x442592(0x1b0),_0x340291=>{const _0x175780=_0x442592;VisuMZ['ConvertParams'](_0x340291,_0x340291),$gameMap[_0x175780(0x148)](_0x340291[_0x175780(0x4ba)]);}),PluginManager[_0x442592(0x487)](pluginData[_0x442592(0x332)],_0x442592(0x248),_0xbb7fa1=>{const _0x1db644=_0x442592;VisuMZ['ConvertParams'](_0xbb7fa1,_0xbb7fa1),$gameMap['despawnTerrainTags'](_0xbb7fa1[_0x1db644(0x3a3)]);}),PluginManager[_0x442592(0x487)](pluginData['name'],_0x442592(0x36e),_0x388cf7=>{const _0x47c566=_0x442592;VisuMZ[_0x47c566(0x30c)](_0x388cf7,_0x388cf7),$gameMap['despawnEverything']();}),VisuMZ['EventsMoveCore'][_0x442592(0x160)]=Scene_Boot[_0x442592(0x34e)][_0x442592(0x17b)],Scene_Boot[_0x442592(0x34e)][_0x442592(0x17b)]=function(){const _0x555c79=_0x442592;VisuMZ[_0x555c79(0x3dc)][_0x555c79(0x160)][_0x555c79(0x98)](this),this['process_VisuMZ_EventsMoveCore_LoadTemplateMaps'](),this[_0x555c79(0x418)]();if(VisuMZ[_0x555c79(0x3dc)]['CustomPageConditions'])VisuMZ[_0x555c79(0x3dc)]['CustomPageConditions'][_0x555c79(0x327)]();},VisuMZ[_0x442592(0xb1)]=[],VisuMZ['EventTemplates']={},Scene_Boot[_0x442592(0x34e)][_0x442592(0x34c)]=function(){const _0x217b46=_0x442592;if(DataManager[_0x217b46(0xb4)]()||DataManager['isEventTest']())return;const _0xd3e5dc=VisuMZ[_0x217b46(0x3dc)][_0x217b46(0x240)][_0x217b46(0x2bf)],_0x326242=_0xd3e5dc[_0x217b46(0x26f)][_0x217b46(0x1f4)](0x0);for(const _0x450809 of _0xd3e5dc[_0x217b46(0x51a)]){_0x450809[_0x217b46(0x494)]=_0x450809[_0x217b46(0x494)][_0x217b46(0x22e)]()['trim'](),VisuMZ[_0x217b46(0x550)][_0x450809[_0x217b46(0x494)]]=_0x450809;if(!_0x326242[_0x217b46(0xb5)](_0x450809[_0x217b46(0x3cb)]))_0x326242[_0x217b46(0x314)](_0x450809[_0x217b46(0x3cb)]);}for(const _0x521e1d of _0x326242){if(VisuMZ[_0x217b46(0xb1)][_0x521e1d])continue;const _0x32f128='Map%1.json'[_0x217b46(0x2e6)](_0x521e1d[_0x217b46(0x4ff)](0x3)),_0x5710f3='$preloadedMap_%1'['format'](_0x521e1d);DataManager[_0x217b46(0x389)](_0x5710f3,_0x32f128),setTimeout(this[_0x217b46(0x1a0)][_0x217b46(0x97)](this,_0x521e1d,_0x5710f3),0x64);}},Scene_Boot[_0x442592(0x34e)][_0x442592(0x1a0)]=function(_0x340c9e,_0x26e8b3){const _0x651523=_0x442592;window[_0x26e8b3]?(VisuMZ[_0x651523(0xb1)][_0x340c9e]=window[_0x26e8b3],window[_0x26e8b3]=undefined):setTimeout(this[_0x651523(0x1a0)]['bind'](this,_0x340c9e,_0x26e8b3),0x64);},VisuMZ['AdvancedSwitches']=[],VisuMZ['SelfSwitches']=[],VisuMZ[_0x442592(0x369)]=[],VisuMZ[_0x442592(0x2db)]={},VisuMZ['AdvancedVariables']=[],VisuMZ[_0x442592(0x250)]=[],VisuMZ[_0x442592(0x1c6)]=[],VisuMZ['RefVariables']={},Scene_Boot[_0x442592(0x34e)]['process_VisuMZ_EventsMoveCore_Switches_Variables']=function(){const _0x2d386f=_0x442592;for(let _0x17e3ea=0x1;_0x17e3ea<$dataSystem['switches']['length'];_0x17e3ea++){if($dataSystem[_0x2d386f(0x481)][_0x17e3ea][_0x2d386f(0x110)](/<JS>\s*([\s\S]*)\s*<\/JS>/i))VisuMZ['AdvancedSwitches'][_0x2d386f(0x314)](_0x17e3ea);if($dataSystem[_0x2d386f(0x481)][_0x17e3ea][_0x2d386f(0x110)](/<SELF>/i))VisuMZ[_0x2d386f(0xbc)][_0x2d386f(0x314)](_0x17e3ea);if($dataSystem[_0x2d386f(0x481)][_0x17e3ea][_0x2d386f(0x110)](/<MAP>/i))VisuMZ[_0x2d386f(0x369)][_0x2d386f(0x314)](_0x17e3ea);if($dataSystem[_0x2d386f(0x481)][_0x17e3ea][_0x2d386f(0x110)](/\(\((.*)\)\)/i)){const _0x88c20=String(RegExp['$1'])[_0x2d386f(0x22e)]()[_0x2d386f(0x36f)]();VisuMZ[_0x2d386f(0x2db)][_0x88c20]=_0x17e3ea;}else{if($dataSystem[_0x2d386f(0x481)][_0x17e3ea][_0x2d386f(0x110)](/\（\（(.*)\）\）/i)){const _0x3d2374=String(RegExp['$1'])[_0x2d386f(0x22e)]()[_0x2d386f(0x36f)]();VisuMZ['RefSwitches'][_0x3d2374]=_0x17e3ea;}}}for(let _0x28a8fd=0x1;_0x28a8fd<$dataSystem[_0x2d386f(0x4a2)][_0x2d386f(0x2fe)];_0x28a8fd++){if($dataSystem['variables'][_0x28a8fd][_0x2d386f(0x110)](/<JS>\s*([\s\S]*)\s*<\/JS>/i))VisuMZ['AdvancedVariables'][_0x2d386f(0x314)](_0x28a8fd);if($dataSystem[_0x2d386f(0x4a2)][_0x28a8fd][_0x2d386f(0x110)](/<SELF>/i))VisuMZ['SelfVariables']['push'](_0x28a8fd);if($dataSystem[_0x2d386f(0x4a2)][_0x28a8fd][_0x2d386f(0x110)](/<MAP>/i))VisuMZ[_0x2d386f(0x1c6)][_0x2d386f(0x314)](_0x28a8fd);if($dataSystem['variables'][_0x28a8fd][_0x2d386f(0x110)](/\(\((.*)\)\)/i)){const _0x52427f=String(RegExp['$1'])[_0x2d386f(0x22e)]()[_0x2d386f(0x36f)]();VisuMZ[_0x2d386f(0x356)][_0x52427f]=_0x28a8fd;}else{if($dataSystem[_0x2d386f(0x4a2)][_0x28a8fd][_0x2d386f(0x110)](/\（\（(.*)\）\）/i)){const _0x460a52=String(RegExp['$1'])[_0x2d386f(0x22e)]()[_0x2d386f(0x36f)]();VisuMZ[_0x2d386f(0x356)][_0x460a52]=_0x28a8fd;}}}},VisuMZ[_0x442592(0x3dc)]['CustomPageConditions']={},VisuMZ[_0x442592(0x3dc)][_0x442592(0x1d4)][_0x442592(0x327)]=function(){const _0x29da00=_0x442592;this['_interpreter']=new Game_CPCInterpreter(),this[_0x29da00(0x33e)]();},VisuMZ[_0x442592(0x3dc)]['CustomPageConditions']['determineCommonEventsWithCPC']=function(){const _0x38da33=_0x442592;this[_0x38da33(0x383)]=[];for(const _0x3b8cee of $dataCommonEvents){if(!_0x3b8cee)continue;VisuMZ[_0x38da33(0x3dc)][_0x38da33(0x1d4)][_0x38da33(0x4b6)](_0x3b8cee);if(_0x3b8cee[_0x38da33(0x51b)]['length']>0x0)this[_0x38da33(0x383)]['push'](_0x3b8cee['id']);}},VisuMZ[_0x442592(0x3dc)][_0x442592(0x1d4)][_0x442592(0xc9)]=function(_0x1443c6,_0x3aee33,_0x3918e){const _0x189811=_0x442592;return this[_0x189811(0x465)][_0x189811(0x204)](_0x1443c6,_0x3aee33),_0x3918e?this[_0x189811(0x465)][_0x189811(0x2c0)](_0x3918e):this['_interpreter'][_0x189811(0x3e8)](),this[_0x189811(0x465)]['_cpc'];},VisuMZ['EventsMoveCore']['CustomPageConditions'][_0x442592(0x4b6)]=function(_0x138e48){const _0x291aee=_0x442592;let _0x2b649e=![];_0x138e48[_0x291aee(0x51b)]=[];for(const _0x565dd4 of _0x138e48['list']){if([0x6c,0x198]['includes'](_0x565dd4[_0x291aee(0x308)])){const _0x3416ae=_0x565dd4[_0x291aee(0xdc)][0x0];if(_0x3416ae[_0x291aee(0x110)](/<PAGE (?:CONDITION|CONDITIONS)>/i))_0x2b649e=!![];else _0x3416ae['match'](/<\/PAGE (?:CONDITION|CONDITIONS)>/i)&&(_0x2b649e=![]);}_0x2b649e&&_0x138e48['CPC'][_0x291aee(0x314)](_0x565dd4);}},getSelfSwitchValue=function(_0x1eb968,_0x31e5cf,_0x3da44c){const _0x35f926=_0x442592;let _0xde2652=[_0x1eb968,_0x31e5cf,_0x35f926(0x4a4)[_0x35f926(0x2e6)](_0x3da44c)];return typeof _0x3da44c===_0x35f926(0x1fb)&&(_0xde2652=[_0x1eb968,_0x31e5cf,_0x3da44c[_0x35f926(0x22e)]()[_0x35f926(0x36f)]()]),$gameSelfSwitches[_0x35f926(0x161)](_0xde2652);},getMapSwitchValue=function(_0x3b92d4,_0x4968a4){const _0x274194=_0x442592;let _0x495f78=[0x0,0x0,_0x274194(0x49c)[_0x274194(0x2e6)](_0x3b92d4,_0x4968a4)];return $gameSelfSwitches['value'](_0x495f78);},getMapVariableValue=function(_0x163e08,_0x53310f){const _0x50e6e9=_0x442592;let _0x23f386=[0x0,0x0,_0x50e6e9(0x13d)[_0x50e6e9(0x2e6)](_0x163e08,_0x53310f)];return $gameSelfSwitches[_0x50e6e9(0x161)](_0x23f386);},getSelfVariableValue=function(_0x233345,_0x4c837d,_0x50b07a){const _0x35a50b=_0x442592,_0x3c691b=[_0x233345,_0x4c837d,_0x35a50b(0x39a)['format'](_0x50b07a)];return $gameSelfSwitches[_0x35a50b(0x161)](_0x3c691b);},setSelfSwitchValue=function(_0xe23bfe,_0x12ea6f,_0x3991cc,_0x4fbdff){const _0x1b0826=_0x442592;let _0x207b87=[_0xe23bfe,_0x12ea6f,'Self\x20Switch\x20%1'[_0x1b0826(0x2e6)](_0x3991cc)];typeof _0x3991cc===_0x1b0826(0x1fb)&&(_0x207b87=[_0xe23bfe,_0x12ea6f,_0x3991cc[_0x1b0826(0x22e)]()['trim']()]),$gameSelfSwitches[_0x1b0826(0x3c5)](_0x207b87,_0x4fbdff);},setSelfVariableValue=function(_0x325380,_0x9b8d2c,_0x6f0da9,_0x3845e0){const _0x1604f9=_0x442592,_0x4f1c65=[_0x325380,_0x9b8d2c,_0x1604f9(0x39a)['format'](_0x6f0da9)];$gameSelfSwitches[_0x1604f9(0x3c5)](_0x4f1c65,_0x3845e0);},setMapSwitchValue=function(_0xcaddca,_0x3b9686,_0x3d4968){const _0x43d78f=_0x442592;let _0x43d379=[0x0,0x0,_0x43d78f(0x49c)['format'](_0xcaddca,_0x3b9686)];$gameSelfSwitches['setValue'](_0x43d379,_0x3d4968);},setMapVariableValue=function(_0x1b971f,_0xb9bae0,_0x18105f){const _0x378d57=_0x442592;let _0x54979b=[0x0,0x0,_0x378d57(0x13d)[_0x378d57(0x2e6)](_0x1b971f,_0xb9bae0)];$gameSelfSwitches[_0x378d57(0x3c5)](_0x54979b,_0x18105f);},DataManager[_0x442592(0x198)]=function(_0x112da7){const _0x575b0b=_0x442592;if(SceneManager['_scene']['constructor']===Scene_Debug)return![];return VisuMZ[_0x575b0b(0x111)][_0x575b0b(0xb5)](_0x112da7);},DataManager[_0x442592(0x4e0)]=function(_0x195178){const _0x1e4a00=_0x442592;if(SceneManager[_0x1e4a00(0x29e)][_0x1e4a00(0x2a9)]===Scene_Debug)return![];return VisuMZ[_0x1e4a00(0x331)][_0x1e4a00(0xb5)](_0x195178);},DataManager[_0x442592(0xf8)]=function(_0x2ada73){const _0x52d249=_0x442592;if(SceneManager[_0x52d249(0x29e)][_0x52d249(0x2a9)]===Scene_Debug)return![];return VisuMZ[_0x52d249(0xbc)][_0x52d249(0xb5)](_0x2ada73);},DataManager[_0x442592(0x3cd)]=function(_0xcb026c){const _0x5409c5=_0x442592;if(SceneManager[_0x5409c5(0x29e)][_0x5409c5(0x2a9)]===Scene_Debug)return![];return VisuMZ['SelfVariables'][_0x5409c5(0xb5)](_0xcb026c);},DataManager[_0x442592(0x164)]=function(_0x177774){const _0x1d97ae=_0x442592;if(BattleManager[_0x1d97ae(0xb4)]())return![];return VisuMZ[_0x1d97ae(0x369)][_0x1d97ae(0xb5)](_0x177774);},DataManager[_0x442592(0x20d)]=function(_0xf1f2a6){const _0x2e24a7=_0x442592;if(BattleManager[_0x2e24a7(0xb4)]())return![];return VisuMZ[_0x2e24a7(0x1c6)][_0x2e24a7(0xb5)](_0xf1f2a6);},ImageManager[_0x442592(0xde)]=function(_0xe26c8b){const _0x51ef90=_0x442592;return _0xe26c8b[_0x51ef90(0x110)](/\[INV(?:|ISIBLE)\]/i);},SceneManager[_0x442592(0x479)]=function(){const _0x2d26e8=_0x442592;return this[_0x2d26e8(0x29e)]&&this['_scene'][_0x2d26e8(0x2a9)]===Scene_Map;},SceneManager[_0x442592(0x508)]=function(){const _0x3d2bb4=_0x442592;return this[_0x3d2bb4(0x29e)]&&this[_0x3d2bb4(0x29e)]instanceof Scene_Map;},VisuMZ[_0x442592(0x3dc)][_0x442592(0x51f)]=Game_Temp['prototype'][_0x442592(0x99)],Game_Temp[_0x442592(0x34e)][_0x442592(0x99)]=function(_0x47c399,_0x41a0e9){const _0x51c5dd=_0x442592;if(this[_0x51c5dd(0x2b1)](_0x47c399,_0x41a0e9))return;VisuMZ[_0x51c5dd(0x3dc)][_0x51c5dd(0x51f)][_0x51c5dd(0x98)](this,_0x47c399,_0x41a0e9);},Game_Temp['prototype'][_0x442592(0x2b1)]=function(_0x3b5f41,_0x2cd891){const _0x4434db=_0x442592,_0x29fcd2=$gameMap['eventsXy'](_0x3b5f41,_0x2cd891);for(const _0x841cb4 of _0x29fcd2){if(_0x841cb4&&_0x841cb4['hasClickTrigger']())return _0x841cb4[_0x4434db(0x275)](),!![];}return TouchInput[_0x4434db(0x2ac)]()&&_0x29fcd2[_0x4434db(0x2fe)]>0x0&&_0x29fcd2[_0x4434db(0x474)](_0x22c0f6=>_0x22c0f6&&_0x22c0f6[_0x4434db(0x3d6)]())&&TouchInput[_0x4434db(0x530)](),![];},Game_Event['prototype']['isEventClickStopValid']=function(){const _0x4d3a45=_0x442592;if(this[_0x4d3a45(0x252)])return![];if(!this[_0x4d3a45(0x335)]())return![];if(!this[_0x4d3a45(0x249)]())return![];let _0x21a439=this['list']()[_0x4d3a45(0x35d)]();return _0x21a439=_0x21a439[_0x4d3a45(0xda)](_0x3f72ac=>![0x0,0x6c,0x198]['includes'](_0x3f72ac[_0x4d3a45(0x308)])),_0x21a439['length']>0x0;},Game_Temp[_0x442592(0x34e)][_0x442592(0x493)]=function(_0x492fd3){const _0x14c365=_0x442592;this[_0x14c365(0x426)]=_0x492fd3;},Game_Temp[_0x442592(0x34e)][_0x442592(0x330)]=function(){const _0x48e307=_0x442592;return this[_0x48e307(0x426)];},Game_Temp['prototype'][_0x442592(0x2e9)]=function(_0x57146f){const _0x49aaaa=_0x442592;this[_0x49aaaa(0x3bc)]=_0x57146f;},Game_Temp[_0x442592(0x34e)]['clearSelfTarget']=function(){const _0xfcd22f=_0x442592;this[_0xfcd22f(0x3bc)]=undefined;},Game_Temp[_0x442592(0x34e)][_0x442592(0x4b4)]=function(){const _0x2c79fa=_0x442592;return this[_0x2c79fa(0x3bc)];},VisuMZ[_0x442592(0x3dc)][_0x442592(0x538)]=Game_System[_0x442592(0x34e)]['initialize'],Game_System[_0x442592(0x34e)][_0x442592(0x327)]=function(){const _0xb62aa2=_0x442592;VisuMZ['EventsMoveCore'][_0xb62aa2(0x538)]['call'](this),this[_0xb62aa2(0x47d)](),this[_0xb62aa2(0x393)]();},Game_System[_0x442592(0x34e)]['initEventsMoveCore']=function(){const _0x248927=_0x442592;this[_0x248927(0x41a)]={'DashingEnable':!![],'EventAutoMovement':!![],'VisibleEventLabels':!![]},this[_0x248927(0x35c)]={},this[_0x248927(0x2eb)]=[],this[_0x248927(0x405)]={},this[_0x248927(0x322)]={},this[_0x248927(0x4eb)]=![],this[_0x248927(0x544)]=_0x248927(0x8e);},Game_System[_0x442592(0x34e)][_0x442592(0x39c)]=function(){const _0x4a7a3c=_0x442592;if(this['_EventsMoveCoreSettings']===undefined)this[_0x4a7a3c(0x47d)]();if(this['_EventsMoveCoreSettings'][_0x4a7a3c(0x3eb)]===undefined)this['initEventsMoveCore']();return this[_0x4a7a3c(0x41a)]['DashingEnable'];},Game_System[_0x442592(0x34e)][_0x442592(0x2d3)]=function(_0x5eb5ea){const _0x19b8a0=_0x442592;if(this['_EventsMoveCoreSettings']===undefined)this[_0x19b8a0(0x47d)]();if(this[_0x19b8a0(0x41a)][_0x19b8a0(0x3eb)]===undefined)this['initEventsMoveCore']();this['_EventsMoveCoreSettings']['DashingEnable']=_0x5eb5ea;},Game_System[_0x442592(0x34e)][_0x442592(0x7f)]=function(){const _0x1a5e8a=_0x442592;if(this['_EventsMoveCoreSettings']===undefined)this['initEventsMoveCore']();if(this[_0x1a5e8a(0x41a)][_0x1a5e8a(0x431)]===undefined)this[_0x1a5e8a(0x47d)]();return this['_EventsMoveCoreSettings'][_0x1a5e8a(0x431)];},Game_System[_0x442592(0x34e)]['setAllowEventAutoMovement']=function(_0x3d0f4d){const _0x25435a=_0x442592;if(this['_EventsMoveCoreSettings']===undefined)this['initEventsMoveCore']();if(this[_0x25435a(0x41a)][_0x25435a(0x431)]===undefined)this[_0x25435a(0x47d)]();this['_EventsMoveCoreSettings'][_0x25435a(0x431)]=_0x3d0f4d;},Game_System['prototype'][_0x442592(0x1ad)]=function(){const _0x4fc9fc=_0x442592;if(this['_EventsMoveCoreSettings']===undefined)this[_0x4fc9fc(0x47d)]();if(this[_0x4fc9fc(0x41a)][_0x4fc9fc(0x3e0)]===undefined)this[_0x4fc9fc(0x47d)]();return this[_0x4fc9fc(0x41a)][_0x4fc9fc(0x3e0)];},Game_System[_0x442592(0x34e)][_0x442592(0x376)]=function(_0x571b68){const _0x403790=_0x442592;if(this[_0x403790(0x41a)]===undefined)this[_0x403790(0x47d)]();if(this[_0x403790(0x41a)]['VisibleEventLabels']===undefined)this[_0x403790(0x47d)]();this[_0x403790(0x41a)][_0x403790(0x3e0)]=_0x571b68;},Game_System[_0x442592(0x34e)][_0x442592(0x1cd)]=function(){const _0x4b09d6=_0x442592;return this[_0x4b09d6(0x4eb)]===undefined&&(this['_DisablePlayerControl']=![]),this[_0x4b09d6(0x4eb)];},Game_System[_0x442592(0x34e)][_0x442592(0x48d)]=function(_0x6bda4b){const _0x9cefff=_0x442592;this[_0x9cefff(0x4eb)]=_0x6bda4b;},Game_System[_0x442592(0x34e)]['getPlayerDiagonalSetting']=function(){const _0x75a57e=_0x442592;return this[_0x75a57e(0x544)];},Game_System[_0x442592(0x34e)][_0x442592(0x145)]=function(_0x5138c4){const _0x2fb191=_0x442592;this['_PlayerDiagonalSetting']=String(_0x5138c4)[_0x2fb191(0xdd)]()[_0x2fb191(0x36f)]();},Game_System[_0x442592(0x34e)][_0x442592(0x45b)]=function(_0x1d9d6e){const _0x11dc96=_0x442592;if(this[_0x11dc96(0x35c)]===undefined)this[_0x11dc96(0x47d)]();if(!_0x1d9d6e)return null;if(_0x1d9d6e===$gamePlayer)return this[_0x11dc96(0x35c)][_0x11dc96(0x519)];else{const _0x18d88f=VisuMZ[_0x11dc96(0x3dc)][_0x11dc96(0x240)],_0x32d401='Map%1-Event%2'[_0x11dc96(0x2e6)](_0x1d9d6e[_0x11dc96(0x118)],_0x1d9d6e['_eventId']);return this[_0x11dc96(0x35c)][_0x32d401]=this[_0x11dc96(0x35c)][_0x32d401]||{'iconIndex':0x0,'bufferX':_0x18d88f[_0x11dc96(0x186)][_0x11dc96(0xef)],'bufferY':_0x18d88f['Icon'][_0x11dc96(0x461)],'blendMode':_0x18d88f['Icon'][_0x11dc96(0x2b9)]},this['_EventIcons'][_0x32d401];}},Game_System[_0x442592(0x34e)][_0x442592(0x288)]=function(_0x4ed18e,_0x200568,_0xaa83bf,_0x36a8ae,_0x394a56){const _0x43443e=_0x442592;if(this[_0x43443e(0x35c)]===undefined)this[_0x43443e(0x47d)]();const _0x2a68a5=_0x4ed18e===$gamePlayer?'Player':'Map%1-Event%2'[_0x43443e(0x2e6)](_0x4ed18e['_mapId'],_0x4ed18e[_0x43443e(0xa3)]);this[_0x43443e(0x35c)][_0x2a68a5]={'iconIndex':_0x200568,'bufferX':_0xaa83bf,'bufferY':_0x36a8ae,'blendMode':_0x394a56};},Game_System[_0x442592(0x34e)][_0x442592(0x303)]=function(_0xa27fa3,_0x2954f9,_0x524dca,_0x37a892,_0x58ca22,_0x5a1240,_0x258602){const _0x471d45=_0x442592;if(this[_0x471d45(0x35c)]===undefined)this[_0x471d45(0x47d)]();const _0x5aac74='Map%1-Event%2'[_0x471d45(0x2e6)](_0xa27fa3,_0x2954f9);this[_0x471d45(0x35c)][_0x5aac74]={'iconIndex':_0x524dca,'forced':_0x258602,'bufferX':_0x37a892,'bufferY':_0x58ca22,'blendMode':_0x5a1240};},Game_System[_0x442592(0x34e)][_0x442592(0x203)]=function(_0x21cd3f){const _0x58138a=_0x442592;if(this['_EventIcons']===undefined)this[_0x58138a(0x47d)]();if(!_0x21cd3f)return null;_0x21cd3f===$gamePlayer?delete this[_0x58138a(0x35c)][_0x58138a(0x519)]:this[_0x58138a(0x11d)](_0x21cd3f[_0x58138a(0x118)],_0x21cd3f[_0x58138a(0xa3)]);},Game_System[_0x442592(0x34e)][_0x442592(0x11d)]=function(_0x50ffdd,_0x4a42d1){const _0xc7abff=_0x442592;if(this[_0xc7abff(0x35c)]===undefined)this['initEventsMoveCore']();this[_0xc7abff(0x303)](_0x50ffdd,_0x4a42d1,-0x1,0x0,0x0,0x0,![]);},Game_System['prototype'][_0x442592(0x1a6)]=function(_0x4a9844){const _0x173ccf=_0x442592;if(this[_0x173ccf(0x35c)]===undefined)this['initEventsMoveCore']();if(!_0x4a9844)return null;_0x4a9844===$gamePlayer?delete this['_EventIcons'][_0x173ccf(0x519)]:this['resetIconsOnEventsDataKey'](_0x4a9844[_0x173ccf(0x118)],_0x4a9844[_0x173ccf(0xa3)]);},Game_System['prototype']['resetIconsOnEventsDataKey']=function(_0x1da3c9,_0x44f180){const _0x45fd44=_0x442592;if(this[_0x45fd44(0x35c)]===undefined)this[_0x45fd44(0x47d)]();const _0x123258=_0x45fd44(0xf9)['format'](_0x1da3c9,_0x44f180);if(this[_0x45fd44(0x35c)][_0x123258]){if(this['_EventIcons'][_0x123258][_0x45fd44(0xba)]<0x0)return;if(this[_0x45fd44(0x35c)][_0x123258][_0x45fd44(0x352)])return;}delete this['_EventIcons'][_0x123258];},Game_System[_0x442592(0x34e)]['restoreIconsOnEventsDataKey']=function(_0xc3cd66,_0x156695){const _0x59ace1=_0x442592;if(this[_0x59ace1(0x35c)]===undefined)this[_0x59ace1(0x47d)]();const _0x565e1f=_0x59ace1(0xf9)[_0x59ace1(0x2e6)](_0xc3cd66,_0x156695);delete this[_0x59ace1(0x35c)][_0x565e1f];if(_0xc3cd66!==$gameMap[_0x59ace1(0x321)]())return;const _0x1443c3=$gameMap[_0x59ace1(0x2f2)](_0x156695);if(!_0x1443c3)return;_0x1443c3[_0x59ace1(0x2b5)]();},Game_System[_0x442592(0x34e)][_0x442592(0x3ec)]=function(_0x2db2b4){const _0x1c5965=_0x442592;if(this[_0x1c5965(0x322)]===undefined)this[_0x1c5965(0x47d)]();if(!_0x2db2b4)return null;const _0x51f850=_0x1c5965(0xf9)[_0x1c5965(0x2e6)](_0x2db2b4[_0x1c5965(0x118)],_0x2db2b4[_0x1c5965(0xa3)]);return this[_0x1c5965(0x322)][_0x51f850];},Game_System[_0x442592(0x34e)][_0x442592(0x181)]=function(_0x1a8119){const _0x409e8f=_0x442592;if(this[_0x409e8f(0x322)]===undefined)this['initEventsMoveCore']();if(!_0x1a8119)return;const _0x555c94=_0x409e8f(0xf9)[_0x409e8f(0x2e6)](_0x1a8119[_0x409e8f(0x118)],_0x1a8119[_0x409e8f(0xa3)]);this['_SavedEventLocations'][_0x555c94]={'direction':_0x1a8119[_0x409e8f(0x496)](),'x':Math[_0x409e8f(0x1e4)](_0x1a8119['x']),'y':Math[_0x409e8f(0x1e4)](_0x1a8119['y']),'pageIndex':_0x1a8119['_pageIndex'],'moveRouteIndex':_0x1a8119[_0x409e8f(0x296)]};},Game_System[_0x442592(0x34e)][_0x442592(0x4c1)]=function(_0x2f76a6){const _0xea78c8=_0x442592;if(this[_0xea78c8(0x322)]===undefined)this[_0xea78c8(0x47d)]();if(!_0x2f76a6)return;this['deleteSavedEventLocationKey'](_0x2f76a6[_0xea78c8(0x118)],_0x2f76a6[_0xea78c8(0xa3)]);},Game_System['prototype'][_0x442592(0x226)]=function(_0x5abb79,_0x1302f0){const _0x52c829=_0x442592;if(this['_SavedEventLocations']===undefined)this[_0x52c829(0x47d)]();const _0x18a7e3='Map%1-Event%2'[_0x52c829(0x2e6)](_0x5abb79,_0x1302f0);delete this['_SavedEventLocations'][_0x18a7e3];},Game_System[_0x442592(0x34e)][_0x442592(0x4a0)]=function(_0xbc4771,_0x49fa75,_0x2a99db,_0x59b731,_0x5b11f4,_0x13a675,_0x27bd13){const _0x4ff54c=_0x442592;if(this[_0x4ff54c(0x322)]===undefined)this[_0x4ff54c(0x47d)]();const _0x30f558='Map%1-Event%2'[_0x4ff54c(0x2e6)](_0xbc4771,_0x49fa75);this['_SavedEventLocations'][_0x30f558]={'direction':_0x5b11f4,'x':Math[_0x4ff54c(0x1e4)](_0x2a99db),'y':Math[_0x4ff54c(0x1e4)](_0x59b731),'pageIndex':_0x13a675,'moveRouteIndex':_0x27bd13};},Game_System['prototype'][_0x442592(0x4e5)]=function(_0x2d8f23){const _0x2b41d2=_0x442592;if(this[_0x2b41d2(0x405)]===undefined)this['initEventsMoveCore']();if(!_0x2d8f23)return;const _0x5d33b2=_0x2b41d2(0xf9)[_0x2b41d2(0x2e6)](_0x2d8f23[_0x2b41d2(0x118)],_0x2d8f23['_eventId']);return this[_0x2b41d2(0x405)][_0x5d33b2];},Game_System[_0x442592(0x34e)][_0x442592(0x2cb)]=function(_0x30f9cd,_0x2f7359,_0x398ad2,_0x181a32,_0x39fa44){const _0x20dc1f=_0x442592;if(this[_0x20dc1f(0x405)]===undefined)this['initEventsMoveCore']();const _0x2cebd1=_0x20dc1f(0xf9)[_0x20dc1f(0x2e6)](_0x30f9cd,_0x2f7359);this[_0x20dc1f(0x405)][_0x2cebd1]={'template':_0x398ad2,'mapId':_0x181a32,'eventId':_0x39fa44};},Game_System[_0x442592(0x34e)]['deletePreservedMorphEventDataKey']=function(_0x2d25e9,_0x56836f){const _0x137242=_0x442592;if(this[_0x137242(0x405)]===undefined)this['initEventsMoveCore']();const _0x5d2553='Map%1-Event%2'['format'](_0x2d25e9,_0x56836f);delete this[_0x137242(0x405)][_0x5d2553];},Game_System['prototype'][_0x442592(0x105)]=function(_0x23fa41){const _0x54ab72=_0x442592;if(this['_MapSpawnedEventData']===undefined)this['initEventsMoveCore']();return this[_0x54ab72(0x2eb)][_0x23fa41]=this[_0x54ab72(0x2eb)][_0x23fa41]||[],this[_0x54ab72(0x2eb)][_0x23fa41];},Game_System['prototype'][_0x442592(0x3c1)]=function(_0x584944){const _0x8d65d8=_0x442592,_0x136116=this[_0x8d65d8(0x105)](_0x584944);for(const _0x596390 of _0x136116){if(!_0x596390)continue;if(_0x596390[_0x8d65d8(0x340)])continue;const _0x1c67ed=_0x136116[_0x8d65d8(0x269)](_0x596390);_0x136116[_0x1c67ed]=null;}},Game_System[_0x442592(0x34e)][_0x442592(0x393)]=function(){const _0x26f20a=_0x442592;this[_0x26f20a(0x135)]=0x0,this['_followerChaseOff']=![];},Game_System['prototype']['getControlledFollowerID']=function(){const _0x4d9140=_0x442592;if(this[_0x4d9140(0x135)]===undefined)this[_0x4d9140(0x393)]();return this[_0x4d9140(0x135)];},Game_System[_0x442592(0x34e)][_0x442592(0x4ac)]=function(_0x51fae9){const _0x2faa1a=_0x442592;if(this['_followerControlID']===undefined)this[_0x2faa1a(0x393)]();this[_0x2faa1a(0x135)]=_0x51fae9;;},VisuMZ[_0x442592(0x3dc)][_0x442592(0x4ad)]=Game_Interpreter[_0x442592(0x34e)]['character'],Game_Interpreter[_0x442592(0x34e)][_0x442592(0x1f7)]=function(_0x11a5d1){const _0x25acb1=_0x442592;if(!$gameParty[_0x25acb1(0x1da)]()&&_0x11a5d1<0x0){let _0x5109e0=$gameSystem['getControlledFollowerID']();if(_0x5109e0>0x0)return $gamePlayer['followers']()[_0x25acb1(0x177)](_0x5109e0-0x1);}return VisuMZ[_0x25acb1(0x3dc)]['Game_Interpreter_character']['call'](this,_0x11a5d1);},Game_System[_0x442592(0x34e)]['isStopFollowerChasing']=function(){const _0x4f4df3=_0x442592;if(this[_0x4f4df3(0x4ef)]===undefined)this[_0x4f4df3(0x393)]();return this[_0x4f4df3(0x4ef)];},Game_System[_0x442592(0x34e)][_0x442592(0x88)]=function(_0x5a0156){const _0xf3f70e=_0x442592;if(this[_0xf3f70e(0x4ef)]===undefined)this[_0xf3f70e(0x393)]();this[_0xf3f70e(0x4ef)]=_0x5a0156;;},VisuMZ['EventsMoveCore']['Game_Followers_jumpAll']=Game_Followers['prototype'][_0x442592(0x41f)],Game_Followers[_0x442592(0x34e)][_0x442592(0x41f)]=function(){const _0x504c76=_0x442592;if($gameSystem[_0x504c76(0x384)]())return;VisuMZ['EventsMoveCore'][_0x504c76(0x516)]['call'](this);},VisuMZ['EventsMoveCore'][_0x442592(0xf1)]=Game_Timer[_0x442592(0x34e)]['initialize'],Game_Timer[_0x442592(0x34e)][_0x442592(0x327)]=function(){const _0x53f781=_0x442592;VisuMZ['EventsMoveCore'][_0x53f781(0xf1)][_0x53f781(0x98)](this),this[_0x53f781(0x47d)]();},Game_Timer[_0x442592(0x34e)][_0x442592(0x47d)]=function(){const _0x452c7e=_0x442592;this[_0x452c7e(0x304)]=![],this['_speed']=-0x1,this[_0x452c7e(0x49e)]=0x0;},Game_Timer[_0x442592(0x34e)][_0x442592(0x441)]=function(_0x39a389){const _0x426b31=_0x442592;if(!_0x39a389)return;if(!this[_0x426b31(0x4d9)])return;if(this['_paused'])return;if(this[_0x426b31(0x339)]<=0x0)return;if(this['_speed']===undefined)this[_0x426b31(0x47d)]();this[_0x426b31(0x339)]+=this[_0x426b31(0x12d)],this['_frames']<=0x0&&this[_0x426b31(0x484)]();},VisuMZ[_0x442592(0x3dc)][_0x442592(0x116)]=Game_Timer[_0x442592(0x34e)][_0x442592(0x2bb)],Game_Timer[_0x442592(0x34e)][_0x442592(0x2bb)]=function(_0x45ce50){const _0x344174=_0x442592;VisuMZ[_0x344174(0x3dc)][_0x344174(0x116)][_0x344174(0x98)](this,_0x45ce50);if(this[_0x344174(0x304)]===undefined)this[_0x344174(0x47d)]();this[_0x344174(0x304)]=![];},VisuMZ[_0x442592(0x3dc)]['Game_Timer_stop']=Game_Timer[_0x442592(0x34e)][_0x442592(0x502)],Game_Timer[_0x442592(0x34e)][_0x442592(0x502)]=function(){const _0x12fd76=_0x442592;VisuMZ[_0x12fd76(0x3dc)][_0x12fd76(0x3fe)][_0x12fd76(0x98)](this);if(this[_0x12fd76(0x304)]===undefined)this[_0x12fd76(0x47d)]();this[_0x12fd76(0x304)]=![];},Game_Timer[_0x442592(0x34e)]['pause']=function(){const _0x5a202f=_0x442592;if(this[_0x5a202f(0x339)]<=0x0)return;this[_0x5a202f(0x304)]=!![],this['_working']=!![];},Game_Timer[_0x442592(0x34e)][_0x442592(0x378)]=function(){const _0x1b0d2f=_0x442592;if(this[_0x1b0d2f(0x339)]<=0x0)return;this[_0x1b0d2f(0x304)]=![],this[_0x1b0d2f(0x4d9)]=!![];},Game_Timer[_0x442592(0x34e)][_0x442592(0x1e5)]=function(_0x2a0561){const _0x50fc90=_0x442592;this[_0x50fc90(0x339)]=this['_frames']||0x0,this[_0x50fc90(0x339)]+=_0x2a0561,this['_working']=!![],this[_0x50fc90(0x339)]=Math[_0x50fc90(0xcc)](0x1,this[_0x50fc90(0x339)]);},Game_Timer[_0x442592(0x34e)]['setFrames']=function(_0xb0b59d){const _0x574968=_0x442592;this['_frames']=this[_0x574968(0x339)]||0x0,this['_frames']=_0xb0b59d,this[_0x574968(0x4d9)]=!![],this[_0x574968(0x339)]=Math[_0x574968(0xcc)](0x1,this['_frames']);},Game_Timer[_0x442592(0x34e)][_0x442592(0x85)]=function(_0x58f1a0){const _0x3ceeaf=_0x442592;this[_0x3ceeaf(0x12d)]=_0x58f1a0,this[_0x3ceeaf(0x4d9)]=!![],_0x58f1a0>0x0&&(this['_frames']=Math[_0x3ceeaf(0xcc)](this[_0x3ceeaf(0x339)],0x1));},Game_Timer[_0x442592(0x34e)][_0x442592(0x4c5)]=function(_0x223ef7){const _0x3bbd4d=_0x442592;if(this[_0x3bbd4d(0x49e)]===undefined)this[_0x3bbd4d(0x47d)]();this[_0x3bbd4d(0x49e)]=_0x223ef7;},VisuMZ[_0x442592(0x3dc)][_0x442592(0x171)]=Game_Timer[_0x442592(0x34e)][_0x442592(0x484)],Game_Timer[_0x442592(0x34e)]['onExpire']=function(){const _0x1767cc=_0x442592;if(this['_expireCommonEvent']===undefined)this['initEventsMoveCore']();this['_expireCommonEvent']?$gameTemp[_0x1767cc(0x1bf)](this['_expireCommonEvent']):VisuMZ[_0x1767cc(0x3dc)]['Game_Timer_onExpire'][_0x1767cc(0x98)](this);},VisuMZ[_0x442592(0x3dc)][_0x442592(0x89)]=Game_Message[_0x442592(0x34e)][_0x442592(0x7a)],Game_Message[_0x442592(0x34e)][_0x442592(0x7a)]=function(_0x2b6b1e){const _0x237851=_0x442592;VisuMZ[_0x237851(0x3dc)]['Game_Message_add'][_0x237851(0x98)](this,_0x2b6b1e),this[_0x237851(0x25e)]=$gameTemp[_0x237851(0x4b4)]();},Game_Message[_0x442592(0x34e)]['registerSelfEvent']=function(){const _0x1ae9d9=_0x442592;$gameTemp[_0x1ae9d9(0x2e9)](this[_0x1ae9d9(0x25e)]);},VisuMZ[_0x442592(0x3dc)][_0x442592(0x3c8)]=Game_Switches[_0x442592(0x34e)]['value'],Game_Switches['prototype'][_0x442592(0x161)]=function(_0x32c058){const _0x5435d8=_0x442592;typeof _0x32c058==='string'&&(_0x32c058=VisuMZ[_0x5435d8(0x2db)][_0x32c058['toUpperCase']()[_0x5435d8(0x36f)]()]||0x1);if(DataManager[_0x5435d8(0x198)](_0x32c058))return!!this[_0x5435d8(0x101)](_0x32c058);else{if(DataManager['isSelfSwitch'](_0x32c058))return!!this['selfValue'](_0x32c058);else return DataManager[_0x5435d8(0x164)](_0x32c058)?!!this[_0x5435d8(0xff)](_0x32c058):VisuMZ['EventsMoveCore'][_0x5435d8(0x3c8)][_0x5435d8(0x98)](this,_0x32c058);}},Game_Switches[_0x442592(0x1fa)]={},Game_Switches['prototype']['advancedValue']=function(_0x183a19){const _0xed80d6=_0x442592;if(!Game_Switches[_0xed80d6(0x1fa)][_0x183a19]){$dataSystem[_0xed80d6(0x481)][_0x183a19][_0xed80d6(0x110)](/<JS>\s*([\s\S]*)\s*<\/JS>/i);const _0x4ab078=_0xed80d6(0x28f)[_0xed80d6(0x2e6)](String(RegExp['$1']));Game_Switches[_0xed80d6(0x1fa)][_0x183a19]=new Function('switchId',_0x4ab078);}const _0x19870c=$gameTemp[_0xed80d6(0x4b4)]()||this;return Game_Switches['advancedFunc'][_0x183a19]['call'](_0x19870c,_0x183a19);},Game_Switches[_0x442592(0x34e)][_0x442592(0x477)]=function(_0x3f4444){const _0x7aed61=_0x442592,_0xb29dc9=$gameTemp['getSelfTarget']()||this;if(_0xb29dc9[_0x7aed61(0x2a9)]!==Game_Event)return VisuMZ[_0x7aed61(0x3dc)]['Game_Switches_value'][_0x7aed61(0x98)](this,_0x3f4444);else{const _0x4456cc=[_0xb29dc9[_0x7aed61(0x118)],_0xb29dc9[_0x7aed61(0xa3)],'Self\x20Switch\x20%1'['format'](_0x3f4444)];return $gameSelfSwitches[_0x7aed61(0x161)](_0x4456cc);}},Game_Switches['prototype']['mapValue']=function(_0x3bd8e1){const _0x439552=_0x442592,_0x121729=$gameMap?$gameMap[_0x439552(0x321)]():0x0,_0xbf714f=[0x0,0x0,_0x439552(0x49c)[_0x439552(0x2e6)](_0x121729,_0x3bd8e1)];return $gameSelfSwitches[_0x439552(0x161)](_0xbf714f);},VisuMZ[_0x442592(0x3dc)][_0x442592(0x1a5)]=Game_Switches[_0x442592(0x34e)][_0x442592(0x3c5)],Game_Switches['prototype'][_0x442592(0x3c5)]=function(_0x2d2a95,_0x22ba72){const _0xe25480=_0x442592;typeof _0x2d2a95===_0xe25480(0x1fb)&&(_0x2d2a95=VisuMZ['RefSwitches'][_0x2d2a95[_0xe25480(0x22e)]()[_0xe25480(0x36f)]()]||0x1);if(DataManager['isSelfSwitch'](_0x2d2a95))this[_0xe25480(0x20a)](_0x2d2a95,_0x22ba72);else DataManager['isMapSwitch'](_0x2d2a95)?this[_0xe25480(0x326)](_0x2d2a95,_0x22ba72):VisuMZ[_0xe25480(0x3dc)][_0xe25480(0x1a5)]['call'](this,_0x2d2a95,_0x22ba72);},Game_Switches['prototype'][_0x442592(0x20a)]=function(_0x49008b,_0x1e333e){const _0x748271=_0x442592,_0x1de672=$gameTemp[_0x748271(0x4b4)]()||this;if(_0x1de672[_0x748271(0x2a9)]!==Game_Event)VisuMZ[_0x748271(0x3dc)][_0x748271(0x1a5)]['call'](this,_0x49008b,_0x1e333e);else{const _0x4ef83b=[_0x1de672[_0x748271(0x118)],_0x1de672[_0x748271(0xa3)],_0x748271(0x4a4)[_0x748271(0x2e6)](_0x49008b)];$gameSelfSwitches[_0x748271(0x3c5)](_0x4ef83b,_0x1e333e);}},Game_Switches[_0x442592(0x34e)]['setMapValue']=function(_0x155e5f,_0x162b79){const _0x4ecb37=_0x442592,_0x5c0124=$gameMap?$gameMap['mapId']():0x0,_0x1c7826=[0x0,0x0,_0x4ecb37(0x49c)[_0x4ecb37(0x2e6)](_0x5c0124,_0x155e5f)];return $gameSelfSwitches[_0x4ecb37(0x3c5)](_0x1c7826,_0x162b79);},VisuMZ['EventsMoveCore'][_0x442592(0x32f)]=Game_Variables[_0x442592(0x34e)][_0x442592(0x161)],Game_Variables[_0x442592(0x34e)][_0x442592(0x161)]=function(_0xd89409){const _0x3485af=_0x442592;typeof _0xd89409===_0x3485af(0x1fb)&&(_0xd89409=VisuMZ[_0x3485af(0x356)][switchId[_0x3485af(0x22e)]()['trim']()]||0x1);if(DataManager[_0x3485af(0x4e0)](_0xd89409))return this['advancedValue'](_0xd89409);else{if(DataManager[_0x3485af(0x3cd)](_0xd89409))return this[_0x3485af(0x477)](_0xd89409);else return DataManager[_0x3485af(0x20d)](_0xd89409)?this[_0x3485af(0xff)](_0xd89409):VisuMZ[_0x3485af(0x3dc)]['Game_Variables_value'][_0x3485af(0x98)](this,_0xd89409);}},Game_Variables[_0x442592(0x1fa)]={},Game_Variables[_0x442592(0x34e)][_0x442592(0x101)]=function(_0x2e53f3){const _0x3976e3=_0x442592;if(!Game_Variables[_0x3976e3(0x1fa)][_0x2e53f3]){$dataSystem[_0x3976e3(0x4a2)][_0x2e53f3][_0x3976e3(0x110)](/<JS>\s*([\s\S]*)\s*<\/JS>/i);const _0x17a742='return\x20%1'[_0x3976e3(0x2e6)](String(RegExp['$1']));Game_Variables[_0x3976e3(0x1fa)][_0x2e53f3]=new Function(_0x3976e3(0x2d5),_0x17a742);}const _0x333822=$gameTemp[_0x3976e3(0x4b4)]()||this;return Game_Variables[_0x3976e3(0x1fa)][_0x2e53f3][_0x3976e3(0x98)](_0x333822,_0x2e53f3);},Game_Variables[_0x442592(0x34e)][_0x442592(0x477)]=function(_0x361e46){const _0x38b152=_0x442592,_0x7865c1=$gameTemp[_0x38b152(0x4b4)]()||this;if(_0x7865c1[_0x38b152(0x2a9)]!==Game_Event)return VisuMZ[_0x38b152(0x3dc)]['Game_Variables_value'][_0x38b152(0x98)](this,_0x361e46);else{const _0x11080e=[_0x7865c1['_mapId'],_0x7865c1[_0x38b152(0xa3)],_0x38b152(0x39a)[_0x38b152(0x2e6)](_0x361e46)];return $gameSelfSwitches['value'](_0x11080e);}},Game_Variables[_0x442592(0x34e)][_0x442592(0xff)]=function(_0x9c20e8){const _0x445dee=_0x442592,_0x32f279=$gameMap?$gameMap[_0x445dee(0x321)]():0x0,_0x76faa0=[0x0,0x0,_0x445dee(0x13d)[_0x445dee(0x2e6)](_0x32f279,_0x9c20e8)];return $gameSelfSwitches['value'](_0x76faa0)||0x0;},VisuMZ[_0x442592(0x3dc)][_0x442592(0x317)]=Game_Variables[_0x442592(0x34e)]['setValue'],Game_Variables['prototype'][_0x442592(0x3c5)]=function(_0xc5db5b,_0x16ab72){const _0x168e50=_0x442592;typeof _0xc5db5b===_0x168e50(0x1fb)&&(_0xc5db5b=VisuMZ[_0x168e50(0x356)][switchId[_0x168e50(0x22e)]()[_0x168e50(0x36f)]()]||0x1);if(DataManager['isSelfVariable'](_0xc5db5b))this[_0x168e50(0x20a)](_0xc5db5b,_0x16ab72);else DataManager[_0x168e50(0x20d)](_0xc5db5b)?this[_0x168e50(0x326)](_0xc5db5b,_0x16ab72):VisuMZ['EventsMoveCore'][_0x168e50(0x317)][_0x168e50(0x98)](this,_0xc5db5b,_0x16ab72);},Game_Variables[_0x442592(0x34e)]['setSelfValue']=function(_0x3f8336,_0x3c6f94){const _0x498ce8=_0x442592,_0x345b7c=$gameTemp[_0x498ce8(0x4b4)]()||this;if(_0x345b7c['constructor']!==Game_Event)VisuMZ[_0x498ce8(0x3dc)][_0x498ce8(0x317)][_0x498ce8(0x98)](this,_0x3f8336,_0x3c6f94);else{const _0x9ee8df=[_0x345b7c['_mapId'],_0x345b7c[_0x498ce8(0xa3)],_0x498ce8(0x39a)[_0x498ce8(0x2e6)](_0x3f8336)];$gameSelfSwitches['setValue'](_0x9ee8df,_0x3c6f94);}},Game_Variables[_0x442592(0x34e)]['setMapValue']=function(_0x73d46e,_0xb67e59){const _0x5bc8bb=_0x442592,_0x5901d6=$gameMap?$gameMap[_0x5bc8bb(0x321)]():0x0,_0x4c0160=[0x0,0x0,_0x5bc8bb(0x13d)['format'](_0x5901d6,_0x73d46e)];$gameSelfSwitches[_0x5bc8bb(0x3c5)](_0x4c0160,_0xb67e59);},VisuMZ[_0x442592(0x3dc)][_0x442592(0x146)]=Game_SelfSwitches[_0x442592(0x34e)]['value'],Game_SelfSwitches['prototype'][_0x442592(0x161)]=function(_0x5daf55){const _0x25797e=_0x442592;if(_0x5daf55[0x2][_0x25797e(0x110)](/(?:SELF|MAP)/i))return this[_0x25797e(0x477)](_0x5daf55);else{return VisuMZ['EventsMoveCore'][_0x25797e(0x146)][_0x25797e(0x98)](this,_0x5daf55);;}},Game_SelfSwitches[_0x442592(0x34e)][_0x442592(0x477)]=function(_0x1e459c){const _0x2fe762=_0x442592;return _0x1e459c[0x2][_0x2fe762(0x110)](/VAR/i)?this[_0x2fe762(0x3ed)][_0x1e459c]||0x0:!!this['_data'][_0x1e459c];},VisuMZ[_0x442592(0x3dc)][_0x442592(0x94)]=Game_SelfSwitches[_0x442592(0x34e)][_0x442592(0x3c5)],Game_SelfSwitches[_0x442592(0x34e)]['setValue']=function(_0x4124fa,_0x2eb0a3){const _0x4b6ee0=_0x442592;_0x4124fa[0x2]['match'](/(?:SELF|MAP)/i)?this[_0x4b6ee0(0x20a)](_0x4124fa,_0x2eb0a3):VisuMZ[_0x4b6ee0(0x3dc)]['Game_SelfSwitches_setValue'][_0x4b6ee0(0x98)](this,_0x4124fa,_0x2eb0a3);},Game_SelfSwitches[_0x442592(0x34e)]['setSelfValue']=function(_0x3ccc1c,_0x293847){const _0x505720=_0x442592;this[_0x505720(0x3ed)][_0x3ccc1c]=_0x3ccc1c[0x2]['match'](/VAR/i)?_0x293847:!!_0x293847,this[_0x505720(0x86)]();},VisuMZ['EventsMoveCore']['Scene_Map_createDisplayObjects']=Scene_Map['prototype']['createDisplayObjects'],Scene_Map[_0x442592(0x34e)]['createDisplayObjects']=function(){const _0x43fd48=_0x442592;$gameMap[_0x43fd48(0x1f9)](),VisuMZ[_0x43fd48(0x3dc)][_0x43fd48(0x396)]['call'](this);},Game_Map['prototype'][_0x442592(0x1f9)]=function(){const _0x2f3945=_0x442592;if(this[_0x2f3945(0x4e2)]===this[_0x2f3945(0x321)]())return;this['_lastSesetExitSelfSwitchesMapId']=this[_0x2f3945(0x321)](),this[_0x2f3945(0x191)]=undefined;const _0x584dd8=this[_0x2f3945(0x35b)]();for(const _0x4d8582 of _0x584dd8){if(_0x4d8582)$gameSelfSwitches['resetSelfSwitchesForEvent'](_0x4d8582);}},Game_SelfSwitches[_0x442592(0x34e)]['resetSelfSwitchesForEvent']=function(_0x1821a7){const _0x49842e=_0x442592;if(!_0x1821a7)return;if(!_0x1821a7[_0x49842e(0x2f2)]())return;const _0x5b7699=_0x1821a7[_0x49842e(0x2f2)]()['note']||'';if(_0x5b7699[_0x49842e(0x110)](/<(?:EXIT RESET|EXIT|TEMP|TEMPORARY) (?:SELF|SELF SWITCH|SELF SWITCHES|SELF DATA)>/i)){const _0x439e9b='%1,%2,'[_0x49842e(0x2e6)]($gameMap['_mapId'],_0x1821a7[_0x49842e(0xa3)]),_0x5453e0=Object[_0x49842e(0x2ad)](this[_0x49842e(0x3ed)])[_0x49842e(0xda)](_0x11a8c4=>_0x11a8c4[_0x49842e(0x3ea)](_0x439e9b));while(_0x5453e0[_0x49842e(0x2fe)]>0x0){const _0x44dbc7=_0x5453e0[_0x49842e(0x259)]();delete this[_0x49842e(0x3ed)][_0x44dbc7];}}},Game_SelfSwitches[_0x442592(0x34e)][_0x442592(0x200)]=function(_0x168cd3){const _0x48fd22=_0x442592,_0x3fca35=_0x48fd22(0x185)[_0x48fd22(0x2e6)]($gameMap['_mapId']),_0x47d913=Object[_0x48fd22(0x2ad)](this['_data'])[_0x48fd22(0xda)](_0x1e90fd=>_0x1e90fd[_0x48fd22(0x3ea)](_0x3fca35));while(_0x47d913[_0x48fd22(0x2fe)]>0x0){const _0x39bd28=_0x47d913[_0x48fd22(0x259)]();delete this[_0x48fd22(0x3ed)][_0x39bd28];}_0x168cd3===$gameMap[_0x48fd22(0x321)]()&&$gameMap[_0x48fd22(0x29b)]();},VisuMZ[_0x442592(0x3dc)][_0x442592(0xb9)]=Game_Enemy[_0x442592(0x34e)][_0x442592(0x3df)],Game_Enemy[_0x442592(0x34e)][_0x442592(0x3df)]=function(_0xc84b1b){const _0x1febbb=_0x442592;$gameTemp[_0x1febbb(0x2e9)](this);const _0x2b3302=VisuMZ[_0x1febbb(0x3dc)][_0x1febbb(0xb9)]['call'](this,_0xc84b1b);return $gameTemp[_0x1febbb(0x1ed)](),_0x2b3302;},VisuMZ[_0x442592(0x3dc)][_0x442592(0x3ab)]=Game_Party['prototype'][_0x442592(0x366)],Game_Party['prototype'][_0x442592(0x366)]=function(){const _0x3efdbc=_0x442592;if(this[_0x3efdbc(0x3a4)]())return!![];return VisuMZ[_0x3efdbc(0x3dc)][_0x3efdbc(0x3ab)][_0x3efdbc(0x98)](this);},Game_Party[_0x442592(0x34e)][_0x442592(0x3a4)]=function(){if(this['_checkEncounterRaw'])return![];return $isTileEncounterHalf($gamePlayer['x'],$gamePlayer['y']);},VisuMZ[_0x442592(0x3dc)]['Game_Party_hasEncounterNone']=Game_Party[_0x442592(0x34e)][_0x442592(0x3be)],Game_Party[_0x442592(0x34e)][_0x442592(0x3be)]=function(){const _0x15ca22=_0x442592;if(this[_0x15ca22(0x17e)]())return!![];return VisuMZ['EventsMoveCore'][_0x15ca22(0x258)][_0x15ca22(0x98)](this);},Game_Party[_0x442592(0x34e)][_0x442592(0x17e)]=function(){const _0x202d60=_0x442592;if(this[_0x202d60(0x9b)])return![];return $isTileEncounterNone($gamePlayer['x'],$gamePlayer['y']);};var $isTileEncounterHalf=function(_0x23ff06,_0x348de8){const _0x1d1515=_0x442592;if(!$gameMap)return![];_0x23ff06=Math[_0x1d1515(0x1e4)](_0x23ff06||0x0),_0x348de8=Math['round'](_0x348de8||0x0);const _0x1c8e4d=$gameMap[_0x1d1515(0x35b)]();for(const _0x3df557 of _0x1c8e4d){if(!_0x3df557)continue;if(_0x3df557[_0x1d1515(0x427)])continue;const _0x5e19da=_0x3df557[_0x1d1515(0x3fa)](!![]),_0x1d78e5=_0x3df557[_0x1d1515(0x54f)](!![]);if($gameMap[_0x1d1515(0x1cf)](_0x23ff06,_0x348de8,_0x3df557,_0x5e19da,_0x1d78e5))return!![];}return![];},$isTileEncounterNone=function(_0x47de21,_0xe2f7f5){const _0x146d8d=_0x442592;if(!$gameMap)return![];_0x47de21=Math[_0x146d8d(0x1e4)](_0x47de21||0x0),_0xe2f7f5=Math[_0x146d8d(0x1e4)](_0xe2f7f5||0x0);const _0x2e8db7=$gameMap[_0x146d8d(0x35b)]();for(const _0x338fee of _0x2e8db7){if(!_0x338fee)continue;if(_0x338fee[_0x146d8d(0x427)])continue;const _0x3007b1=_0x338fee[_0x146d8d(0x3fa)](![]),_0x360a26=_0x338fee[_0x146d8d(0x54f)](![]);if($gameMap[_0x146d8d(0x1cf)](_0x47de21,_0xe2f7f5,_0x338fee,_0x3007b1,_0x360a26))return!![];}return![];};VisuMZ[_0x442592(0x3dc)][_0x442592(0x82)]=Game_Troop[_0x442592(0x34e)][_0x442592(0xe3)],Game_Troop[_0x442592(0x34e)]['meetsConditions']=function(_0x4ee41a){const _0x4bc9fa=_0x442592;$gameTemp[_0x4bc9fa(0x2e9)](this);const _0x5b44ed=VisuMZ[_0x4bc9fa(0x3dc)][_0x4bc9fa(0x82)]['call'](this,_0x4ee41a);return $gameTemp[_0x4bc9fa(0x1ed)](),_0x5b44ed;},VisuMZ[_0x442592(0x3dc)]['Game_Map_setup']=Game_Map[_0x442592(0x34e)]['setup'],Game_Map[_0x442592(0x34e)][_0x442592(0x204)]=function(_0x1ba879){const _0x5d9d1b=_0x442592;this[_0x5d9d1b(0x3c1)](_0x1ba879),this[_0x5d9d1b(0x3ef)](),VisuMZ[_0x5d9d1b(0x3dc)]['Game_Map_setup'][_0x5d9d1b(0x98)](this,_0x1ba879),this['clearEventCache'](),this[_0x5d9d1b(0x414)](),this[_0x5d9d1b(0x49a)](),this[_0x5d9d1b(0x2ea)](),this[_0x5d9d1b(0x54d)](),this[_0x5d9d1b(0x38b)](),this[_0x5d9d1b(0x377)](),this[_0x5d9d1b(0x499)](),this[_0x5d9d1b(0x42b)](),this[_0x5d9d1b(0x3ef)]();},VisuMZ[_0x442592(0x3dc)][_0x442592(0xb3)]=Game_Map[_0x442592(0x34e)][_0x442592(0x54b)],Game_Map[_0x442592(0x34e)][_0x442592(0x54b)]=function(){const _0x466118=_0x442592;VisuMZ[_0x466118(0x3dc)][_0x466118(0xb3)][_0x466118(0x98)](this),this[_0x466118(0x158)]();},Game_Map[_0x442592(0x165)]=0xc8,Game_Map[_0x442592(0x34e)]['determineEventOverload']=function(){const _0x5407f1=_0x442592,_0x42455f=Game_Map[_0x5407f1(0x165)];this[_0x5407f1(0x3a6)]=this[_0x5407f1(0x35b)]()[_0x5407f1(0x2fe)]>_0x42455f;if(this[_0x5407f1(0x3a6)]&&$gameTemp[_0x5407f1(0x255)]()){}},Game_Map[_0x442592(0x34e)][_0x442592(0x14e)]=function(){return this['_eventOverload'];},Game_Map[_0x442592(0x34e)][_0x442592(0x3ef)]=function(){const _0x464494=_0x442592;this[_0x464494(0x191)]=undefined;},Game_Map[_0x442592(0x34e)][_0x442592(0x414)]=function(){const _0x57f9c1=_0x442592;this[_0x57f9c1(0x4fe)]=VisuMZ[_0x57f9c1(0x3dc)][_0x57f9c1(0x240)][_0x57f9c1(0x41e)][_0x57f9c1(0x1a8)];const _0x594a34=$dataMap['note']||'';if(_0x594a34[_0x57f9c1(0x110)](/<DIAGONAL MOVEMENT: ON>/i))this[_0x57f9c1(0x4fe)]=!![];else _0x594a34['match'](/<DIAGONAL MOVEMENT: OFF>/i)&&(this[_0x57f9c1(0x4fe)]=![]);},Game_Map[_0x442592(0x273)]=VisuMZ[_0x442592(0x3dc)][_0x442592(0x240)][_0x442592(0x41e)][_0x442592(0xac)]??![],Game_Map[_0x442592(0x34e)]['isSupportDiagonalMovement']=function(){const _0x14025b=_0x442592;if(Utils[_0x14025b(0x1fc)]()){if(!Game_Map[_0x14025b(0x273)])return![];}const _0x4a41b1=$gameSystem['getPlayerDiagonalSetting']();if(_0x4a41b1===_0x14025b(0x485))return!![];if(_0x4a41b1===_0x14025b(0x2f6))return![];if(this['_diagonalSupport']===undefined)this['setupDiagonalSupport']();return this[_0x14025b(0x4fe)];},Game_Map[_0x442592(0x34e)][_0x442592(0xd4)]=function(_0xf69c99,_0x1fd995){const _0x40d833=_0x442592;if([0x1,0x4,0x7][_0x40d833(0xb5)](_0x1fd995))_0xf69c99-=0x1;if([0x3,0x6,0x9][_0x40d833(0xb5)](_0x1fd995))_0xf69c99+=0x1;return this['roundX'](_0xf69c99);},Game_Map[_0x442592(0x34e)]['roundYWithDirection']=function(_0x493ed1,_0x4441f8){const _0x2eada6=_0x442592;if([0x1,0x2,0x3][_0x2eada6(0xb5)](_0x4441f8))_0x493ed1+=0x1;if([0x7,0x8,0x9]['includes'](_0x4441f8))_0x493ed1-=0x1;return this[_0x2eada6(0x142)](_0x493ed1);},Game_Map[_0x442592(0x34e)][_0x442592(0x268)]=function(_0x59f86f,_0x10ca5c,_0x2560fa,_0x4c7fad){const _0x5cc716=_0x442592;return Math[_0x5cc716(0xcc)](Math[_0x5cc716(0x382)](this[_0x5cc716(0x71)](_0x59f86f,_0x2560fa)),Math[_0x5cc716(0x382)](this[_0x5cc716(0xca)](_0x10ca5c,_0x4c7fad)));},Game_Map[_0x442592(0x34e)][_0x442592(0x49a)]=function(){const _0x349051=_0x442592,_0x4f92b1=VisuMZ[_0x349051(0x3dc)][_0x349051(0x240)][_0x349051(0x4ba)],_0x1c5ac6={},_0x268374=[_0x349051(0x2c6),'Forbid',_0x349051(0x325)],_0x2f48dd=[_0x349051(0x528),'Walk','Player','Event',_0x349051(0x278),_0x349051(0x3fd),_0x349051(0x270),_0x349051(0x374)];for(const _0x153efa of _0x268374){for(const _0x35a68b of _0x2f48dd){const _0x209f1a='%1%2'['format'](_0x35a68b,_0x153efa);_0x4f92b1[_0x209f1a]&&(_0x1c5ac6[_0x209f1a]=_0x4f92b1[_0x209f1a]['slice'](0x0));}}const _0x294d3a=$dataMap[_0x349051(0x2fc)]||'',_0xe5a13b=_0x294d3a['match'](/<(.*) (.*) REGION:[ ]*(\d+(?:\s*,\s*\d+)*)>/gi);if(_0xe5a13b)for(const _0x407ff4 of _0xe5a13b){_0x407ff4[_0x349051(0x110)](/<(.*) (.*) REGION:[ ]*(\d+(?:\s*,\s*\d+)*)>/i);let _0x137b4a=String(RegExp['$1'])[_0x349051(0xdd)]()[_0x349051(0x36f)](),_0x5c3a4f=String(RegExp['$2'])[_0x349051(0xdd)]()[_0x349051(0x36f)]();const _0x5d6956=JSON[_0x349051(0x2ab)]('['+RegExp['$3'][_0x349051(0x110)](/\d+/g)+']');_0x137b4a=_0x137b4a[_0x349051(0x4ea)](0x0)[_0x349051(0x22e)]()+_0x137b4a['slice'](0x1),_0x5c3a4f=_0x5c3a4f[_0x349051(0x4ea)](0x0)['toUpperCase']()+_0x5c3a4f[_0x349051(0x1f4)](0x1);const _0x32bc66=_0x349051(0x3ca)[_0x349051(0x2e6)](_0x137b4a,_0x5c3a4f);if(_0x1c5ac6[_0x32bc66])_0x1c5ac6[_0x32bc66]=_0x1c5ac6[_0x32bc66][_0x349051(0x2c2)](_0x5d6956);}this[_0x349051(0x472)]=_0x1c5ac6;},Game_Map[_0x442592(0x34e)][_0x442592(0x36c)]=function(_0x42654e,_0x5a4478,_0x4def51,_0x18c527){const _0x258e80=_0x442592,_0xa7f8ef=this[_0x258e80(0xd4)](_0x42654e,_0x4def51),_0x4ec121=this[_0x258e80(0x114)](_0x5a4478,_0x4def51),_0x68a6b=this['regionId'](_0xa7f8ef,_0x4ec121),_0xdc851=this[_0x258e80(0x472)];if(_0xdc851[_0x258e80(0x17f)]['includes'](_0x68a6b))return!![];else{if(_0x18c527===_0x258e80(0x17a))return _0xdc851[_0x258e80(0x2d7)][_0x258e80(0xb5)](_0x68a6b)||_0xdc851[_0x258e80(0x168)][_0x258e80(0xb5)](_0x68a6b);else{if(_0x18c527===_0x258e80(0x2f2))return _0xdc851['EventAllow'][_0x258e80(0xb5)](_0x68a6b)||_0xdc851['WalkAllow'][_0x258e80(0xb5)](_0x68a6b);else{if(_0xdc851['VehicleAllow'][_0x258e80(0xb5)](_0x68a6b))return!![];else{const _0x1b8f1c=_0x258e80(0xd7)[_0x258e80(0x2e6)](_0x18c527[_0x258e80(0x4ea)](0x0)[_0x258e80(0x22e)]()+_0x18c527[_0x258e80(0x1f4)](0x1));if(_0xdc851[_0x1b8f1c])return _0xdc851[_0x1b8f1c][_0x258e80(0xb5)](_0x68a6b);}}}}return![];},Game_Map[_0x442592(0x34e)]['isRegionForbidPass']=function(_0x21c0d1,_0x1ea73b,_0x4d6438,_0x195e4d){const _0xa63710=_0x442592,_0x20fcf1=this[_0xa63710(0xd4)](_0x21c0d1,_0x4d6438),_0x2ece60=this['roundYWithDirection'](_0x1ea73b,_0x4d6438),_0x123482=this['regionId'](_0x20fcf1,_0x2ece60),_0x32e3ce=this[_0xa63710(0x472)];if(_0x32e3ce[_0xa63710(0x170)][_0xa63710(0xb5)](_0x123482))return!![];else{if(_0x195e4d===_0xa63710(0x17a))return _0x32e3ce[_0xa63710(0x398)]['includes'](_0x123482)||_0x32e3ce[_0xa63710(0x104)][_0xa63710(0xb5)](_0x123482);else{if(_0x195e4d===_0xa63710(0x2f2))return _0x32e3ce[_0xa63710(0x12b)][_0xa63710(0xb5)](_0x123482)||_0x32e3ce[_0xa63710(0x104)][_0xa63710(0xb5)](_0x123482);else{if(_0x32e3ce[_0xa63710(0x50a)][_0xa63710(0xb5)](_0x123482))return!![];else{const _0x335a3c=_0xa63710(0x169)['format'](_0x195e4d['charAt'](0x0)[_0xa63710(0x22e)]()+_0x195e4d[_0xa63710(0x1f4)](0x1));if(_0x32e3ce[_0x335a3c])return _0x32e3ce[_0x335a3c][_0xa63710(0xb5)](_0x123482);}}}}return![];},Game_Map['prototype'][_0x442592(0x3c6)]=function(_0x514992,_0x45825e,_0x22e67c,_0x3cadc7){const _0x289964=_0x442592;_0x22e67c=_0x3cadc7===_0x289964(0x4e9)?0x5:_0x22e67c;const _0x51691c=this[_0x289964(0xd4)](_0x514992,_0x22e67c),_0x13764f=this[_0x289964(0x114)](_0x45825e,_0x22e67c),_0x20ca2a=this['regionId'](_0x51691c,_0x13764f),_0x5acf53=this['_regionRules'];if(_0x5acf53[_0x289964(0x31b)][_0x289964(0xb5)](_0x20ca2a))return!![];else{const _0x588c9d=_0x289964(0x529)['format'](_0x3cadc7[_0x289964(0x4ea)](0x0)[_0x289964(0x22e)]()+_0x3cadc7['slice'](0x1));if(_0x5acf53[_0x588c9d])return _0x5acf53[_0x588c9d][_0x289964(0xb5)](_0x20ca2a);}return![];},VisuMZ[_0x442592(0x3dc)][_0x442592(0x44f)]=Game_Map[_0x442592(0x34e)][_0x442592(0x29c)],Game_Map[_0x442592(0x34e)][_0x442592(0x29c)]=function(){const _0x573322=_0x442592;VisuMZ[_0x573322(0x3dc)][_0x573322(0x44f)][_0x573322(0x98)](this),this['checkNeedForPeriodicRefresh']();},Game_Map[_0x442592(0x34e)][_0x442592(0x183)]=function(){const _0x544f98=_0x442592;this[_0x544f98(0x90)]=![];if(this[_0x544f98(0x35b)]()[_0x544f98(0x474)](_0x4eed5f=>_0x4eed5f[_0x544f98(0x43d)]())){this['_needsPeriodicRefresh']=!![];return;}if(this[_0x544f98(0x35b)]()[_0x544f98(0x474)](_0x957bd7=>_0x957bd7['hasCPCs']())){this[_0x544f98(0x90)]=!![];return;}if(this[_0x544f98(0x383)][_0x544f98(0x474)](_0x31a00c=>_0x31a00c[_0x544f98(0x43d)]())){this[_0x544f98(0x90)]=!![];return;}if(this[_0x544f98(0x383)][_0x544f98(0x474)](_0x505278=>_0x505278[_0x544f98(0x22f)]())){this['_needsPeriodicRefresh']=!![];return;}},VisuMZ['EventsMoveCore']['Game_Map_update']=Game_Map[_0x442592(0x34e)]['update'],Game_Map[_0x442592(0x34e)]['update']=function(_0xc90047){const _0x13f204=_0x442592;this[_0x13f204(0x260)](),VisuMZ['EventsMoveCore'][_0x13f204(0x307)][_0x13f204(0x98)](this,_0xc90047);},Game_Map[_0x442592(0x34e)]['updatePeriodicRefresh']=function(){const _0x3ad3db=_0x442592;if(!this[_0x3ad3db(0x90)])return;this[_0x3ad3db(0x54a)]=this[_0x3ad3db(0x54a)]||0x3c,this['_periodicRefreshTimer']--,this['_periodicRefreshTimer']<=0x0&&(this[_0x3ad3db(0x29b)](),this['_periodicRefreshTimer']=0x3c);},VisuMZ[_0x442592(0x3dc)][_0x442592(0x26d)]=Game_Map['prototype'][_0x442592(0x453)],Game_Map[_0x442592(0x34e)]['isDashDisabled']=function(){const _0x45a379=_0x442592;if(!$gameSystem[_0x45a379(0x39c)]())return!![];return VisuMZ[_0x45a379(0x3dc)][_0x45a379(0x26d)]['call'](this);},Game_Map[_0x442592(0x34e)][_0x442592(0x2ea)]=function(){const _0x352237=_0x442592;this['_saveEventLocations']=![];const _0x2538fa=$dataMap[_0x352237(0x2fc)]||'';_0x2538fa[_0x352237(0x110)](/<SAVE EVENT (?:LOCATION|LOCATIONS)>/i)&&(this['_saveEventLocations']=!![]);},Game_Map[_0x442592(0x34e)][_0x442592(0x113)]=function(){const _0x47309f=_0x442592;if(this[_0x47309f(0x443)]===undefined)this[_0x47309f(0x2ea)]();return this[_0x47309f(0x443)];},Game_Map[_0x442592(0x34e)][_0x442592(0x3c1)]=function(_0x286981){const _0x293651=_0x442592;_0x286981!==this[_0x293651(0x321)]()&&$gamePlayer&&$gameSystem['removeTemporaryMapSpawnedEvents'](this[_0x293651(0x321)]());},Game_Map[_0x442592(0x34e)]['setupSpawnedEvents']=function(){const _0x39b934=_0x442592;this[_0x39b934(0x292)]=$gameSystem[_0x39b934(0x105)](this['mapId']()),this[_0x39b934(0x47f)]=!![];},VisuMZ[_0x442592(0x3dc)][_0x442592(0x1a2)]=Game_Map['prototype'][_0x442592(0x35b)],Game_Map[_0x442592(0x34e)][_0x442592(0x35b)]=function(){const _0x13dde6=_0x442592;if(this['_eventCache'])return this[_0x13dde6(0x191)];const _0x10669d=VisuMZ[_0x13dde6(0x3dc)][_0x13dde6(0x1a2)][_0x13dde6(0x98)](this),_0x1af16e=_0x10669d[_0x13dde6(0x2c2)](this[_0x13dde6(0x292)]||[]);return this[_0x13dde6(0x191)]=_0x1af16e[_0x13dde6(0xda)](_0x172c51=>!!_0x172c51),this[_0x13dde6(0x191)];},VisuMZ[_0x442592(0x3dc)][_0x442592(0x14f)]=Game_Map['prototype'][_0x442592(0x2f2)],Game_Map[_0x442592(0x34e)][_0x442592(0x2f2)]=function(_0x2a6741){const _0x1f68aa=_0x442592;return _0x2a6741>=0x3e8?(_0x2a6741-=0x3e8,this[_0x1f68aa(0x292)][_0x2a6741]):VisuMZ[_0x1f68aa(0x3dc)]['Game_Map_event'][_0x1f68aa(0x98)](this,_0x2a6741);},Game_Map[_0x442592(0x34e)][_0x442592(0x318)]=function(_0x334f2c){const _0x193903=_0x442592,_0x5c350f=this[_0x193903(0x2f2)](_0x334f2c);if(_0x5c350f)_0x5c350f[_0x193903(0x4cd)]();},Game_Map[_0x442592(0x34e)][_0x442592(0x33a)]=function(){const _0x548571=_0x442592,_0x2957ea={'template':'Button','mapId':0x1,'eventId':0xc,'x':$gamePlayer['x']+0x1,'y':$gamePlayer['y']+0x1,'spawnPreserved':!![],'spawnEventId':this[_0x548571(0x292)][_0x548571(0x2fe)]+0x3e8};this[_0x548571(0x4cf)](_0x2957ea);},Game_Map[_0x442592(0x34e)][_0x442592(0x4a6)]=function(_0x16def,_0x4d205d){const _0x131ad3=_0x442592;if(this[_0x131ad3(0x33b)](_0x16def,_0x4d205d)[_0x131ad3(0x2fe)]>0x0)return!![];if($gamePlayer['x']===_0x16def&&$gamePlayer['y']===_0x4d205d)return!![];if(this[_0x131ad3(0x2e0)]()['posNt'](_0x16def,_0x4d205d))return!![];if(this[_0x131ad3(0x313)]()[_0x131ad3(0x503)](_0x16def,_0x4d205d))return!![];return![];},Game_Map[_0x442592(0x34e)][_0x442592(0x4b8)]=function(_0x582b1b,_0x174883,_0x225a2b){const _0x416212=_0x442592;$gameTemp[_0x416212(0x3c7)]=_0x582b1b;const _0x4eb493=new Game_Event(_0x582b1b['mapId'],_0x582b1b[_0x416212(0xb6)]);$gameTemp['_spawnData']=undefined,_0x4eb493[_0x416212(0x29c)]();let _0x3c728d=_0x174883-_0x4eb493[_0x416212(0x1c5)][_0x416212(0x1c4)],_0x3f24dc=_0x174883+_0x4eb493[_0x416212(0x1c5)][_0x416212(0x28a)],_0x5d0c01=_0x225a2b-_0x4eb493[_0x416212(0x1c5)]['up'],_0x526dad=_0x225a2b+_0x4eb493[_0x416212(0x1c5)][_0x416212(0x17d)];for(let _0x393386=_0x3c728d;_0x393386<=_0x3f24dc;_0x393386++){for(let _0x362697=_0x5d0c01;_0x362697<=_0x526dad;_0x362697++){if(this[_0x416212(0x4a6)](_0x393386,_0x362697))return![];}}return!![];},Game_Map['prototype'][_0x442592(0x4cf)]=function(_0x5d4781){const _0x8f44b3=_0x442592;$gameTemp['_spawnData']=_0x5d4781;const _0x490772=new Game_Event(_0x5d4781[_0x8f44b3(0x321)],_0x5d4781[_0x8f44b3(0xb6)]);$gameTemp[_0x8f44b3(0x3c7)]=undefined,this[_0x8f44b3(0x292)][_0x8f44b3(0x314)](_0x490772),_0x490772['setupSpawn'](_0x5d4781),this['clearEventCache']();},Game_Map[_0x442592(0x34e)]['prepareSpawnedEventAtXY']=function(_0x1a5758,_0x3f4bb2,_0x369f31){const _0x3f43a1=_0x442592,_0x1610bf=_0x1a5758['template'][_0x3f43a1(0x22e)]()[_0x3f43a1(0x36f)]();if(_0x1610bf!==_0x3f43a1(0x2b8)){const _0x683f0d=VisuMZ['EventTemplates'][_0x1610bf];_0x683f0d&&(_0x1a5758[_0x3f43a1(0x321)]=_0x683f0d['MapID'],_0x1a5758[_0x3f43a1(0xb6)]=_0x683f0d[_0x3f43a1(0x28b)]);}const _0x140958=_0x1a5758['x'],_0x2fe157=_0x1a5758['y'];if(!this[_0x3f43a1(0x1ec)](_0x140958,_0x2fe157))return![];if(_0x3f4bb2){if(this[_0x3f43a1(0x4a6)](_0x140958,_0x2fe157))return![];if(!this['isSpawnHitboxCollisionOk'](_0x1a5758,_0x140958,_0x2fe157))return![];}if(_0x369f31){if(!this['isPassableByAnyDirection'](_0x140958,_0x2fe157))return![];}return this['createSpawnedEventWithData'](_0x1a5758),!![];},Game_Map[_0x442592(0x34e)][_0x442592(0x3d4)]=function(_0x3de7f3,_0x92965a,_0x143d34,_0x1c38c4){const _0x59fcec=_0x442592,_0x436ea5=_0x3de7f3['template'][_0x59fcec(0x22e)]()[_0x59fcec(0x36f)]();if(_0x436ea5!==_0x59fcec(0x2b8)){const _0x53a30=VisuMZ[_0x59fcec(0x550)][_0x436ea5];_0x53a30&&(_0x3de7f3['mapId']=_0x53a30['MapID'],_0x3de7f3[_0x59fcec(0xb6)]=_0x53a30[_0x59fcec(0x28b)]);}const _0x327720=[],_0xd9c686=this[_0x59fcec(0x33f)](),_0x2ce196=this[_0x59fcec(0xc4)]();for(let _0x17c733=0x0;_0x17c733<_0xd9c686;_0x17c733++){for(let _0x34699c=0x0;_0x34699c<_0x2ce196;_0x34699c++){if(!_0x92965a[_0x59fcec(0xb5)](this['regionId'](_0x17c733,_0x34699c)))continue;if(!this[_0x59fcec(0x1ec)](_0x17c733,_0x34699c))continue;if(_0x143d34){if(this[_0x59fcec(0x4a6)](_0x17c733,_0x34699c))continue;if(!this[_0x59fcec(0x4b8)](_0x3de7f3,_0x17c733,_0x34699c))continue;}if(_0x1c38c4){if(!this['isPassableByAnyDirection'](_0x17c733,_0x34699c))continue;}_0x327720[_0x59fcec(0x314)]([_0x17c733,_0x34699c]);}}if(_0x327720[_0x59fcec(0x2fe)]>0x0){const _0x5bd3b3=_0x327720[Math[_0x59fcec(0x506)](_0x327720[_0x59fcec(0x2fe)])];return _0x3de7f3['x']=_0x5bd3b3[0x0],_0x3de7f3['y']=_0x5bd3b3[0x1],this[_0x59fcec(0x4cf)](_0x3de7f3),!![];}return![];},Game_Map[_0x442592(0x34e)][_0x442592(0x1a7)]=function(_0x4e46ef,_0x79d879,_0x5238ae,_0x5f5057){const _0x387b87=_0x442592,_0x285266=_0x4e46ef[_0x387b87(0x379)]['toUpperCase']()[_0x387b87(0x36f)]();if(_0x285266!=='UNTITLED'){const _0x3aac55=VisuMZ['EventTemplates'][_0x285266];_0x3aac55&&(_0x4e46ef[_0x387b87(0x321)]=_0x3aac55[_0x387b87(0x3cb)],_0x4e46ef['eventId']=_0x3aac55[_0x387b87(0x28b)]);}const _0x38f184=[],_0x4a562b=this[_0x387b87(0x33f)](),_0x5718ce=this[_0x387b87(0xc4)]();for(let _0x1d1fdd=0x0;_0x1d1fdd<_0x4a562b;_0x1d1fdd++){for(let _0x221e4b=0x0;_0x221e4b<_0x5718ce;_0x221e4b++){if(!_0x79d879[_0x387b87(0xb5)](this['terrainTag'](_0x1d1fdd,_0x221e4b)))continue;if(!this[_0x387b87(0x1ec)](_0x1d1fdd,_0x221e4b))continue;if(_0x5238ae){if(this[_0x387b87(0x4a6)](_0x1d1fdd,_0x221e4b))continue;if(!this['isSpawnHitboxCollisionOk'](_0x4e46ef,_0x1d1fdd,_0x221e4b))continue;}if(_0x5f5057){if(!this[_0x387b87(0xce)](_0x1d1fdd,_0x221e4b))continue;}_0x38f184[_0x387b87(0x314)]([_0x1d1fdd,_0x221e4b]);}}if(_0x38f184['length']>0x0){const _0x2792f4=_0x38f184[Math['randomInt'](_0x38f184[_0x387b87(0x2fe)])];return _0x4e46ef['x']=_0x2792f4[0x0],_0x4e46ef['y']=_0x2792f4[0x1],this[_0x387b87(0x4cf)](_0x4e46ef),!![];}return![];},Game_Map[_0x442592(0x34e)][_0x442592(0xce)]=function(_0x28e7cd,_0x216531){const _0x4bcf3f=_0x442592;if(this[_0x4bcf3f(0x251)](_0x28e7cd,_0x216531,0x2))return!![];if(this[_0x4bcf3f(0x251)](_0x28e7cd,_0x216531,0x4))return!![];if(this[_0x4bcf3f(0x251)](_0x28e7cd,_0x216531,0x6))return!![];if(this[_0x4bcf3f(0x251)](_0x28e7cd,_0x216531,0x8))return!![];return![];},Game_Map[_0x442592(0x34e)][_0x442592(0xa6)]=function(_0x5f4ab9){const _0x283ca7=_0x442592;if(_0x5f4ab9<0x3e8)return;if(!this['_spawnedEvents'])return;const _0x491807=this['event'](_0x5f4ab9);_0x491807[_0x283ca7(0x40b)](-0x1,-0x1),_0x491807['erase'](),this[_0x283ca7(0x292)][_0x5f4ab9-0x3e8]=null,this[_0x283ca7(0x3ef)]();},Game_Map[_0x442592(0x34e)][_0x442592(0x470)]=function(){for(const _0xa0beae of this['_spawnedEvents']){if(_0xa0beae)return _0xa0beae;}return null;},Game_Map[_0x442592(0x34e)][_0x442592(0x20f)]=function(){const _0x214452=_0x442592,_0x426191=this[_0x214452(0x470)]();return _0x426191?_0x426191[_0x214452(0xa3)]:0x0;},Game_Map[_0x442592(0x34e)][_0x442592(0xa7)]=function(){const _0x278ef3=_0x442592,_0x1232d7=this['_spawnedEvents']['slice'](0x0)[_0x278ef3(0x4e4)]();for(const _0x4d8819 of _0x1232d7){if(_0x4d8819)return _0x4d8819;}return null;},Game_Map['prototype'][_0x442592(0x395)]=function(){const _0x2bb4ab=_0x442592,_0x38df5f=this[_0x2bb4ab(0xa7)]();return _0x38df5f?_0x38df5f[_0x2bb4ab(0xa3)]:0x0;},Game_Map[_0x442592(0x34e)]['despawnAtXY']=function(_0x5f153c,_0x337884){const _0x3f400e=_0x442592,_0x58df81=this['eventsXy'](_0x5f153c,_0x337884);for(const _0x509df9 of _0x58df81){if(!_0x509df9)continue;if(_0x509df9[_0x3f400e(0x3f2)]())this['despawnEventId'](_0x509df9[_0x3f400e(0xa3)]);}},Game_Map['prototype']['despawnRegions']=function(_0x3faeb8){const _0xfd927c=_0x442592;for(const _0x226f11 of this[_0xfd927c(0x292)]){if(!_0x226f11)continue;_0x3faeb8['includes'](_0x226f11[_0xfd927c(0x8b)]())&&this['despawnEventId'](_0x226f11[_0xfd927c(0xa3)]);}},Game_Map[_0x442592(0x34e)]['despawnTerrainTags']=function(_0x23c422){const _0x5577af=_0x442592;for(const _0x3e3878 of this[_0x5577af(0x292)]){if(!_0x3e3878)continue;_0x23c422[_0x5577af(0xb5)](_0x3e3878[_0x5577af(0xc7)]())&&this[_0x5577af(0xa6)](_0x3e3878['_eventId']);}},Game_Map[_0x442592(0x34e)][_0x442592(0x1ee)]=function(){const _0x53555b=_0x442592;for(const _0x1c6149 of this[_0x53555b(0x292)]){if(!_0x1c6149)continue;this[_0x53555b(0xa6)](_0x1c6149[_0x53555b(0xa3)]);}},VisuMZ['EventsMoveCore'][_0x442592(0x2bd)]=Game_Map[_0x442592(0x34e)][_0x442592(0x32b)],Game_Map[_0x442592(0x34e)][_0x442592(0x32b)]=function(_0x408b38){const _0x1940a6=_0x442592;VisuMZ[_0x1940a6(0x3dc)][_0x1940a6(0x2bd)]['call'](this,_0x408b38);if(_0x408b38>=0x3e8){const _0x5e5d7c=this[_0x1940a6(0x2f2)](_0x408b38);if(_0x5e5d7c)_0x5e5d7c[_0x1940a6(0x354)]();}},Game_Map['prototype'][_0x442592(0x38b)]=function(){const _0xf06165=_0x442592;this[_0xf06165(0x358)]=![],this['_forceHidePlayer']=![];if(!$dataMap)return;const _0x275f8d=$dataMap[_0xf06165(0x2fc)]||'';if(_0x275f8d['match'](/<HIDE PLAYER>/i))this[_0xf06165(0x358)]=![],this[_0xf06165(0x447)]=!![];else _0x275f8d[_0xf06165(0x110)](/<SHOW PLAYER>/i)&&(this['_forceShowPlayer']=!![],this[_0xf06165(0x447)]=![]);},Game_Map[_0x442592(0x34e)][_0x442592(0x50c)]=function(){const _0x3f4e1c=_0x442592;return this[_0x3f4e1c(0x358)]===undefined&&this[_0x3f4e1c(0x38b)](),this['_forceShowPlayer'];},Game_Map['prototype'][_0x442592(0x1d0)]=function(){const _0x374593=_0x442592;return this[_0x374593(0x447)]===undefined&&this[_0x374593(0x38b)](),this[_0x374593(0x447)];},VisuMZ['EventsMoveCore'][_0x442592(0x36d)]=Game_CharacterBase[_0x442592(0x34e)][_0x442592(0x16f)],Game_CharacterBase[_0x442592(0x34e)][_0x442592(0x16f)]=function(){const _0x3d3508=_0x442592;if(this===$gamePlayer){if($gameMap['isPlayerForceShown']())return![];if($gameMap['isPlayerForceHidden']())return!![];}return VisuMZ[_0x3d3508(0x3dc)][_0x3d3508(0x36d)][_0x3d3508(0x98)](this);},Game_Map['prototype'][_0x442592(0x377)]=function(){const _0x5e609d=_0x442592;this[_0x5e609d(0xdf)]=![],this['_forceHideFollower']=![];if(!$dataMap)return;const _0xa39020=$dataMap[_0x5e609d(0x2fc)]||'';if(_0xa39020[_0x5e609d(0x110)](/<HIDE FOLLOWERS>/i))this[_0x5e609d(0xdf)]=![],this['_forceHideFollower']=!![];else _0xa39020['match'](/<SHOW FOLLOWERS>/i)&&(this[_0x5e609d(0xdf)]=!![],this[_0x5e609d(0x45f)]=![]);},Game_Map[_0x442592(0x34e)][_0x442592(0xb7)]=function(){const _0x2acdb9=_0x442592;return this[_0x2acdb9(0xdf)]===undefined&&this[_0x2acdb9(0x377)](),this[_0x2acdb9(0xdf)];},Game_Map[_0x442592(0x34e)][_0x442592(0xaf)]=function(){const _0x5134ab=_0x442592;return this['_forceHideFollower']===undefined&&this[_0x5134ab(0x377)](),this['_forceHideFollower'];},VisuMZ[_0x442592(0x3dc)]['Game_Followers_isVisible']=Game_Followers['prototype'][_0x442592(0x3d8)],Game_Followers[_0x442592(0x34e)][_0x442592(0x3d8)]=function(){const _0x311639=_0x442592;if($gameMap['areFollowersForceShown']())return!![];if($gameMap[_0x311639(0xaf)]())return![];return VisuMZ[_0x311639(0x3dc)][_0x311639(0x482)][_0x311639(0x98)](this);},Game_Map['prototype'][_0x442592(0x499)]=function(){const _0x40685e=_0x442592,_0x124d58=this['events'](),_0x3dbf63=[];$gameParty[_0x40685e(0x9b)]=!![];for(const _0x2cd077 of _0x124d58){if(!_0x2cd077)continue;if(_0x2cd077[_0x40685e(0x427)])continue;_0x2cd077[_0x40685e(0x137)]()&&_0x3dbf63[_0x40685e(0x314)](_0x2cd077);}$gameParty[_0x40685e(0x9b)]=undefined;for(const _0x43963c of _0x3dbf63){if(!_0x43963c)continue;if(_0x43963c[_0x40685e(0x427)])continue;this['eraseEvent'](_0x43963c['eventId']());}},Game_Event[_0x442592(0x34e)]['processEraseEncounterSpawn']=function(){const _0x2659e8=_0x442592,_0x12b15b=this['event']()['note']||'';if(_0x12b15b[_0x2659e8(0x110)](/<ERASE IF ENC(?:|OUNTER) HALF>/i)){if($gameParty['hasEncounterHalf']())return!![];if($isTileEncounterHalf(this['x'],this['y']))return!![];}if(_0x12b15b[_0x2659e8(0x110)](/<ERASE IF ENC(?:|OUNTER) NONE>/i)){if($gameParty['hasEncounterNone']())return!![];if($isTileEncounterNone(this['x'],this['y']))return!![];}return![];},VisuMZ[_0x442592(0x3dc)][_0x442592(0x521)]=Scene_Map[_0x442592(0x34e)][_0x442592(0x509)],Scene_Map['prototype'][_0x442592(0x509)]=function(){const _0x59cded=_0x442592;VisuMZ[_0x59cded(0x3dc)][_0x59cded(0x521)]['call'](this),$gameMap['processEraseEncounterEvents']();},Game_Map[_0x442592(0x34e)][_0x442592(0x42b)]=function(){const _0x9a4468=_0x442592;if(!$dataMap)return;if(!$dataMap['note'])return;const _0x4492ef=$dataMap[_0x9a4468(0x2fc)];if(_0x4492ef['match'](/<MAP LOAD COMMON EVENT(?:|S):[ ](.*)>/i)){const _0x5ab4d3=String(RegExp['$1'])['split'](',')[_0x9a4468(0x510)](_0x5684bd=>Number(_0x5684bd));for(const _0x52accc of _0x5ab4d3){$gameTemp['reserveCommonEvent'](_0x52accc);}}},Game_CommonEvent['prototype']['hasAdvancedSwitchVariable']=function(){const _0x315fac=_0x442592,_0x40271f=this['event']();return this[_0x315fac(0x10c)]()&&_0x40271f['trigger']>=0x1&&DataManager['isAdvancedSwitch'](_0x40271f['switchId']);},Game_CommonEvent['prototype'][_0x442592(0x22f)]=function(){const _0x587ee9=_0x442592;return VisuMZ[_0x587ee9(0x3dc)][_0x587ee9(0x1d4)][_0x587ee9(0x383)]['includes'](this[_0x587ee9(0x511)]);},VisuMZ['EventsMoveCore']['Game_CommonEvent_isActive']=Game_CommonEvent[_0x442592(0x34e)][_0x442592(0x10c)],Game_CommonEvent[_0x442592(0x34e)]['isActive']=function(){const _0x42c2e2=_0x442592;if(VisuMZ[_0x42c2e2(0x3dc)][_0x42c2e2(0x4fb)][_0x42c2e2(0x98)](this))return!![];else{const _0x316c94=this['event']();return VisuMZ[_0x42c2e2(0x3dc)][_0x42c2e2(0x1d4)][_0x42c2e2(0xc9)](this[_0x42c2e2(0x2f2)]()['CPC'],this['_commonEventId'],_0x316c94);}},VisuMZ[_0x442592(0x3dc)][_0x442592(0x4fc)]=Game_Map[_0x442592(0x34e)][_0x442592(0x157)],Game_Map[_0x442592(0x34e)][_0x442592(0x157)]=function(){const _0x27ae1e=_0x442592,_0x46a0c0=VisuMZ['EventsMoveCore'][_0x27ae1e(0x4fc)][_0x27ae1e(0x98)](this),_0x1afe5c=VisuMZ[_0x27ae1e(0x3dc)]['CustomPageConditions'][_0x27ae1e(0x383)][_0x27ae1e(0x510)](_0x253f22=>$dataCommonEvents[_0x253f22]);return _0x46a0c0['concat'](_0x1afe5c)[_0x27ae1e(0xda)]((_0x370bc3,_0x4a4cca,_0x16b342)=>_0x16b342[_0x27ae1e(0x269)](_0x370bc3)===_0x4a4cca);},Game_CharacterBase[_0x442592(0xc0)]=VisuMZ[_0x442592(0x3dc)]['Settings'][_0x442592(0x41e)][_0x442592(0x7b)]??![],VisuMZ[_0x442592(0x3dc)][_0x442592(0x4a8)]=Game_CharacterBase[_0x442592(0x34e)]['initMembers'],Game_CharacterBase[_0x442592(0x34e)][_0x442592(0x324)]=function(){const _0x35e3e9=_0x442592;VisuMZ['EventsMoveCore']['Game_CharacterBase_initMembers']['call'](this),this[_0x35e3e9(0x156)]();},Game_CharacterBase[_0x442592(0x34e)][_0x442592(0x156)]=function(){const _0x5779c6=_0x442592;this[_0x5779c6(0x1ba)]=0x1,this[_0x5779c6(0x348)]=0x1,this['_patternLocked']=![],this[_0x5779c6(0x2b3)](),this[_0x5779c6(0x126)](),this[_0x5779c6(0x3a9)](),this[_0x5779c6(0xc1)]();},VisuMZ[_0x442592(0x3dc)][_0x442592(0x175)]=Game_CharacterBase[_0x442592(0x34e)][_0x442592(0x253)],Game_CharacterBase[_0x442592(0x34e)][_0x442592(0x253)]=function(){const _0xe926b9=_0x442592;let _0x5d15d4=VisuMZ[_0xe926b9(0x3dc)][_0xe926b9(0x175)][_0xe926b9(0x98)](this);return _0x5d15d4=this[_0xe926b9(0x13c)](_0x5d15d4),_0x5d15d4;},Game_CharacterBase['prototype'][_0x442592(0x13c)]=function(_0xc54e65){return _0xc54e65;},Game_CharacterBase['prototype'][_0x442592(0x1fe)]=function(){const _0x26aab6=_0x442592;if(this[_0x26aab6(0x2a9)]===Game_Player&&this['isInVehicle']())return this[_0x26aab6(0xe9)]()[_0x26aab6(0x3bd)]()[_0x26aab6(0x110)](/\[VS8\]/i);else return Imported[_0x26aab6(0x3d3)]&&this[_0x26aab6(0x365)]()?!![]:this[_0x26aab6(0x3bd)]()[_0x26aab6(0x110)](/\[VS8\]/i);},VisuMZ[_0x442592(0x3dc)][_0x442592(0x490)]=Game_CharacterBase[_0x442592(0x34e)]['direction'],Game_CharacterBase['prototype'][_0x442592(0x496)]=function(){const _0x15fd3c=_0x442592;if(!$dataMap)return this['_direction']||0x2;if(this['isOnLadder']()&&!this['isJumping']()&&this['isSpriteVS8dir']())return this[_0x15fd3c(0x3ba)]();else{if(this['isOnLadder']()&&!this[_0x15fd3c(0x373)]())return 0x8;else return this[_0x15fd3c(0x2d9)]()&&this[_0x15fd3c(0x1fe)]()?this[_0x15fd3c(0x24f)]():VisuMZ[_0x15fd3c(0x3dc)][_0x15fd3c(0x490)][_0x15fd3c(0x98)](this);}},VisuMZ[_0x442592(0x3dc)]['Game_CharacterBase_setDirection']=Game_CharacterBase[_0x442592(0x34e)]['setDirection'],Game_CharacterBase[_0x442592(0x34e)][_0x442592(0x421)]=function(_0x15383a){const _0x2a9a43=_0x442592;if(!this[_0x2a9a43(0x1fe)]())_0x15383a=this[_0x2a9a43(0x9d)](_0x15383a);VisuMZ[_0x2a9a43(0x3dc)][_0x2a9a43(0x245)]['call'](this,_0x15383a),this[_0x2a9a43(0x507)]();},Game_CharacterBase[_0x442592(0x34e)][_0x442592(0x9d)]=function(_0x2e976e){const _0x4e400f=_0x442592;if(_0x2e976e===0x1)return this['canPass'](this['_x'],this['_y'],0x4)?0x4:0x2;if(_0x2e976e===0x3)return this[_0x4e400f(0xe1)](this['_x'],this['_y'],0x6)?0x6:0x2;if(_0x2e976e===0x7)return this[_0x4e400f(0xe1)](this['_x'],this['_y'],0x4)?0x4:0x8;if(_0x2e976e===0x9)return this[_0x4e400f(0xe1)](this['_x'],this['_y'],0x6)?0x6:0x8;return _0x2e976e;},Game_CharacterBase[_0x442592(0x34e)][_0x442592(0x504)]=function(_0x27de9c){const _0x3bac98=_0x442592;return[0x1,0x3,0x5,0x7,0x9][_0x3bac98(0xb5)](_0x27de9c);},Game_CharacterBase['prototype']['lastMovedDirection']=function(){return this['_lastMovedDirection']||0x0;},VisuMZ['EventsMoveCore'][_0x442592(0x279)]=Game_CharacterBase[_0x442592(0x34e)][_0x442592(0x219)],Game_CharacterBase[_0x442592(0x34e)][_0x442592(0x219)]=function(_0x2caa83){const _0x40a2c1=_0x442592;this[_0x40a2c1(0x4a5)]=_0x2caa83,VisuMZ[_0x40a2c1(0x3dc)][_0x40a2c1(0x279)]['call'](this,_0x2caa83);},Game_CharacterBase[_0x442592(0x34e)][_0x442592(0x8c)]=function(_0x55c43b){const _0x3a032b=_0x442592;if(!this[_0x3a032b(0x504)](_0x55c43b))return this['moveStraight'](_0x55c43b);let _0x186635=0x0,_0x372f6d=0x0;switch(_0x55c43b){case 0x1:_0x186635=0x4,_0x372f6d=0x2;break;case 0x3:_0x186635=0x6,_0x372f6d=0x2;break;case 0x7:_0x186635=0x4,_0x372f6d=0x8;break;case 0x9:_0x186635=0x6,_0x372f6d=0x8;break;}if(VisuMZ[_0x3a032b(0x3dc)]['Settings'][_0x3a032b(0x41e)][_0x3a032b(0x18a)]){if(!this[_0x3a032b(0xe1)](this['_x'],this['_y'],_0x186635))return this[_0x3a032b(0x219)](_0x372f6d);if(!this['canPass'](this['_x'],this['_y'],_0x372f6d))return this['moveStraight'](_0x186635);if(!this[_0x3a032b(0x2d1)](this['_x'],this['_y'],_0x186635,_0x372f6d)){let _0x87bcd2=VisuMZ[_0x3a032b(0x3dc)][_0x3a032b(0x240)]['Movement'][_0x3a032b(0x52d)]?_0x186635:_0x372f6d;return this['moveStraight'](_0x87bcd2);}}this[_0x3a032b(0x4a5)]=_0x55c43b,this['moveDiagonally'](_0x186635,_0x372f6d);},VisuMZ[_0x442592(0x3dc)][_0x442592(0x359)]=Game_CharacterBase[_0x442592(0x34e)]['realMoveSpeed'],Game_CharacterBase[_0x442592(0x34e)][_0x442592(0x2dc)]=function(){const _0x171dfb=_0x442592;let _0x296830=this['_moveSpeed'];return this[_0x171dfb(0x23f)]()&&(_0x296830+=this['dashSpeedModifier']()),this[_0x171dfb(0xa2)](_0x296830);},Game_CharacterBase[_0x442592(0x34e)]['dashSpeedModifier']=function(){const _0x5d7693=_0x442592,_0x2302bd=VisuMZ[_0x5d7693(0x3dc)]['Settings'][_0x5d7693(0x41e)];return _0x2302bd[_0x5d7693(0x41b)]!==undefined?_0x2302bd[_0x5d7693(0x41b)]:VisuMZ['EventsMoveCore']['Game_CharacterBase_realMoveSpeed'][_0x5d7693(0x98)](this)-this[_0x5d7693(0x4dd)];},Game_CharacterBase[_0x442592(0x34e)][_0x442592(0xa2)]=function(_0x5b454c){const _0x2bbfd8=_0x442592,_0x27e0f9=VisuMZ['EventsMoveCore'][_0x2bbfd8(0x240)][_0x2bbfd8(0x41e)];if(!_0x27e0f9[_0x2bbfd8(0x4c7)])return _0x5b454c;return[0x1,0x3,0x7,0x9]['includes'](this['_lastMovedDirection'])&&(_0x5b454c*=_0x27e0f9['DiagonalSpeedMultiplier']||0.01),_0x5b454c;},VisuMZ[_0x442592(0x3dc)][_0x442592(0x45d)]=Game_CharacterBase[_0x442592(0x34e)][_0x442592(0x23f)],Game_CharacterBase[_0x442592(0x34e)][_0x442592(0x23f)]=function(){const _0x2a7f3f=_0x442592;if(!Game_CharacterBase[_0x2a7f3f(0xc0)]&&this['isOnLadder']())return![];if(this['_forceDashing'])return!![];return VisuMZ[_0x2a7f3f(0x3dc)][_0x2a7f3f(0x45d)]['call'](this);},Game_CharacterBase[_0x442592(0x34e)]['isDashingAndMoving']=function(){const _0x1f0fbc=_0x442592;return this['isDashing']()&&this[_0x1f0fbc(0x4c8)]===0x0;},VisuMZ[_0x442592(0x3dc)]['Game_CharacterBase_pattern']=Game_CharacterBase[_0x442592(0x34e)][_0x442592(0x107)],Game_CharacterBase['prototype'][_0x442592(0x107)]=function(){const _0x4728aa=_0x442592;return this['isPosing']()?this['getPosingCharacterPattern']():VisuMZ[_0x4728aa(0x3dc)]['Game_CharacterBase_pattern'][_0x4728aa(0x98)](this);},VisuMZ[_0x442592(0x3dc)][_0x442592(0x436)]=Game_CharacterBase[_0x442592(0x34e)][_0x442592(0x337)],Game_CharacterBase[_0x442592(0x34e)][_0x442592(0x337)]=function(){const _0x272293=_0x442592;VisuMZ['EventsMoveCore'][_0x272293(0x436)][_0x272293(0x98)](this),this[_0x272293(0x2b3)]();},VisuMZ['EventsMoveCore'][_0x442592(0x1d6)]=Game_CharacterBase['prototype'][_0x442592(0xbf)],Game_CharacterBase[_0x442592(0x34e)]['characterIndex']=function(){const _0x1a17c=_0x442592;if(this['isSpriteVS8dir']())return this['characterIndexVS8']();return VisuMZ[_0x1a17c(0x3dc)][_0x1a17c(0x1d6)][_0x1a17c(0x98)](this);},Game_CharacterBase[_0x442592(0x34e)]['characterIndexVS8']=function(){const _0x41af3f=_0x442592,_0x21edf8=this[_0x41af3f(0x496)]();if(this[_0x41af3f(0x373)]()){if([0x2,0x4,0x6,0x8][_0x41af3f(0xb5)](_0x21edf8))return 0x4;if([0x1,0x3,0x7,0x9][_0x41af3f(0xb5)](_0x21edf8))return 0x5;}else{if(this[_0x41af3f(0x9c)]())return 0x6;else{if(this[_0x41af3f(0x2d9)]())return this[_0x41af3f(0x419)]();else{if(this['_forceCarrying']){if([0x2,0x4,0x6,0x8][_0x41af3f(0xb5)](_0x21edf8))return 0x4;if([0x1,0x3,0x7,0x9][_0x41af3f(0xb5)](_0x21edf8))return 0x5;}else{if(this['hasEventIcon']()&&this[_0x41af3f(0x394)]()){if([0x2,0x4,0x6,0x8][_0x41af3f(0xb5)](_0x21edf8))return 0x4;if([0x1,0x3,0x7,0x9]['includes'](_0x21edf8))return 0x5;}else{if(this[_0x41af3f(0x42d)]()){if([0x2,0x4,0x6,0x8][_0x41af3f(0xb5)](_0x21edf8))return 0x2;if([0x1,0x3,0x7,0x9][_0x41af3f(0xb5)](_0x21edf8))return 0x3;}else{if([0x2,0x4,0x6,0x8][_0x41af3f(0xb5)](_0x21edf8))return 0x0;if([0x1,0x3,0x7,0x9][_0x41af3f(0xb5)](_0x21edf8))return 0x1;}}}}}}},Game_CharacterBase[_0x442592(0x34e)]['useCarryPoseForIcons']=function(){const _0x4c66a0=_0x442592;return VisuMZ['EventsMoveCore']['Settings'][_0x4c66a0(0x11e)][_0x4c66a0(0x442)];},Game_CharacterBase[_0x442592(0x34e)][_0x442592(0x345)]=function(){const _0x235daf=_0x442592;return this['isOnLadder']()&&this[_0x235daf(0xc7)]()===VisuMZ[_0x235daf(0x3dc)]['Settings']['TerrainTag'][_0x235daf(0x2cc)];},Game_CharacterBase['prototype']['directionOnLadderSpriteVS8dir']=function(){const _0x451202=_0x442592;return this[_0x451202(0x345)]()?0x4:0x2;},VisuMZ[_0x442592(0x3dc)]['Game_CharacterBase_update']=Game_CharacterBase[_0x442592(0x34e)][_0x442592(0x441)],Game_CharacterBase[_0x442592(0x34e)][_0x442592(0x441)]=function(){const _0x590104=_0x442592;this[_0x590104(0x4b0)](),VisuMZ[_0x590104(0x3dc)][_0x590104(0x40a)][_0x590104(0x98)](this),this[_0x590104(0x34b)]();},Game_CharacterBase['prototype'][_0x442592(0x4b0)]=function(){const _0x55a1c4=_0x442592;this[_0x55a1c4(0x29d)]=this['_scaleBaseX']??0x1,this[_0x55a1c4(0x4e1)]=this[_0x55a1c4(0x348)]??0x1;},VisuMZ['EventsMoveCore']['Game_CharacterBase_bushDepth']=Game_CharacterBase[_0x442592(0x34e)][_0x442592(0x1ca)],Game_CharacterBase[_0x442592(0x34e)][_0x442592(0x1ca)]=function(){const _0x20a6c0=_0x442592;let _0x4626bf=VisuMZ['EventsMoveCore']['Game_CharacterBase_bushDepth'][_0x20a6c0(0x98)](this);return this[_0x20a6c0(0x4e1)]!==undefined&&(_0x4626bf/=Math[_0x20a6c0(0xcc)](this[_0x20a6c0(0x4e1)],0.00001)),Math[_0x20a6c0(0x446)](_0x4626bf);},Game_CharacterBase[_0x442592(0x34e)][_0x442592(0x34b)]=function(){const _0xf54342=_0x442592;this['_poseDuration']=this[_0xf54342(0xd0)]||0x0;if(this[_0xf54342(0xd0)]>0x0){this[_0xf54342(0xd0)]--;if(this[_0xf54342(0xd0)]<=0x0&&this[_0xf54342(0x134)]!==_0xf54342(0x2b2))this[_0xf54342(0x2b3)]();}},VisuMZ['EventsMoveCore']['Game_CharacterBase_moveDiagonally']=Game_CharacterBase['prototype']['moveDiagonally'],Game_CharacterBase['prototype'][_0x442592(0x37d)]=function(_0x5a7ce3,_0x4f775e){const _0x4defe5=_0x442592;VisuMZ[_0x4defe5(0x3dc)]['Game_CharacterBase_moveDiagonally'][_0x4defe5(0x98)](this,_0x5a7ce3,_0x4f775e);if(this[_0x4defe5(0x1fe)]())this[_0x4defe5(0x3d0)](_0x5a7ce3,_0x4f775e);},Game_CharacterBase[_0x442592(0x34e)]['setDiagonalDirection']=function(_0x13c903,_0x54f672){const _0x66344f=_0x442592;if(_0x13c903===0x4&&_0x54f672===0x2)this['setDirection'](0x1);if(_0x13c903===0x6&&_0x54f672===0x2)this[_0x66344f(0x421)](0x3);if(_0x13c903===0x4&&_0x54f672===0x8)this[_0x66344f(0x421)](0x7);if(_0x13c903===0x6&&_0x54f672===0x8)this[_0x66344f(0x421)](0x9);},VisuMZ[_0x442592(0x3dc)][_0x442592(0x440)]=Game_CharacterBase[_0x442592(0x34e)][_0x442592(0x37e)],Game_CharacterBase[_0x442592(0x34e)][_0x442592(0x37e)]=function(){const _0x2552b9=_0x442592;if(this['isPosing']()&&this[_0x2552b9(0x19e)]()===_0x2552b9(0x2b2))return!![];return VisuMZ['EventsMoveCore'][_0x2552b9(0x440)][_0x2552b9(0x98)](this);},Game_CharacterBase[_0x442592(0x34e)]['setPose']=function(_0x2c0c46,_0x47675f){const _0x3185a2=_0x442592;if(_0x2c0c46[_0x3185a2(0x110)](/Z/i))_0x2c0c46=_0x3185a2(0x2b2);if(_0x2c0c46['match'](/SLEEP/i))_0x2c0c46='ZZZ';this[_0x3185a2(0x1fe)]()&&(this['_pose']=_0x2c0c46[_0x3185a2(0x22e)]()[_0x3185a2(0x36f)](),this[_0x3185a2(0xd0)]=_0x47675f||Infinity);},Game_CharacterBase[_0x442592(0x34e)][_0x442592(0x19e)]=function(){const _0x8ccf95=_0x442592;return this['isSpriteVS8dir']()?(this[_0x8ccf95(0x134)]||'')['toUpperCase']()[_0x8ccf95(0x36f)]():''[_0x8ccf95(0x22e)]()[_0x8ccf95(0x36f)]();},Game_CharacterBase['prototype'][_0x442592(0xdb)]=function(_0x52cf76,_0xe81f01){const _0x10661a=_0x442592;if(this[_0x10661a(0x1fe)]()){const _0x492a5e=['','EXCLAMATION',_0x10661a(0x4d6),'MUSIC\x20NOTE',_0x10661a(0x4b3),_0x10661a(0x194),'SWEAT',_0x10661a(0x35e),_0x10661a(0x433),'LIGHT\x20BULB',_0x10661a(0x2b2),'','','','',''][_0x52cf76];this[_0x10661a(0x3e2)](_0x492a5e,_0xe81f01);}},Game_CharacterBase['prototype'][_0x442592(0x2b3)]=function(){const _0x2b5ae0=_0x442592;this[_0x2b5ae0(0x134)]='',this[_0x2b5ae0(0xd0)]=0x0;},Game_CharacterBase[_0x442592(0x34e)][_0x442592(0x2d9)]=function(){const _0x3b31b5=_0x442592;return this[_0x3b31b5(0x1fe)]()&&!!this[_0x3b31b5(0x134)];},Game_CharacterBase[_0x442592(0x34e)][_0x442592(0x419)]=function(){const _0x35e6a3=_0x442592,_0x28cf2b=this['_pose'][_0x35e6a3(0x22e)]();switch(this[_0x35e6a3(0x134)]['toUpperCase']()[_0x35e6a3(0x36f)]()){case _0x35e6a3(0x263):case'HMPH':case _0x35e6a3(0x24a):case _0x35e6a3(0x1b4):case _0x35e6a3(0x29a):case'COLLAPSE':return 0x6;break;default:return 0x7;break;}},Game_CharacterBase[_0x442592(0x34e)][_0x442592(0x24f)]=function(){const _0x11beff=_0x442592;switch(this[_0x11beff(0x134)]['toUpperCase']()){case _0x11beff(0x45e):case'QUESTION':case _0x11beff(0xfa):case'!':case'?':return 0x2;break;case'HEART':case _0x11beff(0x194):case _0x11beff(0x38e):return 0x4;break;case _0x11beff(0x263):case _0x11beff(0x10a):case _0x11beff(0x24a):case _0x11beff(0x35e):case _0x11beff(0x433):case _0x11beff(0x2e7):return 0x6;break;case _0x11beff(0x1b4):case _0x11beff(0x29a):case _0x11beff(0x92):case _0x11beff(0x2b2):case _0x11beff(0x2f4):return 0x8;break;default:return VisuMZ['EventsMoveCore']['Game_CharacterBase_setDirection']['call'](this);break;}},Game_CharacterBase[_0x442592(0x34e)][_0x442592(0x4bc)]=function(){const _0x1cf839=_0x442592;switch(this[_0x1cf839(0x134)]['toUpperCase']()){case _0x1cf839(0x263):case'HURT':case'EXCLAMATION':case'!':case _0x1cf839(0x4b3):case _0x1cf839(0x35e):return 0x0;break;case _0x1cf839(0x10a):case _0x1cf839(0x29a):case'QUESTION':case'?':case _0x1cf839(0x194):case _0x1cf839(0x433):return 0x1;break;case _0x1cf839(0x24a):case _0x1cf839(0x92):case'MUSIC\x20NOTE':case'SWEAT':case'LIGHT\x20BULB':return 0x2;break;default:return VisuMZ[_0x1cf839(0x3dc)]['Game_CharacterBase_pattern'][_0x1cf839(0x98)](this);break;}},Game_CharacterBase[_0x442592(0x34e)][_0x442592(0xa1)]=function(){this['_forceCarrying']=!![];},Game_CharacterBase[_0x442592(0x34e)][_0x442592(0x16a)]=function(){const _0x1146b3=_0x442592;this[_0x1146b3(0x3fc)]=![];},Game_CharacterBase[_0x442592(0x34e)][_0x442592(0x285)]=function(){const _0x2b7da5=_0x442592;this[_0x2b7da5(0x233)]=!![];},Game_CharacterBase['prototype'][_0x442592(0x126)]=function(){const _0x4470fd=_0x442592;this[_0x4470fd(0x233)]=![];},Game_CharacterBase['prototype'][_0x442592(0x478)]=function(){const _0x5869e4=_0x442592;if(this['isTile']())return![];if(this['_isObjectCharacter'])return![];if(this[_0x5869e4(0x4b2)]==='')return![];if(this[_0x5869e4(0x2a9)]===Game_Vehicle)return![];if(this[_0x5869e4(0x16f)]())return![];if(this[_0x5869e4(0x517)])return![];return!![];},Game_Follower[_0x442592(0x34e)]['isShadowVisible']=function(){const _0x30d56a=_0x442592;if($gamePlayer[_0x30d56a(0x4d3)])return![];return Game_CharacterBase[_0x30d56a(0x34e)][_0x30d56a(0x478)][_0x30d56a(0x98)](this);},Game_CharacterBase[_0x442592(0x34e)]['isShadowShrink']=function(){const _0x4bc624=_0x442592;if(this[_0x4bc624(0x9c)]())return!![];if(this[_0x4bc624(0x2a9)]===Game_Player&&this['isInVehicle']())return!![];return![];},Game_CharacterBase['prototype'][_0x442592(0x1be)]=function(){const _0x16b8db=_0x442592;return VisuMZ['EventsMoveCore'][_0x16b8db(0x240)][_0x16b8db(0x41e)][_0x16b8db(0x150)];},Game_CharacterBase[_0x442592(0x34e)][_0x442592(0x15a)]=function(){const _0x103514=_0x442592;return this[_0x103514(0x223)]();},Game_CharacterBase[_0x442592(0x34e)]['shadowY']=function(){const _0xc85545=_0x442592,_0x21c3a8=$gameMap['tileHeight']();return Math[_0xc85545(0x446)](this[_0xc85545(0x328)]()*_0x21c3a8+_0x21c3a8);},Game_CharacterBase[_0x442592(0x3f7)]=0x64,Game_CharacterBase[_0x442592(0x34e)][_0x442592(0x428)]=function(_0x289eac,_0xb29134){const _0x4bdc92=_0x442592;if(TouchInput[_0x4bdc92(0x243)]())return![];if(!$gameMap[_0x4bdc92(0x397)]())return![];if($gameMap['eventsXyNt'](_0x289eac,_0xb29134)['length']>0x0)return![];if(!$gameMap['isPassableByAnyDirection'](_0x289eac,_0xb29134))return![];const _0x1fbc09=$gameMap[_0x4bdc92(0x46f)][_0x4bdc92(0x2fe)];if(_0x1fbc09>=Game_CharacterBase['DIAGONAL_PATHFINDING_EVENT_LIMIT'])return![];return!![];},Game_Character[_0x442592(0x34e)][_0x442592(0x371)]=function(_0x2c0ae4,_0x3c7070){const _0x121cda=_0x442592;let _0x209df7=this[_0x121cda(0x1e3)](_0x2c0ae4,_0x3c7070);if(!this['getDiagonalDestination'](_0x2c0ae4,_0x3c7070))return _0x209df7;if(this[_0x121cda(0x1b5)](_0x2c0ae4,_0x3c7070))return _0x209df7;const _0x23a86d=_0x209df7;if(_0x209df7===0x2){if(_0x2c0ae4>this['x']&&this[_0x121cda(0xe1)](this['x'],this['y'],0x6))_0x209df7=0x3;if(_0x2c0ae4<this['x']&&this[_0x121cda(0xe1)](this['x'],this['y'],0x4))_0x209df7=0x1;}else{if(_0x209df7===0x4){if(_0x3c7070>this['y']&&this[_0x121cda(0xe1)](this['x'],this['y'],0x4))_0x209df7=0x1;if(_0x3c7070<this['y']&&this[_0x121cda(0xe1)](this['x'],this['y'],0x6))_0x209df7=0x7;}else{if(_0x209df7===0x6){if(_0x3c7070>this['y']&&this[_0x121cda(0xe1)](this['x'],this['y'],0x4))_0x209df7=0x3;if(_0x3c7070<this['y']&&this[_0x121cda(0xe1)](this['x'],this['y'],0x6))_0x209df7=0x9;}else{if(_0x209df7===0x8){if(_0x2c0ae4>this['x']&&this[_0x121cda(0xe1)](this['x'],this['y'],0x6))_0x209df7=0x9;if(_0x2c0ae4<this['x']&&this['canPass'](this['x'],this['y'],0x4))_0x209df7=0x7;}}}}if(!this[_0x121cda(0xe1)](this['x'],this['y'],_0x209df7))return _0x23a86d;const _0x60302a=$gameMap[_0x121cda(0xd4)](this['x'],_0x209df7),_0x3e111f=$gameMap[_0x121cda(0x114)](this['y'],_0x209df7);if(this['isCollidedWithEvents'](_0x60302a,_0x3e111f))_0x209df7=_0x23a86d;return _0x209df7;},VisuMZ[_0x442592(0x3dc)][_0x442592(0x2c7)]=Game_CharacterBase[_0x442592(0x34e)]['canPass'],Game_CharacterBase['prototype'][_0x442592(0xe1)]=function(_0x181578,_0x2dce43,_0x548978){const _0x3c5e1e=_0x442592;return this['_vehicleType']===_0x3c5e1e(0x4e9)?this[_0x3c5e1e(0xe9)]()[_0x3c5e1e(0x4c6)](_0x181578,_0x2dce43,_0x548978):VisuMZ[_0x3c5e1e(0x3dc)][_0x3c5e1e(0x2c7)][_0x3c5e1e(0x98)](this,_0x181578,_0x2dce43,_0x548978);},Game_CharacterBase['prototype']['clearSpriteOffsets']=function(){const _0x3f798a=_0x442592;this[_0x3f798a(0xf4)]=0x0,this[_0x3f798a(0x515)]=0x0;},VisuMZ[_0x442592(0x3dc)]['Game_CharacterBase_screenX']=Game_CharacterBase['prototype']['screenX'],Game_CharacterBase['prototype'][_0x442592(0x223)]=function(){const _0x55e29e=_0x442592;return VisuMZ[_0x55e29e(0x3dc)][_0x55e29e(0x457)][_0x55e29e(0x98)](this)+(this[_0x55e29e(0xf4)]||0x0);},VisuMZ[_0x442592(0x3dc)]['Game_CharacterBase_screenY']=Game_CharacterBase['prototype']['screenY'],Game_CharacterBase[_0x442592(0x34e)][_0x442592(0x500)]=function(){const _0x1b20d0=_0x442592;return VisuMZ['EventsMoveCore'][_0x1b20d0(0x22b)][_0x1b20d0(0x98)](this)+(this[_0x1b20d0(0x515)]||0x0);},Game_CharacterBase[_0x442592(0x4d1)]=VisuMZ['EventsMoveCore'][_0x442592(0x240)]['Movement'][_0x442592(0x2fb)]??-0x6,Game_CharacterBase[_0x442592(0x34e)][_0x442592(0x95)]=function(){const _0x3255a8=_0x442592;let _0x1ba75a=this[_0x3255a8(0x513)]()?0x0:-Game_CharacterBase[_0x3255a8(0x4d1)];return this['_scaleY']&&(_0x1ba75a*=this[_0x3255a8(0x4e1)]),Math[_0x3255a8(0x1e4)](_0x1ba75a);},Game_CharacterBase['prototype']['clearStepPattern']=function(){const _0x41b3f9=_0x442592;this[_0x41b3f9(0x466)]='';},VisuMZ[_0x442592(0x3dc)][_0x442592(0x439)]=Game_CharacterBase[_0x442592(0x34e)][_0x442592(0x48a)],Game_CharacterBase[_0x442592(0x34e)][_0x442592(0x48a)]=function(){const _0x45029f=_0x442592;if(this[_0x45029f(0xc5)])return;if(this[_0x45029f(0x4bb)]())return;VisuMZ[_0x45029f(0x3dc)][_0x45029f(0x439)][_0x45029f(0x98)](this);},Game_CharacterBase['prototype'][_0x442592(0x4bb)]=function(){const _0x560895=_0x442592;if(!this['hasStepAnime']()&&this['_stopCount']>0x0)return![];switch(String(this[_0x560895(0x466)])[_0x560895(0x22e)]()[_0x560895(0x36f)]()){case _0x560895(0x2e5):this['_pattern']+=0x1;if(this[_0x560895(0x483)]>0x2)this[_0x560895(0x1dd)](0x0);break;case _0x560895(0x202):this[_0x560895(0x483)]-=0x1;if(this[_0x560895(0x483)]<0x0)this['setPattern'](0x2);break;case _0x560895(0x306):case _0x560895(0x3fb):this['turnRight90']();break;case _0x560895(0x3ad):case _0x560895(0x206):case _0x560895(0x77):case _0x560895(0x3e4):this[_0x560895(0xb8)]();break;default:return![];}return!![];},Game_CharacterBase[_0x442592(0x34e)][_0x442592(0x45b)]=function(){return $gameSystem['getEventIconData'](this);},Game_CharacterBase[_0x442592(0x34e)]['hasEventIcon']=function(){const _0x43ec66=_0x442592,_0x5ecbb1=this[_0x43ec66(0x45b)]();if(!_0x5ecbb1)return![];return _0x5ecbb1[_0x43ec66(0xba)]>0x0;},Game_CharacterBase[_0x442592(0x34e)][_0x442592(0x17c)]=function(){const _0x7289b9=_0x442592,_0x25f8d9=this[_0x7289b9(0x496)]();return $gameMap[_0x7289b9(0xd4)](this['x'],_0x25f8d9);},Game_CharacterBase[_0x442592(0x34e)][_0x442592(0x6d)]=function(){const _0x181c69=_0x442592,_0x12fc9f=this[_0x181c69(0x496)]();return $gameMap[_0x181c69(0x114)](this['y'],_0x12fc9f);},Game_CharacterBase[_0x442592(0x34e)]['backX']=function(){const _0x82f269=_0x442592,_0xf850b=this[_0x82f269(0x44e)](this[_0x82f269(0x496)]());return $gameMap[_0x82f269(0xd4)](this['x'],_0xf850b);},Game_CharacterBase[_0x442592(0x34e)][_0x442592(0x141)]=function(){const _0x2008d7=_0x442592,_0x3c5f7f=this[_0x2008d7(0x44e)](this[_0x2008d7(0x496)]());return $gameMap[_0x2008d7(0x114)](this['y'],_0x3c5f7f);},Game_CharacterBase[_0x442592(0x34e)][_0x442592(0x38f)]=function(){const _0xde0077=_0x442592,_0x311393=[0x0,0x3,0x6,0x9,0x2,0x5,0x8,0x1,0x4,0x7][this['direction']()];return $gameMap[_0xde0077(0xd4)](this['x'],_0x311393);},Game_CharacterBase[_0x442592(0x34e)][_0x442592(0x2c9)]=function(){const _0x32333a=_0x442592,_0x401af6=[0x0,0x3,0x6,0x9,0x2,0x5,0x8,0x1,0x4,0x7][this[_0x32333a(0x496)]()];return $gameMap[_0x32333a(0x114)](this['y'],_0x401af6);},Game_CharacterBase[_0x442592(0x34e)]['cwX']=function(){const _0x541751=_0x442592,_0x36979f=[0x0,0x7,0x4,0x1,0x8,0x5,0x2,0x9,0x6,0x3][this[_0x541751(0x496)]()];return $gameMap['roundXWithDirection'](this['x'],_0x36979f);},Game_CharacterBase['prototype'][_0x442592(0xfb)]=function(){const _0x15503c=_0x442592,_0x46e798=[0x0,0x7,0x4,0x1,0x8,0x5,0x2,0x9,0x6,0x3][this['direction']()];return $gameMap[_0x15503c(0x114)](this['y'],_0x46e798);},VisuMZ[_0x442592(0x3dc)][_0x442592(0x48f)]=Game_Character[_0x442592(0x34e)][_0x442592(0x2a3)],Game_Character[_0x442592(0x34e)][_0x442592(0x2a3)]=function(_0x53c30b){const _0x362672=_0x442592;route=JsonEx['makeDeepCopy'](_0x53c30b),VisuMZ['EventsMoveCore'][_0x362672(0x48f)][_0x362672(0x98)](this,route);},VisuMZ['EventsMoveCore']['Game_Character_forceMoveRoute']=Game_Character[_0x442592(0x34e)][_0x442592(0x7d)],Game_Character[_0x442592(0x34e)][_0x442592(0x7d)]=function(_0x568ae5){const _0x3f1f2d=_0x442592;route=JsonEx[_0x3f1f2d(0x1e0)](_0x568ae5),VisuMZ[_0x3f1f2d(0x3dc)]['Game_Character_forceMoveRoute'][_0x3f1f2d(0x98)](this,route);},VisuMZ[_0x442592(0x3dc)][_0x442592(0x1ab)]=Game_Character['prototype'][_0x442592(0x413)],Game_Character['prototype'][_0x442592(0x413)]=function(_0x5d1894){const _0x4e1957=_0x442592,_0x3fb36d=Game_Character,_0x173cff=_0x5d1894['parameters'];if(_0x5d1894[_0x4e1957(0x308)]===_0x3fb36d['ROUTE_SCRIPT']){let _0x2369f5=_0x5d1894[_0x4e1957(0xdc)][0x0];_0x2369f5=this['convertVariableValuesInScriptCall'](_0x2369f5),_0x2369f5=this['convertSelfVariableValuesInScriptCall'](_0x2369f5),this[_0x4e1957(0x287)](_0x5d1894,_0x2369f5);}else VisuMZ['EventsMoveCore']['Game_Character_processMoveCommand'][_0x4e1957(0x98)](this,_0x5d1894);},Game_Character['prototype'][_0x442592(0x52f)]=function(_0x244457){const _0x4e646a=_0x442592,_0x205714=/\$gameVariables\.value\((\d+)\)/gi,_0x5c9325=/\\V\[(\d+)\]/gi;while(_0x244457[_0x4e646a(0x110)](_0x205714)){_0x244457=_0x244457[_0x4e646a(0x25c)](_0x205714,(_0x1a7fef,_0x57b25e)=>$gameVariables[_0x4e646a(0x161)](parseInt(_0x57b25e)));}while(_0x244457[_0x4e646a(0x110)](_0x5c9325)){_0x244457=_0x244457[_0x4e646a(0x25c)](_0x5c9325,(_0x239acf,_0x513f26)=>$gameVariables[_0x4e646a(0x161)](parseInt(_0x513f26)));}return _0x244457;},Game_Character[_0x442592(0x34e)][_0x442592(0x280)]=function(_0x3601e8){const _0x3895bf=_0x442592,_0x2418fd=/\\SELFVAR\[(\d+)\]/gi;while(_0x3601e8['match'](_0x2418fd)){_0x3601e8=_0x3601e8[_0x3895bf(0x25c)](_0x2418fd,(_0x5da6a5,_0x368ecc)=>getSelfVariableValue(this['_mapId'],this[_0x3895bf(0xa3)],parseInt(_0x368ecc)));}return _0x3601e8;},Game_Character[_0x442592(0x34e)]['processMoveCommandEventsMoveCore']=function(_0x2c9d5e,_0x34412e){const _0x661eac=_0x442592;if(_0x34412e[_0x661eac(0x110)](/ANIMATION:[ ](\d+)/i))return this['processMoveRouteAnimation'](Number(RegExp['$1']));if(_0x34412e[_0x661eac(0x110)](/BALLOON:[ ](.*)/i))return this['processMoveRouteBalloon'](String(RegExp['$1']));if(_0x34412e['match'](/FADE IN:[ ](\d+)/i))return this[_0x661eac(0xe6)](Number(RegExp['$1']));if(_0x34412e[_0x661eac(0x110)](/FADE OUT:[ ](\d+)/i))return this[_0x661eac(0xfe)](Number(RegExp['$1']));if(_0x34412e['match'](/FORCE (?:CARRY|CARRYING|HOLD|HOLDING):[ ](?:TRUE|ON)/i))return this['forceCarrying']();if(_0x34412e[_0x661eac(0x110)](/FORCE (?:CARRY|CARRYING|HOLD|HOLDING):[ ](?:FALSE|OFF)/i))return this[_0x661eac(0x16a)]();if(_0x34412e[_0x661eac(0x110)](/FORCE (?:DASH|DASHING|RUN|RUNNING):[ ](?:TRUE|ON)/i))return this[_0x661eac(0x285)]();if(_0x34412e[_0x661eac(0x110)](/FORCE (?:DASH|DASHING|RUN|RUNNING):[ ](?:FALSE|OFF)/i))return this[_0x661eac(0x126)]();if(_0x34412e['match'](/HUG:[ ]LEFT/i))return this[_0x661eac(0x522)](_0x661eac(0x1c4));if(_0x34412e[_0x661eac(0x110)](/HUG:[ ]RIGHT/i))return this[_0x661eac(0x522)]('right');if(_0x34412e[_0x661eac(0x110)](/INDEX:[ ](\d+)/i))return this[_0x661eac(0x497)](Number(RegExp['$1']));if(_0x34412e['match'](/INDEX:[ ]([\+\-]\d+)/i)){const _0x4b86f7=this[_0x661eac(0xc2)]+Number(RegExp['$1']);return this[_0x661eac(0x497)](_0x4b86f7);}if(_0x34412e[_0x661eac(0x110)](/JUMP FORWARD:[ ](\d+)/i))return this[_0x661eac(0x1c7)](Number(RegExp['$1']));if(_0x34412e[_0x661eac(0x110)](/JUMP TO:\s*(\d+)\s*[, ]\s*(\d+)/i))return this['processMoveRouteJumpTo'](Number(RegExp['$1']),Number(RegExp['$2']));if(_0x34412e['match'](/JUMP TO EVENT:[ ](\d+)/i)){const _0x1a7327=$gameMap['event'](Number(RegExp['$1']));return this[_0x661eac(0x4e8)](_0x1a7327);}if(_0x34412e[_0x661eac(0x110)](/JUMP TO PLAYER/i))return this[_0x661eac(0x4e8)]($gamePlayer);if(_0x34412e[_0x661eac(0x110)](/JUMP TO HOME/i)&&this[_0x661eac(0xb6)]){const _0x518419=this[_0x661eac(0x450)],_0x5cab40=this[_0x661eac(0x4f6)];return this[_0x661eac(0x276)](_0x518419,_0x5cab40);}if(_0x34412e[_0x661eac(0x110)](/MOVE[ ](.*)[ ]UNTIL STOP/i)){const _0x546e44=String(RegExp['$1']),_0x5a35f2=this[_0x661eac(0x3a8)](_0x34412e);return this[_0x661eac(0x415)](_0x546e44,_0x5a35f2);}if(_0x34412e['match'](/MOVE TO:\s*(\d+)\s*[, ]\s*(\d+)/i)){const _0x296957=Number(RegExp['$1']),_0xe29cc4=Number(RegExp['$2']),_0x232571=this['checkCollisionKeywords'](_0x34412e);return this[_0x661eac(0x1f0)](_0x296957,_0xe29cc4,_0x232571);}if(_0x34412e[_0x661eac(0x110)](/MOVE TO EVENT:[ ](\d+)/i)){const _0x1cf3c5=$gameMap[_0x661eac(0x2f2)](Number(RegExp['$1'])),_0x88d73e=this[_0x661eac(0x3a8)](_0x34412e);return this[_0x661eac(0x368)](_0x1cf3c5,_0x88d73e);}if(_0x34412e['match'](/MOVE TO PLAYER/i)){const _0x497d68=this[_0x661eac(0x3a8)](_0x34412e);return this[_0x661eac(0x368)]($gamePlayer,_0x497d68);}if(_0x34412e[_0x661eac(0x110)](/MOVE TO HOME/i)&&this[_0x661eac(0xb6)]){const _0x6a755b=this['_randomHomeX'],_0x3c74d1=this[_0x661eac(0x4f6)],_0x66d9f1=this['checkCollisionKeywords'](_0x34412e);return this['processMoveRouteMoveTo'](_0x6a755b,_0x3c74d1,_0x66d9f1);}if(_0x34412e[_0x661eac(0x110)](/MOVE LOWER LEFT:[ ](\d+)/i))return this['processMoveRouteMoveRepeat'](0x1,Number(RegExp['$1']));if(_0x34412e[_0x661eac(0x110)](/MOVE DOWN:[ ](\d+)/i))return this[_0x661eac(0x25d)](0x2,Number(RegExp['$1']));if(_0x34412e[_0x661eac(0x110)](/MOVE LOWER RIGHT:[ ](\d+)/i))return this[_0x661eac(0x25d)](0x3,Number(RegExp['$1']));if(_0x34412e[_0x661eac(0x110)](/MOVE LEFT:[ ](\d+)/i))return this[_0x661eac(0x25d)](0x4,Number(RegExp['$1']));if(_0x34412e['match'](/MOVE RIGHT:[ ](\d+)/i))return this[_0x661eac(0x25d)](0x6,Number(RegExp['$1']));if(_0x34412e[_0x661eac(0x110)](/MOVE UPPER LEFT:[ ](\d+)/i))return this['processMoveRouteMoveRepeat'](0x7,Number(RegExp['$1']));if(_0x34412e['match'](/MOVE UP:[ ](\d+)/i))return this['processMoveRouteMoveRepeat'](0x8,Number(RegExp['$1']));if(_0x34412e[_0x661eac(0x110)](/MOVE UPPER RIGHT:[ ](\d+)/i))return this['processMoveRouteMoveRepeat'](0x9,Number(RegExp['$1']));if(_0x34412e[_0x661eac(0x110)](/OPACITY:[ ](\d+)([%％])/i)){const _0x520dad=Math[_0x661eac(0x1e4)](Number(RegExp['$1'])/0x64*0xff);return this[_0x661eac(0xd3)](_0x520dad[_0x661eac(0x551)](0x0,0xff));}if(_0x34412e[_0x661eac(0x110)](/OPACITY:[ ]([\+\-]\d+)([%％])/i)){const _0xb957e3=this['_opacity']+Math[_0x661eac(0x1e4)](Number(RegExp['$1'])/0x64*0xff);return this[_0x661eac(0xd3)](_0xb957e3['clamp'](0x0,0xff));}if(_0x34412e[_0x661eac(0x110)](/OPACITY:[ ]([\+\-]\d+)/i)){const _0x3c3264=this[_0x661eac(0x52a)]+Number(RegExp['$1']);return this[_0x661eac(0xd3)](_0x3c3264[_0x661eac(0x551)](0x0,0xff));}if(_0x34412e['match'](/PATTERN LOCK:[ ](\d+)/i))return this[_0x661eac(0x351)](Number(RegExp['$1']));if(_0x34412e[_0x661eac(0x110)](/PATTERN UNLOCK/i))return this['_patternLocked']=![];if(_0x34412e[_0x661eac(0x110)](/POSE:[ ](.*)/i)){const _0x369ead=String(RegExp['$1'])[_0x661eac(0x22e)]()[_0x661eac(0x36f)]();return this[_0x661eac(0x3e2)](_0x369ead);}if(_0x34412e['match'](/STEP TOWARD:\s*(\d+)\s*[, ]\s*(\d+)/i)){const _0x5593af=Number(RegExp['$1']),_0x37a976=Number(RegExp['$2']);return this[_0x661eac(0xeb)](_0x5593af,_0x37a976);}if(_0x34412e[_0x661eac(0x110)](/STEP TOWARD EVENT:[ ](\d+)/i)){const _0x4e3982=$gameMap[_0x661eac(0x2f2)](Number(RegExp['$1']));return this[_0x661eac(0x1f8)](_0x4e3982);}if(_0x34412e['match'](/STEP TOWARD PLAYER/i))return this['processMoveRouteStepToCharacter']($gamePlayer);if(_0x34412e[_0x661eac(0x110)](/STEP TOWARD HOME/i)&&this['eventId']){const _0x26c7eb=this[_0x661eac(0x450)],_0x3d396f=this[_0x661eac(0x4f6)];return this['processMoveRouteStepTo'](_0x26c7eb,_0x3d396f);}if(_0x34412e[_0x661eac(0x110)](/STEP AWAY FROM:\s*(\d+)\s*[, ]\s*(\d+)/i))return this[_0x661eac(0x4d4)](Number(RegExp['$1']),Number(RegExp['$2']));if(_0x34412e[_0x661eac(0x110)](/STEP AWAY FROM EVENT:[ ](\d+)/i)){const _0x12f0ad=$gameMap['event'](Number(RegExp['$1']));return this[_0x661eac(0x3b0)](_0x12f0ad);}if(_0x34412e[_0x661eac(0x110)](/STEP AWAY FROM PLAYER/i))return this[_0x661eac(0x3b0)]($gamePlayer);if(_0x34412e[_0x661eac(0x110)](/STEP AWAY FROM HOME/i)&&this['eventId']){const _0x1eec5a=this[_0x661eac(0x450)],_0x25d768=this['_randomHomeY'];return this['moveAwayFromPoint'](_0x1eec5a,_0x25d768);}if(_0x34412e[_0x661eac(0x110)](/TURN TO:\s*(\d+)\s*[, ]\s*(\d+)/i))return this[_0x661eac(0x540)](Number(RegExp['$1']),Number(RegExp['$2']));if(_0x34412e[_0x661eac(0x110)](/TURN TO EVENT:[ ](\d+)/i)){const _0x168b9b=$gameMap[_0x661eac(0x2f2)](Number(RegExp['$1']));return this[_0x661eac(0x2e8)](_0x168b9b);}if(_0x34412e[_0x661eac(0x110)](/TURN TO PLAYER/i))return this['turnTowardCharacter']($gamePlayer);if(_0x34412e[_0x661eac(0x110)](/TURN TO HOME/i)&&this[_0x661eac(0xb6)]){const _0x129ffa=this[_0x661eac(0x450)],_0x34b39e=this[_0x661eac(0x4f6)];return this[_0x661eac(0x262)](_0x129ffa,_0x34b39e);}if(_0x34412e['match'](/TURN AWAY FROM:\s*(\d+)\s*[, ]\s*(\d+)/i))return this[_0x661eac(0xcd)](Number(RegExp['$1']),Number(RegExp['$2']));if(_0x34412e[_0x661eac(0x110)](/TURN AWAY FROM EVENT:[ ](\d+)/i)){const _0x377589=$gameMap[_0x661eac(0x2f2)](Number(RegExp['$1']));return this[_0x661eac(0x155)](_0x377589);}if(_0x34412e['match'](/TURN AWAY FROM PLAYER/i))return this['turnAwayFromCharacter']($gamePlayer);if(_0x34412e[_0x661eac(0x110)](/TURN AWAY FROM HOME/i)&&this[_0x661eac(0xb6)]){const _0x4c219a=this[_0x661eac(0x450)],_0x58cabb=this[_0x661eac(0x4f6)];return this[_0x661eac(0xcd)](_0x4c219a,_0x58cabb);}if(_0x34412e[_0x661eac(0x110)](/TURN LOWER LEFT/i))return this['setDirection'](0x1);if(_0x34412e['match'](/TURN LOWER RIGHT/i))return this['setDirection'](0x3);if(_0x34412e['match'](/TURN UPPER LEFT/i))return this['setDirection'](0x7);if(_0x34412e[_0x661eac(0x110)](/TURN UPPER RIGHT/i))return this['setDirection'](0x9);if(_0x34412e[_0x661eac(0x110)](/Self Switch[ ](.*):[ ](.*)/i))return this['processMoveRouteSelfSwitch'](RegExp['$1'],RegExp['$2']);if(_0x34412e[_0x661eac(0x110)](/Self Variable[ ](.*):[ ](.*)/i))return this[_0x661eac(0x523)](RegExp['$1'],RegExp['$2']);if(_0x34412e['match'](/TELEPORT TO:\s*(\d+)\s*[, ]\s*(\d+)/i))return this[_0x661eac(0x4f8)](Number(RegExp['$1']),Number(RegExp['$2']));if(_0x34412e['match'](/TELEPORT TO EVENT:[ ](\d+)/i)){const _0x14ebe0=$gameMap['event'](Number(RegExp['$1']));return this[_0x661eac(0x30d)](_0x14ebe0);}if(_0x34412e['match'](/TELEPORT TO PLAYER/i))return this[_0x661eac(0x30d)]($gamePlayer);if(_0x34412e['match'](/TELEPORT TO HOME/i)&&this[_0x661eac(0xb6)]){const _0x263c5f=this['_randomHomeX'],_0x56e01b=this[_0x661eac(0x4f6)];return this[_0x661eac(0x4f8)](_0x263c5f,_0x56e01b);}try{VisuMZ[_0x661eac(0x3dc)]['Game_Character_processMoveCommand']['call'](this,_0x2c9d5e);}catch(_0x22576e){if($gameTemp['isPlaytest']())console[_0x661eac(0x28e)](_0x22576e);}},Game_Character[_0x442592(0x34e)][_0x442592(0x458)]=function(_0x145c52){const _0x574be0=_0x442592;$gameTemp[_0x574be0(0x15d)]([this],_0x145c52);},Game_Character[_0x442592(0x34e)][_0x442592(0x549)]=function(_0x7519ab){const _0x46239b=_0x442592;let _0x3ab898=0x0;switch(_0x7519ab[_0x46239b(0x22e)]()['trim']()){case'!':case'EXCLAMATION':_0x3ab898=0x1;break;case'?':case'QUESTION':_0x3ab898=0x2;break;case _0x46239b(0x19d):case _0x46239b(0x2c5):case'MUSIC\x20NOTE':case _0x46239b(0xbe):case _0x46239b(0x180):_0x3ab898=0x3;break;case'HEART':case _0x46239b(0x44b):_0x3ab898=0x4;break;case'ANGER':_0x3ab898=0x5;break;case _0x46239b(0x38e):_0x3ab898=0x6;break;case _0x46239b(0x35e):case _0x46239b(0x73):case'FRUSTRATION':_0x3ab898=0x7;break;case'SILENCE':case _0x46239b(0x1bd):_0x3ab898=0x8;break;case'LIGHT':case _0x46239b(0x228):case _0x46239b(0x2e7):case _0x46239b(0xc3):case _0x46239b(0x1d9):_0x3ab898=0x9;break;case'Z':case'ZZ':case _0x46239b(0x2b2):case'SLEEP':_0x3ab898=0xa;break;case'USER-DEFINED\x201':_0x3ab898=0xb;break;case'USER-DEFINED\x202':_0x3ab898=0xc;break;case _0x46239b(0x2a0):_0x3ab898=0xd;break;case _0x46239b(0x74):_0x3ab898=0xe;break;case _0x46239b(0x4d0):_0x3ab898=0xf;break;}$gameTemp[_0x46239b(0x49f)](this,_0x3ab898);},Game_Character[_0x442592(0x34e)][_0x442592(0xe6)]=function(_0x1ebb3a){const _0x1b5e44=_0x442592;_0x1ebb3a+=this[_0x1b5e44(0x52a)],this['setOpacity'](_0x1ebb3a[_0x1b5e44(0x551)](0x0,0xff));if(this['_opacity']<0xff)this[_0x1b5e44(0x296)]--;},Game_Character[_0x442592(0x34e)][_0x442592(0xfe)]=function(_0x415ba3){const _0x60017=_0x442592;_0x415ba3=this[_0x60017(0x52a)]-_0x415ba3,this[_0x60017(0xd3)](_0x415ba3['clamp'](0x0,0xff));if(this[_0x60017(0x52a)]>0x0)this[_0x60017(0x296)]--;},Game_Character[_0x442592(0x34e)]['processMoveRouteHugWall']=function(_0x16271e){const _0xbc6883=_0x442592,_0x379edc=[0x0,0x3,0x6,0x9,0x2,0x0,0x8,0x1,0x4,0x7],_0x17e13c=[0x0,0x7,0x4,0x1,0x8,0x0,0x2,0x9,0x6,0x3],_0x1fc930=this['direction'](),_0xf8fc9b=(_0x16271e==='left'?_0x379edc:_0x17e13c)[_0x1fc930],_0x5a4cae=(_0x16271e===_0xbc6883(0x1c4)?_0x17e13c:_0x379edc)[_0x1fc930];if(this['canPass'](this['x'],this['y'],_0xf8fc9b))_0x16271e===_0xbc6883(0x1c4)?this[_0xbc6883(0xb8)]():this[_0xbc6883(0x3af)]();else!this['canPass'](this['x'],this['y'],this[_0xbc6883(0x496)]())&&(this[_0xbc6883(0xe1)](this['x'],this['y'],_0x5a4cae)?_0x16271e===_0xbc6883(0x1c4)?this[_0xbc6883(0x3af)]():this['turnLeft90']():this[_0xbc6883(0x8d)]());this[_0xbc6883(0xe1)](this['x'],this['y'],this[_0xbc6883(0x496)]())&&this[_0xbc6883(0x1ef)]();},Game_Character['prototype'][_0x442592(0x497)]=function(_0x3c8496){const _0x4f351b=_0x442592;if(ImageManager[_0x4f351b(0x2ae)](this[_0x4f351b(0x4b2)]))return;_0x3c8496=_0x3c8496['clamp'](0x0,0x7),this[_0x4f351b(0x293)](this[_0x4f351b(0x4b2)],_0x3c8496);},Game_Character[_0x442592(0x34e)][_0x442592(0x1c7)]=function(_0x141587){const _0x6c899a=_0x442592;switch(this[_0x6c899a(0x496)]()){case 0x1:this[_0x6c899a(0x15b)](-_0x141587,_0x141587);break;case 0x2:this[_0x6c899a(0x15b)](0x0,_0x141587);break;case 0x3:this[_0x6c899a(0x15b)](_0x141587,_0x141587);break;case 0x4:this[_0x6c899a(0x15b)](-_0x141587,0x0);break;case 0x6:this[_0x6c899a(0x15b)](_0x141587,0x0);break;case 0x7:this[_0x6c899a(0x15b)](-_0x141587,-_0x141587);break;case 0x8:this[_0x6c899a(0x15b)](0x0,-_0x141587);break;case 0x9:this[_0x6c899a(0x15b)](_0x141587,-_0x141587);break;}},Game_Character['prototype']['processMoveRouteJumpTo']=function(_0x186d17,_0x54374f){const _0x98820e=_0x442592,_0xaa218a=Math[_0x98820e(0x1e4)](_0x186d17-this['x']),_0x267fd4=Math[_0x98820e(0x1e4)](_0x54374f-this['y']);this[_0x98820e(0x15b)](_0xaa218a,_0x267fd4);},Game_Character[_0x442592(0x34e)][_0x442592(0x4e8)]=function(_0x7b881c){if(_0x7b881c)this['processMoveRouteJumpTo'](_0x7b881c['x'],_0x7b881c['y']);},Game_Character['prototype'][_0x442592(0xeb)]=function(_0x568955,_0xf57a41,_0x8951e8){const _0x5005a2=_0x442592;let _0x20c8da=0x0;if(_0x8951e8)$gameTemp[_0x5005a2(0x232)]=!![];$gameMap['isSupportDiagonalMovement']()?_0x20c8da=this[_0x5005a2(0x371)](_0x568955,_0xf57a41):_0x20c8da=this[_0x5005a2(0x1e3)](_0x568955,_0xf57a41);if(_0x8951e8)$gameTemp['_moveAllowPlayerCollision']=![];this[_0x5005a2(0x8c)](_0x20c8da),this[_0x5005a2(0x84)](!![]);},Game_Character[_0x442592(0x34e)]['processMoveRouteStepToCharacter']=function(_0xe0ebef){const _0x6ff0f7=_0x442592;if(_0xe0ebef)this[_0x6ff0f7(0xeb)](_0xe0ebef['x'],_0xe0ebef['y']);},Game_Character[_0x442592(0x34e)][_0x442592(0x53c)]=function(_0x9e8ae2,_0x1a42bb){const _0x476568=_0x442592,_0x3c1167=this['deltaXFrom'](_0x9e8ae2),_0x183f84=this[_0x476568(0x11f)](_0x1a42bb);},Game_Character[_0x442592(0x34e)][_0x442592(0x3a8)]=function(_0x4b0af9){const _0x2ed0a8=_0x442592;if(_0x4b0af9[_0x2ed0a8(0x110)](/(?:CRASH|COLLIDE|COLLISION|ENCOUNTER|TOUCH)/i))return!![];else return _0x4b0af9[_0x2ed0a8(0x110)](/(?:AVOID|EVADE|DODGE)/i)?![]:![];},VisuMZ[_0x442592(0x3dc)][_0x442592(0x30e)]=Game_Event['prototype'][_0x442592(0x53f)],Game_Event[_0x442592(0x34e)][_0x442592(0x53f)]=function(_0x514193,_0x4ad48a){const _0x1a42fd=_0x442592;if($gameTemp['_moveAllowPlayerCollision'])return![];return VisuMZ[_0x1a42fd(0x3dc)][_0x1a42fd(0x30e)][_0x1a42fd(0x98)](this,_0x514193,_0x4ad48a);},Game_Character[_0x442592(0x34e)]['processMoveRouteMoveUntilStop']=function(_0x1dcc34,_0x42cfb9){const _0x5ed7fd=_0x442592,_0x34d0c9=['','LOWER\x20LEFT','DOWN','LOWER\x20RIGHT',_0x5ed7fd(0x469),'',_0x5ed7fd(0x1c1),_0x5ed7fd(0x302),'UP',_0x5ed7fd(0x37c)],_0x694c16=_0x34d0c9[_0x5ed7fd(0x269)](_0x1dcc34[_0x5ed7fd(0x22e)]()['trim']());if(_0x694c16<=0x0)return;_0x42cfb9&&($gameTemp[_0x5ed7fd(0x232)]=!![]),this['canPass'](this['x'],this['y'],_0x694c16)&&(_0x42cfb9&&($gameTemp[_0x5ed7fd(0x232)]=![]),this[_0x5ed7fd(0x8c)](_0x694c16),this[_0x5ed7fd(0x296)]-=0x1),_0x42cfb9&&($gameTemp[_0x5ed7fd(0x232)]=![]);},VisuMZ[_0x442592(0x3dc)][_0x442592(0x1c9)]=Game_Event[_0x442592(0x34e)][_0x442592(0x211)],Game_Event[_0x442592(0x34e)][_0x442592(0x211)]=function(_0x4365fb,_0x49a456){const _0x20741c=_0x442592;if(VisuMZ[_0x20741c(0x3dc)][_0x20741c(0x1c9)][_0x20741c(0x98)](this,_0x4365fb,_0x49a456))return!![];if($gameMap[_0x20741c(0x4f3)]())return![];for(let _0x2088e3=-this['_addedHitbox'][_0x20741c(0x1c4)];_0x2088e3<=this[_0x20741c(0x1c5)][_0x20741c(0x28a)];_0x2088e3++){for(let _0x744398=-this['_addedHitbox']['up'];_0x744398<=this[_0x20741c(0x1c5)][_0x20741c(0x17d)];_0x744398++){if(VisuMZ[_0x20741c(0x3dc)][_0x20741c(0x1c9)][_0x20741c(0x98)](this,_0x4365fb+_0x2088e3,_0x49a456+_0x744398))return!![];}}return![];},Game_Character[_0x442592(0x34e)][_0x442592(0x1f0)]=function(_0x3d90a4,_0x4e5995,_0xc68bd7){const _0x2fb4e8=_0x442592;this[_0x2fb4e8(0xeb)](_0x3d90a4,_0x4e5995,_0xc68bd7);if(this['x']!==_0x3d90a4||this['y']!==_0x4e5995)this['_moveRouteIndex']--;},Game_Character['prototype'][_0x442592(0x368)]=function(_0x595492,_0x5416c9){const _0x14ec6c=_0x442592;if(_0x595492&&!_0x595492[_0x14ec6c(0x427)]){this[_0x14ec6c(0x1f0)](_0x595492['x'],_0x595492['y'],_0x5416c9);if(_0x595492[_0x14ec6c(0x44c)]()&&this[_0x14ec6c(0x44c)]()){const _0x1a294b=$gameMap[_0x14ec6c(0x451)](this['x'],this['y'],_0x595492['x'],_0x595492['y']);if(_0x1a294b<=0x1)this[_0x14ec6c(0x296)]++;}}},Game_Character[_0x442592(0x34e)][_0x442592(0x25d)]=function(_0x5bcead,_0x5a7b52){const _0x3c4a60=_0x442592;_0x5a7b52=_0x5a7b52||0x0;const _0x251b3b={'code':0x1,'indent':null,'parameters':[]};_0x251b3b[_0x3c4a60(0x308)]=[0x0,0x5,0x1,0x6,0x2,0x0,0x3,0x7,0x4,0x8][_0x5bcead],this['_moveRoute']['list'][this[_0x3c4a60(0x296)]][_0x3c4a60(0xdc)][0x0]='';while(_0x5a7b52--){this[_0x3c4a60(0xaa)]['list'][_0x3c4a60(0x2d6)](this[_0x3c4a60(0x296)]+0x1,0x0,_0x251b3b);}},Game_Character[_0x442592(0x34e)]['processMoveRoutePatternLock']=function(_0x4a6f9b){const _0x222c12=_0x442592;this[_0x222c12(0xc5)]=!![],this[_0x222c12(0x1dd)](_0x4a6f9b);},Game_Character[_0x442592(0x34e)][_0x442592(0x122)]=function(_0x467ac0,_0x71ebba){const _0x1c8a49=_0x442592;if(this===$gamePlayer)return;const _0x129ab9=[this['_mapId'],this[_0x1c8a49(0xa3)],'A'];_0x467ac0[_0x1c8a49(0x110)](/\b[ABCD]\b/i)?_0x129ab9[0x2]=String(_0x467ac0)[_0x1c8a49(0x4ea)](0x0)['toUpperCase']()['trim']():_0x129ab9[0x2]=_0x1c8a49(0x4a4)[_0x1c8a49(0x2e6)](_0x467ac0);switch(_0x71ebba[_0x1c8a49(0x22e)]()['trim']()){case'ON':case'TRUE':$gameSelfSwitches[_0x1c8a49(0x3c5)](_0x129ab9,!![]);break;case _0x1c8a49(0x1f3):case'FALSE':$gameSelfSwitches[_0x1c8a49(0x3c5)](_0x129ab9,![]);break;case'TOGGLE':$gameSelfSwitches[_0x1c8a49(0x3c5)](_0x129ab9,!$gameSelfSwitches['value'](_0x129ab9));break;}},Game_Character[_0x442592(0x34e)][_0x442592(0x523)]=function(_0x3e47d9,_0x342e5e){const _0x4452a4=_0x442592;if(this===$gamePlayer)return;const _0x47345b=[this[_0x4452a4(0x118)],this[_0x4452a4(0xa3)],_0x4452a4(0x39a)[_0x4452a4(0x2e6)](_0x3e47d9)];$gameSelfSwitches[_0x4452a4(0x3c5)](_0x47345b,Number(_0x342e5e));},Game_Character[_0x442592(0x34e)]['processMoveRouteTeleportTo']=function(_0x3ad82d,_0x39bb22){const _0x3f5cfd=_0x442592;this[_0x3f5cfd(0x40b)](_0x3ad82d,_0x39bb22);},Game_Character[_0x442592(0x34e)][_0x442592(0x30d)]=function(_0x13f3a0){const _0x4d2192=_0x442592;if(_0x13f3a0)this[_0x4d2192(0x4f8)](_0x13f3a0['x'],_0x13f3a0['y']);},Game_Character[_0x442592(0x34e)][_0x442592(0x3af)]=function(){const _0x3a7ff5=_0x442592;switch(this[_0x3a7ff5(0x496)]()){case 0x1:this[_0x3a7ff5(0x421)](0x7);break;case 0x2:this[_0x3a7ff5(0x421)](0x4);break;case 0x3:this[_0x3a7ff5(0x421)](0x1);break;case 0x4:this['setDirection'](0x8);break;case 0x6:this['setDirection'](0x2);break;case 0x7:this['setDirection'](0x9);break;case 0x8:this[_0x3a7ff5(0x421)](0x6);break;case 0x9:this['setDirection'](0x3);break;}},Game_Character['prototype'][_0x442592(0xb8)]=function(){const _0x13cbac=_0x442592;switch(this[_0x13cbac(0x496)]()){case 0x1:this['setDirection'](0x3);break;case 0x2:this[_0x13cbac(0x421)](0x6);break;case 0x3:this[_0x13cbac(0x421)](0x9);break;case 0x4:this['setDirection'](0x2);break;case 0x6:this[_0x13cbac(0x421)](0x8);break;case 0x7:this[_0x13cbac(0x421)](0x1);break;case 0x8:this['setDirection'](0x4);break;case 0x9:this[_0x13cbac(0x421)](0x7);break;}},Game_Character['prototype']['getDirectionToPoint']=function(_0x58a07d,_0x5713a4,_0x45bca3){const _0x4915ec=_0x442592,_0x4120f8=this[_0x4915ec(0x455)](_0x58a07d),_0x283fa4=this[_0x4915ec(0x11f)](_0x5713a4);if($gameMap['isSupportDiagonalMovement']()){if(_0x45bca3||this[_0x4915ec(0x1fe)]()){if(_0x4120f8>0x0&&_0x283fa4<0x0)return 0x1;if(_0x4120f8<0x0&&_0x283fa4<0x0)return 0x3;if(_0x4120f8>0x0&&_0x283fa4>0x0)return 0x7;if(_0x4120f8<0x0&&_0x283fa4>0x0)return 0x9;}}if(Math[_0x4915ec(0x382)](_0x4120f8)>Math[_0x4915ec(0x382)](_0x283fa4))return _0x4120f8>0x0?0x4:0x6;else{if(_0x283fa4!==0x0)return _0x283fa4>0x0?0x8:0x2;}return 0x0;},Game_Character[_0x442592(0x34e)][_0x442592(0x3ae)]=function(_0x198282,_0x21b406,_0x3d7ef5){const _0x4fb25d=_0x442592,_0xe6492c=this['deltaXFrom'](_0x198282),_0x2bcf52=this[_0x4fb25d(0x11f)](_0x21b406);if($gameMap[_0x4fb25d(0x397)]()){if(_0x3d7ef5||this[_0x4fb25d(0x1fe)]()){if(_0xe6492c>0x0&&_0x2bcf52<0x0)return 0x9;if(_0xe6492c<0x0&&_0x2bcf52<0x0)return 0x7;if(_0xe6492c>0x0&&_0x2bcf52>0x0)return 0x3;if(_0xe6492c<0x0&&_0x2bcf52>0x0)return 0x1;}}if(Math[_0x4fb25d(0x382)](_0xe6492c)>Math[_0x4fb25d(0x382)](_0x2bcf52))return _0xe6492c>0x0?0x6:0x4;else{if(_0x2bcf52!==0x0)return _0x2bcf52>0x0?0x2:0x8;}return 0x0;},Game_Character[_0x442592(0x34e)][_0x442592(0x540)]=function(_0x1ffc1f,_0x65a0d7){const _0x300292=_0x442592,_0x103b3c=this[_0x300292(0x535)](_0x1ffc1f,_0x65a0d7,!![]);if(_0x103b3c)this[_0x300292(0x8c)](_0x103b3c);},Game_Character[_0x442592(0x34e)][_0x442592(0x4d4)]=function(_0x42cee8,_0xd0a980){const _0x2e7352=_0x442592,_0x19177e=this[_0x2e7352(0x3ae)](_0x42cee8,_0xd0a980,!![]);if(_0x19177e)this['executeMoveDir8'](_0x19177e);},Game_Character[_0x442592(0x34e)]['turnTowardPoint']=function(_0x19ba13,_0x45daf6){const _0x5aa707=_0x442592,_0xf4c0a9=this['getDirectionToPoint'](_0x19ba13,_0x45daf6,![]);if(_0xf4c0a9)this[_0x5aa707(0x421)](_0xf4c0a9);},Game_Character['prototype'][_0x442592(0xcd)]=function(_0x54a08a,_0x1ea57b){const _0x6a1f02=_0x442592,_0x3eca3d=this[_0x6a1f02(0x3ae)](_0x54a08a,_0x1ea57b,![]);if(_0x3eca3d)this[_0x6a1f02(0x421)](_0x3eca3d);},Game_Character[_0x442592(0x34e)][_0x442592(0x38d)]=function(_0x5dc2ba){const _0x2261a3=_0x442592;if(_0x5dc2ba)this[_0x2261a3(0x540)](_0x5dc2ba['x'],_0x5dc2ba['y']);},Game_Character[_0x442592(0x34e)][_0x442592(0x3b0)]=function(_0x12ed0b){const _0x17aaf5=_0x442592;if(_0x12ed0b)this[_0x17aaf5(0x4d4)](_0x12ed0b['x'],_0x12ed0b['y']);},Game_Character[_0x442592(0x34e)][_0x442592(0x2e8)]=function(_0x160413){const _0x250ad7=_0x442592;if(_0x160413)this[_0x250ad7(0x262)](_0x160413['x'],_0x160413['y']);},Game_Character[_0x442592(0x34e)][_0x442592(0x155)]=function(_0x540fd9){const _0x3e8abe=_0x442592;if(_0x540fd9)this[_0x3e8abe(0xcd)](_0x540fd9['x'],_0x540fd9['y']);},VisuMZ[_0x442592(0x3dc)][_0x442592(0x21f)]=Game_Player[_0x442592(0x34e)][_0x442592(0x23f)],Game_Player[_0x442592(0x34e)]['isDashing']=function(){const _0x8a454b=_0x442592;if(!Game_CharacterBase[_0x8a454b(0xc0)]&&this[_0x8a454b(0x9c)]())return![];if(this['_forceDashing'])return!![];return VisuMZ[_0x8a454b(0x3dc)][_0x8a454b(0x21f)][_0x8a454b(0x98)](this);},VisuMZ['EventsMoveCore']['Game_Player_getInputDirection']=Game_Player['prototype'][_0x442592(0x2d0)],Game_Player[_0x442592(0x34e)][_0x442592(0x2d0)]=function(){const _0x43c474=_0x442592;return $gameMap[_0x43c474(0x397)]()?this[_0x43c474(0x4de)]():VisuMZ[_0x43c474(0x3dc)][_0x43c474(0x147)]['call'](this);},Game_Player[_0x442592(0x34e)]['getInputDir8']=function(){return Input['dir8'];},Game_Player[_0x442592(0x34e)][_0x442592(0x422)]=function(){const _0x5c7316=_0x442592;if($gameSystem[_0x5c7316(0x1cd)]())return 0x0;if(!this[_0x5c7316(0x43c)]()&&this[_0x5c7316(0x38c)]()){let _0x5a689b=this[_0x5c7316(0x2d0)]();if(_0x5a689b>0x0)$gameTemp[_0x5c7316(0x333)]();else{if($gameTemp[_0x5c7316(0x2a8)]()){const _0x42a22a=$gameTemp[_0x5c7316(0x140)](),_0x300ba0=$gameTemp[_0x5c7316(0x162)]();this['getDiagonalDestination'](_0x42a22a,_0x300ba0)?_0x5a689b=this[_0x5c7316(0x371)](_0x42a22a,_0x300ba0):_0x5a689b=this['findDirectionTo'](_0x42a22a,_0x300ba0);}}_0x5a689b>0x0?(this[_0x5c7316(0x43b)]=this[_0x5c7316(0x43b)]||0x0,this[_0x5c7316(0x40d)]()?this['setDirection'](_0x5a689b):this[_0x5c7316(0xcb)](_0x5a689b),this[_0x5c7316(0x43b)]++):this[_0x5c7316(0x43b)]=0x0;}},Game_Player[_0x442592(0x34e)][_0x442592(0x40d)]=function(){const _0x25a835=_0x442592,_0x1d30bb=VisuMZ[_0x25a835(0x3dc)][_0x25a835(0x240)][_0x25a835(0x41e)];if(!_0x1d30bb[_0x25a835(0x18d)])return![];if($gameTemp[_0x25a835(0x2a8)]())return![];if(this[_0x25a835(0x23f)]()||this[_0x25a835(0x43c)]()||this[_0x25a835(0x9c)]())return![];return this['_inputTime']<_0x1d30bb['TurnInPlaceDelay'];},VisuMZ['EventsMoveCore'][_0x442592(0x390)]=Game_Player[_0x442592(0x34e)]['executeMove'],Game_Player[_0x442592(0x34e)][_0x442592(0xcb)]=function(_0xa08f73){const _0x176d99=_0x442592;$gameMap[_0x176d99(0x397)]()?this[_0x176d99(0x8c)](_0xa08f73):VisuMZ[_0x176d99(0x3dc)]['Game_Player_executeMove'][_0x176d99(0x98)](this,_0xa08f73);},VisuMZ[_0x442592(0x3dc)]['Game_Player_isMapPassable']=Game_Player[_0x442592(0x34e)]['isMapPassable'],Game_Player['prototype'][_0x442592(0x14c)]=function(_0x495798,_0x15bcdd,_0x1caaae){const _0x45d9e4=_0x442592;if($gameMap[_0x45d9e4(0x36c)](_0x495798,_0x15bcdd,_0x1caaae,'player'))return this[_0x45d9e4(0x4ed)]()&&this[_0x45d9e4(0xe9)]()?this[_0x45d9e4(0xe9)]()[_0x45d9e4(0x14c)](_0x495798,_0x15bcdd,_0x1caaae):!![];if($gameMap[_0x45d9e4(0x30a)](_0x495798,_0x15bcdd,_0x1caaae,_0x45d9e4(0x17a)))return![];return VisuMZ[_0x45d9e4(0x3dc)][_0x45d9e4(0x179)][_0x45d9e4(0x98)](this,_0x495798,_0x15bcdd,_0x1caaae);},VisuMZ[_0x442592(0x3dc)][_0x442592(0x218)]=Game_Player['prototype'][_0x442592(0x216)],Game_Player[_0x442592(0x34e)][_0x442592(0x216)]=function(_0x9bdde5){const _0x653b4f=_0x442592;VisuMZ[_0x653b4f(0x3dc)][_0x653b4f(0x218)][_0x653b4f(0x98)](this,_0x9bdde5);if(this['canStartLocalEvents']()){this['checkEventTriggerEventsMoveCore'](_0x9bdde5);if(_0x9bdde5['includes'](0x0)&&this['startMapCommonEventOnOKTarget']()===_0x653b4f(0x52c))this[_0x653b4f(0x19b)](this['x'],this['y']);else(_0x9bdde5[_0x653b4f(0xb5)](0x1)||_0x9bdde5[_0x653b4f(0xb5)](0x2))&&this[_0x653b4f(0x545)]();}},VisuMZ[_0x442592(0x3dc)][_0x442592(0x31a)]=Game_Player[_0x442592(0x34e)]['checkEventTriggerThere'],Game_Player[_0x442592(0x34e)][_0x442592(0x9e)]=function(_0x25c94d){const _0x3235cd=_0x442592;VisuMZ[_0x3235cd(0x3dc)]['Game_Player_checkEventTriggerThere']['call'](this,_0x25c94d);if(this[_0x3235cd(0x173)]()&&_0x25c94d['includes'](0x0)&&this[_0x3235cd(0x392)]()===_0x3235cd(0x380)){const _0xc3bf30=this[_0x3235cd(0x496)](),_0x15b38c=$gameMap['roundXWithDirection'](this['x'],_0xc3bf30),_0x336f16=$gameMap[_0x3235cd(0x114)](this['y'],_0xc3bf30);this[_0x3235cd(0x19b)](_0x15b38c,_0x336f16);}},Game_Player[_0x442592(0x34e)][_0x442592(0x196)]=function(_0x395d68){const _0x38efe2=_0x442592;if($gameMap[_0x38efe2(0x4f3)]())return;if($gameMap[_0x38efe2(0x124)]())return;const _0x4dbaa3=$gameMap[_0x38efe2(0x35b)]();for(const _0x4f7ec1 of _0x4dbaa3){if(!_0x4f7ec1)continue;if(!_0x4f7ec1['isTriggerIn'](_0x395d68))continue;if(this[_0x38efe2(0x4fa)](_0x4f7ec1))return _0x4f7ec1[_0x38efe2(0x2bb)]();if(this[_0x38efe2(0x47c)](_0x4f7ec1))return _0x4f7ec1['start']();}},Game_Player['prototype'][_0x442592(0x4fa)]=function(_0x2855cd){const _0x4525b9=_0x442592;if($gameMap['isEventRunning']())return![];if($gameMap[_0x4525b9(0x124)]())return![];return _0x2855cd[_0x4525b9(0x533)]()[_0x4525b9(0xb5)](this[_0x4525b9(0x8b)]());},Game_Player['prototype'][_0x442592(0x47c)]=function(_0x40b468){const _0x4b9114=_0x442592;if($gameMap['isEventRunning']())return![];if($gameMap[_0x4b9114(0x124)]())return![];if([_0x4b9114(0xd9),'region'][_0x4b9114(0xb5)](_0x40b468['activationProximityType']()))return![];const _0x230160=_0x40b468[_0x4b9114(0x81)](),_0x4637fd=_0x40b468['activationProximityDistance']();return this[_0x4b9114(0x1cf)](_0x40b468,_0x230160,_0x4637fd);},Game_Map[_0x442592(0x34e)][_0x442592(0x1cf)]=function(_0x3961c2,_0x1e1346,_0x38d036,_0x56b45c,_0x3c7ef9){const _0x213240=_0x442592;switch(_0x56b45c){case _0x213240(0x52e):return _0x3c7ef9>=Math[_0x213240(0x382)](_0x38d036[_0x213240(0x455)](_0x3961c2))&&_0x3c7ef9>=Math['abs'](_0x38d036[_0x213240(0x11f)](_0x1e1346));break;case _0x213240(0x230):const _0x5cbc8f=Math[_0x213240(0x404)](_0x38d036['x']-_0x3961c2,0x2),_0x9dd0df=Math['pow'](_0x38d036['y']-_0x1e1346,0x2);return _0x3c7ef9>=Math[_0x213240(0x1e4)](Math[_0x213240(0x271)](_0x5cbc8f+_0x9dd0df));break;case _0x213240(0x31e):case _0x213240(0x184):case _0x213240(0x220):const _0x21d7b6=$gameMap[_0x213240(0x451)](_0x3961c2,_0x1e1346,_0x38d036['x'],_0x38d036['y']);return _0x3c7ef9>=_0x21d7b6;break;case _0x213240(0x3c9):return _0x3c7ef9>=Math['abs'](_0x38d036[_0x213240(0x11f)](_0x1e1346));break;case _0x213240(0x11c):return _0x3c7ef9>=Math['abs'](_0x38d036[_0x213240(0x455)](_0x3961c2));break;}return![];},Game_Player[_0x442592(0x34e)][_0x442592(0x1cf)]=function(_0x5a9470,_0x4ba2fa,_0x391495){const _0x110066=this['x'],_0x1e7061=this['y'];return $gameMap['checkEventProximity'](_0x110066,_0x1e7061,_0x5a9470,_0x4ba2fa,_0x391495);},Game_Player[_0x442592(0x34e)][_0x442592(0x19b)]=function(_0x1a7e69,_0x1f66e2){const _0x3a6d8e=_0x442592;if($gameMap['isEventRunning']())return;if($gameMap[_0x3a6d8e(0x124)]())return;let _0x2b8f26=VisuMZ[_0x3a6d8e(0x3dc)]['Settings'][_0x3a6d8e(0x3ac)],_0x5dbb5e=$gameMap['regionId'](_0x1a7e69,_0x1f66e2);const _0x56948f='Region%1'[_0x3a6d8e(0x2e6)](_0x5dbb5e);_0x2b8f26[_0x56948f]&&$gameTemp[_0x3a6d8e(0x1bf)](_0x2b8f26[_0x56948f]);},Game_Player[_0x442592(0x34e)][_0x442592(0x392)]=function(){const _0x101df4=_0x442592;return VisuMZ['EventsMoveCore'][_0x101df4(0x240)][_0x101df4(0x1a1)];},Game_Player['prototype'][_0x442592(0x545)]=function(){const _0x301035=_0x442592;if($gameMap['isEventRunning']())return;if($gameMap[_0x301035(0x124)]())return;let _0x5c321f=VisuMZ['EventsMoveCore'][_0x301035(0x240)][_0x301035(0x1e8)];const _0x4211e0=_0x301035(0x154)[_0x301035(0x2e6)](this[_0x301035(0x8b)]());_0x5c321f[_0x4211e0]&&$gameTemp[_0x301035(0x1bf)](_0x5c321f[_0x4211e0]);},VisuMZ[_0x442592(0x3dc)]['Game_Player_increaseSteps']=Game_Player['prototype'][_0x442592(0x337)],Game_Player[_0x442592(0x34e)][_0x442592(0x337)]=function(){const _0x21665b=_0x442592;VisuMZ['EventsMoveCore'][_0x21665b(0x125)][_0x21665b(0x98)](this),VisuMZ[_0x21665b(0x1d1)](0x0);},Game_Player[_0x442592(0x34e)][_0x442592(0x507)]=function(){const _0x2bb874=_0x442592;VisuMZ[_0x2bb874(0x3f8)](0x0);},VisuMZ[_0x442592(0x3dc)]['Game_Follower_initialize']=Game_Follower[_0x442592(0x34e)][_0x442592(0x327)],Game_Follower[_0x442592(0x34e)][_0x442592(0x327)]=function(_0x4658a0){const _0x42f296=_0x442592;VisuMZ[_0x42f296(0x3dc)]['Game_Follower_initialize']['call'](this,_0x4658a0),this['_chaseOff']=![];},Game_Follower[_0x442592(0x34e)][_0x442592(0x23f)]=function(){const _0x55fff3=_0x442592;if(this['_chaseOff'])return Game_Character[_0x55fff3(0x34e)][_0x55fff3(0x23f)][_0x55fff3(0x98)](this);return $gamePlayer[_0x55fff3(0x23f)]();},Game_Follower[_0x442592(0x34e)][_0x442592(0x42d)]=function(){const _0xbae75a=_0x442592;if(this[_0xbae75a(0x138)])return Game_Character[_0xbae75a(0x34e)][_0xbae75a(0x42d)][_0xbae75a(0x98)](this);return $gamePlayer['isDashingAndMoving']()&&this[_0xbae75a(0x305)];},Game_Follower['prototype'][_0x442592(0x2dc)]=function(){const _0x501c5a=_0x442592;return $gamePlayer[_0x501c5a(0x2dc)]();},Game_Follower[_0x442592(0x34e)]['updateStop']=function(){const _0xaeee67=_0x442592;Game_Character[_0xaeee67(0x34e)][_0xaeee67(0x22d)][_0xaeee67(0x98)](this),this[_0xaeee67(0x4c8)]>0x0&&(this[_0xaeee67(0x305)]=![]);},Game_Follower[_0x442592(0x34e)]['setChaseOff']=function(_0x3724fc){this['_chaseOff']=_0x3724fc;},VisuMZ['EventsMoveCore']['Game_Follower_chaseCharacter']=Game_Follower[_0x442592(0x34e)]['chaseCharacter'],Game_Follower[_0x442592(0x34e)][_0x442592(0x227)]=function(_0x13cab0){const _0x3c422e=_0x442592;if(this[_0x3c422e(0x138)])return;if($gameSystem[_0x3c422e(0x384)]())return;VisuMZ['EventsMoveCore'][_0x3c422e(0x256)][_0x3c422e(0x98)](this,_0x13cab0),this[_0x3c422e(0x305)]=!![];},VisuMZ[_0x442592(0x3dc)]['Game_Vehicle_isMapPassable']=Game_Vehicle[_0x442592(0x34e)]['isMapPassable'],Game_Vehicle[_0x442592(0x34e)][_0x442592(0x14c)]=function(_0x141ef0,_0x2f3751,_0x344378){const _0x117670=_0x442592;if($gameMap[_0x117670(0x36c)](_0x141ef0,_0x2f3751,_0x344378,this[_0x117670(0x15e)]))return!![];if($gameMap[_0x117670(0x30a)](_0x141ef0,_0x2f3751,_0x344378,this[_0x117670(0x15e)]))return![];return VisuMZ['EventsMoveCore'][_0x117670(0x265)]['call'](this,_0x141ef0,_0x2f3751,_0x344378);},Game_Vehicle[_0x442592(0x34e)][_0x442592(0x4c6)]=function(_0x30c6c8,_0x73f412,_0x4710a9){const _0x486f5c=_0x442592;if($gameMap['isRegionAllowPass'](_0x30c6c8,_0x73f412,_0x4710a9,this[_0x486f5c(0x15e)]))return!![];if($gameMap[_0x486f5c(0x30a)](_0x30c6c8,_0x73f412,_0x4710a9,this[_0x486f5c(0x15e)]))return![];return VisuMZ['EventsMoveCore'][_0x486f5c(0x2c7)][_0x486f5c(0x98)]($gamePlayer,_0x30c6c8,_0x73f412,_0x4710a9);},VisuMZ[_0x442592(0x3dc)][_0x442592(0xad)]=Game_Vehicle[_0x442592(0x34e)][_0x442592(0x1f2)],Game_Vehicle[_0x442592(0x34e)][_0x442592(0x1f2)]=function(_0x4460a9,_0x4cc44c,_0xa4b273){const _0x3e0e65=_0x442592;if($gameMap['isRegionDockable'](_0x4460a9,_0x4cc44c,_0xa4b273,this[_0x3e0e65(0x15e)]))return!![];const _0x494904=this[_0x3e0e65(0x15e)][_0x3e0e65(0x4ea)](0x0)['toUpperCase']()+this[_0x3e0e65(0x15e)][_0x3e0e65(0x1f4)](0x1),_0x334dd1='%1DockRegionOnly'[_0x3e0e65(0x2e6)](_0x494904);return VisuMZ[_0x3e0e65(0x3dc)][_0x3e0e65(0x240)][_0x3e0e65(0x4ba)][_0x334dd1]?![]:VisuMZ['EventsMoveCore'][_0x3e0e65(0xad)][_0x3e0e65(0x98)](this,_0x4460a9,_0x4cc44c,_0xa4b273);},VisuMZ['EventsMoveCore'][_0x442592(0x33c)]=Game_Vehicle[_0x442592(0x34e)][_0x442592(0x12f)],Game_Vehicle['prototype'][_0x442592(0x12f)]=function(){const _0x2da30b=_0x442592;VisuMZ[_0x2da30b(0x3dc)][_0x2da30b(0x33c)][_0x2da30b(0x98)](this);const _0x19a629=VisuMZ['EventsMoveCore'][_0x2da30b(0x240)][_0x2da30b(0x41e)];if(this[_0x2da30b(0x37b)]()){if(_0x19a629[_0x2da30b(0x23c)])this[_0x2da30b(0x18f)](_0x19a629[_0x2da30b(0x23c)]);}else{if(this[_0x2da30b(0x197)]()){if(_0x19a629[_0x2da30b(0x541)])this[_0x2da30b(0x18f)](_0x19a629['ShipSpeed']);}else{if(this[_0x2da30b(0x399)]()){if(_0x19a629['AirshipSpeed'])this[_0x2da30b(0x18f)](_0x19a629['AirshipSpeed']);}}}},VisuMZ[_0x442592(0x3dc)][_0x442592(0xf6)]=Game_Event['prototype'][_0x442592(0x327)],Game_Event['prototype'][_0x442592(0x327)]=function(_0x2e4255,_0x26d5e8){const _0x4a5588=_0x442592;this[_0x4a5588(0x2b0)]=!![],VisuMZ[_0x4a5588(0x3dc)][_0x4a5588(0xf6)][_0x4a5588(0x98)](this,_0x2e4255,_0x26d5e8),this[_0x4a5588(0x2b0)]=undefined,this['setupCopyEvent'](),this[_0x4a5588(0x2ec)](),this[_0x4a5588(0x24e)]();},Game_Map[_0x442592(0x34e)][_0x442592(0x2ef)]=function(_0x15f212,_0x1b4cae){const _0x2624e3=_0x442592;return _0x15f212===$gameMap['mapId']()?$dataMap[_0x2624e3(0x35b)][_0x1b4cae]:VisuMZ[_0x2624e3(0xb1)][_0x15f212][_0x2624e3(0x35b)][_0x1b4cae];},VisuMZ[_0x442592(0x3dc)][_0x442592(0x1ea)]=Game_Event[_0x442592(0x34e)]['event'],Game_Event[_0x442592(0x34e)]['event']=function(){const _0x61846=_0x442592;if(this[_0x61846(0x4cb)]!==undefined){const _0x6b8157=this[_0x61846(0x4cb)]['mapId'],_0x253f3d=this[_0x61846(0x4cb)][_0x61846(0xb6)];return $gameMap[_0x61846(0x2ef)](_0x6b8157,_0x253f3d);}if(this[_0x61846(0x24d)]!==undefined){const _0x3e643f=this[_0x61846(0x24d)][_0x61846(0x321)],_0x20bffd=this[_0x61846(0x24d)][_0x61846(0xb6)];return $gameMap[_0x61846(0x2ef)](_0x3e643f,_0x20bffd);}if(this[_0x61846(0x349)]!==undefined){const _0x356cec=this[_0x61846(0x349)][_0x61846(0x321)],_0x2c6ec8=this[_0x61846(0x349)][_0x61846(0xb6)];return $gameMap[_0x61846(0x2ef)](_0x356cec,_0x2c6ec8);}if($gameTemp[_0x61846(0x3c7)]!==undefined){const _0x3799c6=$gameTemp[_0x61846(0x3c7)][_0x61846(0x321)],_0x3165a3=$gameTemp[_0x61846(0x3c7)][_0x61846(0xb6)];return $gameMap[_0x61846(0x2ef)](_0x3799c6,_0x3165a3);}return VisuMZ[_0x61846(0x3dc)][_0x61846(0x1ea)][_0x61846(0x98)](this);},Game_Event[_0x442592(0x34e)]['checkValidEventerMap']=function(_0x181dc7,_0x275c3f){const _0x557786=_0x442592;if(_0x181dc7===0x0||_0x275c3f===0x0)return![];if(_0x181dc7===$gameMap[_0x557786(0x321)]())return!![];if(!VisuMZ['PreloadedMaps'][_0x181dc7]&&_0x181dc7!==$gameMap['mapId']())return $gameTemp[_0x557786(0x255)]()&&console[_0x557786(0x28e)](_0x557786(0x3e1)[_0x557786(0x2e6)](_0x181dc7)),![];return!![];},VisuMZ[_0x442592(0x3dc)][_0x442592(0x39b)]=Game_Event[_0x442592(0x34e)]['start'],Game_Event[_0x442592(0x34e)][_0x442592(0x2bb)]=function(){const _0x412d1a=_0x442592;VisuMZ[_0x412d1a(0x3dc)][_0x412d1a(0x39b)][_0x412d1a(0x98)](this),Imported[_0x412d1a(0x201)]&&Input[_0x412d1a(0x243)](VisuMZ['MessageCore'][_0x412d1a(0x240)][_0x412d1a(0x151)]['FastForwardKey'])&&Input[_0x412d1a(0x530)]();},Game_Event['prototype']['setupCopyEvent']=function(){const _0x9664bc=_0x442592,_0x2cc0f6=this['event']()['note'];if(_0x2cc0f6==='')return;if(DataManager[_0x9664bc(0xb4)]()||DataManager[_0x9664bc(0x488)]())return;const _0xd5f268=VisuMZ[_0x9664bc(0x3dc)]['Settings']['Template'];let _0x9d3b02=null,_0x5f0b72=0x0,_0x5a1e0f=0x0;if(_0x2cc0f6['match'](/<COPY EVENT:[ ]MAP[ ](\d+),[ ]EVENT[ ](\d+)>/i)){_0x5f0b72=Number(RegExp['$1']),_0x5a1e0f=Number(RegExp['$2']);if(_0x5f0b72===0x0)_0x5f0b72=$gameMap[_0x9664bc(0x321)]();}else{if(_0x2cc0f6['match'](/<COPY EVENT:[ ](\d+),[ ](\d+)>/i)){_0x5f0b72=Number(RegExp['$1']),_0x5a1e0f=Number(RegExp['$2']);if(_0x5f0b72===0x0)_0x5f0b72=$gameMap[_0x9664bc(0x321)]();}else{if(_0x2cc0f6[_0x9664bc(0x110)](/<COPY EVENT:[ ](.*?)>/i)){const _0x15bf4d=String(RegExp['$1'])[_0x9664bc(0x22e)]()[_0x9664bc(0x36f)]();_0x9d3b02=VisuMZ[_0x9664bc(0x550)][_0x15bf4d];if(!_0x9d3b02)return;_0x5f0b72=_0x9d3b02['MapID'],_0x5a1e0f=_0x9d3b02[_0x9664bc(0x28b)];}}}if(!this[_0x9664bc(0x182)](_0x5f0b72,_0x5a1e0f))return;_0xd5f268[_0x9664bc(0x309)][_0x9664bc(0x98)](this,_0x5f0b72,_0x5a1e0f,this);if(_0x9d3b02)_0x9d3b02[_0x9664bc(0x309)]['call'](this,_0x5f0b72,_0x5a1e0f,this);this[_0x9664bc(0x24d)]={'mapId':_0x5f0b72,'eventId':_0x5a1e0f},this[_0x9664bc(0x34f)]=-0x2,this[_0x9664bc(0x29c)](),_0xd5f268[_0x9664bc(0x10f)][_0x9664bc(0x98)](this,_0x5f0b72,_0x5a1e0f,this);if(_0x9d3b02)_0x9d3b02[_0x9664bc(0x10f)]['call'](this,_0x5f0b72,_0x5a1e0f,this);$gameMap[_0x9664bc(0x3ef)]();},Game_Event['prototype'][_0x442592(0x2ec)]=function(){const _0x398ced=_0x442592,_0x49a751=$gameSystem[_0x398ced(0x4e5)](this);if(!_0x49a751)return;const _0x1fcb5b=_0x49a751[_0x398ced(0x379)][_0x398ced(0x22e)]()[_0x398ced(0x36f)]();_0x1fcb5b!=='UNTITLED'?this['morphIntoTemplate'](_0x1fcb5b,!![]):this[_0x398ced(0x35a)](_0x49a751[_0x398ced(0x321)],_0x49a751[_0x398ced(0xb6)],!![]);},Game_Event[_0x442592(0x34e)][_0x442592(0x35a)]=function(_0xa6ab02,_0x51b059,_0x1275ab){const _0xafc9dc=_0x442592;if(!this[_0xafc9dc(0x182)](_0xa6ab02,_0x51b059))return;const _0x401582=VisuMZ[_0xafc9dc(0x3dc)][_0xafc9dc(0x240)]['Template'];if(!_0x1275ab)_0x401582[_0xafc9dc(0x283)]['call'](this,_0xa6ab02,_0x51b059,this);this[_0xafc9dc(0x4cb)]={'mapId':_0xa6ab02,'eventId':_0x51b059},this['_pageIndex']=-0x2,this[_0xafc9dc(0x29c)]();if(!_0x1275ab)_0x401582[_0xafc9dc(0x16c)]['call'](this,_0xa6ab02,_0x51b059,this);$gameMap[_0xafc9dc(0x3ef)]();},Game_Event[_0x442592(0x34e)]['morphIntoTemplate']=function(_0x5651f0,_0x516103){const _0x5aa352=_0x442592;_0x5651f0=_0x5651f0[_0x5aa352(0x22e)]()[_0x5aa352(0x36f)]();const _0x8c1e70=VisuMZ[_0x5aa352(0x550)][_0x5651f0];if(!_0x8c1e70)return;const _0x2efa1a=_0x8c1e70['MapID'],_0x57dd9b=_0x8c1e70[_0x5aa352(0x28b)];if(!this[_0x5aa352(0x182)](_0x2efa1a,_0x57dd9b))return;if(!_0x516103)_0x8c1e70['PreMorphJS'][_0x5aa352(0x98)](this,_0x2efa1a,_0x57dd9b,this);this[_0x5aa352(0x35a)](_0x2efa1a,_0x57dd9b,_0x516103);if(!_0x516103)_0x8c1e70[_0x5aa352(0x16c)][_0x5aa352(0x98)](this,_0x2efa1a,_0x57dd9b,this);if($gameMap)$gameMap[_0x5aa352(0x3ef)]();},Game_Event[_0x442592(0x34e)][_0x442592(0x4d2)]=function(){const _0x1f276e=_0x442592;this[_0x1f276e(0x4cb)]=undefined,this[_0x1f276e(0x34f)]=-0x2,this[_0x1f276e(0x29c)]();},Game_Event[_0x442592(0x34e)][_0x442592(0x189)]=function(_0x3dbade){const _0x40e76b=_0x442592,_0x42e0e9=VisuMZ['EventsMoveCore'][_0x40e76b(0x240)][_0x40e76b(0x2bf)],_0x41d209=_0x3dbade['template'][_0x40e76b(0x22e)]()[_0x40e76b(0x36f)](),_0x58109f=!['',_0x40e76b(0x2b8)][_0x40e76b(0xb5)](_0x41d209);let _0x430122=0x0,_0x3295f9=0x0;if(_0x58109f){const _0x16827a=VisuMZ['EventTemplates'][_0x41d209];if(!_0x16827a)return;_0x430122=_0x16827a[_0x40e76b(0x3cb)],_0x3295f9=_0x16827a['EventID'];}else _0x430122=_0x3dbade[_0x40e76b(0x321)],_0x3295f9=_0x3dbade['eventId'];if(!this[_0x40e76b(0x182)](_0x430122,_0x3295f9))return;if(_0x58109f){const _0x2a3aae=VisuMZ[_0x40e76b(0x550)][_0x41d209];_0x2a3aae['PreSpawnJS']['call'](this,_0x430122,_0x3295f9,this);}_0x42e0e9['PreSpawnJS'][_0x40e76b(0x98)](this,_0x430122,_0x3295f9,this),this['_eventSpawnData']=_0x3dbade,this[_0x40e76b(0x34f)]=-0x2,this[_0x40e76b(0x118)]=$gameMap[_0x40e76b(0x321)](),this[_0x40e76b(0xa3)]=_0x3dbade[_0x40e76b(0x3c2)],this[_0x40e76b(0x340)]=_0x3dbade[_0x40e76b(0x210)],this[_0x40e76b(0x40b)](_0x3dbade['x'],_0x3dbade['y']),this[_0x40e76b(0x421)](_0x3dbade[_0x40e76b(0x496)]),this[_0x40e76b(0x29c)]();if(_0x58109f){const _0x5d8393=VisuMZ[_0x40e76b(0x550)][_0x41d209];if(!_0x5d8393)return;_0x5d8393[_0x40e76b(0x237)][_0x40e76b(0x98)](this,_0x430122,_0x3295f9,this);}_0x42e0e9[_0x40e76b(0x237)][_0x40e76b(0x98)](this,_0x430122,_0x3295f9,this);const _0x43b572=SceneManager['_scene'];if(_0x43b572&&_0x43b572[_0x40e76b(0x54c)])_0x43b572[_0x40e76b(0x54c)][_0x40e76b(0xd1)](this);},Game_Event[_0x442592(0x34e)]['isSpawnedEvent']=function(){const _0x20dcb4=_0x442592;return!!this[_0x20dcb4(0x349)];},Game_Event[_0x442592(0x34e)][_0x442592(0x2bb)]=function(){const _0x21d5a9=_0x442592;if(!this['list']())return;const _0x131db3=this[_0x21d5a9(0x249)]()[_0x21d5a9(0xda)](_0x406633=>_0x406633[_0x21d5a9(0x308)]!==0x6c&&_0x406633[_0x21d5a9(0x308)]!==0x198);_0x131db3[_0x21d5a9(0x2fe)]>0x1&&(this[_0x21d5a9(0x2da)]=!![],this[_0x21d5a9(0x4dc)]([0x0,0x1,0x2])&&this[_0x21d5a9(0x21d)]());},VisuMZ[_0x442592(0x3dc)][_0x442592(0x39d)]=Game_Event[_0x442592(0x34e)][_0x442592(0x3f9)],Game_Event[_0x442592(0x34e)][_0x442592(0x3f9)]=function(){const _0x469428=_0x442592;VisuMZ[_0x469428(0x3dc)][_0x469428(0x39d)]['call'](this),this[_0x469428(0x102)](),this['autosaveEventLocation']();},VisuMZ[_0x442592(0x3dc)]['Game_Event_setupPageSettings']=Game_Event[_0x442592(0x34e)][_0x442592(0x2b5)],Game_Event[_0x442592(0x34e)]['setupPageSettings']=function(){const _0x5486f8=_0x442592;this['_activationProximityAutoTriggerBypass']=!![],VisuMZ[_0x5486f8(0x3dc)]['Game_Event_setupPageSettings'][_0x5486f8(0x98)](this),this[_0x5486f8(0x49b)](),this[_0x5486f8(0x297)](),this['_activationProximityAutoTriggerBypass']=![];},Game_Event[_0x442592(0x34e)][_0x442592(0x49b)]=function(){const _0x42702a=_0x442592;if(!this[_0x42702a(0x2f2)]())return;this[_0x42702a(0x102)](),this[_0x42702a(0x291)](),this[_0x42702a(0x121)](),this['updateEventsMoveCoreTagChanges']();},Game_Event[_0x442592(0x34e)]['setupEventsMoveCoreNotetags']=function(){const _0x425a26=_0x442592,_0x3bd28=this['event']()[_0x425a26(0x2fc)];if(_0x3bd28==='')return;this[_0x425a26(0x1bc)](_0x3bd28);},Game_Event[_0x442592(0x34e)]['setupEventsMoveCoreCommentTags']=function(){const _0x251e37=_0x442592;if(!this['page']())return;const _0x3969fe=this[_0x251e37(0x249)]();let _0x58060f='';for(const _0x372800 of _0x3969fe){if([0x6c,0x198]['includes'](_0x372800[_0x251e37(0x308)])){if(_0x58060f!=='')_0x58060f+='\x0a';_0x58060f+=_0x372800[_0x251e37(0xdc)][0x0];}}this[_0x251e37(0x1bc)](_0x58060f);},Game_Event['prototype'][_0x442592(0x102)]=function(){const _0x5d5c67=_0x442592,_0x3d6274=VisuMZ[_0x5d5c67(0x3dc)]['Settings'];this[_0x5d5c67(0xa0)]={'type':_0x5d5c67(0xd9),'distance':0x0,'regionList':[]},this[_0x5d5c67(0x2de)]=![],this[_0x5d5c67(0x252)]=![],this[_0x5d5c67(0x23a)](),this[_0x5d5c67(0x1a3)]=![],this['_customZ']=![],(this[_0x5d5c67(0x391)]()||this[_0x5d5c67(0x513)]())&&this[_0x5d5c67(0x20e)]===0x0&&(this[_0x5d5c67(0x3b5)]=0x0),this['_addedHitbox']={'up':0x0,'down':0x0,'left':0x0,'right':0x0},this['_encounterHalfProximity']={'type':_0x5d5c67(0xd9),'distance':0x0},this[_0x5d5c67(0x381)]={'type':'none','distance':0x0},$gameSystem[_0x5d5c67(0x1a6)](this),this['_eventIcon']=$gameSystem['getEventIconData'](this),this[_0x5d5c67(0x2a4)]={'originalText':'','text':'','visibleRange':_0x3d6274[_0x5d5c67(0x403)][_0x5d5c67(0x4a9)],'rangeType':_0x3d6274[_0x5d5c67(0x403)]['RangeType'],'offsetX':_0x3d6274[_0x5d5c67(0x403)][_0x5d5c67(0x21e)],'offsetY':_0x3d6274['Label'][_0x5d5c67(0x1a4)],'hueShift':0x0},this[_0x5d5c67(0xc6)]=![],this[_0x5d5c67(0x199)]=[],this[_0x5d5c67(0x187)]={'target':-0x1,'type':_0x5d5c67(0x129),'delay':0x1,'opacityDelta':0x0},this[_0x5d5c67(0x1d8)]=_0x3d6274['Movement'][_0x5d5c67(0x468)]??0x0,this[_0x5d5c67(0x438)]=![],this['_scaleBaseX']=0x1,this[_0x5d5c67(0x348)]=0x1,this['_screenActivation']=![],this[_0x5d5c67(0x24b)]=![],this['_screenParallelOnce']=![],this[_0x5d5c67(0x128)]={'visible':!![],'filename':_0x3d6274[_0x5d5c67(0x41e)][_0x5d5c67(0x150)]},this[_0x5d5c67(0x1d2)]={'up':0x0,'down':0x0,'left':0x0,'right':0x0},this[_0x5d5c67(0x3a9)](),this[_0x5d5c67(0xc1)]();},Game_Event[_0x442592(0x34e)][_0x442592(0x1bc)]=function(_0x4ab763){const _0x369f39=_0x442592;if(_0x4ab763[_0x369f39(0x110)](/<ACTIVATION[ ](?:REGION|REGIONS):[ ]*(\d+(?:\s*,\s*\d+)*)>/i))this[_0x369f39(0xa0)]['regionList']=JSON[_0x369f39(0x2ab)]('['+RegExp['$1']['match'](/\d+/g)+']'),this[_0x369f39(0xa0)][_0x369f39(0x4b7)]=_0x369f39(0x4d7);else _0x4ab763[_0x369f39(0x110)](/<ACTIVATION[ ](.*?):[ ](\d+)>/i)&&(type=String(RegExp['$1'])[_0x369f39(0xdd)]()[_0x369f39(0x36f)](),this['_activationProximity'][_0x369f39(0x4b7)]=type,this[_0x369f39(0xa0)][_0x369f39(0x451)]=Number(RegExp['$2']));_0x4ab763[_0x369f39(0x110)](/<(?:ATTACH |)PICTURE FILENAME:[ ](.*?)>/i)&&(this[_0x369f39(0x3db)][_0x369f39(0x19c)]=String(RegExp['$1']),this[_0x369f39(0x3db)][_0x369f39(0x4b7)]=_0x369f39(0x4df));if(_0x4ab763[_0x369f39(0x110)](/<(?:ATTACH |)(?:PICTURE|ENEMY|SV ENEMY) BLEND MODE:[ ](.*?)>/i)){const _0x4e1ddf=String(RegExp['$1'])[_0x369f39(0x22e)]()[_0x369f39(0x36f)](),_0x3b6561=['NORMAL','ADDITIVE',_0x369f39(0x400),_0x369f39(0x40f)];this[_0x369f39(0x3db)]['blendMode']=_0x3b6561[_0x369f39(0x269)](_0x4e1ddf)[_0x369f39(0x551)](0x0,0x3);}_0x4ab763['match'](/<(?:ATTACH |)(?:PICTURE|ENEMY|SV ENEMY) (?:SIZE|MAX SIZE|MAX):[ ](\d+)>/i)&&(this[_0x369f39(0x3db)][_0x369f39(0x1af)]=Number(RegExp['$1']));_0x4ab763['match'](/<(?:ATTACH |)(?:PICTURE|ENEMY|SV ENEMY) OFFSET X:[ ]([\+\-]\d+)>/i)&&(this[_0x369f39(0x3db)][_0x369f39(0x532)]=Number(RegExp['$1']));_0x4ab763[_0x369f39(0x110)](/<(?:ATTACH |)(?:PICTURE|ENEMY|SV ENEMY) OFFSET Y:[ ]([\+\-]\d+)>/i)&&(this[_0x369f39(0x3db)][_0x369f39(0x149)]=Number(RegExp['$1']));_0x4ab763[_0x369f39(0x110)](/<(?:ATTACH |)(?:PICTURE|ENEMY|SV ENEMY) OFFSET:[ ]([\+\-]\d+),[ ]([\+\-]\d+)>/i)&&(this[_0x369f39(0x3db)][_0x369f39(0x532)]=Number(RegExp['$1']),this[_0x369f39(0x3db)][_0x369f39(0x149)]=Number(RegExp['$2']));_0x4ab763[_0x369f39(0x110)](/<(?:ATTACH |)(?:PICTURE|ENEMY|SV ENEMY) SCALE:[ ](\d+)([%％])>/i)&&(this[_0x369f39(0x3db)][_0x369f39(0x24c)]=Number(RegExp['$1'])*0.01);_0x4ab763[_0x369f39(0x110)](/<(?:ATTACH |)PICTURE TYPE:[ ](.*?)>/i)&&(this[_0x369f39(0x3db)]['type']=String(RegExp['$1'])[_0x369f39(0xdd)]()[_0x369f39(0x36f)]());_0x4ab763[_0x369f39(0x110)](/<(?:ATTACH |)ENEMY FILENAME:[ ](.*?)>/i)&&(this['_attachPicture']['filename']=String(RegExp['$1']),this[_0x369f39(0x3db)][_0x369f39(0x4b7)]=_0x369f39(0x188));_0x4ab763[_0x369f39(0x110)](/<(?:ATTACH |)SV ENEMY FILENAME:[ ](.*?)>/i)&&(this[_0x369f39(0x3db)][_0x369f39(0x19c)]=String(RegExp['$1']),this['_attachPicture'][_0x369f39(0x4b7)]=_0x369f39(0x1eb));_0x4ab763['match'](/<ALWAYS UPDATE MOVEMENT>/i)&&(this['_alwaysUpdateMove']=!![]);_0x4ab763[_0x369f39(0x110)](/<BYPASS CLICK STOP>/i)&&(this[_0x369f39(0x252)]=!![]);_0x4ab763[_0x369f39(0x110)](/<CLICK TRIGGER>/i)&&(this[_0x369f39(0x1a3)]=!![]);_0x4ab763['match'](/<CUSTOM Z:[ ](.*?)>/i)&&(this['_customZ']=Number(RegExp['$1'])||0x0);_0x4ab763[_0x369f39(0x110)](/<ENC(?:|OUNTER) HALF[ ](.*?):[ ](\d+)>/i)&&(type=String(RegExp['$1'])[_0x369f39(0xdd)]()[_0x369f39(0x36f)](),this[_0x369f39(0x501)][_0x369f39(0x4b7)]=type,this[_0x369f39(0x501)][_0x369f39(0x451)]=Number(RegExp['$2']));_0x4ab763[_0x369f39(0x110)](/<ENC(?:|OUNTER) NONE[ ](.*?):[ ](\d+)>/i)&&(type=String(RegExp['$1'])['toLowerCase']()[_0x369f39(0x36f)](),this[_0x369f39(0x381)]['type']=type,this[_0x369f39(0x381)][_0x369f39(0x451)]=Number(RegExp['$2']));const _0x2607a8=_0x4ab763[_0x369f39(0x110)](/<HITBOX[ ](.*?):[ ](\d+)>/gi);if(_0x2607a8)for(const _0x1115f8 of _0x2607a8){if(_0x1115f8[_0x369f39(0x110)](/<HITBOX[ ](.*?):[ ](\d+)>/i)){const _0x4dd01a=String(RegExp['$1'])[_0x369f39(0xdd)]()[_0x369f39(0x36f)](),_0x53595b=Number(RegExp['$2']);this['_addedHitbox'][_0x4dd01a]=_0x53595b;}}if(this[_0x369f39(0x4ca)]['iconIndex']>=0x0&&!this[_0x369f39(0x4ca)][_0x369f39(0x352)]){_0x4ab763[_0x369f39(0x110)](/<ICON:[ ](\d+)>/i)&&(this[_0x369f39(0x4ca)][_0x369f39(0xba)]=Number(RegExp['$1']));_0x4ab763[_0x369f39(0x110)](/<ICON (?:BUFFER|OFFSET) X:[ ]([\+\-]\d+)>/i)&&(this['_eventIcon'][_0x369f39(0x18e)]=Number(RegExp['$1']));_0x4ab763[_0x369f39(0x110)](/<ICON (?:BUFFER|OFFSET) Y:[ ]([\+\-]\d+)>/i)&&(this['_eventIcon'][_0x369f39(0x2ed)]=Number(RegExp['$1']));_0x4ab763['match'](/<ICON (?:BUFFER|OFFSET):[ ]([\+\-]\d+),[ ]([\+\-]\d+)>/i)&&(this[_0x369f39(0x4ca)]['bufferX']=Number(RegExp['$1']),this[_0x369f39(0x4ca)][_0x369f39(0x2ed)]=Number(RegExp['$2']));if(_0x4ab763[_0x369f39(0x110)](/<ICON BLEND MODE:[ ](.*?)>/i)){const _0x18b08e=String(RegExp['$1'])[_0x369f39(0x22e)]()['trim'](),_0x13d79c=[_0x369f39(0x460),_0x369f39(0x4be),_0x369f39(0x400),'SCREEN'];this['_eventIcon'][_0x369f39(0x7c)]=_0x13d79c[_0x369f39(0x269)](_0x18b08e)[_0x369f39(0x551)](0x0,0x3);}$gameSystem['setEventIconData'](this,this[_0x369f39(0x4ca)][_0x369f39(0xba)],this[_0x369f39(0x4ca)]['bufferX'],this['_eventIcon'][_0x369f39(0x2ed)],this[_0x369f39(0x4ca)][_0x369f39(0x7c)]);}if(_0x4ab763[_0x369f39(0x110)](/<LABEL:[ ](.*?)>/i)){let _0x5c0a55=String(RegExp['$1'])[_0x369f39(0x36f)]();this[_0x369f39(0x2a4)][_0x369f39(0x410)]=_0x5c0a55,this['_labelWindow'][_0x369f39(0x43f)]=_0x5c0a55;}if(_0x4ab763['match'](/<LABEL>\s*([\s\S]*)\s*<\/LABEL>/i)){let _0x1442cd=String(RegExp['$1'])[_0x369f39(0x36f)]();this['_labelWindow']['text']=_0x1442cd,this[_0x369f39(0x2a4)]['originalText']=_0x1442cd;}_0x4ab763['match'](/<LABEL (?:BUFFER|OFFSET) X:[ ]([\+\-]\d+)>/i)&&(this[_0x369f39(0x2a4)][_0x369f39(0x532)]=Number(RegExp['$1']));_0x4ab763[_0x369f39(0x110)](/<LABEL (?:BUFFER|OFFSET) Y:[ ]([\+\-]\d+)>/i)&&(this[_0x369f39(0x2a4)][_0x369f39(0x149)]=Number(RegExp['$1']));_0x4ab763[_0x369f39(0x110)](/<LABEL (?:BUFFER|OFFSET):[ ]([\+\-]\d+),[ ]([\+\-]\d+)>/i)&&(this[_0x369f39(0x2a4)]['offsetX']=Number(RegExp['$1']),this[_0x369f39(0x2a4)][_0x369f39(0x149)]=Number(RegExp['$2']));_0x4ab763[_0x369f39(0x110)](/<LABEL HUE SHIFT:[ ](.*?)>/i)&&(this[_0x369f39(0x2a4)][_0x369f39(0x178)]=Number(RegExp['$1']));_0x4ab763[_0x369f39(0x110)](/<LABEL RANGE:[ ](\d+)>/i)&&(this['_labelWindow']['visibleRange']=Number(RegExp['$1']));_0x4ab763[_0x369f39(0x110)](/<LABEL RANGE TYPE: SQUARE>/i)&&(this[_0x369f39(0x2a4)][_0x369f39(0xee)]=_0x369f39(0x52e));_0x4ab763[_0x369f39(0x110)](/<LABEL RANGE TYPE: (?:RADIUS|DELTA|DIAMOND)>/i)&&(this[_0x369f39(0x2a4)][_0x369f39(0xee)]='delta');_0x4ab763['match'](/<LABEL RANGE TYPE: CIRCLE>/i)&&(this['_labelWindow']['rangeType']=_0x369f39(0x230));this[_0x369f39(0x132)]();_0x4ab763['match'](/<MIRROR SPRITE>/i)&&(this[_0x369f39(0xc6)]=!![]);if(_0x4ab763[_0x369f39(0x110)](/<MOVE ONLY (?:REGION|REGIONS):[ ](\d+(?:\s*,\s*\d+)*)>/i)){const _0x5d6561=JSON[_0x369f39(0x2ab)]('['+RegExp['$1']['match'](/\d+/g)+']');this[_0x369f39(0x199)]=this[_0x369f39(0x199)][_0x369f39(0x2c2)](_0x5d6561),this[_0x369f39(0x199)][_0x369f39(0x486)](0x0);}if(_0x4ab763[_0x369f39(0x110)](/<MOVE SYNCH TARGET:[ ](.*?)>/i)){const _0x287cb7=String(RegExp['$1']);if(_0x287cb7[_0x369f39(0x110)](/PLAYER/i))this[_0x369f39(0x187)]['target']=0x0;else _0x287cb7['match'](/EVENT[ ](\d+)/i)&&(this['_moveSynch']['target']=Number(RegExp['$1']));}_0x4ab763[_0x369f39(0x110)](/<MOVE SYNCH TYPE:[ ](.*?)>/i)&&(this['_moveSynch'][_0x369f39(0x4b7)]=String(RegExp['$1'])['toLowerCase']()['trim']());_0x4ab763[_0x369f39(0x110)](/<MOVE SYNCH DELAY:[ ](\d+)>/i)&&(this[_0x369f39(0x187)]['delay']=Number(RegExp['$1']));_0x4ab763['match'](/<MOVE SYNCH DISTANCE OPACITY:[ ](.*?)>/i)&&(this[_0x369f39(0x187)]['opacityDelta']=Number(RegExp['$1']));if(_0x4ab763[_0x369f39(0x110)](/<TRUE RANDOM MOVE>/i))this[_0x369f39(0x1d8)]=0x0;else _0x4ab763[_0x369f39(0x110)](/<RANDOM MOVE WEIGHT:[ ](.*?)>/i)&&(this['_randomMoveWeight']=Number(RegExp['$1'])||0x0);_0x4ab763[_0x369f39(0x110)](/<SAVE EVENT (?:LOCATION|LOCATIONS)>/i)&&(this[_0x369f39(0x438)]=!![]);_0x4ab763['match'](/<SCALE X:[ ](\d+)([%％])>/i)&&(this['_scaleBaseX']=Number(RegExp['$1'])*0.01);_0x4ab763[_0x369f39(0x110)](/<SCALE Y:[ ](\d+)([%％])>/i)&&(this[_0x369f39(0x348)]=Number(RegExp['$1'])*0.01);if(_0x4ab763[_0x369f39(0x110)](/<SCALE:[ ](\d+)([%％])>/i)){const _0x43740b=Number(RegExp['$1'])*0.01;this[_0x369f39(0x1ba)]=_0x43740b,this[_0x369f39(0x348)]=_0x43740b;}_0x4ab763['match'](/<SCREEN ACTIVATION>/i)&&(this[_0x369f39(0x195)]=!![],this[_0x369f39(0x24b)]=![],this[_0x369f39(0x2e1)]=![]);if(_0x4ab763[_0x369f39(0x110)](/<SCREEN PARALLEL>/i))this[_0x369f39(0x195)]=![],this[_0x369f39(0x24b)]=!![],this[_0x369f39(0x2e1)]=![];else _0x4ab763[_0x369f39(0x110)](/<SCREEN PARALLEL ONCE>/i)&&(this['_screenActivation']=![],this[_0x369f39(0x24b)]=!![],this[_0x369f39(0x2e1)]=!![]);_0x4ab763[_0x369f39(0x110)](/<HIDE SHADOW>/i)&&(this[_0x369f39(0x128)]['visible']=![]),_0x4ab763[_0x369f39(0x110)](/<SHADOW FILENAME:[ ](.*?)>/i)&&(this[_0x369f39(0x128)][_0x369f39(0x19c)]=String(RegExp['$1'])),_0x4ab763['match'](/<SPRITE OFFSET X:[ ]([\+\-]\d+)>/i)&&(this[_0x369f39(0xf4)]=Number(RegExp['$1'])),_0x4ab763[_0x369f39(0x110)](/<SPRITE OFFSET Y:[ ]([\+\-]\d+)>/i)&&(this[_0x369f39(0x515)]=Number(RegExp['$1'])),_0x4ab763[_0x369f39(0x110)](/<SPRITE OFFSET:[ ]([\+\-]\d+),[ ]([\+\-]\d+)>/i)&&(this[_0x369f39(0xf4)]=Number(RegExp['$1']),this[_0x369f39(0x515)]=Number(RegExp['$2'])),_0x4ab763[_0x369f39(0x110)](/<STEP PATTERN:[ ](.*)>/i)&&(this[_0x369f39(0x466)]=String(RegExp['$1'])[_0x369f39(0x22e)]()['trim']()),_0x4ab763['match'](/<(?:TILE EXPAND|EXPAND TILE) UP:[ ](\d+)>/i)&&(this[_0x369f39(0x1d2)]=this['_tileExpand']||{},this[_0x369f39(0x1d2)]['up']=Number(RegExp['$1'])),_0x4ab763[_0x369f39(0x110)](/<(?:TILE EXPAND|EXPAND TILE) DOWN:[ ](\d+)>/i)&&(this[_0x369f39(0x1d2)]=this[_0x369f39(0x1d2)]||{},this[_0x369f39(0x1d2)][_0x369f39(0x17d)]=Number(RegExp['$1'])),_0x4ab763['match'](/<(?:TILE EXPAND|EXPAND TILE) LEFT:[ ](\d+)>/i)&&(this[_0x369f39(0x1d2)]=this[_0x369f39(0x1d2)]||{},this[_0x369f39(0x1d2)][_0x369f39(0x1c4)]=Number(RegExp['$1'])),_0x4ab763['match'](/<(?:TILE EXPAND|EXPAND TILE) RIGHT:[ ](\d+)>/i)&&(this[_0x369f39(0x1d2)]=this[_0x369f39(0x1d2)]||{},this['_tileExpand']['right']=Number(RegExp['$1']));},Game_Event['prototype'][_0x442592(0x132)]=function(){const _0x3785d5=_0x442592;$gameTemp['registerSelfTarget'](this),this[_0x3785d5(0x2a4)][_0x3785d5(0x410)]=this[_0x3785d5(0x2a4)][_0x3785d5(0x43f)];for(;;){if(this[_0x3785d5(0x2a4)][_0x3785d5(0x410)][_0x3785d5(0x110)](/\\V\[(\d+)\]/gi))this[_0x3785d5(0x2a4)][_0x3785d5(0x410)]=this[_0x3785d5(0x2a4)]['originalText'][_0x3785d5(0x25c)](/\\V\[(\d+)\]/gi,(_0x2f4540,_0x2e6d14)=>$gameVariables[_0x3785d5(0x161)](parseInt(_0x2e6d14)));else break;}$gameTemp['clearSelfTarget']();},Game_Event[_0x442592(0x34e)][_0x442592(0x1cc)]=function(){this['updateShadowChanges']();},Game_Event['prototype'][_0x442592(0x42f)]=function(){const _0x2d4c30=_0x442592;if(this[_0x2d4c30(0x2de)])return!![];return Game_Character[_0x2d4c30(0x34e)][_0x2d4c30(0x42f)][_0x2d4c30(0x98)](this);},VisuMZ[_0x442592(0x3dc)][_0x442592(0x435)]=Game_Event[_0x442592(0x34e)][_0x442592(0x163)],Game_Event[_0x442592(0x34e)]['updateSelfMovement']=function(){const _0x2b22e9=_0x442592;if(this['isPreventSelfMovement']())return;VisuMZ['EventsMoveCore']['Game_Event_updateSelfMovement']['call'](this),this['isMoving']()&&VisuMZ[_0x2b22e9(0x1d1)](this[_0x2b22e9(0xa3)]);},Game_Event['prototype'][_0x442592(0x294)]=function(){const _0x11a374=_0x442592,_0x2cdf18=VisuMZ['EventsMoveCore'][_0x11a374(0x240)][_0x11a374(0x41e)];if($gameMap[_0x11a374(0x4f3)]()&&_0x2cdf18[_0x11a374(0x3b7)])return!![];if($gameMessage[_0x11a374(0x12c)]()&&_0x2cdf18[_0x11a374(0xe5)])return!![];if(!$gameSystem[_0x11a374(0x7f)]())return!![];if(this[_0x11a374(0x372)]()>=0x0)return!![];if(!SceneManager['_scene'][_0x11a374(0x386)])return!![];return![];},Game_Event[_0x442592(0x34e)]['updateShadowChanges']=function(){const _0xe007ac=_0x442592,_0x4eed26=SceneManager['_scene']['_spriteset'];if(_0x4eed26){const _0x325445=_0x4eed26[_0xe007ac(0x20c)](this);_0x325445&&_0x325445[_0xe007ac(0x342)]&&_0x325445[_0xe007ac(0x342)][_0xe007ac(0x525)]!==this['shadowFilename']()&&(_0x325445[_0xe007ac(0x342)]['_filename']=this[_0xe007ac(0x1be)](),_0x325445[_0xe007ac(0x342)][_0xe007ac(0x547)]=ImageManager[_0xe007ac(0x329)](_0x325445[_0xe007ac(0x342)]['_filename']));}},Game_Event[_0x442592(0x34e)][_0x442592(0x1be)]=function(){const _0x2ab24b=_0x442592;return this[_0x2ab24b(0x128)]['filename'];},Game_Event[_0x442592(0x34e)][_0x442592(0x478)]=function(){const _0x2de78f=_0x442592;if(!this[_0x2de78f(0x128)][_0x2de78f(0x4ce)])return![];if($gamePlayer[_0x2de78f(0x2f9)])return![];return Game_CharacterBase[_0x2de78f(0x34e)][_0x2de78f(0x478)][_0x2de78f(0x98)](this);},Game_Event[_0x442592(0x34e)][_0x442592(0x388)]=function(){const _0x4ee77b=_0x442592;return this[_0x4ee77b(0x2a4)][_0x4ee77b(0x410)];},Game_Event[_0x442592(0x34e)][_0x442592(0x209)]=function(){const _0x3074d7=_0x442592;return this[_0x3074d7(0x2a4)][_0x3074d7(0x312)]??VisuMZ[_0x3074d7(0x3dc)]['Settings'][_0x3074d7(0x403)][_0x3074d7(0x4a9)];},Game_Event[_0x442592(0x34e)]['labelWindowRangeType']=function(){const _0x22271d=_0x442592;return this['_labelWindow'][_0x22271d(0xee)]??VisuMZ[_0x22271d(0x3dc)][_0x22271d(0x240)][_0x22271d(0x403)][_0x22271d(0x411)]??_0x22271d(0x52e);},VisuMZ['EventsMoveCore'][_0x442592(0xa5)]=function(_0x117015){const _0x1412c0=_0x442592,_0x5d1b13=_0x117015[_0x1412c0(0x364)](),_0x3902de=_0x117015['labelWindowRange']();return $gameMap[_0x1412c0(0x1cf)]($gamePlayer['x'],$gamePlayer['y'],_0x117015,_0x5d1b13,_0x3902de);},Game_Event[_0x442592(0x34e)][_0x442592(0x14c)]=function(_0xd35957,_0x57fcec,_0x2952a2){const _0x1ec46f=_0x442592;if(this[_0x1ec46f(0x224)]())return this['isMoveOnlyRegionPassable'](_0xd35957,_0x57fcec,_0x2952a2);if($gameMap[_0x1ec46f(0x36c)](_0xd35957,_0x57fcec,_0x2952a2,_0x1ec46f(0x2f2)))return!![];if($gameMap[_0x1ec46f(0x30a)](_0xd35957,_0x57fcec,_0x2952a2,_0x1ec46f(0x2f2)))return![];return Game_Character[_0x1ec46f(0x34e)][_0x1ec46f(0x14c)][_0x1ec46f(0x98)](this,_0xd35957,_0x57fcec,_0x2952a2);},Game_Event[_0x442592(0x34e)][_0x442592(0x224)]=function(){const _0x4a2af5=_0x442592;if(this[_0x4a2af5(0x199)]===undefined)this[_0x4a2af5(0x102)]();return this[_0x4a2af5(0x199)][_0x4a2af5(0x2fe)]>0x0;},Game_Event[_0x442592(0x34e)]['isMoveOnlyRegionPassable']=function(_0x10e8ec,_0x29bfb4,_0x28586a){const _0x265e3a=_0x442592,_0x5eed3c=$gameMap[_0x265e3a(0xd4)](_0x10e8ec,_0x28586a),_0x2aa7b9=$gameMap['roundYWithDirection'](_0x29bfb4,_0x28586a),_0x3de34d=$gameMap[_0x265e3a(0x8b)](_0x5eed3c,_0x2aa7b9);return this[_0x265e3a(0x199)]['includes'](_0x3de34d);},VisuMZ[_0x442592(0x3dc)]['Game_Event_findProperPageIndex']=Game_Event[_0x442592(0x34e)][_0x442592(0x78)],Game_Event[_0x442592(0x34e)]['findProperPageIndex']=function(){const _0x56eec9=_0x442592;if(this['event']()&&!$gameTemp[_0x56eec9(0x255)]()){if(this[_0x56eec9(0x2f2)]()[_0x56eec9(0x2fc)][_0x56eec9(0x110)](/<(?:PLAYTEST|PLAY TEST)>/i))return-0x1;}return this[_0x56eec9(0x4c3)]=![],this[_0x56eec9(0x3cf)]=![],this[_0x56eec9(0x2f2)]()?VisuMZ[_0x56eec9(0x3dc)][_0x56eec9(0x4f0)]['call'](this):-0x1;},VisuMZ[_0x442592(0x3dc)]['Game_Event_meetsConditions']=Game_Event[_0x442592(0x34e)][_0x442592(0xe3)],Game_Event[_0x442592(0x34e)][_0x442592(0xe3)]=function(_0x72f776){const _0x2c9fb9=_0x442592;this[_0x2c9fb9(0x53a)](_0x72f776),$gameTemp[_0x2c9fb9(0x2e9)](this);const _0x17c581=VisuMZ[_0x2c9fb9(0x3dc)][_0x2c9fb9(0x3d7)][_0x2c9fb9(0x98)](this,_0x72f776);return $gameTemp['clearSelfTarget'](),_0x17c581;},Game_Event[_0x442592(0x34e)][_0x442592(0x43d)]=function(){const _0x417788=_0x442592;return this[_0x417788(0x4c3)];},Game_Event[_0x442592(0x34e)][_0x442592(0x53a)]=function(_0x3a9513){const _0x50d746=_0x442592,_0x22a5db=_0x3a9513['conditions'];if(_0x22a5db['switch1Valid']&&DataManager['isAdvancedSwitch'](_0x22a5db[_0x50d746(0x277)]))this['_advancedSwitchVariable']=!![];else{if(_0x22a5db[_0x50d746(0x190)]&&DataManager[_0x50d746(0x198)](_0x22a5db[_0x50d746(0x10d)]))this['_advancedSwitchVariable']=!![];else _0x22a5db[_0x50d746(0x4ae)]&&DataManager[_0x50d746(0x4e0)](_0x22a5db[_0x50d746(0x2d5)])&&(this[_0x50d746(0x4c3)]=!![]);}},Game_Event['prototype'][_0x442592(0x2be)]=function(){const _0x335d48=_0x442592;if(this[_0x335d48(0x427)])return![];return this[_0x335d48(0x1a3)];},Game_Event[_0x442592(0x34e)][_0x442592(0x275)]=function(){const _0xa35c5d=_0x442592;$gameTemp[_0xa35c5d(0x333)](),this[_0xa35c5d(0x2bb)]();},Game_Event[_0x442592(0x34e)][_0x442592(0x6e)]=function(_0x40a3c5,_0x35da6b){const _0x5a6a0b=_0x442592;return this[_0x5a6a0b(0x1c5)]?this[_0x5a6a0b(0x22a)](_0x40a3c5,_0x35da6b):Game_Character[_0x5a6a0b(0x34e)][_0x5a6a0b(0x6e)]['call'](this,_0x40a3c5,_0x35da6b);},Game_Event[_0x442592(0x34e)][_0x442592(0x22a)]=function(_0x21fb8a,_0x285af6){const _0xe67bd6=_0x442592;var _0x3b4e9b=this['x']-this['_addedHitbox'][_0xe67bd6(0x1c4)],_0x512f23=this['x']+this[_0xe67bd6(0x1c5)][_0xe67bd6(0x28a)],_0x24b254=this['y']-this[_0xe67bd6(0x1c5)]['up'],_0x23e728=this['y']+this['_addedHitbox']['down'];return _0x3b4e9b<=_0x21fb8a&&_0x21fb8a<=_0x512f23&&_0x24b254<=_0x285af6&&_0x285af6<=_0x23e728;},VisuMZ[_0x442592(0x3dc)][_0x442592(0xf3)]=Game_Event['prototype'][_0x442592(0xe1)],Game_Event[_0x442592(0x34e)][_0x442592(0xe1)]=function(_0x342b33,_0x37b025,_0x2ff6ab){const _0x50acaf=_0x442592;for(let _0x1d4fbb=-this[_0x50acaf(0x1c5)]['left'];_0x1d4fbb<=this[_0x50acaf(0x1c5)][_0x50acaf(0x28a)];_0x1d4fbb++){for(let _0x39dbbd=-this[_0x50acaf(0x1c5)]['up'];_0x39dbbd<=this[_0x50acaf(0x1c5)][_0x50acaf(0x17d)];_0x39dbbd++){if(!Game_Character['prototype'][_0x50acaf(0xe1)]['call'](this,_0x342b33+_0x1d4fbb,_0x37b025+_0x39dbbd,_0x2ff6ab))return![];}}return!![];},Game_Event[_0x442592(0x34e)][_0x442592(0x1b5)]=function(_0x3b1ff3,_0x4327e9){const _0x3008a8=_0x442592;if(Imported[_0x3008a8(0x454)]&&this[_0x3008a8(0x492)]())return this['checkSmartEventCollision'](_0x3b1ff3,_0x4327e9);else{const _0x4deeee=$gameMap[_0x3008a8(0x1de)](_0x3b1ff3,_0x4327e9)[_0x3008a8(0xda)](_0xc37ae0=>_0xc37ae0!==this);return _0x4deeee[_0x3008a8(0x2fe)]>0x0;}},Game_Event['prototype'][_0x442592(0x13f)]=function(_0x1a4307,_0x50ef58){const _0x56a0e8=_0x442592;if(!this[_0x56a0e8(0x44c)]())return![];else{const _0x13275c=$gameMap[_0x56a0e8(0x1de)](_0x1a4307,_0x50ef58)[_0x56a0e8(0xda)](_0x18f75c=>_0x18f75c!==this&&_0x18f75c[_0x56a0e8(0x44c)]());return _0x13275c[_0x56a0e8(0x2fe)]>0x0;}},Game_Event['prototype'][_0x442592(0x81)]=function(){const _0x3083d2=_0x442592;if(!this[_0x3083d2(0xa0)])return'none';return this[_0x3083d2(0xa0)][_0x3083d2(0x4b7)]||'none';},Game_Event[_0x442592(0x34e)][_0x442592(0x2ce)]=function(){const _0x460fed=_0x442592;if(!this['_activationProximity'])return 0x0;return this[_0x460fed(0xa0)][_0x460fed(0x451)]||0x0;},Game_Event[_0x442592(0x34e)][_0x442592(0x533)]=function(){const _0x308c3a=_0x442592;if(!this[_0x308c3a(0xa0)])return[];return this[_0x308c3a(0xa0)][_0x308c3a(0x174)]||[];},Game_Event[_0x442592(0x34e)][_0x442592(0x337)]=function(){const _0x32e4f9=_0x442592;Game_Character[_0x32e4f9(0x34e)][_0x32e4f9(0x337)][_0x32e4f9(0x98)](this);if([_0x32e4f9(0xd9),'region'][_0x32e4f9(0xb5)](this[_0x32e4f9(0x81)]()))return;$gamePlayer[_0x32e4f9(0x196)]([0x2]);},Game_Event[_0x442592(0x34e)][_0x442592(0x222)]=function(){const _0x1386dc=_0x442592,_0x4285b3=Math[_0x1386dc(0x1e4)]($gameMap[_0x1386dc(0x31c)]),_0xb3ca16=_0x4285b3+Math['ceil']($gameMap[_0x1386dc(0x4da)]())-0x1,_0x42e8cb=Math[_0x1386dc(0x1e4)]($gameMap[_0x1386dc(0x18b)]),_0x1df324=_0x42e8cb+Math[_0x1386dc(0x51c)]($gameMap[_0x1386dc(0x3ff)]())-0x1;return this['x']>=_0x4285b3&&this['x']<=_0xb3ca16&&this['y']>=_0x42e8cb&&this['y']<=_0x1df324;},VisuMZ['EventsMoveCore'][_0x442592(0x417)]=Game_Event[_0x442592(0x34e)]['checkEventTriggerAuto'],Game_Event[_0x442592(0x34e)][_0x442592(0x2c3)]=function(){const _0x171748=_0x442592;if(this[_0x171748(0x195)]||this[_0x171748(0x24b)]){if(this['isOnScreen']()){if(!this['_screenActivated']){this['_screenActivated']=!![];if(this[_0x171748(0x195)])this[_0x171748(0x2bb)]();else this[_0x171748(0x24b)]&&(!this[_0x171748(0x465)]&&(this['_interpreter']=new Game_Interpreter()),this[_0x171748(0x465)][_0x171748(0x204)](this['list'](),this['_eventId']));}return;}else{this[_0x171748(0x3ce)]=![];return;}}if(this[_0x171748(0x445)]!==0x3)return;if(this['_activationProximityAutoTriggerBypass'])return;if(!this['checkRegionEventTrigger'](![]))return;if(!this['checkActivationProximity'](![]))return;VisuMZ['EventsMoveCore'][_0x171748(0x417)][_0x171748(0x98)](this);},VisuMZ[_0x442592(0x3dc)][_0x442592(0x3bf)]=Game_Event[_0x442592(0x34e)][_0x442592(0x4d5)],Game_Event[_0x442592(0x34e)]['updateParallel']=function(){const _0xb78e21=_0x442592;if(!this[_0xb78e21(0x465)])return;if(!this[_0xb78e21(0x475)](!![]))return;if(!this['checkActivationProximity'](!![]))return;if(this['_interpreter']&&!this['_interpreter'][_0xb78e21(0xd5)]()&&this[_0xb78e21(0x24b)]){!this[_0xb78e21(0x2e1)]&&(this[_0xb78e21(0x3ce)]=![]);return;}VisuMZ[_0xb78e21(0x3dc)]['Game_Event_updateParallel']['call'](this);},Game_Event[_0x442592(0x34e)][_0x442592(0x475)]=function(_0x597a7d){const _0xf47be7=_0x442592;if(!_0x597a7d&&$gameMap[_0xf47be7(0x4f3)]())return![];if(!_0x597a7d&&$gameMap['isAnyEventStarting']())return![];if(this[_0xf47be7(0x533)]()<=0x0)return!![];return $gamePlayer[_0xf47be7(0x4fa)](this);},Game_Event[_0x442592(0x34e)][_0x442592(0x241)]=function(_0xd84e36){const _0x43b93d=_0x442592;if(!_0xd84e36&&$gameMap[_0x43b93d(0x4f3)]())return![];if(!_0xd84e36&&$gameMap['isAnyEventStarting']())return![];if(['none',_0x43b93d(0x4d7)][_0x43b93d(0xb5)](this[_0x43b93d(0x81)]()))return!![];return $gamePlayer[_0x43b93d(0x47c)](this);},Game_Event[_0x442592(0x34e)][_0x442592(0x3fa)]=function(_0xcd101e){const _0x486947=_0x442592,_0x47f57f=_0xcd101e?this[_0x486947(0x501)]:this[_0x486947(0x381)];return _0x47f57f?_0x47f57f[_0x486947(0x4b7)]:'none';},Game_Event[_0x442592(0x34e)]['encounterProximityDistance']=function(_0x5012cd){const _0x196d6e=_0x442592,_0x1fbed8=_0x5012cd?this[_0x196d6e(0x501)]:this[_0x196d6e(0x381)];return _0x1fbed8?_0x1fbed8[_0x196d6e(0x451)]:0x0;},VisuMZ[_0x442592(0x1d1)]=function(_0x872b8){const _0x1fd58d=_0x442592;for(const _0x41d233 of $gameMap[_0x1fd58d(0x35b)]()){if(!_0x41d233)continue;_0x41d233[_0x1fd58d(0x372)]()===_0x872b8&&_0x41d233[_0x1fd58d(0x1a9)]();}},VisuMZ[_0x442592(0x3f5)]=function(_0x177fc8){if(_0x177fc8===0x0)return $gamePlayer;return $gameMap['event'](_0x177fc8);},Game_CharacterBase['prototype'][_0x442592(0x507)]=function(){},Game_Event[_0x442592(0x34e)]['updateMoveSynchDirection']=function(){const _0x22f048=_0x442592;VisuMZ['FaceSynchAllSynchTargets'](this[_0x22f048(0xa3)]);},VisuMZ[_0x442592(0x3f8)]=function(_0x156d17){const _0x2bb73e=_0x442592;for(const _0x15797b of $gameMap[_0x2bb73e(0x35b)]()){if(!_0x15797b)continue;_0x15797b[_0x2bb73e(0x372)]()===_0x156d17&&_0x15797b[_0x2bb73e(0x32e)]();}},Game_Event[_0x442592(0x34e)][_0x442592(0x372)]=function(){const _0x52e3d9=_0x442592;return this[_0x52e3d9(0x187)][_0x52e3d9(0x29f)];},Game_Event[_0x442592(0x34e)]['moveSynchType']=function(){const _0x3732d0=_0x442592;return this[_0x3732d0(0x187)][_0x3732d0(0x4b7)];},Game_Event[_0x442592(0x34e)][_0x442592(0x2dc)]=function(){const _0x24162c=_0x442592;if(this[_0x24162c(0x372)]()>=0x0){const _0x4972dd=VisuMZ[_0x24162c(0x3f5)](this[_0x24162c(0x372)]());if(_0x4972dd)return _0x4972dd['realMoveSpeed']();}return Game_Character[_0x24162c(0x34e)]['realMoveSpeed'][_0x24162c(0x98)](this);},Game_Event[_0x442592(0x34e)]['updateMoveSynch']=function(){const _0x2ab0a9=_0x442592;this[_0x2ab0a9(0x187)][_0x2ab0a9(0x471)]=this[_0x2ab0a9(0x187)][_0x2ab0a9(0x471)]||0x0,this[_0x2ab0a9(0x187)]['timer']--;if(this[_0x2ab0a9(0x187)][_0x2ab0a9(0x471)]>0x0)return;this['_moveSynch']['timer']=this['_moveSynch']['delay'],this['processMoveSynch']();},Game_Event['prototype'][_0x442592(0x13c)]=function(_0x99507d){const _0x5da9ad=_0x442592;if(this['moveSynchTarget']()>=0x0){const _0x58e75c=VisuMZ[_0x5da9ad(0x3f5)](this[_0x5da9ad(0x372)]());if(_0x58e75c){const _0x4e4a0e=$gameMap[_0x5da9ad(0x451)](this[_0x5da9ad(0x3c4)],this[_0x5da9ad(0x3e3)],_0x58e75c[_0x5da9ad(0x3c4)],_0x58e75c[_0x5da9ad(0x3e3)])-0x1,_0x3f9930=Math[_0x5da9ad(0x50d)]($gameMap['tileWidth'](),$gameMap['tileHeight']()),_0x18fe2a=this['_moveSynch'][_0x5da9ad(0x215)]||0x0;_0x99507d-=Math['max'](0x0,_0x4e4a0e)*_0x3f9930*_0x18fe2a;}}return _0x99507d;},Game_Event[_0x442592(0x34e)][_0x442592(0x539)]=function(){const _0x59bd26=_0x442592;switch(this[_0x59bd26(0x3f1)]()){case _0x59bd26(0x129):this[_0x59bd26(0x192)]();break;case'approach':this['processMoveSynchApproach']();break;case _0x59bd26(0x437):this['processMoveSynchAway']();break;case'custom':this[_0x59bd26(0x2ba)]();break;case _0x59bd26(0x4fd):case _0x59bd26(0x3b6):this['processMoveSynchMimic']();break;case _0x59bd26(0x28c):case'reverse\x20copy':this['processMoveSynchReverseMimic']();break;case _0x59bd26(0x1d3):case'horizontal\x20mirror':case _0x59bd26(0x229):case'horz\x20mirror':this[_0x59bd26(0x1f6)]();break;case'mirror\x20vertical':case _0x59bd26(0x543):case _0x59bd26(0x267):case'vert\x20mirror':this[_0x59bd26(0x3f3)]();break;default:this[_0x59bd26(0x192)]();break;}this[_0x59bd26(0x441)]();},Game_Event[_0x442592(0x34e)][_0x442592(0x192)]=function(){const _0x3f2dec=_0x442592,_0x2d2eea=[0x2,0x4,0x6,0x8];$gameMap[_0x3f2dec(0x397)]()&&_0x2d2eea[_0x3f2dec(0x314)](0x1,0x3,0x7,0x9);const _0x8b3d78=[];for(const _0x3cae62 of _0x2d2eea){if(this[_0x3f2dec(0xe1)](this['x'],this['y'],_0x3cae62))_0x8b3d78[_0x3f2dec(0x314)](_0x3cae62);}if(_0x8b3d78['length']>0x0){const _0x2fcb92=_0x8b3d78[Math['randomInt'](_0x8b3d78[_0x3f2dec(0x2fe)])];this[_0x3f2dec(0x8c)](_0x2fcb92);}},Game_Event[_0x442592(0x34e)][_0x442592(0x34a)]=function(){const _0x36634f=_0x442592,_0x51ba6f=VisuMZ['GetMoveSynchTarget'](this[_0x36634f(0x372)]());this[_0x36634f(0x38d)](_0x51ba6f);},Game_Event['prototype'][_0x442592(0x19f)]=function(){const _0x30d218=_0x442592,_0x1a51fd=VisuMZ[_0x30d218(0x3f5)](this[_0x30d218(0x372)]());this[_0x30d218(0x3b0)](_0x1a51fd);},Game_Event['prototype'][_0x442592(0x2ba)]=function(){const _0x6cb70b=_0x442592;this[_0x6cb70b(0x11a)]();},Game_Event[_0x442592(0x34e)]['processMoveSynchMimic']=function(){const _0x5c0c7e=_0x442592,_0x326e6a=VisuMZ['GetMoveSynchTarget'](this['moveSynchTarget']());this['executeMoveDir8'](_0x326e6a[_0x5c0c7e(0x19a)]());},Game_Event[_0x442592(0x34e)]['processMoveSynchReverseMimic']=function(){const _0x3052cd=_0x442592,_0x4580b6=VisuMZ['GetMoveSynchTarget'](this[_0x3052cd(0x372)]());this[_0x3052cd(0x8c)](this[_0x3052cd(0x44e)](_0x4580b6[_0x3052cd(0x19a)]()));},Game_Event['prototype'][_0x442592(0x1f6)]=function(){const _0x16999c=_0x442592,_0x3047ec=VisuMZ[_0x16999c(0x3f5)](this[_0x16999c(0x372)]()),_0x4080ed=[0x0,0x7,0x8,0x9,0x4,0x0,0x6,0x1,0x2,0x3][_0x3047ec[_0x16999c(0x19a)]()];this[_0x16999c(0x8c)](_0x4080ed);},Game_Event[_0x442592(0x34e)][_0x442592(0x3f3)]=function(){const _0x3170d0=_0x442592,_0x19f8fa=VisuMZ['GetMoveSynchTarget'](this[_0x3170d0(0x372)]()),_0x39a6c7=[0x0,0x3,0x2,0x1,0x6,0x0,0x4,0x9,0x8,0x7][_0x19f8fa[_0x3170d0(0x19a)]()];this[_0x3170d0(0x8c)](_0x39a6c7);},Game_Event['prototype'][_0x442592(0x32e)]=function(){const _0x3cf860=_0x442592,_0x5cb704=VisuMZ[_0x3cf860(0x3f5)](this[_0x3cf860(0x372)]()),_0xdcc61a=_0x5cb704[_0x3cf860(0x496)]();switch(this['moveSynchType']()){case _0x3cf860(0x4fd):case'copy':this[_0x3cf860(0x421)](_0xdcc61a);break;case _0x3cf860(0x28c):case _0x3cf860(0x2f5):this['setDirection'](this[_0x3cf860(0x44e)](_0xdcc61a));break;case _0x3cf860(0x1d3):case _0x3cf860(0x2d8):case'mirror\x20horz':case _0x3cf860(0x3d9):this['setDirection']([0x0,0x7,0x8,0x9,0x4,0x0,0x6,0x1,0x2,0x3][_0xdcc61a]);break;case _0x3cf860(0x2fa):case _0x3cf860(0x543):case _0x3cf860(0x267):case _0x3cf860(0x505):this[_0x3cf860(0x421)]([0x0,0x3,0x2,0x1,0x6,0x0,0x4,0x9,0x8,0x7][_0xdcc61a]);break;default:return;}this[_0x3cf860(0x441)]();},Game_Event[_0x442592(0x34e)][_0x442592(0x24e)]=function(){const _0x703d8a=_0x442592,_0xc53807=$gameSystem[_0x703d8a(0x3ec)](this);if(!_0xc53807)return;this[_0x703d8a(0x272)](_0xc53807['x'],_0xc53807['y']),this[_0x703d8a(0x31f)](),this['setDirection'](_0xc53807['direction']),this[_0x703d8a(0x34f)]===_0xc53807[_0x703d8a(0x25f)]&&(this['_moveRouteIndex']=_0xc53807[_0x703d8a(0x16e)]);},VisuMZ[_0x442592(0x3dc)][_0x442592(0x473)]=Game_Event['prototype'][_0x442592(0x441)],Game_Event[_0x442592(0x34e)][_0x442592(0x441)]=function(){const _0x3bdd86=_0x442592;VisuMZ['EventsMoveCore']['Game_Event_update'][_0x3bdd86(0x98)](this),!Utils[_0x3bdd86(0x1fc)]()&&this[_0x3bdd86(0x4c2)]();},Game_Event['prototype']['updateMove']=function(){const _0x4bdac2=_0x442592;Game_Character[_0x4bdac2(0x34e)][_0x4bdac2(0x25b)][_0x4bdac2(0x98)](this),this[_0x4bdac2(0x297)]();},Game_Event[_0x442592(0x34e)][_0x442592(0x408)]=function(){const _0x1d4871=_0x442592;if($gameMap['isSaveEventLocations']())return!![];return this[_0x1d4871(0x438)];},Game_Event[_0x442592(0x34e)][_0x442592(0x297)]=function(){const _0xae6bac=_0x442592;if(!this[_0xae6bac(0x408)]())return;this[_0xae6bac(0x181)]();},Game_Event[_0x442592(0x34e)][_0x442592(0x181)]=function(){const _0x56690f=_0x442592;this[_0x56690f(0x462)]=!![];},Game_Event[_0x442592(0x34e)][_0x442592(0x4c2)]=function(){const _0x1ce974=_0x442592;this[_0x1ce974(0x462)]&&this[_0x1ce974(0xe0)]();},Game_Event[_0x442592(0x34e)][_0x442592(0xe0)]=function(){const _0x2fedf4=_0x442592;this['_requestSaveEventLocation']=![],$gameSystem[_0x2fedf4(0x181)](this);},Game_Event[_0x442592(0x34e)][_0x442592(0x4e6)]=function(){const _0x922e36=_0x442592;$gameSystem[_0x922e36(0x4c1)](this);},Game_Event[_0x442592(0x34e)]['getEventIconData']=function(){const _0x202ec2=_0x442592;return $gameSystem[_0x202ec2(0x45b)](this)?Game_Character['prototype'][_0x202ec2(0x45b)][_0x202ec2(0x98)](this):{'iconIndex':0x0,'bufferX':settings[_0x202ec2(0x186)][_0x202ec2(0xef)],'bufferY':settings[_0x202ec2(0x186)][_0x202ec2(0x461)],'blendMode':settings['Icon'][_0x202ec2(0x2b9)]};},Game_Event[_0x442592(0x34e)]['hasCPCs']=function(){const _0x5edf40=_0x442592;return this[_0x5edf40(0x3cf)];},VisuMZ[_0x442592(0x3dc)]['Game_Event_meetsConditionsCPC']=Game_Event[_0x442592(0x34e)][_0x442592(0xe3)],Game_Event[_0x442592(0x34e)]['meetsConditions']=function(_0xa7fa8d){const _0x1d328f=_0x442592,_0xdbea84=VisuMZ[_0x1d328f(0x3dc)][_0x1d328f(0x2b7)][_0x1d328f(0x98)](this,_0xa7fa8d);if(!_0xdbea84)return![];return this['meetsCPC'](_0xa7fa8d);},Game_Event[_0x442592(0x34e)][_0x442592(0x430)]=function(_0x3e0a88){const _0x5a37ad=_0x442592;VisuMZ[_0x5a37ad(0x3dc)][_0x5a37ad(0x1d4)]['loadCPC'](_0x3e0a88),this[_0x5a37ad(0x3cf)]=_0x3e0a88[_0x5a37ad(0x51b)][_0x5a37ad(0x2fe)]>0x0;_0x3e0a88[_0x5a37ad(0x51b)]===undefined&&VisuMZ[_0x5a37ad(0x3dc)][_0x5a37ad(0x1d4)]['loadCPC'](_0x3e0a88);if(_0x3e0a88[_0x5a37ad(0x51b)][_0x5a37ad(0x2fe)]>0x0)return $gameMap[_0x5a37ad(0x2f2)](this['_eventId'])&&VisuMZ[_0x5a37ad(0x3dc)][_0x5a37ad(0x1d4)][_0x5a37ad(0xc9)](_0x3e0a88[_0x5a37ad(0x51b)],this[_0x5a37ad(0xa3)]);return!![];},VisuMZ[_0x442592(0x3dc)]['Game_Troop_meetsConditionsCPC']=Game_Troop[_0x442592(0x34e)][_0x442592(0xe3)],Game_Troop[_0x442592(0x34e)]['meetsConditions']=function(_0x46d376){const _0x3525d4=_0x442592;var _0x5e80e6=VisuMZ['EventsMoveCore'][_0x3525d4(0x526)][_0x3525d4(0x98)](this,_0x46d376);return _0x5e80e6&&this[_0x3525d4(0x3e5)](_0x46d376);},Game_Troop['prototype'][_0x442592(0x3e5)]=function(_0xf62dbc){const _0x4c5068=_0x442592;_0xf62dbc['CPC']===undefined&&VisuMZ[_0x4c5068(0x3dc)]['CustomPageConditions'][_0x4c5068(0x4b6)](_0xf62dbc);if(_0xf62dbc[_0x4c5068(0x51b)]['length']>0x0)return VisuMZ[_0x4c5068(0x3dc)][_0x4c5068(0x1d4)][_0x4c5068(0xc9)](_0xf62dbc[_0x4c5068(0x51b)],0x0);return!![];},VisuMZ[_0x442592(0x3dc)]['Game_Event_locate']=Game_Event[_0x442592(0x34e)]['locate'],Game_Event[_0x442592(0x34e)][_0x442592(0x40b)]=function(_0x779f2f,_0x31335e){const _0x126f0c=_0x442592;if(this[_0x126f0c(0x2b0)]){const _0x3a6376=this[_0x126f0c(0x2f2)]()[_0x126f0c(0x2fc)]||'';if(_0x3a6376[_0x126f0c(0x110)](/<(?:LOCATION|START|START LOCATION):[ ](.*?)>/i)){const _0x22103b=String(RegExp['$1'])[_0x126f0c(0x315)](',')['map'](_0x4634fa=>Number(_0x4634fa));_0x779f2f+=Number(_0x22103b[0x0]||0x0)||0x0,_0x31335e+=Number(_0x22103b[0x1]||0x0)||0x0;}_0x3a6376['match'](/<(?:LOCATION|START|START LOCATION) X:[ ](.*?)>/i)&&(_0x779f2f+=Number(RegExp['$1'])),_0x3a6376[_0x126f0c(0x110)](/<(?:LOCATION|START|START LOCATION) Y:[ ](.*?)>/i)&&(_0x31335e+=Number(RegExp['$1']));}VisuMZ[_0x126f0c(0x3dc)][_0x126f0c(0x39e)][_0x126f0c(0x98)](this,_0x779f2f,_0x31335e),this['_randomHomeX']=_0x779f2f,this['_randomHomeY']=_0x31335e,this[_0x126f0c(0x297)]();},VisuMZ[_0x442592(0x3dc)][_0x442592(0x402)]=Game_Event['prototype'][_0x442592(0x4a3)],Game_Event['prototype'][_0x442592(0x4a3)]=function(){const _0x934066=_0x442592,_0x40bcd4=$gameMap[_0x934066(0x451)](this['x'],this['y'],this[_0x934066(0x450)],this[_0x934066(0x4f6)]),_0x4a9e03=_0x40bcd4*(this[_0x934066(0x1d8)]||0x0);Math['random']()>=_0x4a9e03?VisuMZ[_0x934066(0x3dc)][_0x934066(0x402)][_0x934066(0x98)](this):this['moveBackToRandomHome']();},Game_Event[_0x442592(0x34e)]['moveBackToRandomHome']=function(){const _0x3a02ce=_0x442592,_0x5f0adf=this[_0x3a02ce(0x455)](this[_0x3a02ce(0x450)]),_0x5ba537=this[_0x3a02ce(0x11f)](this[_0x3a02ce(0x4f6)]);if(Math['abs'](_0x5f0adf)>Math['abs'](_0x5ba537))this['moveStraight'](_0x5f0adf>0x0?0x4:0x6),!this[_0x3a02ce(0x48e)]()&&_0x5ba537!==0x0&&this[_0x3a02ce(0x219)](_0x5ba537>0x0?0x8:0x2);else _0x5ba537!==0x0&&(this[_0x3a02ce(0x219)](_0x5ba537>0x0?0x8:0x2),!this[_0x3a02ce(0x48e)]()&&_0x5f0adf!==0x0&&this[_0x3a02ce(0x219)](_0x5f0adf>0x0?0x4:0x6));},Game_CharacterBase[_0x442592(0x34e)]['clearAttachPictureSettings']=function(){const _0x49dee4=_0x442592;this[_0x49dee4(0x3db)]={'filename':'','type':_0x49dee4(0x4df),'blendMode':0x0,'maxSize':0x0,'offsetX':0x0,'offsetY':0x0,'scale':0x1};},Game_CharacterBase['prototype'][_0x442592(0x1fd)]=function(){const _0x398e2e=_0x442592;if(this[_0x398e2e(0x3db)]===undefined)this[_0x398e2e(0x23a)]();return this[_0x398e2e(0x3db)];},Game_CharacterBase[_0x442592(0x34e)][_0x442592(0x46c)]=function(){const _0x51e582=_0x442592;return this[_0x51e582(0x1fd)]()['filename']??'';},Game_CharacterBase[_0x442592(0x34e)][_0x442592(0x2fd)]=function(){const _0x440dc1=_0x442592;return this[_0x440dc1(0x1fd)]()[_0x440dc1(0x4df)]??_0x440dc1(0x4df);},Game_CharacterBase['prototype'][_0x442592(0x100)]=function(){const _0x39f750=_0x442592;return this[_0x39f750(0x1fd)]()['blendMode']??0x0;},Game_CharacterBase[_0x442592(0x34e)][_0x442592(0x7e)]=function(){const _0xa79386=_0x442592;return this[_0xa79386(0x1fd)]()['maxSize']??0x0;},Game_CharacterBase['prototype'][_0x442592(0x360)]=function(){const _0x4d9c01=_0x442592;return this[_0x4d9c01(0x1fd)]()[_0x4d9c01(0x532)]??0x0;},Game_CharacterBase['prototype'][_0x442592(0x207)]=function(){const _0x548724=_0x442592;return this[_0x548724(0x1fd)]()['offsetY']??0x0;},Game_CharacterBase['prototype']['attachPictureScale']=function(){const _0xf93426=_0x442592;return this[_0xf93426(0x1fd)]()[_0xf93426(0x24c)]??0x1;},VisuMZ['EventsMoveCore'][_0x442592(0x8a)]=Game_Interpreter['prototype'][_0x442592(0x282)],Game_Interpreter[_0x442592(0x34e)][_0x442592(0x282)]=function(){const _0x2eeac9=_0x442592;if(this['_waitMode']==='CallEvent'){if(window[this[_0x2eeac9(0x133)]])this[_0x2eeac9(0x33d)]='',this[_0x2eeac9(0x449)]();else return!![];}else return VisuMZ[_0x2eeac9(0x3dc)][_0x2eeac9(0x8a)]['call'](this);},VisuMZ[_0x442592(0x3dc)][_0x442592(0x316)]=Game_Interpreter[_0x442592(0x34e)]['executeCommand'],Game_Interpreter[_0x442592(0x34e)]['executeCommand']=function(){const _0x44a4f2=_0x442592,_0x1c00d3=$gameMap&&this[_0x44a4f2(0xa3)]?$gameMap[_0x44a4f2(0x2f2)](this[_0x44a4f2(0xa3)]):null;$gameTemp[_0x44a4f2(0x2e9)](_0x1c00d3);const _0x3216a2=VisuMZ[_0x44a4f2(0x3dc)][_0x44a4f2(0x316)][_0x44a4f2(0x98)](this);return $gameTemp['clearSelfTarget'](),_0x3216a2;},VisuMZ['EventsMoveCore'][_0x442592(0xf7)]=Game_Interpreter[_0x442592(0x34e)][_0x442592(0xbb)],Game_Interpreter['prototype'][_0x442592(0xbb)]=function(_0x2f1883){const _0x64e9c8=_0x442592;return $gameTemp['setLastPluginCommandInterpreter'](this),VisuMZ[_0x64e9c8(0x3dc)][_0x64e9c8(0xf7)][_0x64e9c8(0x98)](this,_0x2f1883);},Game_Interpreter[_0x442592(0x34e)][_0x442592(0x3a5)]=function(_0x2cf962){const _0x2f25b3=_0x442592;this[_0x2f25b3(0x3b2)]=_0x2cf962;const _0x1b067b=_0x2f25b3(0x127)[_0x2f25b3(0x2e6)](_0x2cf962['mapId'][_0x2f25b3(0x4ff)](0x3));this['_callEventMap']=_0x2f25b3(0x53e)+Graphics['frameCount']+'_'+this[_0x2f25b3(0xb6)](),DataManager[_0x2f25b3(0x389)](this['_callEventMap'],_0x1b067b),window[this[_0x2f25b3(0x133)]]?this[_0x2f25b3(0x449)]():this[_0x2f25b3(0x448)](_0x2f25b3(0x27c));},Game_Interpreter[_0x442592(0x34e)][_0x442592(0x449)]=function(){const _0x4acb52=_0x442592,_0x1367eb=this[_0x4acb52(0x3b2)],_0x2d2e90=window[this[_0x4acb52(0x133)]],_0x5d8138=_0x2d2e90[_0x4acb52(0x35b)][_0x1367eb[_0x4acb52(0xb6)]];if(_0x5d8138&&_0x5d8138[_0x4acb52(0x1aa)][_0x1367eb[_0x4acb52(0x344)]-0x1]){const _0x3f962e=_0x5d8138['pages'][_0x1367eb[_0x4acb52(0x344)]-0x1][_0x4acb52(0x249)];this[_0x4acb52(0x139)](_0x3f962e,this['eventId']());}window[this['_callEventMap']]=undefined,this['_callEventMap']=undefined,this[_0x4acb52(0x3b2)]=undefined;};function _0x2440(_0x25f650,_0x14e873){const _0x3f8497=_0x3f84();return _0x2440=function(_0x244016,_0x2c3799){_0x244016=_0x244016-0x6d;let _0x35d874=_0x3f8497[_0x244016];return _0x35d874;},_0x2440(_0x25f650,_0x14e873);}function Game_CPCInterpreter(){const _0x3f6357=_0x442592;this[_0x3f6357(0x327)][_0x3f6357(0x281)](this,arguments);};function _0x3f84(){const _0x3dadd5=['CPCsMet','create','Hours','execute','move','startsWith','DashingEnable','getSavedEventLocation','_data','_tileId','clearEventCache','STRUCT','moveSynchType','isSpawnedEvent','processMoveSynchMirrorVert','Sprite_Character_setTileBitmap','GetMoveSynchTarget','Visible','DIAGONAL_PATHFINDING_EVENT_LIMIT','FaceSynchAllSynchTargets','clearPageSettings','encounterProximityType','SPIN\x20CW','_forceCarrying','Boat','Game_Timer_stop','screenTileY','MULTIPLY','_targetAngle','Game_Event_moveTypeRandom','Label','pow','_PreservedEventMorphData','addChild','textSizeEx','isSaveEventLocation','EventIconDelete','Game_CharacterBase_update','locate','PosX','isTurnInPlace','setItemChoice','SCREEN','text','RangeType','tileCoordinates','processMoveCommand','setupDiagonalSupport','processMoveRouteMoveUntilStop','Direction','Game_Event_checkEventTriggerAuto','process_VisuMZ_EventsMoveCore_Switches_Variables','getPosingCharacterIndex','_EventsMoveCoreSettings','DashModifier','_visiblePlayerX','Plugin\x20Parameters\x20>\x20Event\x20Template\x20Settings\x20>\x0a','Movement','jumpAll','SelfSwitchID','setDirection','moveByInput','_dummyWindow','rotation','Seconds','_lastPluginCommandInterpreter','_erased','getDiagonalDestination','_lastAttachPictureType','updateVisibility','requestMapLoadCommonEvents','morphIntoTemplate','isDashingAndMoving','_eventIconSprite','isNearTheScreen','meetsCPC','EventAutoMovement','updateText','SILENCE','Collision','Game_Event_updateSelfMovement','Game_CharacterBase_increaseSteps','away','_saveEventLocation','Game_CharacterBase_updatePattern','deletePreservedMorphEventDataKey','_inputTime','isMoving','hasAdvancedSwitchVariable','_dragonbones','originalText','Game_CharacterBase_hasStepAnime','update','CarryPose','_saveEventLocations','BalloonOffsetY','_trigger','floor','_forceHidePlayer','setWaitMode','startCallEvent','VariableId','LOVE','isNormalPriority','_eventLabelOffsetX','reverseDir','Game_Map_refresh','_randomHomeX','distance','duration','isDashDisabled','VisuMZ_0_CoreEngine','deltaXFrom','setTileBitmap','Game_CharacterBase_screenX','processMoveRouteAnimation','EventTimerFramesSet','arc','getEventIconData','EventTimerExpireEvent','Game_CharacterBase_isDashing','EXCLAMATION','_forceHideFollower','NORMAL','BufferY','_requestSaveEventLocation','_offsetX','createShadow','_interpreter','_stepPattern','MsgPopupEvent','RandomMoveWeight','LEFT','tileHeight','Preserve','attachPictureFilename','_hidden','FollowerSetGlobalChase','_events','firstSpawnedEvent','timer','_regionRules','Game_Event_update','some','checkRegionEventTrigger','1288452pCcLbT','selfValue','isShadowVisible','isSceneMap','ShadowShowPlayer','updateTextAngle','meetActivationProximityConditions','initEventsMoveCore','fittingHeight','_needsRefresh','of\x20Preloaded\x20Maps.\x0a\x0a','switches','Game_Followers_isVisible','_pattern','onExpire','enable','remove','registerCommand','isEventTest','IconBufferX','updatePattern','setFrames','577680WFHVGf','setPlayerControlDisable','isMovementSucceeded','Game_Character_setMoveRoute','Game_CharacterBase_direction','_cpc','isSmartEventCollisionOn','setLastPluginCommandInterpreter','Name','FollowerReset','direction','processMoveRouteSetIndex','FUNC','processEraseEncounterEvents','setupRegionRestrictions','setupEventsMoveCoreEffects','Map\x20%1\x20Switch\x20%2','updateOpacity','_expireCommonEvent','requestBalloon','createSaveEventLocationData','_attachPictureSprite','variables','moveTypeRandom','Self\x20Switch\x20%1','_lastMovedDirection','checkExistingEntitiesAt','_visiblePlayerY','Game_CharacterBase_initMembers','VisibleRange','setNumberInput','updateTextPosition','setControlledFollowerID','Game_Interpreter_character','variableValid','_eventErased','updateScaleBase','hideShadows','_characterName','HEART','getSelfTarget','VisuMZ_1_MessageCore\x20is\x20required\x20to\x20run\x20','loadCPC','type','isSpawnHitboxCollisionOk','end','Region','updatePatternEventsMoveCore','getPosingCharacterPattern','Scene_Map_startEncounterEffect','ADDITIVE','createCharacterShadow','TiltRight','deleteSavedEventLocation','updateSaveEventLocation','_advancedSwitchVariable','Operation','setCommonEvent','isAirshipPassable','SlowerSpeed','_stopCount','_targetX','_eventIcon','_eventMorphData','createAttachPictureSprite','erase','visible','createSpawnedEventWithData','USER-DEFINED\x205','DEFAULT_SHIFT_Y','removeMorph','_noFollowerMovementShadow','moveAwayFromPoint','updateParallel','QUESTION','region','getAttachPictureBitmapWidth','_working','screenTileX','updateFadeIn','isTriggerIn','_moveSpeed','getInputDir8','picture','isAdvancedVariable','_scaleY','_lastSesetExitSelfSwitchesMapId','_startY','reverse','getPreservedMorphEventData','deleteEventLocation','AutoBalloon','processMoveRouteJumpToCharacter','airship','charAt','_DisablePlayerControl','BalloonOffsetX','isInVehicle','PosY','_followerChaseOff','Game_Event_findProperPageIndex','TemplateName','updateEventIconSprite','isEventRunning','startScaleY','MsgPopupFollower','_randomHomeY','setBackgroundType','processMoveRouteTeleportTo','fadeDuration','meetActivationRegionConditions','Game_CommonEvent_isActive','Game_Map_parallelCommonEvents','mimic','_diagonalSupport','padZero','screenY','_encounterHalfProximity','stop','posNt','isDiagonalDirection','vert\x20mirror','randomInt','updateMoveSynchDirection','isInstanceOfSceneMap','onMapLoaded','VehicleForbid','registerSelfEvent','isPlayerForceShown','min','isWorking','endOffsetX','map','_commonEventId','canUpdate','isObjectCharacter','_wholeDuration','_spriteOffsetY','Game_Followers_jumpAll','_noMovementShadow','updateTilt','Player','List','CPC','ceil','setupAttachPictureBitmap','TargetSwitchId','Game_Temp_setDestination','_event','Scene_Map_onMapLoadedEncErase','processMoveRouteHugWall','processMoveRouteSelfVariable','endScale','_filename','Game_Troop_meetsConditionsCPC','ARRAYSTR','All','%1Dock','_opacity','angle','standing','FavorHorz','square','convertVariableValuesInScriptCall','clear','tileWidth','offsetX','activationRegionList','Game_System_onAfterLoad','getDirectionToPoint','updateHueShift','1823556DrQLjl','Game_System_initialize','processMoveSynch','checkAdvancedSwitchVariablePresent','Passability','processMoveRouteStepFrom','updateEventMirrorSprite','$callEventMap','isCollidedWithPlayerCharacters','moveTowardPoint','ShipSpeed','Spriteset_Map_createShadow','vertical\x20mirror','_PlayerDiagonalSetting','startMapCommonEventOnTouch','setFrame','bitmap','Step2MapId','processMoveRouteBalloon','_periodicRefreshTimer','setupEvents','_spriteset','setupSpawnedEvents','defaultFontSize','encounterProximityDistance','EventTemplates','clamp','frontY','pos','endScaleY','smooth','deltaX','endAngle','ANNOYED','USER-DEFINED\x204','ARRAYFUNC','Window_EventItem_onCancel','SPIN\x20ANTICLOCKWISE','findProperPageIndex','ShadowHidePlayer','add','DashOnLadder','blendMode','forceMoveRoute','attachPictureMaxSize','isAllowEventAutoMovement','Game_Message_setItemChoice','activationProximityType','Game_Troop_meetsConditions','updateAttachPictureBitmap','setMovementSuccess','changeSpeed','onChange','iconSize','setStopFollowerChasing','Game_Message_add','Game_Interpreter_updateWaitMode','regionId','executeMoveDir8','turn180','default','createDummyWindow','_needsPeriodicRefresh','drawIcon','COLLAPSE','opacitySpeed','Game_SelfSwitches_setValue','shiftY','TiltLeft','bind','call','setDestination','isAutoBufferIcon','_checkEncounterRaw','isOnLadder','correctFacingDirection','checkEventTriggerThere','updateVS8BalloonOffsets','_activationProximity','forceCarrying','adjustDir8MovementSpeed','_eventId','_tilemap','isInsideLabelRange','despawnEventId','lastSpawnedEvent','itemPadding','Scene_Load_onLoadSuccess','_moveRoute','isShadowShrink','PathfindMobileEnabled','Game_Vehicle_isLandOk','resizeWindow','areFollowersForceHidden','FontSize','PreloadedMaps','characterPatternY','Game_Map_setupEvents','isBattleTest','includes','eventId','areFollowersForceShown','turnLeft90','Game_Enemy_meetsSwitchCondition','iconIndex','command357','SelfSwitches','setAllowEventAutoMovement','MUSIC-NOTE','characterIndex','ALLOW_LADDER_DASH','clearStepPattern','_characterIndex','LIGHT-BULB','height','_patternLocked','_mirrorSprite','terrainTag','_visibleEventY','metCPC','deltaY','executeMove','max','turnAwayFromPoint','isPassableByAnyDirection','_lastAttachPictureScale','_poseDuration','createSpawnedEvent','timerText','setOpacity','roundXWithDirection','isRunning','mainFontSize','%1Allow','createProxyWindow','none','filter','setBalloonPose','parameters','toLowerCase','isInvisibleCharacter','_forceShowFollower','processSaveEventLocation','canPass','startOffsetX','meetsConditions','Window_ScrollText_startMessage','StopAutoMoveMessages','processMoveRouteFadeIn','EventTimerFramesGain','_encounterEffectDuration','vehicle','Preloaded\x20Maps\x20and\x20add\x20in\x20Map\x20%1','processMoveRouteStepTo','Disable','setChaseOff','rangeType','BufferX','_textSprite','Game_Timer_initialize','%1:%2','Game_Event_canPass','_spriteOffsetX','autoEventIconBuffer','Game_Event_initialize','Game_Interpreter_PluginCommand','isSelfSwitch','Map%1-Event%2','MUSIC\x20NOTE','cwY','destroy','Sprite_Character_initMembers','processMoveRouteFadeOut','mapValue','attachPictureBlendMode','advancedValue','initEventsMoveCoreEffects','JSON','WalkForbid','getMapSpawnedEventData','_startScaleX','pattern','isLabelVisible','updatePosition','HMPH','_target','isActive','switch2Id','zoomScale','PostCopyJS','match','AdvancedSwitches','updateScale','isSaveEventLocations','roundYWithDirection','_text','Game_Timer_start','IconSet','_mapId','SpawnEventAtXY','updateRoutineMove','isEmptyCharacter','column','deleteIconsOnEventsDataKey','VS8','deltaYFrom','_targetScaleX','setupEventsMoveCoreCommentTags','processMoveRouteSelfSwitch','MOBILE_EVENT_LABELS','isAnyEventStarting','Game_Player_increaseSteps','clearDashing','Map%1.json','_shadowGraphic','random','Sprite_Character_update','EventForbid','isBusy','_speed','36EhLhmd','initMoveSpeed','76uvWVMP','spriteId','updateEventLabelText','_callEventMap','_pose','_followerControlID','boxWidth','processEraseEncounterSpawn','_chaseOff','setupChild','_selfTargetItemChoice','Set\x20this\x20up\x20in\x20Events\x20&\x20Movement\x20Core\x27s\x0a','adjustMoveSynchOpacityDelta','Map\x20%1\x20Variable\x20%2','executeCommand','checkSmartEventCollision','destinationX','backY','roundY','parent','_targetY','setPlayerDiagonalSetting','Game_SelfSwitches_value','Game_Player_getInputDirection','despawnRegions','offsetY','followers','createEventsMoveCoreTileMessagePopup','isMapPassable','_characterSprites','isEventOverloaded','Game_Map_event','DefaultShadow','General','needsAttachPictureUpdate','Value','Region%1','turnAwayFromCharacter','initEventsMoveCoreSettings','parallelCommonEvents','refreshIfNeeded','Step1MapId','shadowX','jump','AutoBuffer','requestAnimation','_type','onOk','Scene_Boot_onDatabaseLoaded','value','destinationY','updateSelfMovement','isMapSwitch','_eventOverloadThreshold','MoveRouteIndex','VariableGetSelfVariableID','WalkAllow','%1Forbid','clearCarrying','command108','PostMorphJS','shadowY','moveRouteIndex','isTransparent','AllForbid','Game_Timer_onExpire','Sprite_Balloon_setup','canStartLocalEvents','regionList','Game_CharacterBase_opacity','executeCommandCommonEvent','follower','hueShift','Game_Player_isMapPassable','player','onDatabaseLoaded','frontX','down','isPlayerWithinEncounterNoneEvents','AllAllow','MUSICNOTE','saveEventLocation','checkValidEventerMap','checkNeedForPeriodicRefresh','delta','%1,','Icon','_moveSynch','enemy','setupSpawn','StrictCollision','_displayY','_isCharacterSpriteSheetInvisible','EnableTurnInPlace','bufferX','setMoveSpeed','switch2Valid','_eventCache','processMoveSynchRandom','%1\x20is\x20incorrectly\x20placed\x20on\x20the\x20plugin\x20list.\x0aIt\x20is\x20a\x20Tier\x20%2\x20plugin\x20placed\x20over\x20other\x20Tier\x20%3\x20plugins.\x0aPlease\x20reorder\x20the\x20plugin\x20list\x20from\x20smallest\x20to\x20largest\x20tier\x20numbers.','ANGER','_screenActivation','checkEventTriggerEventsMoveCore','isShip','isAdvancedSwitch','_moveOnlyRegions','lastMovedDirection','startMapCommonEventOnOK','filename','MUSIC','getPose','processMoveSynchAway','VisuMZ_Setup_Preload_Map','RegionOkTarget','Game_Map_events','_clickTrigger','OffsetY','Game_Switches_setValue','resetIconsOnEventsData','prepareSpawnedEventAtTerrainTag','EnableDir8','updateMoveSynch','pages','Game_Character_processMoveCommand','Chase','eventLabelsVisible','ARRAYJSON','maxSize','SpawnEventDespawnRegions','ShadowShowAllEvents','_settings','createIconSprite','HURT','isCollidedWithEvents','EventLocationSave','EventTimerExpireClear','Step2Preserve','initMembersEventsMoveCore','_scaleBaseX','isEventsMoveCoreInvisible','checkEventsMoveCoreStringTags','...','shadowFilename','reserveCommonEvent','SelfSwitchABCD','RIGHT','%1\x27s\x20version\x20does\x20not\x20match\x20plugin\x27s.\x20Please\x20update\x20it\x20in\x20the\x20Plugin\x20Manager.','setCharacterBitmap','left','_addedHitbox','MapVariables','processMoveRouteJumpForward','Sprite_Character_setCharacterBitmap','Game_Event_checkEventTriggerTouch','bushDepth','_visibleEventX','updateEventsMoveCoreTagChanges','isPlayerControlDisabled','_arcPeak','checkEventProximity','isPlayerForceHidden','MoveAllSynchTargets','_tileExpand','mirror\x20horizontal','CustomPageConditions','innerWidth','Game_CharacterBase_characterIndex','MorphEventRemove','_randomMoveWeight','LIGHTBULB','inBattle','status','misc','setPattern','eventsXyNt','FollowerID','makeDeepCopy','IconBufferY','drawTextEx','findDirectionTo','round','gainFrames','drawText','EventLabelVisible','RegionTouch','updateTileFrame','Game_Event_event','sv\x20enemy','isValid','clearSelfTarget','despawnEverything','moveForward','processMoveRouteMoveTo','Spriteset_Map_createLowerLayer','isLandOk','OFF','slice','_fadeInDuration','processMoveSynchMirrorHorz','character','processMoveRouteStepToCharacter','resetExitSelfSwitches','advancedFunc','string','isMobileDevice','attachPictureSettings','isSpriteVS8dir','MessageText','resetSelfSwitchesForMap','VisuMZ_1_MessageCore','RIGHT\x20TO\x20LEFT','deleteIconsOnEventsData','setup','contents','SPIN\x20CCW','attachPictureOffsetY','blt','labelWindowRange','setSelfValue','_startX','findTargetSprite','isMapVariable','_priorityType','firstSpawnedEventID','spawnPreserved','checkEventTriggerTouch','fadeOut','SpawnEventDespawnAtXY','endOffset','opacityDelta','checkEventTriggerHere','Step2EventId','Game_Player_checkEventTriggerHere','moveStraight','IconBlendMode','fadeInDuration','_labelWindows','lock','OffsetX','Game_Player_isDashing','diamond','updateShadow','isOnScreen','screenX','hasMoveOnlyRegions','_screenZoomScale','deleteSavedEventLocationKey','chaseCharacter','BULB','mirror\x20horz','posEventsMoveCore','Game_CharacterBase_screenY','STR','updateStop','toUpperCase','hasCPCs','circle','ShadowLayer','_moveAllowPlayerCollision','_forceDashing','PlayerIconDelete','OperateValues','updateBitmapSmoothing','PostSpawnJS','3287650bWcpZD','fadeOutDuration','clearAttachPictureSettings','processOk','BoatSpeed','EventIconChangeForced','MsgPopupTargetTile','isDashing','Settings','checkActivationProximity','createContents','isPressed','Arc','Game_CharacterBase_setDirection','_proxyWindow','patternWidth','SpawnEventDespawnTerrainTags','list','VICTORY','_screenParallel','scale','_eventCopyData','restoreSavedEventPosition','getPosingCharacterDirection','SelfVariables','isPassable','_bypassClickStop','opacity','_lastAttachPictureMaxSize','isPlaytest','Game_Follower_chaseCharacter','FollowerSetTargetChase','Game_Party_hasEncounterNone','shift','\x22Event\x20Popup:\x20Player\x22\x20plugin\x20command!','updateMove','replace','processMoveRouteMoveRepeat','_selfEvent','pageIndex','updatePeriodicRefresh','setCharacterSpriteSheetInvisible','turnTowardPoint','ITEM','OpacitySpeed','Game_Vehicle_isMapPassable','SuccessSwitchId','mirror\x20vert','absDistance','indexOf','TargetVariableId','createShadows','characterPatternYBasic','Game_Map_isDashDisabled','CommonEventID','PreloadMaps','Ship','sqrt','setPosition','MOBILE_DIAGONAL_PATHFINDING','25065MHTkQf','onClickTrigger','processMoveRouteJumpTo','switch1Id','Vehicle','Game_CharacterBase_moveStraight','getAttachPictureBitmapHeight','refreshEventLabels','CallEvent','226230lptGsi','removeChild','EnableDashTilt','convertSelfVariableValuesInScriptCall','apply','updateWaitMode','PreMorphJS','ARRAYEVAL','forceDashing','ApplyPopupExtraSettings','processMoveCommandEventsMoveCore','setEventIconData','Hidden','right','EventID','reverse\x20mimic','EventTimerPause','log','return\x20%1','PageId','setupEventsMoveCoreNotetags','_spawnedEvents','setImage','isPreventSelfMovement','Sprite_Balloon_updatePosition','_moveRouteIndex','autosaveEventLocation','createLabelWindowForTarget','Window_Message_startMessage','KNEEL','requestRefresh','refresh','_scaleX','_scene','target','USER-DEFINED\x203','exit','PlayerMovementDiagonal','setMoveRoute','_labelWindow','fontSize','contentsOpacity','EventId','isDestinationValid','constructor','onLoadAttachPicture','parse','isLongPressed','keys','isBigCharacter','Enable','_checkRelocateNotetag','isEventClickTriggered','ZZZ','clearPose','Frames','setupPageSettings','SwitchGetSelfSwitchID','Game_Event_meetsConditionsCPC','UNTITLED','BlendMode','processMoveSynchCustom','start','3QSFApB','Game_Map_unlockEvent','hasClickTrigger','Template','executeCommonEvent','_startAngle','concat','checkEventTriggerAuto','_eventScreenY','NOTE','Allow','Game_CharacterBase_canPass','Stop','ccwY','MobileEnabled','savePreservedMorphEventDataKey','Rope','Step1EventId','activationProximityDistance','PlayerMovementChange','getInputDirection','canPassDiagonally','%1\x20is\x20missing\x20a\x20required\x20plugin.\x0aPlease\x20install\x20%2\x20into\x20the\x20Plugin\x20Manager.','setDashingEnabled','_duration','variableId','splice','PlayerAllow','horizontal\x20mirror','isPosing','_starting','RefSwitches','realMoveSpeed','endScaleX','_alwaysUpdateMove','windowPadding','boat','_screenParallelOnce','_eventScreenX','updateEventsAndMovementCore','SpawnEventDespawnEventID','LEFT\x20TO\x20RIGHT','format','LIGHT\x20BULB','turnTowardCharacter','registerSelfTarget','setupSaveEventLocations','_MapSpawnedEventData','setupMorphEvent','bufferY','addLoadListener','referEvent','createEventsMoveCoreMessagePopup','getTileExpandData','event','resetPattern','SLEEP','reverse\x20copy','disable','PopupExtra','_selfTargetNumberInput','_noEventMovementShadow','mirror\x20vertical','ShiftY','note','attachPictureType','length','restoreIconsOnEventsDataKey','updateDuration','Letter','UPPER\x20LEFT','setEventIconDataKey','_paused','_actuallyMoving','SPIN\x20CLOCKWISE','Game_Map_update','code','PreCopyJS','isRegionForbidPass','setHue','ConvertParams','processMoveRouteTeleportToCharacter','Game_Event_isCollidedWithPlayerCharacters','resetFontSettings','_offsetY','startOffset','visibleRange','ship','push','split','Game_Interpreter_executeCommand','Game_Variables_setValue','eraseEvent','updateAttachPictureSprite','Game_Player_checkEventTriggerThere','VehicleDock','_displayX','pause','radius','refreshBushDepth','outlineColor','mapId','_SavedEventLocations','_lastAttachPictureFilename','initMembers','Dock','setMapValue','initialize','scrolledY','loadSystem','onLoadSuccess','unlockEvent','_character','updateSpritePosition','processMoveSynchDirection','Game_Variables_value','getLastPluginCommandInterpreter','AdvancedVariables','name','clearDestination','isAllowCharacterTilt','page','iconHeight','increaseSteps','_cacheVisibility','_frames','setupSpawnTest','eventsXy','Game_Vehicle_initMoveSpeed','_waitMode','determineCommonEventsWithCPC','width','_spawnPreserved','adjustX','_shadowSprite','anchor','pageId','isOnRope','Setting','startScaleX','_scaleBaseY','_eventSpawnData','processMoveSynchApproach','updatePose','process_VisuMZ_EventsMoveCore_LoadTemplateMaps','lineHeight','prototype','_pageIndex','fontFace','processMoveRoutePatternLock','forced','getEventIconIndex','unlock','_eventPageIndex','RefVariables','_eventLabelOffsetY','_forceShowPlayer','Game_CharacterBase_realMoveSpeed','morphInto','events','_EventIcons','clone','COBWEB','endOffsetY','attachPictureOffsetX','startMessage','SwitchId','MsgDuration','labelWindowRangeType','hasDragonbones','hasEncounterHalf','loadEnemy','processMoveRouteMoveToCharacter','MapSwitches','You\x20do\x20not\x20have\x20Map\x20%1\x20added\x20to\x20the\x20list\x0a','Window_EventItem_onOk','isRegionAllowPass','Game_CharacterBase_isTransparent','SpawnEventDespawnEverything','trim','EventLabelRefresh','findDiagonalDirectionTo','moveSynchTarget','isJumping','Airship','_seconds','setEventLabelsVisible','setupFollowerVisibilityOverrides','resume','template','EventTimerResume','isBoat','UPPER\x20RIGHT','moveDiagonally','hasStepAnime','IconIndex','front','_encounterNoneProximity','abs','_commonEvents','isStopFollowerChasing','581402VaIFCW','_active','ShowShadows','labelWindowText','loadDataFile','EventLocationCreate','setupPlayerVisibilityOverrides','canMove','moveTowardCharacter','SWEAT','ccwX','Game_Player_executeMove','isTile','startMapCommonEventOnOKTarget','initFollowerController','useCarryPoseForIcons','lastSpawnedEventID','Scene_Map_createDisplayObjects','isSupportDiagonalMovement','PlayerForbid','isAirship','Self\x20Variable\x20%1','Game_Event_start','isDashingEnabled','Game_Event_clearPageSettings','Game_Event_locate','return\x200','MapId','_reflection','createLowerLayer','TerrainTags','isPlayerWithinEncounterHalfEvents','pluginCommandCallEvent','_eventOverload','description','checkCollisionKeywords','clearSpriteOffsets','createTextSprite','Game_Party_hasEncounterHalf','RegionOk','SPIN\x20COUNTERCLOCKWISE','getDirectionFromPoint','turnRight90','moveAwayFromCharacter','createBitmap','_callEventData','iconWidth','TileY','_customZ','copy','StopAutoMoveEvents','Toggle','_startScaleY','directionOnLadderSpriteVS8dir','startScale','_selfTarget','characterName','hasEncounterNone','Game_Event_updateParallel','Window_NumberInput_start','removeTemporaryMapSpawnedEvents','spawnEventId','FollowerIndex','_realX','setValue','isRegionDockable','_spawnData','Game_Switches_value','row','%1%2','MapID','_fadeOutDuration','isSelfVariable','_screenActivated','_CPCs','setDiagonalDirection','FollowerSetControl','SwitchGetSelfSwitchABCD','VisuMZ_2_DragonbonesUnion','prepareSpawnedEventAtRegion','_fadeOutStart','isEventClickStopValid','Game_Event_meetsConditions','isVisible','horz\x20mirror','processDrawIcon','_attachPicture','EventsMoveCore','TileX','characterPatternYVS8','meetsSwitchCondition','VisibleEventLabels','ERROR:\x20Map\x20%1\x20has\x20not\x20been\x20preloaded\x20for\x20remove\x20usage.','setPose','_realY','SPIN\x20ACW'];_0x3f84=function(){return _0x3dadd5;};return _0x3f84();}Game_CPCInterpreter[_0x442592(0x34e)]=Object[_0x442592(0x3e6)](Game_Interpreter[_0x442592(0x34e)]),Game_CPCInterpreter['prototype']['constructor']=Game_CPCInterpreter,Game_CPCInterpreter[_0x442592(0x34e)][_0x442592(0x530)]=function(){const _0x218a98=_0x442592;Game_Interpreter['prototype'][_0x218a98(0x530)]['call'](this),this[_0x218a98(0x491)]=![];},Game_CPCInterpreter[_0x442592(0x34e)][_0x442592(0x3e8)]=function(){const _0x2f449b=_0x442592;while(this[_0x2f449b(0xd5)]()){this[_0x2f449b(0x13e)]();}},Game_CPCInterpreter[_0x442592(0x34e)][_0x442592(0x2c0)]=function(_0x441a5f){const _0x38fa4d=_0x442592;while(this['isRunning']()){this[_0x38fa4d(0x176)](_0x441a5f);}},Game_CPCInterpreter['prototype'][_0x442592(0x176)]=function(_0x33931c){const _0x4123d6=_0x442592,_0x48d309=_0x33931c;$gameTemp[_0x4123d6(0x2e9)](_0x48d309);const _0x59ff23=VisuMZ[_0x4123d6(0x3dc)][_0x4123d6(0x316)][_0x4123d6(0x98)](this);return $gameTemp[_0x4123d6(0x1ed)](),_0x59ff23;},Game_CPCInterpreter['prototype'][_0x442592(0x16b)]=function(_0xeafa5f){const _0x10f537=_0x442592;return Game_Interpreter[_0x10f537(0x34e)][_0x10f537(0x16b)][_0x10f537(0x98)](this,_0xeafa5f),this['_comments'][_0x10f537(0x474)](_0x2b5464=>_0x2b5464[_0x10f537(0x110)](/<(?:CONDITION|CONDITIONS) MET>/i))&&(this[_0x10f537(0x491)]=!![]),!![];},VisuMZ[_0x442592(0x3dc)][_0x442592(0x4bd)]=Scene_Map[_0x442592(0x34e)]['startEncounterEffect'],Scene_Map['prototype']['startEncounterEffect']=function(){const _0xf87e1f=_0x442592;VisuMZ[_0xf87e1f(0x3dc)][_0xf87e1f(0x4bd)][_0xf87e1f(0x98)](this),this[_0xf87e1f(0x54c)][_0xf87e1f(0x4b1)]();},VisuMZ[_0x442592(0x3dc)][_0x442592(0xa9)]=Scene_Load[_0x442592(0x34e)][_0x442592(0x32a)],Scene_Load[_0x442592(0x34e)]['onLoadSuccess']=function(){const _0x95d5ab=_0x442592;if($gameMap)$gameMap[_0x95d5ab(0x3ef)]();VisuMZ[_0x95d5ab(0x3dc)][_0x95d5ab(0xa9)][_0x95d5ab(0x98)](this);},VisuMZ['EventsMoveCore']['Game_System_onAfterLoad']=Game_System[_0x442592(0x34e)]['onAfterLoad'],Game_System[_0x442592(0x34e)]['onAfterLoad']=function(){const _0x6854c8=_0x442592;VisuMZ['EventsMoveCore'][_0x6854c8(0x534)][_0x6854c8(0x98)](this);if($gameMap)$gameMap[_0x6854c8(0x3ef)]();},VisuMZ[_0x442592(0x3dc)][_0x442592(0xfd)]=Sprite_Character['prototype'][_0x442592(0x324)],Sprite_Character[_0x442592(0x34e)][_0x442592(0x324)]=function(){const _0x429398=_0x442592;VisuMZ[_0x429398(0x3dc)][_0x429398(0xfd)]['call'](this),this[_0x429398(0x1b9)](),this['createAttachPictureSprite'](),this[_0x429398(0x1b3)]();},Sprite_Character[_0x442592(0x34e)][_0x442592(0x1b9)]=function(){this['_shadowOpacity']=0xff,this['_isCharacterSpriteSheetInvisible']=![];},Sprite_Character[_0x442592(0x34e)][_0x442592(0x1fe)]=function(){const _0x5467f4=_0x442592;return this['_characterName']&&this['_characterName'][_0x5467f4(0x110)](/\[VS8\]/i);},Sprite_Character[_0x442592(0x34e)][_0x442592(0x9a)]=function(){const _0x1c0249=_0x442592;return this[_0x1c0249(0x1fe)]()&&VisuMZ[_0x1c0249(0x3dc)][_0x1c0249(0x240)][_0x1c0249(0x11e)][_0x1c0249(0x15c)];},Sprite_Character[_0x442592(0x34e)][_0x442592(0x4cc)]=function(){const _0x319ae9=_0x442592;this[_0x319ae9(0x4a1)]=new Sprite(),this[_0x319ae9(0x4a1)][_0x319ae9(0x343)]['x']=0.5,this['_attachPictureSprite'][_0x319ae9(0x343)]['y']=0x1,this[_0x319ae9(0x406)](this[_0x319ae9(0x4a1)]),this['updateAttachPictureSprite']();},Sprite_Character[_0x442592(0x34e)][_0x442592(0x1b3)]=function(){const _0x5888c6=_0x442592;this[_0x5888c6(0x42e)]=new Sprite(),this[_0x5888c6(0x42e)][_0x5888c6(0x547)]=ImageManager[_0x5888c6(0x329)](_0x5888c6(0x117)),this['_eventIconSprite'][_0x5888c6(0x547)][_0x5888c6(0x70)]=![],this[_0x5888c6(0x42e)][_0x5888c6(0x546)](0x0,0x0,0x0,0x0),this[_0x5888c6(0x42e)][_0x5888c6(0x343)]['x']=0.5,this[_0x5888c6(0x42e)][_0x5888c6(0x343)]['y']=0x1,this[_0x5888c6(0x406)](this[_0x5888c6(0x42e)]);},VisuMZ['EventsMoveCore'][_0x442592(0x12a)]=Sprite_Character[_0x442592(0x34e)][_0x442592(0x441)],Sprite_Character[_0x442592(0x34e)][_0x442592(0x441)]=function(){const _0x353c5c=_0x442592;VisuMZ[_0x353c5c(0x3dc)][_0x353c5c(0x12a)][_0x353c5c(0x98)](this),this[_0x353c5c(0x2e3)]();},Sprite_Character[_0x442592(0x34e)][_0x442592(0x42a)]=function(){const _0x38ebdc=_0x442592;Sprite[_0x38ebdc(0x34e)][_0x38ebdc(0x42a)]['call'](this),this[_0x38ebdc(0x1bb)]()&&(this[_0x38ebdc(0x4ce)]=![]);},Sprite_Character[_0x442592(0x34e)]['isEventsMoveCoreInvisible']=function(){const _0x44d468=_0x442592;if(this[_0x44d468(0x353)]()>0x0)return![];if(this[_0x44d468(0x32c)]){if(this['_character'][_0x44d468(0x46c)]()!=='')return![];}return this[_0x44d468(0x11b)]()||this['_character']&&this[_0x44d468(0x32c)]['isTransparent']();},Sprite_Character[_0x442592(0x34e)][_0x442592(0x236)]=function(){const _0x13b2a1=_0x442592;if(!this[_0x13b2a1(0x547)])return;this[_0x13b2a1(0x547)][_0x13b2a1(0x70)]=!!VisuMZ[_0x13b2a1(0x3dc)]['Settings'][_0x13b2a1(0x41e)]['BitmapSmoothing'];},Sprite_Character['prototype']['updateEventsAndMovementCore']=function(){const _0x3db018=_0x442592;this[_0x3db018(0x4b0)](),this[_0x3db018(0x518)](),this[_0x3db018(0x221)](),this['updateEventIconSprite'](),this['updateEventCustomZ'](),this[_0x3db018(0x53d)](),this[_0x3db018(0x319)]();},VisuMZ['EventsMoveCore'][_0x442592(0x3f4)]=Sprite_Character['prototype'][_0x442592(0x456)],Sprite_Character['prototype'][_0x442592(0x456)]=function(){const _0x279942=_0x442592;VisuMZ[_0x279942(0x3dc)][_0x279942(0x3f4)][_0x279942(0x98)](this),this['bitmap']['addLoadListener'](this[_0x279942(0x236)][_0x279942(0x97)](this));},Sprite_Character['prototype'][_0x442592(0x1e9)]=function(){const _0x15c20b=_0x442592,_0x142175=this[_0x15c20b(0x3ee)],_0x462eef=this[_0x15c20b(0x247)](),_0x308980=this['patternHeight'](),_0x66646e=(Math[_0x15c20b(0x446)](_0x142175/0x80)%0x2*0x8+_0x142175%0x8)*_0x462eef,_0x15c19a=Math['floor'](_0x142175%0x100/0x8)%0x10*_0x308980,_0x317790=this[_0x15c20b(0x2f1)]();let _0x38d5e1=_0x66646e,_0x1cabae=_0x15c19a,_0xc611b7=_0x462eef,_0x25288c=_0x308980;_0x317790['up']&&_0x317790['up']>0x0&&(_0x1cabae-=_0x308980*_0x317790['up'],_0x25288c+=_0x308980*_0x317790['up']),_0x317790[_0x15c20b(0x17d)]&&_0x317790[_0x15c20b(0x17d)]>0x0&&(_0x25288c+=_0x308980*_0x317790['down']),_0x317790[_0x15c20b(0x1c4)]&&_0x317790[_0x15c20b(0x1c4)]>0x0&&(_0x38d5e1-=_0x462eef*_0x317790[_0x15c20b(0x1c4)],_0xc611b7+=_0x462eef*_0x317790['left']),_0x317790[_0x15c20b(0x28a)]&&_0x317790[_0x15c20b(0x28a)]>0x0&&(_0xc611b7+=_0x462eef*_0x317790[_0x15c20b(0x28a)]),this[_0x15c20b(0x546)](_0x38d5e1,_0x1cabae,_0xc611b7,_0x25288c);},Sprite_Character['prototype'][_0x442592(0x2f1)]=function(){const _0x11cae0=_0x442592;return this[_0x11cae0(0x32c)]?this[_0x11cae0(0x32c)][_0x11cae0(0x1d2)]||{}:{};},VisuMZ[_0x442592(0x3dc)][_0x442592(0x1c8)]=Sprite_Character['prototype'][_0x442592(0x1c3)],Sprite_Character[_0x442592(0x34e)]['setCharacterBitmap']=function(){const _0x1af2df=_0x442592;VisuMZ[_0x1af2df(0x3dc)]['Sprite_Character_setCharacterBitmap'][_0x1af2df(0x98)](this),this[_0x1af2df(0x547)][_0x1af2df(0x2ee)](this['updateBitmapSmoothing'][_0x1af2df(0x97)](this)),this['_isCharacterSpriteSheetInvisible']=ImageManager['isInvisibleCharacter'](this[_0x1af2df(0x4b2)]),this[_0x1af2df(0x18c)]&&this[_0x1af2df(0x547)]['addLoadListener'](this[_0x1af2df(0x261)]['bind'](this));},Sprite_Character[_0x442592(0x34e)][_0x442592(0x261)]=function(){const _0xe0047f=_0x442592;this[_0xe0047f(0x547)]=new Bitmap(this['bitmap']['width'],this[_0xe0047f(0x547)][_0xe0047f(0xc4)]);},VisuMZ['EventsMoveCore']['Sprite_Character_characterPatternY']=Sprite_Character['prototype'][_0x442592(0xb2)],Sprite_Character[_0x442592(0x34e)]['characterPatternY']=function(){const _0x5b6ba3=_0x442592;return this[_0x5b6ba3(0x1fe)]()?this['characterPatternYVS8']():this['characterPatternYBasic']();},Sprite_Character[_0x442592(0x34e)][_0x442592(0x3de)]=function(){const _0x5accca=_0x442592,_0x29025f=this[_0x5accca(0x32c)][_0x5accca(0x496)]();let _0x245616=[0x2,0x2,0x2,0x4,0x4,0x2,0x6,0x6,0x8,0x8];return this[_0x5accca(0x32c)][_0x5accca(0xc6)]&&(_0x245616=[0x2,0x4,0x2,0x2,0x6,0x2,0x4,0x8,0x8,0x6]),(_0x245616[_0x29025f]-0x2)/0x2;},Sprite_Character[_0x442592(0x34e)][_0x442592(0x26c)]=function(){const _0x507db0=_0x442592;let _0x3e2128=this[_0x507db0(0x32c)][_0x507db0(0x496)]();if(this[_0x507db0(0x32c)]['_mirrorSprite']){if(_0x3e2128===0x4)_0x3e2128=0x6;else _0x3e2128===0x6&&(_0x3e2128=0x4);}return(_0x3e2128-0x2)/0x2;},Sprite_Character['prototype'][_0x442592(0x4b0)]=function(){this['scale']['x']=this['_character']['_scaleX']??0x1,this['scale']['y']=this['_character']['_scaleY']??0x1;},Sprite_Character[_0x442592(0x34e)][_0x442592(0x518)]=function(){const _0x3df7cf=_0x442592;if(!VisuMZ['EventsMoveCore'][_0x3df7cf(0x240)][_0x3df7cf(0x41e)][_0x3df7cf(0x27f)])return;this[_0x3df7cf(0x424)]=0x0;if(this[_0x3df7cf(0x334)]()){const _0x3a782e=VisuMZ[_0x3df7cf(0x3dc)][_0x3df7cf(0x240)][_0x3df7cf(0x41e)],_0x5dac73=this[_0x3df7cf(0x32c)][_0x3df7cf(0x496)]();let _0x308570=0x0;if([0x1,0x4,0x7][_0x3df7cf(0xb5)](_0x5dac73))_0x308570=_0x3a782e[_0x3df7cf(0x96)];if([0x3,0x6,0x9][_0x3df7cf(0xb5)](_0x5dac73))_0x308570=_0x3a782e[_0x3df7cf(0x4c0)];[0x2,0x8][_0x3df7cf(0xb5)](_0x5dac73)&&(_0x308570=[-_0x3a782e['TiltVert'],0x0,_0x3a782e['TiltVert']][this['_character'][_0x3df7cf(0x107)]()]);if(this[_0x3df7cf(0x3a1)])_0x308570*=-0x1;this[_0x3df7cf(0x424)]=_0x308570;}},Sprite_Character[_0x442592(0x34e)][_0x442592(0x334)]=function(){const _0x11bda0=_0x442592;if(this[_0x11bda0(0x43e)])return![];return this[_0x11bda0(0x32c)][_0x11bda0(0x42d)]()&&!this[_0x11bda0(0x32c)]['isOnLadder']()&&!this[_0x11bda0(0x32c)]['isPosing']()&&this[_0x11bda0(0x353)]()===0x0;},Sprite_Character[_0x442592(0x34e)][_0x442592(0x221)]=function(){const _0x54a6fd=_0x442592;if(!this['_shadowSprite'])return;this[_0x54a6fd(0x342)]['x']=this['_character'][_0x54a6fd(0x15a)](),this[_0x54a6fd(0x342)]['y']=this[_0x54a6fd(0x32c)][_0x54a6fd(0x16d)](),this[_0x54a6fd(0x342)][_0x54a6fd(0x253)]=this[_0x54a6fd(0x253)],this[_0x54a6fd(0x342)]['visible']=this[_0x54a6fd(0x32c)][_0x54a6fd(0x478)](),this[_0x54a6fd(0x342)][_0x54a6fd(0x46d)]=this[_0x54a6fd(0x46d)];if(this[_0x54a6fd(0x32c)][_0x54a6fd(0xab)]())this[_0x54a6fd(0x342)][_0x54a6fd(0x24c)]['x']=Math[_0x54a6fd(0xcc)](0x0,this['_shadowSprite'][_0x54a6fd(0x24c)]['x']-0.1),this[_0x54a6fd(0x342)]['scale']['y']=Math[_0x54a6fd(0xcc)](0x0,this[_0x54a6fd(0x342)][_0x54a6fd(0x24c)]['y']-0.1);else{if(this[_0x54a6fd(0x342)][_0x54a6fd(0x24c)]['x']!==this['scale']['x']){if(this[_0x54a6fd(0x342)][_0x54a6fd(0x24c)]['x']>this[_0x54a6fd(0x24c)]['x'])this['_shadowSprite'][_0x54a6fd(0x24c)]['x']=Math[_0x54a6fd(0x50d)](this[_0x54a6fd(0x342)][_0x54a6fd(0x24c)]['x']+0.1,this[_0x54a6fd(0x24c)]['x']);if(this[_0x54a6fd(0x342)][_0x54a6fd(0x24c)]['x']<this[_0x54a6fd(0x24c)]['x'])this[_0x54a6fd(0x342)][_0x54a6fd(0x24c)]['x']=Math[_0x54a6fd(0xcc)](this[_0x54a6fd(0x342)][_0x54a6fd(0x24c)]['x']-0.1,this[_0x54a6fd(0x24c)]['x']);}if(this['_shadowSprite'][_0x54a6fd(0x24c)]['y']!==this[_0x54a6fd(0x24c)]['y']){if(this[_0x54a6fd(0x342)]['scale']['y']>this[_0x54a6fd(0x24c)]['y'])this['_shadowSprite'][_0x54a6fd(0x24c)]['y']=Math[_0x54a6fd(0x50d)](this[_0x54a6fd(0x342)]['scale']['y']+0.1,this[_0x54a6fd(0x24c)]['y']);if(this[_0x54a6fd(0x342)][_0x54a6fd(0x24c)]['y']<this['scale']['y'])this[_0x54a6fd(0x342)][_0x54a6fd(0x24c)]['y']=Math[_0x54a6fd(0xcc)](this[_0x54a6fd(0x342)][_0x54a6fd(0x24c)]['y']-0.1,this['scale']['y']);}}},Sprite_Character[_0x442592(0x34e)][_0x442592(0x4f2)]=function(){const _0x10b6a7=_0x442592;if(!this[_0x10b6a7(0x42e)])return;const _0x418fcf=this[_0x10b6a7(0x42e)],_0x539939=this[_0x10b6a7(0x353)]();if(_0x539939<=0x0)return _0x418fcf[_0x10b6a7(0x546)](0x0,0x0,0x0,0x0);else{const _0x2ea9b2=ImageManager['iconWidth'],_0x597d51=ImageManager['iconHeight'],_0x4706e1=_0x539939%0x10*_0x2ea9b2,_0xe66692=Math[_0x10b6a7(0x446)](_0x539939/0x10)*_0x597d51;_0x418fcf['setFrame'](_0x4706e1,_0xe66692,_0x2ea9b2,_0x597d51),this[_0x10b6a7(0x4ce)]=!![];}const _0x4fd098=this[_0x10b6a7(0x32c)][_0x10b6a7(0x45b)]();this[_0x10b6a7(0x9a)]()?this[_0x10b6a7(0xf5)](_0x418fcf):(_0x418fcf['x']=_0x4fd098?_0x4fd098[_0x10b6a7(0x18e)]:0x0,_0x418fcf['y']=_0x4fd098?-this['height']+_0x4fd098[_0x10b6a7(0x2ed)]:0x0),_0x418fcf[_0x10b6a7(0x7c)]=_0x4fd098?_0x4fd098[_0x10b6a7(0x7c)]:0x0,this[_0x10b6a7(0x27e)](_0x418fcf),this[_0x10b6a7(0x406)](_0x418fcf),_0x418fcf[_0x10b6a7(0x424)]=-this[_0x10b6a7(0x424)];},Sprite_Character[_0x442592(0x34e)][_0x442592(0xf5)]=function(_0x3f9f40){const _0x438f73=_0x442592;_0x3f9f40['x']=0x0,_0x3f9f40['y']=-this[_0x438f73(0xc4)]+this[_0x438f73(0xc4)]*0x2/0x5,this['_character'][_0x438f73(0x107)]()!==0x1&&(_0x3f9f40['y']+=0x1);},Sprite_Character[_0x442592(0x34e)]['getEventIconIndex']=function(){const _0x4e522d=_0x442592;if(!this[_0x4e522d(0x32c)])return 0x0;if(this['_character']['_erased'])return 0x0;const _0x3a0fd6=this['_character'][_0x4e522d(0x45b)]();return _0x3a0fd6?_0x3a0fd6[_0x4e522d(0xba)]||0x0:0x0;},Sprite_Character[_0x442592(0x34e)]['updateEventCustomZ']=function(){const _0x5aaf6a=_0x442592;if(!this[_0x5aaf6a(0x32c)])return;if(this['_character'][_0x5aaf6a(0x3b5)]===undefined)return;if(this[_0x5aaf6a(0x32c)][_0x5aaf6a(0x3b5)]===![])return;this['z']=this['_character'][_0x5aaf6a(0x3b5)],this[_0x5aaf6a(0x342)]&&(this['z']<0x0?this[_0x5aaf6a(0x342)]['z']=this['z']-0x1:this['_shadowSprite']['z']=0x0);},Sprite_Character[_0x442592(0x34e)]['updateEventMirrorSprite']=function(){const _0x252ee1=_0x442592;if(!this['_character'])return;let _0x4243f0=!!this[_0x252ee1(0x32c)][_0x252ee1(0xc6)];this['scale']['x']=Math['abs'](this[_0x252ee1(0x24c)]['x'])*(_0x4243f0?-0x1:0x1);},Sprite_Character[_0x442592(0x34e)][_0x442592(0x319)]=function(){const _0x1918b5=_0x442592;if(!this[_0x1918b5(0x4a1)])return;if(!this[_0x1918b5(0x32c)])return;this[_0x1918b5(0x51d)](),this[_0x1918b5(0x83)]();},Sprite_Character[_0x442592(0x34e)]['setupAttachPictureBitmap']=function(){const _0x40d12b=_0x442592;if(!this['needsAttachPictureUpdate']())return;const _0x48d106=this['_character'][_0x40d12b(0x1fd)]();this[_0x40d12b(0x323)]=_0x48d106[_0x40d12b(0x19c)],this[_0x40d12b(0x429)]=_0x48d106['type'],this['_lastAttachPictureMaxSize']=_0x48d106['maxSize'],this[_0x40d12b(0xcf)]=_0x48d106[_0x40d12b(0x24c)];if(_0x48d106[_0x40d12b(0x19c)]!==''){if(_0x48d106['type']===_0x40d12b(0x188)){const _0x49b740=ImageManager[_0x40d12b(0x367)](_0x48d106['filename']);_0x49b740[_0x40d12b(0x2ee)](this[_0x40d12b(0x2aa)]['bind'](this,_0x49b740));}else{if(_0x48d106['type']===_0x40d12b(0x1eb)){const _0x4d0542=ImageManager['loadSvEnemy'](_0x48d106[_0x40d12b(0x19c)]);_0x4d0542[_0x40d12b(0x2ee)](this[_0x40d12b(0x2aa)][_0x40d12b(0x97)](this,_0x4d0542));}else{const _0x1b5818=ImageManager['loadPicture'](_0x48d106[_0x40d12b(0x19c)]);_0x1b5818[_0x40d12b(0x2ee)](this[_0x40d12b(0x2aa)][_0x40d12b(0x97)](this,_0x1b5818));}}}else this['_attachPictureSprite'][_0x40d12b(0x547)]=new Bitmap(0x1,0x1);},Sprite_Character[_0x442592(0x34e)][_0x442592(0x83)]=function(){const _0x1a0be2=_0x442592,_0x50609b=this[_0x1a0be2(0x4a1)];_0x50609b['x']=this['_character']['attachPictureOffsetX'](),_0x50609b['y']=this[_0x1a0be2(0x32c)][_0x1a0be2(0x207)](),_0x50609b['blendMode']=this[_0x1a0be2(0x32c)][_0x1a0be2(0x100)]();},Sprite_Character[_0x442592(0x34e)][_0x442592(0x152)]=function(){const _0x39390c=_0x442592,_0x2cad55=this[_0x39390c(0x32c)][_0x39390c(0x1fd)]();if(_0x2cad55){if(this['_lastAttachPictureFilename']!==_0x2cad55[_0x39390c(0x19c)])return!![];if(this[_0x39390c(0x429)]!==_0x2cad55[_0x39390c(0x4b7)])return!![];if(this[_0x39390c(0x254)]!==_0x2cad55[_0x39390c(0x1af)])return!![];if(this[_0x39390c(0xcf)]!==_0x2cad55[_0x39390c(0x24c)])return!![];}return![];},Sprite_Character[_0x442592(0x34e)]['onLoadAttachPicture']=function(_0x172be6){const _0x766f44=_0x442592,_0x1a8875=this[_0x766f44(0x4a1)];_0x1a8875[_0x766f44(0x547)]=_0x172be6;const _0x58bdfc=this[_0x766f44(0x32c)][_0x766f44(0x1fd)](),_0x2bd3d1=_0x58bdfc['maxSize'],_0x428b05=_0x58bdfc[_0x766f44(0x24c)];let _0x2a5ccc=0x1;if(_0x2bd3d1>0x0){let _0x19c45e=this[_0x766f44(0x4d8)]()||0x1,_0x382790=this['getAttachPictureBitmapHeight']()||0x1;const _0x38bbd3=Math[_0x766f44(0xcc)](0x1,_0x19c45e,_0x382790);_0x2a5ccc=_0x2bd3d1/_0x38bbd3;}_0x2a5ccc*=_0x428b05,_0x2a5ccc!==0x1&&(this[_0x766f44(0x4a1)][_0x766f44(0x547)][_0x766f44(0x70)]=!![]),_0x1a8875['scale']['x']=_0x2a5ccc,_0x1a8875['scale']['y']=_0x2a5ccc,this[_0x766f44(0x4ce)]=!![],this[_0x766f44(0x83)]();},Sprite_Character[_0x442592(0x34e)][_0x442592(0x4d8)]=function(){const _0x3a2994=_0x442592,_0x39e828=this[_0x3a2994(0x4a1)];if(!_0x39e828)return 0x0;return _0x39e828['bitmap'][_0x3a2994(0x33f)];},Sprite_Character['prototype'][_0x442592(0x27a)]=function(){const _0x22f79c=_0x442592,_0x1bb240=this['_attachPictureSprite'];if(!_0x1bb240)return 0x0;return _0x1bb240[_0x22f79c(0x547)]['height'];},VisuMZ[_0x442592(0x3dc)][_0x442592(0x172)]=Sprite_Balloon['prototype'][_0x442592(0x204)],Sprite_Balloon[_0x442592(0x34e)]['setup']=function(_0x1ea6b6,_0x19dd2d){const _0x2e07be=_0x442592;VisuMZ[_0x2e07be(0x3dc)][_0x2e07be(0x172)][_0x2e07be(0x98)](this,_0x1ea6b6,_0x19dd2d),VisuMZ[_0x2e07be(0x3dc)][_0x2e07be(0x240)][_0x2e07be(0x11e)][_0x2e07be(0x4e7)]&&this[_0x2e07be(0x10b)][_0x2e07be(0x32c)][_0x2e07be(0xdb)](_0x19dd2d,this['_duration']);},VisuMZ[_0x442592(0x3dc)][_0x442592(0x295)]=Sprite_Balloon[_0x442592(0x34e)][_0x442592(0x109)],Sprite_Balloon[_0x442592(0x34e)][_0x442592(0x109)]=function(){const _0x231243=_0x442592;VisuMZ['EventsMoveCore'][_0x231243(0x295)][_0x231243(0x98)](this),this[_0x231243(0x9f)]();},Sprite_Balloon[_0x442592(0x34e)][_0x442592(0x9f)]=function(){const _0x4482de=_0x442592;this[_0x4482de(0x10b)]['_character']['isSpriteVS8dir']()&&(this['x']+=VisuMZ['EventsMoveCore'][_0x4482de(0x240)][_0x4482de(0x11e)][_0x4482de(0x4ec)],this['y']+=VisuMZ[_0x4482de(0x3dc)][_0x4482de(0x240)][_0x4482de(0x11e)][_0x4482de(0x444)]);},Sprite_Timer['prototype'][_0x442592(0x3b1)]=function(){const _0x5677e1=_0x442592;this[_0x5677e1(0x547)]=new Bitmap(Math[_0x5677e1(0x1e4)](Graphics[_0x5677e1(0x136)]/0x2),0x30),this['bitmap'][_0x5677e1(0x350)]=this[_0x5677e1(0x350)](),this[_0x5677e1(0x547)][_0x5677e1(0x2a5)]=this['fontSize'](),this['bitmap'][_0x5677e1(0x320)]=ColorManager['outlineColor']();},Sprite_Timer[_0x442592(0x34e)][_0x442592(0xd2)]=function(){const _0x2db9cd=_0x442592,_0x57dac2=Math[_0x2db9cd(0x446)](this[_0x2db9cd(0x375)]/0x3c/0x3c),_0x5db26e=Math['floor'](this[_0x2db9cd(0x375)]/0x3c)%0x3c,_0xe1c59a=this['_seconds']%0x3c;let _0xb09e91=_0x5db26e[_0x2db9cd(0x4ff)](0x2)+':'+_0xe1c59a['padZero'](0x2);if(_0x57dac2>0x0)_0xb09e91=_0x2db9cd(0xf2)['format'](_0x57dac2,_0xb09e91);return _0xb09e91;};function Sprite_EventLabel(){const _0x2f4767=_0x442592;this[_0x2f4767(0x327)](...arguments);}Sprite_EventLabel[_0x442592(0x34e)]=Object[_0x442592(0x3e6)](Sprite[_0x442592(0x34e)]),Sprite_EventLabel[_0x442592(0x34e)][_0x442592(0x2a9)]=Sprite_EventLabel,Sprite_EventLabel[_0x442592(0x34e)][_0x442592(0x327)]=function(_0x141219){const _0x1336e6=_0x442592;this[_0x1336e6(0x520)]=_0x141219,Sprite[_0x1336e6(0x34e)][_0x1336e6(0x327)][_0x1336e6(0x98)](this),this['initMembers'](),this[_0x1336e6(0xd8)]();},Sprite_EventLabel[_0x442592(0x34e)][_0x442592(0x324)]=function(){const _0x52f044=_0x442592;this[_0x52f044(0x343)]['x']=0.5,this[_0x52f044(0x343)]['y']=0x1;},Sprite_EventLabel[_0x442592(0x34e)][_0x442592(0xd8)]=function(){const _0x32fc4e=_0x442592,_0xaa26cf=new Rectangle(0x0,0x0,0x1,0x1);this[_0x32fc4e(0x246)]=new Window_Base(_0xaa26cf),this['_proxyWindow']['padding']=0x0,this[_0x32fc4e(0x253)]=this['isLabelVisible']()?0xff:0x0;},Sprite_EventLabel['prototype'][_0x442592(0x441)]=function(){const _0x4fdfb1=_0x442592;Sprite[_0x4fdfb1(0x34e)][_0x4fdfb1(0x441)][_0x4fdfb1(0x98)](this),this['updateText'](),this[_0x4fdfb1(0x112)](),this[_0x4fdfb1(0x109)](),this['updateOpacity'](),this[_0x4fdfb1(0x536)]();},Sprite_EventLabel[_0x442592(0x34e)]['updateText']=function(){const _0x4f1716=_0x442592;this[_0x4f1716(0x520)][_0x4f1716(0x388)]()!==this[_0x4f1716(0x115)]&&(this[_0x4f1716(0x115)]=this[_0x4f1716(0x520)][_0x4f1716(0x388)](),this[_0x4f1716(0x29c)]());},Sprite_EventLabel[_0x442592(0x34e)]['refresh']=function(){const _0x38257d=_0x442592;if(!this[_0x38257d(0x246)])return;this['resizeWindow'](),this['drawText']();},Sprite_EventLabel['prototype'][_0x442592(0xae)]=function(){const _0x17b266=_0x442592,_0x29d35a=this[_0x17b266(0x246)]['textSizeEx'](this['_text']),_0x3f6247=this['_proxyWindow'][_0x17b266(0xa8)](),_0x5a6a49=_0x29d35a[_0x17b266(0x33f)]+_0x3f6247*0x2,_0x60fdfa=_0x29d35a[_0x17b266(0xc4)];this[_0x17b266(0x246)]['move'](0x0,0x0,_0x5a6a49,_0x60fdfa),this[_0x17b266(0x246)]['createContents'](),this[_0x17b266(0x547)]=this['_proxyWindow'][_0x17b266(0x205)];},Sprite_EventLabel[_0x442592(0x34e)][_0x442592(0x1e6)]=function(){const _0x3c27db=_0x442592,_0x279a4b=this[_0x3c27db(0x246)][_0x3c27db(0xa8)]();this[_0x3c27db(0x246)][_0x3c27db(0x1e2)](this['_text'],_0x279a4b,0x0);},Sprite_EventLabel[_0x442592(0x34e)][_0x442592(0x112)]=function(){const _0x31adf6=_0x442592,_0x133d30=VisuMZ[_0x31adf6(0x3dc)][_0x31adf6(0x240)]['Label'][_0x31adf6(0xb0)],_0x3c56b0=$gameSystem[_0x31adf6(0xd6)]()||0x1;this[_0x31adf6(0x24c)]['x']=this[_0x31adf6(0x24c)]['y']=_0x133d30/_0x3c56b0;},Sprite_EventLabel[_0x442592(0x34e)]['updatePosition']=function(){const _0x14b3da=_0x442592;if(!SceneManager['_scene'])return;if(!SceneManager[_0x14b3da(0x29e)][_0x14b3da(0x54c)])return;const _0xe352da=SceneManager['_scene']['_spriteset']['findTargetSprite'](this['_event']);if(!_0xe352da)return;this['x']=this[_0x14b3da(0x520)]['screenX'](),this['x']+=this[_0x14b3da(0x520)][_0x14b3da(0x2a4)][_0x14b3da(0x532)];if(_0xe352da[_0x14b3da(0x323)]){const _0x12c2b4=_0xe352da[_0x14b3da(0x4a1)];this['y']=this[_0x14b3da(0x520)][_0x14b3da(0x500)]()-_0x12c2b4['height']*_0x12c2b4['scale']['y'];}else this['y']=this[_0x14b3da(0x520)][_0x14b3da(0x500)]()-_0xe352da[_0x14b3da(0xc4)]*_0xe352da[_0x14b3da(0x24c)]['y'];this['y']+=$gameSystem[_0x14b3da(0x2df)]()*-0.5,this['y']+=this[_0x14b3da(0x520)][_0x14b3da(0x2a4)][_0x14b3da(0x149)];},Sprite_EventLabel[_0x442592(0x34e)]['updateOpacity']=function(){const _0x3976ab=_0x442592;if(this['isLabelVisible']())this[_0x3976ab(0x253)]+=this[_0x3976ab(0x93)]();else SceneManager['_scene'][_0x3976ab(0xe8)]>0x0?this[_0x3976ab(0x253)]=0x0:this[_0x3976ab(0x253)]-=this[_0x3976ab(0x93)]();},Sprite_EventLabel['prototype']['updateHueShift']=function(){const _0x3190dd=_0x442592;if(this[_0x3190dd(0x108)]()&&this[_0x3190dd(0x520)]&&this[_0x3190dd(0x520)][_0x3190dd(0x2a4)]['hueShift']){const _0x52ba29=this['_hue']+(this[_0x3190dd(0x520)][_0x3190dd(0x2a4)][_0x3190dd(0x178)]||0x0);this[_0x3190dd(0x30b)](_0x52ba29);}},Sprite_EventLabel[_0x442592(0x34e)]['isLabelVisible']=function(){const _0x1476db=_0x442592;if(!$gameSystem['eventLabelsVisible']())return![];if(this[_0x1476db(0x520)]?.[_0x1476db(0x427)])return![];if(this[_0x1476db(0x520)]&&this[_0x1476db(0x520)]['_pageIndex']<0x0)return![];if(SceneManager[_0x1476db(0x29e)][_0x1476db(0xe8)]>0x0)return![];const _0xe6e3d5=$gamePlayer['x'],_0x263580=$gamePlayer['y'],_0x48aa69=this[_0x1476db(0x520)]['x'],_0x133081=this[_0x1476db(0x520)]['y'];if(this[_0x1476db(0x41c)]===_0xe6e3d5&&this[_0x1476db(0x4a7)]===_0x263580&&this[_0x1476db(0x1cb)]===_0x48aa69&&this[_0x1476db(0xc8)]===_0x133081)return this[_0x1476db(0x338)];this['_visiblePlayerX']=$gamePlayer['x'],this[_0x1476db(0x4a7)]=$gamePlayer['y'],this[_0x1476db(0x1cb)]=this['_event']['x'],this[_0x1476db(0xc8)]=this[_0x1476db(0x520)]['y'];if(!VisuMZ['EventsMoveCore']['isInsideLabelRange'](this[_0x1476db(0x520)]))return this[_0x1476db(0x338)]=![],![];return this[_0x1476db(0x338)]=!![],!![];},Sprite_EventLabel['prototype']['opacitySpeed']=function(){const _0x41ba0=_0x442592;return VisuMZ[_0x41ba0(0x3dc)][_0x41ba0(0x240)][_0x41ba0(0x403)]['OpacitySpeed'];};function Sprite_VisuMz_MessagePopup(){const _0xc65c44=_0x442592;this[_0xc65c44(0x327)](...arguments);}Sprite_VisuMz_MessagePopup['prototype']=Object[_0x442592(0x3e6)](Sprite[_0x442592(0x34e)]),Sprite_VisuMz_MessagePopup['prototype'][_0x442592(0x2a9)]=Sprite_VisuMz_MessagePopup,Sprite_VisuMz_MessagePopup[_0x442592(0x34e)]['initialize']=function(_0x1413a3){const _0x350239=_0x442592;this[_0x350239(0x1b2)]=_0x1413a3,Sprite['prototype'][_0x350239(0x327)]['call'](this),this[_0x350239(0x324)](),this[_0x350239(0x8f)](),this[_0x350239(0x3aa)](),this[_0x350239(0x441)]();},Sprite_VisuMz_MessagePopup['prototype'][_0x442592(0x324)]=function(){const _0x149ef3=_0x442592;this[_0x149ef3(0x2d4)]=this[_0x149ef3(0x1b2)][_0x149ef3(0x452)],this['_wholeDuration']=this[_0x149ef3(0x1b2)]['duration'],this['z']=0x6,this[_0x149ef3(0x1f5)]=this[_0x149ef3(0x1b2)][_0x149ef3(0x4f9)]['fadeIn'],this['_fadeInDuration']>0x0&&this[_0x149ef3(0x1f5)]>=Math[_0x149ef3(0x446)](this[_0x149ef3(0x2d4)]*0.48)&&(this['_fadeInDuration']=Math['floor'](this[_0x149ef3(0x2d4)]*0.48)),this[_0x149ef3(0x253)]=this['_fadeInDuration']>0x0?0x0:0xff,this['_fadeOutDuration']=this[_0x149ef3(0x1b2)][_0x149ef3(0x4f9)][_0x149ef3(0x212)],this['_fadeOutDuration']>0x0&&this['_fadeOutDuration']>=Math['floor'](this[_0x149ef3(0x2d4)]*0.48)&&(this['_fadeOutDuration']=Math['floor'](this['_duration']*0.48)),this[_0x149ef3(0x3d5)]=this[_0x149ef3(0x3cc)],this[_0x149ef3(0x20b)]=this[_0x149ef3(0x1b2)][_0x149ef3(0x311)]['x'],this[_0x149ef3(0x4e3)]=this[_0x149ef3(0x1b2)][_0x149ef3(0x311)]['y'],this[_0x149ef3(0x4c9)]=this[_0x149ef3(0x1b2)]['endOffset']['x'],this[_0x149ef3(0x144)]=this[_0x149ef3(0x1b2)][_0x149ef3(0x214)]['y'],this[_0x149ef3(0x463)]=this[_0x149ef3(0x20b)],this['_offsetY']=this[_0x149ef3(0x4e3)],this[_0x149ef3(0x106)]=this['_settings']['startScale']['x'],this['_startScaleY']=this[_0x149ef3(0x1b2)][_0x149ef3(0x3bb)]['y'],this[_0x149ef3(0x120)]=this[_0x149ef3(0x1b2)]['endScale']['x'],this['_targetScaleY']=this['_settings'][_0x149ef3(0x524)]['y'],this[_0x149ef3(0x2c1)]=-this['_settings'][_0x149ef3(0x52b)][_0x149ef3(0x2bb)],this[_0x149ef3(0x401)]=-this[_0x149ef3(0x1b2)][_0x149ef3(0x52b)][_0x149ef3(0x4b9)],this['_arcPeak']=-this[_0x149ef3(0x1b2)][_0x149ef3(0x1dc)][_0x149ef3(0x45a)],this['_currentArc']=0x0;},Sprite_VisuMz_MessagePopup[_0x442592(0x34e)]['createDummyWindow']=function(){const _0x3bf2e3=_0x442592,_0x3e5a94=this['_settings'],_0x1882fa=new Rectangle(0x0,0x0,Graphics[_0x3bf2e3(0x33f)],Graphics[_0x3bf2e3(0xc4)]);this['_dummyWindow']=new Window_Base(_0x1882fa);const _0x37fb94=this['_dummyWindow'][_0x3bf2e3(0x407)](_0x3e5a94[_0x3bf2e3(0x410)]),_0x442e05=_0x37fb94['width'],_0x3a6634=_0x37fb94[_0x3bf2e3(0xc4)],_0x58edbd=_0x442e05+$gameSystem['windowPadding']()*0x2,_0x1575cf=_0x3a6634+$gameSystem[_0x3bf2e3(0x2df)]()*0x2;this[_0x3bf2e3(0x423)][_0x3bf2e3(0x3e9)](0x0,0x0,_0x58edbd,_0x1575cf),this[_0x3bf2e3(0x423)][_0x3bf2e3(0x242)](),this[_0x3bf2e3(0x423)][_0x3bf2e3(0x1e2)](_0x3e5a94[_0x3bf2e3(0x410)],0x0,0x0);},Sprite_VisuMz_MessagePopup['prototype']['createTextSprite']=function(){const _0x4420d6=_0x442592;this[_0x4420d6(0xf0)]=new Sprite(),this[_0x4420d6(0xf0)][_0x4420d6(0x547)]=this[_0x4420d6(0x423)][_0x4420d6(0x205)],this[_0x4420d6(0xf0)]['anchor']['x']=0.5,this[_0x4420d6(0xf0)][_0x4420d6(0x343)]['y']=0.5,this[_0x4420d6(0xf0)]['x']=this['_startX'],this[_0x4420d6(0xf0)]['y']=this[_0x4420d6(0x4e3)],this[_0x4420d6(0xf0)]['scale']['x']=this[_0x4420d6(0x106)],this[_0x4420d6(0xf0)]['scale']['y']=this[_0x4420d6(0x3b9)],this[_0x4420d6(0xf0)]['angle']=this[_0x4420d6(0x2c1)],this[_0x4420d6(0x406)](this['_textSprite']);},Sprite_VisuMz_MessagePopup[_0x442592(0x34e)][_0x442592(0x441)]=function(){const _0x5541db=_0x442592;Sprite[_0x5541db(0x34e)][_0x5541db(0x441)][_0x5541db(0x98)](this);if(!this[_0x5541db(0x512)]())return;this[_0x5541db(0x32d)](),this[_0x5541db(0x4ab)](),this['updateTextScale'](),this[_0x5541db(0x47b)](),this[_0x5541db(0x49d)](),this['updateDuration']();},Sprite_VisuMz_MessagePopup[_0x442592(0x34e)][_0x442592(0x512)]=function(){return!!this['_textSprite'];},Sprite_VisuMz_MessagePopup['prototype'][_0x442592(0x32d)]=function(){const _0x563590=_0x442592,_0x789e6=this['_settings'];{const _0xdff5d1=$gameMap[_0x563590(0x531)](),_0x23f363=_0x789e6[_0x563590(0x412)]['x'],_0xe322db=$gameMap[_0x563590(0x341)](_0x23f363);this['x']=Math[_0x563590(0x446)](_0xe322db*_0xdff5d1+_0xdff5d1/0x2);}{const _0x2eef4a=$gameMap[_0x563590(0x46a)](),_0x2c560a=_0x789e6['tileCoordinates']['y'],_0x1025b7=$gameMap['adjustY'](_0x2c560a);this['y']=Math['floor'](_0x1025b7*_0x2eef4a+_0x2eef4a);}},Sprite_VisuMz_MessagePopup[_0x442592(0x34e)][_0x442592(0x4ab)]=function(){const _0x1996e9=_0x442592;if(this[_0x1996e9(0x2d4)]<=0x0)return;const _0xe3fbaf=this[_0x1996e9(0x2d4)],_0xfcab0=this[_0x1996e9(0x514)];{this['_offsetX']=(this[_0x1996e9(0x463)]*(_0xe3fbaf-0x1)+this['_targetX'])/_0xe3fbaf,this[_0x1996e9(0x310)]=(this[_0x1996e9(0x310)]*(_0xe3fbaf-0x1)+this[_0x1996e9(0x144)])/_0xe3fbaf;}{const _0x8f37b9=_0xfcab0-_0xe3fbaf,_0x3836d8=_0xfcab0/0x2,_0x578795=this[_0x1996e9(0x1ce)],_0x1d48e5=-_0x578795/Math[_0x1996e9(0x404)](_0x3836d8,0x2);this['_currentArc']=_0x1d48e5*Math[_0x1996e9(0x404)](_0x8f37b9-_0x3836d8,0x2)+_0x578795;}this[_0x1996e9(0xf0)]['x']=this[_0x1996e9(0x463)],this[_0x1996e9(0xf0)]['y']=this[_0x1996e9(0x310)]+this['_currentArc'];},Sprite_VisuMz_MessagePopup['prototype']['updateTextScale']=function(){const _0x38a764=_0x442592;if(this[_0x38a764(0x2d4)]<=0x0)return;const _0x4006a4=this[_0x38a764(0x2d4)];this['_textSprite'][_0x38a764(0x24c)]['x']=(this[_0x38a764(0xf0)][_0x38a764(0x24c)]['x']*(_0x4006a4-0x1)+this['_targetScaleX'])/_0x4006a4,this[_0x38a764(0xf0)][_0x38a764(0x24c)]['y']=(this[_0x38a764(0xf0)][_0x38a764(0x24c)]['y']*(_0x4006a4-0x1)+this['_targetScaleY'])/_0x4006a4;},Sprite_VisuMz_MessagePopup[_0x442592(0x34e)][_0x442592(0x47b)]=function(){const _0x5b44fc=_0x442592;if(this[_0x5b44fc(0x2d4)]<=0x0)return;const _0x1a3d75=this[_0x5b44fc(0x2d4)];this['_textSprite'][_0x5b44fc(0x52b)]=(this[_0x5b44fc(0xf0)][_0x5b44fc(0x52b)]*(_0x1a3d75-0x1)+this[_0x5b44fc(0x401)])/_0x1a3d75;},Sprite_VisuMz_MessagePopup[_0x442592(0x34e)][_0x442592(0x49d)]=function(){const _0x34db2b=_0x442592;this[_0x34db2b(0x4db)](),this['updateFadeOut']();},Sprite_VisuMz_MessagePopup[_0x442592(0x34e)][_0x442592(0x4db)]=function(){const _0x44f182=_0x442592;if(this['_fadeInDuration']<=0x0)return;const _0x32891f=this['_fadeInDuration'];this[_0x44f182(0x253)]=(this['opacity']*(_0x32891f-0x1)+0xff)/_0x32891f,this[_0x44f182(0x1f5)]--,this[_0x44f182(0x1f5)]<=0x0&&(this[_0x44f182(0x253)]=0xff);},Sprite_VisuMz_MessagePopup[_0x442592(0x34e)]['updateFadeOut']=function(){const _0x2c262b=_0x442592;if(this[_0x2c262b(0x3cc)]<=0x0)return;if(this[_0x2c262b(0x2d4)]>this[_0x2c262b(0x3d5)])return;const _0x2efa47=this['_fadeOutDuration'];this[_0x2c262b(0x253)]=(this[_0x2c262b(0x253)]*(_0x2efa47-0x1)+0x0)/_0x2efa47,this[_0x2c262b(0x3cc)]--,this[_0x2c262b(0x3cc)]<=0x0&&(this[_0x2c262b(0x253)]=0x0);},Sprite_VisuMz_MessagePopup[_0x442592(0x34e)][_0x442592(0x300)]=function(){const _0x2c343f=_0x442592;if(this[_0x2c343f(0x2d4)]<=0x0)return;this[_0x2c343f(0x2d4)]--;if(this['_duration']<=0x0){if(this['parent'])this[_0x2c343f(0x143)]['removeChild'](this);this[_0x2c343f(0xf0)][_0x2c343f(0x547)]&&this[_0x2c343f(0xf0)]['bitmap'][_0x2c343f(0xfc)]();}},VisuMZ['EventsMoveCore'][_0x442592(0x1f1)]=Spriteset_Map[_0x442592(0x34e)][_0x442592(0x3a2)],Spriteset_Map[_0x442592(0x34e)][_0x442592(0x3a2)]=function(){const _0xb6fcaf=_0x442592;VisuMZ[_0xb6fcaf(0x3dc)]['Spriteset_Map_createLowerLayer']['call'](this),this['createLabelWindows']();},VisuMZ[_0x442592(0x3dc)][_0x442592(0x542)]=Spriteset_Map[_0x442592(0x34e)][_0x442592(0x464)],Spriteset_Map['prototype'][_0x442592(0x464)]=function(){const _0x451953=_0x442592;VisuMZ[_0x451953(0x3dc)]['Spriteset_Map_createShadow']['call'](this),this[_0x451953(0x26b)]();},Spriteset_Map[_0x442592(0x34e)][_0x442592(0x26b)]=function(){const _0x58e883=_0x442592;if(!VisuMZ[_0x58e883(0x3dc)][_0x58e883(0x240)][_0x58e883(0x41e)][_0x58e883(0x387)])return;for(const _0x3465c3 of this[_0x58e883(0x14d)]){this[_0x58e883(0x4bf)](_0x3465c3);}},Spriteset_Map[_0x442592(0x34e)][_0x442592(0x4bf)]=function(_0x31f650){const _0x55b2d5=_0x442592;_0x31f650[_0x55b2d5(0x342)]=new Sprite(),_0x31f650[_0x55b2d5(0x342)][_0x55b2d5(0x525)]=_0x31f650[_0x55b2d5(0x32c)][_0x55b2d5(0x1be)](),_0x31f650[_0x55b2d5(0x342)][_0x55b2d5(0x547)]=ImageManager[_0x55b2d5(0x329)](_0x31f650['_shadowSprite'][_0x55b2d5(0x525)]),_0x31f650[_0x55b2d5(0x342)]['anchor']['x']=0.5,_0x31f650['_shadowSprite'][_0x55b2d5(0x343)]['y']=0x1;const _0x45515e=VisuMZ[_0x55b2d5(0x3dc)][_0x55b2d5(0x240)]['Movement'][_0x55b2d5(0x231)]??0.5;_0x31f650[_0x55b2d5(0x342)]['z']=_0x45515e,this[_0x55b2d5(0xa4)][_0x55b2d5(0x406)](_0x31f650[_0x55b2d5(0x342)]);},Spriteset_Map['prototype'][_0x442592(0x4b1)]=function(){const _0x1d8fbc=_0x442592;if(!VisuMZ[_0x1d8fbc(0x3dc)][_0x1d8fbc(0x240)][_0x1d8fbc(0x41e)][_0x1d8fbc(0x387)])return;for(const _0xac3344 of this[_0x1d8fbc(0x14d)]){this[_0x1d8fbc(0xa4)][_0x1d8fbc(0x27e)](_0xac3344['_shadowSprite']);}},Spriteset_Map['prototype']['createLabelWindows']=function(){const _0x3b8669=_0x442592;this[_0x3b8669(0x21c)]=[];for(const _0x451b0c of $gameMap['events']()){this[_0x3b8669(0x298)](_0x451b0c);}},Spriteset_Map[_0x442592(0x123)]=VisuMZ[_0x442592(0x3dc)][_0x442592(0x240)][_0x442592(0x403)][_0x442592(0x2ca)]??!![],Spriteset_Map[_0x442592(0x34e)][_0x442592(0x298)]=function(_0x214b10){const _0x3c292f=_0x442592;if(!this['isTargetEventValidForLabelWindow'](_0x214b10))return;if(Utils[_0x3c292f(0x1fc)]()){if(!Spriteset_Map[_0x3c292f(0x123)])return;}let _0x5ded54;const _0x3f7939=VisuMZ[_0x3c292f(0x3dc)][_0x3c292f(0x240)][_0x3c292f(0x403)]['SpriteBased']??!![];_0x5ded54=_0x3f7939?new Sprite_EventLabel(_0x214b10):new Window_EventLabel(_0x214b10),_0x5ded54['z']=0x8,_0x5ded54[_0x3c292f(0x131)]=Sprite['_counter']++,this[_0x3c292f(0xa4)]['addChild'](_0x5ded54),this[_0x3c292f(0x21c)]['push'](_0x5ded54);},Spriteset_Map['prototype']['isTargetEventValidForLabelWindow']=function(_0x4a4807){const _0x2a84d5=_0x442592,_0x3aec12=_0x4a4807[_0x2a84d5(0x2f2)]();if(_0x3aec12['note'][_0x2a84d5(0x110)](/<LABEL:[ ](.*?)>/i))return!![];if(_0x3aec12[_0x2a84d5(0x2fc)][_0x2a84d5(0x110)](/<LABEL>\s*([\s\S]*)\s*<\/LABEL>/i))return!![];for(const _0x4d88dd of _0x3aec12[_0x2a84d5(0x1aa)]){let _0x1ca020='';for(const _0x2645fe of _0x4d88dd[_0x2a84d5(0x249)]){[0x6c,0x198]['includes'](_0x2645fe[_0x2a84d5(0x308)])&&(_0x1ca020+=_0x2645fe['parameters'][0x0]);}if(_0x1ca020['match'](/<LABEL:[ ](.*?)>/i))return!![];if(_0x1ca020[_0x2a84d5(0x110)](/<LABEL>\s*([\s\S]*)\s*<\/LABEL>/i))return!![];}return![];},Spriteset_Map[_0x442592(0x34e)][_0x442592(0xd1)]=function(_0x4509fe){const _0x2921d2=_0x442592;this[_0x2921d2(0x14d)]=this[_0x2921d2(0x14d)]||[];const _0x31997a=new Sprite_Character(_0x4509fe);this['_characterSprites'][_0x2921d2(0x314)](_0x31997a),this['_tilemap'][_0x2921d2(0x406)](_0x31997a),this[_0x2921d2(0x4bf)](_0x31997a),this[_0x2921d2(0x298)](_0x4509fe),_0x31997a['update'](),_0x4509fe[_0x2921d2(0x2f3)](),_0x31997a['updateFrame']();},Spriteset_Map[_0x442592(0x34e)][_0x442592(0x27b)]=function(){const _0x2e59eb=_0x442592;if(!this[_0x2e59eb(0x21c)])return;for(const _0xa71a33 of this[_0x2e59eb(0x21c)]){_0xa71a33&&(_0xa71a33['_visiblePlayerX']=undefined,_0xa71a33['refresh']());}},Spriteset_Map[_0x442592(0x34e)][_0x442592(0x2f0)]=function(_0x22f64b,_0x5928a3){const _0x35885c=_0x442592;if(!_0x22f64b)return;_0x5928a3[_0x35885c(0x412)]={'x':_0x22f64b['x'],'y':_0x22f64b['y']},this[_0x35885c(0x14b)](_0x5928a3);},Spriteset_Map[_0x442592(0x34e)]['createEventsMoveCoreTileMessagePopup']=function(_0x30df48){const _0x2f8cc8=_0x442592;if(!this[_0x2f8cc8(0xa4)])return;const _0x34c83c=new Sprite_VisuMz_MessagePopup(_0x30df48);this['_tilemap'][_0x2f8cc8(0x406)](_0x34c83c);},VisuMZ[_0x442592(0x3dc)]['Game_Message_setNumberInput']=Game_Message[_0x442592(0x34e)][_0x442592(0x4aa)],Game_Message[_0x442592(0x34e)]['setNumberInput']=function(_0x5232ef,_0x42b9fc){const _0x37303f=_0x442592;this[_0x37303f(0x2f8)]=$gameTemp[_0x37303f(0x4b4)](),VisuMZ[_0x37303f(0x3dc)]['Game_Message_setNumberInput'][_0x37303f(0x98)](this,_0x5232ef,_0x42b9fc);},VisuMZ[_0x442592(0x3dc)][_0x442592(0x3c0)]=Window_NumberInput['prototype'][_0x442592(0x2bb)],Window_NumberInput[_0x442592(0x34e)]['start']=function(){const _0x4598ae=_0x442592;$gameTemp['registerSelfTarget']($gameMessage[_0x4598ae(0x2f8)]),VisuMZ['EventsMoveCore'][_0x4598ae(0x3c0)][_0x4598ae(0x98)](this),$gameTemp[_0x4598ae(0x1ed)]();},VisuMZ[_0x442592(0x3dc)]['Window_NumberInput_processOk']=Window_NumberInput['prototype'][_0x442592(0x23b)],Window_NumberInput[_0x442592(0x34e)]['processOk']=function(){const _0x3d6b50=_0x442592;$gameTemp[_0x3d6b50(0x2e9)]($gameMessage[_0x3d6b50(0x2f8)]),VisuMZ[_0x3d6b50(0x3dc)]['Window_NumberInput_processOk'][_0x3d6b50(0x98)](this),$gameTemp[_0x3d6b50(0x1ed)](),$gameMessage[_0x3d6b50(0x2f8)]=undefined;},VisuMZ['EventsMoveCore'][_0x442592(0x80)]=Game_Message['prototype'][_0x442592(0x40e)],Game_Message[_0x442592(0x34e)][_0x442592(0x40e)]=function(_0x53b39a,_0x554128){const _0x37c62c=_0x442592;this[_0x37c62c(0x13a)]=$gameTemp[_0x37c62c(0x4b4)](),VisuMZ[_0x37c62c(0x3dc)][_0x37c62c(0x80)][_0x37c62c(0x98)](this,_0x53b39a,_0x554128);},VisuMZ[_0x442592(0x3dc)][_0x442592(0x36b)]=Window_EventItem[_0x442592(0x34e)][_0x442592(0x15f)],Window_EventItem[_0x442592(0x34e)][_0x442592(0x15f)]=function(){const _0x398957=_0x442592;$gameTemp[_0x398957(0x2e9)]($gameMessage['_selfTargetItemChoice']),VisuMZ[_0x398957(0x3dc)][_0x398957(0x36b)][_0x398957(0x98)](this),$gameTemp[_0x398957(0x1ed)](),$gameMessage[_0x398957(0x13a)]=undefined;},VisuMZ[_0x442592(0x3dc)]['Window_EventItem_onCancel']=Window_EventItem[_0x442592(0x34e)]['onCancel'],Window_EventItem[_0x442592(0x34e)]['onCancel']=function(){const _0x525b69=_0x442592;$gameTemp[_0x525b69(0x2e9)]($gameMessage[_0x525b69(0x13a)]),VisuMZ[_0x525b69(0x3dc)][_0x525b69(0x76)][_0x525b69(0x98)](this),$gameTemp[_0x525b69(0x1ed)](),$gameMessage[_0x525b69(0x13a)]=undefined;},VisuMZ['EventsMoveCore']['Window_Message_startMessage']=Window_Message['prototype'][_0x442592(0x361)],Window_Message[_0x442592(0x34e)][_0x442592(0x361)]=function(){const _0x2a7cc0=_0x442592;$gameMessage['registerSelfEvent'](),VisuMZ['EventsMoveCore'][_0x2a7cc0(0x299)][_0x2a7cc0(0x98)](this),$gameTemp[_0x2a7cc0(0x1ed)]();},VisuMZ[_0x442592(0x3dc)][_0x442592(0xe4)]=Window_ScrollText[_0x442592(0x34e)][_0x442592(0x361)],Window_ScrollText[_0x442592(0x34e)][_0x442592(0x361)]=function(){const _0xf2e748=_0x442592;$gameMessage[_0xf2e748(0x50b)](),VisuMZ[_0xf2e748(0x3dc)][_0xf2e748(0xe4)][_0xf2e748(0x98)](this),$gameTemp[_0xf2e748(0x1ed)]();};function Window_EventLabel(){const _0x38ba30=_0x442592;this[_0x38ba30(0x327)](...arguments);}Window_EventLabel['prototype']=Object['create'](Window_Base[_0x442592(0x34e)]),Window_EventLabel['prototype'][_0x442592(0x2a9)]=Window_EventLabel,Window_EventLabel[_0x442592(0x34e)][_0x442592(0x327)]=function(_0x4b8a85){const _0x5a9cd5=_0x442592;this[_0x5a9cd5(0x520)]=_0x4b8a85;const _0x56811c=new Rectangle(0x0,0x0,Graphics[_0x5a9cd5(0x136)]/0x4,this[_0x5a9cd5(0x47e)](0x1));this[_0x5a9cd5(0x324)](),Window_Base[_0x5a9cd5(0x34e)][_0x5a9cd5(0x327)][_0x5a9cd5(0x98)](this,_0x56811c),this[_0x5a9cd5(0x2a6)]=0x0,this[_0x5a9cd5(0x4f7)](0x2),this[_0x5a9cd5(0x115)]='';},Window_EventLabel[_0x442592(0x34e)][_0x442592(0x324)]=function(){const _0x1ac253=_0x442592;this[_0x1ac253(0x4af)]=![],this[_0x1ac253(0x225)]=$gameScreen[_0x1ac253(0x10e)](),this[_0x1ac253(0x2e2)]=this[_0x1ac253(0x520)][_0x1ac253(0x223)](),this[_0x1ac253(0x2c4)]=this[_0x1ac253(0x520)][_0x1ac253(0x500)](),this['_eventLabelOffsetX']=this[_0x1ac253(0x520)][_0x1ac253(0x2a4)][_0x1ac253(0x532)],this[_0x1ac253(0x357)]=this[_0x1ac253(0x520)]['_labelWindow'][_0x1ac253(0x149)],this['_eventPageIndex']=this[_0x1ac253(0x520)][_0x1ac253(0x34f)],this['_cacheVisibility']=this[_0x1ac253(0x108)](),this['_cacheSystemVisible']=$gameSystem[_0x1ac253(0x1ad)](),this['_visiblePlayerX']=$gamePlayer['x'],this[_0x1ac253(0x4a7)]=$gamePlayer['y'],this[_0x1ac253(0x1cb)]=this[_0x1ac253(0x520)]['x'],this[_0x1ac253(0xc8)]=this[_0x1ac253(0x520)]['y'];},Window_EventLabel[_0x442592(0x34e)]['update']=function(){const _0x1a5215=_0x442592;Window_Base[_0x1a5215(0x34e)]['update']['call'](this);if(!this['needsUpdate']())return;this[_0x1a5215(0x432)](),this[_0x1a5215(0x112)](),this['updatePosition'](),this[_0x1a5215(0x49d)]();},Window_EventLabel['prototype']['needsUpdate']=function(){const _0x4d9a24=_0x442592;if(!this[_0x4d9a24(0x520)])return![];if(!this[_0x4d9a24(0x520)][_0x4d9a24(0x2a4)])return![];if(this[_0x4d9a24(0x355)]!==this[_0x4d9a24(0x520)][_0x4d9a24(0x34f)])return!![];if(this[_0x4d9a24(0x520)][_0x4d9a24(0x427)]&&!this[_0x4d9a24(0x4af)])return!![];if(this[_0x4d9a24(0x520)][_0x4d9a24(0x2a4)][_0x4d9a24(0x410)]==='')return![];if(this[_0x4d9a24(0x225)]!==$gameScreen[_0x4d9a24(0x10e)]())return!![];if(this[_0x4d9a24(0x2e2)]!==this[_0x4d9a24(0x520)][_0x4d9a24(0x223)]())return!![];if(this[_0x4d9a24(0x2c4)]!==this['_event'][_0x4d9a24(0x500)]())return!![];if(this['_eventLabelOffsetX']!==this[_0x4d9a24(0x520)][_0x4d9a24(0x2a4)][_0x4d9a24(0x532)])return!![];if(this[_0x4d9a24(0x357)]!==this[_0x4d9a24(0x520)][_0x4d9a24(0x2a4)][_0x4d9a24(0x149)])return!![];if(this[_0x4d9a24(0x41c)]!==$gamePlayer['x'])return!![];if(this[_0x4d9a24(0x4a7)]!==$gamePlayer['y'])return!![];if(this[_0x4d9a24(0x1cb)]!==this[_0x4d9a24(0x520)]['x'])return!![];if(this[_0x4d9a24(0xc8)]!==this[_0x4d9a24(0x520)]['y'])return!![];if(this['_cacheSystemVisible']!==$gameSystem[_0x4d9a24(0x1ad)]())return!![];if(this[_0x4d9a24(0x338)]&&this[_0x4d9a24(0x2a6)]<0xff)return!![];if(!this['_cacheVisibility']&&this[_0x4d9a24(0x2a6)]>0x0)return!![];if(SceneManager['_scene']['_encounterEffectDuration']>0x0)return!![];return![];},Window_EventLabel[_0x442592(0x34e)][_0x442592(0x432)]=function(){const _0x171560=_0x442592;this['_event']['labelWindowText']()!==this[_0x171560(0x115)]&&(this[_0x171560(0x115)]=this[_0x171560(0x520)][_0x171560(0x388)](),this[_0x171560(0x29c)]());},Window_EventLabel[_0x442592(0x34e)]['updateScale']=function(){const _0x579dd2=_0x442592;this[_0x579dd2(0x24c)]['x']=0x1/$gameScreen[_0x579dd2(0x10e)](),this[_0x579dd2(0x24c)]['y']=0x1/$gameScreen[_0x579dd2(0x10e)](),this[_0x579dd2(0x225)]=$gameScreen[_0x579dd2(0x10e)]();},Window_EventLabel['prototype'][_0x442592(0x109)]=function(){const _0x4917f7=_0x442592;if(!SceneManager['_scene'])return;if(!SceneManager[_0x4917f7(0x29e)]['_spriteset'])return;const _0x362752=SceneManager[_0x4917f7(0x29e)]['_spriteset'][_0x4917f7(0x20c)](this[_0x4917f7(0x520)]);if(!_0x362752)return;this['x']=Math[_0x4917f7(0x1e4)](this[_0x4917f7(0x520)]['screenX']()-Math[_0x4917f7(0x446)](this['width']*this[_0x4917f7(0x24c)]['x']/0x2)),this['x']+=this[_0x4917f7(0x520)][_0x4917f7(0x2a4)][_0x4917f7(0x532)],this['y']=this[_0x4917f7(0x520)][_0x4917f7(0x500)]()-_0x362752[_0x4917f7(0xc4)],this['y']+=Math[_0x4917f7(0x1e4)]($gameSystem[_0x4917f7(0x2df)]()*0.5),this['y']-=Math[_0x4917f7(0x1e4)](this[_0x4917f7(0xc4)]*this[_0x4917f7(0x24c)]['y']),this['y']+=this[_0x4917f7(0x520)][_0x4917f7(0x2a4)][_0x4917f7(0x149)],this[_0x4917f7(0x4af)]=this[_0x4917f7(0x520)][_0x4917f7(0x427)],this[_0x4917f7(0x2e2)]=this[_0x4917f7(0x520)][_0x4917f7(0x223)](),this[_0x4917f7(0x2c4)]=this[_0x4917f7(0x520)][_0x4917f7(0x500)](),this[_0x4917f7(0x44d)]=this['_event']['_labelWindow'][_0x4917f7(0x532)],this[_0x4917f7(0x357)]=this['_event']['_labelWindow']['offsetY'],this[_0x4917f7(0x355)]=this['_event'][_0x4917f7(0x34f)],this['_eventErased']&&(this[_0x4917f7(0x2a6)]=0x0);},Window_EventLabel['prototype'][_0x442592(0x49d)]=function(){const _0x212147=_0x442592;if(this[_0x212147(0x108)]())this[_0x212147(0x2a6)]+=this[_0x212147(0x93)]();else SceneManager[_0x212147(0x29e)]['_encounterEffectDuration']>0x0?this[_0x212147(0x2a6)]=0x0:this['contentsOpacity']-=this['opacitySpeed']();},Window_EventLabel[_0x442592(0x34e)][_0x442592(0x108)]=function(){const _0x2e0562=_0x442592;if(!$gameSystem[_0x2e0562(0x1ad)]())return![];if(this[_0x2e0562(0x520)]?.[_0x2e0562(0x427)])return![];if(SceneManager[_0x2e0562(0x29e)][_0x2e0562(0xe8)]>0x0)return![];const _0x10ef1a=$gamePlayer['x'],_0x5f5419=$gamePlayer['y'],_0x2bd28d=this[_0x2e0562(0x520)]['x'],_0x6e6552=this[_0x2e0562(0x520)]['y'];if(this[_0x2e0562(0x41c)]===_0x10ef1a&&this[_0x2e0562(0x4a7)]===_0x5f5419&&this[_0x2e0562(0x1cb)]===_0x2bd28d&&this['_visibleEventY']===_0x6e6552)return this[_0x2e0562(0x338)];this['_visiblePlayerX']=$gamePlayer['x'],this[_0x2e0562(0x4a7)]=$gamePlayer['y'],this[_0x2e0562(0x1cb)]=this[_0x2e0562(0x520)]['x'],this[_0x2e0562(0xc8)]=this[_0x2e0562(0x520)]['y'];if(!VisuMZ[_0x2e0562(0x3dc)][_0x2e0562(0xa5)](this[_0x2e0562(0x520)]))return this[_0x2e0562(0x338)]=![],![];return this[_0x2e0562(0x338)]=!![],!![];},Window_EventLabel[_0x442592(0x34e)][_0x442592(0x93)]=function(){const _0xb9f7cb=_0x442592;return VisuMZ[_0xb9f7cb(0x3dc)][_0xb9f7cb(0x240)][_0xb9f7cb(0x403)][_0xb9f7cb(0x264)];},Window_EventLabel['prototype'][_0x442592(0xae)]=function(){const _0x431e7b=_0x442592,_0x4356fc=this['textSizeEx'](this[_0x431e7b(0x115)]);this['width']=_0x4356fc[_0x431e7b(0x33f)]+($gameSystem[_0x431e7b(0x2df)]()+this[_0x431e7b(0xa8)]())*0x2,this[_0x431e7b(0xc4)]=Math[_0x431e7b(0xcc)](this[_0x431e7b(0x34d)](),_0x4356fc['height'])+$gameSystem[_0x431e7b(0x2df)]()*0x2,this[_0x431e7b(0x242)]();},Window_EventLabel[_0x442592(0x34e)][_0x442592(0x34d)]=function(){const _0x7e735f=_0x442592;return VisuMZ[_0x7e735f(0x3dc)][_0x7e735f(0x240)][_0x7e735f(0x403)]['LineHeight'];},Window_EventLabel[_0x442592(0x34e)][_0x442592(0x30f)]=function(){const _0x5e767a=_0x442592;Window_Base['prototype']['resetFontSettings'][_0x5e767a(0x98)](this),this[_0x5e767a(0x205)][_0x5e767a(0x2a5)]=this[_0x5e767a(0x54e)]();},Window_EventLabel[_0x442592(0x34e)]['defaultFontSize']=function(){const _0x13883b=_0x442592;return VisuMZ['EventsMoveCore']['Settings'][_0x13883b(0x403)][_0x13883b(0xb0)];},Window_EventLabel['prototype'][_0x442592(0x29c)]=function(){const _0x8a9958=_0x442592;this[_0x8a9958(0xae)](),this['contents'][_0x8a9958(0x530)]();const _0xccbdcc=this[_0x8a9958(0x115)]['split'](/[\r\n]+/);let _0x564d45=0x0;for(const _0x5126f4 of _0xccbdcc){const _0x19ac08=this[_0x8a9958(0x407)](_0x5126f4),_0x52ec50=Math[_0x8a9958(0x446)]((this[_0x8a9958(0x1d5)]-_0x19ac08[_0x8a9958(0x33f)])/0x2);this[_0x8a9958(0x1e2)](_0x5126f4,_0x52ec50,_0x564d45),_0x564d45+=_0x19ac08[_0x8a9958(0xc4)];}},Window_EventLabel[_0x442592(0x34e)][_0x442592(0x3da)]=function(_0x3006a6,_0x173992){const _0x1a2965=_0x442592;_0x173992['drawing']&&this['drawIcon'](_0x3006a6,_0x173992['x']+0x2,_0x173992['y']),_0x173992['x']+=Math['min'](this['iconSize'](),ImageManager[_0x1a2965(0x3b3)])+0x4;},Window_EventLabel[_0x442592(0x34e)][_0x442592(0x91)]=function(_0x5cabe3,_0x22db9b,_0x3f4ac9){const _0x19f470=_0x442592,_0x955784=ImageManager[_0x19f470(0x329)](_0x19f470(0x117)),_0x2ef391=ImageManager['iconWidth'],_0x24bd45=ImageManager[_0x19f470(0x336)],_0x5837f9=_0x5cabe3%0x10*_0x2ef391,_0x1a75ec=Math[_0x19f470(0x446)](_0x5cabe3/0x10)*_0x24bd45,_0x562890=Math[_0x19f470(0x50d)](this[_0x19f470(0x87)]()),_0x3ced31=Math[_0x19f470(0x50d)](this[_0x19f470(0x87)]());this[_0x19f470(0x205)][_0x19f470(0x208)](_0x955784,_0x5837f9,_0x1a75ec,_0x2ef391,_0x24bd45,_0x22db9b,_0x3f4ac9,_0x562890,_0x3ced31);},Window_EventLabel['prototype'][_0x442592(0x87)]=function(){const _0x5da93d=_0x442592;return VisuMZ[_0x5da93d(0x3dc)]['Settings'][_0x5da93d(0x403)]['IconSize'];};