/*:
 * @target MZ
 * @plugindesc Start with an actor command in time progress battle.
 * @author yuwaka
 *
 * @help
 * Skip the first party command “Fight, Escape” in Time Progress Battle
 * and start by entering the actor command. (Does not affect turn-based combat)
 * 
 * To invoke a party command, press the cancel key or the back icon.
 *
 * There is no plug-in command.
 * Privately for RPG Maker MZ.
 *
 */

/*:ja
 * @target MZ
 * @plugindesc タイムプログレス戦闘でアクターコマンドから開始します。
 * @author ゆわか
 *
 * @help
 * タイムプログレス戦闘で最初のパーティコマンド「戦う、逃げる」をスキップし
 * アクターコマンドの入力から開始します。 （ターン制戦闘には影響しません）
 * 
 * パーティコマンドを呼び出すには、キャンセルキーまたは戻るアイコンを押します。
 *
 * プラグインコマンドはありません。
 * RPGツクールMZ専用です。
 *
 */

/*:zh
 * @target MZ
 * @plugindesc 在时间进度战斗（TPB）中，从角色指令开始输入。
 * @author yuwaka
 *
 * @help
 * 本插件在时间进度战斗（TPB）中会跳过最初的队伍指令
 * “战斗 / 逃跑”，直接从角色指令输入开始。
 * （此设置不会影响回合制战斗模式）
 * 
 * 若要呼出队伍指令，请按取消键或点击返回图标。
 *
 * 本插件没有插件命令。
 * 本插件仅适用于 RPG Maker MZ。
 */

// プラグインで定義した変数が外に漏れないようにする（競合対策）
(() => {
    // 元のBattleManager.initMembers関数を覚えておく
    const _BattleManager_initMembers = BattleManager.initMembers;
    // 再定義して・・・
    BattleManager.initMembers = function() {
        // 覚えておいた元の関数を呼び出す
        _BattleManager_initMembers.apply(this, arguments);
        // 追加部分
        this._tpbNeedsPartyCommand = false;
    };
})();

