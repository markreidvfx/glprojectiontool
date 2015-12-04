#include "camera.h"
#include <iostream>
#include "glm/gtc/type_ptr.hpp"
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "glm/gtx/matrix_decompose.hpp"
#include <cfloat>

Camera::Camera()
    : m_rotation( 0.0, 0.0, 0.0 ),
      m_scale( 1.0, 1.0, 1.0 ),
      m_translation( 0.0, 0.0, 2.0),
      m_point_of_interest( 90.0 ),
      m_fov_y( 45.0 ),
      m_clip( 0.1, 1000.0 ),
      m_overscan(.98),
      m_image_size(1920, 1080),
      m_viewport_size(720, 405)

{

    lookAt(m_translation, glm::vec3(0,0,0));
}

static inline void anglesToAxes(const glm::vec3 angles, glm::vec3 & left, glm::vec3 & up, glm::vec3& forward)
{
    const float DEG2RAD = 3.141593f / 180;
    float sx, sy, sz, cx, cy, cz, theta;

    // rotation angle about X-axis (pitch)
    theta = angles.x * DEG2RAD;
    sx = sinf(theta);
    cx = cosf(theta);

    // rotation angle about Y-axis (yaw)
    theta = angles.y * DEG2RAD;
    sy = sinf(theta);
    cy = cosf(theta);

    // rotation angle about Z-axis (roll)
    theta = angles.z * DEG2RAD;
    sz = sinf(theta);
    cz = cosf(theta);

    // determine left axis
    left.x = cy*cz;
    left.y = sx*sy*cz + cx*sz;
    left.z = -cx*sy*cz + sx*sz;

    // determine up axis
    up.x = -cy*sz;
    up.y = -sx*sy*sz + cx*cz;
    up.z = cx*sy*sz + sx*cz;

    // determine forward axis
    forward.x = sy;
    forward.y = -sx*cy;
    forward.z = cx*cy;
}

static inline void rotateVector( float rx, float ry, glm::vec3 &v )
{
    v = glm::rotate(v, glm::radians(rx), glm::vec3(1.0f, 0.0f, 0.0f));
    v = glm::rotate(v, glm::radians(ry), glm::vec3(0.0f, 1.0f, 0.0f));
}

const glm::mat4 Camera::viewMatrix()
{

    glm::mat4 m;
    m *= glm::scale(glm::vec3(1,1,1) / m_scale);
    m *= glm::rotate(-glm::radians(m_rotation.x) , glm::vec3(1,0,0));
    m *= glm::rotate(-glm::radians(m_rotation.y) , glm::vec3(0,1,0));
    m *= glm::rotate(-glm::radians(m_rotation.z) , glm::vec3(0,0,1));
    m *= glm::translate( -m_translation);
    return m;
}

const glm::mat4 Camera::projectionMatrix()
{

    float aspect = m_image_size.x / m_image_size.y;
    glm::mat4 m = glm::perspective(float(glm::radians(m_fov_y)),
                                   aspect, m_clip.x, m_clip.y);
    return m;
}

const glm::mat4 Camera::viewportMatrix()
{

    float sx = 1;
    float sy = 1;

    float image_aspect = m_image_size.x / m_image_size.y;
    float viewport_aspect = m_viewport_size.x / m_viewport_size.y;

    if (image_aspect < viewport_aspect)
        sx = m_viewport_size.y * m_image_size.x /  m_image_size.y / m_viewport_size.x;
    else
        sy = m_viewport_size.x * m_image_size.y / m_image_size.x / m_viewport_size.y;

    glm::mat4 m;

    m *= glm::scale(glm::vec3(sx,sy, 1 ));
    m *= glm::scale(glm::vec3(m_overscan, m_overscan, 1));

    return m;

}


void Camera::forward(float amount)
{
    float rotX = m_rotation.x;
    float rotY = m_rotation.y;

    glm::vec3 v (0.0, 0.0, -m_point_of_interest);
    rotateVector( rotX, rotY, v);
    m_translation += v * amount;

}

void Camera::right(float amount)
{
    pan(glm::vec2(200,0) * -amount);
}

void Camera::up(float amount)
{
    pan(glm::vec2(0,200) * amount);
}

void Camera::lookAt(const glm::vec3 &eye, const glm::vec3 &at)
{
    m_translation = eye;
    glm::vec3 dt = at - eye;

    double xzLen = sqrt( (dt.x * dt.x) + (dt.z * dt.z));

    m_rotation.x = glm::degrees(atan2(dt.y, xzLen));
    m_rotation.y = glm::degrees(atan2(dt.x, -dt.z));
    m_point_of_interest = glm::length(dt);

}

void Camera::pan(const glm::vec2 &point)
{
    float rotX = m_rotation.x;
    float rotY = m_rotation.y;

    glm::vec3 dS( 1.0, 0.0, 0.0);

    rotateVector(rotX, rotY, dS);

    glm::vec3 dT( 0.0, 1.0, 0.0);
    rotateVector( rotX, rotY, dT );

    float multS = 2.0 * m_point_of_interest * tanf( glm::radians( fovy() ) / 2.0 );
    const float multT = multS / float( m_viewport_size.y );
    multS /= float( m_viewport_size.x );

    const float s = -multS * point.x;
    const float t = multT * point.y;

    setTranslation( (m_translation +
                     ( s * dS ) + ( t * dT ) ) );

}

void Camera::dolly(const glm::vec2 &point, double dollySpeed)
{
    float rotX = m_rotation.x;
    float rotY = m_rotation.y;
    const glm::vec3 &eye = m_translation;

    glm::vec3 v (0.0, 0.0, -m_point_of_interest);
    rotateVector( rotX, rotY, v);

    glm::vec3 view = eye + v;
    v = glm::normalize(v);

    double t = point.x / double(m_viewport_size.x);

    float dollyBy = 1.0 - expf( -dollySpeed * t);
    //float dollyBy = dollySpeed * t;

    if (dollyBy > 1.0) {
        dollyBy = 1.0;
    } else if (dollyBy < -1.0) {
        dollyBy = -1.0;
    }

    dollyBy *= m_point_of_interest;

    glm::vec3 new_eye = eye + (dollyBy *v);

    setTranslation(new_eye);
    v = new_eye - view;
    m_point_of_interest = glm::length(v);
}

void Camera::rotate(const glm::vec2 &point, double rotateSpeed)
{
    double rotX = m_rotation.x;
    double rotY = m_rotation.y;
    const float rotZ = m_rotation.z;
    glm::vec3 eye = m_translation;

    glm::vec3 v(0.0f, 0.0f, -m_point_of_interest);
    rotateVector(rotX, rotY, v);

    const glm::vec3 view = eye + v;

    rotY += -rotateSpeed * ( point.x / double( m_viewport_size.x ) );
    rotX += -rotateSpeed * ( point.y / double( m_viewport_size.y ) );

    v.x = 0.0f;
    v.y = 0.0f;
    v.z = m_point_of_interest;
    rotateVector(rotX, rotY, v);

    const glm::vec3 new_eye = view + v;

    setTranslation(new_eye);
    setRotation(glm::vec3( rotX, rotY, rotZ ) );

}

void Camera::auto_clipping_plane( const Imath::Box3d &bounds)
{
    const double rotX = m_rotation.x;
    const double rotY = m_rotation.y;
    const glm::vec3 eye = m_translation;

    double clipNear = FLT_MAX;
    double clipFar = FLT_MIN;

    glm::vec3 v(0.0, 0.0, -m_point_of_interest);
    rotateVector(rotX, rotY, v);
    const glm::vec3 view = eye + v;
    v = glm::normalize(v);

    glm::vec3 points[8];

    points[0] = glm::vec3(bounds.min.x, bounds.min.y, bounds.min.z);
    points[1] = glm::vec3(bounds.min.x, bounds.min.y, bounds.max.z);
    points[2] = glm::vec3(bounds.min.x, bounds.max.y, bounds.min.z);
    points[3] = glm::vec3(bounds.min.x, bounds.max.y, bounds.max.z);
    points[4] = glm::vec3(bounds.max.x, bounds.min.y, bounds.min.z);
    points[5] = glm::vec3(bounds.max.x, bounds.min.y, bounds.max.z);
    points[6] = glm::vec3(bounds.max.x, bounds.max.y, bounds.min.z);
    points[7] = glm::vec3(bounds.max.x, bounds.max.y, bounds.max.z);

    for( int p = 0; p < 8; ++p )
    {
        glm::vec3 dp = points[p] - eye;
        double proj = glm::dot(dp, v);
        clipNear = std::min(proj, clipNear);
        clipFar = std::max(proj, clipFar);
    }

    clipNear -= 0.5f;
    clipFar  += 0.5f;
    clipNear = glm::clamp(clipNear, 0.1, 100000.0);
    clipFar  = glm::clamp(clipFar, 500.0, 100000.0);

    if (clipFar <= clipNear) {
        clipFar = clipNear + 0.1;
    }

    m_clip.x = clipNear;
    m_clip.y = clipFar;
}
