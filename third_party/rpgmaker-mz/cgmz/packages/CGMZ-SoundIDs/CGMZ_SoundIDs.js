/*:
 * @author Casper Gaming
 * @url https://www.caspergaming.com/plugins/cgmz/soundids/
 * @target MZ
 * @base CGMZ_Core
 * @orderAfter CGMZ_Core
 * @plugindesc Play sound by ID instead of filename
 * @help
 * ============================================================================
 * For terms and conditions using this plugin in your game please visit:
 * https://www.caspergaming.com/terms-of-use/
 * ============================================================================
 * Become a Patron to get access to beta/alpha plugins plus other goodies!
 * https://www.patreon.com/CasperGamingRPGM
 * ============================================================================
 * Version: 1.1.0
 * ----------------------------------------------------------------------------
 * Compatibility: Only tested with my CGMZ plugins.
 * Made for RPG Maker MZ 1.9.0
 * ----------------------------------------------------------------------------
 * Description: Allows you to associate music files with IDs, and then play
 * the music file by ID. This will cut down dev time if you want to swap a
 * sound later in development as you will just switch it in one area instead
 * of anywhere the sound appears.
 * ----------------------------------------------------------------------------
 * Documentation:
 * -------------------------------Sound IDs------------------------------------
 * Each sound ID must be unique, even if the sound is a different type. For
 * example, a BGM id cannot be the same as a SE id.
 * ----------------------------Plugin Commands---------------------------------
 * This plugin supports the following plugin commands:
 *
 * • Play BGM
 * Plays the BGM associated with the provided ID.
 *
 * • Play BGS
 * Plays the BGS associated with the provided ID.
 *
 * • Play ME
 * Plays the ME associated with the provided ID.
 *
 * • Play SE
 * Plays the SE associated with the provided ID.
 *
 * • Play Sound
 * Generic command which will play the sound based on provided ID, either as
 * a BGM, BGS, ME, or SE, whatever the sound is set up as in the Plugin Manager
 *
 * • Play Random Sound
 * Takes a list of sound ids, and randomly selects one to play. You can also
 * mix different types, and it will play the sound id as whatever type it is
 * in the plugin parameters.
 * ------------------------------Saved Games-----------------------------------
 * This plugin fully supports saved games.
 * 
 * This means the following should cause no issues even in saved games:
 * ✓ Add this plugin to your game
 * ✓ Modify this plugin's parameters
 * ✓ Remove this plugin from your game
 * -----------------------------Filename---------------------------------------
 * The filename for this plugin MUST remain CGMZ_SoundIDs.js
 * This is what it comes as when downloaded. The filename is used to load
 * parameters and execute plugin commands. If you change it, things will begin
 * behaving incorrectly and your game will probably crash. Please do not
 * rename the js file.
 * --------------------------Latest Version------------------------------------
 * Hi all, this latest version adds volume, pitch, and pan variances to your
 * sound id objects. When playing a sound effect, bgm, etc. if set up the
 * played sound will randomly vary within the range given in the parameters.
 * This can be used to slightly change sound effects and other audio in your
 * game so that it is not exactly the same every time.
 *
 * Version 1.1.0
 * - Added variance to volume
 * - Added variance to pitch
 * - Added variance to pan
 *
 * @command Play BGM
 * @desc Plays a BGM by ID
 *
 * @arg id
 * @desc The ID of the BGM to play
 *
 * @command Play BGS
 * @desc Plays a BGS by ID
 *
 * @arg id
 * @desc The ID of the BGS to play
 *
 * @command Play ME
 * @desc Plays a ME by ID
 *
 * @arg id
 * @desc The ID of the ME to play
 *
 * @command Play SE
 * @desc Plays a SE by ID
 *
 * @arg id
 * @desc The ID of the SE to play
 *
 * @command Play Sound
 * @desc Plays any sound by its ID
 *
 * @arg id
 * @desc The ID of the sound to play
 *
 * @command Play Random Sound
 * @desc Plays a random sound from the given list of ids
 *
 * @arg ids
 * @type text[]
 * @default []
 * @desc Array of possible IDs to play
 *
 * @param BGM
 * @type struct<BGMid>[]
 * @default []
 * @desc Set up BGM Ids here
 *
 * @param BGS
 * @type struct<BGSid>[]
 * @default []
 * @desc Set up BGS Ids here
 *
 * @param ME
 * @type struct<MEid>[]
 * @default []
 * @desc Set up ME Ids here
 *
 * @param SE
 * @type struct<SEid>[]
 * @default []
 * @desc Set up SE Ids here
*/
/*~struct~BGMid:
 * @param id
 * @desc The id of this BGM
 *
 * @param bgm
 * @type file
 * @dir audio/bgm
 * @desc The BGM file to play
 *
 * @param Volume
 * @type number
 * @min 0
 * @max 100
 * @default 90
 * @desc The volume the sound is played
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
 *
 * @param Volume Variance
 * @type number
 * @default 0
 * @desc Variance to apply (add/subtract) to the volume.
 *
 * @param Pitch Variance
 * @type number
 * @default 0
 * @desc Variance to apply (add/subtract) to the pitch.
 *
 * @param Pan Variance
 * @type number
 * @default 0
 * @desc Variance to apply (add/subtract) to the pan.
*/
/*~struct~BGSid:
 * @param id
 * @desc The id of this BGS
 *
 * @param bgs
 * @type file
 * @dir audio/bgs
 * @desc The BGS file to play
 *
 * @param Volume
 * @type number
 * @min 0
 * @max 100
 * @default 90
 * @desc The volume the sound is played
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
 *
 * @param Volume Variance
 * @type number
 * @default 0
 * @desc Variance to apply (add/subtract) to the volume.
 *
 * @param Pitch Variance
 * @type number
 * @default 0
 * @desc Variance to apply (add/subtract) to the pitch.
 *
 * @param Pan Variance
 * @type number
 * @default 0
 * @desc Variance to apply (add/subtract) to the pan.
*/
/*~struct~MEid:
 * @param id
 * @desc The id of this ME
 *
 * @param me
 * @type file
 * @dir audio/me
 * @desc The ME file to play
 *
 * @param Volume
 * @type number
 * @min 0
 * @max 100
 * @default 90
 * @desc The volume the sound is played
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
 *
 * @param Volume Variance
 * @type number
 * @default 0
 * @desc Variance to apply (add/subtract) to the volume.
 *
 * @param Pitch Variance
 * @type number
 * @default 0
 * @desc Variance to apply (add/subtract) to the pitch.
 *
 * @param Pan Variance
 * @type number
 * @default 0
 * @desc Variance to apply (add/subtract) to the pan.
*/
/*~struct~SEid:
 * @param id
 * @desc The id of this SE
 *
 * @param se
 * @type file
 * @dir audio/se
 * @desc The SE file to play
 *
 * @param Volume
 * @type number
 * @min 0
 * @max 100
 * @default 90
 * @desc The volume the sound is played
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
 *
 * @param Volume Variance
 * @type number
 * @default 0
 * @desc Variance to apply (add/subtract) to the volume.
 *
 * @param Pitch Variance
 * @type number
 * @default 0
 * @desc Variance to apply (add/subtract) to the pitch.
 *
 * @param Pan Variance
 * @type number
 * @default 0
 * @desc Variance to apply (add/subtract) to the pan.
*/
Imported.CGMZ_SoundIDs = true;
CGMZ.Versions["Sound IDs"] = "1.1.0";
CGMZ.SoundIDs = {};
CGMZ.SoundIDs.parameters = PluginManager.parameters('CGMZ_SoundIDs');
CGMZ.SoundIDs.BGM = CGMZ_Utils.parseJSON(CGMZ.SoundIDs.parameters["BGM"], [], "[CGMZ] Sound IDs", "The BGM parameter had invalid JSON and could not be read");
CGMZ.SoundIDs.BGS = CGMZ_Utils.parseJSON(CGMZ.SoundIDs.parameters["BGS"], [], "[CGMZ] Sound IDs", "The BGS parameter had invalid JSON and could not be read");
CGMZ.SoundIDs.ME = CGMZ_Utils.parseJSON(CGMZ.SoundIDs.parameters["ME"], [], "[CGMZ] Sound IDs", "The ME parameter had invalid JSON and could not be read");
CGMZ.SoundIDs.SE = CGMZ_Utils.parseJSON(CGMZ.SoundIDs.parameters["SE"], [], "[CGMZ] Sound IDs", "The SE parameter had invalid JSON and could not be read");
//=============================================================================
// CGMZ_SoundID
//-----------------------------------------------------------------------------
// Data class for sound IDs. Not saved
//=============================================================================
function CGMZ_SoundID() {
	this.initialize(...arguments);
}
//-----------------------------------------------------------------------------
// Initialize
//-----------------------------------------------------------------------------
CGMZ_SoundID.prototype.initialize = function(sound) {
	this.name = sound.se || sound.me || sound.bgs || sound.bgm;
	this.volume = Number(sound.Volume);
	this.pitch = Number(sound.Pitch);
	this.pan = Number(sound.Pan);
	this.type = (sound.se) ? "se" : (sound.me) ? "me" : (sound.bgs) ? "bgs" : "bgm";
	this.volumeVariance = Number(sound["Volume Variance"]);
	this.pitchVariance = Number(sound["Pitch Variance"]);
	this.panVariance = Number(sound["Pan Variance"]);
};
//-----------------------------------------------------------------------------
// Initialize
//-----------------------------------------------------------------------------
CGMZ_SoundID.prototype.getSoundObject = function() {
	return {
		name: this.name,
		type: this.type,
		volume: CGMZ_Utils.applyVariance(this.volume, this.volumeVariance + 1),
		pitch: CGMZ_Utils.applyVariance(this.pitch, this.pitchVariance + 1),
		pan: CGMZ_Utils.applyVariance(this.pan, this.panVariance + 1)
	}
};
//=============================================================================
// CGMZ_Temp
//-----------------------------------------------------------------------------
// Add parsed sound ids to temp data, add plugin commands
//=============================================================================
//-----------------------------------------------------------------------------
// Also initialize sound id data
//-----------------------------------------------------------------------------
const alias_CGMZ_SoundIDs_CGMZ_Temp_createPluginData = CGMZ_Temp.prototype.createPluginData;
CGMZ_Temp.prototype.createPluginData = function() {
	alias_CGMZ_SoundIDs_CGMZ_Temp_createPluginData.call(this);
	this.initializeSoundIDData();
};
//-----------------------------------------------------------------------------
// Initialize sound id data
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.initializeSoundIDData = function() {
	this._soundIDs = {};
	for(const soundArray of [CGMZ.SoundIDs.BGM, CGMZ.SoundIDs.BGS, CGMZ.SoundIDs.ME, CGMZ.SoundIDs.SE]) {
		for(soundJSON of soundArray) {
			const sound = CGMZ_Utils.parseJSON(soundJSON, null, "[CGMZ] Sound IDs", "One of your sound ids had invalid JSON and could not be parsed.");
			if(!sound) continue;
			if(!sound.id) {
				CGMZ_Utils.reportError("One of your sound ids had a blank id", "[CGMZ] Sound IDs", "Make sure your id parameter for each sound is unique and not blank");
				continue;
			}
			const obj = new CGMZ_SoundID(sound);
			this._soundIDs[sound.id] = obj;
		}
	}
};
//-----------------------------------------------------------------------------
// Get a sound object by ID
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.getSoundID = function(id) {
	const soundObj = this._soundIDs[id];
	return (soundObj) ? soundObj.getSoundObject() : null;
};
//-----------------------------------------------------------------------------
// Register Sound IDs Plugin Commands
//-----------------------------------------------------------------------------
const alias_CGMZ_SoundIDs_CGMZ_Temp_registerPluginCommands = CGMZ_Temp.prototype.registerPluginCommands;
CGMZ_Temp.prototype.registerPluginCommands = function() {
	alias_CGMZ_SoundIDs_CGMZ_Temp_registerPluginCommands.call(this);
	PluginManager.registerCommand("CGMZ_SoundIDs", "Play BGM", this.pluginCommandSoundIDsPlayBGM);
	PluginManager.registerCommand("CGMZ_SoundIDs", "Play BGS", this.pluginCommandSoundIDsPlayBGS);
	PluginManager.registerCommand("CGMZ_SoundIDs", "Play ME", this.pluginCommandSoundIDsPlayME);
	PluginManager.registerCommand("CGMZ_SoundIDs", "Play SE", this.pluginCommandSoundIDsPlaySE);
	PluginManager.registerCommand("CGMZ_SoundIDs", "Play Sound", this.pluginCommandSoundIDsPlaySound);
	PluginManager.registerCommand("CGMZ_SoundIDs", "Play Random Sound", this.pluginCommandSoundIDsPlayRandomSound);
};
//-----------------------------------------------------------------------------
// Plugin Command - Play BGM
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandSoundIDsPlayBGM = function(args) {
	const sound = $cgmzTemp.getSoundID(args.id);
	if(sound?.type === "bgm") AudioManager.playBgm(sound);
};
//-----------------------------------------------------------------------------
// Plugin Command - Play BGS
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandSoundIDsPlayBGS = function(args) {
	const sound = $cgmzTemp.getSoundID(args.id);
	if(sound?.type === "bgs") AudioManager.playBgs(sound);
};
//-----------------------------------------------------------------------------
// Plugin Command - Play ME
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandSoundIDsPlayME = function(args) {
	const sound = $cgmzTemp.getSoundID(args.id);
	if(sound?.type === "me") AudioManager.playMe(sound);
};
//-----------------------------------------------------------------------------
// Plugin Command - Play SE
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandSoundIDsPlaySE = function(args) {
	const sound = $cgmzTemp.getSoundID(args.id);
	if(sound?.type === "se") AudioManager.playSe(sound);
};
//-----------------------------------------------------------------------------
// Plugin Command - Play Sound
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandSoundIDsPlaySound = function(args) {
	const sound = $cgmzTemp.getSoundID(args.id);
	switch(sound?.type) {
		case "bgm": AudioManager.playBgm(sound); break;
		case "bgs": AudioManager.playBgs(sound); break;
		case "me": AudioManager.playMe(sound); break;
		case "se": AudioManager.playSe(sound); break;
	}
};
//-----------------------------------------------------------------------------
// Plugin Command - Play Random Sound
//-----------------------------------------------------------------------------
CGMZ_Temp.prototype.pluginCommandSoundIDsPlayRandomSound = function(args) {
	const array = CGMZ_Utils.parseJSON(args.ids, null, "[CGMZ] Sound IDs", "Your Plugin Command 'Play Random Sound' had invalid JSON and could not be parsed, it will do nothing.");
	if(!array || array.length === 0) return;
	const id = Math.randomInt(array.length);
	const sound = $cgmzTemp.getSoundID(array[id]);
	switch(sound?.type) {
		case "bgm": AudioManager.playBgm(sound); break;
		case "bgs": AudioManager.playBgs(sound); break;
		case "me": AudioManager.playMe(sound); break;
		case "se": AudioManager.playSe(sound); break;
	}
};