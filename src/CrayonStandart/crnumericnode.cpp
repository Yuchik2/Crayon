#include "crnumericnode.h"

#include <QSpinBox>
#include <QUndoCommand>
#include <QStyleOptionViewItem>

#include <crproject.h>

#include <QDebug>

CrNumericNode::CrNumericNode(CrNode *parent) : CrNode (parent) {}

CrNumericNode::~CrNumericNode(){}

QString CrNumericNode::suffix() const
{
    return suffix_;
}

void CrNumericNode::setSuffix(const QString &suffix)
{
    QString newSuffix(suffix.simplified());
    if(suffix_ == newSuffix)
        return;
    suffix_ = newSuffix;
    emit suffixChanged();
}

bool CrNumericNode::isValidInputNode(CrNode *inputNode) const
{
    return static_cast<bool>(qobject_cast<CrNumericNode*>(inputNode));
}

QByteArray CrNumericNode::saveState() const
{
    QByteArray state;
    QDataStream out(&state, QIODevice::WriteOnly);
    out << CrNode::saveState();
    out << suffix_;
    return state;
}

bool CrNumericNode::restoreState(const QByteArray &state)
{
    QDataStream in(state);
    QByteArray nodeState;
    in >> nodeState;
    CrNode::restoreState(nodeState);
    QString suffix;
    in >> suffix;
    setSuffix(suffix);
    return true;
}

void CrNumericNode::changeInputNode(CrNode *newNode)
{
    if(inputNode()){
        CrNumericNode* numNode = static_cast<CrNumericNode*>(inputNode());
        disconnect(numNode, &CrNumericNode::valueChanged, this, &CrNumericNode::changeInputNodeValue);
    }

    if(newNode){
        CrNumericNode* numNode = static_cast<CrNumericNode*>(newNode);
        connect(numNode, &CrNumericNode::valueChanged, this, &CrNumericNode::changeInputNodeValue);
    }

}

CrIntegerNode::CrIntegerNode(CrNode *parent) : CrNumericNode (parent) {

}

CrIntegerNode::~CrIntegerNode() {}

int CrIntegerNode::intValue() const
{
    return value_;
}

void CrIntegerNode::setIntValue(int value)
{
    if(value_ == value)
        return;
    value_ = value;
    emit valueChanged();
    emit updateEditorPreview();
}

qreal CrIntegerNode::realValue() const
{
    return static_cast<qreal>(value_);
}

void CrIntegerNode::setRealValue(qreal value)
{
    setIntValue(static_cast<int>(value));
}

QWidget *CrIntegerNode::createCompactWidgetEditor()
{
    class ChangeIntValueCommand : public QUndoCommand
    {
    public:
        ChangeIntValueCommand(CrIntegerNode* node, int newValue , QUndoCommand *parent = nullptr) :
            QUndoCommand(parent), node_(node), newValue_(newValue){
            oldValue_ = node->intValue();
            setText(QString(QObject::tr("Node %1 change value %2 to %3.")).arg(node_->name()).arg(oldValue_).arg(newValue_));
        }
        virtual void undo(){
            node_->setIntValue(oldValue_);
        }
        virtual void redo(){
            node_->setIntValue(newValue_);
        }
    private:
        CrIntegerNode* node_;
        int newValue_;
        int oldValue_;
    };

    auto spinBox = new QSpinBox();
    spinBox->setRange(INT_MIN, INT_MAX);
    spinBox->setValue(intValue());
    spinBox->setSuffix(suffix());
    connect(spinBox, &QSpinBox::editingFinished, this, [this, spinBox](){
        if(intValue() != spinBox->value()){
            if(project()){
                project()->pushUndoCommand(new ChangeIntValueCommand(this, spinBox->value()));
            }else {
                setIntValue(spinBox->value());
            }
        }
    });

    return spinBox;
}

QSize CrIntegerNode::sizeHintEditorPreview(const QStyleOptionViewItem &option) const
{
    QStyleOptionViewItem opt(option);
    opt.text = QString::number(value_) + " " + suffix();
    return CrNode::sizeHintEditorPreview(opt);
}

void CrIntegerNode::paintEditorPreview(QPainter *painter, const QStyleOptionViewItem &option) const
{
    QStyleOptionViewItem opt(option);
    opt.text = QString::number(value_) + " " + suffix();
    CrNode::paintEditorPreview(painter, opt);
}

QByteArray CrIntegerNode::saveState() const
{
    QByteArray state;
    QDataStream out(&state, QIODevice::WriteOnly);
    out << CrNumericNode::saveState();
    out << value_;
    return state;
}

bool CrIntegerNode::restoreState(const QByteArray &state)
{
    QDataStream in(state);
    QByteArray nodeState;
    in >> nodeState;
    CrNumericNode::restoreState(nodeState);
    int value;
    in >> value;
    setIntValue(value);
    return true;
}

void CrIntegerNode::changeInputNode(CrNode *newNode)
{
    CrNumericNode::changeInputNode(newNode);
    if(newNode){
        setIntValue(static_cast<CrNumericNode*>(newNode)->intValue());
    }
}

void CrIntegerNode::changeInputNodeValue()
{
    setIntValue(static_cast<CrNumericNode*>(inputNode())->intValue());
}


CrIntegerLimitedNode::CrIntegerLimitedNode(CrNode *parent) : CrIntegerNode (parent) {}

CrIntegerLimitedNode::~CrIntegerLimitedNode() {}

void CrIntegerLimitedNode::setIntValue(int value)
{
    value = qBound(min_, value, max_);
    CrIntegerNode::setIntValue(value);
}

int CrIntegerLimitedNode::min() const
{
    return min_;
}

void CrIntegerLimitedNode::setMin(int min)
{
    setRange(min, max_);
}

int CrIntegerLimitedNode::max() const
{
    return max_;
}

void CrIntegerLimitedNode::setMax(int max)
{
    setRange(min_, max);
}

void CrIntegerLimitedNode::setRange(int min, int max)
{
    if(min_ == min && max_ == max)
        return;
    min_ = min;
    max_ = max;
    setIntValue(intValue());
}

QWidget *CrIntegerLimitedNode::createCompactWidgetEditor()
{
    class ChangeIntValueCommand : public QUndoCommand
    {
    public:
        ChangeIntValueCommand(CrIntegerNode* node, int newValue , QUndoCommand *parent = nullptr) :
            QUndoCommand(parent), node_(node), newValue_(newValue){
            oldValue_ = node->intValue();
            setText(QString(QObject::tr("Node %1 change value %2 to %3.")).arg(node_->name()).arg(oldValue_).arg(newValue_));
        }
        virtual void undo(){
            node_->setIntValue(oldValue_);
        }
        virtual void redo(){
            node_->setIntValue(newValue_);
        }
    private:
        CrIntegerNode* node_;
        int newValue_;
        int oldValue_;
    };

    auto spinBox = new QSpinBox();
    spinBox->setRange(min_, max_);
    spinBox->setValue(intValue());
    spinBox->setSuffix(suffix());
    connect(spinBox, &QSpinBox::editingFinished, this, [this, spinBox](){
        if(intValue() != spinBox->value()){
            if(project()){
                project()->pushUndoCommand(new ChangeIntValueCommand(this, spinBox->value()));
            }else {
                setIntValue(spinBox->value());
            }
        }
    });

    return spinBox;
}

QByteArray CrIntegerLimitedNode::saveState() const
{
    QByteArray state;
    QDataStream out(&state, QIODevice::WriteOnly);
    out << CrIntegerNode::saveState();
    out << min_ << max_;
    return state;
}

bool CrIntegerLimitedNode::restoreState(const QByteArray &state)
{
    QDataStream in(state);
    QByteArray nodeState;
    in >> nodeState;
    CrIntegerNode::restoreState(nodeState);
    int min, max;
    in >> min >> max;
    setRange(min, max);
    return true;
}



CrRealNode::CrRealNode(CrNode *parent) : CrNumericNode (parent) {}


CrRealNode::~CrRealNode() {}


int CrRealNode::intValue() const
{
    return static_cast<int>(value_);
}

void CrRealNode::setIntValue(int value)
{
    setRealValue(static_cast<qreal>(value));
}

qreal CrRealNode::realValue() const
{
    return value_;
}

void CrRealNode::setRealValue(qreal value)
{
    if(value_ == value)
        return;
    value_ = value;
    emit valueChanged();
    emit updateEditorPreview();
}

QWidget *CrRealNode::createCompactWidgetEditor()
{
    class ChangeRealValueCommand : public QUndoCommand
    {
    public:
        ChangeRealValueCommand(CrRealNode* node, qreal newValue , QUndoCommand *parent = nullptr) :
            QUndoCommand(parent), node_(node), newValue_(newValue){
            oldValue_ = node->realValue();
            setText(QString(QObject::tr("Node %1 change value %2 to %3.")).arg(node_->name()).arg(oldValue_).arg(newValue_));
        }
        virtual void undo(){
            node_->setRealValue(oldValue_);
        }
        virtual void redo(){
            node_->setRealValue(newValue_);
        }
    private:
        CrRealNode* node_;
        qreal newValue_;
        qreal oldValue_;
    };

    auto spinBox = new QDoubleSpinBox();
    spinBox->setRange(LONG_LONG_MIN, LONG_LONG_MAX);
    spinBox->setValue(value_);
    spinBox->setSuffix(suffix());
    connect(spinBox, &QDoubleSpinBox::editingFinished, this, [this, spinBox](){
        if(value_ != spinBox->value()){
            if(project()){
                project()->pushUndoCommand(new ChangeRealValueCommand(this, spinBox->value()));
            }else {
                setRealValue(spinBox->value());
            }
        }
    });

    return spinBox;
}

QSize CrRealNode::sizeHintEditorPreview(const QStyleOptionViewItem &option) const
{
    QStyleOptionViewItem opt(option);
    opt.text = QString::number(value_) + " " + suffix();
    return CrNode::sizeHintEditorPreview(opt);
}

void CrRealNode::paintEditorPreview(QPainter *painter, const QStyleOptionViewItem &option) const
{
    QStyleOptionViewItem opt(option);
    opt.text = QString::number(value_) + " " + suffix();
    CrNode::paintEditorPreview(painter, opt);
}

QByteArray CrRealNode::saveState() const
{
    QByteArray state;
    QDataStream out(&state, QIODevice::WriteOnly);
    out << CrNumericNode::saveState();
    out << value_;
    return state;
}

bool CrRealNode::restoreState(const QByteArray &state)
{
    QDataStream in(state);
    QByteArray nodeState;
    in >> nodeState;
    CrNumericNode::restoreState(nodeState);
    qreal value;
    in >> value;
    setRealValue(value);
    return true;
}

void CrRealNode::changeInputNode(CrNode *newNode)
{
    CrNumericNode::changeInputNode(newNode);
    if(newNode){
        setIntValue(static_cast<CrNumericNode*>(newNode)->intValue());
    }
}

void CrRealNode::changeInputNodeValue()
{
    setRealValue(static_cast<CrNumericNode*>(inputNode())->realValue());
}

CrRealLimitedNode::CrRealLimitedNode(CrNode *parent) : CrRealNode (parent) {}


CrRealLimitedNode::~CrRealLimitedNode() {}


void CrRealLimitedNode::setRealValue(qreal value)
{
    value = qBound(min_, value, max_);
    CrRealNode::setRealValue(value);
}

qreal CrRealLimitedNode::min() const
{
    return min_;
}

void CrRealLimitedNode::setMin(qreal min)
{
    setRange(min, max_);
}

qreal CrRealLimitedNode::max() const
{
    return max_;
}

void CrRealLimitedNode::setMax(qreal max)
{

    setRange(min_, max);
}

void CrRealLimitedNode::setRange(qreal min, qreal max)
{
    if(min_ == min && max_ == max)
        return;
    min_ = min;
    max_ = max;
    setRealValue(realValue());
}

QWidget *CrRealLimitedNode::createCompactWidgetEditor()
{
    class ChangeRealValueCommand : public QUndoCommand
    {
    public:
        ChangeRealValueCommand(CrRealNode* node, qreal newValue , QUndoCommand *parent = nullptr) :
            QUndoCommand(parent), node_(node), newValue_(newValue){
            oldValue_ = node->realValue();
            setText(QString(QObject::tr("Node %1 change value %2 to %3.")).arg(node_->name()).arg(oldValue_).arg(newValue_));
        }
        virtual void undo(){
            node_->setRealValue(oldValue_);
        }
        virtual void redo(){
            node_->setRealValue(newValue_);
        }
    private:
        CrRealNode* node_;
        qreal newValue_;
        qreal oldValue_;
    };

    auto spinBox = new QDoubleSpinBox();
    spinBox->setRange(min_, max_);
    spinBox->setValue(realValue());
    spinBox->setSuffix(suffix());
    connect(spinBox, &QDoubleSpinBox::editingFinished, this, [this, spinBox](){
        if(realValue() != spinBox->value()){
            if(project()){
                project()->pushUndoCommand(new ChangeRealValueCommand(this, spinBox->value()));
            }else {
                setRealValue(spinBox->value());
            }
        }
    });

    return spinBox;
}

QByteArray CrRealLimitedNode::saveState() const
{
    QByteArray state;
    QDataStream out(&state, QIODevice::WriteOnly);
    out << CrRealNode::saveState();
    out << min_ << max_;
    return state;
}

bool CrRealLimitedNode::restoreState(const QByteArray &state)
{
    QDataStream in(state);
    QByteArray nodeState;
    in >> nodeState;
    CrRealNode::restoreState(nodeState);
    qreal min, max;
    in >> min >> max;
    setRange(min, max);
    return true;
}




