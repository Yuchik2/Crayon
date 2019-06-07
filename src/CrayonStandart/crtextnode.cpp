#include "crtextnode.h"

#include <QLineEdit>
#include <QUndoCommand>
#include <QStyleOptionViewItem>

#include <crproject.h>

CrTextNode::CrTextNode(CrNode *parent) : CrNode (parent) {}

CrTextNode::~CrTextNode() {}

QString CrTextNode::text() const
{
    return text_;
}

void CrTextNode::setText(const QString &text)
{
    QString t = text.trimmed();
    if(text_ == t)
        return;

    text_ = t;
    emit textChanged();
    emit updateEditorPreview();
}

bool CrTextNode::isValidInputNode(CrNode *inputNode) const
{
     return static_cast<bool>(qobject_cast<CrTextNode*>(inputNode));
}

QWidget *CrTextNode::createCompactWidgetEditor()
{
    class ChangeTextCommand : public QUndoCommand
    {
    public:
        ChangeTextCommand(CrTextNode* node, const QString& text , QUndoCommand *parent = nullptr) :
            QUndoCommand(parent), node_(node), newText_(text){
            oldText_ = node_->text();
            setText(QString(QObject::tr("Node %1 change value %2 to %3.")).arg(node_->name()).arg(oldText_).arg(newText_));
        }
        virtual void undo(){
            node_->setText(oldText_);
        }
        virtual void redo(){
            node_->setText(newText_);
        }
    private:
        CrTextNode* node_;
        QString newText_;
        QString oldText_;
    };

    auto lineEdit = new QLineEdit();
    lineEdit->setText(text_);
    connect(lineEdit, &QLineEdit::editingFinished, this, [this, lineEdit](){
        if(text_ != lineEdit->text()){
            if(project()){
                project()->pushUndoCommand(new ChangeTextCommand(this, lineEdit->text()));
            }else {
                setText(lineEdit->text());
            }
        }
    });

    return lineEdit;
}

QSize CrTextNode::sizeHintEditorPreview(const QStyleOptionViewItem &option) const
{
    QStyleOptionViewItem opt(option);
    opt.text = text_;
    return CrNode::sizeHintEditorPreview(opt);
}

void CrTextNode::paintEditorPreview(QPainter *painter, const QStyleOptionViewItem &option) const
{
    QStyleOptionViewItem opt(option);
    opt.text = text_;
    CrNode::paintEditorPreview(painter, opt);
}

QByteArray CrTextNode::saveState() const
{
    QByteArray state;
    QDataStream out(&state, QIODevice::WriteOnly);
    out << CrNode::saveState();
    out << text_;
    return state;
}

bool CrTextNode::restoreState(const QByteArray &state)
{
    QDataStream in(state);
    QByteArray nodeState;
    in >> nodeState;
    CrNode::restoreState(nodeState);
    QString text;
    in >> text;
    setText(text);
    return true;
}

void CrTextNode::changeInputNode(CrNode *newNode)
{
    if(inputNode()){
        CrTextNode* textNode = static_cast<CrTextNode*>(inputNode());
        disconnect(textNode, &CrTextNode::textChanged, this, &CrTextNode::changeInputNodeValue);
    }

    if(newNode){
        CrTextNode* textNode = static_cast<CrTextNode*>(newNode);
        connect(textNode, &CrTextNode::textChanged, this, &CrTextNode::changeInputNodeValue);
        setText(static_cast<CrTextNode*>(newNode)->text_);
    }

}

void CrTextNode::changeInputNodeValue()
{
     setText(static_cast<CrTextNode*>(inputNode())->text_);
}
