#include "templaterenderer.h"

#include <GL/glew.h>
#include "shaders.h"
#include <iostream>
#include <list>

#include <Magick++.h>

TemplateRenderer::TemplateRenderer()
{
    m_texture_loaded = false;
    m_width = 1920;
    m_height = 1080;

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
    //m_texture_reader.read("/Users/mreid/Dev/ProjectionWidget/guides.tif", 0);
    m_framebuffer.create();
    m_contour_render.create();

    load_shaders();

    glGenTextures(1, &m_template_texture_id);
}

void TemplateRenderer::set_template_texture(const std::vector<float> &image_data, const int width, const int height)
{

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
    //if (!m_texture_loaded && m_texture_reader.dataReady(t)) {
    //    load_texture_data(m_texture_reader.width(),
    //                      m_texture_reader.height(),
    //                      m_texture_reader.image_data);
    //    std::cerr << "template texture loaded\n";
    //    redraw_offscreen_buffers = true;

    //}

    //if (!m_texture_loaded)
    //   return;

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
static void prep_image(Magick::Image &image)
{
    image.modifyImage();
    Magick::PixelPacket *pixels = image.getPixels(0, 0, image.size().width(), 1);

    for (int i = 0; i < image.size().width(); i++ ) {
        float alpha = .01 * 1.0 / (i+1);
        Magick::Color c = pixels[i];
        c.alpha(1.0 - alpha + c.alpha());
        pixels[i] = c;
    }
    image.syncPixels();

}

void TemplateRenderer::render_template_data(FloatImageData &color_data,
                                            FloatImageData &alpha_data,
                                            FloatImageData &contour_data)
{

    m_framebuffer.read_texture(Color, color_data.width, color_data.height, color_data.data);
    m_framebuffer.read_texture(Alpha, alpha_data.width, alpha_data.height, alpha_data.data);
    m_contour_render.read_contour(contour_data.width, contour_data.height, contour_data.data);
}

void TemplateRenderer::render_template(std::string image_plane, std::string dest)
{
    std::vector<float> color_data;
    std::vector<float> alpha_data;
    std::vector<float> contour_data;

    Magick::Image plane;
    Magick::Image color;
    Magick::Image alpha;
    Magick::Image contour;

    Magick::Geometry geo("1920x1080!");

    int width,height;

    set_progress(20);

    color.read(width, height, "RGBA", Magick::FloatPixel, &color_data[0]);

    set_progress(30);
    m_framebuffer.read_texture(Alpha, width, height, alpha_data);
    alpha.read(width, height, "RGBA", Magick::FloatPixel, &alpha_data[0]);
    alpha.channel(Magick::RedChannel);

    color.composite(alpha, 0, 0, Magick::CopyOpacityCompositeOp);
    color.attribute("label", "guides");
    color.resize(geo);
    color.flip();

    set_progress(40);
    m_contour_render.read_contour(width, height, contour_data);
    contour.read(width, height, "RGBA", Magick::FloatPixel, &contour_data[0]);
    contour.flip();
    contour.attribute("label", "contour");
    contour.resize(geo);

    set_progress(50);
    Magick::Image empty(geo,"rgba(0,0,0,0.0)" );
    empty.type(Magick::TrueColorMatteType);
    empty.attribute("label", "clones");

    prep_image(empty);
    prep_image(color);
    prep_image(contour);

    set_progress(60);
    plane.read(image_plane);

    set_progress(70);

    /*
    plane.write("plane.png");
    alpha.write("alpha.png");
    color.write("color.png");
    contour.write("contour.png");
    empty.write("empty.png");*/

    set_progress(80);

    //std::cerr << plane.attribute("colorspace") << "\n";
    plane.attribute("colorspace", "rgb");
    plane.magick("PSD");
    plane.depth(16);
    plane.verbose(true);
    plane.compressType(Magick::RLECompression);
    plane.attribute("label", "background");

    Magick::Image liquify = plane;
    liquify.attribute("label", "liquify");

    std::list<Magick::Image> images;
    images.push_back(plane);
    images.push_back(plane);
    images.push_back(liquify);
    images.push_back(empty);
    images.push_back(color);
    images.push_back(contour);

    set_progress(90);

    Magick::writeImages(images.begin(), images.end(), "out.psd");


}
