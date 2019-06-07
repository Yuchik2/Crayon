#include "crgraphicsscenenodeitem.h"

#include <QStack>
#include <QtMath>
#include <QPainter>
#include <QStyleOptionFrame>
#include <QStyleOptionViewItem>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QMouseEvent>

#include <CrNode>
#include <CrProject>
#include <CrCommands>
#include "crgraphicsscene.h"

#include <QDebug>

CrGraphicsSceneConnectorItem::CrGraphicsSceneConnectorItem(CrGraphicsSceneNodeItem *inItem, CrGraphicsSceneNodeItem *outItem) :
    QGraphicsItem (),
    inItem_(inItem),
    outItem_(outItem)
{
    setFlags(ItemIsSelectable);
    setZValue(0);
    stroker_.setWidth(2.5);
    stroker_.setDashPattern(Qt::SolidLine);

    if(inItem_){
        inItem_->inConnector_ = this;
        inItem_->updatePos();
    }
    if(outItem_){
        outItem_->outConnectors_.insert(this);
        outItem_->updatePos();
    }
}

CrGraphicsSceneConnectorItem::~CrGraphicsSceneConnectorItem()
{
    if(inItem_){
        inItem_->inConnector_ = nullptr;
    }
    if(outItem_){
        outItem_->outConnectors_.remove(this);
    }
}

void CrGraphicsSceneConnectorItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    painter->save();
    Q_UNUSED(widget)
    painter->setRenderHint(QPainter::Antialiasing, true);

    QPalette palette = option->palette;

    if(isSelected()){
        painter->setPen(QPen(palette.highlight(),1));
        painter->setBrush(palette.highlight());
    }else{
        painter->setPen(QPen(palette.window(),1));
        painter->setBrush(palette.window());
    }

    painter->drawPath(shape_);
    painter->restore();
}

QRectF CrGraphicsSceneConnectorItem::boundingRect() const { return shape_.controlPointRect();}

CrGraphicsSceneNodeItem *CrGraphicsSceneConnectorItem::inItem() const
{
    return inItem_;
}

CrGraphicsSceneNodeItem *CrGraphicsSceneConnectorItem::outItem() const
{
    return outItem_;
}

bool CrGraphicsSceneConnectorItem::invisibleMode() const
{
    return invisibleMode_;
}

void CrGraphicsSceneConnectorItem::setInvisibleMode(bool invisibleMode)
{
    if(invisibleMode_ == invisibleMode)
        return;
    invisibleMode_ = invisibleMode;
    if(invisibleMode){
        stroker_.setWidth(1.5);
        stroker_.setDashPattern({5, 3});
    }else {
        stroker_.setWidth(2.5);
        stroker_.setDashPattern(Qt::SolidLine);
    }
    update();
}

void CrGraphicsSceneConnectorItem::setInPos(const QPointF &inPos) {
    inPos_ = inPos;
    updateGeometry();
}

void CrGraphicsSceneConnectorItem::setOutPos(const QPointF &outPos){
    outPos_ = outPos;
    updateGeometry();
}

void CrGraphicsSceneConnectorItem::updateGeometry(){
    QPainterPath path = QPainterPath();
    path.moveTo(outPos_);
    qreal curvature = qAbs(inPos_.x() - outPos_.x() - 150) * 0.7;

    curvature = qBound(100.0, curvature, 500.0);

    path.cubicTo(QPointF(outPos_.x() + curvature, outPos_.y()),
                 QPointF(inPos_.x() - curvature, inPos_.y()), inPos_);

    shapeRect_ = path.controlPointRect();
    setPos(shapeRect_.topLeft());
    path.translate(-shapeRect_.topLeft().x(), -shapeRect_.topLeft().y());
    shape_ = stroker_.createStroke(path);
    prepareGeometryChange ();
}

CrGraphicsSceneConnectorItem* CrGraphicsSceneNodeItem::targetConnector_ = new CrGraphicsSceneConnectorItem(nullptr, nullptr);

CrGraphicsSceneNodeItem* CrGraphicsSceneNodeItem::targetItem_ = nullptr;

CrGraphicsSceneNodeItem::CrGraphicsSceneNodeItem(CrNode *node, CrGraphicsScene *s) :
    QGraphicsItem(),
    node_(node),
    scene_(s)
{
    Q_ASSERT(node_);
    Q_ASSERT(scene_);

    setFlags(ItemIsSelectable);
    setZValue(1);

    QObject::connect(node_, &CrNode::nameChanged, scene_, [this](){
        updateGeometry();
        if(scene()){
            update(nameRect_);
        }
    });

    QObject::connect(node_, &CrNode::updateEditorPreview, scene_, [this](){
        updateGeometry();
        if(scene()){
            update(editorRect_);
        }
    });

    QObject::connect(node_, &CrNode::flagsChanged, scene_, [this](){
        updateGeometry();
        if(scene()){
            update();
        }
    });

    QObject::connect(node_, &CrNode::selectedChanged, scene_, [this](){
        setSelected(node_->isSelected());
    });
    updateGeometry();
}

CrGraphicsSceneNodeItem::~CrGraphicsSceneNodeItem()
{
    QObject::disconnect(node_, &CrNode::nameChanged, scene_, nullptr);
    QObject::disconnect(node_, &CrNode::updateEditorPreview, scene_,  nullptr);
    QObject::disconnect(node_, &CrNode::flagsChanged, scene_,  nullptr);
    QObject::disconnect(node_, &CrNode::selectedChanged, scene_, nullptr);

}

CrNode *CrGraphicsSceneNodeItem::node() const
{
    return node_;
}

QList<CrGraphicsSceneNodeItem *> CrGraphicsSceneNodeItem::childItems()
{
    return childItems_;
}

void CrGraphicsSceneNodeItem::insertChildItem(int index, CrGraphicsSceneNodeItem *item)
{
    childItems_.insert(index, item);
    item->parentItem_ = this;
    if(expanded_){
        item->setParentItem(this);
    }

    updateGeometry();
}

void CrGraphicsSceneNodeItem::removeChildItem(int index)
{
    CrGraphicsSceneNodeItem* item = childItems_.at(index);
    item->parentItem_ = nullptr;
    childItems_.removeAt(index);
    if(expanded_){
        updateGeometry();
    }
}

void CrGraphicsSceneNodeItem::setExpanded(bool expanded)
{
    if(expanded_ == expanded)
        return;

    expanded_ = expanded;

    if(expanded_){
        foreach (CrGraphicsSceneNodeItem* childItem, childItems_) {
            childItem->setParentItem(this);
        }
    }else {
        foreach(CrGraphicsSceneNodeItem* childItem, childItems_){
            childItem->setParentItem(nullptr);
            scene_->removeItem(childItem);
        }
    }

    QStack<CrGraphicsSceneNodeItem*> stack;
    stack << this;
    while (!stack.isEmpty()) {
        CrGraphicsSceneNodeItem* item = stack.pop();
        if(item->inConnector_)
            item->inConnector_->setInvisibleMode(!item->scene() || !item->inConnector_->outItem()->scene());
        foreach(CrGraphicsSceneConnectorItem* outConnector, item->outConnectors_){
            outConnector->setInvisibleMode(!item->scene() || !outConnector->inItem()->scene());
        }
        stack << item->childItems_.toVector();
    }

    updateGeometry();
}

void CrGraphicsSceneNodeItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    if(isSelected()){
        painter->fillRect(headerRect_.adjusted(1,1,-2,-2), option->palette.highlight());
    }else{
        painter->fillRect(headerRect_.adjusted(1,1,-2,-2), option->palette.window());
    }

    QStyleOptionFrame frameOpt;
    frameOpt.initFrom(widget);
    frameOpt.frameShape = QFrame::WinPanel;
    frameOpt.lineWidth = 1;
    frameOpt.midLineWidth = 1;
    frameOpt.rect = headerRect_.toRect();
    frameOpt.state |= QStyle::State_Raised;

    if(isSelected())
        frameOpt.state |= QStyle::State_Selected;
    scene_->style()->drawControl(QStyle::CE_ShapedFrame, &frameOpt, painter, nullptr);


    QPen borderPen;
    borderPen.setBrush(option->palette.midlight());
    painter->setPen(borderPen);

    if(node_->flags() & CrNode::In){
        painter->setBrush(option->palette.window());
        if(anchorsState_ & InAccept){
            painter->setBrush(Qt::green);
        }
        if(anchorsState_ & InReject){
            painter->setBrush(Qt::red);
        }
        painter->drawChord(inAnchorRect_, 45 * 16 , 270 * 16);
    }
    if(node_->flags() & CrNode::Out){
        painter->setBrush(option->palette.window());
        if(anchorsState_ & OutAccept){
            painter->setBrush(Qt::green);
        }
        if(anchorsState_ & OutReject){
            painter->setBrush(Qt::red);
        }
        painter->drawChord(outAnchorRect_,  225 * 16 , 270 * 16);
    }

    QStyleOption branchOption(*option);
    branchOption.rect = branchRect_.toRect();
    branchOption.state |= QStyle::State_Item
            | (childItems_.isEmpty() ? QStyle::State_None : QStyle::State_Children)
            | (expanded_ ? QStyle::State_Open : QStyle::State_None)
            | (isSelected() ? QStyle::State_Selected : QStyle::State_None);

    scene_->style()->drawPrimitive(QStyle::PE_IndicatorBranch, &branchOption, painter, nullptr);


    QStyleOptionViewItem nameOpt;
    nameOpt.initFrom(widget);
    nameOpt.rect = nameRect_.toRect();
    nameOpt.text = node_->name();
    nameOpt.features = QStyleOptionViewItem::HasDisplay;
    nameOpt.displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
    nameOpt.state = QStyle::State_Enabled;
    nameOpt.state |= isSelected() ? QStyle::State_Selected : QStyle::State_None;

    if(!node_->icon().isNull()){
        nameOpt.icon = node_->icon();
        nameOpt.features |= QStyleOptionViewItem::HasDecoration;
        int iconSize = scene_->style()->pixelMetric(QStyle::PM_SmallIconSize, nullptr, nullptr);
        nameOpt.decorationSize = QSize(iconSize, iconSize);
        nameOpt.decorationAlignment = Qt::AlignCenter;
    }
    scene_->style()->drawControl(QStyle::CE_ItemViewItem, &nameOpt, painter, nullptr);

    //        if(node_->flags() & CrNode::ActiveEditor){
    //            painter->fillRect(viewRect_, sceneNode_->palette_.base());
    //        }

    QStyleOptionViewItem editorOpt;
    editorOpt.initFrom(widget);
    editorOpt.rect = editorRect_.toRect();
    editorOpt.features = QStyleOptionViewItem::HasDisplay;
    editorOpt.displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
    editorOpt.state = QStyle::State_Enabled;
    editorOpt.state |= isSelected() ? QStyle::State_Selected : QStyle::State_None;
    editorOpt.state |= (node_->flags() & CrNode::ActiveEditor ? QStyle::State_Enabled : QStyle::State_None);
    node_->paintEditorPreview(painter, editorOpt);

    painter->restore();
}

QRectF CrGraphicsSceneNodeItem::boundingRect() const
{
    return (inAnchorRect_ | branchRect_ | nameRect_ | editorRect_ | outAnchorRect_).marginsAdded({5,5,5,5});
}

QVariant CrGraphicsSceneNodeItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    switch (change) {
    case QGraphicsItem::ItemPositionHasChanged :
        updatePos();
        break;
    case QGraphicsItem::ItemSelectedHasChanged :{
        node_->setSelected(value.toBool());

//        if(value.toBool()){
//            setZValue(3);
//        }else {
//            if(!editor_)
//                setZValue(1);
//        }
        break;
    }
    default:
        break;
    }
    return QGraphicsItem::itemChange(change, value);
}

void CrGraphicsSceneNodeItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if(nameRect_.contains(event->pos()) && node_->flags() & CrNode::EditableName){
        scene_->openNameEditor(node_);
        return;
    }

    if(editorRect_.contains(event->pos()) && node_->flags() & CrNode::ActiveEditor){
        scene_->openEditor(node_);
        return;
    }

    QGraphicsItem::mouseDoubleClickEvent(event);
}

void CrGraphicsSceneNodeItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    //    if(nodeItem){
    //        QPointF pos = nodeItem->mapFromScene(mapToScene(event->pos()));

    //        if(nodeItem && nodeItem->editorRect_.contains(pos)){
    //            QMouseEvent ev(QEvent::MouseMove, pos, event->windowPos(),
    //                           event->screenPos(), event->button(), event->buttons(), event->modifiers(), event->source());
    //            QStyleOptionViewItem opt;
    //            opt.initFrom(this);
    //            opt.rect = nodeItem->editorRect_.toRect();
    //            if(nodeItem->node_->eventEditorPreview(&ev, opt))
    //                return;
    //        }
    //    }

    //    QGraphicsView::mouseMoveEvent(event);
    //    return;

    if(anchorsState_ == InAccept){
        targetConnector_->setOutPos(event->scenePos());

        if(targetItem_){
            targetItem_->anchorsState_ = None;
            targetItem_->update(targetItem_->outAnchorRect_);
            targetItem_ = nullptr;
        }

        CrGraphicsSceneNodeItem* nodeItem = qgraphicsitem_cast<CrGraphicsSceneNodeItem*>(scene_->itemAt(event->scenePos(), QTransform()));

        if(nodeItem && nodeItem->node_->flags() & CrNode::Out){
            targetItem_ = nodeItem;
            if(node_->isValidInputNode(targetItem_->node_)){
                targetItem_->anchorsState_ = OutAccept;
                targetItem_->update(targetItem_->outAnchorRect_);
                targetConnector_->setOutPos(targetItem_->mapToScene(targetItem_->outAnchorRect_.center()));
            }else {
                targetItem_->anchorsState_ = OutReject;
                targetItem_->update(targetItem_->outAnchorRect_);
            }
        }
        return;

    } else if(anchorsState_ == OutAccept){
        targetConnector_->setInPos(event->scenePos());

        if(targetItem_){
            targetItem_->anchorsState_ = None;
            targetItem_->update(targetItem_->inAnchorRect_);
            targetItem_ = nullptr;
        }

        CrGraphicsSceneNodeItem* nodeItem = qgraphicsitem_cast<CrGraphicsSceneNodeItem*>(scene_->itemAt(event->scenePos(), QTransform()));

        if(nodeItem && nodeItem->node_->flags() & CrNode::In){
            targetItem_= nodeItem;
            if(targetItem_->node_->isValidInputNode(node_)){
                targetItem_->anchorsState_ = InAccept;
                targetItem_->update(targetItem_->inAnchorRect_);
                targetConnector_->setInPos(targetItem_->mapToScene(targetItem_->inAnchorRect_.center()));
            }else {
                targetItem_->anchorsState_ = InReject;
                targetItem_->update(targetItem_->inAnchorRect_);
            }
        }
        return;
    }

    foreach (QGraphicsItem* item, scene_->selectedItems()) {
        CrGraphicsSceneNodeItem* movableItem = qgraphicsitem_cast<CrGraphicsSceneNodeItem*>(item);
        if(!movableItem || movableItem->parentItem_)
            continue;

        movableItem->setPos(movableItem->pos() + (event->scenePos() - event->lastScenePos()));
    }
}

void CrGraphicsSceneNodeItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{

    if(branchRect_.contains(event->pos())){
        setExpanded(!expanded_);
        QGraphicsItem::mousePressEvent(event);
        return;
    }

    if(headerRect_.contains(event->pos())){

        if(!(event->modifiers() & Qt::ControlModifier)){
            scene_->clearSelection();
        }
        setSelected(true);
    }

    if(inAnchorRect_.contains(event->pos())){
        if(!(node_->flags() & CrNode::In))
            return;

        targetConnector_->setInPos(mapToScene(inAnchorRect_.center()));
        targetConnector_->setOutPos(event->scenePos());
        scene_->addItem(targetConnector_);
        anchorsState_ = InAccept;
        return;
    }

    if(outAnchorRect_.contains(event->pos())){

        if(!(node_->flags() & CrNode::Out))
            return;

        targetConnector_->setOutPos(mapToScene(outAnchorRect_.center()));
        targetConnector_->setInPos(event->scenePos());
        scene_->addItem(targetConnector_);
        anchorsState_ = OutAccept;
        return;

    }
    if(editorRect_.contains(event->pos())){
        QMouseEvent ev(QEvent::MouseButtonPress, event->pos(), event->screenPos(),
                       event->button(), event->buttons(), event->modifiers());

        QStyleOptionViewItem editorOpt;
        //editorOpt.initFrom(widget);
        editorOpt.rect = editorRect_.toRect();
        editorOpt.features = QStyleOptionViewItem::HasDisplay;
        editorOpt.displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
        editorOpt.state = QStyle::State_Enabled;
        editorOpt.state |= isSelected() ? QStyle::State_Selected : QStyle::State_None;
        editorOpt.state |= (node_->flags() & CrNode::ActiveEditor ? QStyle::State_Enabled : QStyle::State_None);

        if(node_->eventEditorPreview(&ev, editorOpt)){
            return;
        }
    }
}

void CrGraphicsSceneNodeItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if(targetConnector_->scene()){
        if(targetItem_){
            if(anchorsState_ == InAccept && targetItem_->anchorsState_ == OutAccept){
                node_->project()->pushUndoCommand(new CrCommands::ChangeInputNode( node_, targetItem_->node_));
            }else if(anchorsState_ == OutAccept && targetItem_->anchorsState_ == InAccept) {
                node_->project()->pushUndoCommand(new CrCommands::ChangeInputNode(targetItem_->node_, node_));
            }
            targetItem_->anchorsState_ = None;
            targetItem_->update();
            targetItem_ = nullptr;
        }
        anchorsState_ = None;
        update();

        scene_->removeItem(targetConnector_);
    }

    foreach (QGraphicsItem* item, scene_->selectedItems()) {
        CrGraphicsSceneNodeItem* movableItem = qgraphicsitem_cast<CrGraphicsSceneNodeItem*>(item);
        if(!movableItem || movableItem->parentItem_)
            continue;


        qreal x = qRound(movableItem->x() / scene_->moveStep()) * scene_->moveStep();
        qreal y = qRound(movableItem->y() / scene_->moveStep()) * scene_->moveStep();
        movableItem->setPos(QPointF(x, y));
    }



}

void CrGraphicsSceneNodeItem::updateGeometry()
{
    qreal margin = scene_->style()->pixelMetric(QStyle::PM_HeaderMargin);
    qreal anchorSize = scene_->style()->pixelMetric(QStyle::PM_SizeGripSize);
    qreal indent = scene_->style()->pixelMetric(QStyle::PM_TreeViewIndentation);
    qreal sizeStep = 15;
    int iconSize = scene_->style()->pixelMetric(QStyle::PM_SmallIconSize, nullptr, nullptr);
    QSizeF minFieldSize = QSizeF(15,15);
    QSizeF maxFieldSize = QSizeF(250.0, 150.0);

    branchRect_.moveLeft(margin);
    nameRect_.moveTop(margin);
    editorRect_.moveTop(margin);


    inAnchorRect_.setSize(QSizeF(anchorSize, anchorSize));
    inAnchorRect_.moveRight(0);
    outAnchorRect_.setSize(QSizeF(anchorSize, anchorSize));


    if(!childItems_.isEmpty()){
        int branchSize = scene_->style()->pixelMetric(QStyle::PM_HeaderMarkSize);
        branchRect_.setSize(QSizeF(branchSize, branchSize));
    }else{
        branchRect_.setSize(QSizeF(0, 0));
    }

    nameRect_.moveLeft(branchRect_.right() + margin);

    QStyleOptionViewItem nameOpt;
    // nameOpt.initFrom(widget);
    nameOpt.rect = nameRect_.toRect();
    nameOpt.text = node_->name();
    nameOpt.features = QStyleOptionViewItem::HasDisplay;
    nameOpt.displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
    nameOpt.state = QStyle::State_Enabled;
    nameOpt.state |= isSelected() ? QStyle::State_Selected : QStyle::State_None;

    if(!node_->icon().isNull()){
        nameOpt.icon = node_->icon();
        nameOpt.features |= QStyleOptionViewItem::HasDecoration;
        nameOpt.decorationSize = QSize(iconSize, iconSize);
        nameOpt.decorationAlignment = Qt::AlignCenter;
    }

    QSizeF nameSizeHint = scene_->style()->sizeFromContents(QStyle::CT_ItemViewItem, &nameOpt, QSize(), nullptr);
    nameSizeHint = nameSizeHint.boundedTo(maxFieldSize).expandedTo(minFieldSize);
    nameSizeHint.setWidth(nameSizeHint.width() + scene_->style()->pixelMetric(QStyle::PM_HeaderMargin));

    QStyleOptionViewItem editorOpt;
    // editorOpt.initFrom(widget);
    editorOpt.rect = editorRect_.toRect();
    editorOpt.features = QStyleOptionViewItem::HasDisplay;
    editorOpt.displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
    editorOpt.state = QStyle::State_Enabled;
    editorOpt.state |= isSelected() ? QStyle::State_Selected : QStyle::State_None;
    editorOpt.state |= (node_->flags() & CrNode::ActiveEditor ? QStyle::State_Enabled : QStyle::State_None);

    QSizeF viewSizeHint;
    viewSizeHint = node_->sizeHintEditorPreview(editorOpt);
    viewSizeHint = viewSizeHint.boundedTo(maxFieldSize).expandedTo(minFieldSize);
    viewSizeHint.setWidth(viewSizeHint.width() + scene_->style()->pixelMetric(QStyle::PM_HeaderMargin));

    if(editor_){
        viewSizeHint = viewSizeHint.expandedTo(editor_->sizeHint());
    }

    if(expanded_){
        foreach (CrGraphicsSceneNodeItem* child, childItems_) {
            nameSizeHint.rwidth() = qMax(nameSizeHint.width(), child->nameSizeHint_.width() + indent + child->branchRect_.width());
            viewSizeHint.rwidth() = qMax(viewSizeHint.width(), child->editorSizeHint_.width());
        }
    }

    QSizeF sizeHint;
    sizeHint.rwidth() = branchRect_.width() + nameSizeHint.width() + viewSizeHint.width() + margin * 4;
    sizeHint.rheight() = qMax(branchRect_.height(), qMax(nameSizeHint.height(),viewSizeHint.height())) + margin * 2;


    sizeHint.rwidth() = qCeil(sizeHint.width() / sizeStep) * sizeStep;
    sizeHint.rheight() = qCeil(sizeHint.height() / sizeStep) * sizeStep;

    headerRect_.setHeight(sizeHint.height());
    branchRect_.moveTop((headerRect_.height() - branchRect_.height()) / 2);
    nameRect_.setHeight(sizeHint.height() - margin * 2);
    editorRect_.setHeight(sizeHint.height() - margin * 2);
    inAnchorRect_.moveTop((headerRect_.height() - inAnchorRect_.height()) / 2) ;
    outAnchorRect_.moveTop((headerRect_.height() - inAnchorRect_.height()) / 2) ;

    if(expanded_){
        foreach (CrGraphicsSceneNodeItem* child, childItems_) {
            sizeHint.rwidth() = qMax(sizeHint.width(), child->sizeHint_.width() + indent);
            sizeHint.rheight() += child->sizeHint_.height();
        }
    }

    if(sizeHint != sizeHint_ || nameSizeHint != nameSizeHint_ || viewSizeHint != editorSizeHint_){
        sizeHint_ = sizeHint;
        nameSizeHint_ = nameSizeHint;
        editorSizeHint_ = viewSizeHint;

        if(parentItem_ && scene()){
            parentItem_->updateGeometry();
        }else{
            headerRect_.setWidth(sizeHint_.width());

            nameRect_.setWidth(nameSizeHint_.width());

            editorRect_.setWidth(editorSizeHint_.width());
            editorRect_.moveLeft(nameRect_.right() + margin);
            editorRect_.setRight(headerRect_.right() - margin);

            outAnchorRect_.moveLeft(headerRect_.right());

            QStack<CrGraphicsSceneNodeItem*> stack;
            auto it = childItems_.end();
            while(it != childItems_.begin()){
                --it;
                stack << (*it);
            }

            qreal y = headerRect_.bottom() + scenePos().y();
            while (!stack.isEmpty()) {
                CrGraphicsSceneNodeItem* item = stack.pop();

                item->headerRect_.setWidth(item->parentItem_->headerRect_.width() - indent);

                item->nameRect_.setWidth(item->parentItem_->nameRect_.width() - item->branchRect_.width());

                item->editorRect_.setWidth(item->parentItem_->editorRect_.width());
                item->editorRect_.moveLeft(item->nameRect_.right() + margin);
                item->editorRect_.setRight(item->headerRect_.right() - margin);

                item->outAnchorRect_.moveLeft(item->headerRect_.right());

                if(item->editor_){
                    item->editor_->move(item->editorRect_.topLeft().toPoint());
                }

                item->setPos(indent, y - item->parentItem_->scenePos().y());
                y += item->headerRect_.height();

                if(item->expanded_){
                    auto it = item->childItems_.end();
                    while(it != item->childItems_.begin()){
                        --it;
                        stack.push(*it);
                    }
                }
            }
            updatePos();
        }
        prepareGeometryChange();
    }
    update();
}

void CrGraphicsSceneNodeItem::updatePos()
{
    CrGraphicsSceneNodeItem* visibleItem = this;

    while (!visibleItem->scene()) {
        visibleItem = visibleItem->parentItem_;
        if(!visibleItem)
            return;
    }

    if(inConnector_){
        inConnector_->setInPos(visibleItem->mapToScene(visibleItem->inAnchorRect_.center()));
    }

    foreach (CrGraphicsSceneConnectorItem* outConnector, outConnectors_) {
        outConnector->setOutPos(visibleItem->mapToScene(visibleItem->outAnchorRect_.center()));
    }

    foreach (CrGraphicsSceneNodeItem* child, childItems_) {
        child->updatePos();
    }

}
