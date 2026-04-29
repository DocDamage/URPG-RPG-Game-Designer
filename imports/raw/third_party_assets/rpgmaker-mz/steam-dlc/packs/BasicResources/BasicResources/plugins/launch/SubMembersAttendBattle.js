//=============================================================================
// Plugin for RPG Maker MZ
// SubMembersAttendBattle.js
//=============================================================================
// [Update History]
// 2020.Jan.24: Ver0.0.1 First release to closed members
// 2020.Feb.15: Ver0.0.2 Fix bug : invoke error when member > 4 and open menu
// 2020.Jul.07: Ver1.0.0 First release
// 2020.Oct.09: Ver1.0.1 Fix bug : when sub member was attacked by counter,
//   the member might be dead and no method to revive.
// 2020.Nov.29: Ver1.1.0 Add plugin command that dynamically change an actor
//   whether to attend battle or not.

/*:
 * @target MZ
 * @plugindesc [Ver1.1.0]Make Sub Members(=NPC) Attend Battle and does auto actions.
 * @author Sasuke KANNAZUKI
 *
 * @param subMemberIdVal1
 * @text Var ID for sub member 1
 * @desc Variable ID for actor ID of sub member 1.
 * @type variable
 * @default 1
 *
 * @param subMemberIdVal2
 * @text Var ID for sub member 2
 * @desc Variable ID for actor ID of sub member 2.
 * @type variable
 * @default 2
 *
 * @param subMemberIdVal3
 * @text Var ID for sub member 3
 * @desc Variable ID for actor ID of sub member 3.
 * @type variable
 * @default 3
 *
 * @param subMemberIdVal4
 * @text Var ID for sub member 4
 * @desc Variable ID for actor ID of sub member 4.
 * @type variable
 * @default 4
 *
 * @command set
 * @text Set NPC Attend Battle
 * @desc Set whether to attend battle the sub member
 *
 * @arg settingMode
 * @text ActorId or NPC Position
 * @desc Which to set actor id.
 * @type select
 * @option Actor Id
 * @value actorId
 * @option NPC Position
 * @value followerId
 * @default actorId
 *
 * @arg actorId
 * @parent settingMode
 * @text Actor ID
 * @desc When you adopt NPC Position, this setting is ignored.
 * @type actor
 * @default 1
 *
 * @arg followerId
 * @parent settingMode
 * @text NPC Position
 * @desc Set between from 1 to 4. If you adopt Actor Id, this setting is ignored.
 * @type number
 * @max 4
 * @min 1
 * @default 1
 *
 * @arg doesAttend
 * @text Attend the battle?
 * @desc When you select 'reset', it'll be default(note description) setting.
 * @type select
 * @option Yes to attend
 * @value yes
 * @option No to attend
 * @value no
 * @option Reset the plugin setting
 * @value reset
 * @default reset
 *
 * @help
 * This plugin runs under RPG Maker MZ.
 * 
 * This plugin enables Sub Members(=NPC) attend battle.
 *
 * [Summary]
 * A sub member is an actor but not displayed actor list in menu,
 * because sum members are not included in the party.
 *
 * Since sub members are not party members...
 * - at battle scene, sub members' HP/MP and other status is not displayed.
 *   Actually, sub members never become attack target.
 * - Sub members never consume HP/MP.
 * (i.e. Sub members' hp and mp is not considered while all battle situations.)
 * - Sub members never get EXP at battle end.
 * 
 * [Recommended Usage]
 * Use together with MenuSubMembersMZ.js to display sub members on menu and
 * followers on map.
 * In that case, set the options 'Var ID for submember' at the same value
 * as that of MenuSubMembersMZ.js.
 *
 * [Advanced Option]
 * Write an actor's note <NonFightNPC> and the actor doesn't attend the battle
 * when the actor become sub member.
 *
 * [Plugin Command]
 * When you change the battle attending condition for any sub member,
 * You can do it by a plugin command.
 * NOTE: Please call this pluguin command on map event.
 * If you call the plugin command on battle, that doesn't work well.
 *
 * [License]
 * this plugin is released under MIT license.
 * http://opensource.org/licenses/mit-license.php
 */

/*:ja
 * @target MZ
 * @plugindesc [Ver1.1.0]同行者(NPC)を戦闘に参加させ、自動戦闘を行わせます
 * @author 神無月サスケ
 *
 * @param subMemberIdVal1
 * @text 同行者1用変数ID
 * @desc 同行者1のアクターIDを指定する変数ID
 * @type variable
 * @default 1
 *
 * @param subMemberIdVal2
 * @text 同行者2用変数ID
 * @desc 同行者2のアクターIDを指定する変数ID
 * @type variable
 * @default 2
 *
 * @param subMemberIdVal3
 * @text 同行者3用変数ID
 * @desc 同行者3のアクターIDを指定する変数ID
 * @type variable
 * @default 3
 *
 * @param subMemberIdVal4
 * @text 同行者4用変数ID
 * @desc 同行者4のアクターIDを指定する変数ID
 * @type variable
 * @default 4
 *
 * @command set
 * @text NPC戦闘参加設定
 * @desc 指定したアクターが同行者の時の戦闘参加の如何を動的に設定します。
 *
 * @arg settingMode
 * @text アクターID？同行者位置？
 * @desc 対象のアクターをどちらで設定するか
 * @type select
 * @option アクターID
 * @value actorId
 * @option 同行者位置
 * @value followerId
 * @default actorId
 *
 * @arg actorId
 * @parent settingMode
 * @text アクターID
 * @desc アクターIDで指定する場合、ここで設定。同行者位置の時は無視
 * @type actor
 * @default 1
 *
 * @arg followerId
 * @parent settingMode
 * @text 同行者位置
 * @desc 前から順に1～4の値で設定。アクターIDの時は無視
 * @type number
 * @max 4
 * @min 1
 * @default 1
 *
 * @arg doesAttend
 * @text 戦闘に参加？
 * @desc リセットを選ぶとメモの設定を参照するようになります。
 * @type select
 * @option 参加させる
 * @value yes
 * @option 参加させない
 * @value no
 * @option リセット
 * @value reset
 * @default reset
 *
 * @help
 * このプラグインは、RPGツクールMZに対応しています。
 *
 * このプラグインは、同行者(NPC)を戦闘にさせることが出来ます。
 *
 * ■概要
 * 同行者とは、パーティーメンバーに属さず、メニューのアクターリストにも
 * 表示されないが、同行しているアクターです。
 *
 * 同行者はパーティーメンバーではないため、戦闘中、ステータスは表示されません。
 * 敵からの攻撃の対象になることも、仲間の回復スキルで回復することもありません。
 * すなわち、同行者のHP/MPは一切考慮されないということです。
 * また、同行者は戦闘勝利時に経験値を得ることもありません。
 *
 * ■推奨される使用法
 * MenuSubMembersMZ.js はメニュー画面および隊列に同行者を表示させます。
 * こちらと併用する場合、同行者用の変数IDを同じように設定するといいでしょう。
 *
 * ■メモの記述
 * アクターのメモに <NonFightNPC> と書いた場合、そのアクターが同行者の時
 * 戦闘には参加しません。
 *
 * ■プラグインコマンド
 * 同行者の戦闘参加の是非を途中で変更したい時に呼び出してください。
 * デフォルト(メモの記述)に戻すことも可能です。
 * 注意：この設定は戦闘中は無効です。必ずマップ上で行ってください。
 *
 * ■ライセンス表記
 * このプラグインは MIT ライセンスで配布されます。
 * ご自由にお使いください。
 * http://opensource.org/licenses/mit-license.php
 */

/*:zh
 * @target MZ
 * @plugindesc [Ver1.1.0] 让随行成员（NPC）参与战斗并执行自动行动
 * @author Sasuke KANNAZUKI
 *
 * @param subMemberIdVal1
 * @text 随行成员1变量ID
 * @desc 用于指定随行成员1的角色ID的变量ID。
 * @type variable
 * @default 1
 *
 * @param subMemberIdVal2
 * @text 随行成员2变量ID
 * @desc 用于指定随行成员2的角色ID的变量ID。
 * @type variable
 * @default 2
 *
 * @param subMemberIdVal3
 * @text 随行成员3变量ID
 * @desc 用于指定随行成员3的角色ID的变量ID。
 * @type variable
 * @default 3
 *
 * @param subMemberIdVal4
 * @text 随行成员4变量ID
 * @desc 用于指定随行成员4的角色ID的变量ID。
 * @type variable
 * @default 4
 *
 * @command set
 * @text 设置NPC战斗参与状态
 * @desc 动态设置指定角色（作为随行成员）是否参加战斗。
 *
 * @arg settingMode
 * @text 角色ID 或 随行位置
 * @desc 选择以哪种方式指定目标角色。
 * @type select
 * @option 角色ID
 * @value actorId
 * @option 随行成员位置
 * @value followerId
 * @default actorId
 *
 * @arg actorId
 * @parent settingMode
 * @text 角色ID
 * @desc 若选择以角色ID指定，则在此设置。若以随行位置指定，则忽略此项。
 * @type actor
 * @default 1
 *
 * @arg followerId
 * @parent settingMode
 * @text 随行位置
 * @desc 从前到后依次为1～4。当以角色ID指定时此设置无效。
 * @type number
 * @max 4
 * @min 1
 * @default 1
 *
 * @arg doesAttend
 * @text 是否参与战斗
 * @desc 若选择“重置”，则恢复为默认（备注中设置）的状态。
 * @type select
 * @option 参加战斗
 * @value yes
 * @option 不参加战斗
 * @value no
 * @option 重置为默认
 * @value reset
 * @default reset
 *
 * @help
 * 本插件适用于 RPG Maker MZ。
 *
 * 本插件允许随行成员（NPC）参与战斗。
 *
 * 【概要】
 * 随行成员指未加入正式队伍、不会显示在菜单角色列表中的角色，
 * 但会跟随主角行动。
 *
 * 因为随行成员不是队伍成员，因此：
 * - 战斗中不会显示其HP/MP等状态；
 * - 敌人无法以随行成员为攻击目标；
 * - 随行成员不会消耗HP/MP；
 * - 战斗胜利后不会获得经验值。
 *
 * 【推荐用法】
 * 可与插件 **MenuSubMembersMZ.js** 一起使用，
 * 使随行成员在菜单与地图中同时可见。
 * 若搭配使用，请在两个插件中设置相同的“随行成员变量ID”。
 *
 * 【备注说明】
 * 若在角色的备注栏中写入：
 *   <NonFightNPC>
 * 则该角色作为随行成员时不会参加战斗。
 *
 * 【插件命令】
 * 可在游戏过程中动态切换随行成员是否参与战斗，
 * 也可以通过“重置”恢复备注中的默认设置。
 *
 * 注意：
 * - 请务必在“地图事件”中调用插件命令；
 * - 在战斗中调用不会生效。
 *
 * 【许可证】
 * 本插件基于 MIT 许可证发布。
 * 您可自由修改、再分发或用于任何形式的作品（包括商业与成人游戏）。
 * http://opensource.org/licenses/mit-license.php
 */

(() => {
  const pluginName = 'SubMembersAttendBattle';
  //
  // process parameters
  //
  const parameters = PluginManager.parameters(pluginName);
  const submemberVar1 = Number(parameters['subMemberIdVal1'] || 1);
  const submemberVar2 = Number(parameters['subMemberIdVal2'] || 2);
  const submemberVar3 = Number(parameters['subMemberIdVal3'] || 3);
  const submemberVar4 = Number(parameters['subMemberIdVal4'] || 4);

  //
  // get sub members
  //
  const subMemberVarList = [submemberVar1, submemberVar2, submemberVar3,
    submemberVar4
  ];

  Game_Party.prototype.isBattleSubMember = function(actor) {
    // this._isBattleSubMember = { actorId => boolean or undefined }
    this._isBattleSubMember = this._isBattleSubMember || {};
    // when set by plugin commands
    const actorId = actor ? actor.id : 0;
    const isSub = actorId ? this._isBattleSubMember[actorId] : false;
    if (isSub != null) {
      return isSub;
    }
    // check the note description
    return actor && !actor.meta.NonFightNPC;
  };

  const _subMemberIds = () => {
    let members = [];
    for (const varId of subMemberVarList) {
      if (varId > 0) {
        const subMemberID = $gameVariables.value(varId);
        if (subMemberID > 0) {
          const actor = $dataActors[subMemberID];
          if ($gameParty.isBattleSubMember(actor)) {
            members.push(subMemberID);
          }
        }
      }
    }
    return members;
  };

  //
  // process plugin command
  //
  PluginManager.registerCommand(pluginName, 'set', args => {
    let actor = null;
    switch (args.settingMode) {
    case 'actorId':
      actor = $dataActors[+args.actorId];
      break;
    case 'followerId':
      const varIdForActor = subMemberVarList[+args.followerId - 1];
      if (varIdForActor) {
        const actorId = $gameVariables.value(varIdForActor);
        actor = $dataActors[actorId];
      }
    }
    if (actor) {
      const actorId2 = actor.id;
      $gameParty._isBattleSubMember = $gameParty._isBattleSubMember || {};
      switch (args.doesAttend) {
      case 'yes':
        $gameParty._isBattleSubMember[actorId2] = true;
        break;
      case 'no':
        $gameParty._isBattleSubMember[actorId2] = false;
        break;
      case 'reset':
        $gameParty._isBattleSubMember[actorId2] = null;
        break;
      }
    }
  });

  //--------------------------------------------------------------------------
  // Game_SubMembers
  // The third unit following Game_Party and Game_Troop.
  //
  function Game_SubMembers() {
    this.initialize(...arguments);
  }

  Game_SubMembers.prototype = Object.create(Game_Unit.prototype);
  Game_SubMembers.prototype.constructor = Game_SubMembers;

  Game_SubMembers.prototype.initialize = function() {
    Game_Unit.prototype.initialize.call(this);
    this._inBattle = true;
    this._subMembers = _subMemberIds();
  };

  Game_SubMembers.prototype.members = function() {
    return this._subMembers.map(id => $gameActors.actor(id));
  };

  //
  // define sub member variable used only in the battle
  //
  let $gameSubMembers = null;

  const hasSubMember = () => !!$gameSubMembers;

  const _Game_Party_onBattleStart = Game_Party.prototype.onBattleStart;
  Game_Party.prototype.onBattleStart = function(advantageous) {
    _Game_Party_onBattleStart.call(this, advantageous);
    $gameSubMembers = new Game_SubMembers();
    $gameSubMembers.onBattleStart(advantageous);
  };

  const _Scene_Battle_terminate = Scene_Battle.prototype.terminate;
  Scene_Battle.prototype.terminate = function() {
    _Scene_Battle_terminate.call(this);
    $gameSubMembers = null;
  };

  //
  // add sub members to battle members
  //
  const _BattleManager_allBattleMembers = BattleManager.allBattleMembers;
  BattleManager.allBattleMembers = function() {
    let battleMembers = _BattleManager_allBattleMembers.call(this);
    if (hasSubMember()) {
      battleMembers = battleMembers.concat($gameSubMembers.members());
    }
    return battleMembers;
  };

  const _BattleManager_makeActionOrders = BattleManager.makeActionOrders;
  BattleManager.makeActionOrders = function() {
    _BattleManager_makeActionOrders.call(this);
    const subMembers = hasSubMember() ? $gameSubMembers.members() : [];
    for (const battler of subMembers) {
      battler.makeSpeed();
    }
    const battlers = subMembers.concat(this._actionBattlers);
    battlers.sort((a, b) => b.speed() - a.speed());
    this._actionBattlers = battlers;
  };

  //
  // judge functions whether the actor is sub member or not.
  //
  Game_BattlerBase.prototype.isSubMember = function() {
    return false;
  };

  Game_Actor.prototype.isSubMember = function () {
    if (!hasSubMember()) {
      return false;
    }
    const actorId = this.actorId();
    return $gameSubMembers._subMembers.includes(actorId) &&
      !$gameParty._actors.includes(actorId);
  };

  const _Game_Actor_isBattleMember = Game_Actor.prototype.isBattleMember;
  Game_Actor.prototype.isBattleMember = function() {
    if (_Game_Actor_isBattleMember.call(this)) {
      return true;
    } else if (hasSubMember()) {
      return $gameSubMembers.members().includes(this);
    }
    return false;
  };


  //
  // sub members cannot accept input but decide action automatically
  //
  const _Game_BattlerBase_isAutoBattle =
    Game_BattlerBase.prototype.isAutoBattle;
  Game_BattlerBase.prototype.isAutoBattle = function() {
    if (this.isSubMember()) {
      return true;
    }
    return _Game_BattlerBase_isAutoBattle.call(this);
  };

  //
  // sub members doesn't consider HP/MP
  //
  const _Game_BattlerBase_canPaySkillCost =
    Game_BattlerBase.prototype.canPaySkillCost;
  Game_BattlerBase.prototype.canPaySkillCost = function(skill) {
    if (this.isSubMember()) {
      return true;
    }
    return _Game_BattlerBase_canPaySkillCost.call(this, skill);
  };

  const _Game_Battler_useItem = Game_Battler.prototype.useItem;
  Game_Battler.prototype.useItem = function(item) {
    if (this.isSubMember()) {
      return;
    }
    _Game_Battler_useItem.call(this, item);
  };

  //
  // make sub members' action on battle
  //
  const _Game_Party_makeActions = Game_Party.prototype.makeActions;
  Game_Party.prototype.makeActions = function() {
    _Game_Party_makeActions.call(this);
    if (hasSubMember()) {
      $gameSubMembers.makeActions();
    }
  };

  const _BattleManager_updateTpb = BattleManager.updateTpb;
  BattleManager.updateTpb = function() {
    if (hasSubMember()) {
      $gameSubMembers.updateTpb();
    }
    _BattleManager_updateTpb.call(this);
  };

  //
  // sub members aren't affected by counter attack and magic reflection
  //
  const _Game_Temp_initialize = Game_Temp.prototype.initialize;
  Game_Temp.prototype.initialize = function() {
    _Game_Temp_initialize.call(this);
    this.attackToSubMember = false;
  }

  const _BattleManager_invokeCounterAttack = BattleManager.invokeCounterAttack;
  BattleManager.invokeCounterAttack = function(subject, target) {
    $gameTemp.attackToSubMember = subject.isSubMember();
    _BattleManager_invokeCounterAttack.call(this, subject, target);
    $gameTemp.attackToSubMember = false;
  };

  const _BattleManager_invokeMagicReflection = BattleManager.invokeMagicReflection;
  BattleManager.invokeMagicReflection = function(subject, target) {
    $gameTemp.attackToSubMember = subject.isSubMember();
    _BattleManager_invokeMagicReflection.call(this, subject, target);
    $gameTemp.attackToSubMember = false;
  };

  const _Game_Action_apply = Game_Action.prototype.apply;
  Game_Action.prototype.apply = function(target) {
    // sub members supposed to have limitless HP.
    if ($gameTemp.attackToSubMember) {
      return;
    }
    _Game_Action_apply.call(this, target);
  };

})();
