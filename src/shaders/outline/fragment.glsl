#version 330 core

layout(location = 0)out vec4 out_color;
layout(location = 1)out vec4 out_normals;

in vec2 uv;
in vec3 normal;

uniform vec3 fill_color;

void main()
{
    //color = vec4(normal, 1.0);
    out_color = vec4(fill_color, 1.0);
    out_normals = vec4(normal, 1.0);
}
