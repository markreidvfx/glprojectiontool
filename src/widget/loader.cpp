#include "loader.h"
#include "../scene/imagereader.h"
#include <QTimer>
#include <QThread>

#include <iostream>

Loader::Loader(Scene *scene, QObject *parent) : QObject(parent)
{
    m_scene = scene;
}

void Loader::wait_for_condition()
{
    m_lock.lock();
    wait_condition.wait(&m_lock);
    m_lock.unlock();
}

void Loader::set_imageplane_path(QString path, int frame)
{

    QPair <QString, int > p;

    p.first = path;
    p.second = frame;

    QMutexLocker locker(&m_lock);
    m_imagelane_list.append(p);
    locker.unlock();


    QTimer::singleShot(0, this, SLOT(load_imageplane()));
}
void Loader::set_template_texture(QString path)
{
    std::cerr << "setting texture " << path.toStdString() << "\n";
    std::vector<float> data;

    int width;
    int height;

    read_image(path.toStdString(), data, width, height);
    emit template_texture_ready(data, width, height);

}

void Loader::set_subdivision_level(int value)
{
    std::cerr << value << "\n";
    //m_scene->caculate();
    m_scene->setSubdivLevel(value);
    m_scene->caculate(m_scene->time());
    emit update_mesh();
}

void Loader::create_template(QString imageplane_path, QString dest, int frame)
{

    FloatImageData color;
    FloatImageData alpha;
    FloatImageData contour;

    progress.set_value(5);

    m_scene->caculate(frame);

    progress.set_value(10);

    emit request_template_textures(color, alpha, contour, frame);

    progress.set_value(25);

    std::cerr << color.width << "\n";

    write_template_psd(imageplane_path.toStdString(),
                       dest.toStdString(),
                       color,
                       alpha,
                       contour,
                       progress);


    //QThread::currentThread()->sleep(3);

    progress.set_value(100);
    emit projection_template_complete(imageplane_path, dest, frame);
}

void Loader::load_imageplane()
{
    QPair <QString, int > p;
    QMutexLocker locker(&m_lock);

    if (m_imagelane_list.empty())
        return;

    p = m_imagelane_list.last();
    m_imagelane_list.clear();
    locker.unlock();

    std::cerr << p.first.toStdString() << " " << p.second << "\n";
    //QThread::currentThread()->sleep(3);

    std::vector<float> data;

    int width;
    int height;

    //read_image(p.first.toStdString(), data, width, height);

    FloatImageData image;
    read_image(p.first.toStdString(), image);

    m_scene->caculate(p.second);

    // this signal should block
    emit imageplane_ready(image.data, image.width, image.height, p.second);

}
