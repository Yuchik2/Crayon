#include "crbytearraynode.h"

#include <QLineEdit>
#include <QRegExpValidator>
#include <QUndoCommand>
#include <QStyleOptionViewItem>
#include <QStringBuilder>

#include <crproject.h>

CrByteArrayNode::CrByteArrayNode(CrNode *parent) : CrNode (parent) {}


CrByteArrayNode::~CrByteArrayNode() {}

QByteArray CrByteArrayNode::bytes() const
{
    return bytes_;
}

void CrByteArrayNode::setBytes(const QByteArray &bytes)
{
    if(bytes_ == bytes)
        return;

    bytes_ = bytes;
    emit bytesChanged();
    emit updateEditorPreview();
}

bool CrByteArrayNode::isValidInputNode(CrNode *inputNode) const
{
    return static_cast<bool>(qobject_cast<CrByteArrayNode*>(inputNode));
}

bool CrByteArrayNode::isValidOunputNode(CrNode *outputNode) const
{
    return static_cast<bool>(qobject_cast<CrByteArrayNode*>(outputNode));
}

QWidget *CrByteArrayNode::createCompactWidgetEditor()
{
    class ChangeBytesCommand : public QUndoCommand
    {
    public:
        ChangeBytesCommand(CrByteArrayNode* node, const QByteArray& bytes , QUndoCommand *parent = nullptr) :
            QUndoCommand(parent), node_(node), newBytes_(bytes){
            oldBytes_ = node_->bytes();
            setText(QString(QObject::tr("Node %1 change value %2 to %3.")).
                    arg(node_->name()).
                    arg(QString::fromLatin1(oldBytes_.toHex())).
                    arg(QString::fromLatin1(newBytes_.toHex())));
        }
        virtual void undo(){
            node_->setBytes(oldBytes_);
        }
        virtual void redo(){
            node_->setBytes(newBytes_);
        }
    private:
        CrByteArrayNode* node_;
        QByteArray newBytes_;
        QByteArray oldBytes_;
    };

    static QRegExpValidator* validator =  new QRegExpValidator(QRegExp("[0-9, a-f, A-F]"));

    auto lineEdit = new QLineEdit();
    lineEdit->setText(bytes_.toHex());
    lineEdit->setValidator(validator);

    connect(lineEdit, &QLineEdit::editingFinished, this, [this, lineEdit](){
        QByteArray bytes = QByteArray::fromHex(lineEdit->text().toLatin1());
        if(bytes_ != bytes){
            if(project()){
                project()->pushUndoCommand(new ChangeBytesCommand(this, bytes));
            }else {
                setBytes(bytes);
            }
        }
    });

    return lineEdit;
}

QSize CrByteArrayNode::sizeHintEditorPreview(const QStyleOptionViewItem &option) const
{
    QStyleOptionViewItem opt(option);
    opt.text = "0x" % bytes_.toHex();
    return CrNode::sizeHintEditorPreview(opt);
}

void CrByteArrayNode::paintEditorPreview(QPainter *painter, const QStyleOptionViewItem &option) const
{
    QStyleOptionViewItem opt(option);
    opt.text = "0x" % bytes_.toHex();
    CrNode::paintEditorPreview(painter, opt);
}

QByteArray CrByteArrayNode::saveState() const
{
    QByteArray state;
    QDataStream out(&state, QIODevice::WriteOnly);
    out << CrNode::saveState();
    out << bytes_;
    return state;
}

bool CrByteArrayNode::restoreState(const QByteArray &state)
{
    QDataStream in(state);
    QByteArray nodeState;
    in >> nodeState;
    CrNode::restoreState(nodeState);
    QByteArray bytes;
    in >> bytes;
    setBytes(bytes);
    return true;
}

void CrByteArrayNode::changeInputNode(CrNode *newNode)
{
    if(inputNode()){
        CrByteArrayNode* numNode = static_cast<CrByteArrayNode*>(inputNode());
        disconnect(numNode, &CrByteArrayNode::bytesChanged, this, &CrByteArrayNode::changeInputNodeValue);
    }

    if(newNode){
        CrByteArrayNode* numNode = static_cast<CrByteArrayNode*>(newNode);
        connect(numNode, &CrByteArrayNode::bytesChanged, this, &CrByteArrayNode::changeInputNodeValue);
        setBytes(static_cast<CrByteArrayNode*>(newNode)->bytes_);
    }

}

void CrByteArrayNode::changeInputNodeValue()
{
    setBytes(static_cast<CrByteArrayNode*>(inputNode())->bytes_);
}
