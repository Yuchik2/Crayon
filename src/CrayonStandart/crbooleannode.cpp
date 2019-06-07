#include "crbooleannode.h"
#include <QApplication>
#include <QEvent>
#include <QUndoCommand>
#include <QStyleOptionViewItem>

#include <CrProject>

CrBooleanNode::CrBooleanNode(CrNode* parent) : CrNode (parent) {}

CrBooleanNode::~CrBooleanNode() {}

bool CrBooleanNode::value() const
{
    return value_;
}

void CrBooleanNode::setValue(bool value)
{
    if(value_ == value)
        return;
    value_ = value;
    emit valueChanged();
    emit updateEditorPreview();
}

bool CrBooleanNode::isValidInputNode(CrNode *inputNode) const
{
    return static_cast<bool>(qobject_cast<CrBooleanNode*>(inputNode));
}

bool CrBooleanNode::isValidOunputNode(CrNode *outputNode) const
{
    return static_cast<bool>(qobject_cast<CrBooleanNode*>(outputNode));
}

bool CrBooleanNode::eventEditorPreview(QEvent *event, const QStyleOptionViewItem &option)
{
    Q_UNUSED(option);

    class ChangeBoolValue : public QUndoCommand
    {
    public:
        ChangeBoolValue(CrBooleanNode* node, bool newValue, QUndoCommand *parent = nullptr) :
            QUndoCommand(parent), node_(node), newValue_(newValue){
            oldValue_ = node_->value();
            setText(QString(QObject::tr("Node %1 change value %2 to %3.")).arg(node_->name()).arg(oldValue_).arg(newValue_));
        }
        virtual void undo(){
            node_->setValue(oldValue_);
        }
        virtual void redo(){
            node_->setValue(newValue_);
        }
    private:
        CrBooleanNode* node_;
        bool newValue_;
        bool oldValue_;
    };

    if(flags() & CrNode::ActiveEditor){
        if(event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonDblClick){
            if(project()){
                project()->pushUndoCommand(new ChangeBoolValue(this, !value_));
            }else {
                setValue(!value_);
            }

            return true;
        }
    }
    return false;
}

QSize CrBooleanNode::sizeHintEditorPreview(const QStyleOptionViewItem &option) const
{
    Q_UNUSED(option)
    return  QSize(18, 18);
}

void CrBooleanNode::paintEditorPreview(QPainter *painter, const QStyleOptionViewItem &option) const
{
    QStyleOptionViewItem p(option);
    p.text = QString();
    CrNode::paintEditorPreview(painter, p);

    QStyleOptionButton opt;
    QRect rect(0, 0, 18, 18);
    rect.moveCenter(option.rect.center());
    rect.moveLeft(option.rect.left());
    opt.rect = rect;
    opt.state = option.state | QStyle::State_Editing;
    opt.state |= (value_ ? QStyle::State_On : QStyle::State_Off);

    const QWidget *widget = option.widget;
    QStyle *style = widget ? widget->style() : QApplication::style();
    style->drawControl(QStyle::CE_CheckBox, &opt, painter, widget);
}

QByteArray CrBooleanNode::saveState() const
{
    QByteArray state;
    QDataStream out(&state, QIODevice::WriteOnly);
    out << CrNode::saveState();
    out << value_;
    return state;
}

bool CrBooleanNode::restoreState(const QByteArray &state)
{
    QDataStream in(state);
    QByteArray nodeState;
    in >> nodeState;
    CrNode::restoreState(nodeState);
    bool value;
    in >> value;
    setValue(value);
    return true;
}

void CrBooleanNode::changeInputNode(CrNode *newNode)
{
    if(inputNode()){
        CrBooleanNode* numNode = static_cast<CrBooleanNode*>(inputNode());
        disconnect(numNode, &CrBooleanNode::valueChanged, this, &CrBooleanNode::changeInputNodeValue);
    }

    if(newNode){
        CrBooleanNode* numNode = static_cast<CrBooleanNode*>(newNode);
        connect(numNode, &CrBooleanNode::valueChanged, this, &CrBooleanNode::changeInputNodeValue);
        setValue(static_cast<CrBooleanNode*>(newNode)->value_);
    }
}

void CrBooleanNode::changeInputNodeValue()
{
     setValue(static_cast<CrBooleanNode*>(inputNode())->value_);
}
