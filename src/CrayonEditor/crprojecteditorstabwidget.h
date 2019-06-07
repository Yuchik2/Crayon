#ifndef CRGRAPHICSEDITORSWIDGET_H
#define CRGRAPHICSEDITORSWIDGET_H

#include <QWidget>

class QTabBar;
class QStackedWidget;
class QLabel;

class CrNode;
class CrProject;
class CrProjectEditorsTabWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CrProjectEditorsTabWidget(CrProject *project, QWidget *parent = nullptr);
    virtual ~CrProjectEditorsTabWidget() override;

    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void openEditor(CrNode* node);
    void closeEditor(CrNode* node);
    void detachEditor(CrNode* node);

private:
    CrProject* project_;

    QTabBar* tabBar_;
    QStackedWidget* body_;
    QLabel* dragPreview_;

    QList<CrNode*> tabNodes_;

    QHash<CrNode*, QWidget*> editors_;

    int currentDragIndex_ = -1;
    bool startDrag_ = false;

};

#endif // CRGRAPHICSEDITORSWIDGET_H
