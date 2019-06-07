#include "crserialportnode.h"

#include <QThread>
#include <QSerialPort>
#include <QSerialPortInfo>

#include <crproject.h>
#include <crlog.h>

CrSerialPortNode::CrSerialPortNode() : CrNode()
{
    connect(this, &CrNode::inputNodeChanged, this, [this]{
        if(inputNode()){
            setSerialPort(static_cast<CrSerialPortNode*>(inputNode())->serialPort_);
        }
    });
}

QSerialPort *CrSerialPortNode::serialPort() const
{
//    if(inputNode()){
//        return static_cast<CrSerialPortNode*>(inputNode())->serialPort();
//    }
    return serialPort_;
}

void CrSerialPortNode::setSerialPort(QSerialPort *serialPort)
{
    if(serialPort_ == serialPort)
        return;
    serialPort_ = serialPort;
    if(serialPort_ && serialPort_->thread() == thread()){
        serialPort_->moveToThread(serialPortThread());
    }
    emit serialPortChanged();
    foreach (CrNode* outputNode, outputNodes()) {
        CrSerialPortNode* serialProtNode = static_cast<CrSerialPortNode*>(outputNode);
        serialProtNode->setSerialPort(serialPort_);
    }
}

QThread *CrSerialPortNode::serialPortThread()
{
    static QThread* thread = nullptr;
    if(!thread){
        thread = new QThread();
        thread->setObjectName("serialPort Thread");
        thread->start();
    }
    return thread;
}

bool CrSerialPortNode::isValidInputNode(CrNode *inputNode) const
{
    return static_cast<bool>(qobject_cast<CrSerialPortNode*>(inputNode));
}

bool CrSerialPortNode::isValidOunputNode(CrNode *outputNode) const
{
    return static_cast<bool>(qobject_cast<CrSerialPortNode*>(outputNode));
}



