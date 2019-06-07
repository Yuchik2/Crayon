#ifndef CRCOMBOBOXNODE_H
#define CRCOMBOBOXNODE_H

#include <crayonstandart_global.h>
#include <CrNode>

struct CrComboBoxItem;
class CRAYON_STANDART_EXPORT CrComboBoxNode : public CrNode
{
    Q_OBJECT
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)

public:
    explicit CrComboBoxNode(CrNode* parent = nullptr);
    virtual ~CrComboBoxNode() override;

    void insertItem(int index, const QIcon &icon, const QString &text, const QVariant &data = QVariant());
    inline void insertItem(int index, const QString &text, const QVariant &data = QVariant()){
        insertItem(index, QIcon(), text, data);
    }
    inline void addItem(const QIcon &icon, const QString &text, const QVariant &data = QVariant()){
        insertItem(count(), icon, text, data);
    }
    inline void addItem(const QString &text, const QVariant &data = QVariant()){
        addItem(QIcon(), text, data);
    }
    inline  void removeItem(int index);

    inline int count() const { return items_.count();}

    QIcon icon(int index) const;
    QString text(int index) const;
    QVariant data(int index) const;

    inline int currentIndex() const { return currentIndex_;}  
    void setCurrentIndex(int index);

    inline QIcon currentIcon() const { return icon(currentIndex_);}
    inline QString currentText() const { return text(currentIndex_);}
    inline QVariant currentData() const { return data(currentIndex_);}

    virtual QWidget* createCompactWidgetEditor() override;

    virtual QSize sizeHintEditorPreview(const QStyleOptionViewItem &option) const override;
    virtual void paintEditorPreview(QPainter *painter, const QStyleOptionViewItem& option) const override;

    virtual QByteArray saveState() const override;
    virtual bool restoreState(const QByteArray &state) override;

signals:
    void currentIndexChanged(int index);

private:
    Q_DISABLE_COPY(CrComboBoxNode)
    int currentIndex_ = -1;
    QList<CrComboBoxItem*> items_;
};

#endif // CRCOMBOBOXNODE_H
