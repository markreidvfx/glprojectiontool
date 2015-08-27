#version 330 core

layout(location = 0)out vec4 color;

in vec2 uv;
in vec3 normal;

void main()
{
    //color = vec4(uv, 0,1);
    color = vec4(normal, 1.0);
    //color = vec4(0,0,0, 1);
}
