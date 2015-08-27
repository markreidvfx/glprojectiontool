#ifndef CONTOURRENDER_H
#define CONTOURRENDER_H

#include <vector>

class ContourRender
{
public:
    ContourRender();
    void create();
    void render_countour(unsigned int normal_texture_id,
                         unsigned int depth_texture_id);
    void resize(int width, int height);
    void read_contour(int &width, int &height, std::vector<float> &data);
    unsigned int m_texture_id;

    float line_width;

private:

    void load_shader();

    unsigned int m_framebuffer_id;
    unsigned int m_normal_buffer_loc;
    unsigned int m_depth_buffer_loc;

    unsigned int m_width_loc;
    unsigned int m_height_loc;
    unsigned int m_line_width_loc;

    unsigned int m_width;
    unsigned int m_height;

    unsigned int m_program_id;

    // card buffers
    unsigned int m_vertex_array_id;
    unsigned int m_vertex_buffer_id;
    unsigned int m_uv_buffer_id;
    unsigned int m_index_buffer_id;

    unsigned int m_index_size;

};

#endif // CONTOURRENDER_H
