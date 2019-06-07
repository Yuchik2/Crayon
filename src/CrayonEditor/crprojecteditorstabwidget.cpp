#include "crprojecteditorstabwidget.h"

#include <QTabBar>
#include <QStackedWidget>
#include <QVBoxLayout>

#include <QMouseEvent>
#include <QApplication>
#include <QLabel>
#include <QDrag>
#include <QCursor>


#include <crproject.h>
#include <CrNode>

#include <QDebug>

CrProjectEditorsTabWidget::CrProjectEditorsTabWidget(CrProject *project, QWidget *parent) :
    QWidget(parent),
    project_(project),
    tabBar_(new QTabBar()),
    body_(new QStackedWidget()),
    dragPreview_(new QLabel())
{
    auto layout = new QVBoxLayout();
    layout->setMargin(0);
    tabBar_->setTabsClosable(true);
    tabBar_->setMovable(true);
    tabBar_->setExpanding(false);
    tabBar_->installEventFilter(this);
    layout->addWidget(tabBar_);

    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    layout->addWidget(body_);

    setLayout(layout);

    connect(project_, QOverload<CrNode*>::of(&CrProject::requestOpenEditor),
            this, QOverload<CrNode*>::of(&CrProjectEditorsTabWidget::openEditor));

    //    connect(project_, &CrProject::focusNodeChanged, this, [this](){

    //        QWidget* editor = editors_.value(project_->focusNode(), nullptr);
    //        if(!editor)
    //            return;
    //        int index = indexOf(editor);

    //        if(index == -1){
    //            editor->show();
    //            editor->activateWindow();
    //        }else {
    //            setCurrentIndex(index);
    //        }
    //    });

    connect(tabBar_, QOverload<int>::of(&QTabBar::currentChanged),
            this, [this](int index){
        //        if(index < 0)
        //            return;
        body_->setCurrentIndex(index);
    });
    connect(tabBar_, QOverload<int, int>::of(&QTabBar::tabMoved),
            this, [this](int from, int to){
       QWidget* editor = body_->widget(from);
       body_->removeWidget(editor);
       body_->insertWidget(to, editor);
       tabNodes_.swap(from, to);
    });

    connect(tabBar_, QOverload<int>::of(&QTabBar::tabCloseRequested),
            this, [this](int index){
        closeEditor(tabNodes_.at(index));

    });
}

CrProjectEditorsTabWidget::~CrProjectEditorsTabWidget()
{

    foreach(CrNode* node, editors_.keys()){
        closeEditor(node);
    }
    delete dragPreview_;
}

bool CrProjectEditorsTabWidget::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == tabBar_){

        switch (event->type()) {
        case QEvent::MouseMove:{
            if(startDrag_){
                dragPreview_->move(QCursor::pos());
                return true;
            }

            if(currentDragIndex_ != -1){
                QPoint pos = static_cast<QMouseEvent*>(event)->pos();
                int dragDist = QApplication::startDragDistance();
                QRect dragRect = QRect(pos.x() - dragDist, pos.y() - dragDist, dragDist * 2, dragDist *2);

                if(!tabBar_->geometry().intersects(dragRect)){
                    startDrag_ = true;
                    QPixmap preview = editors_.value(tabNodes_.at(currentDragIndex_))->grab();
                    preview = preview.scaled(320, 240, Qt::KeepAspectRatio);
                    dragPreview_->setPixmap(preview);
                    dragPreview_->move(QCursor::pos());
                    dragPreview_->show();
                    return true;
                }
            }
            return false;
        }
        case QEvent::MouseButtonPress:{
            currentDragIndex_ = tabBar_->tabAt(static_cast<QMouseEvent*>(event)->pos());
            return false;
        }
        case QEvent::MouseButtonRelease:{
            if(startDrag_){
                QPoint pos = static_cast<QMouseEvent*>(event)->pos();
                if(!tabBar_->geometry().contains(pos)){
                    detachEditor(tabNodes_.at(currentDragIndex_));
                }
                dragPreview_->hide();
                startDrag_ = false;
            }
            currentDragIndex_ = -1;
            return false;
        }
        default:
            return false;
        }

    }
    return false;
}

void CrProjectEditorsTabWidget::openEditor(CrNode *node)
{
    //  Q_ASSERT(index >= 0 && index <= tabNodes_.size());
    QWidget* editor = editors_.value(node, nullptr);

    if(!editor){
        editor = node->createWidgetEditor();
        if(!editor){
            QWindow* windowEditor = node->createWindowEditor();

            if(!windowEditor)
                return;

            editor = QWidget::createWindowContainer(windowEditor);
            editor->setMinimumSize({640, 480});
        }

        int index = tabNodes_.size();
        tabNodes_.insert(index, node);
        editors_.insert(node, editor);

        tabBar_->insertTab(index, node->name());
        connect(node, &CrNode::nameChanged, this, [this, node](){
            tabBar_->setTabText(tabNodes_.indexOf(node), node->name());
        });

        tabBar_->setTabToolTip(index, node->path());
        connect(node, &CrNode::pathChanged, this, [this, node](){
            tabBar_->setTabToolTip(tabNodes_.indexOf(node), node->path());
        });

        body_->insertWidget(index, editor);
    }

    int index = tabNodes_.indexOf(node);
    if(index >= 0){
        tabBar_->setCurrentIndex(index);
    }else {
        editors_.value(node)->activateWindow();
    }
}

void CrProjectEditorsTabWidget::closeEditor(CrNode *node)
{
    QWidget* editor = editors_.value(node, nullptr);
    Q_ASSERT(editor);

    int index = tabNodes_.indexOf(node);

    if(index >= 0){
        disconnect(node, &CrNode::nameChanged, this, nullptr);
        disconnect(node, &CrNode::pathChanged, this, nullptr);
        tabBar_->removeTab(index);
        delete editor;
        tabNodes_.removeAt(index);

    }else {
        disconnect(editor, &QObject::destroyed, this, nullptr);
    }

    editors_.remove(node);

}

void CrProjectEditorsTabWidget::detachEditor(CrNode *node)
{
    QWidget* editor = editors_.value(node, nullptr);
    Q_ASSERT(editor);

    int index = tabNodes_.indexOf(node);
    Q_ASSERT(index >= 0);

    disconnect(node, &CrNode::nameChanged, this, nullptr);
    disconnect(node, &CrNode::pathChanged, this, nullptr);
    tabBar_->removeTab(index);
    body_->removeWidget(editor);
    tabNodes_.removeAt(index);

    connect(editor, &QObject::destroyed, this, [this, node]{
        closeEditor(node);
    });

    editor->setParent(nullptr);
    editor->setAttribute(Qt::WA_DeleteOnClose);
    editor->setGeometry(QRect(QCursor::pos(), editor->sizeHint()));
    editor->show();

}


