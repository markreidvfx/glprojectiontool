#include "shaders.h"
#include <GL/glew.h>

#include <string>

#include "rc_helper.h"
#include <iostream>
#include <vector>


bool compile_shader(const char *shader_code,
                    GLenum shader_type,
                    GLuint &shaderId)
{
    shaderId = glCreateShader(shader_type);
    glShaderSource(shaderId, 1, &shader_code, NULL);
    glCompileShader(shaderId);

    // Check if compiled properly
    GLint result = GL_FALSE;
    int log_length;

    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &result);
    glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &log_length);
    if (log_length > 0) {
        std::vector<char> error_message(log_length+1);
        glGetShaderInfoLog(shaderId, log_length, NULL, &error_message[0]);
        fprintf(stderr, "%s\n", &error_message[0]);
    }

    if(result != GL_TRUE)
        return false;

    return true;
}

bool build_shader_from_rc(const char *vertex_shader_path,
                          const char *fragment_shader_path,
                          unsigned int &programId)
{
    return build_shader_from_rc(vertex_shader_path, NULL, fragment_shader_path, programId);
}

bool build_shader_from_rc(const char *vertex_shader_path,
                          const char *geometry_shader_path,
                          const char *fragment_shader_path,
                          unsigned int &programId)
{
    GLuint vertexShaderId;
    GLuint geometryShaderId;
    GLuint fragmentShaderId;

    std::string vertex_shader_code;
    std::string geometry_shader_code;
    std::string fragment_shader_code;

    GLuint tmp_programId;

    GLint result = GL_FALSE;
    int log_length;

    if (!read_rc_data(vertex_shader_path, vertex_shader_code)) {
        std::cerr << "unable to read vertex shader code\n";
        goto fail;
    }

    if (geometry_shader_path) {
        if (!read_rc_data(geometry_shader_path, geometry_shader_code) ){
            std::cerr << "unable to read geometry shader code\n";
            goto fail;
        }

    }

    if (!read_rc_data(fragment_shader_path, fragment_shader_code)) {
        std::cerr << "unable to read fragment shader code\n";
        goto fail;
    }

    if(!compile_shader(vertex_shader_code.c_str(), GL_VERTEX_SHADER, vertexShaderId)) {
        std::cerr << "unable to compile vertex shader code\n";
        goto fail;
    }

    if (geometry_shader_path) {
        if(!compile_shader(geometry_shader_code.c_str(), GL_GEOMETRY_SHADER, geometryShaderId)) {
            std::cerr << "unable to compile  geometry shader code\n";
            goto fail;
        }

    }

    if(!compile_shader(fragment_shader_code.c_str(), GL_FRAGMENT_SHADER, fragmentShaderId)) {
        std::cerr << "unable to compile fragment shader code\n";
        goto fail;
    }

    tmp_programId = glCreateProgram();

    glAttachShader(tmp_programId, vertexShaderId);
    glAttachShader(tmp_programId, fragmentShaderId);

    if (geometry_shader_path)
        glAttachShader(tmp_programId, geometryShaderId);

    glLinkProgram(tmp_programId);

    // Check the program
    glGetProgramiv(tmp_programId, GL_LINK_STATUS, &result);
    glGetProgramiv(tmp_programId, GL_INFO_LOG_LENGTH, &log_length);
    if ( log_length > 0 ) {
       std::vector<char> error_message(log_length+1);
       glGetProgramInfoLog(tmp_programId, log_length, NULL, &error_message[0]);
       printf("%s\n", &error_message[0]);
    }

    if(result != GL_TRUE)
        goto fail;

    glDeleteShader(vertexShaderId);
    glDeleteShader(fragmentShaderId);
    programId = tmp_programId;
    return true;

fail:
    std::cerr << "failed to build shader\n";
    glDeleteShader(vertexShaderId);
    glDeleteShader(fragmentShaderId);
    glDeleteProgram(tmp_programId);
    return false;
}
