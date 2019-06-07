#ifndef CRPROJECTMODEL_H
#define CRPROJECTMODEL_H

#include "crayoncore_global.h"

#include <QAbstractItemModel>
#include <QBrush>

class CrNode;
class CrProject;
class CrProjectModelPrivate;
class CRAYON_CORE_EXPORT CrProjectModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_PROPERTY(CrProject* project READ project WRITE setProject NOTIFY projectChanged)
public:
    enum Columns{
        Index,
        Name,
        Comments,
        Editor,
        InputNode,
        OutputNodes,
        Module
    };

    explicit CrProjectModel(QObject* object = nullptr);
    virtual ~CrProjectModel() override;

    CrProject *project() const;
    void setProject(CrProject *project);

    CrNode* getNode(const QModelIndex& index) const;
    QModelIndex getIndex(CrNode* node) const;

public:
    //Reimplemented
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    virtual QModelIndex parent(const QModelIndex &child) const override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
    virtual QMimeData *mimeData(const QModelIndexList &indexes) const override;
    virtual bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const override;
    virtual QStringList	mimeTypes() const override;
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;

signals:
    void projectChanged();

private:
    Q_DISABLE_COPY(CrProjectModel)
    CrProjectModelPrivate* d_;
    friend class CrProjectModelPrivate;
};

#include <QStyledItemDelegate>

class CRAYON_CORE_EXPORT CrProjectModelDelegate: public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit CrProjectModelDelegate(QObject *parent = nullptr);
    virtual ~CrProjectModelDelegate() override;

public:
    //Reimplemented
    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;
    virtual bool editorEvent(QEvent *event, QAbstractItemModel *model,
                             const QStyleOptionViewItem &option, const QModelIndex &index) override;
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    virtual void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    Q_DISABLE_COPY(CrProjectModelDelegate)
};


#endif // CRPROJECTMODEL_H
