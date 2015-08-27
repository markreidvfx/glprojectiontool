#version 330 core

#define KERNEL_SIZE 9

out vec4 color;

in vec2 uv;
in vec3 normal;

uniform sampler2D rendered_texture;
uniform sampler2D rendered_texture_alpha;

uniform sampler2D normal_buffer;
uniform sampler2D depth_buffer;
uniform sampler2D c_buffer;

uniform int mode;
// 0 - texture
// 1 - normals
// 2 - contours

uniform vec4 current_color;

uniform float alpha;

uniform int image_width;
uniform int image_height;


int width = 1920;
int height = 1080;

float step_w = 1.0/width;
float step_h = 1.0/height;
vec2 offset[KERNEL_SIZE];

float depth[KERNEL_SIZE];
vec4 norm[KERNEL_SIZE];

float g_depth[KERNEL_SIZE];
float g_norm[KERNEL_SIZE];
float gdMax, gdMin;
float gnMax, gnMin;

float threshold_d = 0.000001;
float threshold_n = 0.2;

void loadPixels(vec2 texCoord)
{
        vec4 c;
        for (int i = 0; i < KERNEL_SIZE; i++)
        {
                depth[i] = texture(depth_buffer, texCoord + offset[i]).r;
                norm[i] = texture(normal_buffer, texCoord + offset[i]);
        }
}

void findMaxMin()
{
        gdMax = 0.0;
        gdMin = 1.0;
        gnMax = 0.0;
        gnMin = 1.0;

        for (int i = 0; i < KERNEL_SIZE; i++)
        {
                if (g_depth[i] > gdMax)
                        gdMax = g_depth[i];
                if (g_depth[i] < gdMin)
                        gdMin = g_depth[i];

                if (g_norm[i] > gnMax)
                        gnMax = g_norm[i];
                if (g_norm[i] < gnMin)
                        gnMin = g_norm[i];
        }
}

void main()
{
    float limit= 0.2;
    float step = 1/image_width;
    color = vec4(0,0,0,0);

    vec4 z = texture(c_buffer, uv);

    if (mode == 0) {
        vec4 diffuse_color = texture(rendered_texture, uv);
        vec4 alpha = texture(rendered_texture_alpha, uv);
        //alpha += alpha + alpha;
        color = vec4(diffuse_color.rgb / alpha.r, alpha.r);

        //color = diffuse_color;
    } else if ( mode == 1 ){
        vec4 n = texture(normal_buffer, uv);
        //vec3 r = sobel(step, uv, rendered_texture, limit);
        n.a *= .4;
        color = n;
        //coror.a = .5;

    } else if ( mode == 3 ) {
        color = texture(c_buffer, uv);
        //color = vec4(0,0,0,0);
    } else if ( mode == 2 ) {
        vec2 tCoord;
        int i;

        offset[0] = vec2(-step_w, -step_h);
        offset[1] = vec2(0.0, -step_h);
        offset[2] = vec2(step_w, -step_h);
        offset[3] = vec2(-step_w, 0.0);
        offset[4] = vec2(0.0, 0.0);
        offset[5] = vec2(step_w, 0.0);
        offset[6] = vec2(-step_w, step_h);
        offset[7] = vec2(0.0, step_h);
        offset[8] = vec2(step_w, step_h);

        for (int a = 0; a < KERNEL_SIZE; a++) {
            tCoord = uv + offset[a];
            loadPixels(tCoord);

            g_depth[a] = (1.0/8.0)*(abs(depth[0]-depth[4])
                              + 2.0*abs(depth[1]-depth[4])
                              + abs(depth[2]-depth[4])
                              + 2.0*abs(depth[3]-depth[4])
                              + 2.0*abs(depth[5]-depth[4])
                              + abs(depth[6]-depth[4])
                              + 2.0*abs(depth[7]-depth[4])
                              + abs(depth[8]-depth[4]));
            g_norm[a] = (1.0/8.0)*(length(norm[0]-norm[4])
                              + 2.0*length(norm[1]-norm[4])
                              + length(norm[2]-norm[4])
                              + 2.0*length(norm[3]-norm[4])
                              + 2.0*length(norm[5]-norm[4])
                              + length(norm[6]-norm[4])
                              + 2.0*length(norm[7]-norm[4])
                              + length(norm[8]-norm[4]));

        }

        findMaxMin();
        float td = (gdMax-gdMin)/threshold_d;
        float tn = (gnMax-gnMin)/threshold_n;

        float edge_d = min(td*td, 1.0);
        float edge_n = min(tn*tn, 1.0);

        color = vec4(0,0,0,0);

        if (edge_n == 1.0 ) {
            color += vec4(0,1,0,1);
            color.a = 1.0;
        }

        if (edge_d == 1.0) {
            color += vec4(1,0,0,1);
            color.a = 1.0;
        }




    }

}
