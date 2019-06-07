#include <crqmlmonitornode.h>

#include <QQuickItem>
#include "crqmlmonitorwindow.h"
#include <crproject.h>

#include <QSGRectangleNode>
#include <QSGImageNode>

class CrQmlMonitorItem : public QQuickItem
{
public:
    CrQmlMonitorItem(QQuickItem *parent = nullptr);
    inline QColor backgroundColor() const { return backgroundColor_;}
    void setBackgroundColor(const QColor &backgroundColor);

    inline QImage backgroundImage() const { return backgroundImage_;}
    void setBackgroundImage(const QImage &backgroundImage);

    QHash<CrQmlNode*, QQuickItem*> items_;

protected:
    virtual QSGNode* updatePaintNode(QSGNode *oldNode, QQuickItem::UpdatePaintNodeData *updatePaintNodeData);

private:
    QColor backgroundColor_;
    QImage backgroundImage_;

    QSGImageNode* image_ = nullptr;

};

CrQmlMonitorItem::CrQmlMonitorItem(QQuickItem *parent) : QQuickItem (parent)
{
    setFlag(ItemHasContents);
}

QSGNode *CrQmlMonitorItem::updatePaintNode(QSGNode *oldNode, QQuickItem::UpdatePaintNodeData *updatePaintNodeData)
{
    QSGRectangleNode *n = static_cast<QSGRectangleNode *>(oldNode);
    if (!n) {
        n = window()->createRectangleNode();
        image_ = window()->createImageNode();
        image_->setFlag(QSGNode::OwnedByParent);
        image_->setOwnsTexture(true);
        n->appendChildNode(image_);
    }

    n->setColor(backgroundColor_);
    n->setRect(boundingRect());

    image_->setTexture(window()->createTextureFromImage(backgroundImage_));
    if(!backgroundImage_.isNull()){
        image_->setRect(boundingRect());
        image_->setSourceRect(boundingRect());
    }else{
        image_->setRect(QRectF());
        image_->setSourceRect(QRectF());
    }

    // n->markDirty(QSGNode::DirtyMaterial);
    return n;
}

void CrQmlMonitorItem::setBackgroundColor(const QColor &backgroundColor)
{
    if(backgroundColor_ == backgroundColor)
        return;
    backgroundColor_ = backgroundColor;
    update();
}

void CrQmlMonitorItem::setBackgroundImage(const QImage &backgroundPixmap)
{
    backgroundImage_ = backgroundPixmap;
    update();
}

CrQmlMonitorNode::CrQmlMonitorNode(CrNode *parent) :
    CrQmlNode (parent),
    monitorSize_(QSizeF(640.0, 480.0)),
    backgroundColor_(Qt::darkGray)
{

    connect(this, QOverload<int, CrNode*>::of(&CrNode::childNodeInserted), this, [this](int index, CrNode* node){
        Q_UNUSED(index);
        CrQmlNode* qmlNode = qobject_cast<CrQmlNode*>(node);
        if(!qmlNode)
            return;
        addQmlNode(qmlNode);
    });

    connect(this, QOverload<int, CrNode*>::of(&CrNode::childNodeRemoved), this, [this](int index, CrNode* node){
        Q_UNUSED(index);
        CrQmlNode* qmlNode = qobject_cast<CrQmlNode*>(node);
        if(!qmlNode)
            return;
        removeQmlNode(qmlNode);
    });

}

CrQmlMonitorNode::~CrQmlMonitorNode()
{

}

QWindow *CrQmlMonitorNode::createWindowEditor()
{
    CrQmlMonitorWindow* view = new CrQmlMonitorWindow(this);
    return view;
}

QQuickItem *CrQmlMonitorNode::createItem()
{
    CrQmlMonitorItem* monitorItem = new CrQmlMonitorItem();
    monitorItem->setSize(monitorSize_);
    connect(this, &CrQmlMonitorNode::monitorSizeChanged, monitorItem, [this, monitorItem]{
        monitorItem->setSize(monitorSize_);
    });
    monitorItem->setBackgroundColor(backgroundColor_);
    connect(this, &CrQmlMonitorNode::backgroundColorChanged, monitorItem, [this, monitorItem]{
        monitorItem->setBackgroundColor(backgroundColor_);
    });
    monitorItem->setBackgroundImage(backgroundImage_);
    connect(this, &CrQmlMonitorNode::backgroundImageChanged, monitorItem, [this, monitorItem]{
        monitorItem->setBackgroundImage(backgroundImage_);
    });

    auto addNode = [this, monitorItem](CrQmlNode* node){

        QQuickItem* item = node->createItem();
        Q_ASSERT(item);

        monitorItem->items_.insert(node, item);

        QRectF geom = geometryItem(node);
        if(geom.isValid()){
            item->setSize(geom.size());
            item->setPosition(geom.topLeft());
        }else {
            setGeometryItem(node, QRectF(item->position(), item->size()));
        }

        item->setParentItem(monitorItem);
    };

    foreach(CrQmlNode* node, nodes_){
        addNode(node);
    }

    connect(this, QOverload<CrQmlNode*>::of(&CrQmlMonitorNode::nodeAdded), monitorItem, addNode);

    connect(this, QOverload<CrQmlNode*>::of(&CrQmlMonitorNode::nodeRemoved), monitorItem, [this, monitorItem](CrQmlNode* node){

        QQuickItem* item = monitorItem->items_.value(node, nullptr);
        Q_ASSERT(item);

        monitorItem->items_.remove(node);
        item->setParentItem(nullptr);
        item->deleteLater();
    });

    connect(this, QOverload<CrQmlNode*, QRectF>::of(&CrQmlMonitorNode::nodeGeometryChanged),
            monitorItem, [this, monitorItem](CrQmlNode* node,  QRectF geom){

        QQuickItem* item = monitorItem->items_.value(node, nullptr);

        if(!item)
            return;

        if(geom.isValid()){
            item->setSize(geom.size());
            item->setPosition(geom.topLeft());
        }
    });

    return monitorItem;
}

QSizeF CrQmlMonitorNode::monitorSize() const
{
    return monitorSize_;
}

void CrQmlMonitorNode::setMonitorSize(const QSizeF &monitorSize)
{
    monitorSize_ = monitorSize;
}

QColor CrQmlMonitorNode::backgroundColor() const
{
    return backgroundColor_;
}

void CrQmlMonitorNode::setBackgroundColor(const QColor &backgroundColor)
{
    if(backgroundColor_ == backgroundColor)
        return;
    backgroundColor_ = backgroundColor;
    emit backgroundColorChanged();
}

QImage CrQmlMonitorNode::backgroundImage() const
{
    return backgroundImage_;
}

void CrQmlMonitorNode::setBackgroundImage(const QImage &backgroundImage)
{
    backgroundImage_ = backgroundImage;
    emit backgroundImageChanged();
}

QList<CrQmlNode *> CrQmlMonitorNode::qmlNodes() const
{
    return nodes_;
}

QList<CrQmlNode *> CrQmlMonitorNode::selectedQmlNodes() const
{
    QList<CrQmlNode*> selected;
    foreach(CrQmlNode* node, nodes_){
        if(node->isSelected())
            selected << node;
    }
    return selected;
}

void CrQmlMonitorNode::addQmlNode(CrQmlNode *node)
{
    if(nodes_.contains(node))
        return;
    nodes_.append(node);

    connect(node,  &CrQmlNode::itemReset, this, &CrQmlMonitorNode::resetQmlNode);

    emit nodeAdded(node);
}

void CrQmlMonitorNode::removeQmlNode(CrQmlNode *node)
{
    int index = nodes_.indexOf(node);
    if(index == -1)
        return;

    nodes_.removeAt(index);

    disconnect(node, &CrQmlNode::itemReset, this, &CrQmlMonitorNode::resetQmlNode);

    emit nodeRemoved(node);
}

void CrQmlMonitorNode::resetQmlNode()
{
    CrQmlNode* node = static_cast<CrQmlNode*>(sender());
    removeQmlNode(node);
    addQmlNode(node);
}

QRectF CrQmlMonitorNode::geometryItem(CrQmlNode *node) const
{
    return geometryItems_.value(node, QRectF(-1.0, -1.0, 0, 0));
}

void CrQmlMonitorNode::setGeometryItem(CrQmlNode *node, QRectF geom)
{
    if(geom == geometryItems_[node])
        return;

    geometryItems_[node] = geom;

    emit nodeGeometryChanged(node, geom);
}

QByteArray CrQmlMonitorNode::saveState() const
{
    QByteArray state;
    QDataStream out(&state, QIODevice::WriteOnly);
    out << CrNode::saveState();


    QHash<QUuid, QRectF> geomMap;

    foreach(CrQmlNode* node, nodes_){
        geomMap.insert(node->uuid(), geometryItems_.value(node));
    }
    out << geomMap;

    return state;
}

bool CrQmlMonitorNode::restoreState(const QByteArray &state)
{
    QDataStream in(state);
    QByteArray nodeState;
    in >> nodeState;
    CrNode::restoreState(nodeState);

    if(project()){
        geometryItems_.clear();


        QHash<QUuid, QRectF> geomMap;

        in >> geomMap;
        auto it = geomMap.cbegin();
        while (it != geomMap.cend()) {
            CrQmlNode* node = qobject_cast<CrQmlNode*>(project()->findNodeByUuid(it.key()));
            if(node){
                geometryItems_.insert(node, it.value());
                emit nodeGeometryChanged(node, it.value());
            }
            ++it;
        }
    }
    return true;
}









