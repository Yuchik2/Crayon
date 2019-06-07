#ifndef CRGRAPHICSEDITOR_H
#define CRGRAPHICSEDITOR_H

#include <QMainWindow>
#include <crlog.h>

class QMenu;
class QAction;
class QDockWidget;
class QTabBar;
class QStackedWidget;
class QFileDialog;
class QUndoView;
class QUndoCommand;
class QSettings;


class CrProject;
class CrProjectLogView;
class CrProjectModulesExplorer;
class CrProjectSettingsDialog;

class CrGraphicsEditor : public QMainWindow
{
    Q_OBJECT
public:
    explicit CrGraphicsEditor(QWidget *parent = nullptr);
    virtual ~CrGraphicsEditor() override;

protected:
    virtual void closeEvent(QCloseEvent * event) override;

private:
    void initPanels();
    void initMenu();
    void initToolBar();

private:
    //WITGETS
//    QMenu* fileMenu_;
//    QMenu* editMenu_;
    QMenu* viewsMenu_;
//    QMenu* helpMenu_;


    QSettings* settings_;
    QFileDialog* fileDialog_;
    CrProjectSettingsDialog* projectSettingsDialog_;

    QTabBar* projectsTabBar_;
    QStackedWidget* projectStackedWidget_;
    QStackedWidget* editorStackedWidget_;
    QStackedWidget* modulesStackedWidget_;
    QUndoView* undoStackView_;
    CrProjectLogView* logView_;

private:
    QList<CrProject*> projects_;
    int currentProjectIndex_ = -1;
    CrProject* currentProject();

    QByteArray projectStackedState_;
    QByteArray modulesStackedState_;

signals:
    void currentProjectChanged();

private slots:
    void createNewProject();
    void openProject();
    void saveProject();
    void saveAsProject();
    bool closeCurrentProject();
    bool closeProject(int index);
    void exit();

    void undo();
    void redo();
    void openProjectSettings();

    void setCurrentProject(int index);

    void insertProject(int index, CrProject* project);
    void removeProject(int index);

};

#endif // CRGRAPHICSEDITOR_H
