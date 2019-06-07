#ifndef CRNODEEDITORWIDRET_H
#define CRNODEEDITORWIDRET_H

#include "crayoncore_global.h"

#include <QWidget>

class CrNode;
class CRAYON_CORE_EXPORT CrNodeEditorWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CrNodeEditorWidget(QWidget *parent = nullptr);
    virtual ~CrNodeEditorWidget() override;

    CrNode *node() const;
    void setNode(CrNode *node);

    virtual QSize sizeHint() const override;
    virtual QSize minimumSizeHint() const override;

   virtual bool eventFilter(QObject *watched, QEvent *event) override;

protected:
    virtual bool event(QEvent *event) override;
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    CrNode* node_;
    QWidget* editor_;
};

#endif // CRNODEEDITORWIDRET_H
