/*:
 * @target MZ
 * @plugindesc Add 1-turn auto battle to the party command
 * @author yuwaka
 *
 * @param autosuteid
 * @text State ID
 * @desc ID of the state to be granted during auto battle
 * @default 11
 * @type number
 * @min 1
 *
 * @param autocommandname
 * @text Command Name
 * @desc The name of the command to be added to the party command
 * @default Auto
 * @type string
 *
 * @help
 * It is a simple automatic battle system with states.
 * 
 * If you want to have automatic battle for one turn only, create a state with trait -> "special flag" -> "Auto Battle" as a feature.
 * Check the "Remove at Battle End" and "Remove by Restriction" checkboxes,
 * and specify "0~0" as the turn duration for Auto-removal Timing.
 * 
 * The first number of "Duration in Turns:" should be 0, even if you want it to last several turns.
 * If you want to release it at any time, you can control it with a battle event or use another plugin.
 * 
 * The command insertion location is between "Fight" and "Escape".
 * If you want to change it, please edit this file directly.
 * Conflicts with other plugin that change the party commands.
 *
 * here are no plugin commands in this plugin.
 * This is a plugin for RPG Maker MZ only.
 * Please feel free to modify and use it in accordance with the terms of the RPG Maker.
 * I will not be responsible for any problems that may occur. Please understand.
 */
/*:ja
 * @target MZ
 * @plugindesc パーティコマンドに1ターン自動戦闘を追加
 * @author ゆわか
 *
 * @param autosuteid
 * @text ステート番号
 * @desc 自動戦闘時に付与するステートのID
 * @default 11
 * @type number
 * @min 1
 *
 * @param autocommandname
 * @text コマンド名
 * @desc パーティコマンドに追加するコマンドの呼び名
 * @default オート
 * @type string
 *
 * @help
 * ステートを使った簡易自動戦闘システムです。
 * 
 * 1ターンだけ自動戦闘をさせる場合は
 * 特徴に「特殊フラグ・自動戦闘」をつけたステートを作成してください。
 * 解除条件は
 * 「戦闘終了時に解除」と「行動制約によって解除」にチェックを入れて
 * 自動解除のタイミングの持続ターンは「0～0」と指定してください。
 * 
 * 数ターン継続させる場合でも、ターンの最初の数字は0にしてください。
 * 任意のタイミングで解除したい場合は
 * バトルイベントで頑張るか別のプラグインを探してください。
 * 
 * コマンド挿入場所は、「戦う」と「逃げる」の間です。
 * 変更したい場合は、直接このファイルを編集してください。
 * パーティコマンドを変更するほかのプラグインとは競合します。
 *
 * プラグインコマンドはありません。
 * RPGツクールMZ専用です。
 * ツクールの規約に沿って自由に改変、使用してください。
 */
/*:zh
 * @target MZ
 * @plugindesc 在队伍指令中添加“一回合自动战斗”功能
 * @author yuwaka
 *
 * @param autosuteid
 * @text 状态ID
 * @desc 自动战斗时赋予角色的状态ID。
 * @default 11
 * @type number
 * @min 1
 *
 * @param autocommandname
 * @text 指令名称
 * @desc 添加到队伍指令中的命令名称。
 * @default 自动
 * @type string
 *
 * @help
 * 这是一个使用“状态”机制实现的简易自动战斗系统。
 *
 * 【设置方法】
 * 若希望实现“一回合自动战斗”，请创建一个状态，
 * 并在其“特征”中勾选：
 * 「特殊标志 → 自动战斗」。
 *
 * 同时，请勾选以下两项解除条件：
 * - “战斗结束时解除”
 * - “根据行动限制解除”
 *
 * 在“自动解除的持续回合”中输入「0～0」。
 *
 * 即使希望持续数回合，也请将起始数值设为 0。
 *
 * 若希望在任意时机手动解除，
 * 可以通过“战斗事件”或其他插件进行控制。
 *
 * 【指令位置】
 * 新的“自动”指令会插入在“战斗”和“逃跑”之间。
 * 若需更改位置，可直接编辑此插件文件中的相关代码。
 *
 * 【注意事项】
 * - 若使用其他修改“队伍指令”的插件，可能会产生冲突。
 * - 本插件没有插件命令。
 * - 仅适用于 RPG Maker MZ。
 * - 可自由修改、再分发或商用。
 */

(function(){

//パラメータ用変数の設定
    var parameters = PluginManager.parameters('StateAutoBattle');
    var autosuteid = Number(parameters['autosuteid'] || 11);
    var autocommandname = parameters['autocommandname'] || 'オート';


// rmmz_windows コマンドの順番を変えたい時はthis.addCommandの３行の順番を変える
Window_PartyCommand.prototype.makeCommandList = function() {
    this.addCommand(TextManager.fight, "fight");
    this.addCommand(autocommandname, "auto");//追加
    this.addCommand(TextManager.escape, "escape", BattleManager.canEscape());
};

//rmmz_scenes
Scene_Battle.prototype.createPartyCommandWindow = function() {
    const rect = this.partyCommandWindowRect();
    const commandWindow = new Window_PartyCommand(rect);
    commandWindow.setHandler("fight", this.commandFight.bind(this));
    commandWindow.setHandler("auto", this.commandAuto.bind(this));//追加
    commandWindow.setHandler("escape", this.commandEscape.bind(this));
    commandWindow.deselect();
    this.addWindow(commandWindow);
    this._partyCommandWindow = commandWindow;
};

//新しいシーンを追加、ここでステートを付与している。
Scene_Battle.prototype.commandAuto = function() {
    $gameParty.members().forEach(function(actor) {
    actor.addState(autosuteid);
    });
   //BattleManager.updateTurnEnd();
    if (BattleManager.isTpb()) {
        BattleManager.startTurn();
    } else {
        BattleManager.startInput();
    }
};


}());
