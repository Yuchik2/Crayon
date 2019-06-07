#include "owenplugin.h"

#include <QApplication>
#include <QSerialPort>
#include <QThread>
#include <QTimer>
#include <QtEndian>
#include <cstring>

#include <CrPlugin>
#include <crmodule.h>

#include <crnumericnode.h>
#include <crbooleannode.h>
#include <crbytearraynode.h>
#include <crcomboboxnode.h>
#include <crserialportnode.h>
#include <crmodbusclientnode.h>
#include <crmodbusrtuclient.h>
#include <crmodbusrequest.h>


OwenPlugin::OwenPlugin(QObject *parent) : QObject(parent)
{

}

OwenPlugin::~OwenPlugin()
{

}

void OwenPlugin::install(CrPlugin *plugin)
{

    new CrModule(plugin,
                 "owen/modbus/mu110-32",
                 QVersionNumber(1,0,0),create_MU_110_32,
                 tr("OWEN/MU 110 - 32"),
                 tr("MU 110 - 32. Discrete output module"),
                 QIcon(":/OwenPluginResources/mu110-32r.png"));

    new CrModule(plugin,
                 "owen/modbus/mv110-32",
                 QVersionNumber(1,0,0),
                 create_MV_110_32,
                 tr("OWEN/MV 110 - 32"),
                 tr("MV 110 - 32. Discrete input module"),
                 QIcon(":/OwenPluginResources/mv110-32dn.png"));

}

void OwenPlugin::uninstall(CrPlugin *plugin)
{
    Q_UNUSED(plugin)
}

CrNode *OwenPlugin::create_MU_110_32(const CrModule *module)
{

    CrModbusClientNode* node = new CrModbusClientNode();
    node->setName(tr("MU 110 - 32"));
    node->setComments(tr("Discrete output module"));
    node->setFlags(CrNode::EditableName | CrNode::In);

    CrModbusRequest* request = new CrModbusRequest();
    request->setFunctionCode(CrModbusRequest::WriteMultipleRegisters);
    request->setSizeData(2);
    request->setStartAddress(97);
    request->moveToThread(CrSerialPortNode::serialPortThread());

    connect(node, &QObject::destroyed, request, &QObject::deleteLater);
    connect(request, &CrModbusRequest::errorOccurred, node, [node, request](){
        QTimer::singleShot(500, request, [request, node]{
            if(node->client()){
                node->client()->processRequest(request);
            }
        });
        node->pushErrorMessage(tr("Modbus request error."), request->errorString());
    });

    CrNode* doNodes = new CrNode();
    doNodes->setName(tr("DO"));
    doNodes->setComments(tr("Discrete outputs"));
    node->appendChildNode(doNodes);

    quint32* data = new quint32(0);

    for (int i = 0; i < 32; ++i) {
        CrBooleanNode* outNode = new CrBooleanNode();
        outNode->setName(tr("DO %1").arg((i + 1)));
        outNode->setFlags(CrNode::ActiveEditor | CrNode::In);
        doNodes->appendChildNode(outNode);
        connect(outNode, &CrBooleanNode::valueChanged, request, [request, outNode, data, i]{

            if (outNode->value())
                (*data)|=(1<<i);
            else
                (*data)&=~(1<<i);

            quint32 swData = qbswap(*data);
            request->setData(QByteArray(reinterpret_cast<char*>(&swData), 4));

        });
    }

    connect(node, &QObject::destroyed, node, [data]{ delete data; });

    CrIntegerLimitedNode* serverAddressNode = new CrIntegerLimitedNode();
    serverAddressNode->setName(tr("Server address"));
    serverAddressNode->setFlags(CrNode::ActiveEditor);
    serverAddressNode->setRange(0, 255);
    serverAddressNode->setIntValue(request->serverAddress());

    connect(serverAddressNode, &CrNumericNode::valueChanged, request, [request, serverAddressNode](){
        request->setServerAddress(static_cast<quint8>(serverAddressNode->intValue()));
    }, Qt::BlockingQueuedConnection);
    node->appendChildNode(serverAddressNode);

    CrIntegerLimitedNode* timeoutNode = new CrIntegerLimitedNode();
    timeoutNode->setName(tr("Timeout"));
    timeoutNode->setRange(0, 10000);
    timeoutNode->setIntValue(request->timeout());
    timeoutNode->setFlags(CrNode::ActiveEditor);

    connect(timeoutNode, &CrNumericNode::valueChanged, request, [request, timeoutNode]{
        request->setTimeout(timeoutNode->intValue());
    }, Qt::BlockingQueuedConnection);
    node->appendChildNode(timeoutNode);


    CrIntegerLimitedNode* retriesNode = new CrIntegerLimitedNode();
    retriesNode->setName(tr("Retries"));
    retriesNode->setRange(0, 100);
    retriesNode->setIntValue(request->retries());
    retriesNode->setFlags(CrNode::ActiveEditor);

    connect(retriesNode, &CrNumericNode::valueChanged, request, [request, retriesNode]{
        request->setRetries(retriesNode->intValue());
    }, Qt::BlockingQueuedConnection);
    node->appendChildNode(retriesNode);

    CrBooleanNode* enableNode = new CrBooleanNode();
    enableNode->setName(tr("Enable"));
    enableNode->setFlags(CrNode::ActiveEditor);

    connect(enableNode, &CrBooleanNode::valueChanged, request, [request, enableNode, node]{
        if(!node->client())
            return;
        if(enableNode->value()){
            node->client()->addRequest(request);
        }else {
            node->client()->removeRequest(request);
        }

    }, Qt::BlockingQueuedConnection);
    node->appendChildNode(enableNode);

    connect(node, &CrModbusClientNode::clientChanged, request, [oldClient = node->client(), request, enableNode, node] () mutable {
        if(!enableNode->value()){
            oldClient = node->client();
            return;
        }
        if(oldClient){
            oldClient->removeRequest(request);
        }
        if(node->client()){
            node->client()->addRequest(request);
        }
        oldClient = node->client();
    }, Qt::BlockingQueuedConnection);

    connect(node, &CrNode::inputNodeChanged, node, [node] () mutable {
        if(!node->inputNode()){
            node->setClient(nullptr);
        }
    });

    return node;
}

CrNode *OwenPlugin::create_MV_110_32(const CrModule *module)
{
    CrModbusClientNode* node = new CrModbusClientNode();
    node->setName(tr("MV 110 - 32"));
    node->setComments(tr("Discrete input module"));
    node->setFlags(CrNode::EditableName | CrNode::In);

    CrModbusRequest* request = new CrModbusRequest();
    request->setFunctionCode(CrModbusRequest::ReadHoldingRegisters);
    request->setSizeData(2);
    request->setStartAddress(99);
    request->moveToThread(CrSerialPortNode::serialPortThread());

    connect(node, &QObject::destroyed, request, &QObject::deleteLater);

    connect(request, &CrModbusRequest::errorOccurred, node, [node, request](){
        node->pushErrorMessage(tr("Modbus request error."), request->errorString());
    });

    CrNode* diNodes = new CrNode();
    diNodes->setName(tr("DI"));
    diNodes->setComments(tr("Discrete inputs"));
    node->appendChildNode(diNodes);

    QList<CrBooleanNode*> inNodes;
    for (int i = 0; i < 32; ++i) {
        CrBooleanNode* inNode = new CrBooleanNode();
        inNode->setName(tr("DI %1").arg((i + 1)));
        inNode->setFlags(CrNode::Out);
        diNodes->appendChildNode(inNode);
        inNodes << inNode;
    }

    connect(request, &CrModbusRequest::dataChanged, diNodes, [inNodes, request]{
        quint32 data;
        std::memcpy(&data, request->data().constData(), 4);
        data = qbswap(data);
        for (int i = 0; i < 32; ++i) {
            inNodes.at(i)->setValue((data>>i)&1);
        }
    });

    CrIntegerLimitedNode* serverAddressNode = new CrIntegerLimitedNode();
    serverAddressNode->setName(tr("Server address"));
    serverAddressNode->setFlags(CrNode::ActiveEditor);
    serverAddressNode->setRange(0, 255);
    serverAddressNode->setIntValue(request->serverAddress());

    connect(serverAddressNode, &CrNumericNode::valueChanged, request, [request, serverAddressNode](){
        request->setServerAddress(static_cast<quint8>(serverAddressNode->intValue()));
    }, Qt::BlockingQueuedConnection);
    node->appendChildNode(serverAddressNode);

    CrRealLimitedNode* frequencyNode = new CrRealLimitedNode();
    frequencyNode->setName(tr("Frequency"));
    frequencyNode->setIntValue(0);
    frequencyNode->setRange(0, 100);
    frequencyNode->setSuffix(tr("Hz"));
    frequencyNode->setFlags(CrNode::ActiveEditor);

    connect(frequencyNode, &CrNumericNode::valueChanged, request, [request, frequencyNode]{
        if(frequencyNode->realValue() > 0){
            request->setInterval(1000 / frequencyNode->realValue());
        }else{
            request->setInterval(0);
        }
    }, Qt::BlockingQueuedConnection);
    node->appendChildNode(frequencyNode);

    CrIntegerLimitedNode* timeoutNode = new CrIntegerLimitedNode();
    timeoutNode->setName(tr("Timeout"));
    timeoutNode->setRange(0, 10000);
    timeoutNode->setIntValue(request->timeout());
    timeoutNode->setFlags(CrNode::ActiveEditor);

    connect(timeoutNode, &CrNumericNode::valueChanged, request, [request, timeoutNode]{
        request->setTimeout(timeoutNode->intValue());
    }, Qt::BlockingQueuedConnection);
    node->appendChildNode(timeoutNode);


    CrIntegerLimitedNode* retriesNode = new CrIntegerLimitedNode();
    retriesNode->setName(tr("Retries"));
    retriesNode->setRange(0, 100);
    retriesNode->setIntValue(request->retries());
    retriesNode->setFlags(CrNode::ActiveEditor);

    connect(retriesNode, &CrNumericNode::valueChanged, request, [request, retriesNode]{
        request->setRetries(retriesNode->intValue());
    }, Qt::BlockingQueuedConnection);
    node->appendChildNode(retriesNode);

    CrBooleanNode* enableNode = new CrBooleanNode();
    enableNode->setName(tr("Enable"));
    enableNode->setFlags(CrNode::ActiveEditor);

    connect(enableNode, &CrBooleanNode::valueChanged, request, [request, enableNode, node]{
        if(!node->client())
            return;
        if(enableNode->value()){
            node->client()->addRequest(request);
        }else {
            node->client()->removeRequest(request);
        }

    }, Qt::BlockingQueuedConnection);
    node->appendChildNode(enableNode);

    connect(node, &CrModbusClientNode::clientChanged, request, [oldClient = node->client(), request, enableNode, node] () mutable {
        if(!enableNode->value()){
            oldClient = node->client();
            return;
        }
        if(oldClient){
            oldClient->removeRequest(request);
        }
        if(node->client()){
            node->client()->addRequest(request);
        }
        oldClient = node->client();
    }, Qt::BlockingQueuedConnection);

    connect(node, &CrNode::inputNodeChanged, node, [node] () mutable {
        if(!node->inputNode()){
            node->setClient(nullptr);
        }
    });

    return node;
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(CrModulePlugin, CrQmlPlugin);
#endif // QT_VERSION < 0x050000
