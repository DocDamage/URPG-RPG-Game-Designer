/*:
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
 * plugin commands.
 *
 * For all the plugin commands explosed by this plugin, you can specify
 * - speed: Animation speed. Values 1 to 7 are the same options as for the Scroll Map command:
 *     - 1: 8x Slower
 *     - 2: 4x Slower
 *     - 3: 2x Slower
 *     - 4: Normal
 *     - 5: 2x Faster
 *     - 6: 4x Faster
 *     - 7: 8x Faster
 *     - 8: Custom (Specify a number of frames)
 * - duration: Number of frames for the animation (for custom speed)
 * - easing: Name of the easing function (they are all listed at the end)
 *
 *  * CameraMovement::MoveToFixed: Moves the camera to a fixed X/Y coordinate.
 *    - x: X Coordinate
 *    - y: Y Coordinate
 *    - speed
 *    - duration
 *    - easing
 *
 *  * CameraMovement::MoveToVariable: Moves the camera to coordinates stored in
 *  two variables.
 *    - xVar: Variable ID that has the X Coordinate
 *    - yVar: Variable ID that has the Y Coordinate
 *    - speed
 *    - duration
 *    - easing
 *
 *  * CameraMovement::MoveToEvent: Moves the camera so that the specified event
 *  is centered.
 *    - eventID: Target event ID
 *    - speed
 *    - duration
 *    - easing
 *
 *  * CameraMovement::Reset: Moves the camera back to the main character.
 *    - speed
 *    - duration
 *    - easing
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
 * @arg speed
 * @text Speed
 * @type select
 * @option 1: x8 Slower
 * @value 1
 * @option 2: x4 Slower
 * @value 2
 * @option 3: x2 Slower
 * @value 3
 * @option 4: Normal
 * @value 4
 * @option 5: x2 Faster
 * @value 5
 * @option 6: x4 Faster
 * @value 6
 * @option 7: x8 Faster
 * @value 7
 * @option Custom (Specify number of frames)
 * @value 8
 *
 * @arg duration
 * @text Duration in frames (for Custom Speed)
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
 * @arg speed
 * @text Speed
 * @type select
 * @option 1: x8 Slower
 * @value 1
 * @option 2: x4 Slower
 * @value 2
 * @option 3: x2 Slower
 * @value 3
 * @option 4: Normal
 * @value 4
 * @option 5: x2 Faster
 * @value 5
 * @option 6: x4 Faster
 * @value 6
 * @option 7: x8 Faster
 * @value 7
 * @option Custom (Specify number of frames)
 * @value 8
 *
 * @arg duration
 * @text Duration in frames (for Custom Speed)
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
 * @arg speed
 * @text Speed
 * @type select
 * @option 1: x8 Slower
 * @value 1
 * @option 2: x4 Slower
 * @value 2
 * @option 3: x2 Slower
 * @value 3
 * @option 4: Normal
 * @value 4
 * @option 5: x2 Faster
 * @value 5
 * @option 6: x4 Faster
 * @value 6
 * @option 7: x8 Faster
 * @value 7
 * @option Custom (Specify number of frames)
 * @value 8
 *
 * @arg duration
 * @text Duration in frames (for Custom Speed)
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
 * @arg speed
 * @text Speed
 * @type select
 * @option 1: x8 Slower
 * @value 1
 * @option 2: x4 Slower
 * @value 2
 * @option 3: x2 Slower
 * @value 3
 * @option 4: Normal
 * @value 4
 * @option 5: x2 Faster
 * @value 5
 * @option 6: x4 Faster
 * @value 6
 * @option 7: x8 Faster
 * @value 7
 * @option Custom (Specify number of frames)
 * @value 8
 *
 * @arg duration
 * @text Duration in frames (for Custom Speed)
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
 * @plugindesc イージング関数によるスムーズなカメラ移動プラグインです。
 * @author Mourad Kejji
 * @url https://github.com/Mouradif
 *
 * @help
 * ============================================================================
 *                      イージング付きスムーズカメラ移動
 * ============================================================================
 *
 * このプラグインを使うと、目標の座標・移動時間（フレーム数）・
 * イージング関数を指定して、カメラをスムーズに移動させることができます。
 * 以下のプラグインコマンドを使用してください。
 *
 * 本プラグインで使用できるすべてのコマンドには以下の共通パラメータがあります：
 * - speed: アニメーション速度。値1～7は「画面のスクロール」と同じ速度設定です。
 *     - 1: 8倍遅い
 *     - 2: 4倍遅い
 *     - 3: 2倍遅い
 *     - 4: 通常
 *     - 5: 2倍速い
 *     - 6: 4倍速い
 *     - 7: 8倍速い
 *     - 8: カスタム（フレーム数を直接指定）
 * - duration: カスタム速度時の移動時間（フレーム数）
 * - easing: 使用するイージング関数（一覧は下記参照）
 *
 *  * CameraMovement::MoveToFixed: 指定したX/Y座標へカメラを移動します。
 *    - x: X座標
 *    - y: Y座標
 *    - speed
 *    - duration
 *    - easing
 *
 *  * CameraMovement::MoveToVariable: 2つの変数に格納された座標へカメラを移動します。
 *    - xVar: X座標を保持する変数のID
 *    - yVar: Y座標を保持する変数のID
 *    - speed
 *    - duration
 *    - easing
 *
 *  * CameraMovement::MoveToEvent: 指定イベントが画面中央に来るようにカメラを移動します。
 *    - eventID: 対象イベントのID
 *    - speed
 *    - duration
 *    - easing
 *
 *  * CameraMovement::Reset: カメラをプレイヤーキャラクターに戻します。
 *    - speed
 *    - duration
 *    - easing
 *
 * 対応イージング関数一覧：
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
 * 各イージングの見た目や詳細は以下のサイトで確認できます：
 * https://easings.net/
 *
 * 特別な設定は必要ありません。
 *
 * @command MoveToFixed
 * @text 固定座標へ移動
 * @desc カメラを指定したX/Y座標（タイル単位）に移動します。
 *
 * @arg x
 * @text X座標
 * @type number
 * @desc 移動先のX座標（タイル単位）です。
 * @default 0
 *
 * @arg y
 * @text Y座標
 * @type number
 * @desc 移動先のY座標（タイル単位）です。
 * @default 0
 *
 * @arg speed
 * @text スピード
 * @type select
 * @option 1: 8倍遅い
 * @value 1
 * @option 2: 4倍遅い
 * @value 2
 * @option 3: 2倍遅い
 * @value 3
 * @option 4: 通常
 * @value 4
 * @option 5: 2倍速い
 * @value 5
 * @option 6: 4倍速い
 * @value 6
 * @option 7: 8倍速い
 * @value 7
 * @option カスタム（フレーム数を指定）
 * @value 8
 *
 * @arg duration
 * @text 移動時間（カスタム速度時）
 * @type number
 * @desc 移動にかかるフレーム数（例: 60 = 1秒）
 * @default 60
 *
 * @arg easing
 * @text イージング関数
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
 * @desc 使用するイージング関数を選択してください。
 * @default linear
 *
 * @command MoveToVariable
 * @text 変数座標へ移動
 * @desc 2つの変数に格納された座標へカメラを移動します。
 *
 * @arg xVar
 * @text X座標の変数
 * @type variable
 * @desc X座標を保持する変数IDです。
 * @default 1
 *
 * @arg yVar
 * @text Y座標の変数
 * @type variable
 * @desc Y座標を保持する変数IDです。
 * @default 2
 *
 * @arg speed
 * @text スピード
 * @type select
 * @option 1: 8倍遅い
 * @value 1
 * @option 2: 4倍遅い
 * @value 2
 * @option 3: 2倍遅い
 * @value 3
 * @option 4: 通常
 * @value 4
 * @option 5: 2倍速い
 * @value 5
 * @option 6: 4倍速い
 * @value 6
 * @option 7: 8倍速い
 * @value 7
 * @option カスタム（フレーム数を指定）
 * @value 8
 *
 * @arg duration
 * @text 移動時間（カスタム速度時）
 * @type number
 * @desc 移動にかかるフレーム数です。
 * @default 60
 *
 * @arg easing
 * @text イージング関数
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
 * @desc 使用するイージング関数を選択してください。
 * @default linear
 *
 * @command MoveToEvent
 * @text イベントに移動
 * @desc 指定したイベントを画面中央に表示するようにカメラを移動します。
 *
 * @arg eventId
 * @text イベントID
 * @type number
 * @desc 中心に移動するイベントのIDです。
 * @default 1
 *
 * @arg speed
 * @text スピード
 * @type select
 * @option 1: 8倍遅い
 * @value 1
 * @option 2: 4倍遅い
 * @value 2
 * @option 3: 2倍遅い
 * @value 3
 * @option 4: 通常
 * @value 4
 * @option 5: 2倍速い
 * @value 5
 * @option 6: 4倍速い
 * @value 6
 * @option 7: 8倍速い
 * @value 7
 * @option カスタム（フレーム数を指定）
 * @value 8
 *
 * @arg duration
 * @text 移動時間（カスタム速度時）
 * @type number
 * @desc 移動にかかるフレーム数です。
 * @default 60
 *
 * @arg easing
 * @text イージング関数
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
 * @desc 使用するイージング関数を選択してください。
 * @default linear
 *
 * @command Reset
 * @text カメラをリセット
 * @desc カメラをプレイヤーの中心に戻します。
 *
 * @arg speed
 * @text スピード
 * @type select
 * @option 1: 8倍遅い
 * @value 1
 * @option 2: 4倍遅い
 * @value 2
 * @option 3: 2倍遅い
 * @value 3
 * @option 4: 通常
 * @value 4
 * @option 5: 2倍速い
 * @value 5
 * @option 6: 4倍速い
 * @value 6
 * @option 7: 8倍速い
 * @value 7
 * @option カスタム（フレーム数を指定）
 * @value 8
 *
 * @arg duration
 * @text 移動時間（カスタム速度時）
 * @type number
 * @desc 移動にかかるフレーム数です。
 * @default 60
 *
 * @arg easing
 * @text イージング関数
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
 * @desc 使用するイージング関数を選択してください。
 * @default linear
 */


(function() {
  'use strict';

  const pluginName = 'CameraMovement';

  function speedToFrames(to, speed) {
    const tileSize = $gameMap.tileWidth();
    const [x, y] = [$gameMap._displayX, $gameMap._displayY];
    const from = {
      x: x + ((Graphics.width / tileSize) - 1) / 2,
      y: y + ((Graphics.height / tileSize) - 1) / 2
    };
    const linearDistance = Math.sqrt(
      Math.pow(from.x - to.x, 2) +
      Math.pow(from.y - to.y, 2)
    );
    const scrollDistancePerFrame = Math.pow(2, speed) / 256;
    const frames = Math.round(linearDistance / scrollDistancePerFrame);
    console.log({
      linearDistance,
      speed,
      scrollDistancePerFrame,
      frames,
    });
    return frames;
  }

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
    const speed = Number(args.speed);
    const duration = isNaN(speed) || speed === 8 ? Number(args.duration) : speedToFrames({x, y}, speed);
    const easing = String(args.easing);
    this.setWaitMode('advanced_scroll');
    cameraController.moveTo(x, y, duration, easing);
  }

  function moveToVariable(args) {
    const x = Number($gameVariables.value(args.xVar));
    const y = Number($gameVariables.value(args.yVar));
    const speed = Number(args.speed);
    const duration = isNaN(speed) || speed === 8 ? Number(args.duration) : speedToFrames({x, y}, speed);
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
    const speed = Number(args.speed);
    const duration = isNaN(speed) || speed === 8 ? Number(args.duration) : speedToFrames(event, speed);
    const easing = String(args.easing);
    this.setWaitMode('advanced_scroll');
    cameraController.moveTo(event.x, event.y, duration, easing);
  }

  function resetCamera(args) {
    const speed = Number(args.speed);
    const duration = isNaN(speed) || speed === 8 ? Number(args.duration) : speedToFrames($gamePlayer, speed);
    const easing = String(args.easing);
    this.setWaitMode('advanced_scroll');
    cameraController.moveTo(Number($gamePlayer.x), Number($gamePlayer.y), duration, easing);
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
            x: args[0],
            y: args[1],
            speed: args[2],
            duration: args[3],
            easing: args[4] || 'linear',
          });
          break;
        case [pluginName, 'MoveToVariable'].join('::'):
          moveToVariable.call(this, {
            xVar: args[0],
            yVar: args[1],
            speed: args[2],
            duration: args[3],
            easing: args[4] || 'linear',
          });
          break;
        case [pluginName, 'MoveToEvent'].join('::'):
          moveToEvent.call(this, {
            eventId: args[0],
            speed: args[1],
            duration: args[2],
            easing: args[3] || 'linear',
          });
          break;
        case [pluginName, 'Reset'].join('::'):
          resetCamera.call(this, {
            speed: args[0],
            duration: args[1],
            easing: args[2] || 'linear',
          });
          break;
        default:
          _Game_Interpreter_pluginCommand.call(this, command, args);
          break;
      }
    };
  }
})();