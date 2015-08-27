#ifndef SHADERS_H
#define SHADERS_H


bool build_shader_from_rc(const char *vertex_shader_path,
                          const char *fragment_shader_path,
                          unsigned int &programId);

bool build_shader_from_rc(const char *vertex_shader_path,
                          const char *geometry_shader_path,
                          const char *fragment_shader_path,
                          unsigned int &programId);

#endif // SHADERS_H
