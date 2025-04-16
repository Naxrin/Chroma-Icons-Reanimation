#version 100

precision mediump float;

varying vec2 TexCoords;

uniform sampler2D screen;
uniform vec2 screenSize;
uniform bool fast;
uniform bool first;
uniform float radius;

void main() {
    float scaledRadius = radius * screenSize.y * 0.5;
    vec2 texOffset = 1.0 / screenSize; // gets size of single texel

    vec3 result = texture2D(screen, TexCoords).rgb;
    if (fast) {
        scaledRadius *= radius * 10.0 / ((radius * 10.0 + 1.0) * (radius * 10.0 + 1.0) - 1.0);
        float weight = 1.0;
        float weightSum = weight;
        if (first) {
            for (int i = 1; float(i) < scaledRadius; i++) {
                weight -= 1.0 / scaledRadius;
                weightSum += weight * 2.0;
                result += texture2D(screen, TexCoords + vec2(texOffset.x * float(i), 0.0)).rgb * weight;
                result += texture2D(screen, TexCoords - vec2(texOffset.x * float(i), 0.0)).rgb * weight;
            }
        }
        else {
            for (int i = 1; float(i) < scaledRadius; i++) {
                weight -= 1.0 / scaledRadius;
                weightSum += weight * 2.0;
                result += texture2D(screen, TexCoords + vec2(0.0, texOffset.y * float(i))).rgb * weight;
                result += texture2D(screen, TexCoords - vec2(0.0, texOffset.y * float(i))).rgb * weight;
            }
        }
        result /= weightSum;
    }
    else {
        float firstWeight = 0.84089642 / pow(scaledRadius, 0.96);
        result *= firstWeight;
        float weightSum = firstWeight;
        if (first) {
            for (int i = 1; float(i) <= ceil(scaledRadius); i++) {
                float weight = firstWeight * exp(-float(i) * float(i) / (2.0 * scaledRadius));
                weightSum += weight * 2.0;
                result += texture2D(screen, TexCoords + vec2(texOffset.x * float(i), 0.0)).rgb * weight;
                result += texture2D(screen, TexCoords - vec2(texOffset.x * float(i), 0.0)).rgb * weight;
            }
        }
        else {
            for (int i = 1; float(i) <= ceil(scaledRadius); i++) {
                float weight = firstWeight * exp(-float(i) * float(i) / (2.0 * scaledRadius));
                weightSum += weight * 2.0;
                result += texture2D(screen, TexCoords + vec2(0.0, texOffset.y * float(i))).rgb * weight;
                result += texture2D(screen, TexCoords - vec2(0.0, texOffset.y * float(i))).rgb * weight;
            }
        }
        result /= weightSum;
    }

    gl_FragColor = vec4(result, 1.0);
}