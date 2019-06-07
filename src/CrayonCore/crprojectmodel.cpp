#include "crprojectmodel.h"

#include <QApplication>
#include <QUndoCommand>
#include <QMimeData>
#include <QDialog>
#include <QStyleOptionViewItem>

#include <crproject.h>
#include <crcommands.h>
#include <crprojectmodel.h>
#include <CrNode>
#include <CrModule>
#include <crprojectexplorerdialog.h>

#include <QDebug>

class CrProjectModelItem
{
public:
    CrNode* node = nullptr;
    CrProjectModelItem* parent = nullptr;
    QList<CrProjectModelItem*> children;
    int index = 0;
};

class CrProjectModelPrivate{
public:
    CrProjectModelPrivate(CrProjectModel* model);
    virtual ~CrProjectModelPrivate();

    CrProjectModel* model_;
    CrProject* project_;
    QObject* receiver_;
    CrProjectModelItem* rootItem_;

    void registerItem(CrProjectModelItem* item);
    void unregisterItem(CrProjectModelItem* item);

    QHash<CrNode*, CrProjectModelItem*> items_;


};

CrProjectModelPrivate::CrProjectModelPrivate(CrProjectModel *model) :
    model_(model),
    project_(nullptr),
    receiver_(new QObject()),
    rootItem_(new CrProjectModelItem())
{}

CrProjectModelPrivate::~CrProjectModelPrivate()
{
    delete rootItem_;
    delete receiver_;
}

void CrProjectModelPrivate::registerItem(CrProjectModelItem *item)
{
    CrNode* node = item->node;
    items_.insert(node, item);

    QObject::connect(node, &CrNode::nameChanged, receiver_, [this, item](){
        QModelIndex index = model_->createIndex(item->index, CrProjectModel::Name, item);
        emit model_->dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
    });

    QObject::connect(node, &CrNode::updateEditorPreview, receiver_, [this, item](){
        QModelIndex index = model_->createIndex(item->index, CrProjectModel::Editor, item);
        emit model_->dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
    });

    QObject::connect(node, &CrNode::commentsChanged, receiver_, [this, item](){
        QModelIndex index = model_->createIndex(item->index, CrProjectModel::Comments, item);
        emit model_->dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
    });

    QObject::connect(node, &CrNode::iconChanged, receiver_, [this, item](){
        QModelIndex index = model_->createIndex(item->index, CrProjectModel::Name, item);
        emit model_->dataChanged(index, index, {Qt::DecorationRole});
    });

    QObject::connect(node, &CrNode::enableChanged, receiver_, [this, item](){
        QModelIndex index = model_->createIndex(item->index, CrProjectModel::Index, item);
        emit model_->dataChanged(index, index.siblingAtColumn(CrProjectModel::InputNode), {Qt::DisplayRole});
    });

    QObject::connect(node, &CrNode::inputNodeChanged, receiver_, [this, item](){
        QModelIndex index = model_->createIndex(item->index, CrProjectModel::InputNode, item);
        emit model_->dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
    });

    QObject::connect(node, QOverload<int, CrNode*>::of(&CrNode::childNodeInserted),
                     receiver_, [this, item](int i, CrNode* child){

        QModelIndex index = model_->createIndex(item->index, CrProjectModel::Index, item);

        model_->beginInsertRows(index, i, i);

        CrProjectModelItem* childItem = new CrProjectModelItem();
        childItem->node = child;
        childItem->parent = item;
        item->children.insert(i, childItem);
        childItem->index = i;
        for (int n = i + 1; n < item->children.size(); ++n) {
            ++item->children.at(n)->index;
        }
        model_->endInsertRows();

        registerItem(childItem);
    });

    QObject::connect(node, QOverload<int, CrNode*>::of(&CrNode::childNodeRemoved),
                     receiver_, [this, item](int i, CrNode* child){
        Q_UNUSED(child)

        QModelIndex index = model_->createIndex(item->index, CrProjectModel::Index, item);

        model_->beginRemoveRows(index, i, i);

        CrProjectModelItem* childItem = item->children.at(i);
        item->children.removeAt(i);
        for (int n = i; n < item->children.size(); ++n) {
            --item->children.at(n)->index;
        }
        model_->endRemoveRows();

        unregisterItem(childItem);
        delete childItem;
    });

    if(!node->childNodes().isEmpty()){

        QModelIndex index = model_->createIndex(item->index, CrProjectModel::Index, item);

        model_->beginInsertRows(index, 0, node->childNodes().size() - 1);

        for (int i = 0; i < node->childNodes().size(); ++i) {
            CrProjectModelItem* childItem = new CrProjectModelItem();
            childItem->node = node->childNodes().at(i);
            childItem->parent = item;
            item->children.insert(i, childItem);
            childItem->index = i;
        }
        model_->endInsertRows();

        for (int i = 0; i < item->children.size(); ++i) {
            CrProjectModelItem* childItem = item->children.at(i);
            registerItem(childItem);
        }

    }
}

void CrProjectModelPrivate::unregisterItem(CrProjectModelItem *item)
{
    CrNode* node = item->node;
    items_.remove(node);

    QObject::disconnect(node, &CrNode::nameChanged,         receiver_, nullptr);
    QObject::disconnect(node, &CrNode::updateEditorPreview, receiver_, nullptr);
    QObject::disconnect(node, &CrNode::commentsChanged,     receiver_, nullptr);
    QObject::disconnect(node, &CrNode::iconChanged,         receiver_, nullptr);
    QObject::disconnect(node, &CrNode::enableChanged,       receiver_, nullptr);
    QObject::disconnect(node, &CrNode::inputNodeChanged,    receiver_, nullptr);
    QObject::disconnect(node, &CrNode::childNodeInserted,   receiver_, nullptr);
    QObject::disconnect(node, &CrNode::childNodeRemoved,    receiver_, nullptr);

    foreach (CrProjectModelItem* child, item->children) {
        unregisterItem(child);
        delete child;
    }
}

CrProjectModel::CrProjectModel(QObject *object) :
    QAbstractItemModel(object),
    d_(new CrProjectModelPrivate(this))
{

}

CrProjectModel::~CrProjectModel()
{
    setProject(nullptr);
    delete d_;
}

QModelIndex  CrProjectModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    if (!parent.isValid()){
        return  createIndex(row, column, d_->rootItem_->children.at(row));
    }

    auto parentItem = static_cast<const CrProjectModelItem*>(parent.internalPointer());
    return createIndex(row, column, parentItem->children.at(row));
}

QModelIndex  CrProjectModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return QModelIndex();

    auto parentItem = static_cast<const CrProjectModelItem*>(child.internalPointer())->parent;

    if(!parentItem || parentItem == d_->rootItem_)
        return QModelIndex();

    return createIndex(parentItem->index, 0, parentItem);
}

int  CrProjectModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return d_->rootItem_->children.size();

    return static_cast<CrProjectModelItem*>(parent.internalPointer())->children.size();
}

int  CrProjectModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 7;
}

QVariant  CrProjectModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    CrNode* node = static_cast<CrProjectModelItem*>(index.internalPointer())->node;

    if(role == Qt::DisplayRole || role == Qt::EditRole)
        switch (index.column()) {
        case Index:
            return index.row();
        case Name:
            return node->name();
        case Comments:
            return node->comments();
        case Editor:
            return QVariant::fromValue(node);
        case InputNode:
            return QVariant::fromValue(node->inputNode());
        case OutputNodes:
            return QVariant::fromValue(node->outputNodes());
        case Module:
            return QVariant::fromValue(node->module());
        default:
            return QVariant();
        }

    if(role == Qt::DecorationRole && index.column() == Name){
        return node->icon();
    }
    return QVariant();
}

bool CrProjectModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    CrNode* node = static_cast<CrProjectModelItem*>(index.internalPointer())->node;

    Q_ASSERT(node);

    switch (index.column()) {
    case Name:{
        if(node->name() == value.toString())
            return false;
        d_->project_->pushUndoCommand(new CrCommands::ChangeName(node, value.toString()));
        return true;
    }
    case Comments:{
        if(node->comments() == value.toString())
            return false;
        d_->project_->pushUndoCommand(new CrCommands::ChangeComments(node, value.toString()));
        return true;
    }
    case InputNode:{
        CrNode* inputNode = value.value<CrNode*>();
        if(!inputNode)
            return false;

        if(inputNode == node->inputNode()           ||
                !(inputNode->flags() & CrNode::Out) ||
                !(node->flags() & CrNode::In)    ||
                !inputNode->isValidOunputNode(node) ||
                !node->isValidInputNode(inputNode))
            return false;

        d_->project_->pushUndoCommand(new CrCommands::ChangeInputNode(node, inputNode));
        return true;
    }
    case OutputNodes:{
        //        CrNode* inputNode = value.value<CrNode*>();
        //        if(!inputNode)
        //            return false;

        //        if(inputNode == node->inputNode()           ||
        //                !(inputNode->flags() & CrNode::Out) ||
        //                !(node->flags() & CrNode::In)    ||
        //                !inputNode->isValidOunputNode(node) ||
        //                !node->isValidInputNode(inputNode))
        //            return false;

        //        d_->project_->pushUndoCommand(new CrCommands::ChangeInputNode(node, inputNode));
        //        return true;
        return false;
    }

    default:
        return false;
    }
}

QVariant  CrProjectModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal){
        switch (section) {
        case Index:
            return tr("№");
        case Name:
            return tr("Name");
        case Comments:
            return tr("Comments");
        case Editor:
            return tr("Editor");
        case InputNode:
            return tr("Input node");
        case OutputNodes:
            return tr("Output nodes");
        case Module:
            return tr("Module");
        default:
            return QVariant();
        }
    }else{
        return section;
    }
}

Qt::ItemFlags  CrProjectModel::flags(const QModelIndex &index) const
{
    if(!index.isValid()){
        return Qt::ItemIsDropEnabled | Qt::ItemIsEnabled;
    }

    CrNode* node = static_cast<CrProjectModelItem*>(index.internalPointer())->node;

    Q_ASSERT(node);

    Qt::ItemFlags flag =  node->isEnable() ? Qt::ItemIsEnabled : Qt::NoItemFlags;
    flag |= Qt::ItemIsDropEnabled | Qt::ItemIsSelectable;

    if(node->isModule())
        flag |= Qt::ItemIsDragEnabled;

    switch (index.column()) {
    case Name:
        if(node->flags() & CrNode::EditableName)
            flag |= Qt::ItemIsEditable;
        break;
    case Comments:
        flag |= Qt::ItemIsEditable;
        break;
    case Editor:
        if(node->flags() & CrNode::ActiveEditor)
            flag |= Qt::ItemIsEditable;
        break;
    case InputNode:
        if(node->flags() & CrNode::In)
            flag |= Qt::ItemIsEditable;
        break;
    case OutputNodes:
        if(node->flags() & CrNode::Out)
            flag |= Qt::ItemIsEditable;
        break;
    }
    return flag;
}

QMimeData *CrProjectModel::mimeData(const QModelIndexList &indexes) const
{
    QList<quintptr> nodes;
    foreach(QModelIndex index, indexes){
        if(index.column() != Index)
            continue;
        CrNode* node = static_cast<CrProjectModelItem*>(index.internalPointer())->node;
        Q_ASSERT(node);
        nodes << reinterpret_cast<quintptr>(node);
    }

    QMimeData *mimeData = new QMimeData();

    if(!nodes.isEmpty()){
        QByteArray array;
        QDataStream stream(&array, QIODevice::WriteOnly);
        stream << nodes;
        mimeData->setData("application/crayonf/nodes", array);
    }

    return mimeData;
}

bool CrProjectModel::canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(action);
    Q_UNUSED(row);
    Q_UNUSED(column);
    Q_UNUSED(parent);

    if(data->hasFormat("application/crayonf/modulekey"))
        return true;
    if(data->hasFormat("application/crayonf/nodes"))
        return true;

    return false;
}

QStringList CrProjectModel::mimeTypes() const
{
    return QStringList() << "application/crayonf/nodes" << "application/crayonf/modulekey";
}

bool CrProjectModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(action)
    Q_UNUSED(column)

    CrNode* parentNode;
    if(parent.isValid()){
        parentNode = static_cast<CrProjectModelItem*>(parent.internalPointer())->node;
    }else {
        parentNode = d_->project_->rootNode();
    }
    Q_ASSERT(parentNode);

    if(data->hasFormat("application/crayonf/modulekey")){

        QString id;
        QVersionNumber version;
        QDataStream stream(data->data("application/crayonf/modulekey"));
        stream >> id >> version;

        CrModule* module = CrModule::module(id, version);
        if(!module){
            d_->project_->pushWarningMessage(QObject::tr("Create module fault."),
                                             QObject::tr("Create module fault. Module by id:\"%1\" version: \"%2\" not found.").
                                             arg(id).arg(version.toString()), nullptr);
            return false;
        }

        CrNode* newNode = module->createNode();
        if(!newNode){
            d_->project_->pushWarningMessage(QObject::tr("Create module fault."),
                                             QObject::tr("Create module fault. Module by id:\"%1\" version: \"%2\" error create node.").
                                             arg(id).arg(version.toString()), nullptr);
            return false;
        }

        if(row < 0)
            row = parentNode->childNodes().size();

        d_->project_->pushUndoCommand(new CrCommands::MoveNode(newNode, parentNode, row));
        return true;
    }

    if(data->hasFormat("application/crayonf/nodes")){

        QList<quintptr> ptrNodes;
        QDataStream stream(data->data("application/crayonf/nodes"));
        stream >> ptrNodes;

        if(ptrNodes.isEmpty())
            return false;

        QList<CrNode*> movableNodes;
        foreach(quintptr ptr, ptrNodes){
            CrNode* node = reinterpret_cast<CrNode*>(ptr);
            Q_ASSERT(node);

            if(!node->isModule())
                continue;

            CrNode* testParent = parentNode;
            while (testParent) {
                if(testParent == node)
                    break;
                testParent = testParent->parentNode();
            }
            if(testParent != node)
                movableNodes << node;
        }

        if(movableNodes.isEmpty())
            return false;

        if(row < 0)
            row = parentNode->childNodes().size();

        if(movableNodes.size() == 1){
            d_->project_->pushUndoCommand(new CrCommands::MoveNode(movableNodes.first(), parentNode, row));
        }else {
            QUndoCommand* parentCommand = new QUndoCommand();
            parentCommand->setText(QString("Move %1 nodes.").arg(movableNodes.size()));
            foreach(CrNode* node, movableNodes){
                new CrCommands::MoveNode(node, parentNode, row++, parentCommand);
            }
            d_->project_->pushUndoCommand(parentCommand);
        }
        return true;
    }
    return false;
}

CrProject *CrProjectModel::project() const
{
    return d_->project_;
}

void CrProjectModel::setProject(CrProject *project)
{
    if(d_->project_ == project)
        return;

    if(d_->project_){
        disconnect(d_->project_->rootNode(), &CrNode::childNodeInserted, d_->receiver_, nullptr);
        disconnect(d_->project_->rootNode(), &CrNode::childNodeRemoved, d_->receiver_, nullptr);

        if(!d_->rootItem_->children.isEmpty()){
            beginRemoveRows(QModelIndex(), 0, d_->rootItem_->children.size() - 1);

            foreach (CrProjectModelItem* childItem, d_->rootItem_->children) {
                d_->unregisterItem(childItem);
                delete childItem;
            }
            d_->rootItem_->children.clear();

            endRemoveRows();
        }
    }

    d_->project_ = project;

    if(d_->project_){

        CrNode* rootNode = d_->project_->rootNode();

        d_->rootItem_->node = rootNode;

        QObject::connect(rootNode, QOverload<int, CrNode*>::of(&CrNode::childNodeInserted),
                         d_->receiver_, [this](int i, CrNode* child){

            beginInsertRows(QModelIndex(), i, i);

            CrProjectModelItem* childItem = new CrProjectModelItem();
            childItem->node = child;
            childItem->parent = d_->rootItem_;
            d_->rootItem_->children.insert(i, childItem);
            childItem->index = i;
            for (int n = i + 1; n < d_->rootItem_->children.size(); ++n) {
                ++d_->rootItem_->children.at(n)->index;
            }
            endInsertRows();

            d_->registerItem(childItem);
        });

        QObject::connect(rootNode, QOverload<int, CrNode*>::of(&CrNode::childNodeRemoved),
                         d_->receiver_, [this](int i, CrNode* child){
            Q_UNUSED(child)

            beginRemoveRows(QModelIndex(), i, i);

            CrProjectModelItem* childItem = d_->rootItem_->children.at(i);
            d_->rootItem_->children.removeAt(i);
            for (int n = i; n < d_->rootItem_->children.size(); ++n) {
                --d_->rootItem_->children.at(n)->index;
            }
            endRemoveRows();

            d_->unregisterItem(childItem);
            delete childItem;
        });

        if(!rootNode->childNodes().isEmpty()){
            beginInsertRows(QModelIndex(), 0, rootNode->childNodes().size() - 1);

            for (int i = 0; i < rootNode->childNodes().size(); ++i) {
                CrProjectModelItem* childItem = new CrProjectModelItem();
                childItem->node = rootNode->childNodes().at(i);
                childItem->parent = d_->rootItem_;
                d_->rootItem_->children.insert(i, childItem);
                childItem->index = i;
            }
            endInsertRows();

            for (int i = 0; i < d_->rootItem_->children.size(); ++i) {
                CrProjectModelItem* childItem = d_->rootItem_->children.at(i);
                d_->registerItem(childItem);
            }
        }
    }

    emit projectChanged();
}

CrNode *CrProjectModel::getNode(const QModelIndex &index) const
{
    if(!index.isValid())
        return d_->project_->rootNode();
    return static_cast<CrProjectModelItem*>(index.internalPointer())->node;
}

QModelIndex CrProjectModel::getIndex(CrNode *node) const
{
    CrProjectModelItem* item = d_->items_.value(node, nullptr);
    if(!item || item == d_->rootItem_)
        return QModelIndex();

    return createIndex(item->index, 0, item);
}


CrProjectModelDelegate::CrProjectModelDelegate(QObject *parent) :
    QStyledItemDelegate(parent) {}


CrProjectModelDelegate::~CrProjectModelDelegate() {}

QWidget *CrProjectModelDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    switch (index.column()) {

    case CrProjectModel::Editor:{
        CrNode* node = index.model()->data(index).value<CrNode*>();
        QWidget* editor = node->createCompactWidgetEditor();

        if(editor){
            editor->setParent(parent);
            return editor;
        }

        QDialog* dialog = node->createDialogEditor();

        if(dialog){
            dialog->exec();
            dialog->deleteLater();
            return nullptr;
        }
        node->requestOpenEditor();

        node->setFocus(true);
        return nullptr;
    }
    case CrProjectModel::InputNode:{
        CrNode* node = index.model()->data(index.siblingAtColumn(CrProjectModel::Editor)).value<CrNode*>();
        if(!(node->flags() & CrNode::In))
            return nullptr;
        CrProjectExplorerDialog* dialog = CrProjectExplorerDialog::commonDialog();
        dialog->setParent(parent);

        return dialog;

        //        if(CrProjectExplorerDialog::commonDialog()->exec()){

        //            if(!CrProjectExplorerDialog::commonDialog()->selectedNodes().isEmpty()){

        //                CrNode* inNode =  CrProjectExplorerDialog::commonDialog()->selectedNodes().first();

        //                if( inNode == node                   ||
        //                        inNode == node->inputNode()      ||
        //                        !(inNode->flags() & CrNode::Out) ||
        //                        !(node->flags() & CrNode::In)    ||
        //                        !inNode->isValidOunputNode(node) ||
        //                        !node->isValidInputNode(inNode)
        //                        )
        //                    return nullptr;

        //                node->project()->pushUndoCommand(new CrCommands::ChangeInputNode(node, inNode));
        //            }
        //        }
        //        return nullptr;
    }
    case CrProjectModel::OutputNodes:{
        CrNode* node = index.model()->data(index.siblingAtColumn(CrProjectModel::Editor)).value<CrNode*>();
        if(!(node->flags() & CrNode::Out))
            return nullptr;
        CrProjectExplorerDialog* dialog = CrProjectExplorerDialog::commonDialog();

        dialog->setParent(parent);
        return dialog;
    }
    default:{
        return QStyledItemDelegate::createEditor(parent, option, index);
    }
    }
}

bool CrProjectModelDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if(index.column() == CrProjectModel::Editor){
        CrNode* node = model->data(index).value<CrNode*>();
        return node->eventEditorPreview(event, option);
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

void CrProjectModelDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    switch (index.column()) {

    case CrProjectModel::Editor:{
        CrNode* node = index.model()->data(index).value<CrNode*>();
        QStyleOptionViewItem opt(option);
        initStyleOption(&opt, index);
        node->paintEditorPreview(painter, opt);
        return;
    }
    case CrProjectModel::InputNode:{
        const QWidget *widget = option.widget;
        QStyle *style = widget ? widget->style() : QApplication::style();
        QStyleOptionViewItem opt(option);
        initStyleOption(&opt, index);

        CrNode* inputNode = index.model()->data(index).value<CrNode*>();
        if(inputNode){
            opt.text = inputNode->path();
            opt.icon = inputNode->icon();
        }
        style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);
        return;
    }
    case CrProjectModel::OutputNodes:{
        QList<CrNode*> outputNodes = index.model()->data(index).value<QList<CrNode*>>();

        const QWidget *widget = option.widget;
        QStyle *style = widget ? widget->style() : QApplication::style();
        QStyleOptionViewItem opt(option);
        initStyleOption(&opt, index);

        opt.text = outputNodes.isEmpty() ? tr("<null>") : tr("%1 node(s)").arg(outputNodes.size());

        style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);
        return;
    }
    default:{
        return QStyledItemDelegate::paint(painter, option, index);
    }
    }
}

void CrProjectModelDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    switch (index.column()) {

    case CrProjectModel::InputNode:{
        CrProjectExplorerDialog* dialog = static_cast<CrProjectExplorerDialog*>(editor);

        CrNode* node = index.model()->data(index.siblingAtColumn(CrProjectModel::Editor)).value<CrNode*>();

        dialog->setProject(node->project());
        dialog->setFilter(CrProjectExplorerDialog::ValidInputNode, node);
        return;
    }
    case CrProjectModel::OutputNodes:{
        CrProjectExplorerDialog* dialog = static_cast<CrProjectExplorerDialog*>(editor);

        CrNode* node = index.model()->data(index.siblingAtColumn(CrProjectModel::Editor)).value<CrNode*>();

        dialog->setProject(node->project());
        dialog->setFilter(CrProjectExplorerDialog::ValidOutputNode, node);
        return;
    }
    default:{
        return QStyledItemDelegate::setEditorData(editor, index);
    }
    }
}

void CrProjectModelDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    switch (index.column()) {

    case CrProjectModel::InputNode:{
        CrProjectExplorerDialog* dialog = static_cast<CrProjectExplorerDialog*>(editor);
        QList<CrNode*> nodes = dialog->selectedNodes();
        if(!nodes.isEmpty())
            model->setData(index, QVariant::fromValue(nodes.first()));
        return;
    }
    case CrProjectModel::OutputNodes:{
        CrProjectExplorerDialog* dialog = static_cast<CrProjectExplorerDialog*>(editor);
        QList<CrNode*> nodes = dialog->selectedNodes();
        if(!nodes.isEmpty())
            model->setData(index, QVariant::fromValue(nodes));
        return;
    }
    default:{
        return QStyledItemDelegate::setModelData(editor, model, index);
    }
    }
}

QSize CrProjectModelDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{

    switch (index.column()) {

    case CrProjectModel::Editor:{
        CrNode* node = index.model()->data(index).value<CrNode*>();
        return node->sizeHintEditorPreview(option);
    }
    case CrProjectModel::InputNode:{
        const QWidget *widget = option.widget;
        QStyle *style = widget ? widget->style() : QApplication::style();
        QStyleOptionViewItem opt(option);
        initStyleOption(&opt, index);

        CrNode* inputNode = index.model()->data(index).value<CrNode*>();
        if(inputNode){
            opt.text = inputNode->path();
            opt.icon = inputNode->icon();
        }

        return style->sizeFromContents(QStyle::CT_ItemViewItem, &opt, QSize(), widget);
    }
    case CrProjectModel::OutputNodes:{
        QList<CrNode*> outputNodes = index.model()->data(index).value<QList<CrNode*>>();

        const QWidget *widget = option.widget;
        QStyle *style = widget ? widget->style() : QApplication::style();
        QStyleOptionViewItem opt(option);
        initStyleOption(&opt, index);

        opt.text = outputNodes.isEmpty() ? tr("<null>") : tr("%1 node(s)").arg(outputNodes.size());

        return style->sizeFromContents(QStyle::CT_ItemViewItem, &opt, QSize(), widget);
    }
    default:{
        return QStyledItemDelegate::sizeHint(option, index);
    }
    }
}


