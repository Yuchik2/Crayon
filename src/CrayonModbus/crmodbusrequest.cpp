#include "crmodbusrequest.h"

CrModbusRequest::CrModbusRequest(QObject *parent) : QObject(parent)
{

}

quint8 CrModbusRequest::serverAddress() const
{
    return serverAddress_;
}

void CrModbusRequest::setServerAddress(quint8 serverAddress)
{
    if(serverAddress_ == serverAddress)
        return;
    serverAddress_ = serverAddress;
    emit serverAddressChanged(serverAddress_);
}

CrModbusRequest::FunctionCode CrModbusRequest::functionCode() const
{
    return functionCode_;
}

void CrModbusRequest::setFunctionCode(FunctionCode functionCode)
{
    if(functionCode_ == functionCode)
        return;
    functionCode_ = functionCode;
    emit functionCodeChanged(functionCode_);
}

quint16 CrModbusRequest::startAddress() const
{
    return startAddress_;
}

void CrModbusRequest::setStartAddress(quint16 startAddress)
{
    if(startAddress_ == startAddress)
        return;
    startAddress_ = startAddress;
    emit startAddressChanged(startAddress_);
}

quint16 CrModbusRequest::sizeData() const
{
    return sizeData_;
}

void CrModbusRequest::setSizeData(quint16 sizeData)
{
    if(sizeData_ == sizeData)
        return;
    sizeData_ = sizeData;
    emit sizeDataChanged(sizeData_);
}

QByteArray CrModbusRequest::data() const
{
    return data_;
}

void CrModbusRequest::setData(const QByteArray &data)
{
    if(data_ == data)
        return;
    data_ = data;
    emit dataChanged(data_);
}

int CrModbusRequest::interval() const
{
    return interval_;
}

void CrModbusRequest::setInterval(int interval)
{
    // interval = qMax(10, interval);
    if(interval_ == interval)
        return;
    interval_ = interval;
    emit intervalChanged(interval_);
}

int CrModbusRequest::timeout() const
{
    return timeout_;
}

void CrModbusRequest::setTimeout(int timeout)
{
    if(timeout_ == timeout)
        return;
    timeout_ = timeout;
    emit timeoutChanged(timeout_);
}

int CrModbusRequest::retries() const
{
    return retries_;
}

void CrModbusRequest::setRetries(int retries)
{
    if(retries_ == retries)
        return;
    retries_ = retries;
    emit retriesChanged(retries_);
}

bool CrModbusRequest::isEnable() const
{
    return enable_;
}

void CrModbusRequest::setEnable(bool enable)
{
    if(enable_ == enable)
        return;
    enable_ = enable;
    emit enableChanged(enable_);
}

CrModbusRequest::ErrorCode CrModbusRequest::error() const
{
    return error_;
}

QString CrModbusRequest::errorString()
{
    switch (error_) {
    case CrModbusRequest::IllegalFunction:
        return tr("Illegal function. "
                  "Function code received in the query is not recognized or allowed by slave.");

    case CrModbusRequest::IllegalDataAddress:
        return tr("Illegal data address. "
                  "Data address of some or all the required entities are not allowed or do not exist in slave.");

    case CrModbusRequest::IllegalDataValue:
        return tr("Illegal data value. "
                  "Value is not accepted by slave.");

    case CrModbusRequest::SlaveDeviceFailure:
        return tr("Slave device failure. "
                  "Unrecoverable error occurred while slave was attempting to perform requested action.");

    case CrModbusRequest::Acknowledge:
        return tr("Acknowledge. "
                  "Slave has accepted request and is processing it, but a long duration of time is required.");

    case CrModbusRequest::SlaveDeviceBusy:
        return tr("Slave device busy. "
                  "Slave is engaged in processing a long-duration command. Master should retry later.");

    case CrModbusRequest::NegativeAcknowledge:
        return tr("Negative acknowledge. "
                  "Slave cannot perform the programming functions. Master should request diagnostic or error information from slave.");

    case CrModbusRequest::MemoryParityError:
        return tr("Memory parity error. "
                  "Slave detected a parity error in memory. Master can retry the request, but service may be required on the slave device.");

    case CrModbusRequest::GatewayPathUnavailable:
        return tr("Gateway path unavailable. "
                  "Gateway misconfigured.");

    case CrModbusRequest::DeviceFailedToRespond:
        return tr("Gateway target device failed to respond. "
                  "Slave fails to respond.");

    case CrModbusRequest::TimeoutError:
        return tr("Timeout error.");

    case CrModbusRequest::CrcError:
        return tr("Fault сheck CRC code.");

    default:
        return tr("Unknown error.");
    }
}

void CrModbusRequest::pushError(CrModbusRequest::ErrorCode error)
{
    error_ = error;
    errorOccurred(error_);
}
