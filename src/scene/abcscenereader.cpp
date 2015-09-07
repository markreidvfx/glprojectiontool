#include "abcscenereader.h"

#include "abcmesh.h"
#include "abccamera.h"

#include <iostream>
#include <algorithm>

AbcSceneReader::AbcSceneReader()
{

}


void AbcSceneReader::open(const std::string &path)
{
    std::cerr << "opening abc file " << path << "\n";
    // should error check
    AbcF::IFactory factory;
    factory.setPolicy(Abc::ErrorHandler::kQuietNoopPolicy);
    AbcF::IFactory::CoreType coreType;
    m_archive = factory.getArchive(path, coreType);

    // determin frame range
    double fps = 24.0;
    has_range = false;

    double f,l;
    Abc::GetArchiveStartAndEndTime(m_archive, f,l);

    first = (int)(f*fps);
    last = (int)(l*fps);
    has_range =true;

}

void AbcSceneReader::read(std::vector< std::shared_ptr<Mesh> > &objects,
                          std::vector<std::shared_ptr<Camera>> &cameras)
{
    IObject obj = m_archive.getTop();
    //cerr <<obj.getFullName() << " children " << obj.getNumChildren() << "\n";

    read_object(obj, objects, cameras);

}

void AbcSceneReader::read_object(const IObject &object,
                                 std::vector< std::shared_ptr<Mesh> > &objects,
                                 std::vector<std::shared_ptr<Camera>> &cameras)
{
    const size_t child_count = object.getNumChildren();
    int mesh_count = 0;
    std::string path = object.getFullName();
    //cerr << path << " children: " << child_count << "\n";
    //std::vector <<IXform> xforms;
    std::cerr << object.getMetaData().serialize() << "\n";
    for (size_t i = 0; i < child_count; ++i) {
        const ObjectHeader& child_header = object.getChildHeader(i);
        if (IPolyMesh::matches(child_header)) {
            mesh_count++;
            cerr << "found mesh " << mesh_count << "\n";
            IPolyMesh m = IPolyMesh(object, child_header.getName());
            objects.push_back(std::make_shared<AbcMesh>(m, this));
            //read_polymesh(IPolyMesh(object, child_header.getName()));
        } else if (ICamera::matches(child_header)) {

            ICamera camera(object, child_header.getName());

            //cerr << "camera found " <<  camera.getFullName()<<"\n";
            cameras.push_back(std::make_shared<AbcCamera>(camera, this));

        }


        read_object(object.getChild(i), objects, cameras);
    }

}
