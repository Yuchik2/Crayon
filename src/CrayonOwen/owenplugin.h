#ifndef CRMODBUSPLUGIN_H
#define CRMODBUSPLUGIN_H

#include "crplugininterface.h"

class CrNode;
class CrModule;
class OwenPlugin : public QObject, public CrPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(CrPluginInterface)
#if QT_VERSION >= 0x050000
    Q_PLUGIN_METADATA(IID "com.crayonf.plugin/1.0" FILE "OwenPlugin.json")
#endif // QT_VERSION >= 0x050000

public:
   OwenPlugin(QObject *parent = nullptr);
   virtual ~OwenPlugin() override;
   virtual void install(CrPlugin* plugin) override;
   virtual void uninstall(CrPlugin *plugin) override;

private:
   static CrNode* create_MU_110_32(const CrModule* module);
   static CrNode* create_MV_110_32(const CrModule* module);

};
#endif // CRMODBUSPLUGIN_H
