#include "application.h"
#include <iostream>

bool Application::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::FileOpen:
        emit file_open_event(static_cast<QFileOpenEvent *>(event)->url());
        return true;
    default:
        return QApplication::event(event);
    }

}

