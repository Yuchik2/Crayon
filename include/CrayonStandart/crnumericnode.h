#ifndef CRNUMERICNODE_H
#define CRNUMERICNODE_H

#include <crayonstandart_global.h>
#include <CrNode>

class CRAYON_STANDART_EXPORT CrNumericNode : public CrNode
{
    Q_OBJECT
    Q_PROPERTY(int intValue READ intValue WRITE setIntValue NOTIFY valueChanged)
    Q_PROPERTY(qreal realValue READ realValue WRITE setRealValue NOTIFY valueChanged)
    Q_PROPERTY(QString suffix READ suffix WRITE setSuffix NOTIFY suffixChanged)
public:
    explicit CrNumericNode(CrNode* parent = nullptr);
    virtual ~CrNumericNode() override;

    virtual int intValue() const = 0;
    virtual void setIntValue(int value) = 0;

    virtual qreal realValue() const = 0;
    virtual void setRealValue(qreal value) = 0;

    QString suffix() const;
    void setSuffix(const QString &suffix);

    virtual bool isValidInputNode(CrNode* inputNode) const override;

    virtual QByteArray saveState() const override;
    virtual bool restoreState(const QByteArray &state) override;

protected:
    virtual void changeInputNode(CrNode* newNode) override;
    virtual void changeInputNodeValue() = 0;
signals:
    void valueChanged();
    void suffixChanged();

private:
    Q_DISABLE_COPY(CrNumericNode)
    QString suffix_;
};

class CRAYON_STANDART_EXPORT CrIntegerNode : public CrNumericNode
{
    Q_OBJECT
public:
    explicit CrIntegerNode(CrNode* parent = nullptr);
    virtual ~CrIntegerNode() override;

    virtual int intValue() const override;
    virtual void setIntValue(int value) override;

    virtual qreal realValue() const override;
    virtual void setRealValue(qreal value) override;

    virtual QWidget* createCompactWidgetEditor() override;

    virtual QSize sizeHintEditorPreview(const QStyleOptionViewItem &option) const override;
    virtual void paintEditorPreview(QPainter *painter, const QStyleOptionViewItem& option) const override;

    virtual QByteArray saveState() const override;
    virtual bool restoreState(const QByteArray &state) override;

protected:
    virtual void changeInputNode(CrNode* newNode) override;
    virtual void changeInputNodeValue() override;

private:
    Q_DISABLE_COPY(CrIntegerNode)
    int value_ = 0;
};

class CRAYON_STANDART_EXPORT CrIntegerLimitedNode : public CrIntegerNode
{
    Q_OBJECT
    Q_PROPERTY(int min READ min WRITE setMin NOTIFY rangeChanged)
    Q_PROPERTY(int max READ max WRITE setMax NOTIFY rangeChanged)
public:
    explicit CrIntegerLimitedNode(CrNode* parent = nullptr);
    virtual ~CrIntegerLimitedNode() override;

    virtual void setIntValue(int value) override;

    int min() const;
    void setMin(int min);

    int max() const;
    void setMax(int max);

    void setRange(int min, int max);

    virtual QWidget* createCompactWidgetEditor() override;

    virtual QByteArray saveState() const override;
    virtual bool restoreState(const QByteArray &state) override;

signals:
    void rangeChanged();

private:
    Q_DISABLE_COPY(CrIntegerLimitedNode)
    int min_ = 0;
    int max_ = 100;
};

class CRAYON_STANDART_EXPORT CrRealNode : public CrNumericNode
{
    Q_OBJECT
public:
    explicit CrRealNode(CrNode* parent = nullptr);
    virtual ~CrRealNode() override;

    virtual int intValue() const override;
    virtual void setIntValue(int value) override;

    virtual qreal realValue()const override;
    virtual void setRealValue(qreal value) override;

    virtual QWidget* createCompactWidgetEditor() override;

    virtual QSize sizeHintEditorPreview(const QStyleOptionViewItem &option) const override;
    virtual void paintEditorPreview(QPainter *painter, const QStyleOptionViewItem& option) const override;

    virtual QByteArray saveState() const override;
    virtual bool restoreState(const QByteArray &state) override;

protected:
    virtual void changeInputNode(CrNode* newNode) override;
    virtual void changeInputNodeValue() override;
private:
    Q_DISABLE_COPY(CrRealNode)
    qreal value_ = 0.0;
};


class CRAYON_STANDART_EXPORT CrRealLimitedNode : public CrRealNode
{
    Q_OBJECT
    Q_PROPERTY(qreal min READ min WRITE setMin NOTIFY rangeChanged)
    Q_PROPERTY(qreal max READ max WRITE setMax NOTIFY rangeChanged)
public:
    explicit CrRealLimitedNode(CrNode* parent = nullptr);
    virtual ~CrRealLimitedNode() override;

    virtual void setRealValue(qreal value) override;

    qreal min() const;
    void setMin(qreal min);

    qreal max() const;
    void setMax(qreal max);

    void setRange(qreal min, qreal max);

    virtual QWidget* createCompactWidgetEditor() override;

    virtual QByteArray saveState() const override;
    virtual bool restoreState(const QByteArray &state) override;

signals:
    void rangeChanged();

private:
    Q_DISABLE_COPY(CrRealLimitedNode)
    qreal min_ = 0.0;
    qreal max_ = 100.0;
};

#endif // CRNUMERICNODE_H
