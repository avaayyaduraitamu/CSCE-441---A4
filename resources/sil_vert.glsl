#version 120

uniform mat4 P;
uniform mat4 MV;
uniform mat4 MVit;

attribute vec4 aPos;
attribute vec3 aNor;

varying vec3 vPosEye;
varying vec3 vNorEye;

void main() {
    vPosEye = vec3(MV * aPos);
    vNorEye = vec3(MVit * vec4(aNor, 0.0));
    gl_Position = P * MV * aPos;
}