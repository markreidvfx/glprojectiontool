#include "projectionwidget.h"
#include <QApplication>
#include <QVBoxLayout>

ProjectionWidget::ProjectionWidget(QWidget *parent) : QWidget(parent)
{

    glwidget = new OpenGLWidget(&scene, this);

    loader = new Loader(&scene);
    loader_thread = new QThread();
    loader->moveToThread(loader_thread);
    loader_thread->start();

    QSize windowSize(400, 320);
    glwidget->setMinimumSize(windowSize);

    QObject::connect(loader, SIGNAL(imageplane_ready(const FloatImage&,int,int,int)),
                     glwidget, SLOT(set_imageplane_data(const FloatImage&,int,int,int)),
                     Qt::BlockingQueuedConnection);

    QObject::connect(loader, SIGNAL(template_texture_ready(const FloatImage&,int,int)),
                     glwidget, SLOT(set_template_texture(const FloatImage&,int,int)),
                     Qt::BlockingQueuedConnection);

    QObject::connect(loader, SIGNAL(request_template_textures(FloatImageData&,FloatImageData&,FloatImageData&,int)),
                     glwidget, SLOT(render_template_data(FloatImageData&,FloatImageData&,FloatImageData&,int)),
                     Qt::BlockingQueuedConnection);

    QObject::connect(loader, SIGNAL(update_mesh()),
                     glwidget, SLOT(update_mesh()),
                     Qt::BlockingQueuedConnection);

    QObject::connect(loader, SIGNAL(request_open_scene_file(QString)),
                     glwidget, SLOT(open_scene_file(QString)),
                     Qt::BlockingQueuedConnection);

    glwidget->setFocusPolicy(Qt::StrongFocus);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setMargin(0);
    layout->addWidget(glwidget);
    setLayout(layout);

}

ProjectionWidget::~ProjectionWidget()
{
    loader_thread->quit();
}
