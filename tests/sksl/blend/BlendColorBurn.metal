#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    half4 src;
    half4 dst;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
half _color_burn_component_hh2h2(half2 s, half2 d) {
    if (d.y == d.x) {
        return (s.y * d.y + s.x * (1.0h - d.y)) + d.x * (1.0h - s.y);
    } else if (s.x == 0.0h) {
        return d.x * (1.0h - s.y);
    } else {
        half delta = max(0.0h, d.y - ((d.y - d.x) * s.y) / s.x);
        return (delta * s.y + s.x * (1.0h - d.y)) + d.x * (1.0h - s.y);
    }
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = half4(_color_burn_component_hh2h2(_uniforms.src.xw, _uniforms.dst.xw), _color_burn_component_hh2h2(_uniforms.src.yw, _uniforms.dst.yw), _color_burn_component_hh2h2(_uniforms.src.zw, _uniforms.dst.zw), _uniforms.src.w + (1.0h - _uniforms.src.w) * _uniforms.dst.w);
    return _out;
}
