vec3 rainbow(float t) {
    return vec3(sin(t), sin(t + 2.0), sin(t + 4.0)) * 0.5 + 0.5;
}
