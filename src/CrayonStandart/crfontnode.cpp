#include "crfontnode.h"

#include <QFontDialog>
#include <QUndoCommand>
#include <QStyleOptionViewItem>

#include <crproject.h>

CrFontNode::CrFontNode(CrNode *parent) : CrNode (parent) {}

CrFontNode::~CrFontNode() {}

QFont CrFontNode::font() const
{
    return font_;
}

void CrFontNode::setFont(const QFont &font)
{
    if(font_ == font)
        return;

    font_ = font;
    emit fontChanged();
    emit updateEditorPreview();
}

bool CrFontNode::isValidInputNode(CrNode *inputNode) const
{
     return static_cast<bool>(qobject_cast<CrFontNode*>(inputNode));
}

QDialog *CrFontNode::createDialogEditor()
{
    class ChangeFontCommand : public QUndoCommand
    {
    public:
        ChangeFontCommand(CrFontNode* node, const QFont& font , QUndoCommand *parent = nullptr) :
            QUndoCommand(parent), node_(node), newFont_(font){
            oldFont_ = node_->font();
            setText(QString(QObject::tr("Node %1 change value %2 to %3.")).arg(node_->name()).arg(oldFont_.family()).arg(newFont_.family()));
        }
        virtual void undo(){
            node_->setFont(oldFont_);
        }
        virtual void redo(){
            node_->setFont(newFont_);
        }
    private:
        CrFontNode* node_;
        QFont newFont_;
        QFont oldFont_;
    };

    auto fontDialog = new QFontDialog();
    fontDialog->setFont(font_);
    connect(fontDialog, &QFontDialog::accepted, this, [this, fontDialog](){
        if(font_ != fontDialog->font()){
            if(project()){
                project()->pushUndoCommand(new ChangeFontCommand(this, fontDialog->font()));
            }else {
                setFont(fontDialog->font());
            }
        }
    });

    return fontDialog;
}

QSize CrFontNode::sizeHintEditorPreview(const QStyleOptionViewItem &option) const
{
    QStyleOptionViewItem opt(option);
    opt.text = font().family();
    return CrNode::sizeHintEditorPreview(opt);
}

void CrFontNode::paintEditorPreview(QPainter *painter, const QStyleOptionViewItem &option) const
{
    QStyleOptionViewItem opt(option);
    opt.text = font_.family();
    CrNode::paintEditorPreview(painter, opt);
}

QByteArray CrFontNode::saveState() const
{
    QByteArray state;
    QDataStream out(&state, QIODevice::WriteOnly);
    out << CrNode::saveState();
    out << font_;
    return state;
}

bool CrFontNode::restoreState(const QByteArray &state)
{
    QDataStream in(state);
    QByteArray nodeState;
    in >> nodeState;
    CrNode::restoreState(nodeState);
    QFont font;
    in >> font;
    setFont(font);
    return true;
}

void CrFontNode::changeInputNode(CrNode *newNode)
{
    if(inputNode()){
        CrFontNode* fontNode = static_cast<CrFontNode*>(inputNode());
        disconnect(fontNode, &CrFontNode::fontChanged, this, &CrFontNode::changeInputNodeValue);
    }

    if(newNode){
        CrFontNode* fontNode = static_cast<CrFontNode*>(newNode);
        connect(fontNode, &CrFontNode::fontChanged, this, &CrFontNode::changeInputNodeValue);
        setFont(static_cast<CrFontNode*>(newNode)->font());
    }
}

void CrFontNode::changeInputNodeValue()
{
     setFont(static_cast<CrFontNode*>(inputNode())->font());
}


