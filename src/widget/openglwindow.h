#ifndef OPENGLWINDOW_H
#define OPENGLWINDOW_H

#include "glew_helper.h"
#include <QWindow>
#include <QMutex>
#include <QSharedPointer>
#include <QOpenGLContext>
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
    OpenGLWidget(QWidget *parent = 0);
    ~OpenGLWidget();

    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    Scene scene;

private:
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent * event) Q_DECL_OVERRIDE;


    QFlags<Qt::MouseButton> m_mouseButtons;
    glm::vec2 m_mousePressPosition;
    QTime m_mouseLastTime;

public slots:
    void open_abc(QString path);
    void set_imageplane_data(const FloatImage &data, int width, int height, int frame);
    void set_template_texture(const FloatImage &data, int width, int height);

    void render_template_data(FloatImageData &color_data,
                              FloatImageData &alpha_data,
                              FloatImageData &contour_data,
                              int frame);


signals:
    void scene_loaded(QString);
    void frame_range_changed(int, int);

};

#endif // OPENGLWINDOW_H
