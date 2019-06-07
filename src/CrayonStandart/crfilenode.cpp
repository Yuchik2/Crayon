#include "crfilenode.h"

#include <QFileDialog>
#include <QUndoCommand>
#include <QStyleOptionViewItem>

#include <crproject.h>

CrFileNode::CrFileNode(CrNode *parent) : CrNode (parent) {}

CrFileNode::~CrFileNode() {}

QStringList CrFileNode::files() const
{
    return files_;
}

void CrFileNode::setFiles(const QStringList &files)
{
    if(files_ == files)
        return;

    files_ = files;
    emit filesChanged();
    emit updateEditorPreview();
}

QString CrFileNode::nameFilter() const
{
    return nameFilter_;
}

void CrFileNode::setNameFilter(const QString &nameFilter)
{
    if(nameFilter_ == nameFilter)
        return;

    nameFilter_ = nameFilter;
    emit nameFilterChanged();
}

bool CrFileNode::isValidInputNode(CrNode *inputNode) const
{
     return static_cast<bool>(qobject_cast<CrFileNode*>(inputNode));
}

QDialog *CrFileNode::createDialogEditor()
{
    class ChangeFilesCommand : public QUndoCommand
    {
    public:
        ChangeFilesCommand(CrFileNode* node, const QStringList& files , QUndoCommand *parent = nullptr) :
            QUndoCommand(parent), node_(node), newFiles_(files){
            oldFiles_ = node_->files_;
            setText(QString(QObject::tr("Node %1 change value %2 to %3.")).arg(node_->name()).arg(oldFiles_.join("; ").arg(newFiles_.join("; "))));
        }
        virtual void undo(){
            node_->setFiles(oldFiles_);
        }
        virtual void redo(){
            node_->setFiles(newFiles_);
        }
    private:
        CrFileNode* node_;
        QStringList newFiles_;
        QStringList oldFiles_;
    };

    auto fileDialog = new QFileDialog();
    fileDialog->setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog->setFileMode(QFileDialog::ExistingFile);
    fileDialog->setNameFilter(nameFilter_);
    if(!files_.isEmpty())
        fileDialog->selectFile(files_.first());

    connect(fileDialog, &QDialog::accepted, this, [this, fileDialog](){
        if(files_ != fileDialog->selectedFiles()){
            if(project()){
                project()->pushUndoCommand(new ChangeFilesCommand(this, fileDialog->selectedFiles()));
            }else {
                setFiles(fileDialog->selectedFiles());
            }
        }
    });

    return fileDialog;
}

QSize CrFileNode::sizeHintEditorPreview(const QStyleOptionViewItem &option) const
{
    QStyleOptionViewItem opt(option);
    opt.text = files_.join("; ");
    return CrNode::sizeHintEditorPreview(opt);
}

void CrFileNode::paintEditorPreview(QPainter *painter, const QStyleOptionViewItem &option) const
{
    QStyleOptionViewItem opt(option);
    opt.text = files_.join("; ");
    CrNode::paintEditorPreview(painter, opt);
}

QByteArray CrFileNode::saveState() const
{
    QByteArray state;
    QDataStream out(&state, QIODevice::WriteOnly);
    out << CrNode::saveState();
    out << files_;
    return state;
}

bool CrFileNode::restoreState(const QByteArray &state)
{
    QDataStream in(state);
    QByteArray nodeState;
    in >> nodeState;
    CrNode::restoreState(nodeState);
    QStringList files;
    in >> files;
    setFiles(files);
    return true;
}

void CrFileNode::changeInputNode(CrNode *newNode)
{
    if(inputNode()){
        CrFileNode* fontNode = static_cast<CrFileNode*>(inputNode());
        disconnect(fontNode, &CrFileNode::filesChanged, this, &CrFileNode::changeInputNodeValue);
    }

    if(newNode){
        CrFileNode* fontNode = static_cast<CrFileNode*>(newNode);
        connect(fontNode, &CrFileNode::filesChanged, this, &CrFileNode::changeInputNodeValue);
        setFiles(static_cast<CrFileNode*>(newNode)->files());
    }
}

void CrFileNode::changeInputNodeValue()
{
     setFiles(static_cast<CrFileNode*>(inputNode())->files());
}




