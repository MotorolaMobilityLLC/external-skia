/*
 * This externs file prevents the Closure JS compiler from minifying away
 * names of objects created by Emscripten.
 * Basically, by defining empty objects and functions here, Closure will
 * know not to rename them.  This is needed because of our pre-js files,
 * that is, the JS we hand-write to bundle into the output. That JS will be
 * hit by the closure compiler and thus needs to know about what functions
 * have special names and should not be minified.
 *
 * Emscripten does not support automatically generating an externs file, so we
 * do it by hand. The general process is to write some JS code, and then put any
 * calls to CanvasKit or related things in here. Running ./compile.sh and then
 * looking at the minified results or running the Release trybot should
 * verify nothing was missed. Optionally, looking directly at the minified
 * pathkit.js can be useful when developing locally.
 *
 * Docs:
 *   https://github.com/cljsjs/packages/wiki/Creating-Externs
 *   https://github.com/google/closure-compiler/wiki/Types-in-the-Closure-Type-System
 *
 * Example externs:
 *   https://github.com/google/closure-compiler/tree/master/externs
 */

var CanvasKit = {
  // public API (i.e. things we declare in the pre-js file or in the cpp bindings)
  Color: function() {},
  Color4f: function() {},
  ColorAsInt: function() {},
  LTRBRect: function() {},
  XYWHRect: function() {},
  LTRBiRect: function() {},
  XYWHiRect: function() {},
  RRectXY: function() {},
  /** @return {ImageData} */
  ImageData: function() {},

  GetWebGLContext: function() {},
  MakeBlurMaskFilter: function() {},
  MakeCanvas: function() {},
  MakeCanvasSurface: function() {},
  MakeGrContext: function() {},
  /** @return {CanvasKit.SkAnimatedImage} */
  MakeAnimatedImageFromEncoded: function() {},
  /** @return {CanvasKit.SkImage} */
  MakeImage: function() {},
  /** @return {CanvasKit.SkImage} */
  MakeImageFromEncoded: function() {},
  MakeImageFromCanvasImageSource: function() {},
  MakeOnScreenGLSurface: function() {},
  MakePathFromCmds: function() {},
  MakePathFromOp: function() {},
  MakePathFromSVGString: function() {},
  MakeRenderTarget: function() {},
  MakeSkPicture: function() {},
  MakeSWCanvasSurface: function() {},
  MakeManagedAnimation: function() {},
  MakeParticles: function() {},
  MakeSkVertices: function() {},
  MakeSurface: function() {},
  MakeWebGLCanvasSurface: function() {},
  /** @return {TypedArray} */
  Malloc: function() {},
  Free: function() {},
  computeTonalColors: function() {},
  currentContext: function() {},
  getColorComponents: function() {},
  getDecodeCacheLimitBytes: function() {},
  getDecodeCacheUsageBytes: function() {},
  getSkDataBytes: function() {},
  multiplyByAlpha: function() {},
  parseColorString: function() {},
  setCurrentContext: function() {},
  setDecodeCacheLimitBytes: function() {},

  // Defined by emscripten.
  createContext:function() {},

  // private API (i.e. things declared in the bindings that we use
  // in the pre-js file)
  _computeTonalColors: function() {},
  _MakeImage: function() {},
  _MakeLinearGradientShader: function() {},
  _MakeRadialGradientShader: function() {},
  _MakeSweepGradientShader: function() {},
  _MakeManagedAnimation: function() {},
  _MakeParticles: function() {},
  _MakeSkPicture: function() {},
  _MakeSkVertices: function() {},
  _MakeTwoPointConicalGradientShader: function() {},
  _decodeAnimatedImage: function() {},
  _decodeImage: function() {},
  _drawShapedText: function() {},
  _getRasterDirectSurface: function() {},
  _getRasterN32PremulSurface: function() {},

  // The testing object is meant to expose internal functions
  // for more fine-grained testing, e.g. parseColor
  _testing: {},

  // Objects and properties on CanvasKit

  Animation: {
    prototype: {
      render: function() {},
    },
    _render: function() {},
  },

  GrContext: {
    // public API (from C++ bindings)
    getResourceCacheLimitBytes: function() {},
    getResourceCacheUsageBytes: function() {},
    releaseResourcesAndAbandonContext: function() {},
    setResourceCacheLimitBytes: function() {},
  },

  ManagedAnimation: {
    prototype: {
      render: function() {},
      seek: function() {},
      seekFrame: function() {},
      setColor: function() {},
    },
    _render: function() {},
    _seek: function() {},
    _seekFrame: function() {},
  },

  Paragraph: {
    // public API (from C++ bindings)
    didExceedMaxLines: function() {},
    getAlphabeticBaseline: function() {},
    getGlyphPositionAtCoordinate: function() {},
    getHeight: function() {},
    getIdeographicBaseline: function() {},
    getLongestLine: function() {},
    getMaxIntrinsicWidth: function() {},
    getMaxWidth: function() {},
    getMinIntrinsicWidth: function() {},
    getWordBoundary: function() {},
    layout: function() {},

    // private API
    /** @return {Float32Array} */
    _getRectsForRange: function() {},
  },

  ParagraphBuilder: {
    Make: function() {},
    MakeFromFontProvider: function() {},
    addText: function() {},
    build: function() {},
    pop: function() {},

    prototype: {
      pushStyle: function() {},
      pushPaintStyle: function() {},
    },

    // private API
    _Make: function() {},
    _MakeFromFontProvider: function() {},
    _pushStyle: function() {},
    _pushPaintStyle: function() {},
  },

  SkRuntimeEffect: {
    // public API (from C++ bindings)
    Make: function() {},

    // private API
    _makeShader: function() {},
    _makeShaderWithChildren: function() {},
  },

  ParagraphStyle: function() {},
  RSXFormBuilder: function() {},
  SkColorBuilder: function() {},
  SkRectBuilder: function() {},

  ShapedText: {
    prototype: {
      getBounds: function() {},
    },
    // private API (from C++ bindings)
    _getBounds: function() {},
  },

  SkAnimatedImage: {
    // public API (from C++ bindings)
    decodeNextFrame: function() {},
    getFrameCount: function() {},
    getRepetitionCount: function() {},
    height: function() {},
    reset: function() {},
    width: function() {},
  },

  SkCanvas: {
    // public API (from C++ bindings)
    clipPath: function() {},
    drawAnimatedImage: function() {},
    drawCircle: function() {},
    drawColorInt: function() {},
    drawImage: function() {},
    drawLine: function() {},
    drawPaint: function() {},
    drawParagraph: function() {},
    drawPath: function() {},
    drawPicture: function() {},
    drawText: function() {},
    drawTextBlob: function() {},
    drawVertices: function() {},
    flush: function() {},
    getSaveCount: function() {},
    makeSurface: function() {},
    markCTM: function() {},
    findMarkedCTM: function() {},
    restore: function() {},
    restoreToCount: function() {},
    rotate: function() {},
    save: function() {},
    scale: function() {},
    skew: function() {},
    translate: function() {},

    prototype: {
      clear: function() {},
      clipRRect: function() {},
      clipRect: function() {},
      concat44: function() {}, // deprecated
      concat: function() {},
      drawArc: function() {},
      drawAtlas: function() {},
      drawColor: function() {},
      drawColorComponents: function() {},
      drawDRRect:  function() {},
      drawImageNine: function() {},
      drawImageRect: function() {},
      drawOval: function() {},
      drawPoints: function() {},
      drawRect: function() {},
      drawRRect:  function() {},
      drawShadow: function() {},
      drawText: function() {},
      findMarkedCTM: function() {},
      getLocalToDevice: function() {},
      getTotalMatrix: function() {},
      readPixels: function() {},
      saveLayer: function() {},
      writePixels : function() {},
    },

    // private API
    _clear: function() {},
    _clipRRect: function() {},
    _clipRect: function() {},
    _concat: function() {},
    _drawArc: function() {},
    _drawAtlas: function() {},
    _drawColor: function() {},
    _drawDRRect:  function() {},
    _drawImageNine: function() {},
    _drawImageRect: function() {},
    _drawOval: function() {},
    _drawPoints: function() {},
    _drawRect: function() {},
    _drawRRect:  function() {},
    _drawShadow: function() {},
    _drawSimpleText: function() {},
    _findMarkedCTM: function() {},
    _getLocalToDevice: function() {},
    _getTotalMatrix: function() {},
    _readPixels: function() {},
    _saveLayer: function() {},
    _writePixels: function() {},
    delete: function() {},
  },

  SkColorFilter: {
    // public API (from C++ bindings and JS interface)
    MakeBlend: function() {},
    MakeCompose: function() {},
    MakeLerp: function() {},
    MakeLinearToSRGBGamma: function() {},
    MakeMatrix: function() {},
    MakeSRGBToLinearGamma: function() {},
    // private API (from C++ bindings)
    _MakeBlend: function() {},
    _makeMatrix: function() {},
  },

  SkColorMatrix: {
    concat: function() {},
    identity: function() {},
    postTranslate: function() {},
    rotated: function() {},
    scaled: function() {},
  },

  SkColorSpace: {
    Equals: function() {},
    SRGB: {},
    DISPLAY_P3: {},
    ADOBE_RGB: {},
    // private API (from C++ bindings)
    _MakeSRGB: function() {},
    _MakeDisplayP3: function() {},
    _MakeAdobeRGB: function() {},
  },

  SkContourMeasureIter: {
    next: function() {},
  },

  SkContourMeasure: {
    getPosTan: function() {},
    getSegment: function() {},
    isClosed: function() {},
    length: function() {},
  },

  SkFont: {
    // public API (from C++ bindings)
    getScaleX: function() {},
    getSize: function() {},
    getSkewX: function() {},
    getTypeface: function() {},
    measureText: function() {},
    setHinting: function() {},
    setLinearMetrics: function() {},
    setScaleX: function() {},
    setSize: function() {},
    setSkewX: function() {},
    setSubpixel: function() {},
    setTypeface: function() {},
    // private API (from C++ bindings)
    _getWidths: function() {},
  },

  SkFontMgr: {
    // public API (from C++ and JS bindings)
    FromData: function() {},
    RefDefault: function() {},
    countFamilies: function() {},
    getFamilyName: function() {},

    // private API
    _makeTypefaceFromData: function() {},
    _fromData: function() {},
  },

  TypefaceFontProvider: {
    // public API (from C++ and JS bindings)
    Make: function() {},
    registerFont: function() {},

    // private API
    _registerFont: function() {},
  },

  SkImage: {
    // public API (from C++ bindings)
    height: function() {},
    width: function() {},
    // private API
    _encodeToData: function() {},
    _encodeToDataWithFormat: function() {},
    _makeShader: function() {},
  },

  SkImageFilter: {
    MakeBlur: function() {},
    MakeColorFilter: function() {},
    MakeCompose: function() {},
    MakeMatrixTransform: function() {},

    // private API
    _MakeMatrixTransform: function() {},
  },

  // These are defined in interface.js
  SkM44: {
    identity: function() {},
    invert: function() {},
    mustInvert: function() {},
    multiply: function() {},
    rotatedUnitSinCos: function() {},
    rotated: function() {},
    scaled: function() {},
    translated: function() {},
    lookat: function() {},
    perspective: function() {},
    rc: function() {},
    transpose: function() {},
    setupCamera: function() {},
  },

  SkMatrix: {
    identity: function() {},
    invert: function() {},
    mapPoints: function() {},
    multiply: function() {},
    rotated: function() {},
    scaled: function() {},
    skewed: function() {},
    translated: function() {},
  },

  SkMaskFilter: {
    MakeBlur: function() {},
  },

  SkPaint: {
    // public API (from C++ bindings)
    /** @return {CanvasKit.SkPaint} */
    copy: function() {},
    getBlendMode: function() {},
    getColor: function() {},
    getFilterQuality: function() {},
    getStrokeCap: function() {},
    getStrokeJoin: function() {},
    getStrokeMiter: function() {},
    getStrokeWidth: function() {},
    setAntiAlias: function() {},
    setBlendMode: function() {},
    setColorInt: function() {},
    setFilterQuality: function() {},
    setImageFilter: function() {},
    setMaskFilter: function() {},
    setPathEffect: function() {},
    setShader: function() {},
    setStrokeCap: function() {},
    setStrokeJoin: function() {},
    setStrokeMiter: function() {},
    setStrokeWidth: function() {},
    setStyle: function() {},

    prototype: {
      setColor: function() {},
      setColorComponents: function() {},
      setColorInt: function() {},
    },

    // Private API
    delete: function() {},
    _getColor: function() {},
    _setColor: function() {},
  },

  SkPathEffect: {
    MakeCorner: function() {},
    MakeDash: function() {},
    MakeDiscrete: function() {},

    // Private C++ API
    _MakeDash: function() {},
  },

  SkParticleEffect: {
    // public API (from C++ bindings)
    draw: function() {},
    getEffectUniform: function() {},
    getEffectUniformCount: function() {},
    getEffectUniformFloatCount: function() {},
    getEffectUniformName: function() {},
    getParticleUniformCount: function() {},
    getParticleUniformFloatCount: function() {},
    getParticleUniformName: function() {},
    getParticleUniform: function() {},
    setPosition: function() {},
    setRate: function() {},
    start: function() {},
    update: function() {},

    // private API (from C++ bindings)
    _effectUniformPtr: function() {},
    _particleUniformPtr: function() {},
  },

  SkPath: {
    // public API (from C++ and JS bindings)
    MakeFromCmds: function() {},
    MakeFromVerbsPointsWeights: function() {},
    computeTightBounds: function() {},
    contains: function() {},
    /** @return {CanvasKit.SkPath} */
    copy: function() {},
    countPoints: function() {},
    equals: function() {},
    getBounds: function() {},
    getFillType: function() {},
    getPoint: function() {},
    isEmpty: function() {},
    isVolatile: function() {},
    reset: function() {},
    rewind: function() {},
    setFillType: function() {},
    setIsVolatile: function() {},
    toCmds: function() {},
    toSVGString: function() {},

    prototype: {
      addArc: function() {},
      addOval: function() {},
      addPath: function() {},
      addPoly: function() {},
      addRect: function() {},
      addRRect: function() {},
      addVerbsPointsWeights: function() {},
      arc: function() {},
      arcToOval: function() {},
      arcToRotated: function() {},
      arcToTangent: function() {},
      close: function() {},
      conicTo: function() {},
      cubicTo: function() {},
      dash: function() {},
      lineTo: function() {},
      moveTo: function() {},
      offset: function() {},
      op: function() {},
      quadTo: function() {},
      rArcTo: function() {},
      rConicTo: function() {},
      rCubicTo: function() {},
      rect: function() {},
      rLineTo: function() {},
      rMoveTo: function() {},
      rQuadTo: function() {},
      simplify: function() {},
      stroke: function() {},
      transform: function() {},
      trim: function() {},
    },

    // private API
    _MakeFromCmds: function() {},
    _MakeFromVerbsPointsWeights: function() {},
    _addArc: function() {},
    _addOval: function() {},
    _addPath: function() {},
    _addPoly: function() {},
    _addRect: function() {},
    _addRRect: function() {},
    _addVerbsPointsWeights: function() {},
    _arcToOval: function() {},
    _arcToRotated: function() {},
    _arcToTangent: function() {},
    _close: function() {},
    _conicTo: function() {},
    _cubicTo: function() {},
    _dash: function() {},
    _lineTo: function() {},
    _moveTo: function() {},
    _op: function() {},
    _quadTo: function() {},
    _rArcTo: function() {},
    _rConicTo: function() {},
    _rCubicTo: function() {},
    _rect: function() {},
    _rLineTo: function() {},
    _rMoveTo: function() {},
    _rQuadTo: function() {},
    _simplify: function() {},
    _stroke: function() {},
    _transform: function() {},
    _trim: function() {},
    delete: function() {},
    dump: function() {},
    dumpHex: function() {},
  },

  SkPathMeasure: {
    getLength: function() {},
    getSegment: function() {},
    getPosTan: function() {},
    isClosed: function() {},
    nextContour: function() {},
  },

  SkPicture: {
    serialize: function() {},
  },

  SkPictureRecorder: {
    finishRecordingAsPicture: function() {},
    prototype: {
      beginRecording: function() {},
    },
    _beginRecording: function() {},
  },

  SkShader: {
    Blend: function() {},
    Color: function() {},
    Empty: function() {},
    Lerp: function() {},
    MakeLinearGradient: function() {},
    MakeRadialGradient: function() {},
    MakeTwoPointConicalGradient: function() {},
    MakeSweepGradient: function() {},

    // private API (from C++ bindings)
    _Color: function() {},
  },

  SkSurface: {
    // public API (from C++ bindings)
    /** @return {CanvasKit.SkCanvas} */
    getCanvas: function() {},
    imageInfo: function() {},

    makeSurface: function() {},
    sampleCnt: function() {},
    reportBackendTypeIsGPU: function() {},
    grContext: {},
    openGLversion: {},

    prototype: {
      /** @return {CanvasKit.SkImage} */
      makeImageSnapshot: function() {},
    },

    // private API
    _flush: function() {},
    _getRasterN32PremulSurface: function() {},
    _makeImageSnapshot: function() {},
    delete: function() {},
  },

  SkTextBlob: {
    // public API (both C++ and JS bindings)
    MakeFromRSXform: function() {},
    MakeFromText: function() {},
    MakeOnPath: function() {},
    // private API (from C++ bindings)
    _MakeFromRSXform: function() {},
    _MakeFromText: function() {},
  },

  // These are defined in interface.js
  SkVector: {
    add: function() {},
    sub: function() {},
    dot: function() {},
    cross: function() {},
    normalize: function() {},
    mulScalar: function() {},
    length: function() {},
    lengthSquared: function() {},
    dist: function() {},
  },

  SkVertices: {
    // public API (from C++ bindings)
    uniqueID: function() {},

    prototype: {
      bounds: function() {},
    },
    // private API (from C++ bindings)

    _bounds: function() {},
  },

  _SkVerticesBuilder: {
    colors: function() {},
    detach: function() {},
    indices: function() {},
    positions: function() {},
    texCoords: function() {},
  },

  TextStyle: function() {},

  // Constants and Enums
  gpu: {},
  skottie: {},

  TRANSPARENT: {},
  BLACK: {},
  WHITE: {},
  RED: {},
  GREEN: {},
  BLUE: {},
  YELLOW: {},
  CYAN: {},
  MAGENTA: {},

  MOVE_VERB: {},
  LINE_VERB: {},
  QUAD_VERB: {},
  CONIC_VERB: {},
  CUBIC_VERB: {},
  CLOSE_VERB: {},

  NoDecoration: {},
  UnderlineDecoration: {},
  OverlineDecoration: {},
  LineThroughDecoration: {},

  SaveLayerInitWithPrevious: {},
  SaveLayerF16ColorType: {},

  Affinity: {
    Upstream: {},
    Downstream: {},
  },

  AlphaType: {
    Opaque: {},
    Premul: {},
    Unpremul: {},
  },

  BlendMode: {
    Clear: {},
    Src: {},
    Dst: {},
    SrcOver: {},
    DstOver: {},
    SrcIn: {},
    DstIn: {},
    SrcOut: {},
    DstOut: {},
    SrcATop: {},
    DstATop: {},
    Xor: {},
    Plus: {},
    Modulate: {},
    Screen: {},
    Overlay: {},
    Darken: {},
    Lighten: {},
    ColorDodge: {},
    ColorBurn: {},
    HardLight: {},
    SoftLight: {},
    Difference: {},
    Exclusion: {},
    Multiply: {},
    Hue: {},
    Saturation: {},
    Color: {},
    Luminosity: {},
  },

  BlurStyle: {
    Normal: {},
    Solid: {},
    Outer: {},
    Inner: {},
  },

  ClipOp: {
    Difference: {},
    Intersect: {},
  },

  ColorType: {
    Alpha_8: {},
    RGB_565: {},
    ARGB_4444: {},
    RGBA_8888: {},
    RGB_888x: {},
    BGRA_8888: {},
    RGBA_1010102: {},
    RGB_101010x: {},
    Gray_8: {},
    RGBA_F16: {},
    RGBA_F32: {},
  },

  FillType: {
    Winding: {},
    EvenOdd: {},
  },

  FilterQuality: {
    None: {},
    Low: {},
    Medium: {},
    High: {},
  },

  FontSlant: {
    Upright: {},
    Italic: {},
    Oblique: {},
  },

  FontHinting: {
    None: {},
    Slight: {},
    Normal: {},
    Full: {},
  },

  FontWeight: {
    Invisible: {},
    Thin: {},
    ExtraLight: {},
    Light: {},
    Normal: {},
    Medium: {},
    SemiBold: {},
    Bold: {},
    ExtraBold: {},
    Black: {},
    ExtraBlack: {},
  },

  FontWidth: {
    UltraCondensed: {},
    ExtraCondensed: {},
    Condensed: {},
    SemiCondensed: {},
    Normal: {},
    SemiExpanded: {},
    Expanded: {},
    ExtraExpanded: {},
    UltraExpanded: {},
  },

  ImageFormat: {
    PNG: {},
    JPEG: {},
  },

  PaintStyle: {
    Fill: {},
    Stroke: {},
  },

  PathOp: {
    Difference: {},
    Intersect: {},
    Union: {},
    XOR: {},
    ReverseDifference: {},
  },

  PointMode: {
    Points: {},
    Lines: {},
    Polygon: {},
  },

  RectHeightStyle: {
    Tight: {},
    Max: {},
    IncludeLineSpacingMiddle: {},
    IncludeLineSpacingTop: {},
    IncludeLineSpacingBottom: {},
  },

  RectWidthStyle: {
    Tight: {},
    Max: {},
  },

  StrokeCap: {
    Butt: {},
    Round: {},
    Square: {},
  },

  StrokeJoin: {
    Miter: {},
    Round: {},
    Bevel: {},
  },

  TextAlign: {
    Left: {},
    Right: {},
    Center: {},
    Justify: {},
    Start: {},
    End: {},
  },

  TextDirection: {
    LTR: {},
    RTL: {},
  },

  TileMode: {
    Clamp: {},
    Repeat: {},
    Mirror: {},
    Decal: {},
  },

  VertexMode: {
    Triangles: {},
    TrianglesStrip: {},
    TriangleFan: {},
  },

  // Things Enscriptem adds for us

  /**
   * @type {Float32Array}
   */
  HEAPF32: {},
  /**
   * @type {Float64Array}
   */
  HEAPF64: {},
  /**
   * @type {Uint8Array}
   */
  HEAPU8: {},
  /**
   * @type {Uint16Array}
   */
  HEAPU16: {},
  /**
   * @type {Uint32Array}
   */
  HEAPU32: {},
  /**
   * @type {Int8Array}
   */
  HEAP8: {},
  /**
   * @type {Int16Array}
   */
  HEAP16: {},
  /**
   * @type {Int32Array}
   */
  HEAP32: {},

  _malloc: function() {},
  _free: function() {},
  onRuntimeInitialized: function() {},
};

// Public API things that are newly declared in the JS should go here.
// It's not enough to declare them above, because closure can still erase them
// unless they go on the prototype.
CanvasKit.Paragraph.prototype.getRectsForRange = function() {};

CanvasKit.SkPicture.prototype.saveAsFile = function() {};

CanvasKit.SkSurface.prototype.dispose = function() {};
CanvasKit.SkSurface.prototype.flush = function() {};
CanvasKit.SkSurface.prototype.requestAnimationFrame = function() {};
CanvasKit.SkSurface.prototype.drawOnce = function() {};
CanvasKit.SkSurface.prototype.captureFrameAsSkPicture = function() {};

CanvasKit.SkImage.prototype.encodeToData = function() {};
CanvasKit.SkImage.prototype.makeShader = function() {};

CanvasKit.SkFontMgr.prototype.MakeTypefaceFromData = function() {};

CanvasKit.SkFont.prototype.getWidths = function() {};

CanvasKit.RSXFormBuilder.prototype.build = function() {};
CanvasKit.RSXFormBuilder.prototype.delete = function() {};
CanvasKit.RSXFormBuilder.prototype.push = function() {};
CanvasKit.RSXFormBuilder.prototype.set = function() {};

CanvasKit.SkColorBuilder.prototype.build = function() {};
CanvasKit.SkColorBuilder.prototype.delete = function() {};
CanvasKit.SkColorBuilder.prototype.push = function() {};
CanvasKit.SkColorBuilder.prototype.set = function() {};

CanvasKit.SkRuntimeEffect.prototype.makeShader = function() {};
CanvasKit.SkRuntimeEffect.prototype.makeShaderWithChildren = function() {};

CanvasKit.SkParticleEffect.prototype.effectUniforms = function() {};
CanvasKit.SkParticleEffect.prototype.particleUniforms = function() {};

// Define StrokeOpts object
var StrokeOpts = {};
StrokeOpts.prototype.width;
StrokeOpts.prototype.miter_limit;
StrokeOpts.prototype.cap;
StrokeOpts.prototype.join;
StrokeOpts.prototype.precision;

// Define everything created in the canvas2d spec here
var HTMLCanvas = {};
HTMLCanvas.prototype.decodeImage = function() {};
HTMLCanvas.prototype.dispose = function() {};
HTMLCanvas.prototype.getContext = function() {};
HTMLCanvas.prototype.loadFont = function() {};
HTMLCanvas.prototype.makePath2D = function() {};
HTMLCanvas.prototype.toDataURL = function() {};

var ImageBitmapRenderingContext = {};
ImageBitmapRenderingContext.prototype.transferFromImageBitmap = function() {};

var CanvasRenderingContext2D = {};
CanvasRenderingContext2D.prototype.addHitRegion = function() {};
CanvasRenderingContext2D.prototype.arc = function() {};
CanvasRenderingContext2D.prototype.arcTo = function() {};
CanvasRenderingContext2D.prototype.beginPath = function() {};
CanvasRenderingContext2D.prototype.bezierCurveTo = function() {};
CanvasRenderingContext2D.prototype.clearHitRegions = function() {};
CanvasRenderingContext2D.prototype.clearRect = function() {};
CanvasRenderingContext2D.prototype.clip = function() {};
CanvasRenderingContext2D.prototype.closePath = function() {};
CanvasRenderingContext2D.prototype.createImageData = function() {};
CanvasRenderingContext2D.prototype.createLinearGradient = function() {};
CanvasRenderingContext2D.prototype.createPattern = function() {};
CanvasRenderingContext2D.prototype.createRadialGradient = function() {};
CanvasRenderingContext2D.prototype.drawFocusIfNeeded = function() {};
CanvasRenderingContext2D.prototype.drawImage = function() {};
CanvasRenderingContext2D.prototype.ellipse = function() {};
CanvasRenderingContext2D.prototype.fill = function() {};
CanvasRenderingContext2D.prototype.fillRect = function() {};
CanvasRenderingContext2D.prototype.fillText = function() {};
CanvasRenderingContext2D.prototype.getImageData = function() {};
CanvasRenderingContext2D.prototype.getLineDash = function() {};
CanvasRenderingContext2D.prototype.isPointInPath = function() {};
CanvasRenderingContext2D.prototype.isPointInStroke = function() {};
CanvasRenderingContext2D.prototype.lineTo = function() {};
CanvasRenderingContext2D.prototype.measureText = function() {};
CanvasRenderingContext2D.prototype.moveTo = function() {};
CanvasRenderingContext2D.prototype.putImageData = function() {};
CanvasRenderingContext2D.prototype.quadraticCurveTo = function() {};
CanvasRenderingContext2D.prototype.rect = function() {};
CanvasRenderingContext2D.prototype.removeHitRegion = function() {};
CanvasRenderingContext2D.prototype.resetTransform = function() {};
CanvasRenderingContext2D.prototype.restore = function() {};
CanvasRenderingContext2D.prototype.rotate = function() {};
CanvasRenderingContext2D.prototype.save = function() {};
CanvasRenderingContext2D.prototype.scale = function() {};
CanvasRenderingContext2D.prototype.scrollPathIntoView = function() {};
CanvasRenderingContext2D.prototype.setLineDash = function() {};
CanvasRenderingContext2D.prototype.setTransform = function() {};
CanvasRenderingContext2D.prototype.stroke = function() {};
CanvasRenderingContext2D.prototype.strokeRect = function() {};
CanvasRenderingContext2D.prototype.strokeText = function() {};
CanvasRenderingContext2D.prototype.transform = function() {};
CanvasRenderingContext2D.prototype.translate = function() {};

var Path2D = {};
Path2D.prototype.addPath = function() {};
Path2D.prototype.arc = function() {};
Path2D.prototype.arcTo = function() {};
Path2D.prototype.bezierCurveTo = function() {};
Path2D.prototype.closePath = function() {};
Path2D.prototype.ellipse = function() {};
Path2D.prototype.lineTo = function() {};
Path2D.prototype.moveTo = function() {};
Path2D.prototype.quadraticCurveTo = function() {};
Path2D.prototype.rect = function() {};

var LinearCanvasGradient = {};
LinearCanvasGradient.prototype.addColorStop = function() {};
var RadialCanvasGradient = {};
RadialCanvasGradient.prototype.addColorStop = function() {};
var CanvasPattern = {};
CanvasPattern.prototype.setTransform = function() {};

var ImageData = {
  /**
   * @type {Uint8ClampedArray}
   */
  data: {},
  height: {},
  width: {},
};

var DOMMatrix = {
  a: {},
  b: {},
  c: {},
  d: {},
  e: {},
  f: {},
};

// Not sure why this is needed - might be a bug in emsdk that this isn't properly declared.
function loadWebAssemblyModule() {};

// This is a part of emscripten's webgl glue code. Preserving this attribute is necessary
// to override it in the puppeteer tests
var LibraryEGL = {
  contextAttributes: {
    majorVersion: {}
  }
}
