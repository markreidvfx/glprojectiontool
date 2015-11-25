#ifndef CAMERA_H
#define CAMERA_H

#include <algorithm>
#include <glm/glm.hpp>

#include <string>
#include <memory>
#include <OpenEXR/ImathBox.h>

class Camera
{
public:
    Camera();
    ~Camera() {};

    const glm::mat4 viewMatrix();
    const glm::mat4 projectionMatrix();
    const glm::mat4 viewportMatrix();

    const glm::vec3 rotation() const {return m_rotation;}
    void setRotation(const glm::vec3 &r) {m_rotation = r;}

    const glm::vec3 scale() const {return m_scale;}
    void setScale(const glm::vec3 &s) {m_scale = s;}

    const glm::vec3 translation()
    {
        return m_translation;
    }
    void setTranslation( const glm::vec3 &t ) { m_translation = t; }

    double pointOfInterest() const { return m_point_of_interest; }
    void setPointOfInterest( double poi ) {m_point_of_interest = std::max( poi, 0.1 );}

    double fovy() const { return m_fov_y; }
    void setFovy( double fvy ) { m_fov_y = fvy; }

    glm::vec2 clippingPlanes() const { return m_clip; }
    void setClippingPlanes( double nearValue, double farValue )
    {
        m_clip.x = nearValue;
        m_clip.y = farValue;
    }

    void forward(float amount = 1.0);
    void right(float amount = 1.0);
    void up(float amount = 1.0);

    void pan(const glm::vec2 &point);
    void dolly(const glm::vec2 &point, double dollySpeed = 5.0);
    void rotate(const glm::vec2 &point, double rotateSpeed = 200.0);

    void lookAt(const glm::vec3 &eye, const glm::vec3 &at);

    void setImageSize( const glm::vec2 &size)
    {
        m_image_size = size;
    }

    void setViewportSize(const glm::vec2 &size)
    {
        m_viewport_size = size;
    }
    glm::vec2 viewportSize()
    {
        return m_viewport_size;
    }

    virtual void update(double seconds=0) {}

    std::string name() {
        return m_name;
    }

    void setName(const std::string &name) {
        m_name = name;
    }

    void auto_clipping_plane(const Imath::Box3d &bounds);

protected:
    //std::recursive_mutex m_lock;

    glm::vec3 m_rotation;
    glm::vec3 m_scale;
    glm::vec3 m_translation;

    double m_point_of_interest;
    double m_fov_y;

    glm::vec2 m_clip;

    glm::vec2 m_image_size;
    glm::vec2 m_viewport_size;
    double m_overscan;

    std::string m_name;

};

#endif // CAMERA_H
