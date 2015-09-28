#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <glm/glm.hpp>
#include <vector>

enum FrameBufferChannel {
    Color,
    Alpha,
    Normal,
    Depth
};

class FrameBuffer
{
public:
    FrameBuffer();
    ~FrameBuffer();
    void create();
    void bind();
    void draw(unsigned int contour_texture_id);

    void loadShader();
    void setViewportMatrix(glm::mat4 &m) {m_viewport_matrix = m;}

    void read_texture(FrameBufferChannel chan,
                      int &width, int &height, std::vector<float> &data);

    void resize(int width, int height);

    unsigned int m_texture_id;
    unsigned int m_texture_alpha_id;
    unsigned int m_depth_buffer_id;
    unsigned int m_normal_buffer_id;

    bool texture_loaded;

private:

    unsigned int m_framebuffer_id;

    unsigned int m_width;
    unsigned int m_height;

    glm::mat4 m_viewport_matrix;
    unsigned int m_program_id;
    unsigned int m_contour_program_id;

    // glsl uniforms
    unsigned int m_texture_loc;
    unsigned int m_texture_alpha_loc;
    unsigned int m_normal_buffer_loc;
    unsigned int m_contour_buffer_loc;
    unsigned int m_depth_buffer_loc;

    unsigned int m_mode_loc;
    unsigned int m_z_pos_loc;

    unsigned int m_viewport_matrix_loc;
    unsigned int m_width_loc;
    unsigned int m_height_loc;

    // card buffers
    unsigned int m_vertex_array_id;
    unsigned int m_vertex_buffer_id;
    unsigned int m_uv_buffer_id;
    unsigned int m_index_buffer_id;

    unsigned int m_index_size;


};

#endif // FRAMEBUFFER_H
