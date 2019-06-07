#ifndef CRPROJECTEXPLORERVIEW_H
#define CRPROJECTEXPLORERVIEW_H

#include <QWidget>
#include <QIcon>

class QTreeView;
class QSortFilterProxyModel;
class QMenu;
class QLineEdit;

class CrNode;
class CrProject;
class CrProjectModel;
class CrNodeSettingsDialog;
class CrProjectExplorer : public QWidget
{
    Q_OBJECT
public:
    explicit CrProjectExplorer(CrProject *project, QWidget *parent = nullptr);

    CrProject *project() const;

    CrNode* rootNode();
    void setRootNode(CrNode* node);

    bool restoreState(const QByteArray &state);
    QByteArray saveState() const;

    QList<CrNode*> selectedNodes() const;
    QList<CrNode*> selectedModuleNodes() const;
signals:
   // void headerRestored();
private:   
    CrProject *project_;
    CrProjectModel* model_;
    QSortFilterProxyModel* proxyModel_;
    QTreeView* view_;
  //  QLineEdit* pathField_;
    CrNode* rootNode_ = nullptr;
    CrNode* currentNode_ = nullptr;
    bool blockSelector_ = false;
};

#endif // CRPROJECTEXPLORERVIEW_H
