#version 330 core
out vec4 FragColor;

uniform vec3 diffuse_color;

void main()
{
    FragColor = vec4(diffuse_color, 1.0);
}