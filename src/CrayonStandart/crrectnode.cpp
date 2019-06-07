#include "crrectnode.h"

#include <QSpinBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QUndoCommand>
#include <QStyleOptionViewItem>

#include <crproject.h>

CrAbstractRectNode::CrAbstractRectNode(CrNode *parent) : CrNode (parent) {}

CrAbstractRectNode::~CrAbstractRectNode() {}

bool CrAbstractRectNode::isValidInputNode(CrNode *inputNode) const
{
    return static_cast<bool>(qobject_cast<CrAbstractRectNode*>(inputNode));
}


CrRectNode::CrRectNode(CrNode *parent) : CrAbstractRectNode (parent) {}

CrRectNode::~CrRectNode() {}

QRect CrRectNode::rect() const
{
    return rect_;
}

void CrRectNode::setRect(const QRect &rect)
{
    if(rect_ == rect)
        return;

    rect_ = rect;
    emit rectChanged();
    emit updateEditorPreview();
}

QRectF CrRectNode::rectF() const
{
    return rect_;
}

void CrRectNode::setRectF(const QRectF &rect)
{
    setRect(rect.toRect());
}

QWidget *CrRectNode::createCompactWidgetEditor()
{
    class ChangeRectCommand : public QUndoCommand
    {
    public:
        ChangeRectCommand(CrRectNode* node, const QRect& rect , QUndoCommand *parent = nullptr) :
            QUndoCommand(parent), node_(node), newRect_(rect){
            oldRect_ = node_->rect();
            setText(QString(QObject::tr("Node %1 change value %2 to %3.")).arg(node_->name()).
                    arg(QString("(%1, %2; %3 x %4)").arg(oldRect_.x()).arg(oldRect_.y()).arg(oldRect_.width()).arg(oldRect_.height())).
                    arg(QString("(%1, %2; %3 x %4)").arg(newRect_.x()).arg(newRect_.y()).arg(newRect_.width()).arg(newRect_.height())));
        }
        virtual void undo(){
            node_->setRect(oldRect_);
        }
        virtual void redo(){
            node_->setRect(newRect_);
        }
    private:
        CrRectNode* node_;
        QRect newRect_;
        QRect oldRect_;
    };
    auto parent = new QWidget();
    auto columnLayout = new QVBoxLayout();
    columnLayout->setMargin(0);

    auto rowlayout1 = new QHBoxLayout();
    rowlayout1->setMargin(0);

    auto xEditor = new QSpinBox();
    xEditor->setRange(INT_MIN, INT_MAX);
    xEditor->setValue(rect_.x());

    auto yEditor = new QSpinBox();
    yEditor->setRange(INT_MIN, INT_MAX);
    yEditor->setValue(rect_.y());

    rowlayout1->addWidget(xEditor);
    rowlayout1->addWidget(yEditor);
    columnLayout->addLayout(rowlayout1);

    auto rowlayout2 = new QHBoxLayout();
    rowlayout1->setMargin(0);

    auto wEditor = new QSpinBox();
    wEditor->setRange(INT_MIN, INT_MAX);
    wEditor->setValue(rect_.width());

    auto hEditor = new QSpinBox();
    hEditor->setRange(INT_MIN, INT_MAX);
    hEditor->setValue(rect_.height());

    rowlayout2->addWidget(wEditor);
    rowlayout2->addWidget(hEditor);
    columnLayout->addLayout(rowlayout2);

    auto change = [this, xEditor, yEditor, wEditor, hEditor](){
        QRect newRect(xEditor->value(), yEditor->value(), wEditor->value(), hEditor->value());
        if(rect_ != newRect){
            if(project()){
                project()->pushUndoCommand(new ChangeRectCommand(this, newRect));
            }else {
                setRect(newRect);
            }
        }
    };
    connect(xEditor, &QSpinBox::editingFinished, this, change);
    connect(yEditor, &QSpinBox::editingFinished, this, change);
    connect(wEditor, &QSpinBox::editingFinished, this, change);
    connect(hEditor, &QSpinBox::editingFinished, this, change);

    return parent;
}

QSize CrRectNode::sizeHintEditorPreview(const QStyleOptionViewItem &option) const
{
    QStyleOptionViewItem opt(option);
    opt.text = QString("(%1, %2; %3 x %4)").arg(rect_.x()).arg(rect_.y()).arg(rect_.width()).arg(rect_.height());
    return CrNode::sizeHintEditorPreview(opt);
}

void CrRectNode::paintEditorPreview(QPainter *painter, const QStyleOptionViewItem &option) const
{
    QStyleOptionViewItem opt(option);
    opt.text = QString("(%1, %2; %3 x %4)").arg(rect_.x()).arg(rect_.y()).arg(rect_.width()).arg(rect_.height());
    CrNode::paintEditorPreview(painter, opt);
}

QByteArray CrRectNode::saveState() const
{
    QByteArray state;
    QDataStream out(&state, QIODevice::WriteOnly);
    out << CrNode::saveState();
    out << rect_;
    return state;
}

bool CrRectNode::restoreState(const QByteArray &state)
{
    QDataStream in(state);
    QByteArray nodeState;
    in >> nodeState;
    CrNode::restoreState(nodeState);
    QRect rect;
    in >> rect;
    setRect(rect);
    return true;
}

void CrRectNode::changeInputNode(CrNode *newNode)
{
    if(inputNode()){
        CrAbstractRectNode* rectNode = static_cast<CrAbstractRectNode*>(inputNode());
        disconnect(rectNode, &CrAbstractRectNode::rectChanged, this, &CrRectNode::changeInputNodeValue);
    }

    if(newNode){
        CrAbstractRectNode* rectNode = static_cast<CrAbstractRectNode*>(newNode);
        connect(rectNode, &CrAbstractRectNode::rectChanged, this, &CrRectNode::changeInputNodeValue);
        setRect(static_cast<CrAbstractRectNode*>(newNode)->rect());
    }
}

void CrRectNode::changeInputNodeValue()
{
    setRect(static_cast<CrAbstractRectNode*>(inputNode())->rect());
}

CrRectFNode::CrRectFNode(CrNode *parent) : CrAbstractRectNode(parent) {}

CrRectFNode::~CrRectFNode() {}

QRect CrRectFNode::rect() const
{
    return rect_.toRect();
}

void CrRectFNode::setRect(const QRect &rect)
{
   setRectF(rect);
}

QRectF CrRectFNode::rectF() const
{
    return rect_;
}

void CrRectFNode::setRectF(const QRectF &rect)
{
    if(rect_ == rect)
        return;

    rect_ = rect;
    emit rectChanged();
    emit updateEditorPreview();
}

QWidget *CrRectFNode::createCompactWidgetEditor()
{
    class ChangeRectCommand : public QUndoCommand
    {
    public:
        ChangeRectCommand(CrRectFNode* node, const QRectF& rect , QUndoCommand *parent = nullptr) :
            QUndoCommand(parent), node_(node), newRect_(rect){
            oldRect_ = node_->rect();
            setText(QString(QObject::tr("Node %1 change value %2 to %3.")).arg(node_->name()).
                    arg(QString("(%1, %2; %3 x %4)").arg(oldRect_.x()).arg(oldRect_.y()).arg(oldRect_.width()).arg(oldRect_.height())).
                    arg(QString("(%1, %2; %3 x %4)").arg(newRect_.x()).arg(newRect_.y()).arg(newRect_.width()).arg(newRect_.height())));
        }
        virtual void undo(){
            node_->setRectF(oldRect_);
        }
        virtual void redo(){
            node_->setRectF(newRect_);
        }
    private:
        CrRectFNode* node_;
        QRectF newRect_;
        QRectF oldRect_;
    };
    auto parent = new QWidget();
    auto columnLayout = new QVBoxLayout();
    columnLayout->setMargin(0);

    auto rowlayout1 = new QHBoxLayout();
    rowlayout1->setMargin(0);

    auto xEditor = new QDoubleSpinBox();
    xEditor->setRange(INT_MIN, INT_MAX);
    xEditor->setValue(rect_.x());

    auto yEditor = new QDoubleSpinBox();
    yEditor->setRange(INT_MIN, INT_MAX);
    yEditor->setValue(rect_.y());

    rowlayout1->addWidget(xEditor);
    rowlayout1->addWidget(yEditor);
    columnLayout->addLayout(rowlayout1);

    auto rowlayout2 = new QHBoxLayout();
    rowlayout1->setMargin(0);

    auto wEditor = new QDoubleSpinBox();
    wEditor->setRange(INT_MIN, INT_MAX);
    wEditor->setValue(rect_.width());

    auto hEditor = new QDoubleSpinBox();
    hEditor->setRange(INT_MIN, INT_MAX);
    hEditor->setValue(rect_.height());

    rowlayout2->addWidget(wEditor);
    rowlayout2->addWidget(hEditor);
    columnLayout->addLayout(rowlayout2);

    auto change = [this, xEditor, yEditor, wEditor, hEditor](){
        QRectF newRect(xEditor->value(), yEditor->value(), wEditor->value(), hEditor->value());
        if(rect_ != newRect){
            if(project()){
                project()->pushUndoCommand(new ChangeRectCommand(this, newRect));
            }else {
                setRectF(newRect);
            }
        }
    };
    connect(xEditor, &QSpinBox::editingFinished, this, change);
    connect(yEditor, &QSpinBox::editingFinished, this, change);
    connect(wEditor, &QSpinBox::editingFinished, this, change);
    connect(hEditor, &QSpinBox::editingFinished, this, change);

    return parent;
}

QSize CrRectFNode::sizeHintEditorPreview(const QStyleOptionViewItem &option) const
{
    QStyleOptionViewItem opt(option);
    opt.text = QString("(%1, %2; %3 x %4)").arg(rect_.x()).arg(rect_.y()).arg(rect_.width()).arg(rect_.height());
    return CrNode::sizeHintEditorPreview(opt);
}

void CrRectFNode::paintEditorPreview(QPainter *painter, const QStyleOptionViewItem &option) const
{
    QStyleOptionViewItem opt(option);
    opt.text = QString("(%1, %2; %3 x %4)").arg(rect_.x()).arg(rect_.y()).arg(rect_.width()).arg(rect_.height());
    CrNode::paintEditorPreview(painter, opt);
}

QByteArray CrRectFNode::saveState() const
{
    QByteArray state;
    QDataStream out(&state, QIODevice::WriteOnly);
    out << CrNode::saveState();
    out << rect_;
    return state;
}

bool CrRectFNode::restoreState(const QByteArray &state)
{
    QDataStream in(state);
    QByteArray nodeState;
    in >> nodeState;
    CrNode::restoreState(nodeState);
    QRectF rect;
    in >> rect;
    setRectF(rect);
    return true;
}

void CrRectFNode::changeInputNode(CrNode *newNode)
{
    if(inputNode()){
        CrAbstractRectNode* rectNode = static_cast<CrAbstractRectNode*>(inputNode());
        disconnect(rectNode, &CrAbstractRectNode::rectChanged, this, &CrRectFNode::changeInputNodeValue);
    }

    if(newNode){
        CrAbstractRectNode* rectNode = static_cast<CrAbstractRectNode*>(newNode);
        connect(rectNode, &CrAbstractRectNode::rectChanged, this, &CrRectFNode::changeInputNodeValue);
        setRectF(static_cast<CrAbstractRectNode*>(newNode)->rectF());
    }
}

void CrRectFNode::changeInputNodeValue()
{
    setRectF(static_cast<CrRectNode*>(inputNode())->rectF());
}


