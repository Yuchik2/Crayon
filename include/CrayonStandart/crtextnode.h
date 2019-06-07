#ifndef CRTEXTNODE_H
#define CRTEXTNODE_H

#include <crayonstandart_global.h>
#include <CrNode>

class CRAYON_STANDART_EXPORT CrTextNode : public CrNode
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
public:
    explicit CrTextNode(CrNode *parent = nullptr);
    virtual ~CrTextNode() override;

    QString text() const;
    void setText(const QString &text);

    virtual bool isValidInputNode(CrNode* inputNode) const override;

    virtual QWidget* createCompactWidgetEditor() override;

    virtual QSize sizeHintEditorPreview(const QStyleOptionViewItem &option) const override;
    virtual void paintEditorPreview(QPainter *painter, const QStyleOptionViewItem& option) const override;

    virtual QByteArray saveState() const override;
    virtual bool restoreState(const QByteArray &state) override;

protected:
    virtual void changeInputNode(CrNode* newNode) override;
    virtual void changeInputNodeValue();

signals:
    void textChanged();

private:
    Q_DISABLE_COPY(CrTextNode)
    QString text_;
};

#endif // CRTEXTNODE_H
