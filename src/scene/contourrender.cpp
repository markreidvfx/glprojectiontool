#include "contourrender.h"

#include <GL/glew.h>
#include "shaders.h"

#include <iostream>
#include <vector>


static const GLfloat g_vertex_buffer_data[] = {
    -1.0, +1.0, 0.0,
    -1.0, -1.0, 0.0,
    +1.0, -1.0, 0.0,
    +1.0, +1.0, 0.0,
};

static const GLfloat g_uv_buffer_data[] = {

    0.0, 1.0,
    0.0, 0.0,
    1.0, 0.0,
    1.0, 1.0,

};

static unsigned int g_indices_data[] = {
    0, 1, 2,
    2, 3, 0
};


ContourRender::ContourRender()
{
     m_width = 1920;
     m_height = 1080;
     line_width = 1;
}

void ContourRender::load_shader()
{
    build_shader_from_rc(":/shaders/contour/vertex.glsl",
                         ":/shaders/contour/fragment.glsl",
                         m_program_id);

    m_normal_buffer_loc = glGetUniformLocation(m_program_id, "c_normal_buffer");
    m_depth_buffer_loc = glGetUniformLocation(m_program_id, "c_depth_buffer");

    m_width_loc = glGetUniformLocation(m_program_id, "width");
    m_height_loc = glGetUniformLocation(m_program_id, "height");

    m_line_width_loc = glGetUniformLocation(m_program_id, "line_width");
}

void ContourRender::create()
{
    load_shader();
    glGenFramebuffers(1, &m_framebuffer_id);
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer_id);

    // main buffer
    glGenTextures(1, &m_texture_id);
    glBindTexture(GL_TEXTURE_2D, m_texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_FLOAT, 0);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_texture_id, 0);
    //glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depth_buffer_id, 0);

    GLenum DrawBuffers[] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, DrawBuffers);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
        std::cerr << "error creating contour frame buffer\n";
    }

    // Setup card

    glGenVertexArrays(1, &m_vertex_array_id);
    glGenBuffers(1, &m_vertex_buffer_id);
    glGenBuffers(1, &m_uv_buffer_id);
    glGenBuffers(1, &m_index_buffer_id);

    glBindVertexArray(m_vertex_array_id);

    //glActiveTexture(GL_TEXTURE1);
    //glBindTexture(GL_TEXTURE_2D, m_texture_id);

    glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
       0,                  // attribute 0 = position
       3,                  // size
       GL_FLOAT,           // type
       GL_FALSE,           // normalized?
       0,                  // stride
       (void*)0            // array buffer offset
    );

    glBindBuffer(GL_ARRAY_BUFFER, m_uv_buffer_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data), g_uv_buffer_data, GL_STATIC_DRAW);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
       1,                  // attribute 0 = position
       2,                  // size
       GL_FLOAT,           // type
       GL_FALSE,           // normalized?
       0,                  // stride
       (void*)0            // array buffer offset
    );

    m_index_size = 6;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_buffer_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_index_size * sizeof(unsigned int), &g_indices_data[0] , GL_STATIC_DRAW);

    glBindVertexArray(0);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

}

void ContourRender::resize(int width, int height)
{
    m_width = width;
    m_height = height;

    glBindTexture(GL_TEXTURE_2D, m_texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_FLOAT, 0);

}

void ContourRender::render_countour(unsigned int normal_texture_id,
                                    unsigned int depth_texture_id)
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer_id);
    glUseProgram(m_program_id);
    glViewport(0, 0, m_width, m_height);
    glDisable(GL_DEPTH_TEST); // no depth is need
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindVertexArray(m_vertex_array_id);

    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, normal_texture_id);
    glGenerateMipmap(GL_TEXTURE_2D);
    glUniform1i(m_normal_buffer_loc, 6);

    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, depth_texture_id);
    //glGenerateMipmap(GL_TEXTURE_2D);
    glUniform1i(m_depth_buffer_loc, 7);

    glUniform1i(m_width_loc, m_width);
    glUniform1i(m_height_loc, m_height);
    glUniform1f(m_line_width_loc, line_width);

    glDrawElements(GL_TRIANGLES, m_index_size, GL_UNSIGNED_INT, (void*)0 );

    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);


}

void ContourRender::read_contour(int &width, int &height, std::vector<float> &data)
{

    GLint internalFormat;
    glBindTexture(GL_TEXTURE_2D, m_texture_id);

    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPONENTS, &internalFormat); // get internal format type of GL texture
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width); // get width of GL texture
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height); // get height of GL texture

    std::cerr << width << "x" << height <<  "\n";

    data.resize(width * height * 4);

    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, &data[0]);
}

