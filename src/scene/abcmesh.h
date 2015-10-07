#ifndef ABCMESH_H
#define ABCMESH_H

#include "mesh.h"
#include "abcscenereader.h"

#include <Alembic/AbcGeom/All.h>
#include <Alembic/AbcCoreAbstract/All.h>
#include <Alembic/AbcCoreHDF5/All.h>
#include <Alembic/AbcCoreFactory/All.h>
#include <Alembic/Abc/ErrorHandler.h>
#include <Alembic/Abc/All.h>
#include <Alembic/Abc/ISchema.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

#include <opensubdiv/far/primvarRefiner.h>

using namespace std;
using namespace Alembic::AbcGeom;
namespace AbcF = ::Alembic::AbcCoreFactory;


class AbcMesh : public Mesh
{
public:
    AbcMesh(const IPolyMesh &mesh, AbcSceneReader* reader);
    virtual ~AbcMesh();

    void create();
    void update();
    void draw();
    void calculate(double seconds,  int subdivision_level = 0);
    void read_data(MeshData &data, double seconds);
    Imath::Box3d bounds(double seconds);

    void create_refiner();

    void subdivide(std::vector<glm::vec3> &vertices,
                   std::vector<glm::vec2> &uvs,
                   std::vector<glm::vec3> &normals,
                   std::vector<unsigned int> &indices);

    void read(std::vector<glm::vec3> &vertices,
              std::vector<glm::vec2> &uvs,
              std::vector<glm::vec3> &normals,
              std::vector<unsigned int> &indices);

    void read_uvs(std::vector<glm::vec2> &uvs);
    void read_normals(std::vector<glm::vec3> &normals);
    void create_normals(std::vector<glm::vec3> &normals);

private:
    IPolyMesh m_mesh;
    IPolyMeshSchema m_schema;
    IPolyMeshSchema::Sample m_sample;
    double m_time;

    OpenSubdiv::Far::TopologyRefiner * m_refiner;
    AbcSceneReader* m_reader;
};

#endif // ABCMESH_H
