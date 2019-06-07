#include "crcolornode.h"

#include <QColorDialog>
#include <QUndoCommand>
#include <QStyleOptionViewItem>

#include <crproject.h>

CrColorNode::CrColorNode(CrNode *parent) : CrNode (parent) {}

CrColorNode::~CrColorNode() {}

QColor CrColorNode::color() const
{
    return color_;
}

void CrColorNode::setColor(const QColor &color)
{
    if(color_ == color)
        return;

    color_ = color;
    emit colorChanged();
    emit updateEditorPreview();
    color_ = color;
}

bool CrColorNode::isValidInputNode(CrNode *inputNode) const
{
    return static_cast<bool>(qobject_cast<CrColorNode*>(inputNode));
}

QDialog *CrColorNode::createDialogEditor()
{
    class ChangeColorCommand : public QUndoCommand
    {
    public:
        ChangeColorCommand(CrColorNode* node, const QColor& color , QUndoCommand *parent = nullptr) :
            QUndoCommand(parent), node_(node), newColor_(color){
            oldColor_ = node_->color();
            setText(QString(QObject::tr("Node %1 change value %2 to %3.")).arg(node_->name()).arg(oldColor_.name()).arg(newColor_.name()));
        }
        virtual void undo(){
            node_->setColor(oldColor_);
        }
        virtual void redo(){
            node_->setColor(newColor_);
        }
    private:
        CrColorNode* node_;
        QColor newColor_;
        QColor oldColor_;
    };

    auto colorDialog = new QColorDialog();
    colorDialog->setCurrentColor(color_);
    connect(colorDialog, &QColorDialog::accepted, this, [this, colorDialog](){
        if(color_ != colorDialog->currentColor()){
            if(project()){
                project()->pushUndoCommand(new ChangeColorCommand(this, colorDialog->currentColor()));
            }else {
                setColor(colorDialog->currentColor());
            }
        }
    });

    return colorDialog;
}

QSize CrColorNode::sizeHintEditorPreview(const QStyleOptionViewItem &option) const
{
    QStyleOptionViewItem opt(option);
    opt.text = color_.name();
    return CrNode::sizeHintEditorPreview(opt);
}

void CrColorNode::paintEditorPreview(QPainter *painter, const QStyleOptionViewItem &option) const
{
    QStyleOptionViewItem opt(option);
    opt.text = color_.name();
    CrNode::paintEditorPreview(painter, opt);
}

QByteArray CrColorNode::saveState() const
{
    QByteArray state;
    QDataStream out(&state, QIODevice::WriteOnly);
    out << CrNode::saveState();
    out << color_;
    return state;
}

bool CrColorNode::restoreState(const QByteArray &state)
{
    QDataStream in(state);
    QByteArray nodeState;
    in >> nodeState;
    CrNode::restoreState(nodeState);
    QColor color;
    in >> color;
    setColor(color);
    return true;
}

void CrColorNode::changeInputNode(CrNode *newNode)
{
    if(inputNode()){
        CrColorNode* colorNode = static_cast<CrColorNode*>(inputNode());
        disconnect(colorNode, &CrColorNode::colorChanged, this, &CrColorNode::changeInputNodeValue);
    }

    if(newNode){
        CrColorNode* colorNode = static_cast<CrColorNode*>(newNode);
        connect(colorNode, &CrColorNode::colorChanged, this, &CrColorNode::changeInputNodeValue);
        setColor(static_cast<CrColorNode*>(newNode)->color_);
    }
}

void CrColorNode::changeInputNodeValue()
{
    setColor(static_cast<CrColorNode*>(inputNode())->color_);
}

