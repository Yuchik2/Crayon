#include <crqmlnode.h>

#include <QApplication>
#include <QTimer>
#include <QQuickView>
#include <QWidget>
#include <QQuickItem>
#include <QQmlEngine>

#include <QStyleOptionFrame>
#include <QStyleOptionViewItem>


CrQmlNode::CrQmlNode(CrNode* parent) : CrNode (parent)
{

}

CrQmlNode::~CrQmlNode()
{

}

QQuickItem *CrQmlNode::createItem()
{
    if(inputNode()){
        return static_cast<CrQmlNode*>(inputNode())->createItem();
    }
    QQmlComponent c(engine(), "qrc:/CrayonQmlPluginResources/CrNullItem.qml");
    return static_cast<QQuickItem*>(c.create());
}

bool CrQmlNode::isValidInputNode(CrNode *inputNode) const
{
     return static_cast<bool>(qobject_cast<CrQmlNode*>(inputNode));
}


QWindow *CrQmlNode::createWindowEditor()
{
    QQuickView *view = new QQuickView(QUrl("qrc:/CrayonQmlPluginResources/ViewerRootObject.qml"));
    view->setMinimumSize({640, 480});
    //view->setResizeMode(QQuickView::SizeRootObjectToView);

    QQuickItem* item = createItem();
    item->setParentItem(view->rootObject());

    connect(this, &CrQmlNode::itemReset, view, [this, view, it = item]() mutable {
        it->setVisible(false);
        it->setParentItem(nullptr);
        it->deleteLater();

        it = createItem();
        it->setParentItem(view->rootObject());
    });

    return view;
}

QSize CrQmlNode::sizeHintEditorPreview(const QStyleOptionViewItem &option) const
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

void CrQmlNode::paintEditorPreview(QPainter *painter, const QStyleOptionViewItem &option) const
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

void CrQmlNode::changeInputNode(CrNode *newNode)
{
    if(inputNode()){
        disconnect(inputNode(), SIGNAL(itemReset()), this, SIGNAL(itemReset()));
    }

    if(newNode){
        connect(newNode, SIGNAL(itemReset()), this, SIGNAL(itemReset()));
    }
    QTimer::singleShot(0, this, [this]{emit itemReset();});
}

QQmlEngine *CrQmlNode::engine()
{
    static QQmlEngine* engine = new QQmlEngine();
    return engine;
}
