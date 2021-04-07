
out vec4 sk_FragColor;
uniform vec4 colorRed;
uniform vec4 colorGreen;
uniform float unknownInput;
bool test_int_b() {
    bool ok = true;
    ivec4 x = ivec4(6, 6, 7, 8);
    ok = ok && x == ivec4(6, 6, 7, 8);
    x = ivec4(7, 9, 9, 9);
    ok = ok && x == ivec4(7, 9, 9, 9);
    x = ivec4(9, 9, 10, 10);
    ok = ok && x == ivec4(9, 9, 10, 10);
    x.xyz = ivec3(6, 6, 6);
    ok = ok && x == ivec4(6, 6, 6, 10);
    x.xy = ivec2(3, 3);
    ok = ok && x == ivec4(3, 3, 6, 10);
    x = ivec4(6, 6, 6, 6);
    ok = ok && x == ivec4(6);
    x = ivec4(6, 6, 7, 8);
    ok = ok && x == ivec4(6, 6, 7, 8);
    x = ivec4(-7, -9, -9, -9);
    ok = ok && x == ivec4(-7, -9, -9, -9);
    x = ivec4(9, 9, 10, 10);
    ok = ok && x == ivec4(9, 9, 10, 10);
    x.xyz = ivec3(6, 6, 6);
    ok = ok && x == ivec4(6, 6, 6, 10);
    x.xy = ivec2(8, 8);
    ok = ok && x == ivec4(8, 8, 6, 10);
    x = ivec4(200, 100, 50, 25);
    ok = ok && x == ivec4(200, 100, 50, 25);
    x = ivec4(6, 6, 6, 6);
    ok = ok && x == ivec4(6);
    int unknown = int(unknownInput);
    x = ivec4(unknown);
    ok = ok && x == ivec4(unknown);
    x = ivec4(0);
    ok = ok && x == ivec4(0);
    x = ivec4(0);
    ok = ok && x == ivec4(0);
    x = ivec4(unknown);
    ok = ok && x == ivec4(unknown);
    x = ivec4(unknown);
    ok = ok && x == ivec4(unknown);
    x = ivec4(unknown);
    ok = ok && x == ivec4(unknown);
    x = ivec4(unknown);
    ok = ok && x == ivec4(unknown);
    x = ivec4(unknown);
    ok = ok && x == ivec4(unknown);
    x = ivec4(unknown);
    ok = ok && x == ivec4(unknown);
    x = ivec4(0);
    ok = ok && x == ivec4(0);
    x = ivec4(0);
    ok = ok && x == ivec4(0);
    x = ivec4(unknown);
    ok = ok && x == ivec4(unknown);
    x = ivec4(unknown);
    ok = ok && x == ivec4(unknown);
    x = ivec4(0);
    ok = ok && x == ivec4(0);
    x = ivec4(unknown);
    ok = ok && x == ivec4(unknown);
    x = ivec4(unknown);
    ok = ok && x == ivec4(unknown);
    x = ivec4(unknown);
    x += 1;
    x -= 1;
    ok = ok && x == ivec4(unknown);
    x = ivec4(unknown);
    x = x + 1;
    x = x - 1;
    ok = ok && x == ivec4(unknown);
    return ok;
}
vec4 main() {
    bool _0_ok = true;
    vec4 _1_x = vec4(6.0, 6.0, 7.0, 8.0);
    _0_ok = _0_ok && _1_x == vec4(6.0, 6.0, 7.0, 8.0);
    _1_x = vec4(7.0, 9.0, 9.0, 9.0);
    _0_ok = _0_ok && _1_x == vec4(7.0, 9.0, 9.0, 9.0);
    _1_x = vec4(9.0, 9.0, 10.0, 10.0);
    _0_ok = _0_ok && _1_x == vec4(9.0, 9.0, 10.0, 10.0);
    _1_x.xyz = vec3(6.0, 6.0, 6.0);
    _0_ok = _0_ok && _1_x == vec4(6.0, 6.0, 6.0, 10.0);
    _1_x.xy = vec2(3.0, 3.0);
    _0_ok = _0_ok && _1_x == vec4(3.0, 3.0, 6.0, 10.0);
    _1_x = vec4(6.0, 6.0, 6.0, 6.0);
    _0_ok = _0_ok && _1_x == vec4(6.0);
    _1_x = vec4(6.0, 6.0, 7.0, 8.0);
    _0_ok = _0_ok && _1_x == vec4(6.0, 6.0, 7.0, 8.0);
    _1_x = vec4(-7.0, -9.0, -9.0, -9.0);
    _0_ok = _0_ok && _1_x == vec4(-7.0, -9.0, -9.0, -9.0);
    _1_x = vec4(9.0, 9.0, 10.0, 10.0);
    _0_ok = _0_ok && _1_x == vec4(9.0, 9.0, 10.0, 10.0);
    _1_x.xyz = vec3(6.0, 6.0, 6.0);
    _0_ok = _0_ok && _1_x == vec4(6.0, 6.0, 6.0, 10.0);
    _1_x.xy = vec2(8.0, 8.0);
    _0_ok = _0_ok && _1_x == vec4(8.0, 8.0, 6.0, 10.0);
    _1_x = vec4(2.0, 1.0, 0.5, 0.25);
    _0_ok = _0_ok && _1_x == vec4(2.0, 1.0, 0.5, 0.25);
    _1_x = vec4(6.0, 6.0, 6.0, 6.0);
    _0_ok = _0_ok && _1_x == vec4(6.0);
    float _2_unknown = unknownInput;
    _1_x = vec4(_2_unknown);
    _0_ok = _0_ok && _1_x == vec4(_2_unknown);
    _1_x = vec4(0.0);
    _0_ok = _0_ok && _1_x == vec4(0.0);
    _1_x = vec4(0.0);
    _0_ok = _0_ok && _1_x == vec4(0.0);
    _1_x = vec4(_2_unknown);
    _0_ok = _0_ok && _1_x == vec4(_2_unknown);
    _1_x = vec4(_2_unknown);
    _0_ok = _0_ok && _1_x == vec4(_2_unknown);
    _1_x = vec4(_2_unknown);
    _0_ok = _0_ok && _1_x == vec4(_2_unknown);
    _1_x = vec4(_2_unknown);
    _0_ok = _0_ok && _1_x == vec4(_2_unknown);
    _1_x = vec4(_2_unknown);
    _0_ok = _0_ok && _1_x == vec4(_2_unknown);
    _1_x = vec4(_2_unknown);
    _0_ok = _0_ok && _1_x == vec4(_2_unknown);
    _1_x = vec4(0.0);
    _0_ok = _0_ok && _1_x == vec4(0.0);
    _1_x = vec4(0.0);
    _0_ok = _0_ok && _1_x == vec4(0.0);
    _1_x = vec4(_2_unknown);
    _0_ok = _0_ok && _1_x == vec4(_2_unknown);
    _1_x = vec4(_2_unknown);
    _0_ok = _0_ok && _1_x == vec4(_2_unknown);
    _1_x = vec4(0.0);
    _0_ok = _0_ok && _1_x == vec4(0.0);
    _1_x = vec4(_2_unknown);
    _0_ok = _0_ok && _1_x == vec4(_2_unknown);
    _1_x = vec4(_2_unknown);
    _0_ok = _0_ok && _1_x == vec4(_2_unknown);
    _1_x = vec4(_2_unknown);
    _1_x += 1.0;
    _1_x -= 1.0;
    _0_ok = _0_ok && _1_x == vec4(_2_unknown);
    _1_x = vec4(_2_unknown);
    _1_x = _1_x + 1.0;
    _1_x = _1_x - 1.0;
    _0_ok = _0_ok && _1_x == vec4(_2_unknown);
    return _0_ok && test_int_b() ? colorGreen : colorRed;
}
