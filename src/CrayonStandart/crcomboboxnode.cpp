#include "crcomboboxnode.h"

#include <QComboBox>
#include <QUndoCommand>
#include <QStyleOptionViewItem>

#include <CrProject>

#include <QDebug>

struct CrComboBoxItem {
    QIcon icon;
    QString text;
    QVariant data;
};

CrComboBoxNode::CrComboBoxNode(CrNode *parent): CrNode(parent) {}

CrComboBoxNode::~CrComboBoxNode() {
    foreach(CrComboBoxItem* item, items_){
        delete item;
    }
}

void CrComboBoxNode::insertItem(int index, const QIcon &icon, const QString &text, const QVariant &data)
{
    items_.insert(index, new CrComboBoxItem{icon, text, data});
    if(currentIndex_ == -1)
        setCurrentIndex(0);
}

void CrComboBoxNode::setCurrentIndex(int index)
{
    if(currentIndex_ == index)
        return;

    if(index < 0 || index >= items_.size())
        return;

    currentIndex_ = index;
    emit updateEditorPreview();
    emit currentIndexChanged(currentIndex_);
}

void CrComboBoxNode::removeItem(int index)
{
    if(index < 0 || index >= items_.count())
        return;

    delete items_.at(index);
    items_.removeAt(index);
    if(items_.isEmpty()){
        currentIndex_ = -1;
        emit updateEditorPreview();
        emit currentIndexChanged(currentIndex_);
    }else if(index == currentIndex()){
        emit updateEditorPreview();
        emit currentIndexChanged(currentIndex_);
    }
}

QIcon CrComboBoxNode::icon(int index) const
{
    if(index < 0 || index >= items_.count())
        return QIcon();
    return items_.at(index)->icon;
}

QString CrComboBoxNode::text(int index) const
{
    if(index < 0 || index >= items_.count())
        return QString();
    return items_.at(index)->text;
}

QVariant CrComboBoxNode::data(int index) const
{
    if(index < 0 || index >= items_.count())
        return QVariant();
    return items_.at(index)->data;
}

QWidget *CrComboBoxNode::createCompactWidgetEditor()
{
    class ChangeCurrentIndexCommand : public QUndoCommand
    {
    public:
        ChangeCurrentIndexCommand(CrComboBoxNode* node, int index, QUndoCommand *parent = nullptr) :
            QUndoCommand(parent), node_(node), newIndex_(index){
            oldIndex_ = node_->currentIndex();
            setText(QString(QObject::tr("Node %1 change index %2 to %3")).arg(node_->name()).arg(oldIndex_).arg(newIndex_));
        }
        virtual void undo(){
            node_->setCurrentIndex(oldIndex_);
        }
        virtual void redo(){
            node_->setCurrentIndex(newIndex_);
        }
    private:
        CrComboBoxNode* node_;
        int newIndex_;
        int oldIndex_;
    };

    auto combo = new QComboBox();
    for (int i = 0; i < items_.count(); ++i) {
        combo->addItem(icon(i),text(i), data(i));
    }
    combo->setCurrentIndex(currentIndex_);

    connect(combo, QOverload<int>::of(&QComboBox::activated), this, [this](int index){
        if(currentIndex_ != index){
            if(project())
                project()->pushUndoCommand(new ChangeCurrentIndexCommand(this, index));
            else
                setCurrentIndex(index);
        }
    });

    return combo;
}

QSize CrComboBoxNode::sizeHintEditorPreview(const QStyleOptionViewItem &option) const
{
    QStyleOptionViewItem opt(option);
    opt.text = currentText();
    opt.icon = currentIcon();
    return CrNode::sizeHintEditorPreview(opt);
}

void CrComboBoxNode::paintEditorPreview(QPainter *painter, const QStyleOptionViewItem &option) const
{
    QStyleOptionViewItem opt(option);
    opt.text = currentText();
    opt.icon = currentIcon();
    CrNode::paintEditorPreview(painter, opt);
}

QByteArray CrComboBoxNode::saveState() const
{   
    QByteArray state;
    QDataStream out(&state, QIODevice::WriteOnly);
    out << CrNode::saveState();
    out << currentIndex_;
    return state;
}

bool CrComboBoxNode::restoreState(const QByteArray &state)
{    
    QDataStream in(state);
    QByteArray nodeState;
    in >> nodeState;
    CrNode::restoreState(nodeState);
    int currentIndex;
    in >> currentIndex;
    setCurrentIndex(currentIndex);
    return true;
}







