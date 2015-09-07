#include "imageplane.h"

#include <GL/glew.h>
#include "shaders.h"

#include "imagereader.h"

#include <iostream>

static const GLfloat g_vertex_buffer_data[] = {
    -1.0, +1.0, 0.0,
    -1.0, -1.0, 0.0,
    +1.0, -1.0, 0.0,
    +1.0, +1.0, 0.0,
};

static const GLfloat g_uv_buffer_data[] = {

    0.0, 0.0,
    0.0, 1.0,
    1.0, 1.0,
    1.0, 0.0,
};

static const GLfloat g_normal_buffer_data[] = {
   0.0f, 0.0f, -1.0f,
   0.0f, 0.0f, -1.0f,
   0.0f, 0.0f, -1.0f,
   0.0f, 0.0f, -1.0f,
};

static unsigned int g_indices_data[] = {
    0, 1, 2,
    2, 3, 0
};

ImagePlane::ImagePlane() : m_z(0.9999f), m_alpha(1.0f)
{
    m_shader_loaded = false;
    m_geo_loaded = false;
    m_programId = 0;

}

ImagePlane::~ImagePlane()
{
    glDeleteBuffers(1, &vertexBufferId);
    glDeleteBuffers(1, &indexBufferId);
    glDeleteBuffers(1, &uvBufferId);
    glDeleteBuffers(1, &normalBufferId);
    glDeleteTextures(1, &m_textureId);
    glDeleteVertexArrays(1, &vertexArrayId);
}

void ImagePlane::create()
{
    glGenVertexArrays(1, &vertexArrayId);
    glGenBuffers(1, &vertexBufferId);
    glGenBuffers(1, &uvBufferId);
    glGenBuffers(1, &normalBufferId);
    glGenBuffers(1, &indexBufferId);
    glGenTextures(1, &m_textureId);

    created = true;
}

void ImagePlane::loadShader()
{
    build_shader_from_rc(":/shaders/imageplane/vertex.glsl",
                         ":/shaders/imageplane/fragment.glsl",
                         m_programId);

    m_shader_loaded = true;

    m_z_pos_loc = glGetUniformLocation(m_programId, "z_pos");
    m_alpha_loc = glGetUniformLocation(m_programId, "alpha");
    m_viewport_matrix_loc = glGetUniformLocation(m_programId, "viewport_matrix");

    m_texture_loc = glGetUniformLocation(m_programId, "image_sampler");


    std::cerr << "loaded imageplane shader\n";
}

void ImagePlane::setImageData(const int width, const int height,
                                 const std::vector<float> &image_data)
{

    glBindVertexArray(vertexArrayId);

    //glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8,
                 width, height, 0, GL_RGBA, GL_FLOAT,
                 &image_data[0]);

    glBindVertexArray(0);

}

void ImagePlane::update()
{

    if (m_geo_loaded)
        return;

    glBindVertexArray(vertexArrayId);

    if (!m_shader_loaded) {
        loadShader();
    }

    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
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

    glBindBuffer(GL_ARRAY_BUFFER, uvBufferId);
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

    glBindBuffer(GL_ARRAY_BUFFER, normalBufferId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_normal_buffer_data), g_normal_buffer_data, GL_STATIC_DRAW);

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
       2,                  // attribute 0 = position
       3,                  // size
       GL_FLOAT,           // type
       GL_FALSE,           // normalized?
       0,                  // stride
       (void*)0            // array buffer offset
    );

    indexSize = 6;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexSize * sizeof(unsigned int), &g_indices_data[0] , GL_STATIC_DRAW);

    glBindVertexArray(0);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    m_geo_loaded = true;
}

void ImagePlane::draw()
{

    glBindVertexArray(vertexArrayId);
    glUseProgram(m_programId);

    // set uniforms
    glUniform1fv(m_z_pos_loc, 1, &m_z);
    glUniform1fv(m_alpha_loc, 1, &m_alpha);
    glUniformMatrix4fv(m_viewport_matrix_loc, 1, GL_FALSE, &m_viewport_matrix[0][0]);
    //glUniform1i(m_texture_loc, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_textureId);

    // Draw the triangle !
    glDrawElements(
                   GL_TRIANGLES,      // mode
                   indexSize,         // count
                   GL_UNSIGNED_INT,   // type
                   (void*)0           // element array buffer offset
                   );
    glBindVertexArray(0);

}
