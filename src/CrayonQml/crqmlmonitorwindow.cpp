#include "crqmlmonitorwindow.h"

#include <QQuickItem>
#include <QMimeData>
#include <QUndoCommand>

#include <crproject.h>
#include <crcommands.h>
#include <crmodule.h>

#include <crqmlmonitornode.h>
#include "crqmlmonitoritem.h"

#include <QDebug>

CrQmlMonitorWindow::CrQmlMonitorWindow(CrQmlMonitorNode* node, QWindow *parent) :
    QQuickWindow(parent),
    node_(node)
{
    //   setColor(Qt::black);
    setMinimumSize(QSize(320, 240));

    monitor_ = node_->createItem();
    monitor_->setParentItem(contentItem());

    QQmlComponent selectorComponent(CrQmlNode::engine(), "qrc:/CrayonQmlPluginResources/CrQmlMonitorSelector.qml");
    selector_ = static_cast<QQuickItem*>(selectorComponent.create());
    selector_->setVisible(false);
    selector_->setParentItem(monitor_);
    selector_->setZ(1000);

    QQmlComponent rubberBandComponent(CrQmlNode::engine(), "qrc:/CrayonQmlPluginResources/CrQmlMonitorRubberBand.qml");
    rubberBand_ = static_cast<QQuickItem*>(rubberBandComponent.create());
    rubberBand_->setVisible(false);
    rubberBand_->setParentItem(monitor_);
    rubberBand_->setZ(1000);

    connect(node_, SIGNAL(nodeAdded(CrQmlNode*)), this, SLOT(addNode(CrQmlNode*)));
    connect(node_, SIGNAL(nodeRemoved(CrQmlNode*)), this, SLOT(removeNode(CrQmlNode*)));

    foreach(CrQmlNode* node, node_->qmlNodes()){
        addNode(node);
    }
    updateSelector();
}

bool CrQmlMonitorWindow::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::DragMove:{
        QDragMoveEvent* e = static_cast<QDragMoveEvent*>(event);
        if(e->mimeData()->hasFormat("application/crayonf/moduleid") ||
                e->mimeData()->hasFormat("application/crayonf/modulekey")){

            e->acceptProposedAction();
            e->accept();
        }
        return true;
    }
    case QEvent::DragEnter:{
        QDragEnterEvent* e = static_cast<QDragEnterEvent*>(event);
        if(e->mimeData()->hasFormat("application/crayonf/moduleid") ||
                e->mimeData()->hasFormat("application/crayonf/modulekey")){

            e->acceptProposedAction();
            e->accept();
        }
        return true;
    }
    case QEvent::Drop:{
        QDropEvent* e = static_cast<QDropEvent*>(event);
        const QMimeData* data = e->mimeData();

        if(data->hasFormat("application/crayonf/modulekey")){

            QString id;
            QVersionNumber version;
            QDataStream stream(data->data("application/crayonf/modulekey"));
            stream >> id >> version;

            CrModule* module = CrModule::module(id, version);
            if(!module){
                node_->project()->pushWarningMessage(QObject::tr("Create module fault."),
                                                              QObject::tr("Create module fault. Module by id:\"%1\" version: \"%2\" not found.").
                                                              arg(id).arg(version.toString()), nullptr);
                return false;
            }

            CrNode* newNode = module->createNode();
            if(!newNode){
                 node_->project()->pushWarningMessage(QObject::tr("Create module fault."),
                                                              QObject::tr("Create module fault. Module by id:\"%1\" version: \"%2\" error create node.").
                                                              arg(id).arg(version.toString()), nullptr);
                return false;
            }

            node_->project()->pushUndoCommand(new CrCommands::MoveNode(newNode, node_, node_->childNodes().size()));
            if(CrQmlNode* qmlNode = qobject_cast<CrQmlNode*>(newNode)){
                QPointF pos  = monitor_->mapFromScene(e->posF());
                QRectF geom = node_->geometryItem(qmlNode);
                geom.moveLeft(pos.x() - geom.width() / 2);
                geom.moveTop(pos.y() - geom.height() / 2);
                node_->setGeometryItem(qmlNode, geom);
            }


        } else if(data->hasFormat("application/crayonf/nodes")){

            QList<quintptr> ptrNodes;

            QDataStream stream(data->data("application/crayonf/nodes"));
            stream >> ptrNodes;

            if(ptrNodes.isEmpty())
                return false;

            QList<CrNode*> movableNodes;
            foreach(quintptr ptr, ptrNodes){
                CrNode* node = reinterpret_cast<CrNode*>(ptr);
                if(!node)
                    continue;
                if(node->isModule()){
                    CrNode* testParent = node_;
                    while (testParent) {
                        if(testParent == node)
                            break;
                        testParent = testParent->parentNode();
                    }

                    if(testParent != node)
                        movableNodes << node;
                }
            }

            if(movableNodes.isEmpty())
                return false;

            if(movableNodes.size() == 1){
                node_->project()->pushUndoCommand(new CrCommands::MoveNode(movableNodes.first(), node_, node_->childNodes().size()));

                if(CrQmlNode* qmlNode = qobject_cast<CrQmlNode*>(movableNodes.first())){
                    QPointF pos  = monitor_->mapFromScene(e->posF());
                    QRectF geom = node_->geometryItem(qmlNode);
                    geom.moveLeft(pos.x() - geom.width() / 2);
                    geom.moveTop(pos.y() - geom.height() / 2);
                    node_->setGeometryItem(qmlNode, geom);
                }

            }else {
                QUndoCommand* parentCommand = new QUndoCommand();
                parentCommand->setText(QString("Move %1 nodes.").arg(movableNodes.size()));
                int row = node_->childNodes().size();
                foreach(CrNode* node, movableNodes){
                    new CrCommands::MoveNode(node, node_, row++, parentCommand);
                }
                node_->project()->pushUndoCommand(parentCommand);

                QPointF pos  = monitor_->mapFromScene(e->posF());
                qreal dx = 0;
                foreach(CrNode* node, movableNodes){
                    if(CrQmlNode* qmlNode = qobject_cast<CrQmlNode*>(node)){
                        QRectF geom = node_->geometryItem(qmlNode);
                        geom.moveLeft(pos.x() - geom.width() / 2 + dx);
                        geom.moveTop(pos.y() - geom.height() / 2);
                        node_->setGeometryItem(qmlNode, geom);
                        dx += geom.width();
                    }
                }
            }
            return true;
        }
        return false;
    }
    default:
        return QQuickWindow::event(event);
    }
}

void CrQmlMonitorWindow::mouseMoveEvent(QMouseEvent *event)
{
    QPointF pos = monitor_->mapFromScene(event->pos());
    QPointF posPrev = monitor_->mapFromScene(oldPos_);

    QCursor cur(cursor());
    if(selector_->isVisible() && QRectF(selector_->position(), selector_->size()).contains(pos)){
        if(QRectF(selector_->x() + selector_->width() - 10, selector_->y() + selector_->height() - 10, 10, 10).contains(pos)){
            cur.setShape(Qt::SizeFDiagCursor);
        }else {
            cur.setShape(Qt::SizeAllCursor);
        }
    }else{
        cur.setShape(Qt::ArrowCursor);
    }
    setCursor(cur);

    if(state_ == DragCamera){
        monitor_->setPosition(monitor_->position() + (event->pos() - oldPos_));
        oldPos_ = event->pos();

    } else if(state_ == Select){
        QRectF selectionRect = QRectF(posPrev, pos).normalized();
        rubberBand_->setSize(selectionRect.size());
        rubberBand_->setPosition(selectionRect.topLeft());

    } else if(state_ == Move){
        foreach(CrQmlNode* selNode, selectedNodes_){
            QRectF newGeom = node_->geometryItem(selNode);
            newGeom.moveTopLeft(newGeom.topLeft() + (pos - posPrev));
            node_->setGeometryItem(selNode, newGeom);
        }
        selector_->setPosition(selector_->position() + (pos - posPrev));
        oldPos_ = event->pos();
    } else if(state_ == Resize){

        QRectF oldRect = QRectF(selector_->position(), selector_->size());
        QRectF newRect(oldRect);
        newRect.setBottomRight(newRect.bottomRight() + (pos - posPrev));

        qreal kx = newRect.width()/(oldRect.width() > 0 ? oldRect.width(): newRect.width());
        qreal ky = newRect.height()/(oldRect.height() > 0 ? oldRect.height(): newRect.height());

        foreach(CrQmlNode* selNode, selectedNodes_){
            QRectF geom = node_->geometryItem(selNode);
            geom.setWidth(geom.width() * kx);
            geom.setHeight(geom.height() * ky);
            geom.moveLeft(geom.x() * kx + (1 - kx) * newRect.x());
            geom.moveTop(geom.y() * ky + (1 - ky) * newRect.y());
            node_->setGeometryItem(selNode, geom);
        }
        selector_->setPosition(newRect.topLeft());
        selector_->setSize(newRect.size());
        oldPos_ = event->pos();
    }

}

void CrQmlMonitorWindow::mousePressEvent(QMouseEvent *event)
{
    QPointF pos = monitor_->mapFromScene(event->pos());
    oldPos_ = event->pos();
    startPos_ = event->pos();

    if(event->buttons() & Qt::RightButton){
        state_ = DragCamera;
        return;
    }

    currentNode_ = nullptr;
    foreach(CrQmlNode* node,  node_->qmlNodes()){
        if(node_->geometryItem(node).contains(pos)){
            currentNode_ = node;
            break;
        }
    }

    if(currentNode_ && !currentNode_->isSelected()){
        if(event->modifiers() & Qt::CTRL){
            currentNode_->setSelected(true);
        }else{
            node_->project()->setSelectedNodes({currentNode_});
        }
    }

    if(selector_->isVisible() && QRectF(selector_->position(), selector_->size()).contains(pos)){
        if(QRectF(selector_->x() + selector_->width() - 10, selector_->y() + selector_->height() - 10, 10, 10).contains(pos)){
            cursor().setShape(Qt::SizeFDiagCursor);
        }else {
            cursor().setShape(Qt::SizeAllCursor);
        }
    }else{
        cursor().setShape(Qt::ArrowCursor);
    }

    if(selector_->isVisible() && QRectF(selector_->position(), selector_->size()).contains(pos)){
        if(QRectF(selector_->x() + selector_->width() - 10, selector_->y() + selector_->height() - 10, 10, 10).contains(pos)){
            state_ = Resize;
        }else {
            state_ = Move;
        }
    }else{
        state_ = Select;
        QRectF selectionRect = QRectF(pos, pos).normalized();
        rubberBand_->setSize(selectionRect.size());
        rubberBand_->setPosition(selectionRect.topLeft());
        rubberBand_->setVisible(true);
    }

}

void CrQmlMonitorWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if(state_ == Select && rubberBand_->isVisible()){
        if(!(event->modifiers() & Qt::CTRL)){
            node_->project()->clearSelectedNodes();
        }
        QRectF rubberBandGeom = QRectF(rubberBand_->position(), rubberBand_->size());
        foreach(CrQmlNode* node,  node_->qmlNodes()){
            if(node_->geometryItem(node).intersects(rubberBandGeom)){
                node->setSelected(true);
            }
        }
        rubberBand_->setVisible(false);
    } else if(state_ == Move){
        //        if(!(event->modifiers() & Qt::CTRL)){
        //            if(currentNode_){
        //                node_->project()->setSelectedNodes({currentNode_});
        //            }else {
        //                node_->project()->clearSelectedNodes();
        //            }
        //        }
    }

    state_ = None;

    QList<CrQmlNode*> selectedNodes = node_->selectedQmlNodes();

    if(selectedNodes.isEmpty()){
        selector_->setVisible(false);
    }else{
        QRectF selectorGeom = node_->geometryItem(selectedNodes.first());
        for (int i = 1; i < selectedNodes.size(); ++i) {
            selectorGeom |= node_->geometryItem(selectedNodes.at(i));
        }
        selector_->setPosition(selectorGeom.topLeft());
        selector_->setSize(selectorGeom.size());
        selector_->setVisible(true);
    }

}

void CrQmlMonitorWindow::wheelEvent(QWheelEvent *event)
{

    if(event->delta() > 0)
    {
        if(scale_ < 5){
            scale_ *= 1.3;
        }
    }else{
        if(scale_ > 0.1){
            scale_ *= 1 / 1.3;
        }
    }
    monitor_->setScale(scale_);

    //    monitor_->setX(event->x());
    //    monitor_->setY(event->y());

    //    monitor_->setX((event->x() - monitor_->x() - monitor_->width() / 2) / scale_);
    //    monitor_->setY((event->y() - monitor_->y() - monitor_->height() / 2) / scale_);
}

void CrQmlMonitorWindow::addNode(CrQmlNode *node)
{
    connect(node, &CrNode::selectedChanged, this, &CrQmlMonitorWindow::changeSelect);
    if(node->isSelected()){
        selectedNodes_.insert(node);
        updateSelector();
    }
}

void CrQmlMonitorWindow::removeNode(CrQmlNode *node)
{
    disconnect(node, &CrNode::selectedChanged, this, &CrQmlMonitorWindow::changeSelect);
    if(node->isSelected()){
        selectedNodes_.remove(node);
        updateSelector();
    }
}

void CrQmlMonitorWindow::changeSelect()
{
    CrQmlNode* node = static_cast<CrQmlNode*>(sender());
    if(node->isSelected()){
        selectedNodes_.insert(node);
    }else{
        selectedNodes_.remove(node);
    }

    updateSelector();
}

void CrQmlMonitorWindow::updateSelector()
{
    if(selectedNodes_.isEmpty()){
        selector_->setVisible(false);
    }else{
        selector_->setVisible(true);
        QRectF selectorGeom;
        auto it = selectedNodes_.cbegin();
        while(it != selectedNodes_.cend()){
            if(selectorGeom.isValid()){
                selectorGeom |= node_->geometryItem(*it);
            }else{
                selectorGeom = node_->geometryItem(*it);
            }
            ++it;
        }
        selector_->setPosition(selectorGeom.topLeft());
        selector_->setSize(selectorGeom.size());
        selector_->setVisible(true);
    }
}





