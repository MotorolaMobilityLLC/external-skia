#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 testInputs;
    float4 colorGreen;
    float4 colorRed;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};

fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    int4 expected = int4(-1, 0, 0, 1);
    _out.sk_FragColor = ((((((sign(int(_uniforms.testInputs.x)) == expected.x && all(sign(int2(_uniforms.testInputs.xy)) == expected.xy)) && all(sign(int3(_uniforms.testInputs.xyz)) == expected.xyz)) && all(sign(int4(_uniforms.testInputs)) == expected)) && -1 == expected.x) && all(int2(-1, 0) == expected.xy)) && all(int3(-1, 0, 0) == expected.xyz)) && all(int4(-1, 0, 0, 1) == expected) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
