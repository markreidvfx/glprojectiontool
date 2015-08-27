#version 330 core

layout(location = 0) in vec3 vertexPositionModel;
layout(location = 1) in vec2 uv_coord;
layout(location = 2) in vec3 vertexNormalModel;

uniform mat4 modelToViewMatrix;
uniform float normal_scale;

out vec2 gs_uv;
out vec3 gs_normal;

void main(){

    vec3 new_pos = vertexPositionModel + (vertexNormalModel * normal_scale);
    gl_Position = modelToViewMatrix * vec4(new_pos, 1);
    gs_uv = uv_coord;
    gs_normal = vertexNormalModel;
}
