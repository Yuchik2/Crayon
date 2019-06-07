#ifndef CRRECTNODE_H
#define CRRECTNODE_H

#include <crayonstandart_global.h>
#include <CrNode>

class CRAYON_STANDART_EXPORT CrAbstractRectNode : public CrNode
{
    Q_OBJECT
    Q_PROPERTY(QRect rect READ rect WRITE setRect NOTIFY rectChanged)
    Q_PROPERTY(QRectF rectF READ rectF WRITE setRectF NOTIFY rectChanged)
public:
    explicit CrAbstractRectNode(CrNode *parent = nullptr);
    virtual ~CrAbstractRectNode() override;

    virtual bool isValidInputNode(CrNode* inputNode) const override;

    virtual QRect rect() const = 0;
    virtual void setRect(const QRect &rect) = 0;

    virtual QRectF rectF() const = 0;
    virtual void setRectF(const QRectF &rect) = 0;

signals:
    void rectChanged();
};

class CRAYON_STANDART_EXPORT CrRectNode : public CrAbstractRectNode
{
    Q_OBJECT
public:
    explicit CrRectNode(CrNode *parent = nullptr);
    virtual ~CrRectNode() override;

    virtual QRect rect() const override;
    virtual void setRect(const QRect &rect) override;

    virtual QRectF rectF() const override;
    virtual void setRectF(const QRectF &rect) override;

    virtual QWidget* createCompactWidgetEditor() override;

    virtual QSize sizeHintEditorPreview(const QStyleOptionViewItem &option) const override;
    virtual void paintEditorPreview(QPainter *painter, const QStyleOptionViewItem& option) const override;

    virtual QByteArray saveState() const override;
    virtual bool restoreState(const QByteArray &state) override;

protected:
   virtual void changeInputNode(CrNode* newNode) override;
   virtual void changeInputNodeValue();

private:
    Q_DISABLE_COPY(CrRectNode)
    QRect rect_;
};


class CRAYON_STANDART_EXPORT CrRectFNode : public CrAbstractRectNode
{
    Q_OBJECT
public:
    explicit CrRectFNode(CrNode *parent = nullptr);
    virtual ~CrRectFNode() override;

    virtual QRect rect() const override;
    virtual void setRect(const QRect &rect) override;

    virtual QRectF rectF() const override;
    virtual void setRectF(const QRectF &rect) override;

    virtual QWidget* createCompactWidgetEditor() override;

    virtual QSize sizeHintEditorPreview(const QStyleOptionViewItem &option) const override;
    virtual void paintEditorPreview(QPainter *painter, const QStyleOptionViewItem& option) const override;

    virtual QByteArray saveState() const override;
    virtual bool restoreState(const QByteArray &state) override;

protected:
    virtual void changeInputNode(CrNode* newNode) override;
    virtual void changeInputNodeValue();

private:
    Q_DISABLE_COPY(CrRectFNode)
    QRectF rect_;
};
#endif // CRRECTNODE_H
