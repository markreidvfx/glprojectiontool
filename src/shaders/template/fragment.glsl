#version 330 core

layout(location = 0)out vec4 out_color;
layout(location = 1)out vec4 out_alpha;
layout(location = 2)out vec4 out_normals;
layout(location = 3)out vec4 out_contours;

uniform sampler2D surface_texture;
uniform vec3 fill_color;
uniform int mode;

bool wireframe = false;

in vec2 uv;
in vec3 normal;
in vec3 bary;


void main()
{
    //color = vec4(normal, 1.0);
    //out_color = vec4(fill_color, 1.0);

    vec4 tex = texture(surface_texture, vec2(uv.x, 1-uv.y));

    out_color = vec4(tex.rgb, 1.0);
    out_contours = vec4(1,0,0,1);

    //out_color = vec4(tex.a, tex.a, tex.a, 1.0);


    if (mode == 1) {
        out_color = vec4(1.0, 1.0, 1.0, 1.0);
        out_alpha = out_color;


    } else {
        out_normals = vec4(normal, 1.0);
        out_alpha = vec4(tex.a, tex.a, tex.a, 1.0);
        //out_alpha = vec4(1.0, 1.0, 1.0, 1.0);
        //out_color = vec4(bary, 1.0);
        if (wireframe) {
            if (any(lessThan(bary, vec3(0.01)))){
                out_color  = vec4(1.0, 1.0, 1.0, 1.0);
                out_alpha = vec4(1.0, 1.0, 1.0, 1.0);
            }
        }
    }

}
