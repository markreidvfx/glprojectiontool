
#include <QApplication>
#include <QLabel>
#include <QSurfaceFormat>
#include <QCommandLineParser>
#include <iostream>
#include <stdio.h>
#include <QMainWindow>
#include <QDir>
#include "scene/imagereader.h"

#ifndef QT_NO_OPENGL
#include "projector.h"
#endif

struct ProjectorOptions
{
    QString imageplane;
    QString template_path;
    QString project_path;
    QStringList scene_files;
};

enum CommandLineParseResult
{
    CommandLineOk,
    CommandLineError,
    CommandLineVersionRequested,
    CommandLineHelpRequested,
};

CommandLineParseResult parseCommandLine(QCommandLineParser &parser, ProjectorOptions &options, QString &errorMessage)
{
    parser.setApplicationDescription(QCoreApplication::translate(
                                         "Projector", "Projection template tool."));

    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);

    const QCommandLineOption imageplane_option(QStringList() << "i" << "imageplane", "use <image seqence> as imageplane.", "image sequence");
    parser.addOption(imageplane_option);

    const QCommandLineOption project_option(QStringList() << "p" << "project", "project directory, were to output images too.", "project directory");
    parser.addOption(project_option);

    QCommandLineOption template_option(QStringList() << "t" << "template", "template texture.", "template.png");
    template_option.setDefaultValue(":/textures/dog_guides.png");
    parser.addOption(template_option);

    const QCommandLineOption helpOption = parser.addHelpOption();
    const QCommandLineOption versionOption = parser.addVersionOption();

    parser.addPositionalArgument("files", "Files to open", "[files ...]");

    if (!parser.parse(QCoreApplication::arguments())) {
        errorMessage = parser.errorText();
        return CommandLineError;
    }
    if (parser.isSet(versionOption))
        return CommandLineVersionRequested;

    if (parser.isSet(helpOption))
        return CommandLineHelpRequested;

    options.imageplane = parser.value(imageplane_option);
    options.project_path = parser.value(project_option);
    options.template_path = parser.value(template_option);
    options.scene_files = parser.positionalArguments();

    return CommandLineOk;
}



int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName("glprojectiontool");
    app.setApplicationVersion("0.1");
    app.setWindowIcon(QIcon(":/textures/icon.png"));

    qRegisterMetaType<std::vector < float > > ("FloatImage");
    qRegisterMetaType<FloatImageData>("FloatImageData");


    QSurfaceFormat format;
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setSamples(8);
    format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    QSurfaceFormat::setDefaultFormat(format);


    QCommandLineParser parser;
    QString errorMessage;
    ProjectorOptions options;

    switch (parseCommandLine(parser, options, errorMessage)) {

    case CommandLineOk:
        break;
    case CommandLineError:
        fputs(qPrintable(errorMessage), stderr);
        fputs("\n\n", stderr);
        fputs(qPrintable(parser.helpText()), stderr);
        return 1;
    case CommandLineVersionRequested:
        printf("%s %s\n", qPrintable(QCoreApplication::applicationName()),
               qPrintable(QCoreApplication::applicationVersion()));
        return 0;
    case CommandLineHelpRequested:
        parser.showHelp();
        return 0;
    }

    Projector widget;
    //widget.setAttribute(Qt::WA_NoSystemBackground);
    //widget.setAttribute(Qt::WA_OpaquePaintEvent);
    //widget.setAttribute(Qt::WA_NativeWindow);

    //widget.setAttribute(Qt::WA_PaintOnScreen);

    std::cerr << "imageplane " << options.imageplane.toStdString() << "\n";
    widget.set_imageplane(options.imageplane);

    if (!options.template_path.isEmpty())
        widget.set_template_texture(options.template_path);

    for (int i = 0; i < options.scene_files.size(); i++) {
        std::cerr << "opening " << options.scene_files[i].toStdString() << "\n";
        widget.open(options.scene_files[i]);
    }

    QString project_path = options.project_path;

    if (project_path.isEmpty())
        project_path = QDir::currentPath();

    std::cerr << "project " << project_path.toStdString() << "\n";
    widget.set_project(project_path);

    widget.resize(QSize(1200,600));
    widget.show();
    //QMainWindow window;
    //window.setCentralWidget(&widget);
    //window.show();

    return app.exec();
}
