#ifndef CRPROJECTSETTINGSDIALOG_H
#define CRPROJECTSETTINGSDIALOG_H

#include "crayoncore_global.h"
#include <QDialog>

class QTreeWidget;
class CrNode;
class CrProject;
class CrProjectSettingsDialogPrivate;
class CRAYON_CORE_EXPORT CrProjectSettingsDialog : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(CrProject* project READ project WRITE setProject NOTIFY projectChanged)
public:
    explicit CrProjectSettingsDialog(QWidget *parent = nullptr);
    virtual ~CrProjectSettingsDialog();

    CrProject *project() const;
    void setProject(CrProject *project);

    bool restoreState(const QByteArray &state);
    QByteArray 	saveState() const;

signals:
    void projectChanged();

public:
    static CrProjectSettingsDialog* commonDialog();

private:
    CrProjectSettingsDialogPrivate* d_;
};

#endif // CRPROJECTSETTINGSDIALOG_H
