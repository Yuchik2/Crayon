//#include "crqmlmonitoritem.h"

//#include <crqmlmonitornode.h>

//CrQmlMonitorItem::CrQmlMonitorItem(QQuickItem *parent) :
//    QQuickItem (parent),
//    node_(nullptr)
//{

//}

//CrQmlMonitorNode *CrQmlMonitorItem::node() const
//{
//    return node_;
//}

//void CrQmlMonitorItem::setNode(CrQmlMonitorNode *node)
//{
//    if(node_ == node)
//        return;

//    if(node_){
//        foreach (CrQmlNode* node, node_->qmlNodes()) {
//            removeNode(node);
//        }
//        disconnect(node_, SIGNAL(monitorSizeChanged()), this, SLOT(changeMonitorSize()));
//        disconnect(node_, SIGNAL(backgroundColorChanged()),this, SIGNAL(backgroundColorChanged()));
//        disconnect(node_, SIGNAL(backgroundImageSourceChanged()),this, SIGNAL(backgroundImageSourceChanged()));
//        disconnect(node_, SIGNAL(nodeAdded(CrQmlNode*)), this, SLOT(addNode(CrQmlNode*)));
//        disconnect(node_, SIGNAL(nodeRemoved(CrQmlNode*)), this, SLOT(removeNode(CrQmlNode*)));
//        disconnect(node_, SIGNAL(nodeGeometryChanged(CrQmlNode*, QRectF)), this, SLOT(changeNodeGeometry(CrQmlNode*, QRectF)));
//    }

//    node_ = node;

//    if(node_){
//        changeMonitorSize();

//        foreach (CrQmlNode* node, node_->qmlNodes()) {
//            addNode(node);
//            //  changeNodeGeometry(node, node_->geometryItem(node));

//        }

//        connect(node_, SIGNAL(monitorSizeChanged()), this, SLOT(changeMonitorSize()));
//        connect(node_, SIGNAL(backgroundColorChanged()),this, SIGNAL(backgroundColorChanged()));
//        connect(node_, SIGNAL(backgroundImageSourceChanged()),this, SIGNAL(backgroundImageSourceChanged()));
//        connect(node_, SIGNAL(nodeAdded(CrQmlNode*)), this, SLOT(addNode(CrQmlNode*)));
//        connect(node_, SIGNAL(nodeRemoved(CrQmlNode*)), this, SLOT(removeNode(CrQmlNode*)));
//        connect(node_, SIGNAL(nodeGeometryChanged(CrQmlNode*, QRectF)), this, SLOT(changeNodeGeometry(CrQmlNode*, QRectF)));
//    }

//    emit backgroundColorChanged();
//    emit backgroundImageSourceChanged();
//}

//QColor CrQmlMonitorItem::backgroundColor() const
//{
//    return node_ ? node_->backgroundColor() : QColor(Qt::gray);
//}

//QString CrQmlMonitorItem::backgroundImageSource() const
//{
//    return node_ ? node_->backgroundImageSource() : QString();
//}

//QQuickItem *CrQmlMonitorItem::itemByNode(CrQmlNode *node)
//{
//    return items_.value(node, nullptr);
//}

//void CrQmlMonitorItem::changeMonitorSize()
//{
//    setSize(node_->monitorSize());
//}

//void CrQmlMonitorItem::changeNodeGeometry(CrQmlNode *node, QRectF geom)
//{
//    QQuickItem* item = items_.value(node, nullptr);

//    if(!item)
//        return;

//    if(geom.isValid()){
//        item->setSize(geom.size());
//        item->setPosition(geom.topLeft());
//    }
//}

//void CrQmlMonitorItem::addNode(CrQmlNode *node)
//{
//    connect(node, SIGNAL(itemReset()), this, SLOT(resetItem()));
//    QQuickItem* item = node->createItem();

//    Q_ASSERT(item);

//    items_.insert(node, item);

//    QRectF geom = node_->geometryItem(node);
//    if(geom.isValid()){
//        item->setSize(geom.size());
//        item->setPosition(geom.topLeft());
//    }else {
//        node_->setGeometryItem(node, QRectF(item->position(), item->size()));
//    }

//    item->setParentItem(this);
//}

//void CrQmlMonitorItem::removeNode(CrQmlNode *node)
//{
//    disconnect(node, SIGNAL(itemReset()), this, SLOT(resetItem()));
//    QQuickItem* item = items_.value(node, nullptr);

//    Q_ASSERT(item);

//    items_.remove(node);
//    item->setParentItem(nullptr);
//    item->deleteLater();
//}

//void CrQmlMonitorItem::resetItem()
//{
//    CrQmlNode* node = static_cast<CrQmlNode*>(sender());
//    QQuickItem* item = items_.value(node, nullptr);

//    Q_ASSERT(item);

//    item->setVisible(false);
//    item->setParentItem(nullptr);
//    item->deleteLater();

//    item = node->createItem();

//    items_.insert(node, item);

//    QRectF geom = node_->geometryItem(node);
//    if(geom.isValid()){
//        item->setSize(geom.size());
//        item->setPosition(geom.topLeft());
//    }else {
//        node_->setGeometryItem(node, QRectF(item->position(), item->size()));
//    }

//    item->setParentItem(this);
//}
