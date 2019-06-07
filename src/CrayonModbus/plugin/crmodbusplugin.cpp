#include "crmodbusplugin.h"

#include <QApplication>
#include <QSerialPort>
#include <QThread>
#include <QTimer>

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

#include <QDebug>

CrModbusPlugin::CrModbusPlugin(QObject *parent) : QObject(parent)
{

}

CrModbusPlugin::~CrModbusPlugin()
{

}

void CrModbusPlugin::install(CrPlugin *plugin)
{
    qRegisterMetaType<CrModbusRequest::ErrorCode>("ErrorCode");
    qRegisterMetaType<CrModbusRequest::FunctionCode>();

    auto rtuModbusClientNodeFactory = [](const CrModule* module)->CrNode*  {

            CrModbusClientNode* node = new CrModbusClientNode();
            node->setName(tr("Modbus RTU client"));
            node->setComments(tr("Modbus RTU client"));
            node->setFlags(CrNode::EditableName | CrNode::Out);

            CrModbusRtuClient* client = new CrModbusRtuClient();
            client->moveToThread(CrSerialPortNode::serialPortThread());
            connect(node, &QObject::destroyed, client, &QObject::deleteLater);

            QObject::connect(client, QOverload<const QString& >::of(&CrModbusRtuClient::errorOccurred), node, [node](const QString& error){
        node->pushWarningMessage(tr("Modbus RTU client error."), error);

    }, Qt::BlockingQueuedConnection);
            node->setClient(client);

            CrSerialPortNode* serialPortNode = new CrSerialPortNode();
            serialPortNode->setName(tr("Serial port"));
            serialPortNode->setFlags(CrNode::In);

            node->appendChildNode(serialPortNode);

            CrIntegerLimitedNode* delayOfFrameNode = new CrIntegerLimitedNode();
            delayOfFrameNode->setName(tr("Delay of frame"));
            delayOfFrameNode->setFlags(CrNode::ActiveEditor);
            delayOfFrameNode->setRange(0, 1000);
            delayOfFrameNode->setIntValue(15);

            connect(delayOfFrameNode, &CrIntegerNode::valueChanged, client, [client, delayOfFrameNode](){
        client->setDelayOfFrame(delayOfFrameNode->intValue());
    }, Qt::BlockingQueuedConnection);
            node->appendChildNode(delayOfFrameNode);

            CrBooleanNode* enableNode = new CrBooleanNode;
            enableNode->setName(tr("Enable"));
            enableNode->setFlags(CrNode::ActiveEditor);

            connect(enableNode, &CrBooleanNode::valueChanged, client, [client, enableNode, serialPortNode](){
        if(enableNode->value()){
            if(serialPortNode->serialPort()){
                client->open();
            }
        } else {
            client->close();
        }
    }, Qt::BlockingQueuedConnection);


            connect(serialPortNode, &CrSerialPortNode::serialPortChanged, client, [client, serialPortNode, enableNode]{
        client->close();
        client->setSerialPort(serialPortNode->serialPort());
        if(serialPortNode->serialPort()){
            if(enableNode->value())
                client->open();
        }
    }, Qt::BlockingQueuedConnection);

            connect(serialPortNode, &CrNode::inputNodeChanged, serialPortNode, [serialPortNode]{

        if(!serialPortNode->inputNode()){
            serialPortNode->setSerialPort(nullptr);
        }

    });

            node->appendChildNode(enableNode);
            return node;
};

    CrModule* rtuClientModule = new CrModule(plugin, "modbus/rtuclient", QVersionNumber(1,0,0), rtuModbusClientNodeFactory);
    rtuClientModule->setPath(tr("Modbus/RTU Client"));
    rtuClientModule->setDoc(tr("RTU Client"));
    rtuClientModule->setIcon(QIcon(":/CrayonModbusPluginResources/modbus_rtu_icon.png"));



    auto modbusRequestNodeFactory = [](const CrModule* module)->CrNode*  {

            CrModbusClientNode* node = new CrModbusClientNode();
            node->setName(tr("Modbus request"));
            node->setComments(tr("Modbus request"));
            node->setFlags(CrNode::EditableName | CrNode::In);

            CrModbusRequest* request = new CrModbusRequest();
            request->moveToThread(CrSerialPortNode::serialPortThread());
            connect(node, &QObject::destroyed, request, &QObject::deleteLater);

            connect(request, &CrModbusRequest::errorOccurred, node, [node, request](){
        node->pushErrorMessage(tr("Modbus request error."), request->errorString());
    });

            CrByteArrayNode* dataNode = new CrByteArrayNode();
            dataNode->setName(tr("Data"));
            dataNode->setFlags(CrNode::Out | CrNode::EditableName);
            connect(request, QOverload<const QByteArray&>::of(&CrModbusRequest::dataChanged),
                    dataNode, [dataNode](const QByteArray& data){
        dataNode->setBytes(data);
    });

            node->appendChildNode(dataNode);

            CrIntegerLimitedNode* serverAddressNode = new CrIntegerLimitedNode();
            serverAddressNode->setName(tr("Server address"));
            serverAddressNode->setFlags(CrNode::ActiveEditor);
            serverAddressNode->setRange(0, 255);
            serverAddressNode->setIntValue(request->serverAddress());

            connect(serverAddressNode, &CrNumericNode::valueChanged, request, [request, serverAddressNode](){
        request->setServerAddress(static_cast<quint8>(serverAddressNode->intValue()));
    }, Qt::BlockingQueuedConnection);
            node->appendChildNode(serverAddressNode);

            CrComboBoxNode* functionCodeNode = new CrComboBoxNode();
            functionCodeNode->setName(tr("Function code"));
            functionCodeNode->setFlags(CrNode::ActiveEditor);
            functionCodeNode->addItem("(0x01) Read coils",               CrModbusRequest::ReadCoils);
            functionCodeNode->addItem("(0x02) Read discrete inputs",     CrModbusRequest::ReadDiscreteInputs);
            functionCodeNode->addItem("(0x03) Read holding registers",   CrModbusRequest::ReadHoldingRegisters);
            functionCodeNode->addItem("(0x04) Read input registers",     CrModbusRequest::ReadInputRegisters);
            functionCodeNode->addItem("(0x05) Write single coil",        CrModbusRequest::WriteSingleCoil);
            functionCodeNode->addItem("(0x06) Write single register",    CrModbusRequest::WriteSingleRegister);
            functionCodeNode->addItem("(0x0F) Write multiple coils",     CrModbusRequest::WriteMultipleCoils);
            functionCodeNode->addItem("(0x10) Write multiple register",  CrModbusRequest::WriteMultipleRegisters);

            connect(functionCodeNode, &CrComboBoxNode::currentIndexChanged, dataNode, [request, functionCodeNode, dataNode](){
        auto code = functionCodeNode->currentData().value<CrModbusRequest::FunctionCode>();
        request->setFunctionCode(code);

        switch (code) {
        case CrModbusRequest::ReadCoils:
        case CrModbusRequest::ReadDiscreteInputs:
        case CrModbusRequest::ReadHoldingRegisters:
        case CrModbusRequest::ReadInputRegisters:{
            dataNode->setFlags(CrNode::Out | CrNode::EditableName);
            disconnect(dataNode, &CrByteArrayNode::bytesChanged, request, nullptr);
            connect(request, QOverload<const QByteArray&>::of(&CrModbusRequest::dataChanged),
                    dataNode, [dataNode](const QByteArray& data){
                dataNode->setBytes(data);
            });
            break;
        }
        default:{
            dataNode->setFlags(CrNode::In | CrNode::ActiveEditor | CrNode::EditableName);
            disconnect(request, &CrModbusRequest::dataChanged, dataNode, nullptr);
            connect(dataNode, &CrByteArrayNode::bytesChanged, request, [request, dataNode](){
                request->setData(dataNode->bytes());
            }, Qt::BlockingQueuedConnection);
            break;
        }
        }
    });
            node->appendChildNode(functionCodeNode);

            CrIntegerLimitedNode* startAddressNode = new CrIntegerLimitedNode();
            startAddressNode->setName(tr("Start address"));
            startAddressNode->setFlags(CrNode::ActiveEditor);
            startAddressNode->setRange(0, USHRT_MAX);

            connect(startAddressNode, &CrNumericNode::valueChanged, request, [request, startAddressNode]{
        request->setStartAddress(static_cast<quint16>(startAddressNode->intValue()));
    }, Qt::BlockingQueuedConnection);
            node->appendChildNode(startAddressNode);


            CrIntegerLimitedNode* sizeDataNode = new CrIntegerLimitedNode();
            sizeDataNode->setName(tr("Size data"));
            sizeDataNode->setFlags(CrNode::ActiveEditor);
            sizeDataNode->setRange(1, USHRT_MAX);
            connect(sizeDataNode, &CrNumericNode::valueChanged, request, [request, sizeDataNode]{
        request->setSizeData(static_cast<quint16>(sizeDataNode->intValue()));
    }, Qt::BlockingQueuedConnection);
            node->appendChildNode(sizeDataNode);


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
            timeoutNode->setSuffix(tr("ms"));
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
        if(!node->inputNode())
            return;
        auto client  = static_cast<CrModbusClientNode*>(node->inputNode())->client();

        if(!client)
            return;
        if(enableNode->value()){
            client->addRequest(request);
        }else {
            client->removeRequest(request);
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
};

    CrModule* clientModule = new CrModule(plugin, "modbus/request", QVersionNumber(1,0,0), modbusRequestNodeFactory);
    clientModule->setPath(tr("Modbus/Request"));
    clientModule->setDoc(tr("Request"));
    clientModule->setIcon(QIcon(":/CrayonModbusPluginResources/modbus_request_icon.png"));

}

void CrModbusPlugin::uninstall(CrPlugin *plugin)
{
    Q_UNUSED(plugin)
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(CrModulePlugin, CrQmlPlugin);
#endif // QT_VERSION < 0x050000
