#version 330 core
out vec4 FragColor;

uniform vec3 diffuse_color;
uniform vec3 material_diffuse;

void main()
{
    FragColor = vec4(diffuse_color * material_diffuse, 1.0);
}