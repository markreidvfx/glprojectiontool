#version 330 core

layout(location = 0) in vec3 vertexPositionModel;
layout(location = 1) in vec2 uv_coord;
layout(location = 2) in vec3 vertexNormalModel;

uniform mat4 modelToViewMatrix;

out vec2 uv;
out vec3 normal;

void main(){
    gl_Position = modelToViewMatrix * vec4(vertexPositionModel, 1);
    uv = uv_coord;
    normal = vertexNormalModel;
}
