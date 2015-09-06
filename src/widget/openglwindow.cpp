
#include "openglwindow.h"

#include <qmath.h>
#include <QThread>

#include <iostream>

Renderer::Renderer(const QSurfaceFormat &format, Renderer *share, QScreen *screen)
                    : m_initialized(false),
                      m_format(format),
                      m_window(0)
{
    m_context = new QOpenGLContext(this);
    if (screen)
        m_context->setScreen(screen);
    m_context->setFormat(format);
    if (share)
        m_context->setShareContext(share->m_context);
    m_context->create();
}

void Renderer::set_window(OpenGLWindow *window)
{

    QMutexLocker locker(&m_window_lock);

    if (!m_window) {
        m_window = window;
        QTimer::singleShot(0, this, SLOT(render()));
    }

}

void Renderer::initialize()
{
    glew_initialize();

    std::cerr << "using OpenGL " << format().majorVersion() << "." << format().minorVersion() << "\n";

    m_scene.create();
    m_scene.update();

}

bool Renderer::make_current(SurfaceInfo& info)
{
    QMutexLocker locker(&m_window_lock);
    if (!m_window)
        return false;

    if (!m_context->makeCurrent(m_window))
        return false;

    const qreal retinaScale = m_window->devicePixelRatio();
    glm::ivec2 size = glm::ivec2(m_window->size().width() * retinaScale,
                                 m_window->size().height()* retinaScale);
    locker.unlock();

    if (!m_initialized) {
        initialize();
        m_initialized = true;
    }
    std::lock_guard<std::recursive_mutex> lock(m_scene.m_lock);
    m_scene.camera->setViewportSize(size);
    info.size = size;
    info.surface = m_window;

    return true;
}

void Renderer::update()
{
    //QTimer::singleShot(0, this, SLOT(render()));
}

void Renderer::render()
{

    SurfaceInfo info;
    if (!make_current(info))
        return;

    m_scene.draw();
    m_context->swapBuffers(info.surface);

    //QThread::currentThread()->sleep(1);

    QTimer::singleShot(0, this, SLOT(render()));

    std::string path;
    if (m_scene.scene_loaded(path)){
        QString p(path.c_str());
        emit sceneLoaded(p);
        emit frameRangeChanged(m_scene.first(), m_scene.last());
    }

}

void Renderer::set_imageplane_data(const FloatImage &data, int width, int height, int frame)
{
    std::cerr << width << "x" << height << " " << frame << " "<< data.size() << "\n";

    SurfaceInfo info;
    if (!make_current(info)) {
        std::cerr << "error make current\n";
        return;
    }

    m_scene.set_imageplane_data(data, width, height, frame);

}

void Renderer::set_template_texture(const FloatImage &data, int width, int height)
{
    std::cerr << "template texture: " << width << "x" << height  << " "<< data.size() << "\n";

    SurfaceInfo info;
    if (!make_current(info)) {
        std::cerr << "error make current\n";
        return;
    }

    m_scene.set_template_texture(data, width, height);
}

void Renderer::create_template(QString imageplane,
                               QString dest,
                               int frame)
{
    SurfaceInfo info;
    if (!make_current(info))
        return;

    m_scene.render_template(imageplane.toStdString(),
                            dest.toStdString(),
                            frame);

   emit template_complete(imageplane, dest, frame);
}

OpenGLWindow::OpenGLWindow(const QSharedPointer<Renderer> &renderer)
    : m_renderer(renderer)
{
    setSurfaceType(QWindow::OpenGLSurface);
    setFlags(Qt::Window | Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);

    setFormat(renderer->format());
    create();

    connect(this, SIGNAL(requestOpenFile(QString)),
            renderer.data(), SLOT(open(QString)));
}

void OpenGLWindow::exposeEvent(QExposeEvent *)
{
    m_renderer->set_window(this);
}

void OpenGLWindow::mousePressEvent(QMouseEvent *event)
{
    m_mousePressPosition = glm::vec2(event->localPos().x(),
                                     event->localPos().y());
    m_mouseButtons = event->buttons();
    m_mouseLastTime.start();
}

void OpenGLWindow::mouseMoveEvent(QMouseEvent *event)
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

    //std::cerr << "time delta " << time_delta << "\n";

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
        m_renderer->cameraPan(delta);

    } else if (m_mouseButtons & Qt::LeftButton &&
               event->modifiers() & Qt::AltModifier) {
        m_renderer->camearRotate(delta);

    } else if (m_mouseButtons & Qt::RightButton &&
               event->modifiers() & Qt::AltModifier) {
        m_renderer->cameraDolly(delta);
    }
    event->accept();
    m_renderer->update();

}

void OpenGLWindow::mouseReleaseEvent(QMouseEvent *event)
{
    m_mouseButtons = Qt::NoButton;
}

void OpenGLWindow::keyPressEvent(QKeyEvent * event)
{
    if (event->key() == Qt::Key_1){
        //std::cerr << "1\n";
        m_renderer->setSubdivLevel(0);
    } else if (event->key() == Qt::Key_2) {
        m_renderer->setSubdivLevel(1);
    }else if (event->key() == Qt::Key_3) {
        m_renderer->setSubdivLevel(2);
    }else if (event->key() == Qt::Key_4) {
        m_renderer->setSubdivLevel(3);
    }
    m_renderer->update();

}

OpenGLWidget::OpenGLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{

}

OpenGLWidget::~OpenGLWidget()
{

}

void OpenGLWidget::initializeGL()
{
    makeCurrent();
    glew_initialize();

    std::cerr << "using OpenGL " << format().majorVersion() << "." << format().minorVersion() << "\n";
    scene.create();
    scene.update();
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
        scene.camera->pan(delta);

    } else if (m_mouseButtons & Qt::LeftButton &&
               event->modifiers() & Qt::AltModifier) {
         scene.camera->rotate(delta);

    } else if (m_mouseButtons & Qt::RightButton &&
               event->modifiers() & Qt::AltModifier) {
        scene.camera->dolly(delta);
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
        scene.setSubdivLevel(0);
    } else if (event->key() == Qt::Key_2) {
        scene.setSubdivLevel(1);
    }else if (event->key() == Qt::Key_3) {
        scene.setSubdivLevel(2);
    }else if (event->key() == Qt::Key_4) {
        scene.setSubdivLevel(3);
    }
    update();

}


void OpenGLWidget::set_imageplane_data(const FloatImage &data, int width, int height, int frame)
{
    std::cerr << width << "x" << height << " " << frame << " "<< data.size() << "\n";
    makeCurrent();
    scene.set_imageplane_data(data, width, height, frame);
    update();
}
void OpenGLWidget::open_abc(QString path)
{
    makeCurrent();
    scene.open(path.toStdString());
    emit scene_loaded(path);
    emit frame_range_changed(scene.first(), scene.last());
}

void OpenGLWidget::set_template_texture(const FloatImage &data, int width, int height)
{
    makeCurrent();
    scene.set_template_texture(data, width, height);
    update();
}

void OpenGLWidget::resizeGL(int w, int h)
{
    scene.camera->setViewportSize(glm::vec2(w,h));
    glViewport(0, 0, w, h);
}

void OpenGLWidget::paintGL()
{

    scene.draw(defaultFramebufferObject());
}
