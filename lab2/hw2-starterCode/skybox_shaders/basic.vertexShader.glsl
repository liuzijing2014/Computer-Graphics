#version 150

in vec3 position;

out vec3 tex;

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;

void main()
{

    gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0f);
    tex = position;
}

