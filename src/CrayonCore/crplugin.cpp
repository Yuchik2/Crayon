#include "crplugin.h"

#include <QIcon>
#include <QPluginLoader>

#include "crplugininterface.h"
#include "crmodule.h"

#include <QDebug>

class CrPluginPrivate {
public:
    CrPlugin::Status status_ = CrPlugin::NotLoaded;

    QString id_;
    QVersionNumber version_;
    QString name_;
    QString organizationName;
    QString domain_;
    QString doc_;
    QIcon icon_;

    QPluginLoader* loader_;
    CrPluginInterface* pluginInterface_ = nullptr;
    QString errorString_;
    QHash<CrModuleKey, CrModule*> modules_;
    static QHash<CrPluginKey, CrPlugin*> allPlugins_;
};

QHash<CrPluginKey, CrPlugin*> CrPluginPrivate::allPlugins_;

CrPlugin::Status CrPlugin::status() { return d_->status_;}

QString CrPlugin::id() const { return d_->id_;}

QVersionNumber CrPlugin::version() const { return d_->version_;}

CrPluginKey CrPlugin::key() const
{
    return CrPluginKey(d_->id_, d_->version_);
}

QString CrPlugin::filePath() const
{
    return d_->loader_->fileName();
}

QString CrPlugin::name() const {return d_->name_;}

void CrPlugin::setName(const QString &name)
{
    if(d_->name_ == name)
        return;
    d_->name_ = name;
    emit nameChanged();
}

QString CrPlugin::organizationName() const { return d_->organizationName;}

void CrPlugin::setOrganizationName(const QString &orgName)
{
    if(d_->organizationName == orgName)
        return;
    d_->organizationName = orgName;
    emit organizationNameChanged();
}

QString CrPlugin::domain() const { return d_->domain_;}

void CrPlugin::setDomain(const QString &domain)
{
    if(d_->domain_ == domain)
        return;
    d_->domain_ = domain;
    emit domainChanged();
}

QString CrPlugin::doc() const { return d_->doc_;}

void CrPlugin::setDoc(const QString &doc)
{
    if(d_->doc_ == doc)
        return;
    d_->doc_ = doc;
    emit docChanged();
}

QIcon CrPlugin::icon() const { return d_->icon_;}

void CrPlugin::setIcon(const QIcon &icon)
{
    d_->icon_ = icon;
    emit iconChanged();
}

QString CrPlugin::errorString() const { return d_->errorString_;}

QList<CrModule *> CrPlugin::modules() const { return d_->modules_.values();}

bool CrPlugin::load()
{
    if(d_->status_ == Loaded){
        d_->errorString_ = QObject::tr("Plugin already loaded.");
        return false;
    }

    if(!d_->loader_->load()) {
        d_->status_ = Error;
        d_->errorString_ = QObject::tr("Plugin load error. Cause:\"%1\".").
                arg(d_->loader_->errorString());
        return false;
    }

    d_->pluginInterface_ = qobject_cast<CrPluginInterface*>(d_->loader_->instance());

    if(!d_->pluginInterface_){
        d_->status_ = Error;
        d_->errorString_ = QObject::tr("Plugin load error. Cause:\"%1\".").
                arg("Interface instantiation error");
        return false;
    }

    d_->pluginInterface_->install(this);
    d_->status_ = Loaded;
    return true;
}

CrPlugin::CrPlugin(const QString &id, const QVersionNumber version, QPluginLoader *loader)
    : QObject (), d_(new CrPluginPrivate)
{
    d_->id_ = id;
    d_->version_ = version;
    d_->loader_ = loader;
    CrPluginPrivate::allPlugins_.insert(key(), this);
}

CrPlugin::~CrPlugin()
{
    delete d_->loader_;
    delete d_->pluginInterface_;
    delete d_;
}

void CrPlugin::registerModule(CrModule *module)
{
    d_->modules_.insert(module->key(), module);
}

QList<CrPlugin *> CrPlugin::plugins()
{
    return CrPluginPrivate::allPlugins_.values();
}

CrPlugin *CrPlugin::plugin(const CrPluginKey &key)
{
    CrPlugin* plugin = CrPluginPrivate::allPlugins_.value(key, nullptr);
    if(!plugin){
        QVersionNumber lastVersion;
        auto iterator = CrPluginPrivate::allPlugins_.cbegin();
        while (iterator != CrPluginPrivate::allPlugins_.cend()) {
            CrPluginKey nextKey = iterator.key();
            if(nextKey.id() == key.id()
                    && nextKey.version().majorVersion() == key.version().majorVersion()
                    && nextKey.version() > key.version()
                    && (lastVersion.isNull() || nextKey.version() < lastVersion))
            {
                lastVersion = nextKey.version();
                plugin = iterator.value();
            }
            ++iterator;
        }
    }
    return plugin;
}

CrPlugin *CrPlugin::plugin(const QString &id, const QVersionNumber &version)
{
    return plugin(CrPluginKey(id, version));
}

QDataStream &operator<<(QDataStream &out, const CrPluginKey& key)
{
    out << key.id_ << key.version_;
    return out;
}

QDataStream &operator>>(QDataStream &in, CrPluginKey& key)
{
    in >> key.id_ >> key.version_;
    return in;
}


//    template<typename Factory>
//    void registerModule(const QString& id,
//                        const QVersionNumber version,
//                        const QString& path,
//                        Factory factory,
//                        const QUrl& docSource = QUrl(),
//                        const QPixmap& image = QPixmap()) {
//        CrModule* module = new TemplateModule<Factory>(id, version, path, factory, docSource, image);
//        module->plugin_ = this;
//        modules_.append(module);
//    }

