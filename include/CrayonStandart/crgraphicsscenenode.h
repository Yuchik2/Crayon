#ifndef CRNODEGRAPHICSCONTAINER_H
#define CRNODEGRAPHICSCONTAINER_H

#include <crayonstandart_global.h>

#include <CrNode>

class CrGraphicsSceneNodePrivate;
class CRAYON_STANDART_EXPORT CrGraphicsSceneNode : public CrNode
{
    Q_OBJECT
public:
    explicit CrGraphicsSceneNode(CrNode *node = nullptr);
    virtual ~CrGraphicsSceneNode() override;

    virtual QWidget* createWidgetEditor() override;

    virtual QSize sizeHintEditorPreview(const QStyleOptionViewItem &option) const override;
    virtual void paintEditorPreview(QPainter *painter, const QStyleOptionViewItem& option) const override;

    virtual QByteArray saveState() const override;
    virtual bool restoreState(const QByteArray &state) override;

    friend class CrGraphicsNodeItem;
    friend class CrGraphicsSceneConnector;
    friend class CrGraphicsSceneEditor;

private:  
    CrGraphicsSceneNodePrivate* d_;
};

#endif // CRNODEGRAPHICSCONTAINER_H
