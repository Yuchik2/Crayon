#ifndef CRPROJECTEXPLORERDIALOG_H
#define CRPROJECTEXPLORERDIALOG_H

#include "crayoncore_global.h"

#include <QDialog>

class CrNode;
class CrProject;
class CrProjectExplorerDialogPrivate;
class CRAYON_CORE_EXPORT CrProjectExplorerDialog : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(CrProject* project READ project WRITE setProject NOTIFY projectChanged)
    Q_PROPERTY(CrNode* currentNode READ currentNode WRITE setCurrentNode NOTIFY currentNodeChanged)
public:

    enum Filters{
        NoFilter,
        ValidInputNode,
        ValidOutputNode,
        OnlyModules
    };
    Q_ENUM(Filters)

    explicit CrProjectExplorerDialog(QWidget* parent = nullptr);
    virtual ~CrProjectExplorerDialog() override;

    CrProject *project() const;
    void setProject(CrProject *project);

    void setFilter(const Filters &filter, CrNode* target = nullptr);

    void setCurrentNode(CrNode* node);
    CrNode* currentNode();

    QList<CrNode *> selectedNodes() const;

    QByteArray saveState() const;
    bool restoreState(const QByteArray &state);

signals:
    void projectChanged();
    void currentNodeChanged();

public:
    static CrProjectExplorerDialog* commonDialog();

private:
    CrProjectExplorerDialogPrivate* d_;


};

#endif // CRPROJECTEXPLORERDIALOG_H
