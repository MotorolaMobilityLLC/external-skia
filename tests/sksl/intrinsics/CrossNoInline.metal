#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float2 ah;
    float2 bh;
    float2 af;
    float2 bf;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
float cross_hh2h2(float2 a, float2 b) {
    return a.x * b.y - a.y * b.x;
}
float cross_ff2f2(float2 a, float2 b) {
    return a.x * b.y - a.y * b.x;
}

fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor.x = cross_hh2h2(_uniforms.ah, _uniforms.bh);
    _out.sk_FragColor.y = cross_ff2f2(_uniforms.af, _uniforms.bf);
    return _out;
}
