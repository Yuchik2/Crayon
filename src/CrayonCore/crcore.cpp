#include "crcore.h"

#include <QDir>
#include <QPluginLoader>
#include <QIcon>
#include <QApplication>
#include <QTranslator>

#include "crplugin.h"
#include "crplugininterface.h"


class CrCorePrivate {
public:
    CrCore::Mode mode_;
    QList<QTranslator*> translators_;
};

CrCore* CrCore::instance()
{
    static CrCore* instance = new CrCore();
    return instance;
}

CrCore::Mode CrCore::mode()
{
    return instance()->d_->mode_;
}

void CrCore::setMode(CrCore::Mode mode)
{
    if(instance()->d_->mode_ == mode)
        return;

    instance()->d_->mode_ = mode;
    instance()->modeChanged();
}

void CrCore::addPluginsPath(const QString &path)
{ 
    QDir dir(path);
    if(!dir.exists() || dir.isEmpty())
        return;

    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks | QDir::NoDotAndDotDot);

    bool changed_ = false;
    foreach (QFileInfo fileInfo, dir.entryInfoList()) {

        if(!QLibrary::isLibrary(fileInfo.filePath()))
            continue;

        QPluginLoader* loader = new QPluginLoader(fileInfo.filePath());
        // loader->setLoadHints(QLibrary::ExportExternalSymbolsHint);

        QString IID = loader->metaData().value("IID").toString();
        if (IID != "com.crayonf.plugin/1.0"){
            loader->deleteLater();
            continue;
        }

        QJsonObject metaData = loader->metaData().value("MetaData").toObject();

        if (!metaData.contains("id") || !metaData["id"].isString()){
            emit instance()->errorOccured(tr("Error loading plugin \"%1\". Invalid plugin meta data id.")
                                          .arg(fileInfo.absolutePath()));
            loader->deleteLater();
            continue;
        }

        QString id = metaData["id"].toString();

        if (!metaData.contains("version") || !metaData["version"].isString()){
            emit instance()->errorOccured(tr("Error loading plugin \"%1\". Invalid plugin meta data version.")
                                          .arg(fileInfo.absolutePath()));
            loader->deleteLater();
            continue;
        }

        QVersionNumber version = QVersionNumber::fromString(metaData["version"].toString());

        QString name;
        if (metaData.contains("name") && metaData["name"].isString()){
            name = metaData["name"].toString();
        }

        QString organization;
        if (metaData.contains("organization") && metaData["organization"].isString()){
            organization = metaData["organization"].toString();
        }

        QString domain;
        if (metaData.contains("domain") && metaData["domain"].isString()){
            domain = metaData["domain"].toString();
        }

        QString doc;
        if (metaData.contains("doc") && metaData["doc"].isString()){
            doc = metaData["doc"].toString();
        }

        QIcon icon;
        if (metaData.contains("icon") && metaData["icon"].isString()){
            icon = QIcon(metaData["icon"].toString());
        }

        CrPlugin* plugin = new CrPlugin(id, version, loader);
        plugin->setName(name);
        plugin->setOrganizationName(organization);
        plugin->setDomain(domain);
        plugin->setDoc(doc);
        plugin->setIcon(icon);

        changed_ = true;
    }

    if(changed_)
        emit instance()->pluginsChanged();
}

QList<CrPlugin *> CrCore::plugins()
{
    return CrPlugin::plugins();
}

void CrCore::installTranslator(const QString &path, const QLocale &locale)
{
    foreach (QTranslator* translator, instance()->d_->translators_) {
        QApplication::removeTranslator(translator);
        translator->deleteLater();
    }
    instance()->d_->translators_.clear();

    QString localeName = locale.name();
    QString p = path + "/" +localeName;

    QDir dir(p);
    if(dir.exists()){
        dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
        foreach (QFileInfo fileInfo, dir.entryInfoList()) {
            QTranslator* translator = new QTranslator();
            if(translator->load(fileInfo.absoluteFilePath()))
                QApplication::installTranslator(translator);
            else {
                translator->deleteLater();
            }
        }
    }
    emit instance()->translatorsChanged();
}

CrCore::CrCore() : QObject () , d_(new CrCorePrivate()) {}

CrCore::~CrCore()
{
    delete d_;
}
