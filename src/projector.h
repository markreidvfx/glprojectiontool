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
    TemplateTexture,
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

    void set_imageplane(const QString &path);
    void set_project(const QString &path);
    void create_templates();

    void hide_progress();
    void show_progress(QString message, int min, int max, int value);
    void cancel_processing();
    void set_template_texture(QString path);
    void file_open_event(QUrl url);

private slots:
    void hide_selected();
    void show_selected();
    void show_only_selected();
    void updateFrameRange();
    void frameChange(int value);
    void add_current_frame();
    void next_template();
    void check_progress();
    void update_all();
    void browse_imageplane_clicked() {browse_file(Imageplane);}
    void browse_project_clicked() {browse_file(Project);}
    void browse_mesh_clicked(){browse_file(AddMesh);}
    void browse_template_texure_clicked(){browse_file(TemplateTexture);}

    void projection_template_complete(QString imageplane, QString dest, int frame);
    void handle_files(const QList<QUrl> &url_list);
    void open_project_file(QString path);

    void error_message(QString message);

signals:
    void frameChanged(int value);
    void imageplaneChanged(QString path);

    void request_template(QString imageplane,
                          QString project,
                          QString output_dir,
                          QString guides,
                          int frame);

    void request_template_texture(QString path);
    void set_subdivision_level(int value);
    void open_scene_file(QString path);
    void clear();

private:
    void keyPressEvent(QKeyEvent * event) Q_DECL_OVERRIDE;
    void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
    void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;

    Ui::Projector *ui;
    void filter_selected(SelectionFilter f);
    void browse_file(BrowseFilter f);

    bool m_cancel_processing;
    std::vector<int> m_projection_frames;
    int m_projection_count;
};

#endif // PROJECTOR_H
