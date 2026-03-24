#version 120

uniform vec3 ka;
uniform vec3 kd;
uniform vec3 ks;
uniform float s;

uniform vec3 lightPos; 
uniform vec3 lightCol;

// Texture Uniforms
uniform sampler2D texSampler;
uniform int useTexture; // Using int (0 or 1) is more stable in GLSL 120

varying vec3 vPosEye;
varying vec3 vNorEye;
varying vec2 vTex; // Received from vertex shader

void main() {
    // 1. Setup Vectors
    vec3 n = normalize(vNorEye);
    vec3 v = normalize(-vPosEye); 
    vec3 l = normalize(lightPos - vPosEye);
    vec3 h = normalize(l + v);

    // 2. Handle Texture
    vec3 textureRGB = vec3(1.0, 1.0, 1.0);
    if (useTexture == 1) {
        // In version 120, use texture2D instead of texture
        textureRGB = texture2D(texSampler, vTex).rgb;
    }

    // 3. Ambient
    // We multiply ambient by texture so the ground isn't glowing bright green in shadows
    vec3 ambient = ka * textureRGB;

    // 4. Diffuse
    float nDotL = max(0.0, dot(n, l));
    // Multiply kd by textureRGB to "paint" the light with the texture color
    vec3 diffuse = kd * textureRGB * nDotL;

    // 5. Specular
    float specPower = 0.0;
    if (nDotL > 0.0) {
        specPower = pow(max(0.0, dot(n, h)), s);
    }
    vec3 specular = ks * specPower;

    // 6. Final Mix
    vec3 finalColor = ambient + lightCol * (diffuse + specular);

    gl_FragColor = vec4(finalColor, 1.0);
}