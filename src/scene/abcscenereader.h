#ifndef ABCSCENEREADER_H
#define ABCSCENEREADER_H

#include "scenereader.h"
#include <Alembic/AbcGeom/All.h>
#include <Alembic/AbcCoreAbstract/All.h>
#include <Alembic/AbcCoreHDF5/All.h>
#include <Alembic/AbcCoreFactory/All.h>
#include <Alembic/Abc/ErrorHandler.h>
#include <Alembic/Abc/All.h>
#include <Alembic/Abc/ISchema.h>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace Alembic::AbcGeom;
namespace AbcF = ::Alembic::AbcCoreFactory;


class AbcSceneReader : public SceneReader
{
public:
    AbcSceneReader();
    bool open(const std::string &path);
    void read(std::vector< std::shared_ptr<Mesh> > &objects,
              std::vector<std::shared_ptr<Camera>> &cameras);
    void read_object(const IObject &object,
                     std::vector< std::shared_ptr<Mesh> > &objects,
                     std::vector<std::shared_ptr<Camera>> &cameras);

    void export_mesh(OArchive &oArchive, int frame);
    IArchive archive() {return m_archive;}

    void close() {}
private:
    IArchive m_archive;
};

#endif // ABCSCENEREADER_H
