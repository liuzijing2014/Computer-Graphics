#version 150

in vec3 tex;

out vec4 c;

uniform samplerCube skybox;

void main()
{
  c = texture(skybox, tex);
}

