#include "crprojectlogview.h"

#include <crproject.h>
#include <crnode.h>
#include <CrLog>

CrProjectLogView::CrProjectLogView(QWidget *parent) : QTreeWidget(parent)
{
    setHeaderLabels({tr("Name"), tr("Time"), tr("Type"), tr("Title"), tr("Text")});

    connect(this, &CrProjectLogView::projectChanged, this, [this](){
        this->clear();

        if(!project_)
            return;

        auto addMessageFunc = [this](CrLog::Message message){
            QTreeWidgetItem* item = new QTreeWidgetItem();
            item->setData(0, Qt::UserRole, QVariant::fromValue(message.node));
            if(message.node){
                item->setText(0, message.node->name());
                item->setIcon(0, message.node->icon());
            }else {
                item->setText(0, tr("None"));
            }
            item->setText(1, message.time_.toString(Qt::SystemLocaleLongDate));
            QString type;
            QColor background;
            switch (message.type) {
            case CrLog::Info:    type = tr("[Info]");    background = QColor("#f8fff2"); break;
            case CrLog::Warning: type = tr("[Warning]"); background = QColor("#f9f1ae"); break;
            case CrLog::Error:   type = tr("[Error]");   background = QColor("#f9b8ae"); break;
            }
            // item->setBackground(-1, background);
            item->setText(2, type);
            item->setText(3, message.title);
            item->setText(4, message.text);
            this->addTopLevelItem(item);
            scrollToBottom();
        };

        foreach(CrLog::Message message, project_->log()->messages()){
            addMessageFunc(message);
        }

        connect(project_->log(), QOverload<CrLog::Message>::of(&CrLog::messageAdded), this, addMessageFunc);

    });

    connect(this, QOverload<QTreeWidgetItem*, int>::of(&CrProjectLogView::itemDoubleClicked),
            this, [this](QTreeWidgetItem* item, int column){
        Q_UNUSED(column);

        int index = indexOfTopLevelItem(item);
        CrNode* node = project_->log()->messages().at(index).node;
        if(node){
            project_->setSelectedNodes({node});
        }
    });
}

CrProject *CrProjectLogView::project() const
{
    return project_;
}

void CrProjectLogView::setProject(CrProject *project)
{
    if(project_ == project)
        return;
    if(project_){
        disconnect(project_->log(), &CrLog::messageAdded, this, nullptr);
    }
    project_ = project;
    projectChanged();
}

