#include "crproject.h"

#include <QApplication>
#include <QDir>
#include <QUndoStack>
#include <QStack>
#include <QStyleOptionViewItem>
#include <QSet>

#include "crplugin.h"
#include "crlog.h"
#include "crnode.h"
#include "crmonitor.h"

#include <QDebug>

class CrProjectPrivate {
public:
    QString path_;
    QString name_;
    QString autorName_;
    QString editorPassword_;
    QString monitorPassword_;
    QString comments_;

    CrNode* rootNode_;
    CrLog* log_;
    QUndoStack* undoStack_;

    CrNode* focusNode_ = nullptr;
    QSet<CrMonitor*> monitors_;
    QSet<CrPlugin*> plugins_;
    QHash<QUuid, CrNode*> uuidMap_;

    QString projectName() const;
    void setProjectName(const QString &projectName);
};

CrProject::CrProject() : QObject (nullptr), d_(new CrProjectPrivate())
{
    d_->rootNode_ = new CrNode(this);
    d_->log_ = new CrLog(this);
    d_->undoStack_ = new QUndoStack(this);

}

CrProject::~CrProject()
{
    foreach(CrMonitor* monitor, d_->monitors_){
        removeMonitor(monitor);
        delete monitor;
    }
    delete d_->rootNode_;
    delete d_->log_;
    delete d_->undoStack_;
    delete d_;
}

QString CrProject::name() const
{
    return d_->name_;
}

void CrProject::setName(const QString &projectName)
{
    if(d_->name_ == projectName)
        return;
    d_->name_ = projectName;
    emit nameChanged();
}

QString CrProject::autorName() const { return d_->autorName_;}

void CrProject::setAutorName(const QString &autorName)
{
    if(d_->autorName_ == autorName)
        return;
    d_->autorName_ = autorName;
    emit autorNameChanged();
}

QString CrProject::editorPassword() const { return d_->editorPassword_;}

void CrProject::setEditorPassword(const QString &editorPassword)
{
    if(d_->editorPassword_ == editorPassword)
        return;
    d_->editorPassword_ = editorPassword;
    emit editorPasswordChanged();
    //
}

QString CrProject::monitorPassword() const { return d_->monitorPassword_;}

void CrProject::setMonitorPassword(const QString &monitorPassword)
{
    if(d_->monitorPassword_ == monitorPassword)
        return;
    d_->monitorPassword_ = monitorPassword;
    emit monitorPasswordChanged();
}

QString CrProject::comments() const { return d_->comments_;}

void CrProject::setComments(const QString &comments)
{
    if(d_->comments_ == comments)
        return;

    d_->comments_ = comments;
    emit commentsChanged();
}

QString CrProject::path() const { return d_->path_;}

void CrProject::setPath(const QString &path)
{
    if(d_->path_ == path)
        return;

    d_->path_ = path;
    emit pathChanged();
}

CrNode *CrProject::rootNode() const { return d_->rootNode_;}

CrNode *CrProject::focusNode() const { return d_->focusNode_;}

void CrProject::setFocusNode(CrNode *node)
{
    if(d_->focusNode_ == node)
        return;
    CrNode* previousFocus = d_->focusNode_;
    d_->focusNode_ = node;
    if(previousFocus){
        previousFocus->focusChanged();
    }
    if(d_->focusNode_){
        d_->focusNode_->focusChanged();
    }
    emit focusNodeChanged();
}

QList<CrNode *> CrProject::selectedNodes() const
{
    CrProject* that = const_cast<CrProject*>(this);
    QSet<CrNode *> actuallySelectedSet;
    foreach (CrNode * node, that->selectedNodes_) {
        if (node->selected_)
            actuallySelectedSet << node;
    }
    that->selectedNodes_ = actuallySelectedSet;
    return that->selectedNodes_.values();
}

void CrProject::setSelectedNodes(QList<CrNode *> nodes)
{
    const QSet<CrNode*> selectNodes = nodes.toSet();
    QSet<CrNode*> unselectNodes = selectedNodes_ - selectNodes;
    selectedNodes_ = selectNodes;

    bool changed = false;

    for (CrNode* node : selectNodes) {
        if (!node->selected_) {
            changed = true;
            node->selected_ = true;
            node->selectedChanged();
        }
    }

    foreach (CrNode* node, unselectNodes) {
        if (node->selected_) {
            changed = true;
            node->selected_ = false;
            node->selectedChanged();
        }
    }

    if(changed)
        emit selectedChanged();
}

QList<CrMonitor *> CrProject::monitors() const { return d_->monitors_.toList();}

void CrProject::addMonitor(CrMonitor *monitor)
{
    if(d_->monitors_.contains(monitor) || !monitor)
        return;

    d_->monitors_.insert(monitor);
    emit monitorAdded(monitor);
}

void CrProject::removeMonitor(CrMonitor *monitor)
{
    if(!d_->monitors_.contains(monitor))
        return;

    d_->monitors_.remove(monitor);
    emit monitorRemoved(monitor);
}


QList<CrPlugin *> CrProject::plugins() const
{
    return d_->plugins_.toList();
}

void CrProject::addPlugin(CrPlugin *plugin)
{
    if(d_->plugins_.contains(plugin) || !plugin)
        return;

    if(plugin->status() == CrPlugin::Error){
        pushErrorMessage(QString(tr("Fault load plugin by id:\"%1\" version:\"%2\" .").
                                 arg(plugin->id()).arg(plugin->version().toString())), plugin->errorString());
        return;
    }

    if(plugin->status() == CrPlugin::NotLoaded){
        if(!plugin->load()){
            pushErrorMessage(QString(tr("Fault load plugin by id:\"%1\" version:\"%2\" .").
                                     arg(plugin->id()).arg(plugin->version().toString())), plugin->errorString());
            return;
        }
    }
    d_->plugins_.insert(plugin);
    emit pluginAdded(plugin);
}

void CrProject::removePlugin(CrPlugin *plugin)
{
    if(!d_->plugins_.contains(plugin))
        return;

    d_->plugins_.remove(plugin);
    emit pluginRemoved(plugin);
}

CrLog *CrProject::log() const
{
    return d_->log_;
}

void CrProject::pushInfoMessage(const QString &title, const QString &text, CrNode *node)
{
    CrLog::Message ms{title, text, CrLog::Info, node};
    d_->log_->addMessage(ms);
}

void CrProject::pushWarningMessage(const QString &title, const QString &text, CrNode *node)
{
    CrLog::Message ms{title, text, CrLog::Warning, node};
    d_->log_->addMessage(ms);
}

void CrProject::pushErrorMessage(const QString &title, const QString &text, CrNode *node)
{
    CrLog::Message ms{title, text, CrLog::Error, node};
    d_->log_->addMessage(ms);
}

QUndoStack *CrProject::undoStack() const
{
    return d_->undoStack_;
}

void CrProject::pushUndoCommand(QUndoCommand *command)
{
    d_->undoStack_->push(command);
}

QByteArray CrProject::saveState() const
{
    QByteArray state;
    QDataStream out(&state, QIODevice::WriteOnly);

    out << d_->name_ << d_->autorName_ << d_->editorPassword_ << d_->monitorPassword_;

    out << d_->plugins_.size();
    foreach(CrPlugin* plugin, d_->plugins_){
        out << plugin->key();
    }

    out << d_->rootNode_;

    QList<QUuid> connectionList;
    QStack<CrNode*> stack;
    stack << d_->rootNode_;
    while (!stack.isEmpty()) {
        CrNode* node = stack.pop();
        if(node->inputNode()){
            connectionList << node->uuid() << node->inputNode()->uuid();
        }

        stack << node->childNodes().toVector();
    }
    out << connectionList;

    out << d_->monitors_.size();

    foreach(CrMonitor* monitor, d_->monitors_){
        out << monitor;
    }

    return state;
}

bool CrProject::restoreState(const QByteArray &state)
{
    QDataStream in(state);
    QString projectName, autorName, editorPassword, monitorPassword;
    in  >> projectName >> autorName >> editorPassword >> monitorPassword;
    setName(projectName);
    setAutorName(autorName);
    setEditorPassword(editorPassword);
    setMonitorPassword(monitorPassword);

    int pluginsSize;
    in >> pluginsSize;
    for (int i = 0; i < pluginsSize; ++i) {

        CrPluginKey key;
        in >> key;
        CrPlugin* plugin = CrPlugin::plugin(key);
        if(!plugin){
            pushErrorMessage(tr("Load project error."),
                             tr("Plugin by id:\"%1\" version:\"%2\" not found.").arg(plugin->id()).arg(plugin->version().toString()),
                             nullptr);
            continue;
        }
        addPlugin(plugin);
    }

    in >> d_->rootNode_;

    QList<QUuid> connectionList;
    in >> connectionList;

    for (int i = 0; i < connectionList.size(); ++i) {
        QUuid inUuid = connectionList.at(i);
        CrNode* in = findNodeByUuid(inUuid);
        ++i;
        if(i >= connectionList.size())
            break;

        QUuid outUuid = connectionList.at(i);
        CrNode* out = findNodeByUuid(outUuid);
        if(!in){
            pushErrorMessage(tr("Load project error."),
                             tr("Сonnection failed. Input node by UUID: \"%1\" not found.").arg(inUuid.toString()),
                             nullptr);
            continue;
        }
        if(!out){
            pushErrorMessage(tr("Load project error."),
                             tr("Сonnection failed. Ounput node by UUID: \"%1\" not found.").arg(outUuid.toString()),
                             nullptr);
            continue;
        }

        in->setInputNode(out);
    }

    int monitorsSize;
    in >> monitorsSize;
    for (int i = 0; i < monitorsSize; ++i) {
        CrMonitor* monitor = new CrMonitor(this);
        in >> monitor;
        if(!monitor->node()){
            pushErrorMessage(tr("Load project error."),
                             tr("Monitor\"%1\" not loaded.").arg(monitor->name()),
                             nullptr);
            continue;
        }
        addMonitor(monitor);
    }

    return true;
}

QByteArray CrProject::saveRetainState() const
{
    QByteArray state;
    QDataStream out(&state, QIODevice::WriteOnly);

    QHash<QUuid, QByteArray> retainStates;
    QStack<CrNode*> stack;
    stack << d_->rootNode_;
    while (!stack.isEmpty()) {
        CrNode* node = stack.pop();
        if(node->flags() & CrNode::Retain){
            retainStates.insert(node->uuid(), node->saveState());
        }else {
            stack << node->childNodes().toVector();
        }
    }
    out << retainStates;
    return state;
}

bool CrProject::restoreRetainState(const QByteArray &state)
{
    QDataStream in(state);
    QHash<QUuid, QByteArray> retainStates;
    in >> retainStates;

    auto it = retainStates.cbegin();
    while (it != retainStates.cend()) {
        CrNode* node = d_->uuidMap_.value(it.key(), nullptr);
        if(node){
            restoreState(it.value());
        }
        ++it;
    }
    return true;
}

CrNode *CrProject::findNodeByUuid(const QUuid &uuid)
{
    return d_->uuidMap_.value(uuid, nullptr);
}

void CrProject::registerNodeUuid(QUuid uuid, CrNode *node)
{
    d_->uuidMap_.insert(uuid, node);
}

void CrProject::unregisterNodeUuid(QUuid uuid)
{
    d_->uuidMap_.remove(uuid);
}



QDataStream &operator<<(QDataStream &out, const CrProject *project)
{
    out << project->saveState();
    return out;
}

QDataStream &operator>>(QDataStream &in, CrProject *project)
{
    QByteArray nodeState;
    in >> nodeState;
    project->restoreState(nodeState);
    return in;
}

QString CrProjectPrivate::projectName() const
{
return name_;
}

void CrProjectPrivate::setProjectName(const QString &projectName)
{
name_ = projectName;
}
