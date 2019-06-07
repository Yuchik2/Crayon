#include <crqmlcomponentnode.h>

#include <QApplication>
#include <QQuickView>
#include <QWidget>
#include <QQmlComponent>
#include <QQuickItem>

#include <QStyleOptionFrame>
#include <QStyleOptionViewItem>

CrQmlComponentNode::CrQmlComponentNode(CrNode *parent) : CrQmlNode(parent)
{

}

CrQmlComponentNode::~CrQmlComponentNode()
{

}

QQmlComponent *CrQmlComponentNode::component() const
{
    if(inputNode())
        return static_cast<CrQmlComponentNode*>(inputNode())->component();

    return component_;
}

void CrQmlComponentNode::setComponent(QQmlComponent *component)
{
    if(component_ == component)
        return;

    component_ = component;
    emit itemReset();
}

QQuickItem *CrQmlComponentNode::createItem()
{
    return qobject_cast<QQuickItem*>(component()->create(component_->creationContext()));
}

bool CrQmlComponentNode::isValidInputNode(CrNode *inputNode) const
{
     return static_cast<bool>(qobject_cast<CrQmlComponentNode*>(inputNode));
}

