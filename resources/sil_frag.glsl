#version 120

varying vec3 vPosEye;
varying vec3 vNorEye;

void main() {
    vec3 n = normalize(vNorEye);
    // The eye vector in eye space is simply the vector from the point to the origin (0,0,0)
    vec3 v = normalize(-vPosEye); 
    
    // Silhouette condition: dot(n, v) close to 0
    if (abs(dot(n, v)) < 0.3) {
        gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0); // Black
    } else {
        gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0); // White
    }
}