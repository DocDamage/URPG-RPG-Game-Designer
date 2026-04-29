/*:
 * @target MZ
 * @plugindesc Gives you a state when you level up
 * @author yuwaka
 *
 * @param leupid
 * @text State ID
 * @desc Granting state ID
 * @default 11
 * @type number
 * @min 1
 *
 * @help
 * Sideview is a state-based level-up display system for combat.
 * 
 * Create a state to be granted at level up and set the "priority" to about 99.
 * Set "[SV]Overlay".
 * Check the "Removal Conditions", "Remove at Battle End".
 * 
 * Make the "[SV] Overlay" animation yourself.
 *
 * There are no plugin commands.
 * It is exclusive to RPG Maker MZ.
 * Please feel free to modify and use it in accordance with the terms of the RPG Maker.
 * I will not be responsible for any problems that may occur. Please understand.
 */
/*:ja
 * @target MZ
 * @plugindesc レベルアップ時にステートを付与
 * @author ゆわか
 *
 * @param leupid
 * @text ステート番号
 * @desc 付与するステートのID
 * @default 11
 * @type number
 * @min 1
 *
 * @help
 * サイドビュー戦闘用のステートを使ったレベルアップ表示システムです。
 * 
 * レベルアップ時に付与するステートを作成し
 * 「優先度」を99くらいに設定。
 * 「重ね合わせ」を設定。
 * 「解除条件」の「戦闘終了時に解除」をチェック。
 * 
 * 「重ね合わせ」のアニメーションは自分で作ってください。
 * 枠が余ってない場合は、誰かに重ね合わせのアニメを増やすプラグインを
 * 作ってもらうか、探してみてください。
 *
 * レベルアップメッセージと付与タイミングを合わせたい場合は
 * 誰かに作ってもらうか、探してみてください。
 *
 * プラグインコマンドはありません。
 * RPGツクールMZ専用です。
 * ツクールの規約に沿って自由に改変、使用してください。
 */
/*:zh
 * @target MZ
 * @plugindesc 升级时自动附加状态
 * @author yuwaka
 *
 * @param leupid
 * @text 状态编号
 * @desc 要附加的状态ID
 * @default 11
 * @type number
 * @min 1
 *
 * @help
 * 这是一个使用状态来在侧视战斗中显示“升级”效果的系统。
 * 
 * 请先创建一个在升级时附加的状态，
 * 将“优先级”设置为大约 99。
 * 勾选“重叠显示”。
 * 在“解除条件”中勾选“战斗结束时解除”。
 * 
 * “重叠显示”的动画需要自行制作。
 * 如果动画栏位不足，可以找人制作或寻找
 * 扩展重叠动画数量的插件。
 *
 * 如果想与升级消息的显示时机同步，
 * 也可以找现成插件或请他人制作。
 *
 * 本插件没有插件命令。
 * 本插件仅适用于 RPG Maker MZ。
 * 可根据 RPG Maker 使用协议自由修改和使用。
 */


(function(){

//パラメータ用変数の設定
    var parameters = PluginManager.parameters('Levelupstate');
    var leupid = Number(parameters['leupid'] || 11);



// rmmz_objects より抜粋
Game_Actor.prototype.levelUp = function() {
    this._level++;
	if($gameParty.inBattle()){
	$gameActors.actor(this._actorId).addState(leupid);
	}
    for (const learning of this.currentClass().learnings) {
        if (learning.level === this._level) {
            this.learnSkill(learning.skillId);
        }
    }
};


}());