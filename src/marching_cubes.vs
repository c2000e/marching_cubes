#version 330 core

//	A simple (mostly) pass-through vertex shader.
//	Applies a model-view-projection matrix to each vertex.

layout (location = 0) in vec3 _position;
layout (location = 1) in vec3 _normal;

out vec3 position;
out vec3 normal;

uniform mat4 mvp;

void main()
{
	gl_Position =  mvp * vec4(_position, 1.0);

	position = _position;
	normal = _normal;
};