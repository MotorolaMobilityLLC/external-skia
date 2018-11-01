SkImageInfo Reference
===


<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
enum <a href='#SkAlphaType'>SkAlphaType</a> {
    <a href='#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>,
    <a href='#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>,
    <a href='#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>,
    <a href='#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>,
    <a href='#kLastEnum_SkAlphaType'>kLastEnum_SkAlphaType</a> = <a href='#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>,
};

static bool <a href='#SkAlphaTypeIsOpaque'>SkAlphaTypeIsOpaque</a>(<a href='#SkAlphaType'>SkAlphaType</a> at);

enum <a href='#SkColorType'>SkColorType</a> {
    <a href='#kUnknown_SkColorType'>kUnknown_SkColorType</a>,
    <a href='#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>,
    <a href='#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>,
    <a href='#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a>,
    <a href='#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>,
    <a href='#kRGB_888x_SkColorType'>kRGB_888x_SkColorType</a>,
    <a href='#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>,
    <a href='#kRGBA_1010102_SkColorType'>kRGBA_1010102_SkColorType</a>,
    <a href='#kRGB_101010x_SkColorType'>kRGB_101010x_SkColorType</a>,
    <a href='#kGray_8_SkColorType'>kGray_8_SkColorType</a>,
    <a href='#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a>,
    <a href='#kRGBA_F32_SkColorType'>kRGBA_F32_SkColorType</a>,
    <a href='#kLastEnum_SkColorType'>kLastEnum_SkColorType</a> = <a href='#kRGBA_F32_SkColorType'>kRGBA_F32_SkColorType</a>,
    <a href='#kN32_SkColorType'>kN32_SkColorType</a> = <a href='#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>,
    <a href='#kN32_SkColorType'>kN32_SkColorType</a> = <a href='#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>,
};

int <a href='#SkColorTypeBytesPerPixel'>SkColorTypeBytesPerPixel</a>(<a href='#SkColorType'>SkColorType</a> ct);

bool <a href='#SkColorTypeIsAlwaysOpaque'>SkColorTypeIsAlwaysOpaque</a>(<a href='#SkColorType'>SkColorType</a> ct);

bool <a href='#SkColorTypeValidateAlphaType'>SkColorTypeValidateAlphaType</a>(<a href='#SkColorType'>SkColorType</a> colorType, <a href='#SkAlphaType'>SkAlphaType</a> alphaType,
                                  <a href='#SkAlphaType'>SkAlphaType</a>* canonical = nullptr);

enum <a href='#SkYUVColorSpace'>SkYUVColorSpace</a> {
    <a href='#kJPEG_SkYUVColorSpace'>kJPEG_SkYUVColorSpace</a>,
    <a href='#kRec601_SkYUVColorSpace'>kRec601_SkYUVColorSpace</a>,
    <a href='#kRec709_SkYUVColorSpace'>kRec709_SkYUVColorSpace</a>,
    <a href='#kLastEnum_SkYUVColorSpace'>kLastEnum_SkYUVColorSpace</a> = <a href='#kRec709_SkYUVColorSpace'>kRec709_SkYUVColorSpace</a>,
};

struct <a href='#SkImageInfo'>SkImageInfo</a> {
    // <i><a href='#SkImageInfo'>SkImageInfo</a> interface</i>
};
</pre>

<a href='#Image_Info'>Image Info</a> specifies the dimensions and encoding of the pixels in a <a href='SkBitmap_Reference#Bitmap'>Bitmap</a>.
The dimensions are integral width and height. The encoding is how pixel
bits describe <a href='SkColor_Reference#Alpha'>Color Alpha</a>, transparency; <a href='SkColor_Reference#Color'>Color</a> components red, blue,
and green; and <a href='undocumented#Color_Space'>Color Space</a>, the range and linearity of colors.

<a href='#Image_Info'>Image Info</a> describes an uncompressed raster pixels. In contrast, <a href='SkImage_Reference#Image'>Image</a>
additionally describes compressed pixels like PNG, and <a href='SkSurface_Reference#Surface'>Surface</a> describes
destinations on the GPU. <a href='SkImage_Reference#Image'>Image</a> and <a href='SkSurface_Reference#Surface'>Surface</a> may be specified by <a href='#Image_Info'>Image Info</a>,
but <a href='SkImage_Reference#Image'>Image</a> and <a href='SkSurface_Reference#Surface'>Surface</a> may not contain <a href='#Image_Info'>Image Info</a>.

<a name='Alpha_Type'></a>

<a name='SkAlphaType'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
enum <a href='#SkAlphaType'>SkAlphaType</a> {
    <a href='#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>,
    <a href='#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>,
    <a href='#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>,
    <a href='#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>,
    <a href='#kLastEnum_SkAlphaType'>kLastEnum_SkAlphaType</a> = <a href='#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>,
};
</pre>

Describes how to interpret the alpha component of a pixel. A pixel may
be opaque, or <a href='SkColor_Reference#Alpha'>Color Alpha</a>, describing multiple levels of transparency.

In simple blending, <a href='SkColor_Reference#Alpha'>Color Alpha</a> weights the draw color and the destination
color to create a new color. If alpha describes a weight from zero to one,
new color is set to: <code>draw color&nbsp;\*&nbsp;alpha&nbsp;\+&nbsp;destination color&nbsp;\*&nbsp;\(1&nbsp;\-&nbsp;alpha\)</code>.

In practice alpha is encoded in two or more bits, where 1.0 equals all bits set.

RGB may have <a href='SkColor_Reference#Alpha'>Color Alpha</a> included in each component value; the stored
value is the original RGB multiplied by <a href='SkColor_Reference#Alpha'>Color Alpha</a>. <a href='undocumented#Premultiply'>Premultiplied</a> color
components improve performance.

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Details</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kUnknown_SkAlphaType'><code>kUnknown_SkAlphaType</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
<a href='#Alpha_Type'>Alpha Type</a> is uninitialized.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kOpaque_SkAlphaType'><code>kOpaque_SkAlphaType</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Opaque'>Opaque</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Pixels are opaque. The <a href='#Color_Type'>Color Type</a> must have no explicit alpha
component, or all alpha components must be set to their maximum value.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kPremul_SkAlphaType'><code>kPremul_SkAlphaType</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Premul'>Premul</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Pixels have <a href='SkColor_Reference#Alpha'>Alpha</a> <a href='undocumented#Premultiply'>Premultiplied</a> into color components.
<a href='SkSurface_Reference#Surface'>Surface</a> pixels must be <a href='undocumented#Premultiply'>Premultiplied</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kUnpremul_SkAlphaType'><code>kUnpremul_SkAlphaType</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>3</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Unpremul'>Unpremul</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
<a href='undocumented#Pixel'>Pixel</a> color component values are independent of alpha value.
Images generated from encoded data like PNG do not <a href='undocumented#Premultiply'>Premultiply</a> pixel color
components. <a href='#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a> is supported for <a href='SkImage_Reference#Image'>Image</a> pixels, but not for
<a href='SkSurface_Reference#Surface'>Surface</a> pixels.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kLastEnum_SkAlphaType'><code>kLastEnum_SkAlphaType</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>3</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Used by tests to iterate through all valid values.
</td>
  </tr>
</table>

### See Also

<a href='#SkColorType'>SkColorType</a> <a href='undocumented#SkColorSpace'>SkColorSpace</a>

<a name='Alpha_Type_Opaque'></a>

---

Use <a href='#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a> as a hint to optimize drawing when <a href='SkColor_Reference#Alpha'>Alpha</a> component
of all pixel is set to its maximum value of 1.0; all alpha component bits are set.
If <a href='#Image_Info'>Image Info</a> is set to <a href='#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a> but all alpha values are not 1.0,
results are undefined.

### Example

<div><fiddle-embed name="79146a1a41d58d22582fdc567c6ffe4e"><div><a href='SkColor_Reference#SkPreMultiplyARGB'>SkPreMultiplyARGB</a> parameter a is set to 255, its maximum value, and is interpreted
as <a href='SkColor_Reference#Alpha'>Color Alpha</a> of 1.0. <a href='#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a> may be set to improve performance.
If <a href='SkColor_Reference#SkPreMultiplyARGB'>SkPreMultiplyARGB</a> parameter a is set to a value smaller than 255,
<a href='#kPremul_SkAlphaType'>kPremul_SkAlphaType</a> must be used instead to avoid undefined results.
The four displayed values are the original component values, though not necessarily
in the same order.
</div></fiddle-embed></div>

<a name='Alpha_Type_Premul'></a>

---

Use <a href='#kPremul_SkAlphaType'>kPremul_SkAlphaType</a> when stored color components are the original color
multiplied by the alpha component. The alpha component range of 0.0 to 1.0 is
achieved by dividing the integer bit value by the maximum bit value.

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
stored color = original color * alpha / max alpha
</pre>

The color component must be equal to or smaller than the alpha component,
or the results are undefined.

### Example

<div><fiddle-embed name="ad696b39c915803d566e96896ec3a36c"><div><a href='SkColor_Reference#SkPreMultiplyARGB'>SkPreMultiplyARGB</a> parameter a is set to 150, less than its maximum value, and is
interpreted as <a href='SkColor_Reference#Alpha'>Color Alpha</a> of about 0.6. <a href='#kPremul_SkAlphaType'>kPremul_SkAlphaType</a> must be set, since
<a href='SkColor_Reference#SkPreMultiplyARGB'>SkPreMultiplyARGB</a> parameter a is set to a value smaller than 255,
to avoid undefined results.
The four displayed values reflect that the alpha component has been multiplied
by the original color.
</div></fiddle-embed></div>

<a name='Alpha_Type_Unpremul'></a>

---

Use <a href='#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a> if stored color components are not divided by the
alpha component. Some drawing destinations may not support
<a href='#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>.

### Example

<div><fiddle-embed name="b8216a9e5ff5bc61a0e46eba7d36307b"><div><a href='SkColor_Reference#SkColorSetARGB'>SkColorSetARGB</a> parameter a is set to 150, less than its maximum value, and is
interpreted as <a href='SkColor_Reference#Alpha'>Color Alpha</a> of about 0.6. color is not <a href='undocumented#Premultiply'>Premultiplied</a>;
color components may have values greater than color alpha.
The four displayed values are the original component values, though not necessarily
in the same order.
</div></fiddle-embed></div>

<a name='SkAlphaTypeIsOpaque'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static inline bool <a href='#SkAlphaTypeIsOpaque'>SkAlphaTypeIsOpaque</a>(<a href='#SkAlphaType'>SkAlphaType</a> at)
</pre>

Returns true if <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> equals <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>. <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a> is a
hint that the <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> is opaque, or that all <a href='SkColor_Reference#Alpha'>alpha</a> values are set to
their 1.0 equivalent. If <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> is <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>, and <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> is not
opaque, then the result of drawing any <a href='undocumented#Pixel'>pixel</a> with a <a href='SkColor_Reference#Alpha'>alpha</a> value less than
1.0 is undefined.

### Parameters

<table>  <tr>    <td><a name='SkAlphaTypeIsOpaque_at'><code><strong>at</strong></code></a></td>
    <td>one of:</td>
  </tr>
</table>

<a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>, <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>, <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>,
<a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>

### Return Value

true if <a href='#SkAlphaTypeIsOpaque_at'>at</a> equals <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>

<a name='Color_Type'></a>

<a name='SkColorType'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
enum <a href='#SkColorType'>SkColorType</a> {
    <a href='#kUnknown_SkColorType'>kUnknown_SkColorType</a>,
    <a href='#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>,
    <a href='#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>,
    <a href='#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a>,
    <a href='#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>,
    <a href='#kRGB_888x_SkColorType'>kRGB_888x_SkColorType</a>,
    <a href='#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>,
    <a href='#kRGBA_1010102_SkColorType'>kRGBA_1010102_SkColorType</a>,
    <a href='#kRGB_101010x_SkColorType'>kRGB_101010x_SkColorType</a>,
    <a href='#kGray_8_SkColorType'>kGray_8_SkColorType</a>,
    <a href='#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a>,
    <a href='#kRGBA_F32_SkColorType'>kRGBA_F32_SkColorType</a>,
    <a href='#kLastEnum_SkColorType'>kLastEnum_SkColorType</a> = <a href='#kRGBA_F32_SkColorType'>kRGBA_F32_SkColorType</a>,
    <a href='#kN32_SkColorType'>kN32_SkColorType</a> = <a href='#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>,
    <a href='#kN32_SkColorType'>kN32_SkColorType</a> = <a href='#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>,
};
</pre>

Describes how pixel bits encode color. A pixel may be an alpha mask, a
grayscale, RGB, or ARGB.

<a href='#kN32_SkColorType'>kN32_SkColorType</a> selects the native 32-bit ARGB format. On <a href='undocumented#Little_Endian'>Little Endian</a>
processors, pixels containing 8-bit ARGB components pack into 32-bit
<a href='#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>. On <a href='undocumented#Big_Endian'>Big Endian</a> processors, pixels pack into 32-bit
<a href='#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>.

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Details</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kUnknown_SkColorType'><code>kUnknown_SkColorType</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
<a href='#Color_Type'>Color Type</a> is set to <a href='#kUnknown_SkColorType'>kUnknown_SkColorType</a> by default. If set,
encoding format and size is unknown.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kAlpha_8_SkColorType'><code>kAlpha_8_SkColorType</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Alpha_8'>Alpha&nbsp;8</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Stores 8-bit byte pixel encoding that represents transparency. Value of zero
is completely transparent; a value of 255 is completely opaque.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kRGB_565_SkColorType'><code>kRGB_565_SkColorType</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#RGB_565'>RGB&nbsp;565</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Stores 16-bit word pixel encoding that contains five bits of blue,
six bits of green, and five bits of red.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kARGB_4444_SkColorType'><code>kARGB_4444_SkColorType</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>3</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#ARGB_4444'>ARGB&nbsp;4444</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Stores 16-bit word pixel encoding that contains four bits of alpha,
four bits of blue, four bits of green, and four bits of red.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kRGBA_8888_SkColorType'><code>kRGBA_8888_SkColorType</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>4</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#RGBA_8888'>RGBA&nbsp;8888</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Stores 32-bit word pixel encoding that contains eight bits of red,
eight bits of green, eight bits of blue, and eight bits of alpha.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kRGB_888x_SkColorType'><code>kRGB_888x_SkColorType</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>5</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#RGB_888'>RGB&nbsp;888</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Stores 32-bit word pixel encoding that contains eight bits of red,
eight bits of green, eight bits of blue, and eight unused bits.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kBGRA_8888_SkColorType'><code>kBGRA_8888_SkColorType</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>6</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#BGRA_8888'>BGRA&nbsp;8888</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Stores 32-bit word pixel encoding that contains eight bits of blue,
eight bits of green, eight bits of red, and eight bits of alpha.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kRGBA_1010102_SkColorType'><code>kRGBA_1010102_SkColorType</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>7</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#RGBA_1010102'>RGBA&nbsp;1010102</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Stores 32-bit word pixel encoding that contains ten bits of red,
ten bits of green, ten bits of blue, and two bits of alpha.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kRGB_101010x_SkColorType'><code>kRGB_101010x_SkColorType</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>8</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#RGB_101010'>RGB&nbsp;101010</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Stores 32-bit word pixel encoding that contains ten bits of red,
ten bits of green, ten bits of blue, and two unused bits.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kGray_8_SkColorType'><code>kGray_8_SkColorType</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>9</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Gray_8'>Gray&nbsp;8</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Stores 8-bit byte pixel encoding that equivalent to equal values for red,
blue, and green, representing colors from black to white.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kRGBA_F16_SkColorType'><code>kRGBA_F16_SkColorType</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>10</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#RGBA_F16'>RGBA&nbsp;F16</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Stores 64-bit word pixel encoding that contains 16 bits of blue,
16 bits of green, 16 bits of red, and 16 bits of alpha. Each component
is encoded as a half float.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kRGBA_F32_SkColorType'><code>kRGBA_F32_SkColorType</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>11</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#RGBA_F32'>RGBA&nbsp;F32</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Stores 128-bit word pixel encoding that contains 32 bits of blue,
32 bits of green, 32 bits of red, and 32 bits of alpha. Each component
is encoded as a single precision float.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kLastEnum_SkColorType'><code>kLastEnum_SkColorType</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>11</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Used by tests to iterate through all valid values.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kN32_SkColorType'><code>kN32_SkColorType</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>4 or 6</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Encodes ARGB as either <a href='#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a> or
<a href='#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>, whichever is native to the platform.
</td>
  </tr>
</table>

### See Also

<a href='#SkAlphaType'>SkAlphaType</a> <a href='undocumented#SkColorSpace'>SkColorSpace</a>

<a name='Color_Type_Alpha_8'></a>

---

<a href='SkColor_Reference#Alpha'>Alpha</a> pixels encode transparency without color information. Value of zero is
completely transparent; a value of 255 is completely opaque. <a href='SkBitmap_Reference#Bitmap'>Bitmap</a>
pixels do not visibly draw, because its pixels have no color information.
When <a href='#SkColorType'>SkColorType</a> is set to <a href='#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, the paired <a href='#SkAlphaType'>SkAlphaType</a> is
ignored.

### Example

<div><fiddle-embed name="21ae21e4ce53d2018e042dd457997300"><div><a href='SkColor_Reference#Alpha'>Alpha</a> pixels can modify another draw. orangePaint fills the bounds of bitmap,
with its transparency set to alpha8 pixel value.
</div></fiddle-embed></div>

### See Also

<a href='SkColor_Reference#Alpha'>Alpha</a> <a href='#Color_Type_Gray_8'>Gray 8</a>

<a name='Color_Type_RGB_565'></a>

---

<a href='#kRGB_565_SkColorType'>kRGB_565_SkColorType</a> encodes RGB to fit in a 16-bit word. Red and blue
components use five bits describing 32 levels. Green components, more sensitive
to the eye, use six bits describing 64 levels. <a href='#kRGB_565_SkColorType'>kRGB_565_SkColorType</a> has no
bits for <a href='SkColor_Reference#Alpha'>Alpha</a>.
Pixels are fully opaque as if its <a href='SkColor_Reference#Alpha'>Color Alpha</a> was set to one, and should
always be paired with <a href='#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>.

![Color_Type_RGB_565](https://fiddle.skia.org/i/6dec0226490a4ac1977dc87a31564147_raster.png "")

### Example

<div><fiddle-embed name="7e7c46bb4572e21e13529ff364eb0a9c"></fiddle-embed></div>

### See Also

<a href='#Color_Type_ARGB_4444'>ARGB 4444</a> <a href='#Color_Type_RGBA_8888'>RGBA 8888</a>

<a name='Color_Type_ARGB_4444'></a>

---

<a href='#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a> encodes ARGB to fit in 16-bit word. Each
component: alpha, blue, green, and red; use four bits, describing 16 levels.
Note that <a href='#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a> is misnamed; the acronym does not
describe the actual component order.

![Color_Type_ARGB_4444](https://fiddle.skia.org/i/e8008512f0d197051e3f26faa67bafc2_raster.png "")

If paired with <a href='#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>: blue, green, and red components are
<a href='undocumented#Premultiply'>Premultiplied</a> by the alpha value. If blue, green, or red is greater than alpha,
the drawn result is undefined.

If paired with <a href='#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>: alpha, blue, green, and red components
may have any value. There may be a performance penalty with Unpremultipled
pixels.

If paired with <a href='#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>: all alpha component values are at the maximum;
blue, green, and red components are fully opaque. If any alpha component is
less than 15, the drawn result is undefined.

### Example

<div><fiddle-embed name="33a360c3404ac21db801943336843d8e"></fiddle-embed></div>

### See Also

<a href='#Color_Type_RGBA_8888'>RGBA 8888</a>

<a name='Color_Type_RGBA_8888'></a>

---

<a href='#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a> encodes ARGB into a 32-bit word. Each component:
red, green, blue, alpha; use eight bits, describing 256 levels.

![Color_Type_RGBA_8888](https://fiddle.skia.org/i/9abc324f670e6468f09385551aae5a1c_raster.png "")

If paired with <a href='#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>: red, green, and blue components are
<a href='undocumented#Premultiply'>Premultiplied</a> by the alpha value. If red, green, or blue is greater than alpha,
the drawn result is undefined.

If paired with <a href='#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>: alpha, red, green, and blue components
may have any value. There may be a performance penalty with Unpremultipled
pixels.

If paired with <a href='#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>: all alpha component values are at the maximum;
red, green, and blue components are fully opaque. If any alpha component is
less than 255, the drawn result is undefined.

On <a href='undocumented#Big_Endian'>Big Endian</a> platforms, <a href='#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a> is the native <a href='#Color_Type'>Color Type</a>, and
will have the best performance. Use <a href='#kN32_SkColorType'>kN32_SkColorType</a> to choose the best
<a href='#Color_Type'>Color Type</a> for the platform at compile time.

### Example

<div><fiddle-embed name="947922a19d59893fe7f9d9ee1954379b"></fiddle-embed></div>

### See Also

<a href='#Color_Type_RGB_888'>RGB 888</a> <a href='#Color_Type_BGRA_8888'>BGRA 8888</a>

<a name='Color_Type_RGB_888'></a>

---

<a href='#kRGB_888x_SkColorType'>kRGB_888x_SkColorType</a> encodes RGB into a 32-bit word. Each component:
red, green, blue; use eight bits, describing 256 levels. Eight bits are
unused. Pixels described by <a href='#kRGB_888x_SkColorType'>kRGB_888x_SkColorType</a> are fully opaque as if
their <a href='SkColor_Reference#Alpha'>Color Alpha</a> was set to one, and should always be paired with
<a href='#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>.

![Color_Type_RGB_888](https://fiddle.skia.org/i/7527d7ade4764302818e250cd4e03962_raster.png "")

### Example

<div><fiddle-embed name="4260d6cc15db2c60c07f6fdc8d9ae425"></fiddle-embed></div>

### See Also

<a href='#Color_Type_RGBA_8888'>RGBA 8888</a> <a href='#Color_Type_BGRA_8888'>BGRA 8888</a>

<a name='Color_Type_BGRA_8888'></a>

---

<a href='#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a> encodes ARGB into a 32-bit word. Each component:
blue, green, red, and alpha; use eight bits, describing 256 levels.

![Color_Type_BGRA_8888](https://fiddle.skia.org/i/6c35ca14d88b0de200ba7f897f889ad7_raster.png "")

If paired with <a href='#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>: blue, green, and red components are
<a href='undocumented#Premultiply'>Premultiplied</a> by the alpha value. If blue, green, or red is greater than alpha,
the drawn result is undefined.

If paired with <a href='#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>: blue, green, red, and alpha components
may have any value. There may be a performance penalty with <a href='undocumented#Unpremultiply'>Unpremultiplied</a>
pixels.

If paired with <a href='#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>: all alpha component values are at the maximum;
blue, green, and red components are fully opaque. If any alpha component is
less than 255, the drawn result is undefined.

On <a href='undocumented#Little_Endian'>Little Endian</a> platforms, <a href='#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a> is the native <a href='#Color_Type'>Color Type</a>,
and will have the best performance. Use <a href='#kN32_SkColorType'>kN32_SkColorType</a> to choose the best
<a href='#Color_Type'>Color Type</a> for the platform at compile time.

### Example

<div><fiddle-embed name="945ce5344fce5470f8604b2e06e9f9ae"></fiddle-embed></div>

### See Also

<a href='#Color_Type_RGBA_8888'>RGBA 8888</a>

<a name='Color_Type_RGBA_1010102'></a>

---

<a href='#kRGBA_1010102_SkColorType'>kRGBA_1010102_SkColorType</a> encodes ARGB into a 32-bit word. Each
<a href='SkColor_Reference#Color'>Color</a> component: red, green, and blue; use ten bits, describing 1024 levels.
Two bits contain alpha, describing four levels. Possible alpha
values are zero: fully transparent; one: 33% opaque; two: 67% opaque;
three: fully opaque.

At present, <a href='SkColor_Reference#Color'>Color</a> in <a href='SkPaint_Reference#Paint'>Paint</a> does not provide enough precision to
draw all colors possible to a <a href='#kRGBA_1010102_SkColorType'>kRGBA_1010102_SkColorType</a> <a href='SkSurface_Reference#Surface'>Surface</a>.

![Color_Type_RGBA_1010102](https://fiddle.skia.org/i/8d78daf69145f611054f289a7443a670_raster.png "")

If paired with <a href='#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>: red, green, and blue components are
<a href='undocumented#Premultiply'>Premultiplied</a> by the alpha value. If red, green, or blue is greater than the
alpha replicated to ten bits, the drawn result is undefined.

If paired with <a href='#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>: alpha, red, green, and blue components
may have any value. There may be a performance penalty with <a href='undocumented#Unpremultiply'>Unpremultiplied</a>
pixels.

If paired with <a href='#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>: all alpha component values are at the maximum;
red, green, and blue components are fully opaque. If any alpha component is
less than three, the drawn result is undefined.

### Example

<div><fiddle-embed name="1282dc1127ce1b0061544619ae4de0f0"></fiddle-embed></div>

### See Also

<a href='#Color_Type_RGB_101010'>RGB 101010</a> <a href='#Color_Type_RGBA_8888'>RGBA 8888</a>

<a name='Color_Type_RGB_101010'></a>

---

<a href='#kRGB_101010x_SkColorType'>kRGB_101010x_SkColorType</a> encodes RGB into a 32-bit word. Each
<a href='SkColor_Reference#Color'>Color</a> component: red, green, and blue; use ten bits, describing 1024 levels.
Two bits are unused. Pixels described by <a href='#kRGB_101010x_SkColorType'>kRGB_101010x_SkColorType</a> are fully
opaque as if its <a href='SkColor_Reference#Alpha'>Color Alpha</a> was set to one, and should always be paired
with <a href='#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>.

At present, <a href='SkColor_Reference#Color'>Color</a> in <a href='SkPaint_Reference#Paint'>Paint</a> does not provide enough precision to
draw all colors possible to a <a href='#kRGB_101010x_SkColorType'>kRGB_101010x_SkColorType</a> <a href='SkSurface_Reference#Surface'>Surface</a>.

![Color_Type_RGB_101010](https://fiddle.skia.org/i/4c9f4d939e2047269d73fa3507caf01f_raster.png "")

### Example

<div><fiddle-embed name="92f81aa0459230459600a01e79ccff29"></fiddle-embed></div>

### See Also

<a href='#Color_Type_RGBA_1010102'>RGBA 1010102</a>

<a name='Color_Type_Gray_8'></a>

---

<a href='#kGray_8_SkColorType'>kGray_8_SkColorType</a> encodes grayscale level in eight bits that is equivalent
to equal values for red, blue, and green, representing colors from black to
white.  Pixels described by <a href='#kGray_8_SkColorType'>kGray_8_SkColorType</a> are fully
opaque as if its <a href='SkColor_Reference#Alpha'>Color Alpha</a> was set to one, and should always be paired with
<a href='#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>.

### Example

<div><fiddle-embed name="93da0eb0b6722a4f33dc7dae094abf0b"></fiddle-embed></div>

### See Also

<a href='#Color_Type_Alpha_8'>Alpha 8</a>

<a name='Color_Type_RGBA_F16'></a>

---

<a href='#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a> encodes ARGB into a 64-bit word. Each component:
blue, green, red, and alpha; use 16 bits, describing a floating point value,
from -65500 to 65000 with 3.31 decimal digits of precision.

At present, <a href='SkColor_Reference#Color'>Color</a> in <a href='SkPaint_Reference#Paint'>Paint</a> does not provide enough precision or range to
draw all colors possible to a <a href='#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a> <a href='SkSurface_Reference#Surface'>Surface</a>.

Each component encodes a floating point value using <a href='https://www.khronos.org/opengl/wiki/Small_Float_Formats'>Half floats</a></a> .
Meaningful colors are represented by the range 0.0 to 1.0, although smaller
and larger values may be useful when used in combination with <a href='undocumented#Transfer_Mode'>Transfer Mode</a>.

![Color_Type_RGBA_F16](https://fiddle.skia.org/i/1bb35ae52173e0fef874022ca8138adc_raster.png "")

If paired with <a href='#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>: blue, green, and red components are
<a href='undocumented#Premultiply'>Premultiplied</a> by the alpha value. If blue, green, or red is greater than alpha,
the drawn result is undefined.

If paired with <a href='#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>: blue, green, red, and alpha components
may have any value. There may be a performance penalty with <a href='undocumented#Unpremultiply'>Unpremultiplied</a>
pixels.

If paired with <a href='#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>: all alpha component values are at the maximum;
blue, green, and red components are fully opaque. If any alpha component is
less than one, the drawn result is undefined.

### Example

<div><fiddle-embed name="dd81527bbdf5eaae7dd21ac04ab84f9e"></fiddle-embed></div>

### See Also

<a href='SkColor4f_Reference#SkColor4f'>SkColor4f</a>

<a name='Color_Type_RGBA_F32'></a>

---

<a href='#kRGBA_F32_SkColorType'>kRGBA_F32_SkColorType</a> encodes ARGB into a 128-bit word. Each component:
blue, green, red, and alpha; use 32 bits, describing a floating point value,
from -3.402823e+38 to 3.402823e+38 with 7.225 decimal digits of precision.

At present, <a href='SkColor_Reference#Color'>Color</a> in <a href='SkPaint_Reference#Paint'>Paint</a> does not provide enough precision or range to
draw all colors possible to a <a href='#kRGBA_F32_SkColorType'>kRGBA_F32_SkColorType</a> <a href='SkSurface_Reference#Surface'>Surface</a>.

Each component encodes a floating point value using <a href='https://en.wikipedia.org/wiki/Single-precision_floating-point_format'>single-precision floats</a></a> .
Meaningful colors are represented by the range 0.0 to 1.0, although smaller
and larger values may be useful when used in combination with <a href='undocumented#Transfer_Mode'>Transfer Mode</a>.

![Color_Type_RGBA_F32](https://fiddle.skia.org/i/4ba31a8f9bc94a996f34da81ef541a9c_raster.png "")

If paired with <a href='#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>: blue, green, and red components are
<a href='undocumented#Premultiply'>Premultiplied</a> by the alpha value. If blue, green, or red is greater than alpha,
the drawn result is undefined.

If paired with <a href='#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>: blue, green, red, and alpha components
may have any value. There may be a performance penalty with <a href='undocumented#Unpremultiply'>Unpremultiplied</a>
pixels.

If paired with <a href='#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>: all alpha component values are at the maximum;
blue, green, and red components are fully opaque. If any alpha component is
less than one, the drawn result is undefined.

### See Also

<a href='SkColor4f_Reference#SkColor4f'>SkColor4f</a>

<a name='SkColorTypeBytesPerPixel'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkColorTypeBytesPerPixel'>SkColorTypeBytesPerPixel</a>(<a href='#SkColorType'>SkColorType</a> ct)
</pre>

Returns the number of bytes required to store a <a href='undocumented#Pixel'>pixel</a>, including unused padding.
Returns zero if <a href='#SkColorTypeBytesPerPixel_ct'>ct</a> is <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a> or invalid.

### Parameters

<table>  <tr>    <td><a name='SkColorTypeBytesPerPixel_ct'><code><strong>ct</strong></code></a></td>
    <td>one of:</td>
  </tr>
</table>

<a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>,
<a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>kRGB_888x_SkColorType</a>,
<a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_1010102_SkColorType'>kRGBA_1010102_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_101010x_SkColorType'>kRGB_101010x_SkColorType</a>,
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a>

### Return Value

bytes per <a href='undocumented#Pixel'>pixel</a>

### Example

<div><fiddle-embed name="09ef49d07cb7005ba3e34d5ea53896f5"><a href='#kUnknown_SkColorType'>kUnknown_SkColorType</a>, <a href='#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, <a href='#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>,
<a href='#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a>, <a href='#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>, <a href='#kRGB_888x_SkColorType'>kRGB_888x_SkColorType</a>,
<a href='#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>, <a href='#kRGBA_1010102_SkColorType'>kRGBA_1010102_SkColorType</a>, <a href='#kRGB_101010x_SkColorType'>kRGB_101010x_SkColorType</a>,
<a href='#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a> </fiddle-embed></div>

### See Also

<a href='#SkImageInfo_bytesPerPixel'>SkImageInfo::bytesPerPixel</a>

<a name='SkColorTypeIsAlwaysOpaque'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkColorTypeIsAlwaysOpaque'>SkColorTypeIsAlwaysOpaque</a>(<a href='#SkColorType'>SkColorType</a> ct)
</pre>

Returns true if <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> always decodes <a href='SkColor_Reference#Alpha'>alpha</a> to 1.0, making the <a href='undocumented#Pixel'>pixel</a>
fully opaque. If true, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> does not reserve bits to encode <a href='SkColor_Reference#Alpha'>alpha</a>.

### Parameters

<table>  <tr>    <td><a name='SkColorTypeIsAlwaysOpaque_ct'><code><strong>ct</strong></code></a></td>
    <td>one of:</td>
  </tr>
</table>

<a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>,
<a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>kRGB_888x_SkColorType</a>,
<a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_1010102_SkColorType'>kRGBA_1010102_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_101010x_SkColorType'>kRGB_101010x_SkColorType</a>,
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a>

### Return Value

true if <a href='SkColor_Reference#Alpha'>alpha</a> is always set to 1.0

### Example

<div><fiddle-embed name="9b3eb5aaa0dfea9feee54e7650fa5446"><a href='#kUnknown_SkColorType'>kUnknown_SkColorType</a>, <a href='#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, <a href='#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>,
<a href='#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a>, <a href='#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>, <a href='#kRGB_888x_SkColorType'>kRGB_888x_SkColorType</a>,
<a href='#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>, <a href='#kRGBA_1010102_SkColorType'>kRGBA_1010102_SkColorType</a>, <a href='#kRGB_101010x_SkColorType'>kRGB_101010x_SkColorType</a>,
<a href='#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a> </fiddle-embed></div>

### See Also

<a href='#SkColorTypeValidateAlphaType'>SkColorTypeValidateAlphaType</a>

<a name='SkColorTypeValidateAlphaType'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkColorTypeValidateAlphaType'>SkColorTypeValidateAlphaType</a>(<a href='#SkColorType'>SkColorType</a> <a href='#SkImageInfo_colorType'>colorType</a>, <a href='#SkAlphaType'>SkAlphaType</a> <a href='#SkImageInfo_alphaType'>alphaType</a>,
                                  <a href='#SkAlphaType'>SkAlphaType</a>* canonical = nullptr)
</pre>

Returns true if <a href='#SkColorTypeValidateAlphaType_canonical'>canonical</a> can be set to a valid <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> for <a href='#SkColorTypeValidateAlphaType_colorType'>colorType</a>. If
there is more than one valid <a href='#SkColorTypeValidateAlphaType_canonical'>canonical</a> <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, set to <a href='#SkColorTypeValidateAlphaType_alphaType'>alphaType</a>, if valid.
If true is returned and <a href='#SkColorTypeValidateAlphaType_canonical'>canonical</a> is not nullptr, store valid <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>.

Returns false only if <a href='#SkColorTypeValidateAlphaType_alphaType'>alphaType</a> is <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>,  <a href='SkImageInfo_Reference#Color_Type'>color type</a> is not
<a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, and <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> is not always opaque. If false is returned,
<a href='#SkColorTypeValidateAlphaType_canonical'>canonical</a> is ignored.

For <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>: set <a href='#SkColorTypeValidateAlphaType_canonical'>canonical</a> to <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a> and return true.
For <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>: set <a href='#SkColorTypeValidateAlphaType_canonical'>canonical</a> to <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a> or
<a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a> and return true if <a href='#SkColorTypeValidateAlphaType_alphaType'>alphaType</a> is not <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>.
For <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>kRGB_888x_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_101010x_SkColorType'>kRGB_101010x_SkColorType</a>, and
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>: set <a href='#SkColorTypeValidateAlphaType_canonical'>canonical</a> to <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a> and return true.
For <a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>,
<a href='SkImageInfo_Reference#kRGBA_1010102_SkColorType'>kRGBA_1010102_SkColorType</a>, and <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a>: set <a href='#SkColorTypeValidateAlphaType_canonical'>canonical</a> to <a href='#SkColorTypeValidateAlphaType_alphaType'>alphaType</a>
and return true if <a href='#SkColorTypeValidateAlphaType_alphaType'>alphaType</a> is not <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>.

### Parameters

<table>  <tr>    <td><a name='SkColorTypeValidateAlphaType_colorType'><code><strong>colorType</strong></code></a></td>
    <td>one of:</td>
  </tr>
</table>

<a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>,
<a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>kRGB_888x_SkColorType</a>,
<a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_1010102_SkColorType'>kRGBA_1010102_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_101010x_SkColorType'>kRGB_101010x_SkColorType</a>,
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a>

### Parameters

<table>  <tr>    <td><a name='SkColorTypeValidateAlphaType_alphaType'><code><strong>alphaType</strong></code></a></td>
    <td>one of:</td>
  </tr>
</table>

<a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>, <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>, <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>,
<a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>

### Parameters

<table>  <tr>    <td><a name='SkColorTypeValidateAlphaType_canonical'><code><strong>canonical</strong></code></a></td>
    <td>storage for <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a></td>
  </tr>
</table>

### Return Value

true if valid <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> can be associated with <a href='#SkColorTypeValidateAlphaType_colorType'>colorType</a>

### Example

<div><fiddle-embed name="befac1c29ed21507d367e4d824383a04"><a href='#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>, <a href='#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>, <a href='#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>,
<a href='#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a> <a href='#kUnknown_SkColorType'>kUnknown_SkColorType</a>, <a href='#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, <a href='#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>,
<a href='#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a>, <a href='#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>, <a href='#kRGB_888x_SkColorType'>kRGB_888x_SkColorType</a>,
<a href='#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>, <a href='#kRGBA_1010102_SkColorType'>kRGBA_1010102_SkColorType</a>, <a href='#kRGB_101010x_SkColorType'>kRGB_101010x_SkColorType</a>,
<a href='#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a> </fiddle-embed></div>

### See Also

<a href='#SkColorTypeIsAlwaysOpaque'>SkColorTypeIsAlwaysOpaque</a>

<a name='YUV_ColorSpace'></a>

<a name='SkYUVColorSpace'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
enum <a href='#SkYUVColorSpace'>SkYUVColorSpace</a> {
    <a href='#kJPEG_SkYUVColorSpace'>kJPEG_SkYUVColorSpace</a>,
    <a href='#kRec601_SkYUVColorSpace'>kRec601_SkYUVColorSpace</a>,
    <a href='#kRec709_SkYUVColorSpace'>kRec709_SkYUVColorSpace</a>,
    <a href='#kLastEnum_SkYUVColorSpace'>kLastEnum_SkYUVColorSpace</a> = <a href='#kRec709_SkYUVColorSpace'>kRec709_SkYUVColorSpace</a>,
};
</pre>

Describes color range of YUV pixels. The color mapping from YUV to RGB varies
depending on the source. YUV pixels may be generated by JPEG images, standard
video streams, or high definition video streams. Each has its own mapping from
YUV and RGB.

JPEG YUV values encode the full range of 0 to 255 for all three components.
Video YUV values range from 16 to 235 for all three components. Details of
encoding and conversion to RGB are described in <a href='https://en.wikipedia.org/wiki/YCbCr'>YCbCr color space</a></a> .

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kJPEG_SkYUVColorSpace'><code>kJPEG_SkYUVColorSpace</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Describes standard JPEG color space;
<a href='https://en.wikipedia.org/wiki/Rec._601'>CCIR 601</a></a> with full range of 0 to 255 for components.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kRec601_SkYUVColorSpace'><code>kRec601_SkYUVColorSpace</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Describes standard used by standard definition television;
<a href='https://en.wikipedia.org/wiki/Rec._601'>CCIR 601</a></a> with studio range of 16 to 235 range for components.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kRec709_SkYUVColorSpace'><code>kRec709_SkYUVColorSpace</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Describes standard used by high definition television;
<a href='https://en.wikipedia.org/wiki/Rec._709'>Rec. 709</a></a> with studio range of 16 to 235 range for components.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='kLastEnum_SkYUVColorSpace'><code>kLastEnum_SkYUVColorSpace</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Used by tests to iterate through all valid values.
</td>
  </tr>
</table>

### See Also

<a href='SkImage_Reference#SkImage_MakeFromYUVTexturesCopy'>SkImage::MakeFromYUVTexturesCopy</a> <a href='SkImage_Reference#SkImage_MakeFromNV12TexturesCopy'>SkImage::MakeFromNV12TexturesCopy</a>

<a name='SkImageInfo'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
struct <a href='#SkImageInfo'>SkImageInfo</a> {
public:
    <a href='#SkImageInfo_empty_constructor'>SkImageInfo()</a>;
    static <a href='#SkImageInfo'>SkImageInfo</a> <a href='#SkImageInfo_Make'>Make</a>(int width, int height, <a href='#SkColorType'>SkColorType</a> ct, <a href='#SkAlphaType'>SkAlphaType</a> at,
                     sk_sp<<a href='undocumented#SkColorSpace'>SkColorSpace</a>> cs = nullptr);
    static <a href='#SkImageInfo'>SkImageInfo</a> <a href='#SkImageInfo_MakeN32'>MakeN32</a>(int width, int height, <a href='#SkAlphaType'>SkAlphaType</a> at,
                        sk_sp<<a href='undocumented#SkColorSpace'>SkColorSpace</a>> cs = nullptr);
    static <a href='#SkImageInfo'>SkImageInfo</a> <a href='#SkImageInfo_MakeS32'>MakeS32</a>(int width, int height, <a href='#SkAlphaType'>SkAlphaType</a> at);
    static <a href='#SkImageInfo'>SkImageInfo</a> <a href='#SkImageInfo_MakeN32Premul'>MakeN32Premul</a>(int width, int height, sk_sp<<a href='undocumented#SkColorSpace'>SkColorSpace</a>> cs = nullptr);
    static <a href='#SkImageInfo'>SkImageInfo</a> <a href='#SkImageInfo_MakeN32Premul_2'>MakeN32Premul</a>(const <a href='undocumented#SkISize'>SkISize</a>& size);
    static <a href='#SkImageInfo'>SkImageInfo</a> <a href='#SkImageInfo_MakeA8'>MakeA8</a>(int width, int height);
    static <a href='#SkImageInfo'>SkImageInfo</a> <a href='#SkImageInfo_MakeUnknown'>MakeUnknown</a>(int width, int height);
    static <a href='#SkImageInfo'>SkImageInfo</a> <a href='#SkImageInfo_MakeUnknown_2'>MakeUnknown</a>();
    int <a href='#SkImageInfo_width'>width</a>() const;
    int <a href='#SkImageInfo_height'>height</a>() const;
    <a href='#SkColorType'>SkColorType</a> <a href='#SkImageInfo_colorType'>colorType</a>() const;
    <a href='#SkAlphaType'>SkAlphaType</a> <a href='#SkImageInfo_alphaType'>alphaType</a>() const;
    <a href='undocumented#SkColorSpace'>SkColorSpace</a>* <a href='#SkImageInfo_colorSpace'>colorSpace</a>() const;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkColorSpace'>SkColorSpace</a>> <a href='#SkImageInfo_refColorSpace'>refColorSpace</a>() const;
    bool <a href='#SkImageInfo_isEmpty'>isEmpty</a>() const;
    bool <a href='#SkImageInfo_isOpaque'>isOpaque</a>() const;
    <a href='undocumented#SkISize'>SkISize</a> <a href='#SkImageInfo_dimensions'>dimensions</a>() const;
    <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkImageInfo_bounds'>bounds</a>() const;
    bool <a href='#SkImageInfo_gammaCloseToSRGB'>gammaCloseToSRGB</a>() const;
    <a href='#SkImageInfo'>SkImageInfo</a> <a href='#SkImageInfo_makeWH'>makeWH</a>(int newWidth, int newHeight) const;
    <a href='#SkImageInfo'>SkImageInfo</a> <a href='#SkImageInfo_makeAlphaType'>makeAlphaType</a>(<a href='#SkAlphaType'>SkAlphaType</a> newAlphaType) const;
    <a href='#SkImageInfo'>SkImageInfo</a> <a href='#SkImageInfo_makeColorType'>makeColorType</a>(<a href='#SkColorType'>SkColorType</a> newColorType) const;
    <a href='#SkImageInfo'>SkImageInfo</a> <a href='#SkImageInfo_makeColorSpace'>makeColorSpace</a>(sk_sp<<a href='undocumented#SkColorSpace'>SkColorSpace</a>> cs) const;
    int <a href='#SkImageInfo_bytesPerPixel'>bytesPerPixel</a>() const;
    int <a href='#SkImageInfo_shiftPerPixel'>shiftPerPixel</a>() const;
    uint64_t <a href='#SkImageInfo_minRowBytes64'>minRowBytes64</a>() const;
    size_t <a href='#SkImageInfo_minRowBytes'>minRowBytes</a>() const;
    size_t <a href='#SkImageInfo_computeOffset'>computeOffset</a>(int x, int y, size_t rowBytes) const;
    bool <a href='#SkImageInfo_equal1_operator'>operator==(const SkImageInfo& other)_const</a>;
    bool <a href='#SkImageInfo_notequal1_operator'>operator!=(const SkImageInfo& other)_const</a>;
    size_t <a href='#SkImageInfo_computeByteSize'>computeByteSize</a>(size_t rowBytes) const;
    size_t <a href='#SkImageInfo_computeMinByteSize'>computeMinByteSize</a>() const;
    static bool <a href='#SkImageInfo_ByteSizeOverflowed'>ByteSizeOverflowed</a>(size_t byteSize);
    bool <a href='#SkImageInfo_validRowBytes'>validRowBytes</a>(size_t rowBytes) const;
    void <a href='#SkImageInfo_reset'>reset</a>();
    void <a href='#SkImageInfo_validate'>validate</a>() const;
};
</pre>

Describes pixel dimensions and encoding. <a href='SkBitmap_Reference#Bitmap'>Bitmap</a>, <a href='SkImage_Reference#Image'>Image</a>, PixMap, and <a href='SkSurface_Reference#Surface'>Surface</a>
can be created from <a href='#Image_Info'>Image Info</a>. <a href='#Image_Info'>Image Info</a> can be retrieved from <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> and
<a href='SkPixmap_Reference#Pixmap'>Pixmap</a>, but not from <a href='SkImage_Reference#Image'>Image</a> and <a href='SkSurface_Reference#Surface'>Surface</a>. For example, <a href='SkImage_Reference#Image'>Image</a> and <a href='SkSurface_Reference#Surface'>Surface</a>
implementations may defer pixel depth, so may not completely specify <a href='#Image_Info'>Image Info</a>.

<a href='#Image_Info'>Image Info</a> contains dimensions, the pixel integral width and height. It encodes
how pixel bits describe <a href='SkColor_Reference#Alpha'>Color Alpha</a>, transparency; <a href='SkColor_Reference#Color'>Color</a> components red, blue,
and green; and <a href='undocumented#Color_Space'>Color Space</a>, the range and linearity of colors.

<a name='SkImageInfo_empty_constructor'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkImageInfo'>SkImageInfo</a>()
</pre>

Creates an empty <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> with <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>,
a width and height of zero, and no <a href='undocumented#SkColorSpace'>SkColorSpace</a>.

### Return Value

empty <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>

### Example

<div><fiddle-embed name="f206f698e7a8db3d84334c26b1a702dc"><div>An empty <a href='#Image_Info'>Image Info</a> may be passed to <a href='SkCanvas_Reference#SkCanvas_accessTopLayerPixels'>SkCanvas::accessTopLayerPixels</a> as storage
for the <a href='SkCanvas_Reference#Canvas'>Canvas</a> actual <a href='#Image_Info'>Image Info</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkImageInfo_Make'>Make</a> <a href='#SkImageInfo_MakeN32'>MakeN32</a> <a href='#SkImageInfo_MakeS32'>MakeS32</a> <a href='#SkImageInfo_MakeA8'>MakeA8</a>

<a name='SkImageInfo_Make'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='#SkImageInfo'>SkImageInfo</a> <a href='#SkImageInfo_Make'>Make</a>(int width, int height, <a href='#SkColorType'>SkColorType</a> ct, <a href='#SkAlphaType'>SkAlphaType</a> at,
                        <a href='undocumented#sk_sp'>sk sp</a>&lt;<a href='undocumented#SkColorSpace'>SkColorSpace</a>&gt; cs = nullptr)
</pre>

Creates <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> from integral dimensions <a href='#SkImageInfo_Make_width'>width</a> and <a href='#SkImageInfo_Make_height'>height</a>, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='#SkImageInfo_Make_ct'>ct</a>,
<a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='#SkImageInfo_Make_at'>at</a>, and optionally <a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='#SkImageInfo_Make_cs'>cs</a>.

If <a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='#SkImageInfo_Make_cs'>cs</a> is nullptr and <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> is part of drawing source: <a href='undocumented#SkColorSpace'>SkColorSpace</a>
defaults to sRGB, mapping into <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='undocumented#SkColorSpace'>SkColorSpace</a>.

Parameters are not validated to see if their values are legal, or that the
combination is supported.

### Parameters

<table>  <tr>    <td><a name='SkImageInfo_Make_width'><code><strong>width</strong></code></a></td>
    <td><a href='undocumented#Pixel'>pixel</a> column count; must be zero or greater</td>
  </tr>
  <tr>    <td><a name='SkImageInfo_Make_height'><code><strong>height</strong></code></a></td>
    <td><a href='undocumented#Pixel'>pixel</a> row count; must be zero or greater</td>
  </tr>
  <tr>    <td><a name='SkImageInfo_Make_ct'><code><strong>ct</strong></code></a></td>
    <td>one of:</td>
  </tr>
</table>

<a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>,
<a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>kRGB_888x_SkColorType</a>,
<a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_1010102_SkColorType'>kRGBA_1010102_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_101010x_SkColorType'>kRGB_101010x_SkColorType</a>,
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a>

### Parameters

<table>  <tr>    <td><a name='SkImageInfo_Make_at'><code><strong>at</strong></code></a></td>
    <td>one of:</td>
  </tr>
</table>

<a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>, <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>, <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>,
<a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>

### Parameters

<table>  <tr>    <td><a name='SkImageInfo_Make_cs'><code><strong>cs</strong></code></a></td>
    <td>range of colors; may be nullptr</td>
  </tr>
</table>

### Return Value

created <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>

### Example

<div><fiddle-embed name="9f47f9c2a99473f5b1113db48096d586"></fiddle-embed></div>

### See Also

<a href='#SkImageInfo_MakeN32'>MakeN32</a> <a href='#SkImageInfo_MakeN32Premul'>MakeN32Premul</a><sup><a href='#SkImageInfo_MakeN32Premul_2'>[2]</a></sup> <a href='#SkImageInfo_MakeS32'>MakeS32</a> <a href='#SkImageInfo_MakeA8'>MakeA8</a>

<a name='SkImageInfo_MakeN32'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='#SkImageInfo'>SkImageInfo</a> <a href='#SkImageInfo_MakeN32'>MakeN32</a>(int width, int height, <a href='#SkAlphaType'>SkAlphaType</a> at, <a href='undocumented#sk_sp'>sk sp</a>&lt;<a href='undocumented#SkColorSpace'>SkColorSpace</a>&gt; cs = nullptr)
</pre>

Creates <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> from integral dimensions <a href='#SkImageInfo_MakeN32_width'>width</a> and <a href='#SkImageInfo_MakeN32_height'>height</a>, <a href='SkImageInfo_Reference#kN32_SkColorType'>kN32_SkColorType</a>,
<a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='#SkImageInfo_MakeN32_at'>at</a>, and optionally <a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='#SkImageInfo_MakeN32_cs'>cs</a>. <a href='SkImageInfo_Reference#kN32_SkColorType'>kN32_SkColorType</a> will equal either
<a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a> or <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>, whichever is optimal.

If <a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='#SkImageInfo_MakeN32_cs'>cs</a> is nullptr and <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> is part of drawing source: <a href='undocumented#SkColorSpace'>SkColorSpace</a>
defaults to sRGB, mapping into <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='undocumented#SkColorSpace'>SkColorSpace</a>.

Parameters are not validated to see if their values are legal, or that the
combination is supported.

### Parameters

<table>  <tr>    <td><a name='SkImageInfo_MakeN32_width'><code><strong>width</strong></code></a></td>
    <td><a href='undocumented#Pixel'>pixel</a> column count; must be zero or greater</td>
  </tr>
  <tr>    <td><a name='SkImageInfo_MakeN32_height'><code><strong>height</strong></code></a></td>
    <td><a href='undocumented#Pixel'>pixel</a> row count; must be zero or greater</td>
  </tr>
  <tr>    <td><a name='SkImageInfo_MakeN32_at'><code><strong>at</strong></code></a></td>
    <td>one of:</td>
  </tr>
</table>

<a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>, <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>, <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>,
<a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>

### Parameters

<table>  <tr>    <td><a name='SkImageInfo_MakeN32_cs'><code><strong>cs</strong></code></a></td>
    <td>range of colors; may be nullptr</td>
  </tr>
</table>

### Return Value

created <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>

### Example

<div><fiddle-embed name="78cea0c4cac205b61ad6f6c982cbd888"></fiddle-embed></div>

### See Also

<a href='#SkImageInfo_Make'>Make</a> <a href='#SkImageInfo_MakeN32Premul'>MakeN32Premul</a><sup><a href='#SkImageInfo_MakeN32Premul_2'>[2]</a></sup> <a href='#SkImageInfo_MakeS32'>MakeS32</a> <a href='#SkImageInfo_MakeA8'>MakeA8</a>

<a name='SkImageInfo_MakeS32'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='#SkImageInfo'>SkImageInfo</a> <a href='#SkImageInfo_MakeS32'>MakeS32</a>(int width, int height, <a href='#SkAlphaType'>SkAlphaType</a> at)
</pre>

Creates <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> from integral dimensions <a href='#SkImageInfo_MakeS32_width'>width</a> and <a href='#SkImageInfo_MakeS32_height'>height</a>, <a href='SkImageInfo_Reference#kN32_SkColorType'>kN32_SkColorType</a>,
<a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='#SkImageInfo_MakeS32_at'>at</a>, with sRGB <a href='undocumented#SkColorSpace'>SkColorSpace</a>.

Parameters are not validated to see if their values are legal, or that the
combination is supported.

### Parameters

<table>  <tr>    <td><a name='SkImageInfo_MakeS32_width'><code><strong>width</strong></code></a></td>
    <td><a href='undocumented#Pixel'>pixel</a> column count; must be zero or greater</td>
  </tr>
  <tr>    <td><a name='SkImageInfo_MakeS32_height'><code><strong>height</strong></code></a></td>
    <td><a href='undocumented#Pixel'>pixel</a> row count; must be zero or greater</td>
  </tr>
  <tr>    <td><a name='SkImageInfo_MakeS32_at'><code><strong>at</strong></code></a></td>
    <td>one of:</td>
  </tr>
</table>

<a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>, <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>, <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>,
<a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>

### Return Value

created <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>

### Example

<div><fiddle-embed name="de418ccb42471d1589508ef3955f8c53"><div>Top gradient is drawn to offScreen without <a href='undocumented#Color_Space'>Color Space</a>. It is darker than middle
gradient, drawn to offScreen with sRGB <a href='undocumented#Color_Space'>Color Space</a>. Bottom gradient shares bits
with middle, but does not specify the <a href='undocumented#Color_Space'>Color Space</a> in noColorSpaceBitmap. A source
without <a href='undocumented#Color_Space'>Color Space</a> is treated as sRGB; the bottom gradient is identical to the
middle gradient.
</div></fiddle-embed></div>

### See Also

<a href='#SkImageInfo_Make'>Make</a> <a href='#SkImageInfo_MakeN32'>MakeN32</a> <a href='#SkImageInfo_MakeN32Premul'>MakeN32Premul</a><sup><a href='#SkImageInfo_MakeN32Premul_2'>[2]</a></sup> <a href='#SkImageInfo_MakeA8'>MakeA8</a>

<a name='SkImageInfo_MakeN32Premul'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='#SkImageInfo'>SkImageInfo</a> <a href='#SkImageInfo_MakeN32Premul'>MakeN32Premul</a>(int width, int height, <a href='undocumented#sk_sp'>sk sp</a>&lt;<a href='undocumented#SkColorSpace'>SkColorSpace</a>&gt; cs = nullptr)
</pre>

Creates <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> from integral dimensions <a href='#SkImageInfo_MakeN32Premul_width'>width</a> and <a href='#SkImageInfo_MakeN32Premul_height'>height</a>, <a href='SkImageInfo_Reference#kN32_SkColorType'>kN32_SkColorType</a>,
<a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>, with optional <a href='undocumented#SkColorSpace'>SkColorSpace</a>.

If <a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='#SkImageInfo_MakeN32Premul_cs'>cs</a> is nullptr and <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> is part of drawing source: <a href='undocumented#SkColorSpace'>SkColorSpace</a>
defaults to sRGB, mapping into <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='undocumented#SkColorSpace'>SkColorSpace</a>.

Parameters are not validated to see if their values are legal, or that the
combination is supported.

### Parameters

<table>  <tr>    <td><a name='SkImageInfo_MakeN32Premul_width'><code><strong>width</strong></code></a></td>
    <td><a href='undocumented#Pixel'>pixel</a> column count; must be zero or greater</td>
  </tr>
  <tr>    <td><a name='SkImageInfo_MakeN32Premul_height'><code><strong>height</strong></code></a></td>
    <td><a href='undocumented#Pixel'>pixel</a> row count; must be zero or greater</td>
  </tr>
  <tr>    <td><a name='SkImageInfo_MakeN32Premul_cs'><code><strong>cs</strong></code></a></td>
    <td>range of colors; may be nullptr</td>
  </tr>
</table>

### Return Value

created <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>

### Example

<div><fiddle-embed name="525650a67e19fdd8ca9f72b7eda65174"></fiddle-embed></div>

### See Also

<a href='#SkImageInfo_MakeN32'>MakeN32</a> <a href='#SkImageInfo_MakeS32'>MakeS32</a> <a href='#SkImageInfo_MakeA8'>MakeA8</a> <a href='#SkImageInfo_Make'>Make</a>

<a name='SkImageInfo_MakeN32Premul_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='#SkImageInfo'>SkImageInfo</a> <a href='#SkImageInfo_MakeN32Premul'>MakeN32Premul</a>(const <a href='undocumented#SkISize'>SkISize</a>& size)
</pre>

Creates <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> from integral dimensions width and height, <a href='SkImageInfo_Reference#kN32_SkColorType'>kN32_SkColorType</a>,
<a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>, with <a href='undocumented#SkColorSpace'>SkColorSpace</a> set to nullptr.

If <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> is part of drawing source: <a href='undocumented#SkColorSpace'>SkColorSpace</a> defaults to sRGB, mapping
into <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='undocumented#SkColorSpace'>SkColorSpace</a>.

Parameters are not validated to see if their values are legal, or that the
combination is supported.

### Parameters

<table>  <tr>    <td><a name='SkImageInfo_MakeN32Premul_2_size'><code><strong>size</strong></code></a></td>
    <td>width and height, each must be zero or greater</td>
  </tr>
</table>

### Return Value

created <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>

### Example

<div><fiddle-embed name="b9026d7f39029756bd7cab9542c64f4e"></fiddle-embed></div>

### See Also

<a href='#SkImageInfo_MakeN32'>MakeN32</a> <a href='#SkImageInfo_MakeS32'>MakeS32</a> <a href='#SkImageInfo_MakeA8'>MakeA8</a> <a href='#SkImageInfo_Make'>Make</a>

<a name='SkImageInfo_MakeA8'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='#SkImageInfo'>SkImageInfo</a> <a href='#SkImageInfo_MakeA8'>MakeA8</a>(int width, int height)
</pre>

Creates <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> from integral dimensions <a href='#SkImageInfo_MakeA8_width'>width</a> and <a href='#SkImageInfo_MakeA8_height'>height</a>, <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>,
<a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>, with <a href='undocumented#SkColorSpace'>SkColorSpace</a> set to nullptr.

### Parameters

<table>  <tr>    <td><a name='SkImageInfo_MakeA8_width'><code><strong>width</strong></code></a></td>
    <td><a href='undocumented#Pixel'>pixel</a> column count; must be zero or greater</td>
  </tr>
  <tr>    <td><a name='SkImageInfo_MakeA8_height'><code><strong>height</strong></code></a></td>
    <td><a href='undocumented#Pixel'>pixel</a> row count; must be zero or greater</td>
  </tr>
</table>

### Return Value

created <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>

### Example

<div><fiddle-embed name="547388991687b8e10d482d8b1c82777d"></fiddle-embed></div>

### See Also

<a href='#SkImageInfo_MakeN32'>MakeN32</a> <a href='#SkImageInfo_MakeS32'>MakeS32</a> <a href='#SkImageInfo_Make'>Make</a>

<a name='SkImageInfo_MakeUnknown'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='#SkImageInfo'>SkImageInfo</a> <a href='#SkImageInfo_MakeUnknown'>MakeUnknown</a>(int width, int height)
</pre>

Creates <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> from integral dimensions <a href='#SkImageInfo_MakeUnknown_width'>width</a> and <a href='#SkImageInfo_MakeUnknown_height'>height</a>, <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>,
<a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>, with <a href='undocumented#SkColorSpace'>SkColorSpace</a> set to nullptr.

Returned <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> as part of source does not draw, and as part of destination
can not be drawn to.

### Parameters

<table>  <tr>    <td><a name='SkImageInfo_MakeUnknown_width'><code><strong>width</strong></code></a></td>
    <td><a href='undocumented#Pixel'>pixel</a> column count; must be zero or greater</td>
  </tr>
  <tr>    <td><a name='SkImageInfo_MakeUnknown_height'><code><strong>height</strong></code></a></td>
    <td><a href='undocumented#Pixel'>pixel</a> row count; must be zero or greater</td>
  </tr>
</table>

### Return Value

created <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>

### Example

<div><fiddle-embed name="75f13a78b28b08c72baf32b7d868de1c"></fiddle-embed></div>

### See Also

<a href='#SkImageInfo_empty_constructor'>SkImageInfo()</a> <a href='#SkImageInfo_MakeN32'>MakeN32</a> <a href='#SkImageInfo_MakeS32'>MakeS32</a> <a href='#SkImageInfo_Make'>Make</a>

<a name='SkImageInfo_MakeUnknown_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='#SkImageInfo'>SkImageInfo</a> <a href='#SkImageInfo_MakeUnknown'>MakeUnknown</a>()
</pre>

Creates <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> from integral dimensions width and height set to zero,
<a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>, with <a href='undocumented#SkColorSpace'>SkColorSpace</a> set to nullptr.

Returned <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> as part of source does not draw, and as part of destination
can not be drawn to.

### Return Value

created <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>

### Example

<div><fiddle-embed name="a1af7696ae0cdd6f379546dd1f211b7a"></fiddle-embed></div>

### See Also

<a href='#SkImageInfo_empty_constructor'>SkImageInfo()</a> <a href='#SkImageInfo_MakeN32'>MakeN32</a> <a href='#SkImageInfo_MakeS32'>MakeS32</a> <a href='#SkImageInfo_Make'>Make</a>

<a name='Property'></a>

<a name='SkImageInfo_width'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkImageInfo_width'>width</a>() const
</pre>

Returns <a href='undocumented#Pixel'>pixel</a> count in each row.

### Return Value

<a href='undocumented#Pixel'>pixel</a> width

### Example

<div><fiddle-embed name="e2491817695290d0218be77f091b8460"></fiddle-embed></div>

### See Also

<a href='#SkImageInfo_height'>height</a> <a href='SkBitmap_Reference#SkBitmap_width'>SkBitmap::width</a> <a href='undocumented#SkPixelRef_width'>SkPixelRef::width</a> <a href='SkImage_Reference#SkImage_width'>SkImage::width</a> <a href='SkSurface_Reference#SkSurface_width'>SkSurface::width</a>

<a name='SkImageInfo_height'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkImageInfo_height'>height</a>() const
</pre>

Returns <a href='undocumented#Pixel'>pixel</a> row count.

### Return Value

<a href='undocumented#Pixel'>pixel</a> height

### Example

<div><fiddle-embed name="72c35baaeddca1d912edf93d19429c8e"></fiddle-embed></div>

### See Also

<a href='#SkImageInfo_width'>width</a> <a href='SkBitmap_Reference#SkBitmap_height'>SkBitmap::height</a> <a href='undocumented#SkPixelRef_height'>SkPixelRef::height</a> <a href='SkImage_Reference#SkImage_height'>SkImage::height</a> <a href='SkSurface_Reference#SkSurface_height'>SkSurface::height</a>

<a name='SkImageInfo_colorType'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkColorType'>SkColorType</a> <a href='#SkImageInfo_colorType'>colorType</a>() const
</pre>

Returns <a href='#Color_Type'>Color Type</a>, one of: <a href='#kUnknown_SkColorType'>kUnknown_SkColorType</a>, <a href='#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, <a href='#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>,
<a href='#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a>, <a href='#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>, <a href='#kRGB_888x_SkColorType'>kRGB_888x_SkColorType</a>,
<a href='#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>, <a href='#kRGBA_1010102_SkColorType'>kRGBA_1010102_SkColorType</a>, <a href='#kRGB_101010x_SkColorType'>kRGB_101010x_SkColorType</a>,
<a href='#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a>.

### Return Value

<a href='#Color_Type'>Color Type</a>

### Example

<div><fiddle-embed name="06ecc3ce7f35cc7f930cbc2a662e3105">

#### Example Output

~~~~
color type: kAlpha_8_SkColorType
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkImageInfo_alphaType'>alphaType</a> <a href='SkPixmap_Reference#SkPixmap_colorType'>SkPixmap::colorType</a> <a href='SkBitmap_Reference#SkBitmap_colorType'>SkBitmap::colorType</a>

<a name='SkImageInfo_alphaType'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkAlphaType'>SkAlphaType</a> <a href='#SkImageInfo_alphaType'>alphaType</a>() const
</pre>

Returns <a href='#Alpha_Type'>Alpha Type</a>, one of: <a href='#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>, <a href='#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>, <a href='#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>,
<a href='#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>.

### Return Value

<a href='#Alpha_Type'>Alpha Type</a>

### Example

<div><fiddle-embed name="5c1d2499a4056b6cff38c1cf924158a1">

#### Example Output

~~~~
alpha type: kPremul_SkAlphaType
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkImageInfo_colorType'>colorType</a> <a href='SkPixmap_Reference#SkPixmap_alphaType'>SkPixmap::alphaType</a> <a href='SkBitmap_Reference#SkBitmap_alphaType'>SkBitmap::alphaType</a>

<a name='SkImageInfo_colorSpace'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkColorSpace'>SkColorSpace</a>* <a href='#SkImageInfo_colorSpace'>colorSpace</a>() const
</pre>

Returns <a href='undocumented#SkColorSpace'>SkColorSpace</a>, the range of colors. The  <a href='undocumented#Reference_Count'>reference count</a> of
<a href='undocumented#SkColorSpace'>SkColorSpace</a> is unchanged. The returned <a href='undocumented#SkColorSpace'>SkColorSpace</a> is immutable.

### Return Value

<a href='undocumented#SkColorSpace'>SkColorSpace</a>, or nullptr

### Example

<div><fiddle-embed name="5602b816d7cf75e3851274ef36a4c10f"><div><a href='undocumented#SkColorSpace_MakeSRGBLinear'>SkColorSpace::MakeSRGBLinear</a> creates <a href='undocumented#Color_Space'>Color Space</a> with linear gamma
and an sRGB gamut. This <a href='undocumented#Color_Space'>Color Space</a> gamma is not close to sRGB gamma.
</div>

#### Example Output

~~~~
gammaCloseToSRGB: false  gammaIsLinear: true  isSRGB: false
~~~~

</fiddle-embed></div>

### See Also

<a href='undocumented#Color_Space'>Color Space</a> <a href='SkPixmap_Reference#SkPixmap_colorSpace'>SkPixmap::colorSpace</a> <a href='SkBitmap_Reference#SkBitmap_colorSpace'>SkBitmap::colorSpace</a>

<a name='SkImageInfo_refColorSpace'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk sp</a>&lt;<a href='undocumented#SkColorSpace'>SkColorSpace</a>&gt; <a href='#SkImageInfo_refColorSpace'>refColorSpace</a>() const
</pre>

Returns  <a href='undocumented#Smart_Pointer'>smart pointer</a> to <a href='undocumented#SkColorSpace'>SkColorSpace</a>, the range of colors. The  <a href='undocumented#Smart_Pointer'>smart pointer</a>
tracks the number of objects sharing this <a href='undocumented#SkColorSpace'>SkColorSpace</a> reference so the memory
is released when the owners destruct.

The returned <a href='undocumented#SkColorSpace'>SkColorSpace</a> is immutable.

### Return Value

<a href='undocumented#SkColorSpace'>SkColorSpace</a> wrapped in a  <a href='undocumented#Smart_Pointer'>smart pointer</a>

### Example

<div><fiddle-embed name="33f65524736736fd91802b4198ba6fa8"></fiddle-embed></div>

### See Also

<a href='undocumented#Color_Space'>Color Space</a> <a href='SkBitmap_Reference#SkBitmap_refColorSpace'>SkBitmap::refColorSpace</a>

<a name='SkImageInfo_isEmpty'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkImageInfo_isEmpty'>isEmpty</a>() const
</pre>

Returns if <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> describes an empty area of pixels by checking if either
width or height is zero or smaller.

### Return Value

true if either dimension is zero or smaller

### Example

<div><fiddle-embed name="b8757200da5be0b43763cf79feb681a7">

#### Example Output

~~~~
width: 0 height: 0 empty: true
width: 0 height: 2 empty: true
width: 2 height: 0 empty: true
width: 2 height: 2 empty: false
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkImageInfo_dimensions'>dimensions</a> <a href='#SkImageInfo_bounds'>bounds</a> <a href='SkBitmap_Reference#SkBitmap_empty'>SkBitmap::empty</a> <a href='SkPixmap_Reference#SkPixmap_bounds'>SkPixmap::bounds</a>

<a name='SkImageInfo_isOpaque'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkImageInfo_isOpaque'>isOpaque</a>() const
</pre>

Returns true if <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> is set to hint that all pixels are opaque; their
<a href='SkColor_Reference#Alpha'>alpha</a> value is implicitly or explicitly 1.0. If true, and all pixels are
not opaque, Skia may draw incorrectly.

Does not check if <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> allows <a href='SkColor_Reference#Alpha'>alpha</a>, or if any <a href='undocumented#Pixel'>pixel</a> value has
transparency.

### Return Value

true if <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> is <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>

### Example

<div><fiddle-embed name="e9bd4f02b6cfb3ac864cb7fee7d7299c">

#### Example Output

~~~~
isOpaque: false
isOpaque: false
isOpaque: true
isOpaque: true
~~~~

</fiddle-embed></div>

### See Also

<a href='SkColor_Reference#Alpha'>Color Alpha</a> <a href='#SkColorTypeValidateAlphaType'>SkColorTypeValidateAlphaType</a> <a href='SkBitmap_Reference#SkBitmap_isOpaque'>SkBitmap::isOpaque</a> <a href='SkImage_Reference#SkImage_isOpaque'>SkImage::isOpaque</a> <a href='SkPixmap_Reference#SkPixmap_isOpaque'>SkPixmap::isOpaque</a>

<a name='SkImageInfo_dimensions'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkISize'>SkISize</a> <a href='#SkImageInfo_dimensions'>dimensions</a>() const
</pre>

Returns <a href='undocumented#SkISize'>SkISize</a> { <a href='#SkImageInfo_width'>width()</a>, <a href='#SkImageInfo_height'>height()</a> }.

### Return Value

integral <a href='undocumented#Size'>size</a> of <a href='#SkImageInfo_width'>width()</a> and <a href='#SkImageInfo_height'>height()</a>

### Example

<div><fiddle-embed name="d5547cd2b302822aa85b7b0ae3f48458">

#### Example Output

~~~~
dimensionsAsBounds == bounds
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkImageInfo_width'>width</a> <a href='#SkImageInfo_height'>height</a> <a href='#SkImageInfo_bounds'>bounds</a> <a href='SkBitmap_Reference#SkBitmap_dimensions'>SkBitmap::dimensions</a>

<a name='SkImageInfo_bounds'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkImageInfo_bounds'>bounds</a>() const
</pre>

Returns <a href='SkIRect_Reference#SkIRect'>SkIRect</a> { 0, 0, <a href='#SkImageInfo_width'>width()</a>, <a href='#SkImageInfo_height'>height()</a> }.

### Return Value

integral rectangle from origin to <a href='#SkImageInfo_width'>width()</a> and <a href='#SkImageInfo_height'>height()</a>

### Example

<div><fiddle-embed name="a818be8945cd0c18f99ffe53e90afa48"></fiddle-embed></div>

### See Also

<a href='#SkImageInfo_width'>width</a> <a href='#SkImageInfo_height'>height</a> <a href='#SkImageInfo_dimensions'>dimensions</a>

<a name='SkImageInfo_gammaCloseToSRGB'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkImageInfo_gammaCloseToSRGB'>gammaCloseToSRGB</a>() const
</pre>

Returns true if associated <a href='undocumented#Color_Space'>Color Space</a> is not nullptr, and <a href='undocumented#Color_Space'>Color Space</a> gamma
is approximately the same as sRGB.
This includes the <a href='https://en.wikipedia.org/wiki/SRGB#The_sRGB_transfer_function_(%22gamma%22)'>sRGB transfer function</a></a> as well as a gamma curve described by a 2.2 exponent.

### Return Value

true if <a href='undocumented#Color_Space'>Color Space</a> gamma is approximately the same as sRGB

### Example

<div><fiddle-embed name="22df72732e898a11773fbfe07388a546"></fiddle-embed></div>

### See Also

<a href='undocumented#SkColorSpace_gammaCloseToSRGB'>SkColorSpace::gammaCloseToSRGB</a>

<a name='SkImageInfo_makeWH'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkImageInfo'>SkImageInfo</a> <a href='#SkImageInfo_makeWH'>makeWH</a>(int newWidth, int newHeight) const
</pre>

Creates <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> with the same <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='undocumented#SkColorSpace'>SkColorSpace</a>, and <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>,
with dimensions set to width and height.

### Parameters

<table>  <tr>    <td><a name='SkImageInfo_makeWH_newWidth'><code><strong>newWidth</strong></code></a></td>
    <td><a href='undocumented#Pixel'>pixel</a> column count; must be zero or greater</td>
  </tr>
  <tr>    <td><a name='SkImageInfo_makeWH_newHeight'><code><strong>newHeight</strong></code></a></td>
    <td><a href='undocumented#Pixel'>pixel</a> row count; must be zero or greater</td>
  </tr>
</table>

### Return Value

created <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>

### Example

<div><fiddle-embed name="cd203a3f9c5fb68272f21f302dd54fbc"></fiddle-embed></div>

### See Also

<a href='#SkImageInfo_Make'>Make</a> <a href='#SkImageInfo_makeAlphaType'>makeAlphaType</a> <a href='#SkImageInfo_makeColorSpace'>makeColorSpace</a> <a href='#SkImageInfo_makeColorType'>makeColorType</a>

<a name='SkImageInfo_makeAlphaType'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkImageInfo'>SkImageInfo</a> <a href='#SkImageInfo_makeAlphaType'>makeAlphaType</a>(<a href='#SkAlphaType'>SkAlphaType</a> newAlphaType) const
</pre>

Creates <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> with same <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='undocumented#SkColorSpace'>SkColorSpace</a>, width, and height,
with <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> set to <a href='#SkImageInfo_makeAlphaType_newAlphaType'>newAlphaType</a>.

Created <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> contains <a href='#SkImageInfo_makeAlphaType_newAlphaType'>newAlphaType</a> even if it is incompatible with
<a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, in which case <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> in <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> is ignored.

### Parameters

<table>  <tr>    <td><a name='SkImageInfo_makeAlphaType_newAlphaType'><code><strong>newAlphaType</strong></code></a></td>
    <td>one of:</td>
  </tr>
</table>

<a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>, <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>, <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>,
<a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>

### Return Value

created <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>

### Example

<div><fiddle-embed name="e72db006f1bea26feceaef8727ff9818"></fiddle-embed></div>

### See Also

<a href='#SkImageInfo_Make'>Make</a> <a href='#SkImageInfo_MakeA8'>MakeA8</a> <a href='#SkImageInfo_makeColorType'>makeColorType</a> <a href='#SkImageInfo_makeColorSpace'>makeColorSpace</a>

<a name='SkImageInfo_makeColorType'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkImageInfo'>SkImageInfo</a> <a href='#SkImageInfo_makeColorType'>makeColorType</a>(<a href='#SkColorType'>SkColorType</a> newColorType) const
</pre>

Creates <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> with same <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='undocumented#SkColorSpace'>SkColorSpace</a>, width, and height,
with <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> set to <a href='#SkImageInfo_makeColorType_newColorType'>newColorType</a>.

### Parameters

<table>  <tr>    <td><a name='SkImageInfo_makeColorType_newColorType'><code><strong>newColorType</strong></code></a></td>
    <td>one of:</td>
  </tr>
</table>

<a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>,
<a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>kRGB_888x_SkColorType</a>,
<a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_1010102_SkColorType'>kRGBA_1010102_SkColorType</a>,
<a href='SkImageInfo_Reference#kRGB_101010x_SkColorType'>kRGB_101010x_SkColorType</a>, <a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a>

### Return Value

created <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>

### Example

<div><fiddle-embed name="3ac267b08b12dc83c95f91d8dd5d70ee"></fiddle-embed></div>

### See Also

<a href='#SkImageInfo_Make'>Make</a> <a href='#SkImageInfo_makeAlphaType'>makeAlphaType</a> <a href='#SkImageInfo_makeColorSpace'>makeColorSpace</a>

<a name='SkImageInfo_makeColorSpace'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkImageInfo'>SkImageInfo</a> <a href='#SkImageInfo_makeColorSpace'>makeColorSpace</a>(<a href='undocumented#sk_sp'>sk sp</a>&lt;<a href='undocumented#SkColorSpace'>SkColorSpace</a>&gt; cs) const
</pre>

Creates <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> with same <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, width, and height,
with <a href='undocumented#SkColorSpace'>SkColorSpace</a> set to <a href='#SkImageInfo_makeColorSpace_cs'>cs</a>.

### Parameters

<table>  <tr>    <td><a name='SkImageInfo_makeColorSpace_cs'><code><strong>cs</strong></code></a></td>
    <td>range of colors; may be nullptr</td>
  </tr>
</table>

### Return Value

created <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>

### Example

<div><fiddle-embed name="fe3c5a755d3dde29bba058a583f18901"></fiddle-embed></div>

### See Also

<a href='#SkImageInfo_Make'>Make</a> <a href='#SkImageInfo_MakeS32'>MakeS32</a> <a href='#SkImageInfo_makeAlphaType'>makeAlphaType</a> <a href='#SkImageInfo_makeColorType'>makeColorType</a>

<a name='SkImageInfo_bytesPerPixel'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkImageInfo_bytesPerPixel'>bytesPerPixel</a>() const
</pre>

Returns number of bytes per <a href='undocumented#Pixel'>pixel</a> required by <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>.
Returns zero if <a href='#SkImageInfo_colorType'>colorType</a>( is <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>.

### Return Value

bytes in <a href='undocumented#Pixel'>pixel</a>

### Example

<div><fiddle-embed name="9b6de4a07b2316228e9340e5a3b82134"><a href='#kUnknown_SkColorType'>kUnknown_SkColorType</a>, <a href='#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, <a href='#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>,
<a href='#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a>, <a href='#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>, <a href='#kRGB_888x_SkColorType'>kRGB_888x_SkColorType</a>,
<a href='#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>, <a href='#kRGBA_1010102_SkColorType'>kRGBA_1010102_SkColorType</a>, <a href='#kRGB_101010x_SkColorType'>kRGB_101010x_SkColorType</a>,
<a href='#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a>

#### Example Output

~~~~
color: kUnknown_SkColorType      bytesPerPixel: 0
color: kAlpha_8_SkColorType      bytesPerPixel: 1
color: kRGB_565_SkColorType      bytesPerPixel: 2
color: kARGB_4444_SkColorType    bytesPerPixel: 2
color: kRGBA_8888_SkColorType    bytesPerPixel: 4
color: kRGB_888x_SkColorType     bytesPerPixel: 4
color: kBGRA_8888_SkColorType    bytesPerPixel: 4
color: kRGBA_1010102_SkColorType bytesPerPixel: 4
color: kRGB_101010x_SkColorType  bytesPerPixel: 4
color: kGray_8_SkColorType       bytesPerPixel: 1
color: kRGBA_F16_SkColorType     bytesPerPixel: 8
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkImageInfo_width'>width</a> <a href='#SkImageInfo_shiftPerPixel'>shiftPerPixel</a> <a href='SkBitmap_Reference#SkBitmap_bytesPerPixel'>SkBitmap::bytesPerPixel</a>

<a name='SkImageInfo_shiftPerPixel'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkImageInfo_shiftPerPixel'>shiftPerPixel</a>() const
</pre>

Returns bit shift converting row bytes to row pixels.
Returns zero for <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>.

### Return Value

one of: 0, 1, 2, 3; left shift to convert pixels to bytes

### Example

<div><fiddle-embed name="e47b911f94fc629f756a829e523a2a89"><a href='#kUnknown_SkColorType'>kUnknown_SkColorType</a>, <a href='#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, <a href='#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>,
<a href='#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a>, <a href='#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>, <a href='#kRGB_888x_SkColorType'>kRGB_888x_SkColorType</a>,
<a href='#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>, <a href='#kRGBA_1010102_SkColorType'>kRGBA_1010102_SkColorType</a>, <a href='#kRGB_101010x_SkColorType'>kRGB_101010x_SkColorType</a>,
<a href='#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a>

#### Example Output

~~~~
color: kUnknown_SkColorType       shiftPerPixel: 0
color: kAlpha_8_SkColorType       shiftPerPixel: 0
color: kRGB_565_SkColorType       shiftPerPixel: 1
color: kARGB_4444_SkColorType     shiftPerPixel: 1
color: kRGBA_8888_SkColorType     shiftPerPixel: 2
color: kRGB_888x_SkColorType      shiftPerPixel: 2
color: kBGRA_8888_SkColorType     shiftPerPixel: 2
color: kRGBA_1010102_SkColorType  shiftPerPixel: 2
color: kRGB_101010x_SkColorType   shiftPerPixel: 2
color: kGray_8_SkColorType        shiftPerPixel: 0
color: kRGBA_F16_SkColorType      shiftPerPixel: 3
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkImageInfo_bytesPerPixel'>bytesPerPixel</a> <a href='#SkImageInfo_minRowBytes'>minRowBytes</a> <a href='SkBitmap_Reference#SkBitmap_shiftPerPixel'>SkBitmap::shiftPerPixel</a> <a href='SkPixmap_Reference#SkPixmap_shiftPerPixel'>SkPixmap::shiftPerPixel</a>

<a name='SkImageInfo_minRowBytes64'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
uint64_t <a href='#SkImageInfo_minRowBytes64'>minRowBytes64</a>() const
</pre>

Returns minimum bytes per row, computed from <a href='undocumented#Pixel'>pixel</a> <a href='#SkImageInfo_width'>width()</a> and <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, which
specifies <a href='#SkImageInfo_bytesPerPixel'>bytesPerPixel</a>(). <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> maximum value for row bytes must fit
in 31 bits.

### Return Value

<a href='#SkImageInfo_width'>width()</a> times <a href='#SkImageInfo_bytesPerPixel'>bytesPerPixel</a>() as unsigned 64-bit integer

### Example

<div><fiddle-embed name="4b5d3904476726a39f1c3e276d6b6ba7">

#### Example Output

~~~~
RGBA_F16 width 16777216 (0x01000000) OK
RGBA_F16 width 33554432 (0x02000000) OK
RGBA_F16 width 67108864 (0x04000000) OK
RGBA_F16 width 134217728 (0x08000000) OK
RGBA_F16 width 268435456 (0x10000000) too large
RGBA_F16 width 536870912 (0x20000000) too large
RGBA_F16 width 1073741824 (0x40000000) too large
RGBA_F16 width -2147483648 (0x80000000) too large
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkImageInfo_minRowBytes'>minRowBytes</a> <a href='#SkImageInfo_computeByteSize'>computeByteSize</a> <a href='#SkImageInfo_computeMinByteSize'>computeMinByteSize</a> <a href='#SkImageInfo_validRowBytes'>validRowBytes</a>

<a name='SkImageInfo_minRowBytes'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
size_t <a href='#SkImageInfo_minRowBytes'>minRowBytes</a>() const
</pre>

Returns minimum bytes per row, computed from <a href='undocumented#Pixel'>pixel</a> <a href='#SkImageInfo_width'>width()</a> and <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, which
specifies <a href='#SkImageInfo_bytesPerPixel'>bytesPerPixel</a>(). <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> maximum value for row bytes must fit
in 31 bits.

### Return Value

<a href='#SkImageInfo_width'>width()</a> times <a href='#SkImageInfo_bytesPerPixel'>bytesPerPixel</a>() as signed 32-bit integer

### Example

<div><fiddle-embed name="897230ecfb36095486beca324fd369f9">

#### Example Output

~~~~
RGBA_F16 width 16777216 (0x01000000) OK
RGBA_F16 width 33554432 (0x02000000) OK
RGBA_F16 width 67108864 (0x04000000) OK
RGBA_F16 width 134217728 (0x08000000) OK
RGBA_F16 width 268435456 (0x10000000) too large
RGBA_F16 width 536870912 (0x20000000) too large
RGBA_F16 width 1073741824 (0x40000000) too large
RGBA_F16 width -2147483648 (0x80000000) too large
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkImageInfo_minRowBytes64'>minRowBytes64</a> <a href='#SkImageInfo_computeByteSize'>computeByteSize</a> <a href='#SkImageInfo_computeMinByteSize'>computeMinByteSize</a> <a href='#SkImageInfo_validRowBytes'>validRowBytes</a>

<a name='SkImageInfo_computeOffset'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
size_t <a href='#SkImageInfo_computeOffset'>computeOffset</a>(int x, int y, size_t rowBytes) const
</pre>

Returns byte offset of <a href='undocumented#Pixel'>pixel</a> from <a href='undocumented#Pixel'>pixel</a> base address.

Asserts in debug build if <a href='#SkImageInfo_computeOffset_x'>x</a> or <a href='#SkImageInfo_computeOffset_y'>y</a> is outside of bounds. Does not assert if
<a href='#SkImageInfo_computeOffset_rowBytes'>rowBytes</a> is smaller than <a href='#SkImageInfo_minRowBytes'>minRowBytes</a>(), even though result may be incorrect.

### Parameters

<table>  <tr>    <td><a name='SkImageInfo_computeOffset_x'><code><strong>x</strong></code></a></td>
    <td>column index, zero or greater, and less than <a href='#SkImageInfo_width'>width()</a></td>
  </tr>
  <tr>    <td><a name='SkImageInfo_computeOffset_y'><code><strong>y</strong></code></a></td>
    <td>row index, zero or greater, and less than <a href='#SkImageInfo_height'>height()</a></td>
  </tr>
  <tr>    <td><a name='SkImageInfo_computeOffset_rowBytes'><code><strong>rowBytes</strong></code></a></td>
    <td><a href='undocumented#Size'>size</a> of <a href='undocumented#Pixel'>pixel</a> row or larger</td>
  </tr>
</table>

### Return Value

offset within <a href='undocumented#Pixel'>pixel</a> array

### Example

<div><fiddle-embed name="818e4e1191e39d2a642902cbf253b399"></fiddle-embed></div>

### See Also

<a href='#SkImageInfo_height'>height</a> <a href='#SkImageInfo_width'>width</a> <a href='#SkImageInfo_minRowBytes'>minRowBytes</a> <a href='#SkImageInfo_computeByteSize'>computeByteSize</a>

<a name='SkImageInfo_equal1_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool operator==(const SkImageInfo& other) const
</pre>

Compares <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> with <a href='#SkImageInfo_operator==(const SkImageInfo& other)_const_other'>other</a>, and returns true if width, height, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>,
<a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, and <a href='undocumented#SkColorSpace'>SkColorSpace</a> are equivalent.

### Parameters

<table>  <tr>    <td><a name='SkImageInfo_equal1_operator_other'><code><strong>other</strong></code></a></td>
    <td><a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> to compare</td>
  </tr>
</table>

### Return Value

true if <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> equals <a href='#SkImageInfo_operator==(const SkImageInfo& other)_const_other'>other</a>

### Example

<div><fiddle-embed name="53c212c4f2449df0b0eedbc6227b6ab7">

#### Example Output

~~~~
info1 != info2
info1 != info2
info1 != info2
info1 == info2
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkImageInfo_notequal1_operator'>operator!=(const SkImageInfo& other) const</a> <a href='undocumented#SkColorSpace_Equals'>SkColorSpace::Equals</a>

<a name='SkImageInfo_notequal1_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool operator!=(const SkImageInfo& other) const
</pre>

Compares <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> with <a href='#SkImageInfo_operator!=(const SkImageInfo& other)_const_other'>other</a>, and returns true if width, height, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>,
<a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, and <a href='undocumented#SkColorSpace'>SkColorSpace</a> are not equivalent.

### Parameters

<table>  <tr>    <td><a name='SkImageInfo_notequal1_operator_other'><code><strong>other</strong></code></a></td>
    <td><a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> to compare</td>
  </tr>
</table>

### Return Value

true if <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> is not equal to <a href='#SkImageInfo_operator!=(const SkImageInfo& other)_const_other'>other</a>

### Example

<div><fiddle-embed name="8c039fde0a476ac1aa62bf9de5d61c77">

#### Example Output

~~~~
info1 != info2
info1 != info2
info1 != info2
info1 == info2
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkImageInfo_equal1_operator'>operator==(const SkImageInfo& other) const</a> <a href='undocumented#SkColorSpace_Equals'>SkColorSpace::Equals</a>

<a name='SkImageInfo_computeByteSize'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
size_t <a href='#SkImageInfo_computeByteSize'>computeByteSize</a>(size_t rowBytes) const
</pre>

Returns storage required by <a href='undocumented#Pixel'>pixel</a> array, given <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> dimensions, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>,
and <a href='#SkImageInfo_computeByteSize_rowBytes'>rowBytes</a>. <a href='#SkImageInfo_computeByteSize_rowBytes'>rowBytes</a> is assumed to be at least as large as <a href='#SkImageInfo_minRowBytes'>minRowBytes</a>().

Returns zero if height is zero.
Returns SIZE_MAX if answer exceeds the range of size_t.

### Parameters

<table>  <tr>    <td><a name='SkImageInfo_computeByteSize_rowBytes'><code><strong>rowBytes</strong></code></a></td>
    <td><a href='undocumented#Size'>size</a> of <a href='undocumented#Pixel'>pixel</a> row or larger</td>
  </tr>
</table>

### Return Value

memory required by <a href='undocumented#Pixel'>pixel</a> buffer

### Example

<div><fiddle-embed name="9def507d2295f7051effd0c83bb04436"></fiddle-embed></div>

### See Also

<a href='#SkImageInfo_computeMinByteSize'>computeMinByteSize</a> <a href='#SkImageInfo_validRowBytes'>validRowBytes</a>

<a name='SkImageInfo_computeMinByteSize'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
size_t <a href='#SkImageInfo_computeMinByteSize'>computeMinByteSize</a>() const
</pre>

Returns storage required by <a href='undocumented#Pixel'>pixel</a> array, given <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> dimensions, and
<a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>. Uses <a href='#SkImageInfo_minRowBytes'>minRowBytes</a>() to compute bytes for <a href='undocumented#Pixel'>pixel</a> row.

Returns zero if height is zero.
Returns SIZE_MAX if answer exceeds the range of size_t.

### Return Value

least memory required by <a href='undocumented#Pixel'>pixel</a> buffer

### Example

<div><fiddle-embed name="fc18640fdde437cb35338aed7c68d399"></fiddle-embed></div>

### See Also

<a href='#SkImageInfo_computeByteSize'>computeByteSize</a> <a href='#SkImageInfo_validRowBytes'>validRowBytes</a>

<a name='SkImageInfo_ByteSizeOverflowed'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static bool <a href='#SkImageInfo_ByteSizeOverflowed'>ByteSizeOverflowed</a>(size_t byteSize)
</pre>

Returns true if <a href='#SkImageInfo_ByteSizeOverflowed_byteSize'>byteSize</a> equals SIZE_MAX. <a href='#SkImageInfo_computeByteSize'>computeByteSize</a>() and
<a href='#SkImageInfo_computeMinByteSize'>computeMinByteSize</a>() return SIZE_MAX if size_t can not hold buffer <a href='undocumented#Size'>size</a>.

### Parameters

<table>  <tr>    <td><a name='SkImageInfo_ByteSizeOverflowed_byteSize'><code><strong>byteSize</strong></code></a></td>
    <td>result of <a href='#SkImageInfo_computeByteSize'>computeByteSize</a>() or <a href='#SkImageInfo_computeMinByteSize'>computeMinByteSize</a>()</td>
  </tr>
</table>

### Return Value

true if <a href='#SkImageInfo_computeByteSize'>computeByteSize</a>() or <a href='#SkImageInfo_computeMinByteSize'>computeMinByteSize</a>() result exceeds size_t

### Example

<div><fiddle-embed name="6a63dfdd62ab77ff57783af8c33d7b78">

#### Example Output

~~~~
rowBytes:100000000 size:99999999900000008 overflowed:false
rowBytes:1000000000 size:999999999000000008 overflowed:false
rowBytes:10000000000 size:9999999990000000008 overflowed:false
rowBytes:100000000000 size:18446744073709551615 overflowed:true
rowBytes:1000000000000 size:18446744073709551615 overflowed:true
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkImageInfo_computeByteSize'>computeByteSize</a> <a href='#SkImageInfo_computeMinByteSize'>computeMinByteSize</a> <a href='#SkImageInfo_validRowBytes'>validRowBytes</a>

<a name='SkImageInfo_validRowBytes'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkImageInfo_validRowBytes'>validRowBytes</a>(size_t rowBytes) const
</pre>

Returns true if <a href='#SkImageInfo_validRowBytes_rowBytes'>rowBytes</a> is smaller than width times <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Size'>size</a>.

### Parameters

<table>  <tr>    <td><a name='SkImageInfo_validRowBytes_rowBytes'><code><strong>rowBytes</strong></code></a></td>
    <td><a href='undocumented#Size'>size</a> of <a href='undocumented#Pixel'>pixel</a> row or larger</td>
  </tr>
</table>

### Return Value

true if <a href='#SkImageInfo_validRowBytes_rowBytes'>rowBytes</a> is large enough to contain <a href='undocumented#Pixel'>pixel</a> row

### Example

<div><fiddle-embed name="c6b0f6a3f493cb08d9abcdefe12de245">

#### Example Output

~~~~
validRowBytes(60): false
validRowBytes(64): true
validRowBytes(68): true
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkImageInfo_ByteSizeOverflowed'>ByteSizeOverflowed</a> <a href='#SkImageInfo_computeByteSize'>computeByteSize</a> <a href='#SkImageInfo_computeMinByteSize'>computeMinByteSize</a>

<a name='SkImageInfo_reset'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkImageInfo_reset'>reset</a>()
</pre>

Creates an empty <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> with <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>,
a width and height of zero, and no <a href='undocumented#SkColorSpace'>SkColorSpace</a>.

### Example

<div><fiddle-embed name="ab7e73786805c936de386b6c1ebe1f13">

#### Example Output

~~~~
info == copy
info != reset copy
SkImageInfo() == reset copy
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkImageInfo_empty_constructor'>SkImageInfo()</a>

<a name='Utility'></a>

<a name='SkImageInfo_validate'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkImageInfo_validate'>validate</a>() const
</pre>

### See Also

<a href='#SkImageInfo_validRowBytes'>validRowBytes</a> <a href='SkBitmap_Reference#SkBitmap_validate'>SkBitmap::validate</a>

