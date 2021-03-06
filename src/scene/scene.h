#ifndef SCENE_H
#define SCENE_H

#include "scenereader.h"
#include "mesh.h"
#include "camera.h"
#include "framebuffer.h"
#include "templaterenderer.h"
#include "imagereader.h"

#include <OpenEXR/ImathBox.h>

#include <vector>
#include <string>
#include <string>
#include <memory>

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
    bool open(const std::string &path);
    void create();
    void update(double seconds = 0);

    void updateImagePlanes();
    void draw(unsigned int default_framebuffer_id=0);

    void set_time(double seconds){m_time = seconds;}
    double time() {return m_time;}

    long int first() {return m_first;}
    long int last() {return m_last;}

    void set_subdiv_level(int value) {m_subdiv_level = value;}
    int subdiv_level(){return m_subdiv_level;}

    bool scene_loaded(std::string &path)
    {
        path = m_scene_path;
        bool result = m_scene_load;
        m_scene_load = false;
        return result;
    }

    void caculate(double seconds);
    void clear();

    std::vector<SceneObject> scene_objects();
    void update_objects(std::vector<SceneObject> &updated_objects);

    std::vector<SceneObject> scene_cameras();
    void update_cameras(std::vector<SceneObject> &updated_cameras);
    std::shared_ptr<Camera> camera;

    bool frame_range_changed;

    void set_imageplane_data(const std::vector<float> &data, int width, int height, double seconds, bool loaded);
    void set_template_texture(const std::vector<float> &data, int width, int height);

    void render_template_data_tiled(std::vector<FloatImageData> &color_data,
                                    std::vector<FloatImageData> &alpha_data,
                                    std::vector<FloatImageData> &contour_data,
                                    double seconds, int tiles, int tile_index
                                    );

    void export_mesh(const std::string &path, double seconds);
    void export_camera(const std::string &path, int frame);

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

    ImagePlane *m_imageplane;
    std::string m_imageplane_path;
    std::string m_prev_imageplane_path;
    double m_imageplane_time;
    bool m_wait_for_imageplane;

    double m_time;

    unsigned int m_subdiv_level;
    unsigned int m_template_texture_crc;

    void update_frame_range();
    long int m_first;
    long int m_last;

    int m_draw_count;
    Imath::Box3d m_bounds;

    TemplateRenderer m_template;

};

#endif // SCENE_H
