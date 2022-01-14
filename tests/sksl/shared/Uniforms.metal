#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    half myHalf;
    half4 myHalf4;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = _uniforms.myHalf4 * _uniforms.myHalf;
    return _out;
}
