#include "crgraphicsscenenodeeditor.h"

#include <QtMath>
#include <QLineEdit>
#include <QMenu>
#include <QMouseEvent>
#include <QDropEvent>
#include <QDragMoveEvent>
#include <QMimeData>
#include <QScrollBar>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QStack>

#include <CrModule>
#include <CrNode>
#include <CrProject>
#include <CrCommands>
#include <CrNodeSettingsDialog>
#include <CrProjectExplorerDialog>
#include <CrProjectModulesExplorerDialog>

#include "crgraphicsscene.h"
#include "crgraphicsscenenodeitem.h"

#include <QDebug>

CrGraphicsSceneEditor::CrGraphicsSceneEditor(CrGraphicsScene *scene) :
    QGraphicsView(),
    scene_(scene)
{
    connect(scene_, &QObject::destroyed, this, &QObject::deleteLater);
    viewport()->installEventFilter(this);
    setScene(scene_);

    setContextMenuPolicy(Qt::CustomContextMenu);
    auto popup = new QMenu(this);

    auto addNewModules = new QAction(tr("&Add modules"), this);
    addNewModules->setIcon(QIcon(":/CrayonGraphicsEditorResources/add_icon.svg"));
    connect(addNewModules, &QAction::triggered, this, [this](){

        CrProjectModulesExplorerDialog* dialog = CrProjectModulesExplorerDialog::commonDialog();
        dialog->setProject(scene_->node()->project());
        if(!dialog->exec())
            return ;

        CrModule* module = dialog->currentModule();
        if(module){

            QList<CrNode*> newNodes;
            for (int i = 0; i < CrProjectModulesExplorerDialog::commonDialog()->count(); ++i) {
                CrNode* newNode = module->createNode();
                if(!newNode){
                    scene_->node()->project()->pushErrorMessage(QObject::tr("error create modular node."),
                                                                QObject::tr("Failed to create modular node."
                                                                            "The module by id: \"%1\" version: \"%2\" contains error.").
                                                                arg(module->id()).arg(module->version().toString()), nullptr);
                    return;
                }
                newNodes <<  newNode;
            }

            QUndoCommand* parentCommand = new QUndoCommand();

            if(dialog->count() == 1){
                parentCommand->setText(QString(tr("Add node.")));
            }else {
                parentCommand->setText(QString(tr("Add %1 nodes.")).arg(dialog->count()));
            }

            for (int i = 0; i < newNodes.size(); ++i) {
                new CrCommands::MoveNode(newNodes.at(i), scene_->node(), -1, parentCommand);
            }
            scene_->node()->project()->pushUndoCommand(parentCommand);

            QPointF pos = mapToScene(startPosCursor_);
            for (int i = 0; i < newNodes.size(); ++i) {
                scene_->setNodePos(newNodes.at(i), pos);
                pos.rx() += 200;
            }

        }
    });
    popup->addAction(addNewModules);
    addAction(addNewModules);

    auto deleteAction = new QAction(tr("&Delete"), this);
    deleteAction->setShortcut(QKeySequence::Delete);
    deleteAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    deleteAction->setIcon(QIcon(":/CrayonGraphicsEditorResources/delete_icon.svg"));
    connect(deleteAction, &QAction::triggered, this, [this](){

        using Connection = QPair<CrNode*, CrNode*>;
        QList<CrNode*> deleteNodes;
        QSet<Connection> deleteConnecions;

        foreach(CrNode* delNode, scene_->selectedNodes()){
            if(!delNode->isModule())
                continue;

            deleteNodes.append(delNode);

            QStack<CrNode*> stack;
            stack.push(delNode);
            while (!stack.isEmpty()) {
                CrNode* n = stack.pop();
                if(n->inputNode()){
                    deleteConnecions.insert(Connection{n, n->inputNode()});
                }
                foreach (CrNode* in, n->outputNodes()) {
                    deleteConnecions.insert(Connection{in, n});
                }
                stack << n->childNodes().toVector();
            }
            continue;
        }

        deleteConnecions += scene_->selectedConnectors().toSet();

        if(deleteNodes.isEmpty() && deleteConnecions.isEmpty())
            return ;

        QString delNodesText = !deleteNodes.isEmpty() ? tr(" %1 nodes").arg(deleteNodes.size()) : QString();
        QString delConnectorsText = !deleteConnecions.isEmpty() ?
                    tr("%1 %2 connectors").
                    arg(!deleteNodes.isEmpty() ? tr(", and") : QString(" ")).
                    arg(deleteConnecions.size()) : QString();

        QString commandText = tr("Delete%1%2.").arg(delNodesText).arg(delConnectorsText);

        QUndoCommand* parentCommand = new QUndoCommand();
        parentCommand->setText(tr("Delete %1 nodes, and %2 connectios").arg(deleteNodes.size()).arg(deleteConnecions.size()));

        foreach (Connection connection, deleteConnecions) {
            new CrCommands::ChangeInputNode(connection.first, nullptr, parentCommand);
        }
        foreach (CrNode* delNode, deleteNodes) {
            new CrCommands::MoveNode(delNode, nullptr, 0, parentCommand);
        }

        scene_->node()->project()->pushUndoCommand(parentCommand);

    });
    popup->addAction(deleteAction);
    addAction(deleteAction);

    auto copyAction = new QAction(tr("&Paste"), this);
    copyAction->setShortcut(QKeySequence::Copy);
    copyAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    copyAction->setIcon(QIcon(":/CrayonGraphicsEditorResources/copy_icon.svg"));
    connect(copyAction, &QAction::triggered, this, [this](){

        QList<quintptr> copyNodes;

        foreach(CrNode* node, scene_->node()->project()->selectedNodes()){
            if(node->parentNode() != scene_->node())
                continue;
            copyNodes << reinterpret_cast<quintptr>(node);
        }

        if(copyNodes.isEmpty())
            return ;

        QMimeData *mimeData = new QMimeData();

        QByteArray array;
        QDataStream stream(&array, QIODevice::WriteOnly);
        stream << copyNodes;
        mimeData->setData("application/crayonf/nodes", array);

        QApplication::clipboard()->setMimeData(mimeData);
    });
    popup->addAction(copyAction);
    addAction(copyAction);

    auto cutAction = new QAction(tr("&Cut"), this);
    cutAction->setShortcut(QKeySequence::Cut);
    cutAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    cutAction->setIcon(QIcon(":/Resources/icons/cut_icon.png"));
    popup->addAction(cutAction);
    connect(cutAction, &QAction::triggered, this, [copyAction, deleteAction](){
        copyAction->trigger();
        deleteAction->trigger();
    });
    popup->addAction(cutAction);
    addAction(cutAction);

    auto pasteAction = new QAction(tr("&Paste"), this);
    pasteAction->setShortcut(QKeySequence::Paste);
    pasteAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    pasteAction->setIcon(QIcon(":/CrayonGraphicsEditorResources/paste_icon.svg"));
    connect(pasteAction, &QAction::triggered, this, [this](){

        const QMimeData* data = QApplication::clipboard()->mimeData();
        if(data->hasFormat("application/crayonf/nodes")){

            QList<quintptr> ptrNodes;
            QDataStream stream(data->data("application/crayonf/nodes"));
            stream >> ptrNodes;

            QList<CrNode*> copies;
            QList<QPointF> positions;
            foreach(quintptr ptr, ptrNodes){
                CrNode* node = reinterpret_cast<CrNode*>(ptr);
                Q_ASSERT(node);

                if(!node->isModule())
                    continue;

                CrNode* testParent = scene_->node();
                while (testParent) {
                    if(testParent == node)
                        break;
                    testParent = testParent->parentNode();
                }
                if(testParent == node)
                    continue;

                CrNode* copy = node->createCopy();
                if(!copy)
                    continue;

                copies << copy;
                positions  << scene_->nodePos(node);

            }

            if(copies.isEmpty())
                return;

            QRectF boundRect(positions.first().x(), positions.first().y(), 1.0, 1.0);
            foreach (QPointF pos, positions) {
                boundRect.setTop(qMin(pos.y(), boundRect.top()));
                boundRect.setBottom(qMax(pos.y(), boundRect.bottom()));
                boundRect.setLeft(qMin(pos.x(), boundRect.left()));
                boundRect.setRight(qMax(pos.x(), boundRect.right()));
            }
            QPointF offset = mapToScene(mapFromGlobal(QCursor::pos())) - boundRect.center();

            if(scene_->node()->project()){
                //  int index = scene_->node()->childNodes().

                QUndoCommand* parentCommand = new QUndoCommand();
                parentCommand->setText(QString("Paste %1 nodes.").arg(copies.size()));

                foreach(CrNode* node, copies){
                    new CrCommands::MoveNode(node, scene_->node(), -1, parentCommand);
                }
                scene_->node()->project()->pushUndoCommand(parentCommand);
            }

            for (int i = 0; i < copies.size(); ++i) {
                scene_->setNodePos(copies.at(i), positions.at(i) + offset);
            }

        }
    });
    popup->addAction(pasteAction);
    addAction(pasteAction);


    auto renameAction = new QAction(tr("&Rename"), this);
    renameAction->setIcon(QIcon(":/CrayonGraphicsEditorResources/rename_icon.svg"));
    connect(renameAction, &QAction::triggered, this, [this](){
        if(!currentNode_)
            return;
        scene_->openNameEditor(currentNode_);
    });
    popup->addAction(renameAction);
    addAction(renameAction);

    auto openEditorAction = new QAction(tr("&Open editor"), this);
    openEditorAction->setIcon(QIcon(":/CrayonGraphicsEditorResources/edit_icon.svg"));
    connect(openEditorAction, &QAction::triggered, this, [this](){
        if(!currentNode_)
            return;
        scene_->openEditor(currentNode_);
    });
    popup->addAction(openEditorAction);
    addAction(openEditorAction);

    auto settingsAction = new QAction(tr("&Settings"), this);
    settingsAction->setIcon(QIcon(":/CrayonGraphicsEditorResources/settings_icon.svg"));
    connect(settingsAction, &QAction::triggered, this, [this](){
        if(!currentNode_)
            return ;
        CrNodeSettingsDialog::commonDialog()->setNode(currentNode_);
        CrNodeSettingsDialog::commonDialog()->exec();
    });
    popup->addAction(settingsAction);
    addAction(settingsAction);

    auto enableAction = new QAction(tr("&Disable"), this);
    enableAction->setIcon(QIcon(":/ToolBarIcon/pause_icon.png"));
    connect(enableAction, &QAction::triggered, this, [this](){
        if(!currentNode_)
            return ;
        currentNode_->project()->pushUndoCommand(new CrCommands::ChangeEnable(currentNode_));
    });

    popup->addAction(enableAction);
    addAction(enableAction);

    auto setInAction = new QAction(tr("&Set in node"), this);
    setInAction->setIcon(QIcon(":/ToolBarIcon/connect_icon.png"));
    connect(setInAction, &QAction::triggered, this, [this](){

        if(!currentNode_ || !(currentNode_->flags() & CrNode::In))
            return ;

        CrProjectExplorerDialog* explorer = CrProjectExplorerDialog::commonDialog();
        explorer->setFilter(CrProjectExplorerDialog::ValidInputNode, currentNode_);
        explorer->setProject(currentNode_->project());
        if(explorer->exec()){

            if(explorer->selectedNodes().isEmpty()){
                return;
            }
            CrNode* inNode =  explorer->selectedNodes().first();

            if(!currentNode_->isValidInputNode(inNode)){
                return;
            }

            currentNode_->project()->pushUndoCommand(new CrCommands::ChangeInputNode(currentNode_, inNode));
        }
    });
    popup->addAction(setInAction);
    addAction(setInAction);

    connect(this, QOverload<const QPoint &>::of(&QWidget::customContextMenuRequested), this,
            [this, popup, renameAction, openEditorAction, setInAction, deleteAction, copyAction, cutAction] (const QPoint & point){

        if(startPosCursor_ != point)
            return;

        currentNode_ = scene_->nodeAt(mapToScene(point));
        if(currentNode_){
            renameAction->setVisible(currentNode_->flags() & CrNode::EditableName);
            openEditorAction->setVisible(currentNode_->flags() & CrNode::ActiveEditor);
            setInAction->setVisible(currentNode_->flags() & CrNode::In);
        }else {
            renameAction->setVisible(false);
            openEditorAction->setVisible(false);
            setInAction->setVisible(false);
        }
        popup->popup(mapToGlobal(point));
    });
}

CrGraphicsSceneEditor::~CrGraphicsSceneEditor()
{

}

CrGraphicsScene *CrGraphicsSceneEditor::scene() const
{
    return scene_;
}

bool CrGraphicsSceneEditor::eventFilter(QObject *watched, QEvent *event)
{
    //    if(event->type() == QEvent::MouseButtonPress ||
    //            event->type() == QEvent::MouseButtonRelease ||
    //            event->type() == QEvent::MouseButtonDblClick ||
    //            event->type() == QEvent::MouseMove){

    //        QMouseEvent* m = static_cast<QMouseEvent*>(event);
    //        QGraphicsItem* item = scene_->scene_->itemAt(mapToScene(m->pos()), QTransform());
    //        if(!item)
    //            return QGraphicsView::eventFilter(watched, event);

    //        CrGraphicsNodeItem* gi = qgraphicsitem_cast<CrGraphicsNodeItem*>(item);
    //        if(!gi)
    //            return QGraphicsView::eventFilter(watched, event);

    //        QPointF pos = gi->mapFromScene(mapToScene(m->pos()));

    //        if(gi->editorRect_.contains(pos)){
    //            auto opt = gi->styleOption();
    //            opt.rect = gi->editorRect_.toRect();
    //            return gi->node_->eventEditorPreview(m, opt);
    //        }
    //    }
    return QGraphicsView::eventFilter(watched, event);
}

bool CrGraphicsSceneEditor::event(QEvent *event)
{
    return QGraphicsView::event(event);
}

void CrGraphicsSceneEditor::drawBackground(QPainter *painter, const QRectF &rect)
{
    QPen pen;
    pen.setBrush(palette().shadow());
    painter->setPen(pen);

    QBrush brush(palette().window().color().darker(180));
    painter->fillRect(rect, brush);

    painter->setRenderHints(QPainter :: HighQualityAntialiasing | QPainter :: SmoothPixmapTransform );


    for (float  i = qCeil(rect.left() / sceneGridSize_) * sceneGridSize_; rect.right() > i; i += sceneGridSize_) {
        painter->drawLine(QPointF(i,rect.top()),QPointF(i,rect.bottom()));
    }

    for (float  i = qCeil(rect.top() / sceneGridSize_) * sceneGridSize_; rect.bottom() > i; i += sceneGridSize_) {
        painter->drawLine(QPointF(rect.left(),i),QPointF(rect.right(),i));
    }

    QGraphicsView::drawBackground(painter, rect);
}

void CrGraphicsSceneEditor::keyPressEvent(QKeyEvent *event)
{
    //    if(event->key() == Qt::Key_Return){
    //        if(editableItem_){
    //            editableItem_->editor_->deleteLater();
    //            editableItem_ = nullptr;
    //            state_ =  NoState;
    //        }
    //    }
    QGraphicsView::keyPressEvent(event);
}

void CrGraphicsSceneEditor::mouseDoubleClickEvent(QMouseEvent *event)
{
    QGraphicsView::mouseDoubleClickEvent(event);
}

void CrGraphicsSceneEditor::mouseMoveEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::RightButton){
        QPoint offset = oldPosCursor_ - event->pos();
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() + offset.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() + offset.y());
        oldPosCursor_ = event->pos();
        return;
    }
    QGraphicsView::mouseMoveEvent(event);
}

void CrGraphicsSceneEditor::mousePressEvent(QMouseEvent *event)
{
    oldPosCursor_ = event->pos();
    startPosCursor_ = event->pos();

    if(event->buttons() & Qt::RightButton){
        return;
    }

    QGraphicsView::mousePressEvent(event);
}

void CrGraphicsSceneEditor::mouseReleaseEvent(QMouseEvent *event)
{
    QGraphicsView::mouseReleaseEvent(event);

}

void CrGraphicsSceneEditor::wheelEvent(QWheelEvent *event)
{
    QMatrix m = matrix();
    if(event->delta() > 0)
    {
        if(m.m11() < 5){
            m *= QMatrix(zoomSpeed_,0, 0, zoomSpeed_,0,0);
        }
    }else{
        if(m.m11() > 0.1){
            m *= QMatrix(1/zoomSpeed_,0, 0, 1/zoomSpeed_,0,0);
        }
    }
    setMatrix(m);
}

void CrGraphicsSceneEditor::dragEnterEvent(QDragEnterEvent *event)
{
    if(event->mimeData()->hasFormat("application/crayonf/moduleid")
            || event->mimeData()->hasFormat("application/crayonf/modulekey")){
        event->acceptProposedAction();
    }
    QGraphicsView::dragEnterEvent(event);
}

void CrGraphicsSceneEditor::dragLeaveEvent(QDragLeaveEvent *event)
{
    QGraphicsView::dragLeaveEvent(event);
}

void CrGraphicsSceneEditor::dragMoveEvent(QDragMoveEvent *event)
{
    if(event->mimeData()->hasFormat("application/crayonf/moduleid")
            || event->mimeData()->hasFormat("application/crayonf/modulekey")){
        event->acceptProposedAction();
    }
}

void CrGraphicsSceneEditor::dropEvent(QDropEvent *event)
{
    const QMimeData* data = event->mimeData();

    CrNode* parentNode = scene_->node();
    if(CrGraphicsSceneNodeItem* item = qgraphicsitem_cast<CrGraphicsSceneNodeItem*>(itemAt(event->pos()))){
        parentNode = item->node();
    }

    if(data->hasFormat("application/crayonf/modulekey")){

        QString id;
        QVersionNumber version;
        QDataStream stream(data->data("application/crayonf/modulekey"));
        stream >> id >> version;

        CrModule* module = CrModule::module(id, version);
        if(!module){
            scene_->node()->project()->pushWarningMessage(QObject::tr("Create module fault."),
                                                          QObject::tr("Create module fault. Module by id:\"%1\" version: \"%2\" not found.").
                                                          arg(id).arg(version.toString()), nullptr);
            return;
        }

        CrNode* newNode = module->createNode();
        if(!newNode){
            scene_->node()->project()->pushWarningMessage(QObject::tr("Create module fault."),
                                                          QObject::tr("Create module fault. Module by id:\"%1\" version: \"%2\" error create node.").
                                                          arg(id).arg(version.toString()), nullptr);
            return;
        }

        parentNode->project()->pushUndoCommand(new CrCommands::MoveNode(newNode, parentNode, parentNode->childNodes().size()));

        if(parentNode == scene_->node()){
            QPointF pos = mapToScene(event->pos());
            pos.setX(qRound(pos.x() / scene_->moveStep()) * scene_->moveStep());
            pos.setY(qRound(pos.y() / scene_->moveStep()) * scene_->moveStep());
            scene_->setNodePos(newNode, pos);
        }

    } else if(data->hasFormat("application/crayonf/nodes")){

        QList<quintptr> ptrNodes;

        QDataStream stream(data->data("application/crayonf/nodes"));
        stream >> ptrNodes;

        if(ptrNodes.isEmpty())
            return;

        QList<CrNode*> movableNodes;
        foreach(quintptr ptr, ptrNodes){
            CrNode* node = reinterpret_cast<CrNode*>(ptr);
            if(!node)
                continue;
            if(node->isModule()){
                CrNode* testParent = scene_->node();
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
            return;

        if(movableNodes.size() == 1){
            parentNode->project()->pushUndoCommand(new CrCommands::MoveNode(movableNodes.first(), parentNode, parentNode->childNodes().size()));

            if(parentNode == scene_->node()){
                QPointF pos = mapToScene(event->pos());
                pos.setX(qRound(pos.x() / scene_->moveStep()) * scene_->moveStep());
                pos.setY(qRound(pos.y() / scene_->moveStep()) * scene_->moveStep());
                scene_->setNodePos(movableNodes.first(), pos);
            }

        }else {
            QUndoCommand* parentCommand = new QUndoCommand();
            parentCommand->setText(QString("Move %1 nodes.").arg(movableNodes.size()));
            int row = parentNode->childNodes().size();
            foreach(CrNode* node, movableNodes){
                new CrCommands::MoveNode(node, parentNode, row++, parentCommand);
            }
            scene_->node()->project()->pushUndoCommand(parentCommand);

            if(parentNode == scene_->node()){
                for (int i = 0; i < movableNodes.size(); ++i) {
                    CrNode* node = movableNodes.at(i);
                    QPointF pos = mapToScene(event->pos());
                    pos.setX(qRound(pos.x() / scene_->moveStep()) * scene_->moveStep() + 200 * i);
                    pos.setY(qRound(pos.y() / scene_->moveStep()) * scene_->moveStep());
                    scene_->setNodePos(node, pos);
                }
            }
        }
        return;
    }
}

void CrGraphicsSceneEditor::closeEvent(QCloseEvent *event)
{
    QGraphicsView::closeEvent(event);
}

