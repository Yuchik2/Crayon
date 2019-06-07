#ifndef CRMODULE_H
#define CRMODULE_H

#include "crayoncore_global.h"

#include <QObject>
#include <QIcon>
#include <QVersionNumber>
#include <functional>

class CrNode;
class CrPlugin;
class CrModuleKey;
class CrModulePrivate;
class CRAYON_CORE_EXPORT CrModule : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString id READ id)
    Q_PROPERTY(QVersionNumber version READ version)
    Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged)
    Q_PROPERTY(QString doc READ doc WRITE setDoc NOTIFY docChanged)
    Q_PROPERTY(QIcon icon READ icon WRITE setIcon NOTIFY iconChanged)

public:
    using FactoryFunction = std::function<CrNode*(const CrModule*)>;
    explicit CrModule(CrPlugin *plugin,
                      const QString& id,
                      const QVersionNumber& version,
                      const FactoryFunction& factory,
                      const QString &path = QString(),
                      const QString &doc = QString(),
                      const QIcon& icon = QIcon());

    virtual ~CrModule() override;

    const CrPlugin *plugin() const;

    QString id() const;

    QVersionNumber version() const;

    CrModuleKey key() const;

    QString path() const;
    void setPath(const QString& path);

    QString doc() const;
    void setDoc(const QString& doc);

    QIcon icon() const;
    void setIcon(const QIcon& icon);

    CrNode* createNode() const;
    friend class CrPlugin;

signals:
    void pathChanged();
    void docChanged();
    void iconChanged();

private:
    Q_DISABLE_COPY(CrModule)
    CrModulePrivate* d_;

    void installPlugin(CrPlugin* plugin);

public:
    static QList<CrModule*> modules();
    static CrModule* module(const CrModuleKey& key);
    static CrModule* module(const QString& id, QVersionNumber version = QVersionNumber());

    static CrNode* createNode(const CrModuleKey& key);
    static CrNode* createNode(const QString& id, QVersionNumber version = QVersionNumber());
};

class CrModuleKey {
public:
    CrModuleKey(QString id = QString(), QVersionNumber version = QVersionNumber()) :
        id_(id), version_(version){}

    inline QString id() const { return id_; }

    QVersionNumber version() const { return version_;}

private:
    QString id_;
    QVersionNumber version_;
    friend QDataStream &operator<<(QDataStream &out, const CrModuleKey& key);
    friend QDataStream &operator>>(QDataStream &in, CrModuleKey& key);
};

inline bool operator==(const CrModuleKey lhs, const CrModuleKey rhs)
{
    return lhs.id() == rhs.id() && lhs.version() == rhs.version();
}

inline uint qHash(const CrModuleKey key, uint seed)
{
    return qHash(key.id(), seed) ^ qHash(key.version(), seed);
}



#endif // CRMODULE_H
