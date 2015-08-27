#ifndef SCENE_H
#define SCENE_H

#include "scenereader.h"
#include "mesh.h"
#include "camera.h"
#include "framebuffer.h"
#include "templaterenderer.h"

#include <vector>
#include <string>
#include <mutex>
#include <string>

#include <iostream>

struct SceneObject
{
    std::string name;
    bool visible;
    bool selected;
};

class Scene
{
public:
    Scene();
    ~Scene();
    void open(const std::string &path);
    void create();
    void update(double time = 0);

    void updateImagePlanes();
    void draw();

    void setImagePlanePath(const std::string &path, double time);
    std::string imagePlanePath()
    {
        std::lock_guard<std::recursive_mutex> lock(m_lock);
        return m_imageplane_path;
    }

    void setTime(double value);
    double time() {
         std::lock_guard<std::recursive_mutex> lock(m_lock);
         return m_time;
    }

    long int first() {
        std::lock_guard<std::recursive_mutex> lock(m_lock);
        return m_first;
    }

    long int last() {
        std::lock_guard<std::recursive_mutex> lock(m_lock);
        return m_last;
    }

    void setSubdivLevel(int value) {
        std::lock_guard<std::recursive_mutex> lock(m_lock);
        m_subdiv_level = value;
    }

    int subdivLevel(){
        std::lock_guard<std::recursive_mutex> lock(m_lock);
        return m_subdiv_level;
    }

    bool scene_loaded(std::string &path)
    {
        path = m_scene_path;
        bool result = m_scene_load;
        m_scene_load = false;
        return result;
    }

    std::vector<SceneObject> scene_objects();
    void update_objects(std::vector<SceneObject> &updated_objects);

    std::vector<SceneObject> scene_cameras();
    void update_cameras(std::vector<SceneObject> &updated_cameras);
    std::shared_ptr<Camera> camera;
    std::recursive_mutex m_lock;

    bool frame_range_changed;

    void render_template(std::string image_plane,
                         std::string dest,
                         int frame);
    int progress(){return m_template.get_progress();}

private:

    std::vector< std::shared_ptr<Mesh> > m_objects;
    std::vector< std::shared_ptr<Camera> > m_cameras;
    std::vector< std::shared_ptr<SceneReader> > m_scene_readers;
    bool m_scene_load;
    std::string m_scene_path;

    void append_crc(void *data, int size);
    unsigned long m_crc;
    unsigned long m_prev_crc;

    ImageReader m_imageplane_reader;
    ImagePlane *m_imageplane;
    std::string m_imageplane_path;
    std::string m_prev_imageplane_path;
    double m_imageplane_time;
    bool m_wait_for_imageplane;

    double m_time;
    double m_prev_time;

    unsigned int m_subdiv_level;

    FrameBuffer m_framebuffer;

    void update_frame_range();
    long int m_first;
    long int m_last;

    int m_draw_count;

    TemplateRenderer m_template;

};

#endif // SCENE_H
