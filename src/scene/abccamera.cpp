#include "abccamera.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include "abcutils.h"

AbcCamera::AbcCamera()
{

}

void AbcCamera::update(double seconds)
{
    M44d xf;
    {
    std::lock_guard<std::recursive_mutex> l(m_reader->lock);
    xf = getFinalMatrix(m_camera, seconds);
    }
    //std::cerr <<xf << "\n";

    glm::mat4 m = glm::make_mat4(&xf[0][0]);

    glm::vec3 scale;
    glm::quat orientation;
    glm::vec3 translate;
    glm::vec3 skew;
    glm::vec4 persp;

    glm::decompose(m, scale, orientation, translate, skew, persp);

    glm::vec3 rotate =  glm::degrees(glm::eulerAngles(glm::inverse(orientation)));// + glm::vec3(180.0f,180.0f, 180.0f);

    //glm::vec3 rotate(165.053373398f,  197.17695553, -180.11395659);

    m_translation = translate;
    m_rotation = rotate;
    m_scale = scale;

    CameraSample sampler;
    ISampleSelector sel( seconds );

    m_camera.getSchema().get(sampler, sel);


    //cerr << "t: " << glm::to_string(m_translation) << "\n";
    //cerr << "r: " << glm::to_string(rotate) << "\n";

    //cerr << "s: " << glm::to_string(m_scale) << "\n";


    // * 10.0 since vertical film aperture is in cm
    m_fov_y =  2.0 * glm::degrees( atan( sampler.getVerticalAperture() * 10.0 /
                                         (2.0 * sampler.getFocalLength() ) ) );

    m_point_of_interest = glm::length(m_translation);

}
