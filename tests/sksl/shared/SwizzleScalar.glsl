
out vec4 sk_FragColor;
uniform float unknownInput;
vec4 main() {
    float x = unknownInput;
    vec4 v = vec4(vec2(x), 0.0, 1.0);
    v = vec4(vec2(unknownInput), 0.0, 1.0);
    v = vec3(unknownInput, 0.0, 1.0).yxzy;
    v = vec3(vec2(unknownInput), 0.0).zxzy;
    return v;
}
