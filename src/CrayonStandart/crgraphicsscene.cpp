#include "crgraphicsscene.h"

#include <QSet>
#include <QApplication>
#include <QDialog>
#include <QLineEdit>

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsProxyWidget>

#include <CrNode>
#include <CrProject>
#include <CrCommands>
#include "crgraphicsscenenodeitem.h"

#include <QDebug>

class CrGraphicsScenePrivate{
public:
    CrNode* node_ = nullptr;
    QStyle* style_ = nullptr;

    QHash<CrNode*,  CrGraphicsSceneNodeItem*> items_;
    QHash<CrNode*,  CrGraphicsSceneConnectorItem*> connectors_;
    QList<CrGraphicsSceneNodeItem*> topLevelItems_;
    CrGraphicsSceneNodeItem* editableItem_ = nullptr;

    CrGraphicsSceneNodeItem* editableNameItem_ = nullptr;

    QGraphicsProxyWidget* nameEdiotrProxy_ =  new QGraphicsProxyWidget();

    QSet<CrNode*> expandedCache_;
    QHash<CrNode*, QPointF> positionCache_;

    QGraphicsRectItem* selector_ = nullptr;
    QPointF startPos_;

    qreal moveStep_ = 20;
};

CrGraphicsScene::CrGraphicsScene(CrNode *node) :
    QGraphicsScene (node),
    d_(new CrGraphicsScenePrivate())
{
    d_->node_ = node;
    d_->style_ = QApplication::style();
    d_->selector_ = new QGraphicsRectItem();
    d_->selector_->setZValue(5);
    addItem(d_->selector_);
    d_->selector_->hide();

    QColor selectRectColor = Qt::blue;
    selectRectColor.setAlpha(180);
    d_->selector_->setBrush(QBrush(selectRectColor));

    QLineEdit* nameEditor_ = new QLineEdit();
    connect(nameEditor_, &QLineEdit::editingFinished, this, [this, nameEditor_](){

        if(!d_->editableNameItem_)
            return;

        if(nameEditor_->text().simplified() == d_->editableNameItem_->node()->name())
            return ;

        if(d_->editableNameItem_->node()->project()){
            d_->editableNameItem_->node()->project()->
                    pushUndoCommand(new CrCommands::ChangeName(d_->editableNameItem_->node(), nameEditor_->text().simplified()));
        }else {
            d_->editableNameItem_->node()->setName(nameEditor_->text().simplified());
        }
        closeNameEditor();
    });

    d_->nameEdiotrProxy_->setWidget(nameEditor_);


    auto insertChild = [this](int index, CrNode* child){
        CrGraphicsSceneNodeItem* childItem = new CrGraphicsSceneNodeItem(child, this);
        childItem->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
        registerItem(childItem);
        d_->topLevelItems_.insert(index, childItem);
        addItem(childItem);
        childItem->setPos(d_->positionCache_.value(child));
    };

    for (int i = 0; i < d_->node_->childNodes().size(); ++i) {
        insertChild(i, d_->node_->childNodes().at(i));
    }

    connect(d_->node_, QOverload<int, CrNode* >::of(&CrNode::childNodeInserted), this, insertChild);
    connect(d_->node_,  QOverload<int, CrNode*>::of(&CrNode::childNodeRemoved), this, [this](int index, CrNode* child)
    {
        Q_UNUSED(child)
        CrGraphicsSceneNodeItem* childItem = d_->topLevelItems_.at(index);
        d_->topLevelItems_.removeAt(index);
        unregisterItem(childItem);
        d_->positionCache_.insert(child, childItem->pos());
        delete childItem;
    });
}

CrGraphicsScene::~CrGraphicsScene()
{
    for (int i = 0; i < d_->topLevelItems_.size(); ++i) {
        CrGraphicsSceneNodeItem* childItem = d_->topLevelItems_.at(i);
        d_->topLevelItems_.removeAt(i);
        unregisterItem(childItem);
        delete childItem;
    }
    delete d_;
}

CrNode *CrGraphicsScene::node() const
{
    return d_->node_;
}

QStyle *CrGraphicsScene::style() const
{
    return  d_->style_;
}

void CrGraphicsScene::setStyle(QStyle *style)
{
    if(d_->style_ == style)
        return;
    d_->style_ = style;
    emit styleChanged();
}

CrNode *CrGraphicsScene::nodeAt(const QPointF &pos) const
{
    if(CrGraphicsSceneNodeItem* item = qgraphicsitem_cast<CrGraphicsSceneNodeItem*>(itemAt(pos, QTransform()))){
        return item->node();
    }
    return nullptr;
}

QPointF CrGraphicsScene::nodePos(CrNode *node) const
{
    if(CrGraphicsSceneNodeItem* item = d_->items_.value(node, nullptr)){
        return item->scenePos();
    }
    return  QPointF();
}

void CrGraphicsScene::setNodePos(CrNode *node, const QPointF &pos)
{
    if(CrGraphicsSceneNodeItem* item = d_->items_.value(node, nullptr)){
        item->setPos(pos);
    }
    return;
}

bool CrGraphicsScene::contains(CrNode *node) const
{
    return d_->items_.contains(node);
}

QList<CrNode *> CrGraphicsScene::selectedNodes() const
{
    QList<CrNode*> selectedNodes;

    foreach(CrNode* node, d_->items_.keys()){
        if(node->isSelected())
            selectedNodes << node;
    }

    return selectedNodes;
}

QList<CrGraphicsScene::Connection> CrGraphicsScene::selectedConnectors() const
{
    QList<Connection> selCon;

    foreach(QGraphicsItem* item, selectedItems()){
        if(CrGraphicsSceneConnectorItem* connector = qgraphicsitem_cast<CrGraphicsSceneConnectorItem*>(item)){
           selCon << Connection(connector->inItem()->node(), connector->outItem()->node());
        }
    }
    return selCon;
}

qreal CrGraphicsScene::moveStep() const
{
    return d_->moveStep_;
}

void CrGraphicsScene::setMoveStep(qreal step)
{
    d_->moveStep_ = step;
}

void CrGraphicsScene::openNameEditor(CrNode *node)
{
    closeNameEditor();

    CrGraphicsSceneNodeItem* item = d_->items_.value(node, nullptr);
    if(!item)
        return;

    static_cast<QLineEdit*>(d_->nameEdiotrProxy_->widget())->setText(node->name());
    d_->nameEdiotrProxy_->setParentItem(item);
    d_->nameEdiotrProxy_->setGeometry(item->nameRect_);
    d_->nameEdiotrProxy_->show();
    d_->nameEdiotrProxy_->setFocus();
    d_->editableNameItem_ = item;
}

void CrGraphicsScene::closeNameEditor()
{
    if(!d_->editableNameItem_)
        return;

    d_->nameEdiotrProxy_->hide();
    d_->editableNameItem_ = nullptr;
}

void CrGraphicsScene::openEditor(CrNode *node)
{
    closeEditor();

    CrGraphicsSceneNodeItem* item = d_->items_.value(node, nullptr);
    if(!item)
        return;

    QWidget* editor = node->createCompactWidgetEditor();

    if(editor){
        QGraphicsProxyWidget* proxyValueEditor = new QGraphicsProxyWidget(item);
        proxyValueEditor->setWidget(editor);
        proxyValueEditor->setGeometry(item->editorRect_);
        editor->show();
        editor->setFocus();
        connect(editor, &QWidget::destroyed, this, &CrGraphicsScene::closeEditor);

        d_->editableItem_ = item;
        d_->editableItem_->editor_ = editor;
        d_->editableItem_->setZValue(4);
        d_->editableItem_->updateGeometry();

        return;
    }

    QDialog* dialog = node->createDialogEditor();

    if(dialog){
        dialog->exec();
        dialog->deleteLater();
        return;
    }

    node->requestOpenEditor();
}

void CrGraphicsScene::closeEditor()
{
    if(!d_->editableItem_)
        return;

    d_->editableItem_->setZValue(1);
    d_->editableItem_->editor_->deleteLater();
    d_->editableItem_->editor_ = nullptr;
    d_->editableItem_->updateGeometry();
    d_->editableItem_ = nullptr;
}

//CrGraphicsSceneNodeItem *CrGraphicsScene::item(CrNode *node) const
//{
//    return d_->items_.value(node, nullptr);
//}

void CrGraphicsScene::registerItem(CrGraphicsSceneNodeItem* item)
{

    CrNode* node = item->node();

    auto insertChild = [this, item](int index, CrNode* child){
        CrGraphicsSceneNodeItem* childItem = new CrGraphicsSceneNodeItem(child, this);
        registerItem(childItem);
        item->insertChildItem(index, childItem);
    };

    for (int i = 0; i < node->childNodes().size(); ++i) {
        insertChild(i, node->childNodes().at(i));
    }

    connect(node, QOverload<int, CrNode* >::of(&CrNode::childNodeInserted), this, insertChild);
    connect(node,  QOverload<int, CrNode*>::of(&CrNode::childNodeRemoved), this, [this, item](int index, CrNode* child)
    {
        Q_UNUSED(child)
        CrGraphicsSceneNodeItem* childItem = item->childItems().at(index);
        item->removeChildItem(index);
        unregisterItem(childItem);
        delete childItem;
    });

    if(node->inputNode()){
        if(CrGraphicsSceneNodeItem* outItem = d_->items_.value(node->inputNode(), nullptr)){
            CrGraphicsSceneConnectorItem* connector = new CrGraphicsSceneConnectorItem(item, outItem);
            d_->connectors_.insert(node, connector);
            addItem(connector);
        }
    }

    foreach (CrNode* inNode, node->outputNodes()) {
        if(CrGraphicsSceneNodeItem* inItem = d_->items_.value(inNode, nullptr)){
            CrGraphicsSceneConnectorItem* connector = new CrGraphicsSceneConnectorItem(inItem, item);
            d_->connectors_.insert(inNode, connector);
            addItem(connector);
        }
    }

    connect(node, QOverload<CrNode* >::of(&CrNode::outputNodeAdded), this, [this, item](CrNode* inNode){
        if(CrGraphicsSceneNodeItem* inItem = d_->items_.value(inNode, nullptr)){
            CrGraphicsSceneConnectorItem* connector = new CrGraphicsSceneConnectorItem(inItem, item);
            d_->connectors_.insert(inNode, connector);
            addItem(connector);
        }
    });

    connect(node, QOverload<CrNode* >::of(&CrNode::outputNodeRemoved), this, [this](CrNode* inNode){
        if(CrGraphicsSceneConnectorItem* connector = d_->connectors_.value(inNode, nullptr)){
            removeItem(connector);
            d_->connectors_.remove(inNode);
            delete connector;
        }
    });

    d_->items_.insert(node, item);

    item->setExpanded(d_->expandedCache_.contains(node));

}

void CrGraphicsScene::unregisterItem(CrGraphicsSceneNodeItem* item)
{
    CrNode* node = item->node();

    while (!item->childItems().isEmpty()) {
        CrGraphicsSceneNodeItem* childItem = item->childItems().first();
        item->removeChildItem(0);
        unregisterItem(childItem);
        delete childItem;
    }

    disconnect(node, &CrNode::childNodeInserted, this, nullptr);
    disconnect(node,  &CrNode::childNodeRemoved, this, nullptr);

    if(node->inputNode()){
        if(CrGraphicsSceneConnectorItem* connector = d_->connectors_.value(node, nullptr)){
            removeItem(connector);
            d_->connectors_.remove(node);
            delete connector;
        }
    }

    foreach (CrNode* inNode, node->outputNodes()) {
        if(CrGraphicsSceneConnectorItem* connector = d_->connectors_.value(inNode, nullptr)){
            removeItem(connector);
            d_->connectors_.remove(inNode);
            delete connector;
        }
    }

    disconnect(node, &CrNode::outputNodeAdded, this, nullptr);
    disconnect(node, &CrNode::outputNodeRemoved, this, nullptr);

    d_->items_.remove(node);

    if(item->isExpanded()){
        d_->expandedCache_.insert(node);
    }else {
        d_->expandedCache_.remove(node);
    }

    d_->items_.remove(node);

}

QByteArray CrGraphicsScene::saveState() const
{
    QByteArray state;
    QDataStream out(&state, QIODevice::WriteOnly);
    out << d_->topLevelItems_.size();
    for (int i = 0; i < d_->topLevelItems_.size(); ++i) {
        CrGraphicsSceneNodeItem* item  = d_->topLevelItems_.at(i);
        out << item->node()->uuid() << item->pos();
    }

    return state;
}

bool CrGraphicsScene::restoreState(const QByteArray &state)
{
    QDataStream in(state);
    int size;
    in >> size;
    for (int i = 0; i < size; ++i) {
        QUuid nodeUuid;
        QPointF pos;
        in >> nodeUuid >> pos;

        CrNode* node = d_->node_->project()->findNodeByUuid(nodeUuid);
        setNodePos(node, pos);
    }

    return true;
}

void CrGraphicsScene::keyPressEvent(QKeyEvent *keyEvent)
{
    QGraphicsScene::keyPressEvent(keyEvent);
    if(d_->editableItem_){
        if(keyEvent->key() == Qt::Key_Return){
            closeEditor();
        }
    }

    if(d_->editableNameItem_){
        if(keyEvent->key() == Qt::Key_Return){
            closeNameEditor();
        }
    }
}


void CrGraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if(!mouseGrabberItem()){
        d_->selector_->setRect(QRectF(d_->startPos_, event->scenePos()).normalized());
    }
    QGraphicsScene::mouseMoveEvent(event);
}

void CrGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsScene::mousePressEvent(event);

    if(!mouseGrabberItem()){
        closeNameEditor();
        closeEditor();

        if(!(event->modifiers() & Qt::ControlModifier)){
            clearSelection();
        }
        d_->startPos_ = event->scenePos();
        d_->selector_->setRect(QRectF(event->pos(), QSize(0,0)));
        d_->selector_->show();
    }
}

void CrGraphicsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if(d_->selector_->isVisible()){
        foreach (QGraphicsItem* item, items(d_->selector_->rect())) {
            item->setSelected(true);
        }
        d_->selector_->hide();
    }
    QGraphicsScene::mouseReleaseEvent(event);
}
