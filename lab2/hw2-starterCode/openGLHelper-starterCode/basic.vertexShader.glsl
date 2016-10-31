#version 150

in vec3 position;
in vec4 color;
in vec2 texcoord;

out vec4 col;
out vec3 pos;
out vec2 tex;

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;

void main()
{
    gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0f);
    col = color;
    pos = position;
    tex = texcoord;
}

