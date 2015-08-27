#version 330 core

layout(location = 0) in vec3 vertexPositionModel;
layout(location = 1) in vec2 uv_coord;
layout(location = 2) in vec3 vertexNormalModel;

uniform mat4 viewport_matrix;

uniform float z_pos;

out vec2 uv;
out vec3 normal;

void main(){
    //gl_Position = modelToViewMatrix * vec4(vertexPositionModel, 1);
    gl_Position = viewport_matrix * vec4(vertexPositionModel.xy, z_pos, 1.0);
    int width = 1920;
    int height = 1080;

    //gl_Position = vec4(vertexPositionModel.x,
    //                   vertexPositionModel.y / 2.0 * width/height,
    //                   vertexPositionModel.z, 1);
    uv = uv_coord;
    normal = vertexNormalModel;
}
