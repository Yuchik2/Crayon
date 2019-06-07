#ifndef CRMODBUSCLIENTNODE_H
#define CRMODBUSCLIENTNODE_H

#include <crayonmodbus_global.h>
#include <CrNode>

class CrModbusAbstractClient;
class CRAYON_MODBUS_EXPORT CrModbusClientNode : public CrNode
{
    Q_OBJECT
    Q_PROPERTY(CrModbusAbstractClient* client READ client WRITE setClient NOTIFY clientChanged)
public:
    explicit CrModbusClientNode();
    virtual ~CrModbusClientNode() override;

    CrModbusAbstractClient *client() const;
    void setClient(CrModbusAbstractClient *client);

    virtual bool isValidInputNode(CrNode* inputNode) const override;
    virtual bool isValidOunputNode(CrNode* outputNode) const override;

signals:
    void clientChanged();

private:
    Q_DISABLE_COPY(CrModbusClientNode)
    CrModbusAbstractClient* client_ = nullptr;

};

#endif // CRMODBUSCLIENTNODE_H
