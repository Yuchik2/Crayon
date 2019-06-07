#ifndef CRPROJECTMODULESMODEL_H
#define CRPROJECTMODULESMODEL_H

#include "crayoncore_global.h"

#include <QAbstractItemModel>
#include <QIcon>

class CrModule;
class CrProject;
class CrProjectModulesModelPrivate;
class CRAYON_CORE_EXPORT CrProjectModulesModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_PROPERTY(CrProject* project READ project WRITE setProject NOTIFY projectChanged)
public:
    enum Columns{
        Name,
        Id,
        Version,
        Doc,
        PluginName
    };

    explicit CrProjectModulesModel(QObject* parent = nullptr);
    virtual ~CrProjectModulesModel() override;

    CrProject* project();
    void setProject(CrProject* project);

    CrModule* getModule(const QModelIndex& index) const;
    QModelIndex getIndex(CrModule* module) const;

    QIcon folderIcon() const;
    void setFolderIcon(const QIcon &folderIcon);

public:
    //Reimplemented
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    virtual QModelIndex parent(const QModelIndex &child) const override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
    virtual QMimeData *mimeData(const QModelIndexList &indexes) const override;

signals:
    void projectChanged();

private:
    CrProjectModulesModelPrivate* d_;
    friend class CrProjectModulesModelPrivate;

};
#endif // CRPROJECTMODULESMODEL_H
