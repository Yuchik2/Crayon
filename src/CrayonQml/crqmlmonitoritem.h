//#ifndef CRQMLMONITORITEM_H
//#define CRQMLMONITORITEM_H

//#include <QQuickItem>

//class CrQmlNode;
//class CrQmlMonitorNode;
//class CrQmlMonitorItem : public QQuickItem
//{
//    Q_OBJECT
//    Q_PROPERTY(QColor backgroundColor READ backgroundColor NOTIFY backgroundColorChanged)
//    Q_PROPERTY(QString backgroundImageSource READ backgroundImageSource NOTIFY backgroundImageSourceChanged)
//public:
//    CrQmlMonitorItem(QQuickItem* parent = nullptr);

//    CrQmlMonitorNode *node() const;
//    void setNode(CrQmlMonitorNode *node);

//    QColor backgroundColor() const;
//    QString backgroundImageSource() const;

//    QQuickItem* itemByNode(CrQmlNode* node);

//    QQuickItem *contentItem() const;

//signals:
//    void backgroundColorChanged();
//    void backgroundImageSourceChanged();

//private:
//    CrQmlMonitorNode* node_;
//    QColor  backgroundColor_;
//    QString backgroundImageSource_;
//    QHash<CrQmlNode*, QQuickItem*> items_;

//private slots:
//    void changeMonitorSize();
//    void changeNodeGeometry(CrQmlNode* node,  QRectF geom);
//    void addNode(CrQmlNode* node);
//    void removeNode(CrQmlNode* node);
//    void resetItem();
//};

//#endif // CRQMLMONITORITEM_H
