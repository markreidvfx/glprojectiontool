#ifndef LOADER_H
#define LOADER_H

#include <QObject>
#include <QMutex>
#include <QList>
#include <QPair>
#include <QSharedPointer>
#include <QWaitCondition>

#include "../scene/imagereader.h"
#include "../scene/scene.h"

#include <vector>

typedef std::vector < float > FloatImage;

class Loader : public QObject
{
    Q_OBJECT
public:
    explicit Loader(Scene *scene, QObject *parent = 0);
    void set_imageplane_path(QString path, int frame);
    Progress progress;
    QWaitCondition wait_condition;

signals:
    void imageplane_ready( const FloatImage &data, int width, int height, double seconds);
    void template_texture_ready(const FloatImage &data, int width, int height);
    void projection_template_complete(QString imageplane_path, QString project,int frame);
    void request_template_data_tiled(std::vector<FloatImageData> &color_data,
                                     std::vector<FloatImageData> &alpha_data,
                                     std::vector<FloatImageData> &contour_data,
                                     double seconds, int tiles, int index);
    void update_mesh();
    void request_open_scene_file(QString path);
    void request_clear();

public slots:
    void wait_for_condition();
    void set_template_texture(QString path);
    void create_template(QString imageplane_path, QString project, QString  output_dir, QString guides, int frame);
    void set_subdivision_level(int value);
    void open_scene_file(QString path){emit request_open_scene_file(path);}
    void clear(){emit request_clear();}

private slots:
    void load_imageplane();

private:
    QMutex m_lock;
    QList< QPair < QString, int > > m_imagelane_list;
    Scene *m_scene;

};

Q_DECLARE_METATYPE(FloatImage)
Q_DECLARE_METATYPE(FloatImageData)

#endif // LOADER_H
