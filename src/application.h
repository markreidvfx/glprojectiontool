#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>
#include <QFileOpenEvent>
#include <QString>

class Application : public QApplication
{
Q_OBJECT
public:
    Application(int & argc, char **argv):QApplication(argc, argv) {}
protected:
    bool event(QEvent *event);

signals:
    void file_open_event(QUrl url);

public slots:
};

#endif // APPLICATION_H
