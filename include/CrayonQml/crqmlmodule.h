#ifndef CRQMLMODULE_H
#define CRQMLMODULE_H

#include <crayonqml_global.h>

#include <QQmlEngine>

class CRAYON_QML_EXPORT CrQmlModule : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString id READ id WRITE setId)
    Q_PROPERTY(QString version READ version WRITE setVersion)
    Q_PROPERTY(QString path READ path WRITE setPath)
    Q_PROPERTY(QString description READ description WRITE setDescription)
    Q_PROPERTY(QString image READ image WRITE setImage)
public:
    explicit CrQmlModule(QObject *parent = nullptr);
    static CrQmlModule *qmlAttachedProperties(QObject *object)
    {
        return new CrQmlModule(object);
    }

    QString id() const;
    void setId(const QString &id);

    QString version() const;
    void setVersion(const QString &version);

    QString path() const;
    void setPath(const QString &path);

    QString description() const;
    void setDescription(const QString &description);

    QString image() const;
    void setImage(const QString &image);

private:
    QString id_;
    QString version_;
    QString path_;
    QString description_;
    QString image_;
};
QML_DECLARE_TYPEINFO(CrQmlModule, QML_HAS_ATTACHED_PROPERTIES)
#endif // CRQMLMODULE_H
