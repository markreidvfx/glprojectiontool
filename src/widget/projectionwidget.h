#ifndef PROJECTIONWIDGET_H
#define PROJECTIONWIDGET_H

#include <QWidget>
#include <QThread>
#include "openglwindow.h"
#include "loader.h"

class ProjectionWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ProjectionWidget(QWidget *parent = 0);
    ~ProjectionWidget();

    OpenGLWidget *glwidget;
    Loader *loader;

    //QThread *render_thread;
    QThread *loader_thread;

    Scene scene;

};

#endif // PROJECTIONWIDGET_H
