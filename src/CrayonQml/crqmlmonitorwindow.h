#ifndef CRQMLMONITORWINDOW_H
#define CRQMLMONITORWINDOW_H

#include <QQuickWindow>

class CrQmlNode;
class CrQmlMonitorNode;
class CrQmlMonitorWindow : public QQuickWindow
{
    Q_OBJECT
public:
    enum State{
        None,
        DragCamera,
        Select,
        Move,
        Resize
    };
    explicit CrQmlMonitorWindow(CrQmlMonitorNode* node, QWindow *parent = nullptr);

protected:
    virtual bool event(QEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void wheelEvent(QWheelEvent * event) override;

private:
    CrQmlMonitorNode* node_;
    QQuickItem* monitor_;
    QQuickItem* rubberBand_;
    QQuickItem* selector_;
    QSet<CrQmlNode*> selectedNodes_;
    CrQmlNode* currentNode_ = nullptr;
    State state_ = None;
    QPoint oldPos_;
    QPoint startPos_;
    qreal scale_ = 1;

private slots:
    void addNode(CrQmlNode* node);
    void removeNode(CrQmlNode* node);
    void changeSelect();

private:
    void updateSelector();
};

#endif // CRQMLMONITORWINDOW_H
