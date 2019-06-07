#ifndef CRBYTEARRAYNODE_H
#define CRBYTEARRAYNODE_H

#include <crayonstandart_global.h>
#include <CrNode>

class CRAYON_STANDART_EXPORT CrByteArrayNode : public CrNode
{
    Q_OBJECT
    Q_PROPERTY(QByteArray bytes READ bytes WRITE setBytes NOTIFY bytesChanged)
public:
    explicit CrByteArrayNode(CrNode* parent = nullptr);
    virtual ~CrByteArrayNode() override;

    QByteArray bytes() const;
    void setBytes(const QByteArray &bytes);

    virtual bool isValidInputNode(CrNode* inputNode) const override;
    virtual bool isValidOunputNode(CrNode* outputNode) const override;

    virtual QWidget* createCompactWidgetEditor() override;

    virtual QSize sizeHintEditorPreview(const QStyleOptionViewItem &option) const override;
    virtual void paintEditorPreview(QPainter *painter, const QStyleOptionViewItem& option) const override;

    virtual QByteArray saveState() const override;
    virtual bool restoreState(const QByteArray &state) override;

protected:
    virtual void changeInputNode(CrNode* newNode) override;
    virtual void changeInputNodeValue();

signals:
    void bytesChanged();

private:
    Q_DISABLE_COPY(CrByteArrayNode)
    QByteArray bytes_;
};
#endif // CRBYTEARRAYNODE_H
