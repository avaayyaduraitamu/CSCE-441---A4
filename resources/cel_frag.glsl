#version 120

uniform vec3 ka;
uniform vec3 kd;
uniform vec3 ks;
uniform float s;
uniform vec3 lightPos[2];
uniform vec3 lightCol[2];

varying vec3 vPosEye;
varying vec3 vNorEye;

// Helper function to quantize a single channel into 5 levels
float quantize(float color) {
    if(color < 0.25)      return 0.0;
    else if(color < 0.5)  return 0.25;
    else if(color < 0.75) return 0.5;
    else if(color < 1.0)  return 0.75;
    else                  return 1.0;
}

void main() {
    vec3 n = normalize(vNorEye);
    vec3 v = normalize(-vPosEye);

    // 1. Silhouette Test (from Task 5)
    if (abs(dot(n, v)) < 0.3) {
        gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    // 2. Standard Blinn-Phong Calculation
    vec3 lighting = ka;
    for(int i = 0; i < 2; i++) {
        vec3 l = normalize(lightPos[i] - vPosEye);
        vec3 h = normalize(l + v);
        float nDotL = max(0.0, dot(n, l));
        float nDotH = max(0.0, dot(n, h));
        
        vec3 D = kd * nDotL;
        vec3 S = ks * pow(nDotH, s);
        lighting += lightCol[i] * (D + S);
    }

    // 3. Quantize each channel separately
    vec3 celColor;
    celColor.r = quantize(lighting.r);
    celColor.g = quantize(lighting.g);
    celColor.b = quantize(lighting.b);

    gl_FragColor = vec4(celColor, 1.0);
}