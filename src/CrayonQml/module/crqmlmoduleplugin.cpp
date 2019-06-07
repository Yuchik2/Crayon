#include "crqmlmoduleplugin.h"


#include <qqml.h>

#include <CrNode>
#include <crnumericnode.h>
#include <crbooleannode.h>
#include <crtextnode.h>
#include <crbytearraynode.h>
#include <crcomboboxnode.h>

#include <crqmlmodule.h>
#include <crqmlcomponentnode.h>
#include <crqmlmonitornode.h>

#include "crqmlnodeextension.h"
#include <QDebug>


void CrayonQmlModule_Plugin::registerTypes(const char *uri)
{    
    //    qmlRegisterSingletonType<CrQmlMetaType>("Crayon", 1, 0, "MetaType", [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject * {
    //        Q_UNUSED(engine)
    //        Q_UNUSED(scriptEngine)
    //        CrQmlMetaType *example = new CrQmlMetaType();
    //        return example;
    //    });

    //    qmlRegisterSingletonType<CrLog>("Crayon", 1, 0, "CrayonLog", [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject * {
    //        Q_UNUSED(engine)
    //        Q_UNUSED(scriptEngine)
    //        return CrLog::instance();
    //    });

    qmlRegisterUncreatableType<CrQmlModule>(uri, 1, 0, "Module", QObject::tr("Module is only available via attached properties"));
    qmlRegisterType<CrNode>(uri, 1, 0, "Node");
    qmlRegisterExtendedType<CrNode, CrQmlNodeExtension>(uri, 1, 0, "Node");

    qmlRegisterType<CrIntegerNode>(uri, 1, 0, "IntegerNode");
    qmlRegisterType<CrIntegerLimitedNode>(uri, 1, 0, "IntegerLimitedNode");
    qmlRegisterType<CrRealNode>(uri, 1, 0, "RealNode");
    qmlRegisterType<CrRealLimitedNode>(uri, 1, 0, "RealLimitedNode");
    qmlRegisterType<CrBooleanNode>(uri, 1, 0, "BooleanNode");
    qmlRegisterType<CrComboBoxNode>(uri, 1, 0, "ComboBoxNode");
    qmlRegisterType<CrQmlNode>(uri, 1, 0, "QmlNode");
    qmlRegisterType<CrQmlComponentNode>(uri, 1, 0, "QmlComponentNode");
}

