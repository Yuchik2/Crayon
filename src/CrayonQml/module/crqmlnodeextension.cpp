#include "crqmlnodeextension.h"
#include <CrNode>

#include <QDebug>

CrQmlNodeExtension::CrQmlNodeExtension(QObject *parent) : QObject(parent), node_(static_cast<CrNode*>(parent))
{

}

QQmlListProperty<QObject> CrQmlNodeExtension::data()
{
    return QQmlListProperty<QObject>(this, this,
                                    &CrQmlNodeExtension::appendData,
                                    &CrQmlNodeExtension::dataCount,
                                    &CrQmlNodeExtension::atData,
                                    &CrQmlNodeExtension::clearData);
}

void CrQmlNodeExtension::appendData(QQmlListProperty<QObject> *list, QObject * object)
{ 
    CrQmlNodeExtension* node = static_cast<CrQmlNodeExtension*>(list->data);
    node->data_.append(object);
    if(CrNode* n = qobject_cast<CrNode*>(object)){
        node->node_->appendChildNode(n);
    }
}

int CrQmlNodeExtension::dataCount(QQmlListProperty<QObject>* list)
{
    return static_cast<CrQmlNodeExtension*>(list->data)->data_.count();
}

QObject *CrQmlNodeExtension::atData(QQmlListProperty<QObject> *list, int index)
{
    return static_cast<CrQmlNodeExtension*>(list->data)->data_.at(index);
}

void CrQmlNodeExtension::clearData(QQmlListProperty<QObject>* list)
{
    CrQmlNodeExtension* node = static_cast<CrQmlNodeExtension*>(list->data);
    foreach (QObject* obj, node->data_) {
        if(CrNode* n = qobject_cast<CrNode*>(obj)){
            node->node_->removeChildNode(n);
        }
    }
    node->data_.clear();
}


