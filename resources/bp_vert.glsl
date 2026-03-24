#version 120

uniform mat4 P;
uniform mat4 MV;
uniform mat4 MVit;

attribute vec4 aPos;
attribute vec3 aNor;
attribute vec2 aTex; // Input from C++

varying vec3 vPosEye; 
varying vec3 vNorEye; 
varying vec2 vTex;    // Output to Fragment Shader

void main() {
    // 1. Position and Normal in Eye Space
    vPosEye = vec3(MV * aPos);
    vNorEye = vec3(MVit * vec4(aNor, 0.0));
    
    // 2. Pass the texture coordinates through
    vTex = aTex; 

    // 3. Final clip-space position
    gl_Position = P * MV * aPos;
}




