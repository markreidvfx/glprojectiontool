#include "triangle.h"
#include <GL/glew.h>

static const GLfloat g_vertex_buffer_data[] = {
   -1.0f, -1.0f, 0.0f,
   1.0f, -1.0f, 0.0f,
   0.0f,  1.0f, 0.0f,
};

static const GLfloat g_uv_buffer_data[] = {
    -1.0f, -1.0,
    1.0f, -1.0f,
    0.0f,  1.0f,
};

static const GLfloat g_normal_buffer_data[] = {
   0.0f, 0.0f, -1.0f,
   0.0f, 0.0f, -1.0f,
   0.0f, 0.0f, -1.0f,
};

static unsigned int g_indices_data[] = {
    0, 1, 2
};

Triangle::Triangle()
{
}

Triangle::~Triangle()
{
    glDeleteBuffers(1, &vertexBufferId);
    glDeleteBuffers(1, &indexBufferId);
    glDeleteBuffers(1, &uvBufferId);
    glDeleteBuffers(1, &normalBufferId);

    glDeleteVertexArrays(1, &vertexArrayId);
}

void Triangle::create()
{
    glGenVertexArrays(1, &vertexArrayId);
    glGenBuffers(1, &vertexBufferId);
    glGenBuffers(1, &uvBufferId);
    glGenBuffers(1, &normalBufferId);
    glGenBuffers(1, &indexBufferId);
    created = true;
}

void Triangle::update(double time)
{
    glBindVertexArray(vertexArrayId);

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


    indexSize = 3;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexSize * sizeof(unsigned int), &g_indices_data[0] , GL_STATIC_DRAW);

    glBindVertexArray(0);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
}

void Triangle::draw()
{
    glBindVertexArray(vertexArrayId);
    // Draw the triangle !
    glDrawElements(
                   GL_TRIANGLES,      // mode
                   indexSize,         // count
                   GL_UNSIGNED_INT,   // type
                   (void*)0           // element array buffer offset
                   );
    glBindVertexArray(0);
}
