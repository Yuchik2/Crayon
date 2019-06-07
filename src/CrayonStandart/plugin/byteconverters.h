#ifndef BYTECONVERTERS_H
#define BYTECONVERTERS_H

#include <crnumericnode.h>
#include <crbooleannode.h>
#include <crbytearraynode.h>
#include <crcomboboxnode.h>

#include <QtEndian>

#include <QDebug>

namespace ByteConverters {

//
//Byte to bits
//
CrNode* byteToBits(const CrModule* module){
    CrIntegerLimitedNode* node = new CrIntegerLimitedNode();


    node->setName(QObject::tr("Byte to bits"));
    node->setComments(QObject::tr("Byte to bits"));
    node->setFlags(CrNode::EditableName | CrNode::ActiveEditor | CrNode::In);
    node->setRange(0, 255);

    QList<CrBooleanNode*> bitNodes;
    for (int i = 0; i < 8; ++i) {
        CrBooleanNode* bitNode = new CrBooleanNode(node);
        bitNode->setName(QString(QObject::tr("Bit %1")).arg(i+1));
        bitNode->setFlags(CrNode::Out);
        bitNodes.append(bitNode);
    }
    QObject::connect(node, &CrNumericNode::valueChanged, node, [node, bitNodes](){
        uchar val = static_cast<uchar>(node->intValue());
        for (int i = 0; i < 8; ++i) {
            bitNodes.at(i)->setValue((val>>i)&1);
        }
    });
    return node;
}

//
//Bits to byte
//
CrNode* bitsToByte(const CrModule* module){
    CrIntegerLimitedNode* node = new CrIntegerLimitedNode();
    node->setName(QObject::tr("Bits to byte"));
    node->setComments(QObject::tr("Byte to bits"));
    node->setFlags(CrNode::EditableName | CrNode::Out);
    node->setRange(0, 255);

    for (int i = 0; i < 8; ++i) {
        CrBooleanNode* bitNode = new CrBooleanNode(node);
        bitNode->setName(QString(QObject::tr("Bit %1")).arg(i+1));
        bitNode->setFlags(CrNode::ActiveEditor | CrNode::In);
        QObject::connect(bitNode, &CrBooleanNode::valueChanged, node, [node, bitNode, i]{
            uchar val =  static_cast<uchar>(node->intValue());
            if (bitNode->value())
                val|=(1<<i);
            else
                val&=~(1<<i);
            node->setIntValue(val);
        });
    }

    return node;
}

//
//Byte arrat to bytes
//
CrNode* byteArrayToBytes(const CrModule* module){
    CrByteArrayNode* node = new CrByteArrayNode();
    node->setName(QObject::tr("Byte array to bytes"));
    node->setComments(QObject::tr("Byte array to bytes"));
    node->setFlags(CrNode::EditableName | CrNode::ActiveEditor | CrNode::In);

    CrIntegerLimitedNode* sizeNode = new CrIntegerLimitedNode(node);
    sizeNode->setName(QObject::tr("Size"));
    sizeNode->setFlags(CrNode::ActiveEditor);
    sizeNode->setRange(0, 255);

    int* size = new int(0);
    QList<CrIntegerLimitedNode*>* byteNodes = new QList<CrIntegerLimitedNode*>();
    auto calculate = [node, size, byteNodes]{
        QByteArray array = node->bytes().leftJustified(*size, 0x00, true);
        for (int i = 0; i < *size; ++i) {
            byteNodes->at(i)->setIntValue(array[i]);
        }
    };

    QObject::connect(sizeNode, &CrIntegerLimitedNode::valueChanged, node, [node, sizeNode, size, byteNodes, calculate](){
        *size = sizeNode->intValue();

        if(byteNodes->size() < *size){
            for (int i = byteNodes->size(); i < *size; ++i) {
                CrIntegerLimitedNode* byteNode = new CrIntegerLimitedNode();
                byteNode->setName(QObject::tr("Byte ")+ QString::number(i));
                byteNode->setFlags(CrNode::Out);
                byteNode->setRange(0, 255);
                byteNodes->append(byteNode);
            }
        }

        while(node->childNodes().size() - 1  > *size){
            node->removeChildNode(node->childNodes().last());
        }

        while(node->childNodes().size() - 1  < *size){
            node->appendChildNode(byteNodes->at(node->childNodes().size() - 1));
        }
        calculate();
    });

    sizeNode->setIntValue(2);
    QObject::connect(node, &CrByteArrayNode::bytesChanged, node, calculate);
    QObject::connect(node, &QObject::destroyed, node, [size, byteNodes]{
        delete size;
        delete byteNodes;
    });
    return node;

}

//
//Bytes to byte array
//
CrNode* bytesToByteArray(const CrModule* module){

    CrByteArrayNode* node = new CrByteArrayNode();
    node->setName(QObject::tr("Bytes to byte array"));
    node->setComments(QObject::tr("Bytes to byte array"));
    node->setFlags(CrNode::EditableName | CrNode::Out);

    CrIntegerLimitedNode* sizeNode = new CrIntegerLimitedNode(node);
    sizeNode->setName(QObject::tr("Size"));
    sizeNode->setFlags(CrNode::ActiveEditor);
    sizeNode->setRange(0, 255);

    int* size = new int(0);
    QList<CrIntegerLimitedNode*>* byteNodes = new QList<CrIntegerLimitedNode*>();
    auto calculate = [node, size, byteNodes]{
        QVarLengthArray<quint8, 16> data;
        for (int i = 0; i < *size; ++i) {
            data.append(static_cast<quint8>(byteNodes->at(i)->intValue()));
        }
        node->setBytes(QByteArray(reinterpret_cast<const char*>(data.constData()), *size));
    };

    QObject::connect(sizeNode, &CrIntegerLimitedNode::valueChanged, node, [node, sizeNode, size, byteNodes, calculate](){

        *size = sizeNode->intValue();

        if(byteNodes->size() < *size){
            for (int i = byteNodes->size(); i < *size; ++i) {
                CrIntegerLimitedNode* byteNode = new CrIntegerLimitedNode();
                byteNode->setName(QObject::tr("Byte ")+ QString::number(i + 1));
                byteNode->setFlags(CrNode::ActiveEditor | CrNode::In);
                byteNode->setRange(0, 255);
                byteNodes->append(byteNode);
                QObject::connect(byteNode, &CrNumericNode::valueChanged, node, calculate);
            }
        }

        while(node->childNodes().size() - 1  > *size){
            node->removeChildNode(node->childNodes().last());
        }

        while(node->childNodes().size() - 1  < *size){
            node->appendChildNode(byteNodes->at(node->childNodes().size() - 1));
        }
        calculate();
    });

    sizeNode->setIntValue(2);
    QObject::connect(node, &QObject::destroyed, node, [size, byteNodes]{
        delete size;
        delete byteNodes;
    });
    return node;

}

//
//Number to byte array
//
CrNode* numberToByteArray(const CrModule* module){

    CrRealNode* node = new CrRealNode();
    node->setName(QObject::tr("Number to byte array"));
    node->setComments(QObject::tr("Number to byte array"));
    node->setFlags(CrNode::EditableName | CrNode::ActiveEditor | CrNode::In);

    CrComboBoxNode* typeNode = new CrComboBoxNode(node);
    typeNode->setName(QObject::tr("Type"));
    typeNode->setFlags(CrNode::ActiveEditor);
    typeNode->addItem(QStringLiteral("Int 8"), static_cast<int>(QMetaType::Char));
    typeNode->addItem(QStringLiteral("Int 16"), static_cast<int>(QMetaType::Short));
    typeNode->addItem(QStringLiteral("Int 32"), static_cast<int>(QMetaType::Int));
    typeNode->addItem(QStringLiteral("Int 64"), static_cast<int>(QMetaType::LongLong));
    typeNode->addItem(QStringLiteral("UInt 8"), static_cast<int>(QMetaType::UChar));
    typeNode->addItem(QStringLiteral("UInt 16"), static_cast<int>(QMetaType::UShort));
    typeNode->addItem(QStringLiteral("UInt 32"), static_cast<int>(QMetaType::UInt));
    typeNode->addItem(QStringLiteral("UInt 64"), static_cast<int>(QMetaType::ULongLong));
    typeNode->addItem(QStringLiteral("Real 32"), static_cast<int>(QMetaType::Float));
    typeNode->addItem(QStringLiteral("Real 64"), static_cast<int>(QMetaType::Double));
    typeNode->setCurrentIndex(2);

    CrComboBoxNode* endianNode = new CrComboBoxNode(node);
    endianNode->setName(QObject::tr("Endian"));
    endianNode->setFlags(CrNode::ActiveEditor);
    endianNode->addItem(QObject::tr("Big endian"), true);
    endianNode->addItem(QObject::tr("Little endian"), false);

    CrByteArrayNode* resultNode = new CrByteArrayNode(node);
    resultNode->setName(QObject::tr("Result"));
    resultNode->setFlags(CrNode::Out);

    auto calculate = [node, typeNode,  endianNode, resultNode](){
        QByteArray bytes;
        switch (typeNode->currentData().toInt()) {
        case QMetaType::Char:{
            char val = node->intValue();
            resultNode->setBytes(QByteArray(&val, sizeof(val)));
            return;
        }
        case QMetaType::Short:{
            qint16 val = endianNode->currentIndex() ?
                        qToBigEndian(static_cast<qint16>(node->intValue())) :
                        qToLittleEndian(static_cast<qint16>(node->intValue()));
            char* pval = reinterpret_cast<char*>(&val);
            resultNode->setBytes(QByteArray(pval, sizeof(val)));
            return;
        }
        case QMetaType::Int:{
            qint32 val = endianNode->currentIndex() ?
                        qToBigEndian(static_cast<qint32>(node->intValue())) :
                        qToLittleEndian(static_cast<qint32>(node->intValue()));
            char* pval = reinterpret_cast<char*>(&val);
            resultNode->setBytes(QByteArray(pval, sizeof(val)));
            return;
        }
        case QMetaType::LongLong:{
            qint64 val = endianNode->currentIndex() ?
                        qToBigEndian(static_cast<qint64>(node->intValue())) :
                        qToLittleEndian(static_cast<qint64>(node->intValue()));
            char* pval = reinterpret_cast<char*>(&val);
            resultNode->setBytes(QByteArray(pval, sizeof(val)));
            return;
        }
        case QMetaType::UChar:{
            uchar val = node->intValue();
            char* pval = reinterpret_cast<char*>(&val);
            resultNode->setBytes(QByteArray(pval, sizeof(val)));
            return;
        }
        case QMetaType::UShort:{
            quint16 val = endianNode->currentIndex() ?
                        qToBigEndian(static_cast<quint16>(node->intValue())) :
                        qToLittleEndian(static_cast<quint16>(node->intValue()));
            char* pval = reinterpret_cast<char*>(&val);
            resultNode->setBytes(QByteArray(pval, sizeof(val)));
            return;
        }
        case QMetaType::UInt:{
            quint32 val = endianNode->currentIndex() ?
                        qToBigEndian(static_cast<quint32>(node->intValue())) :
                        qToLittleEndian(static_cast<quint32>(node->intValue()));
            char* pval = reinterpret_cast<char*>(&val);
            resultNode->setBytes(QByteArray(pval, sizeof(val)));
            return;
        }
        case QMetaType::ULongLong:{
            quint64 val = endianNode->currentIndex() ?
                        qToBigEndian(static_cast<quint64>(node->intValue())) :
                        qToLittleEndian(static_cast<quint64>(node->intValue()));
            char* pval = reinterpret_cast<char*>(&val);
            resultNode->setBytes(QByteArray(pval, sizeof(val)));
            return;
        }
        case QMetaType::Float:{
            union {
                float f;
                quint32 i;
            } x;
            x.f = node->realValue();
            quint32 val = endianNode->currentIndex() ? qToBigEndian(x.i) :  qToLittleEndian(x.i);
            char* pval = reinterpret_cast<char*>(&val);
            resultNode->setBytes(QByteArray(pval, sizeof(val)));
            return;
        }
        case QMetaType::Double:{
            union {
                double f;
                quint64 i;
            } x;
            x.f = node->realValue();
            quint64 val = endianNode->currentIndex() ? qToBigEndian(x.i) :  qToLittleEndian(x.i);
            char* pval = reinterpret_cast<char*>(&val);
            resultNode->setBytes(QByteArray(pval, sizeof(val)));
            return;
        }
        default:

            break;
        };
    };

    QObject::connect(node, &CrNumericNode::valueChanged, node, calculate);
    QObject::connect(typeNode, &CrComboBoxNode::currentIndexChanged, node, calculate);
    QObject::connect(endianNode, &CrComboBoxNode::currentIndexChanged, node, calculate);
    return node;
}

//
//Byte array to number
//
CrNode* byteArrayToNumber(const CrModule* module){

    CrByteArrayNode* node = new CrByteArrayNode();
    node->setName(QObject::tr("Byte array to number"));
    node->setComments(QObject::tr("Byte array to number"));
    node->setFlags(CrNode::EditableName | CrNode::ActiveEditor | CrNode::In);

    CrComboBoxNode* typeNode = new CrComboBoxNode(node);
    typeNode->setName(QObject::tr("Type"));
    typeNode->setFlags(CrNode::ActiveEditor);
    typeNode->addItem(QStringLiteral("Int 8"), static_cast<int>(QMetaType::Char));
    typeNode->addItem(QStringLiteral("Int 16"), static_cast<int>(QMetaType::Short));
    typeNode->addItem(QStringLiteral("Int 32"), static_cast<int>(QMetaType::Int));
    typeNode->addItem(QStringLiteral("Int 64"), static_cast<int>(QMetaType::LongLong));
    typeNode->addItem(QStringLiteral("UInt 8"), static_cast<int>(QMetaType::UChar));
    typeNode->addItem(QStringLiteral("UInt 16"), static_cast<int>(QMetaType::UShort));
    typeNode->addItem(QStringLiteral("UInt 32"), static_cast<int>(QMetaType::UInt));
    typeNode->addItem(QStringLiteral("UInt 64"), static_cast<int>(QMetaType::ULongLong));
    typeNode->addItem(QStringLiteral("Real 32"), static_cast<int>(QMetaType::Float));
    typeNode->addItem(QStringLiteral("Real 64"), static_cast<int>(QMetaType::Double));
    typeNode->setCurrentIndex(2);

    CrComboBoxNode* endianNode = new CrComboBoxNode(node);
    endianNode->setName(QObject::tr("Endian"));
    endianNode->setFlags(CrNode::ActiveEditor);
    endianNode->addItem(QObject::tr("Big endian"), true);
    endianNode->addItem(QObject::tr("Little endian"), false);

    CrRealNode* resultNode = new CrRealNode(node);
    resultNode->setName(QObject::tr("Result"));
    resultNode->setFlags(CrNode::Out);

    auto calculate = [node, typeNode,  endianNode, resultNode](){
        QByteArray array = node->bytes().leftJustified(8, 0x00, true);

        qreal v;
        switch (typeNode->currentData().toInt()) {
        case QMetaType::Char:{

            v = static_cast<const qint8 *>(reinterpret_cast<const void *>(array.constData()))[0];
            break;
        }
        case QMetaType::Short:{
            v = endianNode->currentIndex() ? qFromBigEndian<qint16>(array.constData()) :  qFromLittleEndian<qint16>(array.constData());
            break;
        }
        case QMetaType::Int:{
            v = endianNode->currentIndex() ? qFromBigEndian<qint32>(array.constData()) :  qFromLittleEndian<qint32>(array.constData());
            break;
        }
        case QMetaType::LongLong:{
            v = endianNode->currentIndex() ? qFromBigEndian<qint64>(array.constData()) :  qFromLittleEndian<qint64>(array.constData());
            break;
        }
        case QMetaType::UChar:{
            v = static_cast<const quint8 *>(reinterpret_cast<const void *>(array.constData()))[0];
            break;
        }
        case QMetaType::UShort:{
            v = endianNode->currentIndex() ? qFromBigEndian<quint16>(array.constData()) :  qFromLittleEndian<quint16>(array.constData());
            break;
        }
        case QMetaType::UInt:{
            v = endianNode->currentIndex() ? qFromBigEndian<quint32>(array.constData()) :  qFromLittleEndian<quint32>(array.constData());
            break;
        }
        case QMetaType::ULongLong:{
            v = endianNode->currentIndex() ? qFromBigEndian<quint64>(array.constData()) :  qFromLittleEndian<quint64>(array.constData());
            break;
        }
        case QMetaType::Float:{
            union {
                float f;
                quint32 i;
            } x;
            x.i = endianNode->currentIndex() ? qFromBigEndian<quint32>(array.constData()) :  qFromLittleEndian<quint32>(array.constData());
            v = x.f;
            break;
        }
        case QMetaType::Double:{
            union {
                double f;
                quint64 i;
            } x;
            x.i = endianNode->currentIndex() ? qFromBigEndian<quint64>(array.constData()) :  qFromLittleEndian<quint64>(array.constData());
            v = x.f;
            break;
        }
        default:
            v = 0.0;
            break;
        };
        resultNode->setRealValue(v);
    };

    QObject::connect(node, &CrByteArrayNode::bytesChanged, node, calculate);
    QObject::connect(typeNode, &CrComboBoxNode::currentIndexChanged, node, calculate);
    QObject::connect(endianNode, &CrComboBoxNode::currentIndexChanged, node, calculate);

    return node;
}


}
#endif // BYTECONVERTERS_H
