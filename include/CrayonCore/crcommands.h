#ifndef CRCOMMANDS_H
#define CRCOMMANDS_H

#include "crayoncore_global.h"
#include <QUndoCommand>

class CrNode;
namespace CrCommands {

class CRAYON_CORE_EXPORT MoveNode : public QUndoCommand
{
public:
    MoveNode(CrNode* childNode, CrNode* newParentNode, int newIndex = 0, QUndoCommand *parent = nullptr);
    virtual void undo();
    virtual void redo();
private:
    CrNode* childNode_;
    CrNode* oldParentNode_;
    CrNode* newParentNode_;
    int oldIndex_;
    int newIndex_;
};

class CRAYON_CORE_EXPORT ChangeName : public QUndoCommand
{
public:
    ChangeName(CrNode* node, const QString& newName, QUndoCommand *parent = nullptr);
    virtual void undo();
    virtual void redo();
private:
    CrNode* node_;
    QString oldName_;
    QString newName_;
};

class CRAYON_CORE_EXPORT ChangeComments : public QUndoCommand
{
public:
    ChangeComments(CrNode* node, const QString& newComments, QUndoCommand *parent = nullptr);
    virtual void undo();
    virtual void redo();
private:
    CrNode* node_;
    QString oldComments_;
    QString newComments_;
};


class CRAYON_CORE_EXPORT ChangeEnable : public QUndoCommand
{
public:
    ChangeEnable(CrNode* node, QUndoCommand *parent = nullptr);
    virtual void undo();
    virtual void redo();
private:
    CrNode* node_;
    bool enable_;
};


class CRAYON_CORE_EXPORT ChangeInputNode : public QUndoCommand
{
public:
    ChangeInputNode(CrNode* node, CrNode* inNode, QUndoCommand *parent = nullptr);
    virtual void undo();
    virtual void redo();
private:
    CrNode* node_;
    CrNode* inNode_;
    CrNode* oldInNode_;
};


}
#endif // CRCOMMANDS_H
