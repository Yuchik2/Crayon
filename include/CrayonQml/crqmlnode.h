#ifndef CRQMLNODE_H
#define CRQMLNODE_H

#include <crayonqml_global.h>
#include <CrNode>

class QQuickItem;
class QQmlEngine;
class CRAYON_QML_EXPORT CrQmlNode : public CrNode
{
    Q_OBJECT
public:
    explicit CrQmlNode(CrNode* inputNode = nullptr);
    virtual ~CrQmlNode() override;

    virtual QQuickItem* createItem();
    static QQmlEngine *engine();

    virtual bool isValidInputNode(CrNode* inputNode) const override;
    virtual QWindow* createWindowEditor() override;
    virtual QSize sizeHintEditorPreview(const QStyleOptionViewItem &option) const override;
    virtual void paintEditorPreview(QPainter *painter, const QStyleOptionViewItem& option) const override;

protected:
    virtual void changeInputNode(CrNode* newNode) override;

signals:
    void itemReset();

private:
    Q_DISABLE_COPY(CrQmlNode)
};
#endif // CRQMLNODE_H
