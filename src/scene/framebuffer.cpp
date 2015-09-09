#include "framebuffer.h"

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


FrameBuffer::FrameBuffer()
{
    m_framebuffer_id = 0;
    m_texture_id = 0;
    m_depth_buffer_id = 0;
    m_normal_buffer_id = 0;

    m_width = 1024;
    m_height = 1024;

}

FrameBuffer::~FrameBuffer()
{

}

void FrameBuffer::loadShader()
{
    build_shader_from_rc(":/shaders/framebuffer/vertex.glsl",
                         ":/shaders/framebuffer/fragment.glsl",
                         m_program_id);

    m_texture_loc = glGetUniformLocation(m_program_id, "rendered_texture");
    m_texture_alpha_loc = glGetUniformLocation(m_program_id, "rendered_texture_alpha");

    m_normal_buffer_loc = glGetUniformLocation(m_program_id, "normal_buffer");
    m_contour_buffer_loc = glGetUniformLocation(m_program_id, "c_buffer");
    m_depth_buffer_loc = glGetUniformLocation(m_program_id, "depth_buffer");

    m_mode_loc = glGetUniformLocation(m_program_id, "mode");
    m_z_pos_loc = glGetUniformLocation(m_program_id, "z_pos");

    m_viewport_matrix_loc = glGetUniformLocation(m_program_id, "viewport_matrix");

    m_width_loc = glGetUniformLocation(m_program_id, "image_width");
    m_height_loc = glGetUniformLocation(m_program_id, "image_height");

    std::cerr << "loaded framebuffer shader\n";
}

void FrameBuffer::create()
{

    loadShader();

    glGenFramebuffers(1, &m_framebuffer_id);
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer_id);

    // main buffer
    glGenTextures(1, &m_texture_id);
    glBindTexture(GL_TEXTURE_2D, m_texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, 0);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // texture alpha

    glGenTextures(1, &m_texture_alpha_id);
    glBindTexture(GL_TEXTURE_2D, m_texture_alpha_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, 0);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // normal buffer
    glGenTextures(1, &m_normal_buffer_id);
    glBindTexture(GL_TEXTURE_2D, m_normal_buffer_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, 0);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // depth buffer
    glGenTextures(1, &m_depth_buffer_id);
    glBindTexture(GL_TEXTURE_2D, m_depth_buffer_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_texture_id, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, m_texture_alpha_id, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, m_normal_buffer_id, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depth_buffer_id, 0);

    GLenum DrawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, DrawBuffers);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
        std::cerr << "error creating frame buffer\n";
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
    glBindVertexArray(0);

}

void FrameBuffer::resize(int width, int height)
{
    unsigned int texture_ids[] = {m_texture_id,
                                  m_texture_alpha_id,
                                  m_normal_buffer_id};
    m_width = width;
    m_height = height;

    for (int i = 0; i < 4; i++) {
        unsigned int tex_id = texture_ids[i];
        glBindTexture(GL_TEXTURE_2D, tex_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, 0);

    }
    glBindTexture(GL_TEXTURE_2D, m_depth_buffer_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

}

void FrameBuffer::bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer_id);
    glViewport(0, 0, m_width, m_height);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

}

void FrameBuffer::draw(unsigned int contour_texture_id)
{
    glUseProgram(m_program_id);

    glBindVertexArray(m_vertex_array_id);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_texture_id);
    glGenerateMipmap(GL_TEXTURE_2D);
    glUniform1i(m_texture_loc, 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_texture_alpha_id);
    glGenerateMipmap(GL_TEXTURE_2D);
    glUniform1i(m_texture_alpha_id, 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, m_normal_buffer_id);
    glGenerateMipmap(GL_TEXTURE_2D);
    glUniform1i(m_normal_buffer_loc, 3);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, contour_texture_id);
    glGenerateMipmap(GL_TEXTURE_2D);
    glUniform1i(m_contour_buffer_loc, 4);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, m_depth_buffer_id);
    glUniform1i(m_depth_buffer_loc, 5);

    glUniform1i(m_width_loc, m_width);
    glUniform1i(m_height_loc, m_height);

    glUniformMatrix4fv(m_viewport_matrix_loc, 1, GL_FALSE, &m_viewport_matrix[0][0]);
    glBindVertexArray(m_vertex_array_id);

    float p = 0.0;

    for (int i =0; i < 3; i++) {
        p -= .002;
        glUniform1f(m_z_pos_loc, p);
        glUniform1i(m_mode_loc, 0);
        glDrawElements(GL_TRIANGLES, m_index_size, GL_UNSIGNED_INT, (void*)0 );
    }

    p -= .002;
    glUniform1f(m_z_pos_loc, p);
    glUniform1i(m_mode_loc, 3);
    glDrawElements(GL_TRIANGLES, m_index_size, GL_UNSIGNED_INT, (void*)0 );

    glBindVertexArray(0);

}

void FrameBuffer::read_texture(FrameBufferChannel chan,
                               int &width, int &height, std::vector<float> &data)
{
    unsigned int texture_id = 0;
    switch (chan) {
    case Color:
         texture_id = m_texture_id;
        break;
    case Alpha:
        texture_id = m_texture_alpha_id;
        break;
    case Normal:
        texture_id = m_normal_buffer_id;
        break;
    case Depth:
        texture_id = m_depth_buffer_id;
        break;
    default:
        texture_id = m_texture_id;
        break;
    }

    GLint internalFormat;
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPONENTS, &internalFormat); // get internal format type of GL texture
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width); // get width of GL texture
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height); // get height of GL texture

    std::cerr << width << "x" << height <<  "\n";

    data.resize(width * height * 4);

    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, &data[0]);
}
