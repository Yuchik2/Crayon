#ifndef CRQMLPLUGIN_H
#define CRQMLPLUGIN_H

#include "crplugininterface.h"

class CrNode;
class CrModule;
class CrQmlPlugin : public QObject, public CrPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(CrPluginInterface)
#if QT_VERSION >= 0x050000
    Q_PLUGIN_METADATA(IID "com.crayonf.plugin/1.0" FILE "CrayonQmlPlugin.json")
#endif // QT_VERSION >= 0x050000

public:
    CrQmlPlugin(QObject *parent = nullptr);
    virtual ~CrQmlPlugin();
    virtual void install(CrPlugin* plugin) override;
    virtual void uninstall(CrPlugin *plugin) override;

private:
    void loadQmlModule(CrPlugin* plugin, const QString& path);    
    static CrNode *createMonitorNode(const CrModule *module);
    static CrNode *createNullItemNode(const CrModule *module);

};
#endif // CRQMLPLUGIN_H
