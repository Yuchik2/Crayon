#ifndef CRMODBUSTCPCLIENT_H
#define CRMODBUSTCPCLIENT_H

#include <crmodbusabstractclient.h>

#include <QSerialPort>
#include <QQueue>
#include <QHash>

class QSerialPort;
class QTimer;

class QState;
class QFinalState;
class QStateMachine;

class CrModbusRequest;

class CRAYON_MODBUS_EXPORT CrModbusTcpClient : public CrModbusAbstractClient
{
    Q_OBJECT
public:
    explicit CrModbusTcpClient(QObject *parent = nullptr);

    virtual void processRequest(CrModbusRequest* request) override;

signals:
    void errorOccurred(const QString& error);

private:
    Q_DISABLE_COPY(CrModbusTcpClient)

};

#endif // CRMODBUSTCPCLIENT_H
