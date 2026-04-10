//=============================================================================
// RPG Maker MZ - Unique Data Loading Plugin
//=============================================================================
// Version
// 1.0.1 2024/03/30 日本語環境においてプロパティ名とJSONファイル名の説明が正しく日本語化されていなかった問題を修正
// 1.0.0 2020/08/20 初版
//=============================================================================

/*:
 * @target MZ
 * @plugindesc Unique Data Loading Plugin
 * @author triacontane
 * @base PluginCommonBase
 * @orderAfter PluginCommonBase
 * @url
 *
 * @param GlobalVariableName
 * @text Global Variable Name
 * @desc Global object name used  to defines unique data. Registered variables are generated under this variable.
 * @default $dataUniques
 * @type string
 *
 * @param UniqueDataList
 * @text Data List
 * @desc List of unique data. Place optional json files into the data folder.
 * @default []
 * @type struct<Data>[]
 *
 * @help UniqueDataLoader.js
 *
 * Optional json files existing in the data folder will be loaded.
 * Please create json files as text files that can be parsed as a JSON.
 * Defined files will be loaded on game launch.
 *
 * Data will be stored in the global object with the name specified. 
 * If a window is specified in the global variable name, each object
 * will be defined as its own global variable, but please look out for competing names.
 *
 * This can also be used to load data created with the MZ database converter,
 * and data added with proprietary plugins.
 *
 * Loaded data will be referred to as follows by proprietary plugins and scripts.
 *
 * Reference example where global variable is [$dataUniques] and property name is [property].
 * $dataUniques.property
 *
 * The method below will be called if all unique data is loaded correctly.
 * Please redefine if needed.
 * Scene_Boot.prototype.onUniqueDataLoad
 *
 */

/*~struct~Data:
 *
 * @param PropertyName
 * @text Property Name
 * @desc Property name where loaded data is stored.
 * @default property
 * @type string
 *
 * @param JsonFileName
 * @text JSON File Name
 * @desc File name of JSON to be loaded. Please enter a file from the data folder. File extension unnecessary. 
 * @default Tests
 * @type string
 */

 /*:ja
 * @target MZ
 * @plugindesc オリジナルのデータJSONファイルを読み込んで変数に格納します。
 * @author トリアコンタン
 * @base PluginCommonBase
 * @orderAfter PluginCommonBase
 * @url
 *
 * @param GlobalVariableName
 * @text グローバル変数名
 * @desc 固有データが定義されるグローバルオブジェクトの名称です。登録した変数はこの変数の配下に生成されます。
 * @default $dataUniques
 * @type string
 *
 * @param UniqueDataList
 * @text データリスト
 * @desc 固有データのリストです。dataフォルダ配下に任意のjsonファイルを配置してください。
 * @default []
 * @type struct<Data>[]
 *
 * @help UniqueDataLoader.js
 *
 * dataフォルダ配下に存在する任意のjsonファイルを読み込みます。
 * jsonファイルはJSONとしてparse可能なテキストファイルとして作成してください。
 * 定義したファイルはゲーム起動時に読み込まれます。
 *
 * データは指定した名称のグローバルオブジェクトに格納されます。
 * グローバル変数名に「window」を指定すると、各オブジェクトがそれぞれ
 * グローバル変数として定義されますが、名称の競合には注意してください。
 *
 * データベースコンバータMZで作成したデータや独自のプラグインで追加したデータ
 * の読み込みなどに使えます。
 *
 * 読み込んだデータは、独自のプラグインやスクリプトで以下の通り参照できます。
 *
 * グローバル変数を[$dataUniques]プロパティ名を[property]にした場合の参照例
 * $dataUniques.property
 *
 * すべての固有データを正常に読み込むと以下のメソッドが呼ばれます。
 * 必要であれば再定義してください。
 * Scene_Boot.prototype.onUniqueDataLoad
 *
 *
 */

/*~struct~Data:ja
 *
 * @param PropertyName
 * @text プロパティ名
 * @desc 読み込んだデータが格納されるプロパティ名です。
 * @default property
 * @type string
 *
 * @param JsonFileName
 * @text JSONファイル名
 * @desc 読み込み対象のJSONファイル名です。dataフォルダ配下のファイルを入力してください。拡張子不要。
 * @default Tests
 * @type string
 */

/*:zh
 * @target MZ
 * @plugindesc 读取并管理自定义 JSON 数据文件的插件
 * @author triacontane
 * @base PluginCommonBase
 * @orderAfter PluginCommonBase
 * @url
 *
 * @param GlobalVariableName
 * @text 全局变量名
 * @desc 定义存放自定义数据的全局对象名称。注册的数据会保存在该对象下。
 * @default $dataUniques
 * @type string
 *
 * @param UniqueDataList
 * @text 数据列表
 * @desc 要读取的自定义数据列表。请将 JSON 文件放置在 data 文件夹中。
 * @default []
 * @type struct<Data>[]
 *
 * @help UniqueDataLoader.js
 *
 * 本插件可自动加载位于 data 文件夹中的自定义 JSON 文件。
 * JSON 文件需为可被解析的标准 JSON 格式文本。
 * 这些文件会在游戏启动时被读取并加载。
 *
 * 加载后的数据会保存在指定名称的全局对象中。
 * 如果在“全局变量名”中指定了 `window`，则每个数据文件都会成为独立的全局变量。
 * 但请注意，变量名冲突可能导致意外行为。
 *
 * 该功能可用于：
 * - 加载通过 MZ 数据库转换工具生成的 JSON 数据；
 * - 加载由自定义插件生成的外部数据；
 * - 在游戏启动时初始化外部定义的数据文件。
 *
 * 【使用示例】
 * 若全局变量名为 `$dataUniques`，且属性名为 `property`，
 * 则可通过以下方式访问：
 * ```
 * $dataUniques.property
 * ```
 *
 * 当所有自定义数据加载完成后，会自动调用以下方法：
 * ```
 * Scene_Boot.prototype.onUniqueDataLoad
 * ```
 * 可在插件或脚本中重写该方法以添加自定义初始化逻辑。
 */

/*~struct~Data:zh
 *
 * @param PropertyName
 * @text 属性名
 * @desc 读取的 JSON 数据将在全局对象中以此名称存储。
 * @default property
 * @type string
 *
 * @param JsonFileName
 * @text JSON 文件名
 * @desc 要加载的 JSON 文件名，请输入位于 data 文件夹中的文件名（无需扩展名）。
 * @default Tests
 * @type string
 */

(() => {
    'use strict';
    const script = document.currentScript;
    const param  = PluginManagerEx.createParameter(script);

    let uniqueList;
    if (param.GlobalVariableName === 'window') {
        uniqueList = window;
    } else {
        window[param.GlobalVariableName] = {};
        uniqueList = window[param.GlobalVariableName];
    }

    const _Scene_Boot_create = Scene_Boot.prototype.create;
    Scene_Boot.prototype.create = function() {
        _Scene_Boot_create.apply(this, arguments);
        UniqueDataManager.loadDataList();
    };

    const _Scene_Boot_isReady = Scene_Boot.prototype.isReady;
    Scene_Boot.prototype.isReady = function() {
        if (!this._uniqueDataLoaded) {
            if (UniqueDataManager.isDataLoaded()) {
                this._uniqueDataLoaded = true;
                this.onUniqueDataLoad();
            } else {
                return false;
            }
        }
        return _Scene_Boot_isReady.apply(this, arguments);
    };

    Scene_Boot.prototype.onUniqueDataLoad = function() {};

    /**
     * UniqueDataManager
     * Unique data loading module.
     * @constructor
     */
    function UniqueDataManager() {
        throw new Error("This is a static class");
    }

    UniqueDataManager.loadDataList = function() {
        for (const dataFile of param.UniqueDataList) {
            this.loadDataFile(dataFile.PropertyName, dataFile.JsonFileName + '.json');
        }
    };

    UniqueDataManager.loadDataFile = function(name, src) {
        const xhr = new XMLHttpRequest();
        const url = "data/" + src;
        xhr.open("GET", url);
        xhr.overrideMimeType("application/json");
        xhr.onload = () => this.onXhrDataLoad(xhr, name, src, url);
        xhr.onerror = () => DataManager.onXhrError(name, src, url);
        xhr.send();
    };

    UniqueDataManager.onXhrDataLoad = function(xhr, name, src, url) {
        if (xhr.status < 400) {
            uniqueList[name] = JSON.parse(xhr.responseText);
        } else {
            DataManager.onXhrError(name, src, url);
        }
    };

    UniqueDataManager.isDataLoaded = function() {
        DataManager.checkError();
        return param.UniqueDataList.every(data => !!uniqueList[data.PropertyName]);
    };
})();
