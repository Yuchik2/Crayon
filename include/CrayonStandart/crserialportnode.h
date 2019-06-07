#ifndef CRSERIALPORTNODE_H
#define CRSERIALPORTNODE_H

#include <crayonstandart_global.h>
#include <CrNode>

class QSerialPort;
class CRAYON_STANDART_EXPORT CrSerialPortNode : public CrNode
{
    Q_OBJECT
    Q_PROPERTY(QSerialPort* serialPort READ serialPort WRITE setSerialPort NOTIFY serialPortChanged)
public:
    explicit CrSerialPortNode();

    QSerialPort *serialPort() const;
    void setSerialPort(QSerialPort *serialPort);

    static QThread *serialPortThread();

    virtual bool isValidInputNode(CrNode* inputNode) const override;
    virtual bool isValidOunputNode(CrNode* outputNode) const override;

signals:
    void serialPortChanged();

private:
    QSerialPort* serialPort_ = nullptr;

};

#endif // CRSERIALPORTNODE_H
