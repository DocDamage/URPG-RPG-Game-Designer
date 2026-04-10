//=============================================================================
// Plugin for RPG Maker MZ
// AltMenuScreen2MZ.js
//=============================================================================
// Note: This Plugin is MZ version of AltMenuScreen3.js the KADOKAWA MV Plugin.
// [History of AltMenuScreen3]
// 2015.Nov.23 1.0.0 First Release
// 2018.Sep.19 1.0.1 add function to display current mapname.
//   Following bugs are fixed by Triacontan:
//   - The reserved actors, it doesn't display standing picture at first.
//    (リザーブメンバーの立ち絵が初回表示時に正しく表示されない問題の修正)
//   - At it's scrollable, arrow sprite indicates wrong direction.
//   （スクロール可能であることを示す矢印スプライトの向きがおかしい問題の修正）
// [History]
// 2020.Feb.17 0.0.1 First Release on closed community
// 2020.Sep.03 1.0.0 Add Shop and Name Input Scenes to BG image selection.
// 2020.Sep.12 1.1.0 Enable to change BG Image also user defined scene.
//  Import Munokura's modifications(see Advanced Option 2).
// 2020.Sep.12 1.1.1 Add help message
// 2020.Sep.20 1.1.2 Fix Bug that gauge length too long at several scenes.
// 2021.Jan.08 1.1.3 Fix Bug when one uses item/skill, gauge length was long.
// 2021.Apr.28 1.2.0 Add Plugin Commands to change background dynamically
// 2021.Sep.14 1.3.0 Make menu command window's scroll smooth
// 2022.Apr.28 1.3.1 Fix Bug: "cursor for all" sometimes selected wrong range
// 2022.May.11 1.4.0 Add Option: Enables to select MV-Like Parameter Display
// 2023.Nov.16 1.5.0 Fix Bug: It did invoke error when entering battle.
// 2024.Jul.26 1.6.0 It can set standing picture also various note 
//                   Solve the confliction between LookAtMHPMPP.js
// 2024.Aug.02 1.6.1 Fix bug at using item/skill, gauge length is wrong.
// 2024.Aug.07 1.7.0 On shop,it enables to set original bg at buy and sell.

/*:
 * @target MZ
 * @plugindesc [Ver1.7.0]Yet another menu screen layout
 * @author Sasuke KANNAZUKI, Munokura
 *
 * @param allowWindowDisp
 * @text Display Window At Default
 * @desc Whether to display windows when BG is not set
 * @type boolean
 * @on Yes
 * @off No. Transparent
 * @default true
 *
 * @param maxColsMenu
 * @text Actor Columns At Main Menu
 * @desc The number of actors display on main menu. Set 0 to automatically adjust according to the number of party actors.
 * @type number
 * @min 0
 * @default 4
 * 
 * @param commandRows
 * @text Menu Command Window's Row
 * @desc number of visible rows at command window
 * @type number
 * @min 1
 * @default 2
 *
 * @param commandCols
 * @text Menu Command Window's Colomn
 * @desc number of columns at command window
 * @type number
 * @min 1
 * @default 4
 *
 * @param isDisplayStatus
 * @text Display Status?
 * @desc whether to display each actor's status on main menu
 * @type boolean
 * @default true
 *
 * @param doesDisplayNextLevel
 * @parent isDisplayStatus
 * @text Display Exp To Next Lv?
 * @desc Does display Exp To Next Lv beneath Lv?
 * @on Yes
 * @off No
 * @type boolean
 * @default false
 *
 * @param textNextLevel
 * @parent doesDisplayNextLevel
 * @text Term "Exp Next" (abbr.)
 * @desc the abbr. of "Exp Next". %1 is replaced Level(abbr.)
 * @type string
 * @default Next%1
 *
 * @param statusStyle
 * @parent isDisplayStatus
 * @text Status Dislay Style
 * @desc You can also select MV Style.
 * @type select
 * @option Standard
 * @value standard
 * @option MV Style
 * @value mv
 * @default standard
 *
 * @param display map name
 * @text Display Map Name?
 * @desc whether to display map name on main menu
 * @type boolean
 * @default true
 *
 * @param location string
 * @parent display map name
 * @text Location String
 * @desc prefix of map name. It draws by system color.
 * @type string
 * @default Location:
 *
 * @param bgBitmapMenu
 * @text BG Image of Main Menu
 * @desc background bitmap at main menu scene.
 * put at img/pictures.
 * @default 
 * @require 1
 * @dir img/pictures/
 * @type file
 *
 * @param doesNotSetItemScene
 * @parent bgBitmapMenu
 * @text Set Same BG on Item Scene?
 * @desc Whether to Set Same Image as Main Menu
 * @type boolean
 * @on Yes
 * @off No. Set Original BG
 * @default true
 *
 * @param bgBitmapItem
 * @parent doesNotSetItemScene
 * @text BG Image of Item Menu
 * @desc background bitmap file at item scene.
 * put at img/pictures.
 * @default 
 * @require 1
 * @dir img/pictures/
 * @type file
 *
 * @param doesNotSetSkillScene
 * @parent bgBitmapMenu
 * @text Set Same BG on Skill Scene?
 * @desc Whether to Set Same Image as Main Menu
 * @type boolean
 * @on Yes
 * @off No. Set Original BG
 * @default true
 *
 * @param bgBitmapSkill
 * @parent doesNotSetSkillScene
 * @text BG Image of Skill Menu
 * @desc background bitmap file at skill scene.
 * put at img/pictures.
 * @default 
 * @require 1
 * @dir img/pictures/
 * @type file
 * 
 * @param doesNotSetEquipScene
 * @parent bgBitmapMenu
 * @text Set Same BG on Equip Scene?
 * @desc Whether to Set Same Image as Main Menu
 * @type boolean
 * @on Yes
 * @off No. Set Original BG
 * @default true
 *
 * @param bgBitmapEquip
 * @text BG Image of Equip Menu
 * @parent doesNotSetEquipScene
 * @desc background bitmap file at equip scene.
 * put at img/pictures.
 * @default 
 * @require 1
 * @dir img/pictures/
 * @type file
 * 
 * @param doesNotSetStatusScene
 * @parent bgBitmapMenu
 * @text Set Same BG on Status Scene?
 * @desc Whether to Set Same Image as Main Menu
 * @type boolean
 * @on Yes
 * @off No. Set Original BG
 * @default true
 *
 * @param bgBitmapStatus
 * @parent doesNotSetStatusScene
 * @text BG Image of Status Menu
 * @desc background bitmap file at status scene.
 * put at img/pictures.
 * @default 
 * @require 1
 * @dir img/pictures/
 * @type file
 * 
 * @param doesNotSetOptionScene
 * @parent bgBitmapMenu
 * @text Set Same BG on Option Scene?
 * @desc Whether to Set Same Image as Main Menu
 * @type boolean
 * @on Yes
 * @off No. Set Original BG
 * @default true
 *
 * @param bgBitmapOptions
 * @parent doesNotSetOptionScene
 * @text BG Image of Option Menu
 * @desc background bitmap file at option scene.
 * put at img/pictures.
 * @default 
 * @require 1
 * @dir img/pictures/
 * @type file
 * 
 * @param doesNotSetSaveScene
 * @parent bgBitmapMenu
 * @text Set Same BG on Save Scene?
 * @desc Whether to Set Same Image as Main Menu
 * @type boolean
 * @on Yes
 * @off No. Set Original BG
 * @default true
 *
 * @param bgBitmapFile
 * @parent doesNotSetSaveScene
 * @text BG Image of Save Menu
 * @desc background bitmap file at save/load scene.
 * put at img/pictures.
 * @default 
 * @require 1
 * @dir img/pictures/
 * @type file
 * 
 * @param doesNotSetEndScene
 * @parent bgBitmapMenu
 * @text Set Same BG on Game End Scene?
 * @desc Whether to Set Same Image as Main Menu
 * @type boolean
 * @on Yes
 * @off No. Set Original BG
 * @default true
 *
 * @param bgBitmapGameEnd
 * @parent doesNotSetEndScene
 * @text BG Image of End Menu
 * @desc background bitmap file at gameEnd scene.
 * put at img/pictures.
 * @default 
 * @require 1
 * @dir img/pictures/
 * @type file
 *
 * @param doesNotSetShopScene
 * @parent bgBitmapMenu
 * @text Set Same BG on Shop Scene?
 * @desc Whether to Set Same Image as Main Menu
 * @type boolean
 * @on Yes
 * @off No. Set Original BG
 * @default false
 *
 * @param bgBitmapShop
 * @parent doesNotSetShopScene
 * @text BG Image of Shop
 * @desc background bitmap file at shop scene.
 * put at img/pictures.
 * @default 
 * @require 1
 * @dir img/pictures/
 * @type file
 *
 * @param doesNotSetShopBuy
 * @parent doesNotSetShopScene
 * @text Set Same BG on Shop Buy?
 * @desc Whether to Set Same Image as Shop Main?
 * @type boolean
 * @on Yes
 * @off No. Set Original BG
 * @default false
 *
 * @param bgBitmapShopBuy
 * @parent doesNotSetShopBuy
 * @text BG Image of Shop Buy
 * @desc background bitmap file at buying on shop scene.
 * put at img/pictures.
 * @default 
 * @require 1
 * @dir img/pictures/
 * @type file
 *
 * @param doesNotSetShopBuy
 * @parent doesNotSetShopScene
 * @text Set Same BG on Shop Buy?
 * @desc Whether to Set Same Image as Shop Main?
 * @type boolean
 * @on Yes
 * @off No. Set Original BG
 * @default true
 *
 * @param bgBitmapShopBuy
 * @parent doesNotSetShopBuy
 * @text BG Image of Shop Buy
 * @desc background bitmap file at buying on shop scene.
 * put at img/pictures.
 * @default 
 * @require 1
 * @dir img/pictures/
 * @type file
 *
 * @param doesNotSetShopSell
 * @parent doesNotSetShopScene
 * @text Set Same BG on Shop Sell?
 * @desc Whether to Set Same Image as Shop Main?
 * @type boolean
 * @on Yes
 * @off No. Set Original BG
 * @default true
 *
 * @param bgBitmapShopSell
 * @parent doesNotSetShopSell
 * @text BG Image of Shop Sell
 * @desc background bitmap file at selling on shop scene.
 * put at img/pictures.
 * @default 
 * @require 1
 * @dir img/pictures/
 * @type file
 *
 * @param doesNotSetNameScene
 * @parent bgBitmapMenu
 * @text Set Same BG on NameInput Scene?
 * @desc Whether to Set Same Image as Main Menu
 * @type boolean
 * @on Yes
 * @off No. Set Original BG
 * @default true
 *
 * @param bgBitmapNameInput
 * @parent doesNotSetNameScene
 * @text BG Image of Name Input
 * @desc background bitmap file at NameInput scene.
 * put at img/pictures.
 * @default 
 * @require 1
 * @dir img/pictures/
 * @type file
 *
 * @param bgUserDefined
 * @text BG Image of User Defined Scene
 * @desc background file at added scene at plugin.
 * put at img/pictures.
 * @default 
 * @type struct<userDefinedScene>[]
 * 
 * @noteParam stand_picture
 * @noteRequire 1
 * @noteDir img/pictures/
 * @noteType file
 * @noteData actor
 *
 * @command newMainBg
 * @text Change BG for Main Menu
 * @desc Change the background picture at Scene_Menu
 *
 * @arg fileName
 * @text Background at Scene_Menu
 * @desc Change Scene_Menu's background picture
 * @type file
 * @dir img/pictures/
 * @default
 *
 * @command newOtherBg
 * @text Change BG of Other Scenes
 * @desc Change the background picture except Scene_Menu
 *
 * @arg sceneName
 * @text Scene Name
 * @desc Target Scene to Change Background. If the scene not in the list, change text directly.
 * @type select
 * @option Item(Scene_Item)
 * @value Scene_Item
 * @option Skill(Scene_Skill)
 * @value Scene_Skill
 * @option Equip(Scene_Equip)
 * @value Scene_Equip
 * @option Status(Scene_Status)
 * @value Scene_Status
 * @option Option(Scene_Options)
 * @value Scene_Options
 * @option Save(Scene_Save)
 * @value Scene_Save
 * @option Load(Scene_Load)
 * @value Scene_Load
 * @option End(Scene_GameEnd)
 * @value Scene_GameEnd
 * @option Shop(Scene_Shop)
 * @value Scene_Shop
 * @option Name Input(Scene_Name)
 * @value Scene_Name
 * @default Scene_Item
 *
 * @arg doUseMain
 * @text Same background as Main Menu?
 * @desc Do the scene adopt same as main, or set original for the scene?
 * @type boolean
 * @on Yes.
 * @off No. Set original one.
 * @default true
 *
 * @arg fileName
 * @parent doUseMain
 * @text Background at the Scene
 * @desc Change the Scene's background picture
 * @type file
 * @dir img/pictures/
 * @default
 *
 * @command reset
 * @text Reset
 * @desc Discard all settings done by the plugin commands.
 *
 * @help
 * This plugin runs under RPG Maker MZ.
 * 
 * This plugin changes the menu layout.
 *
 * [Summary]
 *  The differences of AltMenuscreen.js are follows:
 *  - It can set the standing graphic at each actor.
 *   - Describe actor's note following style:
 *    <stand_picture:filename>
 *   - If you don't set the picture, it displays actor's face grapic instead.
 *  - It can set the background image for each scene on menu.
 *   - Put background image file at img/pictures
 *   - If you don't set the picture, the scene' background become
 *     either transparent or display default window.
 *     (You can select at option)
 * - You can change the number of visible actors by option.
 * - You can select whether to display parameters or not.
 *  - When you display the parameter, you can select to display TP or not.
 * - You can display the current map name (option).
 *
 * Actor's Note:
 * <stand_picture:filename> set actor's standing picture at menu.
 *   put file at img/pictures.
 *
 * [Advanced Option 1] (Added at Ver1.1.0)
 * This plugin affects all scenes of sub class of Scene_MenuBase.
 * When you import plugin that affects by this function of this plugin,
 * set option 'BG Image of User Defined Scene'.
 *
 * [Advanced Option 2] (Added by Munokura at Ver1.1.0)
 * - Trim actor graphics(either standing or face picture) to fit according to
 *  display width.
 * - The display width of names etc. is automatically adjusted according to
 *  the number of people that can be displayed.
 * - Added a function to automatically adjust the number of rows
 *  when the number of parties changes.
 * - To change the offset of standing picture, write down following format
 *  at actor's note like this:
 *  <actor_offset_x: 100>
 *    In this case, the picture moves right by 100 px.
 *    To move left, set the minus number.
 *  <actor_offset_y: 100>
 *    In this case, the picture moves below by 100 px.
 *    To move above, set the minus number.
 *  Note that the setting is invalid when you don't set standing picture.
 *
 * [Advanced Option 3] (Added at Ver1.2.0)
 * To change the background during the game, you can call plugin command.
 *
 * [Advanced Option 4] (since Ver1.6.0)
 * It can set <stand_picture:filename>, <actor_offset_x: 100> and
 * <actor_offset_y: 100> not only actor but also various notes.
 * (the priority is state, armor, weapon, class, actor)
 * It assumes to the use of the actor equips specified equipment,
 * and the actor affects specified state.
 *
 * [preferred size of actor's picture]
 * width: 174px(maxColsMenu=4), 240px(maxColsMenu=3)
 * height: 408px(commandRows=2), 444px(commandRows=1)
 *
 * [License]
 * this plugin is released under MIT license.
 * http://opensource.org/licenses/mit-license.php
 */

/*:ja
 * @target MZ
 * @plugindesc [Ver1.7.0]レイアウトの異なるメニュー画面
 * @author 神無月サスケ, ムノクラ
 *
 * @param allowWindowDisp
 * @text 背景未設定の時ウィンドウ表示？
 * @desc 
 * @type boolean
 * @on する
 * @off しない。背景透明に
 * @default true
 *
 * @param maxColsMenu
 * @text アクター表示数
 * @desc メインメニュー画面のアクター表示ウィンドウの1画面の登録数。0にすると、パーティ人数で自動調整
 * @type number
 * @min 0
 * @default 4
 * 
 * @param commandRows
 * @text コマンドウィンドウ行数
 * @desc メインメニューのコマンドウィンドウ行数(既定値:2)
 * @type number
 * @min 1
 * @default 2
 *
 * @param commandCols
 * @text コマンドウィンドウ列数
 * @desc コマンドウィンドウ1行に表示する要素数(既定値:4)
 * @type number
 * @min 1
 * @default 4
 *
 * @param isDisplayStatus
 * @text ステータス表示？
 * @desc メインメニューでアクターのステータスを表示する？
 * @on する
 * @off しない
 * @type boolean
 * @default true
 *
 * @param doesDisplayNextLevel
 * @parent isDisplayStatus
 * @text 必要EXP表示？
 * @desc 次のレベルまでの必要経験値を表示する？
 * @on する
 * @off しない
 * @type boolean
 * @default false
 *
 * @param textNextLevel
 * @parent doesDisplayNextLevel
 * @text 「必要EXP」用語
 * @desc 「次のレベルまでの必要経験値」を短縮した用語。%1は、レベル(略)に置き換えられます。
 * @type string
 * @default Next%1
 *
 * @param statusStyle
 * @parent isDisplayStatus
 * @text ステータス表示スタイル
 * @desc MVタイプのステータス表示も選択可能です
 * @type select
 * @option 標準
 * @value standard
 * @option MVスタイル
 * @value mv
 * @default standard
 *
 * @param display map name
 * @text マップ名表示？
 * @desc メインメニュー画面左下にマップ名を表示する？
 * @on する
 * @off しない
 * @type boolean
 * @default true
 *
 * @param location string
 * @parent display map name
 * @text 「現在地：」を意味するテキスト
 * @desc マップ名表示の際にシステムカラーで表示される文字列
 * @type string
 * @default 現在地:
 *
 * @param bgBitmapMenu
 * @text メインメニュー背景画
 * @desc 背景用の一枚絵のファイル名。
 * img/pictures に置いて下さい
 * @default 
 * @require 1
 * @dir img/pictures/
 * @type file
 *
 * @param doesNotSetItemScene
 * @parent bgBitmapMenu
 * @text アイテム画面は同じ背景画？
 * @desc メインメニューと同じ背景画？
 * @type boolean
 * @on 同じ背景画
 * @off 別に設定
 * @default true
 *
 * @param bgBitmapItem
 * @parent doesNotSetItemScene
 * @text アイテムメニュー背景画
 * @desc 背景用の一枚絵のファイル名。
 * img/pictures に置いて下さい
 * @default 
 * @require 1
 * @dir img/pictures/
 * @type file
 *
 * @param doesNotSetSkillScene
 * @parent bgBitmapMenu
 * @text スキル画面は同じ背景画？
 * @desc メインメニューと同じ背景画？
 * @type boolean
 * @on 同じ背景画
 * @off 別に設定
 * @default true
 *
 * @param bgBitmapSkill
 * @parent doesNotSetSkillScene
 * @text スキル画面背景画
 * @desc 背景用の一枚絵のファイル名。
 * img/pictures に置いて下さい
 * @default 
 * @require 1
 * @dir img/pictures/
 * @type file
 * 
 * @param doesNotSetEquipScene
 * @parent bgBitmapMenu
 * @text 装備画面は同じ背景画？
 * @desc メインメニューと同じ背景画？
 * @type boolean
 * @on 同じ背景画
 * @off 別に設定
 * @default true
 *
 * @param bgBitmapEquip
 * @parent doesNotSetEquipScene
 * @text 装備画面背景画
 * @desc 背景用の一枚絵のファイル名。
 * img/pictures に置いて下さい
 * @default 
 * @require 1
 * @dir img/pictures/
 * @type file
 * 
 * @param doesNotSetStatusScene
 * @parent bgBitmapMenu
 * @text ステータス画面は同じ背景画？
 * @desc メインメニューと同じ背景画？
 * @type boolean
 * @on 同じ背景画
 * @off 別に設定
 * @default true
 *
 * @param bgBitmapStatus
 * @parent doesNotSetStatusScene
 * @text ステータス画面背景画
 * @desc 背景用の一枚絵のファイル名。
 * img/pictures に置いて下さい
 * @default 
 * @require 1
 * @dir img/pictures/
 * @type file
 * 
 * @param doesNotSetOptionScene
 * @parent bgBitmapMenu
 * @text オプション画面は同じ背景画？
 * @desc メインメニューと同じ背景画？
 * @type boolean
 * @on 同じ背景画
 * @off 別に設定
 * @default true
 *
 * @param bgBitmapOptions
 * @parent doesNotSetOptionScene
 * @text オプション画面背景画
 * @desc 背景用の一枚絵のファイル名。
 * img/pictures に置いて下さい
 * @default 
 * @require 1
 * @dir img/pictures/
 * @type file
 * 
 * @param doesNotSetSaveScene
 * @parent bgBitmapMenu
 * @text セーブ画面は同じ背景画？
 * @desc メインメニューと同じ背景画？
 * @type boolean
 * @on 同じ背景画
 * @off 別に設定
 * @default true
 *
 * @param bgBitmapFile
 * @parent doesNotSetSaveScene
 * @text セーブ画面背景画
 * @desc 背景用の一枚絵のファイル名。
 * img/pictures に置いて下さい
 * @default 
 * @require 1
 * @dir img/pictures/
 * @type file
 * 
 * @param doesNotSetEndScene
 * @parent bgBitmapMenu
 * @text 終了画面は同じ背景画？
 * @desc メインメニューと同じ背景画？
 * @type boolean
 * @on 同じ背景画
 * @off 別に設定
 * @default true
 *
 * @param bgBitmapGameEnd
 * @parent doesNotSetEndScene
 * @text 終了画面背景画
 * @desc 背景用の一枚絵のファイル名。
 * img/pictures に置いて下さい
 * @default 
 * @require 1
 * @dir img/pictures/
 * @type file
 *
 * @param doesNotSetShopScene
 * @parent bgBitmapMenu
 * @text ショップ画面は同じ背景画？
 * @desc メインメニューと同じ背景画？
 * @type boolean
 * @on 同じ背景画
 * @off 別に設定
 * @default false
 *
 * @param bgBitmapShop
 * @parent doesNotSetShopScene
 * @text ショップ画面背景画
 * @desc 背景用の一枚絵のファイル名。
 * img/pictures に置いて下さい
 * @default 
 * @require 1
 * @dir img/pictures/
 * @type file
 *
 * @param doesNotSetShopBuy
 * @parent doesNotSetShopScene
 * @text 買う時は同じ背景画？
 * @desc ショップ画面と同じ背景画？
 * @type boolean
 * @on 同じ背景画
 * @off 別に設定
 * @default true
 *
 * @param bgBitmapShopBuy
 * @parent doesNotSetShopBuy
 * @text 購入画面背景画
 * @desc 背景用の一枚絵のファイル名。
 * img/pictures に置いて下さい
 * @default 
 * @require 1
 * @dir img/pictures/
 * @type file
 *
 * @param doesNotSetShopSell
 * @parent doesNotSetShopScene
 * @text 売る時は同じ背景画？
 * @desc ショップ画面と同じ背景画？
 * @type boolean
 * @on 同じ背景画
 * @off 別に設定
 * @default true
 *
 * @param bgBitmapShopSell
 * @parent doesNotSetShopSell
 * @text 売却画面背景画
 * @desc 背景用の一枚絵のファイル名。
 * img/pictures に置いて下さい
 * @default 
 * @require 1
 * @dir img/pictures/
 * @type file
 *
 * @param doesNotSetNameScene
 * @parent bgBitmapMenu
 * @text 名前入力画面は同じ背景画？
 * @desc メインメニューと同じ背景画？
 * @type boolean
 * @on 同じ背景画
 * @off 別に設定
 * @default true
 *
 * @param bgBitmapNameInput
 * @parent doesNotSetNameScene
 * @text 名前入力画面背景画
 * @desc 背景用の一枚絵のファイル名。
 * img/pictures に置いて下さい
 * @default 
 * @require 1
 * @dir img/pictures/
 * @type file
 *
 * @noteParam stand_picture
 * @noteRequire 1
 * @noteDir img/pictures/
 * @noteType file
 * @noteData actor
 *
 * @param bgUserDefined
 * @text プラグイン定義シーン背景画
 * @desc プラグインで追加されたシーンの背景画。
 * img/pictures に置いて下さい
 * @default 
 * @type struct<userDefinedSceneJpn>[]
 * 
 * @command newMainBg
 * @text メインメニュー背景画変更
 * @desc メインメニューおよびそれを参照するシーンの画像を変更
 *
 * @arg fileName
 * @text 画像ファイル
 * @desc メニュー画面(Scene_Menu)の新しい背景画像
 * @type file
 * @dir img/pictures/
 * @default
 *
 * @command newOtherBg
 * @text その他の背景画変更
 * @desc Scene_Menu以外のメニューシーンの画像設定を行います。
 *
 * @arg sceneName
 * @text シーン名
 * @desc 画像を変更するシーン名。一覧にない場合、直接テキストで入力して下さい。
 * @type select
 * @option アイテム(Scene_Item)
 * @value Scene_Item
 * @option スキル(Scene_Skill)
 * @value Scene_Skill
 * @option 装備(Scene_Equip)
 * @value Scene_Equip
 * @option ステータス(Scene_Status)
 * @value Scene_Status
 * @option オプション(Scene_Options)
 * @value Scene_Options
 * @option セーブ(Scene_Save)
 * @value Scene_Save
 * @option ロード(Scene_Load)
 * @value Scene_Load
 * @option 終了確認(Scene_GameEnd)
 * @value Scene_GameEnd
 * @option お店(Scene_Shop)
 * @value Scene_Shop
 * @option 名前入力(Scene_Name)
 * @value Scene_Name
 * @default Scene_Item
 *
 * @arg doUseMain
 * @text メインメニューと同じ？
 * @desc メインメニューと同じにするか、別の画像にするか
 * @type boolean
 * @on 同じ画像
 * @off このシーン特有の画像
 * @default true
 *
 * @arg fileName
 * @parent doUseMain
 * @text 画像ファイル名
 * @desc 変更する場合の画像ファイル名。「同じ画像」を選んだ場合は無視
 * @type file
 * @dir img/pictures/
 * @default
 *
 * @command reset
 * @text リセット
 * @desc プラグインコマンドでの変更を破棄し、最初の画像に戻します。
 *
 * @help 
 * このプラグインは、RPGツクールMZに対応しています。
 *
 * このプラグインは、メニューのレイアウトを変更します。
 *
 * ■概要
 * AltMenuScreen.js との違いは、以下の通りです。
 * ・各アクターに立ち絵を表示可能
 *   アクターのメモ欄に次のような書式で書いてください。
 *   <stand_picture:ファイル名>
 *   ファイル名が、そのアクターの立ち絵になります。
 *   ファイルは img/pictures に置いてください。
 *   立ち絵を表示しない場合は、アクターの顔グラフィックが表示されます。
 * ・各シーン一括で、または特定のシーンにのみ、背景画を表示できます。
 *   背景画を使わないシーンでは、ウィンドウを表示するか、透明にするかを
 *   オプションで選択可能です。
 * ・１画面に表示可能な人数を設定できます。
 *   デフォルトでは４人ですが、３人にしたり、ふたりにしたり、
 *   画面サイズを変更している場合、５人以上も有効です。
 * ・オプションでマップ上の現在地も表示可能です。
 *
 * ■拡張機能１(Ver1.1.0 追加機能)
 * このプラグインは、Scene_MenuBaseのサブクラスのシーン全てに影響します。
 * このため、他のプラグインが影響を受ける場合、オプションの「プラグイン
 * 定義シーン背景画」にて設定を行ってください。
 *
 * ■拡張機能２(Ver1.1.0 ムノクラ様による追加機能)
 * 1.立ち絵や顔画像を表示されている幅に合わせてトリミング表示します。
 * 2.表示可能な人数に合わせて、名前等の表示の幅を自動調整します。
 * 3.パーティ人数の変化で列数を自動調整する機能を追加。
 * 4.アクターのメモタグ<actor_offset_x> <actor_offset_y> で
 * 立ち絵の表示位置指定機能を追加
 * <stand_picture:ファイル名>で指定したアクター表示にのみ反映されます。
 * 通常の顔画像には反映されません。
 * 
 * 立ち絵を横にずらしたい場合
 * <actor_offset_x: 100>
 * ※100の部分をずらす量に指定してください。マイナスだと左に移動します。
 *
 * 立ち絵を縦にずらしたい場合
 * <actor_offset_y: 100>
 * ※100の部分をずらす量に指定してください。マイナスだと上に移動します。
 * 
 * ■拡張機能３(Ver1.2.0 追加機能)
 * プラグインコマンドを呼び出すことで、各メニューシーンの画像が
 * ゲームの途中でも変更可能です。
 *
 * ■拡張機能４(Ver1.6.0 追加機能)
 * メモの<stand_picture:filename>, <actor_offset_x: 100>,
 * <actor_offset_y: 100> は、アクターだけではなく、他のメモでも設定できます。
 * 優先順位は、ステート＞防具＞武器＞職業＞アクター になります。
 * 特定の装備やステートの時だけ立ち絵の変更が可能になります。
 *
 * ■望ましいアクター立ち絵のサイズ：
 * 幅：3列:240px, 4列：174px
 * 高さ： コマンドウィンドウ 1行:444px 2行:408px
 *
 * ■ライセンス表記
 * このプラグインは、RPGツクールMV用準公式プラグインAltMenuScreen3.jsの
 * MZ版です(2ではないことに注意)。
 *
 * このプラグインは MIT ライセンスで配布されます。
 * ご自由にお使いください。
 * http://opensource.org/licenses/mit-license.php
 */

/*~struct~userDefinedScene:
 *
 * @param NewSceneName
 * @text Scene Name
 * @desc Class Name on the Plugin. The class must be the sub class of Scene_MenuBase.
 * @type String
 * @default Scene_Xxxx
 * 
 * @param BgName
 * @text Backgroung Image Name
 * @desc 
 * @default
 * @require 1
 * @dir img/pictures/
 * @type file
 */

/*~struct~userDefinedSceneJpn:
 *
 * @param NewSceneName
 * @text シーン名
 * @desc プラグイン上でのクラス名。Scene_MenuBaseのサブクラスのみ有効です。
 * @type String
 * @default Scene_Xxxx
 * 
 * @param BgName
 * @text 背景画像名
 * @desc 
 * @default
 * @require 1
 * @dir img/pictures/
 * @type file
 */

/*:zh
 * @target MZ
 * @plugindesc [Ver1.7.0] 布局不同的菜单画面。
 * @author Sasuke KANNAZUKI, Munokura
 *
 * @param allowWindowDisp
 * @text 背景未设置时显示窗口？
 * @desc 
 * @type boolean
 * @on 是
 * @off 否。背景透明
 * @default true
 *
 * @param maxColsMenu
 * @text 角色显示数
 * @desc 主菜单画面中，角色显示窗口每页显示的注册数量。设为 0 时，将根据队伍人数自动调整。
 * @type number
 * @min 0
 * @default 4
 * 
 * @param commandRows
 * @text 指令窗口行数
 * @desc 主菜单的指令窗口行数（默认：2）
 * @type number
 * @min 1
 * @default 2
 *
 * @param commandCols
 * @text 指令窗口列数
 * @desc 指令窗口每行显示的项目数（默认：4）
 * @type number
 * @min 1
 * @default 4
 *
 * @param isDisplayStatus
 * @text 显示状态？
 * @desc 是否在主菜单显示角色的状态？
 * @on 显示
 * @off 不显示
 * @type boolean
 * @default true
 *
 * @param doesDisplayNextLevel
 * @parent isDisplayStatus
 * @text 显示所需EXP？
 * @desc 是否显示到下一级所需的经验值？
 * @on 显示
 * @off 不显示
 * @type boolean
 * @default false
 *
 * @param textNextLevel
 * @parent doesDisplayNextLevel
 * @text “所需EXP”用语
 * @desc “到下一级所需经验值”的缩写用语。%1 会被替换为等级（缩写）。
 * @type string
 * @default Next%1
 *
 * @param statusStyle
 * @parent isDisplayStatus
 * @text 状态显示样式
 * @desc 也可选择 MV 类型的状态显示
 * @type select
 * @option 标准
 * @value standard
 * @option MV样式
 * @value mv
 * @default standard
 *
 * @param display map name
 * @text 显示地图名？
 * @desc 是否在主菜单画面左下显示地图名？
 * @on 显示
 * @off 不显示
 * @type boolean
 * @default true
 *
 * @param location string
 * @parent display map name
 * @text 表示“当前位置：”的文本
 * @desc 显示地图名时以系统颜色绘制的字符串
 * @type string
 * @default 当前位置:
 *
 * @param bgBitmapMenu
 * @text 主菜单背景图
 * @desc 作为背景的一张图片文件名。请放在 img/pictures 中
 * @default 
 * @require 1
 * @dir img/pictures/
 * @type file
 *
 * @param doesNotSetItemScene
 * @parent bgBitmapMenu
 * @text 物品画面使用相同背景？
 * @desc 是否与主菜单使用相同背景？
 * @type boolean
 * @on 使用相同背景
 * @off 单独设置
 * @default true
 *
 * @param bgBitmapItem
 * @parent doesNotSetItemScene
 * @text 物品菜单背景图
 * @desc 作为背景的一张图片文件名。请放在 img/pictures 中
 * @default 
 * @require 1
 * @dir img/pictures/
 * @type file
 *
 * @param doesNotSetSkillScene
 * @parent bgBitmapMenu
 * @text 技能画面使用相同背景？
 * @desc 是否与主菜单使用相同背景？
 * @type boolean
 * @on 使用相同背景
 * @off 单独设置
 * @default true
 *
 * @param bgBitmapSkill
 * @parent doesNotSetSkillScene
 * @text 技能画面背景图
 * @desc 作为背景的一张图片文件名。请放在 img/pictures 中
 * @default 
 * @require 1
 * @dir img/pictures/
 * @type file
 * 
 * @param doesNotSetEquipScene
 * @parent bgBitmapMenu
 * @text 装备画面使用相同背景？
 * @desc 是否与主菜单使用相同背景？
 * @type boolean
 * @on 使用相同背景
 * @off 单独设置
 * @default true
 *
 * @param bgBitmapEquip
 * @parent doesNotSetEquipScene
 * @text 装备画面背景图
 * @desc 作为背景的一张图片文件名。请放在 img/pictures 中
 * @default 
 * @require 1
 * @dir img/pictures/
 * @type file
 * 
 * @param doesNotSetStatusScene
 * @parent bgBitmapMenu
 * @text 状态画面使用相同背景？
 * @desc 是否与主菜单使用相同背景？
 * @type boolean
 * @on 使用相同背景
 * @off 单独设置
 * @default true
 *
 * @param bgBitmapStatus
 * @parent doesNotSetStatusScene
 * @text 状态画面背景图
 * @desc 作为背景的一张图片文件名。请放在 img/pictures 中
 * @default 
 * @require 1
 * @dir img/pictures/
 * @type file
 * 
 * @param doesNotSetOptionScene
 * @parent bgBitmapMenu
 * @text 选项画面使用相同背景？
 * @desc 是否与主菜单使用相同背景？
 * @type boolean
 * @on 使用相同背景
 * @off 单独设置
 * @default true
 *
 * @param bgBitmapOptions
 * @parent doesNotSetOptionScene
 * @text 选项画面背景图
 * @desc 作为背景的一张图片文件名。请放在 img/pictures 中
 * @default 
 * @require 1
 * @dir img/pictures/
 * @type file
 * 
 * @param doesNotSetSaveScene
 * @parent bgBitmapMenu
 * @text 存档画面使用相同背景？
 * @desc 是否与主菜单使用相同背景？
 * @type boolean
 * @on 使用相同背景
 * @off 单独设置
 * @default true
 *
 * @param bgBitmapFile
 * @parent doesNotSetSaveScene
 * @text 存档画面背景图
 * @desc 作为背景的一张图片文件名。请放在 img/pictures 中
 * @default 
 * @require 1
 * @dir img/pictures/
 * @type file
 * 
 * @param doesNotSetEndScene
 * @parent bgBitmapMenu
 * @text 结束画面使用相同背景？
 * @desc 是否与主菜单使用相同背景？
 * @type boolean
 * @on 使用相同背景
 * @off 单独设置
 * @default true
 *
 * @param bgBitmapGameEnd
 * @parent doesNotSetEndScene
 * @text 结束画面背景图
 * @desc 作为背景的一张图片文件名。请放在 img/pictures 中
 * @default 
 * @require 1
 * @dir img/pictures/
 * @type file
 *
 * @param doesNotSetShopScene
 * @parent bgBitmapMenu
 * @text 商店画面使用相同背景？
 * @desc 是否与主菜单使用相同背景？
 * @type boolean
 * @on 使用相同背景
 * @off 单独设置
 * @default false
 *
 * @param bgBitmapShop
 * @parent doesNotSetShopScene
 * @text 商店画面背景图
 * @desc 作为背景的一张图片文件名。请放在 img/pictures 中
 * @default 
 * @require 1
 * @dir img/pictures/
 * @type file
 *
 * @param doesNotSetShopBuy
 * @parent doesNotSetShopScene
 * @text 购买时使用相同背景？
 * @desc 是否与商店主画面使用相同背景？
 * @type boolean
 * @on 使用相同背景
 * @off 单独设置
 * @default true
 *
 * @param bgBitmapShopBuy
 * @parent doesNotSetShopBuy
 * @text 购买画面背景图
 * @desc 作为背景的一张图片文件名。请放在 img/pictures 中
 * @default 
 * @require 1
 * @dir img/pictures/
 * @type file
 *
 * @param doesNotSetShopSell
 * @parent doesNotSetShopScene
 * @text 出售时使用相同背景？
 * @desc 是否与商店主画面使用相同背景？
 * @type boolean
 * @on 使用相同背景
 * @off 单独设置
 * @default true
 *
 * @param bgBitmapShopSell
 * @parent doesNotSetShopSell
 * @text 出售画面背景图
 * @desc 作为背景的一张图片文件名。请放在 img/pictures 中
 * @default 
 * @require 1
 * @dir img/pictures/
 * @type file
 *
 * @param doesNotSetNameScene
 * @parent bgBitmapMenu
 * @text 名字输入画面使用相同背景？
 * @desc 是否与主菜单使用相同背景？
 * @type boolean
 * @on 使用相同背景
 * @off 单独设置
 * @default true
 *
 * @param bgBitmapNameInput
 * @parent doesNotSetNameScene
 * @text 名字输入画面背景图
 * @desc 作为背景的一张图片文件名。请放在 img/pictures 中
 * @default 
 * @require 1
 * @dir img/pictures/
 * @type file
 *
 * @noteParam stand_picture
 * @noteRequire 1
 * @noteDir img/pictures/
 * @noteType file
 * @noteData actor
 *
 * @param bgUserDefined
 * @text 插件定义场景背景图
 * @desc 为插件新增的场景设置背景图。请放在 img/pictures 中
 * @default 
 * @type struct<userDefinedSceneJpn>[]
 * 
 * @command newMainBg
 * @text 更改主菜单背景图
 * @desc 更改主菜单及引用其背景的场景的图片
 *
 * @arg fileName
 * @text 图片文件
 * @desc 菜单画面（Scene_Menu）的新背景图片
 * @type file
 * @dir img/pictures/
 * @default
 *
 * @command newOtherBg
 * @text 更改其他背景图
 * @desc 设置 Scene_Menu 之外的菜单场景的图片。
 *
 * @arg sceneName
 * @text 场景名
 * @desc 要更改图片的场景名。不在列表中时，请直接输入文本。
 * @type select
 * @option 物品(Scene_Item)
 * @value Scene_Item
 * @option 技能(Scene_Skill)
 * @value Scene_Skill
 * @option 装备(Scene_Equip)
 * @value Scene_Equip
 * @option 状态(Scene_Status)
 * @value Scene_Status
 * @option 选项(Scene_Options)
 * @value Scene_Options
 * @option 存档(Scene_Save)
 * @value Scene_Save
 * @option 读取(Scene_Load)
 * @value Scene_Load
 * @option 结束确认(Scene_GameEnd)
 * @value Scene_GameEnd
 * @option 商店(Scene_Shop)
 * @value Scene_Shop
 * @option 名字输入(Scene_Name)
 * @value Scene_Name
 * @default Scene_Item
 *
 * @arg doUseMain
 * @text 与主菜单相同？
 * @desc 是否与主菜单相同，或为该场景单独设置图片
 * @type boolean
 * @on 相同图片
 * @off 该场景专用图片
 * @default true
 *
 * @arg fileName
 * @parent doUseMain
 * @text 图片文件名
 * @desc 当选择更改时的图片文件名。若选择“相同图片”则忽略
 * @type file
 * @dir img/pictures/
 * @default
 *
 * @command reset
 * @text 重置
 * @desc 丢弃通过插件命令进行的更改，恢复到初始图片。
 *
 * @help 
 * 本插件适用于 RPG Maker MZ。
 *
 * 本插件用于更改菜单的布局。
 */

/*~struct~userDefinedSceneZh:
 *
 * @param NewSceneName
 * @text 场景名称
 * @desc 插件中的类名。该类必须是 Scene_MenuBase 的子类。
 * @type String
 * @default Scene_Xxxx
 * 
 * @param BgName
 * @text 背景图像名称
 * @desc 
 * @default
 * @require 1
 * @dir img/pictures/
 * @type file
 */

(() => {
  const pluginName = 'AltMenuScreen2MZ';
  //
  // process parameters
  //
  const parameters = PluginManager.parameters(pluginName);
  const allowWindowDisp = eval(parameters['allowWindowDisp'] || 'true');

  const maxColsMenuWnd = Number(parameters['maxColsMenu'] || 4);
  const rowsCommandWnd = Number(parameters['commandRows'] || 2);
  const colsCommandWnd = Number(parameters['commandCols'] || 4);

  const isDisplayStatus = eval(parameters['isDisplayStatus'] || 'true');
  const doesDisplayNextLevel = !!eval(parameters['doesDisplayNextLevel']);
  const textNextLevel = parameters['textNextLevel'] || 'Next%1';
  const isMvStyleStatus = parameters['statusStyle'] === 'mv';


  const isDisplayMapName = eval(parameters['display map name'] || 'true');
  const locationString = parameters['location string'] || 'Location:';

  const bgBitmapMenu = parameters['bgBitmapMenu'] || '';

  let bgBitmap = {};
  bgBitmap['Scene_Item'] = parameters['bgBitmapItem'] || '';
  bgBitmap['Scene_Skill'] = parameters['bgBitmapSkill'] || '';
  bgBitmap['Scene_Equip'] = parameters['bgBitmapEquip'] || '';
  bgBitmap['Scene_Status'] = parameters['bgBitmapStatus'] || '';
  bgBitmap['Scene_Options'] = parameters['bgBitmapOptions'] || '';
  bgBitmap['Scene_Save'] = parameters['bgBitmapFile'] || '';
  bgBitmap['Scene_Load'] = bgBitmap['Scene_Save'];
  bgBitmap['Scene_GameEnd'] = parameters['bgBitmapGameEnd'] || '';
  bgBitmap['Scene_Shop'] = parameters['bgBitmapShop'] || '';
  bgBitmap['Scene_Name'] = parameters['bgBitmapNameInput'] || '';

  let setBg = {};
  setBg['Scene_Item'] = !eval(parameters['doesNotSetItemScene'] || 'true');
  setBg['Scene_Skill'] = !eval(parameters['doesNotSetSkillScene'] || 'true');
  setBg['Scene_Equip'] = !eval(parameters['doesNotSetEquipScene'] || 'true');
  setBg['Scene_Status'] = !eval(parameters['doesNotSetStatusScene'] || 'true');
  setBg['Scene_Options'] = !eval(parameters['doesNotSetOptionScene'] ||'true');
  setBg['Scene_Save'] = !eval(parameters['doesNotSetSaveScene'] || 'true');
  setBg['Scene_Load'] = setBg['Scene_Save'];
  setBg['Scene_GameEnd'] = !eval(parameters['doesNotSetEndScene'] || 'true');
  setBg['Scene_Shop'] = !eval(parameters['doesNotSetShopScene'] || 'false');
  setBg['Scene_Name'] = !eval(parameters['doesNotSetNameScene'] || 'true');

  if (parameters['bgUserDefined']) {
    for (const scene of eval(parameters['bgUserDefined'])) {
      const s = JsonEx.parse(scene);
      setBg[s.NewSceneName] = true;
      bgBitmap[s.NewSceneName] = s.BgName;
    }
  }

  const doesSetBgShopBuy = !eval(parameters["doesNotSetShopBuy"] || 'true');
  const bgNameShopBuy = parameters['bgBitmapShopBuy'] || '';
  const doesSetBgShopSell = !eval(parameters['doesNotSetShopSell'] || 'true');
  const bgNameShopSell = parameters['bgBitmapShopSell'] || '';

  //
  // initialize information that set at plugin command(Ver1.2.0 or later)
  //
  const _Game_System_initialize = Game_System.prototype.initialize;
  Game_System.prototype.initialize = function() {
    _Game_System_initialize.call(this);
    this._initAltMenuInfo();
  };

  const _Game_System_onAfterLoad = Game_System.prototype.onAfterLoad;
  Game_System.prototype.onAfterLoad = function() {
    _Game_System_onAfterLoad.call(this);
    if (this._bgBitmap == null) {
      this._initAltMenuInfo();
    }
  };

  Game_System.prototype._initAltMenuInfo = function() {
    this._bgBitmapMenu = null;
    // key:Scene Name, value:Whether to set original background.
    this._setBg = {};
    // key:Scene Name, value:background image name
    this._bgBitmap = {};
  };

  //
  // proess plugin commands
  //
  PluginManager.registerCommand(pluginName, 'newMainBg', args => {
    $gameSystem._bgBitmapMenu = args.fileName;
  });

  PluginManager.registerCommand(pluginName, 'newOtherBg', args => {
   $gameSystem._setBg[args.sceneName] = !eval(args.doUseMain);
   $gameSystem._bgBitmap[args.sceneName] = args.fileName;
  });

  PluginManager.registerCommand(pluginName, 'reset', args => {
    $gameSystem._initAltMenuInfo();
  });

  //
  // set window positions (based on AltMenuScreen.js MZ ver.)
  //
  Scene_MenuBase.prototype.commandWindowHeight = function() {
    return this.calcWindowHeight(rowsCommandWnd, true);
  };

  Scene_MenuBase.prototype.goldWindowHeight = function() {
    return this.calcWindowHeight(1, true);
  };

  Scene_Menu.prototype.commandWindowRect = function() {
    const ww = Graphics.boxWidth;
    const wh = this.commandWindowHeight();
    const wx = 0;
    const wy = this.mainAreaTop();
    return new Rectangle(wx, wy, ww, wh);
  };

  Scene_Menu.prototype.statusWindowRect = function() {
    const h1 = this.commandWindowHeight();
    const h2 = this.goldWindowHeight();
    const ww = Graphics.boxWidth;
    const wh = this.mainAreaHeight() - h1 - h2;
    const wx = 0;
    const wy = this.mainAreaTop() + this.commandWindowHeight();
    return new Rectangle(wx, wy, ww, wh);
  };

  Scene_ItemBase.prototype.actorWindowRect = function() {
    const rect = Scene_Menu.prototype.statusWindowRect();
    rect.y = this.mainAreaBottom() - rect.height;
    return rect;
  };

  Window_MenuCommand.prototype.maxCols = function () {
    return colsCommandWnd;
  }

  Window_MenuCommand.prototype.numVisibleRows = function() {
    return rowsCommandWnd;
  };

  Window_MenuStatus.prototype.numVisibleRows = function() {
    return 1;
  };

  //
  // Ver1.1.0 Munokura's modification: adjust max cols.
  //
  const realMaxColsMenuWnd = () => maxColsMenuWnd ? maxColsMenuWnd :
    $gameParty.members().length;

  Window_MenuStatus.prototype.maxCols = function() {
    return realMaxColsMenuWnd();
  };

  // delete at Ver1.6.1
  // const _Sprite_Gauge_bitmapWidth = Sprite_Gauge.prototype.bitmapWidth;
  // Sprite_Gauge.prototype.bitmapWidth = function () {
  //   return this.isSceneNeedsAdjustGauge() ? 148 * 4 / realMaxColsMenuWnd() :
  //     _Sprite_Gauge_bitmapWidth.call(this);
  // };

  const _Sprite_Name_bitmapWidth = Sprite_Name.prototype.bitmapWidth;
  Sprite_Name.prototype.bitmapWidth = function () {
    return this.isSceneNeedsAdjustGauge() ? 168 * 4 / realMaxColsMenuWnd() :
      _Sprite_Name_bitmapWidth.call(this);
  };

  //
  // inspect whether the gauge needs adjust size or not.
  // 
  
  _Sprite_Gauge_initialize = Sprite_Gauge.prototype.initialize;
  Sprite_Gauge.prototype.initialize = function() {
    if (arguments[0]) {
      this.defaultGauge = true;
    }
    _Sprite_Gauge_initialize.call(this);
  };

  const _Window_SkillStatus_createInner =
   Window_SkillStatus.prototype.createInnerSprite;
  Window_SkillStatus.prototype.createInnerSprite = function(key, spriteClass) {
    const dict = this._additionalSprites;
    if (!dict[key] && spriteClass === Sprite_Gauge) {
      const sprite = new spriteClass(true);
      dict[key] = sprite;
      this.addInnerChild(sprite);
      return sprite;
    }
    return _Window_SkillStatus_createInner.call(this, key, spriteClass);
  };
  
  Sprite_Gauge.prototype.isSceneNeedsAdjustGauge = 
  Sprite_Name.prototype.isSceneNeedsAdjustGauge = function() {
    const scene = SceneManager._scene.constructor;
    return scene !== Scene_Battle && !this.defaultGauge;
  };

  //
  // adjust cursor for all
  //

  // !!!overwrite!!!
  Window_Selectable.prototype.refreshCursorForAll = function() {
    const maxItems = Math.min(this.maxCols(), this.maxItems());
    if (maxItems > 0) {
      const rect = this.itemRect(0);
      rect.enlarge(this.itemRect(maxItems - 1));
      this.setCursorRect(rect.x, rect.y, rect.width, rect.height);
    } else {
      this.setCursorRect(0, 0, 0, 0);
    }
  };

  //
  // get background bitmap name
  //
  const _bgBitmapMenu = () => {
    const setBySystem = $gameSystem._bgBitmapMenu;
    return setBySystem == null ? bgBitmapMenu : setBySystem;
  };

  const _doesSetBg = className => {
    const setBySystem = $gameSystem._setBg[className];
    return setBySystem == null ? setBg[className] : setBySystem;
  };

  const _originalBgBitmap = className => {
    const setBySystem = $gameSystem._bgBitmap[className];
    return setBySystem == null ? bgBitmap[className] : setBySystem;
  };

  const bgBitmapName = () => {
    const className = SceneManager._scene.constructor.name;
    const doesSet = _doesSetBg(className);
    return doesSet ? _originalBgBitmap(className) : _bgBitmapMenu();
  };

  //
  // process windows' opacity 
  //
  const isWindowVisible = () => allowWindowDisp && !bgBitmapName();

  const _Scene_MenuBase_create = Scene_MenuBase.prototype.create;
  Scene_MenuBase.prototype.create = function () {
    this._allWindows = [];
    _Scene_MenuBase_create.call(this);
  };

  const _Scene_MenuBase_start = Scene_MenuBase.prototype.start;
  Scene_MenuBase.prototype.start = function() {
    this._setWindowsOpacity();
    _Scene_MenuBase_start.call(this);
  };

  const _Scene_MenuBase_addWindow = Scene_MenuBase.prototype.addWindow;
  Scene_MenuBase.prototype.addWindow = function(window) {
    _Scene_MenuBase_addWindow.call(this, window);
    this._allWindows.push(window);
  };

  Scene_MenuBase.prototype._setWindowsOpacity = function () {
    if (!isWindowVisible()) {
      for (const window of this._allWindows) {
        window.opacity = 0;
      }
    }
  };

  const _Scene_MenuBase_createBackground =
    Scene_MenuBase.prototype.createBackground;
  Scene_MenuBase.prototype.createBackground = function () {
    const bgName = bgBitmapName();
    if (bgName){
      this._backgroundSprite = new Sprite();
      this._backgroundSprite.bitmap = ImageManager.loadPicture(bgName);
      this.addChild(this._backgroundSprite);
    } else {
      _Scene_MenuBase_createBackground.call(this);
    }
  };

  //
  // check objects' note
  //
  const standPictureName = object => object.meta.stand_picture;
  const pictureOffsetX = object => +object.meta.actor_offset_x || 0;
  const pictureOffsetY = object => +object.meta.actor_offset_y || 0;

  Game_Actor.prototype.standPictureCandidateObjects = function() {
    const objects = this.states();
    for (const item of this.equips()) {
      if (item) {
        objects.push(item);
      }
    }
    objects.push(this.currentClass(), this.actor());
    return objects;
  };

  Game_Actor.prototype.standPictureObject = function() {
    const objects = this.standPictureCandidateObjects();
    let targetObject = null;
    for (object of objects) {
      if (standPictureName(object)) {
        targetObject = object;
        break;
      }
    }
    return targetObject;
  };

  //
  // draw image
  //
  const _Window_MenuStatus_drawItem = Window_MenuStatus.prototype.drawItem;
  Window_MenuStatus.prototype.drawItem = function(index) {
    const actor = $gameParty.members()[index];
    const bitMapObj = actor ? actor.standPictureObject() : null;
    const bitmapName = bitMapObj ? standPictureName(bitMapObj) : null;
    const bitmap = bitmapName ? ImageManager.loadPicture(bitmapName) : null;
    if (bitmap && !bitmap.isReady()) {
      bitmap.addLoadListener(_Window_MenuStatus_drawItem.bind(this, index));
    } else {
      _Window_MenuStatus_drawItem.call(this, index);
    }
  };

  // !!overwrite!!
  Window_MenuStatus.prototype.drawItemImage = function(index) {
    const actor = this.actor(index);
    if (!actor) {
      return;
    }
    const rect = this.itemRectWithPadding(index);
    // load stand_picture
    const bitMapObj = actor ? actor.standPictureObject() : null;
    const bitmapName = bitMapObj ? standPictureName(bitMapObj) : null;
    const offsetX = bitMapObj ? pictureOffsetX(bitMapObj) : 0;
    const offsetY = bitMapObj ? pictureOffsetY(bitMapObj) : 0;
    const bitmap = bitmapName ? ImageManager.loadPicture(bitmapName) : null;
    const w = Math.min(rect.width, (bitmapName ? bitmap.width : 144));
    const h = Math.min(rect.height, (bitmapName ? bitmap.height : 144));
    const lineHeight = this.lineHeight();
    this.changePaintOpacity(actor.isBattleMember());
    if (bitmap) {
      const sx = (bitmap.width > w) ? (bitmap.width - w) / 2 : 0;
      const sy = (bitmap.height > h) ? (bitmap.height - h) / 2 : 0;
      const dx = (bitmap.width > rect.width) ? rect.x :
        rect.x + (rect.width - bitmap.width) / 2;
      const dy = (bitmap.height > rect.height) ? rect.y :
        rect.y + (rect.height - bitmap.height) / 2;
      this.contents.blt(bitmap, sx - offsetX, sy - offsetY, w, h, dx, dy);
    } else {
      // changed at Ver1.1.0 : not call original function, do another instead
      this.drawActorFace(actor, rect.x, rect.y + lineHeight * 2, w, h);
    }
    this.changePaintOpacity(true);
  };

  //
  // draw parameters
  //
  let gaugeWidth = 148;

  const _Sprite_Gauge_bitmapWidth = Sprite_Gauge.prototype.bitmapWidth;
  Sprite_Gauge.prototype.bitmapWidth = function() {
    if (this.isSceneNeedsAdjustGauge()) {
      return Math.min(gaugeWidth, 148);
    } else {
      return _Sprite_Gauge_bitmapWidth.call(this);
    }
  };

  Window_MenuStatus.prototype.drawItemStatus = function(index) {
    if (!isDisplayStatus) {
      return;
    }
    const actor = this.actor(index);
    const rect = this.itemRectWithPadding(index);
    const x = rect.x;
    const y = rect.y;
    const width = rect.width;
    gaugeWidth = width;
    const bottom = y + rect.height;
    const lineHeight = this.lineHeight();
    const addPos = doesDisplayNextLevel ? 1 : 0;
    this.drawActorName(actor, x, y + lineHeight * 0, width);
    this.drawActorClass(actor, x, y + lineHeight * 1, width);
    this.drawActorIcons(actor, x, bottom - lineHeight * (4 + addPos), width);
    this.drawActorLvlMV(actor, x, bottom - lineHeight * (3 + addPos), width);
    this.drawExpToNext(actor, x, bottom - lineHeight * 3, width);
    this.placeBasicGauges(actor, x, bottom - lineHeight * 2);
  };

  Window_StatusBase.prototype.drawActorLvlMV = function(actor, x, y, width) {
    this.changeTextColor(ColorManager.systemColor());
    this.drawText(TextManager.levelA, x, y, 48);
    this.resetTextColor();
    this.drawText(actor.level, x, y, width, 'right');
  };

  const expNextValue = actor => {
    if (actor.isMaxLevel()) {
        return "-------";
    } else {
        return actor.nextRequiredExp();
    }
  };

  Window_StatusBase.prototype.drawExpToNext = function(actor, x, y, width) {
    if (doesDisplayNextLevel) {
      this.changeTextColor(ColorManager.systemColor());
      const text = textNextLevel.format(TextManager.levelA);
      this.drawText(text, x, y, width, 'left');
      this.resetTextColor();
      this.drawText(expNextValue(actor), x, y, width, 'right');
    }
  };

  const _Window_StatusBase_placeBasicGauges =
   Window_StatusBase.prototype.placeBasicGauges;
  Window_StatusBase.prototype.placeBasicGauges = function(actor, x, y, width) {
    if (isMvStyleStatus && !$gameParty.inBattle()) {
      width = width || 168;
      this.drawGaugesMV(actor, x, y, width);
    }
    else {
      _Window_StatusBase_placeBasicGauges.call(this, actor, x, y, width);
    }
  };

  //
  // draw parameters and gauges for MV style
  //
  Window_StatusBase.prototype.drawGaugesMV = function(actor, x, y, width) {
    const lineHeight = this.lineHeight();
    this.drawActorHpMV(actor, x, y + lineHeight * 0, width);
    this.drawActorMpMV(actor, x, y + lineHeight * 1, width);
  };

  Window_StatusBase.prototype.drawActorHpMV = function(actor, x, y, width) {
    const color1 = ColorManager.hpGaugeColor1();
    const color2 = ColorManager.hpGaugeColor2();
    this.drawGaugeMV(x, y, width, actor.hpRate(), color1, color2);
    this.changeTextColor(this.systemColor());
    const hpColor = ColorManager.hpColor(actor);
    const normalColor = ColorManager.normalColor();
    this.drawText(TextManager.hpA, x, y, 44);
    this.drawCurrentAndMaxMV(actor.hp, actor.mhp, x, y, width, hpColor,
      normalColor
    );
  };

  Window_StatusBase.prototype.drawActorMpMV = function(actor, x, y, width) {
    const color1 = ColorManager.mpGaugeColor1();
    const color2 = ColorManager.mpGaugeColor2();
    this.drawGaugeMV(x, y, width, actor.mpRate(), color1, color2);
    this.changeTextColor(this.systemColor());
    const mpColor = ColorManager.mpColor(actor);
    const normalColor = ColorManager.normalColor();
    this.drawText(TextManager.mpA, x, y, 44);
    this.drawCurrentAndMaxMV(actor.mp, actor.mmp, x, y, width, mpColor,
      normalColor
    );
  };

  Window_Base.prototype.drawGaugeMV = function(x, y, width, rate, c1, c2) {
    const fillW = Math.floor(width * rate);
    const gaugeY = y + this.lineHeight() - 8;
    const bgColor = ColorManager.gaugeBackColor();
    this.contents.fillRect(x, gaugeY, width, 6, bgColor);
    this.contents.gradientFillRect(x, gaugeY, fillW, 6, c1, c2);
  };

  Window_Base.prototype.drawCurrentAndMaxMV = function(current, max, x, y,
   width, color1, color2) {
    const labelWidth = this.textWidth(
      Math.max(TextManager.hpA, TextManager.mpA)
    );
    const valueWidth = this.textWidth('0000');
    const slashWidth = this.textWidth('/');
    const x1 = x + width - valueWidth;
    const x2 = x1 - slashWidth;
    const x3 = x2 - valueWidth;
    if (x3 >= x + labelWidth) {
        this.changeTextColor(color1);
        this.drawText(current, x3, y, valueWidth, 'right');
        this.changeTextColor(color2);
        this.drawText('/', x2, y, slashWidth, 'right');
        this.drawText(max, x1, y, valueWidth, 'right');
    } else {
        this.changeTextColor(color1);
        this.drawText(current, x1, y, valueWidth, 'right');
    }

  };

  //
  // display current map name
  //
  const mapName = () => {
    const name = $gameMap.displayName();
    return name ? name : $dataMapInfos[$gameMap.mapId()].name;
  };

  const _Scene_Menu_create = Scene_Menu.prototype.create;
  Scene_Menu.prototype.create = function() {
    _Scene_Menu_create.call(this);
    this.createMapNameWindow();
  };

  Scene_Menu.prototype.createMapNameWindow = function() {
    if (isDisplayMapName) {
      const rect = this.mapNameAlt3WindowRect();
      this._mapNameWindow = new Window_MapNameAlt3(rect);
      this.addWindow(this._mapNameWindow);
    }
  };

  const _Scene_Menu_terminate = Scene_Menu.prototype.terminate;
  Scene_Menu.prototype.terminate = function () {
    _Scene_Menu_terminate.call(this);
    if (isDisplayMapName) {
      this.removeChild(this._mapNameWindow);
    }
  };

  Scene_Menu.prototype.mapNameAlt3WindowRect = function() {
    const ww = Graphics.boxWidth - this._goldWindow.width;
    const wh = this.calcWindowHeight(1, true);
    const wx = 0;
    const wy = this.mainAreaBottom() - wh;
    return new Rectangle(wx, wy, ww, wh);
  };

  function Window_MapNameAlt3() {
    this.initialize(...arguments);
  }

  Window_MapNameAlt3.prototype = Object.create(Window_MapName.prototype);
  Window_MapNameAlt3.prototype.constructor = Window_MapNameAlt3;

  Window_MapNameAlt3.prototype.initialize = function (rect) {
    // not inherit super class, but Window_Base instead.
    Window_Base.prototype.initialize.call(this, rect);
    this.refresh();
  };

  Window_MapNameAlt3.prototype.update = function () {
    // do nothing
  };

  Window_MapNameAlt3.prototype.refresh = function() {
    // not inherit super class
    this.contents.clear();
    if (mapName()) {
      this.changeTextColor(ColorManager.systemColor());
      const textWidth = this.textWidth(locationString) + this.itemPadding();
      const row = 4;
      const col = 4;
      this.drawText(locationString, row, col, this.width, 'left');
      this.resetTextColor();
      const orgX = row + textWidth;
      this.drawText(mapName(), orgX, col, this.width, 'left');
    }
  };

  //
  // smooth scroll menu command window (Added at Ver1.3.0)
  //
  Window_MenuCommand.prototype.cursorDown = function(wrap) {
    if (wrap) {
      const index = Math.max(0, this.index());
      const maxItems = this.maxItems();
      const maxCols = this.maxCols();
      const nextIndex = index + maxCols;
      if (maxItems % maxCols !== 0 && nextIndex >= maxItems &&
       index < maxItems) {
        if (index === maxItems - 1) {
          this.smoothSelect(nextIndex.mod(maxCols));
        } else {
          this.smoothSelect(maxItems - 1);
        }
      } else {
        this.smoothSelect((index + maxCols) % maxItems);
      }
    }
  };

  Window_MenuCommand.prototype.cursorUp = function(wrap) {
    if (wrap) {
      const index = Math.max(0, this.index());
      const maxItems = this.maxItems();
      const maxCols = this.maxCols();
      const nextIndex = index - maxCols;
      if (maxItems % maxCols !== 0 && nextIndex < 0 &&
       index < maxCols) {
        const indexLeftDown = Math.floor((maxItems - 1) / maxCols) * maxCols;
        const newIndex = Math.min(indexLeftDown + nextIndex.mod(maxCols),
          maxItems - 1
        );
        this.smoothSelect(newIndex);
      } else {
        this.smoothSelect((index - maxCols + maxItems).mod(maxItems));
      }
    }
  };

  Window_MenuCommand.prototype.cursorRight = function(wrap) {
    if (wrap) {
      const index = this.index();
      const maxItems = this.maxItems();
      const maxCols = this.maxCols();
      const indexLeft = Math.floor(index / maxCols) * maxCols;
      const indexColumn = Math.floor((index + 1) % maxCols);
      if (index === maxItems - 1) {
        this.smoothSelect(indexLeft);
      } else {
        const rightMost = Math.min(maxItems - 1 - indexLeft, indexColumn);
        this.smoothSelect(indexLeft + rightMost);
      }
    }
  };

  Window_MenuCommand.prototype.cursorLeft = function(wrap) {
    if (wrap) {
      const index = this.index();
      const maxItems = this.maxItems();
      const maxCols = this.maxCols();
      const indexLeft = Math.floor(index / maxCols) * maxCols;
      const indexRow = Math.floor((index + 1) % maxCols);
      if (index % maxCols === 0) {
        const rightMost = Math.min(maxItems - 1 - indexLeft, maxCols - 1);
        this.smoothSelect(indexLeft + rightMost);
      } else {
        this.smoothSelect(index - 1);
      }
    }
  };

  // ----------------------------------------------------------------------
  // Scene_Shop
  // ----------------------------------------------------------------------

  //
  // preload shop special background bitmaps
  //
  const needsShopBuyBg = () => doesSetBgShopBuy && !!bgNameShopBuy;
  const needsShopSellBg = () => doesSetBgShopSell && !!bgNameShopSell;

  const preloadExtraShopBackground = () => {
    if (needsShopBuyBg()) {
      ImageManager.loadPicture(bgNameShopBuy);
    }
    if (needsShopSellBg()) {
      ImageManager.loadPicture(bgNameShopSell);
    }
  };

  const _Scene_Shop_create = Scene_Shop.prototype.create;
  Scene_Shop.prototype.create = function() {
    preloadExtraShopBackground();
    this.doesShowWindow = null;
    this.pictureName = null;
    _Scene_Shop_create.call(this);
  };

  //
  // change shop window display condition when phase changes
  //
  const shopBuyWindowStatus = () => {
   return doesSetBgShopBuy ? !bgNameShopBuy : isWindowVisible();
  };

  const shopSellWindowStatus = () => {
   return doesSetBgShopSell ? !bgNameShopSell : isWindowVisible();
  };

  Scene_Shop.prototype.modifyShopWindowVisible = function(phase) {
    switch (phase) {
    case "buy":
      this.changeWindowsOpacity(shopBuyWindowStatus());
      break;
    case "sell":
      this.changeWindowsOpacity(shopSellWindowStatus());
      break;
    default:
      this.changeWindowsOpacity(isWindowVisible());
    }
  };

  Scene_Shop.prototype.changeWindowsOpacity = function(doesShow) {
    if (allowWindowDisp && doesShow !== this.doesShowWindow) {
      this.doesShowWindow = doesShow;
      for (const window of this._allWindows) {
        window.opacity = doesShow ? 255 : 0;
      }
    }
  };

  //
  // change shop background bitmap when phase changes
  //
  const shopBgImageName = phase => {
    if (phase === "buy" && doesSetBgShopBuy) {
      return bgNameShopBuy;
    } else if (phase === "sell" && doesSetBgShopSell) {
      return bgNameShopSell;
    } else {
      return bgBitmapName();
    }
  };

  Scene_Shop.prototype.modifyShopBgImage = function(phase) {
    const bgName = shopBgImageName(phase);
    if (bgName !== this.pictureName) {
      if (bgName) {
        this._backgroundSprite.bitmap = ImageManager.loadPicture(bgName);
      } else {
        this._backgroundSprite.bitmap = SceneManager.backgroundBitmap();
      }
    }
  };

  //
  // call bg change
  //
  Scene_Shop.prototype.modifyShopBg = function(phase) {
    this.modifyShopWindowVisible(phase);
    this.modifyShopBgImage(phase);
  };

  const _Scene_Shop_commandBuy = Scene_Shop.prototype.commandBuy;
  Scene_Shop.prototype.commandBuy = function() {
    this.modifyShopBg("buy");
    _Scene_Shop_commandBuy.call(this);
  };

  const _Scene_Shop_onBuyCancel = Scene_Shop.prototype.onBuyCancel;
  Scene_Shop.prototype.onBuyCancel = function() {
    _Scene_Shop_onBuyCancel.call(this);
    this.modifyShopBg("main");
  };

  const _Scene_Shop_commandSell = Scene_Shop.prototype.commandSell;
  Scene_Shop.prototype.commandSell = function() {
    this.modifyShopBg("sell");
    _Scene_Shop_commandSell.call(this);
  };

  const _Scene_Shop_onCategoryCancel = Scene_Shop.prototype.onCategoryCancel;
  Scene_Shop.prototype.onCategoryCancel = function() {
    this.modifyShopBg("main");
    _Scene_Shop_onCategoryCancel.call(this);
  };

  // ----------------------------------------------------------------------

})();
