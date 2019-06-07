#include "crqmlmodule.h"

CrQmlModule::CrQmlModule(QObject *parent) : QObject(parent)
{

}

QString CrQmlModule::id() const
{
    return id_;
}

void CrQmlModule::setId(const QString &id)
{
    id_ = id;
}

QString CrQmlModule::version() const
{
    return version_;
}

void CrQmlModule::setVersion(const QString &version)
{
    version_ = version;
}

QString CrQmlModule::path() const
{
    return path_;
}

void CrQmlModule::setPath(const QString &path)
{
    path_ = path;
}

QString CrQmlModule::description() const
{
    return description_;
}

void CrQmlModule::setDescription(const QString &description)
{
    description_ = description;
}

QString CrQmlModule::image() const
{
    return image_;
}

void CrQmlModule::setImage(const QString &image)
{
    image_ = image;
}
