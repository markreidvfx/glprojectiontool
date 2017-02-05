#version 330 core

out vec4 color;

in vec2 uv;
in vec3 normal;

uniform sampler2D image_sampler;
uniform vec4 current_color;

uniform float alpha;

uniform int image_error;

void main()
{
    //color = vec4(normal, 1.0);
    vec4 diffuse_color = texture(image_sampler, uv);

    if (image_error == 1) {
        //color = vec4(1.0, 0.0, 0.0, 1.0);

        float x = 0.5*(uv.x+1.0); // range [0,1]
        float y = 0.5*(uv.y+1.0); // range [0,1]
        // checkerboard shader:

        float mn = 0.25;
        float mx = 0.5;

        float xstep = 16.0;
        float ystep = 9.0;

        if ((mod(xstep*x, 1.0) < 0.5) ^^ (mod(ystep*y, 1.0) < 0.5))
        {
            color = vec4(mn,mn,mn,1.0);
        }
        else
        {
            color =  vec4(mx,mx,mx,1.0);
        }

    } else {

        color= vec4(diffuse_color.rgb, alpha);
    }

}
