#ifndef PROJECTOR_H
#define PROJECTOR_H

#include <QWidget>
#include <vector>

namespace Ui {
class Projector;
}

enum SelectionFilter {
    ShowSelected,
    ShowOnlySelected,
    HideSelected
};
enum BrowseFilter {
    Imageplane,
    Project,
    AddMesh,
};

class Projector : public QWidget
{
    Q_OBJECT

public:
    explicit Projector(QWidget *parent = 0);
    ~Projector();


public slots:
    void open(const QString &path);

    void sceneDataLoaded(QString path);
    void updateScene();
    void showSceneViewContextMenu(const QPoint&);
    void setFrameRange(int first, int last);

    void on_browse_imageplane_clicked() {browse_file(Imageplane);}
    void on_browse_project_clicked() {browse_file(Project);}
    void on_browse_mesh_clicked(){browse_file(AddMesh);}

    void set_imageplane(const QString &path);
    void set_project(const QString &path) {}
    void create_templates();

    void hide_progress();
    void show_progress(QString message, int min, int max, int value);
    void cancel_processing();

private slots:
    void hide_selected();
    void show_selected();
    void show_only_selected();
    void updateFrameRange();
    void frameChange(int value);
    void add_current_frame();
    void next_template();
    void check_progress();
    void update();

signals:
    void frameChanged(int value);
    void imageplaneChanged(QString path);

    void request_template(QString imageplane,
                          QString dest,
                          int frame);

    void set_template_texture(QString path);
    void set_subdivision_level(int value);
    void open_scene_file(QString path);

private:
    void keyPressEvent(QKeyEvent * event) Q_DECL_OVERRIDE;

    Ui::Projector *ui;
    void filter_selected(SelectionFilter f);
    void browse_file(BrowseFilter f);

    bool m_cancel_processing;
    std::vector<int> m_projection_frames;
    int m_projection_count;
};

#endif // PROJECTOR_H
