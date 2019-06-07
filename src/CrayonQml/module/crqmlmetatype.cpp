//#include "crqmlmetatype.h"



//CrQmlMetaType::CrQmlMetaType(QObject *parent) : QObject(parent)
//{

//}

//int CrQmlMetaType::getType(const QVariant &value)
//{
//    if(value.userType() == QMetaType::QObjectStar){
//        QByteArray name(value.value<QObject*>()->metaObject()->className());
//        name.append("*");
//        return QMetaType::type(name);

//    }else{
//        return  value.userType();
//    }
//}
