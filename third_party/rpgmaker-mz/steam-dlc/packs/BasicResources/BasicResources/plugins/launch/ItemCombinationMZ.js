//=============================================================================
// ItemCombinationMZ.js
//=============================================================================
// This plugin is MZ version of ItemCombination.js the RMMV plugin.
// [Update History]
// 2023/Dec/24 Ver1.0.0 First Release
// 2023/Dec/27 Ver1.1.0 Add Plugin Command to Open Scene Directly from Map.

/*:
 * @target MZ
 * @plugindesc [Ver1.1.0]Creates an item combination system.
 * @author Sasuke KANNAZUKI, Jeremy Cannady
 *
 * @param doesAddMenuCommand
 * @text Can Call From Menu Command?
 * @desc Select "No" If It Calls The Scene from Map Directly and It Needs To Hide Command from Menu.
 * @type boolean
 * @on Yes
 * @off No
 * @default true
 *
 * @param dummy4text
 * @text [Text Settings]
 * @desc
 *
 * @param commandName
 * @parent dummy4text
 * @text Command Name
 * @desc Name That Displays At Main Menu
 * @type string
 * @default Combine
 *
 * @param requiredItemText
 * @parent dummy4text
 * @text Required Ingredients
 * @desc The Text That Means "Required Ingredients".
 * @type string
 * @default Required Ingredients :
 *
 * @param combineText
 * @parent dummy4text
 * @text Text of Combine
 * @desc Text That Displays On The Button At Combination Menu
 * @type string
 * @default Combine
 *
 * @param failureText
 * @parent dummy4text
 * @text Text of Failure Item
 * @desc The string that means "item obtains when combine is failure". Set empty not to display item.
 * @type string
 * @default Item Gets When Failure :
 *
 * @command callMenu
 * @text Open Combination Scene
 * @desc Open Item Combination Scene Directly from Map
 *
 * @command learn
 * @text Learn Recipe
 * @desc Learn Recipe for Specified Items
 *
 * @arg itemIds
 * @text Target Items
 * @desc Items to Learn Its Recipe
 * @type item[]
 * @default []
 *
 * @command forget
 * @text Forget Recipe
 * @desc Forget Recipe for Specified Items
 *
 * @arg itemIds
 * @text Target Items
 * @desc Items To Forget Its Recipe
 * @type item[]
 * @default []
 *
 * @command changeChance
 * @text Change Success Rate
 * @desc Change Success Rate for Specified Items
 *
 * @arg itemIds
 * @text Target Items
 * @desc Items To Change Success Rate
 * @type item[]
 * @default []
 *
 * @arg chance
 * @text Success Rate
 * @desc set between 0 and 1. Set 1 to Success Always.
 * @type number
 * @decimals 2
 * @max 1.00
 * @min 0.00
 * @default 1.00
 *
 * @command setEnable
 * @text Enable To Combine?
 * @desc If It Sets False, It Becomes Disable to Item Combination.
 * @type boolean
 * @on Enable
 * @off Disable
 * @default true
 *
 * @arg isEnable
 * @text Validate?
 * @type boolean
 * @on Enable
 * @off Disable
 * @default true
 *
 * @help 
 * This plugin runs under RPG Maker MZ.
 *
 * [Summary]
 * This plugin adds Combine command to main menu.
 * At combine scene, you can do it.
 *
 * NOTE: The command is not added if party doesn't learn any recipies.
 *
 * [NoteTags]
 * Write down following setting in the note of object item:
 * <comboChance:0.95>      where 0.95 is 95%
 * <comboIngredient1:1,1>  where you require one item #1 
 * <comboIngredient2:2,3>	 where you require three of item #2
 * <comboFail:4>           If you fail you make item #4
 *
 * Note: It can set comboIngredient tag at most 5 (comboIngredient5).
 *
 * [Plugin Commands]
 * NOTE: If comboChance Tag Is Not Written in Note, The Item Is Invalid.
 *
 * There's following 5 plugin commands:
 * - Call Item Combination Scene Directly from Map Scene (Since Ver1.1.0)
 * - Learn New Recipe
 *   Be Sure to Run This in Order to Validate Item Combination.
 * - Forget Recipe
 * - Change The Success Rate to Combine
 * - Toggle to Enable/Disable to Do the Item Combination.
 *
 * [Display Failure Item] (since Ver1.1.0)
 * It displays the item that when combination failed.
 * But it doesn't display when...
 * - it sets the parameter "Text of Failure Item" empty string.
 * - it doesn't describe comboFail tag on its note.
 * - comboChance is set >= 1. (i.e. Success Always)
 *
 * [License]
 * this plugin is released under MIT license.
 * http://opensource.org/licenses/mit-license.php
 */

/*:ja
 * @target MZ
 * @plugindesc [Ver1.1.0]アイテム合成システム
 * @author 神無月サスケ, Jeremy Cannady
 *
 * @param doesAddMenuCommand
 * @text メニューコマンドに表示？
 * @desc マップから直接呼び出す場合、メニューに表示しないことも出来ます。
 * @type boolean
 * @on 表示する
 * @off 表示しない
 * @default true
 *
 * @param dummy4text
 * @text [文字列設定]
 * @desc
 *
 * @param commandName
 * @parent dummy4text
 * @text コマンド名
 * @desc メインメニューで表示されるコマンド名です。
 * @type string
 * @default アイテム合成
 *
 * @param requiredItemText
 * @parent dummy4text
 * @text 必要素材
 * @desc アイテム合成画面で表示される「必要素材」を意味する文字列です。
 * @type string
 * @default 必要素材：
 *
 * @param combineText
 * @parent dummy4text
 * @text 合成実行
 * @desc 合成実行の項目に表示される文字列です。
 * @type string
 * @default 合成実行
 *
 * @param failureText
 * @parent dummy4text
 * @text 失敗時獲得アイテム
 * @desc 「失敗時に得られるアイテム」を意味する文字列です。空欄にするとアイテムも表示されません。
 * @type string
 * @default 失敗時獲得アイテム :
 *
 * @command callMenu
 * @text メニューを開く
 * @desc マップから直接アイテム合成シーンをを呼び出します。
 *
 * @command learn
 * @text レシピ習得
 * @desc 指定したアイテムのレシピを習得します。
 *
 * @arg itemIds
 * @text 習得するアイテムレシピ
 * @desc 複数選択が可能です。
 * @type item[]
 * @default []
 *
 * @command forget
 * @text レシピ放棄
 * @desc 指定したアイテムのレシピを忘れ、以後使えなくします。
 *
 * @arg itemIds
 * @text 放棄するアイテムレシピ
 * @desc 複数選択が可能です。
 * @type item[]
 * @default []
 *
 * @command changeChance
 * @text 成功率変更
 * @desc 指定したアイテム(複数指定可)の成功率を一律に変更します。
 *
 * @arg itemIds
 * @text 対象アイテム
 * @desc 成功率を変更するアイテム
 * @type item[]
 * @default []
 *
 * @arg chance
 * @text 成功率
 * @desc 0.00～1.00の間で指定してください。1.00にすると常に成功します。
 * @type number
 * @decimals 2
 * @max 1.00
 * @min 0.00
 * @default 1.00
 *
 * @command setEnable
 * @text 合成を有効化？
 * @desc OFFにすると、次にONにするまでは、合成が出来なくなります。
 *
 * @arg isEnable
 * @text 許可？
 * @type boolean
 * @on 可能
 * @off 禁止
 * @default true
 *
 * @help 
 * このプラグインは、RPGツクールMZに対応しています。
 *
 * このプラグインは、アイテム合成システムを導入します。
 *
 * ■概要
 * このプラグインは、アイテム合成コマンドをメニューに追加します。
 * アイテム合成シーンを呼び出すと、合成が可能になります。
 *
 * 注意：レシピをひとつも習得していない場合、コマンドは追加されません。
 *
 * ■レシピの設定
 * 合成後に完成するアイテムのメモで、以下の記述をしてください。
 * <comboChance:0.95>     成功確率。0.95 の場合 95% になります。
 * <comboIngredient1:1,1> 必要アイテムIDと個数。ID1のアイテムがひとつ必要に。
 * <comboIngredient2:2,3>	さらに必要なアイテム。ID2のアイテムが3個必要に。
 * <comboFail:4>      合成失敗時に生成されるアイテム。ここではID4になります。
 *
 * 必要素材(comboIngredient)は最高5個(comboIngredient5)まで設定できます。
 *
 * ■プラグインコマンド
 * 注意：メモに comboChance が設定されているアイテムのみ有効です。
 *
 * 以下のことが可能です。
 * ・マップから直接メニューを呼び出す (Ver1.1.0～)
 * ・特定のアイテムのレシピを習得
 *   これを有効なアイテムに対して行うことで、アイテム合成コマンドが現れます。
 * ・特定のアイテムのレシピを放棄
 * ・特定のアイテムの合成成功率の変更
 * ・合成が可能かどうかの切り替え
 *
 * ■失敗時生成アイテムの表示(Ver1.1.0～)
 * 合成失敗時に生成されるアイテムが表示されます。
 * ただし、以下の場合は表示されません。
 * - パラメータ「失敗時獲得アイテム」を空文字に設定した場合
 * - 失敗時生成アイテム(comboFail)未設定の時
 * - 成功確率(comboChance)が100％の時
 *
 * ■ライセンス表記
 * このプラグインは MIT ライセンスで配布されます。
 * ご自由にお使いください。
 * http://opensource.org/licenses/mit-license.php
 */

/*:zh
 * @target MZ
 * @plugindesc [Ver1.1.0] 添加物品合成系统。
 * @author Sasuke KANNAZUKI, Jeremy Cannady
 *
 * @param doesAddMenuCommand
 * @text 是否在菜单中显示？
 * @desc 如果希望从地图直接打开合成界面并隐藏菜单命令，请选择“否”。
 * @type boolean
 * @on 是
 * @off 否
 * @default true
 *
 * @param dummy4text
 * @text 【文字设置】
 * @desc
 *
 * @param commandName
 * @parent dummy4text
 * @text 命令名称
 * @desc 主菜单中显示的命令名称。
 * @type string
 * @default 合成
 *
 * @param requiredItemText
 * @parent dummy4text
 * @text 所需材料
 * @desc 表示“所需材料”的文字。
 * @type string
 * @default 所需材料：
 *
 * @param combineText
 * @parent dummy4text
 * @text 合成按钮文字
 * @desc 合成界面中按钮上显示的文字。
 * @type string
 * @default 合成
 *
 * @param failureText
 * @parent dummy4text
 * @text 失败时获得物品
 * @desc 表示“合成失败时获得的物品”的文字。留空则不显示此项。
 * @type string
 * @default 失败时获得物品：
 *
 * @command callMenu
 * @text 打开合成界面
 * @desc 直接从地图打开物品合成界面。
 *
 * @command learn
 * @text 学习配方
 * @desc 学习指定物品的配方。
 *
 * @arg itemIds
 * @text 目标物品
 * @desc 要学习配方的物品（可多选）。
 * @type item[]
 * @default []
 *
 * @command forget
 * @text 忘记配方
 * @desc 忘记指定物品的配方。
 *
 * @arg itemIds
 * @text 目标物品
 * @desc 要遗忘配方的物品（可多选）。
 * @type item[]
 * @default []
 *
 * @command changeChance
 * @text 更改成功率
 * @desc 更改指定物品的合成成功率。
 *
 * @arg itemIds
 * @text 目标物品
 * @desc 要更改成功率的物品（可多选）。
 * @type item[]
 * @default []
 *
 * @arg chance
 * @text 成功率
 * @desc 取值范围 0.00～1.00（1.00 = 永远成功）。
 * @type number
 * @decimals 2
 * @max 1.00
 * @min 0.00
 * @default 1.00
 *
 * @command setEnable
 * @text 是否启用合成？
 * @desc 若设为否，则暂时无法进行物品合成。
 *
 * @arg isEnable
 * @text 启用？
 * @type boolean
 * @on 启用
 * @off 禁用
 * @default true
 *
 * @help
 * 本插件在 RPG Maker MZ 下运行。
 *
 * 【功能概要】
 * 本插件为游戏添加“物品合成”系统。
 * 可以在主菜单中添加“合成”命令，也可通过插件指令直接打开合成界面。
 *
 * 【使用方法】
 * 在想要作为“合成产物”的物品的备注中写入以下标签：
 *
 * <comboChance:0.95>     // 成功率（此处为95%）
 * <comboIngredient1:1,1> // 材料1：物品ID 1 × 1个
 * <comboIngredient2:2,3> // 材料2：物品ID 2 × 3个
 * <comboFail:4>          // 失败时获得的物品ID 4
 *
 * 最多可以设置5种材料（comboIngredient1～comboIngredient5）。
 *
 * 【注意事项】
 * 若未设置 <comboChance> 标签，则该物品不会被视为可合成物品。
 *
 * 【插件指令】
 * - 打开合成界面（callMenu）
 * - 学习配方（learn）
 * - 遗忘配方（forget）
 * - 更改成功率（changeChance）
 * - 启用/禁用合成功能（setEnable）
 *
 * 【合成失败显示】
 * 若设置了“失败时获得物品”且成功率小于100%，则会在界面中显示失败时获得的物品。
 * 若以下任一条件成立，则不显示：
 * - “失败时获得物品”文字参数为空
 * - 未设置 comboFail 标签
 * - 成功率为 100%
 *
 * 【备注】
 * - 仅当玩家已学习至少一个配方时，菜单中才会显示“合成”命令。
 * - 支持从地图场景直接打开合成界面。
 *
 * 【许可】
 * 本插件以 MIT 许可证发布。
 * 可自由修改、再分发、商用或成人游戏中使用，无需署名。
 * http://opensource.org/licenses/mit-license.php
 */


(() => {
  const pluginName = 'ItemCombinationMZ';
  //
  // process parameters
  //
  const parameters = PluginManager.parameters(pluginName);
  const commandName = parameters['commandName'] || "Combine";
  const requiredItemText = parameters['requiredItemText'] ||
    "Required Ingredients :";
  const combineText = parameters['combineText'] || "Combine";
  const failureText = parameters['failureText'] || "";
  const doesAddMenuCommand = eval(parameters['doesAddMenuCommand']);

  //
  // essential variables
  //
  Game_Party.prototype._CombineMenuEnabled = true;
  Game_Party.prototype._craftingRecipes = {};
  Game_Party.prototype._craftingLearnedRecipes = [];

  //
  // process plugin commands
  //
  PluginManager.registerCommand(pluginName, "callMenu", args => {
    if (isItemComboEnabled()) {
      SceneManager.goto(Scene_CraftingMenu);
    } else {
      SoundManager.playBuzzer();
    }
  });

  PluginManager.registerCommand(pluginName, "learn", args => {
    const itemIdsToLearn = eval(args.itemIds);
    for (const itemId of itemIdsToLearn) {
      let item, name;
      if ((item = $dataItems[+itemId]) && (name = item.name)) {
        if (item.meta.comboChance) {
          $gameParty.learnRecipe(name);
        }
      }
    }
  });

  PluginManager.registerCommand(pluginName, "forget", args => {
    const itemIdsToForget = eval(args.itemIds);
    for (const itemId of itemIdsToForget) {
      let item, name;
      if ((item = $dataItems[+itemId]) && (name = item.name)) {
        $gameParty.forgetRecipe(name);
      }
    }
  });

  PluginManager.registerCommand(pluginName, "changeChance", args => {
    const itemIdsToSetChance = eval(args.itemIds);
    for (const itemId of itemIdsToSetChance) {
      let item, name;
      if ((item = $dataItems[+itemId]) && (name = item.name)) {
        $gameParty.changeItemComboChance(name, +args.chance);
      }
    }
  });

  PluginManager.registerCommand(pluginName, "setEnable", args => {
    $gameParty._CombineMenuEnabled = !!eval(args.isEnable);
  });

  //
  // manipulate recipies
  //
  Game_Party.prototype.learnRecipe = function(value) {
    if (this._craftingLearnedRecipes.indexOf(value) === -1) {
      this._craftingLearnedRecipes.push(value);
    }
  };

  Game_Party.prototype.forgetRecipe = function(value) {
    // Stores the value of the index,
    // if the value is -1 then we do not have the recipe.
    const index = this._craftingLearnedRecipes.indexOf(value);
    // If we do have the recipe learned
    // then delete it from out learned recipes list.
    if (this._craftingLearnedRecipes.indexOf(value) !== -1) {
      this._craftingLearnedRecipes.splice(index, 1);
	  }
  };

  Game_Party.prototype.changeItemComboChance = function(recipeName, newChance
  ) {
    const recipeDetails = Game_Party.prototype._craftingRecipes[recipeName];
    recipeDetails[1] = newChance;
  };

  //
  // setting recipe data to Game_Party
  //
  const copyOfInitializeCombo = Game_Party.prototype.initialize;
  Game_Party.prototype.initialize = function() {
    copyOfInitializeCombo.call(this);
    this.populateCraftingRecipes();
  };

  Game_Party.prototype.populateCraftingRecipes = function() {
    const lengthOfItemList = $dataItems.length;
    for(let i = 1; i < lengthOfItemList; i++) {
      const ingredientArray = [];
      if ($dataItems[i].meta.comboChance != null) {
        ingredientArray.push(Number($dataItems[i].id));
        ingredientArray.push(Number($dataItems[i].meta.comboChance));
        ingredientArray.push(Number($dataItems[i].meta.comboFail));
      }
      for (let j = 1; j <= 5; j++) {
        const noteTag = "comboIngredient" + j;
        const tagValue = $dataItems[i].meta[noteTag];
        if (tagValue != null) {
          const ingredientArrayIngredient = tagValue.split(",");
          ingredientArrayIngredient.forEach(k => parseInt(k));
          ingredientArray.push(ingredientArrayIngredient);
        } else {
          break;
        }
      }
      if ($dataItems[i].meta.comboChance != null) {
        const name = $dataItems[i].name;
        Game_Party.prototype._craftingRecipes[name] = ingredientArray;
      }
    }
  };

  Game_Party.prototype.canCombine = function(recipeKeyName) {
    const ingredientArray = $gameParty.ingredients(recipeKeyName);
    let haveAllIngredients = 0;
    for (let i = 0; i < ingredientArray.length; i++) {
      const itemId = ingredientArray[i][0];
      const requiredAmount = ingredientArray[i][1];
      if ($gameParty.numItems($dataItems[itemId]) >= requiredAmount) {
        haveAllIngredients++;
      }
    }
    const ingredientNum = $gameParty.ingredients(recipeKeyName).length;
	  return haveAllIngredients === ingredientNum;
  };

  Game_Party.prototype.ingredients = function(recipeKeyName) {
    const recipeDetails = Game_Party.prototype._craftingRecipes[recipeKeyName];
    const ingredientsList = [];
    for (let i = 3; i < recipeDetails.length; i++) {
      const itemId = recipeDetails[i];
      ingredientsList.push(itemId);
	  }
	  return ingredientsList;
  };

  //
  // add command to menu
  //
  const isItemComboEnabled = () => {
    if ($gameParty._craftingLearnedRecipes.length < 1) {
      return false;
    } else if ($gameParty._CombineMenuEnabled) {
      return true;
    } else {
      return false;
    }
  };

  const copyOfAddOrginalCommands =
    Window_MenuCommand.prototype.addOriginalCommands;
  Window_MenuCommand.prototype.addOriginalCommands = function() {
    copyOfAddOrginalCommands.call(this);
    if (doesAddMenuCommand && isItemComboEnabled()) {
      this.addCommand(commandName, 'itemCombinationCommand', true);
	  }
  };

  const copyOfCreateCommandWindow = Scene_Menu.prototype.createCommandWindow;
  Scene_Menu.prototype.createCommandWindow = function() {
    copyOfCreateCommandWindow.call(this);
    this._commandWindow.setHandler('itemCombinationCommand',
      this.commandCombinationMenu.bind(this)
    );
  };

  Scene_Menu.prototype.commandCombinationMenu = function() {
    SceneManager.push(Scene_CraftingMenu);
  };

  // ----------------------------------------------------------------------
  // craftingListWindow
  // ----------------------------------------------------------------------
  function craftingListWindow() {
    this.initialize.apply(this, arguments);
  }

  craftingListWindow.prototype = Object.create(Window_Command.prototype);
  craftingListWindow.prototype.constructor = craftingListWindow;

  //
  // initialize
  //
  craftingListWindow.prototype.initialize = function(rect) {
    Window_Command.prototype.initialize.call(this, rect);
    this.selectLast();
  };

  craftingListWindow.initCommandPosition = function() {
    this._lastCommandSymbol = null;
  };

  //
  // command settings
  //
  craftingListWindow.prototype.numVisibleRows = function() {
    return this.maxItems();
  };

  craftingListWindow.prototype.makeCommandList = function() {
    this.addOriginalCommands();
  };

  craftingListWindow.prototype.addOriginalCommands = function() {
    const numberOfLearnedRecipes = $gameParty._craftingLearnedRecipes;
    for(let i = 0; i < numberOfLearnedRecipes.length; i++){
      const recipeKeyName = $gameParty._craftingLearnedRecipes[i];
      const enabled = $gameParty.canCombine(recipeKeyName);
      const recipeDetails = $gameParty._craftingRecipes[recipeKeyName];
      const itemId = recipeDetails[0];
      const nameAndSymbol = $dataItems[itemId].name;
      this.addCommand(nameAndSymbol, nameAndSymbol, enabled);
	  }
 };

  craftingListWindow.prototype.isCurrentItemEnabled = function() {
    const index = craftingDetailsWindow.prototype.currentItemIndex;
    const recipeKeyName = $gameParty._craftingLearnedRecipes[index];
    return $gameParty.canCombine(recipeKeyName);
  };

  //
  // process handler
  //
  craftingListWindow.prototype.processOk = function() {
    craftingListWindow._lastCommandSymbol = this.currentSymbol();
    Window_Command.prototype.processOk.call(this);
  };

  craftingListWindow.prototype.selectLast = function() {
    this.selectSymbol(craftingListWindow._lastCommandSymbol);
  };

  // ----------------------------------------------------------------------
  // craftingDetailsWindow
  // ----------------------------------------------------------------------

  function craftingDetailsWindow() {
    this.initialize.apply(this, arguments);
  };

  craftingDetailsWindow.prototype = Object.create(Window_Base.prototype);
  craftingDetailsWindow.prototype.constructor = craftingDetailsWindow;

  craftingDetailsWindow.prototype.currentItemIndex = 0;

  craftingDetailsWindow.prototype.initialize = function(rect) {
	  Window_Base.prototype.initialize.call(this, rect);
    this.update();
  };

  //
  // refresh window for each frame
  //
  craftingDetailsWindow.prototype.update = function() {
	  Window_Base.prototype.update.call(this);
    this.contents.clear();
    this.drawDetailsName(0, this.width);
    this.drawIngredient(this.fittingHeight(5));
    this.drawComboChance(0, this.fittingHeight(5) + 32 * 5 + 8);
    this.drawFailure();
  };

  //
  // draw details name
  //
  craftingDetailsWindow.prototype.drawDetailsName = function(y, width) {
    const itemId = getItemIdAndSetOpacity(this);
    drawItemNameAndDescription(this, itemId, y, width);
    drawBiggerIcon(this, itemId, y);
  };

  const getItemIdAndSetOpacity = _window => {
    const itemIndex = _window.currentItemIndex;
    const recipeKeyName = $gameParty._craftingLearnedRecipes[itemIndex];
    // Gray out the details page if we can't craft it
    _window.changePaintOpacity($gameParty.canCombine(recipeKeyName));
    const recipeDetails = $gameParty._craftingRecipes[recipeKeyName];
    return recipeDetails[0];
  };

  const drawItemNameAndDescription = (_window, itemId, y, width) => {
    const item = $dataItems[itemId];
    // Draw Item Name 
    _window.resetTextColor();
    _window.makeFontBigger();
    _window.drawText(item.name, 0, y, width, 'center');
    _window.makeFontSmaller();
    // Draw description
    const itemDesc1 = item.description.split('\n')[0] || '';
    const itemDesc2 = item.description.split('\n')[1] || '';
    _window.drawText(itemDesc1, 0, y + 106, width, 'center');
    _window.drawText(itemDesc2, 0, y + 132, width, 'center');
  };

  const drawBiggerIcon = (_window, itemId, y) => {
    const item = $dataItems[itemId];
    _window.bitmapIcon = new Sprite(ImageManager.loadSystem('IconSet'));
	  // Change the scale to twice as big
    const pw = ImageManager.iconWidth;
    const ph = ImageManager.iconHeight;
    _window.bitmapIcon.scale.x = 2;
    _window.bitmapIcon.scale.y = 2;
    _window.bitmapIcon.x = _window.width / 2 - pw;
    _window.bitmapIcon.y = y + ph * 2 - 12;
    const sx = item.iconIndex % 16 * pw;
    const sy = Math.floor(item.iconIndex / 16) * ph;
    _window.addChild(_window.bitmapIcon);
    _window.bitmapIcon.setFrame(sx, sy, pw, ph);
  };

  //
  // draw ingredient
  //
  craftingDetailsWindow.prototype.drawIngredient = function(y) {
    drawGuideText(this, y - 32);
    const ingredientArray = getIngredientArray();
    for(let i = 0; i < ingredientArray.length; i++) {
      const itemId = ingredientArray[i][0];
      const requiredAmount = ingredientArray[i][1];
      const ingredient = $dataItems[itemId];
      setTextColor(this, ingredient, requiredAmount);
      drawIconAndParams(this, i, ingredient, requiredAmount, y);
	  }
	  this.resetTextColor();
  };

  const drawGuideText = (_window, y) => {
    _window.changeTextColor(_window.systemColor());
    const text = requiredItemText;
    _window.drawText(text, 0, y, _window.width, "center");
    _window.resetTextColor();
  };

  const getIngredientArray = () => {
    const index = craftingDetailsWindow.prototype.currentItemIndex;
    const recipeKeyName = $gameParty._craftingLearnedRecipes[index];
    return $gameParty.ingredients(recipeKeyName);
  };

  const setTextColor = (_window, ingredient, requiredAmount) => {
    if ($gameParty.numItems(ingredient) >= requiredAmount) {
      _window.resetTextColor();
    } else {
      _window.changeTextColor(ColorManager.hpGaugeColor1());
    }
  };

  const drawIconAndParams = (_window, index, item, requireNum, Y) => {
    const ww = _window.width;
    const y = Y + 32 * index;
    // draw current number and required number
    const numItems = $gameParty.numItems(item);
    const text = numItems + '/' + requireNum;
    const _x = ww / 2 + _window.textWidth(': ') + 6;
    _window.drawText(text, _x, y, ww / 2, 'left');
    _window.drawIcon(item.iconIndex, ww / 2 - 32 , y);
    _window.drawText(item.name, 0 , y, ww / 2 - 38, 'right');
    _window.drawText(': ', ww / 2 + 6 , y, ww / 2, 'left');

  };

  //
  // draw chance
  //
  craftingDetailsWindow.prototype.drawComboChance = function(x, y) {
    this.resetTextColor();
    const index = this.currentItemIndex;
    const recipeKeyName = $gameParty._craftingLearnedRecipes[index];
    const recipeDetails = $gameParty._craftingRecipes[recipeKeyName];
    const chance = recipeDetails[1] * 100;
    if (chance > 75){
      this.changeTextColor(ColorManager.textColor(3));
    } else if (chance > 50) {
      this.changeTextColor(ColorManager.textColor(14));
    } else if (chance > 25) {
      this.changeTextColor(ColorManager.textColor(20));
    } else {
      this.changeTextColor(ColorManager.textColor(10));
	  }
    this.drawText(chance + ' %', x, y, this.width - 32 , 'center');
  };

  //
  // draw failure
  //
  const doesSuccessWhenever = () => {
    const index = craftingDetailsWindow.prototype.currentItemIndex;
    const recipeKeyName = $gameParty._craftingLearnedRecipes[index];
    const recipeDetails = $gameParty._craftingRecipes[recipeKeyName];
    return recipeDetails[1] >= 1;
  };

  const failureItem = () => {
    const index = craftingDetailsWindow.prototype.currentItemIndex;
    const recipeKeyName = $gameParty._craftingLearnedRecipes[index];
    const recipeDetails = $gameParty._craftingRecipes[recipeKeyName];
    return $dataItems[recipeDetails[2]];
  };

  craftingDetailsWindow.prototype.drawFailure = function() {
    const item = failureItem();
    if (failureText && item && !doesSuccessWhenever()) {
      const y = this.fittingHeight(5) + 32 * 6 + 6;
      this.changeTextColor(this.systemColor());
      this.drawText(failureText, 0, y, this.width, 'center');
      this.resetTextColor();
      this.drawText(item.name, 0, y + 32, this.width, 'center');
      const iconWidth = ImageManager.iconWidth;
      const iconX = (this.width - this.textWidth(item.name)) / 2 - iconWidth;
      this.drawIcon(item.iconIndex, iconX, y + 32);
    }
  };

  // ----------------------------------------------------------------------
  // craftingCombineCommand
  // ----------------------------------------------------------------------

  function craftingCombineCommand() {
    this.initialize.apply(this, arguments);
  }

  craftingCombineCommand.prototype = Object.create(Window_Command.prototype);
  craftingCombineCommand.prototype.constructor = craftingCombineCommand;

  craftingCombineCommand.prototype.canCombine = false;

  //
  // initialization
  //
  craftingCombineCommand.prototype.initialize = function(rect) {
    Window_Command.prototype.initialize.call(this, rect);
    this.selectLast();
    this.deselect();
    this.deactivate();
  };

  craftingCombineCommand.prototype.initCommandPosition = function() {
    this._lastCommandSymbol = null;
  };

  //
  // refresh for each frame
  //
  craftingCombineCommand.prototype.update = function() {
	  Window_Command.prototype.update.call(this);
	  this.drawItem(0);
  };

  //
  // setting and drawing commands
  //
  craftingCombineCommand.prototype.isCurrentItemEnabled = function() {
    const index = craftingDetailsWindow.prototype.currentItemIndex;
    const recipeKeyName = $gameParty._craftingLearnedRecipes[index];
    return $gameParty.canCombine(recipeKeyName);
  };

  craftingCombineCommand.prototype.drawItem = function(index){
    const currentIndex = craftingDetailsWindow.prototype.currentItemIndex;
    const recipeKeyName = $gameParty._craftingLearnedRecipes[currentIndex];
    this.changePaintOpacity($gameParty.canCombine(recipeKeyName));
    this.resetTextColor();
    const name = combineText;
    this.addCommand(name, "Combine", $gameParty.canCombine(recipeKeyName));
    const commandName = this.commandName(index);
    const rect = this.itemRect(index);
    this.drawText(commandName, rect.x , rect.y, rect.width, 'center');
    this.resetTextColor();
  };

  craftingCombineCommand.prototype.selectLast = function() {
    this.selectSymbol(craftingDetailsWindow._lastCommandSymbol);
  };

  //
  // process handlers
  //
  craftingCombineCommand.prototype.processOk = function() {
    craftingCombineCommand._lastCommandSymbol = this.currentSymbol();
    Window_Command.prototype.processOk.call(this);
  };

  //
  // window settings
  //
  craftingCombineCommand.prototype.maxItems = function() {
    return 1;
  };

  craftingCombineCommand.prototype.maxCols = function() {
    return 1;
  };

  craftingCombineCommand.prototype.standardPadding = function() {
    return 6;
  };

  // ----------------------------------------------------------------------
  // Scene_CraftingMenu
  // ----------------------------------------------------------------------

  function Scene_CraftingMenu() {
    this.initialize.apply(this, arguments);
  }

  Scene_CraftingMenu.prototype = Object.create(Scene_MenuBase.prototype);
  Scene_CraftingMenu.prototype.constructor = Scene_CraftingMenu;

  //
  // update for each frame
  //
  Scene_CraftingMenu.prototype.update = function() {
    Scene_MenuBase.prototype.update.call(this);
    this.updateDetails();
  };

  Scene_CraftingMenu.prototype.updateDetails = function(){
    if (this._commandItemWindow.active) {
      craftingDetailsWindow.prototype.currentItemIndex =
        this._commandItemWindow.index();
    }
  };

  //
  // create windows and set handlers
  //
  Scene_CraftingMenu.prototype.create = function() {
    Scene_MenuBase.prototype.create.call(this);
    this.createCommandWindow();
    this.createDetailsWindow();
    this.createCombineWindow();
  };

  Scene_CraftingMenu.prototype.createCommandWindow = function() {
    const rect = this.commandWindowRect();
    this._commandItemWindow = new craftingListWindow(rect);
    this._commandItemWindow.setHandler('ok', this.commandCombine.bind(this));
    this._commandItemWindow.setHandler('cancel',this.commandCancel.bind(this));
    this.addWindow(this._commandItemWindow);
  };

  Scene_CraftingMenu.prototype.commandWindowRect = function() {
    const wx = 0;
    const wy = this.mainAreaTop();
    const ww = 280;
    const wh = Graphics.boxHeight - wy;
    return new Rectangle(wx, wy, ww, wh);
  };

  Scene_CraftingMenu.prototype.commandCancel = function() {
    if ($gamePlayer._CombineMenuEnabled === true) {
      SceneManager.pop();
    } else {
      SceneManager.goto(Scene_Map);
    }
  };

  Scene_CraftingMenu.prototype.createDetailsWindow = function() {
    const rect = this.detailsWindowRect();
    this._commandDetailsWindow = new craftingDetailsWindow(rect);
    this.addWindow(this._commandDetailsWindow);
  };

  Scene_CraftingMenu.prototype.detailsWindowRect = function() {
    const wx = this._commandItemWindow.width;
    const wy = this._commandItemWindow.y;
    const ww = Graphics.boxWidth - wx;
    const wh = this._commandItemWindow.height;
    return new Rectangle(wx, wy, ww, wh);
  };

  Scene_CraftingMenu.prototype.createCombineWindow = function() {
    const rect = this.combineWindowRect();
    this._commandCombineWindow = new craftingCombineCommand(rect);
    this.addWindow(this._commandCombineWindow);
  };

  Scene_CraftingMenu.prototype.combineWindowRect = function() {
    const ww = 240;
    const x1 = this._commandItemWindow.width;
    const x2 = this._commandDetailsWindow.width / 2;
    const wx = x1 + x2 - ww / 2;
    const wh = this._commandItemWindow.fittingHeight(1);
    const wy = Graphics.boxHeight - wh - 12;
    return new Rectangle(wx, wy, ww, wh);
  };

  Scene_CraftingMenu.prototype.commandCombine = function() {
    this._commandCombineWindow.selectLast();
    this._commandItemWindow.deselect();
    this._commandCombineWindow.activate();
    this._commandCombineWindow.setHandler('ok', this.onDetailsOk.bind(this));
    this._commandCombineWindow.setHandler('cancel',
      this.onDetailsCancel.bind(this)
    );
  };

  Scene_CraftingMenu.prototype.onDetailsOk = function() {
    const index = craftingDetailsWindow.prototype.currentItemIndex;
    const recipeKeyName = $gameParty._craftingLearnedRecipes[index];
    const ingredientArray = $gameParty.ingredients(recipeKeyName);
    const recipeDetails = $gameParty._craftingRecipes[recipeKeyName];
    const itemId = recipeDetails[0];
    if ($gameParty.canCombine(recipeKeyName)) {
      for(let i = 0;i < ingredientArray.length; i++) {
        const requiredAmount = ingredientArray[i][1];
        const ingredient = $dataItems[ingredientArray[i][0]];
        $gameParty.loseItem(ingredient, requiredAmount);
		  }
      if (Math.random() < recipeDetails[1]) {
        $gameParty.gainItem($dataItems[itemId], 1);
		  } else {
        $gameParty.gainItem($dataItems[recipeDetails[2]], 1);
        SoundManager.playBuzzer();
      }
	  }
    SceneManager.goto(Scene_CraftingMenu);
  };

  Scene_CraftingMenu.prototype.onDetailsCancel = function() {
    this._commandItemWindow.selectLast();
    this._commandItemWindow.activate();
  };

})();
