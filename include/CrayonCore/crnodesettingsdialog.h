#ifndef CRNODESETTINGSDIALOG_H
#define CRNODESETTINGSDIALOG_H

#include "crayoncore_global.h"

#include <QDialog>

class CrNode;
class CrNodeSettingsDialogPrivate;
class CRAYON_CORE_EXPORT CrNodeSettingsDialog : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(CrNode* node READ node WRITE setNode NOTIFY nodeChanged)
public:
    explicit CrNodeSettingsDialog(QWidget* parent = nullptr);
    virtual ~CrNodeSettingsDialog() override;

    CrNode *node() const;
    void setNode(CrNode *node);

signals:
    void nodeChanged();

private:
    CrNodeSettingsDialogPrivate* d_;

public:
    static CrNodeSettingsDialog* commonDialog();

};


#endif // CRNODESETTINGSDIALOG_H
