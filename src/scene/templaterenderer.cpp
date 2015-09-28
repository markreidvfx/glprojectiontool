#include "templaterenderer.h"

#include <GL/glew.h>
#include "shaders.h"
#include <iostream>
#include <list>

TemplateRenderer::TemplateRenderer()
{
    m_texture_loaded = false;
    m_width = 1920;
    m_height = 1080;
    m_template_texture_id = 0;
    m_created = false;
}

void TemplateRenderer::load_shaders()
{
    /*build_shader_from_rc(":/shaders/template/vertex.glsl",
                           ":/shaders/template/fragment.glsl",
                            m_template_shader_id);*/

    build_shader_from_rc(":/shaders/template/vertex.glsl",
                         ":/shaders/template/geometry.glsl",
                         ":/shaders/template/fragment.glsl",
                             m_template_shader_id);

    m_mvp_mat_loc = glGetUniformLocation(m_template_shader_id, "modelToViewMatrix");
    m_tex_loc = glGetUniformLocation(m_template_shader_id, "surface_texture");
    m_mode_loc = glGetUniformLocation(m_template_shader_id, "mode");
    m_normal_scale_loc = glGetUniformLocation(m_template_shader_id, "normal_scale");
}


void TemplateRenderer::create()
{
    if (m_created)
        return;

    load_shaders();
    m_framebuffer.create();
    m_contour_render.create();
    glGenTextures(1, &m_template_texture_id);

    m_created = true;
}

void TemplateRenderer::set_template_texture(const std::vector<float> &image_data, const int width, const int height)
{
    if (!m_created)
        create();

    glBindTexture(GL_TEXTURE_2D, m_template_texture_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
                 width, height, 0, GL_RGBA, GL_FLOAT,
                 &image_data[0]);
    glGenerateMipmap(GL_TEXTURE_2D);

     m_texture_loaded = true;
    //glBindVertexArray(0);

}

void TemplateRenderer::draw(std::vector< std::shared_ptr<Mesh> > objects,
                            glm::mat4 &modelToProjectionMatrix,
                            glm::mat4 &viewportMatrix,
                            glm::vec2 &viewport_size,
                            bool redraw_offscreen_buffers,
                            unsigned int default_framebuffer_id)

{
    double t;

    if (!objects.size())
        return;

    //if (!m_texture_loaded)
    //   return;

    m_framebuffer.texture_loaded = m_texture_loaded;

    if (redraw_offscreen_buffers) {
        m_framebuffer.bind();

        glUseProgram(m_template_shader_id);
        glUniformMatrix4fv(m_mvp_mat_loc, 1, GL_FALSE, &modelToProjectionMatrix[0][0]);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_template_texture_id);
        glUniform1i(m_tex_loc, 1);
        glUniform1i(m_mode_loc, 0);
        glUniform1f(m_normal_scale_loc, 0.0);

        glDisable(GL_CULL_FACE);
        glCullFace(GL_FRONT);

        for (int i = 0; i < objects.size(); i++) {
            objects[i]->draw();
        }

        glUniform1f(m_normal_scale_loc, 0.05);
        glUniform1i(m_mode_loc, 1);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        for (int i = 0; i < objects.size(); i++) {
            //objects[i]->draw();
        }

        m_contour_render.render_countour(m_framebuffer.m_normal_buffer_id,
                                         m_framebuffer.m_depth_buffer_id);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, default_framebuffer_id);
    glViewport(0,0, viewport_size.x, viewport_size.y);

    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    m_framebuffer.setViewportMatrix(viewportMatrix);
    m_framebuffer.draw(m_contour_render.m_texture_id);
}

void TemplateRenderer::render_template_data(FloatImageData &color_data,
                                            FloatImageData &alpha_data,
                                            FloatImageData &contour_data)
{

    m_framebuffer.read_texture(Color, color_data.width, color_data.height, color_data.data);
    m_framebuffer.read_texture(Alpha, alpha_data.width, alpha_data.height, alpha_data.data);
    m_contour_render.read_contour(contour_data.width, contour_data.height, contour_data.data);
}
