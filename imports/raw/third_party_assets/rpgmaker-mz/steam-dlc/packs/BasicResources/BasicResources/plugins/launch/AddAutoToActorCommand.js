//=============================================================================
// Plugin for RPG Maker MZ
// AddAutoToActorCommand.js
//=============================================================================
/*:
 * @target MZ
 * @plugindesc Add 'Auto' command on the top or bottom of Actor Command
 * @author Sasuke KANNAZUKI
 *
 * @param commandName
 * @text Command Name
 * @desc Displaying command name that 'Auto' command.
 * @type string
 * @default Auto Select
 *
 * @param autoCommandPos
 * @text Auto Command Position
 * @desc Adding position of 'Auto' command in the window
 *  (0:Top 1:Bottom)
 * @type select
 * @option Top
 * @value 0
 * @option Bottom
 * @value 1
 * @default 0
 *
 * @help This plugin does not provide plugin commands.
 * This plugin runs under RPG Maker MZ.
 * 
 * This plugin adds 'Auto' command on the top or bottom of Actor Command.
 *
 * [Summary]
 * When player select 'Auto' command, the actor performs appropriate action.
 * - The action is the same as one when the actor has the trait 'Auto Battle'.
 * - The 'Auto' commands works only current turn. The next turn, actor command
 *  window appears again.
 *
 * [Note]
 * When the actor can more than 1 actions and once select 'Auto',
 * all actions become ones that auto battle routine decide,
 * and previous inputted actions are ignored.
 *
 * [Note Setting]
 * Write actor's note <NoAutoCommand> not to display 'Auto'
 * to the actor's command.
 *
 * [License]
 * this plugin is released under MIT license.
 * http://opensource.org/licenses/mit-license.php
 */

/*:ja
 * @target MZ
 * @plugindesc 戦闘のアクターコマンドの先頭か後尾に「オート」を追加します
 * @author 神無月サスケ
 *
 * @param commandName
 * @text コマンド名
 * @desc オートコマンドの表示名です。
 * @type string
 * @default おまかせ
 *
 * @param autoCommandPos
 * @text オートコマンド位置
 * @desc オートコマンドを先頭か末尾、どちらに置くか
 *  (0:先頭, 1:末尾)
 * @type select
 * @option 先頭
 * @value 0
 * @option 末尾
 * @value 1
 * @default 0
 *
 * @help このプラグインにプラグインコマンドはありません。
 * このプラグインは、RPGツクールMZに対応しています。
 *
 * このプラグインを導入すると、戦闘時のアクターコマンドの先頭か末尾に
 * オート（自動で適切な行動を決める）が追加されるようになります。
 *
 * ■概要
 * - オートが選ばれたときに採用される行動は、そのアクターが「自動戦闘」の
 *   特徴を持っている時と同様になります。
 * - 自動で行動するのは、オートが選ばれたターンだけです。
 *   次のターンでは再びアクターコマンドが表示されます。
 *
 * ■注意
 * 複数回行動が可能なアクターの場合、先に別のコマンドを入力していても
 * オートを選択した場合、それらは無視され、全て自動戦闘のコマンドに
 * 切り替わります。
 *
 * ■メモの書式
 * アクターのメモに <NoAutoCommand> と記述すると、
 * そのアクターのコマンドにはオートが追加されなくなります。
 *
 * ■ライセンス表記
 * このプラグインは MIT ライセンスで配布されます。
 * ご自由にお使いください。
 * http://opensource.org/licenses/mit-license.php
 */

/*:zh
 * @target MZ
 * @plugindesc 在战斗的角色指令中添加“自动战斗”选项（可置于顶部或底部）
 * @author Sasuke KANNAZUKI
 *
 * @param commandName
 * @text 指令名称
 * @desc “自动战斗”指令在战斗菜单中显示的名称。
 * @type string
 * @default 自动选择
 *
 * @param autoCommandPos
 * @text 指令位置
 * @desc “自动战斗”指令在窗口中的位置。（0：顶部  1：底部）
 * @type select
 * @option 顶部
 * @value 0
 * @option 底部
 * @value 1
 * @default 0
 *
 * @help AddAutoToActorCommand.js
 *
 * 本插件无需插件命令，仅适用于 RPG Maker MZ。
 * 
 * 功能说明：
 * 该插件会在战斗时的角色指令列表中添加“自动战斗（Auto）”选项，
 * 位置可设置为顶部或底部。
 *
 * 【概要】
 * 当玩家选择“自动战斗”时，该角色会执行系统自动选择的行动：
 * - 其行为逻辑与角色拥有“自动战斗（Auto Battle）”特征时相同。
 * - “自动战斗”仅在本回合有效，下一回合将恢复为普通指令选择。
 *
 * 【注意事项】
 * 若角色能在一回合内执行多个行动，
 * 一旦选择了“自动战斗”，则所有行动都将由系统自动决定，
 * 已经手动输入的指令将被忽略。
 *
 * 【备注设置】
 * 若希望某个角色不显示“自动战斗”选项，
 * 请在该角色的备注栏中添加以下标签：
 * <NoAutoCommand>
 *
 * 【许可信息】
 * 本插件以 MIT 协议发布，可自由使用、修改与分发。
 * 详情请参阅：
 * http://opensource.org/licenses/mit-license.php
 */

(() => {
  const pluginName = 'AddAutoToActorCommand';
  //
  // process parameters
  //
  const parameters = PluginManager.parameters(pluginName);
  const commandName = parameters['commandName'] || 'Auto Select';
  const yPosType = Number(parameters['autoCommandPos'] || 0);

  //
  // add command to actor command window
  //
  const _Scene_Battle_createActorCommandWindow =
   Scene_Battle.prototype.createActorCommandWindow;
  Scene_Battle.prototype.createActorCommandWindow = function() {
    _Scene_Battle_createActorCommandWindow.call(this);
    this._actorCommandWindow.setHandler('auto', this.commandAuto.bind(this));
  };

  const doesDisplayAuto = actor => actor && !actor.actor().meta.NoAutoCommand;

  const _Window_ActorCommand_makeCommandList =
   Window_ActorCommand.prototype.makeCommandList;
  Window_ActorCommand.prototype.makeCommandList = function() {
    if (doesDisplayAuto(this._actor) && yPosType === 0) {
      this.addAutoCommand();
    }
    _Window_ActorCommand_makeCommandList.call(this);
    if (doesDisplayAuto(this._actor) && yPosType === 1) {
      this.addAutoCommand();
    }
  };

  Window_ActorCommand.prototype.addAutoCommand = function() {
    this.addCommand(commandName, 'auto');
  };

  Scene_Battle.prototype.commandAuto = function() {
    const actor = BattleManager._currentActor;
    if (actor) {
      actor.makeAutoBattleActions();
    }
    BattleManager.finishActorInput();
    this.changeInputWindow();
    BattleManager.selectNextActor();
  };

})();
