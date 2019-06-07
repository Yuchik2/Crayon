#ifndef CRGRAPHICSSCENENODEITEM_H
#define CRGRAPHICSSCENENODEITEM_H

#include <QGraphicsItem>

#include <QSet>

class CrNode;
class CrGraphicsScene;
class CrGraphicsSceneNodeItem;
class CrGraphicsSceneConnectorItem : public QGraphicsItem
{
public:
    enum { Type = UserType + 4 };
    explicit CrGraphicsSceneConnectorItem(CrGraphicsSceneNodeItem* inItem, CrGraphicsSceneNodeItem* outItem);
    virtual ~CrGraphicsSceneConnectorItem() override;

    CrGraphicsSceneNodeItem *inItem() const;
    CrGraphicsSceneNodeItem *outItem() const;

    inline QPointF inPos() const { return outPos_;}
    void setInPos(const QPointF &inPos);

    inline QPointF outPos() const { return outPos_;}
    void setOutPos(const QPointF &outPos);

    bool invisibleMode() const;
    void setInvisibleMode(bool invisibleMode);

    void updateGeometry();

public:
    inline virtual int type() const override { return Type; }
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    virtual QRectF boundingRect() const override;
    virtual QPainterPath shape () const override { return shape_; }

private:
    CrGraphicsSceneNodeItem* inItem_  = nullptr;
    CrGraphicsSceneNodeItem* outItem_ = nullptr;

    QPointF inPos_;
    QPointF outPos_;
    bool invisibleMode_ = false;

    qreal width_;
    QPainterPathStroker stroker_;

    QPainterPath shape_;
    QRectF shapeRect_;
};



class CrGraphicsSceneNodeItem : public QGraphicsItem
{
public:
    enum { Type = UserType + 2 };
    inline virtual int type() const override { return Type;}

    explicit CrGraphicsSceneNodeItem(CrNode* node, CrGraphicsScene* s);
    virtual ~CrGraphicsSceneNodeItem() override;

    CrNode *node() const;

    QList<CrGraphicsSceneNodeItem*> childItems();
    void insertChildItem(int index, CrGraphicsSceneNodeItem* item);
    void removeChildItem(int index);

    inline bool isExpanded() const { return expanded_;}
    void setExpanded(bool expanded);

public:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    virtual QRectF boundingRect() const override;

protected:
    virtual QVariant itemChange ( GraphicsItemChange change, const QVariant & value ) override;
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    friend class CrGraphicsScene;
    friend class CrGraphicsSceneConnectorItem;

    enum AnchorsState{
        None = 0x0,
        InAccept = 0x1,
        InReject = 0x2,
        OutAccept = 0x4,
        OutReject = 0x8
    } anchorsState_ = None;

    CrNode* node_;
    CrGraphicsScene* scene_;
    QWidget* editor_ = nullptr;

    CrGraphicsSceneNodeItem *parentItem_ = nullptr;
    QList<CrGraphicsSceneNodeItem*> childItems_;

    CrGraphicsSceneConnectorItem* inConnector_ = nullptr;
    QSet<CrGraphicsSceneConnectorItem*> outConnectors_;

    static CrGraphicsSceneConnectorItem* targetConnector_;
    static CrGraphicsSceneNodeItem * targetItem_;

    bool expanded_= false;

    //Geometry
    void updateGeometry();
    void updatePos();

    QSizeF size_;
    QSizeF sizeHint_;
    QSizeF nameSizeHint_;
    QSizeF editorSizeHint_;

    QRectF branchRect_;
    QRectF nameRect_;
    QRectF editorRect_;
    QRectF inAnchorRect_;
    QRectF outAnchorRect_;
    QRectF headerRect_;

};

#endif // CRGRAPHICSSCENENODEITEM_H
