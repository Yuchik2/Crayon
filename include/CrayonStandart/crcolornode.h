#ifndef CRCOLORNODE_H
#define CRCOLORNODE_H

#include <crayonstandart_global.h>
#include <CrNode>

class CRAYON_STANDART_EXPORT CrColorNode : public CrNode
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
public:
    explicit CrColorNode(CrNode *parent = nullptr);
    virtual ~CrColorNode() override;

    QColor color() const;
    void setColor(const QColor &color);

    virtual bool isValidInputNode(CrNode* inputNode) const override ;

    virtual QDialog* createDialogEditor() override;

    virtual QSize sizeHintEditorPreview(const QStyleOptionViewItem &option) const override;
    virtual void paintEditorPreview(QPainter *painter, const QStyleOptionViewItem& option) const override;

    virtual QByteArray saveState() const override;
    virtual bool restoreState(const QByteArray &state) override;

protected:
    virtual void changeInputNode(CrNode* newNode) override;
    virtual void changeInputNodeValue();

signals:
    void colorChanged();

private:
    Q_DISABLE_COPY(CrColorNode)
    QColor color_;
};

#endif // CRCOLORNODE_H
