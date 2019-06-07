#ifndef CRFONTNODE_H
#define CRFONTNODE_H

#include <crayonstandart_global.h>
#include <CrNode>
#include <QFont>

class CRAYON_STANDART_EXPORT CrFontNode : public CrNode
{
    Q_OBJECT
    Q_PROPERTY(QFont font READ font WRITE setFont NOTIFY fontChanged)
public:
    explicit CrFontNode(CrNode *parent = nullptr);
    virtual ~CrFontNode() override;

    QFont font() const;
    void setFont(const QFont &font);

    virtual bool isValidInputNode(CrNode* inputNode) const override;

    virtual QDialog* createDialogEditor() override;

    virtual QSize sizeHintEditorPreview(const QStyleOptionViewItem &option) const override;
    virtual void paintEditorPreview(QPainter *painter, const QStyleOptionViewItem& option) const override;

    virtual QByteArray saveState() const override;
    virtual bool restoreState(const QByteArray &state) override;

protected:
    virtual void changeInputNode(CrNode* newNode) override;
    virtual void changeInputNodeValue();

signals:
    void fontChanged();

private:
    Q_DISABLE_COPY(CrFontNode)
    QFont font_;
};

#endif // CRFONTNODE_H
