#include "crnodesettingsdialog.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QTabWidget>

#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QTreeWidget>

#include <QCheckBox>
#include <QPushButton>
#include <QToolButton>

#include <QDialogButtonBox>
#include <QApplication>
#include <QUndoCommand>
#include <QSettings>

#include <CrNode>
#include <CrProject>
#include <CrModule>
#include <crprojectexplorerdialog.h>

#include "crnodeeditorwidget.h"

#include <QDebug>

class CrNodeSettingsDialogPrivate {
public:
    CrNodeSettingsDialogPrivate(CrNodeSettingsDialog* dialog);
    CrNodeSettingsDialog* dialog_;
    CrNode* node_ = nullptr;

    QLineEdit* nameEditor_;
    CrNodeEditorWidget* nodeEditor_;

    QLabel* parentIconLabel_;
    QLabel* parentPathLabel_;
    CrNode* parentNode_ = nullptr;
    QToolButton* editParentNodeButton_;

    QTextEdit* commentsEditor_;
    QCheckBox* enableEditor_;

    QLabel* inputNodeIconLabel_;
    QLabel* inputNodePathLabel_;
    CrNode* inputNode_ = nullptr;
    QToolButton* editInputNodeButton_;

    QTreeWidget* outputNodesView_;
    QSet<CrNode*> outputNodes_;
    QHash<CrNode*, QTreeWidgetItem*> outputNodeItems_;
    QPushButton* addOutputNodeButton_;
    QPushButton* removeOutputNodeButton_;

    QLabel* pathLabel_;
    QLabel* moduleIconLabel_;
    QLabel* moduleNameLabel_;
    QLabel* projectIconLabel_;
    QLabel* projectNameLabel_;
    QLabel* flagsLabel_;
    QLabel* uuidLabel_;

public:
    void setParentNode(CrNode* node);
    void setInputNode(CrNode* node);
    void addOutputNode(CrNode* node);
    void removeOutputNode(CrNode* node);
};

CrNodeSettingsDialogPrivate::CrNodeSettingsDialogPrivate(CrNodeSettingsDialog *dialog)
    :dialog_(dialog){

}

void CrNodeSettingsDialogPrivate::setParentNode(CrNode *node)
{
    if(parentNode_ == node)
        return;
    parentNode_ = node;
    if(parentNode_){
        parentIconLabel_->setPixmap(parentNode_->icon().pixmap(24, 24));
        parentPathLabel_->setText(parentNode_->path());
    }else {
        parentIconLabel_->clear();
        parentPathLabel_->clear();
    }
}

void CrNodeSettingsDialogPrivate::setInputNode(CrNode *node)
{
    if(inputNode_ == node)
        return;
    inputNode_ = node;
    if(inputNode_){
        inputNodeIconLabel_->setPixmap(inputNode_->icon().pixmap(24, 24));
        inputNodePathLabel_->setText(inputNode_->path());
    }else {
        inputNodeIconLabel_->clear();
        inputNodePathLabel_->clear();
    }
}

void CrNodeSettingsDialogPrivate::addOutputNode(CrNode *node)
{
    if(outputNodes_.contains(node))
        return;

    QTreeWidgetItem* item = new QTreeWidgetItem();
    item->setData(0, Qt::UserRole, QVariant::fromValue(node));
    item->setText(0, node->name());

    QObject::connect(node, &CrNode::nameChanged, dialog_, [node, item]{
        item->setText(0, node->name());
    });

    item->setIcon(0, node->icon());
    QObject::connect(node, &CrNode::iconChanged, dialog_, [node, item]{
        item->setIcon(0, node->icon());
    });

    item->setText(1, node->path());
    QObject::connect(node, &CrNode::pathChanged, dialog_, [node, item]{
        item->setText(1, node->path());
    });

    item->setText(2, node->comments());
    QObject::connect(node, &CrNode::commentsChanged, dialog_, [node, item]{
        item->setText(2, node->comments());
    });
    outputNodes_.insert(node);
    outputNodesView_->addTopLevelItem(item);
    outputNodeItems_.insert(node, item);
}

void CrNodeSettingsDialogPrivate::removeOutputNode(CrNode *node)
{
    QObject::disconnect(node, &CrNode::nameChanged, dialog_, nullptr);
    QObject::disconnect(node, &CrNode::iconChanged, dialog_, nullptr);
    QObject::disconnect(node, &CrNode::pathChanged, dialog_, nullptr);
    QObject::disconnect(node, &CrNode::commentsChanged, dialog_, nullptr);
    outputNodes_.remove(node);
    delete outputNodeItems_.value(node);
    outputNodeItems_.remove(node);
}


CrNodeSettingsDialog::CrNodeSettingsDialog(QWidget *parent) :
    QDialog (parent),
    d_(new CrNodeSettingsDialogPrivate(this))
{
    setModal(true);
    setSizeGripEnabled(true);

    auto mainLayout = new QVBoxLayout();
    setLayout(mainLayout);

    auto mainTabWidget = new QTabWidget();
    mainLayout->addWidget(mainTabWidget);

    // General tab
    //

    auto generalTab = new QWidget();
    mainTabWidget->addTab(generalTab, tr("General"));

    auto generalTabLayuout = new QVBoxLayout();
    generalTab->setLayout(generalTabLayuout);

    auto generalFormLayout = new QFormLayout();
    generalTabLayuout->addLayout(generalFormLayout);

    d_->nameEditor_ = new QLineEdit();
    generalFormLayout->addRow(tr("Name:"), d_->nameEditor_);

    d_->nodeEditor_ = new CrNodeEditorWidget();
    generalFormLayout->addRow(tr("Editor:"), d_->nodeEditor_);

    auto parentEditorLayout = new QHBoxLayout();
    generalFormLayout->addRow(tr("Parent:"), parentEditorLayout);

    d_->parentIconLabel_ = new QLabel();
    parentEditorLayout->addWidget(d_->parentIconLabel_);

    d_->parentPathLabel_ = new QLabel();
    parentEditorLayout->addWidget(d_->parentPathLabel_);

    d_->editParentNodeButton_ = new QToolButton();
    parentEditorLayout->addWidget(d_->editParentNodeButton_);
    connect(d_->editParentNodeButton_, &QToolButton::clicked, this, [this]{

        if(!d_->node_)
            return;

        CrProjectExplorerDialog* explorer = CrProjectExplorerDialog::commonDialog();
        explorer->setProject(d_->node_->project());
        explorer->setFilter(CrProjectExplorerDialog::NoFilter);
        if(!explorer->exec())
            return;

        if(!explorer->selectedNodes().isEmpty()){
            d_->setParentNode(explorer->selectedNodes().first());
        }

    });

    auto commentsLabel = new QLabel(tr("Comments:"));
    generalTabLayuout->addWidget(commentsLabel);

    d_->commentsEditor_ = new QTextEdit();
    generalTabLayuout->addWidget(d_->commentsEditor_);

    d_->enableEditor_ = new QCheckBox();
    d_->enableEditor_->setText(tr("Enable"));
    generalTabLayuout->addWidget(d_->enableEditor_);


    // Connections tab
    //

    auto connectionsTab = new QWidget();
    mainTabWidget->addTab(connectionsTab, tr("Connections"));

    auto connectionsTabLayuout = new QVBoxLayout();
    connectionsTab->setLayout(connectionsTabLayuout);

    auto inputNodeGroupBox = new QGroupBox();
    connectionsTabLayuout->addWidget(inputNodeGroupBox);
    auto inputNodeGroupBoxLayout = new QHBoxLayout();
    inputNodeGroupBox->setLayout(inputNodeGroupBoxLayout);

    d_->inputNodeIconLabel_ = new QLabel();
    inputNodeGroupBoxLayout->addWidget(d_->inputNodeIconLabel_);

    d_->inputNodePathLabel_ = new QLabel();
    inputNodeGroupBoxLayout->addWidget(d_->inputNodePathLabel_);

    d_->editInputNodeButton_ = new QToolButton();
    inputNodeGroupBoxLayout->addWidget(d_->editInputNodeButton_);
    connect(d_->editInputNodeButton_, &QToolButton::clicked, this, [this]{
        CrProjectExplorerDialog* explorer = CrProjectExplorerDialog::commonDialog();
        explorer->setProject(d_->node_->project());
        explorer->setFilter(CrProjectExplorerDialog::ValidInputNode, d_->node_);
        if(explorer->exec()){
            if(explorer->selectedNodes().isEmpty()){
                d_->setInputNode(nullptr);
            }else {
                d_->setInputNode(explorer->selectedNodes().first());
            }
        }
    });

    auto outputNodesGroupBox = new QGroupBox();
    connectionsTabLayuout->addWidget(outputNodesGroupBox);
    auto outputNodesGroupBoxLayout = new QVBoxLayout();
    outputNodesGroupBox->setLayout(outputNodesGroupBoxLayout);

    auto outputNodesButtonsLayout = new QHBoxLayout();
    outputNodesGroupBoxLayout->addLayout(outputNodesButtonsLayout);

    d_->addOutputNodeButton_ = new QPushButton(tr("Add..."));
    outputNodesButtonsLayout->addWidget(d_->addOutputNodeButton_);
    connect(d_->addOutputNodeButton_, &QPushButton::clicked, this, [this]{
        CrProjectExplorerDialog* explorer = CrProjectExplorerDialog::commonDialog();
        explorer->setProject(d_->node_->project());
        explorer->setFilter(CrProjectExplorerDialog::ValidOutputNode, d_->node_);
        if(explorer->exec()){
            foreach(CrNode* node, explorer->selectedNodes()){
                d_->addOutputNode(node);
            }
        }
    });

    d_->removeOutputNodeButton_ = new QPushButton(tr("Remove"));
    outputNodesButtonsLayout->addWidget(d_->removeOutputNodeButton_);
    connect(d_->removeOutputNodeButton_, &QPushButton::clicked, this, [this]{
        foreach(QTreeWidgetItem* selectItem, d_->outputNodesView_->selectedItems()){
            CrNode* node = selectItem->data(0, Qt::UserRole).value<CrNode*>();
            d_->removeOutputNode(node);
        }
    });

    d_->outputNodesView_ = new QTreeWidget();
    outputNodesGroupBoxLayout->addWidget(d_->outputNodesView_);


    // Info tab
    //

    auto infoTab = new QWidget();
    mainTabWidget->addTab(infoTab, tr("Connections"));

    auto infoTabLayuout = new QFormLayout();
    infoTab->setLayout(infoTabLayuout);

    d_->pathLabel_ = new QLabel();
    infoTabLayuout->addRow(tr("Path:"), d_->pathLabel_);

    auto moduleLayout = new QHBoxLayout();
    d_->moduleIconLabel_ = new QLabel();
    moduleLayout->addWidget(d_->moduleIconLabel_);
    d_->moduleNameLabel_ = new QLabel();
    moduleLayout->addWidget(d_->moduleNameLabel_);
    infoTabLayuout->addRow(tr("Module:"), moduleLayout);

    auto projectLayout = new QHBoxLayout();
    d_->projectIconLabel_ = new QLabel();
    projectLayout->addWidget(d_->projectIconLabel_);
    d_->projectNameLabel_ = new QLabel();
    projectLayout->addWidget(d_->projectNameLabel_);
    infoTabLayuout->addRow(tr("Project:"), projectLayout);

    d_->flagsLabel_ = new QLabel();
    infoTabLayuout->addRow(tr("Flags:"), d_->flagsLabel_);

    d_->uuidLabel_ = new QLabel();
    infoTabLayuout->addRow(tr("UUID:"), d_->uuidLabel_);

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    mainLayout->addWidget(buttonBox);

    connect(this, &QDialog::accepted, this, [this](){
        if(!d_->node_)
            return;

        QString name = d_->nameEditor_->text().simplified();
        QString comments = d_->commentsEditor_->toPlainText().simplified();

        if(name == d_->node_->name()
                && d_->parentNode_ == d_->node_->parentNode()
                && comments == d_->node_->comments()
                && d_->inputNode_ == d_->node_->inputNode()
                && d_->outputNodes_ == d_->node_->outputNodes().toSet())
            return;

        class ChangeNodeSettingsCommand : public QUndoCommand
        {
        public:
            ChangeNodeSettingsCommand(CrNode* node, const QString& name, CrNode* parentNode, const QString& comments,
                                      CrNode* inputNode, QSet<CrNode*> outputNodes, QUndoCommand *parent = nullptr) :
                QUndoCommand(parent), node_(node), newName_(name), newParentNode_(parentNode), newComments_(comments),
                newInputNode_(inputNode), newOutputNodes_(outputNodes){

                oldName_ = node_->name();
                oldParentNode_ = node_->parentNode();
                oldComments_ = node_->comments();
                oldInputNode_ = node_->inputNode();
                oldOutputNodes_ = node_->outputNodes().toSet();
                setText(QString(QObject::tr("Node %1 change settings")).arg(node_->name()));
            }
            virtual void undo(){
                node_->setName(oldName_);
                node_->setParentNode(oldParentNode_);
                node_->setComments(oldComments_);
                node_->setInputNode(oldInputNode_);
                node_->removeOutputNodes((newOutputNodes_ - oldOutputNodes_).toList());
                node_->addOutputNodes((oldOutputNodes_ - newOutputNodes_).toList());
            }
            virtual void redo(){
                node_->setName(newName_);
                node_->setParentNode(newParentNode_);
                node_->setComments(newComments_);
                node_->setInputNode(newInputNode_);
                node_->removeOutputNodes((oldOutputNodes_ - newOutputNodes_).toList());
                node_->addOutputNodes((newOutputNodes_ - oldOutputNodes_).toList());
            }
        private:
            CrNode* node_;

            QString oldName_;
            QString newName_;

            CrNode* oldParentNode_;
            CrNode* newParentNode_;

            QString oldComments_;
            QString newComments_;

            CrNode* oldInputNode_;
            CrNode* newInputNode_;

            QSet<CrNode*> oldOutputNodes_;
            QSet<CrNode*> newOutputNodes_;

        };
        if(d_->node_->project()){
            d_->node_->project()->pushUndoCommand(new ChangeNodeSettingsCommand(d_->node_, name, d_->parentNode_, comments, d_->inputNode_, d_->outputNodes_));
        }else {
            d_->node_->setName(name);
            d_->node_->setParentNode(d_->parentNode_);
            d_->node_->setComments(comments);
            d_->node_->setInputNode(d_->inputNode_);
            d_->node_->removeOutputNodes((d_->outputNodes_ - d_->node_->outputNodes().toSet()).toList());
            d_->node_->addOutputNodes((d_->node_->outputNodes().toSet() - d_->outputNodes_).toList());
        }
    });
}

CrNodeSettingsDialog::~CrNodeSettingsDialog()
{
    delete d_;
}

CrNodeSettingsDialog *CrNodeSettingsDialog::commonDialog()
{
    static CrNodeSettingsDialog* settingsDialog = nullptr;
    if(!settingsDialog){
        settingsDialog = new CrNodeSettingsDialog();
        QSettings settings("CrayonEnt", "Crayon");
        settingsDialog->restoreGeometry(settings.value("projectExplorerSettingsDialogGeom").toByteArray());
        connect(QApplication::instance(), &QApplication::aboutToQuit, settingsDialog, [](){
            QSettings settings("CrayonEnt", "Crayon");
            settings.setValue("projectExplorerSettingsDialogGeom", settingsDialog->saveGeometry());
        });
    }
    return settingsDialog;
}

CrNode *CrNodeSettingsDialog::node() const
{
    return d_->node_;
}

void CrNodeSettingsDialog::setNode(CrNode *node)
{
    if(d_->node_ == node)
        return;

    if(d_->node_){
        disconnect(d_->node_, &CrNode::nameChanged, this, nullptr);
        disconnect(d_->node_, &CrNode::parentNodeChanged, this, nullptr);
        disconnect(d_->node_, &CrNode::commentsChanged,  this, nullptr);
        disconnect(d_->node_, &CrNode::enableChanged,  this, nullptr);
        disconnect(d_->node_, &CrNode::inputNodeChanged, this, nullptr);
        foreach(CrNode* node, d_->node_->outputNodes()){
            d_->removeOutputNode(node);
        }
        disconnect(d_->node_, &CrNode::outputNodeAdded, this, nullptr);
        disconnect(d_->node_, &CrNode::outputNodeRemoved, this, nullptr);
        disconnect(d_->node_, &CrNode::pathChanged, this, nullptr);
        disconnect(d_->node_, &CrNode::projectChanged, this, nullptr);
        disconnect(d_->node_, &CrNode::flagsChanged, this, nullptr);
        disconnect(d_->node_, &CrNode::uuidChanged, this, nullptr);
    }

    d_->node_ = node;

    auto updateFlags = [this]{

        if(!d_->node_){
            d_->nameEditor_->setEnabled(false);
            d_->nodeEditor_->setEnabled(false);
            d_->editInputNodeButton_->setEnabled(false);
            d_->addOutputNodeButton_->setEnabled(false);
            d_->removeOutputNodeButton_->setEnabled(false);
            d_->outputNodesView_->setEnabled(false);

            d_->flagsLabel_->setText(tr("<None>"));
            return;
        }

        QString flags;
        if(d_->node_->flags() & CrNode::EditableName){
            flags += " | " + tr("Editable name");
        }
        d_->nameEditor_->setEnabled(d_->node_->flags() & CrNode::EditableName);


        if(d_->node_->flags() & CrNode::ActiveEditor){
            flags += " | " + tr("Editable value");
        }
        d_->nodeEditor_->setEnabled(d_->node_->flags() & CrNode::ActiveEditor);

        if(d_->node_->flags() & CrNode::In){
            flags += " | " + tr("Input");
        }
        d_->editInputNodeButton_->setEnabled(d_->node_->flags() & CrNode::In);

        if(d_->node_->flags() & CrNode::Out){
            flags += " | " + tr("Output");
        }
        d_->addOutputNodeButton_->setEnabled(d_->node_->flags() & CrNode::Out);
        d_->removeOutputNodeButton_->setEnabled(d_->node_->flags() & CrNode::Out);
        d_->outputNodesView_->setEnabled(d_->node_->flags() & CrNode::Out);

        if(d_->node_->flags() & CrNode::Invisible)
            flags += " | " + tr("Invisible");

        if(d_->node_->flags() & CrNode::Retain)
            flags += " | " + tr("Retain");

        if(!flags.isEmpty())
            flags.remove(0, 3);

        d_->flagsLabel_->setText(flags);

    };

    if(d_->node_){
        d_->nameEditor_->setText(d_->node_->name());
        connect(d_->node_, &CrNode::nameChanged, this, [this]{
            d_->nameEditor_->setText(d_->node_->name());
        });

        d_->setParentNode(d_->node_->parentNode());
        connect(d_->node_, &CrNode::parentNodeChanged, this, [this]{
            d_->setParentNode(d_->node_->parentNode());
        });

        d_->commentsEditor_->setText(d_->node_->comments());
        connect(d_->node_, &CrNode::commentsChanged, this, [this]{
            d_->commentsEditor_->setText(d_->node_->comments());
        });

        d_->enableEditor_->setChecked(d_->node_->isEnable());
        connect(d_->node_, &CrNode::enableChanged, this, [this]{
            d_->enableEditor_->setChecked(d_->node_->isEnable());
        });

        d_->setInputNode(d_->node_->inputNode());
        connect(d_->node_, &CrNode::inputNodeChanged, this, [this]{
            d_->setInputNode(d_->node_->inputNode());
        });

        foreach(CrNode* node, d_->node_->outputNodes()){
            d_->addOutputNode(node);
        }

        connect(d_->node_, QOverload<CrNode*>::of(&CrNode::outputNodeAdded), this, [this](CrNode* node){
            d_->addOutputNode(node);
        });

        connect(d_->node_, QOverload<CrNode*>::of(&CrNode::outputNodeRemoved), this, [this](CrNode* node){
            d_->removeOutputNode(node);
        });

        d_->pathLabel_->setText(d_->node_->path());
        connect(d_->node_, &CrNode::pathChanged, this, [this]{
            d_->pathLabel_->setText(d_->node_->path());
        });

        if(d_->node_->module()){
            d_->moduleIconLabel_->setPixmap(d_->node_->module()->icon().pixmap(24, 24));
            d_->moduleNameLabel_->setText(d_->node_->module()->path());
        }else {
            d_->moduleIconLabel_->clear();
            d_->moduleNameLabel_->setText(tr("<None>"));
        }

        if(d_->node_->project()){
            d_->projectNameLabel_->setText(d_->node_->project()->name());
        }else {
            d_->projectNameLabel_->setText(tr("<None>"));
        }
        connect(d_->node_, &CrNode::projectChanged, this, [this]{
            if(d_->node_->project()){
                d_->projectNameLabel_->setText(d_->node_->project()->name());
            }else {
                d_->projectNameLabel_->setText(tr("<None>"));
            }
        });

        connect(d_->node_, &CrNode::flagsChanged, this, [updateFlags]{
            updateFlags();
        });

        d_->uuidLabel_->setText(d_->node_->uuid().toString());
        connect(d_->node_, &CrNode::uuidChanged, this, [this]{
            d_->uuidLabel_->setText(d_->node_->uuid().toString());
        });

    }else {

        d_->nameEditor_->clear();
        d_->setParentNode(nullptr);
        d_->commentsEditor_->clear();
        d_->enableEditor_->setChecked(false);
        d_->setInputNode(nullptr);
        d_->pathLabel_->clear();
        d_->moduleIconLabel_->clear();
        d_->moduleNameLabel_->clear();
        d_->projectNameLabel_->clear();
        d_->uuidLabel_->clear();
    }

    updateFlags();

    emit nodeChanged();
}


