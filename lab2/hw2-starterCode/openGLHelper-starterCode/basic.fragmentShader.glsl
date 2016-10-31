#version 150

in vec4 col;
in vec3 pos;
in vec2 tex;

out vec4 c;

uniform sampler2D textureImage;
uniform samplerCube skyboxMap;

void main()
{
	vec4 cout;
	if(col.w != -1)
	{
		cout = texture(textureImage, tex);
	}
	else
	{
		cout = texture(skyboxMap, pos * vec3(1.03, -1.0, 1.0));
	}
	c = cout;
}

