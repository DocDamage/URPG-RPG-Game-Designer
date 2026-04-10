//=============================================================================
// NoGameover2.js
//=============================================================================
/*:
 * @target MZ
 * @plugindesc If you lose in a battle, the game is not over.
 * @author yuwaka

 * @param Switch ID
 * @desc The switch ID to turn on when you Dead on the map or in a battle.
 * @default 0
 * @type switch
 *
 * @param Switch ID2
 * @desc The ID of the switch to turn on when lose in a battle.
 * @default 0
 * @type switch
 *
 * @help There are no plugin commands in this plugin.
 *
 *　Allow it to fade out automatically.
 *　Fade in at any time you put into the common event you call by switch.
 *
 *　Automatically "Change Menu Acsess" is "Disable".
 *　(To prevent the menu icon from blinking.)
 *　If necessary, remove the Disable.
 *　Comment out if you don't want to fade out or ban the menu.
 *
 *　If the "Battle Processing" use, if you have a set of defeats, that takes precedence.
 *
 * There are no plug-in commands.
 * This is a plugin for RPG Maker MZ only.
 * Please feel free to modify and use it in accordance with the terms of the RPG Maker.
 * I will not be responsible for any problems that may occur. Please understand.
 *
 * Special Thanks:I used the plugin for the RPG Maker MV sample game "Sea Pirate(Auther Tachi)" as a reference.
 */
/*:ja
 * @target MZ
 * @plugindesc 全滅してもゲームオーバーにならない。
 * @author ゆわか

 * @param Switch ID
 * @desc マップ上でも戦闘でも全滅したときにONにするスイッチのIDです。
 * @default 0
 * @type switch
 *
 * @param Switch ID2
 * @desc 戦闘で全滅したときにONにするスイッチのIDです。
 * @default 0
 * @type switch
 *
 * @help このプラグインには、プラグインコマンドはありません。
 *
 *　自動的にフェードアウトするようにしています。
 *　フェードインはスイッチで呼び出すコモンイベントへ入れて
 *　好きなタイミングで行ってください。
 *
 *　自動的にメニューを禁止するようにしています。
 *　メニューアイコンの点滅を防ぐためです。
 *　必要に応じて、禁止を解いてください。
 *
 *　（フェードアウトやメニューを禁止したくなければ、コメントアウトしてね）
 *
 *　イベント中の戦闘で、敗北の設定をしている場合は、そちらが優先されます。
 *
 * プラグインコマンドはありません。
 * ＲＰＧツクールＭＺ専用のプラグインです。
 * ツクールの規約に沿って自由に改変、使用してください。
 *
 *　ＲＰＧツクールＭＶサンプルゲーム「シーピラート」のプラグインを
 *　参考にさせて頂きました。
 *　ありがとうございます。
 */

/*:zh
 * @target MZ
 * @plugindesc 即使在战斗中失败，游戏也不会结束。
 * @author yuwaka
 *
 * @param Switch ID
 * @text 开关ID
 * @desc 当在地图上或战斗中全灭时要打开的开关ID。
 * @default 0
 * @type switch
 *
 * @param Switch ID2
 * @text 战斗失败开关ID
 * @desc 当在战斗中失败时要打开的开关ID。
 * @default 0
 * @type switch
 *
 * @help 本插件没有插件命令。
 *
 * 会在全灭或战斗失败后自动进行淡出处理。
 * 淡入请放入通过开关调用的公共事件中，
 * 并在你希望的时机执行。
 *
 * 插件会自动禁止菜单，
 * 以防止菜单图标闪烁。
 * 如有需要，请解除禁止。
 * 如果不想淡出或禁止菜单，可以将对应部分注释掉。
 *
 * 当事件中使用“战斗处理”并设置了“战败时的处理”时，
 * 会优先执行事件中设置的处理。
 *
 * 本插件没有插件命令。
 * 本插件仅适用于 RPG Maker MZ。
 * 你可以根据 RPG Maker 的使用条款自由修改和使用。
 * 使用本插件造成的任何问题，作者概不负责。
 *
 * 特别鸣谢：本插件参考了 RPG Maker MV 示例游戏
 * “海盗（Sea Pirate，作者：Tachi）”中的插件。
 */

(function() {

    var parameters = PluginManager.parameters('NoGameover2');
    var switchId = Number(parameters['Switch ID'] || 0);
    var switchId2 = Number(parameters['Switch ID2'] || 0);

//rpg_managers.jsより（戦闘で全滅した場合）
BattleManager.updateBattleEnd = function() {
    if (this.isBattleTest()) {
        AudioManager.stopBgm();
        SceneManager.exit();
    } else if (!this._escaped && $gameParty.isAllDead()) {
        if (this._canLose) {
            $gameParty.reviveBattleMembers();
            SceneManager.pop();
        } else {
            //SceneManager.goto(Scene_Gameover);//ゲームオーバーを表示するぜ
	    $gameSystem.disableMenu()//メニューを禁止するぜ
            $gameScreen.startFadeOut(10); //フェードアウトするぜ
	    $gameSwitches.setValue(switchId, true); //全滅したぜ
	    $gameSwitches.setValue(switchId2, true); //戦闘中に全滅したぜ
            $gameParty.reviveBattleMembers(); //みんな生き返るぜ
            SceneManager.pop(); //マップ画面へ移動するぜ
        }
    } else {
        SceneManager.pop();
    }
    this._phase = "";
};

//rmmz_scenes.jsより（フィールドで全滅した場合）
Scene_Base.prototype.checkGameover = function() {
    if ($gameParty.isAllDead()) {
//        SceneManager.goto(Scene_Gameover);//ゲームオーバーを表示するぜ
	    $gameSystem.disableMenu()//メニューを禁止するぜ
            $gameScreen.startFadeOut(10); //フェードアウトするぜ
	    $gameSwitches.setValue(switchId, true); //全滅したぜ
            $gameParty.reviveBattleMembers(); //みんな生き返るぜ
    }
};

})();
