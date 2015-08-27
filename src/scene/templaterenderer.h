#ifndef TEMPLATERENDERER_H
#define TEMPLATERENDERER_H

#include "mesh.h"
#include "camera.h"
#include "framebuffer.h"
#include "contourrender.h"
#include "imagereader.h"
#include <mutex>

class TemplateRenderer
{
public:
    TemplateRenderer();

    void create();
    void draw(std::vector< std::shared_ptr<Mesh> > objects,
              glm::mat4 &modelToProjectionMatrix,
              glm::mat4 &viewportMatrix,
              glm::vec2 &viewport_size,
              bool redraw_buffers = true);

    void set_template_texture(std::string path);
    void render_template(std::string image_plane, std::string dest);

    void resize_buffers(int width, int height)
    {
        m_framebuffer.resize(width, height);
        m_contour_render.resize(width, height);
        m_width = width;
        m_height = height;
    }

    int width(){return m_width;}
    int height(){return m_height;}
    float line_width(){return m_contour_render.line_width;}
    void set_line_width(float value) {m_contour_render.line_width = value;}

    int get_progress(){
        std::lock_guard<std::recursive_mutex> lock(m_lock);
        return m_progress;
    }
    void set_progress(int value) {
        std::lock_guard<std::recursive_mutex> lock(m_lock);
        m_progress = value;
    }

private:
    void load_shaders();
    void load_texture_data(const int width, const int height,
                           const std::vector<float> image_data);

    ImageReader m_texture_reader;
    FrameBuffer m_framebuffer;
    ContourRender m_contour_render;

    bool m_texture_loaded;
    int m_width;
    int m_height;

    unsigned int m_template_texture_id;

    unsigned int m_template_shader_id;
    unsigned int m_mvp_mat_loc;
    unsigned int m_tex_loc;
    unsigned int m_mode_loc;
    unsigned int m_normal_scale_loc;

    std::recursive_mutex m_lock;
    int m_progress;


};

#endif // TEMPLATERENDERER_H
