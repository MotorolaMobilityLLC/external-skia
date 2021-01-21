#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
struct Globals {
    float2 v1;
    float2 v2;
    float2 v3;
    float3 v4;
    int2 v5;
    int2 v6;
    float2 v7;
    float2 v8;
    float4 v9;
    int2 v10;
    bool4 v11;
    float2 v12;
    float2 v13;
    float2 v14;
    bool2 v15;
    bool2 v16;
    bool3 v17;
};

















fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _skGlobals{float2(1.0), float2(1.0, 2.0), float2(1.0), float3(float2(1.0), 1.0), int2(1), int2(float2(1.0, 2.0)), float2(int2(1, 2)), float2(_skGlobals.v5), float4(float(_skGlobals.v6.x), sqrt(2.0), float2(int2(3, 4))), int2(3, int(_skGlobals.v1.x)), bool4(bool2(true, false), true, false), float2(1.0, 0.0), float2(0.0), float2(bool2(false)), bool2(true), bool2(float2(1.0)), bool3(true, bool2(int2(77)))};
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    _out->sk_FragColor.x = (((((((((((((((_skGlobals.v1.x + _skGlobals.v2.x) + _skGlobals.v3.x) + _skGlobals.v4.x) + float(_skGlobals.v5.x)) + float(_skGlobals.v6.x)) + _skGlobals.v7.x) + _skGlobals.v8.x) + _skGlobals.v9.x) + float(_skGlobals.v10.x)) + float(_skGlobals.v11.x)) + _skGlobals.v12.x) + _skGlobals.v13.x) + _skGlobals.v14.x) + float(_skGlobals.v15.x)) + float(_skGlobals.v16.x)) + float(_skGlobals.v17.x);
    return *_out;
}
