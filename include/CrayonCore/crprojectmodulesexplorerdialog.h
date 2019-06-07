#ifndef CRPROJECTMODULESEXPLORERDIALOG_H
#define CRPROJECTMODULESEXPLORERDIALOG_H

#include "crayoncore_global.h"

#include <QDialog>

class CrProject;
class CrModule;
class CrProjectModulesExplorerDialogPrivate;
class CRAYON_CORE_EXPORT CrProjectModulesExplorerDialog : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(CrProject* project READ project WRITE setProject NOTIFY projectChanged)
public:
    explicit CrProjectModulesExplorerDialog(QWidget* parent = nullptr);
    virtual ~CrProjectModulesExplorerDialog() override;

    CrProject* project() const;
    void setProject(CrProject* project);

    int count() const;
    void setCount(int count);

    CrModule *currentModule() const;
    void setCurrentModule(CrModule *currentModule);

    QByteArray saveState() const;
    bool restoreState(const QByteArray &state);

signals:
    void projectChanged();

private:
   CrProjectModulesExplorerDialogPrivate* d_;

public:
   static CrProjectModulesExplorerDialog* commonDialog();
};
#endif // CRPROJECTMODULESEXPLORERDIALOG_H


