#include "crstandartplugin.h"

#include <QApplication>
#include <QFont>
#include <QStyleOptionButton>
#include <QVersionNumber>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>

#include <crplugin.h>
#include <crmodule.h>
#include <crproject.h>
#include <CrComboBoxNode>
#include <crnumericnode.h>

#include <crserialportnode.h>
#include "crgraphicsscenenode.h"
#include "byteconverters.h"

#include <QDebug>

CrStandartPlugin::CrStandartPlugin(QObject *parent) : QObject(parent)
{

}

CrStandartPlugin::~CrStandartPlugin()
{

}

void CrStandartPlugin::install(CrPlugin *plugin)
{
    // Install converters

    //    CrModule* clientModule = new CrModule(plugin, "modbus/request", QVersionNumber(1,0,0), modbusRequestNodeFactory);
    //    clientModule->setPath(tr("Modbus/Request"));
    //    clientModule->setDoc(tr("Request"));
    //    clientModule->setIcon(QIcon(":/CrayonModbusPluginResources/modbus_request_icon.png"));

    new CrModule(plugin, "standart/bytetobits",
                 QVersionNumber(1,0,0),
                 ByteConverters::byteToBits,
                 tr("Standart/Converters/Byte => Bits"),
                 tr("Byte to bits"),
                 QIcon(":/Resources/noduleIcons/converters.png"));

    new CrModule(plugin, "standart/bitstobyte",
                 QVersionNumber(1,0,0),
                 ByteConverters::bitsToByte,
                 tr("Standart/Converters/Bits => Byte"),
                 tr("Bits to byte"),
                 QIcon(":/Resources/noduleIcons/converters.png"));

    new CrModule(plugin, "standart/bytearraytobytes",
                 QVersionNumber(1,0,0),
                 ByteConverters::byteArrayToBytes,
                 tr("Standart/Converters/Byte array => Bytes"),
                 tr("Byte array to bytes"),
                 QIcon(":/Resources/noduleIcons/converters.png"));

    new CrModule(plugin, "standart/bytestobytearray",
                 QVersionNumber(1,0,0),
                 ByteConverters::bytesToByteArray,
                 tr("Standart/Converters/Bytes => Byte array"),
                 tr("Bytes to byte array"),
                 QIcon(":/Resources/noduleIcons/converters.png"));

    new CrModule(plugin, "standart/numbertobytearray",
                 QVersionNumber(1,0,0),
                 ByteConverters::numberToByteArray,
                 tr("Standart/Converters/Number => Byte array"),
                 tr("Number to byte array"),
                 QIcon(":/Resources/noduleIcons/converters.png"));

    new CrModule(plugin, "standart/bytearraytonumber",
                 QVersionNumber(1,0,0),
                 ByteConverters::byteArrayToNumber,
                 tr("Standart/Converters/Byte array => Number"),
                 tr("Byte array to number"),
                 QIcon(":/Resources/noduleIcons/converters.png"));


    // Install Hardware

    qRegisterMetaType<QSerialPort::Direction>();
    qRegisterMetaType<QSerialPort::BaudRate>();
    qRegisterMetaType<QSerialPort::DataBits>();
    qRegisterMetaType<QSerialPort::Parity>();
    qRegisterMetaType<QSerialPort::StopBits>();
    qRegisterMetaType<QSerialPort::FlowControl>();
    qRegisterMetaType<QSerialPort::SerialPortError>();

    auto serialPortFactory = [](const CrModule* module)->CrNode* {
        CrSerialPortNode* serialPortNode = new CrSerialPortNode();

        serialPortNode->setName(tr("Serial port"));
        serialPortNode->setComments(tr("Serial port"));
        serialPortNode->setFlags(CrNode::EditableName | CrNode::Out);

        QSerialPort* serialPort = new QSerialPort();

        serialPort->moveToThread(CrSerialPortNode::serialPortThread());
        connect(serialPortNode, &QObject::destroyed, serialPort, &QObject::deleteLater);
        serialPortNode->setSerialPort(serialPort);

        CrComboBoxNode* portNameNode = new CrComboBoxNode();
        portNameNode->setName(tr("Port name"));
        portNameNode->setFlags(CrNode::ActiveEditor);
        foreach (QSerialPortInfo info, QSerialPortInfo::availablePorts()) {
            portNameNode->addItem(info.portName());
        }
        connect(portNameNode, &CrComboBoxNode::currentIndexChanged, serialPort, [serialPort, portNameNode](){
            serialPort->setPortName(portNameNode->currentText());
        });
        serialPortNode->appendChildNode(portNameNode);


        CrComboBoxNode* baudRateNode = new CrComboBoxNode();
        baudRateNode->setName(tr("Baud rate"));
        baudRateNode->setFlags(CrNode::ActiveEditor);
        baudRateNode->addItem(tr("110"),    110);
        baudRateNode->addItem(tr("300"),    300);
        baudRateNode->addItem(tr("600"),    600);
        baudRateNode->addItem(tr("1200"),   1200);
        baudRateNode->addItem(tr("2400"),   2400);
        baudRateNode->addItem(tr("4800"),   4800);
        baudRateNode->addItem(tr("9600"),   9600);
        baudRateNode->addItem(tr("14400"),  14400);
        baudRateNode->addItem(tr("19200"),  19200);
        baudRateNode->addItem(tr("38400"),  38400);
        baudRateNode->addItem(tr("56000"),  56000);
        baudRateNode->addItem(tr("57600"),  57600);
        baudRateNode->addItem(tr("115200"), 115200);
        baudRateNode->addItem(tr("128000"), 128000);
        baudRateNode->addItem(tr("256000"), 256000);

        connect(baudRateNode, &CrComboBoxNode::currentIndexChanged, serialPort, [serialPort, baudRateNode](){
            serialPort->setBaudRate(baudRateNode->currentData().toInt());
        });
        baudRateNode->setCurrentIndex(9);

        serialPortNode->appendChildNode(baudRateNode);

        CrComboBoxNode* dataBitsNode = new CrComboBoxNode();
        dataBitsNode->setName(tr("Data bits"));
        dataBitsNode->setFlags(CrNode::ActiveEditor);
        dataBitsNode->addItem(tr("5 bits"), QSerialPort::Data5);
        dataBitsNode->addItem(tr("6 bits"), QSerialPort::Data6);
        dataBitsNode->addItem(tr("7 bits"), QSerialPort::Data7);
        dataBitsNode->addItem(tr("8 bits"), QSerialPort::Data8);
        dataBitsNode->setCurrentIndex(3);
        connect(dataBitsNode, &CrComboBoxNode::currentIndexChanged, serialPort, [serialPort, dataBitsNode](){
            serialPort->setDataBits(static_cast<QSerialPort::DataBits>(dataBitsNode->currentData().toInt()));
        });

        serialPortNode->appendChildNode(dataBitsNode);

        CrComboBoxNode* parityNode = new CrComboBoxNode();
        parityNode->setName(tr("Parity"));
        parityNode->setFlags(CrNode::ActiveEditor);
        parityNode->addItem(tr("None"),  QSerialPort::NoParity);
        parityNode->addItem(tr("Even"),  QSerialPort::EvenParity);
        parityNode->addItem(tr("Odd"),   QSerialPort::OddParity);
        parityNode->addItem(tr("Space"), QSerialPort::SpaceParity);
        parityNode->addItem(tr("Mark"),  QSerialPort::MarkParity);

        connect(parityNode, &CrComboBoxNode::currentIndexChanged, serialPort, [serialPort, parityNode](){
            serialPort->setParity(static_cast<QSerialPort::Parity>(parityNode->currentData().toInt()));
        });

        serialPortNode->appendChildNode(parityNode);

        CrComboBoxNode* stopBitsNode = new CrComboBoxNode();
        stopBitsNode->setName(tr("Stop bits"));
        stopBitsNode->setFlags(CrNode::ActiveEditor);
        stopBitsNode->addItem(tr("One"),             QSerialPort::OneStop);
        stopBitsNode->addItem(tr("One and half"),    QSerialPort::OneAndHalfStop);
        stopBitsNode->addItem(tr("Two"),             QSerialPort::TwoStop);

        connect(stopBitsNode, &CrComboBoxNode::currentIndexChanged, serialPort, [serialPort, stopBitsNode](){
            serialPort->setStopBits(static_cast<QSerialPort::StopBits>(stopBitsNode->currentData().toInt()));
        });

        serialPortNode->appendChildNode(stopBitsNode);

        CrComboBoxNode* flowControlNode = new CrComboBoxNode();
        flowControlNode->setName(tr("Flow control"));
        flowControlNode->setFlags(CrNode::ActiveEditor);
        flowControlNode->addItem(tr("None"),     QSerialPort::NoFlowControl);
        flowControlNode->addItem(tr("Hardware"), QSerialPort::HardwareControl);
        flowControlNode->addItem(tr("Software"), QSerialPort::SoftwareControl);

        connect(flowControlNode, &CrComboBoxNode::currentIndexChanged, serialPort, [serialPort, flowControlNode](){
            serialPort->setFlowControl(static_cast<QSerialPort::FlowControl>(flowControlNode->currentData().toInt()));
        });

        serialPortNode->appendChildNode(flowControlNode);

        CrRealLimitedNode* sizeBufferNode = new CrRealLimitedNode();
        sizeBufferNode->setName(tr("Read size buffer"));
        sizeBufferNode->setFlags(CrNode::ActiveEditor);
        sizeBufferNode->setRange(0, 4096);

        connect(sizeBufferNode, &CrNumericNode::valueChanged, serialPort, [serialPort, sizeBufferNode](){
            serialPort->setReadBufferSize(sizeBufferNode->intValue());
        });
        serialPortNode->appendChildNode(sizeBufferNode);

        CrBooleanNode* openNode = new CrBooleanNode();
        openNode->setName(tr("Open"));
        openNode->setFlags(CrNode::ActiveEditor);

        connect(openNode, &CrBooleanNode::valueChanged, serialPort, [serialPort, openNode](){
            if(openNode->value()){
                serialPort->open(QIODevice::ReadWrite);
            }else{
                serialPort->close();
            }
        });
        serialPortNode->appendChildNode(openNode);

        connect(serialPort, QOverload<QSerialPort::SerialPortError>::of(&QSerialPort::errorOccurred),
                serialPortNode, [serialPortNode, serialPort, openNode](QSerialPort::SerialPortError error){



            if(error == QSerialPort::NoError)
                return ;

            QString me =tr("Serial port error.");
            serialPortNode->pushWarningMessage(me, serialPort->errorString());

            switch (error) {
            case QSerialPort::PermissionError:
            case QSerialPort::OpenError:
            case QSerialPort::DeviceNotFoundError:
            case QSerialPort::ResourceError:{
                if(openNode->value()){
                    if(serialPort->isOpen()){
                        QTimer::singleShot(0, serialPort, [serialPort]{
                            serialPort->close();
                        });
                    }
                    QTimer::singleShot(2000, serialPort, [serialPort, openNode]{
                        if(openNode->value())
                            serialPort->open(QIODevice::ReadWrite);
                    });
                }
                break;
            }
            default:{

                break;
            }
            }

        });

        return serialPortNode;
    };

    new CrModule(plugin,
                 "standart/serialport",
                 QVersionNumber(1,0,0),
                 serialPortFactory,
                 tr("Standart/Hardware/Serial port"),
                 tr("Serial port"),
                 QIcon(":/Resources/noduleIcons/com.png"));

    // Install Containers

    new CrModule(plugin,
                 "standart/graphicsscene",
                 QVersionNumber(1,0,0),
                 [](const CrModule* module)->CrNode* { return new CrGraphicsSceneNode(); },
    tr("Standart/Containers/Graphisc scene"),
    tr("Graphics scene"),
    QIcon(":/Resources/noduleIcons/GraphicsScene.png"));


}

void CrStandartPlugin::uninstall(CrPlugin *plugin)
{
    Q_UNUSED(plugin);
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(CrModulePlugin, CrStandartPlugin);
#endif // QT_VERSION < 0x050000
