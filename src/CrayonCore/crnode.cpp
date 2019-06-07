#include "crnode.h"

#include <QStringBuilder>
#include <QApplication>
#include <QStyle>
#include <QWidget>
#include <QStyleOptionViewItem>
#include <QStack>

#include <CrProject>
#include <CrModule>

#include <QDebug>

class CrNodePrivate {
public:
    QUuid uuid_ = QUuid::createUuid();
    QString name_;
    QString comments_;
    CrNode::Flags flags_ = CrNode::NoFlags;

    QIcon icon_;
    bool enable_ = true;
    CrModule* module_ = nullptr;

    QList<CrNode*> childNodes_;
    CrNode* parentNode_ = nullptr;
    CrProject* project_ = nullptr;

    CrNode* inputNode_ = nullptr;
    QList<CrNode*> outputNodes_;
};

CrNode::CrNode(CrNode *parent) :
    QObject (nullptr),
    d_(new CrNodePrivate)
{
    setParentNode(parent);
}

CrNode::CrNode(CrProject *project) :
    QObject (nullptr),
    d_(new CrNodePrivate)
{
    d_->project_ = project;
}

void CrNode::setModule(CrModule *module)
{
    d_->module_ = module;
}

CrNode::~CrNode()
{
    setInputNode(nullptr);
    clearOutputNodes();

    foreach (CrNode* child, d_->childNodes_) {
        delete child;
    }

    setParentNode(nullptr);

    delete d_;
}

QUuid CrNode::uuid() const { return d_->uuid_;}

void CrNode::setUuid(const QUuid& uuid)
{
    if(d_->uuid_ == uuid)
        return;
    if(d_->project_){
        if(d_->project_->findNodeByUuid(uuid))
            return;
        d_->project_->unregisterNodeUuid(d_->uuid_);
        d_->project_->registerNodeUuid(uuid, this);
    }
    d_->uuid_ = uuid;
    uuidChanged();
}

QString CrNode::name() const { return d_->name_; }

void CrNode::setName(const QString &name)
{
    QString newName = name.simplified();
    if(d_->name_ == newName) return;

    if(d_->parentNode_){
        QStringList childNames;
        foreach(CrNode* child, d_->parentNode_->childNodes())
            childNames << child->d_->name_;
        QString tempName = newName;
        int i = 1;
        while (true) {
            if(childNames.contains(newName)){
                ++i;
                newName = QString("%1_%2").arg(tempName).arg(i);
                continue;
            }
            break;
        }
    }

    d_->name_ = newName;
    emit nameChanged();
    emit pathChanged();
    foreach(CrNode* child, d_->childNodes_){
        emit child->pathChanged();
    }
}

QString CrNode::path()
{
    QString pt = d_->name_;
    CrNode* parentNode = d_->parentNode_;
    while (parentNode) {
        pt = parentNode->name() % QStringLiteral("/") % pt;
        parentNode = parentNode->d_->parentNode_;
    }
    return pt;
}

QString CrNode::comments() const { return d_->comments_; }

void CrNode::setComments(const QString &toolTip)
{
    QString tempToolTip = toolTip.trimmed();
    if(d_->comments_ == tempToolTip) return;
    d_->comments_ = tempToolTip;
    emit commentsChanged();
}

CrNode::Flags CrNode::flags() const { return d_->flags_; }

void CrNode::setFlags(CrNode::Flags nodeFlags)
{
    if(d_->flags_ == nodeFlags) return;
    d_->flags_ = nodeFlags;
    emit flagsChanged();
}

QIcon CrNode::icon() const
{
    if(d_->icon_.isNull() && d_->module_){
        return QIcon(d_->module_->icon());
    }
    return d_->icon_;
}

void CrNode::setIcon(const QIcon &icon)
{ 
    d_->icon_ = icon;
    emit iconChanged();
}

bool CrNode::isEnable() const
{
    return d_->enable_;
}

void CrNode::setEnable(bool enable)
{
    if(d_->enable_ == enable)
        return;

    d_->enable_ = enable;
    emit enableChanged();
}

QList<CrNode *> CrNode::childNodes() const { return d_->childNodes_; }

bool CrNode::insertChildNode(int index, CrNode * node)
{   
    if(!node || node == this)
        return false;

    if(index < 0){
        index = d_->childNodes_.size();
    }

    CrNode* testParent = d_->parentNode_;
    while (testParent) {
        if(testParent == node)
            return false;
        testParent = testParent->d_->parentNode_;
    }

    node->setSelected(false);

    CrNode* oldParentNode = node->d_->parentNode_;
    int oldIndex = 0;
    if(oldParentNode){
        oldIndex =  oldParentNode->d_->childNodes_.indexOf(node);

        if(oldParentNode == this){
            if(oldIndex == index)
                return false;
            if(oldIndex < index)
                --index;
        }

        oldParentNode->d_->childNodes_.removeAt(oldIndex);
        emit oldParentNode->childNodeRemoved(oldIndex, node);

    }

    node->d_->parentNode_ = this;

    QStringList childNames;
    foreach(CrNode* child, childNodes())
        childNames << child->d_->name_;

    QString tempName = node->d_->name_;
    int i = 1;
    while (true) {
        if(childNames.contains(tempName)){
            ++i;
            tempName = QString("%1_%2").arg(node->d_->name_).arg(i);
            continue;
        }
        break;
    }
    node->setName(tempName);

    if(index > d_->childNodes_.size())
        index = d_->childNodes_.size();

    d_->childNodes_.insert(index, node);

    if(node->d_->project_ != d_->project_){
        QStack<CrNode*> stack;
        stack << node;
        while (!stack.isEmpty()) {
            CrNode* node = stack.pop();
            if(node->d_->project_){
                node->d_->project_->unregisterNodeUuid(node->d_->uuid_);
            }
            node->d_->project_ = d_->project_;
            if(d_->project_){
                if(d_->project_->findNodeByUuid(node->d_->uuid_)){
                    node->d_->uuid_ = QUuid::createUuid();
                    node->uuidChanged();
                }
                d_->project_->registerNodeUuid(node->d_->uuid_, node);
            }
            emit node->projectChanged();
            stack << node->childNodes().toVector();
        }
    }

    if(oldParentNode != this){
        emit node->parentNodeChanged();
        emit node->pathChanged();
        foreach(CrNode* child, node->d_->childNodes_){
            emit child->pathChanged();
        }
    }

    emit childNodeInserted(index, node);

    return true;
}

bool CrNode::appendChildNode(CrNode *node){ return insertChildNode(d_->childNodes_.size(), node); }

bool CrNode::removeChildNode(CrNode *node) {

    int index = d_->childNodes_.indexOf(node);

    if(index == -1)
        return false;

    node->setSelected(false);
    node->d_->parentNode_ = nullptr;
    if(node->d_->project_){
        QStack<CrNode*> stack;
        stack << node;
        while (!stack.isEmpty()) {
            CrNode* node = stack.pop();
            if(node->d_->project_){
                node->d_->project_->unregisterNodeUuid(node->d_->uuid_);
            }
            node->d_->project_ = nullptr;
            emit node->projectChanged();
            stack << node->childNodes().toVector();
        }
    }
    d_->childNodes_.removeAt(index);
    emit node->parentNodeChanged();
    emit node->pathChanged();
    foreach(CrNode* child, node->d_->childNodes_){
        emit child->pathChanged();
    }
    emit childNodeRemoved(index, node);

    return true;
}

void CrNode::clearChildNodes(){
    foreach (CrNode* child, d_->childNodes_) {
        removeChildNode(child);
    }
}

CrNode *CrNode::parentNode() const { return d_->parentNode_; }

bool CrNode::setParentNode(CrNode *parentNode)
{   
    if(d_->parentNode_ == parentNode)
        return false;

    if(parentNode){
        return parentNode->appendChildNode(this);
    }else{
        return d_->parentNode_->removeChildNode(this);
    }
}

CrNode *CrNode::rootNode() const
{
    CrNode* parentNode = const_cast<CrNode*>(this);
    while(true){
        if(parentNode->parentNode())
            parentNode = parentNode->parentNode();
        else
            break;
    }
    return parentNode;
}

CrProject *CrNode::project() const { return d_->project_;}

bool CrNode::isFocus() const
{
    return d_->project_ ? d_->project_->focusNode() == this : false;
}

void CrNode::setFocus(bool focus)
{
    if(!d_->project_)
        return;

    if(isFocus() == focus)
        return;

    if(focus){
        d_->project_->setFocusNode(this);
    }
    else {
        d_->project_->setFocusNode(nullptr);
    }
}

bool CrNode::isSelected() const {
    return selected_;
}

void CrNode::setSelected(bool selected)
{
    if(selected_ == selected)
        return;

    selected_ = selected;
    emit selectedChanged();

    if(!d_->project_)
        return;

    if(selected_)
        d_->project_->selectedNodes_ << this;

    emit d_->project_->selectedChanged();


}

CrNode *CrNode::inputNode() const
{
    return d_->inputNode_;
}

bool CrNode::setInputNode(CrNode *inNode)
{
    if(d_->inputNode_ == inNode || (inNode && inNode->d_->project_ != d_->project_))
        return false;


    if(inNode && !isValidInputNode(inNode))
        return false;

    if(d_->inputNode_){
        d_->inputNode_->d_->outputNodes_.removeOne(this);
        emit d_->inputNode_->outputNodeRemoved(this);
    }

    changeInputNode(inNode);
    d_->inputNode_ = inNode;

    if(d_->inputNode_){
        d_->inputNode_->d_->outputNodes_.append(this);
        emit d_->inputNode_->outputNodeAdded(this);
    }
    inputNodeChanged();
    return true;
}

bool CrNode::isValidInputNode(CrNode *inputNode) const
{
    Q_UNUSED(inputNode)
    return false;
}

QList<CrNode *> CrNode::outputNodes() const { return d_->outputNodes_;}

void CrNode::clearOutputNodes() {
    foreach (CrNode* outNode, d_->outputNodes_) {
        removeOutputNode(outNode);
    }
}

bool CrNode::isValidOunputNode(CrNode *outputNode) const
{
    Q_UNUSED(outputNode)
    return true;
}

bool CrNode::isModule() const { return static_cast<bool>(d_->module_);}

CrModule *CrNode::module() const { return  d_->module_;}

QByteArray CrNode::saveState() const
{
    QByteArray state;
    QDataStream out(&state, QIODevice::WriteOnly);

    out << d_->uuid_
        << d_->name_
        << d_->comments_
        << d_->flags_
        << d_->enable_;

    out << d_->childNodes_.count();

    foreach(CrNode* child, d_->childNodes_){
        out << child->saveState();
        if(child->d_->module_){
            out << true;
            out << d_->uuid_;
            out << child->d_->module_->key();
        }else {
            out << false;
        }
    }

    return state;
}

bool CrNode::restoreState(const QByteArray &state)
{
    QDataStream in(state);

    QUuid uuid;
    in >> uuid;
    setUuid(uuid);

    QString name, comments;
    in  >> name >> comments;
    setName(name);
    setComments(comments);

    Flags flags;
    in >> flags;
    setFlags(flags);

    bool enable;
    in >> enable;
    setEnable(enable);

    int childrenCount;
    in >> childrenCount;

    for (int i = 0; i < childrenCount; ++i) {
        CrNode* child;

        QByteArray childnState;
        in >> childnState;
        bool isModule;
        in >> isModule;

        if(isModule){
            QUuid uuid;
            in >> uuid;
            child = d_->childNodes_.value(i, nullptr);
            if(child && child->uuid() == uuid){
                child->restoreState(childnState);
                continue;
            }
            CrModuleKey moduleKey;

            in >> moduleKey;

            CrModule* module = CrModule::module(moduleKey);
            if(!module){
                --childrenCount;
                --i;
                d_->project_->pushErrorMessage(QObject::tr("Child loading error."),
                                               QObject::tr("Failed to load modular child."
                                                           "The module by id: \"%1\" version: \"%2\" not found.").
                                               arg(moduleKey.id()).arg(moduleKey.version().toString()), this);
                continue;
            }

            child = CrModule::createNode(moduleKey);

            if(!child){
                --childrenCount;
                --i;
                if(d_->project_){
                    d_->project_->pushErrorMessage(QObject::tr("Child loading error."),
                                                   QObject::tr("Failed to load modular child."
                                                               "The module by id: \"%1\" version: \"%2\" contains error.").
                                                   arg(moduleKey.id()).arg(moduleKey.version().toString()), this);
                    continue;
                }
            }
            insertChildNode(i, child);
        }else{
            child = d_->childNodes_.value(i, nullptr);
            if(!child && d_->project_){
                d_->project_->pushErrorMessage(QObject::tr("Child loading error."),
                                               QObject::tr("Incorrect amount of descendants in the loaded node."
                                                           "Expected amount: %1; Present amount: %2 ").
                                               arg(childrenCount).arg(d_->childNodes_.count()), this);
                break;
            }
        }

        child->restoreState(childnState);

    }

    if(childrenCount != d_->childNodes_.count()){
        if(d_->project_){
            d_->project_->pushErrorMessage(QObject::tr("Child loading error."),
                                           QObject::tr("Incorrect number of descendants in the loaded node."
                                                       "Expected amount: %1; Present amount: %2 ").
                                           arg(childrenCount).arg(d_->childNodes_.count()), this);
        }
    }
    return true;
}

void CrNode::changeInputNode(CrNode *newNode)
{
    Q_UNUSED(newNode);
}


CrNode *CrNode::createCopy() const
{
    if(!d_->module_ || !d_->project_)
        return nullptr;

    CrNode* copy =d_-> module_->createNode();

    if(!copy)
        return nullptr;

    copy->restoreState(saveState());
    copy->setName(d_->name_ + "_copy");

    return copy;
}

void CrNode::pushInfoMessage(const QString &title, const QString &text)
{
    if(d_->project_){
        d_->project_->pushInfoMessage(title, text, this);
    }
}

void CrNode::pushWarningMessage(const QString &title, const QString &text)
{
    if(d_->project_){
        d_->project_->pushWarningMessage(title, text, this);
    }
}

void CrNode::pushErrorMessage(const QString &title, const QString &text)
{
    if(d_->project_){
        d_->project_->pushErrorMessage(title, text, this);
    }
}

QWidget *CrNode::createCompactWidgetEditor()
{
    return nullptr;
}

QWidget *CrNode::createWidgetEditor()
{
    return nullptr;
}

QDialog *CrNode::createDialogEditor()
{
    return nullptr;
}

QWindow *CrNode::createWindowEditor()
{
    return nullptr;
}

void CrNode::requestOpenEditor()
{
    if(d_->project_){
        d_->project_->requestOpenEditor(this);
    }
}

bool CrNode::eventEditorPreview(QEvent *event, const QStyleOptionViewItem &option)
{
    Q_UNUSED(event)
    Q_UNUSED(option)
    return false;
}

QSize CrNode::sizeHintEditorPreview(const QStyleOptionViewItem &option) const
{
    const QWidget *widget = option.widget;
    QStyle *style = widget ? widget->style() : QApplication::style();
    return style->sizeFromContents(QStyle::CT_ItemViewItem, &option, QSize(), widget);
}

void CrNode::paintEditorPreview(QPainter *painter, const QStyleOptionViewItem &option) const
{
    const QWidget *widget = option.widget;
    QStyle *style = widget ? widget->style() : QApplication::style();
    style->drawControl(QStyle::CE_ItemViewItem, &option, painter, widget);
}

QDataStream &operator<<(QDataStream &out, const CrNode* node)
{   
    out << node->saveState();
    return out;
}

QDataStream &operator>>(QDataStream &in, CrNode* node)
{  
    QByteArray nodeState;
    in >> nodeState;
    node->restoreState(nodeState);
    return in;
}

