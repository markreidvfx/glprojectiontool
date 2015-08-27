#version 330 core

out vec4 color;

in vec2 uv;
in vec3 normal;

uniform sampler2D image_sampler;
uniform vec4 current_color;

uniform float alpha;

void main()
{
    //color = vec4(normal, 1.0);

    vec4 diffuse_color = texture(image_sampler, uv);
    color= vec4(diffuse_color.rgb, alpha);
    //color = vec4(1.0, 0.0, 0.0, 1.0);
}
