#include <crmodbusrtuclient.h>

#include <QSerialPort>
#include <QDataStream>
#include <QTimer>

#include <crmodbusrequest.h>

#include <QDebug>

CrModbusRtuClient::CrModbusRtuClient(QObject *parent) : CrModbusAbstractClient (parent)
{

    timeoutTimer_ = new QTimer(this);
    timeoutTimer_->setSingleShot(true);
    connect(timeoutTimer_, &QTimer::timeout, this, [this](){
        if(retries_ == 0)
            QTimer::singleShot(0, currentRequest_, [request = currentRequest_](){
                request->pushError(CrModbusRequest::TimeoutError);
            });
        nextRequest();
    });
}

bool CrModbusRtuClient::open()
{

    if(isRun() || !serialPort_ || !serialPort_->isOpen())
        return false;

    connect(serialPort_, &QSerialPort::readyRead, this, &CrModbusRtuClient::readResponce);
    queue_.clear();
    retries_ = 0;
    currentRequest_ = nullptr;
    return CrModbusAbstractClient::open();
}

bool CrModbusRtuClient::close()
{
    if(!isRun())
        return false;

    if(serialPort_){
        disconnect(serialPort_, &QSerialPort::readyRead, this, &CrModbusRtuClient::readResponce);
    }

    return CrModbusAbstractClient::close();
}

QSerialPort *CrModbusRtuClient::serialPort() const
{
    return serialPort_;
}

void CrModbusRtuClient::setSerialPort(QSerialPort *port)
{
    if(serialPort_ == port)
        return;

    if(serialPort_ && isRun()){
        disconnect(serialPort_, &QSerialPort::readyRead, this, &CrModbusRtuClient::readResponce);
    }

    serialPort_ = port;

    if(serialPort_ && isRun()){
        connect(serialPort_, &QSerialPort::readyRead, this, &CrModbusRtuClient::readResponce);
    }else {
        close();
    }
    return;
}

int CrModbusRtuClient::delayOfFrame() const
{
    return delayOfFrame_;
}

void CrModbusRtuClient::setDelayOfFrame(int delayOfFrame)
{
    delayOfFrame_ = delayOfFrame;
}

void CrModbusRtuClient::processRequest(CrModbusRequest *request)
{
    if(!currentRequest_){
        currentRequest_ = request;
        retries_ = currentRequest_->retries();
        nextRequest();
    }else{
        if(!queue_.contains(request))
            queue_.enqueue(request);
    }
}

void CrModbusRtuClient::nextRequest()
{
    timeoutTimer_->stop();

    if(!serialPort_ || !serialPort_->isOpen()){
        queue_.clear();
        currentRequest_ = nullptr;
        return;
    }

    if(!isRun()){
        return;
    }

    if(!currentRequest_ || retries_ <= 0){
        if(!queue_.isEmpty()){
            currentRequest_ = queue_.dequeue();
            retries_ = currentRequest_->retries();
        }else{
            currentRequest_ = nullptr;
            return;
        }
    }

    --retries_;
    serialPort_->clear();
    readBuffer_.clear();
    timeoutTimer_->start(currentRequest_->timeout());

    writeRequest();

}

void CrModbusRtuClient::writeRequest()
{  
    QByteArray adu;
    QDataStream aduStream(&adu, QIODevice::WriteOnly);
    aduStream << currentRequest_->serverAddress()
              << static_cast<quint8>(currentRequest_->functionCode())
              << currentRequest_->startAddress();

    switch (currentRequest_->functionCode()) {
    case CrModbusRequest::ReadCoils:
    case CrModbusRequest::ReadDiscreteInputs:
    case CrModbusRequest::ReadHoldingRegisters:
    case CrModbusRequest::ReadInputRegisters:{
        aduStream << quint16(currentRequest_->sizeData());
        break;
    }
    case CrModbusRequest::WriteSingleCoil:{
        quint16 result;
        char c = currentRequest_->data()[0];
        if( c != 0){
            result = 0xff00;
        }else{
            result = 0x0000;
        }
        aduStream << result;
        break;
    }
    case CrModbusRequest::WriteSingleRegister:{
        aduStream.writeRawData(currentRequest_->data().leftJustified(2, 0x00, true), 2);
        break;
    }
    case CrModbusRequest::WriteMultipleCoils:{
        quint8 size = 1 + currentRequest_->sizeData() / 8 + currentRequest_->sizeData() % 8 ? 1 : 0;
        aduStream << currentRequest_->sizeData() << size;
        aduStream.writeRawData(currentRequest_->data().leftJustified(size, 0x00, true), size);
        break;
    }
    case CrModbusRequest::WriteMultipleRegisters:{
        quint8 size =  currentRequest_->sizeData() * 2;
        aduStream << currentRequest_->sizeData() << size;
        aduStream.writeRawData(currentRequest_->data().leftJustified(size, 0x00, true), size);
        break;
    }
    }

    aduStream << calculateCRC(adu.constData(), adu.size());

    serialPort_->write(adu);
}

void CrModbusRtuClient::readResponce()
{ 

    readBuffer_ += serialPort_->read(serialPort_->bytesAvailable());

    //check size
    if(readBuffer_.size() < 5){
        return;
    }

    QDataStream bufferStream(&readBuffer_, QIODevice::ReadOnly);

    quint8 serverAddress;
    quint8 functionCode;
    bufferStream >> serverAddress >> functionCode;

    //check serverAddress
    if(serverAddress != currentRequest_->serverAddress()){
        if(retries_ == 0)
            QTimer::singleShot(0, currentRequest_, [request = currentRequest_](){
                request->pushError(CrModbusRequest::UnknownError);

            });
        timeoutTimer_->stop();
        QTimer::singleShot(delayOfFrame_, this, [this](){
            nextRequest();
        });
        return;
    }

    //check function code
    if(functionCode != currentRequest_->functionCode()){

        if(functionCode == (currentRequest_->functionCode() | 0x80)){

            quint8 errorCode;
            bufferStream >> errorCode;
            quint16 crc;
            bufferStream >> crc;
            if(crc != calculateCRC(readBuffer_.constData(), 3)){
                if(retries_ == 0)
                    QTimer::singleShot(0, currentRequest_, [request = currentRequest_](){
                        request->pushError(CrModbusRequest::CrcError);

                    });
                timeoutTimer_->stop();
                QTimer::singleShot(delayOfFrame_, this, [this](){
                    nextRequest();
                });
                return;
            };
            if(retries_ == 0)
                QTimer::singleShot(0, currentRequest_, [errorCode, request = currentRequest_](){
                    request->pushError(CrModbusRequest::ErrorCode(errorCode));
                });
        }else{
            if(retries_ == 0)
                QTimer::singleShot(0, currentRequest_, [request = currentRequest_](){
                    request->pushError(CrModbusRequest::UnknownError);
                });
        }
        timeoutTimer_->stop();
        QTimer::singleShot(delayOfFrame_, this, [this](){
            nextRequest();
        });
        return;
    }

    // read responce
    switch (currentRequest_->functionCode()) {
    case CrModbusRequest::ReadCoils:
    case CrModbusRequest::ReadDiscreteInputs:
    case CrModbusRequest::ReadHoldingRegisters:
    case CrModbusRequest::ReadInputRegisters:{

        quint8 size;
        bufferStream >> size;
        if(readBuffer_.size() < 5 + size){
            return;
        }

        char data[size];
        bufferStream.readRawData(data, size);

        quint16 crc;
        bufferStream >> crc;
        if(crc != calculateCRC(readBuffer_.constData(), 3 + size)){
            if(retries_ == 0)
                QTimer::singleShot(0, currentRequest_, [request = currentRequest_](){
                    request->pushError(CrModbusRequest::CrcError);

                });
            timeoutTimer_->stop();
            QTimer::singleShot(delayOfFrame_, this, [this](){
                nextRequest();
            });
            return;
        };
        QByteArray arrData(data, size);

        if(currentRequest_->data() != arrData){
            QTimer::singleShot(0, currentRequest_, [request = currentRequest_, arrData](){
                request->setData(arrData);
            });
        }
        break;
    }
    case CrModbusRequest::WriteSingleCoil:
    case CrModbusRequest::WriteSingleRegister:{
        quint16 startAddress, data, crc;
        bufferStream >> startAddress >> data >> crc;
        if(crc != calculateCRC(readBuffer_.constData(), 6)){
            if(retries_ == 0)
                QTimer::singleShot(0, currentRequest_, [request = currentRequest_](){
                    request->pushError(CrModbusRequest::CrcError);

                });
            timeoutTimer_->stop();
            QTimer::singleShot(delayOfFrame_, this, [this](){
                nextRequest();
            });
            return;
        };
        break;
    }
    case CrModbusRequest::WriteMultipleCoils:
    case CrModbusRequest::WriteMultipleRegisters:{
        quint16 startAddress, size, crc;
        bufferStream >> startAddress >> size >> crc;
        if(crc != calculateCRC(readBuffer_.constData(), 6)){
            if(retries_ == 0)
                QTimer::singleShot(0, currentRequest_, [request = currentRequest_](){
                    request->pushError(CrModbusRequest::CrcError);

                });
            timeoutTimer_->stop();
            QTimer::singleShot(delayOfFrame_, this, [this](){
                nextRequest();
            });
            return;
        };
        break;
    }
    }
    retries_ = 0;
    timeoutTimer_->stop();
    QTimer::singleShot(delayOfFrame_, this, [this](){
        nextRequest();
    });
    return;
}
