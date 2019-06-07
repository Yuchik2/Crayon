#ifndef CRMODBUSABSTRACTCLIENT_H
#define CRMODBUSABSTRACTCLIENT_H

#include <crayonmodbus_global.h>

#include <QObject>
#include <QHash>

class CrModbusRequest;
class QTimer;
class CRAYON_MODBUS_EXPORT CrModbusAbstractClient : public QObject
{
    Q_OBJECT
public:
    explicit CrModbusAbstractClient(QObject *parent = nullptr);
    virtual ~CrModbusAbstractClient();

    virtual bool open();
    virtual bool close();

    bool isRun() const;

    bool addRequest(CrModbusRequest* request);
    bool removeRequest(CrModbusRequest* request);
    virtual void processRequest(CrModbusRequest* request) = 0;

    QList<CrModbusRequest *> requests() const;

signals:
    void errorOccurred(const QString& error);

private:
    Q_DISABLE_COPY(CrModbusAbstractClient)
    QList<CrModbusRequest*> requests_;
    QHash<CrModbusRequest*, QTimer*> timers_;
    bool run_ = false;

private slots:
    void changeInterval();
    void changeFunctionCode();
    void changeData();
};

#endif // CRMODBUSABSTRACTCLIENT_H
