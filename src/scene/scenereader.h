#ifndef SCENEREADER_H
#define SCENEREADER_H

#include "mesh.h"
#include "imageplane.h"
#include "camera.h"

#include <string>
#include <vector>
#include <mutex>

class SceneReader
{
public:
    SceneReader():first(1),last(100),has_range(false) {}
    virtual void open(const std::string &path) = 0;
    virtual void read(std::vector< std::shared_ptr<Mesh> > &objects,
                      std::vector<std::shared_ptr<Camera> > &cameras) = 0;
    virtual void close() = 0;

    bool has_range;
    long int first;
    long int last;
    std::recursive_mutex lock;

private:

};

#endif // SCENEREADER_H
