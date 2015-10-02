#include <GL/glew.h>
#include "scene.h"
#include "shaders.h"
#include "triangle.h"
#include "imageplane.h"
#include "imagereader.h"
#include "abcscenereader.h"
#include "abcutils.h"

#include "rc_helper.h"
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include <chrono>

#define DEFAULT_BUFFER_SIZE 1024 + 128


static uint32_t crc32(uint32_t crc, const uint8_t *p, unsigned int len)
{
    unsigned int i;

    while (len--) {
       crc ^= *p++;
       for (i = 0; i < 8; i++)
           crc = (crc >> 1) ^ ((crc & 1) ? 0xedb88320 : 0);
    }
    return crc;
}

Scene::Scene() : m_time(0),
                 m_first(1),
                 m_last(100)
{
    camera = std::make_shared<Camera>();
    camera->setName("Persp");
    m_cameras.push_back(camera);
    m_imageplane_time = 0;
    m_template_texture_crc = 0;
    m_subdiv_level = 1;
    m_draw_count = 0;
    m_wait_for_imageplane = false;
}

Scene::~Scene()
{

}

void Scene::open(const std::string &path)
{
    auto reader = std::make_shared<AbcSceneReader>();
    reader->open(path);
    reader->read(m_objects, m_cameras);

    camera = m_cameras[m_cameras.size() -1 ];
    m_scene_readers.push_back(reader);

    m_scene_load = true;
    m_scene_path = path;

    update_frame_range();

}

std::vector<SceneObject> Scene::scene_cameras()
{
    std::vector<SceneObject> objects(m_cameras.size());

    for (int i = 0; i < m_cameras.size(); i++) {
        objects[i].name = m_cameras[i]->name();
        objects[i].visible = true;
        if (m_cameras[i]->name() == camera->name())
            objects[i].visible = true;

    }

    return objects;
}

void Scene::update_cameras(std::vector<SceneObject> &updated_cameras)
{
    for (int i = 0; i < m_cameras.size(); i++) {
        for (int j =0; j < updated_cameras.size(); j++){
            if (updated_cameras[j].visible &&
                updated_cameras[j].name == m_cameras[i]->name()) {
                camera = m_cameras[i];
                break;
            }

        }
    }

}

std::vector<SceneObject> Scene::scene_objects()
{
    std::vector<SceneObject> objects(m_objects.size());
    for (int i = 0; i < m_objects.size(); i++) {
        objects[i].name = m_objects[i]->name;
        objects[i].visible = m_objects[i]->visible;
        objects[i].selected = m_objects[i]->selected;
    }

    return objects;
}

void Scene::update_objects(std::vector<SceneObject> &updated_objects)
{
    for (int i = 0; i < m_objects.size(); i++) {
        for (int j =0; j < updated_objects.size(); j++) {
            if (updated_objects[j].name == m_objects[i]->name) {
                m_objects[i]->visible = updated_objects[j].visible;
                m_objects[i]->selected = updated_objects[j].selected;
                break;
            }
        }
    }
}
void Scene::update_frame_range()
{
    long int f = 1;
    long int l = 100;

    bool range_found = false;
    for (int i =0; i <m_scene_readers.size(); i++){

        if (m_scene_readers[i]->has_range) {
            if (range_found) {
                f = std::min(f, m_scene_readers[i]->first);
                l = std::max(l, m_scene_readers[i]->last);
            } else {
                f = m_scene_readers[i]->first;
                l = m_scene_readers[i]->last;
                range_found = true;
            }
        }
    }

    m_first  = f;
    m_last = l;

}

void Scene::setTime(double value)
{
    cerr << value/24.0 << " " << m_first / 24.0 <<"\n";
    m_time = value/24.0;
}


void Scene::create()
{
    glClearColor(0.85f, 0.85f, 0.85f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    ImagePlane *t = new ImagePlane();
    t->create();
    m_imageplane = t;
    m_template.create();

    m_template.resize_buffers(DEFAULT_BUFFER_SIZE,
                              DEFAULT_BUFFER_SIZE);
}

static void _update(std::shared_ptr<Mesh> object)
{
    std::cerr << "update \n";
    object->update();
}

// this is to run in bg thread
void Scene::caculate(double time)
{
    std::chrono::time_point<std::chrono::system_clock> start;
    std::chrono::duration<double> elapsed_seconds;
    start = std::chrono::system_clock::now();

    int level = subdivLevel();
    for (int i = 0; i < m_objects.size(); i++) {
        m_objects[i]->subdiv_level = level;
        m_objects[i]->time = time;
        m_objects[i]->calculate(time, level);
    }
    m_time = time;

    elapsed_seconds = std::chrono::system_clock::now()-start;
    std::cerr << "geo caculated in " << elapsed_seconds.count() << " secs \n";
}

void Scene::update(double time)
{
    std::chrono::time_point<std::chrono::system_clock> start;
    std::chrono::duration<double> elapsed_seconds;
    start = std::chrono::system_clock::now();

    for (int i = 0; i < m_objects.size(); i++) {
        m_objects[i]->update();
    }

    for (int i = 0; i < m_cameras.size(); i++) {
        m_cameras[i]->update(time);
    }

    elapsed_seconds = std::chrono::system_clock::now()-start;
    std::cerr << "geo updated in " << elapsed_seconds.count() << " secs \n";

}

void Scene::set_imageplane_data(const std::vector <float> &data, int width, int height, int frame)
{

    m_imageplane->update();
    m_imageplane->setImageData(width, height, data);
    update(frame);
}

void Scene::set_template_texture(const std::vector<float> &data, int width, int height)
{
    m_template.set_template_texture(data, width, height);
    m_template_texture_crc++;
}

void Scene::clear()
{
    m_cameras.clear();
    m_objects.clear();
    m_scene_readers.clear();

    camera = std::make_shared<Camera>();
    camera->setName("Persp");
    m_cameras.push_back(camera);
}

void Scene::render_template_data(FloatImageData &color_data,
                                 FloatImageData &alpha_data,
                                 FloatImageData &contour_data,
                                 int frame)
{

    m_template.resize_buffers(1024 * 2 , 1024 * 2);
    float line_width = m_template.line_width();
    m_template.set_line_width(0.5);

    update(frame);
    draw();
    m_template.render_template_data(color_data, alpha_data, contour_data);

    //reset sizes
    m_template.resize_buffers(DEFAULT_BUFFER_SIZE,
                              DEFAULT_BUFFER_SIZE);
    m_template.set_line_width(line_width);

}

void Scene::render_template_data_tiled(std::vector<FloatImageData> &color_data,
                                       std::vector<FloatImageData> &alpha_data,
                                       std::vector<FloatImageData> &contour_data,
                                       int frame, int tiles, int tile_index
                                       )
{
    std::cerr << "rendering tiled index " << tile_index << std::endl ;

    //color_data.resize(tiles * tiles);
    //alpha_data.resize(tiles * tiles);
    //contour_data.resize(tiles * tiles);

    m_template.resize_buffers(1024, 1024);
    float line_width = m_template.line_width();
    m_template.set_line_width(0.5);

    update(frame);
    m_template.buffer_scale = glm::vec2(tiles, tiles);
    m_template.buffer_offset = glm::vec2(0, 0);
    int i = 0;
    for (int y = 0; y < tiles; y++) {


        for (int x = 0; x < tiles; x++) {
            if (i == tile_index) {
                std::cerr << "drawing tile " << i << std::endl;
                draw();
                m_template.render_template_data(color_data[i], alpha_data[i], contour_data[i]);
            }
            m_template.buffer_offset.x -= 2.0 / (double) tiles;
            i++;
        }

        m_template.buffer_offset.y -= 2.0 / (double) tiles;
        m_template.buffer_offset.x = 0;

    }
    m_template.resize_buffers(DEFAULT_BUFFER_SIZE,
                              DEFAULT_BUFFER_SIZE);

    m_template.set_line_width(line_width);
    m_template.buffer_scale = glm::vec2(1, 1);
    m_template.buffer_offset = glm::vec2(0, 0);
}

void Scene::export_mesh(const std::string &path, int frame)
{
    std::vector< IArchive > archives;

    for (int i =0; i< m_scene_readers.size(); i++){
        std::shared_ptr<AbcSceneReader> r = std::dynamic_pointer_cast<AbcSceneReader>(m_scene_readers[i]);
        if (r)
            archives.push_back(r->archive());
    }
    export_abc(archives, path, frame);

}

void Scene::export_camera(const std::string &path, int frame)
{

}


void Scene::draw(unsigned int default_framebuffer_id)
{

    m_crc = crc32(0L, NULL, 0);

    glm::mat4 modelToVewMatrix = camera->viewMatrix();
    glm::mat4 projectionMatrix = camera->projectionMatrix();
    glm::mat4 viewportMatrix = camera->viewportMatrix();
    glm::vec3 camera_world = camera->translation();
    glm::vec2 viewport_size = camera->viewportSize();

    glm::mat4 modelToProjectionMatrix = projectionMatrix * modelToVewMatrix;

    glBindFramebuffer(GL_FRAMEBUFFER, default_framebuffer_id);
    glViewport(0,0, viewport_size.x, viewport_size.y);
    glClearColor(0.85f, 0.85f, 0.85f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDisable(GL_CULL_FACE);

    m_imageplane->setViewportMatrix(viewportMatrix);
    m_imageplane->setAlpha(1.0);
    m_imageplane->setZ(0.99999f);
    m_imageplane->draw();
    //return;

    for (int i = 0; i < m_objects.size(); i++) {
        append_crc(&m_objects[i]->update_count, sizeof(unsigned int));
        append_crc(&m_objects[i]->visible, sizeof(bool));
    }

    glm::ivec2 buffer_size(m_template.width(),m_template.height());
    float line_width = m_template.line_width();

    append_crc(&m_template_texture_crc, sizeof(unsigned int));
    append_crc(&line_width, sizeof(float));
    append_crc(&buffer_size[0], sizeof(glm::ivec2));
    append_crc(&modelToProjectionMatrix[0][0], sizeof(glm::mat4));
    append_crc(&m_template.buffer_offset[0], sizeof(glm::vec2));
    append_crc(&m_template.buffer_scale[0], sizeof(glm::vec2));

    bool redraw_offscreen_buffers = false;

    if (m_crc != m_prev_crc) {
        redraw_offscreen_buffers = true;
    }

    //if (redraw_offscreen_buffers) {
    //    cerr << "redrawing offscreen frame buffers " << m_draw_count << "\n";
    //    cerr << "crc " << m_crc << "\n";
    //}

    m_template.draw(m_objects, modelToProjectionMatrix, viewportMatrix, viewport_size, redraw_offscreen_buffers, default_framebuffer_id);
    glBindFramebuffer(GL_FRAMEBUFFER, default_framebuffer_id);

    m_prev_crc = m_crc;
    m_draw_count++;

}

void Scene::append_crc(void *data, int size)
{
    m_crc = crc32(m_crc, (const uint8_t *)data, size);
}



