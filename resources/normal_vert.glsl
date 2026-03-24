#version 120

uniform mat4 P;
uniform mat4 MV;

attribute vec4 aPos; 
attribute vec3 aNor; 
attribute vec2 aTex; // Declared

varying vec3 color; 

void main()
{
	// Trick the compiler into keeping aTex without changing the position
	float dummy = aTex.x * 0.0;
	
	gl_Position = P * (MV * aPos) + dummy;
	color = 0.5 * aNor + vec3(0.5, 0.5, 0.5);
}
