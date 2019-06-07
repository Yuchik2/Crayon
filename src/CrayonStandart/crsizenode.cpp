#include "crsizenode.h"

#include <QSpinBox>
#include <QHBoxLayout>
#include <QUndoCommand>
#include <QStyleOptionViewItem>

#include <crproject.h>

CrAbstractSizeNode::CrAbstractSizeNode(CrNode *parent) : CrNode (parent) {}

CrAbstractSizeNode::~CrAbstractSizeNode() {}

bool CrAbstractSizeNode::isValidInputNode(CrNode *inputNode) const
{
    return static_cast<bool>(qobject_cast<CrAbstractSizeNode*>(inputNode));
}


CrSizeNode::CrSizeNode(CrNode *parent) : CrAbstractSizeNode (parent) {}

CrSizeNode::~CrSizeNode() {}

QSize CrSizeNode::size() const
{
    return size_;
}

void CrSizeNode::setSize(const QSize &size)
{
    if(size_ == size)
        return;

    size_ = size;
    emit sizeChanged();
    emit updateEditorPreview();
}

QSizeF CrSizeNode::sizeF() const
{
    return size_;
}

void CrSizeNode::setSizeF(const QSizeF &size)
{
    setSize(size.toSize());
}

QWidget *CrSizeNode::createCompactWidgetEditor()
{
    class ChangeSizeCommand : public QUndoCommand
    {
    public:
        ChangeSizeCommand(CrSizeNode* node, const QSize& size , QUndoCommand *parent = nullptr) :
            QUndoCommand(parent), node_(node), newSize_(size){
            oldSize_ = node_->size();
            setText(QString(QObject::tr("Node %1 change value %2 to %3.")).arg(node_->name()).
                    arg(QString("(%1, %2)").arg(oldSize_.width()).arg(oldSize_.height())).
                    arg(QString("(%1, %2)").arg(oldSize_.width()).arg(oldSize_.height())));
        }
        virtual void undo(){
            node_->setSize(oldSize_);
        }
        virtual void redo(){
            node_->setSize(newSize_);
        }
    private:
        CrSizeNode* node_;
        QSize newSize_;
        QSize oldSize_;
    };
    auto parent = new QWidget();
    auto layout = new QHBoxLayout();
    layout->setMargin(0);

    auto wEditor = new QSpinBox();
    wEditor->setRange(INT_MIN, INT_MAX);
    wEditor->setValue(size_.width());

    auto hEditor = new QSpinBox();
    hEditor->setRange(INT_MIN, INT_MAX);
    hEditor->setValue(size_.height());

    layout->addWidget(wEditor);
    layout->addWidget(hEditor);
    parent->setLayout(layout);

    auto change = [this, wEditor, hEditor](){
        QSize newSize(wEditor->value(), hEditor->value());
        if(size_ != newSize){
            if(project()){
                project()->pushUndoCommand(new ChangeSizeCommand(this, newSize));
            }else {
                setSize(newSize);
            }
        }
    };
    connect(wEditor, &QSpinBox::editingFinished, this, change);
    connect(hEditor, &QSpinBox::editingFinished, this, change);

    return parent;
}

QSize CrSizeNode::sizeHintEditorPreview(const QStyleOptionViewItem &option) const
{
    QStyleOptionViewItem opt(option);
    opt.text = QString("(%1 x %2)").arg(size_.width()).arg(size_.height());
    return CrNode::sizeHintEditorPreview(opt);
}

void CrSizeNode::paintEditorPreview(QPainter *painter, const QStyleOptionViewItem &option) const
{
    QStyleOptionViewItem opt(option);
    opt.text = QString("(%1 x %2)").arg(size_.width()).arg(size_.height());
    CrNode::paintEditorPreview(painter, opt);
}

QByteArray CrSizeNode::saveState() const
{
    QByteArray state;
    QDataStream out(&state, QIODevice::WriteOnly);
    out << CrNode::saveState();
    out << size_;
    return state;
}

bool CrSizeNode::restoreState(const QByteArray &state)
{
    QDataStream in(state);
    QByteArray nodeState;
    in >> nodeState;
    CrNode::restoreState(nodeState);
    QSize size;
    in >> size;
    setSize(size);
    return true;
}

void CrSizeNode::changeInputNode(CrNode *newNode)
{
    if(inputNode()){
        CrAbstractSizeNode* sizeNode = static_cast<CrAbstractSizeNode*>(inputNode());
        disconnect(sizeNode, &CrAbstractSizeNode::sizeChanged, this, &CrSizeNode::changeInputNodeValue);
    }

    if(newNode){
        CrAbstractSizeNode* sizeNode = static_cast<CrAbstractSizeNode*>(newNode);
        connect(sizeNode, &CrAbstractSizeNode::sizeChanged, this, &CrSizeNode::changeInputNodeValue);
        setSize(static_cast<CrAbstractSizeNode*>(newNode)->size());
    }
}

void CrSizeNode::changeInputNodeValue()
{
    setSize(static_cast<CrAbstractSizeNode*>(inputNode())->size());
}

CrSizeFNode::CrSizeFNode(CrNode *parent) : CrAbstractSizeNode(parent) {}

CrSizeFNode::~CrSizeFNode() {}

QSize CrSizeFNode::size() const
{
    return size_.toSize();
}

void CrSizeFNode::setSize(const QSize &size)
{
   setSizeF(size);
}

QSizeF CrSizeFNode::sizeF() const
{
    return size_;
}

void CrSizeFNode::setSizeF(const QSizeF &size)
{
    if(size_ == size)
        return;

    size_ = size;
    emit sizeChanged();
    emit updateEditorPreview();
}

QWidget *CrSizeFNode::createCompactWidgetEditor()
{
    class ChangeSizeCommand : public QUndoCommand
    {
    public:
        ChangeSizeCommand(CrSizeFNode* node, const QSizeF& size , QUndoCommand *parent = nullptr) :
            QUndoCommand(parent), node_(node), newSize_(size){
            oldSize_ = node_->size();
            setText(QString(QObject::tr("Node %1 change value %2 to %3.")).arg(node_->name()).
                    arg(QString("(%1, %2)").arg(oldSize_.width()).arg(oldSize_.height())).
                    arg(QString("(%1, %2)").arg(oldSize_.width()).arg(oldSize_.height())));
        }
        virtual void undo(){
            node_->setSizeF(oldSize_);
        }
        virtual void redo(){
            node_->setSizeF(newSize_);
        }
    private:
        CrSizeFNode* node_;
        QSizeF newSize_;
        QSizeF oldSize_;
    };
    auto parent = new QWidget();
    auto layout = new QHBoxLayout();
    layout->setMargin(0);

    auto wEditor = new QDoubleSpinBox();
    wEditor->setRange(INT_MIN, INT_MAX);
    wEditor->setValue(size_.width());

    auto hEditor = new QDoubleSpinBox();
    hEditor->setRange(INT_MIN, INT_MAX);
    hEditor->setValue(size_.height());

    layout->addWidget(wEditor);
    layout->addWidget(hEditor);
    parent->setLayout(layout);

    auto change = [this, wEditor, hEditor](){
        QSizeF newSize(wEditor->value(), hEditor->value());
        if(size_ != newSize){
            if(project()){
                project()->pushUndoCommand(new ChangeSizeCommand(this, newSize));
            }else {
                setSizeF(newSize);
            }
        }
    };
    connect(wEditor, &QSpinBox::editingFinished, this, change);
    connect(hEditor, &QSpinBox::editingFinished, this, change);

    return parent;
}

QSize CrSizeFNode::sizeHintEditorPreview(const QStyleOptionViewItem &option) const
{
    QStyleOptionViewItem opt(option);
    opt.text = QString("(%1 x %2)").arg(size_.width()).arg(size_.height());
    return CrNode::sizeHintEditorPreview(opt);
}

void CrSizeFNode::paintEditorPreview(QPainter *painter, const QStyleOptionViewItem &option) const
{
    QStyleOptionViewItem opt(option);
    opt.text = QString("(%1 x %2)").arg(size_.width()).arg(size_.height());
    CrNode::paintEditorPreview(painter, opt);
}

QByteArray CrSizeFNode::saveState() const
{
    QByteArray state;
    QDataStream out(&state, QIODevice::WriteOnly);
    out << CrNode::saveState();
    out << size_;
    return state;
}

bool CrSizeFNode::restoreState(const QByteArray &state)
{
    QDataStream in(state);
    QByteArray nodeState;
    in >> nodeState;
    CrNode::restoreState(nodeState);
    QSizeF size;
    in >> size;
    setSizeF(size);
    return true;
}

void CrSizeFNode::changeInputNode(CrNode *newNode)
{
    if(inputNode()){
        CrAbstractSizeNode* sizeNode = static_cast<CrAbstractSizeNode*>(inputNode());
        disconnect(sizeNode, &CrAbstractSizeNode::sizeChanged, this, &CrSizeFNode::changeInputNodeValue);
    }

    if(newNode){
        CrAbstractSizeNode* sizeNode = static_cast<CrAbstractSizeNode*>(newNode);
        connect(sizeNode, &CrAbstractSizeNode::sizeChanged, this, &CrSizeFNode::changeInputNodeValue);
        setSizeF(static_cast<CrAbstractSizeNode*>(newNode)->sizeF());
    }
}

void CrSizeFNode::changeInputNodeValue()
{
    setSizeF(static_cast<CrSizeNode*>(inputNode())->sizeF());
}


