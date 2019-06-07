#ifndef CRPOINTNODE_H
#define CRPOINTNODE_H

#include <crayonstandart_global.h>
#include <CrNode>

class CRAYON_STANDART_EXPORT CrAbstractPointNode : public CrNode
{
    Q_OBJECT
    Q_PROPERTY(QPoint point READ point WRITE setPoint NOTIFY pointChanged)
    Q_PROPERTY(QPointF pointF READ pointF WRITE setPointF NOTIFY pointChanged)
public:
    explicit CrAbstractPointNode(CrNode *parent = nullptr);
    virtual ~CrAbstractPointNode() override;

    virtual bool isValidInputNode(CrNode* inputNode) const override;

    virtual QPoint point() const = 0;
    virtual void setPoint(const QPoint &point) = 0;

    virtual QPointF pointF() const = 0;
    virtual void setPointF(const QPointF &point) = 0;

signals:
    void pointChanged();
};

class CRAYON_STANDART_EXPORT CrPointNode : public CrAbstractPointNode
{
    Q_OBJECT  
public:
    explicit CrPointNode(CrNode *parent = nullptr);
    virtual ~CrPointNode() override;

    virtual QPoint point() const override;
    virtual void setPoint(const QPoint &point) override;

    virtual QPointF pointF() const override;
    virtual void setPointF(const QPointF &point) override;

    virtual QWidget* createCompactWidgetEditor() override;

    virtual QSize sizeHintEditorPreview(const QStyleOptionViewItem &option) const override;
    virtual void paintEditorPreview(QPainter *painter, const QStyleOptionViewItem& option) const override;

    virtual QByteArray saveState() const override;
    virtual bool restoreState(const QByteArray &state) override;

protected:
   virtual void changeInputNode(CrNode* newNode) override;
   virtual void changeInputNodeValue();

private:
    Q_DISABLE_COPY(CrPointNode)
    QPoint point_;
};


class CRAYON_STANDART_EXPORT CrPointFNode : public CrAbstractPointNode
{
    Q_OBJECT
public:
    explicit CrPointFNode(CrNode *parent = nullptr);
    virtual ~CrPointFNode() override;

    virtual QPoint point() const override;
    virtual void setPoint(const QPoint &point) override;

    virtual QPointF pointF() const override;
    virtual void setPointF(const QPointF &point) override;

    virtual QWidget* createCompactWidgetEditor() override;

    virtual QSize sizeHintEditorPreview(const QStyleOptionViewItem &option) const override;
    virtual void paintEditorPreview(QPainter *painter, const QStyleOptionViewItem& option) const override;

    virtual QByteArray saveState() const override;
    virtual bool restoreState(const QByteArray &state) override;

protected:
    virtual void changeInputNode(CrNode* newNode) override;
    virtual void changeInputNodeValue();

private:
    Q_DISABLE_COPY(CrPointFNode)
    QPointF point_;
};

#endif // CRPOINTNODE_H
