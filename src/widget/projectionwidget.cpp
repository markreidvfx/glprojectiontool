#include "projectionwidget.h"
#include <QApplication>
#include <QVBoxLayout>

ProjectionWidget::ProjectionWidget(QWidget *parent) : QWidget(parent)
{



    loader = new Loader();
    loader_thread = new QThread;
    loader->moveToThread(loader_thread);
    loader_thread->start();

    glwidget = new OpenGLWidget(this);
    //glwidget->setFormat(format);
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

    glwidget->setFocusPolicy(Qt::StrongFocus);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setMargin(0);
    layout->addWidget(glwidget);
    setLayout(layout);


    /*
    QSurfaceFormat format;
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setSamples(8);
    format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);

    QCoreApplication *app = QApplication::instance();

    QSharedPointer<Renderer> gl_renderer(new Renderer(format));
    glwidget = new OpenGLWindow(gl_renderer);
    renderer = gl_renderer.data();
    QWidget *container = QWidget::createWindowContainer(glwidget);

    loader = new Loader();

    //container->setAttribute(Qt::WA_NoSystemBackground);
    //container->setAttribute(Qt::WA_OpaquePaintEvent);
    //setAttribute(Qt::WA_NoSystemBackground);
    //setAttribute(Qt::WA_OpaquePaintEvent);

    QSize windowSize(400, 320);
    container->setMinimumSize(windowSize);
    container->setFocusPolicy(Qt::StrongFocus);

    render_thread = new QThread;
    renderer->moveToThread(render_thread);

    loader_thread = new QThread;
    loader->moveToThread(loader_thread);

    //QObject::connect(app, SIGNAL(lastWindowClosed()), render_thread, SLOT(quit()));
    render_thread->start();

    //QObject::connect(app, SIGNAL(lastWindowClosed()), loader_thread, SLOT(quit()));
    loader_thread->start();

    // this signal needs to block
    QObject::connect(loader, SIGNAL(imageplane_ready(const FloatImage&,int,int,int)),
                     renderer, SLOT(set_imageplane_data(const FloatImage&,int,int,int)),
                     Qt::BlockingQueuedConnection);

    // this signal needs to block
    QObject::connect(loader, SIGNAL(template_texture_ready(const FloatImage&,int,int)),
                     renderer, SLOT(set_template_texture(const FloatImage&,int,int)),
                     Qt::BlockingQueuedConnection);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setMargin(0);
    layout->addWidget(container);
    setLayout(layout); */

}

ProjectionWidget::~ProjectionWidget()
{
    loader_thread->quit();
    loader_thread->wait();

    /*
    render_thread->quit();
    render_thread->wait();
    delete render_thread; */

}
