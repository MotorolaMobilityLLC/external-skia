
out vec4 sk_FragColor;
uniform vec4 input;
uniform vec4 expected;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    return ((((((log2(input.x) == expected.x && log2(input.xy) == expected.xy) && log2(input.xyz) == expected.xyz) && log2(input) == expected) && 0.0 == expected.x) && vec2(0.0, 1.0) == expected.xy) && vec3(0.0, 1.0, 2.0) == expected.xyz) && vec4(0.0, 1.0, 2.0, 3.0) == expected ? colorGreen : colorRed;
}
