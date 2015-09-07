#ifndef MESH_H
#define MESH_H

#include <string>
#include <vector>
#include <glm/glm.hpp>

struct MeshData {
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;
    std::vector<unsigned int> indices;
};

class Mesh
{
public:
    Mesh();
    virtual ~Mesh() {}

    virtual void create() = 0;
    virtual void update() = 0;
    virtual void draw() = 0;
    virtual void read_data(MeshData &data, double time) = 0;

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

};

#endif // MESH_H
