#ifndef CRPROJECTMODULESEXPLORER_H
#define CRPROJECTMODULESEXPLORER_H

#include <QWidget>

class QTreeView;
class QMenu;

class CrNode;
class CrProject;
class CrProjectModulesModel;
class CrNodeSettingsDialog;
class CrProjectModulesExplorer : public QWidget
{
    Q_OBJECT
public:
    explicit CrProjectModulesExplorer(CrProject* project, QWidget *parent = nullptr);

    virtual ~CrProjectModulesExplorer() override;

    bool restoreState(const QByteArray &state);
    QByteArray 	saveState() const;

private:
    QTreeView* view_;
    CrProject* project_;
   // CrNode* currentNode_;
};


#endif // CRPROJECTMODULESEXPLORER_H
