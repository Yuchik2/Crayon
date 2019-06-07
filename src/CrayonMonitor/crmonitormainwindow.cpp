//#include "crmonitormainwindow.h"
//#include <QApplication>
//#include <QFileInfo>
//#include <QFile>
//#include <QAction>
//#include <QMenu>
//#include <QMenuBar>
//#include <QFileDialog>
//#include <QHBoxLayout>
//#include <QSettings>

//#include <QTimer>

//#include <CrayonCore/crlog.h>



//#include <QDebug>

//CrMonitorMainWindow::CrMonitorMainWindow(QWidget *parent) : QMainWindow(parent)
//{
//    mainSettings_ = new QSettings("CrayonEnt", "Crayon");
//    QAction* openProjectAction_ = new QAction(tr("&Open"), this);
//    openProjectAction_->setShortcut(QKeySequence::Open);
//    openProjectAction_->setIcon(QIcon(":/ToolBarIcon/folder1_icon.png"));
//    connect(openProjectAction_, &QAction::triggered, this, [this](){
//        // Open project
//        QString filePath = QFileDialog::getOpenFileName(0,"Open file","","*.cr");
//        QFile file(filePath);
//        if(!file.open(QIODevice::ReadOnly)){
//            return;
//        }
//        QDataStream stream(&file);

//        CrProjectNode* project = new CrProjectNode();
//        project->setIcon(QIcon(":/ToolBarIcon/new_project.png"));
//        stream >> project;
//        file.close();

//        setProject(project);
//    });

//    QAction* exitAction_ = new QAction(tr("&Exit"), this);
//    exitAction_->setShortcut(QKeySequence::Close);
//    exitAction_->setIcon(QIcon(":/ToolBarIcon/exit_icon.png"));
//    connect(exitAction_, &QAction::triggered, this, [this](){
//        // Exit programm
//        close();
//    });

//    QMenu* fileMenu_  = new QMenu(tr("File"));
//    fileMenu_->addAction(openProjectAction_);
//    fileMenu_->addAction(exitAction_);
//    menuBar()->addMenu(fileMenu_);

//    QTimer::singleShot(0, this, [this](){
//        QStringList args = QApplication::arguments();

//        for (int i = 1; i < args.size(); ++i) {
//            QFileInfo fileInfo(args.at(i));

//            if(fileInfo.exists() && fileInfo.suffix() == "cr"){

//                QFile file(fileInfo.absoluteFilePath());
//                if(!file.open(QIODevice::ReadOnly)){
//                    continue;
//                }
//                CrProjectNode* project = new CrProjectNode();
//                QDataStream stream(&file);
//                stream >> project;
//                file.close();
//                setProject(project);
//                hide();
//                break;
//            }
//        }
//    });
//    connect(QApplication::instance(), &QApplication::aboutToQuit, this, [this](){
//        for (int i = 0; i < monitors_.size(); ++i) {
//            auto monitor = monitors_.at(i);
//            mainSettings_->setValue("monitor_" + QString::number(i), monitor->saveGeometry());
//        }
//    });

//    fullScreenAction_ = new QAction(tr("&Full screen"), this);
//    fullScreenAction_->setShortcut(QKeySequence::FullScreen);
//    fullScreenAction_->setIcon(QIcon(":/ToolBarIcon/folder1_icon.png"));
//    connect(fullScreenAction_, &QAction::triggered, this, [this](){
//        foreach(QWidget* widget, monitors_){
//            if(widget->isFullScreen()){
//                widget->showNormal();
//            }else {
//                widget->showFullScreen();
//            }
//        }
//    });
//    addAction(fullScreenAction_);
//    //    projectFile.setFileName(file.absoluteFilePath());
//    //    setCurrentProject(newProject);
//    //    projectExplorerModel_->addProject(newProject);
//    //    QString filePath = QFileDialog::getOpenFileName(0,"Open file","","*.cr");

//}

//void CrMonitorMainWindow::closeEvent(QCloseEvent *event)
//{

//}

//void CrMonitorMainWindow::setProject(CrProjectNode *project)
//{
//    if(project_){
//        foreach(QWidget* monitor, monitors_){
//            monitor->deleteLater();
//        }
//        monitors_.clear();
//        project_->deleteLater();
//    }
//    project_ = project;

//    QApplication::processEvents();

//    for (int i = 0; i < project_->monitors().size(); ++i) {
//        CrNode* monitor = project_->monitors().at(i);
//        const CrNodeEditor* view = monitor->view();

//        if(view->type() == CrNodeEditor::Widget){
//            if(QWidget* editor = view->widget(monitor)){
//                QWidget* container = new QWidget();
//                container->addAction(fullScreenAction_);
//                QHBoxLayout* layout = new QHBoxLayout();
//                layout->addWidget(editor);
//                container->setLayout(layout);
//                monitors_.append(container);
//                container->restoreGeometry(mainSettings_->value("monitor_" + QString::number(i)).toByteArray());
//                container->show();
//            }
//        }

////        if(QWidget* editor = CrCore::getViewNode(monitor)){
////            QWidget* container = new QWidget();
////            container->addAction(fullScreenAction_);
////            QHBoxLayout* layout = new QHBoxLayout();
////            layout->addWidget(editor);
////            container->setLayout(layout);
////            monitors_.append(container);
////            container->restoreGeometry(mainSettings_->value("monitor_" + QString::number(i)).toByteArray());
////            container->show();
////        }
//    }
//}




