#include "crpointnode.h"
#include <QSpinBox>
#include <QHBoxLayout>
#include <QUndoCommand>
#include <QStyleOptionViewItem>

#include <crproject.h>

CrAbstractPointNode::CrAbstractPointNode(CrNode *parent) : CrNode (parent) {}

CrAbstractPointNode::~CrAbstractPointNode() {}

bool CrAbstractPointNode::isValidInputNode(CrNode *inputNode) const
{
    return static_cast<bool>(qobject_cast<CrAbstractPointNode*>(inputNode));
}


CrPointNode::CrPointNode(CrNode *parent) : CrAbstractPointNode (parent) {}

CrPointNode::~CrPointNode() {}

QPoint CrPointNode::point() const
{
    return point_;
}

void CrPointNode::setPoint(const QPoint &point)
{
    if(point_ == point)
        return;

    point_ = point;
    emit pointChanged();
    emit updateEditorPreview();
}

QPointF CrPointNode::pointF() const
{
    return point_;
}

void CrPointNode::setPointF(const QPointF &point)
{
    setPoint(point.toPoint());
}

QWidget *CrPointNode::createCompactWidgetEditor()
{
    class ChangePointCommand : public QUndoCommand
    {
    public:
        ChangePointCommand(CrPointNode* node, const QPoint& point , QUndoCommand *parent = nullptr) :
            QUndoCommand(parent), node_(node), newPoint_(point){
            oldPoint_ = node_->point();
            setText(QString(QObject::tr("Node %1 change value %2 to %3.")).arg(node_->name()).
                    arg(QString("(%1, %2)").arg(oldPoint_.x()).arg(oldPoint_.y())).
                    arg(QString("(%1, %2)").arg(newPoint_.x()).arg(newPoint_.y())));
        }
        virtual void undo(){
            node_->setPoint(oldPoint_);
        }
        virtual void redo(){
            node_->setPoint(newPoint_);
        }
    private:
        CrPointNode* node_;
        QPoint newPoint_;
        QPoint oldPoint_;
    };
    auto parent = new QWidget();
    auto layout = new QHBoxLayout();
    layout->setMargin(0);

    auto xEditor = new QSpinBox();
    xEditor->setRange(INT_MIN, INT_MAX);
    xEditor->setValue(point_.x());

    auto yEditor = new QSpinBox();
    yEditor->setRange(INT_MIN, INT_MAX);
    yEditor->setValue(point_.y());

    layout->addWidget(xEditor);
    layout->addWidget(yEditor);
    parent->setLayout(layout);

    auto change = [this, xEditor, yEditor](){
        QPoint newPoint(xEditor->value(), yEditor->value());
        if(point_ != newPoint){
            if(project()){
                project()->pushUndoCommand(new ChangePointCommand(this, newPoint));
            }else {
                setPoint(newPoint);
            }
        }
    };
    connect(xEditor, &QSpinBox::editingFinished, this, change);
    connect(yEditor, &QSpinBox::editingFinished, this, change);

    return parent;
}

QSize CrPointNode::sizeHintEditorPreview(const QStyleOptionViewItem &option) const
{
    QStyleOptionViewItem opt(option);
    opt.text = QString("(%1, %2)").arg(point_.x()).arg(point_.y());
    return CrNode::sizeHintEditorPreview(opt);
}

void CrPointNode::paintEditorPreview(QPainter *painter, const QStyleOptionViewItem &option) const
{
    QStyleOptionViewItem opt(option);
    opt.text = QString("(%1, %2)").arg(point_.x()).arg(point_.y());
    CrNode::paintEditorPreview(painter, opt);
}

QByteArray CrPointNode::saveState() const
{
    QByteArray state;
    QDataStream out(&state, QIODevice::WriteOnly);
    out << CrNode::saveState();
    out << point_;
    return state;
}

bool CrPointNode::restoreState(const QByteArray &state)
{
    QDataStream in(state);
    QByteArray nodeState;
    in >> nodeState;
    CrNode::restoreState(nodeState);
    QPoint point;
    in >> point;
    setPoint(point);
    return true;
}

void CrPointNode::changeInputNode(CrNode *newNode)
{
    if(inputNode()){
        CrAbstractPointNode* pointNode = static_cast<CrAbstractPointNode*>(inputNode());
        disconnect(pointNode, &CrAbstractPointNode::pointChanged, this, &CrPointNode::changeInputNodeValue);
    }

    if(newNode){
        CrAbstractPointNode* pointNode = static_cast<CrAbstractPointNode*>(newNode);
        connect(pointNode, &CrAbstractPointNode::pointChanged, this, &CrPointNode::changeInputNodeValue);
        setPoint(static_cast<CrAbstractPointNode*>(newNode)->point());
    }
}

void CrPointNode::changeInputNodeValue()
{
    setPoint(static_cast<CrAbstractPointNode*>(inputNode())->point());
}

CrPointFNode::CrPointFNode(CrNode *parent) : CrAbstractPointNode(parent) {}

CrPointFNode::~CrPointFNode() {}

QPoint CrPointFNode::point() const
{
    return point_.toPoint();
}

void CrPointFNode::setPoint(const QPoint &point)
{
   setPointF(point);
}

QPointF CrPointFNode::pointF() const
{
    return point_;
}

void CrPointFNode::setPointF(const QPointF &point)
{
    if(point_ == point)
        return;

    point_ = point;
    emit pointChanged();
    emit updateEditorPreview();
}

QWidget *CrPointFNode::createCompactWidgetEditor()
{
    class ChangePointCommand : public QUndoCommand
    {
    public:
        ChangePointCommand(CrPointFNode* node, const QPointF& point , QUndoCommand *parent = nullptr) :
            QUndoCommand(parent), node_(node), newPoint_(point){
            oldPoint_ = node_->point();
            setText(QString(QObject::tr("Node %1 change value %2 to %3.")).arg(node_->name()).
                    arg(QString("(%1, %2)").arg(oldPoint_.x()).arg(oldPoint_.y())).
                    arg(QString("(%1, %2)").arg(newPoint_.x()).arg(newPoint_.y())));
        }
        virtual void undo(){
            node_->setPointF(oldPoint_);
        }
        virtual void redo(){
            node_->setPointF(newPoint_);
        }
    private:
        CrPointFNode* node_;
        QPointF newPoint_;
        QPointF oldPoint_;
    };
    auto parent = new QWidget();
    auto layout = new QHBoxLayout();
    layout->setMargin(0);

    auto xEditor = new QDoubleSpinBox();
    xEditor->setRange(INT_MIN, INT_MAX);
    xEditor->setValue(point_.x());

    auto yEditor = new QDoubleSpinBox();
    yEditor->setRange(INT_MIN, INT_MAX);
    yEditor->setValue(point_.y());

    layout->addWidget(xEditor);
    layout->addWidget(yEditor);
    parent->setLayout(layout);

    auto change = [this, xEditor, yEditor](){
        QPointF newPoint(xEditor->value(), yEditor->value());
        if(point_ != newPoint){
            if(project()){
                project()->pushUndoCommand(new ChangePointCommand(this, newPoint));
            }else {
                setPointF(newPoint);
            }
        }
    };
    connect(xEditor, &QSpinBox::editingFinished, this, change);
    connect(yEditor, &QSpinBox::editingFinished, this, change);

    return parent;
}

QSize CrPointFNode::sizeHintEditorPreview(const QStyleOptionViewItem &option) const
{
    QStyleOptionViewItem opt(option);
    opt.text = QString("(%1, %2)").arg(point_.x()).arg(point_.y());
    return CrNode::sizeHintEditorPreview(opt);
}

void CrPointFNode::paintEditorPreview(QPainter *painter, const QStyleOptionViewItem &option) const
{
    QStyleOptionViewItem opt(option);
    opt.text = QString("(%1, %2)").arg(point_.x()).arg(point_.y());
    CrNode::paintEditorPreview(painter, opt);
}

QByteArray CrPointFNode::saveState() const
{
    QByteArray state;
    QDataStream out(&state, QIODevice::WriteOnly);
    out << CrNode::saveState();
    out << point_;
    return state;
}

bool CrPointFNode::restoreState(const QByteArray &state)
{
    QDataStream in(state);
    QByteArray nodeState;
    in >> nodeState;
    CrNode::restoreState(nodeState);
    QPointF point;
    in >> point;
    setPointF(point);
    return true;
}

void CrPointFNode::changeInputNode(CrNode *newNode)
{
    if(inputNode()){
        CrAbstractPointNode* pointNode = static_cast<CrAbstractPointNode*>(inputNode());
        disconnect(pointNode, &CrAbstractPointNode::pointChanged, this, &CrPointFNode::changeInputNodeValue);
    }

    if(newNode){
        CrAbstractPointNode* pointNode = static_cast<CrAbstractPointNode*>(newNode);
        connect(pointNode, &CrAbstractPointNode::pointChanged, this, &CrPointFNode::changeInputNodeValue);
        setPointF(static_cast<CrAbstractPointNode*>(newNode)->pointF());
    }
}

void CrPointFNode::changeInputNodeValue()
{
    setPointF(static_cast<CrPointNode*>(inputNode())->pointF());
}


