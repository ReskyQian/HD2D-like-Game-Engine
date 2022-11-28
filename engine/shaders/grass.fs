#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D grass_texture;

void main()
{
    vec4 texColor = texture(grass_texture, TexCoords);
    if(texColor.a < 0.1)
        discard;
    FragColor = texColor;
}