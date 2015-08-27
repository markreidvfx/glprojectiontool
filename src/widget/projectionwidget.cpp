#include "projectionwidget.h"
#include <QApplication>
#include <QVBoxLayout>

ProjectionWidget::ProjectionWidget(QWidget *parent) : QWidget(parent)
{
    QSurfaceFormat format;
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setSamples(8);
    format.setSwapBehavior(QSurfaceFormat::TripleBuffer);

    QCoreApplication *app = QApplication::instance();

    QSharedPointer<Renderer> gl_renderer(new Renderer(format));
    glwidget = new OpenGLWindow(gl_renderer);
    renderer = gl_renderer.data();
    QWidget *container = QWidget::createWindowContainer(glwidget);

    //container->setAttribute(Qt::WA_NoSystemBackground);
    //container->setAttribute(Qt::WA_OpaquePaintEvent);
    //setAttribute(Qt::WA_NoSystemBackground);
    //setAttribute(Qt::WA_OpaquePaintEvent);

    QSize windowSize(400, 320);
    container->setMinimumSize(windowSize);
    container->setFocusPolicy(Qt::StrongFocus);

    render_thread = new QThread;
    renderer->moveToThread(render_thread);

    QObject::connect(app, SIGNAL(lastWindowClosed()), render_thread, SLOT(quit()));
    render_thread->start();

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setMargin(0);
    layout->addWidget(container);
    setLayout(layout);

}

