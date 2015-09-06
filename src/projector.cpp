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
#include "scene/scene.h"


Projector::Projector(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Projector)
{
    ui->setupUi(this);

    OpenGLWindow *glwidget = ui->projector->glwidget;
    Renderer *renderer = ui->projector->renderer;

    QTreeWidget *scene_view = ui->scene_view;
    QComboBox *camera_select = ui->camera_select;

    renderer->setTime(0);

    scene_view->setContextMenuPolicy(Qt::CustomContextMenu);
    scene_view->header()->setSectionResizeMode(0, QHeaderView::Stretch);

    connect(renderer, SIGNAL(sceneLoaded(QString)),
            this, SLOT(sceneDataLoaded(QString)));

    connect(camera_select, SIGNAL(currentIndexChanged(int)),
            this, SLOT(updateScene()));
    //connect(scene_view, SIGNAL(itemSelectionChanged()),
    //        this, SLOT(updateScene()));
    connect(scene_view, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
            this, SLOT(updateScene()));

    connect(scene_view, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showSceneViewContextMenu(QPoint)));

    connect(ui->browse_imageplane_button, SIGNAL(clicked(bool)),
            this, SLOT(on_browse_imageplane_clicked()));
    connect(ui->browse_mesh_button, SIGNAL(clicked(bool)),
            this, SLOT(on_browse_mesh_clicked()));
    connect(ui->browse_project_button, SIGNAL(clicked(bool)),
            this, SLOT(on_browse_project_clicked()));

    connect(ui->imageplane_path, SIGNAL(editingFinished()),
            this, SLOT(update()));

    connect(ui->add_current_frame, SIGNAL(clicked(bool)),
            this, SLOT(add_current_frame()));

    //time attributes

    connect(ui->currentTime, SIGNAL(valueChanged(int)),
            this, SLOT(frameChange(int)));

    connect(renderer, SIGNAL(frameRangeChanged(int,int)),
            this, SLOT(setFrameRange(int,int)));

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
            renderer, SLOT(create_template(QString, QString, int)));

    connect(renderer, SIGNAL(template_complete(QString,QString,int)),
            this, SLOT(next_template()));

    connect(ui->progress_cancel, SIGNAL(clicked(bool)),
            this, SLOT(cancel_processing()));

    connect(this, SIGNAL(set_template_texture(QString)),
            ui->projector->loader, SLOT(set_template_texture(QString)));

    QString template_texture = "/home/mreid/Projects/Samples/guides.tif";
   // QTimer::singleShot(0 )
    emit set_template_texture(template_texture);

    hide_progress();
}

Projector::~Projector()
{
    delete ui;
}

void Projector::open(const QString &path)
{
    OpenGLWindow *glwidget = ui->projector->glwidget;
    glwidget->open(path);
}

void Projector::browse_file(BrowseFilter f)
{
    QString path;

    if(f == Imageplane) {
        path = QFileDialog::getOpenFileName(this,
                            tr("Open Image Sequence"), ui->imageplane_path->text(),
                            tr("Image Files (*.dpx *.jpg *.png *.exr)"));
        if(!path.isNull())
            set_imageplane(path);

    } else if (f == AddMesh) {
        path = QFileDialog::getOpenFileName(this,
                            tr("Add Alembic File"), "",
                            tr("ABC Files (*.abc)"));

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

void Projector::update()
{
    frameChange(ui->currentTime->value());
}

void Projector::sceneDataLoaded(QString path)
{
    std::cerr << "scene loaded " << path.toStdString() << "\n";

    std::vector<SceneObject> scene_objects;
    std::vector<SceneObject> cameras;

    Renderer *renderer = ui->projector->renderer;
    QTreeWidget *scene_view = ui->scene_view;
    QComboBox *camera_select = ui->camera_select;

    scene_view->clear();
    scene_objects = renderer->m_scene.scene_objects();
    cameras =  renderer->m_scene.scene_cameras();

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


    Renderer *renderer = ui->projector->renderer;
    QTreeWidget *scene_view = ui->scene_view;
    QComboBox *camera_select = ui->camera_select;

    std::vector<SceneObject> scene_objects(scene_view->topLevelItemCount());
    std::vector<SceneObject> cameras;

    QString camera = camera_select->currentText();
    if (camera != "UV") {
        SceneObject c = {camera.toStdString(), true };
        cameras.push_back(c);
        renderer->m_scene.update_cameras(cameras);
    }

    for (int i =0; i < scene_view->topLevelItemCount(); i++){
        QTreeWidgetItem *item = scene_view->topLevelItem(i);
        scene_objects[i].name = item->text(0).toStdString();
        scene_objects[i].visible = item->checkState(1) == Qt::Checked? true: false;
        scene_objects[i].selected = item->isSelected();
    }
    renderer->m_scene.update_objects(scene_objects);
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

    QString dest;
    dest.sprintf("test.%04d.psd", frame);

    emit request_template(imageplane, dest, frame);

    frameChange(frame);

    QString message;
    message.sprintf("Creating template frame: %d %02d/%02d ", frame,
                                                       m_projection_count - m_projection_frames.size(),
                                                       m_projection_count);
    show_progress(message, 0, 100, 0);
    QTimer::singleShot(1000, this, SLOT(check_progress()));
}

void Projector::check_progress()
{

    if (m_cancel_processing){
       return;
    }

    int value = ui->projector->renderer->m_scene.progress();
    ui->progress->setValue(value);

    QTimer::singleShot(1000, this, SLOT(check_progress()));

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
