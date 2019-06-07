#include "crcommands.h"
#include <CrNode>

CrCommands::MoveNode::MoveNode(CrNode *childNode, CrNode *newParentNode, int newIndex, QUndoCommand *parent) :
    QUndoCommand(parent), childNode_(childNode), newParentNode_(newParentNode), newIndex_(newIndex){
    oldParentNode_ = childNode_->parentNode();
    oldIndex_ = oldParentNode_ ? oldParentNode_->childNodes().indexOf(childNode_) : 0;
    if(newParentNode_){
        setText(QString(QObject::tr("Move node %1 to %2")).arg(childNode_->name()).arg(newParentNode_->name()));
    }else {
        setText(QString(QObject::tr("Remove node %1")).arg(childNode_->name()));
    }
}

void CrCommands::MoveNode::undo(){
    if(oldParentNode_){
        oldParentNode_->insertChildNode(oldIndex_, childNode_);
    }else {
        childNode_->setParentNode(nullptr);
    }
}

void CrCommands::MoveNode::redo(){
    if(newParentNode_){
        newParentNode_->insertChildNode(newIndex_, childNode_);
    }else {
        childNode_->setParentNode(nullptr);
    }
}

CrCommands::ChangeName::ChangeName(CrNode *node, const QString &newName, QUndoCommand *parent) :
    QUndoCommand(parent), node_(node), newName_(newName){
    oldName_ = node_->name();
    setText(QString(QObject::tr("Node %1 change name %2 to %3")).arg(node_->name()).arg(oldName_).arg(newName_));
}

void CrCommands::ChangeName::undo(){
    node_->setName(oldName_);
}

void CrCommands::ChangeName::redo(){
    node_->setName(newName_);
}


CrCommands::ChangeComments::ChangeComments(CrNode *node, const QString &newComments, QUndoCommand *parent) :
    QUndoCommand(parent), node_(node), newComments_(newComments){
    oldComments_ = node_->comments();
    setText(QString(QObject::tr("Node %1 change comments %2 to %3")).arg(node_->name()).arg(oldComments_).arg(newComments_));
}

void CrCommands::ChangeComments::undo(){
    node_->setComments(oldComments_);
}

void CrCommands::ChangeComments::redo(){
    node_->setComments(newComments_);
}


CrCommands::ChangeEnable::ChangeEnable(CrNode *node, QUndoCommand *parent) :
    QUndoCommand(parent), node_(node), enable_(node->isEnable()){
    setText(QString(QObject::tr("Node %1 change enable")).arg(node_->name()));
}

void CrCommands::ChangeEnable::undo(){
    node_->setEnable(enable_);
}

void CrCommands::ChangeEnable::redo(){
    node_->setEnable(!enable_);
}



CrCommands::ChangeInputNode::ChangeInputNode(CrNode *node, CrNode *inNode, QUndoCommand *parent) :
    QUndoCommand(parent), node_(node), inNode_(inNode){
    oldInNode_ = node_->inputNode();
    setText(QString(QObject::tr("Node %1 change inputNode to %2")).arg(node_->name()).arg(inNode_ ? inNode_->name() : "Null"));
}

void CrCommands::ChangeInputNode::undo(){
    node_->setInputNode(oldInNode_);
}

void CrCommands::ChangeInputNode::redo(){
    node_->setInputNode(inNode_);
}



