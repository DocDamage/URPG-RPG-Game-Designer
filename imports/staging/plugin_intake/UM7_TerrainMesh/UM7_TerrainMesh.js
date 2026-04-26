/*:
 * @target MZ
 * @plugindesc  The Continuous Mesh (Stacking & Airship Fixes)
 * @author Paradajz 
 * @base UltraMode7
 * @orderAfter UM7_CurvatureAddon
 * @orderAfter UM7_DualOcean
 *
 * @param --- Mesh Heights ---
 * * @param Forest Height
 * @desc Vertical Z-axis pop for trees (Tag 1).
 * @type number
 * @default 12
 *
 * @param Elevated Height
 * @desc Base vertical lift for raised ground / plateaus (Tag 4).
 * @type number
 * @default 8
 *
 * @param Bridge Height
 * @desc Vertical Z-axis pop for bridges (Tag 5).
 * @type number
 * @default 16
 *
 * @param Hill Height
 * @desc Vertical Z-axis pop for ground hills (Tag 3).
 * @type number
 * @default 16
 *
 * @param Mountain Height
 * @desc Vertical Z-axis pop for mountains (Tag 2).
 * @type number
 * @default 32
 * * @param --- Player Collision ---
 * * @param Walkable Region
 * @desc Region ID that forces passability for the player (0 to disable).
 * @type number
 * @default 0
 * * @param Blocked Region
 * @desc Region ID that forces impassability for the player (0 to disable).
 * @type number
 * @default 0
 * * @param --- Visuals ---
 * * @param Enable Vignette
 * @desc Automatically draws a cinematic dark shadow around the screen edges.
 * @type boolean
 * @default true
 * * @param Vignette Intensity
 * @desc How dark the edge shadows are (0 to 255).
 * @type number
 * @min 0
 * @max 255
 * @default 200
 */

(() => {
    const scriptName = document.currentScript ? document.currentScript.src.split('/').pop().replace('.js', '') : "UM7_TerrainMesh";
    const params = PluginManager.parameters(scriptName) || PluginManager.parameters("UM7_TerrainMesh");
    
   
    const hForest = Number(params["Forest Height"] || 12);
    const hElevated = Number(params["Elevated Height"] || 8);
    const hBridge = Number(params["Bridge Height"] || 16);
    const hHill = Number(params["Hill Height"] || 16);
    const hMnt = Number(params["Mountain Height"] || 32);

    // Collision Parameters
    const walkableRegion = Number(params["Walkable Region"] || 0);
    const blockedRegion = Number(params["Blocked Region"] || 0);

    // Visual Parameters
    const enableVignette = String(params["Enable Vignette"]) === "true";
    const vignetteIntensity = Number(params["Vignette Intensity"] || 200);

    
    function isOverworldMap() {
        return typeof $dataMap !== 'undefined' && $dataMap && $dataMap.meta && $dataMap.meta.overworld;
    }

    function getVertexZ(px, py, isVisual) {
        if (!isOverworldMap()) return 0;
        const tw = $gameMap.tileWidth();
        const th = $gameMap.tileHeight();
        
        const tx1 = Math.floor((px - 1) / tw), ty1 = Math.floor((py - 1) / th);
        const tx2 = Math.floor((px + 1) / tw), ty2 = Math.floor((py - 1) / th);
        const tx3 = Math.floor((px - 1) / tw), ty3 = Math.floor((py + 1) / th);
        const tx4 = Math.floor((px + 1) / tw), ty4 = Math.floor((py + 1) / th);

        const z1 = getStackedZ(tx1, ty1, isVisual);
        const z2 = getStackedZ(tx2, ty2, isVisual);
        const z3 = getStackedZ(tx3, ty3, isVisual);
        const z4 = getStackedZ(tx4, ty4, isVisual);

        return Math.min(z1, z2, z3, z4);
    }

    function getStackedZ(tx, ty, isVisual) {
        if (!$gameMap) return 0;
        
        if ($gameMap.isLoopHorizontal()) tx = (tx % $gameMap.width() + $gameMap.width()) % $gameMap.width();
        else tx = Math.max(0, Math.min(tx, $gameMap.width() - 1));
        
        if ($gameMap.isLoopVertical()) ty = (ty % $gameMap.height() + $gameMap.height()) % $gameMap.height();
        else ty = Math.max(0, Math.min(ty, $gameMap.height() - 1));

        const flags = $gameMap.tilesetFlags();
        const tiles = $gameMap.layeredTiles(tx, ty);
        
        let groundZ = 0;
        
        
        for (const tile of tiles) {
            if (tile > 0 && flags[tile]) {
                const tag = flags[tile] >> 12;
                if (tag === 4) groundZ += hElevated;
                if (tag === 3) groundZ += hHill;
                if (tag === 2) groundZ += hMnt;
                if (tag === 5) groundZ += hBridge; 
                
                if (tag === 1 && isVisual) groundZ += hForest;
            }
        }
        
        const region = $gameMap.regionId(tx, ty);
        let regionZ = 0;
        if (isOverworldMap() && $dataMap.meta.hillregion) {
            const hillReg = Number($dataMap.meta.hillregion);
            if (hillReg > 0 && region === hillReg) regionZ = hHill;
        }
        if (region > 0 && $dataMap.meta['RegionHeight' + region]) {
            regionZ = Number($dataMap.meta['RegionHeight' + region]);
        }
        
        if (regionZ > groundZ) groundZ = regionZ;
        
        return groundZ;
    }

    window.$getUM7PixelHeight = function(px, py, isVisual) {
        if (!isOverworldMap()) return 0;

        const qx1 = Math.floor(px / 24) * 24;
        const qy1 = Math.floor(py / 24) * 24;
        const qx2 = qx1 + 24;
        const qy2 = qy1 + 24;

        const h11 = getVertexZ(qx1, qy1, isVisual);
        const h21 = getVertexZ(qx2, qy1, isVisual);
        const h12 = getVertexZ(qx1, qy2, isVisual);
        const h22 = getVertexZ(qx2, qy2, isVisual);

        const fx = (px - qx1) / 24;
        const fy = (py - qy1) / 24;

        const hTop = h11 * (1 - fx) + h21 * fx;
        const hBot = h12 * (1 - fx) + h22 * fx;
        
        return hTop * (1 - fy) + hBot * fy; 
    };

    
    if (typeof Tilemap !== 'undefined') {
        if (Tilemap.ULTRA_MODE_7_VERTEX_SHADER) {
            let vSrc = Tilemap.ULTRA_MODE_7_VERTEX_SHADER;
            vSrc = vSrc.replace(
                'attribute vec2 aAnimation;', 
                'attribute vec2 aAnimation;\n\tattribute float aHeight;'
            );
            vSrc = vSrc.replace(
                /vec4\s*\(\s*aVertexPosition\s*,\s*0\.0\s*,\s*1\.0\s*\)/g, 
                'vec4(aVertexPosition, aHeight, 1.0)' 
            );
            Tilemap.ULTRA_MODE_7_VERTEX_SHADER = vSrc;
        }

        window._um7_mx = 0; window._um7_my = 0;
        window._um7_baseDx = 0; window._um7_baseDy = 0;
        window._um7_currentLayer = 0;

        const _Tilemap_readMapData = Tilemap.prototype._readMapData;
        Tilemap.prototype._readMapData = function(x, y, z) {
            window._um7_currentLayer = z;
            return _Tilemap_readMapData.call(this, x, y, z);
        };

        const _Tilemap_paintTiles = Tilemap.prototype._paintTiles;
        Tilemap.prototype._paintTiles = function(startX, startY, x, y) {
            window._um7_mx = startX + x;
            window._um7_my = startY + y;
            const lw = this._layerWidth || 2048;
            const lh = this._layerHeight || 2048;
            let bdx = (window._um7_mx * this._tileWidth) % lw;
            let bdy = (window._um7_my * this._tileHeight) % lh;
            if (bdx < 0) bdx += lw;
            if (bdy < 0) bdy += lh;
            window._um7_baseDx = bdx;
            window._um7_baseDy = bdy;
            _Tilemap_paintTiles.apply(this, arguments);
        };

        Tilemap.Layer.ULTRA_MODE_7_VERTEX_STRIDE = Tilemap.Layer.VERTEX_STRIDE + 3 * 4;

        const _UM7_createVao = Tilemap.Layer.prototype._createVao;
        Tilemap.Layer.prototype._createVao = function() {
            if (!UltraMode7.isActive()) { _UM7_createVao.call(this); return; }
            const type = PIXI.TYPES.FLOAT;
            const stride = Tilemap.Layer.ULTRA_MODE_7_VERTEX_STRIDE;
            const size = 4;
            const indexBuffer = new PIXI.Buffer(null, true, true);
            const vertexBuffer = new PIXI.Buffer(null, true, false);
            const geometry = new PIXI.Geometry();
            const vao = geometry
                .addIndex(indexBuffer)
                .addAttribute("aTextureId", vertexBuffer, 1, false, type, stride, 0)
                .addAttribute("aFrame", vertexBuffer, 4, false, type, stride, 1 * size)
                .addAttribute("aTextureCoord", vertexBuffer, 2, false, type, stride, 5 * size)
                .addAttribute("aVertexPosition", vertexBuffer, 2, false, type, stride, 7 * size)
                .addAttribute("aAnimation", vertexBuffer, 2, false, type, stride, 9 * size)
                .addAttribute("aHeight", vertexBuffer, 1, false, type, stride, 11 * size); 
            
            this._allIndexBuffers.push(indexBuffer);
            this._allIndexArrays.push(new Float32Array(0));
            this._allVertexBuffers.push(vertexBuffer);
            this._allVertexArrays.push(new Float32Array(0));
            this._allVaos.push(vao);
        };

        const _Tilemap_Layer_addRect = Tilemap.Layer.prototype.addRect;
        Tilemap.Layer.prototype.addRect = function(setNumber, sx, sy, dx, dy, w, h, ax, ay) {
            if (!UltraMode7.isActive()) {
                _Tilemap_Layer_addRect.call(this, setNumber, sx, sy, dx, dy, w, h, ax, ay);
                return;
            }

            const pushQuad = (orig_sx, orig_sy, orig_w, orig_h, q_sx, q_sy, q_w, q_h, x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4) => {
                if (this._allElements.length === 0 || this._allElements[this._allElements.length - 1].length >= 16300) {
                    this._allElements.push([]);
                }
                this._allElements[this._allElements.length - 1].push([
                    setNumber, orig_sx, orig_sy, orig_w, orig_h, q_sx, q_sy, q_w, q_h, ax || 0, ay || 0,
                    x1, y1, z1,  x2, y2, z2,  x3, y3, z3,  x4, y4, z4
                ]);
            };

            const tw = $gameMap.tileWidth();
            const th = $gameMap.tileHeight();
            const offsetX = dx - window._um7_baseDx;
            const offsetY = dy - window._um7_baseDy;
            const absBaseX = (window._um7_mx * tw) + offsetX;
            const absBaseY = (window._um7_my * th) + offsetY;
            const layerZOffset = window._um7_currentLayer * 0.5;

            if (isOverworldMap()) { 
                const stepX = 24; 
                const stepY = 24;
                
                for (let qy = dy; qy < dy + h; qy += stepY) {
                    for (let qx = dx; qx < dx + w; qx += stepX) {
                        const subW = Math.min(stepX, dx + w - qx);
                        const subH = Math.min(stepY, dy + h - qy);
                        const subSx = sx + (qx - dx);
                        const subSy = sy + (qy - dy);
                        const absQx = absBaseX + (qx - dx);
                        const absQy = absBaseY + (qy - dy);

                        // True forces it to be a Visual Mesh (Includes Trees)
                        const zTL = getVertexZ(absQx, absQy, true);
                        const zTR = getVertexZ(absQx + subW, absQy, true);
                        const zBR = getVertexZ(absQx + subW, absQy + subH, true);
                        const zBL = getVertexZ(absQx, absQy + subH, true);

                        pushQuad(
                            sx, sy, w, h,               
                            subSx, subSy, subW, subH,   
                            qx, qy, zTL + layerZOffset,
                            qx + subW, qy, zTR + layerZOffset,
                            qx + subW, qy + subH, zBR + layerZOffset,
                            qx, qy + subH, zBL + layerZOffset
                        );
                    }
                }
            } else { 
                pushQuad(sx, sy, w, h, sx, sy, w, h, dx, dy, layerZOffset, dx+w, dy, layerZOffset, dx+w, dy+h, layerZOffset, dx, dy+h, layerZOffset);
            }
        };

        const _UM7_updateVertexBuffer = Tilemap.Layer.prototype._updateVertexBuffer;
        Tilemap.Layer.prototype._updateVertexBuffer = function() {
            if (!UltraMode7.isActive()) { _UM7_updateVertexBuffer.call(this); return; }
            
            const numElements = this._elements.length;
            const required = numElements * Tilemap.Layer.ULTRA_MODE_7_VERTEX_STRIDE;
            if (this._vertexArray.length < required) this._vertexArray = new Float32Array(required * 2);
            
            let index = 0;
            const data = this._vertexArray;
            const uvEps = 0.50; 
            
            for (let i = 0; i < numElements; ++i) {
                const item = this._elements[i];
                const tid = (item[0] >> 2);
                const sxOffset = 1024 * (item[0] & 1);
                const syOffset = 1024 * ((item[0] >> 1) & 1);
                const orig_sx = item[1] + sxOffset;
                const orig_sy = item[2] + syOffset;
                const orig_w = item[3];
                const orig_h = item[4];
                
                const fL = orig_sx + uvEps;
                const fT = orig_sy + uvEps;
                const fR = orig_sx + orig_w - uvEps;
                const fB = orig_sy + orig_h - uvEps;
                
                const uL = item[5] + sxOffset + uvEps;
                const uT = item[6] + syOffset + uvEps;
                const uR = item[5] + sxOffset + item[7] - uvEps;
                const uB = item[6] + syOffset + item[8] - uvEps;
                
                const ax = item[9];
                const ay = item[10];

                data[index++] = tid;           
                data[index++] = fL; data[index++] = fT; data[index++] = fR; data[index++] = fB; 
                data[index++] = uL; data[index++] = uT; 
                data[index++] = item[11]; data[index++] = item[12]; 
                data[index++] = ax; data[index++] = ay; 
                data[index++] = item[13];      
                
                data[index++] = tid;
                data[index++] = fL; data[index++] = fT; data[index++] = fR; data[index++] = fB;
                data[index++] = uR; data[index++] = uT; 
                data[index++] = item[14]; data[index++] = item[15]; 
                data[index++] = ax; data[index++] = ay;
                data[index++] = item[16]; 
                
                data[index++] = tid;
                data[index++] = fL; data[index++] = fT; data[index++] = fR; data[index++] = fB;
                data[index++] = uR; data[index++] = uB; 
                data[index++] = item[17]; data[index++] = item[18]; 
                data[index++] = ax; data[index++] = ay;
                data[index++] = item[19]; 
                
                data[index++] = tid;
                data[index++] = fL; data[index++] = fT; data[index++] = fR; data[index++] = fB;
                data[index++] = uL; data[index++] = uB; 
                data[index++] = item[20]; data[index++] = item[21]; 
                data[index++] = ax; data[index++] = ay;
                data[index++] = item[22]; 
            }
            this._vertexBuffer.update(data);
        };
    }

   
    if (enableVignette) {
        const _Spriteset_Map_createUpperLayer = Spriteset_Map.prototype.createUpperLayer;
        Spriteset_Map.prototype.createUpperLayer = function() {
            _Spriteset_Map_createUpperLayer.call(this);
            if (isOverworldMap()) this.createVignetteOverlay();
        };

        Spriteset_Map.prototype.createVignetteOverlay = function() {
            const w = Graphics.width;
            const h = Graphics.height;
            const canvas = document.createElement('canvas');
            canvas.width = w;
            canvas.height = h;
            const ctx = canvas.getContext('2d');
            
            const gradient = ctx.createRadialGradient(
                w / 2, h / 2, h / 3, 
                w / 2, h / 2, h / 1.1 
            );
            
            const alpha = vignetteIntensity / 255;
            gradient.addColorStop(0, 'rgba(0, 0, 0, 0)');
            gradient.addColorStop(1, `rgba(0, 0, 0, ${alpha})`);
            
            ctx.fillStyle = gradient;
            ctx.fillRect(0, 0, w, h);
            
            const texture = PIXI.Texture.from(canvas);
            this._vignetteSprite = new PIXI.Sprite(texture);
            this._vignetteSprite.z = 10; 
            this.addChild(this._vignetteSprite);
        };
    }

  
    
    
    const _Sprite_Character_updatePosition = Sprite_Character.prototype.updatePosition;
    Sprite_Character.prototype.updatePosition = function() {
        let isFlying = false;
        if (this._character) {
            if (this._character === $gamePlayer && $gamePlayer.isInAirship()) isFlying = true;
            if (this._character instanceof Game_Vehicle && this._character.isAirship()) isFlying = true;
        }
        
        window._um7_disableElevation = isFlying;
        _Sprite_Character_updatePosition.call(this);
        window._um7_disableElevation = false;
    };

    const _UM7_mapToScreen = UltraMode7.mapToScreen;
    UltraMode7.mapToScreen = function(x, y, z) {
        if (!isOverworldMap()) return _UM7_mapToScreen.call(this, x, y, z);

        const tw = $gameMap.tileWidth();
        const th = $gameMap.tileHeight();
        const absX = x + $gameMap.displayX() * tw;
        const absY = y + $gameMap.displayY() * th;
        
        let meshElevation = 0;
        if (!window._um7_disableElevation) {
            
            meshElevation = window.$getUM7PixelHeight(absX, absY, false);
        }

        return _UM7_mapToScreen.call(this, x, y, (z || 0) + meshElevation);
    };

    // ------------------------------------------------------------------------
    // 4. FOREST TRANSPARENCY
    // ------------------------------------------------------------------------
    const _Sprite_Character_update = Sprite_Character.prototype.update;
    Sprite_Character.prototype.update = function() {
        _Sprite_Character_update.call(this);
        
        if (!$gameMap || !UltraMode7.isActive() || !this._character) return;
        
        if (this._character instanceof Game_Vehicle) return;

        const rx = this._character._realX;
        const ry = this._character._realY;
        const tag = $gameMap.terrainTag(Math.floor(rx), Math.floor(ry));
        
        if (tag === 1) { 
            
            this.alpha = 0.6;
            this.setBlendColor([0, 50, 0, 80]); // Deep green shadow
        } else {
            this.alpha = 1.0;
            this.setBlendColor([0, 0, 0, 0]);
        }
    };

    // ------------------------------------------------------------------------
    // 5. REGION PATHFINDING
    // ------------------------------------------------------------------------
    const _Game_Player_isMapPassable = Game_Player.prototype.isMapPassable;
    Game_Player.prototype.isMapPassable = function(x, y, d) {
        const x2 = $gameMap.roundXWithDirection(x, d);
        const y2 = $gameMap.roundYWithDirection(y, d);
        const targetRegion = $gameMap.regionId(x2, y2);
        
        // Only override collision if the player is NOT in a vehicle
        if (!this.isInVehicle()) {
            if (blockedRegion > 0 && targetRegion === blockedRegion) return false;
            if (walkableRegion > 0 && targetRegion === walkableRegion) return true;
        }
        
        return _Game_Player_isMapPassable.call(this, x, y, d);
    };
})();