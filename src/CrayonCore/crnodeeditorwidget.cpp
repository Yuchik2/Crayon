#include "crnodeeditorwidget.h"

#include <QStyleOptionViewItem>
#include <QEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QDialog>

#include <CrNode>

#include <QDebug>

CrNodeEditorWidget::CrNodeEditorWidget(QWidget *parent) :
    QWidget(parent), node_(nullptr), editor_(nullptr)
{
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
}

CrNodeEditorWidget::~CrNodeEditorWidget()
{

}

CrNode *CrNodeEditorWidget::node() const
{
    return node_;
}

void CrNodeEditorWidget::setNode(CrNode *node)
{
    if(node_ == node)
        return;
    node_ = node;
    if(editor_){
        editor_->deleteLater();
        editor_ = nullptr;
    }
    updateGeometry();
    update();
}

QSize CrNodeEditorWidget::sizeHint() const
{
    if(!node_)
        return QSize();
    QStyleOptionViewItem opt;
    opt.features = QStyleOptionViewItem::HasDisplay | QStyleOptionViewItem::HasDecoration;
    opt.displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
    opt.initFrom(this);
    QSize sz = node_->sizeHintEditorPreview(opt);
    if(editor_)
        sz = sz.expandedTo(editor_->sizeHint());
    return sz;
}

QSize CrNodeEditorWidget::minimumSizeHint() const
{
    QSize ms = sizeHint().expandedTo(QSize(32, 15));
    if(editor_)
        ms = ms.expandedTo(editor_->minimumSize());
    return ms;
}

bool CrNodeEditorWidget::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == editor_){
        if(event->type() == QEvent::KeyPress){
            if(static_cast<QKeyEvent*>(event)->key() == Qt::Key_Return){
                editor_->deleteLater();
                editor_ = nullptr;
            }
        }
        if(event->type() == QEvent::FocusOut){
            editor_->deleteLater();
            editor_ = nullptr;
        }
    }
    return QWidget::eventFilter(watched, event);
}

bool CrNodeEditorWidget::event(QEvent *event)
{
    if(!node_)
        return false;
    QStyleOptionViewItem opt;
    opt.initFrom(this);
    opt.features = QStyleOptionViewItem::HasDisplay | QStyleOptionViewItem::HasDecoration;
    opt.displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
    if(node_->eventEditorPreview(event, opt)){
        update();
        return true;
    }

    return QWidget::event(event);
}

void CrNodeEditorWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    if(!node_)
        return;
    QStyleOptionViewItem opt;
    opt.initFrom(this);
    opt.features = QStyleOptionViewItem::HasDisplay | QStyleOptionViewItem::HasDecoration;
    opt.displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
    QPainter painter(this);
    node_->paintEditorPreview(&painter, opt);
    painter.fillRect(geometry(), QBrush(QColor("black")));
}

void CrNodeEditorWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event);

    if(!node_)
        return;

    if(editor_){
        editor_->deleteLater();
    }

    editor_ = node_->createCompactWidgetEditor();

    if(editor_){
        if(editor_){
            editor_->setParent(this);
            editor_->setFocus();
            editor_->show();
            editor_->installEventFilter(this);
            updateGeometry();
        }
        return;
    }

    editor_ = node_->createDialogEditor();

    if(editor_){
        static_cast<QDialog*>(editor_)->exec();
        editor_->deleteLater();
        editor_ = nullptr;
        return;
    }
    node_->setFocus(true);
}
