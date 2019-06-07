#include "crgraphicseditor.h"

#include <limits>

#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QDockWidget>
#include <QTabBar>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QLabel>
#include <QUndoView>
#include <QUndoStack>
#include <QHeaderView>
#include <QMessageBox>

#include <QSettings>
#include <QStringBuilder>
#include <QApplication>
#include <QAction>
#include <QFile>
#include <QFileDialog>
#include <QCloseEvent>

#include <crproject.h>
#include <crplugin.h>
#include <crprojectsettingsdialog.h>

#include "crprojectexplorer.h"
#include "crprojectmodulesexplorer.h"
#include "crprojectlogview.h"
#include "crprojecteditorstabwidget.h"

#include <QDebug>

CrGraphicsEditor::CrGraphicsEditor(QWidget *parent) :
    QMainWindow(parent),
    settings_(new QSettings("CrayonEnt", "Crayon")),
    fileDialog_(new QFileDialog()),
    projectSettingsDialog_(new CrProjectSettingsDialog())
{
    initPanels();
    initMenu();
    initToolBar();

    settings_ = new QSettings("CrayonEnt", "Crayon");
    restoreGeometry(settings_->value("mainWindowGeom").toByteArray());
    restoreState(settings_->value("mainWindowState").toByteArray());

    projectStackedState_ = settings_->value("projectsExplorerState").toByteArray();
    modulesStackedState_ = settings_->value("modulesExplorerState").toByteArray();

    fileDialog_->restoreGeometry(settings_->value("fileDialogGeom").toByteArray());
    fileDialog_->restoreState(settings_->value("fileDialogState").toByteArray());

    projectSettingsDialog_->restoreGeometry(settings_->value("projectSettingsDialogGeometry").toByteArray());
    projectSettingsDialog_->restoreState(settings_->value("projectSettingsDialogState").toByteArray());

    logView_->header()->restoreState(settings_->value("logViewState").toByteArray());

}

CrGraphicsEditor::~CrGraphicsEditor()
{
    delete settings_;
    delete fileDialog_;
    delete projectSettingsDialog_;
}

void CrGraphicsEditor::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    exit();
    event->ignore();
}

void CrGraphicsEditor::initPanels()
{
    setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

    editorStackedWidget_ = new QStackedWidget();
    QLabel* emptyEditorWidget = new QLabel(tr("No project"));
    emptyEditorWidget->setAlignment(Qt::AlignCenter);
    editorStackedWidget_->addWidget(emptyEditorWidget);
    setCentralWidget(editorStackedWidget_);

    projectStackedWidget_ = new QStackedWidget();
    QLabel* emptyProjectWidget = new QLabel(tr("No project"));
    emptyProjectWidget->setAlignment(Qt::AlignCenter);
    projectStackedWidget_->addWidget(emptyProjectWidget);

    viewsMenu_ = new QMenu(tr("Views"));

    QDockWidget* projectsDock = new QDockWidget(tr("Projects"), this);
    projectsDock->setObjectName("ProjectsDock");
    projectsDock->setWidget(projectStackedWidget_);
    projectsDock->setBackgroundRole(QPalette::Base);
    projectsDock->setAutoFillBackground(true);
    addDockWidget(Qt::RightDockWidgetArea, projectsDock);
    viewsMenu_ ->addAction(projectsDock->toggleViewAction());

    modulesStackedWidget_ = new QStackedWidget();
    QLabel* emptyModulesWidget = new QLabel(tr("No project"));
    emptyModulesWidget->setAlignment(Qt::AlignCenter);
    modulesStackedWidget_->addWidget(emptyModulesWidget);

    QDockWidget* modulesDock = new QDockWidget(tr("Modules"), this);
    modulesDock->setObjectName("ModulesDock");
    modulesDock->setWidget(modulesStackedWidget_);
    modulesDock->setBackgroundRole(QPalette::Base);
    modulesDock->setAutoFillBackground(true);
    addDockWidget(Qt::RightDockWidgetArea, modulesDock);
    viewsMenu_ ->addAction(modulesDock->toggleViewAction());

    undoStackView_ = new QUndoView();
    QDockWidget* undoStackDock = new QDockWidget(tr("Undo stack"), this);
    undoStackDock->setObjectName("UndoStackDock");
    undoStackDock->setWidget(undoStackView_);
    addDockWidget(Qt::BottomDockWidgetArea, undoStackDock);
    viewsMenu_ ->addAction(undoStackDock->toggleViewAction());

    logView_ = new CrProjectLogView();
    QDockWidget* consoleDock = new QDockWidget(tr("Log"), this);
    consoleDock->setObjectName("LogDock");
    consoleDock->setWidget(logView_);
    addDockWidget(Qt::BottomDockWidgetArea, consoleDock);
    viewsMenu_ ->addAction(consoleDock->toggleViewAction());
}

void CrGraphicsEditor::initMenu()
{
    QWidget* menuWidget = new QWidget();
    menuWidget->setBackgroundRole(QPalette::Base);
    menuWidget->setAutoFillBackground(true);

    QVBoxLayout* menuLayout = new QVBoxLayout();
    menuLayout->setContentsMargins(2,2,2,2);
    menuWidget->setLayout(menuLayout);

    QMenuBar* menuBar = new QMenuBar();
    menuBar->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    menuLayout->addWidget(menuBar);

    QMenu* fileMenu  = new QMenu(tr("File"));

    QAction* createNewProjectAction = new QAction(tr("&New project"), this);
    createNewProjectAction->setShortcut(QKeySequence::New);
    createNewProjectAction->setIcon(QIcon(":/CrayonGraphicsEditorResources/newFile_icon.svg"));
    connect(createNewProjectAction, &QAction::triggered, this, &CrGraphicsEditor::createNewProject);
    fileMenu->addAction(createNewProjectAction);

    QAction* openProjectAction = new QAction(tr("&Open"), this);
    openProjectAction->setShortcut(QKeySequence::Open);
    openProjectAction->setIcon(QIcon(":/CrayonGraphicsEditorResources/folder_icon.svg"));
    connect(openProjectAction, &QAction::triggered, this, &CrGraphicsEditor::openProject);
    fileMenu->addAction(openProjectAction);

    QAction* saveProjectAction = new QAction(tr("&Save"), this);
    saveProjectAction->setShortcut(QKeySequence::Save);
    saveProjectAction->setIcon(QIcon(":/CrayonGraphicsEditorResources/save_icon.svg"));
    saveProjectAction->setEnabled(false);
    connect(saveProjectAction, &QAction::triggered, this, &CrGraphicsEditor::saveProject);
    fileMenu->addAction(saveProjectAction);

    QAction* saveAsProjectAction = new QAction(tr("&Save as"), this);
    saveAsProjectAction->setShortcut(QKeySequence::SaveAs);
    saveAsProjectAction->setIcon(QIcon(":/CrayonGraphicsEditorResources/save_icon.svg"));
    saveAsProjectAction->setEnabled(false);
    connect(saveAsProjectAction, &QAction::triggered, this, &CrGraphicsEditor::saveAsProject);
    fileMenu->addAction(saveAsProjectAction);

    QAction *closeProjectAction = new QAction(tr("&Close project"), this);
    closeProjectAction->setShortcut(QKeySequence::Cancel);
    closeProjectAction->setIcon(QIcon(":/CrayonGraphicsEditorResources/close_icon.svg"));
    closeProjectAction->setEnabled(false);
    connect(closeProjectAction, &QAction::triggered, this, &CrGraphicsEditor::closeCurrentProject);
    fileMenu->addAction(closeProjectAction);

    QAction* exitAction = new QAction(tr("&Exit"), this);
    exitAction->setShortcut(QKeySequence::Close);
    exitAction->setIcon(QIcon(":/CrayonGraphicsEditorResources/exit_icon.svg"));
    connect(exitAction, &QAction::triggered, this, &CrGraphicsEditor::exit);
    fileMenu->addAction(exitAction);

    menuBar->addMenu(fileMenu);

    QMenu* editMenu = new QMenu(tr("Editor"));

    QAction* undoAction = new QAction(tr("&Undo"), this);
    undoAction->setShortcut(QKeySequence::Undo);
    undoAction->setIcon(QIcon(":/CrayonGraphicsEditorResources/undo_icon.svg"));
    undoAction->setEnabled(false);
    connect(undoAction, &QAction::triggered, this, &CrGraphicsEditor::undo);
    editMenu->addAction(undoAction);

    QAction* redoAction = new QAction(tr("&Redo"), this);
    redoAction->setShortcut(QKeySequence::Redo);
    redoAction->setIcon(QIcon(":/CrayonGraphicsEditorResources/redo_icon.svg"));
    redoAction->setEnabled(false);
    connect(redoAction, &QAction::triggered, this, &CrGraphicsEditor::redo);
    editMenu->addAction(redoAction);

    QAction* openProjectSettingsAction  = new QAction(tr("&Project settings"), this);
    openProjectSettingsAction->setIcon(QIcon(":/CrayonGraphicsEditorResources/settings_icon.svg"));
    openProjectSettingsAction->setEnabled(false);
    connect(openProjectSettingsAction, &QAction::triggered, this, &CrGraphicsEditor::openProjectSettings);
    editMenu->addAction(openProjectSettingsAction);
    menuBar->addMenu(editMenu);

    menuBar->addMenu(viewsMenu_);

    QMenu* helpMenu = new QMenu(tr("Help"));
    helpMenu->addAction("Help");
    helpMenu->addAction("About");
    menuBar->addMenu(helpMenu);

    projectsTabBar_ = new QTabBar();
    projectsTabBar_->setExpanding(false);
    projectsTabBar_->setTabsClosable(true);
    projectsTabBar_->setShape(QTabBar::RoundedNorth);
    projectsTabBar_->setBackgroundRole(QPalette::Base);
    projectsTabBar_->setAutoFillBackground(true);
    connect(projectsTabBar_, QOverload<int>::of(&QTabBar::currentChanged),
            this, QOverload<int>::of(&CrGraphicsEditor::setCurrentProject));

    connect(projectsTabBar_, QOverload<int>::of(&QTabBar::tabCloseRequested),
            this, QOverload<int>::of(&CrGraphicsEditor::closeProject));

    menuLayout->addWidget(projectsTabBar_);
    setMenuWidget(menuWidget);

    connect(this, &CrGraphicsEditor::currentProjectChanged,
            this, [this, saveProjectAction, saveAsProjectAction,
            closeProjectAction, undoAction, redoAction, openProjectSettingsAction]
    {
        bool enable = currentProjectIndex_ != -1;
        saveProjectAction->setEnabled(enable);
        saveAsProjectAction->setEnabled(enable);
        closeProjectAction->setEnabled(enable);
        undoAction->setEnabled(enable);
        redoAction->setEnabled(enable);
        openProjectSettingsAction->setEnabled(enable);
    });
}

void CrGraphicsEditor::initToolBar()
{
    //    QToolBar* fileToolBar = new QToolBar(tr("File tool panel"));
    //    fileToolBar->setObjectName("FileToolBar");
    //    fileToolBar->addAction(createNewProjectAction_);
    //    fileToolBar->addAction(openProjectAction_);
    //    fileToolBar->addAction(saveProjectAction_);
    //    addToolBar(Qt::TopToolBarArea, fileToolBar);

    //    QToolBar* editToolBar = new QToolBar(tr("Edit tool panel"));
    //    editToolBar->setObjectName("EditToolBar");
    //    editToolBar->addAction(undoAction_);
    //    editToolBar->addAction(redoAction_);
    //    addToolBar(Qt::TopToolBarArea, editToolBar);

    //    QToolBar* monitorToolBar = new QToolBar(tr("Monitor tool panel"));
    //    monitorToolBar->setObjectName("MonitorToolBar");
    //    monitorToolBar->addAction(runAction_);
    //    addToolBar(Qt::TopToolBarArea, monitorToolBar);
}

void CrGraphicsEditor::createNewProject()
{

    CrProject* newProject = new CrProject();
    newProject->setName(tr("New project"));

    CrPlugin* standartPlugin = CrPlugin::plugin("com.crayonf.standartplugin", {1,0,0});
    if(standartPlugin){
        newProject->addPlugin(standartPlugin);
    }

    insertProject(projects_.size(), newProject);
    setCurrentProject(projects_.size() - 1);
}

void CrGraphicsEditor::openProject()
{
    fileDialog_->setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog_->setDefaultSuffix("cr");
    fileDialog_->setNameFilter("Crayon project files (*.cr)");
    if(!fileDialog_->exec())
        return ;

    QStringList selectedFiles = fileDialog_->selectedFiles();
    if(selectedFiles.isEmpty())
        return;

    QFile file(selectedFiles.first());
    if(!file.open(QIODevice::ReadOnly)){
        return;
    }

    CrProject* newProject = new CrProject();
    newProject->setPath(file.fileName());

    QDataStream stream(&file);
    stream >> newProject;
    file.close();

    QFile retainFile(file.fileName() % ".retains");
    if(retainFile.open(QIODevice::ReadOnly)){
        QDataStream stream(&retainFile);
        QByteArray retainState;
        stream >> retainState;
        newProject->restoreRetainState(retainState);
        retainFile.close();
    }

    QFileInfo info(file);
    newProject->setName(info.baseName());

    insertProject(projects_.size(), newProject);
    setCurrentProject(projects_.size() - 1);
}

void CrGraphicsEditor::saveProject()
{
    Q_ASSERT(currentProjectIndex_ >= 0);

    CrProject* project = currentProject();

    if(project->path().isEmpty()){
        saveAsProject();
        return;
    }

    QFile file(project->path());
    if(!file.open(QIODevice::WriteOnly)){
        return;
    }

    QDataStream stream(&file);
    stream << project->saveState();
    file.close();

    project->undoStack()->setClean();
}

void CrGraphicsEditor::saveAsProject()
{
    Q_ASSERT(currentProjectIndex_ >= 0);

    fileDialog_->setAcceptMode(QFileDialog::AcceptSave);
    fileDialog_->setDefaultSuffix("cr");
    fileDialog_->setNameFilter("Crayon project files (*.cr)");

    if(!fileDialog_->exec())
        return ;

    QStringList selectedFiles = fileDialog_->selectedFiles();
    if(selectedFiles.isEmpty())
        return;

    QFile file(selectedFiles.first());
    if(!file.open(QIODevice::WriteOnly)){
        return;
    }

    CrProject* project = currentProject();
    QDataStream stream(&file);
    stream << project->saveState();
    file.close();

    project->undoStack()->setClean();
    project->setPath(file.fileName());

    if(project->name().isEmpty()){
        QFileInfo info(file);
        currentProject()->setName(info.baseName());
    }
}

bool CrGraphicsEditor::closeCurrentProject()
{
    Q_ASSERT(currentProjectIndex_ >= 0);
    return closeProject(currentProjectIndex_);
}

bool CrGraphicsEditor::closeProject(int index)
{
    Q_ASSERT(index >= 0 && index < projects_.size());

    CrProject* project = projects_.at(index);

    if(!project->undoStack()->isClean()){
        int ret = QMessageBox::warning(this, tr("Crayon"), tr("The document has been modified. " "Do you want to save your changes?"),
                                       QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Yes);
        switch (ret) {
        case QMessageBox::Yes:{
            setCurrentProject(index);
            saveProject();
            break;
        }
        case QMessageBox::No:
            break;
        case QMessageBox::Cancel:
            return false;
        default:
            break;
        }
    }

    QFile retainFile(project->path() % ".retains");
    if(retainFile.open(QIODevice::WriteOnly)){
        QDataStream stream(&retainFile);
        stream << project->saveRetainState();
        retainFile.close();
    }

    removeProject(index);
    project->deleteLater();
    return true;
}

void CrGraphicsEditor::exit()
{
    // Exit

    while (!projects_.isEmpty()) {
        if(!closeProject(0))
            return;
    }

    settings_->setValue("mainWindowGeom", saveGeometry());
    settings_->setValue("mainWindowState", saveState());

    settings_->setValue("projectsExplorerState", projectStackedState_);
    settings_->setValue("modulesExplorerState", modulesStackedState_);

    settings_->setValue("fileDialogGeom", fileDialog_->saveGeometry());
    settings_->setValue("fileDialogState", fileDialog_->saveState());

    settings_->setValue("projectSettingsDialogGeometry", projectSettingsDialog_->saveGeometry());
    settings_->setValue("projectSettingsDialogState", projectSettingsDialog_->saveState());

    settings_->setValue("logViewState", logView_->header()->saveState());

    QApplication::exit();
}

void CrGraphicsEditor::undo()
{
    Q_ASSERT(currentProjectIndex_ >= 0);
    if(undoStackView_->stack()->canUndo())
        undoStackView_->stack()->undo();
}

void CrGraphicsEditor::redo()
{
    Q_ASSERT(currentProjectIndex_ >= 0);
    if(undoStackView_->stack()->canRedo())
        undoStackView_->stack()->redo();
}

void CrGraphicsEditor::openProjectSettings()
{
    Q_ASSERT(currentProjectIndex_ >= 0);
    projectSettingsDialog_->setProject(currentProject());
    projectSettingsDialog_->exec();
}

CrProject *CrGraphicsEditor::currentProject()
{
    return projects_.value(currentProjectIndex_, nullptr);
}

void CrGraphicsEditor::setCurrentProject(int index)
{
    Q_ASSERT(index < projects_.size());

    if(currentProjectIndex_ == index)
        return;

    //    if(projectsTabBar_->currentIndex() != index){
    //        projectsTabBar_->setCurrentIndex(index);
    //        return;
    //    }

    if(currentProjectIndex_ >= 0){
        projectStackedState_ = static_cast<CrProjectExplorer*>(projectStackedWidget_->widget(currentProjectIndex_ + 1))->saveState();
        modulesStackedState_ = static_cast<CrProjectModulesExplorer*>(modulesStackedWidget_->widget(currentProjectIndex_ + 1))->saveState();
    }

    currentProjectIndex_ = index;

    if(currentProjectIndex_ >= 0){

        editorStackedWidget_->setCurrentIndex(currentProjectIndex_ + 1);
        projectStackedWidget_->setCurrentIndex(currentProjectIndex_ + 1);
        modulesStackedWidget_->setCurrentIndex(currentProjectIndex_ + 1);

        undoStackView_->setStack(projects_.at(currentProjectIndex_)->undoStack());
        logView_->setProject(projects_.at(currentProjectIndex_));

        static_cast<CrProjectExplorer*>(projectStackedWidget_->widget(index + 1))->restoreState(projectStackedState_);
        static_cast<CrProjectModulesExplorer*>(modulesStackedWidget_->widget(index +1))->restoreState(modulesStackedState_);

    } else {
        editorStackedWidget_->setCurrentIndex(0);
        projectStackedWidget_->setCurrentIndex(0);
        modulesStackedWidget_->setCurrentIndex(0);

        undoStackView_->setStack(nullptr);
        logView_->setProject(nullptr);

        projectStackedState_ = static_cast<CrProjectExplorer*>(projectStackedWidget_->widget(1))->saveState();
        modulesStackedState_ = static_cast<CrProjectModulesExplorer*>(modulesStackedWidget_->widget(1))->saveState();

        currentProjectIndex_ = -1;
    }

    emit currentProjectChanged();
}

void CrGraphicsEditor::insertProject(int index, CrProject* project)
{
    Q_ASSERT(index >= 0 && index <= projects_.size());
    Q_ASSERT(!projects_.contains(project));

    projects_.insert(index, project);


    auto editorsWidget = new CrProjectEditorsTabWidget(project);
    editorStackedWidget_->insertWidget(index + 1, editorsWidget);

    auto projectExplorer = new CrProjectExplorer(project);
    projectStackedWidget_->insertWidget(index + 1, projectExplorer);

    auto modulesExplorer = new CrProjectModulesExplorer(project);
    modulesStackedWidget_->insertWidget(index + 1, modulesExplorer);

    projectsTabBar_->insertTab(index, project->name());
    connect(project, &CrProject::nameChanged, this, [this, project](){
        projectsTabBar_->setTabText(projects_.indexOf(project), project->name());
    });

    projectsTabBar_->setTabToolTip(index, project->path());
    connect(project, &CrProject::pathChanged, this, [this, project](){
        projectsTabBar_->setTabToolTip(projects_.indexOf(project), project->path());
    });

}

void CrGraphicsEditor::removeProject(int index)
{
    Q_ASSERT(index >= 0 && index < projects_.size());

    projectsTabBar_->removeTab(index);

    delete editorStackedWidget_->widget(index + 1);
    delete projectStackedWidget_->widget(index + 1);
    delete modulesStackedWidget_->widget(index + 1);

    CrProject* project = projects_.at(index);
    disconnect(project, &CrProject::nameChanged, this, nullptr);
    disconnect(project, &CrProject::pathChanged, this, nullptr);


    projects_.removeAt(index);
}




