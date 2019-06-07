#ifndef CRMODBUSPLUGIN_H
#define CRMODBUSPLUGIN_H

#include "crplugininterface.h"

class CrModbusPlugin : public QObject, public CrPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(CrPluginInterface)
#if QT_VERSION >= 0x050000
    Q_PLUGIN_METADATA(IID "com.crayonf.plugin/1.0" FILE "CrayonModbusPlugin.json")
#endif // QT_VERSION >= 0x050000

public:
   CrModbusPlugin(QObject *parent = nullptr);
   virtual ~CrModbusPlugin() override;
   virtual void install(CrPlugin* plugin) override;
   virtual void uninstall(CrPlugin *plugin) override;


};
#endif // CRMODBUSPLUGIN_H
