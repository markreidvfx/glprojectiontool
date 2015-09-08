#ifndef LOADER_H
#define LOADER_H

#include <QObject>
#include <QMutex>
#include <QList>
#include <QPair>
#include <QSharedPointer>
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

signals:
    void imageplane_ready( const FloatImage &data, int width, int height, int frame);
    void template_texture_ready(const FloatImage &data, int width, int height);
    void projection_template_complete(QString imageplane_path, QString dest,int frame);
    void request_template_textures(FloatImageData &color_data,
                                   FloatImageData &alpha_data,
                                   FloatImageData &contour_data,
                                   int frame);

public slots:
    void set_template_texture(QString path);
    void create_template(QString imageplane_path, QString dest, int frame);

private slots:
    void load_imageplane();

private:
    QMutex m_imageplane_lock;
    QList< QPair < QString, int > > m_imagelane_list;
    Scene *m_scene;

};

Q_DECLARE_METATYPE(FloatImage)
Q_DECLARE_METATYPE(FloatImageData)

#endif // LOADER_H
