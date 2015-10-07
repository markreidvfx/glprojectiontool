#ifndef ABCUTILS_H
#define ABCUTILS_H

#include <Alembic/AbcGeom/All.h>
#include <Alembic/AbcCoreAbstract/All.h>
#include <Alembic/AbcCoreHDF5/All.h>
#include <Alembic/AbcCoreFactory/All.h>
#include <Alembic/Abc/ErrorHandler.h>
#include <Alembic/Abc/All.h>
#include <Alembic/Abc/ISchema.h>

using namespace Alembic::AbcGeom;
using namespace Alembic::AbcCoreAbstract;

M44d getFinalMatrix( IObject &iObj, chrono_t seconds);
void export_abc(std::vector< IArchive > &archives, std::string path, double seconds);

#endif // ABCUTILS_H
