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

void OpenGLWidget::initializeGL()
{
    makeCurrent();
    glew_initialize();

    std::cerr << "using OpenGL " << format().majorVersion() << "." << format().minorVersion() << "\n";
    m_scene->create();
    m_scene->update();
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

    time_delta /= 50;
    glm::vec2 cur = glm::vec2(event->localPos().x(),
                                     event->localPos().y());
    glm::vec2 last = m_mousePressPosition;

    m_mouseLastTime.start();
    m_mousePressPosition = cur;

    glm::vec2 delta = (cur - last) * time_delta;

    if (m_mouseButtons & Qt::LeftButton &&
            event->modifiers() & Qt::AltModifier &&
            event->modifiers() & Qt::ControlModifier) {
        m_scene->camera->pan(delta);

    } else if (m_mouseButtons & Qt::LeftButton &&
               event->modifiers() & Qt::AltModifier) {
         m_scene->camera->rotate(delta);

    } else if (m_mouseButtons & Qt::RightButton &&
               event->modifiers() & Qt::AltModifier) {
        m_scene->camera->dolly(delta);
    }
    event->accept();
    update();

}

void OpenGLWidget::mouseReleaseEvent(QMouseEvent *event)
{
    m_mouseButtons = Qt::NoButton;
}

void OpenGLWidget::keyPressEvent(QKeyEvent * event)
{
    if (event->key() == Qt::Key_1){
        //std::cerr << "1\n";
        m_scene->setSubdivLevel(0);
    } else if (event->key() == Qt::Key_2) {
        m_scene->setSubdivLevel(1);
    }else if (event->key() == Qt::Key_3) {
        m_scene->setSubdivLevel(2);
    }else if (event->key() == Qt::Key_4) {
        m_scene->setSubdivLevel(3);
    }
    update();

}


void OpenGLWidget::set_imageplane_data(const FloatImage &data, int width, int height, int frame)
{
    std::cerr << width << "x" << height << " " << frame << " "<< data.size() << "\n";
    makeCurrent();
    m_scene->set_imageplane_data(data, width, height, frame);
    update();
}
void OpenGLWidget::open_abc(QString path)
{
    makeCurrent();
    m_scene->open(path.toStdString());
    emit scene_loaded(path);
    emit frame_range_changed(m_scene->first(), m_scene->last());
}

void OpenGLWidget::set_template_texture(const FloatImage &data, int width, int height)
{
    makeCurrent();
    m_scene->set_template_texture(data, width, height);
    update();
}

void OpenGLWidget::render_template_data(FloatImageData &color_data,
                                        FloatImageData &alpha_data,
                                        FloatImageData &contour_data,
                                        int frame)
{
    makeCurrent();
    m_scene->render_template_data(color_data, alpha_data, contour_data, frame);
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
