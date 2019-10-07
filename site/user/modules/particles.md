Particles
=========

Skia’s particle module provides a way to quickly generate large numbers of
drawing primitives with dynamic, animated behavior. Particles can be used to
create effects like fireworks, spark trails, ambient “weather”, and much more.
Nearly all properties and behavior are controlled by scripts written in Skia’s
custom language, SkSL.


Samples
-------

<style>
  #demo canvas {
    border: 1px dashed #AAA;
    margin: 2px;
  }

  figure {
    display: inline-block;
    margin: 0;
  }

  figcaption > a {
    margin: 2px 10px;
  }
</style>

<div id=demo>
  <figure>
    <canvas id=confetti width=400 height=400></canvas>
    <figcaption>
      <a href="https://particles.skia.org/84a757d92c424b3d378b55481a4b2394"
         target=_blank rel=noopener>Confetti</a>
    </figcaption>
  </figure>
  <figure>
    <canvas id=curves width=400 height=400></canvas>
    <figcaption>
      <a href="https://particles.skia.org/e1b1b1f5e3d31b9fae57bf90dce729a8"
         target=_blank rel=noopener>Curves</a>
    </figcaption>
  </figure>
  <figure>
    <canvas id=text width=400 height=400></canvas>
    <figcaption>
      <a href="https://particles.skia.org/7c13116e4b61c18b828bfc281903efe8"
         target=_blank rel=noopener>Text</a>
    </figcaption>
  </figure>

</div>

<script type="text/javascript" charset="utf-8">
(function() {
  // Tries to load the WASM version if supported, shows error otherwise
  let s = document.createElement('script');
  var locate_file = '';
  if (window.WebAssembly && typeof window.WebAssembly.compile === 'function') {
    console.log('WebAssembly is supported!');
//    locate_file = 'https://particles.skia.org/static/';
    locate_file = 'https://unpkg.com/canvaskit-wasm@0.7.0/bin/';
  } else {
    console.log('WebAssembly is not supported (yet) on this browser.');
    document.getElementById('demo').innerHTML = "<div>WASM not supported by your browser. Try a recent version of Chrome, Firefox, Edge, or Safari.</div>";
    return;
  }
  s.src = locate_file + 'canvaskit.js';
  s.onload = () => {
  var CanvasKit = null;
  CanvasKitInit({
    locateFile: (file) => locate_file + file,
  }).ready().then((CK) => {
    CanvasKit = CK;
    ParticleExample(CanvasKit, 'confetti', confetti, 200, 200);
    ParticleExample(CanvasKit, 'curves', curves, 200, 200);
    ParticleExample(CanvasKit, 'text', text, 75, 250);
  });

  function preventScrolling(canvas) {
    canvas.addEventListener('touchmove', (e) => {
      // Prevents touch events in the canvas from scrolling the canvas.
      e.preventDefault();
      e.stopPropagation();
    });
  }

  function ParticleExample(CanvasKit, id, jsonData, cx, cy) {
    if (!CanvasKit || !jsonData) {
      return;
    }
    const surface = CanvasKit.MakeCanvasSurface(id);
    if (!surface) {
      console.error('Could not make surface');
      return;
    }
    const context = CanvasKit.currentContext();
    const canvas = surface.getCanvas();
    canvas.translate(cx, cy);

    const particles = CanvasKit.MakeParticles(JSON.stringify(jsonData));
    particles.start(Date.now() / 1000.0, true);

    function drawFrame(canvas) {
      particles.update(Date.now() / 1000.0);

      canvas.clear(CanvasKit.WHITE);
      particles.draw(canvas);
      surface.requestAnimationFrame(drawFrame);
    }
    surface.requestAnimationFrame(drawFrame);
  }

const confetti ={
   "MaxCount": 200,
   "Drawable": {
      "Type": "SkCircleDrawable",
      "Radius": 8
   },
   "EffectCode": [
      "void effectSpawn(inout Effect effect) {",
      "  effect.lifetime = 2;",
      "}",
      "",
      "void effectUpdate(inout Effect effect) {",
      "  if (effect.age < 0.25 || effect.age > 0.75) { effect.rate = 0; }",
      "  else { effect.rate = 200; }",
      "}",
      ""
   ],
   "Code": [
      "void spawn(inout Particle p) {",
      "  float3 colors[4];",
      "  colors[0] = float3(0.87, 0.24, 0.11);",
      "  colors[1] = float3(1, 0.9, 0.2);",
      "  colors[2] = float3(0.44, 0.73, 0.24);",
      "  colors[3] = float3(0.38, 0.54, 0.95);",
      "  int idx = int(rand * 4);",
      "  p.color.rgb = colors[idx];",
      "",
      "  p.lifetime = (1 - effect.age) * effect.lifetime;",
      "  p.scale = mix(0.6, 1, rand);",
      "}",
      "",
      "void update(inout Particle p) {",
      "  p.color.a = 1 - p.age;",
      "",
      "  float a = radians(rand * 360);",
      "  float invAge = 1 - p.age;",
      "  p.vel = float2(cos(a), sin(a)) * mix(250, 550, rand) * invAge * invAge;",
      "}",
      ""
   ],
   "Bindings": []
};

const curves = {
   "MaxCount": 1000,
   "Drawable": {
      "Type": "SkCircleDrawable",
      "Radius": 2
   },
   "EffectCode": [
      "void effectSpawn(inout Effect effect) {",
      "  effect.rate = 200; effect.pos.y = 100;",
      "}",
      ""
   ],
   "Code": [
      "void spawn(inout Particle p) {",
      "  p.lifetime = 3 + rand;",
      "  p.vel.y = -50;",
      "}",
      "",
      "void update(inout Particle p) {",
      "  p.pos.x = wave(p.age);",
      "  p.color = color(p.age);",
      "}",
      ""
   ],
   "Bindings": [
      {
         "Type": "SkCurveBinding",
         "Name": "wave",
         "Curve": {
            "XValues": [],
            "Segments": [
               {
                  "Type": "Cubic",
                  "Ranged": true,
                  "Bidirectional": true,
                  "A0": -20,
                  "B0": 50,
                  "C0": -30,
                  "D0": -10,
                  "A1": 20,
                  "B1": 60,
                  "C1": -20,
                  "D1": 0
               }
            ]
         }
      },
      {
         "Type": "SkColorCurveBinding",
         "Name": "color",
         "Curve": {
            "XValues": [],
            "Segments": [
               {
                  "Type": "Linear",
                  "Ranged": true,
                  "A0": [ 1, 0, 0, 1 ],
                  "D0": [ 1, 0.735294, 0, 0.2 ],
                  "A1": [ 1, 0.588235, 0, 1 ],
                  "D1": [ 0.941177, 1, 0, 0.2 ]
               }
            ]
         }
      }
   ]
};

const raincloud = {
   "MaxCount": 128,
   "Drawable": {
      "Type": "SkCircleDrawable",
      "Radius": 2
   },
   "EffectCode": [
      "void effectSpawn(inout Effect effect) {",
      "  if (effect.loop == 0) {",
      "    cloud(true);",
      "  }",
      "  effect.color = float4(0.1, 0.1, 1.0, 1.0);",
      "  effect.rate = 10;",
      "}",
      ""
   ],
   "Code": [
      "void spawn(inout Particle p) {",
      "  p.lifetime = 4;",
      "  p.pos.x = mix(-50, 50, rand);",
      "  p.vel.y = 50;",
      "}",
      "",
      "bool once(bool cond, inout uint flags, uint flag) {",
      "  bool result = false;",
      "  if (cond && (flags & flag) == 0) {",
      "    flags |= flag;",
      "    result = true;",
      "  }",
      "  return result;",
      "}",
      "",
      "void update(inout Particle p) {",
      "  p.vel.y += 20 * dt;",
      "  if (once(p.pos.y > 150, p.flags, 0x1)) {",
      "    p.scale = 0;",
      "    splash(false);",
      "  }",
      "}",
      ""
   ],
   "Bindings": [
      {
         "Type": "SkEffectBinding",
         "Name": "cloud",
         "MaxCount": 60,
         "Drawable": {
            "Type": "SkCircleDrawable",
            "Radius": 16
         },
         "EffectCode": [
            "void effectSpawn(inout Effect effect) {",
            "  effect.color = float4(1, 1, 1, 1);",
            "  effect.rate = 30;",
            "}",
            ""
         ],
         "Code": [
            "float2 circle() {",
            "  float2 xy;",
            "  do {",
            "    xy.x = 2 * rand - 1;",
            "    xy.y = 2 * rand - 1;",
            "  } while (dot(xy, xy) > 1);",
            "  return xy;",
            "}",
            "",
            "void spawn(inout Particle p) {",
            "  p.lifetime = 2.5;",
            "  p.pos = circle() * float2(50, 10);",
            "  p.vel.x = mix(-10, 10, rand);",
            "  p.vel.y = mix(-10, 10, rand);",
            "}",
            "",
            "void update(inout Particle p) {",
            "  p.color.a = 1 - (length(p.pos) / 150);",
            "}",
            ""
         ],
         "Bindings": []
      },
      {
         "Type": "SkEffectBinding",
         "Name": "splash",
         "MaxCount": 8,
         "Drawable": {
            "Type": "SkCircleDrawable",
            "Radius": 1
         },
         "EffectCode": [
            "void effectSpawn(inout Effect effect) {",
            "  effect.burst = 8;",
            "  effect.scale = 1;",
            "}",
            ""
         ],
         "Code": [
            "void spawn(inout Particle p) {",
            "  p.lifetime = rand;",
            "  float a = radians(mix(-80, 80, rand) - 90);",
            "  p.vel.x = cos(a) * 20;",
            "  p.vel.y = sin(a) * 20;",
            "}",
            "",
            "void update(inout Particle p) {",
            "  p.vel.y += dt * 20;",
            "}",
            ""
         ],
         "Bindings": []
      }
   ]
};

const text = {
   "MaxCount": 2000,
   "Drawable": {
      "Type": "SkCircleDrawable",
      "Radius": 1
   },
   "EffectCode": [
      "void effectSpawn(inout Effect effect) {",
      "  effect.rate = 1000;",
      "}",
      ""
   ],
   "Code": [
      "void spawn(inout Particle p) {",
      "  p.lifetime = mix(1, 3, rand);",
      "  float a = radians(mix(250, 290, rand));",
      "  float s = mix(10, 30, rand);",
      "  p.vel.x = cos(a) * s;",
      "  p.vel.y = sin(a) * s;",
      "  p.pos = text(rand).xy;",
      "}",
      "",
      "void update(inout Particle p) {",
      "  float4 startColor = float4(1, 0.196, 0.078, 1);",
      "  float4 endColor   = float4(1, 0.784, 0.078, 1);",
      "  p.color = mix(startColor, endColor, p.age);",
      "}",
      ""
   ],
   "Bindings": [
      {
         "Type": "SkTextBinding",
         "Name": "text",
         "Text": "SKIA",
         "FontSize": 96
      }
   ]
};

  }
  document.head.appendChild(s);
})();
</script>
