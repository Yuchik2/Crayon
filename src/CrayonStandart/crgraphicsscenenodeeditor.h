#ifndef CRGRAPHICSSCENENODEEDITOR_H
#define CRGRAPHICSSCENENODEEDITOR_H

#include <QGraphicsView>

class QLineEdit;
class CrGraphicsScene;
class CrNode;
class CrGraphicsSceneEditor : public QGraphicsView
{
    Q_OBJECT
public:
    explicit CrGraphicsSceneEditor(CrGraphicsScene* scene);
    virtual ~CrGraphicsSceneEditor() override;

    CrGraphicsScene *scene() const;

    virtual bool eventFilter(QObject *watched, QEvent *event) override;

protected:
    virtual bool event(QEvent *event) override;
    virtual void drawBackground(QPainter *painter, const QRectF &rect) override;
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void mouseDoubleClickEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void wheelEvent(QWheelEvent * event) override;
    virtual void dragEnterEvent(QDragEnterEvent *event) override;
    virtual void dragLeaveEvent(QDragLeaveEvent *event) override;
    virtual void dragMoveEvent(QDragMoveEvent *event) override;
    virtual void dropEvent(QDropEvent *event) override;
    virtual void closeEvent(QCloseEvent * event) override;

private:
    CrGraphicsScene* scene_;
    CrNode* currentNode_ = nullptr;

    QPoint oldPosCursor_;
    QPoint startPosCursor_;

    //magic
    qreal sceneGridSize_ = 50.0;
  //  qreal moveStep_ = 25.0;
    qreal zoomSpeed_ = 1.3;

};

#endif // CRGRAPHICSSCENENODEEDITOR_H
