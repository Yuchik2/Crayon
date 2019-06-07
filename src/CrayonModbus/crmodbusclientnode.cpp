#include <crmodbusclientnode.h>

CrModbusClientNode::CrModbusClientNode()
{
    connect(this, &CrNode::inputNodeChanged, this, [this]{
        if(inputNode()){
            setClient(static_cast<CrModbusClientNode*>(inputNode())->client_);
        }
    });
}

CrModbusClientNode::~CrModbusClientNode()
{

}

CrModbusAbstractClient *CrModbusClientNode::client() const
{
    return client_;
}

void CrModbusClientNode::setClient(CrModbusAbstractClient *client)
{
    if(client_ == client)
        return;

    client_ = client;

    emit clientChanged();
    foreach (CrNode* outputNode, outputNodes()) {
        CrModbusClientNode* clientNode = static_cast<CrModbusClientNode*>(outputNode);
        clientNode->setClient(client_);
    }
}

bool CrModbusClientNode::isValidInputNode(CrNode *inputNode) const
{
     return static_cast<bool>(qobject_cast<CrModbusClientNode*>(inputNode));
}

bool CrModbusClientNode::isValidOunputNode(CrNode *outputNode) const
{
     return static_cast<bool>(qobject_cast<CrModbusClientNode*>(outputNode));
}
