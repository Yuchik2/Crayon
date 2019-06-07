#ifndef CRSTANDARTPLUGIN_H
#define CRSTANDARTPLUGIN_H

#include "crplugininterface.h"

class CrStandartPlugin : public QObject, public CrPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(CrPluginInterface)
#if QT_VERSION >= 0x050000
    Q_PLUGIN_METADATA(IID "com.crayonf.plugin/1.0" FILE "CrayonStandartPlugin.json")
#endif // QT_VERSION >= 0x050000

public:
    CrStandartPlugin(QObject *parent = nullptr);
    virtual ~CrStandartPlugin() override;
    virtual void install(CrPlugin* plugin) override;
    virtual void uninstall(CrPlugin *plugin) override;

};

#endif // CRSTANDARTPLUGIN_H
