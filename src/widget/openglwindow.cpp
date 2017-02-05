#include "openglwindow.h"
#include <QThread>

#include <iostream>


OpenGLWidget::OpenGLWidget(Scene *scene, QWidget *parent)
    : QOpenGLWidget(parent)
{
    m_scene = scene;
}

OpenGLWidget::~OpenGLWidget()
{

}

void OpenGLWidget::mousePressEvent(QMouseEvent *event)
{
    m_mousePressPosition = glm::vec2(event->localPos().x(),
                                     event->localPos().y());
    m_mouseButtons = event->buttons();
    m_mouseLastTime.start();
}

void OpenGLWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_mouseButtons == Qt::NoButton) {
        event->ignore();
        return;
    }
    float time_delta = m_mouseLastTime.elapsed();
    if (time_delta < 25) {
        event->ignore();
        return;
    }

    time_delta /= 100;
    glm::vec2 cur = glm::vec2(event->localPos().x(),
                                     event->localPos().y());
    glm::vec2 last = m_mousePressPosition;

    m_mouseLastTime.start();
    m_mousePressPosition = cur;

    glm::vec2 delta = (cur - last) * time_delta;

    if ((m_mouseButtons & Qt::RightButton &&
         event->modifiers() & Qt::AltModifier) ||
        (m_mouseButtons & Qt::LeftButton &&
         m_mouseButtons & Qt::MiddleButton &&
         event->modifiers() & Qt::AltModifier)) {

        m_scene->camera->dolly(delta);
        update();

    } else if ((m_mouseButtons & Qt::LeftButton &&
         event->modifiers() & Qt::AltModifier &&
         event->modifiers() & Qt::ControlModifier) ||
        (m_mouseButtons & Qt::MiddleButton && Qt::AltModifier)){

        m_scene->camera->pan(delta);
        update();

    } else if (m_mouseButtons & Qt::LeftButton &&
               event->modifiers() & Qt::AltModifier) {
         m_scene->camera->rotate(delta);
        update();

    }

    event->accept();


}

void OpenGLWidget::mouseReleaseEvent(QMouseEvent *event)
{
    m_mouseButtons = Qt::NoButton;
}

void OpenGLWidget::set_imageplane_data(const FloatImage &data, int width, int height, double seconds, bool loaded)
{
    makeCurrent();
    m_scene->set_imageplane_data(data, width, height,  seconds, loaded);
    doneCurrent();
    update();
    QSize s = size();
    s.setWidth(s.width() + 1);
    resize(s);
    s.setWidth(s.width() - 1);
    resize(s);
}

void OpenGLWidget::update_mesh()
{
    makeCurrent();
    m_scene->update();
    doneCurrent();
    update();
}

void OpenGLWidget::open_scene_file(QString path)
{
    makeCurrent();
    if(!m_scene->open(path.toStdString())){
        emit error_message("Error Reading Alembic File: \n" + path);
    }

    QSize s = size();
    m_scene->camera->setViewportSize(glm::vec2(s.width(),s.height()));
    doneCurrent();

    emit scene_loaded(path);
    emit frame_range_changed(m_scene->first(), m_scene->last());
    update();
}

void OpenGLWidget::set_template_texture(const FloatImage &data, int width, int height)
{
    makeCurrent();
    m_scene->set_template_texture(data, width, height);
    doneCurrent();
    QSize s = size();
    s.setWidth(s.width() + 1);
    resize(s);
    s.setWidth(s.width() - 1);
    resize(s);
}

void OpenGLWidget::render_template_data_tiled(std::vector<FloatImageData> &color_data,
                                              std::vector<FloatImageData> &alpha_data,
                                              std::vector<FloatImageData> &contour_data,
                                              double seconds, int tiles, int index)
{
    makeCurrent();
    m_scene->render_template_data_tiled(color_data, alpha_data, contour_data, seconds, tiles, index);
    doneCurrent();
}

void OpenGLWidget::clear()
{
    m_scene->clear();
    emit scene_loaded("");
    update();

    force_update();
}

void OpenGLWidget::force_update()
{
    update();

    QSize s = size();
    s.setWidth(s.width() + 1);
    resize(s);
    s.setWidth(s.width() - 1);
    resize(s);
}

void OpenGLWidget::initializeGL()
{
    glew_initialize();

    std::cerr << "using OpenGL " << format().majorVersion() << "." << format().minorVersion() << "\n";
    m_scene->create();
    //m_scene->update();

    emit opengl_initialized();
    emit scene_loaded("");
}


void OpenGLWidget::resizeGL(int w, int h)
{
    m_scene->camera->setViewportSize(glm::vec2(w,h));
    glViewport(0, 0, w, h);
}

void OpenGLWidget::paintGL()
{

    m_scene->draw(defaultFramebufferObject());

}
