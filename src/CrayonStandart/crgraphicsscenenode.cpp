#include "crgraphicsscenenode.h"

#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include <QScrollBar>
#include <QDialog>
#include <QLineEdit>
#include <QMenu>
#include <QAction>
#include <QDropEvent>
#include <QMimeData>
#include <QGraphicsSceneMouseEvent>

#include <QUndoCommand>

#include <QGraphicsView>
#include <QGraphicsItem>
#include <QStyleOptionViewItem>
#include <QHash>
#include <QSet>
#include <QPointF>

#include <QApplication>
#include <QClipboard>

#include <QtMath>
#include <QStack>
#include <QTimer>
#include <QPair>
#include <QRandomGenerator64>

#include <crproject.h>
#include <crcommands.h>
#include <crnode.h>
#include <crmodule.h>

#include "crgraphicsscene.h"
#include "crgraphicsscenenodeeditor.h"
#include "crgraphicsscenenodeitem.h"

#include <QDebug>

class CrGraphicsSceneNodePrivate {
public:
    CrGraphicsScene* scene_ = nullptr;
    int editorCount_ = 0;
    QStyle* style_;
    QByteArray sceneState_;

};

CrGraphicsSceneNode::CrGraphicsSceneNode(CrNode* node) :
    CrNode(node),
    d_(new CrGraphicsSceneNodePrivate())
{
    setName("Graphics scene");
    setFlags(EditableName | ActiveEditor | FixedChildren);
}

CrGraphicsSceneNode::~CrGraphicsSceneNode()
{
    delete d_;
}

QWidget *CrGraphicsSceneNode::createWidgetEditor()
{
    if(!d_->scene_){
        d_->scene_ = new CrGraphicsScene(this);
        d_->scene_->setSceneRect(-16000,-9000,32000,18000);
        if(!d_->sceneState_.isEmpty())
            d_->scene_->restoreState(d_->sceneState_);
    }

    ++d_->editorCount_;

    auto editor = new CrGraphicsSceneEditor(d_->scene_);

    editor->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    editor->setOptimizationFlags(QGraphicsView::DontSavePainterState | QGraphicsView::DontAdjustForAntialiasing);
    editor->setCacheMode(QGraphicsView::CacheBackground);
    editor->setRenderHints(QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
    editor->setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
    editor->setWindowTitle(tr("Graphics editor"));

    editor->setWindowTitle(name());
    connect(editor, &QObject::destroyed, this, [this]{
        --d_->editorCount_;
        if(d_->editorCount_ == 0){
            d_->sceneState_ = d_->scene_->saveState();
            delete d_->scene_;
            d_->scene_ = nullptr;
        }
    });
    return editor;
}

QSize CrGraphicsSceneNode::sizeHintEditorPreview(const QStyleOptionViewItem &option) const
{
    const QWidget *widget = option.widget;
    QStyle *style = widget ? widget->style() : QApplication::style();
    QSize size = option.fontMetrics.size(Qt::TextSingleLine, tr("Open editor"));

    QStyleOptionButton opt;
    opt.state = option.state;
    int bm = style->pixelMetric(QStyle::PM_ButtonMargin, &opt, widget),
            fw = style->pixelMetric(QStyle::PM_DefaultFrameWidth, &opt, widget) * 2;
    size.rwidth() += bm + fw;
    size.rheight() += bm + fw;
    return  size;
}

void CrGraphicsSceneNode::paintEditorPreview(QPainter *painter, const QStyleOptionViewItem &option) const
{
    CrNode::paintEditorPreview(painter, option);
    if(flags() & CrNode::ActiveEditor){
        QStyleOptionButton opt;
        opt.rect = option.rect;
        opt.state = option.state;
        opt.text = tr("Open editor");
        const QWidget *widget = option.widget;
        QStyle *style = widget ? widget->style() : QApplication::style();
        style->drawControl(QStyle::CE_PushButton, &opt, painter, widget);
    }
}

QByteArray CrGraphicsSceneNode::saveState() const
{
    QByteArray state;
    QDataStream out(&state, QIODevice::WriteOnly);
    out << CrNode::saveState();
    if(d_->scene_){
        out << d_->scene_->saveState();
    }else {
        out << d_->sceneState_;
    }

    return state;
}

bool CrGraphicsSceneNode::restoreState(const QByteArray &state)
{
    QDataStream in(state);
    QByteArray nodeState;
    in >> nodeState;
    CrNode::restoreState(nodeState);
    in >> d_->sceneState_;
    if(d_->scene_){
        d_->scene_->restoreState(d_->sceneState_);
    }

    return true;
}








