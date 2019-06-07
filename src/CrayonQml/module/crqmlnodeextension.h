#ifndef CRQMLNODEEXTENSION_H
#define CRQMLNODEEXTENSION_H

#include <QObject>
#include <QQmlListProperty>

class CrNode;
class CrQmlNodeExtension : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<QObject> data READ data)
    Q_CLASSINFO("DefaultProperty", "data")

public:
    explicit CrQmlNodeExtension(QObject *parent = nullptr);

    QQmlListProperty<QObject> data();

signals:
    void valueChangedExtension();
private:
    CrNode* node_;
    QList<QObject*> data_;

    static void appendData(QQmlListProperty<QObject>*list, QObject*object);
    static int dataCount(QQmlListProperty<QObject> *list);
    static QObject* atData(QQmlListProperty<QObject>*list, int index);
    static void clearData(QQmlListProperty<QObject> *list);
};

#endif // CRQMLNODEEXTENSION_H
