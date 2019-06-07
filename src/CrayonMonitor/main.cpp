#include <QApplication>
#include <QLibraryInfo>
#include <QTimer>
#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QAction>

#include <CrCore>
#include <CrProject>
#include <CrMonitor>
#include <CrNode>

#include "crmonitormainwindow.h"

#include <QDebug>

static CrProject* project = nullptr;
static QList<QWidget*> editors_;

void chooseProject();
void loadProject(const QString& path);

int main(int argc, char *argv[])
{

    QApplication appliaction(argc, argv);
    QObject::connect(&appliaction, &QApplication::aboutToQuit, &appliaction, []{
        if(!project)
            return ;

        QFile retainFile(project->path() + ".retains");
        if(retainFile.open(QIODevice::WriteOnly)){
            QDataStream stream(&retainFile);
            stream << project->saveRetainState();
            retainFile.close();
        }

        QSettings settings("CrayonEnt", "Crayon");
        for (int i = 0; i < editors_.size(); ++i) {
            QWidget* editor = editors_.at(i);
            settings.setValue("monitor_" + QString::number(i), editor->saveGeometry());
        }
    });

    CrCore::installTranslator(QLibraryInfo::location(QLibraryInfo::TranslationsPath), QLocale::system());

    CrCore::addPluginsPath(QLibraryInfo::location(QLibraryInfo::LibraryExecutablesPath));


    QTimer::singleShot(0, &appliaction, [&appliaction](){
        QStringList args = appliaction.arguments();

        for (int i = 1; i < args.size(); ++i) {
            QFileInfo fileInfo(args.at(i));

            if(fileInfo.exists() && fileInfo.suffix() == "cr"){
                loadProject(fileInfo.absoluteFilePath());
                return;
            }
        }
        chooseProject();
    });

    //    CrGraphicsEditor* editor = new CrGraphicsEditor();
    //    editor->show();
    return appliaction.exec();
}


void chooseProject() {

    static QFileDialog* dialog  = new QFileDialog();

    dialog->setAcceptMode(QFileDialog::AcceptOpen);
    dialog->setDefaultSuffix("cr");
    dialog->setNameFilter("Crayon project files (*.cr)");
    if(!dialog->exec()){
        return;
    }

    QStringList selectedFiles = dialog->selectedFiles();
    if(selectedFiles.isEmpty()){
        int result = QMessageBox::question(nullptr, QObject::tr("No project selected"),
                                           QObject::tr("Repeat project selection?"));

        if(result == QMessageBox::Yes){
            chooseProject();
        }
        return;
    }

    loadProject(selectedFiles.first());
}

void loadProject(const QString& path){

    QFile file(path);
    if(!file.open(QIODevice::ReadOnly)){
        int result = QMessageBox::question(nullptr, QObject::tr("Error opening file"),
                                           QObject::tr("File not open. Cause: %1. Repeat project selection?"));

        if(result == QMessageBox::Yes){
            chooseProject();
        }
        return;
    }

    CrProject* newProject = new CrProject();
    newProject->setPath(file.fileName());

    QDataStream stream(&file);
    stream >> newProject;
    file.close();

    QFile retainFile(file.fileName() + ".retains");
    if(retainFile.open(QIODevice::ReadOnly)){
        QDataStream stream(&retainFile);
        QByteArray retainState;
        stream >> retainState;
        newProject->restoreRetainState(retainState);
        retainFile.close();
    }

    QFileInfo info(file);
    newProject->setName(info.baseName());

    QSettings settings("CrayonEnt", "Crayon");

    foreach(CrMonitor* monitor, newProject->monitors()){
        CrNode* node = monitor->node();
        if(!node)
            continue;

        QWidget* editor = node->createWidgetEditor();
        if(!editor){
            QWindow* windowEditor = node->createWindowEditor();

            if(!windowEditor)
                continue;
            editor = QWidget::createWindowContainer(windowEditor);
            editor->setMinimumSize({640, 480});
        }

        int number = editors_.size();
        editor->restoreGeometry(settings.value("monitor_" + QString::number(number)).toByteArray());

        QAction* fullScreenAction = new QAction(QObject::tr("&Full screen"), editor);
        fullScreenAction->setShortcut(QKeySequence::FullScreen);
        QObject::connect(fullScreenAction, &QAction::triggered, editor, [editor](){
            if(editor->isFullScreen()){
                editor->showNormal();
            }else {
                editor->showFullScreen();
            }
        });
        editor->addAction(fullScreenAction);

        editors_.append(editor);
        QObject::connect(editor, &QWidget::destroyed, editor, [editor](){
            editors_.removeOne(editor);
        });

        editor->show();
    }

    if(editors_.isEmpty()){
        int result = QMessageBox::question(nullptr, QObject::tr("Not found monitors"),
                                           QObject::tr("The project does not contain any valid monitor. Choose another project?"));

        if(result == QMessageBox::Yes){
            chooseProject();
        }
    }else {
        project = newProject;
    }
}
