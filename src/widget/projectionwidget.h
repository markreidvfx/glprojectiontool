#ifndef PROJECTIONWIDGET_H
#define PROJECTIONWIDGET_H

#include <QWidget>
#include <QThread>
#include "openglwindow.h"

class ProjectionWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ProjectionWidget(QWidget *parent = 0);

    OpenGLWindow *glwidget;
    Renderer *renderer;
    QThread *render_thread;

};

#endif // PROJECTIONWIDGET_H
