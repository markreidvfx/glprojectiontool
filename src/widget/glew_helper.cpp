#include "glew_helper.h"
#include <GL/glew.h>

#include <iostream>

int glew_initialize()
{
    glewExperimental=true;
    int err = glewInit();

    if (err != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW\n";
        std::cerr << glewGetErrorString(err);
    }
    return err;


}

