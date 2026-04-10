//=============================================================================
// Plugin for RPG Maker MZ
// SimplePassiveSkillMZ.js
//=============================================================================
// [Update History]
// 2020.Apr.XX Ver1.0.0 First Release
// 2022.Feb.15 Ver1.1.0 Check also note of weapon/armor

/*:
 * @target MZ
 * @plugindesc [1.1.0]Make passive skill that change parameters and set traits.
 * @author Sasuke KANNAZUKI
 *
 * @help
 * This plugin runs under RPG Maker MZ.
 *
 * This pluguin enables to make passive skill by introducing parameter and
 * traits from equipments.
 *
 * [Summary]
 * Write down skill's note as follows, then when an actor learn the skill,
 * affect parameters and traits from specified weapon/armor.
 *
 * <passiveWeapon:2>      # Gain parameters and traits of Weapon that ID is 2
 * <passiveArmor:4>       # Gain parameters and traits of Armor that ID is 4
 *
 * It is preffered to make weapon/armor only for passive skills.
 *
 * [Usage Example]
 * You can make such passive skills as...
 * - To increase parameter(s) (ex.MaxHP+50, Agility+30, and so on...)
 * - To add equip type by adding it as a trait.
 * - To be always regenerates HP by setting it as a trait.
 * - Since the skill checks weapon/armor's note, enable various thing.
 *
 * [Note]
 * The skill cannot be passive that actor is usable temporally by the traits in
 * equipments, states, and so on.
 *
 * [License]
 * this plugin is released under MIT license.
 * http://opensource.org/licenses/mit-license.php
 */

/*:ja
 * @target MZ
 * @plugindesc [Ver1.1.0]能力値や特徴が付けられるパッシブスキルを作成します
 * @author 神無月サスケ
 *
 * @help
 * このプラグインは、RPGツクールMZに対応しています。
 *
 * このプラグインはパッシブスキルを作成可能にします。
 * 武器や防具の設定値や特徴を参照する形でのパッシブスキルです。
 *
 * ■概要
 * スキルのメモ欄に以下のように書くことで、そのIDの武器/防具の
 * 能力値と特徴が、スキルを覚えている間ずっと付加されます。
 *
 * <passiveWeapon:2>      # ID2番の武器の能力値と特徴が付加されます。
 * <passiveArmor:4>       # ID4番の防具の能力値と特徴が付加されます。
 *
 * パッシブスキル専用の武器/防具を作るといいでしょう。
 *
 * ■パッシブスキル作成例
 * - 能力値を上げる(最大HP+50や敏捷性+30など)スキル
 * - 特徴で装備タイプを追加するスキル
 * - 特徴でオートリジェネなどの状態が継続するようにするスキル
 * - 装備のメモ欄も参照するため、他のプラグインとの併用が可能(Ver1.1.0より)
 *
 * ■注意
 * 装備やステートなどの「特徴」で一時的に覚えているスキルは、
 * パッシブスキルにすることはできません。
 *
 * ■ライセンス表記
 * このプラグインは MIT ライセンスで配布されます。
 * ご自由にお使いください。
 * http://opensource.org/licenses/mit-license.php
 */

/*:zh
 * @target MZ
 * @plugindesc [Ver1.1.0] 创建可赋予能力值和特征的被动技能
 * @author Sasuke KANNAZUKI
 *
 * @help
 * 本插件适用于 RPG Maker MZ。
 *
 * 该插件允许通过引用武器或防具的参数和特征来创建“被动技能”。
 *
 * 【概要】
 * 在技能的备注栏中写入以下格式，
 * 当角色学会该技能后，将自动获得指定武器/防具的参数和特征。
 *
 * <passiveWeapon:2>      # 获得ID为2的武器的能力值与特征
 * <passiveArmor:4>       # 获得ID为4的防具的能力值与特征
 *
 * 建议为被动技能专门创建一套用于引用的武器/防具。
 *
 * 【被动技能示例】
 * - 提升能力值的技能（如最大HP+50、敏捷+30等）
 * - 通过特征添加额外装备类型的技能
 * - 拥有自动恢复HP（再生）效果的常驻技能
 * - 由于会读取装备备注栏，也可与其他插件组合使用（Ver1.1.0起支持）
 *
 * 【注意事项】
 * 通过“特征”（装备、状态等）临时获得的技能，
 * 无法作为被动技能生效。
 *
 * 【许可证】
 * 本插件基于 MIT 许可证发布。
 * 您可以自由修改、再分发，或用于任何用途（包括商业或成人游戏）。
 * http://opensource.org/licenses/mit-license.php
 */

(() => {
  const pluginName = 'SimplePassiveSkillMZ';

  //
  // for efficiency, process the note at first.
  //
  var _Scene_Boot_start = Scene_Boot.prototype.start;
  Scene_Boot.prototype.start = function() {
    _Scene_Boot_start.call(this);
    DataManager.processPassiveSkill();
  };

  DataManager.processPassiveSkill = function() {
    let id = 0;
    for (let skill of $dataSkills) {
      if (!skill) {
        continue;
      } else if (id = skill.meta.passiveWeapon) {
        skill.passive = new Game_Item($dataWeapons[+id]);
        addMetaTag(skill.meta, $dataWeapons[+id].meta);
      } else if (id = skill.meta.passiveArmor) {
        skill.passive = new Game_Item($dataArmors[+id]);
        addMetaTag(skill.meta, $dataArmors[+id].meta);
      }
    }
  };

  const addMetaTag = (origin, target) => {
    for (const tag of Object.keys(target)) {
      origin[tag] = target[tag];
    }
  };

  //
  // get the skills that have passive skill
  //
  Game_Actor.prototype.passiveSkills = function() {
    let _passiveSkills = [];
    for (const id of this._skills) {
      const skill = $dataSkills[id];
      if (skill && skill.passive) {
        _passiveSkills.push(skill.passive.object());
      }
    }
    return _passiveSkills;
  };

  //
  // Add the parameters and traits
  //
  const _Game_Actor_paramPlus = Game_Actor.prototype.paramPlus;
  Game_Actor.prototype.paramPlus = function(paramId) {
    let value = _Game_Actor_paramPlus.call(this, paramId);
    for (const passive of this.passiveSkills()) {
      if (passive) {
        value += passive.params[paramId];
      }
    }
    return value;
  };

  const _Game_Actor_traitObjects = Game_Actor.prototype.traitObjects;
  Game_Actor.prototype.traitObjects = function() {
    let objects = _Game_Actor_traitObjects.call(this);
    return objects.concat(this.passiveSkills());
  };
})();
