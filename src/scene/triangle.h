#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "mesh.h"

class Triangle : public Mesh
{
public:
    Triangle();
    virtual ~Triangle();
    void create();
    void update(double time=0);
    void draw();
};

#endif // TRIANGLE_H
