<script>
  import Config from "./Config.svelte";
  import Viewer from "./Viewer.svelte";
  //import CheckTileExistence from "./Tiler.ts"
  import { beforeUpdate, afterUpdate, onMount } from "svelte";

  let config = {};

  let viewConfig = {
    previewOutputLevel: 0,
    showPixelRange: true,
    showInputTiles: true,
    showOutputTiles: true,
  };

  let buttonText = "Process Dataset";
  let buttonColor = "#060";

  const getButtonColor = (state) => {
    switch (state) {
      case "Process Dataset":
        return "#060";
      case "Cancel":
        return "#700";
    }
    return "#006";
  };

  const buttonExecute = () => {
    const req = new XMLHttpRequest();
    req.open("POST", "/process");
    req.setRequestHeader("Content-Type", "application/json;charset=UTF-8");
    req.send(JSON.stringify(config));
  };

  const getCanvas = () => document.getElementById("myCanvas");

  // all of these values are in view space units
  const drawView = {
    scale: { x: -7, y: -7 },
    offset: { x: 0, y: 0 },
    dragStart: { x: 0, y: 0 },
    dragging: false,
    dragOffsetStart: { x: 0, y: 0 },
    cursor: { x: 0, y: 0 },
    canvasSize: { x: 0, y: 0 },
    // from pixel space
    toViewSpace: function (x, y) {
      return {
        x: x * Math.pow(2, this.scale.x) + this.offset.x,
        y:
          this.canvasSize.y - 1 - y * Math.pow(2, this.scale.y) + this.offset.y,
      };
    },
    // from view space
    toPixelSpace: function (x, y) {
      return {
        x: (x - this.offset.x) / Math.pow(2, this.scale.x),
        y:
          (this.canvasSize.y + 1 - (y - this.offset.y)) /
          Math.pow(2, this.scale.y),
      };
    },
  };

  const generationState = {
    logs: [],
    logsLastRetrieved: 0, // micros since server started
    cachedTiles: [],
    processedTiles: [],
    queryNewLogs: function () {
      const req = new XMLHttpRequest();
      req.open("POST", "/status");
      req.setRequestHeader("Content-Type", "application/json;charset=UTF-8");
      req.onreadystatechange = (e) => {
        if (e.target.readyState != XMLHttpRequest.DONE) return;
        let recentLogs;
        try {
          recentLogs = JSON.parse(e.target.response);
        } catch (ex) {
          console.log("malformed response");
        }
        this.logsLastRetrieved = recentLogs.time;
        if (recentLogs.logs.length > 0) {
          this.logs = this.logs.concat(recentLogs.logs);
          recentLogs.logs.forEach((log) => console.log(log.content));
        }
      };
      req.send(JSON.stringify({ beginMicros: this.logsLastRetrieved }));
    },
  };

  // modified the given drawView
  const changeScaleAround = (drawView, scaleDiff, x, y) => {
    //const oldScaleFac = {x: Math.pow(2, drawView.scale.x), y: Math.pow(2, drawView.scale.y)};
    const ps = drawView.toPixelSpace(x, y);
    const oldvs = drawView.toViewSpace(ps.x, ps.y);
    drawView.scale.x += scaleDiff;
    drawView.scale.x = Math.min(Math.max(drawView.scale.x, -10), 10);
    drawView.scale.y = drawView.scale.x;
    const newvs = drawView.toViewSpace(ps.x, ps.y);
    drawView.offset.x -= newvs.x - oldvs.x;
    drawView.offset.y -= newvs.y - oldvs.y;
  };

  const humanize = (x) => {
    return x.toFixed(2).replace(/\.?0*$/, "");
  };

  const correctMod = (val, mod) => {
    const modVal = Math.abs(Math.floor(val)) % mod;
    if (Math.floor(val) < 0 && modVal) return mod - modVal;
    else return modVal;
  };

  const floorOnInterval = (val, mod) => {
    return Math.floor(val) - correctMod(val, mod);
  };

  const ceilOnInterval = (val, mod) => {
    return floorOnInterval(val, mod) + mod;
  };

  const drawConfig = (config, canvas) => {
    const ctx = canvas.getContext("2d");
    canvas.width = window.innerWidth;
    canvas.height = window.innerHeight;

    drawView.canvasSize = { x: canvas.width, y: canvas.height };

    ctx.font = "14px Consolas, monaco, monospace";

    ctx.lineWidth = 1;

    const cursorPX = drawView.toPixelSpace(
      drawView.cursor.x,
      drawView.cursor.y
    );

    const text = (x, y, val, color = "#fff") => {
      const px = drawView.toViewSpace(x, y);
      ctx.fillStyle = color;
      ctx.fillText(val, px.x, px.y);
    };

    const rect = (x, y, w, h, color = "#fff") => {
      const start = drawView.toViewSpace(x, y);
      ctx.beginPath();
      ctx.rect(start.x, start.y, w, h);
      ctx.strokeStyle = color;
      ctx.stroke();
    };

    const fillRect = (x, y, w, h, color = "#fff") => {
      ctx.fillStyle = color;
      const begin = drawView.toViewSpace(x, y);
      const end = drawView.toViewSpace(x + w, y + h);
      ctx.fillRect(begin.x, begin.y, end.x - begin.x, end.y - begin.y);
    };

    const horizontal = (y) => {
      ctx.beginPath();
      ctx.moveTo(0, drawView.toViewSpace(0, y).y);
      ctx.lineTo(canvas.width + 1, drawView.toViewSpace(0, y).y);
      ctx.stroke();
    };

    const vertical = (x) => {
      ctx.beginPath();
      ctx.moveTo(drawView.toViewSpace(x, 0).x, 0);
      ctx.lineTo(drawView.toViewSpace(x, 0).x, canvas.height);
      ctx.stroke();
    };

    const horizontalSeg = (y, begin, end) => {
      const vs0 = drawView.toViewSpace(begin, y);
      const vs1 = drawView.toViewSpace(end, y);
      ctx.beginPath();
      ctx.moveTo(vs0.x, vs0.y);
      ctx.lineTo(vs1.x, vs1.y);
      ctx.stroke();
    };

    const verticalSeg = (x, begin, end) => {
      const vs0 = drawView.toViewSpace(x, begin);
      const vs1 = drawView.toViewSpace(x, end);
      ctx.beginPath();
      ctx.moveTo(vs0.x, vs0.y);
      ctx.lineTo(vs1.x, vs1.y);
      ctx.stroke();
    };

    const hoveredInputTile = {
      x:
        floorOnInterval(cursorPX.x, config.SpatialConfig.InputTileSize[0]) /
        config.SpatialConfig.InputTileSize[0],
      y:
        floorOnInterval(cursorPX.y, config.SpatialConfig.InputTileSize[1]) /
        config.SpatialConfig.InputTileSize[1],
    };
    const hoveredOutputTile = {
      x:
        floorOnInterval(cursorPX.x, config.SpatialConfig.OutputTileSize[0]) /
        config.SpatialConfig.OutputTileSize[0],
      y:
        floorOnInterval(cursorPX.y, config.SpatialConfig.OutputTileSize[1]) /
        config.SpatialConfig.OutputTileSize[1],
    };

    // draw cursor
    {
      const textSS = drawView.toPixelSpace(
        drawView.cursor.x + 6,
        drawView.cursor.y - 10
      );
      const textSS1 = drawView.toPixelSpace(
        drawView.cursor.x + 6,
        drawView.cursor.y - 25
      );
      const textSS2 = drawView.toPixelSpace(
        drawView.cursor.x + 6,
        drawView.cursor.y - 40
      );
      ctx.strokeStyle = "#fff4";
      vertical(cursorPX.x);
      horizontal(cursorPX.y);
      text(
        textSS.x,
        textSS.y,
        "Pixel      : " + humanize(cursorPX.x) + ", " + humanize(cursorPX.y)
      );
      text(
        textSS1.x,
        textSS1.y,
        "Input Tile : " +
          humanize(hoveredInputTile.x) +
          ", " +
          humanize(hoveredInputTile.y)
      );
      text(
        textSS2.x,
        textSS2.y,
        "Output Tile: " +
          humanize(hoveredOutputTile.x) +
          ", " +
          humanize(hoveredOutputTile.y)
      );
    }

    // draw hovered input tile
    {
      fillRect(
        hoveredInputTile.x * config.SpatialConfig.InputTileSize[0],
        hoveredInputTile.y * config.SpatialConfig.InputTileSize[1],
        config.SpatialConfig.InputTileSize[0],
        config.SpatialConfig.InputTileSize[1],
        "#f884"
      );
    }

    {
      fillRect(
        hoveredOutputTile.x * config.SpatialConfig.OutputTileSize[0],
        hoveredOutputTile.y * config.SpatialConfig.OutputTileSize[1],
        config.SpatialConfig.OutputTileSize[0],
        config.SpatialConfig.OutputTileSize[1],
        "#8f84"
      );
    }

    // draw hovered pixel
    {
      fillRect(Math.floor(cursorPX.x), Math.floor(cursorPX.y), 1, 1, "#ffff");
    }

    ctx.globalCompositeOperation = "lighter";

    if (viewConfig.showInputTiles) {
      const intervalX = config.SpatialConfig.InputTileSize[0];
      const intervalY = config.SpatialConfig.InputTileSize[1];

      const beginX =
        floorOnInterval(
          config.SpatialConfig.OutputPixelRange.min[0],
          intervalX
        ) / intervalX;
      const endX =
        ceilOnInterval(
          config.SpatialConfig.OutputPixelRange.max[0] - 1,
          intervalX
        ) / intervalX;
      const beginY =
        floorOnInterval(
          config.SpatialConfig.OutputPixelRange.min[1],
          intervalY
        ) / intervalY;
      const endY =
        ceilOnInterval(
          config.SpatialConfig.OutputPixelRange.max[1] - 1,
          intervalY
        ) / intervalY;

      ctx.strokeStyle = "#fff4";
      for (let i = beginX; i <= endX; ++i) {
        verticalSeg(i * intervalX, beginY * intervalY, endY * intervalY);
      }

      for (let i = beginY; i <= endY; ++i) {
        horizontalSeg(i * intervalY, beginX * intervalX, endX * intervalX);
      }
    }

    if (viewConfig.showOutputTiles) {
      const intervalX =
        config.SpatialConfig.OutputTileSize[0] *
        Math.pow(2, viewConfig.previewOutputLevel);
      const intervalY =
        config.SpatialConfig.OutputTileSize[1] *
        Math.pow(2, viewConfig.previewOutputLevel);

      const beginX =
        floorOnInterval(
          config.SpatialConfig.OutputPixelRange.min[0],
          intervalX
        ) / intervalX;
      const endX =
        ceilOnInterval(
          config.SpatialConfig.OutputPixelRange.max[0] - 1,
          intervalX
        ) / intervalX;
      const beginY =
        floorOnInterval(
          config.SpatialConfig.OutputPixelRange.min[1],
          intervalY
        ) / intervalY;
      const endY =
        ceilOnInterval(
          config.SpatialConfig.OutputPixelRange.max[1] - 1,
          intervalY
        ) / intervalY;

      ctx.strokeStyle = "#fff4";
      for (let i = beginX; i <= endX; ++i) {
        verticalSeg(i * intervalX, beginY * intervalY, endY * intervalY);
      }

      for (let i = beginY; i <= endY; ++i) {
        horizontalSeg(i * intervalY, beginX * intervalX, endX * intervalX);
      }
    }

    ctx.globalCompositeOperation = "source-over";

    if (viewConfig.showPixelRange) {
      fillRect(
        config.SpatialConfig.OutputPixelRange.min[0],
        config.SpatialConfig.OutputPixelRange.min[1],
        config.SpatialConfig.OutputPixelRange.max[0] -
          config.SpatialConfig.OutputPixelRange.min[0],
        config.SpatialConfig.OutputPixelRange.max[1] -
          config.SpatialConfig.OutputPixelRange.min[1],
        "#44f3"
      );
    }
  };

  const drawStatus = (config, logs, canvas) => {};

  window.addEventListener("resize", () => {
    drawConfig(config, getCanvas());
  });

  afterUpdate(() => {
    drawConfig(config, getCanvas());
  });

  onMount(() => {
    const canvas = getCanvas();
    canvas.addEventListener("wheel", (ev) => {
      changeScaleAround(
        drawView,
        ev.wheelDeltaY * 0.0005,
        ev.clientX,
        ev.clientY
      );
      drawConfig(config, getCanvas());
      //clickSound.play();
    });
    canvas.addEventListener("mousedown", (ev) => {
      drawView.dragging = true;
      drawView.dragStart.x = ev.clientX;
      drawView.dragStart.y = ev.clientY;
      drawView.dragOffsetStart = drawView.offset;
    });
    canvas.addEventListener("mouseup", (ev) => {
      drawView.dragging = false;
    });
    canvas.addEventListener("mousemove", (ev) => {
      drawView.cursor = { x: ev.clientX, y: ev.clientY };
      if (drawView.dragging) {
        const mouseDiff = {
          x: ev.clientX - drawView.dragStart.x,
          y: ev.clientY - drawView.dragStart.y,
        };
        drawView.offset = {
          x: drawView.dragOffsetStart.x + mouseDiff.x,
          y: drawView.dragOffsetStart.y + mouseDiff.y,
        };
      }
      drawConfig(config, getCanvas());
    });
    canvas.addEventListener("mouseleave", (ev) => {
      drawView.dragging = false;
    });
  });

  //setInterval(() => generationState.queryNewLogs(), 250);
</script>

<main>
  <canvas id="myCanvas" />
  <div class="leftBlock">
    <Config bind:res={config} />

    <button
      class="executeButton"
      style="background-color:{getButtonColor(buttonText)}"
      on:click={buttonExecute}
    >
      {buttonText}
    </button>

    <input type="text" value={JSON.stringify(config)} readonly />
  </div>
  <div class="leftBlock">
    <Viewer
      bind:previewOutputLevel={viewConfig.previewOutputLevel}
      bind:showPixelRange={viewConfig.showPixelRange}
      bind:showInputTiles={viewConfig.showInputTiles}
      bind:showOutputTiles={viewConfig.showOutputTiles}
      levelMin={config.BeginOutputLevel}
      levelMax={config.EndOutputLevel}
    />
  </div>
</main>

<style>
  .leftBlock {
    margin-left: 1em;
    margin-top: 1em;
    padding: 1em;
    border: solid;
    border-color: #888;
    background-color: #000d;
    display: flex;
    flex-direction: column;
    width: 30em;
  }

  main {
    background-color: #000;
    position: absolute;
    width: 100%;
    height: 100%;
    margin: 0;
    padding: 0;
    z-index: -10;
  }

  .executeButton {
    display: block;
    width: 100%;
    border: 2px solid #fff;
    cursor: pointer;
    height: 3em;
    padding: 2px;
    color: #fff;
  }

  canvas {
    position: absolute;
    z-index: -9;
  }
</style>
