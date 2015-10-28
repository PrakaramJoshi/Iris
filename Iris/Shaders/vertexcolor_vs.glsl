#version 440 core
layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_colour;

// Matrices we'll need
layout (std140) uniform constants
{
   mat4 mvp;
};

// Inputs from vertex shader
out vec3 colour;


void main () {

    gl_Position = mvp* vec4(vertex_position,1.0);
	
	colour = vertex_colour;
}