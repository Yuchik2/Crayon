#include "crprojectexplorer.h"

#include <QApplication>
#include <QTreeView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QSortFilterProxyModel>

#include <QLabel>
#include <QToolButton>
#include <QHeaderView>
#include <QContextMenuEvent>
#include <QMenu>
#include <QAction>

#include <QClipboard>
#include <QUndoCommand>
#include <QMimeData>

#include <crproject.h>
#include <crcommands.h>
#include <crprojectmodel.h>
#include <CrNode>
#include <crmodule.h>
#include <crnodesettingsdialog.h>
#include <crprojectexplorerdialog.h>
#include <crprojectmodulesexplorerdialog.h>

#include <QDebug>

CrProjectExplorer::CrProjectExplorer(CrProject *project, QWidget *parent) :
    QWidget(parent),
    project_(project),
    model_(new CrProjectModel(this)),
    proxyModel_(new QSortFilterProxyModel()),
    view_(new QTreeView(this))
{   
    auto mainLayout = new QVBoxLayout();
    mainLayout->setMargin(0);
    setLayout(mainLayout);

    auto toolsLayout = new QHBoxLayout();
    toolsLayout->setContentsMargins(5,0,5,0);
    mainLayout->addLayout(toolsLayout);

    auto findField = new QLineEdit();
    toolsLayout->addWidget(findField);
    connect(findField, &QLineEdit::textChanged, this, [this, findField]{
        proxyModel_->setFilterFixedString(findField->text());
    });

    auto findLabel = new QLabel();
    findLabel->setPixmap(QPixmap(":/CrayonGraphicsEditorResources/search_icon.svg").scaled(14, 14, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    toolsLayout->addWidget(findLabel);

    model_->setProject(project_);
    proxyModel_->setSourceModel(model_);
    proxyModel_->setFilterKeyColumn(CrProjectModel::Name);
    proxyModel_->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxyModel_->setRecursiveFilteringEnabled(true);
    view_->setModel(proxyModel_);
    view_->setAlternatingRowColors(true);
    view_->setSelectionMode(QAbstractItemView::ExtendedSelection);

    view_->setDragEnabled(true);
    view_->setAcceptDrops(true);
    view_->setDropIndicatorShown(true);
    view_->setDefaultDropAction(Qt::MoveAction);

    view_->header()->setSectionResizeMode(QHeaderView::Interactive);
    view_->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    view_->setAutoExpandDelay(600);
    view_->setUniformRowHeights(true);
    view_->setIconSize(QSize(16, 16));
    view_->setSelectionBehavior(QAbstractItemView::SelectRows);
    CrProjectModelDelegate* viewDelegate = new CrProjectModelDelegate(this);
    view_->setItemDelegate(viewDelegate);
    mainLayout->addWidget(view_);

    {
        auto headerPopup = new QMenu(this);
        auto visibleNameAction = new QAction(tr("Name"), this);
        visibleNameAction->setCheckable(true);
        connect(visibleNameAction, &QAction::triggered, this, [this, visibleNameAction]{
            view_->setColumnHidden(CrProjectModel::Name, !visibleNameAction->isChecked());
        });
        headerPopup->addAction(visibleNameAction);

        auto visibleEditorAction = new QAction(tr("Editor"), this);
        visibleEditorAction->setCheckable(true);
        visibleEditorAction->setChecked(true);
        connect(visibleEditorAction, &QAction::triggered, this, [this, visibleEditorAction]{
            view_->setColumnHidden(CrProjectModel::Editor, !visibleEditorAction->isChecked());
        });
        headerPopup->addAction(visibleEditorAction);

        auto visibleCommentsAction = new QAction(tr("Comments"), this);
        visibleCommentsAction->setCheckable(true);
        visibleCommentsAction->setChecked(true);
        connect(visibleCommentsAction, &QAction::triggered, this, [this, visibleCommentsAction]{
            view_->setColumnHidden(CrProjectModel::Comments, !visibleCommentsAction->isChecked());
        });
        headerPopup->addAction(visibleCommentsAction);

        auto visibleInNodeAction = new QAction(tr("In node"), this);
        visibleInNodeAction->setCheckable(true);
        visibleInNodeAction->setChecked(true);
        connect(visibleInNodeAction, &QAction::triggered, this, [this, visibleInNodeAction]{
            view_->setColumnHidden(CrProjectModel::InputNode, !visibleInNodeAction->isChecked());
        });
        headerPopup->addAction(visibleInNodeAction);

        view_->header()->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(view_->header(), QOverload<const QPoint &>::of(&QWidget::customContextMenuRequested),
                this, [this, headerPopup, visibleNameAction, visibleEditorAction,
                visibleCommentsAction, visibleInNodeAction] (const QPoint & point){

            visibleNameAction->setChecked(!view_->isColumnHidden(CrProjectModel::Name));
            visibleEditorAction->setChecked(!view_->isColumnHidden(CrProjectModel::Editor));
            visibleCommentsAction->setChecked(!view_->isColumnHidden(CrProjectModel::Comments));
            visibleInNodeAction->setChecked(!view_->isColumnHidden(CrProjectModel::InputNode));
            headerPopup->popup(view_->header()->mapToGlobal(point));
        });

        //        connect(this, &CrProjectExplorer::headerRestored, this, [this, visibleNameAction, visibleEditorAction,
        //                visibleCommentsAction, visibleInNodeAction] (){
        //            visibleNameAction->setChecked(!view_->isColumnHidden(CrProjectModel::Name));
        //            visibleEditorAction->setChecked(!view_->isColumnHidden(CrProjectModel::Node));
        //            visibleCommentsAction->setChecked(!view_->isColumnHidden(CrProjectModel::Comments));
        //            visibleInNodeAction->setChecked(!view_->isColumnHidden(CrProjectModel::InputNode));
        //        });
    }

    {
        view_->setContextMenuPolicy(Qt::CustomContextMenu);
        auto popup = new QMenu(this);

        auto addNewModules = new QAction(tr("&Add modules"), this);
        addNewModules->setIcon(QIcon(":/CrayonGraphicsEditorResources/add_icon.svg"));
        connect(addNewModules, &QAction::triggered, this, [this](){

            if(!currentNode_)
                return ;

            CrProjectModulesExplorerDialog* dialog = CrProjectModulesExplorerDialog::commonDialog();
            dialog->setProject(project_);
            if(!dialog->exec())
                return ;

            CrModule* module = dialog->currentModule();
            if(module){

                QUndoCommand* parentCommand = new QUndoCommand();
                if(dialog->count() == 1){
                    parentCommand->setText(QString(tr("Add node.")));
                }else {
                    parentCommand->setText(QString(tr("Add %1 nodes.")).arg(dialog->count()));
                }

                for (int i = 0; i < CrProjectModulesExplorerDialog::commonDialog()->count(); ++i) {
                    CrNode* newNode = module->createNode();
                    if(!newNode){
                        project_->pushErrorMessage(QObject::tr("error create modular node."),
                                                   QObject::tr("Failed to create modular node."
                                                               "The module by id: \"%1\" version: \"%2\" contains error.").
                                                   arg(module->id()).arg(module->version().toString()), nullptr);
                        delete parentCommand;
                        return;
                    }
                    new CrCommands::MoveNode(newNode, currentNode_, -1, parentCommand);
                }
                project_->pushUndoCommand(parentCommand);

            }

        });
        popup->addAction(addNewModules);
        addAction(addNewModules);

        auto deleteAction = new QAction(tr("&Delete"), this);
        deleteAction->setShortcut(QKeySequence::Delete);
        deleteAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        deleteAction->setIcon(QIcon(":/CrayonGraphicsEditorResources/delete_icon.svg"));
        connect(deleteAction, &QAction::triggered, this, [this](){

            QList<CrNode*> delNodes = selectedModuleNodes();

            if(delNodes.isEmpty())
                return;

            if(delNodes.size() == 1){
                project_->pushUndoCommand(new CrCommands::MoveNode(delNodes.first(), nullptr));
            }else {
                QUndoCommand* parentCommand = new QUndoCommand();
                parentCommand->setText(QString("Delete %1 nodes").arg(delNodes.size()));
                foreach (CrNode* node, delNodes) {
                    new CrCommands::MoveNode(node, nullptr, 0, parentCommand);
                }
                project_->pushUndoCommand(parentCommand);
            }
        });

        popup->addAction(deleteAction);
        addAction(deleteAction);

        auto copyAction = new QAction(tr("&Copy"), this);
        copyAction->setShortcut(QKeySequence::Copy);
        copyAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        copyAction->setIcon(QIcon(":/CrayonGraphicsEditorResources/copy_icon.svg"));
        connect(copyAction, &QAction::triggered, this, [this](){

            QList<quintptr> nodes;
            foreach(CrNode* node, selectedModuleNodes()){
                nodes << reinterpret_cast<quintptr>(node);
            }

            if(nodes.isEmpty())
                return ;

            QMimeData *mimeData = new QMimeData();
            QByteArray array;
            QDataStream stream(&array, QIODevice::WriteOnly);
            stream << nodes;
            mimeData->setData("application/crayonf/nodes", array);

            QApplication::clipboard()->setMimeData(mimeData);
        });
        popup->addAction(copyAction);
        addAction(copyAction);

        auto cutAction = new QAction(tr("&Cut"), this);
        cutAction->setShortcut(QKeySequence::Cut);
        cutAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        cutAction->setIcon(QIcon(":/CrayonGraphicsEditorResources/cut_icon.svg"));
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
                if(!currentNode_)
                    return ;
                QList<quintptr> ptrNodes;
                QDataStream stream(data->data("application/crayonf/nodes"));
                stream >> ptrNodes;

                QList<CrNode*> refNodes;
                foreach(quintptr ptr, ptrNodes){
                    CrNode* node = reinterpret_cast<CrNode*>(ptr);

                    if(node->isModule())
                        refNodes << node;
                }

                if(refNodes.isEmpty())
                    return;

                QList<CrNode*> copies;
                foreach(CrNode* node, refNodes){
                    copies << node->createCopy();
                }
                int index = currentNode_->childNodes().size();
                if(copies.size() == 1){
                    project_->pushUndoCommand(new CrCommands::MoveNode(copies.first(), currentNode_, index));
                }else {
                    QUndoCommand* parentCommand = new QUndoCommand();
                    parentCommand->setText(QString("Paste %1 nodes.").arg(refNodes.size()));

                    foreach(CrNode* node, copies){
                        new CrCommands::MoveNode(node, currentNode_, index++, parentCommand);
                    }
                    project_->pushUndoCommand(parentCommand);
                }
                return;
            }
        });
        popup->addAction(pasteAction);
        addAction(pasteAction);

        auto renameAction = new QAction(tr("&Rename"), this);
        renameAction->setIcon(QIcon(":/CrayonGraphicsEditorResources/rename_icon.svg"));
        connect(renameAction, &QAction::triggered, this, [this](){
            if(!currentNode_)
                return;
            view_->edit(proxyModel_->mapFromSource(model_->getIndex(currentNode_).siblingAtColumn(CrProjectModel::Name)));
        });
        popup->addAction(renameAction);
        addAction(renameAction);

        auto openEditorAction = new QAction(tr("&Open editor"), this);
        openEditorAction->setIcon(QIcon(":/CrayonGraphicsEditorResources/edit_icon.svg"));
        connect(openEditorAction, &QAction::triggered, this, [this](){
            if(!currentNode_)
                return;
            view_->edit(proxyModel_->mapFromSource(model_->getIndex(currentNode_).siblingAtColumn(CrProjectModel::Editor)));
        });
        popup->addAction(openEditorAction);
        addAction(openEditorAction);

        auto settingsAction = new QAction(tr("&Settings"), this);
        settingsAction->setIcon(QIcon(":/CrayonGraphicsEditorResources/settings_icon.svg"));
        connect(settingsAction, &QAction::triggered, this, [this](){
            CrNodeSettingsDialog::commonDialog()->setNode(currentNode_);
            CrNodeSettingsDialog::commonDialog()->exec();
        });
        popup->addAction(settingsAction);
        addAction(settingsAction);


        auto enableAction = new QAction(tr("&Disable"), this);
       // enableAction->setIcon(QIcon(":/ToolBarIcon/pause_icon.png"));
        connect(enableAction, &QAction::triggered, this, [this](){
            if(!currentNode_)
                return ;
            project_->pushUndoCommand(new CrCommands::ChangeEnable(currentNode_));

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
            explorer->setProject(project_);
            if(explorer->exec()){

                if(explorer->selectedNodes().isEmpty()){
                    return;
                }
                CrNode* inNode =  explorer->selectedNodes().first();

                if(!currentNode_->isValidInputNode(inNode)){
                    return;
                }

                project_->pushUndoCommand(new CrCommands::ChangeInputNode(currentNode_, inNode));

            }
        });
        popup->addAction(setInAction);
        addAction(setInAction);

        connect(view_, QOverload<const QPoint &>::of(&QWidget::customContextMenuRequested), this,
                [this, popup, renameAction, openEditorAction, setInAction, deleteAction, copyAction, cutAction] (const QPoint & point){
            currentNode_ = model_->getNode(proxyModel_->mapToSource(view_->indexAt(point)));
            if(currentNode_){
                renameAction->setVisible(currentNode_->flags() & CrNode::EditableName);
                openEditorAction->setVisible(currentNode_->flags() & CrNode::ActiveEditor);
                setInAction->setVisible(currentNode_->flags() & CrNode::In);
                deleteAction->setVisible(currentNode_->isModule());
                copyAction->setVisible(currentNode_->isModule());
                cutAction->setVisible(currentNode_->isModule());
            }else {
                renameAction->setVisible(false);
                openEditorAction->setVisible(false);
                setInAction->setVisible(false);
                deleteAction->setVisible(false);
                copyAction->setVisible(false);
                cutAction->setVisible(false);
            }
            popup->popup(view_->viewport()->mapToGlobal(point));
        });
    }

    connect(project_, &CrProject::selectedChanged, this, [this]{
        if(blockSelector_)
            return ;
        blockSelector_ = true;

        view_->selectionModel()->clearSelection();

        foreach(CrNode* node, project_->selectedNodes()){
            QModelIndex index  = proxyModel_->mapFromSource(model_->getIndex(node));
            view_->selectionModel()->select(index , QItemSelectionModel::Select | QItemSelectionModel::Rows);
        }
        blockSelector_ = false;
    });

    connect(view_->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this]{
        if(blockSelector_)
            return ;

        blockSelector_ = true;
        project_->clearSelectedNodes();
        foreach (QModelIndex index, view_->selectionModel()->selectedRows(0)) {
            CrNode* node = model_->getNode(proxyModel_->mapToSource(index));
            node->setSelected(true);
        }
        blockSelector_ = false;
    });
}


CrProject *CrProjectExplorer::project() const
{
    return project_;
}

CrNode *CrProjectExplorer::rootNode()
{
    return rootNode_;
}

void CrProjectExplorer::setRootNode(CrNode *node)
{
    if(rootNode_ == node)
        return;
    if(!node)
        node = project_->rootNode();

    rootNode_ = node;
    view_->setRootIndex(proxyModel_->mapFromSource(model_->getIndex(rootNode_)));
    view_->update(view_->rootIndex());
}

bool CrProjectExplorer::restoreState(const QByteArray &state)
{
    QDataStream stream(state);
    QByteArray headerState;
    stream >> headerState;
    view_->header()->restoreState(headerState);
    //   emit headerRestored();
    return true;
}

QByteArray CrProjectExplorer::saveState() const
{
    QByteArray state;
    QDataStream stream(&state, QIODevice::WriteOnly);
    stream << view_->header()->saveState();
    return state;
}

QList<CrNode *> CrProjectExplorer::selectedNodes() const
{
    QList<CrNode *> nodes;
    foreach (QModelIndex index, view_->selectionModel()->selectedRows(0)) {
        CrNode* node =  model_->getNode(proxyModel_->mapToSource(index));
        if(node)
            nodes << node;
    }
    return nodes;
}

QList<CrNode *> CrProjectExplorer::selectedModuleNodes() const
{
    auto isNotModule = [](CrNode* node) {
        return !node->isModule();
    };

    QList<CrNode*> modlueNodes = selectedNodes();
    modlueNodes.erase(std::remove_if(modlueNodes.begin(), modlueNodes.end(), isNotModule),modlueNodes.end());
    return modlueNodes;
}










