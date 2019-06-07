#ifndef CRBOOLEANNODE_H
#define CRBOOLEANNODE_H

#include <crayonstandart_global.h>
#include <CrNode>

class CRAYON_STANDART_EXPORT CrBooleanNode : public CrNode
{
    Q_OBJECT
    Q_PROPERTY(bool value READ value WRITE setValue NOTIFY valueChanged)
public:
    explicit CrBooleanNode(CrNode *parent = nullptr);
    virtual ~CrBooleanNode() override;

    bool value() const;
    void setValue(bool value);

    virtual bool isValidInputNode(CrNode* inputNode) const override;
    virtual bool isValidOunputNode(CrNode* outputNode) const override;

    virtual bool eventEditorPreview(QEvent *event, const QStyleOptionViewItem &option) override;
    virtual QSize sizeHintEditorPreview(const QStyleOptionViewItem &option) const override;
    virtual void paintEditorPreview(QPainter *painter, const QStyleOptionViewItem& option) const override;

    virtual QByteArray saveState() const override;
    virtual bool restoreState(const QByteArray &state) override;

protected:
    virtual void changeInputNode(CrNode* newNode) override;
    virtual void changeInputNodeValue();

signals:
    void valueChanged();

private:
    Q_DISABLE_COPY(CrBooleanNode)
    bool value_ = false;
};

#endif // CRBOOLEANNODE_H
