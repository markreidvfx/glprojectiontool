#include "loader.h"
#include "../scene/imagereader.h"
#include <QTimer>
#include <QThread>

#include <iostream>

Loader::Loader(QObject *parent) : QObject(parent)
{

}

void Loader::set_imageplane_path(QString path, int frame)
{

    QPair <QString, int > p;

    p.first = path;
    p.second = frame;

    QMutexLocker locker(&m_imageplane_lock);
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

void Loader::load_imageplane()
{
    QPair <QString, int > p;
    QMutexLocker locker(&m_imageplane_lock);

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

    read_image(p.first.toStdString(), data, width, height);

    // this signal should block
    emit imageplane_ready(data, width, height, p.second);

}
