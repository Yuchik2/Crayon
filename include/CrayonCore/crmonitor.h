#ifndef CRMONITOR_H
#define CRMONITOR_H

#include "crayoncore_global.h"
#include <QObject>

class CrNode;
class CrProject;
class CrMonitorPrivate;
class CRAYON_CORE_EXPORT CrMonitor : public QObject{
    Q_OBJECT
    Q_PROPERTY(CrProject* project READ project)
    Q_PROPERTY(CrNode* node READ node WRITE setNode NOTIFY nodeChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(int screenNumber READ screenNumber WRITE setScreenNumber NOTIFY screenNumberChanged)
public:
    explicit CrMonitor(CrProject* project, CrNode* node = nullptr);
    virtual ~CrMonitor() override;

    CrProject* project();

    CrNode* node();
    void setNode(CrNode* node);

    QString name() const;
    void setName(const QString& name);

    int screenNumber();
    void setScreenNumber(int screenNumber);

    QByteArray saveState() const;
    bool restoreState(const QByteArray &state);

signals:
    void nodeChanged();
    void nameChanged();
    void screenNumberChanged();

private:
    CrMonitorPrivate* d_;
};

QDataStream &operator<<(QDataStream &out, const CrMonitor* monitor);
QDataStream &operator>>(QDataStream &in, CrMonitor* monitor);


#endif // CRMONITOR_H
