#ifndef CRPLUGIN_H
#define CRPLUGIN_H

#include "crayoncore_global.h"
#include <QObject>
#include <QVersionNumber>


class QPluginLoader;
class CrModule;
class CrPluginKey;
class CrPluginPrivate;
class  CRAYON_CORE_EXPORT CrPlugin : public QObject{
    Q_OBJECT
    Q_PROPERTY(QString id READ id)
    Q_PROPERTY(QVersionNumber version READ version)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString organizationName READ organizationName WRITE setOrganizationName NOTIFY organizationNameChanged)
    Q_PROPERTY(QString doc READ doc WRITE setDoc NOTIFY docChanged)
    Q_PROPERTY(QIcon icon READ icon WRITE setIcon NOTIFY iconChanged)

public: 
    enum Status {
        NotLoaded,
        Loaded,
        Error
    };
    Q_ENUM(Status)

    Status status();

    QString id() const;

    QVersionNumber version() const;

    CrPluginKey key() const;

    QString filePath() const;

    QString name() const;
    void setName(const QString& name);

    QString organizationName() const;
    void setOrganizationName(const QString& orgName);

    QString domain() const;
    void setDomain(const QString& domain);

    QString doc() const;
    void setDoc(const QString& doc);

    QIcon icon() const;
    void setIcon(const QIcon& icon);

    QList<CrModule*> modules() const;

    bool load();
    QString errorString() const;

signals:
    void nameChanged();
    void organizationNameChanged();
    void domainChanged();
    void docChanged();
    void iconChanged();
    void moduleAdded(CrModule* module);
    void moduleRemoved(CrModule* module);

private:
    explicit CrPlugin(const QString& id, const QVersionNumber version, QPluginLoader* loader);
    virtual ~CrPlugin() override;

    void registerModule(CrModule* module);
    CrPluginPrivate* d_;
    friend class CrCore;
    friend class CrModule;

public:
    static QList<CrPlugin*> plugins();
    static CrPlugin* plugin(const CrPluginKey& key);
    static CrPlugin* plugin(const QString& id, const QVersionNumber& version = QVersionNumber());
};

class CrPluginKey {
public:
    CrPluginKey(QString id = QString(), QVersionNumber version = QVersionNumber()) :
        id_(id), version_(version){}


    inline QString id() const { return id_; }

    QVersionNumber version() const { return version_;}

private:
    QString id_;
    QVersionNumber version_;
    friend QDataStream &operator<<(QDataStream &out, const CrPluginKey& key);
    friend QDataStream &operator>>(QDataStream &in, CrPluginKey& key);
};

inline bool operator==(const CrPluginKey lhs, const CrPluginKey rhs)
{
    return lhs.id() == rhs.id() && lhs.version() == rhs.version();
}

inline uint qHash(const CrPluginKey key, uint seed)
{
    return qHash(key.id(), seed) ^ qHash(key.version(), seed);
}



#endif // CRPLUGIN_H
