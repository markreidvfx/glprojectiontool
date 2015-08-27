#version 330 core

layout(location = 0) in vec3 vertexPositionModel;
layout(location = 1) in vec2 uv_coord;
//layout(location = 2) in vec3 vertexNormalModel;

uniform mat4 viewport_matrix;

uniform float z_pos;

out vec2 uv;
out vec3 normal;

void main(){
    //gl_Position = viewport_matrix * vec4(vertexPositionModel.xy,-.9, 1);
    gl_Position = vec4(vertexPositionModel.xy, z_pos, 1.0);
    uv = uv_coord;

}
