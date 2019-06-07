#ifndef CRAYONQMLMODULE_H
#define CRAYONQMLMODULE_H

#include <QQmlExtensionPlugin>

class CrayonQmlModule_Plugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    void registerTypes(const char *uri) override;
};

#endif // CRAYONQMLMODULE_PLUGIN_H
