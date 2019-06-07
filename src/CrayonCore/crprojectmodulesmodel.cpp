#include "crprojectmodulesmodel.h"

#include <QMimeData>

#include "crplugin.h"
#include "crmodule.h"
#include "crproject.h"

#include <QDebug>

class ModelItem {
public:
    QString name;
    ModelItem* parent = nullptr;
    QList<ModelItem*> children;
    CrModule* module = nullptr;
    int index = 0;
};

class CrProjectModulesModelPrivate : public QObject{
public:
    CrProjectModulesModelPrivate(CrProjectModulesModel* model);
    ~CrProjectModulesModelPrivate() override;
    CrProjectModulesModel* model_;
    CrProject* project_ = nullptr;
    ModelItem* rootItem_ = new ModelItem();
    QHash<CrModule*, ModelItem*> items_;
    QIcon folderIcon_;

    void addModule(CrModule* module);
    void removeModule(CrModule* module);

    void registerPlugin(CrPlugin* plugin);
    void unregisterPlugin(CrPlugin* plugin);
};

CrProjectModulesModelPrivate::CrProjectModulesModelPrivate(CrProjectModulesModel *model) :
    QObject(model),
    model_(model)
{

}

CrProjectModulesModelPrivate::~CrProjectModulesModelPrivate() {}


void CrProjectModulesModelPrivate::addModule(CrModule *module)
{
    QStringList path = module->path().split('/');

    if(path.isEmpty()){
        return;
    }

    ModelItem* parentItem = rootItem_;

    while (!path.isEmpty()) {
        QString name = path.takeFirst();

        if(!path.isEmpty()){
            bool foundNextFolder = false;
            foreach (ModelItem* folder, parentItem->children) {
                if(folder->name == name && !folder->module){
                    foundNextFolder = true;
                    parentItem = folder;
                    break;
                }
            }
            if(foundNextFolder) continue;
        }

        int index = parentItem->children.size();

        QModelIndex modelIndex;

        if(parentItem == rootItem_){
            modelIndex = QModelIndex();
        } else {
            modelIndex = model_->createIndex(parentItem->index, 0, parentItem);
        }

        model_->beginInsertRows(modelIndex, index, index);

        ModelItem* item = new ModelItem();
        item->name = name;
        item->parent = parentItem;
        parentItem->children.insert(index, item);
        item->index = index;

        if(path.isEmpty()){
            item->module = module;
            items_.insert(module, item);
        }
        model_->endInsertRows();

        parentItem = item;
    }

    QObject::connect(module, &CrModule::pathChanged, this, [this, module]{
        removeModule(module);
        addModule(module);
    });

}

void CrProjectModulesModelPrivate::removeModule(CrModule *module)
{
    ModelItem* item = items_.value(module, nullptr);

    if(item)
        return;

    ModelItem* parentItem = item->parent;

    do {
        model_->beginRemoveRows(model_->createIndex(parentItem->index, 0, parentItem), item->index, item->index);
        parentItem->children.removeAt(item->index);
        delete item;
        model_->endRemoveRows();

        item = parentItem;
        parentItem = parentItem->parent;

    } while (parentItem->children.isEmpty() && parentItem != rootItem_);

    items_.remove(module);

    disconnect(module, &CrModule::pathChanged, this, nullptr);
}

void CrProjectModulesModelPrivate::registerPlugin(CrPlugin *plugin)
{
    foreach(CrModule* module, plugin->modules()){
        addModule(module);
    }
    QObject::connect(plugin, QOverload<CrModule*>::of(&CrPlugin::moduleAdded), this, [this](CrModule* module){
        addModule(module);
    });
    QObject::connect(plugin, QOverload<CrModule*>::of(&CrPlugin::moduleRemoved), this, [this](CrModule* module){
        removeModule(module);
    });

}

void CrProjectModulesModelPrivate::unregisterPlugin(CrPlugin *plugin)
{
    foreach(CrModule* module, plugin->modules()){
        removeModule(module);
    }
    QObject::disconnect(plugin, QOverload<CrModule*>::of(&CrPlugin::moduleAdded), this, nullptr);
    QObject::disconnect(plugin, QOverload<CrModule*>::of(&CrPlugin::moduleRemoved), this, nullptr);
}

CrProjectModulesModel::CrProjectModulesModel(QObject* parent) :
    QAbstractItemModel(parent),
    d_(new CrProjectModulesModelPrivate(this))
{
    setFolderIcon(QIcon(":/ToolBarIcon/folder_icon.png"));
}

CrProjectModulesModel::~CrProjectModulesModel()
{
    delete d_;
}

CrProject *CrProjectModulesModel::project()
{
    return d_->project_;
}

void CrProjectModulesModel::setProject(CrProject *project)
{
    if(d_->project_ == project)
        return;

    if(d_->project_){
        foreach(CrPlugin* plugin, d_->project_->plugins()){
            d_->unregisterPlugin(plugin);
        }
        disconnect(d_->project_, QOverload<CrPlugin*>::of(&CrProject::pluginAdded),
                   d_, QOverload<CrPlugin*>::of(&CrProjectModulesModelPrivate::registerPlugin));

        disconnect(d_->project_, QOverload<CrPlugin*>::of(&CrProject::pluginRemoved),
                   d_, QOverload<CrPlugin*>::of(&CrProjectModulesModelPrivate::unregisterPlugin));
    }

    d_->project_ = project;

    if(d_->project_){
        foreach(CrPlugin* plugin, d_->project_->plugins()){
            d_->registerPlugin(plugin);
        }
        connect(d_->project_, QOverload<CrPlugin*>::of(&CrProject::pluginAdded),
                d_, QOverload<CrPlugin*>::of(&CrProjectModulesModelPrivate::registerPlugin));

        connect(d_->project_, QOverload<CrPlugin*>::of(&CrProject::pluginRemoved),
                d_, QOverload<CrPlugin*>::of(&CrProjectModulesModelPrivate::unregisterPlugin));

    }

    emit projectChanged();
}


QModelIndex CrProjectModulesModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();


    if (!parent.isValid())
        return createIndex(row, column, d_->rootItem_->children.at(row));


    auto parentItem = static_cast<ModelItem*>(parent.internalPointer());
    return createIndex(row, column, parentItem->children.at(row));
}

QModelIndex CrProjectModulesModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return QModelIndex();

    auto parentItem = static_cast<const ModelItem*>(child.internalPointer())->parent;
    //  Q_ASSERT(parentItem);

    if(!parentItem || parentItem == d_->rootItem_)
        return QModelIndex();

    return createIndex(parentItem->index, 0, parentItem);
}

int CrProjectModulesModel::rowCount(const QModelIndex &parent) const
{
    const ModelItem* parentNode;

    if (!parent.isValid())
        parentNode = static_cast<ModelItem*>(d_->rootItem_);
    else
        parentNode = static_cast<ModelItem*>(parent.internalPointer());
    return parentNode->children.size();
}

int CrProjectModulesModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 2;
}

QVariant CrProjectModulesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    ModelItem* item = static_cast<ModelItem*>(index.internalPointer());

    if(role == Qt::DisplayRole)
        switch (index.column()) {
        case Name:
            return item->name;
        case Id:
            return item->module ? item->module->id() : QVariant();
        case Version:
            return item->module ? item->module->version().toString() : QVariant();
        case Doc:
            return item->module ? item->module->doc() : QVariant();
        case PluginName:
            return item->module ? item->module->plugin()->name() : QVariant();
        }

    if(role == Qt::DecorationRole && index.column() == Name)
        return item->module ? QIcon(item->module->icon()) : d_->folderIcon_;

    return QVariant();
}

QVariant CrProjectModulesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal){
        switch (section) {
        case Name:
            return tr("Name");
        case Id:
            return tr("Id");
        case Version:
            return  tr("Version");
        case Doc:
            return  tr("Documentation");
        case PluginName:
            return  tr("Plugin");
        }
    }else{
        return section;
    }

    return QVariant();
}

Qt::ItemFlags CrProjectModulesModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
    if (index.isValid()){
        ModelItem* node = static_cast<ModelItem*>(index.internalPointer());
        if(node->module){
            return Qt::ItemIsDragEnabled | defaultFlags;
        }
    }
    return defaultFlags;
}

QMimeData* CrProjectModulesModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData();

    foreach (QModelIndex index, indexes) {
        if (index.isValid()) {
            ModelItem* node = static_cast<ModelItem*>(index.internalPointer());
            if(node->module){
                QByteArray array;
                QDataStream stream(&array, QIODevice::WriteOnly);
                stream << node->module->key();
                mimeData->setData("application/crayonf/modulekey", array);
                break;
            }
        }
    }
    return mimeData;
}

CrModule *CrProjectModulesModel::getModule(const QModelIndex &index) const
{
    if(!index.isValid())
        return nullptr;
    return static_cast<ModelItem*>(index.internalPointer())->module;
}

QModelIndex CrProjectModulesModel::getIndex(CrModule *module) const
{
    ModelItem* item = d_->items_.value(module, nullptr);
    if(!item)
        return QModelIndex();

    return createIndex(item->index, 0, item);
}

QIcon CrProjectModulesModel::folderIcon() const
{
    return d_->folderIcon_;
}

void CrProjectModulesModel::setFolderIcon(const QIcon &folderIcon)
{
    beginResetModel();
    d_->folderIcon_ = folderIcon;
    endResetModel();
}


//void CrProjectModulesModel::construct()
//{
//    beginResetModel();

//    foreach (ModelItem* child, d_->rootItem_->children) {
//        delete child;
//    }
//    d_->rootItem_->children.clear();
//    d_->items_.clear();


//    auto modulesList = CrModule::modules();

//    qSort(modulesList.begin(), modulesList.end(), [](CrModule* m1, CrModule* m2) -> bool{
//        return m1->path() < m2->path();
//    });

//    foreach (CrModule* dynNode, modulesList) {
//        QStringList path = dynNode->path().split('/');

//        if(path.isEmpty()){
//            continue;
//        }

//        ModelItem* currentItem = static_cast<ModelItem*>(rootItem_);
//        while (path.size() > 1) {
//            ModelItem* folderItem = nullptr;

//            foreach (ModelItem* tempNode, currentItem->children) {
//                if(tempNode->name == path.first()){
//                    folderItem = tempNode;
//                }
//            }
//            if(!folderItem){
//                folderItem = new ModelItem();
//                folderItem->name = path.first();
//                folderItem->parent = currentItem;
//                folderItem->index = currentItem->children.size();
//                currentItem->children.append(folderItem);
//            }
//            path.removeFirst();
//            currentItem = folderItem;
//        }
//        ModelItem* item = new ModelItem();
//        item->name = path.first();
//        item->module = dynNode;
//        item->parent = currentItem;
//        item->index = currentItem->children.size();
//        currentItem->children.append(item);
//        items_.insert(dynNode, item);
//    }
//    endResetModel();
//}

