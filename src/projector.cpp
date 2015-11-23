#include "projector.h"
#include "ui_projector.h"
#include <QTimer>
#include <QTreeWidget>
#include <QMenu>
#include <QSlider>
#include <QSpinBox>
#include <QFileDialog>
#include <QFileInfo>
#include <iostream>
#include <algorithm>
#include <QDebug>
#include <QMimeData>
#include <QProcess>
#include <QDesktopServices>
#include <QMessageBox>

#include "scene/scene.h"


Projector::Projector(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Projector)
{
    ui->setupUi(this);
	setAcceptDrops(true);

    OpenGLWidget *glwidget = ui->projector->glwidget;
    //Renderer *renderer = ui->projector->renderer;

    QTreeWidget *scene_view = ui->scene_view;
    QComboBox *camera_select = ui->camera_select;

    //renderer->setTime(0);

    scene_view->setContextMenuPolicy(Qt::CustomContextMenu);
    scene_view->header()->setSectionResizeMode(0, QHeaderView::Stretch);

    connect(camera_select, SIGNAL(currentIndexChanged(int)),
            this, SLOT(updateScene()));
    //connect(scene_view, SIGNAL(itemSelectionChanged()),
    //        this, SLOT(updateScene()));
    connect(scene_view, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
            this, SLOT(updateScene()));

    connect(scene_view, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showSceneViewContextMenu(QPoint)));

    connect(ui->browse_imageplane_button, SIGNAL(clicked(bool)),
            this, SLOT(browse_imageplane_clicked()));
    connect(ui->browse_mesh_button, SIGNAL(clicked(bool)),
            this, SLOT(browse_mesh_clicked()));
    connect(ui->browse_project_button, SIGNAL(clicked(bool)),
            this, SLOT(browse_project_clicked()));
    connect(ui->browse_template_button, SIGNAL(clicked(bool)),
            this, SLOT(browse_template_texure_clicked()));

    connect(ui->clear_button, SIGNAL(clicked(bool)),
            ui->projector->loader, SLOT(clear()));

    connect(ui->imageplane_path, SIGNAL(editingFinished()),
            this, SLOT(update_all()));

    connect(ui->add_current_frame, SIGNAL(clicked(bool)),
            this, SLOT(add_current_frame()));

    //time attributes

    connect(ui->currentTime, SIGNAL(valueChanged(int)),
            this, SLOT(frameChange(int)));

    connect(glwidget, SIGNAL(frame_range_changed(int,int)),
            this, SLOT(setFrameRange(int,int)));

    connect(glwidget, SIGNAL(scene_loaded(QString)),
            this, SLOT(sceneDataLoaded(QString)));

    connect(ui->timeSlider, SIGNAL(valueChanged(int)),
            ui->currentTime, SLOT(setValue(int)));
    connect(ui->currentTime, SIGNAL(valueChanged(int)),
            ui->timeSlider, SLOT(setValue(int)));


    connect(ui->first, SIGNAL(valueChanged(int)),
            this, SLOT(updateFrameRange()));
    connect(ui->last, SIGNAL(valueChanged(int)),
            this, SLOT(updateFrameRange()));

    // procedures

    connect(ui->export_templates, SIGNAL(clicked(bool)),
            this, SLOT(create_templates()));
    connect(this,  SIGNAL(request_template(QString, QString, int)),
            ui->projector->loader, SLOT(create_template(QString, QString, int)));

    connect(ui->projector->loader, SIGNAL(projection_template_complete(QString,QString,int)),
            this, SLOT(projection_template_complete(QString,QString,int)));

    connect(ui->progress_cancel, SIGNAL(clicked(bool)),
            this, SLOT(cancel_processing()));

    connect(ui->template_texture_path, SIGNAL(textChanged(QString)),
            ui->projector->loader, SLOT(set_template_texture(QString)));

    connect(this, SIGNAL(set_subdivision_level(int)),
             ui->projector->loader, SLOT(set_subdivision_level(int)));

    connect(this, SIGNAL(open_scene_file(QString)),
            ui->projector->loader, SLOT(open_scene_file(QString)));

    hide_progress();

    emit ui->template_texture_path->textChanged("");
}

Projector::~Projector()
{
    delete ui;
}

void Projector::open(const QString &path)
{
    //OpenGLWindow *glwidget = ui->projector->glwidget;
    //glwidget->open(path);
    emit open_scene_file(path);
    //ui->projector->glwidget->open_abc(path);
}

void Projector::keyPressEvent(QKeyEvent * event)
{
    if (event->key() == Qt::Key_1){
        emit set_subdivision_level(0);
    } else if (event->key() == Qt::Key_2) {
        emit set_subdivision_level(1);
    }else if (event->key() == Qt::Key_3) {
        emit set_subdivision_level(2);
    }else if (event->key() == Qt::Key_4) {
        emit set_subdivision_level(3);
    }
}

void Projector::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }

}
void Projector::file_open_event(QUrl url)
{
    QList<QUrl> url_list;
    url_list << url;
    handle_files(url_list);
}
void Projector::handle_files(const QList<QUrl> &url_list)
{
    QStringList image_plane_formats;
    QStringList geo_formats;
    QStringList project_formats;

    image_plane_formats << "dpx" << "tif" << "tiff" << "jpg" << "png";
    geo_formats << "abc";

    project_formats << "glpt";

    for (int i=0; i < url_list.size(); i++) {
        QUrl u = url_list.at(i);
        if (!u.isLocalFile())
            continue;

        //std::cerr << u.toString().toStdString() << "\n";
        QFileInfo info(u.toLocalFile());
        if (info.isDir()) {
            QStringList name_filter;
            name_filter << "*.dpx";
            QDir directory(u.toLocalFile());
            QStringList image_files = directory.entryList(name_filter);

            if (!image_files.size())
                continue;

            QString file_path = directory.filePath(image_files.at(0));
            std::cerr << file_path.toStdString() << "\n";
            info = file_path;

        }

        QString suffix = info.suffix().toLower();

        std::cerr << info.suffix().toLower().toStdString() << "\n";
        std::cerr << info.absoluteFilePath().toStdString() << "\n";

        if (project_formats.contains(suffix, Qt::CaseInsensitive)) {
            open_project_file(info.absoluteFilePath());
        }

        if (geo_formats.contains(suffix, Qt::CaseInsensitive)) {
            open(info.absoluteFilePath());
        }

        if (image_plane_formats.contains(suffix, Qt::CaseInsensitive)) {
            set_imageplane(info.absoluteFilePath());
        }
    }
}

void Projector::open_project_file(QString path)
{
    QMessageBox msgBox;
    msgBox.setText(path);
    msgBox.exec();
}

void Projector::dropEvent(QDropEvent *event)
{
    QList<QUrl> url_list = event->mimeData()->urls();
    handle_files(url_list);
    event->acceptProposedAction();
}

void Projector::browse_file(BrowseFilter f)
{
    QString path;

    if(f == Imageplane) {
        path = QFileDialog::getOpenFileName(this,
                            tr("Open Image Sequence"), ui->imageplane_path->text(),
                            tr("Image Files (*.dpx *.jpg *.png *.exr *.tif *.tiff)"));
        if(!path.isNull())
            set_imageplane(path);

    } else if (f == AddMesh) {
        path = QFileDialog::getOpenFileName(this,
                            tr("Add Alembic File"), "",
                            tr("ABC Files (*.abc)"));
        if(!path.isNull())
            open(path);

    } else if (f == TemplateTexture) {
        path = QFileDialog::getOpenFileName(this,
                            tr("Open Template Image"), ui->imageplane_path->text(),
                            tr("Image Files (*.dpx *.jpg *.png *.exr *.tif *.tiff)"));
        if(!path.isNull())
            set_template_texture(path);

    } else if( f == Project) {
        path = QFileDialog::getExistingDirectory(this,tr("Set Project directory"));

        if(!path.isNull())
            ui->project_path->setText(path);
    }

}

static int get_padding(const QString &str)
{
    int pad = 0;
    for(int i = str.length() - 1; i > 0; i--) {
        if(!str.at(i).isDigit())
            break;
        pad++;

    }
    return pad;
}

static QString get_formatting_str(const QString str)
{

    QFileInfo info(str);
    QString ext = info.suffix();
    QDir dirname = info.dir();
    QString basename = info.completeBaseName();
    int pad = get_padding(basename);
    if (!pad)
        return str;

    basename.remove(basename.length() - pad, basename.length());
    QStringList filters;
    filters << basename + "*." + ext;
    QStringList files = dirname.entryList(filters);

    if (files.filter(QRegExp(basename + "\\d+." + ext )).length() <= 1)
        return str;

    basename.append("%0" + QString::number(pad) + "d." + ext );
    return dirname.filePath(basename);
}

void Projector::set_imageplane(const QString &path)
{
    QFileInfo file_info(path);
    QString ext = file_info.suffix();
    QString basename = file_info.completeBaseName();

    if (!basename.contains("%")) {
        std::cerr << ext.toStdString() << " sequence\n";
        ui->imageplane_path->setText(get_formatting_str(path));

    } else {
        ui->imageplane_path->setText(path);
    }

    update_all();
}
void Projector::set_template_texture(QString path)
{
    ui->template_texture_path->setText(path);
    //emit request_template_texture(path);
}

void Projector::set_project(const QString &path)
{
    ui->project_path->setText(path);
}

void Projector::setFrameRange(int first, int last)
{
    std::cerr << "frame range: " << first << " " << last << "\n";

    QSpinBox *f = ui->first;
    QSpinBox *l = ui->last;
    QSlider *timeslider = ui->timeSlider;

    f->setValue(first);
    l->setValue(last);
    ui->currentTime->setValue(first);

}

void Projector::updateFrameRange()
{
    ui->timeSlider->setRange(ui->first->value(), ui->last->value());
}
void Projector::frameChange(int value)
{
    QString path;
    path.sprintf(ui->imageplane_path->text().toStdString().c_str(), value);


    ui->projector->loader->set_imageplane_path(path, value);

    //ui->projector->renderer->setImagePlanePath(path, double(value));

    ui->currentTime->blockSignals(true);
    ui->timeSlider->blockSignals(true);

    ui->currentTime->setValue(value);
    ui->timeSlider->setValue(value);

    ui->currentTime->blockSignals(false);
    ui->timeSlider->blockSignals(false);
}

void Projector::update_all()
{
    frameChange(ui->currentTime->value());
}

void Projector::sceneDataLoaded(QString path)
{
    std::cerr << "scene loaded " << path.toStdString() << "\n";

    std::vector<SceneObject> scene_objects;
    std::vector<SceneObject> cameras;

    OpenGLWidget *renderer = ui->projector->glwidget;
    QTreeWidget *scene_view = ui->scene_view;
    QComboBox *camera_select = ui->camera_select;

    scene_view->clear();
    scene_objects = ui->projector->scene.scene_objects();
    cameras =  ui->projector->scene.scene_cameras();

    QList<QTreeWidgetItem *> items;
    for (int i = 0; i < scene_objects.size(); ++i) {
        std::cerr << "  " << scene_objects[i].name << "\n";
        QTreeWidgetItem * item= new QTreeWidgetItem((QTreeWidget*)0);

        item->setText(0, QString(scene_objects[i].name.c_str()));
        item->setCheckState(1, scene_objects[i].visible? Qt::Checked: Qt::Unchecked);
        items.append(item);
    }
    scene_view->insertTopLevelItems(0, items);
    camera_select->clear();
    int current_index = 0;
    for (int i = 0; i < cameras.size(); ++i) {

        if (cameras[i].visible)
            current_index = i;

        camera_select->addItem(QString(cameras[i].name.c_str()));
    }
    camera_select->addItem(QString("UV"));
    camera_select->setCurrentIndex(current_index);
}

void Projector::updateScene()
{
    std::cerr << "updating scene\n";


    OpenGLWidget *renderer = ui->projector->glwidget;
    QTreeWidget *scene_view = ui->scene_view;
    QComboBox *camera_select = ui->camera_select;

    std::vector<SceneObject> scene_objects(scene_view->topLevelItemCount());
    std::vector<SceneObject> cameras;

    QString camera = camera_select->currentText();
    if (camera != "UV") {
        SceneObject c = {camera.toStdString(), true };
        cameras.push_back(c);
        ui->projector->scene.update_cameras(cameras);
    }

    for (int i =0; i < scene_view->topLevelItemCount(); i++){
        QTreeWidgetItem *item = scene_view->topLevelItem(i);
        scene_objects[i].name = item->text(0).toStdString();
        scene_objects[i].visible = item->checkState(1) == Qt::Checked? true: false;
        scene_objects[i].selected = item->isSelected();
    }
    ui->projector->scene.update_objects(scene_objects);
    update_all();
    //ui->projector->glwidget->force_update();
}
static void qstring_int_list(QString &str, std::vector<int> &values)
{
    QStringList l = str.split(",");
    for (int i = 0; i < l.size(); i++) {
        bool ret = false;
        int v = l[i].toInt(&ret, 10);
        if (ret) {
            values.push_back(v);
            std::cerr << v << "\n";
        }
    }
}

static QString int_list_to_qstring(std::vector<int> &values)
{
    QStringList l;
    QString str;
    for (int i = 0; i < values.size(); i++) {

        l.append(QString::number(values[i], 10));

    }
    return l.join(", ");
}

void Projector::add_current_frame()
{
    int frame = ui->currentTime->value();
    std::cerr << "adding " << frame << std::endl;

    std::vector<int> values;
    QString s = ui->projection_frames->text();
    qstring_int_list(s, values);

    for (int i = 0; i < values.size(); i++) {
        if (values[i] == frame)
            return;
    }
    values.push_back(frame);
    ui->projection_frames->setText(int_list_to_qstring(values));
}

void Projector::create_templates()
{
    QString s = ui->projection_frames->text();
    m_projection_frames.clear();
    qstring_int_list(s, m_projection_frames);
    if (!m_projection_frames.size()) {
        add_current_frame();
        s = ui->projection_frames->text();
        qstring_int_list(s, m_projection_frames);
    }
    std::reverse(m_projection_frames.begin(), m_projection_frames.end());
    m_cancel_processing = false;
    m_projection_count = m_projection_frames.size();
    next_template();
}

static void reveal_file(const QString &file_path)
{

    QStringList args;
    args << "-e";
    args << "tell application \"Finder\"";
    args << "-e";
    args << "activate";
    args << "-e";
    args << "select POSIX file \""+file_path+"\"";
    args << "-e";
    args << "end tell";
    QProcess::startDetached("osascript", args);
/*
#ifdef Q_WS_WIN
    QStringList args;
    args << "/select," << QDir::toNativeSeparators(file_path);
    QProcess::startDetached("explorer", args);
#endif*/

}

void Projector::projection_template_complete(QString imageplane, QString dest, int frame)
{
    reveal_file(dest);
    next_template();
}

void Projector::next_template()
{
    if (!m_projection_frames.size() || m_cancel_processing){
        hide_progress();
        return;
    }

    int frame = m_projection_frames.back();
    m_projection_frames.pop_back();

    QString imageplane;
    imageplane.sprintf(ui->imageplane_path->text().toStdString().c_str(), frame);

    QString project = ui->project_path->text();

    //dest.sprintf("test.%04d.psd", frame);

    frameChange(frame);


    QString message;
    message.sprintf("Creating Template Frame: %d %02d/%02d ", frame,
                                                       m_projection_count - m_projection_frames.size(),
                                                       m_projection_count);
    show_progress(message, 0, 100, 0);
    QTimer::singleShot(50, this, SLOT(check_progress()));

    emit request_template(imageplane, project, frame);
}

void Projector::check_progress()
{

    if (m_cancel_processing){
       return;
    }

    int value = ui->projector->loader->progress.value();
    ui->progress->setValue(value);

    QTimer::singleShot(100, this, SLOT(check_progress()));

}

void Projector::cancel_processing()
{
    m_cancel_processing = true;
    show_progress("Canceling process, please wait...", 0, 0, -1);
}


void Projector::hide_progress()
{
    ui->progress_frame->hide();

    std::cerr << "progress hide\n";
    m_cancel_processing = true;
    bool enable = true;
    ui->path_controls->setEnabled(enable);
    ui->time_controls->setEnabled(enable);
    ui->projector->setEnabled(enable);
    ui->camera_group->setEnabled(enable);
    ui->mode_select->setEnabled(enable);
    ui->scene_view->setEnabled(enable);

}

void Projector::show_progress(QString message, int min, int max, int value)
{

    bool enable = false;
    ui->path_controls->setEnabled(enable);
    ui->time_controls->setEnabled(enable);
    ui->projector->setEnabled(enable);
    ui->camera_group->setEnabled(enable);
    ui->mode_select->setEnabled(enable);
    ui->scene_view->setEnabled(enable);

    ui->progress_frame->show();
    ui->progress_message->setText(message);
    ui->progress->setRange(min, max);
    ui->progress->setValue(value);

    std::cerr << "progress show\n";

}

void Projector::showSceneViewContextMenu(const QPoint& pos)
{
    std::cerr << "context menu\n";
    QTreeWidget *scene_view = ui->scene_view;
    QPoint globalPos = scene_view->mapToGlobal(pos);

    QMenu menu;
    QAction _show_selected("Show Selected",&menu);
    QAction _only_selected("Show Only Selected",&menu);
    QAction _hide_selected("Hide Selected",&menu);

    connect(&_show_selected, SIGNAL(triggered(bool)),
            this, SLOT(show_selected()));
    connect(&_only_selected, SIGNAL(triggered(bool)),
            this, SLOT(show_only_selected()));
    connect(&_hide_selected, SIGNAL(triggered(bool)),
            this, SLOT(hide_selected()));

    menu.addAction(&_show_selected);
    menu.addAction(&_only_selected);
    menu.addAction(&_hide_selected);

    QAction* selectedItem = menu.exec(globalPos);

}

void Projector::filter_selected(SelectionFilter f)
{
    QTreeWidget *scene_view = ui->scene_view;
    for (int i =0; i < scene_view->topLevelItemCount(); i++){
        QTreeWidgetItem *item = scene_view->topLevelItem(i);

        switch (f) {
        case HideSelected:
            if (item->isSelected())
                item->setCheckState(1, Qt::Unchecked);
            break;
        case ShowSelected:
            if (item->isSelected())
                item->setCheckState(1, Qt::Checked);
            break;
        case ShowOnlySelected:
            if (item->isSelected())
                item->setCheckState(1, Qt::Checked);
            else
                item->setCheckState(1, Qt::Unchecked);
            break;
        default:
            break;
        }
    }


}

void Projector::hide_selected()
{
    std::cerr << "hide selected\n";
    filter_selected(HideSelected);
}
void Projector::show_selected()
{
    std::cerr << "show selected\n";
    filter_selected(ShowSelected);
}
void Projector::show_only_selected()
{
    std::cerr << "show only selected\n";
    filter_selected(ShowOnlySelected);
}
