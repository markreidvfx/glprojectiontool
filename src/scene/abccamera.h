#ifndef ABCCAMERA_H
#define ABCCAMERA_H

#include "camera.h"
#include "abcscenereader.h"

#include <Alembic/AbcGeom/All.h>
#include <Alembic/AbcCoreAbstract/All.h>
#include <Alembic/AbcCoreHDF5/All.h>
#include <Alembic/AbcCoreFactory/All.h>
#include <Alembic/Abc/ErrorHandler.h>
#include <Alembic/Abc/All.h>
#include <Alembic/Abc/ISchema.h>

#include <vector>

using namespace std;
using namespace Alembic::AbcGeom;
namespace AbcF = ::Alembic::AbcCoreFactory;

class AbcCamera : public Camera
{
public:

    AbcCamera();
    AbcCamera(ICamera &camera, AbcSceneReader* reader) {
        m_camera = camera;
        m_name = camera.getName();
        m_reader = reader;
    }
    virtual void update(double time = 0);

private:
    ICamera m_camera;
    AbcSceneReader* m_reader;

};

#endif // ABCCAMERA_H
