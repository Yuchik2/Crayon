#ifndef CRPROJECT_H
#define CRPROJECT_H

#include "crayoncore_global.h"
#include <QObject>
#include <QSet>

class QUndoStack;
class QUndoCommand;

class CrLog;
class CrNode;
class CrMonitor;
class CrPlugin;
class CrProjectPrivate;
class CRAYON_CORE_EXPORT CrProject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString autorName READ autorName WRITE setAutorName NOTIFY autorNameChanged)
    Q_PROPERTY(QString editorPassword READ editorPassword WRITE setEditorPassword NOTIFY editorPasswordChanged)
    Q_PROPERTY(QString monitorPassword READ monitorPassword WRITE setMonitorPassword NOTIFY monitorPasswordChanged)
    Q_PROPERTY(QString comments READ comments WRITE setComments NOTIFY commentsChanged)
    Q_PROPERTY(CrNode* focusNode READ focusNode WRITE setFocusNode NOTIFY focusNodeChanged)
    Q_PROPERTY(QList<CrNode*> selectedNodes READ selectedNodes NOTIFY selectedChanged)
    Q_PROPERTY(QList<CrMonitor*> monitors READ monitors)
    Q_PROPERTY(QList<CrPlugin*> plugins READ plugins)
    Q_PROPERTY(CrLog* log READ log)

public:
    explicit CrProject();
    virtual ~CrProject() override;

    QString name() const;
    void setName(const QString &name);

    QString autorName() const;
    void setAutorName(const QString &autorName);

    QString editorPassword() const;
    void setEditorPassword(const QString &editorPassword);

    QString monitorPassword() const;
    void setMonitorPassword(const QString &monitorPassword);

    QString comments() const;
    void setComments(const QString &comments);

    QString path() const;
    void setPath(const QString &path);

    CrNode* rootNode() const;

    CrNode *focusNode() const;
    void setFocusNode(CrNode *node);

    QList<CrNode *> selectedNodes() const;
    Q_INVOKABLE void setSelectedNodes(QList<CrNode *> nodes);
    Q_INVOKABLE inline void clearSelectedNodes() { setSelectedNodes(QList<CrNode*>{});}

    QList<CrMonitor *> monitors() const;
    void addMonitor(CrMonitor* monitor);
    void removeMonitor(CrMonitor* monitor);

    QList<CrPlugin *> plugins() const;
    void addPlugin(CrPlugin* plugin);
    void removePlugin(CrPlugin* plugin);

    CrLog *log() const;
    Q_INVOKABLE void pushInfoMessage(const QString& title, const QString &text = QString(), CrNode* node = nullptr);
    Q_INVOKABLE void pushWarningMessage(const QString& title, const QString &text = QString(), CrNode* node = nullptr);
    Q_INVOKABLE void pushErrorMessage(const QString& title, const QString &text = QString(), CrNode* node = nullptr);

    QUndoStack *undoStack() const;
    void pushUndoCommand(QUndoCommand* command);    

    QByteArray saveState() const;
    bool restoreState(const QByteArray &state);

    QByteArray saveRetainState() const;
    bool restoreRetainState(const QByteArray &state);

    CrNode* findNodeByUuid(const QUuid& uuid);

    friend class CrNode;

signals:
    void pathChanged();
    void nameChanged();
    void autorNameChanged();
    void editorPasswordChanged();
    void monitorPasswordChanged();
    void commentsChanged();
    void focusNodeChanged();
    void selectedChanged();

    void monitorAdded(CrMonitor*);
    void monitorRemoved(CrMonitor*);

    void pluginAdded(CrPlugin*);
    void pluginRemoved(CrPlugin*);

    void requestOpenEditor(CrNode* node);

private:
    void registerNodeUuid(QUuid uuid, CrNode* node);
    void unregisterNodeUuid(QUuid uuid);
    CrProjectPrivate* d_;
    QSet<CrNode*> selectedNodes_;

};

 CRAYON_CORE_EXPORT QDataStream &operator<<(QDataStream &out, const CrProject* project);
 CRAYON_CORE_EXPORT QDataStream &operator>>(QDataStream &in, CrProject* project);


#endif // CRPROJECT_H

