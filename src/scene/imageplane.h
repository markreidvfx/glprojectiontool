#ifndef IMAGEPLANE_H
#define IMAGEPLANE_H

#include "mesh.h"
#include "imagereader.h"
#include <glm/glm.hpp>

class ImagePlane : public Mesh
{
public:
    ImagePlane();
    virtual ~ImagePlane();
    void create();
    void update();
    void calculate(double time,  int subdivision_level = 0) {}
    void read_data(MeshData &data, double time) {}
    void setImageData(const int width, const int height,
                         const std::vector<float> &image_data);
    void draw();

    void setZ(float value){m_z = value;}
    void setAlpha(float value =1.0) {m_alpha = value;}
    void setViewportMatrix(glm::mat4 &m) {m_viewport_matrix = m;}

private:
    void loadShader();
    bool m_shader_loaded;
    bool m_geo_loaded;

    float m_z;
    float m_alpha;

    unsigned int m_programId;
    unsigned int m_textureId;

    glm::mat4 m_viewport_matrix;

    unsigned int m_textureSamplerLocation;
    unsigned int m_z_pos_loc;
    unsigned int m_alpha_loc;
    unsigned int m_viewport_matrix_loc;
    unsigned int m_texture_loc;

};

#endif // IMAGEPLANE_H
