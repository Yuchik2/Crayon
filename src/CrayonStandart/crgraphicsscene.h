#ifndef CRGRAPHICSSCENE_H
#define CRGRAPHICSSCENE_H

#include <QGraphicsScene>

class CrNode;
class CrGraphicsSceneNodeItem;
//class CrGraphicsSceneConnectorItem;
class CrGraphicsScenePrivate;
class CrGraphicsScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit CrGraphicsScene(CrNode* node);
    virtual ~CrGraphicsScene() override;

    CrNode* node() const;

    QStyle* style() const;
    void setStyle(QStyle* style);

    CrNode* nodeAt(const QPointF& pos) const;

    QPointF nodePos(CrNode* node) const;
    void setNodePos(CrNode *node, const QPointF& pos);

    bool contains(CrNode* node) const;

    QList<CrNode*> selectedNodes() const;

    using Connection = QPair<CrNode*, CrNode*>;
    QList<Connection> selectedConnectors() const;

    qreal moveStep() const;
    void setMoveStep(qreal step);

    void openNameEditor(CrNode* node);
    void closeNameEditor();

    void openEditor(CrNode* node);
    void closeEditor();

    //CrGraphicsSceneNodeItem* item(CrNode* node) const;

    virtual QByteArray saveState() const;
    virtual bool restoreState(const QByteArray &state);

signals:
    void styleChanged();

protected:
    virtual void keyPressEvent(QKeyEvent *keyEvent) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    CrGraphicsScenePrivate* d_;
    void registerItem(CrGraphicsSceneNodeItem* item);
    void unregisterItem(CrGraphicsSceneNodeItem* item);

};

#endif // CRGRAPHICSSCENE_H
