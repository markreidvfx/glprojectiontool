#ifndef OPENGLWINDOW_H
#define OPENGLWINDOW_H

#include "glew_helper.h"
#include <QWidget>
#include <QWindow>
#include <QMutex>
#include <QOpenGLWidget>
#include <QMouseEvent>
#include <QTimer>
#include <QTime>

#include <glm/glm.hpp>

#include "scene.h"
#include "loader.h"

class OpenGLWidget : public QOpenGLWidget
{
    Q_OBJECT

public:
    OpenGLWidget(Scene *scene, QWidget *parent = 0);
    ~OpenGLWidget();

    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

private:
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

    QFlags<Qt::MouseButton> m_mouseButtons;
    glm::vec2 m_mousePressPosition;
    QTime m_mouseLastTime;

    Scene *m_scene;

public slots:
    void open_scene_file(QString path);
    void set_imageplane_data(const FloatImage &data, int width, int height, int frame);
    void set_template_texture(const FloatImage &data, int width, int height);

    void render_template_data(FloatImageData &color_data,
                              FloatImageData &alpha_data,
                              FloatImageData &contour_data,
                              int frame);
    void update_mesh();

signals:
    void scene_loaded(QString);
    void frame_range_changed(int, int);
    void opengl_initialized();

};

#endif // OPENGLWINDOW_H
