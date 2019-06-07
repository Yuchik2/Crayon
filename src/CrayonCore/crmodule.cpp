#include "crmodule.h"

#include "crplugin.h"
#include "crnode.h"

class CrModulePrivate {

public:
    QString id_;
    QVersionNumber version_;
    CrModule::FactoryFunction factory_;
    QString path_;
    QString doc_;
    QIcon icon_;
    CrPlugin* plugin_;

    static QHash<CrModuleKey, CrModule*> allModules_;
};

QHash<CrModuleKey, CrModule*> CrModulePrivate::allModules_;

CrModule::CrModule(CrPlugin* plugin,
                   const QString &id,
                   const QVersionNumber &version,
                   const CrModule::FactoryFunction &factory,
                   const QString& path,
                   const QString &doc,
                   const QIcon &icon)
    : QObject (),
      d_(new CrModulePrivate())
{
    d_->plugin_ = plugin;
    d_->id_ = id;
    d_->version_ = version;
    d_->factory_ = factory;
    if(path.isEmpty()){
        d_->path_ = id;
    }else {
        d_->path_ = path;
    }
    d_->doc_ = doc;
    d_->icon_ = icon;
    CrModulePrivate::allModules_.insert(key(), this);
    plugin->registerModule(this);
}

CrModule::~CrModule()
{
    delete d_;
}

const CrPlugin *CrModule::plugin() const { return  d_->plugin_;}

QString CrModule::id() const {return  d_->id_;}

QVersionNumber CrModule::version() const {return  d_->version_;}

CrModuleKey CrModule::key() const
{
    return CrModuleKey(d_->id_, d_->version_);
}

QString CrModule::path() const { return  d_->path_;}

void CrModule::setPath(const QString &path)
{
    if(d_->path_ == path)
        return;

    d_->path_ = path;
    emit pathChanged();
}

QString CrModule::doc() const { return  d_->doc_;}

void CrModule::setDoc(const QString &doc)
{
    if(d_->doc_ == doc)
        return;

    d_->doc_ = doc;
    emit docChanged();
}

QIcon CrModule::icon() const {return d_->icon_;}

void CrModule::setIcon(const QIcon &icon)
{
    d_->icon_ = icon;
    emit iconChanged();
}

CrNode *CrModule::createNode() const
{
    CrNode* instance = d_->factory_(const_cast<CrModule*>(this));
    instance->setModule(const_cast<CrModule*>(this));
    return instance;
}

QList<CrModule *> CrModule::modules()
{
    return CrModulePrivate::allModules_.values();
}

CrModule *CrModule::module(const CrModuleKey &key)
{
    return CrModulePrivate::allModules_.value(key, nullptr);
}

CrModule *CrModule::module(const QString &id, QVersionNumber version)
{
    return module(CrModuleKey(id, version));
}

CrNode *CrModule::createNode(const CrModuleKey& key)
{
    CrModule* m = module(key);
    if(!m)
        return nullptr;
    return m->createNode();
}

CrNode *CrModule::createNode(const QString &id, QVersionNumber version)
{
    return createNode(CrModuleKey(id, version));
}

QDataStream &operator<<(QDataStream &out, const CrModuleKey& key)
{
    out << key.id_ << key.version_;
    return out;
}

QDataStream &operator>>(QDataStream &in, CrModuleKey& key)
{
    in >> key.id_ >> key.version_;
    return in;
}

