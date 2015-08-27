#ifndef MESH_H
#define MESH_H

#include <string>

class Mesh
{
public:
    Mesh();
    virtual ~Mesh() {}

    virtual void create() = 0;
    virtual void update() = 0;
    virtual void draw() = 0;

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
