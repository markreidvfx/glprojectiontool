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
    OpenGLWidget(Scene *scene, QWidget *parent = 0, float scale = 1);
    ~OpenGLWidget();

    void initializeGL() Q_DECL_OVERRIDE;
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;

    void set_scale(float x, float y) {m_scaleX = x; m_scaleY = y;}

private:
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

    QFlags<Qt::MouseButton> m_mouseButtons;
    glm::vec2 m_mousePressPosition;
    QTime m_mouseLastTime;

    float m_scaleX;
    float m_scaleY;

    Scene *m_scene;

public slots:
    void open_scene_file(QString path);
    void set_imageplane_data(const FloatImage &data, int width, int height, double seconds, bool loaded);
    void set_template_texture(const FloatImage &data, int width, int height);

    void render_template_data_tiled(std::vector<FloatImageData> &color_data,
                                    std::vector<FloatImageData> &alpha_data,
                                    std::vector<FloatImageData> &contour_data,
                                    double seconds, int tiles, int index
                                    );
    void update_mesh();
    void force_update();
    void clear();

signals:
    void scene_loaded(QString);
    void frame_range_changed(int, int);
    void opengl_initialized();
    void error_message(QString);

};

#endif // OPENGLWINDOW_H
