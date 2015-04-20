/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "PathOpsExtendedTest.h"
#include "PathOpsTestCommon.h"

#define TEST(name) { name, #name }

static SkPath path1() {
    SkPath path;
path.moveTo(SkBits2Float(0x431d8000), SkBits2Float(0x42823333));  // 157.5f, 65.1f
path.lineTo(SkBits2Float(0x431d8000), SkBits2Float(0x42823333));  // 157.5f, 65.1f
path.cubicTo(SkBits2Float(0x431e3333), SkBits2Float(0x42823333), SkBits2Float(0x431ee666), SkBits2Float(0x4282368d), SkBits2Float(0x431f999a), SkBits2Float(0x42823333));  // 158.2f, 65.1f, 158.9f, 65.1065f, 159.6f, 65.1f
path.cubicTo(SkBits2Float(0x43204ccd), SkBits2Float(0x42822fd9), SkBits2Float(0x43210000), SkBits2Float(0x42822861), SkBits2Float(0x4321b333), SkBits2Float(0x42821f17));  // 160.3f, 65.0935f, 161, 65.0789f, 161.7f, 65.0607f
path.cubicTo(SkBits2Float(0x43226666), SkBits2Float(0x428215ce), SkBits2Float(0x4323199a), SkBits2Float(0x4282071d), SkBits2Float(0x4323cccd), SkBits2Float(0x4281fb7b));  // 162.4f, 65.0426f, 163.1f, 65.0139f, 163.8f, 64.9912f
path.cubicTo(SkBits2Float(0x43248000), SkBits2Float(0x4281efd8), SkBits2Float(0x43253333), SkBits2Float(0x4281e467), SkBits2Float(0x4325e666), SkBits2Float(0x4281d94a));  // 164.5f, 64.9684f, 165.2f, 64.9461f, 165.9f, 64.9244f
path.cubicTo(SkBits2Float(0x4326999a), SkBits2Float(0x4281ce2c), SkBits2Float(0x43274ccd), SkBits2Float(0x4281c15d), SkBits2Float(0x43280000), SkBits2Float(0x4281b8cb));  // 166.6f, 64.9027f, 167.3f, 64.8777f, 168, 64.8609f
path.cubicTo(SkBits2Float(0x4328b333), SkBits2Float(0x4281b039), SkBits2Float(0x43296666), SkBits2Float(0x4281a66d), SkBits2Float(0x432a199a), SkBits2Float(0x4281a5dd));  // 168.7f, 64.8442f, 169.4f, 64.8251f, 170.1f, 64.824f
path.cubicTo(SkBits2Float(0x432acccd), SkBits2Float(0x4281a54c), SkBits2Float(0x432b8000), SkBits2Float(0x4281aecf), SkBits2Float(0x432c3333), SkBits2Float(0x4281b566));  // 170.8f, 64.8228f, 171.5f, 64.8414f, 172.2f, 64.8543f
path.cubicTo(SkBits2Float(0x432ce666), SkBits2Float(0x4281bbfe), SkBits2Float(0x432d999a), SkBits2Float(0x4281c612), SkBits2Float(0x432e4ccd), SkBits2Float(0x4281cd6b));  // 172.9f, 64.8672f, 173.6f, 64.8869f, 174.3f, 64.9012f
path.cubicTo(SkBits2Float(0x432f0000), SkBits2Float(0x4281d4c4), SkBits2Float(0x432fb333), SkBits2Float(0x4281dc73), SkBits2Float(0x43306666), SkBits2Float(0x4281e17e));  // 175, 64.9156f, 175.7f, 64.9306f, 176.4f, 64.9404f
path.cubicTo(SkBits2Float(0x4331199a), SkBits2Float(0x4281e688), SkBits2Float(0x4331cccd), SkBits2Float(0x4281e967), SkBits2Float(0x43328000), SkBits2Float(0x4281ebaa));  // 177.1f, 64.9503f, 177.8f, 64.9559f, 178.5f, 64.9603f
path.cubicTo(SkBits2Float(0x43333333), SkBits2Float(0x4281eded), SkBits2Float(0x4333e666), SkBits2Float(0x4281eec6), SkBits2Float(0x4334999a), SkBits2Float(0x4281ef0f));  // 179.2f, 64.9647f, 179.9f, 64.9664f, 180.6f, 64.9669f
path.cubicTo(SkBits2Float(0x43354ccd), SkBits2Float(0x4281ef57), SkBits2Float(0x43360000), SkBits2Float(0x4281eeba), SkBits2Float(0x4336b333), SkBits2Float(0x4281ed5c));  // 181.3f, 64.9675f, 182, 64.9663f, 182.7f, 64.9636f
path.cubicTo(SkBits2Float(0x43376666), SkBits2Float(0x4281ebfe), SkBits2Float(0x4338199a), SkBits2Float(0x4281e8c9), SkBits2Float(0x4338cccd), SkBits2Float(0x4281e6db));  // 183.4f, 64.9609f, 184.1f, 64.9547f, 184.8f, 64.9509f
path.cubicTo(SkBits2Float(0x43398000), SkBits2Float(0x4281e4ec), SkBits2Float(0x433a3333), SkBits2Float(0x4281e29d), SkBits2Float(0x433ae666), SkBits2Float(0x4281e1c4));  // 185.5f, 64.9471f, 186.2f, 64.9426f, 186.9f, 64.9409f
path.cubicTo(SkBits2Float(0x433b999a), SkBits2Float(0x4281e0eb), SkBits2Float(0x433c4ccd), SkBits2Float(0x4281e188), SkBits2Float(0x433d0000), SkBits2Float(0x4281e1c4));  // 187.6f, 64.9393f, 188.3f, 64.9405f, 189, 64.9409f
path.cubicTo(SkBits2Float(0x433db333), SkBits2Float(0x4281e201), SkBits2Float(0x433e6666), SkBits2Float(0x4281e415), SkBits2Float(0x433f199a), SkBits2Float(0x4281e330));  // 189.7f, 64.9414f, 190.4f, 64.9455f, 191.1f, 64.9437f
path.cubicTo(SkBits2Float(0x433fcccd), SkBits2Float(0x4281e24b), SkBits2Float(0x43408000), SkBits2Float(0x4281df77), SkBits2Float(0x43413333), SkBits2Float(0x4281dc67));  // 191.8f, 64.942f, 192.5f, 64.9365f, 193.2f, 64.9305f
path.cubicTo(SkBits2Float(0x4341e666), SkBits2Float(0x4281d957), SkBits2Float(0x4342999a), SkBits2Float(0x4281d35a), SkBits2Float(0x43434ccd), SkBits2Float(0x4281d0cf));  // 193.9f, 64.9245f, 194.6f, 64.9128f, 195.3f, 64.9078f
path.cubicTo(SkBits2Float(0x43440000), SkBits2Float(0x4281ce44), SkBits2Float(0x4344b333), SkBits2Float(0x4281cd6c), SkBits2Float(0x43456666), SkBits2Float(0x4281cd24));  // 196, 64.9029f, 196.7f, 64.9012f, 197.4f, 64.9007f
path.cubicTo(SkBits2Float(0x4346199a), SkBits2Float(0x4281ccdc), SkBits2Float(0x4346cccd), SkBits2Float(0x4281cf1d), SkBits2Float(0x43478000), SkBits2Float(0x4281cf1d));  // 198.1f, 64.9001f, 198.8f, 64.9045f, 199.5f, 64.9045f
path.cubicTo(SkBits2Float(0x43483333), SkBits2Float(0x4281cf1d), SkBits2Float(0x4348e666), SkBits2Float(0x4281ce8e), SkBits2Float(0x4349999a), SkBits2Float(0x4281cd24));  // 200.2f, 64.9045f, 200.9f, 64.9034f, 201.6f, 64.9007f
path.cubicTo(SkBits2Float(0x434a4ccd), SkBits2Float(0x4281cbba), SkBits2Float(0x434b0000), SkBits2Float(0x4281c854), SkBits2Float(0x434bb333), SkBits2Float(0x4281c6a2));  // 202.3f, 64.8979f, 203, 64.8913f, 203.7f, 64.888f
path.cubicTo(SkBits2Float(0x434c6666), SkBits2Float(0x4281c4f0), SkBits2Float(0x434d199a), SkBits2Float(0x4281c46d), SkBits2Float(0x434dcccd), SkBits2Float(0x4281c2f7));  // 204.4f, 64.8846f, 205.1f, 64.8836f, 205.8f, 64.8808f
path.cubicTo(SkBits2Float(0x434e8000), SkBits2Float(0x4281c182), SkBits2Float(0x434f3333), SkBits2Float(0x4281bf4b), SkBits2Float(0x434fe666), SkBits2Float(0x4281bde1));  // 206.5f, 64.8779f, 207.2f, 64.8736f, 207.9f, 64.8709f
path.cubicTo(SkBits2Float(0x4350999a), SkBits2Float(0x4281bc77), SkBits2Float(0x43514ccd), SkBits2Float(0x4281bb92), SkBits2Float(0x43520000), SkBits2Float(0x4281ba7d));  // 208.6f, 64.8681f, 209.3f, 64.8663f, 210, 64.8642f
path.cubicTo(SkBits2Float(0x4352b333), SkBits2Float(0x4281b967), SkBits2Float(0x43536666), SkBits2Float(0x4281b95a), SkBits2Float(0x4354199a), SkBits2Float(0x4281b75f));  // 210.7f, 64.8621f, 211.4f, 64.862f, 212.1f, 64.8581f
path.cubicTo(SkBits2Float(0x4354cccd), SkBits2Float(0x4281b565), SkBits2Float(0x43558000), SkBits2Float(0x4281b0a4), SkBits2Float(0x43563333), SkBits2Float(0x4281ae9e));  // 212.8f, 64.8543f, 213.5f, 64.845f, 214.2f, 64.841f
path.cubicTo(SkBits2Float(0x4356e666), SkBits2Float(0x4281ac98), SkBits2Float(0x4357999a), SkBits2Float(0x4281aca3), SkBits2Float(0x43584ccd), SkBits2Float(0x4281ab3a));  // 214.9f, 64.8371f, 215.6f, 64.8372f, 216.3f, 64.8344f
path.cubicTo(SkBits2Float(0x43590000), SkBits2Float(0x4281a9d0), SkBits2Float(0x4359b333), SkBits2Float(0x4281a82a), SkBits2Float(0x435a6666), SkBits2Float(0x4281a623));  // 217, 64.8317f, 217.7f, 64.8284f, 218.4f, 64.8245f
path.cubicTo(SkBits2Float(0x435b199a), SkBits2Float(0x4281a41d), SkBits2Float(0x435bcccd), SkBits2Float(0x4281a157), SkBits2Float(0x435c8000), SkBits2Float(0x42819f14));  // 219.1f, 64.8205f, 219.8f, 64.8151f, 220.5f, 64.8107f
path.cubicTo(SkBits2Float(0x435d3333), SkBits2Float(0x42819cd1), SkBits2Float(0x435de666), SkBits2Float(0x42819a39), SkBits2Float(0x435e999a), SkBits2Float(0x42819892));  // 221.2f, 64.8063f, 221.9f, 64.8012f, 222.6f, 64.798f
path.cubicTo(SkBits2Float(0x435f4ccd), SkBits2Float(0x428196ec), SkBits2Float(0x43600000), SkBits2Float(0x42819455), SkBits2Float(0x4360b333), SkBits2Float(0x4281952e));  // 223.3f, 64.7948f, 224, 64.7897f, 224.7f, 64.7914f
path.cubicTo(SkBits2Float(0x43616666), SkBits2Float(0x42819607), SkBits2Float(0x4362199a), SkBits2Float(0x428198e7), SkBits2Float(0x4362cccd), SkBits2Float(0x42819da9));  // 225.4f, 64.793f, 226.1f, 64.7986f, 226.8f, 64.8079f
path.cubicTo(SkBits2Float(0x43638000), SkBits2Float(0x4281a26b), SkBits2Float(0x43643333), SkBits2Float(0x4281ad8a), SkBits2Float(0x4364e666), SkBits2Float(0x4281b1bc));  // 227.5f, 64.8172f, 228.2f, 64.8389f, 228.9f, 64.8471f
path.cubicTo(SkBits2Float(0x4365999a), SkBits2Float(0x4281b5ed), SkBits2Float(0x43664ccd), SkBits2Float(0x4281b70f), SkBits2Float(0x43670000), SkBits2Float(0x4281b6d2));  // 229.6f, 64.8553f, 230.3f, 64.8575f, 231, 64.8571f
path.cubicTo(SkBits2Float(0x4367b333), SkBits2Float(0x4281b695), SkBits2Float(0x43686666), SkBits2Float(0x4281b2db), SkBits2Float(0x4369199a), SkBits2Float(0x4281b050));  // 231.7f, 64.8566f, 232.4f, 64.8493f, 233.1f, 64.8444f
path.cubicTo(SkBits2Float(0x4369cccd), SkBits2Float(0x4281adc5), SkBits2Float(0x436a8000), SkBits2Float(0x4281a9e9), SkBits2Float(0x436b3333), SkBits2Float(0x4281a78f));  // 233.8f, 64.8394f, 234.5f, 64.8319f, 235.2f, 64.8273f
path.cubicTo(SkBits2Float(0x436be666), SkBits2Float(0x4281a535), SkBits2Float(0x436c999a), SkBits2Float(0x4281a55a), SkBits2Float(0x436d4ccd), SkBits2Float(0x4281a232));  // 235.9f, 64.8227f, 236.6f, 64.823f, 237.3f, 64.8168f
path.cubicTo(SkBits2Float(0x436e0000), SkBits2Float(0x42819f0a), SkBits2Float(0x436eb333), SkBits2Float(0x42819ad9), SkBits2Float(0x436f6666), SkBits2Float(0x428194a1));  // 238, 64.8106f, 238.7f, 64.8024f, 239.4f, 64.7903f
path.cubicTo(SkBits2Float(0x4370199a), SkBits2Float(0x42818e69), SkBits2Float(0x4370cccd), SkBits2Float(0x4281843c), SkBits2Float(0x43718000), SkBits2Float(0x42817ce3));  // 240.1f, 64.7781f, 240.8f, 64.7583f, 241.5f, 64.7439f
path.cubicTo(SkBits2Float(0x43723333), SkBits2Float(0x4281758a), SkBits2Float(0x4372e666), SkBits2Float(0x42816c36), SkBits2Float(0x4373999a), SkBits2Float(0x4281688a));  // 242.2f, 64.7296f, 242.9f, 64.7113f, 243.6f, 64.7042f
path.cubicTo(SkBits2Float(0x43744ccd), SkBits2Float(0x428164dd), SkBits2Float(0x43750000), SkBits2Float(0x428167a5), SkBits2Float(0x4375b333), SkBits2Float(0x428166d8));  // 244.3f, 64.697f, 245, 64.7024f, 245.7f, 64.7009f
path.cubicTo(SkBits2Float(0x43766666), SkBits2Float(0x4281660a), SkBits2Float(0x4377199a), SkBits2Float(0x42816651), SkBits2Float(0x4377cccd), SkBits2Float(0x428163ba));  // 246.4f, 64.6993f, 247.1f, 64.6998f, 247.8f, 64.6948f
path.cubicTo(SkBits2Float(0x43788000), SkBits2Float(0x42816123), SkBits2Float(0x43793333), SkBits2Float(0x42815b5b), SkBits2Float(0x4379e666), SkBits2Float(0x4281574e));  // 248.5f, 64.6897f, 249.2f, 64.6784f, 249.9f, 64.6705f
path.cubicTo(SkBits2Float(0x437a999a), SkBits2Float(0x42815342), SkBits2Float(0x437b4ccd), SkBits2Float(0x42814fad), SkBits2Float(0x437c0000), SkBits2Float(0x42814b6f));  // 250.6f, 64.6626f, 251.3f, 64.6556f, 252, 64.6473f
path.cubicTo(SkBits2Float(0x437cb333), SkBits2Float(0x42814732), SkBits2Float(0x437d6666), SkBits2Float(0x42813eb7), SkBits2Float(0x437e199a), SkBits2Float(0x42813dde));  // 252.7f, 64.6391f, 253.4f, 64.6225f, 254.1f, 64.6208f
path.cubicTo(SkBits2Float(0x437ecccd), SkBits2Float(0x42813d05), SkBits2Float(0x437f8000), SkBits2Float(0x428137d7), SkBits2Float(0x4380199a), SkBits2Float(0x42814659));  // 254.8f, 64.6192f, 255.5f, 64.6091f, 256.2f, 64.6374f
path.cubicTo(SkBits2Float(0x43807333), SkBits2Float(0x428154da), SkBits2Float(0x4380cccd), SkBits2Float(0x42817565), SkBits2Float(0x43812666), SkBits2Float(0x428194e8));  // 256.9f, 64.6657f, 257.6f, 64.7293f, 258.3f, 64.7908f
path.cubicTo(SkBits2Float(0x43818000), SkBits2Float(0x4281b46a), SkBits2Float(0x4381d99a), SkBits2Float(0x4281e906), SkBits2Float(0x43823333), SkBits2Float(0x42820368));  // 259, 64.8524f, 259.7f, 64.9551f, 260.4f, 65.0067f
path.cubicTo(SkBits2Float(0x43828ccd), SkBits2Float(0x42821dca), SkBits2Float(0x4382e666), SkBits2Float(0x42822b3c), SkBits2Float(0x43834000), SkBits2Float(0x42823333));  // 261.1f, 65.0582f, 261.8f, 65.0844f, 262.5f, 65.1f
path.cubicTo(SkBits2Float(0x4383999a), SkBits2Float(0x42823b2a), SkBits2Float(0x4383f333), SkBits2Float(0x42823333), SkBits2Float(0x43844ccd), SkBits2Float(0x42823333));  // 263.2f, 65.1156f, 263.9f, 65.1f, 264.6f, 65.1f
path.lineTo(SkBits2Float(0x43844ccd), SkBits2Float(0x42823333));  // 264.6f, 65.1f
path.lineTo(SkBits2Float(0x431d8000), SkBits2Float(0x42823333));  // 157.5f, 65.1f
path.close();
path.moveTo(SkBits2Float(0x438dc000), SkBits2Float(0x42823333));  // 283.5f, 65.1f
path.lineTo(SkBits2Float(0x438dc000), SkBits2Float(0x42823333));  // 283.5f, 65.1f
path.cubicTo(SkBits2Float(0x438e199a), SkBits2Float(0x428230fb), SkBits2Float(0x438e7333), SkBits2Float(0x4282293a), SkBits2Float(0x438ecccd), SkBits2Float(0x428225e0));  // 284.2f, 65.0957f, 284.9f, 65.0805f, 285.6f, 65.074f
path.cubicTo(SkBits2Float(0x438f2666), SkBits2Float(0x42822286), SkBits2Float(0x438f8000), SkBits2Float(0x42821cde), SkBits2Float(0x438fd99a), SkBits2Float(0x42821f17));  // 286.3f, 65.0674f, 287, 65.0564f, 287.7f, 65.0607f
path.cubicTo(SkBits2Float(0x43903333), SkBits2Float(0x42822150), SkBits2Float(0x43908ccd), SkBits2Float(0x42822fd9), SkBits2Float(0x4390e666), SkBits2Float(0x42823333));  // 288.4f, 65.0651f, 289.1f, 65.0935f, 289.8f, 65.1f
path.lineTo(SkBits2Float(0x4390e666), SkBits2Float(0x42823333));  // 289.8f, 65.1f
path.lineTo(SkBits2Float(0x438dc000), SkBits2Float(0x42823333));  // 283.5f, 65.1f
path.close();
path.moveTo(SkBits2Float(0x43994ccd), SkBits2Float(0x42823333));  // 306.6f, 65.1f
path.lineTo(SkBits2Float(0x43994ccd), SkBits2Float(0x42823333));  // 306.6f, 65.1f
path.cubicTo(SkBits2Float(0x4399a666), SkBits2Float(0x42823332), SkBits2Float(0x439a0000), SkBits2Float(0x42823842), SkBits2Float(0x439a599a), SkBits2Float(0x4282332a));  // 307.3f, 65.1f, 308, 65.1099f, 308.7f, 65.0999f
path.cubicTo(SkBits2Float(0x439ab333), SkBits2Float(0x42822e12), SkBits2Float(0x439b0ccd), SkBits2Float(0x42821e94), SkBits2Float(0x439b6666), SkBits2Float(0x428214a4));  // 309.4f, 65.09f, 310.1f, 65.0597f, 310.8f, 65.0403f
path.cubicTo(SkBits2Float(0x439bc000), SkBits2Float(0x42820ab4), SkBits2Float(0x439c199a), SkBits2Float(0x42820185), SkBits2Float(0x439c7333), SkBits2Float(0x4281f789));  // 311.5f, 65.0209f, 312.2f, 65.003f, 312.9f, 64.9835f
path.cubicTo(SkBits2Float(0x439ccccd), SkBits2Float(0x4281ed8d), SkBits2Float(0x439d2666), SkBits2Float(0x4281e391), SkBits2Float(0x439d8000), SkBits2Float(0x4281d8bc));  // 313.6f, 64.964f, 314.3f, 64.9445f, 315, 64.9233f
path.cubicTo(SkBits2Float(0x439dd99a), SkBits2Float(0x4281cde7), SkBits2Float(0x439e3333), SkBits2Float(0x4281c0c4), SkBits2Float(0x439e8ccd), SkBits2Float(0x4281b68b));  // 315.7f, 64.9022f, 316.4f, 64.8765f, 317.1f, 64.8565f
path.cubicTo(SkBits2Float(0x439ee666), SkBits2Float(0x4281ac53), SkBits2Float(0x439f4000), SkBits2Float(0x4281a27a), SkBits2Float(0x439f999a), SkBits2Float(0x42819b69));  // 317.8f, 64.8366f, 318.5f, 64.8173f, 319.2f, 64.8035f
path.cubicTo(SkBits2Float(0x439ff333), SkBits2Float(0x42819459), SkBits2Float(0x43a04ccd), SkBits2Float(0x42818f8b), SkBits2Float(0x43a0a666), SkBits2Float(0x42818c26));  // 319.9f, 64.7897f, 320.6f, 64.7804f, 321.3f, 64.7737f
path.cubicTo(SkBits2Float(0x43a10000), SkBits2Float(0x428188c2), SkBits2Float(0x43a1599a), SkBits2Float(0x42818795), SkBits2Float(0x43a1b333), SkBits2Float(0x42818710));  // 322, 64.7671f, 322.7f, 64.7648f, 323.4f, 64.7638f
path.cubicTo(SkBits2Float(0x43a20ccd), SkBits2Float(0x4281868b), SkBits2Float(0x43a26666), SkBits2Float(0x42818824), SkBits2Float(0x43a2c000), SkBits2Float(0x42818909));  // 324.1f, 64.7628f, 324.8f, 64.7659f, 325.5f, 64.7676f
path.cubicTo(SkBits2Float(0x43a3199a), SkBits2Float(0x428189ee), SkBits2Float(0x43a37333), SkBits2Float(0x42818de2), SkBits2Float(0x43a3cccd), SkBits2Float(0x42818c6d));  // 326.2f, 64.7694f, 326.9f, 64.7771f, 327.6f, 64.7743f
path.cubicTo(SkBits2Float(0x43a42666), SkBits2Float(0x42818af7), SkBits2Float(0x43a48000), SkBits2Float(0x428185be), SkBits2Float(0x43a4d99a), SkBits2Float(0x42818048));  // 328.3f, 64.7714f, 329, 64.7612f, 329.7f, 64.7505f
path.cubicTo(SkBits2Float(0x43a53333), SkBits2Float(0x42817ad1), SkBits2Float(0x43a58ccd), SkBits2Float(0x42816e33), SkBits2Float(0x43a5e666), SkBits2Float(0x42816ba7));  // 330.4f, 64.7399f, 331.1f, 64.7152f, 331.8f, 64.7103f
path.cubicTo(SkBits2Float(0x43a64000), SkBits2Float(0x4281691c), SkBits2Float(0x43a6999a), SkBits2Float(0x42816b46), SkBits2Float(0x43a6f333), SkBits2Float(0x42817104));  // 332.5f, 64.7053f, 333.2f, 64.7095f, 333.9f, 64.7207f
path.cubicTo(SkBits2Float(0x43a74ccd), SkBits2Float(0x428176c3), SkBits2Float(0x43a7a666), SkBits2Float(0x42817fa9), SkBits2Float(0x43a80000), SkBits2Float(0x42818e1f));  // 334.6f, 64.732f, 335.3f, 64.7493f, 336, 64.7776f
path.cubicTo(SkBits2Float(0x43a8599a), SkBits2Float(0x42819c95), SkBits2Float(0x43a8b333), SkBits2Float(0x4281b1ec), SkBits2Float(0x43a90ccd), SkBits2Float(0x4281c7c7));  // 336.7f, 64.8058f, 337.4f, 64.8475f, 338.1f, 64.8902f
path.cubicTo(SkBits2Float(0x43a96666), SkBits2Float(0x4281dda2), SkBits2Float(0x43a9c000), SkBits2Float(0x428209cf), SkBits2Float(0x43aa199a), SkBits2Float(0x42821140));  // 338.8f, 64.9329f, 339.5f, 65.0192f, 340.2f, 65.0337f
path.cubicTo(SkBits2Float(0x43aa7333), SkBits2Float(0x428218b0), SkBits2Float(0x43aacccd), SkBits2Float(0x42820dff), SkBits2Float(0x43ab2666), SkBits2Float(0x4281f46b));  // 340.9f, 65.0482f, 341.6f, 65.0273f, 342.3f, 64.9774f
path.cubicTo(SkBits2Float(0x43ab8000), SkBits2Float(0x4281dad8), SkBits2Float(0x43abd99a), SkBits2Float(0x42819956), SkBits2Float(0x43ac3333), SkBits2Float(0x428177cd));  // 343, 64.9274f, 343.7f, 64.7995f, 344.4f, 64.734f
path.cubicTo(SkBits2Float(0x43ac8ccd), SkBits2Float(0x42815644), SkBits2Float(0x43ace666), SkBits2Float(0x42813910), SkBits2Float(0x43ad4000), SkBits2Float(0x42812b37));  // 345.1f, 64.6685f, 345.8f, 64.6115f, 346.5f, 64.5844f
path.cubicTo(SkBits2Float(0x43ad999a), SkBits2Float(0x42811d5e), SkBits2Float(0x43adf333), SkBits2Float(0x42812394), SkBits2Float(0x43ae4ccd), SkBits2Float(0x428124b5));  // 347.2f, 64.5574f, 347.9f, 64.5695f, 348.6f, 64.5717f
path.cubicTo(SkBits2Float(0x43aea666), SkBits2Float(0x428125d6), SkBits2Float(0x43af0000), SkBits2Float(0x42812c1c), SkBits2Float(0x43af599a), SkBits2Float(0x428131ff));  // 349.3f, 64.5739f, 350, 64.5862f, 350.7f, 64.5976f
path.cubicTo(SkBits2Float(0x43afb333), SkBits2Float(0x428137e3), SkBits2Float(0x43b00ccd), SkBits2Float(0x4281417f), SkBits2Float(0x43b06666), SkBits2Float(0x4281480b));  // 351.4f, 64.6092f, 352.1f, 64.6279f, 352.8f, 64.6407f
path.cubicTo(SkBits2Float(0x43b0c000), SkBits2Float(0x42814e97), SkBits2Float(0x43b1199a), SkBits2Float(0x4281534c), SkBits2Float(0x43b17333), SkBits2Float(0x42815947));  // 353.5f, 64.6535f, 354.2f, 64.6627f, 354.9f, 64.6744f
path.cubicTo(SkBits2Float(0x43b1cccd), SkBits2Float(0x42815f42), SkBits2Float(0x43b22666), SkBits2Float(0x428165ff), SkBits2Float(0x43b28000), SkBits2Float(0x42816bee));  // 355.6f, 64.6861f, 356.3f, 64.6992f, 357, 64.7108f
path.cubicTo(SkBits2Float(0x43b2d99a), SkBits2Float(0x428171de), SkBits2Float(0x43b33333), SkBits2Float(0x42817af5), SkBits2Float(0x43b38ccd), SkBits2Float(0x42817ce3));  // 357.7f, 64.7224f, 358.4f, 64.7402f, 359.1f, 64.7439f
path.cubicTo(SkBits2Float(0x43b3e666), SkBits2Float(0x42817ed2), SkBits2Float(0x43b44000), SkBits2Float(0x42817bcf), SkBits2Float(0x43b4999a), SkBits2Float(0x42817786));  // 359.8f, 64.7477f, 360.5f, 64.7418f, 361.2f, 64.7334f
path.cubicTo(SkBits2Float(0x43b4f333), SkBits2Float(0x4281733d), SkBits2Float(0x43b54ccd), SkBits2Float(0x428167a7), SkBits2Float(0x43b5a666), SkBits2Float(0x4281632d));  // 361.9f, 64.7251f, 362.6f, 64.7024f, 363.3f, 64.6937f
path.cubicTo(SkBits2Float(0x43b60000), SkBits2Float(0x42815eb3), SkBits2Float(0x43b6599a), SkBits2Float(0x42815b7e), SkBits2Float(0x43b6b333), SkBits2Float(0x42815cab));  // 364, 64.685f, 364.7f, 64.6787f, 365.4f, 64.681f
path.cubicTo(SkBits2Float(0x43b70ccd), SkBits2Float(0x42815dd8), SkBits2Float(0x43b76666), SkBits2Float(0x4281644d), SkBits2Float(0x43b7c000), SkBits2Float(0x42816a3c));  // 366.1f, 64.6833f, 366.8f, 64.6959f, 367.5f, 64.7075f
path.cubicTo(SkBits2Float(0x43b8199a), SkBits2Float(0x4281702b), SkBits2Float(0x43b87333), SkBits2Float(0x428179d3), SkBits2Float(0x43b8cccd), SkBits2Float(0x42818048));  // 368.2f, 64.7191f, 368.9f, 64.7379f, 369.6f, 64.7505f
path.cubicTo(SkBits2Float(0x43b92666), SkBits2Float(0x428186bc), SkBits2Float(0x43b98000), SkBits2Float(0x42818d4a), SkBits2Float(0x43b9d99a), SkBits2Float(0x428190f6));  // 370.3f, 64.7632f, 371, 64.776f, 371.7f, 64.7831f
path.cubicTo(SkBits2Float(0x43ba3333), SkBits2Float(0x428194a3), SkBits2Float(0x43ba8ccd), SkBits2Float(0x428193b0), SkBits2Float(0x43bae666), SkBits2Float(0x42819653));  // 372.4f, 64.7903f, 373.1f, 64.7885f, 373.8f, 64.7936f
path.cubicTo(SkBits2Float(0x43bb4000), SkBits2Float(0x428198f6), SkBits2Float(0x43bb999a), SkBits2Float(0x42819840), SkBits2Float(0x43bbf333), SkBits2Float(0x4281a0c6));  // 374.5f, 64.7988f, 375.2f, 64.7974f, 375.9f, 64.814f
path.cubicTo(SkBits2Float(0x43bc4ccd), SkBits2Float(0x4281a94d), SkBits2Float(0x43bca666), SkBits2Float(0x4281bc0d), SkBits2Float(0x43bd0000), SkBits2Float(0x4281c979));  // 376.6f, 64.8307f, 377.3f, 64.8673f, 378, 64.8935f
path.cubicTo(SkBits2Float(0x43bd599a), SkBits2Float(0x4281d6e5), SkBits2Float(0x43bdb333), SkBits2Float(0x4281e6fe), SkBits2Float(0x43be0ccd), SkBits2Float(0x4281f14e));  // 378.7f, 64.9197f, 379.4f, 64.9512f, 380.1f, 64.9713f
path.cubicTo(SkBits2Float(0x43be6666), SkBits2Float(0x4281fb9e), SkBits2Float(0x43bec000), SkBits2Float(0x4281fd75), SkBits2Float(0x43bf199a), SkBits2Float(0x42820759));  // 380.8f, 64.9914f, 381.5f, 64.995f, 382.2f, 65.0144f
path.cubicTo(SkBits2Float(0x43bf7333), SkBits2Float(0x4282113e), SkBits2Float(0x43bfcccd), SkBits2Float(0x42822559), SkBits2Float(0x43c02666), SkBits2Float(0x42822ca8));  // 382.9f, 65.0337f, 383.6f, 65.0729f, 384.3f, 65.0872f
path.lineTo(SkBits2Float(0x43c02666), SkBits2Float(0x42823333));  // 384.3f, 65.1f
path.lineTo(SkBits2Float(0x43994ccd), SkBits2Float(0x42823333));  // 306.6f, 65.1f
path.close();
path.moveTo(SkBits2Float(0x43c24000), SkBits2Float(0x42823333));  // 388.5f, 65.1f
path.lineTo(SkBits2Float(0x43c24000), SkBits2Float(0x42823333));  // 388.5f, 65.1f
path.cubicTo(SkBits2Float(0x43c2999a), SkBits2Float(0x42823333), SkBits2Float(0x43c2f333), SkBits2Float(0x428239f1), SkBits2Float(0x43c34ccd), SkBits2Float(0x42823333));  // 389.2f, 65.1f, 389.9f, 65.1132f, 390.6f, 65.1f
path.cubicTo(SkBits2Float(0x43c3a666), SkBits2Float(0x42822c75), SkBits2Float(0x43c40000), SkBits2Float(0x42822289), SkBits2Float(0x43c4599a), SkBits2Float(0x42820abe));  // 391.3f, 65.0868f, 392, 65.0675f, 392.7f, 65.021f
path.cubicTo(SkBits2Float(0x43c4b333), SkBits2Float(0x4281f2f3), SkBits2Float(0x43c50ccd), SkBits2Float(0x4281be4d), SkBits2Float(0x43c56666), SkBits2Float(0x4281a471));  // 393.4f, 64.9745f, 394.1f, 64.8717f, 394.8f, 64.8212f
path.cubicTo(SkBits2Float(0x43c5c000), SkBits2Float(0x42818a96), SkBits2Float(0x43c6199a), SkBits2Float(0x428177e3), SkBits2Float(0x43c67333), SkBits2Float(0x42816f99));  // 395.5f, 64.7707f, 396.2f, 64.7342f, 396.9f, 64.718f
path.cubicTo(SkBits2Float(0x43c6cccd), SkBits2Float(0x4281674f), SkBits2Float(0x43c72666), SkBits2Float(0x42817195), SkBits2Float(0x43c78000), SkBits2Float(0x428172b7));  // 397.6f, 64.7018f, 398.3f, 64.7218f, 399, 64.7241f
path.cubicTo(SkBits2Float(0x43c7d99a), SkBits2Float(0x428173d8), SkBits2Float(0x43c83333), SkBits2Float(0x42817528), SkBits2Float(0x43c88ccd), SkBits2Float(0x42817661));  // 399.7f, 64.7263f, 400.4f, 64.7288f, 401.1f, 64.7312f
path.cubicTo(SkBits2Float(0x43c8e666), SkBits2Float(0x4281779a), SkBits2Float(0x43c94000), SkBits2Float(0x4281778d), SkBits2Float(0x43c9999a), SkBits2Float(0x42817a0c));  // 401.8f, 64.7336f, 402.5f, 64.7335f, 403.2f, 64.7384f
path.cubicTo(SkBits2Float(0x43c9f333), SkBits2Float(0x42817c8c), SkBits2Float(0x43ca4ccd), SkBits2Float(0x42817f49), SkBits2Float(0x43caa666), SkBits2Float(0x4281855e));  // 403.9f, 64.7433f, 404.6f, 64.7486f, 405.3f, 64.7605f
path.cubicTo(SkBits2Float(0x43cb0000), SkBits2Float(0x42818b72), SkBits2Float(0x43cb599a), SkBits2Float(0x4281985b), SkBits2Float(0x43cbb333), SkBits2Float(0x42819e87));  // 406, 64.7724f, 406.7f, 64.7976f, 407.4f, 64.8096f
path.cubicTo(SkBits2Float(0x43cc0ccd), SkBits2Float(0x4281a4b3), SkBits2Float(0x43cc6666), SkBits2Float(0x4281a6ea), SkBits2Float(0x43ccc000), SkBits2Float(0x4281aa66));  // 408.1f, 64.8217f, 408.8f, 64.826f, 409.5f, 64.8328f
path.cubicTo(SkBits2Float(0x43cd199a), SkBits2Float(0x4281ade2), SkBits2Float(0x43cd7333), SkBits2Float(0x4281aad0), SkBits2Float(0x43cdcccd), SkBits2Float(0x4281b36e));  // 410.2f, 64.8396f, 410.9f, 64.8336f, 411.6f, 64.8504f
path.cubicTo(SkBits2Float(0x43ce2666), SkBits2Float(0x4281bc0c), SkBits2Float(0x43ce8000), SkBits2Float(0x4281d071), SkBits2Float(0x43ced99a), SkBits2Float(0x4281de19));  // 412.3f, 64.8673f, 413, 64.9071f, 413.7f, 64.9338f
path.cubicTo(SkBits2Float(0x43cf3333), SkBits2Float(0x4281ebc2), SkBits2Float(0x43cf8ccd), SkBits2Float(0x4281fb65), SkBits2Float(0x43cfe666), SkBits2Float(0x42820561));  // 414.4f, 64.9605f, 415.1f, 64.991f, 415.8f, 65.0105f
path.cubicTo(SkBits2Float(0x43d04000), SkBits2Float(0x42820f5d), SkBits2Float(0x43d0999a), SkBits2Float(0x428217a6), SkBits2Float(0x43d0f333), SkBits2Float(0x42821a01));  // 416.5f, 65.03f, 417.2f, 65.0462f, 417.9f, 65.0508f
path.cubicTo(SkBits2Float(0x43d14ccd), SkBits2Float(0x42821c5b), SkBits2Float(0x43d1a666), SkBits2Float(0x42821a47), SkBits2Float(0x43d20000), SkBits2Float(0x4282137f));  // 418.6f, 65.0554f, 419.3f, 65.0513f, 420, 65.0381f
path.cubicTo(SkBits2Float(0x43d2599a), SkBits2Float(0x42820cb6), SkBits2Float(0x43d2b333), SkBits2Float(0x4281fcb3), SkBits2Float(0x43d30ccd), SkBits2Float(0x4281f14e));  // 420.7f, 65.0248f, 421.4f, 64.9936f, 422.1f, 64.9713f
path.cubicTo(SkBits2Float(0x43d36666), SkBits2Float(0x4281e5e8), SkBits2Float(0x43d3c000), SkBits2Float(0x4281d645), SkBits2Float(0x43d4199a), SkBits2Float(0x4281cf1d));  // 422.8f, 64.949f, 423.5f, 64.9185f, 424.2f, 64.9045f
path.cubicTo(SkBits2Float(0x43d47333), SkBits2Float(0x4281c7f4), SkBits2Float(0x43d4cccd), SkBits2Float(0x4281c5d7), SkBits2Float(0x43d52666), SkBits2Float(0x4281c65c));  // 424.9f, 64.8905f, 425.6f, 64.8864f, 426.3f, 64.8874f
path.cubicTo(SkBits2Float(0x43d58000), SkBits2Float(0x4281c6e1), SkBits2Float(0x43d5d99a), SkBits2Float(0x4281d040), SkBits2Float(0x43d63333), SkBits2Float(0x4281d23a));  // 427, 64.8884f, 427.7f, 64.9067f, 428.4f, 64.9106f
path.cubicTo(SkBits2Float(0x43d68ccd), SkBits2Float(0x4281d435), SkBits2Float(0x43d6e666), SkBits2Float(0x4281d7ed), SkBits2Float(0x43d74000), SkBits2Float(0x4281d23a));  // 429.1f, 64.9145f, 429.8f, 64.9217f, 430.5f, 64.9106f
path.cubicTo(SkBits2Float(0x43d7999a), SkBits2Float(0x4281cc88), SkBits2Float(0x43d7f333), SkBits2Float(0x4281ba4e), SkBits2Float(0x43d84ccd), SkBits2Float(0x4281b009));  // 431.2f, 64.8995f, 431.9f, 64.8639f, 432.6f, 64.8438f
path.cubicTo(SkBits2Float(0x43d8a666), SkBits2Float(0x4281a5c5), SkBits2Float(0x43d90000), SkBits2Float(0x4281997b), SkBits2Float(0x43d9599a), SkBits2Float(0x428194a1));  // 433.3f, 64.8238f, 434, 64.7998f, 434.7f, 64.7903f
path.cubicTo(SkBits2Float(0x43d9b333), SkBits2Float(0x42818fc7), SkBits2Float(0x43da0ccd), SkBits2Float(0x4281929b), SkBits2Float(0x43da6666), SkBits2Float(0x428192ef));  // 435.4f, 64.7808f, 436.1f, 64.7863f, 436.8f, 64.787f
path.cubicTo(SkBits2Float(0x43dac000), SkBits2Float(0x42819343), SkBits2Float(0x43db199a), SkBits2Float(0x428194dc), SkBits2Float(0x43db7333), SkBits2Float(0x4281969a));  // 437.5f, 64.7876f, 438.2f, 64.7907f, 438.9f, 64.7941f
path.cubicTo(SkBits2Float(0x43dbcccd), SkBits2Float(0x42819858), SkBits2Float(0x43dc2666), SkBits2Float(0x42819925), SkBits2Float(0x43dc8000), SkBits2Float(0x42819d62));  // 439.6f, 64.7975f, 440.3f, 64.7991f, 441, 64.8074f
path.cubicTo(SkBits2Float(0x43dcd99a), SkBits2Float(0x4281a19f), SkBits2Float(0x43dd3333), SkBits2Float(0x4281a9d2), SkBits2Float(0x43dd8ccd), SkBits2Float(0x4281b009));  // 441.7f, 64.8157f, 442.4f, 64.8317f, 443.1f, 64.8438f
path.cubicTo(SkBits2Float(0x43dde666), SkBits2Float(0x4281b641), SkBits2Float(0x43de4000), SkBits2Float(0x4281be1f), SkBits2Float(0x43de999a), SkBits2Float(0x4281c2b1));  // 443.8f, 64.856f, 444.5f, 64.8713f, 445.2f, 64.8803f
path.cubicTo(SkBits2Float(0x43def333), SkBits2Float(0x4281c742), SkBits2Float(0x43df4ccd), SkBits2Float(0x4281ca45), SkBits2Float(0x43dfa666), SkBits2Float(0x4281cb72));  // 445.9f, 64.8892f, 446.6f, 64.8951f, 447.3f, 64.8974f
path.cubicTo(SkBits2Float(0x43e00000), SkBits2Float(0x4281cc9f), SkBits2Float(0x43e0599a), SkBits2Float(0x4281cb72), SkBits2Float(0x43e0b333), SkBits2Float(0x4281c9c0));  // 448, 64.8997f, 448.7f, 64.8974f, 449.4f, 64.894f
path.cubicTo(SkBits2Float(0x43e10ccd), SkBits2Float(0x4281c80e), SkBits2Float(0x43e16666), SkBits2Float(0x4281c34c), SkBits2Float(0x43e1c000), SkBits2Float(0x4281c145));  // 450.1f, 64.8907f, 450.8f, 64.8814f, 451.5f, 64.8775f
path.cubicTo(SkBits2Float(0x43e2199a), SkBits2Float(0x4281bf3f), SkBits2Float(0x43e27333), SkBits2Float(0x4281c026), SkBits2Float(0x43e2cccd), SkBits2Float(0x4281bd9a));  // 452.2f, 64.8735f, 452.9f, 64.8753f, 453.6f, 64.8703f
path.cubicTo(SkBits2Float(0x43e32666), SkBits2Float(0x4281bb0f), SkBits2Float(0x43e38000), SkBits2Float(0x4281b877), SkBits2Float(0x43e3d99a), SkBits2Float(0x4281b202));  // 454.3f, 64.8653f, 455, 64.8603f, 455.7f, 64.8477f
path.cubicTo(SkBits2Float(0x43e43333), SkBits2Float(0x4281ab8e), SkBits2Float(0x43e48ccd), SkBits2Float(0x4281a1fe), SkBits2Float(0x43e4e666), SkBits2Float(0x428196e0));  // 456.4f, 64.8351f, 457.1f, 64.8164f, 457.8f, 64.7947f
path.cubicTo(SkBits2Float(0x43e54000), SkBits2Float(0x42818bc3), SkBits2Float(0x43e5999a), SkBits2Float(0x42817cb2), SkBits2Float(0x43e5f333), SkBits2Float(0x42816f52));  // 458.5f, 64.773f, 459.2f, 64.7435f, 459.9f, 64.7174f
path.cubicTo(SkBits2Float(0x43e64ccd), SkBits2Float(0x428161f2), SkBits2Float(0x43e6a666), SkBits2Float(0x428151a5), SkBits2Float(0x43e70000), SkBits2Float(0x4281469f));  // 460.6f, 64.6913f, 461.3f, 64.6595f, 462, 64.6379f
path.cubicTo(SkBits2Float(0x43e7599a), SkBits2Float(0x42813b9a), SkBits2Float(0x43e7b333), SkBits2Float(0x428132d7), SkBits2Float(0x43e80ccd), SkBits2Float(0x42812d30));  // 462.7f, 64.6164f, 463.4f, 64.5993f, 464.1f, 64.5883f
path.cubicTo(SkBits2Float(0x43e86666), SkBits2Float(0x42812789), SkBits2Float(0x43e8c000), SkBits2Float(0x428125d6), SkBits2Float(0x43e9199a), SkBits2Float(0x428124b5));  // 464.8f, 64.5772f, 465.5f, 64.5739f, 466.2f, 64.5717f
path.cubicTo(SkBits2Float(0x43e97333), SkBits2Float(0x42812394), SkBits2Float(0x43e9cccd), SkBits2Float(0x4281258e), SkBits2Float(0x43ea2666), SkBits2Float(0x42812667));  // 466.9f, 64.5695f, 467.6f, 64.5733f, 468.3f, 64.575f
path.cubicTo(SkBits2Float(0x43ea8000), SkBits2Float(0x42812740), SkBits2Float(0x43ead99a), SkBits2Float(0x42812819), SkBits2Float(0x43eb3333), SkBits2Float(0x428129cb));  // 469, 64.5767f, 469.7f, 64.5783f, 470.4f, 64.5816f
path.cubicTo(SkBits2Float(0x43eb8ccd), SkBits2Float(0x42812b7e), SkBits2Float(0x43ebe666), SkBits2Float(0x42812734), SkBits2Float(0x43ec4000), SkBits2Float(0x42813094));  // 471.1f, 64.5849f, 471.8f, 64.5766f, 472.5f, 64.5949f
path.cubicTo(SkBits2Float(0x43ec999a), SkBits2Float(0x428139f3), SkBits2Float(0x43ecf333), SkBits2Float(0x42814838), SkBits2Float(0x43ed4ccd), SkBits2Float(0x42816208));  // 473.2f, 64.6132f, 473.9f, 64.6411f, 474.6f, 64.6915f
path.cubicTo(SkBits2Float(0x43eda666), SkBits2Float(0x42817bd8), SkBits2Float(0x43ee0000), SkBits2Float(0x4281a8c8), SkBits2Float(0x43ee599a), SkBits2Float(0x4281cb72));  // 475.3f, 64.7419f, 476, 64.8297f, 476.7f, 64.8974f
path.cubicTo(SkBits2Float(0x43eeb333), SkBits2Float(0x4281ee1c), SkBits2Float(0x43ef0ccd), SkBits2Float(0x428220ba), SkBits2Float(0x43ef6666), SkBits2Float(0x42823205));  // 477.4f, 64.9651f, 478.1f, 65.0639f, 478.8f, 65.0977f
path.cubicTo(SkBits2Float(0x43efc000), SkBits2Float(0x42824350), SkBits2Float(0x43f0199a), SkBits2Float(0x42823301), SkBits2Float(0x43f07333), SkBits2Float(0x42823333));  // 479.5f, 65.1315f, 480.2f, 65.0996f, 480.9f, 65.1f
path.lineTo(SkBits2Float(0x43f07333), SkBits2Float(0x42823333));  // 480.9f, 65.1f
path.lineTo(SkBits2Float(0x43c24000), SkBits2Float(0x42823333));  // 388.5f, 65.1f
path.close();
path.moveTo(SkBits2Float(0x43fc0000), SkBits2Float(0x42823333));  // 504, 65.1f
path.lineTo(SkBits2Float(0x43fc0000), SkBits2Float(0x42823333));  // 504, 65.1f
path.cubicTo(SkBits2Float(0x43fc599a), SkBits2Float(0x42823333), SkBits2Float(0x43fcb333), SkBits2Float(0x42823772), SkBits2Float(0x43fd0ccd), SkBits2Float(0x42823333));  // 504.7f, 65.1f, 505.4f, 65.1083f, 506.1f, 65.1f
path.cubicTo(SkBits2Float(0x43fd6666), SkBits2Float(0x42822ef4), SkBits2Float(0x43fdc000), SkBits2Float(0x428227e9), SkBits2Float(0x43fe199a), SkBits2Float(0x428219ba));  // 506.8f, 65.0917f, 507.5f, 65.0779f, 508.2f, 65.0502f
path.cubicTo(SkBits2Float(0x43fe7333), SkBits2Float(0x42820b8b), SkBits2Float(0x43fecccd), SkBits2Float(0x4281ebf3), SkBits2Float(0x43ff2666), SkBits2Float(0x4281de19));  // 508.9f, 65.0225f, 509.6f, 64.9608f, 510.3f, 64.9338f
path.cubicTo(SkBits2Float(0x43ff8000), SkBits2Float(0x4281d040), SkBits2Float(0x43ffd99a), SkBits2Float(0x4281cbde), SkBits2Float(0x4400199a), SkBits2Float(0x4281c6a2));  // 511, 64.9067f, 511.7f, 64.8982f, 512.4f, 64.888f
path.cubicTo(SkBits2Float(0x44004666), SkBits2Float(0x4281c167), SkBits2Float(0x44007333), SkBits2Float(0x4281bf82), SkBits2Float(0x4400a000), SkBits2Float(0x4281beb5));  // 513.1f, 64.8777f, 513.8f, 64.874f, 514.5f, 64.8725f
path.cubicTo(SkBits2Float(0x4400cccd), SkBits2Float(0x4281bde8), SkBits2Float(0x4400f99a), SkBits2Float(0x4281bfe4), SkBits2Float(0x44012666), SkBits2Float(0x4281c1d2));  // 515.2f, 64.8709f, 515.9f, 64.8748f, 516.6f, 64.8786f
path.cubicTo(SkBits2Float(0x44015333), SkBits2Float(0x4281c3c1), SkBits2Float(0x44018000), SkBits2Float(0x4281c822), SkBits2Float(0x4401accd), SkBits2Float(0x4281ca4d));  // 517.3f, 64.8823f, 518, 64.8909f, 518.7f, 64.8951f
path.cubicTo(SkBits2Float(0x4401d99a), SkBits2Float(0x4281cc78), SkBits2Float(0x44020666), SkBits2Float(0x4281cf4e), SkBits2Float(0x44023333), SkBits2Float(0x4281ced6));  // 519.4f, 64.8994f, 520.1f, 64.9049f, 520.8f, 64.904f
path.cubicTo(SkBits2Float(0x44026000), SkBits2Float(0x4281ce5f), SkBits2Float(0x44028ccd), SkBits2Float(0x4281cb81), SkBits2Float(0x4402b99a), SkBits2Float(0x4281c781));  // 521.5f, 64.9031f, 522.2f, 64.8975f, 522.9f, 64.8897f
path.cubicTo(SkBits2Float(0x4402e666), SkBits2Float(0x4281c380), SkBits2Float(0x44031333), SkBits2Float(0x4281babb), SkBits2Float(0x44034000), SkBits2Float(0x4281b6d2));  // 523.6f, 64.8818f, 524.3f, 64.8647f, 525, 64.8571f
path.cubicTo(SkBits2Float(0x44036ccd), SkBits2Float(0x4281b2e9), SkBits2Float(0x4403999a), SkBits2Float(0x4281b0a6), SkBits2Float(0x4403c666), SkBits2Float(0x4281b009));  // 525.7f, 64.8494f, 526.4f, 64.845f, 527.1f, 64.8438f
path.cubicTo(SkBits2Float(0x4403f333), SkBits2Float(0x4281af6d), SkBits2Float(0x44042000), SkBits2Float(0x42819d4b), SkBits2Float(0x44044ccd), SkBits2Float(0x4281b327));  // 527.8f, 64.8426f, 528.5f, 64.8072f, 529.2f, 64.8499f
path.cubicTo(SkBits2Float(0x4404799a), SkBits2Float(0x4281c903), SkBits2Float(0x4404a666), SkBits2Float(0x42821ddc), SkBits2Float(0x4404d333), SkBits2Float(0x42823333));  // 529.9f, 64.8926f, 530.6f, 65.0583f, 531.3f, 65.1f
path.cubicTo(SkBits2Float(0x44050000), SkBits2Float(0x4282488b), SkBits2Float(0x44052ccd), SkBits2Float(0x42823333), SkBits2Float(0x4405599a), SkBits2Float(0x42823333));  // 532, 65.1417f, 532.7f, 65.1f, 533.4f, 65.1f
path.lineTo(SkBits2Float(0x4405599a), SkBits2Float(0x42823333));  // 533.4f, 65.1f
path.lineTo(SkBits2Float(0x43fc0000), SkBits2Float(0x42823333));  // 504, 65.1f
path.close();
    return path;
}

static SkPath path2() {
    SkPath path;
path.moveTo(SkBits2Float(0x431d8000), SkBits2Float(0x42823333));  // 157.5f, 65.1f
path.lineTo(SkBits2Float(0x431d8000), SkBits2Float(0x42823333));  // 157.5f, 65.1f
path.cubicTo(SkBits2Float(0x431e3333), SkBits2Float(0x42823333), SkBits2Float(0x431ee666), SkBits2Float(0x42822fd9), SkBits2Float(0x431f999a), SkBits2Float(0x42823333));  // 158.2f, 65.1f, 158.9f, 65.0935f, 159.6f, 65.1f
path.cubicTo(SkBits2Float(0x43204ccd), SkBits2Float(0x4282368d), SkBits2Float(0x43210000), SkBits2Float(0x42823e05), SkBits2Float(0x4321b333), SkBits2Float(0x4282474f));  // 160.3f, 65.1065f, 161, 65.1211f, 161.7f, 65.1393f
path.cubicTo(SkBits2Float(0x43226666), SkBits2Float(0x42825098), SkBits2Float(0x4323199a), SkBits2Float(0x42825f49), SkBits2Float(0x4323cccd), SkBits2Float(0x42826aeb));  // 162.4f, 65.1574f, 163.1f, 65.1861f, 163.8f, 65.2088f
path.cubicTo(SkBits2Float(0x43248000), SkBits2Float(0x4282768e), SkBits2Float(0x43253333), SkBits2Float(0x428281ff), SkBits2Float(0x4325e666), SkBits2Float(0x42828d1c));  // 164.5f, 65.2316f, 165.2f, 65.2539f, 165.9f, 65.2756f
path.cubicTo(SkBits2Float(0x4326999a), SkBits2Float(0x4282983a), SkBits2Float(0x43274ccd), SkBits2Float(0x4282a509), SkBits2Float(0x43280000), SkBits2Float(0x4282ad9b));  // 166.6f, 65.2973f, 167.3f, 65.3223f, 168, 65.3391f
path.cubicTo(SkBits2Float(0x4328b333), SkBits2Float(0x4282b62d), SkBits2Float(0x43296666), SkBits2Float(0x4282bff9), SkBits2Float(0x432a199a), SkBits2Float(0x4282c089));  // 168.7f, 65.3558f, 169.4f, 65.3749f, 170.1f, 65.376f
path.cubicTo(SkBits2Float(0x432acccd), SkBits2Float(0x4282c11a), SkBits2Float(0x432b8000), SkBits2Float(0x4282b797), SkBits2Float(0x432c3333), SkBits2Float(0x4282b100));  // 170.8f, 65.3772f, 171.5f, 65.3586f, 172.2f, 65.3457f
path.cubicTo(SkBits2Float(0x432ce666), SkBits2Float(0x4282aa68), SkBits2Float(0x432d999a), SkBits2Float(0x4282a054), SkBits2Float(0x432e4ccd), SkBits2Float(0x428298fb));  // 172.9f, 65.3328f, 173.6f, 65.3131f, 174.3f, 65.2988f
path.cubicTo(SkBits2Float(0x432f0000), SkBits2Float(0x428291a2), SkBits2Float(0x432fb333), SkBits2Float(0x428289f3), SkBits2Float(0x43306666), SkBits2Float(0x428284e8));  // 175, 65.2844f, 175.7f, 65.2694f, 176.4f, 65.2596f
path.cubicTo(SkBits2Float(0x4331199a), SkBits2Float(0x42827fde), SkBits2Float(0x4331cccd), SkBits2Float(0x42827cff), SkBits2Float(0x43328000), SkBits2Float(0x42827abc));  // 177.1f, 65.2497f, 177.8f, 65.2441f, 178.5f, 65.2397f
path.cubicTo(SkBits2Float(0x43333333), SkBits2Float(0x42827879), SkBits2Float(0x4333e666), SkBits2Float(0x428277a0), SkBits2Float(0x4334999a), SkBits2Float(0x42827757));  // 179.2f, 65.2353f, 179.9f, 65.2336f, 180.6f, 65.2331f
path.cubicTo(SkBits2Float(0x43354ccd), SkBits2Float(0x4282770f), SkBits2Float(0x43360000), SkBits2Float(0x428277ac), SkBits2Float(0x4336b333), SkBits2Float(0x4282790a));  // 181.3f, 65.2325f, 182, 65.2337f, 182.7f, 65.2364f
path.cubicTo(SkBits2Float(0x43376666), SkBits2Float(0x42827a68), SkBits2Float(0x4338199a), SkBits2Float(0x42827d9d), SkBits2Float(0x4338cccd), SkBits2Float(0x42827f8b));  // 183.4f, 65.2391f, 184.1f, 65.2453f, 184.8f, 65.2491f
path.cubicTo(SkBits2Float(0x43398000), SkBits2Float(0x4282817a), SkBits2Float(0x433a3333), SkBits2Float(0x428283c9), SkBits2Float(0x433ae666), SkBits2Float(0x428284a2));  // 185.5f, 65.2529f, 186.2f, 65.2574f, 186.9f, 65.259f
path.cubicTo(SkBits2Float(0x433b999a), SkBits2Float(0x4282857b), SkBits2Float(0x433c4ccd), SkBits2Float(0x428284de), SkBits2Float(0x433d0000), SkBits2Float(0x428284a2));  // 187.6f, 65.2607f, 188.3f, 65.2595f, 189, 65.259f
path.cubicTo(SkBits2Float(0x433db333), SkBits2Float(0x42828465), SkBits2Float(0x433e6666), SkBits2Float(0x42828251), SkBits2Float(0x433f199a), SkBits2Float(0x42828336));  // 189.7f, 65.2586f, 190.4f, 65.2545f, 191.1f, 65.2563f
path.cubicTo(SkBits2Float(0x433fcccd), SkBits2Float(0x4282841b), SkBits2Float(0x43408000), SkBits2Float(0x428286ef), SkBits2Float(0x43413333), SkBits2Float(0x428289ff));  // 191.8f, 65.258f, 192.5f, 65.2635f, 193.2f, 65.2695f
path.cubicTo(SkBits2Float(0x4341e666), SkBits2Float(0x42828d0f), SkBits2Float(0x4342999a), SkBits2Float(0x4282930c), SkBits2Float(0x43434ccd), SkBits2Float(0x42829597));  // 193.9f, 65.2755f, 194.6f, 65.2872f, 195.3f, 65.2922f
path.cubicTo(SkBits2Float(0x43440000), SkBits2Float(0x42829822), SkBits2Float(0x4344b333), SkBits2Float(0x428298fa), SkBits2Float(0x43456666), SkBits2Float(0x42829942));  // 196, 65.2971f, 196.7f, 65.2988f, 197.4f, 65.2993f
path.cubicTo(SkBits2Float(0x4346199a), SkBits2Float(0x4282998a), SkBits2Float(0x4346cccd), SkBits2Float(0x42829749), SkBits2Float(0x43478000), SkBits2Float(0x42829749));  // 198.1f, 65.2999f, 198.8f, 65.2955f, 199.5f, 65.2955f
path.cubicTo(SkBits2Float(0x43483333), SkBits2Float(0x42829749), SkBits2Float(0x4348e666), SkBits2Float(0x428297d8), SkBits2Float(0x4349999a), SkBits2Float(0x42829942));  // 200.2f, 65.2955f, 200.9f, 65.2966f, 201.6f, 65.2993f
path.cubicTo(SkBits2Float(0x434a4ccd), SkBits2Float(0x42829aac), SkBits2Float(0x434b0000), SkBits2Float(0x42829e12), SkBits2Float(0x434bb333), SkBits2Float(0x42829fc4));  // 202.3f, 65.3021f, 203, 65.3087f, 203.7f, 65.312f
path.cubicTo(SkBits2Float(0x434c6666), SkBits2Float(0x4282a176), SkBits2Float(0x434d199a), SkBits2Float(0x4282a1f9), SkBits2Float(0x434dcccd), SkBits2Float(0x4282a36f));  // 204.4f, 65.3154f, 205.1f, 65.3164f, 205.8f, 65.3192f
path.cubicTo(SkBits2Float(0x434e8000), SkBits2Float(0x4282a4e4), SkBits2Float(0x434f3333), SkBits2Float(0x4282a71b), SkBits2Float(0x434fe666), SkBits2Float(0x4282a885));  // 206.5f, 65.3221f, 207.2f, 65.3264f, 207.9f, 65.3291f
path.cubicTo(SkBits2Float(0x4350999a), SkBits2Float(0x4282a9ef), SkBits2Float(0x43514ccd), SkBits2Float(0x4282aad4), SkBits2Float(0x43520000), SkBits2Float(0x4282abe9));  // 208.6f, 65.3319f, 209.3f, 65.3336f, 210, 65.3358f
path.cubicTo(SkBits2Float(0x4352b333), SkBits2Float(0x4282acff), SkBits2Float(0x43536666), SkBits2Float(0x4282ad0c), SkBits2Float(0x4354199a), SkBits2Float(0x4282af07));  // 210.7f, 65.3379f, 211.4f, 65.338f, 212.1f, 65.3419f
path.cubicTo(SkBits2Float(0x4354cccd), SkBits2Float(0x4282b101), SkBits2Float(0x43558000), SkBits2Float(0x4282b5c2), SkBits2Float(0x43563333), SkBits2Float(0x4282b7c8));  // 212.8f, 65.3457f, 213.5f, 65.355f, 214.2f, 65.3589f
path.cubicTo(SkBits2Float(0x4356e666), SkBits2Float(0x4282b9ce), SkBits2Float(0x4357999a), SkBits2Float(0x4282b9c3), SkBits2Float(0x43584ccd), SkBits2Float(0x4282bb2c));  // 214.9f, 65.3629f, 215.6f, 65.3628f, 216.3f, 65.3656f
path.cubicTo(SkBits2Float(0x43590000), SkBits2Float(0x4282bc96), SkBits2Float(0x4359b333), SkBits2Float(0x4282be3c), SkBits2Float(0x435a6666), SkBits2Float(0x4282c043));  // 217, 65.3683f, 217.7f, 65.3716f, 218.4f, 65.3755f
path.cubicTo(SkBits2Float(0x435b199a), SkBits2Float(0x4282c249), SkBits2Float(0x435bcccd), SkBits2Float(0x4282c50f), SkBits2Float(0x435c8000), SkBits2Float(0x4282c752));  // 219.1f, 65.3795f, 219.8f, 65.3849f, 220.5f, 65.3893f
path.cubicTo(SkBits2Float(0x435d3333), SkBits2Float(0x4282c995), SkBits2Float(0x435de666), SkBits2Float(0x4282cc2d), SkBits2Float(0x435e999a), SkBits2Float(0x4282cdd4));  // 221.2f, 65.3937f, 221.9f, 65.3988f, 222.6f, 65.402f
path.cubicTo(SkBits2Float(0x435f4ccd), SkBits2Float(0x4282cf7a), SkBits2Float(0x43600000), SkBits2Float(0x4282d211), SkBits2Float(0x4360b333), SkBits2Float(0x4282d138));  // 223.3f, 65.4052f, 224, 65.4103f, 224.7f, 65.4086f
path.cubicTo(SkBits2Float(0x43616666), SkBits2Float(0x4282d05f), SkBits2Float(0x4362199a), SkBits2Float(0x4282cd7f), SkBits2Float(0x4362cccd), SkBits2Float(0x4282c8bd));  // 225.4f, 65.407f, 226.1f, 65.4014f, 226.8f, 65.3921f
path.cubicTo(SkBits2Float(0x43638000), SkBits2Float(0x4282c3fb), SkBits2Float(0x43643333), SkBits2Float(0x4282b8dc), SkBits2Float(0x4364e666), SkBits2Float(0x4282b4aa));  // 227.5f, 65.3828f, 228.2f, 65.3611f, 228.9f, 65.3529f
path.cubicTo(SkBits2Float(0x4365999a), SkBits2Float(0x4282b079), SkBits2Float(0x43664ccd), SkBits2Float(0x4282af57), SkBits2Float(0x43670000), SkBits2Float(0x4282af94));  // 229.6f, 65.3447f, 230.3f, 65.3425f, 231, 65.3429f
path.cubicTo(SkBits2Float(0x4367b333), SkBits2Float(0x4282afd1), SkBits2Float(0x43686666), SkBits2Float(0x4282b38b), SkBits2Float(0x4369199a), SkBits2Float(0x4282b616));  // 231.7f, 65.3434f, 232.4f, 65.3507f, 233.1f, 65.3556f
path.cubicTo(SkBits2Float(0x4369cccd), SkBits2Float(0x4282b8a1), SkBits2Float(0x436a8000), SkBits2Float(0x4282bc7d), SkBits2Float(0x436b3333), SkBits2Float(0x4282bed7));  // 233.8f, 65.3606f, 234.5f, 65.3681f, 235.2f, 65.3727f
path.cubicTo(SkBits2Float(0x436be666), SkBits2Float(0x4282c131), SkBits2Float(0x436c999a), SkBits2Float(0x4282c10c), SkBits2Float(0x436d4ccd), SkBits2Float(0x4282c434));  // 235.9f, 65.3773f, 236.6f, 65.377f, 237.3f, 65.3832f
path.cubicTo(SkBits2Float(0x436e0000), SkBits2Float(0x4282c75c), SkBits2Float(0x436eb333), SkBits2Float(0x4282cb8d), SkBits2Float(0x436f6666), SkBits2Float(0x4282d1c5));  // 238, 65.3894f, 238.7f, 65.3976f, 239.4f, 65.4097f
path.cubicTo(SkBits2Float(0x4370199a), SkBits2Float(0x4282d7fd), SkBits2Float(0x4370cccd), SkBits2Float(0x4282e22a), SkBits2Float(0x43718000), SkBits2Float(0x4282e983));  // 240.1f, 65.4219f, 240.8f, 65.4417f, 241.5f, 65.4561f
path.cubicTo(SkBits2Float(0x43723333), SkBits2Float(0x4282f0dc), SkBits2Float(0x4372e666), SkBits2Float(0x4282fa30), SkBits2Float(0x4373999a), SkBits2Float(0x4282fddc));  // 242.2f, 65.4704f, 242.9f, 65.4886f, 243.6f, 65.4958f
path.cubicTo(SkBits2Float(0x43744ccd), SkBits2Float(0x42830189), SkBits2Float(0x43750000), SkBits2Float(0x4282fec1), SkBits2Float(0x4375b333), SkBits2Float(0x4282ff8e));  // 244.3f, 65.503f, 245, 65.4976f, 245.7f, 65.4991f
path.cubicTo(SkBits2Float(0x43766666), SkBits2Float(0x4283005c), SkBits2Float(0x4377199a), SkBits2Float(0x42830015), SkBits2Float(0x4377cccd), SkBits2Float(0x428302ac));  // 246.4f, 65.5007f, 247.1f, 65.5002f, 247.8f, 65.5052f
path.cubicTo(SkBits2Float(0x43788000), SkBits2Float(0x42830543), SkBits2Float(0x43793333), SkBits2Float(0x42830b0b), SkBits2Float(0x4379e666), SkBits2Float(0x42830f18));  // 248.5f, 65.5103f, 249.2f, 65.5216f, 249.9f, 65.5295f
path.cubicTo(SkBits2Float(0x437a999a), SkBits2Float(0x42831324), SkBits2Float(0x437b4ccd), SkBits2Float(0x428316b9), SkBits2Float(0x437c0000), SkBits2Float(0x42831af7));  // 250.6f, 65.5374f, 251.3f, 65.5444f, 252, 65.5527f
path.cubicTo(SkBits2Float(0x437cb333), SkBits2Float(0x42831f34), SkBits2Float(0x437d6666), SkBits2Float(0x428327af), SkBits2Float(0x437e199a), SkBits2Float(0x42832888));  // 252.7f, 65.5609f, 253.4f, 65.5775f, 254.1f, 65.5792f
path.cubicTo(SkBits2Float(0x437ecccd), SkBits2Float(0x42832961), SkBits2Float(0x437f8000), SkBits2Float(0x42832e8f), SkBits2Float(0x4380199a), SkBits2Float(0x4283200d));  // 254.8f, 65.5808f, 255.5f, 65.5909f, 256.2f, 65.5626f
path.cubicTo(SkBits2Float(0x43807333), SkBits2Float(0x4283118c), SkBits2Float(0x4380cccd), SkBits2Float(0x4282f101), SkBits2Float(0x43812666), SkBits2Float(0x4282d17e));  // 256.9f, 65.5343f, 257.6f, 65.4707f, 258.3f, 65.4092f
path.cubicTo(SkBits2Float(0x43818000), SkBits2Float(0x4282b1fc), SkBits2Float(0x4381d99a), SkBits2Float(0x42827d60), SkBits2Float(0x43823333), SkBits2Float(0x428262fe));  // 259, 65.3476f, 259.7f, 65.2449f, 260.4f, 65.1933f
path.cubicTo(SkBits2Float(0x43828ccd), SkBits2Float(0x4282489c), SkBits2Float(0x4382e666), SkBits2Float(0x42823b2a), SkBits2Float(0x43834000), SkBits2Float(0x42823333));  // 261.1f, 65.1418f, 261.8f, 65.1156f, 262.5f, 65.1f
path.cubicTo(SkBits2Float(0x4383999a), SkBits2Float(0x42822b3c), SkBits2Float(0x4383f333), SkBits2Float(0x42823333), SkBits2Float(0x43844ccd), SkBits2Float(0x42823333));  // 263.2f, 65.0844f, 263.9f, 65.1f, 264.6f, 65.1f
path.lineTo(SkBits2Float(0x43844ccd), SkBits2Float(0x42823333));  // 264.6f, 65.1f
path.lineTo(SkBits2Float(0x431d8000), SkBits2Float(0x42823333));  // 157.5f, 65.1f
path.close();
path.moveTo(SkBits2Float(0x438dc000), SkBits2Float(0x42823333));  // 283.5f, 65.1f
path.lineTo(SkBits2Float(0x438dc000), SkBits2Float(0x42823333));  // 283.5f, 65.1f
path.cubicTo(SkBits2Float(0x438e199a), SkBits2Float(0x4282356b), SkBits2Float(0x438e7333), SkBits2Float(0x42823d2c), SkBits2Float(0x438ecccd), SkBits2Float(0x42824086));  // 284.2f, 65.1043f, 284.9f, 65.1195f, 285.6f, 65.126f
path.cubicTo(SkBits2Float(0x438f2666), SkBits2Float(0x428243e0), SkBits2Float(0x438f8000), SkBits2Float(0x42824988), SkBits2Float(0x438fd99a), SkBits2Float(0x4282474f));  // 286.3f, 65.1326f, 287, 65.1436f, 287.7f, 65.1393f
path.cubicTo(SkBits2Float(0x43903333), SkBits2Float(0x42824516), SkBits2Float(0x43908ccd), SkBits2Float(0x4282368d), SkBits2Float(0x4390e666), SkBits2Float(0x42823333));  // 288.4f, 65.1349f, 289.1f, 65.1065f, 289.8f, 65.1f
path.lineTo(SkBits2Float(0x4390e666), SkBits2Float(0x42823333));  // 289.8f, 65.1f
path.lineTo(SkBits2Float(0x438dc000), SkBits2Float(0x42823333));  // 283.5f, 65.1f
path.close();
path.moveTo(SkBits2Float(0x43994ccd), SkBits2Float(0x42823333));  // 306.6f, 65.1f
path.lineTo(SkBits2Float(0x43994ccd), SkBits2Float(0x42823333));  // 306.6f, 65.1f
path.cubicTo(SkBits2Float(0x4399a666), SkBits2Float(0x42823334), SkBits2Float(0x439a0000), SkBits2Float(0x42822e24), SkBits2Float(0x439a599a), SkBits2Float(0x4282333c));  // 307.3f, 65.1f, 308, 65.0901f, 308.7f, 65.1001f
path.cubicTo(SkBits2Float(0x439ab333), SkBits2Float(0x42823854), SkBits2Float(0x439b0ccd), SkBits2Float(0x428247d2), SkBits2Float(0x439b6666), SkBits2Float(0x428251c2));  // 309.4f, 65.11f, 310.1f, 65.1403f, 310.8f, 65.1597f
path.cubicTo(SkBits2Float(0x439bc000), SkBits2Float(0x42825bb2), SkBits2Float(0x439c199a), SkBits2Float(0x428264e1), SkBits2Float(0x439c7333), SkBits2Float(0x42826edd));  // 311.5f, 65.1791f, 312.2f, 65.197f, 312.9f, 65.2165f
path.cubicTo(SkBits2Float(0x439ccccd), SkBits2Float(0x428278d9), SkBits2Float(0x439d2666), SkBits2Float(0x428282d5), SkBits2Float(0x439d8000), SkBits2Float(0x42828daa));  // 313.6f, 65.236f, 314.3f, 65.2555f, 315, 65.2767f
path.cubicTo(SkBits2Float(0x439dd99a), SkBits2Float(0x4282987f), SkBits2Float(0x439e3333), SkBits2Float(0x4282a5a2), SkBits2Float(0x439e8ccd), SkBits2Float(0x4282afdb));  // 315.7f, 65.2978f, 316.4f, 65.3235f, 317.1f, 65.3435f
path.cubicTo(SkBits2Float(0x439ee666), SkBits2Float(0x4282ba13), SkBits2Float(0x439f4000), SkBits2Float(0x4282c3ec), SkBits2Float(0x439f999a), SkBits2Float(0x4282cafd));  // 317.8f, 65.3634f, 318.5f, 65.3827f, 319.2f, 65.3965f
path.cubicTo(SkBits2Float(0x439ff333), SkBits2Float(0x4282d20d), SkBits2Float(0x43a04ccd), SkBits2Float(0x4282d6db), SkBits2Float(0x43a0a666), SkBits2Float(0x4282da40));  // 319.9f, 65.4103f, 320.6f, 65.4196f, 321.3f, 65.4263f
path.cubicTo(SkBits2Float(0x43a10000), SkBits2Float(0x4282dda4), SkBits2Float(0x43a1599a), SkBits2Float(0x4282ded1), SkBits2Float(0x43a1b333), SkBits2Float(0x4282df56));  // 322, 65.4329f, 322.7f, 65.4352f, 323.4f, 65.4362f
path.cubicTo(SkBits2Float(0x43a20ccd), SkBits2Float(0x4282dfdb), SkBits2Float(0x43a26666), SkBits2Float(0x4282de42), SkBits2Float(0x43a2c000), SkBits2Float(0x4282dd5d));  // 324.1f, 65.4372f, 324.8f, 65.4341f, 325.5f, 65.4324f
path.cubicTo(SkBits2Float(0x43a3199a), SkBits2Float(0x4282dc78), SkBits2Float(0x43a37333), SkBits2Float(0x4282d884), SkBits2Float(0x43a3cccd), SkBits2Float(0x4282d9f9));  // 326.2f, 65.4306f, 326.9f, 65.4229f, 327.6f, 65.4257f
path.cubicTo(SkBits2Float(0x43a42666), SkBits2Float(0x4282db6f), SkBits2Float(0x43a48000), SkBits2Float(0x4282e0a8), SkBits2Float(0x43a4d99a), SkBits2Float(0x4282e61e));  // 328.3f, 65.4286f, 329, 65.4388f, 329.7f, 65.4494f
path.cubicTo(SkBits2Float(0x43a53333), SkBits2Float(0x4282eb95), SkBits2Float(0x43a58ccd), SkBits2Float(0x4282f833), SkBits2Float(0x43a5e666), SkBits2Float(0x4282fabf));  // 330.4f, 65.4601f, 331.1f, 65.4848f, 331.8f, 65.4897f
path.cubicTo(SkBits2Float(0x43a64000), SkBits2Float(0x4282fd4a), SkBits2Float(0x43a6999a), SkBits2Float(0x4282fb20), SkBits2Float(0x43a6f333), SkBits2Float(0x4282f562));  // 332.5f, 65.4947f, 333.2f, 65.4905f, 333.9f, 65.4793f
path.cubicTo(SkBits2Float(0x43a74ccd), SkBits2Float(0x4282efa3), SkBits2Float(0x43a7a666), SkBits2Float(0x4282e6bd), SkBits2Float(0x43a80000), SkBits2Float(0x4282d847));  // 334.6f, 65.468f, 335.3f, 65.4507f, 336, 65.4224f
path.cubicTo(SkBits2Float(0x43a8599a), SkBits2Float(0x4282c9d1), SkBits2Float(0x43a8b333), SkBits2Float(0x4282b47a), SkBits2Float(0x43a90ccd), SkBits2Float(0x42829e9f));  // 336.7f, 65.3942f, 337.4f, 65.3525f, 338.1f, 65.3098f
path.cubicTo(SkBits2Float(0x43a96666), SkBits2Float(0x428288c4), SkBits2Float(0x43a9c000), SkBits2Float(0x42825c97), SkBits2Float(0x43aa199a), SkBits2Float(0x42825526));  // 338.8f, 65.2671f, 339.5f, 65.1808f, 340.2f, 65.1663f
path.cubicTo(SkBits2Float(0x43aa7333), SkBits2Float(0x42824db6), SkBits2Float(0x43aacccd), SkBits2Float(0x42825867), SkBits2Float(0x43ab2666), SkBits2Float(0x428271fb));  // 340.9f, 65.1518f, 341.6f, 65.1727f, 342.3f, 65.2226f
path.cubicTo(SkBits2Float(0x43ab8000), SkBits2Float(0x42828b8e), SkBits2Float(0x43abd99a), SkBits2Float(0x4282cd10), SkBits2Float(0x43ac3333), SkBits2Float(0x4282ee99));  // 343, 65.2726f, 343.7f, 65.4005f, 344.4f, 65.466f
path.cubicTo(SkBits2Float(0x43ac8ccd), SkBits2Float(0x42831022), SkBits2Float(0x43ace666), SkBits2Float(0x42832d56), SkBits2Float(0x43ad4000), SkBits2Float(0x42833b2f));  // 345.1f, 65.5315f, 345.8f, 65.5885f, 346.5f, 65.6156f
path.cubicTo(SkBits2Float(0x43ad999a), SkBits2Float(0x42834908), SkBits2Float(0x43adf333), SkBits2Float(0x428342d2), SkBits2Float(0x43ae4ccd), SkBits2Float(0x428341b1));  // 347.2f, 65.6426f, 347.9f, 65.6305f, 348.6f, 65.6283f
path.cubicTo(SkBits2Float(0x43aea666), SkBits2Float(0x42834090), SkBits2Float(0x43af0000), SkBits2Float(0x42833a4a), SkBits2Float(0x43af599a), SkBits2Float(0x42833467));  // 349.3f, 65.6261f, 350, 65.6138f, 350.7f, 65.6023f
path.cubicTo(SkBits2Float(0x43afb333), SkBits2Float(0x42832e83), SkBits2Float(0x43b00ccd), SkBits2Float(0x428324e7), SkBits2Float(0x43b06666), SkBits2Float(0x42831e5b));  // 351.4f, 65.5908f, 352.1f, 65.5721f, 352.8f, 65.5593f
path.cubicTo(SkBits2Float(0x43b0c000), SkBits2Float(0x428317cf), SkBits2Float(0x43b1199a), SkBits2Float(0x4283131a), SkBits2Float(0x43b17333), SkBits2Float(0x42830d1f));  // 353.5f, 65.5465f, 354.2f, 65.5373f, 354.9f, 65.5256f
path.cubicTo(SkBits2Float(0x43b1cccd), SkBits2Float(0x42830724), SkBits2Float(0x43b22666), SkBits2Float(0x42830067), SkBits2Float(0x43b28000), SkBits2Float(0x4282fa78));  // 355.6f, 65.5139f, 356.3f, 65.5008f, 357, 65.4892f
path.cubicTo(SkBits2Float(0x43b2d99a), SkBits2Float(0x4282f488), SkBits2Float(0x43b33333), SkBits2Float(0x4282eb71), SkBits2Float(0x43b38ccd), SkBits2Float(0x4282e983));  // 357.7f, 65.4776f, 358.4f, 65.4598f, 359.1f, 65.4561f
path.cubicTo(SkBits2Float(0x43b3e666), SkBits2Float(0x4282e794), SkBits2Float(0x43b44000), SkBits2Float(0x4282ea97), SkBits2Float(0x43b4999a), SkBits2Float(0x4282eee0));  // 359.8f, 65.4523f, 360.5f, 65.4582f, 361.2f, 65.4666f
path.cubicTo(SkBits2Float(0x43b4f333), SkBits2Float(0x4282f329), SkBits2Float(0x43b54ccd), SkBits2Float(0x4282febf), SkBits2Float(0x43b5a666), SkBits2Float(0x42830339));  // 361.9f, 65.4749f, 362.6f, 65.4976f, 363.3f, 65.5063f
path.cubicTo(SkBits2Float(0x43b60000), SkBits2Float(0x428307b3), SkBits2Float(0x43b6599a), SkBits2Float(0x42830ae8), SkBits2Float(0x43b6b333), SkBits2Float(0x428309bb));  // 364, 65.515f, 364.7f, 65.5213f, 365.4f, 65.519f
path.cubicTo(SkBits2Float(0x43b70ccd), SkBits2Float(0x4283088e), SkBits2Float(0x43b76666), SkBits2Float(0x42830219), SkBits2Float(0x43b7c000), SkBits2Float(0x4282fc2a));  // 366.1f, 65.5167f, 366.8f, 65.5041f, 367.5f, 65.4925f
path.cubicTo(SkBits2Float(0x43b8199a), SkBits2Float(0x4282f63b), SkBits2Float(0x43b87333), SkBits2Float(0x4282ec93), SkBits2Float(0x43b8cccd), SkBits2Float(0x4282e61e));  // 368.2f, 65.4809f, 368.9f, 65.4621f, 369.6f, 65.4494f
path.cubicTo(SkBits2Float(0x43b92666), SkBits2Float(0x4282dfaa), SkBits2Float(0x43b98000), SkBits2Float(0x4282d91c), SkBits2Float(0x43b9d99a), SkBits2Float(0x4282d570));  // 370.3f, 65.4368f, 371, 65.424f, 371.7f, 65.4169f
path.cubicTo(SkBits2Float(0x43ba3333), SkBits2Float(0x4282d1c3), SkBits2Float(0x43ba8ccd), SkBits2Float(0x4282d2b6), SkBits2Float(0x43bae666), SkBits2Float(0x4282d013));  // 372.4f, 65.4097f, 373.1f, 65.4115f, 373.8f, 65.4064f
path.cubicTo(SkBits2Float(0x43bb4000), SkBits2Float(0x4282cd70), SkBits2Float(0x43bb999a), SkBits2Float(0x4282ce26), SkBits2Float(0x43bbf333), SkBits2Float(0x4282c5a0));  // 374.5f, 65.4012f, 375.2f, 65.4026f, 375.9f, 65.386f
path.cubicTo(SkBits2Float(0x43bc4ccd), SkBits2Float(0x4282bd19), SkBits2Float(0x43bca666), SkBits2Float(0x4282aa59), SkBits2Float(0x43bd0000), SkBits2Float(0x42829ced));  // 376.6f, 65.3693f, 377.3f, 65.3327f, 378, 65.3065f
path.cubicTo(SkBits2Float(0x43bd599a), SkBits2Float(0x42828f81), SkBits2Float(0x43bdb333), SkBits2Float(0x42827f68), SkBits2Float(0x43be0ccd), SkBits2Float(0x42827518));  // 378.7f, 65.2803f, 379.4f, 65.2488f, 380.1f, 65.2287f
path.cubicTo(SkBits2Float(0x43be6666), SkBits2Float(0x42826ac8), SkBits2Float(0x43bec000), SkBits2Float(0x428268f1), SkBits2Float(0x43bf199a), SkBits2Float(0x42825f0d));  // 380.8f, 65.2086f, 381.5f, 65.205f, 382.2f, 65.1856f
path.cubicTo(SkBits2Float(0x43bf7333), SkBits2Float(0x42825528), SkBits2Float(0x43bfcccd), SkBits2Float(0x4282410d), SkBits2Float(0x43c02666), SkBits2Float(0x428239be));  // 382.9f, 65.1663f, 383.6f, 65.1271f, 384.3f, 65.1128f
path.lineTo(SkBits2Float(0x43c02666), SkBits2Float(0x42823333));  // 384.3f, 65.1f
path.lineTo(SkBits2Float(0x43994ccd), SkBits2Float(0x42823333));  // 306.6f, 65.1f
path.close();
path.moveTo(SkBits2Float(0x43c24000), SkBits2Float(0x42823333));  // 388.5f, 65.1f
path.lineTo(SkBits2Float(0x43c24000), SkBits2Float(0x42823333));  // 388.5f, 65.1f
path.cubicTo(SkBits2Float(0x43c2999a), SkBits2Float(0x42823333), SkBits2Float(0x43c2f333), SkBits2Float(0x42822c75), SkBits2Float(0x43c34ccd), SkBits2Float(0x42823333));  // 389.2f, 65.1f, 389.9f, 65.0868f, 390.6f, 65.1f
path.cubicTo(SkBits2Float(0x43c3a666), SkBits2Float(0x428239f1), SkBits2Float(0x43c40000), SkBits2Float(0x428243dd), SkBits2Float(0x43c4599a), SkBits2Float(0x42825ba8));  // 391.3f, 65.1132f, 392, 65.1325f, 392.7f, 65.179f
path.cubicTo(SkBits2Float(0x43c4b333), SkBits2Float(0x42827373), SkBits2Float(0x43c50ccd), SkBits2Float(0x4282a819), SkBits2Float(0x43c56666), SkBits2Float(0x4282c1f5));  // 393.4f, 65.2255f, 394.1f, 65.3283f, 394.8f, 65.3788f
path.cubicTo(SkBits2Float(0x43c5c000), SkBits2Float(0x4282dbd0), SkBits2Float(0x43c6199a), SkBits2Float(0x4282ee83), SkBits2Float(0x43c67333), SkBits2Float(0x4282f6cd));  // 395.5f, 65.4293f, 396.2f, 65.4658f, 396.9f, 65.482f
path.cubicTo(SkBits2Float(0x43c6cccd), SkBits2Float(0x4282ff17), SkBits2Float(0x43c72666), SkBits2Float(0x4282f4d1), SkBits2Float(0x43c78000), SkBits2Float(0x4282f3af));  // 397.6f, 65.4982f, 398.3f, 65.4782f, 399, 65.4759f
path.cubicTo(SkBits2Float(0x43c7d99a), SkBits2Float(0x4282f28e), SkBits2Float(0x43c83333), SkBits2Float(0x4282f13e), SkBits2Float(0x43c88ccd), SkBits2Float(0x4282f005));  // 399.7f, 65.4737f, 400.4f, 65.4712f, 401.1f, 65.4688f
path.cubicTo(SkBits2Float(0x43c8e666), SkBits2Float(0x4282eecc), SkBits2Float(0x43c94000), SkBits2Float(0x4282eed9), SkBits2Float(0x43c9999a), SkBits2Float(0x4282ec5a));  // 401.8f, 65.4664f, 402.5f, 65.4665f, 403.2f, 65.4616f
path.cubicTo(SkBits2Float(0x43c9f333), SkBits2Float(0x4282e9da), SkBits2Float(0x43ca4ccd), SkBits2Float(0x4282e71d), SkBits2Float(0x43caa666), SkBits2Float(0x4282e108));  // 403.9f, 65.4567f, 404.6f, 65.4514f, 405.3f, 65.4395f
path.cubicTo(SkBits2Float(0x43cb0000), SkBits2Float(0x4282daf4), SkBits2Float(0x43cb599a), SkBits2Float(0x4282ce0b), SkBits2Float(0x43cbb333), SkBits2Float(0x4282c7df));  // 406, 65.4276f, 406.7f, 65.4024f, 407.4f, 65.3904f
path.cubicTo(SkBits2Float(0x43cc0ccd), SkBits2Float(0x4282c1b3), SkBits2Float(0x43cc6666), SkBits2Float(0x4282bf7c), SkBits2Float(0x43ccc000), SkBits2Float(0x4282bc00));  // 408.1f, 65.3783f, 408.8f, 65.374f, 409.5f, 65.3672f
path.cubicTo(SkBits2Float(0x43cd199a), SkBits2Float(0x4282b884), SkBits2Float(0x43cd7333), SkBits2Float(0x4282bb96), SkBits2Float(0x43cdcccd), SkBits2Float(0x4282b2f8));  // 410.2f, 65.3604f, 410.9f, 65.3664f, 411.6f, 65.3495f
path.cubicTo(SkBits2Float(0x43ce2666), SkBits2Float(0x4282aa5a), SkBits2Float(0x43ce8000), SkBits2Float(0x428295f5), SkBits2Float(0x43ced99a), SkBits2Float(0x4282884d));  // 412.3f, 65.3327f, 413, 65.2929f, 413.7f, 65.2662f
path.cubicTo(SkBits2Float(0x43cf3333), SkBits2Float(0x42827aa4), SkBits2Float(0x43cf8ccd), SkBits2Float(0x42826b01), SkBits2Float(0x43cfe666), SkBits2Float(0x42826105));  // 414.4f, 65.2395f, 415.1f, 65.209f, 415.8f, 65.1895f
path.cubicTo(SkBits2Float(0x43d04000), SkBits2Float(0x42825709), SkBits2Float(0x43d0999a), SkBits2Float(0x42824ec0), SkBits2Float(0x43d0f333), SkBits2Float(0x42824c65));  // 416.5f, 65.17f, 417.2f, 65.1538f, 417.9f, 65.1492f
path.cubicTo(SkBits2Float(0x43d14ccd), SkBits2Float(0x42824a0b), SkBits2Float(0x43d1a666), SkBits2Float(0x42824c1f), SkBits2Float(0x43d20000), SkBits2Float(0x428252e7));  // 418.6f, 65.1446f, 419.3f, 65.1487f, 420, 65.1619f
path.cubicTo(SkBits2Float(0x43d2599a), SkBits2Float(0x428259b0), SkBits2Float(0x43d2b333), SkBits2Float(0x428269b3), SkBits2Float(0x43d30ccd), SkBits2Float(0x42827518));  // 420.7f, 65.1752f, 421.4f, 65.2064f, 422.1f, 65.2287f
path.cubicTo(SkBits2Float(0x43d36666), SkBits2Float(0x4282807e), SkBits2Float(0x43d3c000), SkBits2Float(0x42829021), SkBits2Float(0x43d4199a), SkBits2Float(0x42829749));  // 422.8f, 65.251f, 423.5f, 65.2815f, 424.2f, 65.2955f
path.cubicTo(SkBits2Float(0x43d47333), SkBits2Float(0x42829e72), SkBits2Float(0x43d4cccd), SkBits2Float(0x4282a08f), SkBits2Float(0x43d52666), SkBits2Float(0x4282a00a));  // 424.9f, 65.3095f, 425.6f, 65.3136f, 426.3f, 65.3126f
path.cubicTo(SkBits2Float(0x43d58000), SkBits2Float(0x42829f85), SkBits2Float(0x43d5d99a), SkBits2Float(0x42829626), SkBits2Float(0x43d63333), SkBits2Float(0x4282942c));  // 427, 65.3116f, 427.7f, 65.2933f, 428.4f, 65.2894f
path.cubicTo(SkBits2Float(0x43d68ccd), SkBits2Float(0x42829231), SkBits2Float(0x43d6e666), SkBits2Float(0x42828e79), SkBits2Float(0x43d74000), SkBits2Float(0x4282942c));  // 429.1f, 65.2855f, 429.8f, 65.2783f, 430.5f, 65.2894f
path.cubicTo(SkBits2Float(0x43d7999a), SkBits2Float(0x428299de), SkBits2Float(0x43d7f333), SkBits2Float(0x4282ac18), SkBits2Float(0x43d84ccd), SkBits2Float(0x4282b65d));  // 431.2f, 65.3005f, 431.9f, 65.3361f, 432.6f, 65.3562f
path.cubicTo(SkBits2Float(0x43d8a666), SkBits2Float(0x4282c0a1), SkBits2Float(0x43d90000), SkBits2Float(0x4282cceb), SkBits2Float(0x43d9599a), SkBits2Float(0x4282d1c5));  // 433.3f, 65.3762f, 434, 65.4002f, 434.7f, 65.4097f
path.cubicTo(SkBits2Float(0x43d9b333), SkBits2Float(0x4282d69f), SkBits2Float(0x43da0ccd), SkBits2Float(0x4282d3cb), SkBits2Float(0x43da6666), SkBits2Float(0x4282d377));  // 435.4f, 65.4192f, 436.1f, 65.4137f, 436.8f, 65.413f
path.cubicTo(SkBits2Float(0x43dac000), SkBits2Float(0x4282d323), SkBits2Float(0x43db199a), SkBits2Float(0x4282d18a), SkBits2Float(0x43db7333), SkBits2Float(0x4282cfcc));  // 437.5f, 65.4124f, 438.2f, 65.4093f, 438.9f, 65.4059f
path.cubicTo(SkBits2Float(0x43dbcccd), SkBits2Float(0x4282ce0e), SkBits2Float(0x43dc2666), SkBits2Float(0x4282cd41), SkBits2Float(0x43dc8000), SkBits2Float(0x4282c904));  // 439.6f, 65.4025f, 440.3f, 65.4009f, 441, 65.3926f
path.cubicTo(SkBits2Float(0x43dcd99a), SkBits2Float(0x4282c4c7), SkBits2Float(0x43dd3333), SkBits2Float(0x4282bc94), SkBits2Float(0x43dd8ccd), SkBits2Float(0x4282b65d));  // 441.7f, 65.3843f, 442.4f, 65.3683f, 443.1f, 65.3562f
path.cubicTo(SkBits2Float(0x43dde666), SkBits2Float(0x4282b025), SkBits2Float(0x43de4000), SkBits2Float(0x4282a847), SkBits2Float(0x43de999a), SkBits2Float(0x4282a3b5));  // 443.8f, 65.344f, 444.5f, 65.3287f, 445.2f, 65.3197f
path.cubicTo(SkBits2Float(0x43def333), SkBits2Float(0x42829f24), SkBits2Float(0x43df4ccd), SkBits2Float(0x42829c21), SkBits2Float(0x43dfa666), SkBits2Float(0x42829af4));  // 445.9f, 65.3108f, 446.6f, 65.3049f, 447.3f, 65.3026f
path.cubicTo(SkBits2Float(0x43e00000), SkBits2Float(0x428299c7), SkBits2Float(0x43e0599a), SkBits2Float(0x42829af4), SkBits2Float(0x43e0b333), SkBits2Float(0x42829ca6));  // 448, 65.3003f, 448.7f, 65.3026f, 449.4f, 65.306f
path.cubicTo(SkBits2Float(0x43e10ccd), SkBits2Float(0x42829e58), SkBits2Float(0x43e16666), SkBits2Float(0x4282a31a), SkBits2Float(0x43e1c000), SkBits2Float(0x4282a521));  // 450.1f, 65.3093f, 450.8f, 65.3186f, 451.5f, 65.3225f
path.cubicTo(SkBits2Float(0x43e2199a), SkBits2Float(0x4282a727), SkBits2Float(0x43e27333), SkBits2Float(0x4282a640), SkBits2Float(0x43e2cccd), SkBits2Float(0x4282a8cc));  // 452.2f, 65.3265f, 452.9f, 65.3247f, 453.6f, 65.3297f
path.cubicTo(SkBits2Float(0x43e32666), SkBits2Float(0x4282ab57), SkBits2Float(0x43e38000), SkBits2Float(0x4282adef), SkBits2Float(0x43e3d99a), SkBits2Float(0x4282b464));  // 454.3f, 65.3346f, 455, 65.3397f, 455.7f, 65.3523f
path.cubicTo(SkBits2Float(0x43e43333), SkBits2Float(0x4282bad8), SkBits2Float(0x43e48ccd), SkBits2Float(0x4282c468), SkBits2Float(0x43e4e666), SkBits2Float(0x4282cf86));  // 456.4f, 65.3649f, 457.1f, 65.3836f, 457.8f, 65.4053f
path.cubicTo(SkBits2Float(0x43e54000), SkBits2Float(0x4282daa3), SkBits2Float(0x43e5999a), SkBits2Float(0x4282e9b4), SkBits2Float(0x43e5f333), SkBits2Float(0x4282f714));  // 458.5f, 65.427f, 459.2f, 65.4565f, 459.9f, 65.4826f
path.cubicTo(SkBits2Float(0x43e64ccd), SkBits2Float(0x42830474), SkBits2Float(0x43e6a666), SkBits2Float(0x428314c1), SkBits2Float(0x43e70000), SkBits2Float(0x42831fc7));  // 460.6f, 65.5087f, 461.3f, 65.5405f, 462, 65.5621f
path.cubicTo(SkBits2Float(0x43e7599a), SkBits2Float(0x42832acc), SkBits2Float(0x43e7b333), SkBits2Float(0x4283338f), SkBits2Float(0x43e80ccd), SkBits2Float(0x42833936));  // 462.7f, 65.5836f, 463.4f, 65.6007f, 464.1f, 65.6117f
path.cubicTo(SkBits2Float(0x43e86666), SkBits2Float(0x42833edd), SkBits2Float(0x43e8c000), SkBits2Float(0x42834090), SkBits2Float(0x43e9199a), SkBits2Float(0x428341b1));  // 464.8f, 65.6228f, 465.5f, 65.6261f, 466.2f, 65.6283f
path.cubicTo(SkBits2Float(0x43e97333), SkBits2Float(0x428342d2), SkBits2Float(0x43e9cccd), SkBits2Float(0x428340d8), SkBits2Float(0x43ea2666), SkBits2Float(0x42833fff));  // 466.9f, 65.6305f, 467.6f, 65.6266f, 468.3f, 65.625f
path.cubicTo(SkBits2Float(0x43ea8000), SkBits2Float(0x42833f26), SkBits2Float(0x43ead99a), SkBits2Float(0x42833e4d), SkBits2Float(0x43eb3333), SkBits2Float(0x42833c9b));  // 469, 65.6233f, 469.7f, 65.6217f, 470.4f, 65.6184f
path.cubicTo(SkBits2Float(0x43eb8ccd), SkBits2Float(0x42833ae8), SkBits2Float(0x43ebe666), SkBits2Float(0x42833f32), SkBits2Float(0x43ec4000), SkBits2Float(0x428335d2));  // 471.1f, 65.6151f, 471.8f, 65.6234f, 472.5f, 65.6051f
path.cubicTo(SkBits2Float(0x43ec999a), SkBits2Float(0x42832c73), SkBits2Float(0x43ecf333), SkBits2Float(0x42831e2e), SkBits2Float(0x43ed4ccd), SkBits2Float(0x4283045e));  // 473.2f, 65.5868f, 473.9f, 65.5589f, 474.6f, 65.5085f
path.cubicTo(SkBits2Float(0x43eda666), SkBits2Float(0x4282ea8e), SkBits2Float(0x43ee0000), SkBits2Float(0x4282bd9e), SkBits2Float(0x43ee599a), SkBits2Float(0x42829af4));  // 475.3f, 65.4581f, 476, 65.3703f, 476.7f, 65.3026f
path.cubicTo(SkBits2Float(0x43eeb333), SkBits2Float(0x4282784a), SkBits2Float(0x43ef0ccd), SkBits2Float(0x428245ac), SkBits2Float(0x43ef6666), SkBits2Float(0x42823461));  // 477.4f, 65.2349f, 478.1f, 65.1361f, 478.8f, 65.1023f
path.cubicTo(SkBits2Float(0x43efc000), SkBits2Float(0x42822316), SkBits2Float(0x43f0199a), SkBits2Float(0x42823365), SkBits2Float(0x43f07333), SkBits2Float(0x42823333));  // 479.5f, 65.0685f, 480.2f, 65.1004f, 480.9f, 65.1f
path.lineTo(SkBits2Float(0x43f07333), SkBits2Float(0x42823333));  // 480.9f, 65.1f
path.lineTo(SkBits2Float(0x43c24000), SkBits2Float(0x42823333));  // 388.5f, 65.1f
path.close();
path.moveTo(SkBits2Float(0x43fc0000), SkBits2Float(0x42823333));  // 504, 65.1f
path.lineTo(SkBits2Float(0x43fc0000), SkBits2Float(0x42823333));  // 504, 65.1f
path.cubicTo(SkBits2Float(0x43fc599a), SkBits2Float(0x42823333), SkBits2Float(0x43fcb333), SkBits2Float(0x42822ef4), SkBits2Float(0x43fd0ccd), SkBits2Float(0x42823333));  // 504.7f, 65.1f, 505.4f, 65.0917f, 506.1f, 65.1f
path.cubicTo(SkBits2Float(0x43fd6666), SkBits2Float(0x42823772), SkBits2Float(0x43fdc000), SkBits2Float(0x42823e7d), SkBits2Float(0x43fe199a), SkBits2Float(0x42824cac));  // 506.8f, 65.1083f, 507.5f, 65.122f, 508.2f, 65.1497f
path.cubicTo(SkBits2Float(0x43fe7333), SkBits2Float(0x42825adb), SkBits2Float(0x43fecccd), SkBits2Float(0x42827a73), SkBits2Float(0x43ff2666), SkBits2Float(0x4282884d));  // 508.9f, 65.1775f, 509.6f, 65.2392f, 510.3f, 65.2662f
path.cubicTo(SkBits2Float(0x43ff8000), SkBits2Float(0x42829626), SkBits2Float(0x43ffd99a), SkBits2Float(0x42829a88), SkBits2Float(0x4400199a), SkBits2Float(0x42829fc4));  // 511, 65.2933f, 511.7f, 65.3018f, 512.4f, 65.312f
path.cubicTo(SkBits2Float(0x44004666), SkBits2Float(0x4282a4ff), SkBits2Float(0x44007333), SkBits2Float(0x4282a6e4), SkBits2Float(0x4400a000), SkBits2Float(0x4282a7b1));  // 513.1f, 65.3223f, 513.8f, 65.326f, 514.5f, 65.3275f
path.cubicTo(SkBits2Float(0x4400cccd), SkBits2Float(0x4282a87e), SkBits2Float(0x4400f99a), SkBits2Float(0x4282a682), SkBits2Float(0x44012666), SkBits2Float(0x4282a494));  // 515.2f, 65.3291f, 515.9f, 65.3252f, 516.6f, 65.3214f
path.cubicTo(SkBits2Float(0x44015333), SkBits2Float(0x4282a2a5), SkBits2Float(0x44018000), SkBits2Float(0x42829e44), SkBits2Float(0x4401accd), SkBits2Float(0x42829c19));  // 517.3f, 65.3177f, 518, 65.3091f, 518.7f, 65.3049f
path.cubicTo(SkBits2Float(0x4401d99a), SkBits2Float(0x428299ee), SkBits2Float(0x44020666), SkBits2Float(0x42829718), SkBits2Float(0x44023333), SkBits2Float(0x42829790));  // 519.4f, 65.3006f, 520.1f, 65.2951f, 520.8f, 65.296f
path.cubicTo(SkBits2Float(0x44026000), SkBits2Float(0x42829807), SkBits2Float(0x44028ccd), SkBits2Float(0x42829ae5), SkBits2Float(0x4402b99a), SkBits2Float(0x42829ee5));  // 521.5f, 65.2969f, 522.2f, 65.3025f, 522.9f, 65.3103f
path.cubicTo(SkBits2Float(0x4402e666), SkBits2Float(0x4282a2e6), SkBits2Float(0x44031333), SkBits2Float(0x4282abab), SkBits2Float(0x44034000), SkBits2Float(0x4282af94));  // 523.6f, 65.3182f, 524.3f, 65.3353f, 525, 65.3429f
path.cubicTo(SkBits2Float(0x44036ccd), SkBits2Float(0x4282b37d), SkBits2Float(0x4403999a), SkBits2Float(0x4282b5c0), SkBits2Float(0x4403c666), SkBits2Float(0x4282b65d));  // 525.7f, 65.3506f, 526.4f, 65.355f, 527.1f, 65.3562f
path.cubicTo(SkBits2Float(0x4403f333), SkBits2Float(0x4282b6f9), SkBits2Float(0x44042000), SkBits2Float(0x4282c91b), SkBits2Float(0x44044ccd), SkBits2Float(0x4282b33f));  // 527.8f, 65.3574f, 528.5f, 65.3928f, 529.2f, 65.3501f
path.cubicTo(SkBits2Float(0x4404799a), SkBits2Float(0x42829d63), SkBits2Float(0x4404a666), SkBits2Float(0x4282488a), SkBits2Float(0x4404d333), SkBits2Float(0x42823333));  // 529.9f, 65.3074f, 530.6f, 65.1417f, 531.3f, 65.1f
path.cubicTo(SkBits2Float(0x44050000), SkBits2Float(0x42821ddb), SkBits2Float(0x44052ccd), SkBits2Float(0x42823333), SkBits2Float(0x4405599a), SkBits2Float(0x42823333));  // 532, 65.0583f, 532.7f, 65.1f, 533.4f, 65.1f
path.lineTo(SkBits2Float(0x4405599a), SkBits2Float(0x42823333));  // 533.4f, 65.1f
path.lineTo(SkBits2Float(0x43fc0000), SkBits2Float(0x42823333));  // 504, 65.1f
path.close();
    return path;
}

static SkPath path3() {
    SkPath path;
path.moveTo(SkBits2Float(0x42b06666), SkBits2Float(0x42bd0000));  // 88.2f, 94.5f
path.lineTo(SkBits2Float(0x42b06666), SkBits2Float(0x42bd0000));  // 88.2f, 94.5f
path.cubicTo(SkBits2Float(0x42b1cccd), SkBits2Float(0x42bd0000), SkBits2Float(0x42b33333), SkBits2Float(0x42bd2573), SkBits2Float(0x42b4999a), SkBits2Float(0x42bd0000));  // 88.9f, 94.5f, 89.6f, 94.5731f, 90.3f, 94.5f
path.cubicTo(SkBits2Float(0x42b60000), SkBits2Float(0x42bcda8d), SkBits2Float(0x42b76666), SkBits2Float(0x42bc4598), SkBits2Float(0x42b8cccd), SkBits2Float(0x42bc1f4b));  // 91, 94.4269f, 91.7f, 94.1359f, 92.4f, 94.0611f
path.cubicTo(SkBits2Float(0x42ba3333), SkBits2Float(0x42bbf8ff), SkBits2Float(0x42bb999a), SkBits2Float(0x42bc1b9e), SkBits2Float(0x42bd0000), SkBits2Float(0x42bc1a35));  // 93.1f, 93.9863f, 93.8f, 94.0539f, 94.5f, 94.0512f
path.cubicTo(SkBits2Float(0x42be6666), SkBits2Float(0x42bc18cb), SkBits2Float(0x42bfcccd), SkBits2Float(0x42bc17f2), SkBits2Float(0x42c13333), SkBits2Float(0x42bc16d0));  // 95.2f, 94.0484f, 95.9f, 94.0468f, 96.6f, 94.0446f
path.cubicTo(SkBits2Float(0x42c2999a), SkBits2Float(0x42bc15af), SkBits2Float(0x42c40000), SkBits2Float(0x42bc13fd), SkBits2Float(0x42c56666), SkBits2Float(0x42bc136c));  // 97.3f, 94.0424f, 98, 94.039f, 98.7f, 94.0379f
path.cubicTo(SkBits2Float(0x42c6cccd), SkBits2Float(0x42bc12dc), SkBits2Float(0x42c83333), SkBits2Float(0x42bc12e7), SkBits2Float(0x42c9999a), SkBits2Float(0x42bc136c));  // 99.4f, 94.0368f, 100.1f, 94.0369f, 100.8f, 94.0379f
path.cubicTo(SkBits2Float(0x42cb0000), SkBits2Float(0x42bc13f1), SkBits2Float(0x42cc6666), SkBits2Float(0x42bc15c8), SkBits2Float(0x42cdcccd), SkBits2Float(0x42bc168a));  // 101.5f, 94.0389f, 102.2f, 94.0425f, 102.9f, 94.044f
path.cubicTo(SkBits2Float(0x42cf3333), SkBits2Float(0x42bc174b), SkBits2Float(0x42d0999a), SkBits2Float(0x42bc17ad), SkBits2Float(0x42d20000), SkBits2Float(0x42bc17f5));  // 103.6f, 94.0455f, 104.3f, 94.0462f, 105, 94.0468f
path.cubicTo(SkBits2Float(0x42d36666), SkBits2Float(0x42bc183e), SkBits2Float(0x42d4cccd), SkBits2Float(0x42bc162a), SkBits2Float(0x42d63333), SkBits2Float(0x42bc183c));  // 105.7f, 94.0473f, 106.4f, 94.0433f, 107.1f, 94.0473f
path.cubicTo(SkBits2Float(0x42d7999a), SkBits2Float(0x42bc1a4e), SkBits2Float(0x42d90000), SkBits2Float(0x42bc1e66), SkBits2Float(0x42da6666), SkBits2Float(0x42bc2461));  // 107.8f, 94.0514f, 108.5f, 94.0594f, 109.2f, 94.0711f
path.cubicTo(SkBits2Float(0x42dbcccd), SkBits2Float(0x42bc2a5d), SkBits2Float(0x42dd3333), SkBits2Float(0x42bc33f9), SkBits2Float(0x42de999a), SkBits2Float(0x42bc3c1f));  // 109.9f, 94.0827f, 110.6f, 94.1015f, 111.3f, 94.1174f
path.cubicTo(SkBits2Float(0x42e00000), SkBits2Float(0x42bc4446), SkBits2Float(0x42e16666), SkBits2Float(0x42bc4cce), SkBits2Float(0x42e2cccd), SkBits2Float(0x42bc5548));  // 112, 94.1333f, 112.7f, 94.15f, 113.4f, 94.1666f
path.cubicTo(SkBits2Float(0x42e43333), SkBits2Float(0x42bc5dc3), SkBits2Float(0x42e5999a), SkBits2Float(0x42bc6472), SkBits2Float(0x42e70000), SkBits2Float(0x42bc6eff));  // 114.1f, 94.1831f, 114.8f, 94.1962f, 115.5f, 94.2168f
path.cubicTo(SkBits2Float(0x42e86666), SkBits2Float(0x42bc798b), SkBits2Float(0x42e9cccd), SkBits2Float(0x42bc8607), SkBits2Float(0x42eb3333), SkBits2Float(0x42bc9494));  // 116.2f, 94.2374f, 116.9f, 94.2618f, 117.6f, 94.2902f
path.cubicTo(SkBits2Float(0x42ec999a), SkBits2Float(0x42bca321), SkBits2Float(0x42ee0000), SkBits2Float(0x42bcb9f8), SkBits2Float(0x42ef6666), SkBits2Float(0x42bcc64f));  // 118.3f, 94.3186f, 119, 94.3632f, 119.7f, 94.3873f
path.cubicTo(SkBits2Float(0x42f0cccd), SkBits2Float(0x42bcd2a5), SkBits2Float(0x42f23333), SkBits2Float(0x42bcdb35), SkBits2Float(0x42f3999a), SkBits2Float(0x42bcde9a));  // 120.4f, 94.4114f, 121.1f, 94.4281f, 121.8f, 94.4348f
path.cubicTo(SkBits2Float(0x42f50000), SkBits2Float(0x42bce1fe), SkBits2Float(0x42f66666), SkBits2Float(0x42bcdd3f), SkBits2Float(0x42f7cccd), SkBits2Float(0x42bcdaa8));  // 122.5f, 94.4414f, 123.2f, 94.4321f, 123.9f, 94.4271f
path.cubicTo(SkBits2Float(0x42f93333), SkBits2Float(0x42bcd811), SkBits2Float(0x42fa999a), SkBits2Float(0x42bcd25d), SkBits2Float(0x42fc0000), SkBits2Float(0x42bccf10));  // 124.6f, 94.422f, 125.3f, 94.4109f, 126, 94.4044f
path.cubicTo(SkBits2Float(0x42fd6666), SkBits2Float(0x42bccbc3), SkBits2Float(0x42fecccd), SkBits2Float(0x42bcc95b), SkBits2Float(0x4300199a), SkBits2Float(0x42bcc6dc));  // 126.7f, 94.398f, 127.4f, 94.3933f, 128.1f, 94.3884f
path.cubicTo(SkBits2Float(0x4300cccd), SkBits2Float(0x42bcc45c), SkBits2Float(0x43018000), SkBits2Float(0x42bcc0ec), SkBits2Float(0x43023333), SkBits2Float(0x42bcc013));  // 128.8f, 94.3835f, 129.5f, 94.3768f, 130.2f, 94.3751f
path.cubicTo(SkBits2Float(0x4302e666), SkBits2Float(0x42bcbf3a), SkBits2Float(0x4303999a), SkBits2Float(0x42bcc0b0), SkBits2Float(0x43044ccd), SkBits2Float(0x42bcc1c5));  // 130.9f, 94.3735f, 131.6f, 94.3763f, 132.3f, 94.3785f
path.cubicTo(SkBits2Float(0x43050000), SkBits2Float(0x42bcc2db), SkBits2Float(0x4305b333), SkBits2Float(0x42bcc5bc), SkBits2Float(0x43066666), SkBits2Float(0x42bcc695));  // 133, 94.3806f, 133.7f, 94.3862f, 134.4f, 94.3879f
path.cubicTo(SkBits2Float(0x4307199a), SkBits2Float(0x42bcc76e), SkBits2Float(0x4307cccd), SkBits2Float(0x42bcc688), SkBits2Float(0x43088000), SkBits2Float(0x42bcc6dc));  // 135.1f, 94.3895f, 135.8f, 94.3878f, 136.5f, 94.3884f
path.cubicTo(SkBits2Float(0x43093333), SkBits2Float(0x42bcc730), SkBits2Float(0x4309e666), SkBits2Float(0x42bcc89a), SkBits2Float(0x430a999a), SkBits2Float(0x42bcc88e));  // 137.2f, 94.389f, 137.9f, 94.3918f, 138.6f, 94.3917f
path.cubicTo(SkBits2Float(0x430b4ccd), SkBits2Float(0x42bcc882), SkBits2Float(0x430c0000), SkBits2Float(0x42bcc76e), SkBits2Float(0x430cb333), SkBits2Float(0x42bcc695));  // 139.3f, 94.3916f, 140, 94.3895f, 140.7f, 94.3879f
path.cubicTo(SkBits2Float(0x430d6666), SkBits2Float(0x42bcc5bc), SkBits2Float(0x430e199a), SkBits2Float(0x42bcc445), SkBits2Float(0x430ecccd), SkBits2Float(0x42bcc378));  // 141.4f, 94.3862f, 142.1f, 94.3833f, 142.8f, 94.3818f
path.cubicTo(SkBits2Float(0x430f8000), SkBits2Float(0x42bcc2aa), SkBits2Float(0x43103333), SkBits2Float(0x42bcc32f), SkBits2Float(0x4310e666), SkBits2Float(0x42bcc1c5));  // 143.5f, 94.3802f, 144.2f, 94.3812f, 144.9f, 94.3785f
path.cubicTo(SkBits2Float(0x4311999a), SkBits2Float(0x42bcc05c), SkBits2Float(0x43124ccd), SkBits2Float(0x42bcbd88), SkBits2Float(0x43130000), SkBits2Float(0x42bcbafd));  // 145.6f, 94.3757f, 146.3f, 94.3702f, 147, 94.3652f
path.cubicTo(SkBits2Float(0x4313b333), SkBits2Float(0x42bcb872), SkBits2Float(0x43146666), SkBits2Float(0x42bcb50e), SkBits2Float(0x4315199a), SkBits2Float(0x42bcb282));  // 147.7f, 94.3602f, 148.4f, 94.3536f, 149.1f, 94.3486f
path.cubicTo(SkBits2Float(0x4315cccd), SkBits2Float(0x42bcaff7), SkBits2Float(0x43168000), SkBits2Float(0x42bcac56), SkBits2Float(0x43173333), SkBits2Float(0x42bcabba));  // 149.8f, 94.3437f, 150.5f, 94.3366f, 151.2f, 94.3354f
path.cubicTo(SkBits2Float(0x4317e666), SkBits2Float(0x42bcab1d), SkBits2Float(0x4318999a), SkBits2Float(0x42bcadfe), SkBits2Float(0x43194ccd), SkBits2Float(0x42bcaed8));  // 151.9f, 94.3342f, 152.6f, 94.3398f, 153.3f, 94.3415f
path.cubicTo(SkBits2Float(0x431a0000), SkBits2Float(0x42bcafb1), SkBits2Float(0x431ab333), SkBits2Float(0x42bcb034), SkBits2Float(0x431b6666), SkBits2Float(0x42bcb0d0));  // 154, 94.3431f, 154.7f, 94.3441f, 155.4f, 94.3453f
path.cubicTo(SkBits2Float(0x431c199a), SkBits2Float(0x42bcb16d), SkBits2Float(0x431ccccd), SkBits2Float(0x42bcb119), SkBits2Float(0x431d8000), SkBits2Float(0x42bcb282));  // 156.1f, 94.3465f, 156.8f, 94.3459f, 157.5f, 94.3486f
path.cubicTo(SkBits2Float(0x431e3333), SkBits2Float(0x42bcb3ec), SkBits2Float(0x431ee666), SkBits2Float(0x42bcb708), SkBits2Float(0x431f999a), SkBits2Float(0x42bcb94b));  // 158.2f, 94.3514f, 158.9f, 94.3575f, 159.6f, 94.3619f
path.cubicTo(SkBits2Float(0x43204ccd), SkBits2Float(0x42bcbb8e), SkBits2Float(0x43210000), SkBits2Float(0x42bcbef2), SkBits2Float(0x4321b333), SkBits2Float(0x42bcc013));  // 160.3f, 94.3663f, 161, 94.3729f, 161.7f, 94.3751f
path.cubicTo(SkBits2Float(0x43226666), SkBits2Float(0x42bcc135), SkBits2Float(0x4323199a), SkBits2Float(0x42bcbfe3), SkBits2Float(0x4323cccd), SkBits2Float(0x42bcc013));  // 162.4f, 94.3774f, 163.1f, 94.3748f, 163.8f, 94.3751f
path.cubicTo(SkBits2Float(0x43248000), SkBits2Float(0x42bcc044), SkBits2Float(0x43253333), SkBits2Float(0x42bcbf19), SkBits2Float(0x4325e666), SkBits2Float(0x42bcc138));  // 164.5f, 94.3755f, 165.2f, 94.3732f, 165.9f, 94.3774f
path.cubicTo(SkBits2Float(0x4326999a), SkBits2Float(0x42bcc358), SkBits2Float(0x43274ccd), SkBits2Float(0x42bcc7c6), SkBits2Float(0x43280000), SkBits2Float(0x42bcccd1));  // 166.6f, 94.3815f, 167.3f, 94.3902f, 168, 94.4f
path.cubicTo(SkBits2Float(0x4328b333), SkBits2Float(0x42bcd1db), SkBits2Float(0x43296666), SkBits2Float(0x42bcd9b9), SkBits2Float(0x432a199a), SkBits2Float(0x42bcdf78));  // 168.7f, 94.4099f, 169.4f, 94.4252f, 170.1f, 94.4365f
path.cubicTo(SkBits2Float(0x432acccd), SkBits2Float(0x42bce536), SkBits2Float(0x432b8000), SkBits2Float(0x42bceb84), SkBits2Float(0x432c3333), SkBits2Float(0x42bcef48));  // 170.8f, 94.4477f, 171.5f, 94.46f, 172.2f, 94.4673f
path.cubicTo(SkBits2Float(0x432ce666), SkBits2Float(0x42bcf30c), SkBits2Float(0x432d999a), SkBits2Float(0x42bcf3ce), SkBits2Float(0x432e4ccd), SkBits2Float(0x42bcf611));  // 172.9f, 94.4747f, 173.6f, 94.4762f, 174.3f, 94.4806f
path.cubicTo(SkBits2Float(0x432f0000), SkBits2Float(0x42bcf853), SkBits2Float(0x432fb333), SkBits2Float(0x42bcfb31), SkBits2Float(0x43306666), SkBits2Float(0x42bcfcd9));  // 175, 94.485f, 175.7f, 94.4906f, 176.4f, 94.4938f
path.lineTo(SkBits2Float(0x43306666), SkBits2Float(0x42bd0000));  // 176.4f, 94.5f
path.lineTo(SkBits2Float(0x42b06666), SkBits2Float(0x42bd0000));  // 88.2f, 94.5f
path.close();
path.moveTo(SkBits2Float(0x43413333), SkBits2Float(0x42bd0000));  // 193.2f, 94.5f
path.lineTo(SkBits2Float(0x43413333), SkBits2Float(0x42bd0000));  // 193.2f, 94.5f
path.cubicTo(SkBits2Float(0x4341e666), SkBits2Float(0x42bd0000), SkBits2Float(0x4342999a), SkBits2Float(0x42bd0549), SkBits2Float(0x43434ccd), SkBits2Float(0x42bd0000));  // 193.9f, 94.5f, 194.6f, 94.5103f, 195.3f, 94.5f
path.cubicTo(SkBits2Float(0x43440000), SkBits2Float(0x42bcfab7), SkBits2Float(0x4344b333), SkBits2Float(0x42bcec20), SkBits2Float(0x43456666), SkBits2Float(0x42bce04c));  // 196, 94.4897f, 196.7f, 94.4612f, 197.4f, 94.4381f
path.cubicTo(SkBits2Float(0x4346199a), SkBits2Float(0x42bcd477), SkBits2Float(0x4346cccd), SkBits2Float(0x42bcc14e), SkBits2Float(0x43478000), SkBits2Float(0x42bcb904));  // 198.1f, 94.415f, 198.8f, 94.3775f, 199.5f, 94.3614f
path.cubicTo(SkBits2Float(0x43483333), SkBits2Float(0x42bcb0ba), SkBits2Float(0x4348e666), SkBits2Float(0x42bcaed9), SkBits2Float(0x4349999a), SkBits2Float(0x42bcae91));  // 200.2f, 94.3452f, 200.9f, 94.3415f, 201.6f, 94.341f
path.cubicTo(SkBits2Float(0x434a4ccd), SkBits2Float(0x42bcae49), SkBits2Float(0x434b0000), SkBits2Float(0x42bcb23c), SkBits2Float(0x434bb333), SkBits2Float(0x42bcb752));  // 202.3f, 94.3404f, 203, 94.3481f, 203.7f, 94.358f
path.cubicTo(SkBits2Float(0x434c6666), SkBits2Float(0x42bcbc69), SkBits2Float(0x434d199a), SkBits2Float(0x42bcc612), SkBits2Float(0x434dcccd), SkBits2Float(0x42bccd17));  // 204.4f, 94.368f, 205.1f, 94.3869f, 205.8f, 94.4006f
path.cubicTo(SkBits2Float(0x434e8000), SkBits2Float(0x42bcd41c), SkBits2Float(0x434f3333), SkBits2Float(0x42bcdc5a), SkBits2Float(0x434fe666), SkBits2Float(0x42bce171));  // 206.5f, 94.4143f, 207.2f, 94.4304f, 207.9f, 94.4403f
path.cubicTo(SkBits2Float(0x4350999a), SkBits2Float(0x42bce687), SkBits2Float(0x43514ccd), SkBits2Float(0x42bceb0d), SkBits2Float(0x43520000), SkBits2Float(0x42bceb9d));  // 208.6f, 94.4502f, 209.3f, 94.4591f, 210, 94.4602f
path.cubicTo(SkBits2Float(0x4352b333), SkBits2Float(0x42bcec2e), SkBits2Float(0x43536666), SkBits2Float(0x42bcea70), SkBits2Float(0x4354199a), SkBits2Float(0x42bce4d5));  // 210.7f, 94.4613f, 211.4f, 94.4579f, 212.1f, 94.4469f
path.cubicTo(SkBits2Float(0x4354cccd), SkBits2Float(0x42bcdf39), SkBits2Float(0x43558000), SkBits2Float(0x42bcd432), SkBits2Float(0x43563333), SkBits2Float(0x42bcc9f9));  // 212.8f, 94.436f, 213.5f, 94.4144f, 214.2f, 94.3945f
path.cubicTo(SkBits2Float(0x4356e666), SkBits2Float(0x42bcbfc1), SkBits2Float(0x4357999a), SkBits2Float(0x42bcb9bc), SkBits2Float(0x43584ccd), SkBits2Float(0x42bca782));  // 214.9f, 94.3745f, 215.6f, 94.3628f, 216.3f, 94.3272f
path.cubicTo(SkBits2Float(0x43590000), SkBits2Float(0x42bc9548), SkBits2Float(0x4359b333), SkBits2Float(0x42bc76fe), SkBits2Float(0x435a6666), SkBits2Float(0x42bc5c9e));  // 217, 94.2916f, 217.7f, 94.2324f, 218.4f, 94.1809f
path.cubicTo(SkBits2Float(0x435b199a), SkBits2Float(0x42bc423e), SkBits2Float(0x435bcccd), SkBits2Float(0x42bc19b0), SkBits2Float(0x435c8000), SkBits2Float(0x42bc0940));  // 219.1f, 94.1294f, 219.8f, 94.0502f, 220.5f, 94.0181f
path.cubicTo(SkBits2Float(0x435d3333), SkBits2Float(0x42bbf8cf), SkBits2Float(0x435de666), SkBits2Float(0x42bbfbf7), SkBits2Float(0x435e999a), SkBits2Float(0x42bbf9fc));  // 221.2f, 93.986f, 221.9f, 93.9921f, 222.6f, 93.9883f
path.cubicTo(SkBits2Float(0x435f4ccd), SkBits2Float(0x42bbf802), SkBits2Float(0x43600000), SkBits2Float(0x42bbfad6), SkBits2Float(0x4360b333), SkBits2Float(0x42bbfd61));  // 223.3f, 93.9844f, 224, 93.9899f, 224.7f, 93.9949f
path.cubicTo(SkBits2Float(0x43616666), SkBits2Float(0x42bbffec), SkBits2Float(0x4362199a), SkBits2Float(0x42bc06fd), SkBits2Float(0x4362cccd), SkBits2Float(0x42bc0940));  // 225.4f, 93.9998f, 226.1f, 94.0136f, 226.8f, 94.0181f
path.cubicTo(SkBits2Float(0x43638000), SkBits2Float(0x42bc0b82), SkBits2Float(0x43643333), SkBits2Float(0x42bc0c13), SkBits2Float(0x4364e666), SkBits2Float(0x42bc0af2));  // 227.5f, 94.0225f, 228.2f, 94.0236f, 228.9f, 94.0214f
path.cubicTo(SkBits2Float(0x4365999a), SkBits2Float(0x42bc09d0), SkBits2Float(0x43664ccd), SkBits2Float(0x42bc019e), SkBits2Float(0x43670000), SkBits2Float(0x42bc0277));  // 229.6f, 94.0192f, 230.3f, 94.0032f, 231, 94.0048f
path.cubicTo(SkBits2Float(0x4367b333), SkBits2Float(0x42bc0350), SkBits2Float(0x43686666), SkBits2Float(0x42bc0b3a), SkBits2Float(0x4369199a), SkBits2Float(0x42bc1008));  // 231.7f, 94.0065f, 232.4f, 94.0219f, 233.1f, 94.0313f
path.cubicTo(SkBits2Float(0x4369cccd), SkBits2Float(0x42bc14d6), SkBits2Float(0x436a8000), SkBits2Float(0x42bc1d08), SkBits2Float(0x436b3333), SkBits2Float(0x42bc1f4b));  // 233.8f, 94.0407f, 234.5f, 94.0567f, 235.2f, 94.0611f
path.cubicTo(SkBits2Float(0x436be666), SkBits2Float(0x42bc218e), SkBits2Float(0x436c999a), SkBits2Float(0x42bc1e2a), SkBits2Float(0x436d4ccd), SkBits2Float(0x42bc1d99));  // 235.9f, 94.0655f, 236.6f, 94.0589f, 237.3f, 94.0578f
path.cubicTo(SkBits2Float(0x436e0000), SkBits2Float(0x42bc1d08), SkBits2Float(0x436eb333), SkBits2Float(0x42bc1cc0), SkBits2Float(0x436f6666), SkBits2Float(0x42bc1be7));  // 238, 94.0567f, 238.7f, 94.0562f, 239.4f, 94.0545f
path.cubicTo(SkBits2Float(0x4370199a), SkBits2Float(0x42bc1b0e), SkBits2Float(0x4370cccd), SkBits2Float(0x42bc195c), SkBits2Float(0x43718000), SkBits2Float(0x42bc1883));  // 240.1f, 94.0528f, 240.8f, 94.0495f, 241.5f, 94.0479f
path.cubicTo(SkBits2Float(0x43723333), SkBits2Float(0x42bc17aa), SkBits2Float(0x4372e666), SkBits2Float(0x42bc1719), SkBits2Float(0x4373999a), SkBits2Float(0x42bc16d0));  // 242.2f, 94.0462f, 242.9f, 94.0451f, 243.6f, 94.0446f
path.cubicTo(SkBits2Float(0x43744ccd), SkBits2Float(0x42bc1688), SkBits2Float(0x43750000), SkBits2Float(0x42bc1719), SkBits2Float(0x4375b333), SkBits2Float(0x42bc16d0));  // 244.3f, 94.044f, 245, 94.0451f, 245.7f, 94.0446f
path.cubicTo(SkBits2Float(0x43766666), SkBits2Float(0x42bc1688), SkBits2Float(0x4377199a), SkBits2Float(0x42bc1567), SkBits2Float(0x4377cccd), SkBits2Float(0x42bc151e));  // 246.4f, 94.044f, 247.1f, 94.0418f, 247.8f, 94.0412f
path.cubicTo(SkBits2Float(0x43788000), SkBits2Float(0x42bc14d6), SkBits2Float(0x43793333), SkBits2Float(0x42bc148e), SkBits2Float(0x4379e666), SkBits2Float(0x42bc151e));  // 248.5f, 94.0407f, 249.2f, 94.0401f, 249.9f, 94.0412f
path.cubicTo(SkBits2Float(0x437a999a), SkBits2Float(0x42bc15af), SkBits2Float(0x437b4ccd), SkBits2Float(0x42bc1761), SkBits2Float(0x437c0000), SkBits2Float(0x42bc1883));  // 250.6f, 94.0424f, 251.3f, 94.0457f, 252, 94.0479f
path.cubicTo(SkBits2Float(0x437cb333), SkBits2Float(0x42bc19a4), SkBits2Float(0x437d6666), SkBits2Float(0x42bc1b0e), SkBits2Float(0x437e199a), SkBits2Float(0x42bc1be7));  // 252.7f, 94.0501f, 253.4f, 94.0528f, 254.1f, 94.0545f
path.cubicTo(SkBits2Float(0x437ecccd), SkBits2Float(0x42bc1cc0), SkBits2Float(0x437f8000), SkBits2Float(0x42bc1d99), SkBits2Float(0x4380199a), SkBits2Float(0x42bc1d99));  // 254.8f, 94.0562f, 255.5f, 94.0578f, 256.2f, 94.0578f
path.cubicTo(SkBits2Float(0x43807333), SkBits2Float(0x42bc1d99), SkBits2Float(0x4380cccd), SkBits2Float(0x42bc1d08), SkBits2Float(0x43812666), SkBits2Float(0x42bc1be7));  // 256.9f, 94.0578f, 257.6f, 94.0567f, 258.3f, 94.0545f
path.cubicTo(SkBits2Float(0x43818000), SkBits2Float(0x42bc1ac5), SkBits2Float(0x4381d99a), SkBits2Float(0x42bc183a), SkBits2Float(0x43823333), SkBits2Float(0x42bc16d0));  // 259, 94.0523f, 259.7f, 94.0473f, 260.4f, 94.0446f
path.cubicTo(SkBits2Float(0x43828ccd), SkBits2Float(0x42bc1567), SkBits2Float(0x4382e666), SkBits2Float(0x42bc13fd), SkBits2Float(0x43834000), SkBits2Float(0x42bc136c));  // 261.1f, 94.0418f, 261.8f, 94.039f, 262.5f, 94.0379f
path.cubicTo(SkBits2Float(0x4383999a), SkBits2Float(0x42bc12dc), SkBits2Float(0x4383f333), SkBits2Float(0x42bc1324), SkBits2Float(0x43844ccd), SkBits2Float(0x42bc136c));  // 263.2f, 94.0368f, 263.9f, 94.0374f, 264.6f, 94.0379f
path.cubicTo(SkBits2Float(0x4384a666), SkBits2Float(0x42bc13b5), SkBits2Float(0x43850000), SkBits2Float(0x42bc148e), SkBits2Float(0x4385599a), SkBits2Float(0x42bc151e));  // 265.3f, 94.0385f, 266, 94.0401f, 266.7f, 94.0412f
path.cubicTo(SkBits2Float(0x4385b333), SkBits2Float(0x42bc15af), SkBits2Float(0x43860ccd), SkBits2Float(0x42bc1688), SkBits2Float(0x43866666), SkBits2Float(0x42bc16d0));  // 267.4f, 94.0424f, 268.1f, 94.044f, 268.8f, 94.0446f
path.cubicTo(SkBits2Float(0x4386c000), SkBits2Float(0x42bc1719), SkBits2Float(0x4387199a), SkBits2Float(0x42bc1719), SkBits2Float(0x43877333), SkBits2Float(0x42bc16d0));  // 269.5f, 94.0451f, 270.2f, 94.0451f, 270.9f, 94.0446f
path.cubicTo(SkBits2Float(0x4387cccd), SkBits2Float(0x42bc1688), SkBits2Float(0x43882666), SkBits2Float(0x42bc1567), SkBits2Float(0x43888000), SkBits2Float(0x42bc151e));  // 271.6f, 94.044f, 272.3f, 94.0418f, 273, 94.0412f
path.cubicTo(SkBits2Float(0x4388d99a), SkBits2Float(0x42bc14d6), SkBits2Float(0x43893333), SkBits2Float(0x42bc151e), SkBits2Float(0x43898ccd), SkBits2Float(0x42bc151e));  // 273.7f, 94.0407f, 274.4f, 94.0412f, 275.1f, 94.0412f
path.cubicTo(SkBits2Float(0x4389e666), SkBits2Float(0x42bc151e), SkBits2Float(0x438a4000), SkBits2Float(0x42bc1567), SkBits2Float(0x438a999a), SkBits2Float(0x42bc151e));  // 275.8f, 94.0412f, 276.5f, 94.0418f, 277.2f, 94.0412f
path.cubicTo(SkBits2Float(0x438af333), SkBits2Float(0x42bc14d6), SkBits2Float(0x438b4ccd), SkBits2Float(0x42bc136c), SkBits2Float(0x438ba666), SkBits2Float(0x42bc136c));  // 277.9f, 94.0407f, 278.6f, 94.0379f, 279.3f, 94.0379f
path.cubicTo(SkBits2Float(0x438c0000), SkBits2Float(0x42bc136c), SkBits2Float(0x438c599a), SkBits2Float(0x42bc152a), SkBits2Float(0x438cb333), SkBits2Float(0x42bc151e));  // 280, 94.0379f, 280.7f, 94.0413f, 281.4f, 94.0412f
path.cubicTo(SkBits2Float(0x438d0ccd), SkBits2Float(0x42bc1513), SkBits2Float(0x438d6666), SkBits2Float(0x42bc14ef), SkBits2Float(0x438dc000), SkBits2Float(0x42bc1326));  // 282.1f, 94.0412f, 282.8f, 94.0409f, 283.5f, 94.0374f
path.cubicTo(SkBits2Float(0x438e199a), SkBits2Float(0x42bc115c), SkBits2Float(0x438e7333), SkBits2Float(0x42bc0c17), SkBits2Float(0x438ecccd), SkBits2Float(0x42bc0a64));  // 284.2f, 94.0339f, 284.9f, 94.0236f, 285.6f, 94.0203f
path.cubicTo(SkBits2Float(0x438f2666), SkBits2Float(0x42bc08b2), SkBits2Float(0x438f8000), SkBits2Float(0x42bc0413), SkBits2Float(0x438fd99a), SkBits2Float(0x42bc08f9));  // 286.3f, 94.017f, 287, 94.008f, 287.7f, 94.0175f
path.cubicTo(SkBits2Float(0x43903333), SkBits2Float(0x42bc0dde), SkBits2Float(0x43908ccd), SkBits2Float(0x42bc1476), SkBits2Float(0x4390e666), SkBits2Float(0x42bc27c6));  // 288.4f, 94.0271f, 289.1f, 94.04f, 289.8f, 94.0777f
path.cubicTo(SkBits2Float(0x43914000), SkBits2Float(0x42bc3b15), SkBits2Float(0x4391999a), SkBits2Float(0x42bc5916), SkBits2Float(0x4391f333), SkBits2Float(0x42bc7cd6));  // 290.5f, 94.1154f, 291.2f, 94.174f, 291.9f, 94.2438f
path.cubicTo(SkBits2Float(0x43924ccd), SkBits2Float(0x42bca096), SkBits2Float(0x4392a666), SkBits2Float(0x42bce868), SkBits2Float(0x43930000), SkBits2Float(0x42bcfe45));  // 292.6f, 94.3136f, 293.3f, 94.4539f, 294, 94.4966f
path.cubicTo(SkBits2Float(0x4393599a), SkBits2Float(0x42bd1421), SkBits2Float(0x4393b333), SkBits2Float(0x42bcffb6), SkBits2Float(0x43940ccd), SkBits2Float(0x42bd0000));  // 294.7f, 94.5393f, 295.4f, 94.4994f, 296.1f, 94.5f
path.lineTo(SkBits2Float(0x43940ccd), SkBits2Float(0x42bd0000));  // 296.1f, 94.5f
path.lineTo(SkBits2Float(0x43413333), SkBits2Float(0x42bd0000));  // 193.2f, 94.5f
path.close();
path.moveTo(SkBits2Float(0x43ac3333), SkBits2Float(0x42bd0000));  // 344.4f, 94.5f
path.lineTo(SkBits2Float(0x43ac3333), SkBits2Float(0x42bd0000));  // 344.4f, 94.5f
path.cubicTo(SkBits2Float(0x43ac8ccd), SkBits2Float(0x42bd0000), SkBits2Float(0x43ace666), SkBits2Float(0x42bd03a2), SkBits2Float(0x43ad4000), SkBits2Float(0x42bd0000));  // 345.1f, 94.5f, 345.8f, 94.5071f, 346.5f, 94.5f
path.cubicTo(SkBits2Float(0x43ad999a), SkBits2Float(0x42bcfc5e), SkBits2Float(0x43adf333), SkBits2Float(0x42bd069a), SkBits2Float(0x43ae4ccd), SkBits2Float(0x42bcea32));  // 347.2f, 94.4929f, 347.9f, 94.5129f, 348.6f, 94.4574f
path.cubicTo(SkBits2Float(0x43aea666), SkBits2Float(0x42bccdca), SkBits2Float(0x43af0000), SkBits2Float(0x42bc7b18), SkBits2Float(0x43af599a), SkBits2Float(0x42bc558f));  // 349.3f, 94.4019f, 350, 94.2404f, 350.7f, 94.1671f
path.cubicTo(SkBits2Float(0x43afb333), SkBits2Float(0x42bc3005), SkBits2Float(0x43b00ccd), SkBits2Float(0x42bc16d2), SkBits2Float(0x43b06666), SkBits2Float(0x42bc08f9));  // 351.4f, 94.0938f, 352.1f, 94.0446f, 352.8f, 94.0175f
path.cubicTo(SkBits2Float(0x43b0c000), SkBits2Float(0x42bbfb20), SkBits2Float(0x43b1199a), SkBits2Float(0x42bc03d5), SkBits2Float(0x43b17333), SkBits2Float(0x42bc0277));  // 353.5f, 93.9905f, 354.2f, 94.0075f, 354.9f, 94.0048f
path.cubicTo(SkBits2Float(0x43b1cccd), SkBits2Float(0x42bc0119), SkBits2Float(0x43b22666), SkBits2Float(0x42bc019e), SkBits2Float(0x43b28000), SkBits2Float(0x42bc00c5));  // 355.6f, 94.0021f, 356.3f, 94.0032f, 357, 94.0015f
path.cubicTo(SkBits2Float(0x43b2d99a), SkBits2Float(0x42bbffec), SkBits2Float(0x43b33333), SkBits2Float(0x42bbfdf1), SkBits2Float(0x43b38ccd), SkBits2Float(0x42bbfd61));  // 357.7f, 93.9998f, 358.4f, 93.996f, 359.1f, 93.9949f
path.cubicTo(SkBits2Float(0x43b3e666), SkBits2Float(0x42bbfcd0), SkBits2Float(0x43b44000), SkBits2Float(0x42bbfdf1), SkBits2Float(0x43b4999a), SkBits2Float(0x42bbfd61));  // 359.8f, 93.9938f, 360.5f, 93.996f, 361.2f, 93.9949f
path.cubicTo(SkBits2Float(0x43b4f333), SkBits2Float(0x42bbfcd0), SkBits2Float(0x43b54ccd), SkBits2Float(0x42bbfad6), SkBits2Float(0x43b5a666), SkBits2Float(0x42bbf9fc));  // 361.9f, 93.9938f, 362.6f, 93.9899f, 363.3f, 93.9883f
path.cubicTo(SkBits2Float(0x43b60000), SkBits2Float(0x42bbf923), SkBits2Float(0x43b6599a), SkBits2Float(0x42bbf8db), SkBits2Float(0x43b6b333), SkBits2Float(0x42bbf84a));  // 364, 93.9866f, 364.7f, 93.986f, 365.4f, 93.9849f
path.cubicTo(SkBits2Float(0x43b70ccd), SkBits2Float(0x42bbf7ba), SkBits2Float(0x43b76666), SkBits2Float(0x42bbf729), SkBits2Float(0x43b7c000), SkBits2Float(0x42bbf698));  // 366.1f, 93.9838f, 366.8f, 93.9827f, 367.5f, 93.9816f
path.cubicTo(SkBits2Float(0x43b8199a), SkBits2Float(0x42bbf608), SkBits2Float(0x43b87333), SkBits2Float(0x42bbf52e), SkBits2Float(0x43b8cccd), SkBits2Float(0x42bbf4e6));  // 368.2f, 93.9805f, 368.9f, 93.9789f, 369.6f, 93.9783f
path.cubicTo(SkBits2Float(0x43b92666), SkBits2Float(0x42bbf49e), SkBits2Float(0x43b98000), SkBits2Float(0x42bbf455), SkBits2Float(0x43b9d99a), SkBits2Float(0x42bbf4e6));  // 370.3f, 93.9778f, 371, 93.9772f, 371.7f, 93.9783f
path.cubicTo(SkBits2Float(0x43ba3333), SkBits2Float(0x42bbf577), SkBits2Float(0x43ba8ccd), SkBits2Float(0x42bbf771), SkBits2Float(0x43bae666), SkBits2Float(0x42bbf84a));  // 372.4f, 93.9794f, 373.1f, 93.9833f, 373.8f, 93.9849f
path.cubicTo(SkBits2Float(0x43bb4000), SkBits2Float(0x42bbf923), SkBits2Float(0x43bb999a), SkBits2Float(0x42bbf9b4), SkBits2Float(0x43bbf333), SkBits2Float(0x42bbf9fc));  // 374.5f, 93.9866f, 375.2f, 93.9877f, 375.9f, 93.9883f
path.cubicTo(SkBits2Float(0x43bc4ccd), SkBits2Float(0x42bbfa45), SkBits2Float(0x43bca666), SkBits2Float(0x42bbf9fc), SkBits2Float(0x43bd0000), SkBits2Float(0x42bbf9fc));  // 376.6f, 93.9888f, 377.3f, 93.9883f, 378, 93.9883f
path.cubicTo(SkBits2Float(0x43bd599a), SkBits2Float(0x42bbf9fc), SkBits2Float(0x43bdb333), SkBits2Float(0x42bbf9fc), SkBits2Float(0x43be0ccd), SkBits2Float(0x42bbf9fc));  // 378.7f, 93.9883f, 379.4f, 93.9883f, 380.1f, 93.9883f
path.cubicTo(SkBits2Float(0x43be6666), SkBits2Float(0x42bbf9fc), SkBits2Float(0x43bec000), SkBits2Float(0x42bbf8db), SkBits2Float(0x43bf199a), SkBits2Float(0x42bbf9fc));  // 380.8f, 93.9883f, 381.5f, 93.986f, 382.2f, 93.9883f
path.cubicTo(SkBits2Float(0x43bf7333), SkBits2Float(0x42bbfb1e), SkBits2Float(0x43bfcccd), SkBits2Float(0x42bbfeca), SkBits2Float(0x43c02666), SkBits2Float(0x42bc00c5));  // 382.9f, 93.9905f, 383.6f, 93.9976f, 384.3f, 94.0015f
path.cubicTo(SkBits2Float(0x43c08000), SkBits2Float(0x42bc02bf), SkBits2Float(0x43c0d99a), SkBits2Float(0x42bc0593), SkBits2Float(0x43c13333), SkBits2Float(0x42bc05db));  // 385, 94.0054f, 385.7f, 94.0109f, 386.4f, 94.0114f
path.cubicTo(SkBits2Float(0x43c18ccd), SkBits2Float(0x42bc0624), SkBits2Float(0x43c1e666), SkBits2Float(0x42bc0308), SkBits2Float(0x43c24000), SkBits2Float(0x42bc0277));  // 387.1f, 94.012f, 387.8f, 94.0059f, 388.5f, 94.0048f
path.cubicTo(SkBits2Float(0x43c2999a), SkBits2Float(0x42bc01e6), SkBits2Float(0x43c2f333), SkBits2Float(0x42bc022f), SkBits2Float(0x43c34ccd), SkBits2Float(0x42bc0277));  // 389.2f, 94.0037f, 389.9f, 94.0043f, 390.6f, 94.0048f
path.cubicTo(SkBits2Float(0x43c3a666), SkBits2Float(0x42bc02bf), SkBits2Float(0x43c40000), SkBits2Float(0x42bc02bf), SkBits2Float(0x43c4599a), SkBits2Float(0x42bc0429));  // 391.3f, 94.0054f, 392, 94.0054f, 392.7f, 94.0081f
path.cubicTo(SkBits2Float(0x43c4b333), SkBits2Float(0x42bc0593), SkBits2Float(0x43c50ccd), SkBits2Float(0x42bc08f7), SkBits2Float(0x43c56666), SkBits2Float(0x42bc0af2));  // 393.4f, 94.0109f, 394.1f, 94.0175f, 394.8f, 94.0214f
path.cubicTo(SkBits2Float(0x43c5c000), SkBits2Float(0x42bc0cec), SkBits2Float(0x43c6199a), SkBits2Float(0x42bc0f2f), SkBits2Float(0x43c67333), SkBits2Float(0x42bc1008));  // 395.5f, 94.0252f, 396.2f, 94.0297f, 396.9f, 94.0313f
path.cubicTo(SkBits2Float(0x43c6cccd), SkBits2Float(0x42bc10e1), SkBits2Float(0x43c72666), SkBits2Float(0x42bc0fc0), SkBits2Float(0x43c78000), SkBits2Float(0x42bc1008));  // 397.6f, 94.033f, 398.3f, 94.0308f, 399, 94.0313f
path.cubicTo(SkBits2Float(0x43c7d99a), SkBits2Float(0x42bc1050), SkBits2Float(0x43c83333), SkBits2Float(0x42bc1172), SkBits2Float(0x43c88ccd), SkBits2Float(0x42bc11ba));  // 399.7f, 94.0319f, 400.4f, 94.0341f, 401.1f, 94.0346f
path.cubicTo(SkBits2Float(0x43c8e666), SkBits2Float(0x42bc1202), SkBits2Float(0x43c94000), SkBits2Float(0x42bc11ba), SkBits2Float(0x43c9999a), SkBits2Float(0x42bc11ba));  // 401.8f, 94.0352f, 402.5f, 94.0346f, 403.2f, 94.0346f
path.cubicTo(SkBits2Float(0x43c9f333), SkBits2Float(0x42bc11ba), SkBits2Float(0x43ca4ccd), SkBits2Float(0x42bc124b), SkBits2Float(0x43caa666), SkBits2Float(0x42bc11ba));  // 403.9f, 94.0346f, 404.6f, 94.0357f, 405.3f, 94.0346f
path.cubicTo(SkBits2Float(0x43cb0000), SkBits2Float(0x42bc1129), SkBits2Float(0x43cb599a), SkBits2Float(0x42bc0f77), SkBits2Float(0x43cbb333), SkBits2Float(0x42bc0e56));  // 406, 94.0335f, 406.7f, 94.0302f, 407.4f, 94.028f
path.cubicTo(SkBits2Float(0x43cc0ccd), SkBits2Float(0x42bc0d34), SkBits2Float(0x43cc6666), SkBits2Float(0x42bc0b82), SkBits2Float(0x43ccc000), SkBits2Float(0x42bc0af2));  // 408.1f, 94.0258f, 408.8f, 94.0225f, 409.5f, 94.0214f
path.cubicTo(SkBits2Float(0x43cd199a), SkBits2Float(0x42bc0a61), SkBits2Float(0x43cd7333), SkBits2Float(0x42bc0b3a), SkBits2Float(0x43cdcccd), SkBits2Float(0x42bc0af2));  // 410.2f, 94.0203f, 410.9f, 94.0219f, 411.6f, 94.0214f
path.cubicTo(SkBits2Float(0x43ce2666), SkBits2Float(0x42bc0aa9), SkBits2Float(0x43ce8000), SkBits2Float(0x42bc0aa9), SkBits2Float(0x43ced99a), SkBits2Float(0x42bc0940));  // 412.3f, 94.0208f, 413, 94.0208f, 413.7f, 94.0181f
path.cubicTo(SkBits2Float(0x43cf3333), SkBits2Float(0x42bc07d6), SkBits2Float(0x43cf8ccd), SkBits2Float(0x42bc0502), SkBits2Float(0x43cfe666), SkBits2Float(0x42bc0277));  // 414.4f, 94.0153f, 415.1f, 94.0098f, 415.8f, 94.0048f
path.cubicTo(SkBits2Float(0x43d04000), SkBits2Float(0x42bbffec), SkBits2Float(0x43d0999a), SkBits2Float(0x42bbfc88), SkBits2Float(0x43d0f333), SkBits2Float(0x42bbf9fc));  // 416.5f, 93.9998f, 417.2f, 93.9932f, 417.9f, 93.9883f
path.cubicTo(SkBits2Float(0x43d14ccd), SkBits2Float(0x42bbf771), SkBits2Float(0x43d1a666), SkBits2Float(0x42bbf4e6), SkBits2Float(0x43d20000), SkBits2Float(0x42bbf334));  // 418.6f, 93.9833f, 419.3f, 93.9783f, 420, 93.975f
path.cubicTo(SkBits2Float(0x43d2599a), SkBits2Float(0x42bbf182), SkBits2Float(0x43d2b333), SkBits2Float(0x42bbee66), SkBits2Float(0x43d30ccd), SkBits2Float(0x42bbefd0));  // 420.7f, 93.9717f, 421.4f, 93.9656f, 422.1f, 93.9684f
path.cubicTo(SkBits2Float(0x43d36666), SkBits2Float(0x42bbf13a), SkBits2Float(0x43d3c000), SkBits2Float(0x42bbf52e), SkBits2Float(0x43d4199a), SkBits2Float(0x42bbfbaf));  // 422.8f, 93.9711f, 423.5f, 93.9789f, 424.2f, 93.9916f
path.cubicTo(SkBits2Float(0x43d47333), SkBits2Float(0x42bc022f), SkBits2Float(0x43d4cccd), SkBits2Float(0x42bc1014), SkBits2Float(0x43d52666), SkBits2Float(0x42bc16d0));  // 424.9f, 94.0043f, 425.6f, 94.0314f, 426.3f, 94.0446f
path.cubicTo(SkBits2Float(0x43d58000), SkBits2Float(0x42bc1d8d), SkBits2Float(0x43d5d99a), SkBits2Float(0x42bc1de3), SkBits2Float(0x43d63333), SkBits2Float(0x42bc241b));  // 427, 94.0577f, 427.7f, 94.0584f, 428.4f, 94.0705f
path.cubicTo(SkBits2Float(0x43d68ccd), SkBits2Float(0x42bc2a53), SkBits2Float(0x43d6e666), SkBits2Float(0x42bc30c5), SkBits2Float(0x43d74000), SkBits2Float(0x42bc3c1f));  // 429.1f, 94.0827f, 429.8f, 94.0953f, 430.5f, 94.1174f
path.cubicTo(SkBits2Float(0x43d7999a), SkBits2Float(0x42bc4779), SkBits2Float(0x43d7f333), SkBits2Float(0x42bc6283), SkBits2Float(0x43d84ccd), SkBits2Float(0x42bc6836));  // 431.2f, 94.1396f, 431.9f, 94.1924f, 432.6f, 94.2035f
path.cubicTo(SkBits2Float(0x43d8a666), SkBits2Float(0x42bc6de9), SkBits2Float(0x43d90000), SkBits2Float(0x42bc67e0), SkBits2Float(0x43d9599a), SkBits2Float(0x42bc5e50));  // 433.3f, 94.2147f, 434, 94.2029f, 434.7f, 94.1842f
path.cubicTo(SkBits2Float(0x43d9b333), SkBits2Float(0x42bc54c0), SkBits2Float(0x43da0ccd), SkBits2Float(0x42bc3a23), SkBits2Float(0x43da6666), SkBits2Float(0x42bc2ed5));  // 435.4f, 94.1655f, 436.1f, 94.1135f, 436.8f, 94.0915f
path.cubicTo(SkBits2Float(0x43dac000), SkBits2Float(0x42bc2387), SkBits2Float(0x43db199a), SkBits2Float(0x42bc1e34), SkBits2Float(0x43db7333), SkBits2Float(0x42bc1a7b));  // 437.5f, 94.0694f, 438.2f, 94.059f, 438.9f, 94.0517f
path.cubicTo(SkBits2Float(0x43dbcccd), SkBits2Float(0x42bc16c3), SkBits2Float(0x43dc2666), SkBits2Float(0x42bc18a6), SkBits2Float(0x43dc8000), SkBits2Float(0x42bc1883));  // 439.6f, 94.0445f, 440.3f, 94.0481f, 441, 94.0479f
path.cubicTo(SkBits2Float(0x43dcd99a), SkBits2Float(0x42bc185f), SkBits2Float(0x43dd3333), SkBits2Float(0x42bc19f0), SkBits2Float(0x43dd8ccd), SkBits2Float(0x42bc19a8));  // 441.7f, 94.0476f, 442.4f, 94.0507f, 443.1f, 94.0501f
path.cubicTo(SkBits2Float(0x43dde666), SkBits2Float(0x42bc195f), SkBits2Float(0x43de4000), SkBits2Float(0x42bc15d4), SkBits2Float(0x43de999a), SkBits2Float(0x42bc16d0));  // 443.8f, 94.0496f, 444.5f, 94.0426f, 445.2f, 94.0446f
path.cubicTo(SkBits2Float(0x43def333), SkBits2Float(0x42bc17cd), SkBits2Float(0x43df4ccd), SkBits2Float(0x42bc11a1), SkBits2Float(0x43dfa666), SkBits2Float(0x42bc1f92));  // 445.9f, 94.0465f, 446.6f, 94.0344f, 447.3f, 94.0617f
path.cubicTo(SkBits2Float(0x43e00000), SkBits2Float(0x42bc2d83), SkBits2Float(0x43e0599a), SkBits2Float(0x42bc478f), SkBits2Float(0x43e0b333), SkBits2Float(0x42bc6a76));  // 448, 94.0889f, 448.7f, 94.1398f, 449.4f, 94.2079f
path.cubicTo(SkBits2Float(0x43e10ccd), SkBits2Float(0x42bc8d5c), SkBits2Float(0x43e16666), SkBits2Float(0x42bcd80e), SkBits2Float(0x43e1c000), SkBits2Float(0x42bcf0fa));  // 450.1f, 94.2761f, 450.8f, 94.422f, 451.5f, 94.4707f
path.cubicTo(SkBits2Float(0x43e2199a), SkBits2Float(0x42bd09e7), SkBits2Float(0x43e27333), SkBits2Float(0x42bd0449), SkBits2Float(0x43e2cccd), SkBits2Float(0x42bd0000));  // 452.2f, 94.5193f, 452.9f, 94.5084f, 453.6f, 94.5f
path.cubicTo(SkBits2Float(0x43e32666), SkBits2Float(0x42bcfbb7), SkBits2Float(0x43e38000), SkBits2Float(0x42bcebe7), SkBits2Float(0x43e3d99a), SkBits2Float(0x42bcd744));  // 454.3f, 94.4916f, 455, 94.4607f, 455.7f, 94.4204f
path.cubicTo(SkBits2Float(0x43e43333), SkBits2Float(0x42bcc2a0), SkBits2Float(0x43e48ccd), SkBits2Float(0x42bc8dc8), SkBits2Float(0x43e4e666), SkBits2Float(0x42bc842c));  // 456.4f, 94.3801f, 457.1f, 94.2769f, 457.8f, 94.2581f
path.cubicTo(SkBits2Float(0x43e54000), SkBits2Float(0x42bc7a90), SkBits2Float(0x43e5999a), SkBits2Float(0x42bc8f32), SkBits2Float(0x43e5f333), SkBits2Float(0x42bc9d9c));  // 458.5f, 94.2394f, 459.2f, 94.2797f, 459.9f, 94.3078f
path.cubicTo(SkBits2Float(0x43e64ccd), SkBits2Float(0x42bcac06), SkBits2Float(0x43e6a666), SkBits2Float(0x42bcca42), SkBits2Float(0x43e70000), SkBits2Float(0x42bcdaa8));  // 460.6f, 94.336f, 461.3f, 94.395f, 462, 94.4271f
path.cubicTo(SkBits2Float(0x43e7599a), SkBits2Float(0x42bceb0e), SkBits2Float(0x43e7b333), SkBits2Float(0x42bcf9c7), SkBits2Float(0x43e80ccd), SkBits2Float(0x42bd0000));  // 462.7f, 94.4591f, 463.4f, 94.4878f, 464.1f, 94.5f
path.cubicTo(SkBits2Float(0x43e86666), SkBits2Float(0x42bd0639), SkBits2Float(0x43e8c000), SkBits2Float(0x42bd010b), SkBits2Float(0x43e9199a), SkBits2Float(0x42bd0000));  // 464.8f, 94.5122f, 465.5f, 94.502f, 466.2f, 94.5f
path.cubicTo(SkBits2Float(0x43e97333), SkBits2Float(0x42bcfef5), SkBits2Float(0x43e9cccd), SkBits2Float(0x42bcf9bb), SkBits2Float(0x43ea2666), SkBits2Float(0x42bcf9bb));  // 466.9f, 94.498f, 467.6f, 94.4878f, 468.3f, 94.4878f
path.cubicTo(SkBits2Float(0x43ea8000), SkBits2Float(0x42bcf9bb), SkBits2Float(0x43ead99a), SkBits2Float(0x42bcfef5), SkBits2Float(0x43eb3333), SkBits2Float(0x42bd0000));  // 469, 94.4878f, 469.7f, 94.498f, 470.4f, 94.5f
path.lineTo(SkBits2Float(0x43eb3333), SkBits2Float(0x42bd0000));  // 470.4f, 94.5f
path.lineTo(SkBits2Float(0x43ac3333), SkBits2Float(0x42bd0000));  // 344.4f, 94.5f
path.close();
    return path;
}

static SkPath path4() {
    SkPath path;
path.moveTo(SkBits2Float(0x42b06666), SkBits2Float(0x42bd0000));  // 88.2f, 94.5f
path.lineTo(SkBits2Float(0x42b06666), SkBits2Float(0x42bd0000));  // 88.2f, 94.5f
path.cubicTo(SkBits2Float(0x42b1cccd), SkBits2Float(0x42bd0000), SkBits2Float(0x42b33333), SkBits2Float(0x42bcda8d), SkBits2Float(0x42b4999a), SkBits2Float(0x42bd0000));  // 88.9f, 94.5f, 89.6f, 94.4269f, 90.3f, 94.5f
path.cubicTo(SkBits2Float(0x42b60000), SkBits2Float(0x42bd2573), SkBits2Float(0x42b76666), SkBits2Float(0x42bdba68), SkBits2Float(0x42b8cccd), SkBits2Float(0x42bde0b5));  // 91, 94.5731f, 91.7f, 94.8641f, 92.4f, 94.9389f
path.cubicTo(SkBits2Float(0x42ba3333), SkBits2Float(0x42be0701), SkBits2Float(0x42bb999a), SkBits2Float(0x42bde462), SkBits2Float(0x42bd0000), SkBits2Float(0x42bde5cb));  // 93.1f, 95.0137f, 93.8f, 94.9461f, 94.5f, 94.9488f
path.cubicTo(SkBits2Float(0x42be6666), SkBits2Float(0x42bde735), SkBits2Float(0x42bfcccd), SkBits2Float(0x42bde80e), SkBits2Float(0x42c13333), SkBits2Float(0x42bde930));  // 95.2f, 94.9516f, 95.9f, 94.9532f, 96.6f, 94.9554f
path.cubicTo(SkBits2Float(0x42c2999a), SkBits2Float(0x42bdea51), SkBits2Float(0x42c40000), SkBits2Float(0x42bdec03), SkBits2Float(0x42c56666), SkBits2Float(0x42bdec94));  // 97.3f, 94.9576f, 98, 94.961f, 98.7f, 94.9621f
path.cubicTo(SkBits2Float(0x42c6cccd), SkBits2Float(0x42bded24), SkBits2Float(0x42c83333), SkBits2Float(0x42bded19), SkBits2Float(0x42c9999a), SkBits2Float(0x42bdec94));  // 99.4f, 94.9632f, 100.1f, 94.9631f, 100.8f, 94.9621f
path.cubicTo(SkBits2Float(0x42cb0000), SkBits2Float(0x42bdec0f), SkBits2Float(0x42cc6666), SkBits2Float(0x42bdea38), SkBits2Float(0x42cdcccd), SkBits2Float(0x42bde976));  // 101.5f, 94.9611f, 102.2f, 94.9575f, 102.9f, 94.956f
path.cubicTo(SkBits2Float(0x42cf3333), SkBits2Float(0x42bde8b5), SkBits2Float(0x42d0999a), SkBits2Float(0x42bde853), SkBits2Float(0x42d20000), SkBits2Float(0x42bde80b));  // 103.6f, 94.9545f, 104.3f, 94.9538f, 105, 94.9532f
path.cubicTo(SkBits2Float(0x42d36666), SkBits2Float(0x42bde7c2), SkBits2Float(0x42d4cccd), SkBits2Float(0x42bde9d6), SkBits2Float(0x42d63333), SkBits2Float(0x42bde7c4));  // 105.7f, 94.9527f, 106.4f, 94.9567f, 107.1f, 94.9527f
path.cubicTo(SkBits2Float(0x42d7999a), SkBits2Float(0x42bde5b2), SkBits2Float(0x42d90000), SkBits2Float(0x42bde19a), SkBits2Float(0x42da6666), SkBits2Float(0x42bddb9f));  // 107.8f, 94.9486f, 108.5f, 94.9406f, 109.2f, 94.9289f
path.cubicTo(SkBits2Float(0x42dbcccd), SkBits2Float(0x42bdd5a3), SkBits2Float(0x42dd3333), SkBits2Float(0x42bdcc07), SkBits2Float(0x42de999a), SkBits2Float(0x42bdc3e1));  // 109.9f, 94.9173f, 110.6f, 94.8985f, 111.3f, 94.8826f
path.cubicTo(SkBits2Float(0x42e00000), SkBits2Float(0x42bdbbba), SkBits2Float(0x42e16666), SkBits2Float(0x42bdb332), SkBits2Float(0x42e2cccd), SkBits2Float(0x42bdaab8));  // 112, 94.8667f, 112.7f, 94.85f, 113.4f, 94.8334f
path.cubicTo(SkBits2Float(0x42e43333), SkBits2Float(0x42bda23d), SkBits2Float(0x42e5999a), SkBits2Float(0x42bd9b8e), SkBits2Float(0x42e70000), SkBits2Float(0x42bd9101));  // 114.1f, 94.8169f, 114.8f, 94.8038f, 115.5f, 94.7832f
path.cubicTo(SkBits2Float(0x42e86666), SkBits2Float(0x42bd8675), SkBits2Float(0x42e9cccd), SkBits2Float(0x42bd79f9), SkBits2Float(0x42eb3333), SkBits2Float(0x42bd6b6c));  // 116.2f, 94.7626f, 116.9f, 94.7382f, 117.6f, 94.7098f
path.cubicTo(SkBits2Float(0x42ec999a), SkBits2Float(0x42bd5cdf), SkBits2Float(0x42ee0000), SkBits2Float(0x42bd4608), SkBits2Float(0x42ef6666), SkBits2Float(0x42bd39b1));  // 118.3f, 94.6814f, 119, 94.6368f, 119.7f, 94.6127f
path.cubicTo(SkBits2Float(0x42f0cccd), SkBits2Float(0x42bd2d5b), SkBits2Float(0x42f23333), SkBits2Float(0x42bd24cb), SkBits2Float(0x42f3999a), SkBits2Float(0x42bd2166));  // 120.4f, 94.5886f, 121.1f, 94.5719f, 121.8f, 94.5652f
path.cubicTo(SkBits2Float(0x42f50000), SkBits2Float(0x42bd1e02), SkBits2Float(0x42f66666), SkBits2Float(0x42bd22c1), SkBits2Float(0x42f7cccd), SkBits2Float(0x42bd2558));  // 122.5f, 94.5586f, 123.2f, 94.5679f, 123.9f, 94.5729f
path.cubicTo(SkBits2Float(0x42f93333), SkBits2Float(0x42bd27ef), SkBits2Float(0x42fa999a), SkBits2Float(0x42bd2da3), SkBits2Float(0x42fc0000), SkBits2Float(0x42bd30f0));  // 124.6f, 94.578f, 125.3f, 94.5891f, 126, 94.5956f
path.cubicTo(SkBits2Float(0x42fd6666), SkBits2Float(0x42bd343d), SkBits2Float(0x42fecccd), SkBits2Float(0x42bd36a5), SkBits2Float(0x4300199a), SkBits2Float(0x42bd3924));  // 126.7f, 94.602f, 127.4f, 94.6067f, 128.1f, 94.6116f
path.cubicTo(SkBits2Float(0x4300cccd), SkBits2Float(0x42bd3ba4), SkBits2Float(0x43018000), SkBits2Float(0x42bd3f14), SkBits2Float(0x43023333), SkBits2Float(0x42bd3fed));  // 128.8f, 94.6165f, 129.5f, 94.6232f, 130.2f, 94.6249f
path.cubicTo(SkBits2Float(0x4302e666), SkBits2Float(0x42bd40c6), SkBits2Float(0x4303999a), SkBits2Float(0x42bd3f50), SkBits2Float(0x43044ccd), SkBits2Float(0x42bd3e3b));  // 130.9f, 94.6265f, 131.6f, 94.6237f, 132.3f, 94.6215f
path.cubicTo(SkBits2Float(0x43050000), SkBits2Float(0x42bd3d25), SkBits2Float(0x4305b333), SkBits2Float(0x42bd3a44), SkBits2Float(0x43066666), SkBits2Float(0x42bd396b));  // 133, 94.6194f, 133.7f, 94.6138f, 134.4f, 94.6121f
path.cubicTo(SkBits2Float(0x4307199a), SkBits2Float(0x42bd3892), SkBits2Float(0x4307cccd), SkBits2Float(0x42bd3978), SkBits2Float(0x43088000), SkBits2Float(0x42bd3924));  // 135.1f, 94.6105f, 135.8f, 94.6122f, 136.5f, 94.6116f
path.cubicTo(SkBits2Float(0x43093333), SkBits2Float(0x42bd38d0), SkBits2Float(0x4309e666), SkBits2Float(0x42bd3766), SkBits2Float(0x430a999a), SkBits2Float(0x42bd3772));  // 137.2f, 94.611f, 137.9f, 94.6082f, 138.6f, 94.6083f
path.cubicTo(SkBits2Float(0x430b4ccd), SkBits2Float(0x42bd377e), SkBits2Float(0x430c0000), SkBits2Float(0x42bd3892), SkBits2Float(0x430cb333), SkBits2Float(0x42bd396b));  // 139.3f, 94.6084f, 140, 94.6105f, 140.7f, 94.6121f
path.cubicTo(SkBits2Float(0x430d6666), SkBits2Float(0x42bd3a44), SkBits2Float(0x430e199a), SkBits2Float(0x42bd3bbb), SkBits2Float(0x430ecccd), SkBits2Float(0x42bd3c88));  // 141.4f, 94.6138f, 142.1f, 94.6167f, 142.8f, 94.6182f
path.cubicTo(SkBits2Float(0x430f8000), SkBits2Float(0x42bd3d56), SkBits2Float(0x43103333), SkBits2Float(0x42bd3cd1), SkBits2Float(0x4310e666), SkBits2Float(0x42bd3e3b));  // 143.5f, 94.6198f, 144.2f, 94.6188f, 144.9f, 94.6215f
path.cubicTo(SkBits2Float(0x4311999a), SkBits2Float(0x42bd3fa4), SkBits2Float(0x43124ccd), SkBits2Float(0x42bd4278), SkBits2Float(0x43130000), SkBits2Float(0x42bd4503));  // 145.6f, 94.6243f, 146.3f, 94.6298f, 147, 94.6348f
path.cubicTo(SkBits2Float(0x4313b333), SkBits2Float(0x42bd478e), SkBits2Float(0x43146666), SkBits2Float(0x42bd4af2), SkBits2Float(0x4315199a), SkBits2Float(0x42bd4d7e));  // 147.7f, 94.6398f, 148.4f, 94.6464f, 149.1f, 94.6514f
path.cubicTo(SkBits2Float(0x4315cccd), SkBits2Float(0x42bd5009), SkBits2Float(0x43168000), SkBits2Float(0x42bd53aa), SkBits2Float(0x43173333), SkBits2Float(0x42bd5446));  // 149.8f, 94.6563f, 150.5f, 94.6634f, 151.2f, 94.6646f
path.cubicTo(SkBits2Float(0x4317e666), SkBits2Float(0x42bd54e3), SkBits2Float(0x4318999a), SkBits2Float(0x42bd5202), SkBits2Float(0x43194ccd), SkBits2Float(0x42bd5128));  // 151.9f, 94.6658f, 152.6f, 94.6602f, 153.3f, 94.6585f
path.cubicTo(SkBits2Float(0x431a0000), SkBits2Float(0x42bd504f), SkBits2Float(0x431ab333), SkBits2Float(0x42bd4fcc), SkBits2Float(0x431b6666), SkBits2Float(0x42bd4f30));  // 154, 94.6569f, 154.7f, 94.6559f, 155.4f, 94.6547f
path.cubicTo(SkBits2Float(0x431c199a), SkBits2Float(0x42bd4e93), SkBits2Float(0x431ccccd), SkBits2Float(0x42bd4ee7), SkBits2Float(0x431d8000), SkBits2Float(0x42bd4d7e));  // 156.1f, 94.6535f, 156.8f, 94.6541f, 157.5f, 94.6514f
path.cubicTo(SkBits2Float(0x431e3333), SkBits2Float(0x42bd4c14), SkBits2Float(0x431ee666), SkBits2Float(0x42bd48f8), SkBits2Float(0x431f999a), SkBits2Float(0x42bd46b5));  // 158.2f, 94.6486f, 158.9f, 94.6425f, 159.6f, 94.6381f
path.cubicTo(SkBits2Float(0x43204ccd), SkBits2Float(0x42bd4472), SkBits2Float(0x43210000), SkBits2Float(0x42bd410e), SkBits2Float(0x4321b333), SkBits2Float(0x42bd3fed));  // 160.3f, 94.6337f, 161, 94.6271f, 161.7f, 94.6249f
path.cubicTo(SkBits2Float(0x43226666), SkBits2Float(0x42bd3ecb), SkBits2Float(0x4323199a), SkBits2Float(0x42bd401d), SkBits2Float(0x4323cccd), SkBits2Float(0x42bd3fed));  // 162.4f, 94.6226f, 163.1f, 94.6252f, 163.8f, 94.6249f
path.cubicTo(SkBits2Float(0x43248000), SkBits2Float(0x42bd3fbc), SkBits2Float(0x43253333), SkBits2Float(0x42bd40e7), SkBits2Float(0x4325e666), SkBits2Float(0x42bd3ec8));  // 164.5f, 94.6245f, 165.2f, 94.6268f, 165.9f, 94.6226f
path.cubicTo(SkBits2Float(0x4326999a), SkBits2Float(0x42bd3ca8), SkBits2Float(0x43274ccd), SkBits2Float(0x42bd383a), SkBits2Float(0x43280000), SkBits2Float(0x42bd332f));  // 166.6f, 94.6185f, 167.3f, 94.6098f, 168, 94.6f
path.cubicTo(SkBits2Float(0x4328b333), SkBits2Float(0x42bd2e25), SkBits2Float(0x43296666), SkBits2Float(0x42bd2647), SkBits2Float(0x432a199a), SkBits2Float(0x42bd2088));  // 168.7f, 94.5901f, 169.4f, 94.5748f, 170.1f, 94.5635f
path.cubicTo(SkBits2Float(0x432acccd), SkBits2Float(0x42bd1aca), SkBits2Float(0x432b8000), SkBits2Float(0x42bd147c), SkBits2Float(0x432c3333), SkBits2Float(0x42bd10b8));  // 170.8f, 94.5523f, 171.5f, 94.54f, 172.2f, 94.5327f
path.cubicTo(SkBits2Float(0x432ce666), SkBits2Float(0x42bd0cf4), SkBits2Float(0x432d999a), SkBits2Float(0x42bd0c32), SkBits2Float(0x432e4ccd), SkBits2Float(0x42bd09ef));  // 172.9f, 94.5253f, 173.6f, 94.5238f, 174.3f, 94.5194f
path.cubicTo(SkBits2Float(0x432f0000), SkBits2Float(0x42bd07ad), SkBits2Float(0x432fb333), SkBits2Float(0x42bd04cf), SkBits2Float(0x43306666), SkBits2Float(0x42bd0327));  // 175, 94.515f, 175.7f, 94.5094f, 176.4f, 94.5062f
path.lineTo(SkBits2Float(0x43306666), SkBits2Float(0x42bd0000));  // 176.4f, 94.5f
path.lineTo(SkBits2Float(0x42b06666), SkBits2Float(0x42bd0000));  // 88.2f, 94.5f
path.close();
path.moveTo(SkBits2Float(0x43413333), SkBits2Float(0x42bd0000));  // 193.2f, 94.5f
path.lineTo(SkBits2Float(0x43413333), SkBits2Float(0x42bd0000));  // 193.2f, 94.5f
path.cubicTo(SkBits2Float(0x4341e666), SkBits2Float(0x42bd0000), SkBits2Float(0x4342999a), SkBits2Float(0x42bcfab7), SkBits2Float(0x43434ccd), SkBits2Float(0x42bd0000));  // 193.9f, 94.5f, 194.6f, 94.4897f, 195.3f, 94.5f
path.cubicTo(SkBits2Float(0x43440000), SkBits2Float(0x42bd0549), SkBits2Float(0x4344b333), SkBits2Float(0x42bd13e0), SkBits2Float(0x43456666), SkBits2Float(0x42bd1fb4));  // 196, 94.5103f, 196.7f, 94.5388f, 197.4f, 94.5619f
path.cubicTo(SkBits2Float(0x4346199a), SkBits2Float(0x42bd2b89), SkBits2Float(0x4346cccd), SkBits2Float(0x42bd3eb2), SkBits2Float(0x43478000), SkBits2Float(0x42bd46fc));  // 198.1f, 94.585f, 198.8f, 94.6225f, 199.5f, 94.6386f
path.cubicTo(SkBits2Float(0x43483333), SkBits2Float(0x42bd4f46), SkBits2Float(0x4348e666), SkBits2Float(0x42bd5127), SkBits2Float(0x4349999a), SkBits2Float(0x42bd516f));  // 200.2f, 94.6548f, 200.9f, 94.6585f, 201.6f, 94.659f
path.cubicTo(SkBits2Float(0x434a4ccd), SkBits2Float(0x42bd51b7), SkBits2Float(0x434b0000), SkBits2Float(0x42bd4dc4), SkBits2Float(0x434bb333), SkBits2Float(0x42bd48ae));  // 202.3f, 94.6596f, 203, 94.6519f, 203.7f, 94.642f
path.cubicTo(SkBits2Float(0x434c6666), SkBits2Float(0x42bd4397), SkBits2Float(0x434d199a), SkBits2Float(0x42bd39ee), SkBits2Float(0x434dcccd), SkBits2Float(0x42bd32e9));  // 204.4f, 94.632f, 205.1f, 94.6131f, 205.8f, 94.5994f
path.cubicTo(SkBits2Float(0x434e8000), SkBits2Float(0x42bd2be4), SkBits2Float(0x434f3333), SkBits2Float(0x42bd23a6), SkBits2Float(0x434fe666), SkBits2Float(0x42bd1e8f));  // 206.5f, 94.5857f, 207.2f, 94.5696f, 207.9f, 94.5597f
path.cubicTo(SkBits2Float(0x4350999a), SkBits2Float(0x42bd1979), SkBits2Float(0x43514ccd), SkBits2Float(0x42bd14f3), SkBits2Float(0x43520000), SkBits2Float(0x42bd1463));  // 208.6f, 94.5498f, 209.3f, 94.5409f, 210, 94.5398f
path.cubicTo(SkBits2Float(0x4352b333), SkBits2Float(0x42bd13d2), SkBits2Float(0x43536666), SkBits2Float(0x42bd1590), SkBits2Float(0x4354199a), SkBits2Float(0x42bd1b2b));  // 210.7f, 94.5387f, 211.4f, 94.5421f, 212.1f, 94.5531f
path.cubicTo(SkBits2Float(0x4354cccd), SkBits2Float(0x42bd20c7), SkBits2Float(0x43558000), SkBits2Float(0x42bd2bce), SkBits2Float(0x43563333), SkBits2Float(0x42bd3607));  // 212.8f, 94.564f, 213.5f, 94.5856f, 214.2f, 94.6055f
path.cubicTo(SkBits2Float(0x4356e666), SkBits2Float(0x42bd403f), SkBits2Float(0x4357999a), SkBits2Float(0x42bd4644), SkBits2Float(0x43584ccd), SkBits2Float(0x42bd587e));  // 214.9f, 94.6255f, 215.6f, 94.6372f, 216.3f, 94.6728f
path.cubicTo(SkBits2Float(0x43590000), SkBits2Float(0x42bd6ab8), SkBits2Float(0x4359b333), SkBits2Float(0x42bd8902), SkBits2Float(0x435a6666), SkBits2Float(0x42bda362));  // 217, 94.7084f, 217.7f, 94.7676f, 218.4f, 94.8191f
path.cubicTo(SkBits2Float(0x435b199a), SkBits2Float(0x42bdbdc2), SkBits2Float(0x435bcccd), SkBits2Float(0x42bde650), SkBits2Float(0x435c8000), SkBits2Float(0x42bdf6c0));  // 219.1f, 94.8706f, 219.8f, 94.9498f, 220.5f, 94.9819f
path.cubicTo(SkBits2Float(0x435d3333), SkBits2Float(0x42be0731), SkBits2Float(0x435de666), SkBits2Float(0x42be0409), SkBits2Float(0x435e999a), SkBits2Float(0x42be0604));  // 221.2f, 95.014f, 221.9f, 95.0079f, 222.6f, 95.0117f
path.cubicTo(SkBits2Float(0x435f4ccd), SkBits2Float(0x42be07fe), SkBits2Float(0x43600000), SkBits2Float(0x42be052a), SkBits2Float(0x4360b333), SkBits2Float(0x42be029f));  // 223.3f, 95.0156f, 224, 95.0101f, 224.7f, 95.0051f
path.cubicTo(SkBits2Float(0x43616666), SkBits2Float(0x42be0014), SkBits2Float(0x4362199a), SkBits2Float(0x42bdf903), SkBits2Float(0x4362cccd), SkBits2Float(0x42bdf6c0));  // 225.4f, 95.0002f, 226.1f, 94.9864f, 226.8f, 94.9819f
path.cubicTo(SkBits2Float(0x43638000), SkBits2Float(0x42bdf47e), SkBits2Float(0x43643333), SkBits2Float(0x42bdf3ed), SkBits2Float(0x4364e666), SkBits2Float(0x42bdf50e));  // 227.5f, 94.9775f, 228.2f, 94.9764f, 228.9f, 94.9786f
path.cubicTo(SkBits2Float(0x4365999a), SkBits2Float(0x42bdf630), SkBits2Float(0x43664ccd), SkBits2Float(0x42bdfe62), SkBits2Float(0x43670000), SkBits2Float(0x42bdfd89));  // 229.6f, 94.9808f, 230.3f, 94.9968f, 231, 94.9952f
path.cubicTo(SkBits2Float(0x4367b333), SkBits2Float(0x42bdfcb0), SkBits2Float(0x43686666), SkBits2Float(0x42bdf4c6), SkBits2Float(0x4369199a), SkBits2Float(0x42bdeff8));  // 231.7f, 94.9935f, 232.4f, 94.9781f, 233.1f, 94.9687f
path.cubicTo(SkBits2Float(0x4369cccd), SkBits2Float(0x42bdeb2a), SkBits2Float(0x436a8000), SkBits2Float(0x42bde2f8), SkBits2Float(0x436b3333), SkBits2Float(0x42bde0b5));  // 233.8f, 94.9593f, 234.5f, 94.9433f, 235.2f, 94.9389f
path.cubicTo(SkBits2Float(0x436be666), SkBits2Float(0x42bdde72), SkBits2Float(0x436c999a), SkBits2Float(0x42bde1d6), SkBits2Float(0x436d4ccd), SkBits2Float(0x42bde267));  // 235.9f, 94.9345f, 236.6f, 94.9411f, 237.3f, 94.9422f
path.cubicTo(SkBits2Float(0x436e0000), SkBits2Float(0x42bde2f8), SkBits2Float(0x436eb333), SkBits2Float(0x42bde340), SkBits2Float(0x436f6666), SkBits2Float(0x42bde419));  // 238, 94.9433f, 238.7f, 94.9438f, 239.4f, 94.9455f
path.cubicTo(SkBits2Float(0x4370199a), SkBits2Float(0x42bde4f2), SkBits2Float(0x4370cccd), SkBits2Float(0x42bde6a4), SkBits2Float(0x43718000), SkBits2Float(0x42bde77d));  // 240.1f, 94.9472f, 240.8f, 94.9505f, 241.5f, 94.9521f
path.cubicTo(SkBits2Float(0x43723333), SkBits2Float(0x42bde856), SkBits2Float(0x4372e666), SkBits2Float(0x42bde8e7), SkBits2Float(0x4373999a), SkBits2Float(0x42bde930));  // 242.2f, 94.9538f, 242.9f, 94.9549f, 243.6f, 94.9554f
path.cubicTo(SkBits2Float(0x43744ccd), SkBits2Float(0x42bde978), SkBits2Float(0x43750000), SkBits2Float(0x42bde8e7), SkBits2Float(0x4375b333), SkBits2Float(0x42bde930));  // 244.3f, 94.956f, 245, 94.9549f, 245.7f, 94.9554f
path.cubicTo(SkBits2Float(0x43766666), SkBits2Float(0x42bde978), SkBits2Float(0x4377199a), SkBits2Float(0x42bdea99), SkBits2Float(0x4377cccd), SkBits2Float(0x42bdeae2));  // 246.4f, 94.956f, 247.1f, 94.9582f, 247.8f, 94.9588f
path.cubicTo(SkBits2Float(0x43788000), SkBits2Float(0x42bdeb2a), SkBits2Float(0x43793333), SkBits2Float(0x42bdeb72), SkBits2Float(0x4379e666), SkBits2Float(0x42bdeae2));  // 248.5f, 94.9593f, 249.2f, 94.9599f, 249.9f, 94.9588f
path.cubicTo(SkBits2Float(0x437a999a), SkBits2Float(0x42bdea51), SkBits2Float(0x437b4ccd), SkBits2Float(0x42bde89f), SkBits2Float(0x437c0000), SkBits2Float(0x42bde77d));  // 250.6f, 94.9576f, 251.3f, 94.9543f, 252, 94.9521f
path.cubicTo(SkBits2Float(0x437cb333), SkBits2Float(0x42bde65c), SkBits2Float(0x437d6666), SkBits2Float(0x42bde4f2), SkBits2Float(0x437e199a), SkBits2Float(0x42bde419));  // 252.7f, 94.9499f, 253.4f, 94.9472f, 254.1f, 94.9455f
path.cubicTo(SkBits2Float(0x437ecccd), SkBits2Float(0x42bde340), SkBits2Float(0x437f8000), SkBits2Float(0x42bde267), SkBits2Float(0x4380199a), SkBits2Float(0x42bde267));  // 254.8f, 94.9438f, 255.5f, 94.9422f, 256.2f, 94.9422f
path.cubicTo(SkBits2Float(0x43807333), SkBits2Float(0x42bde267), SkBits2Float(0x4380cccd), SkBits2Float(0x42bde2f8), SkBits2Float(0x43812666), SkBits2Float(0x42bde419));  // 256.9f, 94.9422f, 257.6f, 94.9433f, 258.3f, 94.9455f
path.cubicTo(SkBits2Float(0x43818000), SkBits2Float(0x42bde53b), SkBits2Float(0x4381d99a), SkBits2Float(0x42bde7c6), SkBits2Float(0x43823333), SkBits2Float(0x42bde930));  // 259, 94.9477f, 259.7f, 94.9527f, 260.4f, 94.9554f
path.cubicTo(SkBits2Float(0x43828ccd), SkBits2Float(0x42bdea99), SkBits2Float(0x4382e666), SkBits2Float(0x42bdec03), SkBits2Float(0x43834000), SkBits2Float(0x42bdec94));  // 261.1f, 94.9582f, 261.8f, 94.961f, 262.5f, 94.9621f
path.cubicTo(SkBits2Float(0x4383999a), SkBits2Float(0x42bded24), SkBits2Float(0x4383f333), SkBits2Float(0x42bdecdc), SkBits2Float(0x43844ccd), SkBits2Float(0x42bdec94));  // 263.2f, 94.9632f, 263.9f, 94.9626f, 264.6f, 94.9621f
path.cubicTo(SkBits2Float(0x4384a666), SkBits2Float(0x42bdec4b), SkBits2Float(0x43850000), SkBits2Float(0x42bdeb72), SkBits2Float(0x4385599a), SkBits2Float(0x42bdeae2));  // 265.3f, 94.9615f, 266, 94.9599f, 266.7f, 94.9588f
path.cubicTo(SkBits2Float(0x4385b333), SkBits2Float(0x42bdea51), SkBits2Float(0x43860ccd), SkBits2Float(0x42bde978), SkBits2Float(0x43866666), SkBits2Float(0x42bde930));  // 267.4f, 94.9576f, 268.1f, 94.956f, 268.8f, 94.9554f
path.cubicTo(SkBits2Float(0x4386c000), SkBits2Float(0x42bde8e7), SkBits2Float(0x4387199a), SkBits2Float(0x42bde8e7), SkBits2Float(0x43877333), SkBits2Float(0x42bde930));  // 269.5f, 94.9549f, 270.2f, 94.9549f, 270.9f, 94.9554f
path.cubicTo(SkBits2Float(0x4387cccd), SkBits2Float(0x42bde978), SkBits2Float(0x43882666), SkBits2Float(0x42bdea99), SkBits2Float(0x43888000), SkBits2Float(0x42bdeae2));  // 271.6f, 94.956f, 272.3f, 94.9582f, 273, 94.9588f
path.cubicTo(SkBits2Float(0x4388d99a), SkBits2Float(0x42bdeb2a), SkBits2Float(0x43893333), SkBits2Float(0x42bdeae2), SkBits2Float(0x43898ccd), SkBits2Float(0x42bdeae2));  // 273.7f, 94.9593f, 274.4f, 94.9588f, 275.1f, 94.9588f
path.cubicTo(SkBits2Float(0x4389e666), SkBits2Float(0x42bdeae2), SkBits2Float(0x438a4000), SkBits2Float(0x42bdea99), SkBits2Float(0x438a999a), SkBits2Float(0x42bdeae2));  // 275.8f, 94.9588f, 276.5f, 94.9582f, 277.2f, 94.9588f
path.cubicTo(SkBits2Float(0x438af333), SkBits2Float(0x42bdeb2a), SkBits2Float(0x438b4ccd), SkBits2Float(0x42bdec94), SkBits2Float(0x438ba666), SkBits2Float(0x42bdec94));  // 277.9f, 94.9593f, 278.6f, 94.9621f, 279.3f, 94.9621f
path.cubicTo(SkBits2Float(0x438c0000), SkBits2Float(0x42bdec94), SkBits2Float(0x438c599a), SkBits2Float(0x42bdead6), SkBits2Float(0x438cb333), SkBits2Float(0x42bdeae2));  // 280, 94.9621f, 280.7f, 94.9587f, 281.4f, 94.9588f
path.cubicTo(SkBits2Float(0x438d0ccd), SkBits2Float(0x42bdeaed), SkBits2Float(0x438d6666), SkBits2Float(0x42bdeb11), SkBits2Float(0x438dc000), SkBits2Float(0x42bdecda));  // 282.1f, 94.9588f, 282.8f, 94.9591f, 283.5f, 94.9626f
path.cubicTo(SkBits2Float(0x438e199a), SkBits2Float(0x42bdeea4), SkBits2Float(0x438e7333), SkBits2Float(0x42bdf3e9), SkBits2Float(0x438ecccd), SkBits2Float(0x42bdf59c));  // 284.2f, 94.9661f, 284.9f, 94.9764f, 285.6f, 94.9797f
path.cubicTo(SkBits2Float(0x438f2666), SkBits2Float(0x42bdf74e), SkBits2Float(0x438f8000), SkBits2Float(0x42bdfbed), SkBits2Float(0x438fd99a), SkBits2Float(0x42bdf707));  // 286.3f, 94.983f, 287, 94.992f, 287.7f, 94.9825f
path.cubicTo(SkBits2Float(0x43903333), SkBits2Float(0x42bdf222), SkBits2Float(0x43908ccd), SkBits2Float(0x42bdeb8a), SkBits2Float(0x4390e666), SkBits2Float(0x42bdd83a));  // 288.4f, 94.9729f, 289.1f, 94.96f, 289.8f, 94.9223f
path.cubicTo(SkBits2Float(0x43914000), SkBits2Float(0x42bdc4eb), SkBits2Float(0x4391999a), SkBits2Float(0x42bda6ea), SkBits2Float(0x4391f333), SkBits2Float(0x42bd832a));  // 290.5f, 94.8846f, 291.2f, 94.826f, 291.9f, 94.7562f
path.cubicTo(SkBits2Float(0x43924ccd), SkBits2Float(0x42bd5f6a), SkBits2Float(0x4392a666), SkBits2Float(0x42bd1798), SkBits2Float(0x43930000), SkBits2Float(0x42bd01bb));  // 292.6f, 94.6864f, 293.3f, 94.5461f, 294, 94.5034f
path.cubicTo(SkBits2Float(0x4393599a), SkBits2Float(0x42bcebdf), SkBits2Float(0x4393b333), SkBits2Float(0x42bd004a), SkBits2Float(0x43940ccd), SkBits2Float(0x42bd0000));  // 294.7f, 94.4607f, 295.4f, 94.5006f, 296.1f, 94.5f
path.lineTo(SkBits2Float(0x43940ccd), SkBits2Float(0x42bd0000));  // 296.1f, 94.5f
path.lineTo(SkBits2Float(0x43413333), SkBits2Float(0x42bd0000));  // 193.2f, 94.5f
path.close();
path.moveTo(SkBits2Float(0x43ac3333), SkBits2Float(0x42bd0000));  // 344.4f, 94.5f
path.lineTo(SkBits2Float(0x43ac3333), SkBits2Float(0x42bd0000));  // 344.4f, 94.5f
path.cubicTo(SkBits2Float(0x43ac8ccd), SkBits2Float(0x42bd0000), SkBits2Float(0x43ace666), SkBits2Float(0x42bcfc5e), SkBits2Float(0x43ad4000), SkBits2Float(0x42bd0000));  // 345.1f, 94.5f, 345.8f, 94.4929f, 346.5f, 94.5f
path.cubicTo(SkBits2Float(0x43ad999a), SkBits2Float(0x42bd03a2), SkBits2Float(0x43adf333), SkBits2Float(0x42bcf966), SkBits2Float(0x43ae4ccd), SkBits2Float(0x42bd15ce));  // 347.2f, 94.5071f, 347.9f, 94.4871f, 348.6f, 94.5426f
path.cubicTo(SkBits2Float(0x43aea666), SkBits2Float(0x42bd3236), SkBits2Float(0x43af0000), SkBits2Float(0x42bd84e8), SkBits2Float(0x43af599a), SkBits2Float(0x42bdaa71));  // 349.3f, 94.5981f, 350, 94.7596f, 350.7f, 94.8329f
path.cubicTo(SkBits2Float(0x43afb333), SkBits2Float(0x42bdcffb), SkBits2Float(0x43b00ccd), SkBits2Float(0x42bde92e), SkBits2Float(0x43b06666), SkBits2Float(0x42bdf707));  // 351.4f, 94.9062f, 352.1f, 94.9554f, 352.8f, 94.9825f
path.cubicTo(SkBits2Float(0x43b0c000), SkBits2Float(0x42be04e0), SkBits2Float(0x43b1199a), SkBits2Float(0x42bdfc2b), SkBits2Float(0x43b17333), SkBits2Float(0x42bdfd89));  // 353.5f, 95.0095f, 354.2f, 94.9925f, 354.9f, 94.9952f
path.cubicTo(SkBits2Float(0x43b1cccd), SkBits2Float(0x42bdfee7), SkBits2Float(0x43b22666), SkBits2Float(0x42bdfe62), SkBits2Float(0x43b28000), SkBits2Float(0x42bdff3b));  // 355.6f, 94.9979f, 356.3f, 94.9968f, 357, 94.9985f
path.cubicTo(SkBits2Float(0x43b2d99a), SkBits2Float(0x42be0014), SkBits2Float(0x43b33333), SkBits2Float(0x42be020f), SkBits2Float(0x43b38ccd), SkBits2Float(0x42be029f));  // 357.7f, 95.0002f, 358.4f, 95.004f, 359.1f, 95.0051f
path.cubicTo(SkBits2Float(0x43b3e666), SkBits2Float(0x42be0330), SkBits2Float(0x43b44000), SkBits2Float(0x42be020f), SkBits2Float(0x43b4999a), SkBits2Float(0x42be029f));  // 359.8f, 95.0062f, 360.5f, 95.004f, 361.2f, 95.0051f
path.cubicTo(SkBits2Float(0x43b4f333), SkBits2Float(0x42be0330), SkBits2Float(0x43b54ccd), SkBits2Float(0x42be052a), SkBits2Float(0x43b5a666), SkBits2Float(0x42be0604));  // 361.9f, 95.0062f, 362.6f, 95.0101f, 363.3f, 95.0117f
path.cubicTo(SkBits2Float(0x43b60000), SkBits2Float(0x42be06dd), SkBits2Float(0x43b6599a), SkBits2Float(0x42be0725), SkBits2Float(0x43b6b333), SkBits2Float(0x42be07b6));  // 364, 95.0134f, 364.7f, 95.014f, 365.4f, 95.0151f
path.cubicTo(SkBits2Float(0x43b70ccd), SkBits2Float(0x42be0846), SkBits2Float(0x43b76666), SkBits2Float(0x42be08d7), SkBits2Float(0x43b7c000), SkBits2Float(0x42be0968));  // 366.1f, 95.0162f, 366.8f, 95.0173f, 367.5f, 95.0184f
path.cubicTo(SkBits2Float(0x43b8199a), SkBits2Float(0x42be09f8), SkBits2Float(0x43b87333), SkBits2Float(0x42be0ad2), SkBits2Float(0x43b8cccd), SkBits2Float(0x42be0b1a));  // 368.2f, 95.0195f, 368.9f, 95.0211f, 369.6f, 95.0217f
path.cubicTo(SkBits2Float(0x43b92666), SkBits2Float(0x42be0b62), SkBits2Float(0x43b98000), SkBits2Float(0x42be0bab), SkBits2Float(0x43b9d99a), SkBits2Float(0x42be0b1a));  // 370.3f, 95.0222f, 371, 95.0228f, 371.7f, 95.0217f
path.cubicTo(SkBits2Float(0x43ba3333), SkBits2Float(0x42be0a89), SkBits2Float(0x43ba8ccd), SkBits2Float(0x42be088f), SkBits2Float(0x43bae666), SkBits2Float(0x42be07b6));  // 372.4f, 95.0206f, 373.1f, 95.0167f, 373.8f, 95.0151f
path.cubicTo(SkBits2Float(0x43bb4000), SkBits2Float(0x42be06dd), SkBits2Float(0x43bb999a), SkBits2Float(0x42be064c), SkBits2Float(0x43bbf333), SkBits2Float(0x42be0604));  // 374.5f, 95.0134f, 375.2f, 95.0123f, 375.9f, 95.0117f
path.cubicTo(SkBits2Float(0x43bc4ccd), SkBits2Float(0x42be05bb), SkBits2Float(0x43bca666), SkBits2Float(0x42be0604), SkBits2Float(0x43bd0000), SkBits2Float(0x42be0604));  // 376.6f, 95.0112f, 377.3f, 95.0117f, 378, 95.0117f
path.cubicTo(SkBits2Float(0x43bd599a), SkBits2Float(0x42be0604), SkBits2Float(0x43bdb333), SkBits2Float(0x42be0604), SkBits2Float(0x43be0ccd), SkBits2Float(0x42be0604));  // 378.7f, 95.0117f, 379.4f, 95.0117f, 380.1f, 95.0117f
path.cubicTo(SkBits2Float(0x43be6666), SkBits2Float(0x42be0604), SkBits2Float(0x43bec000), SkBits2Float(0x42be0725), SkBits2Float(0x43bf199a), SkBits2Float(0x42be0604));  // 380.8f, 95.0117f, 381.5f, 95.014f, 382.2f, 95.0117f
path.cubicTo(SkBits2Float(0x43bf7333), SkBits2Float(0x42be04e2), SkBits2Float(0x43bfcccd), SkBits2Float(0x42be0136), SkBits2Float(0x43c02666), SkBits2Float(0x42bdff3b));  // 382.9f, 95.0095f, 383.6f, 95.0024f, 384.3f, 94.9985f
path.cubicTo(SkBits2Float(0x43c08000), SkBits2Float(0x42bdfd41), SkBits2Float(0x43c0d99a), SkBits2Float(0x42bdfa6d), SkBits2Float(0x43c13333), SkBits2Float(0x42bdfa25));  // 385, 94.9946f, 385.7f, 94.9891f, 386.4f, 94.9886f
path.cubicTo(SkBits2Float(0x43c18ccd), SkBits2Float(0x42bdf9dc), SkBits2Float(0x43c1e666), SkBits2Float(0x42bdfcf8), SkBits2Float(0x43c24000), SkBits2Float(0x42bdfd89));  // 387.1f, 94.988f, 387.8f, 94.9941f, 388.5f, 94.9952f
path.cubicTo(SkBits2Float(0x43c2999a), SkBits2Float(0x42bdfe1a), SkBits2Float(0x43c2f333), SkBits2Float(0x42bdfdd1), SkBits2Float(0x43c34ccd), SkBits2Float(0x42bdfd89));  // 389.2f, 94.9963f, 389.9f, 94.9957f, 390.6f, 94.9952f
path.cubicTo(SkBits2Float(0x43c3a666), SkBits2Float(0x42bdfd41), SkBits2Float(0x43c40000), SkBits2Float(0x42bdfd41), SkBits2Float(0x43c4599a), SkBits2Float(0x42bdfbd7));  // 391.3f, 94.9946f, 392, 94.9946f, 392.7f, 94.9919f
path.cubicTo(SkBits2Float(0x43c4b333), SkBits2Float(0x42bdfa6d), SkBits2Float(0x43c50ccd), SkBits2Float(0x42bdf709), SkBits2Float(0x43c56666), SkBits2Float(0x42bdf50e));  // 393.4f, 94.9891f, 394.1f, 94.9825f, 394.8f, 94.9786f
path.cubicTo(SkBits2Float(0x43c5c000), SkBits2Float(0x42bdf314), SkBits2Float(0x43c6199a), SkBits2Float(0x42bdf0d1), SkBits2Float(0x43c67333), SkBits2Float(0x42bdeff8));  // 395.5f, 94.9748f, 396.2f, 94.9703f, 396.9f, 94.9687f
path.cubicTo(SkBits2Float(0x43c6cccd), SkBits2Float(0x42bdef1f), SkBits2Float(0x43c72666), SkBits2Float(0x42bdf040), SkBits2Float(0x43c78000), SkBits2Float(0x42bdeff8));  // 397.6f, 94.967f, 398.3f, 94.9692f, 399, 94.9687f
path.cubicTo(SkBits2Float(0x43c7d99a), SkBits2Float(0x42bdefb0), SkBits2Float(0x43c83333), SkBits2Float(0x42bdee8e), SkBits2Float(0x43c88ccd), SkBits2Float(0x42bdee46));  // 399.7f, 94.9681f, 400.4f, 94.9659f, 401.1f, 94.9654f
path.cubicTo(SkBits2Float(0x43c8e666), SkBits2Float(0x42bdedfe), SkBits2Float(0x43c94000), SkBits2Float(0x42bdee46), SkBits2Float(0x43c9999a), SkBits2Float(0x42bdee46));  // 401.8f, 94.9648f, 402.5f, 94.9654f, 403.2f, 94.9654f
path.cubicTo(SkBits2Float(0x43c9f333), SkBits2Float(0x42bdee46), SkBits2Float(0x43ca4ccd), SkBits2Float(0x42bdedb5), SkBits2Float(0x43caa666), SkBits2Float(0x42bdee46));  // 403.9f, 94.9654f, 404.6f, 94.9643f, 405.3f, 94.9654f
path.cubicTo(SkBits2Float(0x43cb0000), SkBits2Float(0x42bdeed7), SkBits2Float(0x43cb599a), SkBits2Float(0x42bdf089), SkBits2Float(0x43cbb333), SkBits2Float(0x42bdf1aa));  // 406, 94.9665f, 406.7f, 94.9698f, 407.4f, 94.972f
path.cubicTo(SkBits2Float(0x43cc0ccd), SkBits2Float(0x42bdf2cc), SkBits2Float(0x43cc6666), SkBits2Float(0x42bdf47e), SkBits2Float(0x43ccc000), SkBits2Float(0x42bdf50e));  // 408.1f, 94.9742f, 408.8f, 94.9775f, 409.5f, 94.9786f
path.cubicTo(SkBits2Float(0x43cd199a), SkBits2Float(0x42bdf59f), SkBits2Float(0x43cd7333), SkBits2Float(0x42bdf4c6), SkBits2Float(0x43cdcccd), SkBits2Float(0x42bdf50e));  // 410.2f, 94.9797f, 410.9f, 94.9781f, 411.6f, 94.9786f
path.cubicTo(SkBits2Float(0x43ce2666), SkBits2Float(0x42bdf557), SkBits2Float(0x43ce8000), SkBits2Float(0x42bdf557), SkBits2Float(0x43ced99a), SkBits2Float(0x42bdf6c0));  // 412.3f, 94.9792f, 413, 94.9792f, 413.7f, 94.9819f
path.cubicTo(SkBits2Float(0x43cf3333), SkBits2Float(0x42bdf82a), SkBits2Float(0x43cf8ccd), SkBits2Float(0x42bdfafe), SkBits2Float(0x43cfe666), SkBits2Float(0x42bdfd89));  // 414.4f, 94.9847f, 415.1f, 94.9902f, 415.8f, 94.9952f
path.cubicTo(SkBits2Float(0x43d04000), SkBits2Float(0x42be0014), SkBits2Float(0x43d0999a), SkBits2Float(0x42be0378), SkBits2Float(0x43d0f333), SkBits2Float(0x42be0604));  // 416.5f, 95.0002f, 417.2f, 95.0068f, 417.9f, 95.0117f
path.cubicTo(SkBits2Float(0x43d14ccd), SkBits2Float(0x42be088f), SkBits2Float(0x43d1a666), SkBits2Float(0x42be0b1a), SkBits2Float(0x43d20000), SkBits2Float(0x42be0ccc));  // 418.6f, 95.0167f, 419.3f, 95.0217f, 420, 95.025f
path.cubicTo(SkBits2Float(0x43d2599a), SkBits2Float(0x42be0e7e), SkBits2Float(0x43d2b333), SkBits2Float(0x42be119a), SkBits2Float(0x43d30ccd), SkBits2Float(0x42be1030));  // 420.7f, 95.0283f, 421.4f, 95.0344f, 422.1f, 95.0316f
path.cubicTo(SkBits2Float(0x43d36666), SkBits2Float(0x42be0ec6), SkBits2Float(0x43d3c000), SkBits2Float(0x42be0ad2), SkBits2Float(0x43d4199a), SkBits2Float(0x42be0451));  // 422.8f, 95.0289f, 423.5f, 95.0211f, 424.2f, 95.0084f
path.cubicTo(SkBits2Float(0x43d47333), SkBits2Float(0x42bdfdd1), SkBits2Float(0x43d4cccd), SkBits2Float(0x42bdefec), SkBits2Float(0x43d52666), SkBits2Float(0x42bde930));  // 424.9f, 94.9957f, 425.6f, 94.9686f, 426.3f, 94.9554f
path.cubicTo(SkBits2Float(0x43d58000), SkBits2Float(0x42bde273), SkBits2Float(0x43d5d99a), SkBits2Float(0x42bde21d), SkBits2Float(0x43d63333), SkBits2Float(0x42bddbe5));  // 427, 94.9423f, 427.7f, 94.9416f, 428.4f, 94.9295f
path.cubicTo(SkBits2Float(0x43d68ccd), SkBits2Float(0x42bdd5ad), SkBits2Float(0x43d6e666), SkBits2Float(0x42bdcf3b), SkBits2Float(0x43d74000), SkBits2Float(0x42bdc3e1));  // 429.1f, 94.9173f, 429.8f, 94.9047f, 430.5f, 94.8826f
path.cubicTo(SkBits2Float(0x43d7999a), SkBits2Float(0x42bdb887), SkBits2Float(0x43d7f333), SkBits2Float(0x42bd9d7d), SkBits2Float(0x43d84ccd), SkBits2Float(0x42bd97ca));  // 431.2f, 94.8604f, 431.9f, 94.8076f, 432.6f, 94.7965f
path.cubicTo(SkBits2Float(0x43d8a666), SkBits2Float(0x42bd9217), SkBits2Float(0x43d90000), SkBits2Float(0x42bd9820), SkBits2Float(0x43d9599a), SkBits2Float(0x42bda1b0));  // 433.3f, 94.7853f, 434, 94.7971f, 434.7f, 94.8158f
path.cubicTo(SkBits2Float(0x43d9b333), SkBits2Float(0x42bdab40), SkBits2Float(0x43da0ccd), SkBits2Float(0x42bdc5dd), SkBits2Float(0x43da6666), SkBits2Float(0x42bdd12b));  // 435.4f, 94.8345f, 436.1f, 94.8865f, 436.8f, 94.9085f
path.cubicTo(SkBits2Float(0x43dac000), SkBits2Float(0x42bddc79), SkBits2Float(0x43db199a), SkBits2Float(0x42bde1cc), SkBits2Float(0x43db7333), SkBits2Float(0x42bde585));  // 437.5f, 94.9306f, 438.2f, 94.941f, 438.9f, 94.9483f
path.cubicTo(SkBits2Float(0x43dbcccd), SkBits2Float(0x42bde93d), SkBits2Float(0x43dc2666), SkBits2Float(0x42bde75a), SkBits2Float(0x43dc8000), SkBits2Float(0x42bde77d));  // 439.6f, 94.9555f, 440.3f, 94.9519f, 441, 94.9521f
path.cubicTo(SkBits2Float(0x43dcd99a), SkBits2Float(0x42bde7a1), SkBits2Float(0x43dd3333), SkBits2Float(0x42bde610), SkBits2Float(0x43dd8ccd), SkBits2Float(0x42bde658));  // 441.7f, 94.9524f, 442.4f, 94.9493f, 443.1f, 94.9499f
path.cubicTo(SkBits2Float(0x43dde666), SkBits2Float(0x42bde6a1), SkBits2Float(0x43de4000), SkBits2Float(0x42bdea2c), SkBits2Float(0x43de999a), SkBits2Float(0x42bde930));  // 443.8f, 94.9504f, 444.5f, 94.9574f, 445.2f, 94.9554f
path.cubicTo(SkBits2Float(0x43def333), SkBits2Float(0x42bde833), SkBits2Float(0x43df4ccd), SkBits2Float(0x42bdee5f), SkBits2Float(0x43dfa666), SkBits2Float(0x42bde06e));  // 445.9f, 94.9535f, 446.6f, 94.9656f, 447.3f, 94.9383f
path.cubicTo(SkBits2Float(0x43e00000), SkBits2Float(0x42bdd27d), SkBits2Float(0x43e0599a), SkBits2Float(0x42bdb871), SkBits2Float(0x43e0b333), SkBits2Float(0x42bd958a));  // 448, 94.9111f, 448.7f, 94.8602f, 449.4f, 94.7921f
path.cubicTo(SkBits2Float(0x43e10ccd), SkBits2Float(0x42bd72a4), SkBits2Float(0x43e16666), SkBits2Float(0x42bd27f2), SkBits2Float(0x43e1c000), SkBits2Float(0x42bd0f06));  // 450.1f, 94.7239f, 450.8f, 94.578f, 451.5f, 94.5293f
path.cubicTo(SkBits2Float(0x43e2199a), SkBits2Float(0x42bcf619), SkBits2Float(0x43e27333), SkBits2Float(0x42bcfbb7), SkBits2Float(0x43e2cccd), SkBits2Float(0x42bd0000));  // 452.2f, 94.4807f, 452.9f, 94.4916f, 453.6f, 94.5f
path.cubicTo(SkBits2Float(0x43e32666), SkBits2Float(0x42bd0449), SkBits2Float(0x43e38000), SkBits2Float(0x42bd1419), SkBits2Float(0x43e3d99a), SkBits2Float(0x42bd28bc));  // 454.3f, 94.5084f, 455, 94.5393f, 455.7f, 94.5796f
path.cubicTo(SkBits2Float(0x43e43333), SkBits2Float(0x42bd3d60), SkBits2Float(0x43e48ccd), SkBits2Float(0x42bd7238), SkBits2Float(0x43e4e666), SkBits2Float(0x42bd7bd4));  // 456.4f, 94.6199f, 457.1f, 94.7231f, 457.8f, 94.7419f
path.cubicTo(SkBits2Float(0x43e54000), SkBits2Float(0x42bd8570), SkBits2Float(0x43e5999a), SkBits2Float(0x42bd70ce), SkBits2Float(0x43e5f333), SkBits2Float(0x42bd6264));  // 458.5f, 94.7606f, 459.2f, 94.7203f, 459.9f, 94.6922f
path.cubicTo(SkBits2Float(0x43e64ccd), SkBits2Float(0x42bd53fa), SkBits2Float(0x43e6a666), SkBits2Float(0x42bd35be), SkBits2Float(0x43e70000), SkBits2Float(0x42bd2558));  // 460.6f, 94.664f, 461.3f, 94.605f, 462, 94.5729f
path.cubicTo(SkBits2Float(0x43e7599a), SkBits2Float(0x42bd14f2), SkBits2Float(0x43e7b333), SkBits2Float(0x42bd0639), SkBits2Float(0x43e80ccd), SkBits2Float(0x42bd0000));  // 462.7f, 94.5409f, 463.4f, 94.5122f, 464.1f, 94.5f
path.cubicTo(SkBits2Float(0x43e86666), SkBits2Float(0x42bcf9c7), SkBits2Float(0x43e8c000), SkBits2Float(0x42bcfef5), SkBits2Float(0x43e9199a), SkBits2Float(0x42bd0000));  // 464.8f, 94.4878f, 465.5f, 94.498f, 466.2f, 94.5f
path.cubicTo(SkBits2Float(0x43e97333), SkBits2Float(0x42bd010b), SkBits2Float(0x43e9cccd), SkBits2Float(0x42bd0645), SkBits2Float(0x43ea2666), SkBits2Float(0x42bd0645));  // 466.9f, 94.502f, 467.6f, 94.5122f, 468.3f, 94.5122f
path.cubicTo(SkBits2Float(0x43ea8000), SkBits2Float(0x42bd0645), SkBits2Float(0x43ead99a), SkBits2Float(0x42bd010b), SkBits2Float(0x43eb3333), SkBits2Float(0x42bd0000));  // 469, 94.5122f, 469.7f, 94.502f, 470.4f, 94.5f
path.lineTo(SkBits2Float(0x43eb3333), SkBits2Float(0x42bd0000));  // 470.4f, 94.5f
path.lineTo(SkBits2Float(0x43ac3333), SkBits2Float(0x42bd0000));  // 344.4f, 94.5f
path.close();
    return path;
}

static SkPath path5() {
    SkPath path;
path.moveTo(SkBits2Float(0x42b06666), SkBits2Float(0x42c9999a));  // 88.2f, 100.8f
path.lineTo(SkBits2Float(0x42b06666), SkBits2Float(0x42c9999a));  // 88.2f, 100.8f
path.cubicTo(SkBits2Float(0x42b1cccd), SkBits2Float(0x42c9999a), SkBits2Float(0x42b33333), SkBits2Float(0x42c9b407), SkBits2Float(0x42b4999a), SkBits2Float(0x42c9999a));  // 88.9f, 100.8f, 89.6f, 100.852f, 90.3f, 100.8f
path.cubicTo(SkBits2Float(0x42b60000), SkBits2Float(0x42c97f2c), SkBits2Float(0x42b76666), SkBits2Float(0x42c91521), SkBits2Float(0x42b8cccd), SkBits2Float(0x42c8fb07));  // 91, 100.748f, 91.7f, 100.541f, 92.4f, 100.49f
path.cubicTo(SkBits2Float(0x42ba3333), SkBits2Float(0x42c8e0ee), SkBits2Float(0x42bb999a), SkBits2Float(0x42c8fd00), SkBits2Float(0x42bd0000), SkBits2Float(0x42c8fd00));  // 93.1f, 100.439f, 93.8f, 100.494f, 94.5f, 100.494f
path.cubicTo(SkBits2Float(0x42be6666), SkBits2Float(0x42c8fd00), SkBits2Float(0x42bfcccd), SkBits2Float(0x42c8facb), SkBits2Float(0x42c13333), SkBits2Float(0x42c8fb07));  // 95.2f, 100.494f, 95.9f, 100.49f, 96.6f, 100.49f
path.cubicTo(SkBits2Float(0x42c2999a), SkBits2Float(0x42c8fb44), SkBits2Float(0x42c40000), SkBits2Float(0x42c8fd4a), SkBits2Float(0x42c56666), SkBits2Float(0x42c8fe6c));  // 97.3f, 100.491f, 98, 100.495f, 98.7f, 100.497f
path.cubicTo(SkBits2Float(0x42c6cccd), SkBits2Float(0x42c8ff8d), SkBits2Float(0x42c83333), SkBits2Float(0x42c90218), SkBits2Float(0x42c9999a), SkBits2Float(0x42c901d0));  // 99.4f, 100.499f, 100.1f, 100.504f, 100.8f, 100.504f
path.cubicTo(SkBits2Float(0x42cb0000), SkBits2Float(0x42c90187), SkBits2Float(0x42cc6666), SkBits2Float(0x42c8ff39), SkBits2Float(0x42cdcccd), SkBits2Float(0x42c8fcb9));  // 101.5f, 100.503f, 102.2f, 100.498f, 102.9f, 100.494f
path.cubicTo(SkBits2Float(0x42cf3333), SkBits2Float(0x42c8fa3a), SkBits2Float(0x42d0999a), SkBits2Float(0x42c8f364), SkBits2Float(0x42d20000), SkBits2Float(0x42c8f2d3));  // 103.6f, 100.489f, 104.3f, 100.475f, 105, 100.474f
path.cubicTo(SkBits2Float(0x42d36666), SkBits2Float(0x42c8f243), SkBits2Float(0x42d4cccd), SkBits2Float(0x42c8f402), SkBits2Float(0x42d63333), SkBits2Float(0x42c8f955));  // 105.7f, 100.473f, 106.4f, 100.477f, 107.1f, 100.487f
path.cubicTo(SkBits2Float(0x42d7999a), SkBits2Float(0x42c8fea8), SkBits2Float(0x42d90000), SkBits2Float(0x42c90daf), SkBits2Float(0x42da6666), SkBits2Float(0x42c912c5));  // 107.8f, 100.497f, 108.5f, 100.527f, 109.2f, 100.537f
path.cubicTo(SkBits2Float(0x42dbcccd), SkBits2Float(0x42c917db), SkBits2Float(0x42dd3333), SkBits2Float(0x42c918f1), SkBits2Float(0x42de999a), SkBits2Float(0x42c917db));  // 109.9f, 100.547f, 110.6f, 100.549f, 111.3f, 100.547f
path.cubicTo(SkBits2Float(0x42e00000), SkBits2Float(0x42c916c6), SkBits2Float(0x42e16666), SkBits2Float(0x42c90ac2), SkBits2Float(0x42e2cccd), SkBits2Float(0x42c90c43));  // 112, 100.544f, 112.7f, 100.521f, 113.4f, 100.524f
path.cubicTo(SkBits2Float(0x42e43333), SkBits2Float(0x42c90dc4), SkBits2Float(0x42e5999a), SkBits2Float(0x42c91420), SkBits2Float(0x42e70000), SkBits2Float(0x42c920e3));  // 114.1f, 100.527f, 114.8f, 100.539f, 115.5f, 100.564f
path.cubicTo(SkBits2Float(0x42e86666), SkBits2Float(0x42c92da7), SkBits2Float(0x42e9cccd), SkBits2Float(0x42c946ab), SkBits2Float(0x42eb3333), SkBits2Float(0x42c958d9));  // 116.2f, 100.589f, 116.9f, 100.638f, 117.6f, 100.674f
path.cubicTo(SkBits2Float(0x42ec999a), SkBits2Float(0x42c96b07), SkBits2Float(0x42ee0000), SkBits2Float(0x42c9832d), SkBits2Float(0x42ef6666), SkBits2Float(0x42c98df8));  // 118.3f, 100.709f, 119, 100.756f, 119.7f, 100.777f
path.cubicTo(SkBits2Float(0x42f0cccd), SkBits2Float(0x42c998c3), SkBits2Float(0x42f23333), SkBits2Float(0x42c997a9), SkBits2Float(0x42f3999a), SkBits2Float(0x42c9999a));  // 120.4f, 100.798f, 121.1f, 100.796f, 121.8f, 100.8f
path.lineTo(SkBits2Float(0x42f3999a), SkBits2Float(0x42c9999a));  // 121.8f, 100.8f
path.lineTo(SkBits2Float(0x42b06666), SkBits2Float(0x42c9999a));  // 88.2f, 100.8f
path.close();
path.moveTo(SkBits2Float(0x4300199a), SkBits2Float(0x42c9999a));  // 128.1f, 100.8f
path.lineTo(SkBits2Float(0x4300199a), SkBits2Float(0x42c99825));  // 128.1f, 100.797f
path.cubicTo(SkBits2Float(0x4300cccd), SkBits2Float(0x42c99756), SkBits2Float(0x43018000), SkBits2Float(0x42c99482), SkBits2Float(0x43023333), SkBits2Float(0x42c994c1));  // 128.8f, 100.796f, 129.5f, 100.79f, 130.2f, 100.791f
path.cubicTo(SkBits2Float(0x4302e666), SkBits2Float(0x42c994ff), SkBits2Float(0x4303999a), SkBits2Float(0x42c998cb), SkBits2Float(0x43044ccd), SkBits2Float(0x42c9999a));  // 130.9f, 100.791f, 131.6f, 100.798f, 132.3f, 100.8f
path.lineTo(SkBits2Float(0x43044ccd), SkBits2Float(0x42c9999a));  // 132.3f, 100.8f
path.lineTo(SkBits2Float(0x4300199a), SkBits2Float(0x42c9999a));  // 128.1f, 100.8f
path.close();
path.moveTo(SkBits2Float(0x431b6666), SkBits2Float(0x42c9999a));  // 155.4f, 100.8f
path.lineTo(SkBits2Float(0x431b6666), SkBits2Float(0x42c9999a));  // 155.4f, 100.8f
path.cubicTo(SkBits2Float(0x431c199a), SkBits2Float(0x42c9999a), SkBits2Float(0x431ccccd), SkBits2Float(0x42c99dcd), SkBits2Float(0x431d8000), SkBits2Float(0x42c9999a));  // 156.1f, 100.8f, 156.8f, 100.808f, 157.5f, 100.8f
path.cubicTo(SkBits2Float(0x431e3333), SkBits2Float(0x42c99567), SkBits2Float(0x431ee666), SkBits2Float(0x42c98d21), SkBits2Float(0x431f999a), SkBits2Float(0x42c98067));  // 158.2f, 100.792f, 158.9f, 100.776f, 159.6f, 100.751f
path.cubicTo(SkBits2Float(0x43204ccd), SkBits2Float(0x42c973ae), SkBits2Float(0x43210000), SkBits2Float(0x42c95aa1), SkBits2Float(0x4321b333), SkBits2Float(0x42c94d41));  // 160.3f, 100.726f, 161, 100.677f, 161.7f, 100.651f
path.cubicTo(SkBits2Float(0x43226666), SkBits2Float(0x42c93fe1), SkBits2Float(0x4323199a), SkBits2Float(0x42c935e5), SkBits2Float(0x4323cccd), SkBits2Float(0x42c93026));  // 162.4f, 100.625f, 163.1f, 100.605f, 163.8f, 100.594f
path.cubicTo(SkBits2Float(0x43248000), SkBits2Float(0x42c92a68), SkBits2Float(0x43253333), SkBits2Float(0x42c92b66), SkBits2Float(0x4325e666), SkBits2Float(0x42c92ac9));  // 164.5f, 100.583f, 165.2f, 100.585f, 165.9f, 100.584f
path.cubicTo(SkBits2Float(0x4326999a), SkBits2Float(0x42c92a2d), SkBits2Float(0x43274ccd), SkBits2Float(0x42c92759), SkBits2Float(0x43280000), SkBits2Float(0x42c92c7b));  // 166.6f, 100.582f, 167.3f, 100.577f, 168, 100.587f
path.cubicTo(SkBits2Float(0x4328b333), SkBits2Float(0x42c9319e), SkBits2Float(0x43296666), SkBits2Float(0x42c93d57), SkBits2Float(0x432a199a), SkBits2Float(0x42c94996));  // 168.7f, 100.597f, 169.4f, 100.62f, 170.1f, 100.644f
path.cubicTo(SkBits2Float(0x432acccd), SkBits2Float(0x42c955d5), SkBits2Float(0x432b8000), SkBits2Float(0x42c968e8), SkBits2Float(0x432c3333), SkBits2Float(0x42c975f4));  // 170.8f, 100.668f, 171.5f, 100.705f, 172.2f, 100.73f
path.cubicTo(SkBits2Float(0x432ce666), SkBits2Float(0x42c98300), SkBits2Float(0x432d999a), SkBits2Float(0x42c991ed), SkBits2Float(0x432e4ccd), SkBits2Float(0x42c997de));  // 172.9f, 100.756f, 173.6f, 100.785f, 174.3f, 100.797f
path.cubicTo(SkBits2Float(0x432f0000), SkBits2Float(0x42c99dcf), SkBits2Float(0x432fb333), SkBits2Float(0x42c99950), SkBits2Float(0x43306666), SkBits2Float(0x42c9999a));  // 175, 100.808f, 175.7f, 100.799f, 176.4f, 100.8f
path.lineTo(SkBits2Float(0x43306666), SkBits2Float(0x42c9999a));  // 176.4f, 100.8f
path.lineTo(SkBits2Float(0x431b6666), SkBits2Float(0x42c9999a));  // 155.4f, 100.8f
path.close();
path.moveTo(SkBits2Float(0x43478000), SkBits2Float(0x42c9999a));  // 199.5f, 100.8f
path.lineTo(SkBits2Float(0x43478000), SkBits2Float(0x42c9999a));  // 199.5f, 100.8f
path.cubicTo(SkBits2Float(0x43483333), SkBits2Float(0x42c9999a), SkBits2Float(0x4348e666), SkBits2Float(0x42c99cf4), SkBits2Float(0x4349999a), SkBits2Float(0x42c9999a));  // 200.2f, 100.8f, 200.9f, 100.807f, 201.6f, 100.8f
path.cubicTo(SkBits2Float(0x434a4ccd), SkBits2Float(0x42c99640), SkBits2Float(0x434b0000), SkBits2Float(0x42c98bf3), SkBits2Float(0x434bb333), SkBits2Float(0x42c9857d));  // 202.3f, 100.793f, 203, 100.773f, 203.7f, 100.761f
path.cubicTo(SkBits2Float(0x434c6666), SkBits2Float(0x42c97f08), SkBits2Float(0x434d199a), SkBits2Float(0x42c9771f), SkBits2Float(0x434dcccd), SkBits2Float(0x42c972d6));  // 204.4f, 100.748f, 205.1f, 100.733f, 205.8f, 100.724f
path.cubicTo(SkBits2Float(0x434e8000), SkBits2Float(0x42c96e8d), SkBits2Float(0x434f3333), SkBits2Float(0x42c96dc2), SkBits2Float(0x434fe666), SkBits2Float(0x42c96bc7));  // 206.5f, 100.716f, 207.2f, 100.714f, 207.9f, 100.711f
path.cubicTo(SkBits2Float(0x4350999a), SkBits2Float(0x42c969cd), SkBits2Float(0x43514ccd), SkBits2Float(0x42c967c5), SkBits2Float(0x43520000), SkBits2Float(0x42c966f7));  // 208.6f, 100.707f, 209.3f, 100.703f, 210, 100.701f
path.cubicTo(SkBits2Float(0x4352b333), SkBits2Float(0x42c9662a), SkBits2Float(0x43536666), SkBits2Float(0x42c966af), SkBits2Float(0x4354199a), SkBits2Float(0x42c966f7));  // 210.7f, 100.7f, 211.4f, 100.701f, 212.1f, 100.701f
path.cubicTo(SkBits2Float(0x4354cccd), SkBits2Float(0x42c96740), SkBits2Float(0x43558000), SkBits2Float(0x42c96b40), SkBits2Float(0x43563333), SkBits2Float(0x42c968a9));  // 212.8f, 100.702f, 213.5f, 100.709f, 214.2f, 100.704f
path.cubicTo(SkBits2Float(0x4356e666), SkBits2Float(0x42c96612), SkBits2Float(0x4357999a), SkBits2Float(0x42c9624e), SkBits2Float(0x43584ccd), SkBits2Float(0x42c9576e));  // 214.9f, 100.699f, 215.6f, 100.692f, 216.3f, 100.671f
path.cubicTo(SkBits2Float(0x43590000), SkBits2Float(0x42c94c8d), SkBits2Float(0x4359b333), SkBits2Float(0x42c935e7), SkBits2Float(0x435a6666), SkBits2Float(0x42c92765));  // 217, 100.65f, 217.7f, 100.605f, 218.4f, 100.577f
path.cubicTo(SkBits2Float(0x435b199a), SkBits2Float(0x42c918e4), SkBits2Float(0x435bcccd), SkBits2Float(0x42c9097b), SkBits2Float(0x435c8000), SkBits2Float(0x42c90064));  // 219.1f, 100.549f, 219.8f, 100.519f, 220.5f, 100.501f
path.cubicTo(SkBits2Float(0x435d3333), SkBits2Float(0x42c8f74d), SkBits2Float(0x435de666), SkBits2Float(0x42c8f840), SkBits2Float(0x435e999a), SkBits2Float(0x42c8f0db));  // 221.2f, 100.483f, 221.9f, 100.485f, 222.6f, 100.47f
path.cubicTo(SkBits2Float(0x435f4ccd), SkBits2Float(0x42c8e976), SkBits2Float(0x43600000), SkBits2Float(0x42c8df18), SkBits2Float(0x4360b333), SkBits2Float(0x42c8d407));  // 223.3f, 100.456f, 224, 100.436f, 224.7f, 100.414f
path.cubicTo(SkBits2Float(0x43616666), SkBits2Float(0x42c8c8f5), SkBits2Float(0x4362199a), SkBits2Float(0x42c8b92f), SkBits2Float(0x4362cccd), SkBits2Float(0x42c8ae71));  // 225.4f, 100.392f, 226.1f, 100.362f, 226.8f, 100.341f
path.cubicTo(SkBits2Float(0x43638000), SkBits2Float(0x42c8a3b4), SkBits2Float(0x43643333), SkBits2Float(0x42c89926), SkBits2Float(0x4364e666), SkBits2Float(0x42c89396));  // 227.5f, 100.32f, 228.2f, 100.299f, 228.9f, 100.288f
path.cubicTo(SkBits2Float(0x4365999a), SkBits2Float(0x42c88e07), SkBits2Float(0x43664ccd), SkBits2Float(0x42c88baa), SkBits2Float(0x43670000), SkBits2Float(0x42c88d14));  // 229.6f, 100.277f, 230.3f, 100.273f, 231, 100.276f
path.cubicTo(SkBits2Float(0x4367b333), SkBits2Float(0x42c88e7e), SkBits2Float(0x43686666), SkBits2Float(0x42c89675), SkBits2Float(0x4369199a), SkBits2Float(0x42c89c11));  // 231.7f, 100.278f, 232.4f, 100.294f, 233.1f, 100.305f
path.cubicTo(SkBits2Float(0x4369cccd), SkBits2Float(0x42c8a1ac), SkBits2Float(0x436a8000), SkBits2Float(0x42c8a9ea), SkBits2Float(0x436b3333), SkBits2Float(0x42c8aeb8));  // 233.8f, 100.316f, 234.5f, 100.332f, 235.2f, 100.341f
path.cubicTo(SkBits2Float(0x436be666), SkBits2Float(0x42c8b386), SkBits2Float(0x436c999a), SkBits2Float(0x42c8b733), SkBits2Float(0x436d4ccd), SkBits2Float(0x42c8b8e5));  // 235.9f, 100.351f, 236.6f, 100.358f, 237.3f, 100.361f
path.cubicTo(SkBits2Float(0x436e0000), SkBits2Float(0x42c8ba97), SkBits2Float(0x436eb333), SkBits2Float(0x42c8b9be), SkBits2Float(0x436f6666), SkBits2Float(0x42c8b8e5));  // 238, 100.364f, 238.7f, 100.363f, 239.4f, 100.361f
path.cubicTo(SkBits2Float(0x4370199a), SkBits2Float(0x42c8b80c), SkBits2Float(0x4370cccd), SkBits2Float(0x42c8b45f), SkBits2Float(0x43718000), SkBits2Float(0x42c8b3ce));  // 240.1f, 100.359f, 240.8f, 100.352f, 241.5f, 100.351f
path.cubicTo(SkBits2Float(0x43723333), SkBits2Float(0x42c8b33e), SkBits2Float(0x4372e666), SkBits2Float(0x42c8b4f0), SkBits2Float(0x4373999a), SkBits2Float(0x42c8b580));  // 242.2f, 100.35f, 242.9f, 100.353f, 243.6f, 100.354f
path.cubicTo(SkBits2Float(0x43744ccd), SkBits2Float(0x42c8b611), SkBits2Float(0x43750000), SkBits2Float(0x42c8b6ea), SkBits2Float(0x4375b333), SkBits2Float(0x42c8b733));  // 244.3f, 100.356f, 245, 100.357f, 245.7f, 100.358f
path.cubicTo(SkBits2Float(0x43766666), SkBits2Float(0x42c8b77b), SkBits2Float(0x4377199a), SkBits2Float(0x42c8b77b), SkBits2Float(0x4377cccd), SkBits2Float(0x42c8b733));  // 246.4f, 100.358f, 247.1f, 100.358f, 247.8f, 100.358f
path.cubicTo(SkBits2Float(0x43788000), SkBits2Float(0x42c8b6ea), SkBits2Float(0x43793333), SkBits2Float(0x42c8b5c9), SkBits2Float(0x4379e666), SkBits2Float(0x42c8b580));  // 248.5f, 100.357f, 249.2f, 100.355f, 249.9f, 100.354f
path.cubicTo(SkBits2Float(0x437a999a), SkBits2Float(0x42c8b538), SkBits2Float(0x437b4ccd), SkBits2Float(0x42c8b538), SkBits2Float(0x437c0000), SkBits2Float(0x42c8b580));  // 250.6f, 100.354f, 251.3f, 100.354f, 252, 100.354f
path.cubicTo(SkBits2Float(0x437cb333), SkBits2Float(0x42c8b5c9), SkBits2Float(0x437d6666), SkBits2Float(0x42c8b6ea), SkBits2Float(0x437e199a), SkBits2Float(0x42c8b733));  // 252.7f, 100.355f, 253.4f, 100.357f, 254.1f, 100.358f
path.cubicTo(SkBits2Float(0x437ecccd), SkBits2Float(0x42c8b77b), SkBits2Float(0x437f8000), SkBits2Float(0x42c8b77b), SkBits2Float(0x4380199a), SkBits2Float(0x42c8b733));  // 254.8f, 100.358f, 255.5f, 100.358f, 256.2f, 100.358f
path.cubicTo(SkBits2Float(0x43807333), SkBits2Float(0x42c8b6ea), SkBits2Float(0x4380cccd), SkBits2Float(0x42c8b5c9), SkBits2Float(0x43812666), SkBits2Float(0x42c8b580));  // 256.9f, 100.357f, 257.6f, 100.355f, 258.3f, 100.354f
path.cubicTo(SkBits2Float(0x43818000), SkBits2Float(0x42c8b538), SkBits2Float(0x4381d99a), SkBits2Float(0x42c8b580), SkBits2Float(0x43823333), SkBits2Float(0x42c8b580));  // 259, 100.354f, 259.7f, 100.354f, 260.4f, 100.354f
path.cubicTo(SkBits2Float(0x43828ccd), SkBits2Float(0x42c8b580), SkBits2Float(0x4382e666), SkBits2Float(0x42c8b580), SkBits2Float(0x43834000), SkBits2Float(0x42c8b580));  // 261.1f, 100.354f, 261.8f, 100.354f, 262.5f, 100.354f
path.cubicTo(SkBits2Float(0x4383999a), SkBits2Float(0x42c8b580), SkBits2Float(0x4383f333), SkBits2Float(0x42c8b5c9), SkBits2Float(0x43844ccd), SkBits2Float(0x42c8b580));  // 263.2f, 100.354f, 263.9f, 100.355f, 264.6f, 100.354f
path.cubicTo(SkBits2Float(0x4384a666), SkBits2Float(0x42c8b538), SkBits2Float(0x43850000), SkBits2Float(0x42c8b417), SkBits2Float(0x4385599a), SkBits2Float(0x42c8b3ce));  // 265.3f, 100.354f, 266, 100.352f, 266.7f, 100.351f
path.cubicTo(SkBits2Float(0x4385b333), SkBits2Float(0x42c8b386), SkBits2Float(0x43860ccd), SkBits2Float(0x42c8b386), SkBits2Float(0x43866666), SkBits2Float(0x42c8b3ce));  // 267.4f, 100.351f, 268.1f, 100.351f, 268.8f, 100.351f
path.cubicTo(SkBits2Float(0x4386c000), SkBits2Float(0x42c8b417), SkBits2Float(0x4387199a), SkBits2Float(0x42c8b4f0), SkBits2Float(0x43877333), SkBits2Float(0x42c8b580));  // 269.5f, 100.352f, 270.2f, 100.353f, 270.9f, 100.354f
path.cubicTo(SkBits2Float(0x4387cccd), SkBits2Float(0x42c8b611), SkBits2Float(0x43882666), SkBits2Float(0x42c8b6ea), SkBits2Float(0x43888000), SkBits2Float(0x42c8b733));  // 271.6f, 100.356f, 272.3f, 100.357f, 273, 100.358f
path.cubicTo(SkBits2Float(0x4388d99a), SkBits2Float(0x42c8b77b), SkBits2Float(0x43893333), SkBits2Float(0x42c8b6a2), SkBits2Float(0x43898ccd), SkBits2Float(0x42c8b733));  // 273.7f, 100.358f, 274.4f, 100.357f, 275.1f, 100.358f
path.cubicTo(SkBits2Float(0x4389e666), SkBits2Float(0x42c8b7c3), SkBits2Float(0x438a4000), SkBits2Float(0x42c8ba97), SkBits2Float(0x438a999a), SkBits2Float(0x42c8ba97));  // 275.8f, 100.359f, 276.5f, 100.364f, 277.2f, 100.364f
path.cubicTo(SkBits2Float(0x438af333), SkBits2Float(0x42c8ba97), SkBits2Float(0x438b4ccd), SkBits2Float(0x42c8ba5a), SkBits2Float(0x438ba666), SkBits2Float(0x42c8b733));  // 277.9f, 100.364f, 278.6f, 100.364f, 279.3f, 100.358f
path.cubicTo(SkBits2Float(0x438c0000), SkBits2Float(0x42c8b40b), SkBits2Float(0x438c599a), SkBits2Float(0x42c8aad1), SkBits2Float(0x438cb333), SkBits2Float(0x42c8a7a9));  // 280, 100.352f, 280.7f, 100.334f, 281.4f, 100.327f
path.cubicTo(SkBits2Float(0x438d0ccd), SkBits2Float(0x42c8a481), SkBits2Float(0x438d6666), SkBits2Float(0x42c89f23), SkBits2Float(0x438dc000), SkBits2Float(0x42c8a445));  // 282.1f, 100.321f, 282.8f, 100.311f, 283.5f, 100.321f
path.cubicTo(SkBits2Float(0x438e199a), SkBits2Float(0x42c8a967), SkBits2Float(0x438e7333), SkBits2Float(0x42c8b67f), SkBits2Float(0x438ecccd), SkBits2Float(0x42c8c676));  // 284.2f, 100.331f, 284.9f, 100.356f, 285.6f, 100.388f
path.cubicTo(SkBits2Float(0x438f2666), SkBits2Float(0x42c8d66d), SkBits2Float(0x438f8000), SkBits2Float(0x42c8ecb3), SkBits2Float(0x438fd99a), SkBits2Float(0x42c9040f));  // 286.3f, 100.419f, 287, 100.462f, 287.7f, 100.508f
path.cubicTo(SkBits2Float(0x43903333), SkBits2Float(0x42c91b6b), SkBits2Float(0x43908ccd), SkBits2Float(0x42c939b1), SkBits2Float(0x4390e666), SkBits2Float(0x42c9529e));  // 288.4f, 100.554f, 289.1f, 100.613f, 289.8f, 100.661f
path.cubicTo(SkBits2Float(0x43914000), SkBits2Float(0x42c96b8a), SkBits2Float(0x4391999a), SkBits2Float(0x42c98dc5), SkBits2Float(0x4391f333), SkBits2Float(0x42c9999a));  // 290.5f, 100.71f, 291.2f, 100.777f, 291.9f, 100.8f
path.cubicTo(SkBits2Float(0x43924ccd), SkBits2Float(0x42c9a56e), SkBits2Float(0x4392a666), SkBits2Float(0x42c9999a), SkBits2Float(0x43930000), SkBits2Float(0x42c9999a));  // 292.6f, 100.823f, 293.3f, 100.8f, 294, 100.8f
path.lineTo(SkBits2Float(0x43930000), SkBits2Float(0x42c9999a));  // 294, 100.8f
path.lineTo(SkBits2Float(0x43478000), SkBits2Float(0x42c9999a));  // 199.5f, 100.8f
path.close();
path.moveTo(SkBits2Float(0x43ab2666), SkBits2Float(0x42c9999a));  // 342.3f, 100.8f
path.lineTo(SkBits2Float(0x43ab2666), SkBits2Float(0x42c9999a));  // 342.3f, 100.8f
path.cubicTo(SkBits2Float(0x43ab8000), SkBits2Float(0x42c9999a), SkBits2Float(0x43abd99a), SkBits2Float(0x42c9a526), SkBits2Float(0x43ac3333), SkBits2Float(0x42c9999a));  // 343, 100.8f, 343.7f, 100.823f, 344.4f, 100.8f
path.cubicTo(SkBits2Float(0x43ac8ccd), SkBits2Float(0x42c98e0d), SkBits2Float(0x43ace666), SkBits2Float(0x42c9760b), SkBits2Float(0x43ad4000), SkBits2Float(0x42c95450));  // 345.1f, 100.777f, 345.8f, 100.731f, 346.5f, 100.665f
path.cubicTo(SkBits2Float(0x43ad999a), SkBits2Float(0x42c93295), SkBits2Float(0x43adf333), SkBits2Float(0x42c8ebfd), SkBits2Float(0x43ae4ccd), SkBits2Float(0x42c8cf37));  // 347.2f, 100.599f, 347.9f, 100.461f, 348.6f, 100.405f
path.cubicTo(SkBits2Float(0x43aea666), SkBits2Float(0x42c8b270), SkBits2Float(0x43af0000), SkBits2Float(0x42c8afe7), SkBits2Float(0x43af599a), SkBits2Float(0x42c8a7a9));  // 349.3f, 100.349f, 350, 100.344f, 350.7f, 100.327f
path.cubicTo(SkBits2Float(0x43afb333), SkBits2Float(0x42c89f6b), SkBits2Float(0x43b00ccd), SkBits2Float(0x42c8a08b), SkBits2Float(0x43b06666), SkBits2Float(0x42c89dc3));  // 351.4f, 100.311f, 352.1f, 100.314f, 352.8f, 100.308f
path.cubicTo(SkBits2Float(0x43b0c000), SkBits2Float(0x42c89afb), SkBits2Float(0x43b1199a), SkBits2Float(0x42c89864), SkBits2Float(0x43b17333), SkBits2Float(0x42c896fa));  // 353.5f, 100.303f, 354.2f, 100.298f, 354.9f, 100.295f
path.cubicTo(SkBits2Float(0x43b1cccd), SkBits2Float(0x42c89591), SkBits2Float(0x43b22666), SkBits2Float(0x42c89591), SkBits2Float(0x43b28000), SkBits2Float(0x42c89548));  // 355.6f, 100.292f, 356.3f, 100.292f, 357, 100.292f
path.cubicTo(SkBits2Float(0x43b2d99a), SkBits2Float(0x42c89500), SkBits2Float(0x43b33333), SkBits2Float(0x42c89500), SkBits2Float(0x43b38ccd), SkBits2Float(0x42c89548));  // 357.7f, 100.291f, 358.4f, 100.291f, 359.1f, 100.292f
path.cubicTo(SkBits2Float(0x43b3e666), SkBits2Float(0x42c89591), SkBits2Float(0x43b44000), SkBits2Float(0x42c896b2), SkBits2Float(0x43b4999a), SkBits2Float(0x42c896fa));  // 359.8f, 100.292f, 360.5f, 100.294f, 361.2f, 100.295f
path.cubicTo(SkBits2Float(0x43b4f333), SkBits2Float(0x42c89743), SkBits2Float(0x43b54ccd), SkBits2Float(0x42c896fa), SkBits2Float(0x43b5a666), SkBits2Float(0x42c896fa));  // 361.9f, 100.295f, 362.6f, 100.295f, 363.3f, 100.295f
path.cubicTo(SkBits2Float(0x43b60000), SkBits2Float(0x42c896fa), SkBits2Float(0x43b6599a), SkBits2Float(0x42c89743), SkBits2Float(0x43b6b333), SkBits2Float(0x42c896fa));  // 364, 100.295f, 364.7f, 100.295f, 365.4f, 100.295f
path.cubicTo(SkBits2Float(0x43b70ccd), SkBits2Float(0x42c896b2), SkBits2Float(0x43b76666), SkBits2Float(0x42c89591), SkBits2Float(0x43b7c000), SkBits2Float(0x42c89548));  // 366.1f, 100.294f, 366.8f, 100.292f, 367.5f, 100.292f
path.cubicTo(SkBits2Float(0x43b8199a), SkBits2Float(0x42c89500), SkBits2Float(0x43b87333), SkBits2Float(0x42c89548), SkBits2Float(0x43b8cccd), SkBits2Float(0x42c89548));  // 368.2f, 100.291f, 368.9f, 100.292f, 369.6f, 100.292f
path.cubicTo(SkBits2Float(0x43b92666), SkBits2Float(0x42c89548), SkBits2Float(0x43b98000), SkBits2Float(0x42c89548), SkBits2Float(0x43b9d99a), SkBits2Float(0x42c89548));  // 370.3f, 100.292f, 371, 100.292f, 371.7f, 100.292f
path.cubicTo(SkBits2Float(0x43ba3333), SkBits2Float(0x42c89548), SkBits2Float(0x43ba8ccd), SkBits2Float(0x42c894b7), SkBits2Float(0x43bae666), SkBits2Float(0x42c89548));  // 372.4f, 100.292f, 373.1f, 100.29f, 373.8f, 100.292f
path.cubicTo(SkBits2Float(0x43bb4000), SkBits2Float(0x42c895d9), SkBits2Float(0x43bb999a), SkBits2Float(0x42c897d3), SkBits2Float(0x43bbf333), SkBits2Float(0x42c898ac));  // 374.5f, 100.293f, 375.2f, 100.297f, 375.9f, 100.298f
path.cubicTo(SkBits2Float(0x43bc4ccd), SkBits2Float(0x42c89985), SkBits2Float(0x43bca666), SkBits2Float(0x42c89a16), SkBits2Float(0x43bd0000), SkBits2Float(0x42c89a5f));  // 376.6f, 100.3f, 377.3f, 100.301f, 378, 100.302f
path.cubicTo(SkBits2Float(0x43bd599a), SkBits2Float(0x42c89aa7), SkBits2Float(0x43bdb333), SkBits2Float(0x42c89aa7), SkBits2Float(0x43be0ccd), SkBits2Float(0x42c89a5f));  // 378.7f, 100.302f, 379.4f, 100.302f, 380.1f, 100.302f
path.cubicTo(SkBits2Float(0x43be6666), SkBits2Float(0x42c89a16), SkBits2Float(0x43bec000), SkBits2Float(0x42c8993d), SkBits2Float(0x43bf199a), SkBits2Float(0x42c898ac));  // 380.8f, 100.301f, 381.5f, 100.299f, 382.2f, 100.298f
path.cubicTo(SkBits2Float(0x43bf7333), SkBits2Float(0x42c8981c), SkBits2Float(0x43bfcccd), SkBits2Float(0x42c8978b), SkBits2Float(0x43c02666), SkBits2Float(0x42c896fa));  // 382.9f, 100.297f, 383.6f, 100.296f, 384.3f, 100.295f
path.cubicTo(SkBits2Float(0x43c08000), SkBits2Float(0x42c8966a), SkBits2Float(0x43c0d99a), SkBits2Float(0x42c8946f), SkBits2Float(0x43c13333), SkBits2Float(0x42c89548));  // 385, 100.294f, 385.7f, 100.29f, 386.4f, 100.292f
path.cubicTo(SkBits2Float(0x43c18ccd), SkBits2Float(0x42c89621), SkBits2Float(0x43c1e666), SkBits2Float(0x42c898f5), SkBits2Float(0x43c24000), SkBits2Float(0x42c89c11));  // 387.1f, 100.293f, 387.8f, 100.299f, 388.5f, 100.305f
path.cubicTo(SkBits2Float(0x43c2999a), SkBits2Float(0x42c89f2d), SkBits2Float(0x43c2f333), SkBits2Float(0x42c8a5ad), SkBits2Float(0x43c34ccd), SkBits2Float(0x42c8a7ef));  // 389.2f, 100.311f, 389.9f, 100.324f, 390.6f, 100.328f
path.cubicTo(SkBits2Float(0x43c3a666), SkBits2Float(0x42c8aa32), SkBits2Float(0x43c40000), SkBits2Float(0x42c8a9a2), SkBits2Float(0x43c4599a), SkBits2Float(0x42c8a9a2));  // 391.3f, 100.332f, 392, 100.331f, 392.7f, 100.331f
path.cubicTo(SkBits2Float(0x43c4b333), SkBits2Float(0x42c8a9a2), SkBits2Float(0x43c50ccd), SkBits2Float(0x42c8a8c9), SkBits2Float(0x43c56666), SkBits2Float(0x42c8a7ef));  // 393.4f, 100.331f, 394.1f, 100.33f, 394.8f, 100.328f
path.cubicTo(SkBits2Float(0x43c5c000), SkBits2Float(0x42c8a716), SkBits2Float(0x43c6199a), SkBits2Float(0x42c8a5ad), SkBits2Float(0x43c67333), SkBits2Float(0x42c8a48b));  // 395.5f, 100.326f, 396.2f, 100.324f, 396.9f, 100.321f
path.cubicTo(SkBits2Float(0x43c6cccd), SkBits2Float(0x42c8a36a), SkBits2Float(0x43c72666), SkBits2Float(0x42c8a291), SkBits2Float(0x43c78000), SkBits2Float(0x42c8a127));  // 397.6f, 100.319f, 398.3f, 100.318f, 399, 100.315f
path.cubicTo(SkBits2Float(0x43c7d99a), SkBits2Float(0x42c89fbd), SkBits2Float(0x43c83333), SkBits2Float(0x42c89dc3), SkBits2Float(0x43c88ccd), SkBits2Float(0x42c89c11));  // 399.7f, 100.312f, 400.4f, 100.308f, 401.1f, 100.305f
path.cubicTo(SkBits2Float(0x43c8e666), SkBits2Float(0x42c89a5f), SkBits2Float(0x43c94000), SkBits2Float(0x42c898ac), SkBits2Float(0x43c9999a), SkBits2Float(0x42c896fa));  // 401.8f, 100.302f, 402.5f, 100.298f, 403.2f, 100.295f
path.cubicTo(SkBits2Float(0x43c9f333), SkBits2Float(0x42c89548), SkBits2Float(0x43ca4ccd), SkBits2Float(0x42c89305), SkBits2Float(0x43caa666), SkBits2Float(0x42c891e4));  // 403.9f, 100.292f, 404.6f, 100.287f, 405.3f, 100.285f
path.cubicTo(SkBits2Float(0x43cb0000), SkBits2Float(0x42c890c3), SkBits2Float(0x43cb599a), SkBits2Float(0x42c88ec8), SkBits2Float(0x43cbb333), SkBits2Float(0x42c89032));  // 406, 100.283f, 406.7f, 100.279f, 407.4f, 100.282f
path.cubicTo(SkBits2Float(0x43cc0ccd), SkBits2Float(0x42c8919c), SkBits2Float(0x43cc6666), SkBits2Float(0x42c89864), SkBits2Float(0x43ccc000), SkBits2Float(0x42c89a5f));  // 408.1f, 100.284f, 408.8f, 100.298f, 409.5f, 100.302f
path.cubicTo(SkBits2Float(0x43cd199a), SkBits2Float(0x42c89c59), SkBits2Float(0x43cd7333), SkBits2Float(0x42c89dff), SkBits2Float(0x43cdcccd), SkBits2Float(0x42c89c11));  // 410.2f, 100.305f, 410.9f, 100.309f, 411.6f, 100.305f
path.cubicTo(SkBits2Float(0x43ce2666), SkBits2Float(0x42c89a22), SkBits2Float(0x43ce8000), SkBits2Float(0x42c8919a), SkBits2Float(0x43ced99a), SkBits2Float(0x42c88ec6));  // 412.3f, 100.301f, 413, 100.284f, 413.7f, 100.279f
path.cubicTo(SkBits2Float(0x43cf3333), SkBits2Float(0x42c88bf3), SkBits2Float(0x43cf8ccd), SkBits2Float(0x42c88b70), SkBits2Float(0x43cfe666), SkBits2Float(0x42c88b1b));  // 414.4f, 100.273f, 415.1f, 100.272f, 415.8f, 100.272f
path.cubicTo(SkBits2Float(0x43d04000), SkBits2Float(0x42c88ac7), SkBits2Float(0x43d0999a), SkBits2Float(0x42c88bac), SkBits2Float(0x43d0f333), SkBits2Float(0x42c88cce));  // 416.5f, 100.271f, 417.2f, 100.273f, 417.9f, 100.275f
path.cubicTo(SkBits2Float(0x43d14ccd), SkBits2Float(0x42c88def), SkBits2Float(0x43d1a666), SkBits2Float(0x42c89032), SkBits2Float(0x43d20000), SkBits2Float(0x42c891e4));  // 418.6f, 100.277f, 419.3f, 100.282f, 420, 100.285f
path.cubicTo(SkBits2Float(0x43d2599a), SkBits2Float(0x42c89396), SkBits2Float(0x43d2b333), SkBits2Float(0x42c89621), SkBits2Float(0x43d30ccd), SkBits2Float(0x42c896fa));  // 420.7f, 100.288f, 421.4f, 100.293f, 422.1f, 100.295f
path.cubicTo(SkBits2Float(0x43d36666), SkBits2Float(0x42c897d3), SkBits2Float(0x43d3c000), SkBits2Float(0x42c89810), SkBits2Float(0x43d4199a), SkBits2Float(0x42c896fa));  // 422.8f, 100.297f, 423.5f, 100.297f, 424.2f, 100.295f
path.cubicTo(SkBits2Float(0x43d47333), SkBits2Float(0x42c895e5), SkBits2Float(0x43d4cccd), SkBits2Float(0x42c88d99), SkBits2Float(0x43d52666), SkBits2Float(0x42c89078));  // 424.9f, 100.293f, 425.6f, 100.277f, 426.3f, 100.282f
path.cubicTo(SkBits2Float(0x43d58000), SkBits2Float(0x42c89358), SkBits2Float(0x43d5d99a), SkBits2Float(0x42c88f0f), SkBits2Float(0x43d63333), SkBits2Float(0x42c8a836));  // 427, 100.288f, 427.7f, 100.279f, 428.4f, 100.329f
path.cubicTo(SkBits2Float(0x43d68ccd), SkBits2Float(0x42c8c15e), SkBits2Float(0x43d6e666), SkBits2Float(0x42c8ff2a), SkBits2Float(0x43d74000), SkBits2Float(0x42c92765));  // 429.1f, 100.378f, 429.8f, 100.498f, 430.5f, 100.577f
path.cubicTo(SkBits2Float(0x43d7999a), SkBits2Float(0x42c94fa0), SkBits2Float(0x43d7f333), SkBits2Float(0x42c98691), SkBits2Float(0x43d84ccd), SkBits2Float(0x42c9999a));  // 431.2f, 100.656f, 431.9f, 100.763f, 432.6f, 100.8f
path.cubicTo(SkBits2Float(0x43d8a666), SkBits2Float(0x42c9aca2), SkBits2Float(0x43d90000), SkBits2Float(0x42c9999a), SkBits2Float(0x43d9599a), SkBits2Float(0x42c9999a));  // 433.3f, 100.837f, 434, 100.8f, 434.7f, 100.8f
path.lineTo(SkBits2Float(0x43d9599a), SkBits2Float(0x42c9999a));  // 434.7f, 100.8f
path.lineTo(SkBits2Float(0x43ab2666), SkBits2Float(0x42c9999a));  // 342.3f, 100.8f
path.close();
path.moveTo(SkBits2Float(0x43dfa666), SkBits2Float(0x42c9999a));  // 447.3f, 100.8f
path.lineTo(SkBits2Float(0x43dfa666), SkBits2Float(0x42c9999a));  // 447.3f, 100.8f
path.cubicTo(SkBits2Float(0x43e00000), SkBits2Float(0x42c99877), SkBits2Float(0x43e0599a), SkBits2Float(0x42c99312), SkBits2Float(0x43e0b333), SkBits2Float(0x42c992c8));  // 448, 100.798f, 448.7f, 100.787f, 449.4f, 100.787f
path.cubicTo(SkBits2Float(0x43e10ccd), SkBits2Float(0x42c9927e), SkBits2Float(0x43e16666), SkBits2Float(0x42c996bb), SkBits2Float(0x43e1c000), SkBits2Float(0x42c997de));  // 450.1f, 100.786f, 450.8f, 100.794f, 451.5f, 100.797f
path.cubicTo(SkBits2Float(0x43e2199a), SkBits2Float(0x42c99901), SkBits2Float(0x43e27333), SkBits2Float(0x42c9a0aa), SkBits2Float(0x43e2cccd), SkBits2Float(0x42c9999a));  // 452.2f, 100.799f, 452.9f, 100.814f, 453.6f, 100.8f
path.cubicTo(SkBits2Float(0x43e32666), SkBits2Float(0x42c99289), SkBits2Float(0x43e38000), SkBits2Float(0x42c97cb2), SkBits2Float(0x43e3d99a), SkBits2Float(0x42c96d79));  // 454.3f, 100.786f, 455, 100.744f, 455.7f, 100.714f
path.cubicTo(SkBits2Float(0x43e43333), SkBits2Float(0x42c95e40), SkBits2Float(0x43e48ccd), SkBits2Float(0x42c943a3), SkBits2Float(0x43e4e666), SkBits2Float(0x42c93e44));  // 456.4f, 100.684f, 457.1f, 100.632f, 457.8f, 100.622f
path.cubicTo(SkBits2Float(0x43e54000), SkBits2Float(0x42c938e6), SkBits2Float(0x43e5999a), SkBits2Float(0x42c949f4), SkBits2Float(0x43e5f333), SkBits2Float(0x42c94d41));  // 458.5f, 100.611f, 459.2f, 100.644f, 459.9f, 100.651f
path.cubicTo(SkBits2Float(0x43e64ccd), SkBits2Float(0x42c9508e), SkBits2Float(0x43e6a666), SkBits2Float(0x42c94fce), SkBits2Float(0x43e70000), SkBits2Float(0x42c95211));  // 460.6f, 100.657f, 461.3f, 100.656f, 462, 100.66f
path.cubicTo(SkBits2Float(0x43e7599a), SkBits2Float(0x42c95454), SkBits2Float(0x43e7b333), SkBits2Float(0x42c9595c), SkBits2Float(0x43e80ccd), SkBits2Float(0x42c95ad2));  // 462.7f, 100.665f, 463.4f, 100.675f, 464.1f, 100.677f
path.cubicTo(SkBits2Float(0x43e86666), SkBits2Float(0x42c95c47), SkBits2Float(0x43e8c000), SkBits2Float(0x42c959f9), SkBits2Float(0x43e9199a), SkBits2Float(0x42c95ad2));  // 464.8f, 100.68f, 465.5f, 100.676f, 466.2f, 100.677f
path.cubicTo(SkBits2Float(0x43e97333), SkBits2Float(0x42c95bab), SkBits2Float(0x43e9cccd), SkBits2Float(0x42c95d20), SkBits2Float(0x43ea2666), SkBits2Float(0x42c95fe8));  // 466.9f, 100.679f, 467.6f, 100.682f, 468.3f, 100.687f
path.cubicTo(SkBits2Float(0x43ea8000), SkBits2Float(0x42c962b0), SkBits2Float(0x43ead99a), SkBits2Float(0x42c96743), SkBits2Float(0x43eb3333), SkBits2Float(0x42c96b80));  // 469, 100.693f, 469.7f, 100.702f, 470.4f, 100.71f
path.cubicTo(SkBits2Float(0x43eb8ccd), SkBits2Float(0x42c96fbe), SkBits2Float(0x43ebe666), SkBits2Float(0x42c9754c), SkBits2Float(0x43ec4000), SkBits2Float(0x42c97958));  // 471.1f, 100.718f, 471.8f, 100.729f, 472.5f, 100.737f
path.cubicTo(SkBits2Float(0x43ec999a), SkBits2Float(0x42c97d65), SkBits2Float(0x43ecf333), SkBits2Float(0x42c97ef2), SkBits2Float(0x43ed4ccd), SkBits2Float(0x42c983cb));  // 473.2f, 100.745f, 473.9f, 100.748f, 474.6f, 100.757f
path.cubicTo(SkBits2Float(0x43eda666), SkBits2Float(0x42c988a5), SkBits2Float(0x43ee0000), SkBits2Float(0x42c992d0), SkBits2Float(0x43ee599a), SkBits2Float(0x42c99673));  // 475.3f, 100.767f, 476, 100.787f, 476.7f, 100.794f
path.cubicTo(SkBits2Float(0x43eeb333), SkBits2Float(0x42c99a15), SkBits2Float(0x43ef0ccd), SkBits2Float(0x42c99bdc), SkBits2Float(0x43ef6666), SkBits2Float(0x42c9999a));  // 477.4f, 100.801f, 478.1f, 100.804f, 478.8f, 100.8f
path.cubicTo(SkBits2Float(0x43efc000), SkBits2Float(0x42c99757), SkBits2Float(0x43f0199a), SkBits2Float(0x42c99426), SkBits2Float(0x43f07333), SkBits2Float(0x42c988e2));  // 479.5f, 100.796f, 480.2f, 100.789f, 480.9f, 100.767f
path.cubicTo(SkBits2Float(0x43f0cccd), SkBits2Float(0x42c97d9e), SkBits2Float(0x43f12666), SkBits2Float(0x42c96356), SkBits2Float(0x43f18000), SkBits2Float(0x42c95602));  // 481.6f, 100.745f, 482.3f, 100.694f, 483, 100.668f
path.cubicTo(SkBits2Float(0x43f1d99a), SkBits2Float(0x42c948ae), SkBits2Float(0x43f23333), SkBits2Float(0x42c93ee3), SkBits2Float(0x43f28ccd), SkBits2Float(0x42c938e7));  // 483.7f, 100.642f, 484.4f, 100.623f, 485.1f, 100.611f
path.cubicTo(SkBits2Float(0x43f2e666), SkBits2Float(0x42c932ec), SkBits2Float(0x43f34000), SkBits2Float(0x42c93741), SkBits2Float(0x43f3999a), SkBits2Float(0x42c9321f));  // 485.8f, 100.599f, 486.5f, 100.608f, 487.2f, 100.598f
path.cubicTo(SkBits2Float(0x43f3f333), SkBits2Float(0x42c92cfd), SkBits2Float(0x43f44ccd), SkBits2Float(0x42c922f5), SkBits2Float(0x43f4a666), SkBits2Float(0x42c91a1b));  // 487.9f, 100.588f, 488.6f, 100.568f, 489.3f, 100.551f
path.cubicTo(SkBits2Float(0x43f50000), SkBits2Float(0x42c91140), SkBits2Float(0x43f5599a), SkBits2Float(0x42c904ad), SkBits2Float(0x43f5b333), SkBits2Float(0x42c8fd00));  // 490, 100.534f, 490.7f, 100.509f, 491.4f, 100.494f
path.cubicTo(SkBits2Float(0x43f60ccd), SkBits2Float(0x42c8f553), SkBits2Float(0x43f66666), SkBits2Float(0x42c8ef7b), SkBits2Float(0x43f6c000), SkBits2Float(0x42c8ec0b));  // 492.1f, 100.479f, 492.8f, 100.468f, 493.5f, 100.461f
path.cubicTo(SkBits2Float(0x43f7199a), SkBits2Float(0x42c8e89b), SkBits2Float(0x43f77333), SkBits2Float(0x42c8e981), SkBits2Float(0x43f7cccd), SkBits2Float(0x42c8e860));  // 494.2f, 100.454f, 494.9f, 100.456f, 495.6f, 100.454f
path.cubicTo(SkBits2Float(0x43f82666), SkBits2Float(0x42c8e73f), SkBits2Float(0x43f88000), SkBits2Float(0x42c8e658), SkBits2Float(0x43f8d99a), SkBits2Float(0x42c8e542));  // 496.3f, 100.452f, 497, 100.45f, 497.7f, 100.448f
path.cubicTo(SkBits2Float(0x43f93333), SkBits2Float(0x42c8e42d), SkBits2Float(0x43f98ccd), SkBits2Float(0x42c8e348), SkBits2Float(0x43f9e666), SkBits2Float(0x42c8e1de));  // 498.4f, 100.446f, 499.1f, 100.444f, 499.8f, 100.441f
path.cubicTo(SkBits2Float(0x43fa4000), SkBits2Float(0x42c8e074), SkBits2Float(0x43fa999a), SkBits2Float(0x42c8df53), SkBits2Float(0x43faf333), SkBits2Float(0x42c8dcc8));  // 500.5f, 100.438f, 501.2f, 100.436f, 501.9f, 100.431f
path.cubicTo(SkBits2Float(0x43fb4ccd), SkBits2Float(0x42c8da3d), SkBits2Float(0x43fba666), SkBits2Float(0x42c8d5c3), SkBits2Float(0x43fc0000), SkBits2Float(0x42c8d29b));  // 502.6f, 100.426f, 503.3f, 100.418f, 504, 100.411f
path.cubicTo(SkBits2Float(0x43fc599a), SkBits2Float(0x42c8cf73), SkBits2Float(0x43fcb333), SkBits2Float(0x42c8ccf6), SkBits2Float(0x43fd0ccd), SkBits2Float(0x42c8c9da));  // 504.7f, 100.405f, 505.4f, 100.4f, 506.1f, 100.394f
path.cubicTo(SkBits2Float(0x43fd6666), SkBits2Float(0x42c8c6be), SkBits2Float(0x43fdc000), SkBits2Float(0x42c8c310), SkBits2Float(0x43fe199a), SkBits2Float(0x42c8bff4));  // 506.8f, 100.388f, 507.5f, 100.381f, 508.2f, 100.375f
path.cubicTo(SkBits2Float(0x43fe7333), SkBits2Float(0x42c8bcd8), SkBits2Float(0x43fecccd), SkBits2Float(0x42c8b8f0), SkBits2Float(0x43ff2666), SkBits2Float(0x42c8b733));  // 508.9f, 100.369f, 509.6f, 100.361f, 510.3f, 100.358f
path.cubicTo(SkBits2Float(0x43ff8000), SkBits2Float(0x42c8b575), SkBits2Float(0x43ffd99a), SkBits2Float(0x42c8b45f), SkBits2Float(0x4400199a), SkBits2Float(0x42c8b580));  // 511, 100.354f, 511.7f, 100.352f, 512.4f, 100.354f
path.cubicTo(SkBits2Float(0x44004666), SkBits2Float(0x42c8b6a2), SkBits2Float(0x44007333), SkBits2Float(0x42c8bb28), SkBits2Float(0x4400a000), SkBits2Float(0x42c8bdfb));  // 513.1f, 100.357f, 513.8f, 100.366f, 514.5f, 100.371f
path.cubicTo(SkBits2Float(0x4400cccd), SkBits2Float(0x42c8c0cf), SkBits2Float(0x4400f99a), SkBits2Float(0x42c8c31d), SkBits2Float(0x44012666), SkBits2Float(0x42c8c676));  // 515.2f, 100.377f, 515.9f, 100.381f, 516.6f, 100.388f
path.cubicTo(SkBits2Float(0x44015333), SkBits2Float(0x42c8c9ce), SkBits2Float(0x44018000), SkBits2Float(0x42c8caa9), SkBits2Float(0x4401accd), SkBits2Float(0x42c8d20e));  // 517.3f, 100.394f, 518, 100.396f, 518.7f, 100.41f
path.cubicTo(SkBits2Float(0x4401d99a), SkBits2Float(0x42c8d973), SkBits2Float(0x44020666), SkBits2Float(0x42c8e446), SkBits2Float(0x44023333), SkBits2Float(0x42c8f2d3));  // 519.4f, 100.425f, 520.1f, 100.446f, 520.8f, 100.474f
path.cubicTo(SkBits2Float(0x44026000), SkBits2Float(0x42c90161), SkBits2Float(0x44028ccd), SkBits2Float(0x42c917c0), SkBits2Float(0x4402b99a), SkBits2Float(0x42c9295e));  // 521.5f, 100.503f, 522.2f, 100.546f, 522.9f, 100.581f
path.cubicTo(SkBits2Float(0x4402e666), SkBits2Float(0x42c93afb), SkBits2Float(0x44031333), SkBits2Float(0x42c94c14), SkBits2Float(0x44034000), SkBits2Float(0x42c95c84));  // 523.6f, 100.615f, 524.3f, 100.649f, 525, 100.681f
path.cubicTo(SkBits2Float(0x44036ccd), SkBits2Float(0x42c96cf4), SkBits2Float(0x4403999a), SkBits2Float(0x42c981d1), SkBits2Float(0x4403c666), SkBits2Float(0x42c98bff));  // 525.7f, 100.713f, 526.4f, 100.754f, 527.1f, 100.773f
path.cubicTo(SkBits2Float(0x4403f333), SkBits2Float(0x42c9962e), SkBits2Float(0x44042000), SkBits2Float(0x42c99755), SkBits2Float(0x44044ccd), SkBits2Float(0x42c9999a));  // 527.8f, 100.793f, 528.5f, 100.796f, 529.2f, 100.8f
path.lineTo(SkBits2Float(0x44044ccd), SkBits2Float(0x42c9999a));  // 529.2f, 100.8f
path.lineTo(SkBits2Float(0x43dfa666), SkBits2Float(0x42c9999a));  // 447.3f, 100.8f
path.close();
    return path;
}

static SkPath path6() {
    SkPath path;
path.moveTo(SkBits2Float(0x42b06666), SkBits2Float(0x42c9999a));  // 88.2f, 100.8f
path.lineTo(SkBits2Float(0x42b06666), SkBits2Float(0x42c9999a));  // 88.2f, 100.8f
path.cubicTo(SkBits2Float(0x42b1cccd), SkBits2Float(0x42c9999a), SkBits2Float(0x42b33333), SkBits2Float(0x42c97f2d), SkBits2Float(0x42b4999a), SkBits2Float(0x42c9999a));  // 88.9f, 100.8f, 89.6f, 100.748f, 90.3f, 100.8f
path.cubicTo(SkBits2Float(0x42b60000), SkBits2Float(0x42c9b408), SkBits2Float(0x42b76666), SkBits2Float(0x42ca1e13), SkBits2Float(0x42b8cccd), SkBits2Float(0x42ca382d));  // 91, 100.852f, 91.7f, 101.059f, 92.4f, 101.11f
path.cubicTo(SkBits2Float(0x42ba3333), SkBits2Float(0x42ca5246), SkBits2Float(0x42bb999a), SkBits2Float(0x42ca3634), SkBits2Float(0x42bd0000), SkBits2Float(0x42ca3634));  // 93.1f, 101.161f, 93.8f, 101.106f, 94.5f, 101.106f
path.cubicTo(SkBits2Float(0x42be6666), SkBits2Float(0x42ca3634), SkBits2Float(0x42bfcccd), SkBits2Float(0x42ca3869), SkBits2Float(0x42c13333), SkBits2Float(0x42ca382d));  // 95.2f, 101.106f, 95.9f, 101.11f, 96.6f, 101.11f
path.cubicTo(SkBits2Float(0x42c2999a), SkBits2Float(0x42ca37f0), SkBits2Float(0x42c40000), SkBits2Float(0x42ca35ea), SkBits2Float(0x42c56666), SkBits2Float(0x42ca34c8));  // 97.3f, 101.109f, 98, 101.105f, 98.7f, 101.103f
path.cubicTo(SkBits2Float(0x42c6cccd), SkBits2Float(0x42ca33a7), SkBits2Float(0x42c83333), SkBits2Float(0x42ca311c), SkBits2Float(0x42c9999a), SkBits2Float(0x42ca3164));  // 99.4f, 101.101f, 100.1f, 101.096f, 100.8f, 101.096f
path.cubicTo(SkBits2Float(0x42cb0000), SkBits2Float(0x42ca31ad), SkBits2Float(0x42cc6666), SkBits2Float(0x42ca33fb), SkBits2Float(0x42cdcccd), SkBits2Float(0x42ca367b));  // 101.5f, 101.097f, 102.2f, 101.102f, 102.9f, 101.106f
path.cubicTo(SkBits2Float(0x42cf3333), SkBits2Float(0x42ca38fa), SkBits2Float(0x42d0999a), SkBits2Float(0x42ca3fd0), SkBits2Float(0x42d20000), SkBits2Float(0x42ca4061));  // 103.6f, 101.111f, 104.3f, 101.125f, 105, 101.126f
path.cubicTo(SkBits2Float(0x42d36666), SkBits2Float(0x42ca40f1), SkBits2Float(0x42d4cccd), SkBits2Float(0x42ca3f32), SkBits2Float(0x42d63333), SkBits2Float(0x42ca39df));  // 105.7f, 101.127f, 106.4f, 101.123f, 107.1f, 101.113f
path.cubicTo(SkBits2Float(0x42d7999a), SkBits2Float(0x42ca348c), SkBits2Float(0x42d90000), SkBits2Float(0x42ca2585), SkBits2Float(0x42da6666), SkBits2Float(0x42ca206f));  // 107.8f, 101.103f, 108.5f, 101.073f, 109.2f, 101.063f
path.cubicTo(SkBits2Float(0x42dbcccd), SkBits2Float(0x42ca1b59), SkBits2Float(0x42dd3333), SkBits2Float(0x42ca1a43), SkBits2Float(0x42de999a), SkBits2Float(0x42ca1b59));  // 109.9f, 101.053f, 110.6f, 101.051f, 111.3f, 101.053f
path.cubicTo(SkBits2Float(0x42e00000), SkBits2Float(0x42ca1c6e), SkBits2Float(0x42e16666), SkBits2Float(0x42ca2872), SkBits2Float(0x42e2cccd), SkBits2Float(0x42ca26f1));  // 112, 101.056f, 112.7f, 101.079f, 113.4f, 101.076f
path.cubicTo(SkBits2Float(0x42e43333), SkBits2Float(0x42ca2570), SkBits2Float(0x42e5999a), SkBits2Float(0x42ca1f14), SkBits2Float(0x42e70000), SkBits2Float(0x42ca1251));  // 114.1f, 101.073f, 114.8f, 101.061f, 115.5f, 101.036f
path.cubicTo(SkBits2Float(0x42e86666), SkBits2Float(0x42ca058d), SkBits2Float(0x42e9cccd), SkBits2Float(0x42c9ec89), SkBits2Float(0x42eb3333), SkBits2Float(0x42c9da5b));  // 116.2f, 101.011f, 116.9f, 100.962f, 117.6f, 100.926f
path.cubicTo(SkBits2Float(0x42ec999a), SkBits2Float(0x42c9c82d), SkBits2Float(0x42ee0000), SkBits2Float(0x42c9b007), SkBits2Float(0x42ef6666), SkBits2Float(0x42c9a53c));  // 118.3f, 100.891f, 119, 100.844f, 119.7f, 100.823f
path.cubicTo(SkBits2Float(0x42f0cccd), SkBits2Float(0x42c99a71), SkBits2Float(0x42f23333), SkBits2Float(0x42c99b8b), SkBits2Float(0x42f3999a), SkBits2Float(0x42c9999a));  // 120.4f, 100.802f, 121.1f, 100.804f, 121.8f, 100.8f
path.lineTo(SkBits2Float(0x42f3999a), SkBits2Float(0x42c9999a));  // 121.8f, 100.8f
path.lineTo(SkBits2Float(0x42b06666), SkBits2Float(0x42c9999a));  // 88.2f, 100.8f
path.close();
path.moveTo(SkBits2Float(0x4300199a), SkBits2Float(0x42c9999a));  // 128.1f, 100.8f
path.lineTo(SkBits2Float(0x4300199a), SkBits2Float(0x42c99b0f));  // 128.1f, 100.803f
path.cubicTo(SkBits2Float(0x4300cccd), SkBits2Float(0x42c99bde), SkBits2Float(0x43018000), SkBits2Float(0x42c99eb2), SkBits2Float(0x43023333), SkBits2Float(0x42c99e73));  // 128.8f, 100.804f, 129.5f, 100.81f, 130.2f, 100.809f
path.cubicTo(SkBits2Float(0x4302e666), SkBits2Float(0x42c99e35), SkBits2Float(0x4303999a), SkBits2Float(0x42c99a69), SkBits2Float(0x43044ccd), SkBits2Float(0x42c9999a));  // 130.9f, 100.809f, 131.6f, 100.802f, 132.3f, 100.8f
path.lineTo(SkBits2Float(0x43044ccd), SkBits2Float(0x42c9999a));  // 132.3f, 100.8f
path.lineTo(SkBits2Float(0x4300199a), SkBits2Float(0x42c9999a));  // 128.1f, 100.8f
path.close();
path.moveTo(SkBits2Float(0x431b6666), SkBits2Float(0x42c9999a));  // 155.4f, 100.8f
path.lineTo(SkBits2Float(0x431b6666), SkBits2Float(0x42c9999a));  // 155.4f, 100.8f
path.cubicTo(SkBits2Float(0x431c199a), SkBits2Float(0x42c9999a), SkBits2Float(0x431ccccd), SkBits2Float(0x42c99567), SkBits2Float(0x431d8000), SkBits2Float(0x42c9999a));  // 156.1f, 100.8f, 156.8f, 100.792f, 157.5f, 100.8f
path.cubicTo(SkBits2Float(0x431e3333), SkBits2Float(0x42c99dcd), SkBits2Float(0x431ee666), SkBits2Float(0x42c9a613), SkBits2Float(0x431f999a), SkBits2Float(0x42c9b2cd));  // 158.2f, 100.808f, 158.9f, 100.824f, 159.6f, 100.849f
path.cubicTo(SkBits2Float(0x43204ccd), SkBits2Float(0x42c9bf86), SkBits2Float(0x43210000), SkBits2Float(0x42c9d893), SkBits2Float(0x4321b333), SkBits2Float(0x42c9e5f3));  // 160.3f, 100.874f, 161, 100.923f, 161.7f, 100.949f
path.cubicTo(SkBits2Float(0x43226666), SkBits2Float(0x42c9f353), SkBits2Float(0x4323199a), SkBits2Float(0x42c9fd4f), SkBits2Float(0x4323cccd), SkBits2Float(0x42ca030e));  // 162.4f, 100.975f, 163.1f, 100.995f, 163.8f, 101.006f
path.cubicTo(SkBits2Float(0x43248000), SkBits2Float(0x42ca08cc), SkBits2Float(0x43253333), SkBits2Float(0x42ca07ce), SkBits2Float(0x4325e666), SkBits2Float(0x42ca086b));  // 164.5f, 101.017f, 165.2f, 101.015f, 165.9f, 101.016f
path.cubicTo(SkBits2Float(0x4326999a), SkBits2Float(0x42ca0907), SkBits2Float(0x43274ccd), SkBits2Float(0x42ca0bdb), SkBits2Float(0x43280000), SkBits2Float(0x42ca06b9));  // 166.6f, 101.018f, 167.3f, 101.023f, 168, 101.013f
path.cubicTo(SkBits2Float(0x4328b333), SkBits2Float(0x42ca0196), SkBits2Float(0x43296666), SkBits2Float(0x42c9f5dd), SkBits2Float(0x432a199a), SkBits2Float(0x42c9e99e));  // 168.7f, 101.003f, 169.4f, 100.98f, 170.1f, 100.956f
path.cubicTo(SkBits2Float(0x432acccd), SkBits2Float(0x42c9dd5f), SkBits2Float(0x432b8000), SkBits2Float(0x42c9ca4c), SkBits2Float(0x432c3333), SkBits2Float(0x42c9bd40));  // 170.8f, 100.932f, 171.5f, 100.895f, 172.2f, 100.87f
path.cubicTo(SkBits2Float(0x432ce666), SkBits2Float(0x42c9b034), SkBits2Float(0x432d999a), SkBits2Float(0x42c9a147), SkBits2Float(0x432e4ccd), SkBits2Float(0x42c99b56));  // 172.9f, 100.844f, 173.6f, 100.815f, 174.3f, 100.803f
path.cubicTo(SkBits2Float(0x432f0000), SkBits2Float(0x42c99565), SkBits2Float(0x432fb333), SkBits2Float(0x42c999e4), SkBits2Float(0x43306666), SkBits2Float(0x42c9999a));  // 175, 100.792f, 175.7f, 100.801f, 176.4f, 100.8f
path.lineTo(SkBits2Float(0x43306666), SkBits2Float(0x42c9999a));  // 176.4f, 100.8f
path.lineTo(SkBits2Float(0x431b6666), SkBits2Float(0x42c9999a));  // 155.4f, 100.8f
path.close();
path.moveTo(SkBits2Float(0x43478000), SkBits2Float(0x42c9999a));  // 199.5f, 100.8f
path.lineTo(SkBits2Float(0x43478000), SkBits2Float(0x42c9999a));  // 199.5f, 100.8f
path.cubicTo(SkBits2Float(0x43483333), SkBits2Float(0x42c9999a), SkBits2Float(0x4348e666), SkBits2Float(0x42c99640), SkBits2Float(0x4349999a), SkBits2Float(0x42c9999a));  // 200.2f, 100.8f, 200.9f, 100.793f, 201.6f, 100.8f
path.cubicTo(SkBits2Float(0x434a4ccd), SkBits2Float(0x42c99cf4), SkBits2Float(0x434b0000), SkBits2Float(0x42c9a741), SkBits2Float(0x434bb333), SkBits2Float(0x42c9adb7));  // 202.3f, 100.807f, 203, 100.827f, 203.7f, 100.839f
path.cubicTo(SkBits2Float(0x434c6666), SkBits2Float(0x42c9b42c), SkBits2Float(0x434d199a), SkBits2Float(0x42c9bc15), SkBits2Float(0x434dcccd), SkBits2Float(0x42c9c05e));  // 204.4f, 100.852f, 205.1f, 100.867f, 205.8f, 100.876f
path.cubicTo(SkBits2Float(0x434e8000), SkBits2Float(0x42c9c4a7), SkBits2Float(0x434f3333), SkBits2Float(0x42c9c572), SkBits2Float(0x434fe666), SkBits2Float(0x42c9c76d));  // 206.5f, 100.884f, 207.2f, 100.886f, 207.9f, 100.89f
path.cubicTo(SkBits2Float(0x4350999a), SkBits2Float(0x42c9c967), SkBits2Float(0x43514ccd), SkBits2Float(0x42c9cb6f), SkBits2Float(0x43520000), SkBits2Float(0x42c9cc3d));  // 208.6f, 100.893f, 209.3f, 100.897f, 210, 100.899f
path.cubicTo(SkBits2Float(0x4352b333), SkBits2Float(0x42c9cd0a), SkBits2Float(0x43536666), SkBits2Float(0x42c9cc85), SkBits2Float(0x4354199a), SkBits2Float(0x42c9cc3d));  // 210.7f, 100.9f, 211.4f, 100.899f, 212.1f, 100.899f
path.cubicTo(SkBits2Float(0x4354cccd), SkBits2Float(0x42c9cbf4), SkBits2Float(0x43558000), SkBits2Float(0x42c9c7f4), SkBits2Float(0x43563333), SkBits2Float(0x42c9ca8b));  // 212.8f, 100.898f, 213.5f, 100.891f, 214.2f, 100.896f
path.cubicTo(SkBits2Float(0x4356e666), SkBits2Float(0x42c9cd22), SkBits2Float(0x4357999a), SkBits2Float(0x42c9d0e6), SkBits2Float(0x43584ccd), SkBits2Float(0x42c9dbc6));  // 214.9f, 100.901f, 215.6f, 100.908f, 216.3f, 100.929f
path.cubicTo(SkBits2Float(0x43590000), SkBits2Float(0x42c9e6a7), SkBits2Float(0x4359b333), SkBits2Float(0x42c9fd4d), SkBits2Float(0x435a6666), SkBits2Float(0x42ca0bcf));  // 217, 100.95f, 217.7f, 100.995f, 218.4f, 101.023f
path.cubicTo(SkBits2Float(0x435b199a), SkBits2Float(0x42ca1a50), SkBits2Float(0x435bcccd), SkBits2Float(0x42ca29b9), SkBits2Float(0x435c8000), SkBits2Float(0x42ca32d0));  // 219.1f, 101.051f, 219.8f, 101.081f, 220.5f, 101.099f
path.cubicTo(SkBits2Float(0x435d3333), SkBits2Float(0x42ca3be7), SkBits2Float(0x435de666), SkBits2Float(0x42ca3af4), SkBits2Float(0x435e999a), SkBits2Float(0x42ca4259));  // 221.2f, 101.117f, 221.9f, 101.115f, 222.6f, 101.13f
path.cubicTo(SkBits2Float(0x435f4ccd), SkBits2Float(0x42ca49be), SkBits2Float(0x43600000), SkBits2Float(0x42ca541c), SkBits2Float(0x4360b333), SkBits2Float(0x42ca5f2d));  // 223.3f, 101.144f, 224, 101.164f, 224.7f, 101.186f
path.cubicTo(SkBits2Float(0x43616666), SkBits2Float(0x42ca6a3f), SkBits2Float(0x4362199a), SkBits2Float(0x42ca7a05), SkBits2Float(0x4362cccd), SkBits2Float(0x42ca84c3));  // 225.4f, 101.208f, 226.1f, 101.238f, 226.8f, 101.259f
path.cubicTo(SkBits2Float(0x43638000), SkBits2Float(0x42ca8f80), SkBits2Float(0x43643333), SkBits2Float(0x42ca9a0e), SkBits2Float(0x4364e666), SkBits2Float(0x42ca9f9e));  // 227.5f, 101.28f, 228.2f, 101.301f, 228.9f, 101.312f
path.cubicTo(SkBits2Float(0x4365999a), SkBits2Float(0x42caa52d), SkBits2Float(0x43664ccd), SkBits2Float(0x42caa78a), SkBits2Float(0x43670000), SkBits2Float(0x42caa620));  // 229.6f, 101.323f, 230.3f, 101.327f, 231, 101.324f
path.cubicTo(SkBits2Float(0x4367b333), SkBits2Float(0x42caa4b6), SkBits2Float(0x43686666), SkBits2Float(0x42ca9cbf), SkBits2Float(0x4369199a), SkBits2Float(0x42ca9723));  // 231.7f, 101.322f, 232.4f, 101.306f, 233.1f, 101.295f
path.cubicTo(SkBits2Float(0x4369cccd), SkBits2Float(0x42ca9188), SkBits2Float(0x436a8000), SkBits2Float(0x42ca894a), SkBits2Float(0x436b3333), SkBits2Float(0x42ca847c));  // 233.8f, 101.284f, 234.5f, 101.268f, 235.2f, 101.259f
path.cubicTo(SkBits2Float(0x436be666), SkBits2Float(0x42ca7fae), SkBits2Float(0x436c999a), SkBits2Float(0x42ca7c01), SkBits2Float(0x436d4ccd), SkBits2Float(0x42ca7a4f));  // 235.9f, 101.249f, 236.6f, 101.242f, 237.3f, 101.239f
path.cubicTo(SkBits2Float(0x436e0000), SkBits2Float(0x42ca789d), SkBits2Float(0x436eb333), SkBits2Float(0x42ca7976), SkBits2Float(0x436f6666), SkBits2Float(0x42ca7a4f));  // 238, 101.236f, 238.7f, 101.237f, 239.4f, 101.239f
path.cubicTo(SkBits2Float(0x4370199a), SkBits2Float(0x42ca7b28), SkBits2Float(0x4370cccd), SkBits2Float(0x42ca7ed5), SkBits2Float(0x43718000), SkBits2Float(0x42ca7f66));  // 240.1f, 101.241f, 240.8f, 101.248f, 241.5f, 101.249f
path.cubicTo(SkBits2Float(0x43723333), SkBits2Float(0x42ca7ff6), SkBits2Float(0x4372e666), SkBits2Float(0x42ca7e44), SkBits2Float(0x4373999a), SkBits2Float(0x42ca7db4));  // 242.2f, 101.25f, 242.9f, 101.247f, 243.6f, 101.246f
path.cubicTo(SkBits2Float(0x43744ccd), SkBits2Float(0x42ca7d23), SkBits2Float(0x43750000), SkBits2Float(0x42ca7c4a), SkBits2Float(0x4375b333), SkBits2Float(0x42ca7c01));  // 244.3f, 101.244f, 245, 101.243f, 245.7f, 101.242f
path.cubicTo(SkBits2Float(0x43766666), SkBits2Float(0x42ca7bb9), SkBits2Float(0x4377199a), SkBits2Float(0x42ca7bb9), SkBits2Float(0x4377cccd), SkBits2Float(0x42ca7c01));  // 246.4f, 101.242f, 247.1f, 101.242f, 247.8f, 101.242f
path.cubicTo(SkBits2Float(0x43788000), SkBits2Float(0x42ca7c4a), SkBits2Float(0x43793333), SkBits2Float(0x42ca7d6b), SkBits2Float(0x4379e666), SkBits2Float(0x42ca7db4));  // 248.5f, 101.243f, 249.2f, 101.245f, 249.9f, 101.246f
path.cubicTo(SkBits2Float(0x437a999a), SkBits2Float(0x42ca7dfc), SkBits2Float(0x437b4ccd), SkBits2Float(0x42ca7dfc), SkBits2Float(0x437c0000), SkBits2Float(0x42ca7db4));  // 250.6f, 101.246f, 251.3f, 101.246f, 252, 101.246f
path.cubicTo(SkBits2Float(0x437cb333), SkBits2Float(0x42ca7d6b), SkBits2Float(0x437d6666), SkBits2Float(0x42ca7c4a), SkBits2Float(0x437e199a), SkBits2Float(0x42ca7c01));  // 252.7f, 101.245f, 253.4f, 101.243f, 254.1f, 101.242f
path.cubicTo(SkBits2Float(0x437ecccd), SkBits2Float(0x42ca7bb9), SkBits2Float(0x437f8000), SkBits2Float(0x42ca7bb9), SkBits2Float(0x4380199a), SkBits2Float(0x42ca7c01));  // 254.8f, 101.242f, 255.5f, 101.242f, 256.2f, 101.242f
path.cubicTo(SkBits2Float(0x43807333), SkBits2Float(0x42ca7c4a), SkBits2Float(0x4380cccd), SkBits2Float(0x42ca7d6b), SkBits2Float(0x43812666), SkBits2Float(0x42ca7db4));  // 256.9f, 101.243f, 257.6f, 101.245f, 258.3f, 101.246f
path.cubicTo(SkBits2Float(0x43818000), SkBits2Float(0x42ca7dfc), SkBits2Float(0x4381d99a), SkBits2Float(0x42ca7db4), SkBits2Float(0x43823333), SkBits2Float(0x42ca7db4));  // 259, 101.246f, 259.7f, 101.246f, 260.4f, 101.246f
path.cubicTo(SkBits2Float(0x43828ccd), SkBits2Float(0x42ca7db4), SkBits2Float(0x4382e666), SkBits2Float(0x42ca7db4), SkBits2Float(0x43834000), SkBits2Float(0x42ca7db4));  // 261.1f, 101.246f, 261.8f, 101.246f, 262.5f, 101.246f
path.cubicTo(SkBits2Float(0x4383999a), SkBits2Float(0x42ca7db4), SkBits2Float(0x4383f333), SkBits2Float(0x42ca7d6b), SkBits2Float(0x43844ccd), SkBits2Float(0x42ca7db4));  // 263.2f, 101.246f, 263.9f, 101.245f, 264.6f, 101.246f
path.cubicTo(SkBits2Float(0x4384a666), SkBits2Float(0x42ca7dfc), SkBits2Float(0x43850000), SkBits2Float(0x42ca7f1d), SkBits2Float(0x4385599a), SkBits2Float(0x42ca7f66));  // 265.3f, 101.246f, 266, 101.248f, 266.7f, 101.249f
path.cubicTo(SkBits2Float(0x4385b333), SkBits2Float(0x42ca7fae), SkBits2Float(0x43860ccd), SkBits2Float(0x42ca7fae), SkBits2Float(0x43866666), SkBits2Float(0x42ca7f66));  // 267.4f, 101.249f, 268.1f, 101.249f, 268.8f, 101.249f
path.cubicTo(SkBits2Float(0x4386c000), SkBits2Float(0x42ca7f1d), SkBits2Float(0x4387199a), SkBits2Float(0x42ca7e44), SkBits2Float(0x43877333), SkBits2Float(0x42ca7db4));  // 269.5f, 101.248f, 270.2f, 101.247f, 270.9f, 101.246f
path.cubicTo(SkBits2Float(0x4387cccd), SkBits2Float(0x42ca7d23), SkBits2Float(0x43882666), SkBits2Float(0x42ca7c4a), SkBits2Float(0x43888000), SkBits2Float(0x42ca7c01));  // 271.6f, 101.244f, 272.3f, 101.243f, 273, 101.242f
path.cubicTo(SkBits2Float(0x4388d99a), SkBits2Float(0x42ca7bb9), SkBits2Float(0x43893333), SkBits2Float(0x42ca7c92), SkBits2Float(0x43898ccd), SkBits2Float(0x42ca7c01));  // 273.7f, 101.242f, 274.4f, 101.243f, 275.1f, 101.242f
path.cubicTo(SkBits2Float(0x4389e666), SkBits2Float(0x42ca7b71), SkBits2Float(0x438a4000), SkBits2Float(0x42ca789d), SkBits2Float(0x438a999a), SkBits2Float(0x42ca789d));  // 275.8f, 101.241f, 276.5f, 101.236f, 277.2f, 101.236f
path.cubicTo(SkBits2Float(0x438af333), SkBits2Float(0x42ca789d), SkBits2Float(0x438b4ccd), SkBits2Float(0x42ca78da), SkBits2Float(0x438ba666), SkBits2Float(0x42ca7c01));  // 277.9f, 101.236f, 278.6f, 101.236f, 279.3f, 101.242f
path.cubicTo(SkBits2Float(0x438c0000), SkBits2Float(0x42ca7f29), SkBits2Float(0x438c599a), SkBits2Float(0x42ca8863), SkBits2Float(0x438cb333), SkBits2Float(0x42ca8b8b));  // 280, 101.248f, 280.7f, 101.266f, 281.4f, 101.273f
path.cubicTo(SkBits2Float(0x438d0ccd), SkBits2Float(0x42ca8eb3), SkBits2Float(0x438d6666), SkBits2Float(0x42ca9411), SkBits2Float(0x438dc000), SkBits2Float(0x42ca8eef));  // 282.1f, 101.279f, 282.8f, 101.289f, 283.5f, 101.279f
path.cubicTo(SkBits2Float(0x438e199a), SkBits2Float(0x42ca89cd), SkBits2Float(0x438e7333), SkBits2Float(0x42ca7cb5), SkBits2Float(0x438ecccd), SkBits2Float(0x42ca6cbe));  // 284.2f, 101.269f, 284.9f, 101.244f, 285.6f, 101.212f
path.cubicTo(SkBits2Float(0x438f2666), SkBits2Float(0x42ca5cc7), SkBits2Float(0x438f8000), SkBits2Float(0x42ca4681), SkBits2Float(0x438fd99a), SkBits2Float(0x42ca2f25));  // 286.3f, 101.181f, 287, 101.138f, 287.7f, 101.092f
path.cubicTo(SkBits2Float(0x43903333), SkBits2Float(0x42ca17c9), SkBits2Float(0x43908ccd), SkBits2Float(0x42c9f983), SkBits2Float(0x4390e666), SkBits2Float(0x42c9e096));  // 288.4f, 101.046f, 289.1f, 100.987f, 289.8f, 100.939f
path.cubicTo(SkBits2Float(0x43914000), SkBits2Float(0x42c9c7aa), SkBits2Float(0x4391999a), SkBits2Float(0x42c9a56f), SkBits2Float(0x4391f333), SkBits2Float(0x42c9999a));  // 290.5f, 100.89f, 291.2f, 100.823f, 291.9f, 100.8f
path.cubicTo(SkBits2Float(0x43924ccd), SkBits2Float(0x42c98dc6), SkBits2Float(0x4392a666), SkBits2Float(0x42c9999a), SkBits2Float(0x43930000), SkBits2Float(0x42c9999a));  // 292.6f, 100.777f, 293.3f, 100.8f, 294, 100.8f
path.lineTo(SkBits2Float(0x43930000), SkBits2Float(0x42c9999a));  // 294, 100.8f
path.lineTo(SkBits2Float(0x43478000), SkBits2Float(0x42c9999a));  // 199.5f, 100.8f
path.close();
path.moveTo(SkBits2Float(0x43ab2666), SkBits2Float(0x42c9999a));  // 342.3f, 100.8f
path.lineTo(SkBits2Float(0x43ab2666), SkBits2Float(0x42c9999a));  // 342.3f, 100.8f
path.cubicTo(SkBits2Float(0x43ab8000), SkBits2Float(0x42c9999a), SkBits2Float(0x43abd99a), SkBits2Float(0x42c98e0e), SkBits2Float(0x43ac3333), SkBits2Float(0x42c9999a));  // 343, 100.8f, 343.7f, 100.777f, 344.4f, 100.8f
path.cubicTo(SkBits2Float(0x43ac8ccd), SkBits2Float(0x42c9a527), SkBits2Float(0x43ace666), SkBits2Float(0x42c9bd29), SkBits2Float(0x43ad4000), SkBits2Float(0x42c9dee4));  // 345.1f, 100.823f, 345.8f, 100.869f, 346.5f, 100.935f
path.cubicTo(SkBits2Float(0x43ad999a), SkBits2Float(0x42ca009f), SkBits2Float(0x43adf333), SkBits2Float(0x42ca4737), SkBits2Float(0x43ae4ccd), SkBits2Float(0x42ca63fd));  // 347.2f, 101.001f, 347.9f, 101.139f, 348.6f, 101.195f
path.cubicTo(SkBits2Float(0x43aea666), SkBits2Float(0x42ca80c4), SkBits2Float(0x43af0000), SkBits2Float(0x42ca834d), SkBits2Float(0x43af599a), SkBits2Float(0x42ca8b8b));  // 349.3f, 101.251f, 350, 101.256f, 350.7f, 101.273f
path.cubicTo(SkBits2Float(0x43afb333), SkBits2Float(0x42ca93c9), SkBits2Float(0x43b00ccd), SkBits2Float(0x42ca92a9), SkBits2Float(0x43b06666), SkBits2Float(0x42ca9571));  // 351.4f, 101.289f, 352.1f, 101.286f, 352.8f, 101.292f
path.cubicTo(SkBits2Float(0x43b0c000), SkBits2Float(0x42ca9839), SkBits2Float(0x43b1199a), SkBits2Float(0x42ca9ad0), SkBits2Float(0x43b17333), SkBits2Float(0x42ca9c3a));  // 353.5f, 101.297f, 354.2f, 101.302f, 354.9f, 101.305f
path.cubicTo(SkBits2Float(0x43b1cccd), SkBits2Float(0x42ca9da3), SkBits2Float(0x43b22666), SkBits2Float(0x42ca9da3), SkBits2Float(0x43b28000), SkBits2Float(0x42ca9dec));  // 355.6f, 101.308f, 356.3f, 101.308f, 357, 101.308f
path.cubicTo(SkBits2Float(0x43b2d99a), SkBits2Float(0x42ca9e34), SkBits2Float(0x43b33333), SkBits2Float(0x42ca9e34), SkBits2Float(0x43b38ccd), SkBits2Float(0x42ca9dec));  // 357.7f, 101.309f, 358.4f, 101.309f, 359.1f, 101.308f
path.cubicTo(SkBits2Float(0x43b3e666), SkBits2Float(0x42ca9da3), SkBits2Float(0x43b44000), SkBits2Float(0x42ca9c82), SkBits2Float(0x43b4999a), SkBits2Float(0x42ca9c3a));  // 359.8f, 101.308f, 360.5f, 101.306f, 361.2f, 101.305f
path.cubicTo(SkBits2Float(0x43b4f333), SkBits2Float(0x42ca9bf1), SkBits2Float(0x43b54ccd), SkBits2Float(0x42ca9c3a), SkBits2Float(0x43b5a666), SkBits2Float(0x42ca9c3a));  // 361.9f, 101.305f, 362.6f, 101.305f, 363.3f, 101.305f
path.cubicTo(SkBits2Float(0x43b60000), SkBits2Float(0x42ca9c3a), SkBits2Float(0x43b6599a), SkBits2Float(0x42ca9bf1), SkBits2Float(0x43b6b333), SkBits2Float(0x42ca9c3a));  // 364, 101.305f, 364.7f, 101.305f, 365.4f, 101.305f
path.cubicTo(SkBits2Float(0x43b70ccd), SkBits2Float(0x42ca9c82), SkBits2Float(0x43b76666), SkBits2Float(0x42ca9da3), SkBits2Float(0x43b7c000), SkBits2Float(0x42ca9dec));  // 366.1f, 101.306f, 366.8f, 101.308f, 367.5f, 101.308f
path.cubicTo(SkBits2Float(0x43b8199a), SkBits2Float(0x42ca9e34), SkBits2Float(0x43b87333), SkBits2Float(0x42ca9dec), SkBits2Float(0x43b8cccd), SkBits2Float(0x42ca9dec));  // 368.2f, 101.309f, 368.9f, 101.308f, 369.6f, 101.308f
path.cubicTo(SkBits2Float(0x43b92666), SkBits2Float(0x42ca9dec), SkBits2Float(0x43b98000), SkBits2Float(0x42ca9dec), SkBits2Float(0x43b9d99a), SkBits2Float(0x42ca9dec));  // 370.3f, 101.308f, 371, 101.308f, 371.7f, 101.308f
path.cubicTo(SkBits2Float(0x43ba3333), SkBits2Float(0x42ca9dec), SkBits2Float(0x43ba8ccd), SkBits2Float(0x42ca9e7d), SkBits2Float(0x43bae666), SkBits2Float(0x42ca9dec));  // 372.4f, 101.308f, 373.1f, 101.31f, 373.8f, 101.308f
path.cubicTo(SkBits2Float(0x43bb4000), SkBits2Float(0x42ca9d5b), SkBits2Float(0x43bb999a), SkBits2Float(0x42ca9b61), SkBits2Float(0x43bbf333), SkBits2Float(0x42ca9a88));  // 374.5f, 101.307f, 375.2f, 101.303f, 375.9f, 101.302f
path.cubicTo(SkBits2Float(0x43bc4ccd), SkBits2Float(0x42ca99af), SkBits2Float(0x43bca666), SkBits2Float(0x42ca991e), SkBits2Float(0x43bd0000), SkBits2Float(0x42ca98d5));  // 376.6f, 101.3f, 377.3f, 101.299f, 378, 101.299f
path.cubicTo(SkBits2Float(0x43bd599a), SkBits2Float(0x42ca988d), SkBits2Float(0x43bdb333), SkBits2Float(0x42ca988d), SkBits2Float(0x43be0ccd), SkBits2Float(0x42ca98d5));  // 378.7f, 101.298f, 379.4f, 101.298f, 380.1f, 101.299f
path.cubicTo(SkBits2Float(0x43be6666), SkBits2Float(0x42ca991e), SkBits2Float(0x43bec000), SkBits2Float(0x42ca99f7), SkBits2Float(0x43bf199a), SkBits2Float(0x42ca9a88));  // 380.8f, 101.299f, 381.5f, 101.301f, 382.2f, 101.302f
path.cubicTo(SkBits2Float(0x43bf7333), SkBits2Float(0x42ca9b18), SkBits2Float(0x43bfcccd), SkBits2Float(0x42ca9ba9), SkBits2Float(0x43c02666), SkBits2Float(0x42ca9c3a));  // 382.9f, 101.303f, 383.6f, 101.304f, 384.3f, 101.305f
path.cubicTo(SkBits2Float(0x43c08000), SkBits2Float(0x42ca9cca), SkBits2Float(0x43c0d99a), SkBits2Float(0x42ca9ec5), SkBits2Float(0x43c13333), SkBits2Float(0x42ca9dec));  // 385, 101.306f, 385.7f, 101.31f, 386.4f, 101.308f
path.cubicTo(SkBits2Float(0x43c18ccd), SkBits2Float(0x42ca9d13), SkBits2Float(0x43c1e666), SkBits2Float(0x42ca9a3f), SkBits2Float(0x43c24000), SkBits2Float(0x42ca9723));  // 387.1f, 101.307f, 387.8f, 101.301f, 388.5f, 101.295f
path.cubicTo(SkBits2Float(0x43c2999a), SkBits2Float(0x42ca9407), SkBits2Float(0x43c2f333), SkBits2Float(0x42ca8d87), SkBits2Float(0x43c34ccd), SkBits2Float(0x42ca8b45));  // 389.2f, 101.289f, 389.9f, 101.276f, 390.6f, 101.272f
path.cubicTo(SkBits2Float(0x43c3a666), SkBits2Float(0x42ca8902), SkBits2Float(0x43c40000), SkBits2Float(0x42ca8992), SkBits2Float(0x43c4599a), SkBits2Float(0x42ca8992));  // 391.3f, 101.268f, 392, 101.269f, 392.7f, 101.269f
path.cubicTo(SkBits2Float(0x43c4b333), SkBits2Float(0x42ca8992), SkBits2Float(0x43c50ccd), SkBits2Float(0x42ca8a6b), SkBits2Float(0x43c56666), SkBits2Float(0x42ca8b45));  // 393.4f, 101.269f, 394.1f, 101.27f, 394.8f, 101.272f
path.cubicTo(SkBits2Float(0x43c5c000), SkBits2Float(0x42ca8c1e), SkBits2Float(0x43c6199a), SkBits2Float(0x42ca8d87), SkBits2Float(0x43c67333), SkBits2Float(0x42ca8ea9));  // 395.5f, 101.274f, 396.2f, 101.276f, 396.9f, 101.279f
path.cubicTo(SkBits2Float(0x43c6cccd), SkBits2Float(0x42ca8fca), SkBits2Float(0x43c72666), SkBits2Float(0x42ca90a3), SkBits2Float(0x43c78000), SkBits2Float(0x42ca920d));  // 397.6f, 101.281f, 398.3f, 101.282f, 399, 101.285f
path.cubicTo(SkBits2Float(0x43c7d99a), SkBits2Float(0x42ca9377), SkBits2Float(0x43c83333), SkBits2Float(0x42ca9571), SkBits2Float(0x43c88ccd), SkBits2Float(0x42ca9723));  // 399.7f, 101.288f, 400.4f, 101.292f, 401.1f, 101.295f
path.cubicTo(SkBits2Float(0x43c8e666), SkBits2Float(0x42ca98d5), SkBits2Float(0x43c94000), SkBits2Float(0x42ca9a88), SkBits2Float(0x43c9999a), SkBits2Float(0x42ca9c3a));  // 401.8f, 101.299f, 402.5f, 101.302f, 403.2f, 101.305f
path.cubicTo(SkBits2Float(0x43c9f333), SkBits2Float(0x42ca9dec), SkBits2Float(0x43ca4ccd), SkBits2Float(0x42caa02f), SkBits2Float(0x43caa666), SkBits2Float(0x42caa150));  // 403.9f, 101.308f, 404.6f, 101.313f, 405.3f, 101.315f
path.cubicTo(SkBits2Float(0x43cb0000), SkBits2Float(0x42caa271), SkBits2Float(0x43cb599a), SkBits2Float(0x42caa46c), SkBits2Float(0x43cbb333), SkBits2Float(0x42caa302));  // 406, 101.317f, 406.7f, 101.321f, 407.4f, 101.318f
path.cubicTo(SkBits2Float(0x43cc0ccd), SkBits2Float(0x42caa198), SkBits2Float(0x43cc6666), SkBits2Float(0x42ca9ad0), SkBits2Float(0x43ccc000), SkBits2Float(0x42ca98d5));  // 408.1f, 101.316f, 408.8f, 101.302f, 409.5f, 101.299f
path.cubicTo(SkBits2Float(0x43cd199a), SkBits2Float(0x42ca96db), SkBits2Float(0x43cd7333), SkBits2Float(0x42ca9535), SkBits2Float(0x43cdcccd), SkBits2Float(0x42ca9723));  // 410.2f, 101.295f, 410.9f, 101.291f, 411.6f, 101.295f
path.cubicTo(SkBits2Float(0x43ce2666), SkBits2Float(0x42ca9912), SkBits2Float(0x43ce8000), SkBits2Float(0x42caa19a), SkBits2Float(0x43ced99a), SkBits2Float(0x42caa46e));  // 412.3f, 101.299f, 413, 101.316f, 413.7f, 101.321f
path.cubicTo(SkBits2Float(0x43cf3333), SkBits2Float(0x42caa741), SkBits2Float(0x43cf8ccd), SkBits2Float(0x42caa7c4), SkBits2Float(0x43cfe666), SkBits2Float(0x42caa819));  // 414.4f, 101.327f, 415.1f, 101.328f, 415.8f, 101.328f
path.cubicTo(SkBits2Float(0x43d04000), SkBits2Float(0x42caa86d), SkBits2Float(0x43d0999a), SkBits2Float(0x42caa788), SkBits2Float(0x43d0f333), SkBits2Float(0x42caa666));  // 416.5f, 101.329f, 417.2f, 101.327f, 417.9f, 101.325f
path.cubicTo(SkBits2Float(0x43d14ccd), SkBits2Float(0x42caa545), SkBits2Float(0x43d1a666), SkBits2Float(0x42caa302), SkBits2Float(0x43d20000), SkBits2Float(0x42caa150));  // 418.6f, 101.323f, 419.3f, 101.318f, 420, 101.315f
path.cubicTo(SkBits2Float(0x43d2599a), SkBits2Float(0x42ca9f9e), SkBits2Float(0x43d2b333), SkBits2Float(0x42ca9d13), SkBits2Float(0x43d30ccd), SkBits2Float(0x42ca9c3a));  // 420.7f, 101.312f, 421.4f, 101.307f, 422.1f, 101.305f
path.cubicTo(SkBits2Float(0x43d36666), SkBits2Float(0x42ca9b61), SkBits2Float(0x43d3c000), SkBits2Float(0x42ca9b24), SkBits2Float(0x43d4199a), SkBits2Float(0x42ca9c3a));  // 422.8f, 101.303f, 423.5f, 101.303f, 424.2f, 101.305f
path.cubicTo(SkBits2Float(0x43d47333), SkBits2Float(0x42ca9d4f), SkBits2Float(0x43d4cccd), SkBits2Float(0x42caa59b), SkBits2Float(0x43d52666), SkBits2Float(0x42caa2bc));  // 424.9f, 101.307f, 425.6f, 101.323f, 426.3f, 101.318f
path.cubicTo(SkBits2Float(0x43d58000), SkBits2Float(0x42ca9fdc), SkBits2Float(0x43d5d99a), SkBits2Float(0x42caa425), SkBits2Float(0x43d63333), SkBits2Float(0x42ca8afe));  // 427, 101.312f, 427.7f, 101.321f, 428.4f, 101.271f
path.cubicTo(SkBits2Float(0x43d68ccd), SkBits2Float(0x42ca71d6), SkBits2Float(0x43d6e666), SkBits2Float(0x42ca340a), SkBits2Float(0x43d74000), SkBits2Float(0x42ca0bcf));  // 429.1f, 101.222f, 429.8f, 101.102f, 430.5f, 101.023f
path.cubicTo(SkBits2Float(0x43d7999a), SkBits2Float(0x42c9e394), SkBits2Float(0x43d7f333), SkBits2Float(0x42c9aca3), SkBits2Float(0x43d84ccd), SkBits2Float(0x42c9999a));  // 431.2f, 100.944f, 431.9f, 100.837f, 432.6f, 100.8f
path.cubicTo(SkBits2Float(0x43d8a666), SkBits2Float(0x42c98692), SkBits2Float(0x43d90000), SkBits2Float(0x42c9999a), SkBits2Float(0x43d9599a), SkBits2Float(0x42c9999a));  // 433.3f, 100.763f, 434, 100.8f, 434.7f, 100.8f
path.lineTo(SkBits2Float(0x43d9599a), SkBits2Float(0x42c9999a));  // 434.7f, 100.8f
path.lineTo(SkBits2Float(0x43ab2666), SkBits2Float(0x42c9999a));  // 342.3f, 100.8f
path.close();
path.moveTo(SkBits2Float(0x43dfa666), SkBits2Float(0x42c9999a));  // 447.3f, 100.8f
path.lineTo(SkBits2Float(0x43dfa666), SkBits2Float(0x42c9999a));  // 447.3f, 100.8f
path.cubicTo(SkBits2Float(0x43e00000), SkBits2Float(0x42c99abd), SkBits2Float(0x43e0599a), SkBits2Float(0x42c9a022), SkBits2Float(0x43e0b333), SkBits2Float(0x42c9a06c));  // 448, 100.802f, 448.7f, 100.813f, 449.4f, 100.813f
path.cubicTo(SkBits2Float(0x43e10ccd), SkBits2Float(0x42c9a0b6), SkBits2Float(0x43e16666), SkBits2Float(0x42c99c79), SkBits2Float(0x43e1c000), SkBits2Float(0x42c99b56));  // 450.1f, 100.814f, 450.8f, 100.806f, 451.5f, 100.803f
path.cubicTo(SkBits2Float(0x43e2199a), SkBits2Float(0x42c99a33), SkBits2Float(0x43e27333), SkBits2Float(0x42c9928a), SkBits2Float(0x43e2cccd), SkBits2Float(0x42c9999a));  // 452.2f, 100.801f, 452.9f, 100.786f, 453.6f, 100.8f
path.cubicTo(SkBits2Float(0x43e32666), SkBits2Float(0x42c9a0ab), SkBits2Float(0x43e38000), SkBits2Float(0x42c9b682), SkBits2Float(0x43e3d99a), SkBits2Float(0x42c9c5bb));  // 454.3f, 100.814f, 455, 100.856f, 455.7f, 100.886f
path.cubicTo(SkBits2Float(0x43e43333), SkBits2Float(0x42c9d4f4), SkBits2Float(0x43e48ccd), SkBits2Float(0x42c9ef91), SkBits2Float(0x43e4e666), SkBits2Float(0x42c9f4f0));  // 456.4f, 100.916f, 457.1f, 100.968f, 457.8f, 100.978f
path.cubicTo(SkBits2Float(0x43e54000), SkBits2Float(0x42c9fa4e), SkBits2Float(0x43e5999a), SkBits2Float(0x42c9e940), SkBits2Float(0x43e5f333), SkBits2Float(0x42c9e5f3));  // 458.5f, 100.989f, 459.2f, 100.956f, 459.9f, 100.949f
path.cubicTo(SkBits2Float(0x43e64ccd), SkBits2Float(0x42c9e2a6), SkBits2Float(0x43e6a666), SkBits2Float(0x42c9e366), SkBits2Float(0x43e70000), SkBits2Float(0x42c9e123));  // 460.6f, 100.943f, 461.3f, 100.944f, 462, 100.94f
path.cubicTo(SkBits2Float(0x43e7599a), SkBits2Float(0x42c9dee0), SkBits2Float(0x43e7b333), SkBits2Float(0x42c9d9d8), SkBits2Float(0x43e80ccd), SkBits2Float(0x42c9d862));  // 462.7f, 100.935f, 463.4f, 100.925f, 464.1f, 100.923f
path.cubicTo(SkBits2Float(0x43e86666), SkBits2Float(0x42c9d6ed), SkBits2Float(0x43e8c000), SkBits2Float(0x42c9d93b), SkBits2Float(0x43e9199a), SkBits2Float(0x42c9d862));  // 464.8f, 100.92f, 465.5f, 100.924f, 466.2f, 100.923f
path.cubicTo(SkBits2Float(0x43e97333), SkBits2Float(0x42c9d789), SkBits2Float(0x43e9cccd), SkBits2Float(0x42c9d614), SkBits2Float(0x43ea2666), SkBits2Float(0x42c9d34c));  // 466.9f, 100.921f, 467.6f, 100.918f, 468.3f, 100.913f
path.cubicTo(SkBits2Float(0x43ea8000), SkBits2Float(0x42c9d084), SkBits2Float(0x43ead99a), SkBits2Float(0x42c9cbf1), SkBits2Float(0x43eb3333), SkBits2Float(0x42c9c7b4));  // 469, 100.907f, 469.7f, 100.898f, 470.4f, 100.89f
path.cubicTo(SkBits2Float(0x43eb8ccd), SkBits2Float(0x42c9c376), SkBits2Float(0x43ebe666), SkBits2Float(0x42c9bde8), SkBits2Float(0x43ec4000), SkBits2Float(0x42c9b9dc));  // 471.1f, 100.882f, 471.8f, 100.871f, 472.5f, 100.863f
path.cubicTo(SkBits2Float(0x43ec999a), SkBits2Float(0x42c9b5cf), SkBits2Float(0x43ecf333), SkBits2Float(0x42c9b442), SkBits2Float(0x43ed4ccd), SkBits2Float(0x42c9af69));  // 473.2f, 100.855f, 473.9f, 100.852f, 474.6f, 100.843f
path.cubicTo(SkBits2Float(0x43eda666), SkBits2Float(0x42c9aa8f), SkBits2Float(0x43ee0000), SkBits2Float(0x42c9a064), SkBits2Float(0x43ee599a), SkBits2Float(0x42c99cc1));  // 475.3f, 100.833f, 476, 100.813f, 476.7f, 100.806f
path.cubicTo(SkBits2Float(0x43eeb333), SkBits2Float(0x42c9991f), SkBits2Float(0x43ef0ccd), SkBits2Float(0x42c99758), SkBits2Float(0x43ef6666), SkBits2Float(0x42c9999a));  // 477.4f, 100.799f, 478.1f, 100.796f, 478.8f, 100.8f
path.cubicTo(SkBits2Float(0x43efc000), SkBits2Float(0x42c99bdd), SkBits2Float(0x43f0199a), SkBits2Float(0x42c99f0e), SkBits2Float(0x43f07333), SkBits2Float(0x42c9aa52));  // 479.5f, 100.804f, 480.2f, 100.811f, 480.9f, 100.833f
path.cubicTo(SkBits2Float(0x43f0cccd), SkBits2Float(0x42c9b596), SkBits2Float(0x43f12666), SkBits2Float(0x42c9cfde), SkBits2Float(0x43f18000), SkBits2Float(0x42c9dd32));  // 481.6f, 100.855f, 482.3f, 100.906f, 483, 100.932f
path.cubicTo(SkBits2Float(0x43f1d99a), SkBits2Float(0x42c9ea86), SkBits2Float(0x43f23333), SkBits2Float(0x42c9f451), SkBits2Float(0x43f28ccd), SkBits2Float(0x42c9fa4d));  // 483.7f, 100.958f, 484.4f, 100.977f, 485.1f, 100.989f
path.cubicTo(SkBits2Float(0x43f2e666), SkBits2Float(0x42ca0048), SkBits2Float(0x43f34000), SkBits2Float(0x42c9fbf3), SkBits2Float(0x43f3999a), SkBits2Float(0x42ca0115));  // 485.8f, 101.001f, 486.5f, 100.992f, 487.2f, 101.002f
path.cubicTo(SkBits2Float(0x43f3f333), SkBits2Float(0x42ca0637), SkBits2Float(0x43f44ccd), SkBits2Float(0x42ca103f), SkBits2Float(0x43f4a666), SkBits2Float(0x42ca1919));  // 487.9f, 101.012f, 488.6f, 101.032f, 489.3f, 101.049f
path.cubicTo(SkBits2Float(0x43f50000), SkBits2Float(0x42ca21f4), SkBits2Float(0x43f5599a), SkBits2Float(0x42ca2e87), SkBits2Float(0x43f5b333), SkBits2Float(0x42ca3634));  // 490, 101.066f, 490.7f, 101.091f, 491.4f, 101.106f
path.cubicTo(SkBits2Float(0x43f60ccd), SkBits2Float(0x42ca3de1), SkBits2Float(0x43f66666), SkBits2Float(0x42ca43b9), SkBits2Float(0x43f6c000), SkBits2Float(0x42ca4729));  // 492.1f, 101.121f, 492.8f, 101.132f, 493.5f, 101.139f
path.cubicTo(SkBits2Float(0x43f7199a), SkBits2Float(0x42ca4a99), SkBits2Float(0x43f77333), SkBits2Float(0x42ca49b3), SkBits2Float(0x43f7cccd), SkBits2Float(0x42ca4ad4));  // 494.2f, 101.146f, 494.9f, 101.144f, 495.6f, 101.146f
path.cubicTo(SkBits2Float(0x43f82666), SkBits2Float(0x42ca4bf5), SkBits2Float(0x43f88000), SkBits2Float(0x42ca4cdc), SkBits2Float(0x43f8d99a), SkBits2Float(0x42ca4df2));  // 496.3f, 101.148f, 497, 101.15f, 497.7f, 101.152f
path.cubicTo(SkBits2Float(0x43f93333), SkBits2Float(0x42ca4f07), SkBits2Float(0x43f98ccd), SkBits2Float(0x42ca4fec), SkBits2Float(0x43f9e666), SkBits2Float(0x42ca5156));  // 498.4f, 101.154f, 499.1f, 101.156f, 499.8f, 101.159f
path.cubicTo(SkBits2Float(0x43fa4000), SkBits2Float(0x42ca52c0), SkBits2Float(0x43fa999a), SkBits2Float(0x42ca53e1), SkBits2Float(0x43faf333), SkBits2Float(0x42ca566c));  // 500.5f, 101.162f, 501.2f, 101.164f, 501.9f, 101.169f
path.cubicTo(SkBits2Float(0x43fb4ccd), SkBits2Float(0x42ca58f7), SkBits2Float(0x43fba666), SkBits2Float(0x42ca5d71), SkBits2Float(0x43fc0000), SkBits2Float(0x42ca6099));  // 502.6f, 101.174f, 503.3f, 101.183f, 504, 101.189f
path.cubicTo(SkBits2Float(0x43fc599a), SkBits2Float(0x42ca63c1), SkBits2Float(0x43fcb333), SkBits2Float(0x42ca663e), SkBits2Float(0x43fd0ccd), SkBits2Float(0x42ca695a));  // 504.7f, 101.195f, 505.4f, 101.2f, 506.1f, 101.206f
path.cubicTo(SkBits2Float(0x43fd6666), SkBits2Float(0x42ca6c76), SkBits2Float(0x43fdc000), SkBits2Float(0x42ca7024), SkBits2Float(0x43fe199a), SkBits2Float(0x42ca7340));  // 506.8f, 101.212f, 507.5f, 101.219f, 508.2f, 101.225f
path.cubicTo(SkBits2Float(0x43fe7333), SkBits2Float(0x42ca765c), SkBits2Float(0x43fecccd), SkBits2Float(0x42ca7a44), SkBits2Float(0x43ff2666), SkBits2Float(0x42ca7c01));  // 508.9f, 101.231f, 509.6f, 101.239f, 510.3f, 101.242f
path.cubicTo(SkBits2Float(0x43ff8000), SkBits2Float(0x42ca7dbf), SkBits2Float(0x43ffd99a), SkBits2Float(0x42ca7ed5), SkBits2Float(0x4400199a), SkBits2Float(0x42ca7db4));  // 511, 101.246f, 511.7f, 101.248f, 512.4f, 101.246f
path.cubicTo(SkBits2Float(0x44004666), SkBits2Float(0x42ca7c92), SkBits2Float(0x44007333), SkBits2Float(0x42ca780c), SkBits2Float(0x4400a000), SkBits2Float(0x42ca7539));  // 513.1f, 101.243f, 513.8f, 101.234f, 514.5f, 101.229f
path.cubicTo(SkBits2Float(0x4400cccd), SkBits2Float(0x42ca7265), SkBits2Float(0x4400f99a), SkBits2Float(0x42ca7017), SkBits2Float(0x44012666), SkBits2Float(0x42ca6cbe));  // 515.2f, 101.223f, 515.9f, 101.219f, 516.6f, 101.212f
path.cubicTo(SkBits2Float(0x44015333), SkBits2Float(0x42ca6966), SkBits2Float(0x44018000), SkBits2Float(0x42ca688b), SkBits2Float(0x4401accd), SkBits2Float(0x42ca6126));  // 517.3f, 101.206f, 518, 101.204f, 518.7f, 101.19f
path.cubicTo(SkBits2Float(0x4401d99a), SkBits2Float(0x42ca59c1), SkBits2Float(0x44020666), SkBits2Float(0x42ca4eee), SkBits2Float(0x44023333), SkBits2Float(0x42ca4061));  // 519.4f, 101.175f, 520.1f, 101.154f, 520.8f, 101.126f
path.cubicTo(SkBits2Float(0x44026000), SkBits2Float(0x42ca31d3), SkBits2Float(0x44028ccd), SkBits2Float(0x42ca1b74), SkBits2Float(0x4402b99a), SkBits2Float(0x42ca09d6));  // 521.5f, 101.097f, 522.2f, 101.054f, 522.9f, 101.019f
path.cubicTo(SkBits2Float(0x4402e666), SkBits2Float(0x42c9f839), SkBits2Float(0x44031333), SkBits2Float(0x42c9e720), SkBits2Float(0x44034000), SkBits2Float(0x42c9d6b0));  // 523.6f, 100.985f, 524.3f, 100.951f, 525, 100.919f
path.cubicTo(SkBits2Float(0x44036ccd), SkBits2Float(0x42c9c640), SkBits2Float(0x4403999a), SkBits2Float(0x42c9b163), SkBits2Float(0x4403c666), SkBits2Float(0x42c9a735));  // 525.7f, 100.887f, 526.4f, 100.846f, 527.1f, 100.827f
path.cubicTo(SkBits2Float(0x4403f333), SkBits2Float(0x42c99d06), SkBits2Float(0x44042000), SkBits2Float(0x42c99bdf), SkBits2Float(0x44044ccd), SkBits2Float(0x42c9999a));  // 527.8f, 100.807f, 528.5f, 100.804f, 529.2f, 100.8f
path.lineTo(SkBits2Float(0x44044ccd), SkBits2Float(0x42c9999a));  // 529.2f, 100.8f
path.lineTo(SkBits2Float(0x43dfa666), SkBits2Float(0x42c9999a));  // 447.3f, 100.8f
path.close();
    return path;
}

static void issue3651_1(skiatest::Reporter* reporter, const char* filename) {
    SkPath path = path1();
    SkPath pathB = path2();
    testPathOp(reporter, path, pathB, SkPathOp::kUnion_SkPathOp, filename);
}

static void issue3651_2(skiatest::Reporter* reporter, const char* filename) {
    SkPath path = path3();
    SkPath pathB = path4();
    testPathOp(reporter, path, pathB, SkPathOp::kUnion_SkPathOp, filename);
}

static void issue3651_3(skiatest::Reporter* reporter, const char* filename) {
    SkPath path = path5();
    SkPath pathB = path6();
    testPathOp(reporter, path, pathB, SkPathOp::kUnion_SkPathOp, filename);
}

static void (*skipTest)(skiatest::Reporter* , const char* filename) = 0;
static void (*firstTest)(skiatest::Reporter* , const char* filename) = issue3651_3;
static void (*stopTest)(skiatest::Reporter* , const char* filename) = 0;

static struct TestDesc tests[] = {
    TEST(issue3651_1),
    TEST(issue3651_2),
    TEST(issue3651_3),
};

static const size_t testCount = SK_ARRAY_COUNT(tests);

static bool runReverse = false;

DEF_TEST(PathOpsIssue3651, reporter) {
#if DEBUG_SHOW_TEST_NAME
    strncpy(DEBUG_FILENAME_STRING, "", DEBUG_FILENAME_STRING_LENGTH);
#endif
    RunTestSet(reporter, tests, testCount, firstTest, skipTest, stopTest, runReverse);
}
