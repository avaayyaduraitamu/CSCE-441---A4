#version 120

varying vec3 color; // passed from the vertex shader

void main()
{
	gl_FragColor = vec4(color.r, color.g, color.b, 1.0);
}
