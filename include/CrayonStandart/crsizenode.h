#ifndef CRSIZENODE_H
#define CRSIZENODE_H

#include <crayonstandart_global.h>
#include <CrNode>

class CRAYON_STANDART_EXPORT CrAbstractSizeNode : public CrNode
{
    Q_OBJECT
    Q_PROPERTY(QSize size READ size WRITE setSize NOTIFY sizeChanged)
    Q_PROPERTY(QSizeF sizeF READ sizeF WRITE setSizeF NOTIFY sizeChanged)
public:
    explicit CrAbstractSizeNode(CrNode *parent = nullptr);
    virtual ~CrAbstractSizeNode() override;

    virtual bool isValidInputNode(CrNode* inputNode) const override;

    virtual QSize size() const = 0;
    virtual void setSize(const QSize &size) = 0;

    virtual QSizeF sizeF() const = 0;
    virtual void setSizeF(const QSizeF &size) = 0;

signals:
    void sizeChanged();
};

class CRAYON_STANDART_EXPORT CrSizeNode : public CrAbstractSizeNode
{
    Q_OBJECT
public:
    explicit CrSizeNode(CrNode *parent = nullptr);
    virtual ~CrSizeNode() override;

    virtual QSize size() const override;
    virtual void setSize(const QSize &size) override;

    virtual QSizeF sizeF() const override;
    virtual void setSizeF(const QSizeF &size) override;

    virtual QWidget* createCompactWidgetEditor() override;

    virtual QSize sizeHintEditorPreview(const QStyleOptionViewItem &option) const override;
    virtual void paintEditorPreview(QPainter *painter, const QStyleOptionViewItem& option) const override;

    virtual QByteArray saveState() const override;
    virtual bool restoreState(const QByteArray &state) override;

protected:
   virtual void changeInputNode(CrNode* newNode) override;
   virtual void changeInputNodeValue();

private:
    Q_DISABLE_COPY(CrSizeNode)
    QSize size_;
};


class CRAYON_STANDART_EXPORT CrSizeFNode : public CrAbstractSizeNode
{
    Q_OBJECT
public:
    explicit CrSizeFNode(CrNode *parent = nullptr);
    virtual ~CrSizeFNode() override;

    virtual QSize size() const override;
    virtual void setSize(const QSize &size) override;

    virtual QSizeF sizeF() const override;
    virtual void setSizeF(const QSizeF &size) override;

    virtual QWidget* createCompactWidgetEditor() override;

    virtual QSize sizeHintEditorPreview(const QStyleOptionViewItem &option) const override;
    virtual void paintEditorPreview(QPainter *painter, const QStyleOptionViewItem& option) const override;

    virtual QByteArray saveState() const override;
    virtual bool restoreState(const QByteArray &state) override;

protected:
    virtual void changeInputNode(CrNode* newNode) override;
    virtual void changeInputNodeValue();

private:
    Q_DISABLE_COPY(CrSizeFNode)
    QSizeF size_;
};

#endif // CRSIZENODE_H
