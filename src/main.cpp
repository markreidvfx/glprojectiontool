
#include <QApplication>
#include <QLabel>
#include <QSurfaceFormat>
#include <QCommandLineParser>
#include <iostream>

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

    const QCommandLineOption template_option(QStringList() << "t" << "template", "template texture.", "template.png");
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

    app.setApplicationName("projector");
    app.setApplicationVersion("0.1");

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

    std::cerr << "imageplane " << options.imageplane.toStdString() << "\n";
    widget.set_imageplane(options.imageplane);

    for (int i = 0; i < options.scene_files.size(); i++) {
        std::cerr << "opening " << options.scene_files[i].toStdString() << "\n";
        widget.open(options.scene_files[i]);
    }

    widget.resize(QSize(1200,600));
    widget.show();

    return app.exec();
}
