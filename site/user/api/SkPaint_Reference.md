SkPaint Reference
===

# <a name="Paint"></a> Paint
<a href="bmh_SkPaint_Reference?cl=9919#Paint">Paint</a> controls options applied when drawing and measuring. <a href="bmh_SkPaint_Reference?cl=9919#Paint">Paint</a> collects all
options outside of the <a href="bmh_SkCanvas_Reference?cl=9919#Clip">Canvas Clip</a> and <a href="bmh_SkCanvas_Reference?cl=9919#Matrix">Canvas Matrix</a>.

Various options apply to text, strokes and fills, and images. 

Some options may not be implemented on all platforms; in these cases, setting
the option has no effect. Some options are conveniences that duplicate <a href="bmh_SkCanvas_Reference?cl=9919#Canvas">Canvas</a>
functionality; for instance, text size is identical to matrix scale.

<a href="bmh_SkPaint_Reference?cl=9919#Paint">Paint</a> options are rarely exclusive; each option modifies a stage of the drawing
pipeline and multiple pipeline stages may be affected by a single <a href="bmh_SkPaint_Reference?cl=9919#Paint">Paint</a>.

<a href="bmh_SkPaint_Reference?cl=9919#Paint">Paint</a> collects effects and filters that describe single-pass and multiple-pass 
algorithms that alter the drawing geometry, color, and transparency. For instance,
<a href="bmh_SkPaint_Reference?cl=9919#Paint">Paint</a> does not directly implement dashing or blur, but contains the objects that do so. 

The objects contained by <a href="bmh_SkPaint_Reference?cl=9919#Paint">Paint</a> are opaque, and cannot be edited outside of the <a href="bmh_SkPaint_Reference?cl=9919#Paint">Paint</a>
to affect it. The implementation is free to defer computations associated with the
<a href="bmh_SkPaint_Reference?cl=9919#Paint">Paint</a>, or ignore them altogether. For instance, some <a href="bmh_undocumented?cl=9919#GPU">GPU</a> implementations draw all
<a href="bmh_SkPath_Reference?cl=9919#Path">Path</a> geometries with anti-aliasing, regardless of <a href="bmh_SkPaint_Reference?cl=9919#kAntiAlias_Flag">SkPaint::kAntiAlias Flag</a> setting.

<a href="bmh_SkPaint_Reference?cl=9919#Paint">Paint</a> describes a single color, a single font, a single image quality, and so on.
Multiple colors are drawn either by using multiple paints or with objects like
<a href="bmh_undocumented?cl=9919#Shader">Shader</a> attached to <a href="bmh_SkPaint_Reference?cl=9919#Paint">Paint</a>.

# <a name="SkPaint"></a> Class SkPaint

# <a name="Overview"></a> Overview

## <a name="Subtopics"></a> Subtopics

| topics | description |
| --- | ---  |
| <a href="bmh_SkPaint_Reference?cl=9919#Initializers">Initializers</a> | Constructors and initialization. |
| <a href="bmh_SkPaint_Reference?cl=9919#Destructor">Destructor</a> | <a href="bmh_SkPaint_Reference?cl=9919#Paint">Paint</a> termination. |
| <a href="bmh_SkPaint_Reference?cl=9919#Management">Management</a> | <a href="bmh_SkPaint_Reference?cl=9919#Paint">Paint</a> copying, moving, comparing. |
| <a href="bmh_SkPaint_Reference?cl=9919#Hinting">Hinting</a> | <a href="bmh_undocumented?cl=9919#Glyph">Glyph</a> outline adjustment. |
| <a href="bmh_SkPaint_Reference?cl=9919#Flags">Flags</a> | Attributes represented by single bits. |
| <a href="bmh_SkPaint_Reference?cl=9919#Anti_alias">Anti-alias</a> | Approximating coverage with transparency. |
| <a href="bmh_SkPaint_Reference?cl=9919#Dither">Dither</a> | Distributing color error. |
| <a href="bmh_SkPaint_Reference?cl=9919#Device_Text">Device Text</a> | Increase precision of glyph position. |
| <a href="bmh_SkPaint_Reference?cl=9919#Font_Embedded_Bitmaps">Font Embedded Bitmaps</a> | Custom-sized bitmap glyphs. |
| <a href="bmh_SkPaint_Reference?cl=9919#Automatic_Hinting">Automatic Hinting</a> | Always adjust glyph paths. |
| <a href="bmh_SkPaint_Reference?cl=9919#Vertical_Text">Vertical Text</a> | Orient text from top to bottom. |
| <a href="bmh_SkPaint_Reference?cl=9919#Fake_Bold">Fake Bold</a> | Approximate font styles. |
| <a href="bmh_SkPaint_Reference?cl=9919#Full_Hinting_Spacing">Full Hinting Spacing</a> | <a href="bmh_undocumented?cl=9919#Glyph">Glyph</a> spacing affected by hinting. |
| <a href="bmh_SkPaint_Reference?cl=9919#Filter_Quality_Methods">Filter Quality Methods</a> | Get and set <a href="bmh_undocumented?cl=9919#Filter_Quality">Filter Quality</a>. |
| <a href="bmh_SkPaint_Reference?cl=9919#Color_Methods">Color Methods</a> | Get and set <a href="bmh_undocumented?cl=9919#Color">Color</a>. |
| <a href="bmh_SkPaint_Reference?cl=9919#Style">Style</a> | Geometry filling, stroking. |
| <a href="bmh_SkPaint_Reference?cl=9919#Stroke_Width">Stroke Width</a> | Thickness perpendicular to geometry. |
| <a href="bmh_SkPaint_Reference?cl=9919#Miter_Limit">Miter Limit</a> | Maximum length of stroked corners. |
| <a href="bmh_SkPaint_Reference?cl=9919#Stroke_Cap">Stroke Cap</a> | Decorations at ends of open strokes. |
| <a href="bmh_SkPaint_Reference?cl=9919#Stroke_Join">Stroke Join</a> | Decoration at corners of strokes. |
| <a href="bmh_SkPaint_Reference?cl=9919#Fill_Path">Fill Path</a> | Make <a href="bmh_SkPath_Reference?cl=9919#Path">Path</a> from <a href="bmh_undocumented?cl=9919#Path_Effect">Path Effect</a>, stroking. |
| <a href="bmh_SkPaint_Reference?cl=9919#Shader_Methods">Shader Methods</a> | Get and set <a href="bmh_undocumented?cl=9919#Shader">Shader</a>. |
| <a href="bmh_SkPaint_Reference?cl=9919#Color_Filter_Methods">Color Filter Methods</a> | Get and set <a href="bmh_undocumented?cl=9919#Color_Filter">Color Filter</a>. |
| <a href="bmh_SkPaint_Reference?cl=9919#Blend_Mode_Methods">Blend Mode Methods</a> | Get and set <a href="bmh_undocumented?cl=9919#Blend_Mode">Blend Mode</a>. |
| <a href="bmh_SkPaint_Reference?cl=9919#Path_Effect_Methods">Path Effect Methods</a> | Get and set <a href="bmh_undocumented?cl=9919#Path_Effect">Path Effect</a>. |
| <a href="bmh_SkPaint_Reference?cl=9919#Mask_Filter_Methods">Mask Filter Methods</a> | Get and set <a href="bmh_undocumented?cl=9919#Mask_Filter">Mask Filter</a>. |
| <a href="bmh_SkPaint_Reference?cl=9919#Typeface_Methods">Typeface Methods</a> | Get and set <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a>. |
| <a href="bmh_SkPaint_Reference?cl=9919#Rasterizer_Methods">Rasterizer Methods</a> | Get and set <a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a>. |
| <a href="bmh_SkPaint_Reference?cl=9919#Image_Filter_Methods">Image Filter Methods</a> | Get and set <a href="bmh_undocumented?cl=9919#Image_Filter">Image Filter</a>. |
| <a href="bmh_SkPaint_Reference?cl=9919#Draw_Looper_Methods">Draw Looper Methods</a> | Get and set <a href="bmh_undocumented?cl=9919#Draw_Looper">Draw Looper</a>. |
| <a href="bmh_SkPaint_Reference?cl=9919#Text_Align">Text Align</a> | <a href="bmh_undocumented?cl=9919#Text">Text</a> placement relative to position. |
| <a href="bmh_SkPaint_Reference?cl=9919#Text_Size">Text Size</a> | Overall height in points. |
| <a href="bmh_SkPaint_Reference?cl=9919#Text_Scale_X">Text Scale X</a> | <a href="bmh_undocumented?cl=9919#Text">Text</a> horizontal scale. |
| <a href="bmh_SkPaint_Reference?cl=9919#Text_Skew_X">Text Skew X</a> | <a href="bmh_undocumented?cl=9919#Text">Text</a> horizontal slant. |
| <a href="bmh_SkPaint_Reference?cl=9919#Text_Encoding">Text Encoding</a> | <a href="bmh_undocumented?cl=9919#Text">Text</a> encoded as characters or glyphs. |
| <a href="bmh_SkPaint_Reference?cl=9919#Font_Metrics">Font Metrics</a> | Common glyph dimensions. |
| <a href="bmh_SkPaint_Reference?cl=9919#Measure_Text">Measure Text</a> | Width, height, bounds of text. |
| <a href="bmh_SkPaint_Reference?cl=9919#Text_Path">Text Path</a> | Geometry of glyphs. |
| <a href="bmh_SkPaint_Reference?cl=9919#Text_Intercepts">Text Intercepts</a> | Advanced underline, strike through. |
| <a href="bmh_SkPaint_Reference?cl=9919#Fast_Bounds">Fast Bounds</a> | Appproxiate area required by <a href="bmh_SkPaint_Reference?cl=9919#Paint">Paint</a>. |

## <a name="Constants"></a> Constants

| constants | description |
| --- | ---  |
| <a href="bmh_SkPaint_Reference?cl=9919#Align">Align</a> | <a href="bmh_undocumented?cl=9919#Glyph">Glyph</a> locations relative to text position. |
| <a href="bmh_SkPaint_Reference?cl=9919#Cap">Cap</a> | Start and end geometry on stroked shapes. |
| <a href="bmh_SkPaint_Reference?cl=9919#Flags">Flags</a> | Values described by bits and masks. |
| <a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_FontMetricsFlags">FontMetrics::FontMetricsFlags</a> | Valid <a href="bmh_SkPaint_Reference?cl=9919#Font_Metrics">Font Metrics</a>. |
| <a href="bmh_SkPaint_Reference?cl=9919#Hinting">Hinting</a> | Level of glyph outline adjustment. |
| <a href="bmh_SkPaint_Reference?cl=9919#Join">Join</a> | Corner geometry on stroked shapes. |
| <a href="bmh_SkPaint_Reference?cl=9919#Style">Style</a> | Stroke, fill, or both. |
| <a href="bmh_SkPaint_Reference?cl=9919#TextEncoding">TextEncoding</a> | Character or glyph encoding size. |

## <a name="Structs"></a> Structs

| struct | description |
| --- | ---  |
| <a href="bmh_SkPaint_Reference?cl=9919#FontMetrics">FontMetrics</a> | <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a> values. |

## <a name="Constructors"></a> Constructors

|  | description |
| --- | ---  |
| <a href="bmh_SkPaint_Reference?cl=9919#empty_constructor">SkPaint()</a> | Constructs with default values. |
| <a href="bmh_SkPaint_Reference?cl=9919#copy_constructor">SkPaint(const SkPaint& paint)</a> | Makes a shallow copy. |
| <a href="bmh_SkPaint_Reference?cl=9919#move_constructor">SkPaint(SkPaint&& paint)</a> | Moves paint without copying it. |
|  | Decreases <a href="bmh_undocumented?cl=9919#Reference_Count">Reference Count</a> of owned objects. |

## <a name="Operators"></a> Operators

| operator | description |
| --- | ---  |
| <a href="bmh_SkPaint_Reference?cl=9919#copy_assignment_operator">operator=(const SkPaint& paint)</a> | Makes a shallow copy. |
| <a href="bmh_SkPaint_Reference?cl=9919#move_assignment_operator">operator=(SkPaint&& paint)</a> | Moves paint without copying it. |
| <a href="bmh_SkPaint_Reference?cl=9919#equal_operator">operator==(const SkPaint& a, const SkPaint& b)</a> | Compares paints for equality. |
| <a href="bmh_SkPaint_Reference?cl=9919#not_equal_operator">operator!=(const SkPaint& a, const SkPaint& b)</a> | Compares paints for inequality. |

## <a name="Member_Functions"></a> Member Functions

| function | description |
| --- | ---  |
| <a href="bmh_SkPaint_Reference?cl=9919#breakText">breakText</a> | Returns text that fits in a width. |
| <a href="bmh_SkPaint_Reference?cl=9919#canComputeFastBounds">canComputeFastBounds</a> | Returns true if settings allow for fast bounds computation. |
| <a href="bmh_SkPaint_Reference?cl=9919#computeFastBounds">computeFastBounds</a> | Returns fill bounds for quick reject tests. |
| <a href="bmh_SkPaint_Reference?cl=9919#computeFastStrokeBounds">computeFastStrokeBounds</a> | Returns stroke bounds for quick reject tests. |
| <a href="bmh_SkPaint_Reference?cl=9919#containsText">containsText</a> | Returns if all text corresponds to glyphs. |
| <a href="bmh_SkPaint_Reference?cl=9919#countText">countText</a> | Returns number of glyphs in text. |
| <a href="bmh_SkPaint_Reference?cl=9919#doComputeFastBounds">doComputeFastBounds</a> | Returns bounds for quick reject tests. |
| <a href="bmh_SkPaint_Reference?cl=9919#flatten">flatten</a> | Serializes into a buffer. |
| <a href="bmh_SkPaint_Reference?cl=9919#getAlpha">getAlpha</a> | Returns <a href="bmh_undocumented?cl=9919#Alpha">Color Alpha</a>, color opacity. |
| <a href="bmh_SkPaint_Reference?cl=9919#getBlendMode">getBlendMode</a> | Returns <a href="bmh_undocumented?cl=9919#Blend_Mode">Blend Mode</a>, how colors combine with dest. |
| <a href="bmh_SkPaint_Reference?cl=9919#getColor">getColor</a> | Returns <a href="bmh_undocumented?cl=9919#Alpha">Color Alpha</a> and <a href="bmh_undocumented?cl=9919#RGB">Color RGB</a>, one drawing color. |
| <a href="bmh_SkPaint_Reference?cl=9919#getColorFilter">getColorFilter</a> | Returns <a href="bmh_undocumented?cl=9919#Color_Filter">Color Filter</a>, how colors are altered. |
| <a href="bmh_SkPaint_Reference?cl=9919#getDrawLooper">getDrawLooper</a> | Returns <a href="bmh_undocumented?cl=9919#Draw_Looper">Draw Looper</a>, multiple layers. |
| <a href="bmh_SkPaint_Reference?cl=9919#getFillPath">getFillPath</a> | Returns fill path equivalent to stroke. |
| <a href="bmh_SkPaint_Reference?cl=9919#getFilterQuality">getFilterQuality</a> | Returns <a href="bmh_undocumented?cl=9919#Filter_Quality">Filter Quality</a>, image filtering level. |
| <a href="bmh_SkPaint_Reference?cl=9919#getFlags">getFlags</a> | Returns <a href="bmh_SkPaint_Reference?cl=9919#Flags">Flags</a> stored in a bit field. |
| <a href="bmh_SkPaint_Reference?cl=9919#getFontBounds">getFontBounds</a> | Returns union all glyph bounds. |
| <a href="bmh_SkPaint_Reference?cl=9919#getFontMetrics">getFontMetrics</a> | Returns <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a> metrics scaled by text size. |
| <a href="bmh_SkPaint_Reference?cl=9919#getFontSpacing">getFontSpacing</a> | Returns recommended spacing between lines. |
| <a href="bmh_SkPaint_Reference?cl=9919#getHash">getHash</a> | Returns a shallow hash for equality checks. |
| <a href="bmh_SkPaint_Reference?cl=9919#getHinting">getHinting</a> | Returns <a href="bmh_SkPaint_Reference?cl=9919#Hinting">Hinting</a>, glyph outline adjustment level. |
| <a href="bmh_SkPaint_Reference?cl=9919#getImageFilter">getImageFilter</a> | Returns <a href="bmh_undocumented?cl=9919#Image_Filter">Image Filter</a>, alter pixels; blur. |
| <a href="bmh_SkPaint_Reference?cl=9919#getMaskFilter">getMaskFilter</a> | Returns <a href="bmh_undocumented?cl=9919#Mask_Filter">Mask Filter</a>, alterations to <a href="bmh_undocumented?cl=9919#Mask_Alpha">Mask Alpha</a>. |
| <a href="bmh_SkPaint_Reference?cl=9919#getPathEffect">getPathEffect</a> | Returns <a href="bmh_undocumented?cl=9919#Path_Effect">Path Effect</a>, modifications to path geometry; dashing. |
| <a href="bmh_SkPaint_Reference?cl=9919#getPosTextPath">getPosTextPath</a> | Returns <a href="bmh_SkPath_Reference?cl=9919#Path">Path</a> equivalent to positioned text. |
| <a href="bmh_SkPaint_Reference?cl=9919#getPosTextIntercepts">getPosTextIntercepts</a> | Returns where lines intersect positioned text; underlines. |
| <a href="bmh_SkPaint_Reference?cl=9919#getPosTextHIntercepts">getPosTextHIntercepts</a> | Returns where lines intersect horizontally positioned text; underlines. |
| <a href="bmh_SkPaint_Reference?cl=9919#getRasterizer">getRasterizer</a> | Returns <a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a>, <a href="bmh_undocumented?cl=9919#Mask_Alpha">Mask Alpha</a> generation from <a href="bmh_SkPath_Reference?cl=9919#Path">Path</a>. |
| <a href="bmh_SkPaint_Reference?cl=9919#getShader">getShader</a> | Returns <a href="bmh_undocumented?cl=9919#Shader">Shader</a>, multiple drawing colors; gradients. |
| <a href="bmh_SkPaint_Reference?cl=9919#getStrokeCap">getStrokeCap</a> | Returns <a href="bmh_SkPaint_Reference?cl=9919#Cap">Cap</a>, the area drawn at path ends. |
| <a href="bmh_SkPaint_Reference?cl=9919#getStrokeJoin">getStrokeJoin</a> | Returns <a href="bmh_SkPaint_Reference?cl=9919#Join">Join</a>, geometry on path corners. |
| <a href="bmh_SkPaint_Reference?cl=9919#getStrokeMiter">getStrokeMiter</a> | Returns <a href="bmh_SkPaint_Reference?cl=9919#Miter_Limit">Miter Limit</a>, angles with sharp corners. |
| <a href="bmh_SkPaint_Reference?cl=9919#getStrokeWidth">getStrokeWidth</a> | Returns thickness of the stroke. |
| <a href="bmh_SkPaint_Reference?cl=9919#getStyle">getStyle</a> | Returns <a href="bmh_SkPaint_Reference?cl=9919#Style">Style</a>: stroke, fill, or both. |
| <a href="bmh_SkPaint_Reference?cl=9919#getTextAlign">getTextAlign</a> | Returns <a href="bmh_SkPaint_Reference?cl=9919#Align">Align</a>: left, center, or right. |
| <a href="bmh_SkPaint_Reference?cl=9919#getTextBlobIntercepts">getTextBlobIntercepts</a> | Returns where lines intersect <a href="bmh_undocumented?cl=9919#Text_Blob">Text Blob</a>; underlines. |
| <a href="bmh_SkPaint_Reference?cl=9919#getTextEncoding">getTextEncoding</a> | Returns character or glyph encoding size. |
| <a href="bmh_SkPaint_Reference?cl=9919#getTextIntercepts">getTextIntercepts</a> | Returns where lines intersect text; underlines. |
| <a href="bmh_SkPaint_Reference?cl=9919#getTextPath">getTextPath</a> | Returns <a href="bmh_SkPath_Reference?cl=9919#Path">Path</a> equivalent to text. |
| <a href="bmh_SkPaint_Reference?cl=9919#getTextScaleX">getTextScaleX</a> | Returns the text horizontal scale; condensed text. |
| <a href="bmh_SkPaint_Reference?cl=9919#getTextSkewX">getTextSkewX</a> | Returns the text horizontal skew; oblique text. |
| <a href="bmh_SkPaint_Reference?cl=9919#getTextSize">getTextSize</a> | Returns text size in points. |
| <a href="bmh_SkPaint_Reference?cl=9919#getTextWidths">getTextWidths</a> | Returns advance and bounds for each glyph in text. |
| <a href="bmh_SkPaint_Reference?cl=9919#getTypeface">getTypeface</a> | Returns <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a>, font description. |
| <a href="bmh_SkPaint_Reference?cl=9919#glyphsToUnichars">glyphsToUnichars</a> | Converts glyphs into text. |
| <a href="bmh_SkPaint_Reference?cl=9919#isAntiAlias">isAntiAlias</a> | Returns true if <a href="bmh_SkPaint_Reference?cl=9919#Anti_alias">Anti-alias</a> is set. |
| <a href="bmh_SkPaint_Reference?cl=9919#isAutohinted">isAutohinted</a> | Returns true if glyphs are always hinted. |
| <a href="bmh_SkPaint_Reference?cl=9919#isDevKernText">isDevKernText</a> | Returns true if <a href="bmh_SkPaint_Reference?cl=9919#Full_Hinting_Spacing">Full Hinting Spacing</a> is set. |
| <a href="bmh_SkPaint_Reference?cl=9919#isDither">isDither</a> | Returns true if <a href="bmh_SkPaint_Reference?cl=9919#Dither">Dither</a> is set. |
| <a href="bmh_SkPaint_Reference?cl=9919#isEmbeddedBitmapText">isEmbeddedBitmapText</a> | Returns true if <a href="bmh_SkPaint_Reference?cl=9919#Font_Embedded_Bitmaps">Font Embedded Bitmaps</a> is set. |
| <a href="bmh_SkPaint_Reference?cl=9919#isFakeBoldText">isFakeBoldText</a> | Returns true if <a href="bmh_SkPaint_Reference?cl=9919#Fake_Bold">Fake Bold</a> is set. |
| <a href="bmh_SkPaint_Reference?cl=9919#isLCDRenderText">isLCDRenderText</a> | Returns true if <a href="bmh_SkPaint_Reference?cl=9919#LCD_Text">LCD Text</a> is set. |
| <a href="bmh_SkPaint_Reference?cl=9919#isSrcOver">isSrcOver</a> | Returns true if <a href="bmh_undocumented?cl=9919#Blend_Mode">Blend Mode</a> is <a href="bmh_undocumented?cl=9919#kSrcOver">SkBlendMode::kSrcOver</a>. |
| <a href="bmh_SkPaint_Reference?cl=9919#isSubpixelText">isSubpixelText</a> | Returns true if <a href="bmh_SkPaint_Reference?cl=9919#Subpixel_Text">Subpixel Text</a> is set. |
| <a href="bmh_SkPaint_Reference?cl=9919#isVerticalText">isVerticalText</a> | Returns true if <a href="bmh_SkPaint_Reference?cl=9919#Vertical_Text">Vertical Text</a> is set. |
| <a href="bmh_SkPaint_Reference?cl=9919#measureText">measureText</a> | Returns advance width and bounds of text. |
| <a href="bmh_SkPaint_Reference?cl=9919#nothingToDraw">nothingToDraw</a> | Returns true if <a href="bmh_SkPaint_Reference?cl=9919#Paint">Paint</a> prevents all drawing. |
| <a href="bmh_SkPaint_Reference?cl=9919#refColorFilter">refColorFilter</a> | References <a href="bmh_undocumented?cl=9919#Color_Filter">Color Filter</a>, how colors are altered. |
| <a href="bmh_SkPaint_Reference?cl=9919#refDrawLooper">refDrawLooper</a> | References <a href="bmh_undocumented?cl=9919#Draw_Looper">Draw Looper</a>, multiple layers. |
| <a href="bmh_SkPaint_Reference?cl=9919#refImageFilter">refImageFilter</a> | References <a href="bmh_undocumented?cl=9919#Image_Filter">Image Filter</a>, alter pixels; blur. |
| <a href="bmh_SkPaint_Reference?cl=9919#refMaskFilter">refMaskFilter</a> | References <a href="bmh_undocumented?cl=9919#Mask_Filter">Mask Filter</a>, alterations to <a href="bmh_undocumented?cl=9919#Mask_Alpha">Mask Alpha</a>. |
| <a href="bmh_SkPaint_Reference?cl=9919#refPathEffect">refPathEffect</a> | References <a href="bmh_undocumented?cl=9919#Path_Effect">Path Effect</a>, modifications to path geometry; dashing. |
| <a href="bmh_SkPaint_Reference?cl=9919#refRasterizer">refRasterizer</a> | References <a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a>, mask generation from path. |
| <a href="bmh_SkPaint_Reference?cl=9919#refShader">refShader</a> | References <a href="bmh_undocumented?cl=9919#Shader">Shader</a>, multiple drawing colors; gradients. |
| <a href="bmh_SkPaint_Reference?cl=9919#refTypeface">refTypeface</a> | References <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a>, font description. |
| <a href="bmh_SkPaint_Reference?cl=9919#reset">reset</a> | Sets to default values. |
| <a href="bmh_SkPaint_Reference?cl=9919#setAlpha">setAlpha</a> | Sets <a href="bmh_undocumented?cl=9919#Alpha">Color Alpha</a>, color opacity. |
| <a href="bmh_SkPaint_Reference?cl=9919#setAntiAlias">setAntiAlias</a> | Sets or clears <a href="bmh_SkPaint_Reference?cl=9919#Anti_alias">Anti-alias</a>. |
| <a href="bmh_SkPaint_Reference?cl=9919#setARGB">setARGB</a> | Sets color by component. |
| <a href="bmh_SkPaint_Reference?cl=9919#setAutohinted">setAutohinted</a> | Sets glyphs to always be hinted. |
| <a href="bmh_SkPaint_Reference?cl=9919#setBlendMode">setBlendMode</a> | Sets <a href="bmh_undocumented?cl=9919#Blend_Mode">Blend Mode</a>, how colors combine with destination. |
| <a href="bmh_SkPaint_Reference?cl=9919#setColor">setColor</a> | Sets <a href="bmh_undocumented?cl=9919#Alpha">Color Alpha</a> and <a href="bmh_undocumented?cl=9919#RGB">Color RGB</a>, one drawing color. |
| <a href="bmh_SkPaint_Reference?cl=9919#setColorFilter">setColorFilter</a> | Sets <a href="bmh_undocumented?cl=9919#Color_Filter">Color Filter</a>, alters color. |
| <a href="bmh_SkPaint_Reference?cl=9919#setDevKernText">setDevKernText</a> | Sets or clears <a href="bmh_SkPaint_Reference?cl=9919#Full_Hinting_Spacing">Full Hinting Spacing</a>. |
| <a href="bmh_SkPaint_Reference?cl=9919#setDither">setDither</a> | Sets or clears <a href="bmh_SkPaint_Reference?cl=9919#Dither">Dither</a>. |
| <a href="bmh_SkPaint_Reference?cl=9919#setDrawLooper">setDrawLooper</a> | Sets <a href="bmh_undocumented?cl=9919#Draw_Looper">Draw Looper</a>, multiple layers. |
| <a href="bmh_SkPaint_Reference?cl=9919#setEmbeddedBitmapText">setEmbeddedBitmapText</a> | Sets or clears <a href="bmh_SkPaint_Reference?cl=9919#Font_Embedded_Bitmaps">Font Embedded Bitmaps</a>. |
| <a href="bmh_SkPaint_Reference?cl=9919#setFakeBoldText">setFakeBoldText</a> | Sets or clears <a href="bmh_SkPaint_Reference?cl=9919#Fake_Bold">Fake Bold</a>. |
| <a href="bmh_SkPaint_Reference?cl=9919#setFilterQuality">setFilterQuality</a> | Sets <a href="bmh_undocumented?cl=9919#Filter_Quality">Filter Quality</a>, the image filtering level. |
| <a href="bmh_SkPaint_Reference?cl=9919#setFlags">setFlags</a> | Sets multiple <a href="bmh_SkPaint_Reference?cl=9919#Flags">Flags</a> in a bit field. |
| <a href="bmh_SkPaint_Reference?cl=9919#setHinting">setHinting</a> | Sets <a href="bmh_SkPaint_Reference?cl=9919#Hinting">Hinting</a>, glyph outline adjustment level. |
| <a href="bmh_SkPaint_Reference?cl=9919#setLCDRenderText">setLCDRenderText</a> | Sets or clears <a href="bmh_SkPaint_Reference?cl=9919#LCD_Text">LCD Text</a>. |
| <a href="bmh_SkPaint_Reference?cl=9919#setMaskFilter">setMaskFilter</a> | Sets <a href="bmh_undocumented?cl=9919#Mask_Filter">Mask Filter</a>, alterations to <a href="bmh_undocumented?cl=9919#Mask_Alpha">Mask Alpha</a>. |
| <a href="bmh_SkPaint_Reference?cl=9919#setPathEffect">setPathEffect</a> | Sets <a href="bmh_undocumented?cl=9919#Path_Effect">Path Effect</a>, modifications to path geometry; dashing. |
| <a href="bmh_SkPaint_Reference?cl=9919#setRasterizer">setRasterizer</a> | Sets <a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a>, <a href="bmh_undocumented?cl=9919#Mask_Alpha">Mask Alpha</a> generation from <a href="bmh_SkPath_Reference?cl=9919#Path">Path</a>. |
| <a href="bmh_SkPaint_Reference?cl=9919#setImageFilter">setImageFilter</a> | Sets <a href="bmh_undocumented?cl=9919#Image_Filter">Image Filter</a>, alter pixels; blur. |
| <a href="bmh_SkPaint_Reference?cl=9919#setShader">setShader</a> | Sets <a href="bmh_undocumented?cl=9919#Shader">Shader</a>, multiple drawing colors; gradients. |
| <a href="bmh_SkPaint_Reference?cl=9919#setStrokeCap">setStrokeCap</a> | Sets <a href="bmh_SkPaint_Reference?cl=9919#Cap">Cap</a>, the area drawn at path ends. |
| <a href="bmh_SkPaint_Reference?cl=9919#setStrokeJoin">setStrokeJoin</a> | Sets <a href="bmh_SkPaint_Reference?cl=9919#Join">Join</a>, geometry on path corners. |
| <a href="bmh_SkPaint_Reference?cl=9919#setStrokeMiter">setStrokeMiter</a> | Sets <a href="bmh_SkPaint_Reference?cl=9919#Miter_Limit">Miter Limit</a>, angles with sharp corners. |
| <a href="bmh_SkPaint_Reference?cl=9919#setStrokeWidth">setStrokeWidth</a> | Sets thickness of the stroke. |
| <a href="bmh_SkPaint_Reference?cl=9919#setStyle">setStyle</a> | Sets <a href="bmh_SkPaint_Reference?cl=9919#Style">Style</a>: stroke, fill, or both. |
| <a href="bmh_SkPaint_Reference?cl=9919#setSubpixelText">setSubpixelText</a> | Sets or clears <a href="bmh_SkPaint_Reference?cl=9919#Subpixel_Text">Subpixel Text</a>. |
| <a href="bmh_SkPaint_Reference?cl=9919#setTextAlign">setTextAlign</a> | Sets <a href="bmh_SkPaint_Reference?cl=9919#Align">Align</a>: left, center, or right. |
| <a href="bmh_SkPaint_Reference?cl=9919#setTextEncoding">setTextEncoding</a> | Sets character or glyph encoding size. |
| <a href="bmh_SkPaint_Reference?cl=9919#setTextScaleX">setTextScaleX</a> | Sets the text horizontal scale; condensed text. |
| <a href="bmh_SkPaint_Reference?cl=9919#setTextSkewX">setTextSkewX</a> | Sets the text horizontal skew; oblique text. |
| <a href="bmh_SkPaint_Reference?cl=9919#setTextSize">setTextSize</a> | Sets text size in points. |
| <a href="bmh_SkPaint_Reference?cl=9919#setTypeface">setTypeface</a> | Sets <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a>, font description. |
| <a href="bmh_SkPaint_Reference?cl=9919#setVerticalText">setVerticalText</a> | Sets or clears <a href="bmh_SkPaint_Reference?cl=9919#Vertical_Text">Vertical Text</a>. |
| <a href="bmh_SkPaint_Reference?cl=9919#textToGlyphs">textToGlyphs</a> | Converts text into glyph indices. |
| <a href="bmh_SkPaint_Reference?cl=9919#toString">toString</a> | Converts <a href="bmh_SkPaint_Reference?cl=9919#Paint">Paint</a> to machine parsable form (<a href="bmh_undocumented?cl=9919#Developer_Mode">Developer Mode</a>) |
| <a href="bmh_SkPaint_Reference?cl=9919#unflatten">unflatten</a> | Populates from a serialized stream. |

# <a name="Initializers"></a> Initializers

<a name="empty_constructor"></a>
## SkPaint

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkPaint()
</pre>

Constructs <a href="bmh_SkPaint_Reference?cl=9919#Paint">Paint</a> with default values.

| attribute | default value |
| --- | ---  |
| <a href="bmh_SkPaint_Reference?cl=9919#Anti_alias">Anti-alias</a> | false |
| <a href="bmh_undocumented?cl=9919#Blend_Mode">Blend Mode</a> | <a href="bmh_undocumented?cl=9919#kSrcOver">SkBlendMode::kSrcOver</a> |
| <a href="bmh_undocumented?cl=9919#Color">Color</a> | <a href="bmh_undocumented?cl=9919#SK_ColorBLACK">SK ColorBLACK</a> |
| <a href="bmh_undocumented?cl=9919#Alpha">Color Alpha</a> | 255 |
| <a href="bmh_undocumented?cl=9919#Color_Filter">Color Filter</a> | nullptr |
| <a href="bmh_SkPaint_Reference?cl=9919#Dither">Dither</a> | false |
| <a href="bmh_undocumented?cl=9919#Draw_Looper">Draw Looper</a> | nullptr |
| <a href="bmh_SkPaint_Reference?cl=9919#Fake_Bold">Fake Bold</a> | false |
| <a href="bmh_undocumented?cl=9919#Filter_Quality">Filter Quality</a> | <a href="bmh_undocumented?cl=9919#SkFilterQuality">kNone SkFilterQuality</a> |
| <a href="bmh_SkPaint_Reference?cl=9919#Font_Embedded_Bitmaps">Font Embedded Bitmaps</a> | false |
| <a href="bmh_SkPaint_Reference?cl=9919#Automatic_Hinting">Automatic Hinting</a> | false |
| <a href="bmh_SkPaint_Reference?cl=9919#Full_Hinting_Spacing">Full Hinting Spacing</a> | false |
| <a href="bmh_SkPaint_Reference?cl=9919#Hinting">Hinting</a> | <a href="bmh_SkPaint_Reference?cl=9919#kNormal_Hinting">kNormal Hinting</a> |
| <a href="bmh_undocumented?cl=9919#Image_Filter">Image Filter</a> | nullptr |
| <a href="bmh_SkPaint_Reference?cl=9919#LCD_Text">LCD Text</a> | false |
| <a href="bmh_SkPaint_Reference?cl=9919#Linear_Text">Linear Text</a> | false |
| <a href="bmh_SkPaint_Reference?cl=9919#Miter_Limit">Miter Limit</a> | 4 |
| <a href="bmh_undocumented?cl=9919#Mask_Filter">Mask Filter</a> | nullptr |
| <a href="bmh_undocumented?cl=9919#Path_Effect">Path Effect</a> | nullptr |
| <a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a> | nullptr |
| <a href="bmh_undocumented?cl=9919#Shader">Shader</a> | nullptr |
| <a href="bmh_SkPaint_Reference?cl=9919#Style">Style</a> | <a href="bmh_SkPaint_Reference?cl=9919#kFill_Style">kFill Style</a> |
| <a href="bmh_SkPaint_Reference?cl=9919#Text_Align">Text Align</a> | <a href="bmh_SkPaint_Reference?cl=9919#kLeft_Align">kLeft Align</a> |
| <a href="bmh_SkPaint_Reference?cl=9919#Text_Encoding">Text Encoding</a> | <a href="bmh_SkPaint_Reference?cl=9919#kUTF8_TextEncoding">kUTF8 TextEncoding</a> |
| <a href="bmh_SkPaint_Reference?cl=9919#Text_Scale_X">Text Scale X</a> | 1 |
| <a href="bmh_SkPaint_Reference?cl=9919#Text_Size">Text Size</a> | 12 |
| <a href="bmh_SkPaint_Reference?cl=9919#Text_Skew_X">Text Skew X</a> | 0 |
| <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a> | nullptr |
| <a href="bmh_SkPaint_Reference?cl=9919#Stroke_Cap">Stroke Cap</a> | <a href="bmh_SkPaint_Reference?cl=9919#kButt_Cap">kButt Cap</a> |
| <a href="bmh_SkPaint_Reference?cl=9919#Stroke_Join">Stroke Join</a> | <a href="bmh_SkPaint_Reference?cl=9919#kMiter_Join">kMiter Join</a> |
| <a href="bmh_SkPaint_Reference?cl=9919#Stroke_Width">Stroke Width</a> | 0 |
| <a href="bmh_SkPaint_Reference?cl=9919#Subpixel_Text">Subpixel Text</a> | false |
| <a href="bmh_SkPaint_Reference?cl=9919#Vertical_Text">Vertical Text</a> | false |

The flags, text size, hinting, and miter limit may be overridden at compile time by defining
paint default values. The overrides may be included in <a href="bmh_undocumented?cl=9919#SkUserConfig.h">SkUserConfig.h</a> or predefined by the 
build system.

### Return Value

default initialized <a href="bmh_SkPaint_Reference?cl=9919#Paint">Paint</a>

### Example

<div><fiddle-embed name="c4b2186d85c142a481298f7144295ffd"></fiddle-embed></div>

---

<a name="copy_constructor"></a>
## SkPaint

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkPaint(const SkPaint& paint)
</pre>

Makes a shallow copy of <a href="bmh_SkPaint_Reference?cl=9919#Paint">Paint</a>. <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a>, <a href="bmh_undocumented?cl=9919#Path_Effect">Path Effect</a>, <a href="bmh_undocumented?cl=9919#Shader">Shader</a>,
<a href="bmh_undocumented?cl=9919#Mask_Filter">Mask Filter</a>, <a href="bmh_undocumented?cl=9919#Color_Filter">Color Filter</a>, <a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a>, <a href="bmh_undocumented?cl=9919#Draw_Looper">Draw Looper</a>, and <a href="bmh_undocumented?cl=9919#Image_Filter">Image Filter</a> are shared
between the original <a href="bmh_SkPaint_Reference?cl=9919#paint">paint</a> and the copy. These objects' <a href="bmh_undocumented?cl=9919#Reference_Count">Reference Count</a> are increased.

The referenced objects <a href="bmh_undocumented?cl=9919#Path_Effect">Path Effect</a>, <a href="bmh_undocumented?cl=9919#Shader">Shader</a>, <a href="bmh_undocumented?cl=9919#Mask_Filter">Mask Filter</a>, <a href="bmh_undocumented?cl=9919#Color_Filter">Color Filter</a>, <a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a>,
<a href="bmh_undocumented?cl=9919#Draw_Looper">Draw Looper</a>, and <a href="bmh_undocumented?cl=9919#Image_Filter">Image Filter</a> cannot be modified after they are created.
This prevents objects with <a href="bmh_undocumented?cl=9919#Reference_Count">Reference Count</a> from being modified once <a href="bmh_SkPaint_Reference?cl=9919#Paint">Paint</a> refers to them.

### Parameters

<table>  <tr>    <td><code><strong>paint </strong></code></td> <td>
original to copy</td>
  </tr>
</table>

### Return Value

shallow copy of <a href="bmh_SkPaint_Reference?cl=9919#paint">paint</a>

### Example

<div><fiddle-embed name="b99971ad0ef243d617925289d963b62d">

#### Example Output

~~~~
SK_ColorRED == paint1.getColor()
SK_ColorBLUE == paint2.getColor()
~~~~

</fiddle-embed></div>

---

<a name="move_constructor"></a>
## SkPaint

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkPaint(SkPaint&& paint)
</pre>

Implements a move constructor to avoid incrementing the reference counts
of objects referenced by the <a href="bmh_SkPaint_Reference?cl=9919#paint">paint</a>.

After the call, <a href="bmh_SkPaint_Reference?cl=9919#paint">paint</a> is undefined, and can be safely destructed.

### Parameters

<table>  <tr>    <td><code><strong>paint </strong></code></td> <td>
original to move</td>
  </tr>
</table>

### Return Value

content of <a href="bmh_SkPaint_Reference?cl=9919#paint">paint</a>

### Example

<div><fiddle-embed name="8ed1488a503cd5282b86a51614aa90b1">

#### Example Output

~~~~
path effect unique: true
~~~~

</fiddle-embed></div>

---

<a name="reset"></a>
## reset

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void reset()
</pre>

Sets all paint's contents to their initial values. This is equivalent to replacing
the paint with the result of <a href="bmh_SkPaint_Reference?cl=9919#empty_constructor">SkPaint()</a>.

### Example

<div><fiddle-embed name="ef269937ade7e7353635121d9a64f9f7">

#### Example Output

~~~~
paint1 == paint2
~~~~

</fiddle-embed></div>

---

# <a name="Destructor"></a> Destructor

<a name="destructor"></a>
## ~SkPaint

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
~SkPaint()
</pre>

Decreases <a href="bmh_SkPaint_Reference?cl=9919#Paint">Paint</a> <a href="bmh_undocumented?cl=9919#Reference_Count">Reference Count</a> of owned objects: <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a>, <a href="bmh_undocumented?cl=9919#Path_Effect">Path Effect</a>, <a href="bmh_undocumented?cl=9919#Shader">Shader</a>,
<a href="bmh_undocumented?cl=9919#Mask_Filter">Mask Filter</a>, <a href="bmh_undocumented?cl=9919#Color_Filter">Color Filter</a>, <a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a>, <a href="bmh_undocumented?cl=9919#Draw_Looper">Draw Looper</a>, and <a href="bmh_undocumented?cl=9919#Image_Filter">Image Filter</a>. If the
objects' reference count goes to zero, they are deleted.

---

# <a name="Management"></a> Management

<a name="copy_assignment_operator"></a>
## operator=

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkPaint& operator=(const SkPaint& paint)
</pre>

Makes a shallow copy of <a href="bmh_SkPaint_Reference?cl=9919#Paint">Paint</a>. <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a>, <a href="bmh_undocumented?cl=9919#Path_Effect">Path Effect</a>, <a href="bmh_undocumented?cl=9919#Shader">Shader</a>,
<a href="bmh_undocumented?cl=9919#Mask_Filter">Mask Filter</a>, <a href="bmh_undocumented?cl=9919#Color_Filter">Color Filter</a>, <a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a>, <a href="bmh_undocumented?cl=9919#Draw_Looper">Draw Looper</a>, and <a href="bmh_undocumented?cl=9919#Image_Filter">Image Filter</a> are shared
between the original <a href="bmh_SkPaint_Reference?cl=9919#paint">paint</a> and the copy. The objects' <a href="bmh_undocumented?cl=9919#Reference_Count">Reference Count</a> are in the
prior destination are decreased by one, and the referenced objects are deleted if the
resulting count is zero. The objects' <a href="bmh_undocumented?cl=9919#Reference_Count">Reference Count</a> in the parameter <a href="bmh_SkPaint_Reference?cl=9919#paint">paint</a> are increased
by one. <a href="bmh_SkPaint_Reference?cl=9919#paint">paint</a> is unmodified.

### Parameters

<table>  <tr>    <td><code><strong>paint </strong></code></td> <td>
original to copy</td>
  </tr>
</table>

### Return Value

content of <a href="bmh_SkPaint_Reference?cl=9919#paint">paint</a>

### Example

<div><fiddle-embed name="b476a9088f80dece176ed577807d3992">

#### Example Output

~~~~
SK_ColorRED == paint1.getColor()
SK_ColorRED == paint2.getColor()
~~~~

</fiddle-embed></div>

---

<a name="move_assignment_operator"></a>
## operator=

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkPaint& operator=(SkPaint&& paint)
</pre>

Moves the <a href="bmh_SkPaint_Reference?cl=9919#paint">paint</a> to avoid incrementing the reference counts
of objects referenced by the <a href="bmh_SkPaint_Reference?cl=9919#paint">paint</a> parameter. The objects' <a href="bmh_undocumented?cl=9919#Reference_Count">Reference Count</a> are in the
prior destination are decreased by one, and the referenced objects are deleted if the
resulting count is zero.

After the call, <a href="bmh_SkPaint_Reference?cl=9919#paint">paint</a> is undefined, and can be safely destructed.

### Parameters

<table>  <tr>    <td><code><strong>paint </strong></code></td> <td>
original to move</td>
  </tr>
</table>

### Return Value

content of <a href="bmh_SkPaint_Reference?cl=9919#paint">paint</a>

### Example

<div><fiddle-embed name="9fb7459b097d713f5f1fe5675afe14f5">

#### Example Output

~~~~
SK_ColorRED == paint2.getColor()
~~~~

</fiddle-embed></div>

---

<a name="equal_operator"></a>
## operator==

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool operator==(const SkPaint& a, const SkPaint& b)
</pre>

Compares <a href="bmh_SkPaint_Reference?cl=9919#a">a</a> and <a href="bmh_SkPaint_Reference?cl=9919#b">b</a>, and returns true if <a href="bmh_SkPaint_Reference?cl=9919#a">a</a> and <a href="bmh_SkPaint_Reference?cl=9919#b">b</a> are equivalent. May return false
if <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a>, <a href="bmh_undocumented?cl=9919#Path_Effect">Path Effect</a>, <a href="bmh_undocumented?cl=9919#Shader">Shader</a>, <a href="bmh_undocumented?cl=9919#Mask_Filter">Mask Filter</a>, <a href="bmh_undocumented?cl=9919#Color_Filter">Color Filter</a>, <a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a>,
<a href="bmh_undocumented?cl=9919#Draw_Looper">Draw Looper</a>, or <a href="bmh_undocumented?cl=9919#Image_Filter">Image Filter</a> have identical contents but different pointers.

### Parameters

<table>  <tr>    <td><code><strong>a </strong></code></td> <td>
<a href="bmh_SkPaint_Reference?cl=9919#Paint">Paint</a> to compare</td>
  </tr>  <tr>    <td><code><strong>b </strong></code></td> <td>
<a href="bmh_SkPaint_Reference?cl=9919#Paint">Paint</a> to compare</td>
  </tr>
</table>

### Return Value

true if <a href="bmh_SkPaint_Reference?cl=9919#Paint">Paint</a> pair are equivalent

### Example

<div><fiddle-embed name="7481a948e34672720337a631830586dd">

#### Example Output

~~~~
paint1 == paint2
paint1 != paint2
~~~~

</fiddle-embed></div>

---

<a name="not_equal_operator"></a>
## operator!=

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool operator!=(const SkPaint& a, const SkPaint& b)
</pre>

Compares <a href="bmh_SkPaint_Reference?cl=9919#a">a</a> and <a href="bmh_SkPaint_Reference?cl=9919#b">b</a>, and returns true if <a href="bmh_SkPaint_Reference?cl=9919#a">a</a> and <a href="bmh_SkPaint_Reference?cl=9919#b">b</a> are not equivalent. May return true
if <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a>, <a href="bmh_undocumented?cl=9919#Path_Effect">Path Effect</a>, <a href="bmh_undocumented?cl=9919#Shader">Shader</a>, <a href="bmh_undocumented?cl=9919#Mask_Filter">Mask Filter</a>, <a href="bmh_undocumented?cl=9919#Color_Filter">Color Filter</a>, <a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a>,
<a href="bmh_undocumented?cl=9919#Draw_Looper">Draw Looper</a>, or <a href="bmh_undocumented?cl=9919#Image_Filter">Image Filter</a> have identical contents but different pointers.

### Parameters

<table>  <tr>    <td><code><strong>a </strong></code></td> <td>
<a href="bmh_SkPaint_Reference?cl=9919#Paint">Paint</a> to compare</td>
  </tr>  <tr>    <td><code><strong>b </strong></code></td> <td>
<a href="bmh_SkPaint_Reference?cl=9919#Paint">Paint</a> to compare</td>
  </tr>
</table>

### Return Value

true if <a href="bmh_SkPaint_Reference?cl=9919#Paint">Paint</a> pair are not equivalent

### Example

<div><fiddle-embed name="b6c8484b1187f555b435ad5369833be4">

#### Example Output

~~~~
paint1 == paint2
paint1 == paint2
~~~~

</fiddle-embed></div>

---

<a name="getHash"></a>
## getHash

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
uint32_t getHash() const
</pre>

Returns a hash generated from <a href="bmh_SkPaint_Reference?cl=9919#Paint">Paint</a> values and pointers.
Identical hashes guarantee that the paints are
equivalent, but differing hashes do not guarantee that the paints have differing
contents.

If <a href="bmh_SkPaint_Reference?cl=9919#equal_operator">operator==(const SkPaint& a, const SkPaint& b)</a> returns true for two paints,
their hashes are also equal.

The hash returned is platform and implementation specific.

### Return Value

a shallow hash

### Example

<div><fiddle-embed name="7f7e1b701361912b344f90ae6b530393">

#### Example Output

~~~~
paint1 == paint2
paint1.getHash() == paint2.getHash()
~~~~

</fiddle-embed></div>

---

<a name="flatten"></a>
## flatten

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void flatten(SkWriteBuffer& buffer) const
</pre>

Serializes <a href="bmh_SkPaint_Reference?cl=9919#Paint">Paint</a> into a <a href="bmh_SkPaint_Reference?cl=9919#buffer">buffer</a>. A companion <a href="bmh_SkPaint_Reference?cl=9919#unflatten">unflatten</a> call
can reconstitute the paint at a later time.

### Parameters

<table>  <tr>    <td><code><strong>buffer </strong></code></td> <td>
<a href="bmh_undocumented?cl=9919#Write_Buffer">Write Buffer</a> receiving the flattened <a href="bmh_SkPaint_Reference?cl=9919#Paint">Paint</a> data</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="670672b146b50eced4d3dd10c701e0a7">

#### Example Output

~~~~
color = 0xffff0000
~~~~

</fiddle-embed></div>

---

<a name="unflatten"></a>
## unflatten

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void unflatten(SkReadBuffer& buffer)
</pre>

Populates <a href="bmh_SkPaint_Reference?cl=9919#Paint">Paint</a>, typically from a serialized stream, created by calling
<a href="bmh_SkPaint_Reference?cl=9919#flatten">flatten</a> at an earlier time.

<a href="bmh_undocumented?cl=9919#SkReadBuffer">SkReadBuffer</a> class is not public, so <a href="bmh_SkPaint_Reference?cl=9919#unflatten">unflatten</a> cannot be meaningfully called
by the client.

### Parameters

<table>  <tr>    <td><code><strong>buffer </strong></code></td> <td>
serialized data to unflatten</td>
  </tr>
</table>

---

# <a name="Hinting"></a> Hinting

## <a name="SkPaint::Hinting"></a> Enum SkPaint::Hinting

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
enum <a href="bmh_SkPaint_Reference?cl=9919#Hinting">Hinting</a> {
<a href="bmh_SkPaint_Reference?cl=9919#kNo_Hinting">kNo Hinting</a>            = 0,
<a href="bmh_SkPaint_Reference?cl=9919#kSlight_Hinting">kSlight Hinting</a>        = 1,
<a href="bmh_SkPaint_Reference?cl=9919#kNormal_Hinting">kNormal Hinting</a>        = 2,
<a href="bmh_SkPaint_Reference?cl=9919#kFull_Hinting">kFull Hinting</a>          = 3
};</pre>

<a href="bmh_SkPaint_Reference?cl=9919#Hinting">Hinting</a> adjusts the glyph outlines so that the shape provides a uniform
look at a given point size on font engines that support it. <a href="bmh_SkPaint_Reference?cl=9919#Hinting">Hinting</a> may have a
muted effect or no effect at all depending on the platform.

The four levels roughly control corresponding features on platforms that use <a href="bmh_undocumented?cl=9919#FreeType">FreeType</a>
as the <a href="bmh_undocumented?cl=9919#Engine">Font Engine</a>.

### Constants

<table>
  <tr>
    <td><a name="SkPaint::kNo_Hinting"></a> <code><strong>SkPaint::kNo_Hinting </strong></code></td><td>0</td><td>Leaves glyph outlines unchanged from their native representation.
With <a href="bmh_undocumented?cl=9919#FreeType">FreeType</a>, this is equivalent to the <a href="bmh_undocumented?cl=9919#FT_LOAD_NO_HINTING">FT LOAD NO HINTING</a>
bit-field constant supplied to <a href="bmh_undocumented?cl=9919#FT_Load_Glyph">FT Load Glyph</a>, which indicates that the vector
outline being loaded should not be fitted to the pixel grid but simply scaled
to 26.6 fractional pixels.</td>
  </tr>
  <tr>
    <td><a name="SkPaint::kSlight_Hinting"></a> <code><strong>SkPaint::kSlight_Hinting </strong></code></td><td>1</td><td>Modifies glyph outlines minimally to improve constrast.
With <a href="bmh_undocumented?cl=9919#FreeType">FreeType</a>, this is equivalent in spirit to the
<a href="bmh_undocumented?cl=9919#FT_LOAD_TARGET_LIGHT">FT LOAD TARGET LIGHT</a> value supplied to <a href="bmh_undocumented?cl=9919#FT_Load_Glyph">FT Load Glyph</a>. It chooses a 
lighter hinting algorithm for non-monochrome modes.
Generated glyphs may be fuzzy but better resemble their original shape.</td>
  </tr>
  <tr>
    <td><a name="SkPaint::kNormal_Hinting"></a> <code><strong>SkPaint::kNormal_Hinting </strong></code></td><td>2</td><td>Modifies glyph outlines to improve constrast. This is the default.
With <a href="bmh_undocumented?cl=9919#FreeType">FreeType</a>, this supplies <a href="bmh_undocumented?cl=9919#FT_LOAD_TARGET_NORMAL">FT LOAD TARGET NORMAL</a> to <a href="bmh_undocumented?cl=9919#FT_Load_Glyph">FT Load Glyph</a>,
choosing the default hinting algorithm, which is optimized for standard 
gray-level rendering.</td>
  </tr>
  <tr>
    <td><a name="SkPaint::kFull_Hinting"></a> <code><strong>SkPaint::kFull_Hinting </strong></code></td><td>3</td><td>Modifies glyph outlines for maxiumum constrast. With <a href="bmh_undocumented?cl=9919#FreeType">FreeType</a>, this selects
<a href="bmh_undocumented?cl=9919#FT_LOAD_TARGET_LCD">FT LOAD TARGET LCD</a> or <a href="bmh_undocumented?cl=9919#FT_LOAD_TARGET_LCD_V">FT LOAD TARGET LCD V</a> if <a href="bmh_SkPaint_Reference?cl=9919#kLCDRenderText_Flag">kLCDRenderText Flag</a> is set. 
<a href="bmh_undocumented?cl=9919#FT_LOAD_TARGET_LCD">FT LOAD TARGET LCD</a> is a variant of <a href="bmh_undocumented?cl=9919#FT_LOAD_TARGET_NORMAL">FT LOAD TARGET NORMAL</a> optimized for 
horizontally decimated <a href="bmh_undocumented?cl=9919#LCD">LCD</a> displays; <a href="bmh_undocumented?cl=9919#FT_LOAD_TARGET_LCD_V">FT LOAD TARGET LCD V</a> is a 
variant of <a href="bmh_undocumented?cl=9919#FT_LOAD_TARGET_NORMAL">FT LOAD TARGET NORMAL</a> optimized for vertically decimated <a href="bmh_undocumented?cl=9919#LCD">LCD</a> displays.</td>
  </tr>
</table>

On <a href="bmh_undocumented?cl=9919#Windows">Windows</a> with <a href="bmh_undocumented?cl=9919#DirectWrite">DirectWrite</a>, <a href="bmh_SkPaint_Reference?cl=9919#Hinting">Hinting</a> has no effect.

<a href="bmh_SkPaint_Reference?cl=9919#Hinting">Hinting</a> defaults to <a href="bmh_SkPaint_Reference?cl=9919#kNormal_Hinting">kNormal Hinting</a>.
Set <a href="bmh_undocumented?cl=9919#SkPaintDefaults_Hinting">SkPaintDefaults Hinting</a> at compile time to change the default setting.



<a name="getHinting"></a>
## getHinting

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
Hinting getHinting() const
</pre>

Returns level of glyph outline adjustment.

### Return Value

one of: <a href="bmh_SkPaint_Reference?cl=9919#kNo_Hinting">kNo Hinting</a>, <a href="bmh_SkPaint_Reference?cl=9919#kSlight_Hinting">kSlight Hinting</a>, <a href="bmh_SkPaint_Reference?cl=9919#kNormal_Hinting">kNormal Hinting</a>, <a href="bmh_SkPaint_Reference?cl=9919#kFull_Hinting">kFull Hinting</a>

### Example

<div><fiddle-embed name="329e2e5a5919ac431e1c58878a5b99e0">

#### Example Output

~~~~
SkPaint::kNormal_Hinting == paint.getHinting()
~~~~

</fiddle-embed></div>

---

<a name="setHinting"></a>
## setHinting

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setHinting(Hinting hintingLevel)
</pre>

Sets level of glyph outline adjustment.
Does not check for valid values of <a href="bmh_SkPaint_Reference?cl=9919#hintingLevel">hintingLevel</a>.

| <a href="bmh_SkPaint_Reference?cl=9919#Hinting">Hinting</a> | value | effect on generated glyph outlines |
| --- | --- | ---  |
| <a href="bmh_SkPaint_Reference?cl=9919#kNo_Hinting">kNo Hinting</a> | 0 | leaves glyph outlines unchanged from their native representation |
| <a href="bmh_SkPaint_Reference?cl=9919#kSlight_Hinting">kSlight Hinting</a> | 1 | modifies glyph outlines minimally to improve constrast |
| <a href="bmh_SkPaint_Reference?cl=9919#kNormal_Hinting">kNormal Hinting</a> | 2 | modifies glyph outlines to improve constrast |
| <a href="bmh_SkPaint_Reference?cl=9919#kFull_Hinting">kFull Hinting</a> | 3 | modifies glyph outlines for maxiumum constrast |

### Parameters

<table>  <tr>    <td><code><strong>hintingLevel </strong></code></td> <td>
one of: <a href="bmh_SkPaint_Reference?cl=9919#kNo_Hinting">kNo Hinting</a>, <a href="bmh_SkPaint_Reference?cl=9919#kSlight_Hinting">kSlight Hinting</a>, <a href="bmh_SkPaint_Reference?cl=9919#kNormal_Hinting">kNormal Hinting</a>, <a href="bmh_SkPaint_Reference?cl=9919#kFull_Hinting">kFull Hinting</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="78153fbd3f1000cb33b97bbe831ed34e">

#### Example Output

~~~~
paint1 == paint2
~~~~

</fiddle-embed></div>

---

# <a name="Flags"></a> Flags

## <a name="SkPaint::Flags"></a> Enum SkPaint::Flags

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
enum <a href="bmh_SkPaint_Reference?cl=9919#Flags">Flags</a> {
<a href="bmh_SkPaint_Reference?cl=9919#kAntiAlias_Flag">kAntiAlias Flag</a>       = 0x01,
<a href="bmh_SkPaint_Reference?cl=9919#kDither_Flag">kDither Flag</a>          = 0x04,
<a href="bmh_SkPaint_Reference?cl=9919#kFakeBoldText_Flag">kFakeBoldText Flag</a>    = 0x20,
<a href="bmh_SkPaint_Reference?cl=9919#kLinearText_Flag">kLinearText Flag</a>      = 0x40,
<a href="bmh_SkPaint_Reference?cl=9919#kSubpixelText_Flag">kSubpixelText Flag</a>    = 0x80,
<a href="bmh_SkPaint_Reference?cl=9919#kDevKernText_Flag">kDevKernText Flag</a>     = 0x100,
<a href="bmh_SkPaint_Reference?cl=9919#kLCDRenderText_Flag">kLCDRenderText Flag</a>   = 0x200,
<a href="bmh_SkPaint_Reference?cl=9919#kEmbeddedBitmapText_Flag">kEmbeddedBitmapText Flag</a> = 0x400,
<a href="bmh_SkPaint_Reference?cl=9919#kAutoHinting_Flag">kAutoHinting Flag</a>     = 0x800,
<a href="bmh_SkPaint_Reference?cl=9919#kVerticalText_Flag">kVerticalText Flag</a>    = 0x1000,
<a href="bmh_SkPaint_Reference?cl=9919#kGenA8FromLCD_Flag">kGenA8FromLCD Flag</a>    = 0x2000,

<a href="bmh_SkPaint_Reference?cl=9919#kAllFlags">kAllFlags</a> = 0xFFFF,
};
</pre>

The bit values stored in <a href="bmh_SkPaint_Reference?cl=9919#Flags">Flags</a>.
The default value for <a href="bmh_SkPaint_Reference?cl=9919#Flags">Flags</a>, normally zero, can be changed at compile time
with a custom definition of <a href="bmh_undocumented?cl=9919#SkPaintDefaults_Flags">SkPaintDefaults Flags</a>.
All flags can be read and written explicitly; <a href="bmh_SkPaint_Reference?cl=9919#Flags">Flags</a> allows manipulating
multiple settings at once.

### Constants

<table>
  <tr>
    <td><a name="SkPaint::kAntiAlias_Flag"></a> <code><strong>SkPaint::kAntiAlias_Flag </strong></code></td><td>0x0001 </td><td>mask for setting <a href="bmh_SkPaint_Reference?cl=9919#Anti_alias">Anti-alias</a></td>
  </tr>
  <tr>
    <td><a name="SkPaint::kDither_Flag"></a> <code><strong>SkPaint::kDither_Flag </strong></code></td><td>0x0004</td><td>mask for setting <a href="bmh_SkPaint_Reference?cl=9919#Dither">Dither</a></td>
  </tr>
  <tr>
    <td><a name="SkPaint::kFakeBoldText_Flag"></a> <code><strong>SkPaint::kFakeBoldText_Flag </strong></code></td><td>0x0020</td><td>mask for setting <a href="bmh_SkPaint_Reference?cl=9919#Fake_Bold">Fake Bold</a></td>
  </tr>
  <tr>
    <td><a name="SkPaint::kLinearText_Flag"></a> <code><strong>SkPaint::kLinearText_Flag </strong></code></td><td>0x0040</td><td>mask for setting <a href="bmh_SkPaint_Reference?cl=9919#Linear_Text">Linear Text</a></td>
  </tr>
  <tr>
    <td><a name="SkPaint::kSubpixelText_Flag"></a> <code><strong>SkPaint::kSubpixelText_Flag </strong></code></td><td>0x0080</td><td>mask for setting <a href="bmh_SkPaint_Reference?cl=9919#Subpixel_Text">Subpixel Text</a></td>
  </tr>
  <tr>
    <td><a name="SkPaint::kDevKernText_Flag"></a> <code><strong>SkPaint::kDevKernText_Flag </strong></code></td><td>0x0100</td><td>mask for setting <a href="bmh_SkPaint_Reference?cl=9919#Full_Hinting_Spacing">Full Hinting Spacing</a></td>
  </tr>
  <tr>
    <td><a name="SkPaint::kLCDRenderText_Flag"></a> <code><strong>SkPaint::kLCDRenderText_Flag </strong></code></td><td>0x0200</td><td>mask for setting <a href="bmh_SkPaint_Reference?cl=9919#LCD_Text">LCD Text</a></td>
  </tr>
  <tr>
    <td><a name="SkPaint::kEmbeddedBitmapText_Flag"></a> <code><strong>SkPaint::kEmbeddedBitmapText_Flag </strong></code></td><td>0x0400</td><td>mask for setting <a href="bmh_SkPaint_Reference?cl=9919#Font_Embedded_Bitmaps">Font Embedded Bitmaps</a></td>
  </tr>
  <tr>
    <td><a name="SkPaint::kAutoHinting_Flag"></a> <code><strong>SkPaint::kAutoHinting_Flag </strong></code></td><td>0x0800</td><td>mask for setting <a href="bmh_SkPaint_Reference?cl=9919#Automatic_Hinting">Automatic Hinting</a></td>
  </tr>
  <tr>
    <td><a name="SkPaint::kVerticalText_Flag"></a> <code><strong>SkPaint::kVerticalText_Flag </strong></code></td><td>0x1000</td><td>mask for setting <a href="bmh_SkPaint_Reference?cl=9919#Vertical_Text">Vertical Text</a></td>
  </tr>
  <tr>
    <td><a name="SkPaint::kGenA8FromLCD_Flag"></a> <code><strong>SkPaint::kGenA8FromLCD_Flag </strong></code></td><td>0x2000</td><td>not intended for public use</td>
  </tr>
  <tr>
    <td><a name="SkPaint::kAllFlags"></a> <code><strong>SkPaint::kAllFlags </strong></code></td><td>0xFFFF</td><td>mask of all <a href="bmh_SkPaint_Reference?cl=9919#Flags">Flags</a>, including private flags and flags reserved for future use</td>
  </tr>
<a href="bmh_SkPaint_Reference?cl=9919#Flags">Flags</a> default to all flags clear, disabling the associated feature.

</table>

## <a name="SkPaint::ReserveFlags"></a> Enum SkPaint::ReserveFlags

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
enum <a href="bmh_SkPaint_Reference?cl=9919#ReserveFlags">ReserveFlags</a> {
<a href="bmh_SkPaint_Reference?cl=9919#kUnderlineText_ReserveFlag">kUnderlineText ReserveFlag</a>   = 0x08,
<a href="bmh_SkPaint_Reference?cl=9919#kStrikeThruText_ReserveFlag">kStrikeThruText ReserveFlag</a>  = 0x10,
};</pre>

### Constants

<table>
  <tr>
    <td><a name="SkPaint::kUnderlineText_ReserveFlag"></a> <code><strong>SkPaint::kUnderlineText_ReserveFlag </strong></code></td><td>0x0008</td><td>mask for underline text</td>
  </tr>
  <tr>
    <td><a name="SkPaint::kStrikeThruText_ReserveFlag"></a> <code><strong>SkPaint::kStrikeThruText_ReserveFlag </strong></code></td><td>0x0010</td><td>mask for strike-thru text</td>
  </tr>
</table>



<a name="getFlags"></a>
## getFlags

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
uint32_t getFlags() const
</pre>

Returns paint settings described by <a href="bmh_SkPaint_Reference?cl=9919#Flags">Flags</a>. Each setting uses one
bit, and can be tested with <a href="bmh_SkPaint_Reference?cl=9919#Flags">Flags</a> members.

### Return Value

zero, one, or more bits described by <a href="bmh_SkPaint_Reference?cl=9919#Flags">Flags</a>

### Example

<div><fiddle-embed name="8a3f8c309533388b01aa66e1267f322d">

#### Example Output

~~~~
(SkPaint::kAntiAlias_Flag & paint.getFlags()) != 0
~~~~

</fiddle-embed></div>

---

<a name="setFlags"></a>
## setFlags

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setFlags(uint32_t flags)
</pre>

Replaces <a href="bmh_SkPaint_Reference?cl=9919#Flags">Flags</a> with <a href="bmh_SkPaint_Reference?cl=9919#flags">flags</a>, the union of the <a href="bmh_SkPaint_Reference?cl=9919#Flags">Flags</a> members.
All <a href="bmh_SkPaint_Reference?cl=9919#Flags">Flags</a> members may be cleared, or one or more may be set.

### Parameters

<table>  <tr>    <td><code><strong>flags </strong></code></td> <td>
union of <a href="bmh_SkPaint_Reference?cl=9919#Flags">Flags</a> for <a href="bmh_SkPaint_Reference?cl=9919#Paint">Paint</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="54baed3f6bc4b9c31ba664e27767fdc7">

#### Example Output

~~~~
paint.isAntiAlias()
paint.isDither()
~~~~

</fiddle-embed></div>

---

# <a name="Anti-alias"></a> Anti-alias
<a href="bmh_SkPaint_Reference?cl=9919#Anti_alias">Anti-alias</a> drawing approximates partial pixel coverage with transparency.
If <a href="bmh_SkPaint_Reference?cl=9919#kAntiAlias_Flag">kAntiAlias Flag</a> is clear, pixel centers contained by the shape edge are drawn opaque.
If <a href="bmh_SkPaint_Reference?cl=9919#kAntiAlias_Flag">kAntiAlias Flag</a> is set, pixels are drawn with <a href="bmh_undocumented?cl=9919#Alpha">Color Alpha</a> equal to their coverage.

The rule for aliased pixels is inconsistent across platforms. A shape edge 
passing through the pixel center may, but is not required to, draw the pixel.

<a href="bmh_undocumented?cl=9919#Raster_Engine">Raster Engine</a> draws aliased pixels whose centers are on or to the right of the start of an
active <a href="bmh_SkPath_Reference?cl=9919#Path">Path</a> edge, and whose center is to the left of the end of the active <a href="bmh_SkPath_Reference?cl=9919#Path">Path</a> edge.

A platform may only support anti-aliased drawing. Some <a href="bmh_undocumented?cl=9919#GPU_backed">GPU-backed</a> platforms use
supersampling to anti-alias all drawing, and have no mechanism to selectively
alias.

The amount of coverage computed for anti-aliased pixels also varies across platforms.

<a href="bmh_SkPaint_Reference?cl=9919#Anti_alias">Anti-alias</a> is disabled by default.
<a href="bmh_SkPaint_Reference?cl=9919#Anti_alias">Anti-alias</a> can be enabled by default by setting <a href="bmh_undocumented?cl=9919#SkPaintDefaults_Flags">SkPaintDefaults Flags</a> to <a href="bmh_SkPaint_Reference?cl=9919#kAntiAlias_Flag">kAntiAlias Flag</a>
at compile time.

### Example

<div><fiddle-embed name="a6575a49467ce8d28bb01cc7638fa04d"><div>A red line is drawn with transparency on the edges to make it look smoother.
A blue line draws only where the pixel centers are contained.
The lines are drawn into an offscreen bitmap, then drawn magified to make the
aliasing easier to see.</div></fiddle-embed></div>

<a name="isAntiAlias"></a>
## isAntiAlias

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool isAntiAlias() const
</pre>

If true, pixels on the active edges of <a href="bmh_SkPath_Reference?cl=9919#Path">Path</a> may be drawn with partial transparency.

Equivalent to <a href="bmh_SkPaint_Reference?cl=9919#getFlags">getFlags</a> masked with <a href="bmh_SkPaint_Reference?cl=9919#kAntiAlias_Flag">kAntiAlias Flag</a>.

### Return Value

<a href="bmh_SkPaint_Reference?cl=9919#kAntiAlias_Flag">kAntiAlias Flag</a> state

### Example

<div><fiddle-embed name="d7d5f4f7da7acd5104a652f490c6f7b8">

#### Example Output

~~~~
paint.isAntiAlias() == !!(paint.getFlags() & SkPaint::kAntiAlias_Flag)
paint.isAntiAlias() == !!(paint.getFlags() & SkPaint::kAntiAlias_Flag)
~~~~

</fiddle-embed></div>

---

<a name="setAntiAlias"></a>
## setAntiAlias

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setAntiAlias(bool aa)
</pre>

Requests, but does not require, that <a href="bmh_SkPath_Reference?cl=9919#Path">Path</a> edge pixels draw opaque or with
partial transparency.

Sets <a href="bmh_SkPaint_Reference?cl=9919#kAntiAlias_Flag">kAntiAlias Flag</a> if <a href="bmh_SkPaint_Reference?cl=9919#aa">aa</a> is true.
Clears <a href="bmh_SkPaint_Reference?cl=9919#kAntiAlias_Flag">kAntiAlias Flag</a> if <a href="bmh_SkPaint_Reference?cl=9919#aa">aa</a> is false.

### Parameters

<table>  <tr>    <td><code><strong>aa </strong></code></td> <td>
setting for <a href="bmh_SkPaint_Reference?cl=9919#kAntiAlias_Flag">kAntiAlias Flag</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="c2ff148374d01cbef845b223e725905c">

#### Example Output

~~~~
paint1 == paint2
~~~~

</fiddle-embed></div>

---

# <a name="Dither"></a> Dither
<a href="bmh_SkPaint_Reference?cl=9919#Dither">Dither</a> increases fidelity by adjusting the color of adjcent pixels. 
This can help to smooth color transitions and reducing banding in gradients.
Dithering lessens visible banding from <a href="bmh_undocumented?cl=9919#SkColorType">kRGB 565 SkColorType</a>
and <a href="bmh_undocumented?cl=9919#SkColorType">kRGBA 8888 SkColorType</a> gradients, 
and improves rendering into a <a href="bmh_undocumented?cl=9919#SkColorType">kRGB 565 SkColorType</a> <a href="bmh_undocumented?cl=9919#Surface">Surface</a>.

Dithering is always enabled for linear gradients drawing into
<a href="bmh_undocumented?cl=9919#SkColorType">kRGB 565 SkColorType</a> <a href="bmh_undocumented?cl=9919#Surface">Surface</a> and <a href="bmh_undocumented?cl=9919#SkColorType">kRGBA 8888 SkColorType</a> <a href="bmh_undocumented?cl=9919#Surface">Surface</a>.
<a href="bmh_SkPaint_Reference?cl=9919#Dither">Dither</a> cannot be enabled for <a href="bmh_undocumented?cl=9919#SkColorType">kAlpha 8 SkColorType</a> <a href="bmh_undocumented?cl=9919#Surface">Surface</a> and
<a href="bmh_undocumented?cl=9919#SkColorType">kRGBA F16 SkColorType</a> <a href="bmh_undocumented?cl=9919#Surface">Surface</a>.

<a href="bmh_SkPaint_Reference?cl=9919#Dither">Dither</a> is disabled by default.
<a href="bmh_SkPaint_Reference?cl=9919#Dither">Dither</a> can be enabled by default by setting <a href="bmh_undocumented?cl=9919#SkPaintDefaults_Flags">SkPaintDefaults Flags</a> to <a href="bmh_SkPaint_Reference?cl=9919#kDither_Flag">kDither Flag</a>
at compile time.

Some platform implementations may ignore dithering. Setto ignore <a href="bmh_SkPaint_Reference?cl=9919#Dither">Dither</a> on <a href="bmh_undocumented?cl=9919#GPU">GPU Surface</a>.

### Example

<div><fiddle-embed name="8b26507690b71462f44642b911890bbf"><div>Dithering in the bottom half more closely approximates the requested color by
alternating nearby colors from pixel to pixel.</div></fiddle-embed></div>

### Example

<div><fiddle-embed name="76d4d4a7931a48495e4d5f54e073be53"><div>Dithering introduces subtle adjustments to color to smooth gradients.
Drawing the gradient repeatedly with <a href="bmh_undocumented?cl=9919#kPlus">SkBlendMode::kPlus</a> exaggerates the
dither, making it easier to see.</div></fiddle-embed></div>

<a name="isDither"></a>
## isDither

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool isDither() const
</pre>

If true, color error may be distributed to smooth color transition.
Equivalent to <a href="bmh_SkPaint_Reference?cl=9919#getFlags">getFlags</a> masked with <a href="bmh_SkPaint_Reference?cl=9919#kDither_Flag">kDither Flag</a>.

### Return Value

<a href="bmh_SkPaint_Reference?cl=9919#kDither_Flag">kDither Flag</a> state

### Example

<div><fiddle-embed name="f4ce93f6c5e7335436a985377fd980c0">

#### Example Output

~~~~
paint.isDither() == !!(paint.getFlags() & SkPaint::kDither_Flag)
paint.isDither() == !!(paint.getFlags() & SkPaint::kDither_Flag)
~~~~

</fiddle-embed></div>

---

<a name="setDither"></a>
## setDither

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setDither(bool dither)
</pre>

Requests, but does not require, to distribute color error.

Sets <a href="bmh_SkPaint_Reference?cl=9919#kDither_Flag">kDither Flag</a> if <a href="bmh_SkPaint_Reference?cl=9919#dither">dither</a> is true.
Clears <a href="bmh_SkPaint_Reference?cl=9919#kDither_Flag">kDither Flag</a> if <a href="bmh_SkPaint_Reference?cl=9919#dither">dither</a> is false.

### Parameters

<table>  <tr>    <td><code><strong>dither </strong></code></td> <td>
setting for <a href="bmh_SkPaint_Reference?cl=9919#kDither_Flag">kDither Flag</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="69b7162e8324d9239dd02dd9ada2bdff">

#### Example Output

~~~~
paint1 == paint2
~~~~

</fiddle-embed></div>

### See Also

<a href="bmh_undocumented?cl=9919#SkColorType">kRGB 565 SkColorType</a>

---

### See Also

Gradient <a href="bmh_undocumented?cl=9919#RGB_565">Color RGB-565</a>

# <a name="Device_Text"></a> Device Text
<a href="bmh_SkPaint_Reference?cl=9919#LCD_Text">LCD Text</a> and <a href="bmh_SkPaint_Reference?cl=9919#Subpixel_Text">Subpixel Text</a> increase the precision of glyph position.

When set, <a href="bmh_SkPaint_Reference?cl=9919#Flags">Flags</a> <a href="bmh_SkPaint_Reference?cl=9919#kLCDRenderText_Flag">kLCDRenderText Flag</a> takes advantage of the organization of <a href="bmh_undocumented?cl=9919#RGB">Color RGB</a> stripes that 
create a color, and relies
on the small size of the stripe and visual perception to make the color fringing inperceptible.
<a href="bmh_SkPaint_Reference?cl=9919#LCD_Text">LCD Text</a> can be enabled on devices that orient stripes horizontally or vertically, and that order
the color components as <a href="bmh_undocumented?cl=9919#RGB">Color RGB</a> or <a href="bmh_undocumented?cl=9919#RBG">Color RBG</a>.

<a href="bmh_SkPaint_Reference?cl=9919#Flags">Flags</a> <a href="bmh_SkPaint_Reference?cl=9919#kSubpixelText_Flag">kSubpixelText Flag</a> uses the pixel transparency to represent a fractional offset. 
As the opaqueness
of the color increases, the edge of the glyph appears to move towards the outside of the pixel.

Either or both techniques can be enabled.
<a href="bmh_SkPaint_Reference?cl=9919#kLCDRenderText_Flag">kLCDRenderText Flag</a> and <a href="bmh_SkPaint_Reference?cl=9919#kSubpixelText_Flag">kSubpixelText Flag</a> are clear by default.
<a href="bmh_SkPaint_Reference?cl=9919#LCD_Text">LCD Text</a> or <a href="bmh_SkPaint_Reference?cl=9919#Subpixel_Text">Subpixel Text</a> can be enabled by default by setting <a href="bmh_undocumented?cl=9919#SkPaintDefaults_Flags">SkPaintDefaults Flags</a> to 
<a href="bmh_SkPaint_Reference?cl=9919#kLCDRenderText_Flag">kLCDRenderText Flag</a> or <a href="bmh_SkPaint_Reference?cl=9919#kSubpixelText_Flag">kSubpixelText Flag</a> (or both) at compile time.

### Example

<div><fiddle-embed name="4606ae1be792d6bc46d496432f050ee9"><div>Four commas are drawn normally and with combinations of <a href="bmh_SkPaint_Reference?cl=9919#LCD_Text">LCD Text</a> and <a href="bmh_SkPaint_Reference?cl=9919#Subpixel_Text">Subpixel Text</a>.
When <a href="bmh_SkPaint_Reference?cl=9919#Subpixel_Text">Subpixel Text</a> is disabled, the comma glyphs are indentical, but not evenly spaced.
When <a href="bmh_SkPaint_Reference?cl=9919#Subpixel_Text">Subpixel Text</a> is enabled, the comma glyphs are unique, but appear evenly spaced.</div></fiddle-embed></div>

## <a name="Linear_Text"></a> Linear Text

<a href="bmh_SkPaint_Reference?cl=9919#Linear_Text">Linear Text</a> selects whether text is rendered as a <a href="bmh_undocumented?cl=9919#Glyph">Glyph</a> or as a <a href="bmh_SkPath_Reference?cl=9919#Path">Path</a>.
If <a href="bmh_SkPaint_Reference?cl=9919#kLinearText_Flag">kLinearText Flag</a> is set, it has the same effect as setting <a href="bmh_SkPaint_Reference?cl=9919#Hinting">Hinting</a> to <a href="bmh_SkPaint_Reference?cl=9919#kNormal_Hinting">kNormal Hinting</a>.
If <a href="bmh_SkPaint_Reference?cl=9919#kLinearText_Flag">kLinearText Flag</a> is clear, it's the same as setting <a href="bmh_SkPaint_Reference?cl=9919#Hinting">Hinting</a> to <a href="bmh_SkPaint_Reference?cl=9919#kNo_Hinting">kNo Hinting</a>.

<a name="isLinearText"></a>
## isLinearText

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool isLinearText() const
</pre>

If true, text is converted to <a href="bmh_SkPath_Reference?cl=9919#Path">Path</a> before drawing and measuring.

Equivalent to <a href="bmh_SkPaint_Reference?cl=9919#getFlags">getFlags</a> masked with <a href="bmh_SkPaint_Reference?cl=9919#kLinearText_Flag">kLinearText Flag</a>.

### Return Value

<a href="bmh_SkPaint_Reference?cl=9919#kLinearText_Flag">kLinearText Flag</a> state

### Example

<div><fiddle-embed name="2890ad644f980637837e6fcb386fb462"></fiddle-embed></div>

### See Also

<a href="bmh_SkPaint_Reference?cl=9919#setLinearText">setLinearText</a> <a href="bmh_SkPaint_Reference?cl=9919#Hinting">Hinting</a>

---

<a name="setLinearText"></a>
## setLinearText

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setLinearText(bool linearText)
</pre>

If true, text is converted to <a href="bmh_SkPath_Reference?cl=9919#Path">Path</a> before drawing and measuring.
By default, <a href="bmh_SkPaint_Reference?cl=9919#kLinearText_Flag">kLinearText Flag</a> is clear.

Sets <a href="bmh_SkPaint_Reference?cl=9919#kLinearText_Flag">kLinearText Flag</a> if <a href="bmh_SkPaint_Reference?cl=9919#linearText">linearText</a> is true.
Clears <a href="bmh_SkPaint_Reference?cl=9919#kLinearText_Flag">kLinearText Flag</a> if <a href="bmh_SkPaint_Reference?cl=9919#linearText">linearText</a> is false.

### Parameters

<table>  <tr>    <td><code><strong>linearText </strong></code></td> <td>
setting for <a href="bmh_SkPaint_Reference?cl=9919#kLinearText_Flag">kLinearText Flag</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="c93bb912f3bddfb4d96d3ad70ada552b"></fiddle-embed></div>

### See Also

<a href="bmh_SkPaint_Reference?cl=9919#isLinearText">isLinearText</a> <a href="bmh_SkPaint_Reference?cl=9919#Hinting">Hinting</a>

---

## <a name="Subpixel_Text"></a> Subpixel Text

<a href="bmh_SkPaint_Reference?cl=9919#Flags">Flags</a> <a href="bmh_SkPaint_Reference?cl=9919#kSubpixelText_Flag">kSubpixelText Flag</a> uses the pixel transparency to represent a fractional offset. 
As the opaqueness
of the color increases, the edge of the glyph appears to move towards the outside of the pixel.

<a name="isSubpixelText"></a>
## isSubpixelText

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool isSubpixelText() const
</pre>

If true, glyphs at different sub-pixel positions may differ on pixel edge coverage.

Equivalent to <a href="bmh_SkPaint_Reference?cl=9919#getFlags">getFlags</a> masked with <a href="bmh_SkPaint_Reference?cl=9919#kSubpixelText_Flag">kSubpixelText Flag</a>.

### Return Value

<a href="bmh_SkPaint_Reference?cl=9919#kSubpixelText_Flag">kSubpixelText Flag</a> state

### Example

<div><fiddle-embed name="abe9afc0932e2199324ae6cbb396e67c">

#### Example Output

~~~~
paint.isSubpixelText() == !!(paint.getFlags() & SkPaint::kSubpixelText_Flag)
paint.isSubpixelText() == !!(paint.getFlags() & SkPaint::kSubpixelText_Flag)
~~~~

</fiddle-embed></div>

---

<a name="setSubpixelText"></a>
## setSubpixelText

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setSubpixelText(bool subpixelText)
</pre>

Requests, but does not require, that glyphs respect sub-pixel positioning.

Sets <a href="bmh_SkPaint_Reference?cl=9919#kSubpixelText_Flag">kSubpixelText Flag</a> if <a href="bmh_SkPaint_Reference?cl=9919#subpixelText">subpixelText</a> is true.
Clears <a href="bmh_SkPaint_Reference?cl=9919#kSubpixelText_Flag">kSubpixelText Flag</a> if <a href="bmh_SkPaint_Reference?cl=9919#subpixelText">subpixelText</a> is false.

### Parameters

<table>  <tr>    <td><code><strong>subpixelText </strong></code></td> <td>
setting for <a href="bmh_SkPaint_Reference?cl=9919#kSubpixelText_Flag">kSubpixelText Flag</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="a77bbc1a4e3be9a8ab0f842f877c5ee4">

#### Example Output

~~~~
paint1 == paint2
~~~~

</fiddle-embed></div>

---

## <a name="LCD_Text"></a> LCD Text

When set, <a href="bmh_SkPaint_Reference?cl=9919#Flags">Flags</a> <a href="bmh_SkPaint_Reference?cl=9919#kLCDRenderText_Flag">kLCDRenderText Flag</a> takes advantage of the organization of <a href="bmh_undocumented?cl=9919#RGB">Color RGB</a> stripes that 
create a color, and relies
on the small size of the stripe and visual perception to make the color fringing inperceptible.
<a href="bmh_SkPaint_Reference?cl=9919#LCD_Text">LCD Text</a> can be enabled on devices that orient stripes horizontally or vertically, and that order
the color components as <a href="bmh_undocumented?cl=9919#RGB">Color RGB</a> or <a href="bmh_undocumented?cl=9919#RBG">Color RBG</a>.

<a name="isLCDRenderText"></a>
## isLCDRenderText

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool isLCDRenderText() const
</pre>

If true, glyphs may use <a href="bmh_undocumented?cl=9919#LCD">LCD</a> striping to improve glyph edges.

Returns true if <a href="bmh_SkPaint_Reference?cl=9919#Flags">Flags</a> <a href="bmh_SkPaint_Reference?cl=9919#kLCDRenderText_Flag">kLCDRenderText Flag</a> is set.

### Return Value

<a href="bmh_SkPaint_Reference?cl=9919#kLCDRenderText_Flag">kLCDRenderText Flag</a> state

### Example

<div><fiddle-embed name="68e1fd95dd2fd06a333899d2bd2396b9">

#### Example Output

~~~~
paint.isLCDRenderText() == !!(paint.getFlags() & SkPaint::kLCDRenderText_Flag)
paint.isLCDRenderText() == !!(paint.getFlags() & SkPaint::kLCDRenderText_Flag)
~~~~

</fiddle-embed></div>

---

<a name="setLCDRenderText"></a>
## setLCDRenderText

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setLCDRenderText(bool lcdText)
</pre>

Requests, but does not require, that glyphs use <a href="bmh_undocumented?cl=9919#LCD">LCD</a> striping for glyph edges.

Sets <a href="bmh_SkPaint_Reference?cl=9919#kLCDRenderText_Flag">kLCDRenderText Flag</a> if <a href="bmh_SkPaint_Reference?cl=9919#lcdText">lcdText</a> is true.
Clears <a href="bmh_SkPaint_Reference?cl=9919#kLCDRenderText_Flag">kLCDRenderText Flag</a> if <a href="bmh_SkPaint_Reference?cl=9919#lcdText">lcdText</a> is false.

### Parameters

<table>  <tr>    <td><code><strong>lcdText </strong></code></td> <td>
setting for <a href="bmh_SkPaint_Reference?cl=9919#kLCDRenderText_Flag">kLCDRenderText Flag</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="50dedf8450159571a3edaf4f0050defe">

#### Example Output

~~~~
paint1 == paint2
~~~~

</fiddle-embed></div>

---

# <a name="Font_Embedded_Bitmaps"></a> Font Embedded Bitmaps
<a href="bmh_SkPaint_Reference?cl=9919#Font_Embedded_Bitmaps">Font Embedded Bitmaps</a> allows selecting custom-sized bitmap glyphs.
<a href="bmh_SkPaint_Reference?cl=9919#Flags">Flags</a> <a href="bmh_SkPaint_Reference?cl=9919#kEmbeddedBitmapText_Flag">kEmbeddedBitmapText Flag</a> when set chooses an embedded bitmap glyph over an outline contained
in a font if the platform supports this option. 

<a href="bmh_undocumented?cl=9919#FreeType">FreeType</a> selects the bitmap glyph if available when <a href="bmh_SkPaint_Reference?cl=9919#kEmbeddedBitmapText_Flag">kEmbeddedBitmapText Flag</a> is set, and selects
the outline glyph if <a href="bmh_SkPaint_Reference?cl=9919#kEmbeddedBitmapText_Flag">kEmbeddedBitmapText Flag</a> is clear.
<a href="bmh_undocumented?cl=9919#Windows">Windows</a> may select the bitmap glyph but is not required to do so.
<a href="bmh_undocumented?cl=9919#OS_X">OS X</a> and <a href="bmh_undocumented?cl=9919#iOS">iOS</a> do not support this option.

<a href="bmh_SkPaint_Reference?cl=9919#Font_Embedded_Bitmaps">Font Embedded Bitmaps</a> is disabled by default.
<a href="bmh_SkPaint_Reference?cl=9919#Font_Embedded_Bitmaps">Font Embedded Bitmaps</a> can be enabled by default by setting <a href="bmh_undocumented?cl=9919#SkPaintDefaults_Flags">SkPaintDefaults Flags</a> to
<a href="bmh_SkPaint_Reference?cl=9919#kEmbeddedBitmapText_Flag">kEmbeddedBitmapText Flag</a> at compile time.

### Example

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
!fiddle<div>The hintgasp <a href="bmh_undocumented?cl=9919#TrueType">TrueType</a> font in the <a href="bmh_undocumented?cl=9919#Skia">Skia</a> resources/fonts directory includes an embedded
bitmap glyph at odd font sizes. This example works on platforms that use <a href="bmh_undocumented?cl=9919#FreeType">FreeType</a>
as their <a href="bmh_undocumented?cl=9919#Engine">Font Engine</a>.
<a href="bmh_undocumented?cl=9919#Windows">Windows</a> may, but is not required to, return a bitmap glyph if <a href="bmh_SkPaint_Reference?cl=9919#kEmbeddedBitmapText_Flag">kEmbeddedBitmapText Flag</a> is set.</div><a href="bmh_undocumented?cl=9919#SkBitmap">SkBitmap</a> bitmap;
bitmap.allocN32Pixels(30, 15);
bitmap.eraseColor(0);
<a href="bmh_SkCanvas_Reference?cl=9919#SkCanvas">SkCanvas</a> offscreen(bitmap);
<a href="bmh_SkPaint_Reference?cl=9919#SkPaint">SkPaint</a> paint;
paint.</pre>

<a name="isEmbeddedBitmapText"></a>
## isEmbeddedBitmapText

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool isEmbeddedBitmapText() const
</pre>

If true, <a href="bmh_undocumented?cl=9919#Engine">Font Engine</a> may return glyphs from font bitmaps instead of from outlines.

Equivalent to <a href="bmh_SkPaint_Reference?cl=9919#getFlags">getFlags</a> masked with <a href="bmh_SkPaint_Reference?cl=9919#kEmbeddedBitmapText_Flag">kEmbeddedBitmapText Flag</a>.

### Return Value

<a href="bmh_SkPaint_Reference?cl=9919#kEmbeddedBitmapText_Flag">kEmbeddedBitmapText Flag</a> state

### Example

<div><fiddle-embed name="eba10b27b790e87183ae451b3fc5c4b1">

#### Example Output

~~~~
paint.isEmbeddedBitmapText() == !!(paint.getFlags() & SkPaint::kEmbeddedBitmapText_Flag)
paint.isEmbeddedBitmapText() == !!(paint.getFlags() & SkPaint::kEmbeddedBitmapText_Flag)
~~~~

</fiddle-embed></div>

---

<a name="setEmbeddedBitmapText"></a>
## setEmbeddedBitmapText

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setEmbeddedBitmapText(bool useEmbeddedBitmapText)
</pre>

Requests, but does not require, to use bitmaps in fonts instead of outlines.

Sets <a href="bmh_SkPaint_Reference?cl=9919#kEmbeddedBitmapText_Flag">kEmbeddedBitmapText Flag</a> if <a href="bmh_SkPaint_Reference?cl=9919#useEmbeddedBitmapText">useEmbeddedBitmapText</a> is true.
Clears <a href="bmh_SkPaint_Reference?cl=9919#kEmbeddedBitmapText_Flag">kEmbeddedBitmapText Flag</a> if <a href="bmh_SkPaint_Reference?cl=9919#useEmbeddedBitmapText">useEmbeddedBitmapText</a> is false.

### Parameters

<table>  <tr>    <td><code><strong>useEmbeddedBitmapText </strong></code></td> <td>
setting for <a href="bmh_SkPaint_Reference?cl=9919#kEmbeddedBitmapText_Flag">kEmbeddedBitmapText Flag</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="246dffdd93a484ba4ad7ecf71198a5d4">

#### Example Output

~~~~
paint1 == paint2
~~~~

</fiddle-embed></div>

---

# <a name="Automatic_Hinting"></a> Automatic Hinting
If <a href="bmh_SkPaint_Reference?cl=9919#Hinting">Hinting</a> is set to <a href="bmh_SkPaint_Reference?cl=9919#kNormal_Hinting">kNormal Hinting</a> or <a href="bmh_SkPaint_Reference?cl=9919#kFull_Hinting">kFull Hinting</a>, <a href="bmh_SkPaint_Reference?cl=9919#Automatic_Hinting">Automatic Hinting</a>
instructs the <a href="bmh_undocumented?cl=9919#Font_Manager">Font Manager</a> to always hint Glyphs.
<a href="bmh_SkPaint_Reference?cl=9919#Automatic_Hinting">Automatic Hinting</a> has no effect if <a href="bmh_SkPaint_Reference?cl=9919#Hinting">Hinting</a> is set to <a href="bmh_SkPaint_Reference?cl=9919#kNo_Hinting">kNo Hinting</a> or
<a href="bmh_SkPaint_Reference?cl=9919#kSlight_Hinting">kSlight Hinting</a>.

<a href="bmh_SkPaint_Reference?cl=9919#Automatic_Hinting">Automatic Hinting</a> only affects platforms that use <a href="bmh_undocumented?cl=9919#FreeType">FreeType</a> as the <a href="bmh_undocumented?cl=9919#Font_Manager">Font Manager</a>.

<a name="isAutohinted"></a>
## isAutohinted

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool isAutohinted() const
</pre>

If true, and if <a href="bmh_SkPaint_Reference?cl=9919#Hinting">Hinting</a> is set to <a href="bmh_SkPaint_Reference?cl=9919#kNormal_Hinting">kNormal Hinting</a> or <a href="bmh_SkPaint_Reference?cl=9919#kFull_Hinting">kFull Hinting</a>, and if
platform uses <a href="bmh_undocumented?cl=9919#FreeType">FreeType</a> as the <a href="bmh_undocumented?cl=9919#Font_Manager">Font Manager</a>, instruct the <a href="bmh_undocumented?cl=9919#Font_Manager">Font Manager</a> to always hint
Glyphs.

Equivalent to <a href="bmh_SkPaint_Reference?cl=9919#getFlags">getFlags</a> masked with <a href="bmh_SkPaint_Reference?cl=9919#kAutoHinting_Flag">kAutoHinting Flag</a>.

### Return Value

<a href="bmh_SkPaint_Reference?cl=9919#kAutoHinting_Flag">kAutoHinting Flag</a> state

### Example

<div><fiddle-embed name="aa4781afbe3b90e7ef56a287e5b9ce1e">

#### Example Output

~~~~
paint.isAutohinted() == !!(paint.getFlags() & SkPaint::kAutoHinting_Flag)
paint.isAutohinted() == !!(paint.getFlags() & SkPaint::kAutoHinting_Flag)
~~~~

</fiddle-embed></div>

### See Also

<a href="bmh_SkPaint_Reference?cl=9919#setAutohinted">setAutohinted</a> <a href="bmh_SkPaint_Reference?cl=9919#Hinting">Hinting</a>

---

<a name="setAutohinted"></a>
## setAutohinted

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setAutohinted(bool useAutohinter)
</pre>

If <a href="bmh_SkPaint_Reference?cl=9919#Hinting">Hinting</a> is set to <a href="bmh_SkPaint_Reference?cl=9919#kNormal_Hinting">kNormal Hinting</a> or <a href="bmh_SkPaint_Reference?cl=9919#kFull_Hinting">kFull Hinting</a> and <a href="bmh_SkPaint_Reference?cl=9919#useAutohinter">useAutohinter</a> is set,
instruct the <a href="bmh_undocumented?cl=9919#Font_Manager">Font Manager</a> to always hint Glyphs.
<a href="bmh_SkPaint_Reference?cl=9919#Automatic_Hinting">Automatic Hinting</a> has no effect if <a href="bmh_SkPaint_Reference?cl=9919#Hinting">Hinting</a> is set to <a href="bmh_SkPaint_Reference?cl=9919#kNo_Hinting">kNo Hinting</a> or
<a href="bmh_SkPaint_Reference?cl=9919#kSlight_Hinting">kSlight Hinting</a>.

<a href="bmh_SkPaint_Reference?cl=9919#setAutohinted">setAutohinted</a> only affects platforms that use <a href="bmh_undocumented?cl=9919#FreeType">FreeType</a> as the <a href="bmh_undocumented?cl=9919#Font_Manager">Font Manager</a>.

Sets <a href="bmh_SkPaint_Reference?cl=9919#kAutoHinting_Flag">kAutoHinting Flag</a> if <a href="bmh_SkPaint_Reference?cl=9919#useAutohinter">useAutohinter</a> is true.
Clears <a href="bmh_SkPaint_Reference?cl=9919#kAutoHinting_Flag">kAutoHinting Flag</a> if <a href="bmh_SkPaint_Reference?cl=9919#useAutohinter">useAutohinter</a> is false.

### Parameters

<table>  <tr>    <td><code><strong>useAutohinter </strong></code></td> <td>
setting for <a href="bmh_SkPaint_Reference?cl=9919#kAutoHinting_Flag">kAutoHinting Flag</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="4e185306d7de9390fe8445eed0139309"></fiddle-embed></div>

### See Also

<a href="bmh_SkPaint_Reference?cl=9919#isAutohinted">isAutohinted</a> <a href="bmh_SkPaint_Reference?cl=9919#Hinting">Hinting</a>

---

# <a name="Vertical_Text"></a> Vertical Text
<a href="bmh_undocumented?cl=9919#Text">Text</a> may be drawn by positioning each glyph, or by positioning the first glyph and
using <a href="bmh_undocumented?cl=9919#Advance">Font Advance</a> to position subsequent glyphs. By default, each successive glyph
is positioned to the right of the preceeding glyph. <a href="bmh_SkPaint_Reference?cl=9919#Vertical_Text">Vertical Text</a> sets successive
glyphs to position below the preceeding glyph.

<a href="bmh_undocumented?cl=9919#Skia">Skia</a> can translate text character codes as a series of glyphs, but does not implement
font substitution, 
textual substitution, line layout, or contextual spacing like kerning pairs. Use
a text shaping engine likeHarfBuzzhttp://harfbuzz.org/to translate text runs
into glyph series.

<a href="bmh_SkPaint_Reference?cl=9919#Vertical_Text">Vertical Text</a> is clear if text is drawn left to right or set if drawn from top to bottom.

<a href="bmh_SkPaint_Reference?cl=9919#Flags">Flags</a> <a href="bmh_SkPaint_Reference?cl=9919#kVerticalText_Flag">kVerticalText Flag</a> if clear draws text left to right.
<a href="bmh_SkPaint_Reference?cl=9919#Flags">Flags</a> <a href="bmh_SkPaint_Reference?cl=9919#kVerticalText_Flag">kVerticalText Flag</a> if set draws text top to bottom.

<a href="bmh_SkPaint_Reference?cl=9919#Vertical_Text">Vertical Text</a> is clear by default.
<a href="bmh_SkPaint_Reference?cl=9919#Vertical_Text">Vertical Text</a> can be set by default by setting <a href="bmh_undocumented?cl=9919#SkPaintDefaults_Flags">SkPaintDefaults Flags</a> to
<a href="bmh_SkPaint_Reference?cl=9919#kVerticalText_Flag">kVerticalText Flag</a> at compile time.

### Example

<div><fiddle-embed name="8df5800819311b71373d9abb669b49b8"></fiddle-embed></div>

<a name="isVerticalText"></a>
## isVerticalText

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool isVerticalText() const
</pre>

If true, glyphs are drawn top to bottom instead of left to right.

Equivalent to <a href="bmh_SkPaint_Reference?cl=9919#getFlags">getFlags</a> masked with <a href="bmh_SkPaint_Reference?cl=9919#kVerticalText_Flag">kVerticalText Flag</a>.

### Return Value

<a href="bmh_SkPaint_Reference?cl=9919#kVerticalText_Flag">kVerticalText Flag</a> state

### Example

<div><fiddle-embed name="4a269b16e644d473870ffa873396f139">

#### Example Output

~~~~
paint.isVerticalText() == !!(paint.getFlags() & SkPaint::kVerticalText_Flag)
paint.isVerticalText() == !!(paint.getFlags() & SkPaint::kVerticalText_Flag)
~~~~

</fiddle-embed></div>

---

<a name="setVerticalText"></a>
## setVerticalText

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setVerticalText(bool verticalText)
</pre>

If true, text advance positions the next glyph below the previous glyph instead of to the
right of previous glyph.

Sets <a href="bmh_SkPaint_Reference?cl=9919#kVerticalText_Flag">kVerticalText Flag</a> if vertical is true.
Clears <a href="bmh_SkPaint_Reference?cl=9919#kVerticalText_Flag">kVerticalText Flag</a> if vertical is false.

### Parameters

<table>  <tr>    <td><code><strong>verticalText </strong></code></td> <td>
setting for <a href="bmh_SkPaint_Reference?cl=9919#kVerticalText_Flag">kVerticalText Flag</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="6fbd7e9e1a346cb8d7f537786009c736">

#### Example Output

~~~~
paint1 == paint2
~~~~

</fiddle-embed></div>

---

# <a name="Fake_Bold"></a> Fake Bold
<a href="bmh_SkPaint_Reference?cl=9919#Fake_Bold">Fake Bold</a> approximates the bold font style accompanying a normal font when a bold font face
is not available. <a href="bmh_undocumented?cl=9919#Skia">Skia</a> does not provide font substitution; it is up to the client to find the
bold font face using the platform's <a href="bmh_undocumented?cl=9919#Font_Manager">Font Manager</a>.

Use <a href="bmh_SkPaint_Reference?cl=9919#Text_Skew_X">Text Skew X</a> to approximate an italic font style when the italic font face 
is not available.

A <a href="bmh_undocumented?cl=9919#FreeType_based">FreeType-based</a> port may define <a href="bmh_undocumented?cl=9919#SK_USE_FREETYPE_EMBOLDEN">SK USE FREETYPE EMBOLDEN</a> at compile time to direct
the font engine to create the bold glyphs. Otherwise, the extra bold is computed
by increasing the stroke width and setting the <a href="bmh_SkPaint_Reference?cl=9919#Style">Style</a> to <a href="bmh_SkPaint_Reference?cl=9919#kStrokeAndFill_Style">kStrokeAndFill Style</a> as needed.  

<a href="bmh_SkPaint_Reference?cl=9919#Fake_Bold">Fake Bold</a> is disabled by default.

### Example

<div><fiddle-embed name="e811f4829a2daaaeaad3795504a7e02a"></fiddle-embed></div>

<a name="isFakeBoldText"></a>
## isFakeBoldText

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool isFakeBoldText() const
</pre>

If true, approximate bold by increasing the stroke width when creating glyph bitmaps
from outlines.

Equivalent to <a href="bmh_SkPaint_Reference?cl=9919#getFlags">getFlags</a> masked with <a href="bmh_SkPaint_Reference?cl=9919#kFakeBoldText_Flag">kFakeBoldText Flag</a>.

### Return Value

<a href="bmh_SkPaint_Reference?cl=9919#kFakeBoldText_Flag">kFakeBoldText Flag</a> state

### Example

<div><fiddle-embed name="f54d1f85b16073b80b9eef2e1a1d151d">

#### Example Output

~~~~
paint.isFakeBoldText() == !!(paint.getFlags() & SkPaint::kFakeBoldText_Flag)
paint.isFakeBoldText() == !!(paint.getFlags() & SkPaint::kFakeBoldText_Flag)
~~~~

</fiddle-embed></div>

---

<a name="setFakeBoldText"></a>
## setFakeBoldText

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setFakeBoldText(bool fakeBoldText)
</pre>

Use increased stroke width when creating glyph bitmaps to approximate bolding.

Sets <a href="bmh_SkPaint_Reference?cl=9919#kFakeBoldText_Flag">kFakeBoldText Flag</a> if <a href="bmh_SkPaint_Reference?cl=9919#fakeBoldText">fakeBoldText</a> is true.
Clears <a href="bmh_SkPaint_Reference?cl=9919#kFakeBoldText_Flag">kFakeBoldText Flag</a> if <a href="bmh_SkPaint_Reference?cl=9919#fakeBoldText">fakeBoldText</a> is false.

### Parameters

<table>  <tr>    <td><code><strong>fakeBoldText </strong></code></td> <td>
setting for <a href="bmh_SkPaint_Reference?cl=9919#kFakeBoldText_Flag">kFakeBoldText Flag</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="594d47858eb11028cb626515a520910a">

#### Example Output

~~~~
paint1 == paint2
~~~~

</fiddle-embed></div>

---

# <a name="Full_Hinting_Spacing"></a> Full Hinting Spacing
<a href="bmh_SkPaint_Reference?cl=9919#Full_Hinting_Spacing">Full Hinting Spacing</a> adjusts the character spacing by the difference of the 
hinted and unhinted left and right side bearings, 
if <a href="bmh_SkPaint_Reference?cl=9919#Hinting">Hinting</a> is set to <a href="bmh_SkPaint_Reference?cl=9919#kFull_Hinting">kFull Hinting</a>. <a href="bmh_SkPaint_Reference?cl=9919#Full_Hinting_Spacing">Full Hinting Spacing</a> only
applies to platforms that use <a href="bmh_undocumented?cl=9919#FreeType">FreeType</a> as their <a href="bmh_undocumented?cl=9919#Engine">Font Engine</a>.

<a href="bmh_SkPaint_Reference?cl=9919#Full_Hinting_Spacing">Full Hinting Spacing</a> is not related to text kerning, where the space between
a specific pair of characters is adjusted using data in the font's kerning tables.

<a name="isDevKernText"></a>
## isDevKernText

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool isDevKernText() const
</pre>

Returns if character spacing may be adjusted by the hinting difference.

Equivalent to <a href="bmh_SkPaint_Reference?cl=9919#getFlags">getFlags</a> masked with <a href="bmh_SkPaint_Reference?cl=9919#kDevKernText_Flag">kDevKernText Flag</a>.

### Return Value

<a href="bmh_SkPaint_Reference?cl=9919#kDevKernText_Flag">kDevKernText Flag</a> state

### Example

<div><fiddle-embed name="4f69a84b2505b12809c30b0cc09c5157"></fiddle-embed></div>

---

<a name="setDevKernText"></a>
## setDevKernText

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setDevKernText(bool devKernText)
</pre>

Requests, but does not require, to use hinting to adjust glyph spacing.

Sets <a href="bmh_SkPaint_Reference?cl=9919#kDevKernText_Flag">kDevKernText Flag</a> if <a href="bmh_SkPaint_Reference?cl=9919#devKernText">devKernText</a> is true.
Clears <a href="bmh_SkPaint_Reference?cl=9919#kDevKernText_Flag">kDevKernText Flag</a> if <a href="bmh_SkPaint_Reference?cl=9919#devKernText">devKernText</a> is false.

### Parameters

<table>  <tr>    <td><code><strong>devKernText </strong></code></td> <td>
setting for <a href="bmh_SkPaint_Reference?cl=9919#devKernText">devKernText</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="2b718a059072908bf68942503f264797">

#### Example Output

~~~~
paint1 == paint2
~~~~

</fiddle-embed></div>

---

# <a name="Filter_Quality_Methods"></a> Filter Quality Methods
<a href="bmh_undocumented?cl=9919#Filter_Quality">Filter Quality</a> trades speed for image filtering when the image is scaled.
A lower <a href="bmh_undocumented?cl=9919#Filter_Quality">Filter Quality</a> draws faster, but has less fidelity.
A higher <a href="bmh_undocumented?cl=9919#Filter_Quality">Filter Quality</a> draws slower, but looks better.
If the image is unscaled, the <a href="bmh_undocumented?cl=9919#Filter_Quality">Filter Quality</a> choice will not result in a noticable
difference.

<a href="bmh_undocumented?cl=9919#Filter_Quality">Filter Quality</a> is used in <a href="bmh_SkPaint_Reference?cl=9919#Paint">Paint</a> passed as a parameter to

<table>  <tr>
    <td><a href="bmh_SkCanvas_Reference?cl=9919#drawBitmap">SkCanvas::drawBitmap</a></td>  </tr>  <tr>
    <td><a href="bmh_SkCanvas_Reference?cl=9919#drawBitmapRect">SkCanvas::drawBitmapRect</a></td>  </tr>  <tr>
    <td><a href="bmh_SkCanvas_Reference?cl=9919#drawImage">SkCanvas::drawImage</a></td>  </tr>  <tr>
    <td><a href="bmh_SkCanvas_Reference?cl=9919#drawImageRect">SkCanvas::drawImageRect</a></td>  </tr>
</table>

and when <a href="bmh_SkPaint_Reference?cl=9919#Paint">Paint</a> has a <a href="bmh_undocumented?cl=9919#Shader">Shader</a> specialization that uses <a href="bmh_undocumented?cl=9919#Image">Image</a> or <a href="bmh_undocumented?cl=9919#Bitmap">Bitmap</a>.

<a href="bmh_undocumented?cl=9919#Filter_Quality">Filter Quality</a> is <a href="bmh_undocumented?cl=9919#SkFilterQuality">kNone SkFilterQuality</a> by default.

### Example

<div><fiddle-embed name="ee77f83f7291e07ae0d89f1380c7d67c"></fiddle-embed></div>

<a name="getFilterQuality"></a>
## getFilterQuality

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkFilterQuality getFilterQuality() const
</pre>

Returns <a href="bmh_undocumented?cl=9919#Filter_Quality">Filter Quality</a>, the image filtering level. A lower setting
draws faster; a higher setting looks better when the image is scaled.

### Return Value

one of: <a href="bmh_undocumented?cl=9919#SkFilterQuality">kNone SkFilterQuality</a>, <a href="bmh_undocumented?cl=9919#SkFilterQuality">kLow SkFilterQuality</a>, 
<a href="bmh_undocumented?cl=9919#SkFilterQuality">kMedium SkFilterQuality</a>, <a href="bmh_undocumented?cl=9919#SkFilterQuality">kHigh SkFilterQuality</a>

### Example

<div><fiddle-embed name="d4ca1f23809b6835c4ba46ea98a86900">

#### Example Output

~~~~
kNone_SkFilterQuality == paint.getFilterQuality()
~~~~

</fiddle-embed></div>

---

<a name="setFilterQuality"></a>
## setFilterQuality

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setFilterQuality(SkFilterQuality quality)
</pre>

Sets <a href="bmh_undocumented?cl=9919#Filter_Quality">Filter Quality</a>, the image filtering level. A lower setting
draws faster; a higher setting looks better when the image is scaled.
<a href="bmh_SkPaint_Reference?cl=9919#setFilterQuality">setFilterQuality</a> does not check to see if <a href="bmh_SkPaint_Reference?cl=9919#setFilterQuality">quality</a> is valid. 

### Parameters

<table>  <tr>    <td><code><strong>quality </strong></code></td> <td>
one of: <a href="bmh_undocumented?cl=9919#SkFilterQuality">kNone SkFilterQuality</a>, <a href="bmh_undocumented?cl=9919#SkFilterQuality">kLow SkFilterQuality</a>, 
<a href="bmh_undocumented?cl=9919#SkFilterQuality">kMedium SkFilterQuality</a>, <a href="bmh_undocumented?cl=9919#SkFilterQuality">kHigh SkFilterQuality</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="e4288fabf24ee60b645e8bb6ea0afadf">

#### Example Output

~~~~
kHigh_SkFilterQuality == paint.getFilterQuality()
~~~~

</fiddle-embed></div>

### See Also

<a href="bmh_undocumented?cl=9919#SkFilterQuality">SkFilterQuality</a> <a href="bmh_undocumented?cl=9919#Image_Scaling">Image Scaling</a>

---

# <a name="Color_Methods"></a> Color Methods
<a href="bmh_undocumented?cl=9919#Color">Color</a> specifies the <a href="bmh_undocumented?cl=9919#RGB_Red">Color RGB Red</a>, <a href="bmh_undocumented?cl=9919#RGB_Blue">Color RGB Blue</a>, <a href="bmh_undocumented?cl=9919#RGB_Green">Color RGB Green</a>, and <a href="bmh_undocumented?cl=9919#Alpha">Color Alpha</a> values used to draw a filled
or stroked shape in a
32-bit value. Each component occupies 8-bits, ranging from zero: no contribution;
to 255: full intensity. All values in any combination are valid.

<a href="bmh_undocumented?cl=9919#Color">Color</a> is not premultiplied;
<a href="bmh_undocumented?cl=9919#Alpha">Color Alpha</a> sets the transparency independent of <a href="bmh_undocumented?cl=9919#RGB">Color RGB</a>: <a href="bmh_undocumented?cl=9919#RGB_Red">Color RGB Red</a>, <a href="bmh_undocumented?cl=9919#RGB_Blue">Color RGB Blue</a>, and <a href="bmh_undocumented?cl=9919#RGB_Green">Color RGB Green</a>.

The bit positions of <a href="bmh_undocumented?cl=9919#Alpha">Color Alpha</a> and <a href="bmh_undocumented?cl=9919#RGB">Color RGB</a> are independent of the bit positions
on the output device, which may have more or fewer bits, and may have a different arrangement.

| bit positions | <a href="bmh_undocumented?cl=9919#Alpha">Color Alpha</a> | <a href="bmh_undocumented?cl=9919#RGB_Red">Color RGB Red</a> | <a href="bmh_undocumented?cl=9919#RGB_Blue">Color RGB Blue</a> | <a href="bmh_undocumented?cl=9919#RGB_Green">Color RGB Green</a> |
| --- | --- | --- | --- | ---  |
|  | 31 - 24 | 23 - 16 | 15 - 8 | 7 - 0 |

### Example

<div><fiddle-embed name="214b559d75c65a7bef6ef4be1f860053"></fiddle-embed></div>

<a name="getColor"></a>
## getColor

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkColor getColor() const
</pre>

Retrieves <a href="bmh_undocumented?cl=9919#Alpha">Color Alpha</a> and <a href="bmh_undocumented?cl=9919#RGB">Color RGB</a>, unpremultiplied, packed into 32 bits.
Use helpers <a href="bmh_undocumented?cl=9919#SkColorGetA">SkColorGetA</a>, <a href="bmh_undocumented?cl=9919#SkColorGetR">SkColorGetR</a>, <a href="bmh_undocumented?cl=9919#SkColorGetG">SkColorGetG</a>, and <a href="bmh_undocumented?cl=9919#SkColorGetB">SkColorGetB</a> to extract
a color component.

### Return Value

<a href="bmh_undocumented?cl=9919#Unpremultiplied">Unpremultiplied</a> <a href="bmh_undocumented?cl=9919#ARGB">Color ARGB</a>

### Example

<div><fiddle-embed name="72d41f890203109a41f589a7403acae9">

#### Example Output

~~~~
Yellow is 100% red, 100% green, and 0% blue.
~~~~

</fiddle-embed></div>

### See Also

<a href="bmh_undocumented?cl=9919#SkColor">SkColor</a>

---

<a name="setColor"></a>
## setColor

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setColor(SkColor color)
</pre>

Sets <a href="bmh_undocumented?cl=9919#Alpha">Color Alpha</a> and <a href="bmh_undocumented?cl=9919#RGB">Color RGB</a> used when stroking and filling. The <a href="bmh_SkPaint_Reference?cl=9919#color">color</a> is a 32-bit value,
unpremutiplied, packing 8-bit components for <a href="bmh_undocumented?cl=9919#Alpha">Color Alpha</a>, <a href="bmh_undocumented?cl=9919#RGB_Red">Color RGB Red</a>, <a href="bmh_undocumented?cl=9919#RGB_Blue">Color RGB Blue</a>, and <a href="bmh_undocumented?cl=9919#RGB_Green">Color RGB Green</a>. 

### Parameters

<table>  <tr>    <td><code><strong>color </strong></code></td> <td>
<a href="bmh_undocumented?cl=9919#Unpremultiplied">Unpremultiplied</a> <a href="bmh_undocumented?cl=9919#ARGB">Color ARGB</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="6e70f18300bd676a3c056ceb6b62f8df">

#### Example Output

~~~~
green1 == green2
~~~~

</fiddle-embed></div>

### See Also

<a href="bmh_undocumented?cl=9919#SkColor">SkColor</a> <a href="bmh_SkPaint_Reference?cl=9919#setARGB">setARGB</a> <a href="bmh_undocumented?cl=9919#SkColorSetARGB">SkColorSetARGB</a>

---

## <a name="Alpha_Methods"></a> Alpha Methods

<a href="bmh_undocumented?cl=9919#Alpha">Color Alpha</a> sets the transparency independent of <a href="bmh_undocumented?cl=9919#RGB">Color RGB</a>: <a href="bmh_undocumented?cl=9919#RGB_Red">Color RGB Red</a>, <a href="bmh_undocumented?cl=9919#RGB_Blue">Color RGB Blue</a>, and <a href="bmh_undocumented?cl=9919#RGB_Green">Color RGB Green</a>.

<a name="getAlpha"></a>
## getAlpha

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
uint8_t getAlpha() const
</pre>

Retrieves <a href="bmh_undocumented?cl=9919#Alpha">Color Alpha</a> from the <a href="bmh_undocumented?cl=9919#Color">Color</a> used when stroking and filling.

### Return Value

<a href="bmh_undocumented?cl=9919#Alpha">Color Alpha</a> ranging from zero, fully transparent, to 255, fully opaque

### Example

<div><fiddle-embed name="9a85bb62fe3d877b18fb7f952c4fa7f7">

#### Example Output

~~~~
255 == paint.getAlpha()
~~~~

</fiddle-embed></div>

---

<a name="setAlpha"></a>
## setAlpha

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setAlpha(U8CPU a)
</pre>

Replaces <a href="bmh_undocumented?cl=9919#Alpha">Color Alpha</a>, leaving <a href="bmh_undocumented?cl=9919#RGB">Color RGB</a> 
unchanged. An out of range value triggers an assert in the debug
build. <a href="bmh_SkPaint_Reference?cl=9919#a">a</a> is <a href="bmh_SkPaint_Reference?cl=9919#a">a</a> value from zero to 255.
<a href="bmh_SkPaint_Reference?cl=9919#a">a</a> set to zero makes <a href="bmh_undocumented?cl=9919#Color">Color</a> fully transparent; <a href="bmh_SkPaint_Reference?cl=9919#a">a</a> set to 255 makes <a href="bmh_undocumented?cl=9919#Color">Color</a>
fully opaque.

### Parameters

<table>  <tr>    <td><code><strong>a </strong></code></td> <td>
<a href="bmh_undocumented?cl=9919#Alpha">Color Alpha</a> component of <a href="bmh_undocumented?cl=9919#Color">Color</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="6ddc0360512dfb9947e75c17e6a8103d">

#### Example Output

~~~~
0x44112233 == paint.getColor()
~~~~

</fiddle-embed></div>

---

<a name="setARGB"></a>
## setARGB

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setARGB(U8CPU a, U8CPU r, U8CPU g, U8CPU b)
</pre>

Sets <a href="bmh_undocumented?cl=9919#Color">Color</a> used when drawing solid fills. The color components range from 0 to 255.
The color is unpremultiplied;
<a href="bmh_undocumented?cl=9919#Alpha">Color Alpha</a> sets the transparency independent of <a href="bmh_undocumented?cl=9919#RGB">Color RGB</a>.

### Parameters

<table>  <tr>    <td><code><strong>a </strong></code></td> <td>
amount of <a href="bmh_undocumented?cl=9919#Alpha">Color Alpha</a>, from fully transparent (0) to fully opaque (255)</td>
  </tr>  <tr>    <td><code><strong>r </strong></code></td> <td>
amount of <a href="bmh_undocumented?cl=9919#RGB_Red">Color RGB Red</a>, from no red (0) to full red (255)</td>
  </tr>  <tr>    <td><code><strong>g </strong></code></td> <td>
amount of <a href="bmh_undocumented?cl=9919#RGB_Green">Color RGB Green</a>, from no green (0) to full green (255)</td>
  </tr>  <tr>    <td><code><strong>b </strong></code></td> <td>
amount of <a href="bmh_undocumented?cl=9919#RGB_Blue">Color RGB Blue</a>, from no blue (0) to full blue (255)</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="cb62e4755789ed32f7120dc55984959d">

#### Example Output

~~~~
transRed1 == transRed2
~~~~

</fiddle-embed></div>

### See Also

<a href="bmh_SkPaint_Reference?cl=9919#setColor">setColor</a> <a href="bmh_undocumented?cl=9919#SkColorSetARGB">SkColorSetARGB</a>

---

# <a name="Style"></a> Style
<a href="bmh_SkPaint_Reference?cl=9919#Style">Style</a> specifies if the geometry is filled, stroked, or both filled and stroked.
Some shapes ignore <a href="bmh_SkPaint_Reference?cl=9919#Style">Style</a> and are always drawn filled or stroked.

Set <a href="bmh_SkPaint_Reference?cl=9919#Style">Style</a> to <a href="bmh_SkPaint_Reference?cl=9919#kFill_Style">kFill Style</a> to fill the shape.
The fill covers the area inside the geometry for most shapes.

Set <a href="bmh_SkPaint_Reference?cl=9919#Style">Style</a> to <a href="bmh_SkPaint_Reference?cl=9919#kStroke_Style">kStroke Style</a> to stroke the shape.

## <a name="Fill"></a> Fill

### See Also

<a href="bmh_SkPath_Reference?cl=9919#Fill_Type">Path Fill Type</a>

## <a name="Stroke"></a> Stroke

The stroke covers the area described by following the shape's edge with a pen or brush of
<a href="bmh_SkPaint_Reference?cl=9919#Stroke_Width">Stroke Width</a>. The area covered where the shape starts and stops is described by <a href="bmh_SkPaint_Reference?cl=9919#Stroke_Cap">Stroke Cap</a>.
The area covered where the shape turns a corner is described by <a href="bmh_SkPaint_Reference?cl=9919#Stroke_Join">Stroke Join</a>.
The stroke is centered on the shape; it extends equally on either side of the shape's edge.

As <a href="bmh_SkPaint_Reference?cl=9919#Stroke_Width">Stroke Width</a> gets smaller, the drawn path frame is thinner. <a href="bmh_SkPaint_Reference?cl=9919#Stroke_Width">Stroke Width</a> less than one
may have gaps, and if <a href="bmh_SkPaint_Reference?cl=9919#kAntiAlias_Flag">kAntiAlias Flag</a> is set, <a href="bmh_undocumented?cl=9919#Alpha">Color Alpha</a> will increase to visually decrease coverage.

## <a name="Hairline"></a> Hairline

<a href="bmh_SkPaint_Reference?cl=9919#Stroke_Width">Stroke Width</a> of zero has a special meaning and switches drawing to use <a href="bmh_SkPaint_Reference?cl=9919#Hairline">Hairline</a>.
<a href="bmh_SkPaint_Reference?cl=9919#Hairline">Hairline</a> draws the thinnest continuous frame. If <a href="bmh_SkPaint_Reference?cl=9919#kAntiAlias_Flag">kAntiAlias Flag</a> is clear, adjacent pixels 
flow horizontally, vertically,or diagonally. 

<a href="bmh_SkPath_Reference?cl=9919#Path">Path</a> drawing with <a href="bmh_SkPaint_Reference?cl=9919#Hairline">Hairline</a> may hit the same pixel more than once. For instance, <a href="bmh_SkPath_Reference?cl=9919#Path">Path</a> containing
two lines in one <a href="bmh_SkPath_Reference?cl=9919#Contour">Path Contour</a> will draw the corner point once, but may both lines may draw the adjacent
pixel. If <a href="bmh_SkPaint_Reference?cl=9919#kAntiAlias_Flag">kAntiAlias Flag</a> is set, transparency is applied twice, resulting in a darker pixel. Some
<a href="bmh_undocumented?cl=9919#GPU_backed">GPU-backed</a> implementations apply transparency at a later drawing stage, avoiding double hit pixels
while stroking.

## <a name="SkPaint::Style"></a> Enum SkPaint::Style

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
enum <a href="bmh_SkPaint_Reference?cl=9919#Style">Style</a> {
<a href="bmh_SkPaint_Reference?cl=9919#kFill_Style">kFill Style</a>,
<a href="bmh_SkPaint_Reference?cl=9919#kStroke_Style">kStroke Style</a>,
<a href="bmh_SkPaint_Reference?cl=9919#kStrokeAndFill_Style">kStrokeAndFill Style</a>,
};</pre>

Set <a href="bmh_SkPaint_Reference?cl=9919#Style">Style</a> to fill, stroke, or both fill and stroke geometry.
The stroke and fill
share all paint attributes; for instance, they are drawn with the same color.

Use <a href="bmh_SkPaint_Reference?cl=9919#kStrokeAndFill_Style">kStrokeAndFill Style</a> to avoid hitting the same pixels twice with a stroke draw and
a fill draw.

### Constants

<table>
  <tr>
    <td><a name="SkPaint::kFill_Style"></a> <code><strong>SkPaint::kFill_Style </strong></code></td><td>0</td><td>Set to fill geometry.
Applies to <a href="bmh_undocumented?cl=9919#Rect">Rect</a>, <a href="bmh_undocumented?cl=9919#Region">Region</a>, <a href="bmh_undocumented?cl=9919#Round_Rect">Round Rect</a>, <a href="bmh_undocumented?cl=9919#Circle">Circle</a>, <a href="bmh_undocumented?cl=9919#Oval">Oval</a>, <a href="bmh_SkPath_Reference?cl=9919#Path">Path</a>, and <a href="bmh_undocumented?cl=9919#Text">Text</a>. 
<a href="bmh_undocumented?cl=9919#Bitmap">Bitmap</a>, <a href="bmh_undocumented?cl=9919#Image">Image</a>, <a href="bmh_undocumented?cl=9919#Patch">Patch</a>, <a href="bmh_undocumented?cl=9919#Region">Region</a>, <a href="bmh_undocumented?cl=9919#Sprite">Sprite</a>, and <a href="bmh_undocumented?cl=9919#Vertices">Vertices</a> are painted as if
<a href="bmh_SkPaint_Reference?cl=9919#kFill_Style">kFill Style</a> is set, and ignore the set <a href="bmh_SkPaint_Reference?cl=9919#Style">Style</a>.
The <a href="bmh_SkPath_Reference?cl=9919#Fill_Type">Path Fill Type</a> specifies additional rules to fill the area outside the path edge,
and to create an unfilled hole inside the shape.
<a href="bmh_SkPaint_Reference?cl=9919#Style">Style</a> is set to <a href="bmh_SkPaint_Reference?cl=9919#kFill_Style">kFill Style</a> by default.</td>
  </tr>
  <tr>
    <td><a name="SkPaint::kStroke_Style"></a> <code><strong>SkPaint::kStroke_Style </strong></code></td><td>1</td><td>Set to stroke geometry.
Applies to <a href="bmh_undocumented?cl=9919#Rect">Rect</a>, <a href="bmh_undocumented?cl=9919#Region">Region</a>, <a href="bmh_undocumented?cl=9919#Round_Rect">Round Rect</a>, <a href="bmh_undocumented?cl=9919#Arc">Arc</a>, <a href="bmh_undocumented?cl=9919#Circle">Circle</a>, <a href="bmh_undocumented?cl=9919#Oval">Oval</a>,
<a href="bmh_SkPath_Reference?cl=9919#Path">Path</a>, and <a href="bmh_undocumented?cl=9919#Text">Text</a>. 
<a href="bmh_undocumented?cl=9919#Arc">Arc</a>, <a href="bmh_undocumented?cl=9919#Line">Line</a>, <a href="bmh_undocumented?cl=9919#Point">Point</a>, and <a href="bmh_undocumented?cl=9919#Array">Point Array</a> are always drawn as if <a href="bmh_SkPaint_Reference?cl=9919#kStroke_Style">kStroke Style</a> is set,
and ignore the set <a href="bmh_SkPaint_Reference?cl=9919#Style">Style</a>.
The stroke construction is unaffected by the <a href="bmh_SkPath_Reference?cl=9919#Fill_Type">Path Fill Type</a>.</td>
  </tr>
  <tr>
    <td><a name="SkPaint::kStrokeAndFill_Style"></a> <code><strong>SkPaint::kStrokeAndFill_Style </strong></code></td><td>2</td><td>Set to stroke and fill geometry.
Applies to <a href="bmh_undocumented?cl=9919#Rect">Rect</a>, <a href="bmh_undocumented?cl=9919#Region">Region</a>, <a href="bmh_undocumented?cl=9919#Round_Rect">Round Rect</a>, <a href="bmh_undocumented?cl=9919#Circle">Circle</a>, <a href="bmh_undocumented?cl=9919#Oval">Oval</a>, <a href="bmh_SkPath_Reference?cl=9919#Path">Path</a>, and <a href="bmh_undocumented?cl=9919#Text">Text</a>.
<a href="bmh_SkPath_Reference?cl=9919#Path">Path</a> is treated as if it is set to <a href="bmh_SkPath_Reference?cl=9919#kWinding_FillType">SkPath::kWinding FillType</a>,
and the set <a href="bmh_SkPath_Reference?cl=9919#Fill_Type">Path Fill Type</a> is ignored.</td>
  </tr>

</table>

## <a name="SkPaint::_anonymous"></a> Enum SkPaint::_anonymous

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
enum {
<a href="bmh_SkPaint_Reference?cl=9919#kStyleCount">kStyleCount</a> = <a href="bmh_SkPaint_Reference?cl=9919#kStrokeAndFill_Style">kStrokeAndFill Style</a> + 1
};</pre>

### Constants

<table>
  <tr>
    <td><a name="SkPaint::kStyleCount"></a> <code><strong>SkPaint::kStyleCount </strong></code></td><td>3</td><td>The number of different <a href="bmh_SkPaint_Reference?cl=9919#Style">Style</a> values defined.
May be used to verify that <a href="bmh_SkPaint_Reference?cl=9919#Style">Style</a> is a legal value.</td>
  </tr>

</table>

<a name="getStyle"></a>
## getStyle

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
Style getStyle() const
</pre>

Whether the geometry is filled, stroked, or filled and stroked.

### Return Value

one of:<a href="bmh_SkPaint_Reference?cl=9919#kFill_Style">kFill Style</a>, <a href="bmh_SkPaint_Reference?cl=9919#kStroke_Style">kStroke Style</a>, <a href="bmh_SkPaint_Reference?cl=9919#kStrokeAndFill_Style">kStrokeAndFill Style</a>

### Example

<div><fiddle-embed name="1c5e18c3c0102d2dac86a78ba8c8ce01">

#### Example Output

~~~~
SkPaint::kFill_Style == paint.getStyle()
~~~~

</fiddle-embed></div>

### See Also

<a href="bmh_SkPaint_Reference?cl=9919#Style">Style</a> <a href="bmh_SkPaint_Reference?cl=9919#setStyle">setStyle</a>

---

<a name="setStyle"></a>
## setStyle

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setStyle(Style style)
</pre>

Sets whether the geometry is filled, stroked, or filled and stroked.
Has no effect if <a href="bmh_SkPaint_Reference?cl=9919#setStyle">style</a> is not a legal <a href="bmh_SkPaint_Reference?cl=9919#Style">Style</a> value.

### Parameters

<table>  <tr>    <td><code><strong>style </strong></code></td> <td>
one of: <a href="bmh_SkPaint_Reference?cl=9919#kFill_Style">kFill Style</a>, <a href="bmh_SkPaint_Reference?cl=9919#kStroke_Style">kStroke Style</a>, <a href="bmh_SkPaint_Reference?cl=9919#kStrokeAndFill_Style">kStrokeAndFill Style</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="c7bb6248e4735b8d1a32d02fba40d344"></fiddle-embed></div>

### See Also

<a href="bmh_SkPaint_Reference?cl=9919#Style">Style</a> <a href="bmh_SkPaint_Reference?cl=9919#getStyle">getStyle</a>

---

### See Also

<a href="bmh_SkPath_Reference?cl=9919#Fill_Type">Path Fill Type</a> <a href="bmh_undocumented?cl=9919#Path_Effect">Path Effect</a> <a href="bmh_SkPaint_Reference?cl=9919#Style_Fill">Style Fill</a> <a href="bmh_SkPaint_Reference?cl=9919#Style_Stroke">Style Stroke</a>

# <a name="Stroke_Width"></a> Stroke Width
<a href="bmh_SkPaint_Reference?cl=9919#Stroke_Width">Stroke Width</a> sets the width for stroking. The width is the thickness
of the stroke perpendicular to the path's direction when the paint's style is 
set to <a href="bmh_SkPaint_Reference?cl=9919#kStroke_Style">kStroke Style</a> or <a href="bmh_SkPaint_Reference?cl=9919#kStrokeAndFill_Style">kStrokeAndFill Style</a>.

When width is greater than zero, the stroke encompasses as many pixels partially
or fully as needed. When the width equals zero, the paint enables hairlines;
the stroke is always one pixel wide. 

The stroke's dimensions are scaled by the canvas matrix, but <a href="bmh_SkPaint_Reference?cl=9919#Hairline">Hairline</a> stroke
remains one pixel wide regardless of scaling.

The default width for the paint is zero.

### Example

<div><fiddle-embed name="01e3e08a3022a351628ff54e84887756">raster gpu<div>The pixels hit to represent thin lines vary with the angle of the 
line and the platform's implementation.</div></fiddle-embed></div>

<a name="getStrokeWidth"></a>
## getStrokeWidth

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkScalar getStrokeWidth() const
</pre>

Returns the thickness of the pen used by <a href="bmh_SkPaint_Reference?cl=9919#Paint">Paint</a> to
outline the shape.

### Return Value

zero for <a href="bmh_SkPaint_Reference?cl=9919#Hairline">Hairline</a>, greater than zero for pen thickness

### Example

<div><fiddle-embed name="99aa73f64df8bbf06e656cd891a81b9e">

#### Example Output

~~~~
0 == paint.getStrokeWidth()
~~~~

</fiddle-embed></div>

---

<a name="setStrokeWidth"></a>
## setStrokeWidth

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setStrokeWidth(SkScalar width)
</pre>

Sets the thickness of the pen used by the paint to
outline the shape. 
Has no effect if <a href="bmh_SkPaint_Reference?cl=9919#setStrokeWidth">width</a> is less than zero. 

### Parameters

<table>  <tr>    <td><code><strong>width </strong></code></td> <td>
zero thickness for <a href="bmh_SkPaint_Reference?cl=9919#Hairline">Hairline</a>; greater than zero for pen thickness</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="0c4446c0870b5c7b5a2efe77ff92afb8">

#### Example Output

~~~~
5 == paint.getStrokeWidth()
~~~~

</fiddle-embed></div>

---

# <a name="Miter_Limit"></a> Miter Limit
<a href="bmh_SkPaint_Reference?cl=9919#Miter_Limit">Miter Limit</a> specifies the maximum miter length,
relative to the stroke width.

<a href="bmh_SkPaint_Reference?cl=9919#Miter_Limit">Miter Limit</a> is used when the <a href="bmh_SkPaint_Reference?cl=9919#Stroke_Join">Stroke Join</a>
is set to <a href="bmh_SkPaint_Reference?cl=9919#kMiter_Join">kMiter Join</a>, and the <a href="bmh_SkPaint_Reference?cl=9919#Style">Style</a> is either <a href="bmh_SkPaint_Reference?cl=9919#kStroke_Style">kStroke Style</a>
or <a href="bmh_SkPaint_Reference?cl=9919#kStrokeAndFill_Style">kStrokeAndFill Style</a>.

If the miter at a corner exceeds this limit, <a href="bmh_SkPaint_Reference?cl=9919#kMiter_Join">kMiter Join</a>
is replaced with <a href="bmh_SkPaint_Reference?cl=9919#kBevel_Join">kBevel Join</a>.

<a href="bmh_SkPaint_Reference?cl=9919#Miter_Limit">Miter Limit</a> can be computed from the corner angle:

miter limit = 1 / sin ( angle / 2 )<a href="bmh_SkPaint_Reference?cl=9919#Miter_Limit">Miter Limit</a> default value is 4.
The default may be changed at compile time by setting <a href="bmh_undocumented?cl=9919#SkPaintDefaults_MiterLimit">SkPaintDefaults MiterLimit</a>
in <a href="bmh_undocumented?cl=9919#SkUserConfig.h">SkUserConfig.h</a> or as a define supplied by the build environment.

Here are some miter limits and the angles that triggers them.

| miter limit | angle in degrees |
| --- | ---  |
| 10 | 11.48 |
| 9 | 12.76 |
| 8 | 14.36 |
| 7 | 16.43 |
| 6 | 19.19 |
| 5 | 23.07 |
| 4 | 28.96 |
| 3 | 38.94 |
| 2 | 60 |
| 1 | 180 |

### Example

<div><fiddle-embed name="5de2de0f00354e59074a9bb1a42d5a63"><div>This example draws a stroked corner and the miter length beneath.
When the miter limit is decreased slightly, the miter join is replaced
by a bevel join.</div></fiddle-embed></div>

<a name="getStrokeMiter"></a>
## getStrokeMiter

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkScalar getStrokeMiter() const
</pre>

The limit at which a sharp corner is drawn beveled.

### Return Value

zero and greater <a href="bmh_SkPaint_Reference?cl=9919#Miter_Limit">Miter Limit</a>

### Example

<div><fiddle-embed name="50da74a43b725f07a914df588c867d36">

#### Example Output

~~~~
default miter limit == 4
~~~~

</fiddle-embed></div>

### See Also

<a href="bmh_SkPaint_Reference?cl=9919#Miter_Limit">Miter Limit</a> <a href="bmh_SkPaint_Reference?cl=9919#setStrokeMiter">setStrokeMiter</a> <a href="bmh_SkPaint_Reference?cl=9919#Join">Join</a>

---

<a name="setStrokeMiter"></a>
## setStrokeMiter

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setStrokeMiter(SkScalar miter)
</pre>

The limit at which a sharp corner is drawn beveled.
Valid values are zero and greater.
Has no effect if <a href="bmh_SkPaint_Reference?cl=9919#setStrokeMiter">miter</a> is less than zero.

### Parameters

<table>  <tr>    <td><code><strong>miter </strong></code></td> <td>
zero and greater <a href="bmh_SkPaint_Reference?cl=9919#Miter_Limit">Miter Limit</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="700b284dbc97785c6a9c9636088713ad">

#### Example Output

~~~~
default miter limit == 8
~~~~

</fiddle-embed></div>

### See Also

<a href="bmh_SkPaint_Reference?cl=9919#Miter_Limit">Miter Limit</a> <a href="bmh_SkPaint_Reference?cl=9919#getStrokeMiter">getStrokeMiter</a> <a href="bmh_SkPaint_Reference?cl=9919#Join">Join</a>

---

# <a name="Stroke_Cap"></a> Stroke Cap

## <a name="SkPaint::Cap"></a> Enum SkPaint::Cap

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
enum <a href="bmh_SkPaint_Reference?cl=9919#Cap">Cap</a> {
<a href="bmh_SkPaint_Reference?cl=9919#kButt_Cap">kButt Cap</a>,
<a href="bmh_SkPaint_Reference?cl=9919#kRound_Cap">kRound Cap</a>,
<a href="bmh_SkPaint_Reference?cl=9919#kSquare_Cap">kSquare Cap</a>,

<a href="bmh_SkPaint_Reference?cl=9919#kLast_Cap">kLast Cap</a> = <a href="bmh_SkPaint_Reference?cl=9919#kSquare_Cap">kSquare Cap</a>,
<a href="bmh_SkPaint_Reference?cl=9919#kDefault_Cap">kDefault Cap</a> = <a href="bmh_SkPaint_Reference?cl=9919#kButt_Cap">kButt Cap</a>
};
static constexpr int <a href="bmh_SkPaint_Reference?cl=9919#kCapCount">kCapCount</a> = <a href="bmh_SkPaint_Reference?cl=9919#kLast_Cap">kLast Cap</a> + 1;</pre>

<a href="bmh_SkPaint_Reference?cl=9919#Stroke_Cap">Stroke Cap</a> draws at the beginning and end of an open <a href="bmh_SkPath_Reference?cl=9919#Contour">Path Contour</a>.

### Constants

<table>
  <tr>
    <td><a name="SkPaint::kButt_Cap"></a> <code><strong>SkPaint::kButt_Cap </strong></code></td><td>0</td><td>Does not extend the stroke past the beginning or the end.</td>
  </tr>
  <tr>
    <td><a name="SkPaint::kRound_Cap"></a> <code><strong>SkPaint::kRound_Cap </strong></code></td><td>1</td><td>Adds a circle with a diameter equal to <a href="bmh_SkPaint_Reference?cl=9919#Stroke_Width">Stroke Width</a> at the beginning
and end.</td>
  </tr>
  <tr>
    <td><a name="SkPaint::kSquare_Cap"></a> <code><strong>SkPaint::kSquare_Cap </strong></code></td><td>2</td><td>Adds a square with sides equal to <a href="bmh_SkPaint_Reference?cl=9919#Stroke_Width">Stroke Width</a> at the beginning
and end. The square sides are parallel to the initial and final direction
of the stroke.</td>
  </tr>
  <tr>
    <td><a name="SkPaint::kLast_Cap"></a> <code><strong>SkPaint::kLast_Cap </strong></code></td><td>2</td><td>Equivalent to the largest value for <a href="bmh_SkPaint_Reference?cl=9919#Stroke_Cap">Stroke Cap</a>.</td>
  </tr>
  <tr>
    <td><a name="SkPaint::kDefault_Cap"></a> <code><strong>SkPaint::kDefault_Cap </strong></code></td><td>0</td><td>Equivalent to <a href="bmh_SkPaint_Reference?cl=9919#kButt_Cap">kButt Cap</a>.
<a href="bmh_SkPaint_Reference?cl=9919#Stroke_Cap">Stroke Cap</a> is set to <a href="bmh_SkPaint_Reference?cl=9919#kButt_Cap">kButt Cap</a> by default.</td>
  </tr>
  <tr>
    <td><a name="SkPaint::kCapCount"></a> <code><strong>SkPaint::kCapCount </strong></code></td><td>3</td><td>The number of different <a href="bmh_SkPaint_Reference?cl=9919#Stroke_Cap">Stroke Cap</a> values defined.
May be used to verify that <a href="bmh_SkPaint_Reference?cl=9919#Stroke_Cap">Stroke Cap</a> is a legal value.</td>
  </tr>

Stroke describes the area covered by a pen of <a href="bmh_SkPaint_Reference?cl=9919#Stroke_Width">Stroke Width</a> as it 
follows the <a href="bmh_SkPath_Reference?cl=9919#Contour">Path Contour</a>, moving parallel to the contours's direction.

If the <a href="bmh_SkPath_Reference?cl=9919#Contour">Path Contour</a> is not terminated by <a href="bmh_SkPath_Reference?cl=9919#kClose_Verb">SkPath::kClose Verb</a>, the contour has a
visible beginning and end.

<a href="bmh_SkPath_Reference?cl=9919#Contour">Path Contour</a> may start and end at the same point; defining <a href="bmh_SkPath_Reference?cl=9919#Zero_Length">Zero Length Contour</a>.

<a href="bmh_SkPaint_Reference?cl=9919#kButt_Cap">kButt Cap</a> and <a href="bmh_SkPath_Reference?cl=9919#Zero_Length">Zero Length Contour</a> is not drawn.
<a href="bmh_SkPaint_Reference?cl=9919#kRound_Cap">kRound Cap</a> and <a href="bmh_SkPath_Reference?cl=9919#Zero_Length">Zero Length Contour</a> draws a circle of diameter <a href="bmh_SkPaint_Reference?cl=9919#Stroke_Width">Stroke Width</a> 
at the contour point.
<a href="bmh_SkPaint_Reference?cl=9919#kSquare_Cap">kSquare Cap</a> and <a href="bmh_SkPath_Reference?cl=9919#Zero_Length">Zero Length Contour</a> draws an upright square with a side of
<a href="bmh_SkPaint_Reference?cl=9919#Stroke_Width">Stroke Width</a> at the contour point.

<a href="bmh_SkPaint_Reference?cl=9919#Stroke_Cap">Stroke Cap</a> is <a href="bmh_SkPaint_Reference?cl=9919#kButt_Cap">kButt Cap</a> by default.

</table>

### Example

<div><fiddle-embed name="3d92b449b298b4ce4004cfca6b91cee7"></fiddle-embed></div>

<a name="getStrokeCap"></a>
## getStrokeCap

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
Cap getStrokeCap() const
</pre>

The geometry drawn at the beginning and end of strokes.

### Return Value

one of: <a href="bmh_SkPaint_Reference?cl=9919#kButt_Cap">kButt Cap</a>, <a href="bmh_SkPaint_Reference?cl=9919#kRound_Cap">kRound Cap</a>, <a href="bmh_SkPaint_Reference?cl=9919#kSquare_Cap">kSquare Cap</a>

### Example

<div><fiddle-embed name="aabf9baee8e026fae36fca30e955512b">

#### Example Output

~~~~
kButt_Cap == default stroke cap
~~~~

</fiddle-embed></div>

### See Also

<a href="bmh_SkPaint_Reference?cl=9919#Stroke_Cap">Stroke Cap</a> <a href="bmh_SkPaint_Reference?cl=9919#setStrokeCap">setStrokeCap</a>

---

<a name="setStrokeCap"></a>
## setStrokeCap

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setStrokeCap(Cap cap)
</pre>

The geometry drawn at the beginning and end of strokes.

### Parameters

<table>  <tr>    <td><code><strong>cap </strong></code></td> <td>
one of: <a href="bmh_SkPaint_Reference?cl=9919#kButt_Cap">kButt Cap</a>, <a href="bmh_SkPaint_Reference?cl=9919#kRound_Cap">kRound Cap</a>, <a href="bmh_SkPaint_Reference?cl=9919#kSquare_Cap">kSquare Cap</a>;
has no effect if <a href="bmh_SkPaint_Reference?cl=9919#setStrokeCap">cap</a> is not valid</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="de83fbd848a4625345b4b87a6e55d98a">

#### Example Output

~~~~
kRound_Cap == paint.getStrokeCap()
~~~~

</fiddle-embed></div>

### See Also

<a href="bmh_SkPaint_Reference?cl=9919#Stroke_Cap">Stroke Cap</a> <a href="bmh_SkPaint_Reference?cl=9919#getStrokeCap">getStrokeCap</a>

---

# <a name="Stroke_Join"></a> Stroke Join
<a href="bmh_SkPaint_Reference?cl=9919#Stroke_Join">Stroke Join</a> draws at the sharp corners of an open or closed <a href="bmh_SkPath_Reference?cl=9919#Contour">Path Contour</a>.

Stroke describes the area covered by a pen of <a href="bmh_SkPaint_Reference?cl=9919#Stroke_Width">Stroke Width</a> as it 
follows the <a href="bmh_SkPath_Reference?cl=9919#Contour">Path Contour</a>, moving parallel to the contours's direction.

If the contour direction changes abruptly, because the tangent direction leading
to the end of a curve within the contour does not match the tangent direction of
the following curve, the pair of curves meet at <a href="bmh_SkPaint_Reference?cl=9919#Stroke_Join">Stroke Join</a>.

### Example

<div><fiddle-embed name="4a4e41ed89a57d47eab5d1600c33b0e8"></fiddle-embed></div>

## <a name="SkPaint::Join"></a> Enum SkPaint::Join

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
enum <a href="bmh_SkPaint_Reference?cl=9919#Join">Join</a> {
<a href="bmh_SkPaint_Reference?cl=9919#kMiter_Join">kMiter Join</a>,
<a href="bmh_SkPaint_Reference?cl=9919#kRound_Join">kRound Join</a>,
<a href="bmh_SkPaint_Reference?cl=9919#kBevel_Join">kBevel Join</a>,

<a href="bmh_SkPaint_Reference?cl=9919#kLast_Join">kLast Join</a> = <a href="bmh_SkPaint_Reference?cl=9919#kBevel_Join">kBevel Join</a>,
<a href="bmh_SkPaint_Reference?cl=9919#kDefault_Join">kDefault Join</a> = <a href="bmh_SkPaint_Reference?cl=9919#kMiter_Join">kMiter Join</a>
};
static constexpr int <a href="bmh_SkPaint_Reference?cl=9919#kJoinCount">kJoinCount</a> = <a href="bmh_SkPaint_Reference?cl=9919#kLast_Join">kLast Join</a> + 1;</pre>

<a href="bmh_SkPaint_Reference?cl=9919#Join">Join</a> specifies how corners are drawn when a shape is stroked. The paint's <a href="bmh_SkPaint_Reference?cl=9919#Join">Join</a> setting
affects the four corners of a stroked rectangle, and the connected segments in a
stroked path.

Choose miter join to draw sharp corners. Choose round join to draw a circle with a
radius equal to the stroke width on top of the corner. Choose bevel join to minimally
connect the thick strokes.

The fill path constructed to describe the stroked path respects the join setting but may 
not contain the actual join. For instance, a fill path constructed with round joins does
not necessarily include circles at each connected segment.

### Constants

<table>
  <tr>
    <td><a name="SkPaint::kMiter_Join"></a> <code><strong>SkPaint::kMiter_Join </strong></code></td><td>0</td><td>Extends the outside corner to the extent allowed by <a href="bmh_SkPaint_Reference?cl=9919#Miter_Limit">Miter Limit</a>.
If the extension exceeds <a href="bmh_SkPaint_Reference?cl=9919#Miter_Limit">Miter Limit</a>, <a href="bmh_SkPaint_Reference?cl=9919#kBevel_Join">kBevel Join</a> is used instead.</td>
  </tr>
  <tr>
    <td><a name="SkPaint::kRound_Join"></a> <code><strong>SkPaint::kRound_Join </strong></code></td><td>1</td><td>Adds a circle with a diameter of <a href="bmh_SkPaint_Reference?cl=9919#Stroke_Width">Stroke Width</a> at the sharp corner.</td>
  </tr>
  <tr>
    <td><a name="SkPaint::kBevel_Join"></a> <code><strong>SkPaint::kBevel_Join </strong></code></td><td>2</td><td>Connects the outside edges of the sharp corner.</td>
  </tr>
  <tr>
    <td><a name="SkPaint::kLast_Join"></a> <code><strong>SkPaint::kLast_Join </strong></code></td><td>2</td><td>Equivalent to the largest value for <a href="bmh_SkPaint_Reference?cl=9919#Stroke_Join">Stroke Join</a>.</td>
  </tr>
  <tr>
    <td><a name="SkPaint::kDefault_Join"></a> <code><strong>SkPaint::kDefault_Join </strong></code></td><td>1</td><td>Equivalent to <a href="bmh_SkPaint_Reference?cl=9919#kMiter_Join">kMiter Join</a>.
<a href="bmh_SkPaint_Reference?cl=9919#Stroke_Join">Stroke Join</a> is set to <a href="bmh_SkPaint_Reference?cl=9919#kMiter_Join">kMiter Join</a> by default.</td>
  </tr>
  <tr>
    <td><a name="SkPaint::kJoinCount"></a> <code><strong>SkPaint::kJoinCount </strong></code></td><td>3</td><td>The number of different <a href="bmh_SkPaint_Reference?cl=9919#Stroke_Join">Stroke Join</a> values defined.
May be used to verify that <a href="bmh_SkPaint_Reference?cl=9919#Stroke_Join">Stroke Join</a> is a legal value.</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="3b1aebacc21c1836a52876b9b0b3905e"></fiddle-embed></div>

### See Also

<a href="bmh_SkPaint_Reference?cl=9919#setStrokeJoin">setStrokeJoin</a> <a href="bmh_SkPaint_Reference?cl=9919#getStrokeJoin">getStrokeJoin</a> <a href="bmh_SkPaint_Reference?cl=9919#setStrokeMiter">setStrokeMiter</a> <a href="bmh_SkPaint_Reference?cl=9919#getStrokeMiter">getStrokeMiter</a>



<a name="getStrokeJoin"></a>
## getStrokeJoin

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
Join getStrokeJoin() const
</pre>

The geometry drawn at the corners of strokes. 

### Return Value

one of: <a href="bmh_SkPaint_Reference?cl=9919#kMiter_Join">kMiter Join</a>, <a href="bmh_SkPaint_Reference?cl=9919#kRound_Join">kRound Join</a>, <a href="bmh_SkPaint_Reference?cl=9919#kBevel_Join">kBevel Join</a>

### Example

<div><fiddle-embed name="31bf751d0a8ddf176b871810820d8199">

#### Example Output

~~~~
kMiter_Join == default stroke join
~~~~

</fiddle-embed></div>

### See Also

<a href="bmh_SkPaint_Reference?cl=9919#Stroke_Join">Stroke Join</a> <a href="bmh_SkPaint_Reference?cl=9919#setStrokeJoin">setStrokeJoin</a>

---

<a name="setStrokeJoin"></a>
## setStrokeJoin

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setStrokeJoin(Join join)
</pre>

The geometry drawn at the corners of strokes. 

### Parameters

<table>  <tr>    <td><code><strong>join </strong></code></td> <td>
one of: <a href="bmh_SkPaint_Reference?cl=9919#kMiter_Join">kMiter Join</a>, <a href="bmh_SkPaint_Reference?cl=9919#kRound_Join">kRound Join</a>, <a href="bmh_SkPaint_Reference?cl=9919#kBevel_Join">kBevel Join</a>;
otherwise, <a href="bmh_SkPaint_Reference?cl=9919#setStrokeJoin">setStrokeJoin</a> has no effect</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="48d963ad4286eddf680f9c511eb6da91">

#### Example Output

~~~~
kMiter_Join == paint.getStrokeJoin()
~~~~

</fiddle-embed></div>

### See Also

<a href="bmh_SkPaint_Reference?cl=9919#Stroke_Join">Stroke Join</a> <a href="bmh_SkPaint_Reference?cl=9919#getStrokeJoin">getStrokeJoin</a>

---

### See Also

<a href="bmh_SkPaint_Reference?cl=9919#Miter_Limit">Miter Limit</a>

# <a name="Fill_Path"></a> Fill Path
<a href="bmh_SkPaint_Reference?cl=9919#Fill_Path">Fill Path</a> creates a <a href="bmh_SkPath_Reference?cl=9919#Path">Path</a> by applying the <a href="bmh_undocumented?cl=9919#Path_Effect">Path Effect</a>, followed by the <a href="bmh_SkPaint_Reference?cl=9919#Style_Stroke">Style Stroke</a>.

If <a href="bmh_SkPaint_Reference?cl=9919#Paint">Paint</a> contains <a href="bmh_undocumented?cl=9919#Path_Effect">Path Effect</a>, <a href="bmh_undocumented?cl=9919#Path_Effect">Path Effect</a> operates on the source <a href="bmh_SkPath_Reference?cl=9919#Path">Path</a>; the result
replaces the destination <a href="bmh_SkPath_Reference?cl=9919#Path">Path</a>. Otherwise, the source <a href="bmh_SkPath_Reference?cl=9919#Path">Path</a> is replaces the
destination <a href="bmh_SkPath_Reference?cl=9919#Path">Path</a>.

Fill <a href="bmh_SkPath_Reference?cl=9919#Path">Path</a> can request the <a href="bmh_undocumented?cl=9919#Path_Effect">Path Effect</a> to restrict to a culling rectangle, but
the <a href="bmh_undocumented?cl=9919#Path_Effect">Path Effect</a> is not required to do so.

If <a href="bmh_SkPaint_Reference?cl=9919#Style">Style</a> is <a href="bmh_SkPaint_Reference?cl=9919#kStroke_Style">kStroke Style</a> or <a href="bmh_SkPaint_Reference?cl=9919#kStrokeAndFill_Style">kStrokeAndFill Style</a>, 
and <a href="bmh_SkPaint_Reference?cl=9919#Stroke_Width">Stroke Width</a> is greater than zero, the <a href="bmh_SkPaint_Reference?cl=9919#Stroke_Width">Stroke Width</a>, <a href="bmh_SkPaint_Reference?cl=9919#Stroke_Cap">Stroke Cap</a>, <a href="bmh_SkPaint_Reference?cl=9919#Stroke_Join">Stroke Join</a>,
and <a href="bmh_SkPaint_Reference?cl=9919#Miter_Limit">Miter Limit</a> operate on the destination <a href="bmh_SkPath_Reference?cl=9919#Path">Path</a>, replacing it.

Fill <a href="bmh_SkPath_Reference?cl=9919#Path">Path</a> can specify the precision used by <a href="bmh_SkPaint_Reference?cl=9919#Stroke_Width">Stroke Width</a> to approximate the stroke geometry. 

If the <a href="bmh_SkPaint_Reference?cl=9919#Style">Style</a> is <a href="bmh_SkPaint_Reference?cl=9919#kStroke_Style">kStroke Style</a> and the <a href="bmh_SkPaint_Reference?cl=9919#Stroke_Width">Stroke Width</a> is zero, <a href="bmh_SkPaint_Reference?cl=9919#getFillPath">getFillPath</a>
returns false since <a href="bmh_SkPaint_Reference?cl=9919#Hairline">Hairline</a> has no filled equivalent.

<a name="getFillPath"></a>
## getFillPath

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool getFillPath(const SkPath& src, SkPath* dst, const SkRect* cullRect,
                 SkScalar resScale = 1) const
</pre>

The filled equivalent of the stroked path.

### Parameters

<table>  <tr>    <td><code><strong>src </strong></code></td> <td>
<a href="bmh_SkPath_Reference?cl=9919#Path">Path</a> read to create a filled version</td>
  </tr>  <tr>    <td><code><strong>dst </strong></code></td> <td>
resulting <a href="bmh_SkPath_Reference?cl=9919#Path">Path</a>; may be the same as <a href="bmh_SkPaint_Reference?cl=9919#src">src</a>, but may not be nullptr</td>
  </tr>  <tr>    <td><code><strong>cullRect </strong></code></td> <td>
optional limit passed to <a href="bmh_undocumented?cl=9919#Path_Effect">Path Effect</a></td>
  </tr>  <tr>    <td><code><strong>resScale </strong></code></td> <td>
if > 1, increase precision, else if (0 < res < 1) reduce precision
to favor speed and size</td>
  </tr>
</table>

### Return Value

true if the path represents <a href="bmh_SkPaint_Reference?cl=9919#Style_Fill">Style Fill</a>, or false if it represents <a href="bmh_SkPaint_Reference?cl=9919#Hairline">Hairline</a>

### Example

<div><fiddle-embed name="cedd6233848198e1fca4d1e14816baaf"><div>A very small quad stroke is turned into a filled path with increasing levels of precision.
At the lowest precision, the quad stroke is approximated by a rectangle. 
At the highest precision, the filled path has high fidelity compared to the original stroke.</div></fiddle-embed></div>

---

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool getFillPath(const SkPath& src, SkPath* dst) const
</pre>

The filled equivalent of the stroked path.

Replaces <a href="bmh_SkPaint_Reference?cl=9919#dst">dst</a> with the <a href="bmh_SkPaint_Reference?cl=9919#src">src</a> path modified by <a href="bmh_undocumented?cl=9919#Path_Effect">Path Effect</a> and <a href="bmh_SkPaint_Reference?cl=9919#Style_Stroke">Style Stroke</a>.
<a href="bmh_undocumented?cl=9919#Path_Effect">Path Effect</a>, if any, is not culled. <a href="bmh_SkPaint_Reference?cl=9919#Stroke_Width">Stroke Width</a> is created with default precision.

### Parameters

<table>  <tr>    <td><code><strong>src </strong></code></td> <td>
<a href="bmh_SkPath_Reference?cl=9919#Path">Path</a> read to create a filled version</td>
  </tr>  <tr>    <td><code><strong>dst </strong></code></td> <td>
resulting <a href="bmh_SkPath_Reference?cl=9919#Path">Path</a> <a href="bmh_SkPaint_Reference?cl=9919#dst">dst</a> may be the same as <a href="bmh_SkPaint_Reference?cl=9919#src">src</a>, but may not be nullptr</td>
  </tr>
</table>

### Return Value

true if the path represents <a href="bmh_SkPaint_Reference?cl=9919#Style_Fill">Style Fill</a>, or false if it represents <a href="bmh_SkPaint_Reference?cl=9919#Hairline">Hairline</a>

### Example

<div><fiddle-embed name="e6d8ca0cc17e0b475bd54dd995825468"></fiddle-embed></div>

---

### See Also

<a href="bmh_SkPaint_Reference?cl=9919#Style_Stroke">Style Stroke</a> <a href="bmh_SkPaint_Reference?cl=9919#Stroke_Width">Stroke Width</a> <a href="bmh_undocumented?cl=9919#Path_Effect">Path Effect</a>

# <a name="Shader_Methods"></a> Shader Methods
<a href="bmh_undocumented?cl=9919#Shader">Shader</a> defines the colors used when drawing a shape.
<a href="bmh_undocumented?cl=9919#Shader">Shader</a> may be an image, a gradient, or a computed fill.
If <a href="bmh_SkPaint_Reference?cl=9919#Paint">Paint</a> has no <a href="bmh_undocumented?cl=9919#Shader">Shader</a>, then <a href="bmh_undocumented?cl=9919#Color">Color</a> fills the shape. 

<a href="bmh_undocumented?cl=9919#Shader">Shader</a> is modulated by <a href="bmh_undocumented?cl=9919#Alpha">Color Alpha</a> component of <a href="bmh_undocumented?cl=9919#Color">Color</a>.
If <a href="bmh_undocumented?cl=9919#Shader">Shader</a> object defines only <a href="bmh_undocumented?cl=9919#Alpha">Color Alpha</a>, then <a href="bmh_undocumented?cl=9919#Color">Color</a> modulated by <a href="bmh_undocumented?cl=9919#Alpha">Color Alpha</a> describes
the fill.

The drawn transparency can be modified without altering <a href="bmh_undocumented?cl=9919#Shader">Shader</a>, by changing <a href="bmh_undocumented?cl=9919#Alpha">Color Alpha</a>.

### Example

<div><fiddle-embed name="c015dc2010c15e1c00b4f7330232b0f7"></fiddle-embed></div>

If <a href="bmh_undocumented?cl=9919#Shader">Shader</a> generates only <a href="bmh_undocumented?cl=9919#Alpha">Color Alpha</a> then all components of <a href="bmh_undocumented?cl=9919#Color">Color</a> modulate the output.

### Example

<div><fiddle-embed name="9673be7720ba3adcdae42ddc1565b588"></fiddle-embed></div>

<a name="getShader"></a>
## getShader

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkShader* getShader() const
</pre>

Optional colors used when filling a path, such as a gradient.

Does not alter <a href="bmh_undocumented?cl=9919#Shader">Shader</a> <a href="bmh_undocumented?cl=9919#Reference_Count">Reference Count</a>.

### Return Value

<a href="bmh_undocumented?cl=9919#Shader">Shader</a> if previously set, nullptr otherwise

### Example

<div><fiddle-embed name="09f15b9fd88882850da2d235eb86292f">

#### Example Output

~~~~
nullptr == shader
nullptr != shader
~~~~

</fiddle-embed></div>

---

<a name="refShader"></a>
## refShader

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
sk_sp<SkShader> refShader() const
</pre>

Optional colors used when filling a path, such as a gradient.

Increases <a href="bmh_undocumented?cl=9919#Shader">Shader</a> <a href="bmh_undocumented?cl=9919#Reference_Count">Reference Count</a> by one.

### Return Value

<a href="bmh_undocumented?cl=9919#Shader">Shader</a> if previously set, nullptr otherwise

### Example

<div><fiddle-embed name="53da0295972a418cbc9607bbb17feaa8">

#### Example Output

~~~~
shader unique: true
shader unique: false
~~~~

</fiddle-embed></div>

---

<a name="setShader"></a>
## setShader

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setShader(sk_sp<SkShader> shader)
</pre>

Optional colors used when filling a path, such as a gradient.

Sets <a href="bmh_undocumented?cl=9919#Shader">Shader</a> to <a href="bmh_SkPaint_Reference?cl=9919#shader">shader</a>, decrementing <a href="bmh_undocumented?cl=9919#Reference_Count">Reference Count</a> of the previous <a href="bmh_undocumented?cl=9919#Shader">Shader</a>.
Does not alter <a href="bmh_SkPaint_Reference?cl=9919#shader">shader</a> <a href="bmh_undocumented?cl=9919#Reference_Count">Reference Count</a>.

### Parameters

<table>  <tr>    <td><code><strong>shader </strong></code></td> <td>
how geometry is filled with color; if nullptr, <a href="bmh_undocumented?cl=9919#Color">Color</a> is used instead</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="77e64d5bae9b1ba037fd99252bb4aa58"></fiddle-embed></div>

---

# <a name="Color_Filter_Methods"></a> Color Filter Methods
<a href="bmh_undocumented?cl=9919#Color_Filter">Color Filter</a> alters the color used when drawing a shape.
<a href="bmh_undocumented?cl=9919#Color_Filter">Color Filter</a> may apply <a href="bmh_undocumented?cl=9919#Blend_Mode">Blend Mode</a>, transform the color through a matrix, or composite multiple filters.
If <a href="bmh_SkPaint_Reference?cl=9919#Paint">Paint</a> has no <a href="bmh_undocumented?cl=9919#Color_Filter">Color Filter</a>, the color is unaltered.

The drawn transparency can be modified without altering <a href="bmh_undocumented?cl=9919#Color_Filter">Color Filter</a>, by changing <a href="bmh_undocumented?cl=9919#Alpha">Color Alpha</a>.

### Example

<div><fiddle-embed name="5abde56ca2f89a18b8e231abd1b57c56"></fiddle-embed></div>

<a name="getColorFilter"></a>
## getColorFilter

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkColorFilter* getColorFilter() const
</pre>

Returns <a href="bmh_undocumented?cl=9919#Color_Filter">Color Filter</a> if set, or nullptr.
Does not alter <a href="bmh_undocumented?cl=9919#Color_Filter">Color Filter</a> <a href="bmh_undocumented?cl=9919#Reference_Count">Reference Count</a>.

### Return Value

<a href="bmh_undocumented?cl=9919#Color_Filter">Color Filter</a> if previously set, nullptr otherwise

### Example

<div><fiddle-embed name="093bdc627d6b59002670fd290931f6c9">

#### Example Output

~~~~
nullptr == color filter
nullptr != color filter
~~~~

</fiddle-embed></div>

---

<a name="refColorFilter"></a>
## refColorFilter

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
sk_sp<SkColorFilter> refColorFilter() const
</pre>

Returns <a href="bmh_undocumented?cl=9919#Color_Filter">Color Filter</a> if set, or nullptr.
Increases <a href="bmh_undocumented?cl=9919#Color_Filter">Color Filter</a> <a href="bmh_undocumented?cl=9919#Reference_Count">Reference Count</a> by one.

### Return Value

<a href="bmh_undocumented?cl=9919#Color_Filter">Color Filter</a> if set, or nullptr

### Example

<div><fiddle-embed name="b588c95fa4c86ddbc4b0546762f08297">

#### Example Output

~~~~
color filter unique: true
color filter unique: false
~~~~

</fiddle-embed></div>

---

<a name="setColorFilter"></a>
## setColorFilter

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setColorFilter(sk_sp<SkColorFilter> colorFilter)
</pre>

Sets <a href="bmh_undocumented?cl=9919#Color_Filter">Color Filter</a> to filter, decrementing <a href="bmh_undocumented?cl=9919#Reference_Count">Reference Count</a> of the previous <a href="bmh_undocumented?cl=9919#Color_Filter">Color Filter</a>. 
Pass nullptr to clear <a href="bmh_undocumented?cl=9919#Color_Filter">Color Filter</a>.
Does not alter filter <a href="bmh_undocumented?cl=9919#Reference_Count">Reference Count</a>.

### Parameters

<table>  <tr>    <td><code><strong>colorFilter </strong></code></td> <td>
<a href="bmh_undocumented?cl=9919#Color_Filter">Color Filter</a> to apply to subsequent draw</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="c7b786dc9b3501cd0eaba47494b6fa31"></fiddle-embed></div>

---

# <a name="Blend_Mode_Methods"></a> Blend Mode Methods
<a href="bmh_undocumented?cl=9919#Blend_Mode">Blend Mode</a> describes how <a href="bmh_undocumented?cl=9919#Color">Color</a> combines with the destination color.
The default setting, <a href="bmh_undocumented?cl=9919#kSrcOver">SkBlendMode::kSrcOver</a>, draws the source color
over the destination color.

### Example

<div><fiddle-embed name="73092d4d06faecea3c204d852a4dd8a8"></fiddle-embed></div>

### See Also

<a href="bmh_undocumented?cl=9919#Blend_Mode">Blend Mode</a>

<a name="getBlendMode"></a>
## getBlendMode

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkBlendMode getBlendMode() const
</pre>

Returns <a href="bmh_undocumented?cl=9919#Blend_Mode">Blend Mode</a>.
By default, <a href="bmh_SkPaint_Reference?cl=9919#getBlendMode">getBlendMode</a> returns <a href="bmh_undocumented?cl=9919#kSrcOver">SkBlendMode::kSrcOver</a>.

### Return Value

mode used to combine source color with destination color

### Example

<div><fiddle-embed name="4ec1864b8203d52c0810e8605092f45c">

#### Example Output

~~~~
kSrcOver == getBlendMode
kSrcOver != getBlendMode
~~~~

</fiddle-embed></div>

---

<a name="isSrcOver"></a>
## isSrcOver

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool isSrcOver() const
</pre>

Returns true if <a href="bmh_undocumented?cl=9919#Blend_Mode">Blend Mode</a> is <a href="bmh_undocumented?cl=9919#kSrcOver">SkBlendMode::kSrcOver</a>, the default.

### Return Value

true if <a href="bmh_undocumented?cl=9919#Blend_Mode">Blend Mode</a> is <a href="bmh_undocumented?cl=9919#kSrcOver">SkBlendMode::kSrcOver</a>

### Example

<div><fiddle-embed name="257c9473db7a2b3a0fb2b9e2431e59a6">

#### Example Output

~~~~
isSrcOver == true
isSrcOver != true
~~~~

</fiddle-embed></div>

---

<a name="setBlendMode"></a>
## setBlendMode

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setBlendMode(SkBlendMode mode)
</pre>

Sets <a href="bmh_undocumented?cl=9919#Blend_Mode">Blend Mode</a> to <a href="bmh_SkPaint_Reference?cl=9919#mode">mode</a>. 
Does not check for valid input.

### Parameters

<table>  <tr>    <td><code><strong>mode </strong></code></td> <td>
<a href="bmh_undocumented?cl=9919#SkBlendMode">SkBlendMode</a> used to combine source color and destination</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="257c9473db7a2b3a0fb2b9e2431e59a6">

#### Example Output

~~~~
isSrcOver == true
isSrcOver != true
~~~~

</fiddle-embed></div>

---

# <a name="Path_Effect_Methods"></a> Path Effect Methods
<a href="bmh_undocumented?cl=9919#Path_Effect">Path Effect</a> modifies the path geometry before drawing it.
<a href="bmh_undocumented?cl=9919#Path_Effect">Path Effect</a> may implement dashing, custom fill effects and custom stroke effects.
If <a href="bmh_SkPaint_Reference?cl=9919#Paint">Paint</a> has no <a href="bmh_undocumented?cl=9919#Path_Effect">Path Effect</a>, the path geometry is unaltered when filled or stroked.

### Example

<div><fiddle-embed name="8cf5684b187d60f09e11c4a48993ea39"></fiddle-embed></div>

### See Also

<a href="bmh_undocumented?cl=9919#Path_Effect">Path Effect</a>

<a name="getPathEffect"></a>
## getPathEffect

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkPathEffect* getPathEffect() const
</pre>

Returns <a href="bmh_undocumented?cl=9919#Path_Effect">Path Effect</a> if set, or nullptr.
Does not alter <a href="bmh_undocumented?cl=9919#Path_Effect">Path Effect</a> <a href="bmh_undocumented?cl=9919#Reference_Count">Reference Count</a>.

### Return Value

<a href="bmh_undocumented?cl=9919#Path_Effect">Path Effect</a> if previously set, nullptr otherwise

### Example

<div><fiddle-embed name="211a1b14bfa6c4332082c8eab4fbc5fd">

#### Example Output

~~~~
nullptr == path effect
nullptr != path effect
~~~~

</fiddle-embed></div>

---

<a name="refPathEffect"></a>
## refPathEffect

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
sk_sp<SkPathEffect> refPathEffect() const
</pre>

Returns <a href="bmh_undocumented?cl=9919#Path_Effect">Path Effect</a> if set, or nullptr.
Increases <a href="bmh_undocumented?cl=9919#Path_Effect">Path Effect</a> <a href="bmh_undocumented?cl=9919#Reference_Count">Reference Count</a> by one.

### Return Value

<a href="bmh_undocumented?cl=9919#Path_Effect">Path Effect</a> if previously set, nullptr otherwise

### Example

<div><fiddle-embed name="c55c74f8f581870bd2c18f2f99765579">

#### Example Output

~~~~
path effect unique: true
path effect unique: false
~~~~

</fiddle-embed></div>

---

<a name="setPathEffect"></a>
## setPathEffect

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setPathEffect(sk_sp<SkPathEffect> pathEffect)
</pre>

Sets <a href="bmh_undocumented?cl=9919#Path_Effect">Path Effect</a> to <a href="bmh_SkPaint_Reference?cl=9919#pathEffect">pathEffect</a>, 
decrementing <a href="bmh_undocumented?cl=9919#Reference_Count">Reference Count</a> of the previous <a href="bmh_undocumented?cl=9919#Path_Effect">Path Effect</a>. 
Pass nullptr to leave the path geometry unaltered.
Does not alter <a href="bmh_SkPaint_Reference?cl=9919#pathEffect">pathEffect</a> <a href="bmh_undocumented?cl=9919#Reference_Count">Reference Count</a>.

### Parameters

<table>  <tr>    <td><code><strong>pathEffect </strong></code></td> <td>
replace <a href="bmh_SkPath_Reference?cl=9919#Path">Path</a> with a modification when drawn</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="52dd55074ca0b7d520d04e750ca2a0d7"></fiddle-embed></div>

---

# <a name="Mask_Filter_Methods"></a> Mask Filter Methods
<a href="bmh_undocumented?cl=9919#Mask_Filter">Mask Filter</a> uses <a href="bmh_undocumented?cl=9919#Alpha">Color Alpha</a> of the shape drawn to create <a href="bmh_undocumented?cl=9919#Mask_Alpha">Mask Alpha</a>.
<a href="bmh_undocumented?cl=9919#Mask_Filter">Mask Filter</a> operates at a lower level than <a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a>; <a href="bmh_undocumented?cl=9919#Mask_Filter">Mask Filter</a> takes a <a href="bmh_undocumented?cl=9919#Mask">Mask</a>,
and returns a <a href="bmh_undocumented?cl=9919#Mask">Mask</a>.
<a href="bmh_undocumented?cl=9919#Mask_Filter">Mask Filter</a> may change the geometry and transparency of the shape, such as creating a blur effect.
Set <a href="bmh_undocumented?cl=9919#Mask_Filter">Mask Filter</a> to nullptr to prevent <a href="bmh_undocumented?cl=9919#Mask_Filter">Mask Filter</a> from modifying the draw.

### Example

<div><fiddle-embed name="320b04ea1e1291d49f1e61994a0410fe"></fiddle-embed></div>

<a name="getMaskFilter"></a>
## getMaskFilter

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkMaskFilter* getMaskFilter() const
</pre>

Returns <a href="bmh_undocumented?cl=9919#Mask_Filter">Mask Filter</a> if set, or nullptr.
Does not alter <a href="bmh_undocumented?cl=9919#Mask_Filter">Mask Filter</a> <a href="bmh_undocumented?cl=9919#Reference_Count">Reference Count</a>.

### Return Value

<a href="bmh_undocumented?cl=9919#Mask_Filter">Mask Filter</a> if previously set, nullptr otherwise

### Example

<div><fiddle-embed name="8cd53ece8fc83e4560599ace094b0f16">

#### Example Output

~~~~
nullptr == mask filter
nullptr != mask filter
~~~~

</fiddle-embed></div>

---

<a name="refMaskFilter"></a>
## refMaskFilter

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
sk_sp<SkMaskFilter> refMaskFilter() const
</pre>

Returns <a href="bmh_undocumented?cl=9919#Mask_Filter">Mask Filter</a> if set, or nullptr.
Increases <a href="bmh_undocumented?cl=9919#Mask_Filter">Mask Filter</a> <a href="bmh_undocumented?cl=9919#Reference_Count">Reference Count</a> by one.

### Return Value

<a href="bmh_undocumented?cl=9919#Mask_Filter">Mask Filter</a> if previously set, nullptr otherwise

### Example

<div><fiddle-embed name="35a397dce5d44658ee4e9e9dfb9fee22">

#### Example Output

~~~~
mask filter unique: true
mask filter unique: false
~~~~

</fiddle-embed></div>

---

<a name="setMaskFilter"></a>
## setMaskFilter

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setMaskFilter(sk_sp<SkMaskFilter> maskFilter)
</pre>

Sets <a href="bmh_undocumented?cl=9919#Mask_Filter">Mask Filter</a> to <a href="bmh_SkPaint_Reference?cl=9919#maskFilter">maskFilter</a>,
decrementing <a href="bmh_undocumented?cl=9919#Reference_Count">Reference Count</a> of the previous <a href="bmh_undocumented?cl=9919#Mask_Filter">Mask Filter</a>. 
Pass nullptr to clear <a href="bmh_undocumented?cl=9919#Mask_Filter">Mask Filter</a> and leave <a href="bmh_undocumented?cl=9919#Mask_Filter">Mask Filter</a> effect on <a href="bmh_undocumented?cl=9919#Mask_Alpha">Mask Alpha</a> unaltered.
Does not affect <a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a>.
Does not alter <a href="bmh_SkPaint_Reference?cl=9919#maskFilter">maskFilter</a> <a href="bmh_undocumented?cl=9919#Reference_Count">Reference Count</a>.

### Parameters

<table>  <tr>    <td><code><strong>maskFilter </strong></code></td> <td>
modifies clipping mask generated from drawn geometry</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="62c5a826692f85c3de3bab65e9e97aa9"></fiddle-embed></div>

---

# <a name="Typeface_Methods"></a> Typeface Methods
<a href="bmh_undocumented?cl=9919#Typeface">Typeface</a> identifies the font used when drawing and measuring text.
<a href="bmh_undocumented?cl=9919#Typeface">Typeface</a> may be specified by name, from a file, or from a data stream.
The default <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a> defers to the platform-specific default font
implementation.

### Example

<div><fiddle-embed name="c18b1696b8c1649bebf7eb1f8b89e0b0"></fiddle-embed></div>

<a name="getTypeface"></a>
## getTypeface

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkTypeface* getTypeface() const
</pre>

Returns <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a> if set, or nullptr.
Does not alter <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a> <a href="bmh_undocumented?cl=9919#Reference_Count">Reference Count</a>.

### Return Value

<a href="bmh_undocumented?cl=9919#Typeface">Typeface</a> if previously set, nullptr otherwise

### Example

<div><fiddle-embed name="4d9ffb5761b62a9e3bc9b0bca8787bce">

#### Example Output

~~~~
nullptr == typeface
nullptr != typeface
~~~~

</fiddle-embed></div>

---

<a name="refTypeface"></a>
## refTypeface

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
sk_sp<SkTypeface> refTypeface() const
</pre>

Increases <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a> <a href="bmh_undocumented?cl=9919#Reference_Count">Reference Count</a> by one.

### Return Value

<a href="bmh_undocumented?cl=9919#Typeface">Typeface</a> if previously set, nullptr otherwise

### Example

<div><fiddle-embed name="c8edce7b36a3ffda8af4fe89d7187dbc">

#### Example Output

~~~~
typeface1 != typeface2
typeface1 == typeface2
~~~~

</fiddle-embed></div>

---

<a name="setTypeface"></a>
## setTypeface

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setTypeface(sk_sp<SkTypeface> typeface)
</pre>

Sets <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a> to <a href="bmh_SkPaint_Reference?cl=9919#typeface">typeface</a>,
decrementing <a href="bmh_undocumented?cl=9919#Reference_Count">Reference Count</a> of the previous <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a>. 
Pass nullptr to clear <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a> and use the default <a href="bmh_SkPaint_Reference?cl=9919#typeface">typeface</a>.
Does not alter <a href="bmh_SkPaint_Reference?cl=9919#typeface">typeface</a> <a href="bmh_undocumented?cl=9919#Reference_Count">Reference Count</a>.

### Parameters

<table>  <tr>    <td><code><strong>typeface </strong></code></td> <td>
font and style used to draw text</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="3d2656ec4c555ed2c7ec086720124a2a"></fiddle-embed></div>

---

# <a name="Rasterizer_Methods"></a> Rasterizer Methods
<a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a> controls how shapes are converted to <a href="bmh_undocumented?cl=9919#Mask_Alpha">Mask Alpha</a>. 
<a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a> operates at a higher level than <a href="bmh_undocumented?cl=9919#Mask_Filter">Mask Filter</a>; <a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a> takes a <a href="bmh_SkPath_Reference?cl=9919#Path">Path</a>,
and returns a <a href="bmh_undocumented?cl=9919#Mask">Mask</a>.
<a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a> may change the geometry and transparency of the shape, such as
creating a shadow effect. <a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a> forms the base of <a href="bmh_undocumented?cl=9919#Layer">Rasterizer Layer</a>, which
creates effects like embossing and outlining.
<a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a> applies to <a href="bmh_undocumented?cl=9919#Rect">Rect</a>, <a href="bmh_undocumented?cl=9919#Region">Region</a>, <a href="bmh_undocumented?cl=9919#Round_Rect">Round Rect</a>, <a href="bmh_undocumented?cl=9919#Arc">Arc</a>, <a href="bmh_undocumented?cl=9919#Circle">Circle</a>, <a href="bmh_undocumented?cl=9919#Oval">Oval</a>,
<a href="bmh_SkPath_Reference?cl=9919#Path">Path</a>, and <a href="bmh_undocumented?cl=9919#Text">Text</a>.

### Example

<div><fiddle-embed name="e63f8a50996699342a14c6e54d684108"></fiddle-embed></div>

<a name="getRasterizer"></a>
## getRasterizer

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkRasterizer* getRasterizer() const
</pre>

Returns <a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a> if set, or nullptr.
Does not alter <a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a> <a href="bmh_undocumented?cl=9919#Reference_Count">Reference Count</a>.

### Return Value

<a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a> if previously set, nullptr otherwise

### Example

<div><fiddle-embed name="0707d407c3a14388b107af8ae5873e55">

#### Example Output

~~~~
nullptr == rasterizer
nullptr != rasterizer
~~~~

</fiddle-embed></div>

---

<a name="refRasterizer"></a>
## refRasterizer

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
sk_sp<SkRasterizer> refRasterizer() const
</pre>

Returns <a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a> if set, or nullptr.
Increases <a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a> <a href="bmh_undocumented?cl=9919#Reference_Count">Reference Count</a> by one.

### Return Value

<a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a> if previously set, nullptr otherwise

### Example

<div><fiddle-embed name="c0855ce19a33cb7e5747750ef341b7b3">

#### Example Output

~~~~
rasterizer unique: true
rasterizer unique: false
~~~~

</fiddle-embed></div>

---

<a name="setRasterizer"></a>
## setRasterizer

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setRasterizer(sk_sp<SkRasterizer> rasterizer)
</pre>

Sets <a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a> to <a href="bmh_SkPaint_Reference?cl=9919#rasterizer">rasterizer</a>,
decrementing <a href="bmh_undocumented?cl=9919#Reference_Count">Reference Count</a> of the previous <a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a>. 
Pass nullptr to clear <a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a> and leave <a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a> effect on <a href="bmh_undocumented?cl=9919#Mask_Alpha">Mask Alpha</a> unaltered.
Does not affect <a href="bmh_undocumented?cl=9919#Mask_Filter">Mask Filter</a>.
Does not alter <a href="bmh_SkPaint_Reference?cl=9919#rasterizer">rasterizer</a> <a href="bmh_undocumented?cl=9919#Reference_Count">Reference Count</a>.

### Parameters

<table>  <tr>    <td><code><strong>rasterizer </strong></code></td> <td>
how geometry is converted to <a href="bmh_undocumented?cl=9919#Mask_Alpha">Mask Alpha</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="aec8ed9296c1628073086a33039f62b7"></fiddle-embed></div>

---

# <a name="Image_Filter_Methods"></a> Image Filter Methods
<a href="bmh_undocumented?cl=9919#Image_Filter">Image Filter</a> operates on the pixel representation of the shape, as modified by <a href="bmh_SkPaint_Reference?cl=9919#Paint">Paint</a>
with <a href="bmh_undocumented?cl=9919#Blend_Mode">Blend Mode</a> set to <a href="bmh_undocumented?cl=9919#kSrcOver">SkBlendMode::kSrcOver</a>. <a href="bmh_undocumented?cl=9919#Image_Filter">Image Filter</a> creates a new bitmap,
which is drawn to the device using the set <a href="bmh_undocumented?cl=9919#Blend_Mode">Blend Mode</a>.
<a href="bmh_undocumented?cl=9919#Image_Filter">Image Filter</a> is higher level than <a href="bmh_undocumented?cl=9919#Mask_Filter">Mask Filter</a>; for instance, an <a href="bmh_undocumented?cl=9919#Image_Filter">Image Filter</a>
can operate on all channels of <a href="bmh_undocumented?cl=9919#Color">Color</a>, while <a href="bmh_undocumented?cl=9919#Mask_Filter">Mask Filter</a> generates <a href="bmh_undocumented?cl=9919#Alpha">Color Alpha</a> only.
<a href="bmh_undocumented?cl=9919#Image_Filter">Image Filter</a> operates independently of and can be used in combination with
<a href="bmh_undocumented?cl=9919#Mask_Filter">Mask Filter</a> and <a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a>.

### Example

<div><fiddle-embed name="88804938b49eb4f7c7f01ad52f4db0d8"></fiddle-embed></div>

<a name="getImageFilter"></a>
## getImageFilter

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkImageFilter* getImageFilter() const
</pre>

Returns <a href="bmh_undocumented?cl=9919#Image_Filter">Image Filter</a> if set, or nullptr.
Does not alter <a href="bmh_undocumented?cl=9919#Image_Filter">Image Filter</a> <a href="bmh_undocumented?cl=9919#Reference_Count">Reference Count</a>.

### Return Value

<a href="bmh_undocumented?cl=9919#Image_Filter">Image Filter</a> if previously set, nullptr otherwise

### Example

<div><fiddle-embed name="38788d42772641606e08c60d9dd418a2">

#### Example Output

~~~~
nullptr == image filter
nullptr != image filter
~~~~

</fiddle-embed></div>

---

<a name="refImageFilter"></a>
## refImageFilter

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
sk_sp<SkImageFilter> refImageFilter() const
</pre>

Returns <a href="bmh_undocumented?cl=9919#Image_Filter">Image Filter</a> if set, or nullptr.
Increases <a href="bmh_undocumented?cl=9919#Image_Filter">Image Filter</a> <a href="bmh_undocumented?cl=9919#Reference_Count">Reference Count</a> by one.

### Return Value

<a href="bmh_undocumented?cl=9919#Image_Filter">Image Filter</a> if previously set, nullptr otherwise

### Example

<div><fiddle-embed name="13f09088b569251547107d14ae989dc1">

#### Example Output

~~~~
image filter unique: true
image filter unique: false
~~~~

</fiddle-embed></div>

---

<a name="setImageFilter"></a>
## setImageFilter

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setImageFilter(sk_sp<SkImageFilter> imageFilter)
</pre>

Sets <a href="bmh_undocumented?cl=9919#Image_Filter">Image Filter</a> to <a href="bmh_SkPaint_Reference?cl=9919#imageFilter">imageFilter</a>,
decrementing <a href="bmh_undocumented?cl=9919#Reference_Count">Reference Count</a> of the previous <a href="bmh_undocumented?cl=9919#Image_Filter">Image Filter</a>. 
Pass nullptr to clear <a href="bmh_undocumented?cl=9919#Image_Filter">Image Filter</a>, and remove <a href="bmh_undocumented?cl=9919#Image_Filter">Image Filter</a> effect
on drawing.
Does not affect <a href="bmh_undocumented?cl=9919#Rasterizer">Rasterizer</a> or <a href="bmh_undocumented?cl=9919#Mask_Filter">Mask Filter</a>.
Does not alter <a href="bmh_SkPaint_Reference?cl=9919#imageFilter">imageFilter</a> <a href="bmh_undocumented?cl=9919#Reference_Count">Reference Count</a>.

### Parameters

<table>  <tr>    <td><code><strong>imageFilter </strong></code></td> <td>
how <a href="bmh_undocumented?cl=9919#Image">Image</a> is sampled when transformed</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="6679d6e4ec632715ee03e68391bd7f9a"></fiddle-embed></div>

---

# <a name="Draw_Looper_Methods"></a> Draw Looper Methods
<a href="bmh_undocumented?cl=9919#Draw_Looper">Draw Looper</a> sets a modifier that communicates state from one <a href="bmh_undocumented?cl=9919#Draw_Layer">Draw Layer</a>
to another to construct the draw.
<a href="bmh_undocumented?cl=9919#Draw_Looper">Draw Looper</a> draws one or more times, modifying the canvas and paint each time.
<a href="bmh_undocumented?cl=9919#Draw_Looper">Draw Looper</a> may be used to draw multiple colors or create a colored shadow.
Set <a href="bmh_undocumented?cl=9919#Draw_Looper">Draw Looper</a> to nullptr to prevent <a href="bmh_undocumented?cl=9919#Draw_Looper">Draw Looper</a> from modifying the draw. 

### Example

<div><fiddle-embed name="84ec12a36e50df5ac565cc7a75ffbe9f"></fiddle-embed></div>

<a name="getDrawLooper"></a>
## getDrawLooper

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkDrawLooper* getDrawLooper() const
</pre>

Returns <a href="bmh_undocumented?cl=9919#Draw_Looper">Draw Looper</a> if set, or nullptr.
Does not alter <a href="bmh_undocumented?cl=9919#Draw_Looper">Draw Looper</a> <a href="bmh_undocumented?cl=9919#Reference_Count">Reference Count</a>.

### Return Value

<a href="bmh_undocumented?cl=9919#Draw_Looper">Draw Looper</a> if previously set, nullptr otherwise

### Example

<div><fiddle-embed name="af4c5acc7a91e7f23c2af48018903ad4">

#### Example Output

~~~~
nullptr == draw looper
nullptr != draw looper
~~~~

</fiddle-embed></div>

---

<a name="refDrawLooper"></a>
## refDrawLooper

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
sk_sp<SkDrawLooper> refDrawLooper() const
</pre>

Returns <a href="bmh_undocumented?cl=9919#Draw_Looper">Draw Looper</a> if set, or nullptr.
Increases <a href="bmh_undocumented?cl=9919#Draw_Looper">Draw Looper</a> <a href="bmh_undocumented?cl=9919#Reference_Count">Reference Count</a> by one.

### Return Value

<a href="bmh_undocumented?cl=9919#Draw_Looper">Draw Looper</a> if previously set, nullptr otherwise

### Example

<div><fiddle-embed name="2a3782c33f04ed17a725d0e449c6f7c3">

#### Example Output

~~~~
draw looper unique: true
draw looper unique: false
~~~~

</fiddle-embed></div>

---

<a name="getLooper"></a>
## getLooper

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkDrawLooper* getLooper() const
</pre>

Deprecated.

(see bug.skia.org/6259)

### Return Value

<a href="bmh_undocumented?cl=9919#Draw_Looper">Draw Looper</a> if previously set, nullptr otherwise

---

<a name="setDrawLooper"></a>
## setDrawLooper

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setDrawLooper(sk_sp<SkDrawLooper> drawLooper)
</pre>

Sets <a href="bmh_undocumented?cl=9919#Draw_Looper">Draw Looper</a> to <a href="bmh_SkPaint_Reference?cl=9919#drawLooper">drawLooper</a>,
decrementing <a href="bmh_undocumented?cl=9919#Reference_Count">Reference Count</a> of the previous <a href="bmh_SkPaint_Reference?cl=9919#drawLooper">drawLooper</a>. 
Pass nullptr to clear <a href="bmh_undocumented?cl=9919#Draw_Looper">Draw Looper</a> and leave <a href="bmh_undocumented?cl=9919#Draw_Looper">Draw Looper</a> effect on drawing unaltered.
<a href="bmh_SkPaint_Reference?cl=9919#setDrawLooper">setDrawLooper</a> does not alter <a href="bmh_SkPaint_Reference?cl=9919#drawLooper">drawLooper</a> <a href="bmh_undocumented?cl=9919#Reference_Count">Reference Count</a>.

### Parameters

<table>  <tr>    <td><code><strong>drawLooper </strong></code></td> <td>
Iterates through drawing one or more time, altering <a href="bmh_SkPaint_Reference?cl=9919#Paint">Paint</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="bf10f838b330f0a3a3266d42ea68a638"></fiddle-embed></div>

---

<a name="setLooper"></a>
## setLooper

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setLooper(sk_sp<SkDrawLooper> drawLooper)
</pre>

Deprecated.

(see bug.skia.org/6259)

### Parameters

<table>  <tr>    <td><code><strong>drawLooper </strong></code></td> <td>
sets <a href="bmh_undocumented?cl=9919#Draw_Looper">Draw Looper</a> to <a href="bmh_SkPaint_Reference?cl=9919#drawLooper">drawLooper</a></td>
  </tr>

---

</table>

# <a name="Text_Align"></a> Text Align

## <a name="SkPaint::Align"></a> Enum SkPaint::Align

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
enum <a href="bmh_SkPaint_Reference?cl=9919#Align">Align</a> {
<a href="bmh_SkPaint_Reference?cl=9919#kLeft_Align">kLeft Align</a>,
<a href="bmh_SkPaint_Reference?cl=9919#kCenter_Align">kCenter Align</a>,
<a href="bmh_SkPaint_Reference?cl=9919#kRight_Align">kRight Align</a>,
};</pre>

<a href="bmh_SkPaint_Reference?cl=9919#Align">Align</a> adjusts the text relative to the text position.
<a href="bmh_SkPaint_Reference?cl=9919#Align">Align</a> affects glyphs drawn with: <a href="bmh_SkCanvas_Reference?cl=9919#drawText">SkCanvas::drawText</a>, <a href="bmh_SkCanvas_Reference?cl=9919#drawPosText">SkCanvas::drawPosText</a>,
<a href="bmh_SkCanvas_Reference?cl=9919#drawPosTextH">SkCanvas::drawPosTextH</a>, <a href="bmh_SkCanvas_Reference?cl=9919#drawTextOnPath">SkCanvas::drawTextOnPath</a>, 
<a href="bmh_SkCanvas_Reference?cl=9919#drawTextOnPathHV">SkCanvas::drawTextOnPathHV</a>, <a href="bmh_SkCanvas_Reference?cl=9919#drawTextRSXform">SkCanvas::drawTextRSXform</a>, <a href="bmh_SkCanvas_Reference?cl=9919#drawTextBlob">SkCanvas::drawTextBlob</a>,
and <a href="bmh_SkCanvas_Reference?cl=9919#drawString">SkCanvas::drawString</a>; 
as well as calls that place text glyphs like <a href="bmh_SkPaint_Reference?cl=9919#getTextWidths">getTextWidths</a> and <a href="bmh_SkPaint_Reference?cl=9919#getTextPath">getTextPath</a>.

The text position is set by the font for both horizontal and vertical text.
Typically, for horizontal text, the position is to the left side of the glyph on the
base line; and for vertical text, the position is the horizontal center of the glyph
at the caps height.

<a href="bmh_SkPaint_Reference?cl=9919#Align">Align</a> adjusts the glyph position to center it or move it to abut the position 
using the metrics returned by the font.

<a href="bmh_SkPaint_Reference?cl=9919#Align">Align</a> defaults to <a href="bmh_SkPaint_Reference?cl=9919#kLeft_Align">kLeft Align</a>.

### Constants

<table>
  <tr>
    <td><a name="SkPaint::kLeft_Align"></a> <code><strong>SkPaint::kLeft_Align </strong></code></td><td>0</td><td>Leaves the glyph at the position computed by the font offset by the text position.</td>
  </tr>
  <tr>
    <td><a name="SkPaint::kCenter_Align"></a> <code><strong>SkPaint::kCenter_Align </strong></code></td><td>1</td><td>Moves the glyph half its width if <a href="bmh_SkPaint_Reference?cl=9919#Flags">Flags</a> has <a href="bmh_SkPaint_Reference?cl=9919#kVerticalText_Flag">kVerticalText Flag</a> clear, and
half its height if <a href="bmh_SkPaint_Reference?cl=9919#Flags">Flags</a> has <a href="bmh_SkPaint_Reference?cl=9919#kVerticalText_Flag">kVerticalText Flag</a> set.</td>
  </tr>
  <tr>
    <td><a name="SkPaint::kRight_Align"></a> <code><strong>SkPaint::kRight_Align </strong></code></td><td>2</td><td>Moves the glyph by its width if <a href="bmh_SkPaint_Reference?cl=9919#Flags">Flags</a> has <a href="bmh_SkPaint_Reference?cl=9919#kVerticalText_Flag">kVerticalText Flag</a> clear,
and by its height if <a href="bmh_SkPaint_Reference?cl=9919#Flags">Flags</a> has <a href="bmh_SkPaint_Reference?cl=9919#kVerticalText_Flag">kVerticalText Flag</a> set.</td>
  </tr>

</table>

## <a name="SkPaint::_anonymous_2"></a> Enum SkPaint::_anonymous_2

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
enum {
<a href="bmh_SkPaint_Reference?cl=9919#kAlignCount">kAlignCount</a> = 3
};</pre>

### Constants

<table>
  <tr>
    <td><a name="SkPaint::kAlignCount"></a> <code><strong>SkPaint::kAlignCount </strong></code></td><td>3</td><td>The number of different <a href="bmh_SkPaint_Reference?cl=9919#Text_Align">Text Align</a> values defined.</td>
  </tr>

</table>

### Example

<div><fiddle-embed name="702617fd9ebc3f12e30081b5db93e8a8"><div>Each position separately moves the glyph in drawPosText.</div></fiddle-embed></div>

### Example

<div><fiddle-embed name="f1cbbbafe6b3c52b81309cccbf96a308"><div><a href="bmh_SkPaint_Reference?cl=9919#Vertical_Text">Vertical Text</a> treats <a href="bmh_SkPaint_Reference?cl=9919#kLeft_Align">kLeft Align</a> as top align, and <a href="bmh_SkPaint_Reference?cl=9919#kRight_Align">kRight Align</a> as bottom align.</div></fiddle-embed></div>

<a name="getTextAlign"></a>
## getTextAlign

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
Align getTextAlign() const
</pre>

Returns <a href="bmh_SkPaint_Reference?cl=9919#Text_Align">Text Align</a>.
Returns <a href="bmh_SkPaint_Reference?cl=9919#kLeft_Align">kLeft Align</a> if <a href="bmh_SkPaint_Reference?cl=9919#Text_Align">Text Align</a> has not been set.

### Return Value

text placement relative to position

### Example

<div><fiddle-embed name="2df932f526e810f74c89d30ec3f4c947">

#### Example Output

~~~~
kLeft_Align == default
~~~~

</fiddle-embed></div>

---

<a name="setTextAlign"></a>
## setTextAlign

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void    setTextAlign(Align align)
</pre>

Sets <a href="bmh_SkPaint_Reference?cl=9919#Text_Align">Text Align</a> to <a href="bmh_SkPaint_Reference?cl=9919#align">align</a>.
Has no effect if <a href="bmh_SkPaint_Reference?cl=9919#align">align</a> is an invalid value.

### Parameters

<table>  <tr>    <td><code><strong>align </strong></code></td> <td>
text placement relative to position</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="d37540afd918506ac2594665ca63979b"><div><a href="bmh_undocumented?cl=9919#Text">Text</a> is left-aligned by default, and then set to center. Setting the
alignment out of range has no effect.</div></fiddle-embed></div>

---

# <a name="Text_Size"></a> Text Size
<a href="bmh_SkPaint_Reference?cl=9919#Text_Size">Text Size</a> adjusts the overall text size in points.
<a href="bmh_SkPaint_Reference?cl=9919#Text_Size">Text Size</a> can be set to any positive value or zero.
<a href="bmh_SkPaint_Reference?cl=9919#Text_Size">Text Size</a> defaults to 12.
Set <a href="bmh_undocumented?cl=9919#SkPaintDefaults_TextSize">SkPaintDefaults TextSize</a> at compile time to change the default setting.

### Example

<div><fiddle-embed name="91c9a3e498bb9412e4522a95d076ed5f"></fiddle-embed></div>

<a name="getTextSize"></a>
## getTextSize

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkScalar getTextSize() const
</pre>

Returns <a href="bmh_SkPaint_Reference?cl=9919#Text_Size">Text Size</a> in points.

### Return Value

typographic height of text

### Example

<div><fiddle-embed name="983e2a71ba72d4ba8c945420040b8f1c"></fiddle-embed></div>

---

<a name="setTextSize"></a>
## setTextSize

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setTextSize(SkScalar textSize)
</pre>

Sets <a href="bmh_SkPaint_Reference?cl=9919#Text_Size">Text Size</a> in points.
Has no effect if <a href="bmh_SkPaint_Reference?cl=9919#textSize">textSize</a> is not greater than or equal to zero.

### Parameters

<table>  <tr>    <td><code><strong>textSize </strong></code></td> <td>
typographic height of text</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="6510c9e2f57b83c47e67829e7a68d493"></fiddle-embed></div>

---

# <a name="Text_Scale_X"></a> Text Scale X
<a href="bmh_SkPaint_Reference?cl=9919#Text_Scale_X">Text Scale X</a> adjusts the text horizontal scale.
<a href="bmh_undocumented?cl=9919#Text">Text</a> scaling approximates condensed and expanded type faces when the actual face
is not available.
<a href="bmh_SkPaint_Reference?cl=9919#Text_Scale_X">Text Scale X</a> can be set to any value.
<a href="bmh_SkPaint_Reference?cl=9919#Text_Scale_X">Text Scale X</a> defaults to 1.

### Example

<div><fiddle-embed name="d13d787c1e36f515319fc998411c1d91"></fiddle-embed></div>

<a name="getTextScaleX"></a>
## getTextScaleX

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkScalar getTextScaleX() const
</pre>

Returns <a href="bmh_SkPaint_Reference?cl=9919#Text_Scale_X">Text Scale X</a>.
Default value is 1.

### Return Value

text horizontal scale

### Example

<div><fiddle-embed name="5dc8e58f6910cb8e4de9ed60f888188b"></fiddle-embed></div>

---

<a name="setTextScaleX"></a>
## setTextScaleX

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setTextScaleX(SkScalar scaleX)
</pre>

Sets <a href="bmh_SkPaint_Reference?cl=9919#Text_Scale_X">Text Scale X</a>.
Default value is 1.

### Parameters

<table>  <tr>    <td><code><strong>scaleX </strong></code></td> <td>
text horizontal scale</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="a75bbdb8bb866b125c4c1dd5e967d470"></fiddle-embed></div>

---

# <a name="Text_Skew_X"></a> Text Skew X
<a href="bmh_SkPaint_Reference?cl=9919#Text_Skew_X">Text Skew X</a> adjusts the text horizontal slant.
<a href="bmh_undocumented?cl=9919#Text">Text</a> skewing approximates italic and oblique type faces when the actual face
is not available.
<a href="bmh_SkPaint_Reference?cl=9919#Text_Skew_X">Text Skew X</a> can be set to any value.
<a href="bmh_SkPaint_Reference?cl=9919#Text_Skew_X">Text Skew X</a> defaults to 0.

### Example

<div><fiddle-embed name="aff208b0aab265f273045b27e683c17c"></fiddle-embed></div>

<a name="getTextSkewX"></a>
## getTextSkewX

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkScalar getTextSkewX() const
</pre>

Returns <a href="bmh_SkPaint_Reference?cl=9919#Text_Skew_X">Text Skew X</a>.
Default value is zero.

### Return Value

additional shear in x-axis relative to y-axis

### Example

<div><fiddle-embed name="11c10f466dae0d1639dbb9f6a0040218"></fiddle-embed></div>

---

<a name="setTextSkewX"></a>
## setTextSkewX

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setTextSkewX(SkScalar skewX)
</pre>

Sets <a href="bmh_SkPaint_Reference?cl=9919#Text_Skew_X">Text Skew X</a>.
Default value is zero.

### Parameters

<table>  <tr>    <td><code><strong>skewX </strong></code></td> <td>
additional shear in x-axis relative to y-axis</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="6bd705a6e0c5f8ee24f302fe531bfabc"></fiddle-embed></div>

---

# <a name="Text_Encoding"></a> Text Encoding

## <a name="SkPaint::TextEncoding"></a> Enum SkPaint::TextEncoding

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
enum <a href="bmh_SkPaint_Reference?cl=9919#TextEncoding">TextEncoding</a> {
<a href="bmh_SkPaint_Reference?cl=9919#kUTF8_TextEncoding">kUTF8 TextEncoding</a>,
<a href="bmh_SkPaint_Reference?cl=9919#kUTF16_TextEncoding">kUTF16 TextEncoding</a>,
<a href="bmh_SkPaint_Reference?cl=9919#kUTF32_TextEncoding">kUTF32 TextEncoding</a>,
<a href="bmh_SkPaint_Reference?cl=9919#kGlyphID_TextEncoding">kGlyphID TextEncoding</a>
};</pre>

<a href="bmh_SkPaint_Reference?cl=9919#TextEncoding">TextEncoding</a> determines whether text specifies character codes and their encoded size,
or glyph indices. Character codes use the encoding specified by the<a href="bmh_undocumented?cl=9919#Unicode">Unicode</a> standardhttp://unicode.org/standard/standard.html.
Character codes encoded size are specified by <a href="bmh_undocumented?cl=9919#UTF_8">UTF-8</a>, <a href="bmh_undocumented?cl=9919#UTF_16">UTF-16</a>, or <a href="bmh_undocumented?cl=9919#UTF_32">UTF-32</a>.
All character encoding are able to represent all of <a href="bmh_undocumented?cl=9919#Unicode">Unicode</a>, differing only
in the total storage required.

<a href="bmh_undocumented?cl=9919#UTF_8">UTF-8</a> (<a href="bmh_undocumented?cl=9919#RFC">RFC</a> 3629)https://tools.ietf.org/html/rfc3629is made up of 8-bit bytes, 
and is a superset of <a href="bmh_undocumented?cl=9919#ASCII">ASCII</a>.
<a href="bmh_undocumented?cl=9919#UTF_16">UTF-16</a> (<a href="bmh_undocumented?cl=9919#RFC">RFC</a> 2781)https://tools.ietf.org/html/rfc2781is made up of 16-bit words, 
and is a superset of <a href="bmh_undocumented?cl=9919#Unicode">Unicode</a> ranges 0x0000 to 0xD7FF and 0xE000 to 0xFFFF.
<a href="bmh_undocumented?cl=9919#UTF_32">UTF-32</a>http://www.unicode.org/versions/<a href="bmh_undocumented?cl=9919#Unicode5">Unicode5</a>.0.0/ch03.pdfis
made up of 32-bit words, and is a superset of <a href="bmh_undocumented?cl=9919#Unicode">Unicode</a>.

<a href="bmh_undocumented?cl=9919#Font_Manager">Font Manager</a> uses font data to convert character code points into glyph indices. 
A glyph index is a 16-bit word.

<a href="bmh_SkPaint_Reference?cl=9919#TextEncoding">TextEncoding</a> is set to <a href="bmh_SkPaint_Reference?cl=9919#kUTF8_TextEncoding">kUTF8 TextEncoding</a> by default.

### Constants

<table>
  <tr>
    <td><a name="SkPaint::kUTF8_TextEncoding"></a> <code><strong>SkPaint::kUTF8_TextEncoding </strong></code></td><td>0</td><td>Uses bytes to represent <a href="bmh_undocumented?cl=9919#UTF_8">UTF-8</a> or <a href="bmh_undocumented?cl=9919#ASCII">ASCII</a>.</td>
  </tr>
  <tr>
    <td><a name="SkPaint::kUTF16_TextEncoding"></a> <code><strong>SkPaint::kUTF16_TextEncoding </strong></code></td><td>1</td><td>Uses two byte words to represent most of <a href="bmh_undocumented?cl=9919#Unicode">Unicode</a>.</td>
  </tr>
  <tr>
    <td><a name="SkPaint::kUTF32_TextEncoding"></a> <code><strong>SkPaint::kUTF32_TextEncoding </strong></code></td><td>2</td><td>Uses four byte words to represent all of <a href="bmh_undocumented?cl=9919#Unicode">Unicode</a>.</td>
  </tr>
  <tr>
    <td><a name="SkPaint::kGlyphID_TextEncoding"></a> <code><strong>SkPaint::kGlyphID_TextEncoding </strong></code></td><td>3</td><td>Uses two byte words to represent glyph indices.</td>
  </tr>

</table>

### Example

<div><fiddle-embed name="b29294e7f29d160a1b46abf2dcec9d2a"><div>First line has <a href="bmh_undocumented?cl=9919#UTF_8">UTF-8</a> encoding.
Second line has <a href="bmh_undocumented?cl=9919#UTF_16">UTF-16</a> encoding.
Third line has <a href="bmh_undocumented?cl=9919#UTF_32">UTF-32</a> encoding.
Fourth line has 16 bit glyph indices.</div></fiddle-embed></div>

<a name="getTextEncoding"></a>
## getTextEncoding

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
TextEncoding getTextEncoding() const
</pre>

Returns <a href="bmh_SkPaint_Reference?cl=9919#Text_Encoding">Text Encoding</a>.
<a href="bmh_SkPaint_Reference?cl=9919#Text_Encoding">Text Encoding</a> determines how character code points are mapped to font glyph indices.

### Return Value

one of: <a href="bmh_SkPaint_Reference?cl=9919#kUTF8_TextEncoding">kUTF8 TextEncoding</a>, <a href="bmh_SkPaint_Reference?cl=9919#kUTF16_TextEncoding">kUTF16 TextEncoding</a>, <a href="bmh_SkPaint_Reference?cl=9919#kUTF32_TextEncoding">kUTF32 TextEncoding</a>, or 
<a href="bmh_SkPaint_Reference?cl=9919#kGlyphID_TextEncoding">kGlyphID TextEncoding</a>

### Example

<div><fiddle-embed name="70ad28bbf7668b38474d7f225e3540bc">

#### Example Output

~~~~
kUTF8_TextEncoding == text encoding
kGlyphID_TextEncoding == text encoding
~~~~

</fiddle-embed></div>

---

<a name="setTextEncoding"></a>
## setTextEncoding

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setTextEncoding(TextEncoding encoding)
</pre>

Sets <a href="bmh_SkPaint_Reference?cl=9919#Text_Encoding">Text Encoding</a> to <a href="bmh_SkPaint_Reference?cl=9919#setTextEncoding">encoding</a>. 
<a href="bmh_SkPaint_Reference?cl=9919#Text_Encoding">Text Encoding</a> determines how character code points are mapped to font glyph indices.
Invalid values for <a href="bmh_SkPaint_Reference?cl=9919#setTextEncoding">encoding</a> are ignored.

### Parameters

<table>  <tr>    <td><code><strong>encoding </strong></code></td> <td>
one of: <a href="bmh_SkPaint_Reference?cl=9919#kUTF8_TextEncoding">kUTF8 TextEncoding</a>, <a href="bmh_SkPaint_Reference?cl=9919#kUTF16_TextEncoding">kUTF16 TextEncoding</a>, <a href="bmh_SkPaint_Reference?cl=9919#kUTF32_TextEncoding">kUTF32 TextEncoding</a>, or</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="329b92fbc35151dee9aa0c0e70107665">

#### Example Output

~~~~
4 != text encoding
~~~~

</fiddle-embed></div>

---

# <a name="Font_Metrics"></a> Font Metrics
<a href="bmh_SkPaint_Reference?cl=9919#Font_Metrics">Font Metrics</a> describe dimensions common to the glyphs in <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a>.
The dimensions are computed by <a href="bmh_undocumented?cl=9919#Font_Manager">Font Manager</a> from font data and do not take 
<a href="bmh_SkPaint_Reference?cl=9919#Paint">Paint</a> settings other than <a href="bmh_SkPaint_Reference?cl=9919#Text_Size">Text Size</a> into account.

<a href="bmh_undocumented?cl=9919#Font">Font</a> dimensions specify the anchor to the left of the glyph at baseline as the origin.
X-axis values to the left of the glyph are negative, and to the right of the left glyph edge
are positive.
Y-axis values above the baseline are negative, and below the baseline are positive.

### Example

<div><fiddle-embed name="b5b76e0a15da0c3530071186a9006498"></fiddle-embed></div>

# <a name="SkPaint::FontMetrics"></a> Struct SkPaint::FontMetrics

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
struct <a href="bmh_SkPaint_Reference?cl=9919#FontMetrics">FontMetrics</a> {
enum <a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_FontMetricsFlags">FontMetricsFlags</a> {
<a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_kUnderlineThicknessIsValid_Flag">kUnderlineThicknessIsValid Flag</a> = 1 << 0,
<a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_kUnderlinePositionIsValid_Flag">kUnderlinePositionIsValid Flag</a> = 1 << 1,
<a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_kStrikeoutThicknessIsValid_Flag">kStrikeoutThicknessIsValid Flag</a> = 1 << 2,
<a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_kStrikeoutPositionIsValid_Flag">kStrikeoutPositionIsValid Flag</a> = 1 << 3,
};

uint32_t    <a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_fFlags">fFlags</a>;
<a href="bmh_undocumented?cl=9919#SkScalar">SkScalar</a>    <a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_fTop">fTop</a>;
<a href="bmh_undocumented?cl=9919#SkScalar">SkScalar</a>    <a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_fAscent">fAscent</a>;
<a href="bmh_undocumented?cl=9919#SkScalar">SkScalar</a>    <a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_fDescent">fDescent</a>;
<a href="bmh_undocumented?cl=9919#SkScalar">SkScalar</a>    <a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_fBottom">fBottom</a>;
<a href="bmh_undocumented?cl=9919#SkScalar">SkScalar</a>    <a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_fLeading">fLeading</a>;
<a href="bmh_undocumented?cl=9919#SkScalar">SkScalar</a>    <a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_fAvgCharWidth">fAvgCharWidth</a>;
<a href="bmh_undocumented?cl=9919#SkScalar">SkScalar</a>    <a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_fMaxCharWidth">fMaxCharWidth</a>;
<a href="bmh_undocumented?cl=9919#SkScalar">SkScalar</a>    <a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_fXMin">fXMin</a>;
<a href="bmh_undocumented?cl=9919#SkScalar">SkScalar</a>    <a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_fXMax">fXMax</a>;
<a href="bmh_undocumented?cl=9919#SkScalar">SkScalar</a>    <a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_fXHeight">fXHeight</a>;
<a href="bmh_undocumented?cl=9919#SkScalar">SkScalar</a>    <a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_fCapHeight">fCapHeight</a>;
<a href="bmh_undocumented?cl=9919#SkScalar">SkScalar</a>    <a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_fUnderlineThickness">fUnderlineThickness</a>;
<a href="bmh_undocumented?cl=9919#SkScalar">SkScalar</a>    <a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_fUnderlinePosition">fUnderlinePosition</a>;
<a href="bmh_undocumented?cl=9919#SkScalar">SkScalar</a>    <a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_fStrikeoutThickness">fStrikeoutThickness</a>;
<a href="bmh_undocumented?cl=9919#SkScalar">SkScalar</a>    <a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_fStrikeoutPosition">fStrikeoutPosition</a>;

bool <a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_hasUnderlineThickness">hasUnderlineThickness(SkScalar* thickness)</a> const;
bool <a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_hasUnderlinePosition">hasUnderlinePosition(SkScalar* position)</a> const;
bool <a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_hasStrikeoutThickness">hasStrikeoutThickness(SkScalar* thickness)</a> const;
bool <a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_hasStrikeoutPosition">hasStrikeoutPosition(SkScalar* position)</a> const;
};</pre>

<a href="bmh_SkPaint_Reference?cl=9919#FontMetrics">FontMetrics</a> is filled out by <a href="bmh_SkPaint_Reference?cl=9919#getFontMetrics">getFontMetrics</a>. <a href="bmh_SkPaint_Reference?cl=9919#FontMetrics">FontMetrics</a> contents reflect the values
computed by <a href="bmh_undocumented?cl=9919#Font_Manager">Font Manager</a> using <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a>. Values are set to zero if they are
not availble.

<a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_fUnderlineThickness">fUnderlineThickness</a> and <a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_fUnderlinePosition">fUnderlinePosition</a> have a bit set in <a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_fFlags">fFlags</a> if their values
are valid, since their value may be zero.
<a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_fStrikeoutThickness">fStrikeoutThickness</a> and <a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_fStrikeoutPosition">fStrikeoutPosition</a> have a bit set in <a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_fFlags">fFlags</a> if their values
are valid, since their value may be zero.

## <a name="SkPaint::FontMetrics::FontMetricsFlags"></a> Enum SkPaint::FontMetrics::FontMetricsFlags

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
enum <a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_FontMetricsFlags">FontMetricsFlags</a> {
<a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_kUnderlineThicknessIsValid_Flag">kUnderlineThicknessIsValid Flag</a> = 1 << 0,
<a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_kUnderlinePositionIsValid_Flag">kUnderlinePositionIsValid Flag</a> = 1 << 1,
<a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_kStrikeoutThicknessIsValid_Flag">kStrikeoutThicknessIsValid Flag</a> = 1 << 2,
<a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_kStrikeoutPositionIsValid_Flag">kStrikeoutPositionIsValid Flag</a> = 1 << 3,
};</pre>

<a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_FontMetricsFlags">FontMetricsFlags</a> are set in <a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_fFlags">fFlags</a> when underline and strikeout metrics are valid;
the underline or strikeout metric may be valid and zero.
Fonts with embedded bitmaps may not have valid underline or strikeout metrics.

### Constants

<table>
  <tr>
    <td><a name="SkPaint::FontMetrics::kUnderlineThicknessIsValid_Flag"></a> <code><strong>SkPaint::FontMetrics::kUnderlineThicknessIsValid_Flag </strong></code></td><td>0x0001</td><td>Set if <a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_fUnderlineThickness">fUnderlineThickness</a> is valid.</td>
  </tr>
  <tr>
    <td><a name="SkPaint::FontMetrics::kUnderlinePositionIsValid_Flag"></a> <code><strong>SkPaint::FontMetrics::kUnderlinePositionIsValid_Flag </strong></code></td><td>0x0002</td><td>Set if <a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_fUnderlinePosition">fUnderlinePosition</a> is valid.</td>
  </tr>
  <tr>
    <td><a name="SkPaint::FontMetrics::kStrikeoutThicknessIsValid_Flag"></a> <code><strong>SkPaint::FontMetrics::kStrikeoutThicknessIsValid_Flag </strong></code></td><td>0x0004</td><td>Set if <a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_fStrikeoutThickness">fStrikeoutThickness</a> is valid.</td>
  </tr>
  <tr>
    <td><a name="SkPaint::FontMetrics::kStrikeoutPositionIsValid_Flag"></a> <code><strong>SkPaint::FontMetrics::kStrikeoutPositionIsValid_Flag </strong></code></td><td>0x0008</td><td>Set if <a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_fStrikeoutPosition">fStrikeoutPosition</a> is valid.</td>
  </tr>

</table>

<code><strong>uint32_t    fFlags</strong></code>

<a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_fFlags">fFlags</a> is set when underline metrics are valid.

<code><strong>SkScalar    fTop</strong></code>

Largest height for any glyph.
A measure from the baseline, and is less than or equal to zero.

<code><strong>SkScalar    fAscent</strong></code>

Recommended distance above the baseline to reserve for a line of text.
A measure from the baseline, and is less than or equal to zero.

<code><strong>SkScalar    fDescent</strong></code>

Recommended distance below the baseline to reserve for a line of text.
A measure from the baseline, and is greater than or equal to zero.

<code><strong>SkScalar    fBottom</strong></code>

Greatest extent below the baseline for any glyph. 
A measure from the baseline, and is greater than or equal to zero.

<code><strong>SkScalar    fLeading</strong></code>

Recommended distance to add between lines of text.
Greater than or equal to zero.

<code><strong>SkScalar    fAvgCharWidth</strong></code>

Average character width, if it is available.
Zero if no average width is stored in the font.

<code><strong>SkScalar    fMaxCharWidth</strong></code>

Maximum character width.

<code><strong>SkScalar    fXMin</strong></code>

Minimum bounding box x value for all glyphs. 
Typically less than zero.

<code><strong>SkScalar    fXMax</strong></code>

Maximum bounding box x value for all glyphs.
Typically greater than zero.

<code><strong>SkScalar    fXHeight</strong></code>

Height of a lower-case 'x'.
May be zero if no lower-case height is stored in the font.

<code><strong>SkScalar    fCapHeight</strong></code>

Height of an upper-case letter.
May be zero if no upper-case height is stored in the font.

<code><strong>SkScalar    fUnderlineThickness</strong></code>

Underline thickness. If the metric
is valid, the <a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_kUnderlineThicknessIsValid_Flag">kUnderlineThicknessIsValid Flag</a> is set in <a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_fFlags">fFlags</a>.
If <a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_kUnderlineThicknessIsValid_Flag">kUnderlineThicknessIsValid Flag</a> is clear, <a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_fUnderlineThickness">fUnderlineThickness</a> is zero.

<code><strong>SkScalar    fUnderlinePosition</strong></code>

Underline position relative to the baseline.
It may be negative, to draw the underline above the baseline, zero
to draw the underline on the baseline, or positive to draw the underline
below the baseline. 

If the metric is valid, the <a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_kUnderlinePositionIsValid_Flag">kUnderlinePositionIsValid Flag</a> is set in <a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_fFlags">fFlags</a>.
If <a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_kUnderlinePositionIsValid_Flag">kUnderlinePositionIsValid Flag</a> is clear, <a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_fUnderlinePosition">fUnderlinePosition</a> is zero.

<code><strong>SkScalar    fStrikeoutThickness</strong></code>

Strikeout thickness. If the metric
is valid, the <a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_kStrikeoutThicknessIsValid_Flag">kStrikeoutThicknessIsValid Flag</a> is set in <a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_fFlags">fFlags</a>.
If <a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_kStrikeoutThicknessIsValid_Flag">kStrikeoutThicknessIsValid Flag</a> is clear, <a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_fStrikeoutThickness">fStrikeoutThickness</a> is zero.

<code><strong>SkScalar    fStrikeoutPosition</strong></code>

Strikeout position relative to the baseline.
It may be negative, to draw the strikeout above the baseline, zero
to draw the strikeout on the baseline, or positive to draw the strikeout
below the baseline. 

If the metric is valid, the <a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_kStrikeoutPositionIsValid_Flag">kStrikeoutPositionIsValid Flag</a> is set in <a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_fFlags">fFlags</a>.
If <a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_kStrikeoutPositionIsValid_Flag">kStrikeoutPositionIsValid Flag</a> is clear, <a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_fStrikeoutPosition">fStrikeoutPosition</a> is zero.

<a name="hasUnderlineThickness"></a>
## hasUnderlineThickness

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool hasUnderlineThickness(SkScalar* thickness) const
</pre>

If <a href="bmh_SkPaint_Reference?cl=9919#Font_Metrics">Font Metrics</a> has a valid underline <a href="bmh_SkPaint_Reference?cl=9919#thickness">thickness</a>, return true, and set 
<a href="bmh_SkPaint_Reference?cl=9919#thickness">thickness</a> to that value. If it doesn't, return false, and ignore
<a href="bmh_SkPaint_Reference?cl=9919#thickness">thickness</a>.

### Parameters

<table>  <tr>    <td><code><strong>thickness </strong></code></td> <td>
storage for underline width</td>
  </tr>
</table>

### Return Value

true if font specifies underline width

---

<a name="hasUnderlinePosition"></a>
## hasUnderlinePosition

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool hasUnderlinePosition(SkScalar* position) const
</pre>

If <a href="bmh_SkPaint_Reference?cl=9919#Font_Metrics">Font Metrics</a> has a valid underline <a href="bmh_SkPaint_Reference?cl=9919#position">position</a>, return true, and set 
<a href="bmh_SkPaint_Reference?cl=9919#position">position</a> to that value. If it doesn't, return false, and ignore
<a href="bmh_SkPaint_Reference?cl=9919#position">position</a>.

### Parameters

<table>  <tr>    <td><code><strong>position </strong></code></td> <td>
storage for underline <a href="bmh_SkPaint_Reference?cl=9919#position">position</a></td>
  </tr>
</table>

### Return Value

true if font specifies underline <a href="bmh_SkPaint_Reference?cl=9919#position">position</a>

---

<a name="hasStrikeoutThickness"></a>
## hasStrikeoutThickness

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool hasStrikeoutThickness(SkScalar* thickness) const
</pre>

If <a href="bmh_SkPaint_Reference?cl=9919#Font_Metrics">Font Metrics</a> has a valid strikeout <a href="bmh_SkPaint_Reference?cl=9919#thickness">thickness</a>, return true, and set 
<a href="bmh_SkPaint_Reference?cl=9919#thickness">thickness</a> to that value. If it doesn't, return false, and ignore
<a href="bmh_SkPaint_Reference?cl=9919#thickness">thickness</a>.

### Parameters

<table>  <tr>    <td><code><strong>thickness </strong></code></td> <td>
storage for strikeout width</td>
  </tr>
</table>

### Return Value

true if font specifies strikeout width

---

<a name="hasStrikeoutPosition"></a>
## hasStrikeoutPosition

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool hasStrikeoutPosition(SkScalar* position) const
</pre>

If <a href="bmh_SkPaint_Reference?cl=9919#Font_Metrics">Font Metrics</a> has a valid strikeout <a href="bmh_SkPaint_Reference?cl=9919#position">position</a>, return true, and set 
<a href="bmh_SkPaint_Reference?cl=9919#position">position</a> to that value. If it doesn't, return false, and ignore
<a href="bmh_SkPaint_Reference?cl=9919#position">position</a>.

### Parameters

<table>  <tr>    <td><code><strong>position </strong></code></td> <td>
storage for strikeout <a href="bmh_SkPaint_Reference?cl=9919#position">position</a></td>
  </tr>
</table>

### Return Value

true if font specifies strikeout <a href="bmh_SkPaint_Reference?cl=9919#position">position</a>

---

<a name="getFontMetrics"></a>
## getFontMetrics

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkScalar getFontMetrics(FontMetrics* metrics, SkScalar scale = 0) const
</pre>

Returns <a href="bmh_SkPaint_Reference?cl=9919#Font_Metrics">Font Metrics</a> associated with <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a>.
The return value is the recommended spacing between lines: the sum of <a href="bmh_SkPaint_Reference?cl=9919#metrics">metrics</a>
descent, ascent, and leading.
If <a href="bmh_SkPaint_Reference?cl=9919#metrics">metrics</a> is not nullptr, <a href="bmh_SkPaint_Reference?cl=9919#Font_Metrics">Font Metrics</a> is copied to <a href="bmh_SkPaint_Reference?cl=9919#metrics">metrics</a>.
Results are scaled by <a href="bmh_SkPaint_Reference?cl=9919#Text_Size">Text Size</a> but does not take into account
dimensions required by <a href="bmh_SkPaint_Reference?cl=9919#Text_Scale_X">Text Scale X</a>, <a href="bmh_SkPaint_Reference?cl=9919#Text_Skew_X">Text Skew X</a>, <a href="bmh_SkPaint_Reference?cl=9919#Fake_Bold">Fake Bold</a>,
<a href="bmh_SkPaint_Reference?cl=9919#Style_Stroke">Style Stroke</a>, and <a href="bmh_undocumented?cl=9919#Path_Effect">Path Effect</a>.
Results can be additionally scaled by <a href="bmh_SkPaint_Reference?cl=9919#scale">scale</a>; a <a href="bmh_SkPaint_Reference?cl=9919#scale">scale</a> of zero
is ignored.

### Parameters

<table>  <tr>    <td><code><strong>metrics </strong></code></td> <td>
storage for <a href="bmh_SkPaint_Reference?cl=9919#Font_Metrics">Font Metrics</a> from <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a>; may be nullptr</td>
  </tr>  <tr>    <td><code><strong>scale </strong></code></td> <td>
additional multiplier for returned values</td>
  </tr>
</table>

### Return Value

recommended spacing between lines

### Example

<div><fiddle-embed name="b899d84caba6607340322d317992d070"></fiddle-embed></div>

### See Also

<a href="bmh_SkPaint_Reference?cl=9919#Text_Size">Text Size</a> <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a> <a href="bmh_SkPaint_Reference?cl=9919#Typeface_Methods">Typeface Methods</a>

---

<a name="getFontSpacing"></a>
## getFontSpacing

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkScalar getFontSpacing() const
</pre>

Returns the recommended spacing between lines: the sum of metrics
descent, ascent, and leading.
Result is scaled by <a href="bmh_SkPaint_Reference?cl=9919#Text_Size">Text Size</a> but does not take into account
dimensions required by stroking and <a href="bmh_undocumented?cl=9919#Path_Effect">Path Effect</a>.
<a href="bmh_SkPaint_Reference?cl=9919#getFontSpacing">getFontSpacing</a> returns the same result as <a href="bmh_SkPaint_Reference?cl=9919#getFontMetrics">getFontMetrics</a>.

### Return Value

recommended spacing between lines

### Example

<div><fiddle-embed name="424741e26e1b174e43087d67422ce14f">

#### Example Output

~~~~
textSize: 12 fontSpacing: 13.9688
textSize: 18 fontSpacing: 20.9531
textSize: 24 fontSpacing: 27.9375
textSize: 32 fontSpacing: 37.25
~~~~

</fiddle-embed></div>

---

<a name="getFontBounds"></a>
## getFontBounds

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkRect getFontBounds() const
</pre>

Returns the union of bounds of all glyphs.
Returned dimensions are computed by <a href="bmh_undocumented?cl=9919#Font_Manager">Font Manager</a> from font data, 
ignoring <a href="bmh_SkPaint_Reference?cl=9919#Hinting">Hinting</a>. <a href="bmh_SkPaint_Reference?cl=9919#getFontBounds">getFontBounds</a> includes <a href="bmh_SkPaint_Reference?cl=9919#Text_Size">Text Size</a>, <a href="bmh_SkPaint_Reference?cl=9919#Text_Scale_X">Text Scale X</a>,
and <a href="bmh_SkPaint_Reference?cl=9919#Text_Skew_X">Text Skew X</a>, but not <a href="bmh_SkPaint_Reference?cl=9919#Fake_Bold">Fake Bold</a> or <a href="bmh_undocumented?cl=9919#Path_Effect">Path Effect</a>.

If <a href="bmh_SkPaint_Reference?cl=9919#Text_Size">Text Size</a> is large, <a href="bmh_SkPaint_Reference?cl=9919#Text_Scale_X">Text Scale X</a> is one, and <a href="bmh_SkPaint_Reference?cl=9919#Text_Skew_X">Text Skew X</a> is zero,
<a href="bmh_SkPaint_Reference?cl=9919#getFontBounds">getFontBounds</a> returns the same bounds as <a href="bmh_SkPaint_Reference?cl=9919#Font_Metrics">Font Metrics</a> { <a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_fXMin">FontMetrics::fXMin</a>, 
<a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_fTop">FontMetrics::fTop</a>, <a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_fXMax">FontMetrics::fXMax</a>, <a href="bmh_SkPaint_Reference?cl=9919#FontMetrics_fBottom">FontMetrics::fBottom</a> }.

### Return Value

union of bounds of all glyphs

### Example

<div><fiddle-embed name="facaddeec7943bc491988e345e27e65f">

#### Example Output

~~~~
metrics bounds = { -12.2461, -14.7891, 21.5215, 5.55469 }
font bounds    = { -12.2461, -14.7891, 21.5215, 5.55469 }
~~~~

</fiddle-embed></div>

---

<a name="textToGlyphs"></a>
## textToGlyphs

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
int textToGlyphs(const void* text, size_t byteLength, SkGlyphID glyphs[]) const
</pre>

Converts <a href="bmh_SkPaint_Reference?cl=9919#text">text</a> into glyph indices.
Returns the number of glyph indices represented by <a href="bmh_SkPaint_Reference?cl=9919#text">text</a>.
<a href="bmh_SkPaint_Reference?cl=9919#Text_Encoding">Text Encoding</a> specifies how <a href="bmh_SkPaint_Reference?cl=9919#text">text</a> represents characters or <a href="bmh_SkPaint_Reference?cl=9919#glyphs">glyphs</a>.
<a href="bmh_SkPaint_Reference?cl=9919#glyphs">glyphs</a> may be nullptr, to compute the glyph count.

Does not check <a href="bmh_SkPaint_Reference?cl=9919#text">text</a> for valid character encoding or valid
glyph indices.

If <a href="bmh_SkPaint_Reference?cl=9919#byteLength">byteLength</a> equals zero, <a href="bmh_SkPaint_Reference?cl=9919#textToGlyphs">textToGlyphs</a> returns zero.
If <a href="bmh_SkPaint_Reference?cl=9919#byteLength">byteLength</a> includes a partial character, the partial character is ignored.

If <a href="bmh_SkPaint_Reference?cl=9919#Text_Encoding">Text Encoding</a> is <a href="bmh_SkPaint_Reference?cl=9919#kUTF8_TextEncoding">kUTF8 TextEncoding</a> and
<a href="bmh_SkPaint_Reference?cl=9919#text">text</a> contains an invalid <a href="bmh_undocumented?cl=9919#UTF_8">UTF-8</a> sequence, zero is returned.

### Parameters

<table>  <tr>    <td><code><strong>text </strong></code></td> <td>
character stroage encoded with <a href="bmh_SkPaint_Reference?cl=9919#Text_Encoding">Text Encoding</a></td>
  </tr>  <tr>    <td><code><strong>byteLength </strong></code></td> <td>
length of character storage in bytes</td>
  </tr>  <tr>    <td><code><strong>glyphs </strong></code></td> <td>
storage for glyph indices; may be nullptr</td>
  </tr>
</table>

### Return Value

number of <a href="bmh_SkPaint_Reference?cl=9919#glyphs">glyphs</a> represented by <a href="bmh_SkPaint_Reference?cl=9919#text">text</a> of length <a href="bmh_SkPaint_Reference?cl=9919#byteLength">byteLength</a>

### Example

<div><fiddle-embed name="343e9471a7f7b5f09abdc3b44983433b"></fiddle-embed></div>

---

<a name="countText"></a>
## countText

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
int countText(const void* text, size_t byteLength) const
</pre>

Returns the number of glyphs in <a href="bmh_SkPaint_Reference?cl=9919#text">text</a>.
Uses <a href="bmh_SkPaint_Reference?cl=9919#Text_Encoding">Text Encoding</a> to count the glyphs.
Returns the same result as <a href="bmh_SkPaint_Reference?cl=9919#textToGlyphs">textToGlyphs</a>.

### Parameters

<table>  <tr>    <td><code><strong>text </strong></code></td> <td>
character stroage encoded with <a href="bmh_SkPaint_Reference?cl=9919#Text_Encoding">Text Encoding</a></td>
  </tr>  <tr>    <td><code><strong>byteLength </strong></code></td> <td>
length of character storage in bytes</td>
  </tr>
</table>

### Return Value

number of glyphs represented by <a href="bmh_SkPaint_Reference?cl=9919#text">text</a> of length <a href="bmh_SkPaint_Reference?cl=9919#byteLength">byteLength</a>

### Example

<div><fiddle-embed name="85436c71aab5410767fc688ab0573e09">

#### Example Output

~~~~
count = 5
~~~~

</fiddle-embed></div>

---

<a name="containsText"></a>
## containsText

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool containsText(const void* text, size_t byteLength) const
</pre>

Returns true if all <a href="bmh_SkPaint_Reference?cl=9919#text">text</a> corresponds to a non-zero glyph index. 
Returns false if any characters in <a href="bmh_SkPaint_Reference?cl=9919#text">text</a> are not supported in
<a href="bmh_undocumented?cl=9919#Typeface">Typeface</a>.

If <a href="bmh_SkPaint_Reference?cl=9919#Text_Encoding">Text Encoding</a> is <a href="bmh_SkPaint_Reference?cl=9919#kGlyphID_TextEncoding">kGlyphID TextEncoding</a>, <a href="bmh_SkPaint_Reference?cl=9919#containsText">containsText</a>
returns true if all glyph indices in <a href="bmh_SkPaint_Reference?cl=9919#text">text</a> are non-zero; <a href="bmh_SkPaint_Reference?cl=9919#containsText">containsText</a>
does not check to see if <a href="bmh_SkPaint_Reference?cl=9919#text">text</a> contains valid glyph indices for <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a>.

Returns true if bytelength is zero.

### Parameters

<table>  <tr>    <td><code><strong>text </strong></code></td> <td>
array of characters or glyphs</td>
  </tr>  <tr>    <td><code><strong>byteLength </strong></code></td> <td>
number of bytes in <a href="bmh_SkPaint_Reference?cl=9919#text">text</a> array</td>
  </tr>
</table>

### Return Value

true if all <a href="bmh_SkPaint_Reference?cl=9919#text">text</a> corresponds to a non-zero glyph index

### Example

<div><fiddle-embed name="9202369019552f09cd4bec7f3046fee4"><div><a href="bmh_SkPaint_Reference?cl=9919#containsText">containsText</a> succeeds for degree symbol, but cannot find a glyph index
corresponding to the <a href="bmh_undocumented?cl=9919#Unicode">Unicode</a> surrogate code point.</div>

#### Example Output

~~~~
0x00b0 == has char
0xd800 != has char
~~~~

</fiddle-embed></div>

### Example

<div><fiddle-embed name="904227febfd1c2e264955da0ef66da73"><div><a href="bmh_SkPaint_Reference?cl=9919#containsText">containsText</a> returns true that glyph index is greater than zero, not
that it corresponds to an entry in <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a>.</div>

#### Example Output

~~~~
0x01ff == has glyph
0x0000 != has glyph
0xffff == has glyph
~~~~

</fiddle-embed></div>

### See Also

<a href="bmh_SkPaint_Reference?cl=9919#setTextEncoding">setTextEncoding</a> <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a>

---

<a name="glyphsToUnichars"></a>
## glyphsToUnichars

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void glyphsToUnichars(const SkGlyphID glyphs[], int count, SkUnichar text[]) const
</pre>

Converts <a href="bmh_SkPaint_Reference?cl=9919#glyphs">glyphs</a> into <a href="bmh_SkPaint_Reference?cl=9919#text">text</a> if possible. 
<a href="bmh_undocumented?cl=9919#Glyph">Glyph</a> values without direct <a href="bmh_undocumented?cl=9919#Unicode">Unicode</a> equivalents are mapped to zero. 
Uses the <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a>, but is unaffected
by <a href="bmh_SkPaint_Reference?cl=9919#Text_Encoding">Text Encoding</a>; the <a href="bmh_SkPaint_Reference?cl=9919#text">text</a> values returned are equivalent to <a href="bmh_SkPaint_Reference?cl=9919#kUTF32_TextEncoding">kUTF32 TextEncoding</a>.

Only supported on platforms that use <a href="bmh_undocumented?cl=9919#FreeType">FreeType</a> as the <a href="bmh_undocumented?cl=9919#Engine">Font Engine</a>.

### Parameters

<table>  <tr>    <td><code><strong>glyphs </strong></code></td> <td>
array of indices into font</td>
  </tr>  <tr>    <td><code><strong>count </strong></code></td> <td>
length of glyph array</td>
  </tr>  <tr>    <td><code><strong>text </strong></code></td> <td>
storage for character codes, one per glyph</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="c12686b0b3e0a87d0a248bbfc57e9492"><div>Convert <a href="bmh_undocumented?cl=9919#UTF_8">UTF-8</a> <a href="bmh_SkPaint_Reference?cl=9919#text">text</a> to <a href="bmh_SkPaint_Reference?cl=9919#glyphs">glyphs</a>; then convert <a href="bmh_SkPaint_Reference?cl=9919#glyphs">glyphs</a> to <a href="bmh_undocumented?cl=9919#Unichar">Unichar</a> code points.</div></fiddle-embed></div>

---

# <a name="Measure_Text"></a> Measure Text

<a name="measureText"></a>
## measureText

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkScalar measureText(const void* text, size_t length, SkRect* bounds) const
</pre>

Returns the advance width of <a href="bmh_SkPaint_Reference?cl=9919#text">text</a> if <a href="bmh_SkPaint_Reference?cl=9919#kVerticalText_Flag">kVerticalText Flag</a> is clear,
and the height of <a href="bmh_SkPaint_Reference?cl=9919#text">text</a> if <a href="bmh_SkPaint_Reference?cl=9919#kVerticalText_Flag">kVerticalText Flag</a> is set.
The advance is the normal distance to move before drawing additional <a href="bmh_SkPaint_Reference?cl=9919#text">text</a>.
Uses <a href="bmh_SkPaint_Reference?cl=9919#Text_Encoding">Text Encoding</a> to decode <a href="bmh_SkPaint_Reference?cl=9919#text">text</a>, <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a> to get the font metrics,
and <a href="bmh_SkPaint_Reference?cl=9919#Text_Size">Text Size</a>, <a href="bmh_SkPaint_Reference?cl=9919#Text_Scale_X">Text Scale X</a>, <a href="bmh_SkPaint_Reference?cl=9919#Text_Skew_X">Text Skew X</a>, <a href="bmh_SkPaint_Reference?cl=9919#Stroke_Width">Stroke Width</a>, and
<a href="bmh_undocumented?cl=9919#Path_Effect">Path Effect</a> to scale the metrics and <a href="bmh_SkPaint_Reference?cl=9919#bounds">bounds</a>.
Returns the bounding box of <a href="bmh_SkPaint_Reference?cl=9919#text">text</a> if <a href="bmh_SkPaint_Reference?cl=9919#bounds">bounds</a> is not nullptr.
The bounding box is computed as if the <a href="bmh_SkPaint_Reference?cl=9919#text">text</a> was drawn at the origin.

### Parameters

<table>  <tr>    <td><code><strong>text </strong></code></td> <td>
character codes or glyph indices to be measured</td>
  </tr>  <tr>    <td><code><strong>length </strong></code></td> <td>
number of bytes of <a href="bmh_SkPaint_Reference?cl=9919#text">text</a> to measure</td>
  </tr>  <tr>    <td><code><strong>bounds </strong></code></td> <td>
returns bounding box relative to (0, 0) if not nullptr</td>
  </tr>
</table>

### Return Value

advance width or height

### Example

<div><fiddle-embed name="06084f609184470135a9cd9ebc5af149"></fiddle-embed></div>

---

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkScalar measureText(const void* text, size_t length) const
</pre>

Returns the advance width of <a href="bmh_SkPaint_Reference?cl=9919#text">text</a> if <a href="bmh_SkPaint_Reference?cl=9919#kVerticalText_Flag">kVerticalText Flag</a> is clear,
and the height of <a href="bmh_SkPaint_Reference?cl=9919#text">text</a> if <a href="bmh_SkPaint_Reference?cl=9919#kVerticalText_Flag">kVerticalText Flag</a> is set.
The advance is the normal distance to move before drawing additional <a href="bmh_SkPaint_Reference?cl=9919#text">text</a>.
Uses <a href="bmh_SkPaint_Reference?cl=9919#Text_Encoding">Text Encoding</a> to decode <a href="bmh_SkPaint_Reference?cl=9919#text">text</a>, <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a> to get the font metrics,
and <a href="bmh_SkPaint_Reference?cl=9919#Text_Size">Text Size</a> to scale the metrics.
Does not scale the advance or bounds by <a href="bmh_SkPaint_Reference?cl=9919#Fake_Bold">Fake Bold</a> or <a href="bmh_undocumented?cl=9919#Path_Effect">Path Effect</a>.

### Parameters

<table>  <tr>    <td><code><strong>text </strong></code></td> <td>
character codes or glyph indices to be measured</td>
  </tr>  <tr>    <td><code><strong>length </strong></code></td> <td>
number of bytes of <a href="bmh_SkPaint_Reference?cl=9919#text">text</a> to measure</td>
  </tr>
</table>

### Return Value

advance width or height

### Example

<div><fiddle-embed name="f1139a5ddd17fd47c2f45f6e642cac76">

#### Example Output

~~~~
default width = 5
double width = 10
~~~~

</fiddle-embed></div>

---

<a name="breakText"></a>
## breakText

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
size_t breakText(const void* text, size_t length, SkScalar maxWidth,
                 SkScalar* measuredWidth = NULL) const
</pre>

Returns the bytes of <a href="bmh_SkPaint_Reference?cl=9919#text">text</a> that fit within <a href="bmh_SkPaint_Reference?cl=9919#maxWidth">maxWidth</a>.
If <a href="bmh_SkPaint_Reference?cl=9919#kVerticalText_Flag">kVerticalText Flag</a> is clear, the <a href="bmh_SkPaint_Reference?cl=9919#text">text</a> fragment fits if its advance width is less than or
equal to <a href="bmh_SkPaint_Reference?cl=9919#maxWidth">maxWidth</a>.
If <a href="bmh_SkPaint_Reference?cl=9919#kVerticalText_Flag">kVerticalText Flag</a> is set, the <a href="bmh_SkPaint_Reference?cl=9919#text">text</a> fragment fits if its advance height is less than or
equal to <a href="bmh_SkPaint_Reference?cl=9919#maxWidth">maxWidth</a>.
Measures only while the advance is less than or equal to <a href="bmh_SkPaint_Reference?cl=9919#maxWidth">maxWidth</a>.
Returns the advance or the <a href="bmh_SkPaint_Reference?cl=9919#text">text</a> fragment in <a href="bmh_SkPaint_Reference?cl=9919#measuredWidth">measuredWidth</a> if it not nullptr.
Uses <a href="bmh_SkPaint_Reference?cl=9919#Text_Encoding">Text Encoding</a> to decode <a href="bmh_SkPaint_Reference?cl=9919#text">text</a>, <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a> to get the font metrics,
and <a href="bmh_SkPaint_Reference?cl=9919#Text_Size">Text Size</a> to scale the metrics.
Does not scale the advance or bounds by <a href="bmh_SkPaint_Reference?cl=9919#Fake_Bold">Fake Bold</a> or <a href="bmh_undocumented?cl=9919#Path_Effect">Path Effect</a>.

### Parameters

<table>  <tr>    <td><code><strong>text </strong></code></td> <td>
character codes or glyph indices to be measured</td>
  </tr>  <tr>    <td><code><strong>length </strong></code></td> <td>
number of bytes of <a href="bmh_SkPaint_Reference?cl=9919#text">text</a> to measure</td>
  </tr>  <tr>    <td><code><strong>maxWidth </strong></code></td> <td>
advance limit; <a href="bmh_SkPaint_Reference?cl=9919#text">text</a> is measured while advance is less than <a href="bmh_SkPaint_Reference?cl=9919#maxWidth">maxWidth</a></td>
  </tr>  <tr>    <td><code><strong>measuredWidth </strong></code></td> <td>
returns the width of the <a href="bmh_SkPaint_Reference?cl=9919#text">text</a> less than or equal to <a href="bmh_SkPaint_Reference?cl=9919#maxWidth">maxWidth</a></td>
  </tr>
</table>

### Return Value

bytes of <a href="bmh_SkPaint_Reference?cl=9919#text">text</a> that fit, always less than or equal to <a href="bmh_SkPaint_Reference?cl=9919#length">length</a>

### Example

<div><fiddle-embed name="fd0033470ccbd5c7059670fdbf96cffc"><div><a href="bmh_undocumented?cl=9919#Line">Line</a> under "" shows desired width, shorter than available characters.
<a href="bmh_undocumented?cl=9919#Line">Line</a> under "" shows measured width after breaking <a href="bmh_SkPaint_Reference?cl=9919#text">text</a>.</div></fiddle-embed></div>

---

<a name="getTextWidths"></a>
## getTextWidths

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
int getTextWidths(const void* text, size_t byteLength, SkScalar widths[],
                  SkRect bounds[] = NULL) const
</pre>

Retrieves the advance and <a href="bmh_SkPaint_Reference?cl=9919#bounds">bounds</a> for each glyph in <a href="bmh_SkPaint_Reference?cl=9919#text">text</a>, and returns
the glyph count in <a href="bmh_SkPaint_Reference?cl=9919#text">text</a>.
Both <a href="bmh_SkPaint_Reference?cl=9919#widths">widths</a> and <a href="bmh_SkPaint_Reference?cl=9919#bounds">bounds</a> may be nullptr.
If <a href="bmh_SkPaint_Reference?cl=9919#widths">widths</a> is not nullptr, <a href="bmh_SkPaint_Reference?cl=9919#widths">widths</a> must be an array of glyph count entries.
if <a href="bmh_SkPaint_Reference?cl=9919#bounds">bounds</a> is not nullptr, <a href="bmh_SkPaint_Reference?cl=9919#bounds">bounds</a> must be an array of glyph count entries. 
If <a href="bmh_SkPaint_Reference?cl=9919#kVerticalText_Flag">kVerticalText Flag</a> is clear, <a href="bmh_SkPaint_Reference?cl=9919#widths">widths</a> returns the horizontal advance.
If <a href="bmh_SkPaint_Reference?cl=9919#kVerticalText_Flag">kVerticalText Flag</a> is set, <a href="bmh_SkPaint_Reference?cl=9919#widths">widths</a> returns the vertical advance.
Uses <a href="bmh_SkPaint_Reference?cl=9919#Text_Encoding">Text Encoding</a> to decode <a href="bmh_SkPaint_Reference?cl=9919#text">text</a>, <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a> to get the font metrics,
and <a href="bmh_SkPaint_Reference?cl=9919#Text_Size">Text Size</a> to scale the <a href="bmh_SkPaint_Reference?cl=9919#widths">widths</a> and <a href="bmh_SkPaint_Reference?cl=9919#bounds">bounds</a>.
Does not scale the advance by <a href="bmh_SkPaint_Reference?cl=9919#Fake_Bold">Fake Bold</a> or <a href="bmh_undocumented?cl=9919#Path_Effect">Path Effect</a>.
Does include <a href="bmh_SkPaint_Reference?cl=9919#Fake_Bold">Fake Bold</a> and <a href="bmh_undocumented?cl=9919#Path_Effect">Path Effect</a> in the <a href="bmh_SkPaint_Reference?cl=9919#bounds">bounds</a>.

### Parameters

<table>  <tr>    <td><code><strong>text </strong></code></td> <td>
character codes or glyph indices to be measured</td>
  </tr>  <tr>    <td><code><strong>byteLength </strong></code></td> <td>
number of bytes of <a href="bmh_SkPaint_Reference?cl=9919#text">text</a> to measure</td>
  </tr>  <tr>    <td><code><strong>widths </strong></code></td> <td>
returns <a href="bmh_SkPaint_Reference?cl=9919#text">text</a> advances for each glyph; may be nullptr</td>
  </tr>  <tr>    <td><code><strong>bounds </strong></code></td> <td>
returns <a href="bmh_SkPaint_Reference?cl=9919#bounds">bounds</a> for each glyph relative to (0, 0); may be nullptr</td>
  </tr>
</table>

### Return Value

glyph count in <a href="bmh_SkPaint_Reference?cl=9919#text">text</a>

### Example

<div><fiddle-embed name="6b9e101f49e9c2c28755c5bdcef64dfb"><div>Bounds of glyphs increase for stroked <a href="bmh_SkPaint_Reference?cl=9919#text">text</a>, but <a href="bmh_SkPaint_Reference?cl=9919#text">text</a> advance remains the same.
The underlines show the <a href="bmh_SkPaint_Reference?cl=9919#text">text</a> advance, spaced to keep them distinct.</div></fiddle-embed></div>

---

# <a name="Text_Path"></a> Text Path
<a href="bmh_SkPaint_Reference?cl=9919#Text_Path">Text Path</a> describes the geometry of glyphs used to draw text.

<a name="getTextPath"></a>
## getTextPath

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void getTextPath(const void* text, size_t length, SkScalar x, SkScalar y,
                 SkPath* path) const
</pre>

Returns the geometry as <a href="bmh_SkPath_Reference?cl=9919#Path">Path</a> equivalent to the drawn <a href="bmh_SkPaint_Reference?cl=9919#text">text</a>.
Uses <a href="bmh_SkPaint_Reference?cl=9919#Text_Encoding">Text Encoding</a> to decode <a href="bmh_SkPaint_Reference?cl=9919#text">text</a>, <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a> to get the glyph paths,
and <a href="bmh_SkPaint_Reference?cl=9919#Text_Size">Text Size</a>, <a href="bmh_SkPaint_Reference?cl=9919#Fake_Bold">Fake Bold</a>, and <a href="bmh_undocumented?cl=9919#Path_Effect">Path Effect</a> to scale and modify the glyph paths.
All of the glyph paths are stored in <a href="bmh_SkPaint_Reference?cl=9919#path">path</a>.
<a href="bmh_SkPaint_Reference?cl=9919#getTextPath">getTextPath</a> uses <a href="bmh_SkPaint_Reference?cl=9919#x">x</a>, <a href="bmh_SkPaint_Reference?cl=9919#y">y</a>, and <a href="bmh_SkPaint_Reference?cl=9919#Text_Align">Text Align</a> to position <a href="bmh_SkPaint_Reference?cl=9919#path">path</a>.

### Parameters

<table>  <tr>    <td><code><strong>text </strong></code></td> <td>
character codes or glyph indices</td>
  </tr>  <tr>    <td><code><strong>length </strong></code></td> <td>
number of bytes of <a href="bmh_SkPaint_Reference?cl=9919#text">text</a></td>
  </tr>  <tr>    <td><code><strong>x </strong></code></td> <td>
x-coordinate of the origin of the <a href="bmh_SkPaint_Reference?cl=9919#text">text</a></td>
  </tr>  <tr>    <td><code><strong>y </strong></code></td> <td>
y-coordinate of the origin of the <a href="bmh_SkPaint_Reference?cl=9919#text">text</a></td>
  </tr>  <tr>    <td><code><strong>path </strong></code></td> <td>
geometry of the glyphs</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="7c9e6a399f898d68026c1f0865e6f73e"><div><a href="bmh_undocumented?cl=9919#Text">Text</a> is added to <a href="bmh_SkPath_Reference?cl=9919#Path">Path</a>, offset, and subtracted from <a href="bmh_SkPath_Reference?cl=9919#Path">Path</a>, then added at
the offset location. The result is rendered with one draw call.</div></fiddle-embed></div>

---

<a name="getPosTextPath"></a>
## getPosTextPath

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void getPosTextPath(const void* text, size_t length, const SkPoint pos[],
                    SkPath* path) const
</pre>

Returns the geometry as <a href="bmh_SkPath_Reference?cl=9919#Path">Path</a> equivalent to the drawn <a href="bmh_SkPaint_Reference?cl=9919#text">text</a>.
Uses <a href="bmh_SkPaint_Reference?cl=9919#Text_Encoding">Text Encoding</a> to decode <a href="bmh_SkPaint_Reference?cl=9919#text">text</a>, <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a> to get the glyph paths,
and <a href="bmh_SkPaint_Reference?cl=9919#Text_Size">Text Size</a>, <a href="bmh_SkPaint_Reference?cl=9919#Fake_Bold">Fake Bold</a>, and <a href="bmh_undocumented?cl=9919#Path_Effect">Path Effect</a> to scale and modify the glyph paths.
All of the glyph paths are stored in <a href="bmh_SkPaint_Reference?cl=9919#path">path</a>.
Uses <a href="bmh_SkPaint_Reference?cl=9919#pos">pos</a> array and <a href="bmh_SkPaint_Reference?cl=9919#Text_Align">Text Align</a> to position <a href="bmh_SkPaint_Reference?cl=9919#path">path</a>.
<a href="bmh_SkPaint_Reference?cl=9919#pos">pos</a> contains a position for each glyph.

### Parameters

<table>  <tr>    <td><code><strong>text </strong></code></td> <td>
character codes or glyph indices</td>
  </tr>  <tr>    <td><code><strong>length </strong></code></td> <td>
number of bytes of <a href="bmh_SkPaint_Reference?cl=9919#text">text</a></td>
  </tr>  <tr>    <td><code><strong>pos </strong></code></td> <td>
positions of each glyph</td>
  </tr>  <tr>    <td><code><strong>path </strong></code></td> <td>
geometry of the glyphs</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="7f27c93472aa99a7542fb3493076f072"><div>Simplifies three glyphs to eliminate overlaps, and strokes the result.</div></fiddle-embed></div>

---

# <a name="Text_Intercepts"></a> Text Intercepts
<a href="bmh_SkPaint_Reference?cl=9919#Text_Intercepts">Text Intercepts</a> describe the intersection of drawn text glyphs with a pair
of lines parallel to the text advance. <a href="bmh_SkPaint_Reference?cl=9919#Text_Intercepts">Text Intercepts</a> permits creating a
underline that skips descenders.

<a name="getTextIntercepts"></a>
## getTextIntercepts

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
int getTextIntercepts(const void* text, size_t length, SkScalar x, SkScalar y,
                      const SkScalar bounds[2], SkScalar* intervals) const
</pre>

Returns the number of <a href="bmh_SkPaint_Reference?cl=9919#intervals">intervals</a> that intersect <a href="bmh_SkPaint_Reference?cl=9919#bounds">bounds</a>.
<a href="bmh_SkPaint_Reference?cl=9919#bounds">bounds</a> describes a pair of lines parallel to the <a href="bmh_SkPaint_Reference?cl=9919#text">text</a> advance.
The return count is zero or a multiple of two, and is at most twice the number of glyphs in
the string. 
Uses <a href="bmh_SkPaint_Reference?cl=9919#Text_Encoding">Text Encoding</a> to decode <a href="bmh_SkPaint_Reference?cl=9919#text">text</a>, <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a> to get the glyph paths,
and <a href="bmh_SkPaint_Reference?cl=9919#Text_Size">Text Size</a>, <a href="bmh_SkPaint_Reference?cl=9919#Fake_Bold">Fake Bold</a>, and <a href="bmh_undocumented?cl=9919#Path_Effect">Path Effect</a> to scale and modify the glyph paths.
Uses <a href="bmh_SkPaint_Reference?cl=9919#x">x</a>, <a href="bmh_SkPaint_Reference?cl=9919#y">y</a>, and <a href="bmh_SkPaint_Reference?cl=9919#Text_Align">Text Align</a> to position <a href="bmh_SkPaint_Reference?cl=9919#intervals">intervals</a>.
Pass nullptr for <a href="bmh_SkPaint_Reference?cl=9919#intervals">intervals</a> to determine the size of the interval array.
<a href="bmh_SkPaint_Reference?cl=9919#intervals">intervals</a> are cached to improve performance for multiple calls.

### Parameters

<table>  <tr>    <td><code><strong>text </strong></code></td> <td>
character codes or glyph indices</td>
  </tr>  <tr>    <td><code><strong>length </strong></code></td> <td>
number of bytes of <a href="bmh_SkPaint_Reference?cl=9919#text">text</a></td>
  </tr>  <tr>    <td><code><strong>x </strong></code></td> <td>
x-coordinate of the origin of the <a href="bmh_SkPaint_Reference?cl=9919#text">text</a></td>
  </tr>  <tr>    <td><code><strong>y </strong></code></td> <td>
y-coordinate of the origin of the <a href="bmh_SkPaint_Reference?cl=9919#text">text</a></td>
  </tr>  <tr>    <td><code><strong>bounds </strong></code></td> <td>
lower and upper line parallel to the advance</td>
  </tr>  <tr>    <td><code><strong>intervals </strong></code></td> <td>
returned intersections; may be nullptr</td>
  </tr>
</table>

### Return Value

number of intersections; may be zero

### Example

<div><fiddle-embed name="2a0b80ed20d193c688085b79deb5bdc9"><div>Underline uses intercepts to draw on either side of the glyph descender.</div></fiddle-embed></div>

---

<a name="getPosTextIntercepts"></a>
## getPosTextIntercepts

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
int getPosTextIntercepts(const void* text, size_t length, const SkPoint pos[],
                         const SkScalar bounds[2], SkScalar* intervals) const
</pre>

Returns the number of <a href="bmh_SkPaint_Reference?cl=9919#intervals">intervals</a> that intersect <a href="bmh_SkPaint_Reference?cl=9919#bounds">bounds</a>.
<a href="bmh_SkPaint_Reference?cl=9919#bounds">bounds</a> describes a pair of lines parallel to the <a href="bmh_SkPaint_Reference?cl=9919#text">text</a> advance.
The return count is zero or a multiple of two, and is at most twice the number of glyphs in
the string. 
Uses <a href="bmh_SkPaint_Reference?cl=9919#Text_Encoding">Text Encoding</a> to decode <a href="bmh_SkPaint_Reference?cl=9919#text">text</a>, <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a> to get the glyph paths,
and <a href="bmh_SkPaint_Reference?cl=9919#Text_Size">Text Size</a>, <a href="bmh_SkPaint_Reference?cl=9919#Fake_Bold">Fake Bold</a>, and <a href="bmh_undocumented?cl=9919#Path_Effect">Path Effect</a> to scale and modify the glyph paths.
Uses <a href="bmh_SkPaint_Reference?cl=9919#pos">pos</a> array and <a href="bmh_SkPaint_Reference?cl=9919#Text_Align">Text Align</a> to position <a href="bmh_SkPaint_Reference?cl=9919#intervals">intervals</a>.
Pass nullptr for <a href="bmh_SkPaint_Reference?cl=9919#intervals">intervals</a> to determine the size of the interval array.
<a href="bmh_SkPaint_Reference?cl=9919#intervals">intervals</a> are cached to improve performance for multiple calls.

### Parameters

<table>  <tr>    <td><code><strong>text </strong></code></td> <td>
character codes or glyph indices</td>
  </tr>  <tr>    <td><code><strong>length </strong></code></td> <td>
number of bytes of <a href="bmh_SkPaint_Reference?cl=9919#text">text</a></td>
  </tr>  <tr>    <td><code><strong>pos </strong></code></td> <td>
positions of each glyph</td>
  </tr>  <tr>    <td><code><strong>bounds </strong></code></td> <td>
lower and upper line parallel to the advance</td>
  </tr>  <tr>    <td><code><strong>intervals </strong></code></td> <td>
returned intersections; may be nullptr</td>
  </tr>
</table>

### Return Value

The number of intersections; may be zero

### Example

<div><fiddle-embed name="98b2dfc552d0540a7c041fe7a2839bd7"><div><a href="bmh_undocumented?cl=9919#Text">Text</a> intercepts draw on either side of, but not inside, glyphs in a run.</div></fiddle-embed></div>

---

<a name="getPosTextHIntercepts"></a>
## getPosTextHIntercepts

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
int getPosTextHIntercepts(const void* text, size_t length, const SkScalar xpos[],
                          SkScalar constY, const SkScalar bounds[2],
                          SkScalar* intervals) const
</pre>

Returns the number of <a href="bmh_SkPaint_Reference?cl=9919#intervals">intervals</a> that intersect <a href="bmh_SkPaint_Reference?cl=9919#bounds">bounds</a>.
<a href="bmh_SkPaint_Reference?cl=9919#bounds">bounds</a> describes a pair of lines parallel to the <a href="bmh_SkPaint_Reference?cl=9919#text">text</a> advance.
The return count is zero or a multiple of two, and is at most twice the number of glyphs in
the string. 
Uses <a href="bmh_SkPaint_Reference?cl=9919#Text_Encoding">Text Encoding</a> to decode <a href="bmh_SkPaint_Reference?cl=9919#text">text</a>, <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a> to get the glyph paths,
and <a href="bmh_SkPaint_Reference?cl=9919#Text_Size">Text Size</a>, <a href="bmh_SkPaint_Reference?cl=9919#Fake_Bold">Fake Bold</a>, and <a href="bmh_undocumented?cl=9919#Path_Effect">Path Effect</a> to scale and modify the glyph paths.
Uses <a href="bmh_SkPaint_Reference?cl=9919#xpos">xpos</a> array, <a href="bmh_SkPaint_Reference?cl=9919#constY">constY</a>, and <a href="bmh_SkPaint_Reference?cl=9919#Text_Align">Text Align</a> to position <a href="bmh_SkPaint_Reference?cl=9919#intervals">intervals</a>.
Pass nullptr for <a href="bmh_SkPaint_Reference?cl=9919#intervals">intervals</a> to determine the size of the interval array.
<a href="bmh_SkPaint_Reference?cl=9919#intervals">intervals</a> are cached to improve performance for multiple calls.

### Parameters

<table>  <tr>    <td><code><strong>text </strong></code></td> <td>
character codes or glyph indices</td>
  </tr>  <tr>    <td><code><strong>length </strong></code></td> <td>
number of bytes of <a href="bmh_SkPaint_Reference?cl=9919#text">text</a></td>
  </tr>  <tr>    <td><code><strong>xpos </strong></code></td> <td>
positions of each glyph in x</td>
  </tr>  <tr>    <td><code><strong>constY </strong></code></td> <td>
position of each glyph in y</td>
  </tr>  <tr>    <td><code><strong>bounds </strong></code></td> <td>
lower and upper line parallel to the advance</td>
  </tr>  <tr>    <td><code><strong>intervals </strong></code></td> <td>
returned intersections; may be nullptr</td>
  </tr>
</table>

### Return Value

number of intersections; may be zero

### Example

<div><fiddle-embed name="dc9851c43acc3716aca8c9a4d40d452d"><div><a href="bmh_undocumented?cl=9919#Text">Text</a> intercepts do not take stroke thickness into consideration.</div></fiddle-embed></div>

---

<a name="getTextBlobIntercepts"></a>
## getTextBlobIntercepts

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
int getTextBlobIntercepts(const SkTextBlob* blob, const SkScalar bounds[2],
                          SkScalar* intervals) const
</pre>

Returns the number of <a href="bmh_SkPaint_Reference?cl=9919#intervals">intervals</a> that intersect <a href="bmh_SkPaint_Reference?cl=9919#bounds">bounds</a>.
<a href="bmh_SkPaint_Reference?cl=9919#bounds">bounds</a> describes a pair of lines parallel to the text advance.
The return count is zero or a multiple of two, and is at most twice the number of glyphs in
the string. 
Uses <a href="bmh_SkPaint_Reference?cl=9919#Text_Encoding">Text Encoding</a> to decode text, <a href="bmh_undocumented?cl=9919#Typeface">Typeface</a> to get the glyph paths,
and <a href="bmh_SkPaint_Reference?cl=9919#Text_Size">Text Size</a>, <a href="bmh_SkPaint_Reference?cl=9919#Fake_Bold">Fake Bold</a>, and <a href="bmh_undocumented?cl=9919#Path_Effect">Path Effect</a> to scale and modify the glyph paths.
Uses pos array and <a href="bmh_SkPaint_Reference?cl=9919#Text_Align">Text Align</a> to position <a href="bmh_SkPaint_Reference?cl=9919#intervals">intervals</a>.
Pass nullptr for <a href="bmh_SkPaint_Reference?cl=9919#intervals">intervals</a> to determine the size of the interval array.
<a href="bmh_SkPaint_Reference?cl=9919#intervals">intervals</a> are cached to improve performance for multiple calls.

### Parameters

<table>  <tr>    <td><code><strong>blob </strong></code></td> <td>
glyphs, positions, and text paint attributes</td>
  </tr>  <tr>    <td><code><strong>bounds </strong></code></td> <td>
lower and upper line parallel to the advance</td>
  </tr>  <tr>    <td><code><strong>intervals </strong></code></td> <td>
returned intersections; may be nullptr</td>
  </tr>
</table>

### Return Value

number of intersections; may be zero

### Example

<div><fiddle-embed name="4961b05f4f26cf270ab4948a57876341"></fiddle-embed></div>

---

<a name="nothingToDraw"></a>
## nothingToDraw

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool nothingToDraw() const
</pre>

Returns true if <a href="bmh_SkPaint_Reference?cl=9919#Paint">Paint</a> prevents all drawing.
If <a href="bmh_SkPaint_Reference?cl=9919#nothingToDraw">nothingToDraw</a> returns false, the <a href="bmh_SkPaint_Reference?cl=9919#Paint">Paint</a> may or may not allow drawing.

Returns true if <a href="bmh_undocumented?cl=9919#Blend_Mode">Blend Mode</a> and <a href="bmh_undocumented?cl=9919#Alpha">Color Alpha</a> are enabled,
and computed <a href="bmh_undocumented?cl=9919#Alpha">Color Alpha</a> is zero.

### Return Value

true if <a href="bmh_SkPaint_Reference?cl=9919#Paint">Paint</a> prevents all drawing

### Example

<div><fiddle-embed name="fc5a771b915ac341f56554f01d282831">

#### Example Output

~~~~
initial nothing to draw: false
blend dst nothing to draw: true
blend src over nothing to draw: false
alpha 0 nothing to draw: true
~~~~

</fiddle-embed></div>

---

# <a name="Fast_Bounds"></a> Fast Bounds
<a href="bmh_SkPaint_Reference?cl=9919#Fast_Bounds">Fast Bounds</a> methods conservatively outset a drawing bounds by additional area
<a href="bmh_SkPaint_Reference?cl=9919#Paint">Paint</a> may draw to.

<a name="canComputeFastBounds"></a>
## canComputeFastBounds

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool canComputeFastBounds() const
</pre>

Returns true if <a href="bmh_SkPaint_Reference?cl=9919#Paint">Paint</a> does not include elements requiring extensive computation
to compute <a href="bmh_undocumented?cl=9919#Device">Device</a> bounds of drawn geometry. For instance, <a href="bmh_SkPaint_Reference?cl=9919#Paint">Paint</a> with <a href="bmh_undocumented?cl=9919#Path_Effect">Path Effect</a>
always returns false.

### Return Value

true if <a href="bmh_SkPaint_Reference?cl=9919#Paint">Paint</a> allows for fast computation of bounds

---

<a name="computeFastBounds"></a>
## computeFastBounds

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
const SkRect& computeFastBounds(const SkRect& orig, SkRect* storage) const
</pre>

Only call this if <a href="bmh_SkPaint_Reference?cl=9919#canComputeFastBounds">canComputeFastBounds</a> returned true. This takes a
raw rectangle (the raw bounds of a shape), and adjusts it for stylistic
effects in the paint (e.g. stroking). If needed, it uses the <a href="bmh_SkPaint_Reference?cl=9919#storage">storage</a>
rect parameter. It returns the adjusted bounds that can then be used
for <a href="bmh_SkCanvas_Reference?cl=9919#quickReject">SkCanvas::quickReject</a> tests.

The returned rect will either be <a href="bmh_SkPaint_Reference?cl=9919#orig">orig</a> or <a href="bmh_SkPaint_Reference?cl=9919#storage">storage</a>, thus the caller
should not rely on <a href="bmh_SkPaint_Reference?cl=9919#storage">storage</a> being set to the result, but should always
use the retured value. It is legal for <a href="bmh_SkPaint_Reference?cl=9919#orig">orig</a> and <a href="bmh_SkPaint_Reference?cl=9919#storage">storage</a> to be the same
rect.

### Parameters

<table>  <tr>    <td><code><strong>orig </strong></code></td> <td>
geometry modified by <a href="bmh_SkPaint_Reference?cl=9919#Paint">Paint</a> when drawn</td>
  </tr>  <tr>    <td><code><strong>storage </strong></code></td> <td>
computed bounds of geometry; may not be nullptr</td>
  </tr>
</table>

### Return Value

fast computed bounds

---

<a name="computeFastStrokeBounds"></a>
## computeFastStrokeBounds

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
const SkRect& computeFastStrokeBounds(const SkRect& orig, SkRect* storage) const
</pre>

### Parameters

<table>  <tr>    <td><code><strong>orig </strong></code></td> <td>
geometry modified by <a href="bmh_SkPaint_Reference?cl=9919#Paint">Paint</a> when drawn</td>
  </tr>  <tr>    <td><code><strong>storage </strong></code></td> <td>
computed bounds of geometry</td>
  </tr>
</table>

### Return Value

fast computed bounds

---

<a name="doComputeFastBounds"></a>
## doComputeFastBounds

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
const SkRect& doComputeFastBounds(const SkRect& orig, SkRect* storage,
                                  Style style) const
</pre>

Take the <a href="bmh_SkPaint_Reference?cl=9919#style">style</a> explicitly, so the caller can force us to be stroked
without having to make a copy of the paint just to change that field.

### Parameters

<table>  <tr>    <td><code><strong>orig </strong></code></td> <td>
geometry modified by <a href="bmh_SkPaint_Reference?cl=9919#Paint">Paint</a> when drawn</td>
  </tr>  <tr>    <td><code><strong>storage </strong></code></td> <td>
computed bounds of geometry</td>
  </tr>  <tr>    <td><code><strong>style </strong></code></td> <td>
overrides <a href="bmh_SkPaint_Reference?cl=9919#Style">Style</a></td>
  </tr>
</table>

### Return Value

fast computed bounds

---

<a name="toString"></a>
## toString

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void toString(SkString* str) const;
</pre>

Converts <a href="bmh_SkPaint_Reference?cl=9919#Paint">Paint</a> to machine parsable form in developer mode.

### Parameters

<table>  <tr>    <td><code><strong>str </strong></code></td> <td>
storage for string containing parsable <a href="bmh_SkPaint_Reference?cl=9919#Paint">Paint</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="5670c04b4562908169a776c48c92d104">

#### Example Output

~~~~
text size = 12
~~~~

</fiddle-embed></div>

---

