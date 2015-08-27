#version 330 core

layout(location = 0) in vec3 vertexPositionModel;
layout(location = 1) in vec2 uv_coord;
layout(location = 2) in vec3 vertexNormalModel;

uniform mat4 modelToViewMatrix;
uniform vec3 camera_world;
uniform float normal_scale;

out vec2 uv;
out vec3 normal;

void main(){
    //vec3 new_pos = vertexPositionModel;

    float d = distance(vertexPositionModel, camera_world)/100.0;
    vec3 new_pos = vertexPositionModel + (vertexNormalModel * normal_scale);
    gl_Position = modelToViewMatrix * vec4(new_pos, 1);
    uv = uv_coord;
    normal = vertexNormalModel;
}
