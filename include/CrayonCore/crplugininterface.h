#ifndef CRPLUGININTERFACE_H
#define CRPLUGININTERFACE_H

#include "crayoncore_global.h"
#include <QtPlugin>

class CrPlugin;

class CRAYON_CORE_EXPORT CrPluginInterface {
public:
    virtual ~CrPluginInterface() {}
    virtual void install(CrPlugin* plugin) = 0;
    virtual void uninstall(CrPlugin* plugin) = 0;
};

Q_DECLARE_INTERFACE(CrPluginInterface,"com.crayonf.plugin/1.0")

#endif // CRPLUGININTERFACE_H
