#ifndef CRQMLMONITORNODE_H
#define CRQMLMONITORNODE_H

#include <crqmlnode.h>

class QQuickItem;
class CRAYON_QML_EXPORT CrQmlMonitorNode : public CrQmlNode
{
    Q_OBJECT
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY backgroundColorChanged)
    Q_PROPERTY(QImage backgroundImage READ backgroundImage  WRITE setBackgroundImage NOTIFY backgroundImageChanged)
public:
    explicit CrQmlMonitorNode(CrNode* parent = nullptr);
    virtual ~CrQmlMonitorNode() override;

    virtual QWindow* createWindowEditor() override;

    virtual QQuickItem* createItem() override;

    QSizeF monitorSize() const;
    void setMonitorSize(const QSizeF &monitorSize);

    QColor backgroundColor() const;
    void setBackgroundColor(const QColor &backgroundColor);

    QImage backgroundImage() const;
    void setBackgroundImage(const QImage &backgroundImage);

    QList<CrQmlNode*> qmlNodes() const;
    QList<CrQmlNode*> selectedQmlNodes() const;

    QRectF geometryItem(CrQmlNode* node) const;
    void setGeometryItem(CrQmlNode* node, QRectF geom);

    virtual QByteArray saveState() const override;
    virtual bool restoreState(const QByteArray &state) override;

signals:
    void monitorSizeChanged();
    void backgroundColorChanged();
    void backgroundImageChanged();

    void nodeAdded(CrQmlNode* node);
    void nodeRemoved(CrQmlNode* node);
    void nodeGeometryChanged(CrQmlNode* node,  QRectF geom);

private:
    Q_DISABLE_COPY(CrQmlMonitorNode)
    QSizeF monitorSize_;
    QColor  backgroundColor_;
    QImage backgroundImage_;
    QList<CrQmlNode*> nodes_;
    QHash<CrQmlNode*, QRectF> geometryItems_;

    void addQmlNode(CrQmlNode* node);
    void removeQmlNode(CrQmlNode* node);
    void resetQmlNode();

};
#endif // CRQMLMONITORNODE_H
