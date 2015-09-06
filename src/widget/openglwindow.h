#ifndef OPENGLWINDOW_H
#define OPENGLWINDOW_H

#include "glew_helper.h"
#include <QWindow>
#include <QMutex>
#include <QSharedPointer>
#include <QOpenGLContext>
#include <QMouseEvent>
#include <QTimer>
#include <QTime>

#include <glm/glm.hpp>

#include "scene.h"
#include "loader.h"

class OpenGLWindow;

struct SurfaceInfo
{
    OpenGLWindow *surface;
    glm::vec2 size;
};

class Renderer : public QObject
{
    Q_OBJECT

public:
    explicit Renderer(const QSurfaceFormat &format, Renderer *share = 0, QScreen *screen = 0);
    ~Renderer(){}
    QSurfaceFormat format() const { return m_format; }
    void set_window(OpenGLWindow *window);

    void cameraPan(glm::vec2 delta)
    {
        std::lock_guard<std::recursive_mutex> lock(m_scene.m_lock);
        m_scene.camera->pan(delta);
    }
    void cameraDolly(glm::vec2 delta)
    {
        std::lock_guard<std::recursive_mutex> lock(m_scene.m_lock);
        m_scene.camera->dolly(delta);
    }
    void camearRotate(glm::vec2 delta)
    {
        std::lock_guard<std::recursive_mutex> lock(m_scene.m_lock);
        m_scene.camera->rotate(delta);
    }

    void setImagePlanePath(QString path, double value)
    {
        m_scene.setImagePlanePath(path.toStdString(), value);
    }

    void setSubdivLevel(int value){m_scene.setSubdivLevel(value);}
    void update();

    Scene m_scene;

private:
    void initialize();
    bool make_current(SurfaceInfo& info);

    bool m_initialized;
    QSurfaceFormat m_format;
    QOpenGLContext *m_context;
    OpenGLWindow *m_window;

    QMutex m_window_lock;

public slots:
    void open(QString path){m_scene.open(path.toStdString());}
    void setTime(double value){m_scene.setTime(value);}

    void create_template(QString imageplane,
                         QString dest,
                         int frame);

    void set_imageplane_data(const FloatImage &data, int width, int height, int frame);
    void set_template_texture(const FloatImage &data, int width, int height);

private slots:
    void render();

signals:
    void sceneLoaded(QString);
    void frameRangeChanged(int, int);
    void template_complete(QString imageplane,
                           QString dest,
                           int frame);
};

class OpenGLWindow : public QWindow
{
    Q_OBJECT

public:
    explicit OpenGLWindow(const QSharedPointer<Renderer> &renderer);

    void exposeEvent(QExposeEvent *event) Q_DECL_OVERRIDE;
    void open(const QString &path){emit requestOpenFile(path);}

private:
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent * event) Q_DECL_OVERRIDE;

    QFlags<Qt::MouseButton> m_mouseButtons;
    glm::vec2 m_mousePressPosition;
    QTime m_mouseLastTime;

    const QSharedPointer<Renderer> m_renderer;

signals:
    void requestOpenFile(QString);
};

#endif // OPENGLWINDOW_H
