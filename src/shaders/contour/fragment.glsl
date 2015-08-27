#version 330 core
#define KERNEL_SIZE 9

out vec4 color;

in vec2 uv;
in vec3 normal;

uniform sampler2D c_normal_buffer;
uniform sampler2D c_depth_buffer;

uniform int width;
uniform int height;
uniform float line_width;

vec2 offset[KERNEL_SIZE];
float depth_samples[KERNEL_SIZE];
vec4 normal_samples[KERNEL_SIZE];

float g_depth[KERNEL_SIZE];
float g_normal[KERNEL_SIZE];

float g_depth_max, g_depth_min;
float g_normal_max, g_normal_min;

float depth_threshold = 0.0001;
float normal_threshold = 0.1;

void load_pixels(vec2 tex_coord)
{
    vec4 c;

    float near = 0.1;
    float far = 1000.0;

    float d;
    for (int i = 0; i < KERNEL_SIZE; i++)
    {
        // linearize depth
        d = texture(c_depth_buffer, tex_coord + offset[i]).r;
        d = (2 * near) / (far + near - d * (far - near));
        depth_samples[i] = d;

        normal_samples[i] = texture(c_normal_buffer, tex_coord + offset[i]);
    }
}

void find_range()
{
    g_depth_max = 0.0;
    g_depth_min = 1.0;
    g_normal_max = 0.0;
    g_normal_min = 1.0;

    for (int i = 0; i < KERNEL_SIZE; i++)
    {
        if (g_depth[i] > g_depth_max)
            g_depth_max = g_depth[i];
        if (g_depth[i] < g_depth_min)
            g_depth_min = g_depth[i];

        if (g_normal[i] > g_normal_max)
            g_normal_max = g_normal[i];
        if (g_normal[i] < g_normal_min)
            g_normal_min = g_normal[i];
    }
}

void caculate_offsets()
{
    float step_w = 1.0 / width  * line_width;
    float step_h = 1.0 / height * line_width;

    offset[0] = vec2(-step_w, -step_h);
    offset[1] = vec2(0.0,     -step_h);
    offset[2] = vec2(step_w,  -step_h);
    offset[3] = vec2(-step_w, 0.0);
    offset[4] = vec2(0.0,     0.0);
    offset[5] = vec2(step_w,  0.0);
    offset[6] = vec2(-step_w, step_h);
    offset[7] = vec2(0.0,     step_h);
    offset[8] = vec2(step_w,  step_h);

}

void caculate_contour()
{
    vec2 tex_coord;
    caculate_offsets();

    for (int i = 0; i < KERNEL_SIZE; i++) {
        tex_coord = uv + offset[i];
        load_pixels(tex_coord);

        g_depth[i] =  (1.0 / 8.0) * (abs(depth_samples[0] - depth_samples[4])
                      + 2.0 * abs(depth_samples[1] - depth_samples[4])
                      + abs(depth_samples[2] - depth_samples[4])
                      + 2.0 * abs(depth_samples[3] - depth_samples[4])
                      + 2.0 * abs(depth_samples[5] - depth_samples[4])
                      + abs(depth_samples[6] - depth_samples[4])
                      + 2.0 * abs(depth_samples[7] - depth_samples[4])
                          + abs(depth_samples[8] - depth_samples[4]));

        g_normal[i] = (1.0 / 8.0) * (length(normal_samples[0] - normal_samples[4])
                      + 2.0 * length(normal_samples[1] - normal_samples[4])
                      + length(normal_samples[2] - normal_samples[4])
                      + 2.0 * length(normal_samples[3] - normal_samples[4])
                      + 2.0 * length(normal_samples[5] - normal_samples[4])
                      + length(normal_samples[6] - normal_samples[4])
                      + 2.0 * length(normal_samples[7] - normal_samples[4])
                      + length(normal_samples[8] - normal_samples[4]));
    }
    find_range();
}

void main()
{
    caculate_contour();

    float d = (g_depth_max - g_depth_min) / depth_threshold;
    float n = (g_normal_max - g_normal_min) / normal_threshold;

    float edge_depth =  min(d * d, 1.0);
    float edge_normal = min(n * n, 1.0);

    color = vec4(0,0,0,0);

    if (edge_normal == 1.0 ) {
        color += vec4(0.0, 1.0, 1.0, 0.0);
        color.a = 1;
    }

    if (edge_depth == 1.0) {
        color += vec4(1.0, 0.0, 0.0, 0.0);
        color.a = 1;
    }

}
