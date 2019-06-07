#ifndef CRQMLCOMPONENTNODE_H
#define CRQMLCOMPONENTNODE_H

#include <crayonqml_global.h>

#include <crqmlnode.h>

class QQmlComponent;
class CRAYON_QML_EXPORT CrQmlComponentNode : public CrQmlNode
{
    Q_OBJECT
    Q_PROPERTY(QQmlComponent* component READ component WRITE setComponent NOTIFY itemReset)
public:
    explicit CrQmlComponentNode(CrNode* parent = nullptr);
    virtual ~CrQmlComponentNode() override;

    QQmlComponent *component() const;
    void setComponent(QQmlComponent *component);

    virtual QQuickItem* createItem() override;   
    virtual bool isValidInputNode(CrNode* inputNode) const override;

private:
    Q_DISABLE_COPY(CrQmlComponentNode)
    QQmlComponent* component_;
};

#endif // CRQMLCOMPONENTNODE_H
