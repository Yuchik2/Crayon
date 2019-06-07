#include "crmonitor.h"

#include <QDataStream>

#include "crproject.h"
#include "crnode.h"


class CrMonitorPrivate {
public:
    CrProject* project_;
    CrNode* node_ = nullptr;
    QString name_ = "None";
    int screenNumber_ = 0;
};

CrMonitor::CrMonitor(CrProject *project, CrNode *node) : QObject(project), d_(new CrMonitorPrivate())
{
    d_->project_ = project;

}

CrMonitor::~CrMonitor()
{
    delete d_;
}

CrProject *CrMonitor::project()
{
    return d_->project_;
}

CrNode *CrMonitor::node()
{
    return d_->node_;
}

void CrMonitor::setNode(CrNode *node)
{
    if(d_->node_ == node)
        return;
    d_->node_ = node;
    emit nodeChanged();
}

QString CrMonitor::name() const
{
    return d_->name_;
}

void CrMonitor::setName(const QString &name)
{
    if(d_->name_ == name)
        return;
    d_->name_ = name;
    emit nameChanged();
}

int CrMonitor::screenNumber()
{
    return d_->screenNumber_;
}

void CrMonitor::setScreenNumber(int screenNumber)
{
    if(d_->screenNumber_ == screenNumber)
        return;
    d_->screenNumber_ = screenNumber;
    emit screenNumberChanged();
}

QByteArray CrMonitor::saveState() const
{
    QByteArray state;
    QDataStream out(&state, QIODevice::WriteOnly);

    out << d_->node_->uuid()
        << d_->name_
        << d_->screenNumber_;
    return state;
}

bool CrMonitor::restoreState(const QByteArray &state)
{
    QDataStream in(state);

    QUuid nodeUuid;
    in >> nodeUuid;
    CrNode* node = d_->project_->findNodeByUuid(nodeUuid);
    setNode(node);

    QString name;
    in  >> name;
    setName(name);

    int screenNumber;
    in >> screenNumber;
    setScreenNumber(screenNumber);

    return true;
}

QDataStream &operator<<(QDataStream &out, const CrMonitor *monitor)
{
    out << monitor->saveState();
    return out;
}

QDataStream &operator>>(QDataStream &in, CrMonitor *monitor)
{
    QByteArray nodeState;
    in >> nodeState;
    monitor->restoreState(nodeState);
    return in;
}
