#version 330 core

layout(triangles) in;
layout(triangle_strip, max_vertices=3) out;

in vec2 gs_uv[];
in vec3 gs_normal[];

out vec2 uv;
out vec3 normal;
out vec3 bary;

const vec3 BARY[3] = vec3[]( vec3( 1.0, 0.0, 0.0 ),
                             vec3( 0.0, 1.0, 0.0 ),
                             vec3( 0.0, 0.0, 1.0 )
                           );

void main()
{
    for(int i=0; i<3; i++) {
        gl_Position = gl_in[i].gl_Position;
        uv = gs_uv[i];
        normal = gs_normal[i];
        bary = BARY[i];
        EmitVertex();
    }
    EndPrimitive();
}
