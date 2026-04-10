/*:
 * @target MZ
 * @plugindesc Smooth Camera Movement with Easing Functions.
 * @author Mourad Kejji
 * @url https://github.com/Mouradif
 *
 * @help
 * ============================================================================
 *                      Smooth Camera Movement with Easing
 * ============================================================================
 *
 * This plugin allows you to move the camera smoothly by specifying a target
 * position along with a duration (in frames) and an easing function. Use the
 * plugin commands:
 *
 *  - CameraMovement::MoveToFixed: Moves the camera to a fixed X/Y coordinate.
 *    - x: X Coordinate
 *    - y: Y Coordinate
 *    - duration: Number of frames for the animation
 *    - easing: Name of the easing function (they are all listed at the end)
 *
 *  - CameraMovement::MoveToVariable: Moves the camera to coordinates stored in
 *  two variables.
 *    - xVar: Variable ID that has the X Coordinate
 *    - yVar: Variable ID that has the Y Coordinate
 *    - duration: Number of frames for the animation
 *    - easing: Name of the easing function (they are all listed at the end)
 *
 *  - CameraMovement::MoveToEvent: Moves the camera so that the specified event
 *  is centered.
 *    - eventID: Target event ID
 *    - duration: Number of frames for the animation
 *    - easing: Name of the easing function (they are all listed at the end)
 *
 *  - CameraMovement::Reset: Moves the camera back to the main character.
 *    - duration: Number of frames for the animation
 *    - easing: Name of the easing function (they are all listed at the end)
 *
 * Supported Easing Functions:
 * - linear
 * - easeInQuad
 * - easeOutQuad
 * - easeInOutQuad
 * - easeInCubic
 * - easeOutCubic
 * - easeInOutCubic
 * - easeInQuart
 * - easeOutQuart
 * - easeInOutQuart
 * - easeInQuint
 * - easeOutQuint
 * - easeInOutQuint
 * - easeInSine
 * - easeOutSine
 * - easeInOutSine
 * - easeInExpo
 * - easeOutExpo
 * - easeInOutExpo
 * - easeInCirc
 * - easeOutCirc
 * - easeInOutCirc
 * - easeInBack
 * - easeOutBack
 * - easeInOutBack
 * - easeInElastic
 * - easeOutElastic
 * - easeInOutElastic
 * - easeInBounce
 * - easeOutBounce
 * - easeInOutBounce
 *
 * For more info about easing functions and in order to get a preview of what
 * they look like, you can visit https://easings.net/
 *
 * No additional setup is required.
 *
 *
 * @command MoveToFixed
 * @text Move to Fixed Location
 * @desc Moves the camera to a fixed X/Y coordinate (in tiles).
 *
 * @arg x
 * @text X Coordinate
 * @type number
 * @desc The target X coordinate (in tiles).
 * @default 0
 *
 * @arg y
 * @text Y Coordinate
 * @type number
 * @desc The target Y coordinate (in tiles).
 * @default 0
 *
 * @arg duration
 * @text Duration (Frames)
 * @type number
 * @desc Duration of the move (in frames). (e.g. 60 frames = 1 second at 60fps)
 * @default 60
 *
 * @arg easing
 * @text Easing Function
 * @type select
 * @option linear
 * @option easeInQuad
 * @option easeOutQuad
 * @option easeInOutQuad
 * @option easeInCubic
 * @option easeOutCubic
 * @option easeInOutCubic
 * @option easeInQuart
 * @option easeOutQuart
 * @option easeInOutQuart
 * @option easeInQuint
 * @option easeOutQuint
 * @option easeInOutQuint
 * @option easeInSine
 * @option easeOutSine
 * @option easeInOutSine
 * @option easeInExpo
 * @option easeOutExpo
 * @option easeInOutExpo
 * @option easeInCirc
 * @option easeOutCirc
 * @option easeInOutCirc
 * @option easeInBack
 * @option easeOutBack
 * @option easeInOutBack
 * @option easeInElastic
 * @option easeOutElastic
 * @option easeInOutElastic
 * @option easeInBounce
 * @option easeOutBounce
 * @option easeInOutBounce
 * @desc Which easing function to use.
 * @default linear
 *
 *
 * @command MoveToVariable
 * @text Move to Variable Locations
 * @desc Moves the camera to coordinates stored in two variables.
 *
 * @arg xVar
 * @text X Coordinate Variable
 * @type variable
 * @desc The variable holding the target X coordinate.
 * @default 1
 *
 * @arg yVar
 * @text Y Coordinate Variable
 * @type variable
 * @desc The variable holding the target Y coordinate.
 * @default 2
 *
 * @arg duration
 * @text Duration (Frames)
 * @type number
 * @desc Duration of the move (in frames).
 * @default 60
 *
 * @arg easing
 * @text Easing Function
 * @type select
 * @option linear
 * @option easeInQuad
 * @option easeOutQuad
 * @option easeInOutQuad
 * @option easeInCubic
 * @option easeOutCubic
 * @option easeInOutCubic
 * @option easeInQuart
 * @option easeOutQuart
 * @option easeInOutQuart
 * @option easeInQuint
 * @option easeOutQuint
 * @option easeInOutQuint
 * @option easeInSine
 * @option easeOutSine
 * @option easeInOutSine
 * @option easeInExpo
 * @option easeOutExpo
 * @option easeInOutExpo
 * @option easeInCirc
 * @option easeOutCirc
 * @option easeInOutCirc
 * @option easeInBack
 * @option easeOutBack
 * @option easeInOutBack
 * @option easeInElastic
 * @option easeOutElastic
 * @option easeInOutElastic
 * @option easeInBounce
 * @option easeOutBounce
 * @option easeInOutBounce
 * @desc Which easing function to use.
 * @default linear
 *
 *
 * @command MoveToEvent
 * @text Move to Event
 * @desc Moves the camera to center on the specified event.
 *
 * @arg eventId
 * @text Event ID
 * @type number
 * @desc The event ID to center on.
 * @default 1
 *
 * @arg duration
 * @text Duration (Frames)
 * @type number
 * @desc Duration of the move (in frames).
 * @default 60
 *
 * @arg easing
 * @text Easing Function
 * @type select
 * @option linear
 * @option easeInQuad
 * @option easeOutQuad
 * @option easeInOutQuad
 * @option easeInCubic
 * @option easeOutCubic
 * @option easeInOutCubic
 * @option easeInQuart
 * @option easeOutQuart
 * @option easeInOutQuart
 * @option easeInQuint
 * @option easeOutQuint
 * @option easeInOutQuint
 * @option easeInSine
 * @option easeOutSine
 * @option easeInOutSine
 * @option easeInExpo
 * @option easeOutExpo
 * @option easeInOutExpo
 * @option easeInCirc
 * @option easeOutCirc
 * @option easeInOutCirc
 * @option easeInBack
 * @option easeOutBack
 * @option easeInOutBack
 * @option easeInElastic
 * @option easeOutElastic
 * @option easeInOutElastic
 * @option easeInBounce
 * @option easeOutBounce
 * @option easeInOutBounce
 * @desc Which easing function to use.
 * @default linear
 *
 *
 * @command Reset
 * @text Reset Camera
 * @desc Resets the camera to center on the main character.
 *
 * @arg duration
 * @text Duration (Frames)
 * @type number
 * @desc Duration of the move (in frames).
 * @default 60
 *
 * @arg easing
 * @text Easing Function
 * @type select
 * @option linear
 * @option easeInQuad
 * @option easeOutQuad
 * @option easeInOutQuad
 * @option easeInCubic
 * @option easeOutCubic
 * @option easeInOutCubic
 * @option easeInQuart
 * @option easeOutQuart
 * @option easeInOutQuart
 * @option easeInQuint
 * @option easeOutQuint
 * @option easeInOutQuint
 * @option easeInSine
 * @option easeOutSine
 * @option easeInOutSine
 * @option easeInExpo
 * @option easeOutExpo
 * @option easeInOutExpo
 * @option easeInCirc
 * @option easeOutCirc
 * @option easeInOutCirc
 * @option easeInBack
 * @option easeOutBack
 * @option easeInOutBack
 * @option easeInElastic
 * @option easeOutElastic
 * @option easeInOutElastic
 * @option easeInBounce
 * @option easeOutBounce
 * @option easeInOutBounce
 * @desc Which easing function to use.
 * @default linear
 */
/*:ja
 * @target MZ
 * @plugindesc イージング機能でスムーズにカメラを移動。
 * @author Mourad Kejji
 * @url https://github.com/Mouradif
 *
 * @help
 * ============================================================================
 *                      Smooth Camera Movement with Easing
 * ============================================================================
 *
 * このプラグインは、目標位置と継続時間（フレーム単位）とイージング機能を
 * 指定することで、カメラを滑らかに動かすことができます。
 * プラグインコマンドを使用します：
 *
 *  - CameraMovement::MoveToFixed: カメラを固定の X/Y 座標に移動します。
 *    - x: X 座標
 *    - y: Y 座標
 *    - duration: アニメーションのフレーム数
 *    - easing: イージング関数の名前（全て最後にリストアップされています）
 *
 *  - CameraMovement::MoveToVariable: 2つの変数に格納された座標にカメラを移動する。
 *    - xVar: X座標を持つ変数ID
 *    - yVar: Y座標を持つ変数ID
 *    - duration: アニメーションのフレーム数
 *    - easing: イージング関数の名前（全て最後にリストアップされています）
 *
 *  - CameraMovement::MoveToEvent: 指定されたイベントが中央に来るようにカメラを移動する。
 *    - eventID: 対象イベントID
 *    - duration: アニメーションのフレーム数
 *    - easing: イージング関数の名前（全て最後にリストアップされています）
 *
 *  - CameraMovement::Reset: カメラを主人公に戻します。
 *    - duration: アニメーションのフレーム数
 *    - easing: イージング関数の名前（全て最後にリストアップされています）
 *
 * サポートされているイージング機能:
 * - linear
 * - easeInQuad
 * - easeOutQuad
 * - easeInOutQuad
 * - easeInCubic
 * - easeOutCubic
 * - easeInOutCubic
 * - easeInQuart
 * - easeOutQuart
 * - easeInOutQuart
 * - easeInQuint
 * - easeOutQuint
 * - easeInOutQuint
 * - easeInSine
 * - easeOutSine
 * - easeInOutSine
 * - easeInExpo
 * - easeOutExpo
 * - easeInOutExpo
 * - easeInCirc
 * - easeOutCirc
 * - easeInOutCirc
 * - easeInBack
 * - easeOutBack
 * - easeInOutBack
 * - easeInElastic
 * - easeOutElastic
 * - easeInOutElastic
 * - easeInBounce
 * - easeOutBounce
 * - easeInOutBounce
 *
 * イージング機能についての詳細と、そのプレビューについては、
 * https://easings.net/ をご覧ください。
 *
 * 追加のセットアップは必要ありません。
 *
 *
 * @command MoveToFixed
 * @text 定位置へ移動
 * @desc カメラを固定X/Y座標（タイル単位）に移動する。
 *
 * @arg x
 * @text X 座標
 * @type number
 * @desc ターゲットのX座標（タイル単位）。
 * @default 0
 *
 * @arg y
 * @text Y 座標
 * @type number
 * @desc ターゲットのY座標（タイル単位）。
 * @default 0
 *
 * @arg duration
 * @text 持続時間（フレーム）
 * @type number
 * @desc 移動の持続時間（フレーム単位）。(例：60フレーム＝60fpsで1秒）
 * @default 60
 *
 * @arg easing
 * @text イージング機能
 * @type select
 * @option linear
 * @option easeInQuad
 * @option easeOutQuad
 * @option easeInOutQuad
 * @option easeInCubic
 * @option easeOutCubic
 * @option easeInOutCubic
 * @option easeInQuart
 * @option easeOutQuart
 * @option easeInOutQuart
 * @option easeInQuint
 * @option easeOutQuint
 * @option easeInOutQuint
 * @option easeInSine
 * @option easeOutSine
 * @option easeInOutSine
 * @option easeInExpo
 * @option easeOutExpo
 * @option easeInOutExpo
 * @option easeInCirc
 * @option easeOutCirc
 * @option easeInOutCirc
 * @option easeInBack
 * @option easeOutBack
 * @option easeInOutBack
 * @option easeInElastic
 * @option easeOutElastic
 * @option easeInOutElastic
 * @option easeInBounce
 * @option easeOutBounce
 * @option easeInOutBounce
 * @desc どのイージング関数を使用するか。
 * @default linear
 *
 *
 * @command MoveToVariable
 * @text 可変ロケーションへ移動
 * @desc 2つの変数に格納された座標にカメラを移動する。
 *
 * @arg xVar
 * @text X座標変数
 * @type variable
 * @desc ターゲットのX座標を保持する変数。
 * @default 1
 *
 * @arg yVar
 * @text Y 座標変数
 * @type variable
 * @desc ターゲットのY座標を保持する変数。
 * @default 2
 *
 * @arg duration
 * @text 持続時間（フレーム）
 * @type number
 * @desc 移動の持続時間（フレーム単位）。
 * @default 60
 *
 * @arg easing
 * @text イージング機能
 * @type select
 * @option linear
 * @option easeInQuad
 * @option easeOutQuad
 * @option easeInOutQuad
 * @option easeInCubic
 * @option easeOutCubic
 * @option easeInOutCubic
 * @option easeInQuart
 * @option easeOutQuart
 * @option easeInOutQuart
 * @option easeInQuint
 * @option easeOutQuint
 * @option easeInOutQuint
 * @option easeInSine
 * @option easeOutSine
 * @option easeInOutSine
 * @option easeInExpo
 * @option easeOutExpo
 * @option easeInOutExpo
 * @option easeInCirc
 * @option easeOutCirc
 * @option easeInOutCirc
 * @option easeInBack
 * @option easeOutBack
 * @option easeInOutBack
 * @option easeInElastic
 * @option easeOutElastic
 * @option easeInOutElastic
 * @option easeInBounce
 * @option easeOutBounce
 * @option easeInOutBounce
 * @desc どのイージング関数を使用するか。
 * @default linear
 *
 *
 * @command MoveToEvent
 * @text イベントへ移動
 * @desc 指定されたイベントを中心にカメラを移動します。
 *
 * @arg eventId
 * @text イベントID
 * @type number
 * @desc 中心となるイベントID。
 * @default 1
 *
 * @arg duration
 * @text 持続時間（フレーム）
 * @type number
 * @desc 移動の持続時間（フレーム単位）。
 * @default 60
 *
 * @arg easing
 * @text イージング機能
 * @type select
 * @option linear
 * @option easeInQuad
 * @option easeOutQuad
 * @option easeInOutQuad
 * @option easeInCubic
 * @option easeOutCubic
 * @option easeInOutCubic
 * @option easeInQuart
 * @option easeOutQuart
 * @option easeInOutQuart
 * @option easeInQuint
 * @option easeOutQuint
 * @option easeInOutQuint
 * @option easeInSine
 * @option easeOutSine
 * @option easeInOutSine
 * @option easeInExpo
 * @option easeOutExpo
 * @option easeInOutExpo
 * @option easeInCirc
 * @option easeOutCirc
 * @option easeInOutCirc
 * @option easeInBack
 * @option easeOutBack
 * @option easeInOutBack
 * @option easeInElastic
 * @option easeOutElastic
 * @option easeInOutElastic
 * @option easeInBounce
 * @option easeOutBounce
 * @option easeInOutBounce
 * @desc どのイージング関数を使用するか。
 * @default linear
 *
 *
 * @command Reset
 * @text カメラのリセット
 * @desc カメラを主人公の中心にリセットする。
 *
 * @arg duration
 * @text 持続時間（フレーム）
 * @type number
 * @desc 移動の持続時間（フレーム単位）。
 * @default 60
 *
 * @arg easing
 * @textイージング機能
 * @type select
 * @option linear
 * @option easeInQuad
 * @option easeOutQuad
 * @option easeInOutQuad
 * @option easeInCubic
 * @option easeOutCubic
 * @option easeInOutCubic
 * @option easeInQuart
 * @option easeOutQuart
 * @option easeInOutQuart
 * @option easeInQuint
 * @option easeOutQuint
 * @option easeInOutQuint
 * @option easeInSine
 * @option easeOutSine
 * @option easeInOutSine
 * @option easeInExpo
 * @option easeOutExpo
 * @option easeInOutExpo
 * @option easeInCirc
 * @option easeOutCirc
 * @option easeInOutCirc
 * @option easeInBack
 * @option easeOutBack
 * @option easeInOutBack
 * @option easeInElastic
 * @option easeOutElastic
 * @option easeInOutElastic
 * @option easeInBounce
 * @option easeOutBounce
 * @option easeInOutBounce
 * @desc どのイージング関数を使用するか。
 * @default linear
 */

(function() {
  'use strict';

  const pluginName = 'CameraMovement';

  //--------------------------------------------------------------------------
  // Easing Functions
  const EasingFunctions = {
    linear: t => t,
    easeInQuad: t => t * t,
    easeOutQuad: t => t * (2 - t),
    easeInOutQuad: t => t < 0.5 ? 2 * t * t : -1 + (4 - 2 * t) * t,
    easeInCubic: t => t * t * t,
    easeOutCubic: t => (--t) * t * t + 1,
    easeInOutCubic: t => t < 0.5 ? 4 * t * t * t : (t - 1) * (2 * t - 2) * (2 * t - 2) + 1,
    easeInQuart: t => t * t * t * t,
    easeOutQuart: t => 1 - (--t) * t * t * t,
    easeInOutQuart: t => t < 0.5 ? 8 * t * t * t * t : 1 - 8 * (--t) * t * t * t,
    easeInQuint: t => t * t * t * t * t,
    easeOutQuint: t => 1 + (--t) * t * t * t * t,
    easeInOutQuint: t => t < 0.5 ? 16 * t * t * t * t * t : 1 + 16 * (--t) * t * t * t * t,
    easeInSine: t => 1 - Math.cos(t * Math.PI / 2),
    easeOutSine: t => Math.sin(t * Math.PI / 2),
    easeInOutSine: t => -(Math.cos(Math.PI * t) - 1) / 2,
    easeInExpo: t => t === 0 ? 0 : Math.pow(2, 10 * (t - 1)),
    easeOutExpo: t => t === 1 ? 1 : 1 - Math.pow(2, -10 * t),
    easeInOutExpo: t => {
      if (t === 0) return 0;
      if (t === 1) return 1;
      if ((t /= 0.5) < 1) return 0.5 * Math.pow(2, 10 * (t - 1));
      return 0.5 * (-Math.pow(2, -10 * --t) + 2);
    },
    easeInCirc: t => 1 - Math.sqrt(1 - t * t),
    easeOutCirc: t => Math.sqrt(1 - (t - 1) * (t - 1)),
    easeInOutCirc: t => {
      if ((t /= 0.5) < 1) return -0.5 * (Math.sqrt(1 - t * t) - 1);
      return 0.5 * (Math.sqrt(1 - (t - 2) * (t - 2)) + 1);
    },
    easeInBack: t => {
      const s = 1.70158;
      return t * t * ((s + 1) * t - s);
    },
    easeOutBack: t => {
      const s = 1.70158;
      return --t * t * ((s + 1) * t + s) + 1;
    },
    easeInOutBack: t => {
      let s = 1.70158;
      if ((t /= 0.5) < 1) return 0.5 * (t * t * (((s *= 1.525) + 1) * t - s));
      return 0.5 * ((t -= 2) * t * (((s *= 1.525) + 1) * t + s) + 2);
    },
    easeInElastic: t => {
      if (t === 0) return 0;
      if (t === 1) return 1;
      const p = 0.3;
      const s = p / 4;
      return -(Math.pow(2, 10 * (t - 1)) * Math.sin((t - 1 - s) * (2 * Math.PI) / p));
    },
    easeOutElastic: t => {
      if (t === 0) return 0;
      if (t === 1) return 1;
      const p = 0.3;
      const s = p / 4;
      return Math.pow(2, -10 * t) * Math.sin((t - s) * (2 * Math.PI) / p) + 1;
    },
    easeInOutElastic: t => {
      if (t === 0) return 0;
      if (t === 1) return 1;
      const p = 0.45;
      const s = p / 4;
      if ((t /= 0.5) < 1) return -0.5 * (Math.pow(2, 10 * (t - 1)) * Math.sin((t - 1 - s) * (2 * Math.PI) / p));
      return Math.pow(2, -10 * (t - 1)) * Math.sin((t - 1 - s) * (2 * Math.PI) / p) * 0.5 + 1;
    },
    easeInBounce: t => 1 - EasingFunctions.easeOutBounce(1 - t),
    easeOutBounce: t => {
      if (t < (1 / 2.75)) {
        return 7.5625 * t * t;
      } else if (t < (2 / 2.75)) {
        return 7.5625 * (t -= 1.5 / 2.75) * t + 0.75;
      } else if (t < (2.5 / 2.75)) {
        return 7.5625 * (t -= 2.25 / 2.75) * t + 0.9375;
      } else {
        return 7.5625 * (t -= 2.625 / 2.75) * t + 0.984375;
      }
    },
    easeInOutBounce: t => {
      if (t < 0.5) return EasingFunctions.easeInBounce(t * 2) * 0.5;
      return EasingFunctions.easeOutBounce(t * 2 - 1) * 0.5 + 0.5;
    }
  };

  //--------------------------------------------------------------------------
  // CameraController Class
  class CameraController {
    constructor() {
      this.currentX = null;
      this.currentY = null;
      this.startX = 0;
      this.startY = 0;
      this.targetX = 0;
      this.targetY = 0;
      this.duration = 0;
      this.elapsed = 0;
      this.easingFunction = EasingFunctions.linear;
      this.moving = false;
    }

    initialize() {
      if ($gameMap) {
        this.currentX = Math.round($gameMap._displayX + ((Graphics.boxWidth / $gameMap.tileWidth()) / 2));
        this.currentY = Math.round($gameMap._displayY + ((Graphics.boxHeight / $gameMap.tileHeight()) / 2));
        this.startX = this.currentX;
        this.startY = this.currentY;
      }
    }

    /**
     * Initiates a camera move.
     * @param {number} x - The target center X (in tiles).
     * @param {number} y - The target center Y (in tiles).
     * @param {number} duration - Duration of the move in frames.
     * @param {string} easingName - Name of the easing function.
     */
    moveTo(x, y, duration, easingName) {
      this.initialize();
      this.targetX = x;
      this.targetY = y;
      this.duration = Math.max(1, duration);
      this.elapsed = 0;
      this.easingFunction = EasingFunctions[easingName] || EasingFunctions.linear;
      this.moving = true;
    }

    update() {
      if (!this.moving) {
        return;
      }
      this.elapsed++;
      let t = this.elapsed / this.duration;
      if (t > 1) t = 1;
      const easeT = this.easingFunction(t);
      this.currentX = this.startX + (this.targetX - this.startX) * easeT;
      this.currentY = this.startY + (this.targetY - this.startY) * easeT;

      if (this.elapsed >= this.duration) {
        this.currentX = this.targetX;
        this.currentY = this.targetY;
        this.moving = false;
      }
      this.updateDisplay();
    }

    updateDisplay() {
      if (!$gameMap) return;
      const tileWidth = $gameMap.tileWidth();
      const tileHeight = $gameMap.tileHeight();

      const screenTilesX = Graphics.boxWidth / tileWidth;
      const screenTilesY = Graphics.boxHeight / tileHeight;

      const displayX = this.currentX - screenTilesX / 2;
      const displayY = this.currentY - screenTilesY / 2;
      $gameMap.setDisplayPos(displayX, displayY);
    }
  }

  const cameraController = new CameraController();

  //--------------------------------------------------------------------------
  // Hook into Scene_Map update so that the camera controller updates every frame.
  const _Scene_Map_update = Scene_Map.prototype.update;
  Scene_Map.prototype.update = function() {
    _Scene_Map_update.call(this);
    cameraController.update();
  };

  //--------------------------------------------------------------------------
  // Waiting for camera movement to finish
  const Game_Interpreter_updateWaitMode = Game_Interpreter.prototype.updateWaitMode;
  Game_Interpreter.prototype.updateWaitMode = function() {
    if (this._waitMode !== 'advanced_scroll') {
      return Game_Interpreter_updateWaitMode.call(this);
    }
    if (!cameraController.moving) {
      this._waitMode = "";
    }
    return cameraController.moving;
  }


  //--------------------------------------------------------------------------
  // Functions for each plugin command
  function moveToFixed(args) {
    const x = Number(args.x);
    const y = Number(args.y);
    const duration = Number(args.duration);
    const easing = String(args.easing);
    this.setWaitMode('advanced_scroll');
    cameraController.moveTo(x, y, duration, easing);
  }

  function moveToVariable(args) {
    const x = Number($gameVariables.value(args.xVar));
    const y = Number($gameVariables.value(args.yVar));
    const duration = Number(args.duration);
    const easing = String(args.easing);
    this.setWaitMode('advanced_scroll');
    cameraController.moveTo(x, y, duration, easing);
  }

  function moveToEvent(args) {
    const eventId = Number(args.eventId);
    const event = $gameMap.event(eventId);
    if (!event) {
      return;
    }
    const duration = Number(args.duration);
    const easing = String(args.easing);
    this.setWaitMode('advanced_scroll');
    cameraController.moveTo(event.x, event.y, duration, easing);
  }

  function resetCamera(args) {
    const duration = Number(args.duration);
    const easing = String(args.easing);
    this.setWaitMode('advanced_scroll');
    cameraController.moveTo($gamePlayer.x, $gamePlayer.y, duration, easing);
  }

  //--------------------------------------------------------------------------
  // Plugin Command Registrations
  if (typeof PluginManager.registerCommand === 'function') {
    PluginManager.registerCommand(pluginName, 'MoveToFixed', moveToFixed);
    PluginManager.registerCommand(pluginName, 'MoveToVariable', moveToVariable);
    PluginManager.registerCommand(pluginName, 'MoveToEvent', moveToEvent);
    PluginManager.registerCommand(pluginName, 'Reset', resetCamera);
  } else {
    const _Game_Interpreter_pluginCommand = Game_Interpreter.prototype.pluginCommand;
    Game_Interpreter.prototype.pluginCommand = function(command, args) {
      switch (command) {
        case [pluginName, 'MoveToFixed'].join('::'):
          moveToFixed.call(this, {
            x: Number(args[0]),
            y: Number(args[1]),
            duration: Number(args[2]),
            easing: args[3] || 'linear',
          });
          break;
        case [pluginName, 'MoveToVariable'].join('::'):
          moveToVariable.call(this, {
            xVar: Number(args[0]),
            yVar: Number(args[1]),
            duration: Number(args[2]),
            easing: args[3] || 'linear',
          });
          break;
        case [pluginName, 'MoveToEvent'].join('::'):
          moveToEvent.call(this, {
            eventId: Number(args[0]),
            duration: Number(args[1]),
            easing: args[2] || 'linear',
          });
          break;
        case [pluginName, 'Reset'].join('::'):
          resetCamera.call(this, {
            duration: Number(args[0]),
            easing: args[1] || 'linear',
          });
          break;
        default:
          _Game_Interpreter_pluginCommand.call(this, command, args);
          break;
      }
    };
  }
})();