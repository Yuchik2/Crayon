//#ifndef CRQMLMETATYPE_H
//#define CRQMLMETATYPE_H

//#include <QObject>
//#include <QVariant>

//class CrQmlMetaType : public QObject
//{
//    Q_OBJECT
//public:
//    enum Type {
//        Invalid = QMetaType::UnknownType,
//        Bool = QMetaType::Bool,
//        Int = QMetaType::Int,
//        UInt = QMetaType::UInt,
//        LongLong = QMetaType::LongLong,
//        ULongLong = QMetaType::ULongLong,
//        Double = QMetaType::Double,
//        Char = QMetaType::QChar,
//        Map = QMetaType::QVariantMap,
//        List = QMetaType::QVariantList,
//        String = QMetaType::QString,
//        StringList = QMetaType::QStringList,
//        ByteArray = QMetaType::QByteArray,
//        BitArray = QMetaType::QBitArray,
//        Date = QMetaType::QDate,
//        Time = QMetaType::QTime,
//        DateTime = QMetaType::QDateTime,
//        Url = QMetaType::QUrl,
//        Locale = QMetaType::QLocale,
//        Rect = QMetaType::QRect,
//        RectF = QMetaType::QRectF,
//        Size = QMetaType::QSize,
//        SizeF = QMetaType::QSizeF,
//        Line = QMetaType::QLine,
//        LineF = QMetaType::QLineF,
//        Point = QMetaType::QPoint,
//        PointF = QMetaType::QPointF,
//        RegExp = QMetaType::QRegExp,
//        RegularExpression = QMetaType::QRegularExpression,
//        Hash = QMetaType::QVariantHash,
//        EasingCurve = QMetaType::QEasingCurve,
//        Uuid = QMetaType::QUuid,
//        ModelIndex = QMetaType::QModelIndex,
//        PersistentModelIndex = QMetaType::QPersistentModelIndex,
//        LastCoreType = QMetaType::LastCoreType,

//        Font = QMetaType::QFont,
//        Pixmap = QMetaType::QPixmap,
//        Brush = QMetaType::QBrush,
//        Color = QMetaType::QColor,
//        Palette = QMetaType::QPalette,
//        Image = QMetaType::QImage,
//        Polygon = QMetaType::QPolygon,
//        Region = QMetaType::QRegion,
//        Bitmap = QMetaType::QBitmap,
//        Cursor = QMetaType::QCursor,
//        KeySequence = QMetaType::QKeySequence,
//        Pen = QMetaType::QPen,
//        TextLength = QMetaType::QTextLength,
//        TextFormat = QMetaType::QTextFormat,
//        Matrix = QMetaType::QMatrix,
//        Transform = QMetaType::QTransform,
//        Matrix4x4 = QMetaType::QMatrix4x4,
//        Vector2D = QMetaType::QVector2D,
//        Vector3D = QMetaType::QVector3D,
//        Vector4D = QMetaType::QVector4D,
//        Quaternion = QMetaType::QQuaternion,
//        PolygonF = QMetaType::QPolygonF,
//        Icon = QMetaType::QIcon,
//        LastGuiType = QMetaType::LastGuiType,

//        SizePolicy = QMetaType::QSizePolicy,

//        UserType = QMetaType::User,
//        LastType = 0xffffffff // need this so that gcc >= 3.4 allocates 32 bits for Type
//    };
//    Q_ENUM(Type)

//    Q_INVOKABLE static int getType(const QVariant& value);
//    explicit CrQmlMetaType(QObject *parent = nullptr);

//};

//#endif // CRQMLMETATYPE_H
