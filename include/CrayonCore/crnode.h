#ifndef CRNODE_H
#define CRNODE_H

#include "crayoncore_global.h"

#include <QObject>
#include <QVariant>
#include <QIcon>
#include <QUuid>
#include <QHash>
#include <QDataStream>

class QWidget;
class QDialog;
class QWindow;
class QEvent;
class QPainter;
class QStyleOptionViewItem;

class CrProject;
class CrModule;
class CrNodePrivate;

class CRAYON_CORE_EXPORT CrNode : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QUuid uuid READ  uuid WRITE setUuid NOTIFY uuidChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString path READ path)
    Q_PROPERTY(QString comments READ comments WRITE setComments NOTIFY commentsChanged)
    Q_PROPERTY(Flags flags READ flags WRITE setFlags NOTIFY flagsChanged)
    Q_PROPERTY(QIcon icon READ icon WRITE setIcon NOTIFY iconChanged)
    Q_PROPERTY(bool enable READ isEnable WRITE setEnable NOTIFY enableChanged)
    Q_PROPERTY(CrNode* parentNode READ parentNode WRITE setParentNode NOTIFY parentNodeChanged)
    Q_PROPERTY(CrProject* project READ project NOTIFY projectChanged)
    Q_PROPERTY(bool focus READ isFocus WRITE setFocus  NOTIFY focusChanged)
    Q_PROPERTY(bool selected READ isSelected WRITE setSelected NOTIFY selectedChanged)
    Q_PROPERTY(CrNode* inputNode READ inputNode WRITE setInputNode NOTIFY inputNodeChanged)
    Q_PROPERTY(CrModule* module READ module)
public:
    enum Flag{
        NoFlags = 0x00,
        EditableName = 0x01,
        ActiveEditor = 0x02,
        FixedChildren = 0x04,
        In = 0x08,
        Out = 0x10,
        Invisible = 0x20,
        Retain = 0x40,
    };
    Q_DECLARE_FLAGS(Flags, Flag)
    Q_FLAG(Flags)

    explicit CrNode(CrNode* parent = nullptr);
    virtual ~CrNode() override;

    QUuid uuid() const;
    void setUuid(const QUuid& uuid);

    QString name() const;
    void setName(const QString &name);
    QString path();

    QString comments() const;
    void setComments(const QString &comments);

    CrNode::Flags flags() const;
    void setFlags(CrNode::Flags flags);

    QIcon icon() const;
    void setIcon(const QIcon &icon);

    bool isEnable() const;
    void setEnable(bool isEnable);

    Q_INVOKABLE  QList<CrNode*> childNodes() const;
    Q_INVOKABLE  bool insertChildNode(int index, CrNode* node);
    Q_INVOKABLE  bool appendChildNode(CrNode* node);
    Q_INVOKABLE  bool removeChildNode(CrNode* node);
    Q_INVOKABLE  void clearChildNodes();

    CrNode* parentNode() const;
    bool setParentNode(CrNode* parentNode);

    CrNode* rootNode() const;

    CrProject *project() const;

    bool isFocus() const;
    void setFocus(bool focus);

    bool isSelected() const;
    void setSelected(bool selected);

    CrNode *inputNode() const;
    bool setInputNode(CrNode *inputNode);
    Q_INVOKABLE virtual bool isValidInputNode(CrNode* inputNode) const;

    Q_INVOKABLE QList<CrNode *> outputNodes() const;
    Q_INVOKABLE inline bool addOutputNode(CrNode* outNode) { return outNode->setInputNode(this); }
    Q_INVOKABLE inline void addOutputNodes(QList<CrNode*> outNodes) {
        foreach (CrNode* outNode, outNodes) {
            addOutputNode(outNode);
        }
    }
    Q_INVOKABLE inline bool removeOutputNode(CrNode* outNode) { return outNode->setInputNode(nullptr);}
    Q_INVOKABLE inline void removeOutputNodes(QList<CrNode*> outNodes) {
        foreach (CrNode* outNode, outNodes) {
            removeOutputNode(outNode);
        }
    }
    Q_INVOKABLE void clearOutputNodes();
    Q_INVOKABLE virtual bool isValidOunputNode(CrNode* outputNode) const;

    bool isModule() const;
    CrModule* module() const;
    Q_INVOKABLE CrNode* createCopy() const;

    Q_INVOKABLE void pushInfoMessage(const QString& title, const QString &text = QString());
    Q_INVOKABLE void pushWarningMessage(const QString& title, const QString &text = QString());
    Q_INVOKABLE void pushErrorMessage(const QString& title, const QString &text = QString());

    Q_INVOKABLE virtual QWidget *createCompactWidgetEditor();
    Q_INVOKABLE virtual QWidget* createWidgetEditor();
    Q_INVOKABLE virtual QDialog* createDialogEditor();
    Q_INVOKABLE virtual QWindow* createWindowEditor();

    void requestOpenEditor();

    Q_INVOKABLE virtual bool eventEditorPreview(QEvent *event, const QStyleOptionViewItem &option);
    Q_INVOKABLE virtual QSize sizeHintEditorPreview(const QStyleOptionViewItem &option) const;
    Q_INVOKABLE virtual void paintEditorPreview(QPainter *painter, const QStyleOptionViewItem& option) const;

    Q_INVOKABLE virtual QByteArray saveState() const;
    Q_INVOKABLE virtual bool restoreState(const QByteArray &state);

    friend QDataStream &operator<<(QDataStream &out, const CrNode* node);
    friend QDataStream &operator>>(QDataStream &in, CrNode* node);
    friend class CrProject;
    friend class CrModule;
protected:
    virtual void changeInputNode(CrNode* newNode);
signals:
    void uuidChanged();
    void nameChanged();
    void pathChanged();
    void commentsChanged();
    void flagsChanged();
    void iconChanged();
    void enableChanged();

    void childNodeInserted(int index, CrNode* children);
    void childNodeRemoved(int index, CrNode* children);
    void parentNodeChanged();
    void projectChanged();
    void focusChanged();
    void selectedChanged();

    void inputNodeChanged();
    void outputNodeAdded(CrNode* outNode);
    void outputNodeRemoved(CrNode* outNode);

    void updateEditorPreview();

private: 
    explicit CrNode(CrProject* project);
    void setModule(CrModule* module);

    CrNodePrivate* d_;
    bool selected_ = false;

};

Q_DECLARE_OPERATORS_FOR_FLAGS(CrNode::Flags)

QDataStream &operator<<(QDataStream &out, const CrNode* node);
QDataStream &operator>>(QDataStream &in, CrNode* node);



#endif // CRNODE_H
