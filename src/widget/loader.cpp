#include "loader.h"
#include "../scene/imagereader.h"
#include <QTimer>
#include <QThread>
#include <QFileInfo>
#include <QStringList>
#include <QDir>
#include <iostream>
#include <QThread>

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

static QDir stringlist_join(QString root, QStringList subdirs)
{
    QDir dest_dir(root);

    for (int i =0; i < subdirs.size(); i++) {
        dest_dir = QDir(dest_dir).filePath(subdirs.at(i));
    }

    return dest_dir;
}

void Loader::create_template(QString imageplane_path, QString project, int frame)
{

    FloatImageData color;
    FloatImageData alpha;
    FloatImageData contour;

    std::vector<FloatImageData> color_tiles;
    std::vector<FloatImageData> alpha_tiles;
    std::vector<FloatImageData> contour_tiles;

    progress.set_value(5);
    double seconds = frame  / 24.0;

    m_scene->caculate(seconds);

    progress.set_value(10);
    int tiles = 4;
    color_tiles.resize(tiles*tiles);
    alpha_tiles.resize(tiles*tiles);
    contour_tiles.resize(tiles*tiles);

    for (int i =0; i < tiles * tiles; i++) {
        emit request_template_data_tiled(color_tiles, alpha_tiles, contour_tiles, seconds, tiles, i);

        progress.set_value(10 + i*2);
        QThread::currentThread()->msleep(2);

    }
    //montage_tiles(contour_tiles, tiles);
    //emit request_template_textures(color, alpha, contour, frame);

    progress.set_value(45);

    std::cerr << color.width << "\n";

    QFileInfo info(imageplane_path);
    QString basename = info.completeBaseName();

    if (basename.isEmpty())
        basename = "template";

    std::cerr << "base" <<basename.toStdString() << "\n";
    QString dest_name = basename + ".psd";

    QStringList subdirs;
    QString frame_string;
    frame_string.sprintf("%04d", frame);

    subdirs << "sourceimages" << "projections" << frame_string << "gltool_template";

    QDir template_root = stringlist_join(project, subdirs);
    template_root.mkpath(template_root.path());
    template_root.mkdir("data");
    QDir data_root = template_root.filePath("data");
    template_root.mkdir("work");
    QDir work_root = template_root.filePath("work");

    QString dest = work_root.filePath(dest_name);

    std::cerr << dest.toStdString() << "\n";

    write_template_psd(info.exists() ? imageplane_path.toStdString(): "",
                       dest.toStdString(),
                       data_root.path().toStdString(),
                       color_tiles,
                       alpha_tiles,
                       contour_tiles,
                       tiles,
                       progress);

    //m_scene->export_camera(data_root.filePath("camera.abc").toStdString(), seconds);
    m_scene->export_mesh(data_root.filePath("mesh.abc").toStdString(), seconds);
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

    std::cerr << "image plane " << p.first.toStdString() << " " << p.second << "\n";
    //QThread::currentThread()->sleep(3);

    std::vector<float> data;

    int width;
    int height;

    double seconds = p.second / 24.0;

    //read_image(p.first.toStdString(), data, width, height);

    FloatImageData image;
    std::cerr << "reading \n";
    read_image(p.first.toStdString(), image);

    std::cerr << "subdividing\n";

    m_scene->caculate(seconds);

    std::cerr << "sending \n";

    // this signal should block
    emit imageplane_ready(image.data, image.width, image.height, seconds);

    std::cerr << "done\n";

}
