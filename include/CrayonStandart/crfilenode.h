#ifndef CRFILENODE_H
#define CRFILENODE_H

#include <crayonstandart_global.h>
#include <CrNode>
#include <QFont>

class CRAYON_STANDART_EXPORT CrFileNode : public CrNode
{
    Q_OBJECT
    Q_PROPERTY(QStringList files READ files WRITE setFiles NOTIFY filesChanged)
    Q_PROPERTY(QString nameFilter READ nameFilter WRITE setNameFilter NOTIFY nameFilterChanged)

public:
    explicit CrFileNode(CrNode *parent = nullptr);
    virtual ~CrFileNode() override;

    QStringList files() const;
    void setFiles(const QStringList &files);

    QString nameFilter() const;
    void setNameFilter(const QString &nameFilter);

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
    void filesChanged();
    void nameFilterChanged();

private:
    Q_DISABLE_COPY(CrFileNode)
    QStringList files_;
    QString nameFilter_;
};

#endif // CRFILENODE_H
