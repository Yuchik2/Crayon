#include <crmodbusabstractclient.h>
#include <QTimer>

#include <crmodbusrequest.h>

#include <QDebug>



CrModbusAbstractClient::CrModbusAbstractClient(QObject *parent) : QObject (parent)
{

}

CrModbusAbstractClient::~CrModbusAbstractClient()
{

}

bool CrModbusAbstractClient::open()
{
    if(run_)
        return false;
    run_ = true;
    auto it = timers_.begin();
    while (it != timers_.end()) {
        if(it.key()->interval() > 0){
            it.value()->setInterval(it.key()->interval());
            it.value()->start();
        }
        processRequest(it.key());
        ++it;
    }
    return true;
}

bool CrModbusAbstractClient::close()
{
    if(!run_)
        return false;
    run_ = false;
    auto it = timers_.begin();
    while (it != timers_.end()) {
        it.value()->stop();
        ++it;
    }
    return true;
}

bool CrModbusAbstractClient::addRequest(CrModbusRequest *request)
{

    if(requests_.contains(request))
        return false;
    requests_.append(request);
    QTimer* timer = new QTimer(this);
    timers_.insert(request, timer);
    connect(timer, &QTimer::timeout, this, [this, request]{
        processRequest(request);
    });

    connect(request, &CrModbusRequest::intervalChanged, this, &CrModbusAbstractClient::changeInterval);
    connect(request, &CrModbusRequest::functionCodeChanged, this, &CrModbusAbstractClient::changeFunctionCode);

    switch (request->functionCode()) {
    case CrModbusRequest::WriteSingleCoil:
    case CrModbusRequest::WriteMultipleCoils:
    case CrModbusRequest::WriteSingleRegister:
    case CrModbusRequest::WriteMultipleRegisters:
        connect(request, &CrModbusRequest::dataChanged, this, &CrModbusAbstractClient::changeData);
        break;
    default:
        break;
    }

    if(run_ && request->interval() > 0){
        processRequest(request);
        timer->setInterval(request->interval());
        timer->start();
    }
    return true;
}

bool CrModbusAbstractClient::removeRequest(CrModbusRequest *request)
{
    if(!requests_.contains(request))
        return false;

    disconnect(request, &CrModbusRequest::intervalChanged, this, &CrModbusAbstractClient::changeInterval);
    disconnect(request, &CrModbusRequest::functionCodeChanged, this, &CrModbusAbstractClient::changeFunctionCode);

    switch (request->functionCode()) {
    case CrModbusRequest::WriteSingleCoil:
    case CrModbusRequest::WriteMultipleCoils:
    case CrModbusRequest::WriteSingleRegister:
    case CrModbusRequest::WriteMultipleRegisters:
        disconnect(request, &CrModbusRequest::dataChanged, this, &CrModbusAbstractClient::changeData);
        break;
    default:
        break;
    }

    timers_.value(request)->deleteLater();
    timers_.remove(request);
    requests_.removeOne(request);
    return true;
}

QList<CrModbusRequest *> CrModbusAbstractClient::requests() const
{
    return requests_;
}

bool CrModbusAbstractClient::isRun() const
{
    return run_;
}

void CrModbusAbstractClient::changeInterval()
{
    CrModbusRequest* request  = static_cast<CrModbusRequest*>(sender());
    QTimer* timer = timers_.value(request);

    if(run_ && request->interval() > 0){
        timer->setInterval(request->interval());
        timer->start();
    } else {
        timer->stop();
    }
}

void CrModbusAbstractClient::changeFunctionCode()
{
    CrModbusRequest* request  = static_cast<CrModbusRequest*>(sender());
    disconnect(request, &CrModbusRequest::dataChanged, this, &CrModbusAbstractClient::changeData);
    switch (request->functionCode()) {
    case CrModbusRequest::WriteSingleCoil:
    case CrModbusRequest::WriteMultipleCoils:
    case CrModbusRequest::WriteSingleRegister:
    case CrModbusRequest::WriteMultipleRegisters:
        connect(request, &CrModbusRequest::dataChanged, this, &CrModbusAbstractClient::changeData);
        break;
    default:
        break;
    }
}

void CrModbusAbstractClient::changeData()
{
    if(run_)
        processRequest(static_cast<CrModbusRequest*>(sender()));
}
