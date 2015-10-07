#ifndef MESH_H
#define MESH_H

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <OpenEXR/ImathBox.h>

struct MeshData {
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;
    std::vector<unsigned int> indices;

    void clear() {
        vertices.clear();
        uvs.clear();
        normals.clear();
        indices.clear();
    }
};

class Mesh
{
public:
    Mesh();
    virtual ~Mesh() {}

    virtual void create() = 0;
    virtual void update() = 0;
    virtual void draw() = 0;
    virtual void calculate(double seconds, int subdivision_level = 0) = 0;
    virtual void read_data(MeshData &data, double seconds) = 0;
    virtual Imath::Box3d bounds(double seconds) = 0;

    unsigned int vertexArrayId;

    unsigned int vertexBufferId;
    unsigned int normalBufferId;
    unsigned int uvBufferId;
    unsigned int indexBufferId;

    unsigned int indexSize;

    bool created;
    std::string name;
    bool visible;
    bool selected;

    unsigned int subdiv_level;
    double time;
    MeshData data;

    unsigned int update_count;

};

#endif // MESH_H
