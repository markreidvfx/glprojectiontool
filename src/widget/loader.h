#ifndef LOADER_H
#define LOADER_H

#include <QObject>
#include <QMutex>
#include <QList>
#include <QPair>
#include <QSharedPointer>

#include <vector>

typedef std::vector < float > FloatImage;

class Loader : public QObject
{
    Q_OBJECT
public:
    explicit Loader(QObject *parent = 0);
    void set_imageplane_path(QString path, int frame);

signals:
    void imageplane_ready( const FloatImage &data, int width, int height, int frame);
    void template_texture_ready(const FloatImage &data, int width, int height);

public slots:
    void set_template_texture(QString path);

private slots:
    void load_imageplane();

private:
    QMutex m_imageplane_lock;
    QList< QPair < QString, int > > m_imagelane_list;

};

Q_DECLARE_METATYPE(FloatImage)

#endif // LOADER_H
