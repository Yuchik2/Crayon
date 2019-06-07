#include "crqmlplugin.h"

#include <QQmlEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <QDir>
#include <QStyleOptionButton>
#include <QApplication>

#include <CrModule>
#include <CrColorNode>
#include <CrFileNode>

#include <crqmlnode.h>
#include "crqmlmodule.h"

#include <crqmlmonitornode.h>

#include <QDebug>

CrQmlPlugin::CrQmlPlugin(QObject *parent) : QObject(parent)
{

}

CrQmlPlugin::~CrQmlPlugin()
{

}

void CrQmlPlugin::install(CrPlugin* plugin)
{   
    QDir dir("CrayonQmlModules");
    if(dir.exists()){
        dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
        foreach (QFileInfo fileInfo, dir.entryInfoList()) {

            if(fileInfo.suffix() != "qml"){
                continue;
            }

            loadQmlModule(plugin, fileInfo.filePath());
        }
    }

    new CrModule(plugin,
                 "qml/monitor",
                 QVersionNumber(1,0,0),
                 createMonitorNode,
                 tr("QML/Monitor"),
                 tr("Qml monitor"),
                 QIcon(":/CrayonQmlPluginResources/monitor.svg"));

    new CrModule(plugin,
                 "qml/nullitem",
                 QVersionNumber(1,0,0),
                 createNullItemNode,
                 tr("QML/Null item"),
                 tr("Qml null item"),
                 QIcon(":/CrayonQmlPluginResources/null_item.svg"));

}

void CrQmlPlugin::uninstall(CrPlugin* plugin)
{
    Q_UNUSED(plugin);
}


void CrQmlPlugin::loadQmlModule(CrPlugin *plugin, const QString &path)
{
    QQmlComponent component(CrQmlNode::engine(), path);
    if(component.isError()){
        QString errors;
        foreach (QQmlError error, component.errors()) {
            errors.append(error.toString() + "\n");
        }
        qDebug() << errors;
        //        crSendMessage(QObject::tr("Fault load qml module"),
        //                      QObject::tr("Invalid module node: \"%1\". Cause: \"%2\".").
        //                      arg(path).arg(errors), CrLog::Warning);
        return;
    }

    QObject* object = component.create();
    CrNode* node = qobject_cast<CrNode*>(object);
    while(node){
        CrQmlModule *attached = qobject_cast<CrQmlModule*>(qmlAttachedPropertiesObject<CrQmlModule>(node));

        QString id = attached->id();
        if(id.isEmpty())
            break;

        QVersionNumber version = QVersionNumber::fromString(attached->version());
        if(version.isNull())
            break;

        QString p = attached->path();
        if(p.isEmpty())
            break;
        p = QStringLiteral("QML/") + p;

        QString description = attached->description();
        QString pathImage = QFileInfo(path).absolutePath() + "/" + attached->image();

        auto moduleFactory = [path](const CrModule* module) -> CrNode*{
                QQmlComponent component(CrQmlNode::engine(), path);
                return qobject_cast<CrNode*>(component.create());
    };

        new CrModule(plugin, id, version, moduleFactory, p, description, QIcon(pathImage));
        break;
    }
}

CrNode *CrQmlPlugin::createMonitorNode(const CrModule* module)
{
    CrQmlMonitorNode* node = new CrQmlMonitorNode();
    node->setName(QObject::tr("Monitor"));
    node->setComments(QObject::tr("Qml monitor"));
    node->setFlags(CrNode::EditableName | CrNode::ActiveEditor | CrNode::Out);

    CrColorNode* backgroundColorNode = new CrColorNode(node);
    backgroundColorNode->setName(QObject::tr("Background color"));
    backgroundColorNode->setFlags(CrNode::ActiveEditor | CrNode::In);
    connect(backgroundColorNode, &CrColorNode::colorChanged, node, [backgroundColorNode, node]{
        node->setBackgroundColor(backgroundColorNode->color());
    });

    CrFileNode* backgroundImageNode = new CrFileNode(node);
    backgroundImageNode->setName(QObject::tr("Background image"));
    backgroundImageNode->setFlags(CrNode::ActiveEditor | CrNode::In);
    backgroundImageNode->setNameFilter("Image files (*.png *.xpm *.jpg)");
    connect(backgroundImageNode, &CrFileNode::filesChanged, node, [backgroundImageNode, node]{
        if(backgroundImageNode->files().isEmpty()){
            node->setBackgroundImage(QImage());
        }else {
            node->setBackgroundImage(QImage(backgroundImageNode->files().first()));
        }
    });

    return node;
}

CrNode *CrQmlPlugin::createNullItemNode(const CrModule *module)
{

    CrQmlNode* node = new CrQmlNode();
    node->setName(QObject::tr("Null item"));
    node->setFlags(CrNode::EditableName | CrNode::In | CrNode::ActiveEditor);
    return node;
}


#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(CrModulePlugin, CrQmlPlugin);
#endif // QT_VERSION < 0x050000
