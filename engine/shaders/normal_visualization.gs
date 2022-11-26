#version 330 core
layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

in VS_OUT {
    vec3 normal;
} gs_in[];

const float MAGNITUDE = 0.001;

layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

void GenerateLine(int index) 
{
    gl_Position = projection * gl_in[index].gl_Position;
    EmitVertex();
    gl_Position = projection * (gl_in[index].gl_Position + vec4(gs_in[index].normal, 0.0 ) * MAGNITUDE);
    EmitVertex();
    EndPrimitive();
}

void main() 
{
    GenerateLine(0);
    GenerateLine(1);
    GenerateLine(2);
    // for(int i = 0; i < 2; ++i) {
    //     GenerateLine(i);
    // }   
}