#include "rc_helper.h"
#include <QFile>
#include <QTextStream>

#include <QDebug>

#include <iostream>


bool read_rc_data(const char *path,
                  std::string &result)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        std::cerr << " error reading file: " << path << "\n";
        return false;
    }

    QTextStream in(&f);
    QString data = in.readAll();
    result = data.toStdString();

    return true;
}
