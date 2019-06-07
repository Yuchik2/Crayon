#ifndef CRMODBUSREQUEST_H
#define CRMODBUSREQUEST_H

#include <crayonmodbus_global.h>
#include <QObject>

class CRAYON_MODBUS_EXPORT CrModbusRequest : public QObject
{
    Q_OBJECT
public:
    enum FunctionCode {
        ReadCoils = 0x01,
        ReadDiscreteInputs = 0x02,
        ReadHoldingRegisters = 0x03,
        ReadInputRegisters = 0x04,
        WriteSingleCoil = 0x05,
        WriteSingleRegister = 0x06,
        WriteMultipleCoils = 0x0F,
        WriteMultipleRegisters = 0x10
    };
    Q_ENUM(FunctionCode)

    enum ErrorCode{
        UnknownError = 0x00,
        IllegalFunction = 0x01,
        IllegalDataAddress = 0x02,
        IllegalDataValue = 0x03,
        SlaveDeviceFailure = 0x04,
        Acknowledge = 0x05,
        SlaveDeviceBusy = 0x06,
        NegativeAcknowledge = 0x07,
        MemoryParityError = 0x08,
        GatewayPathUnavailable = 0x0a,
        DeviceFailedToRespond = 0x0b,
        TimeoutError = 0x20,
        CrcError = 0x21
    };
    Q_ENUM(ErrorCode)

    explicit CrModbusRequest(QObject *parent = nullptr);

    quint8 serverAddress() const;
    void setServerAddress(quint8 serverAddress);

    FunctionCode functionCode() const;
    void setFunctionCode(FunctionCode functionCode);

    quint16 startAddress() const;
    void setStartAddress(quint16 startAddress);

    quint16 sizeData() const;
    void setSizeData(quint16 sizeData);

    QByteArray data() const;
    void setData(const QByteArray &data);

    int interval() const;
    void setInterval(int interval);

    int timeout() const;
    void setTimeout(int timeout);

    int retries() const;
    void setRetries(int retries);

    bool isEnable() const;
    void setEnable(bool isEnable);

    ErrorCode error() const;
    QString errorString();

    void pushError(CrModbusRequest::ErrorCode error);

signals:
    void serverAddressChanged(quint8 serverAddress);
    void functionCodeChanged(FunctionCode functionCode);
    void startAddressChanged(quint16 startAddress);
    void sizeDataChanged(quint16 sizeData);
    void dataChanged(const QByteArray &data);
    void intervalChanged(int frequency);
    void timeoutChanged(int timeout);
    void retriesChanged(int retries);
    void enableChanged(bool isEnable);

    void errorOccurred(ErrorCode error);

private:
    Q_DISABLE_COPY(CrModbusRequest)
    quint8 serverAddress_ = 0;
    FunctionCode functionCode_ = ReadCoils;
    quint16 startAddress_ = 0;
    quint16 sizeData_ = 1;
    QByteArray data_;
    int interval_ = 0;
    int timeout_ = 100;
    int retries_ = 3;
    bool enable_ = false;

    ErrorCode error_ = UnknownError;
};

Q_DECLARE_METATYPE(CrModbusRequest::ErrorCode)

#endif // CRMODBUSREQUEST_H
