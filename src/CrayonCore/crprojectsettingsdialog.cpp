#include "crprojectsettingsdialog.h"

#include <QtMath>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>

#include <QTabWidget>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QHeaderView>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QSettings>
#include <QApplication>

#include <QUndoCommand>

#include "crcore.h"
#include "crproject.h"
#include "crmonitor.h"
#include "crplugin.h"
#include "crnode.h"


#include "crprojectexplorerdialog.h"

#include <QDebug>

class CrProjectSettingsDialogPrivate{
public:
    CrProjectSettingsDialogPrivate(CrProjectSettingsDialog* dialog);
    ~CrProjectSettingsDialogPrivate();
    QObject receiver_;
    CrProjectSettingsDialog* dialog_;
    CrProject* project_ = nullptr;
    QLineEdit* nameEditor_;
    QLineEdit* autorEditor_;
    QLineEdit* editorPasswordEditor_;
    QLineEdit* monitorPasswordEditor_;
    QTextEdit* commentsEditor_;

    QTreeWidget* monitorsView_;
    QSet<CrMonitor*> monitors_;
    QHash<CrMonitor*, QTreeWidgetItem*> monitorItems_;

    QTreeWidget* pluginsView_;
    QSet<CrPlugin*> plugins_;

    QHash<CrPlugin*, QTreeWidgetItem*> pluginItems_;

public:
    void addMonitor(CrMonitor* monitor);
    void removeMonitor(CrMonitor* monitor);

    void addPlugin(CrPlugin* plugin);
    void removePlugin(CrPlugin* plugin);

    void setItemBackground(QTreeWidgetItem* item, const QBrush& brush);
};

CrProjectSettingsDialogPrivate::CrProjectSettingsDialogPrivate(CrProjectSettingsDialog *dialog) :
    dialog_(dialog),
    pluginsView_(new QTreeWidget())
{

    auto resetPlugins = [this]{

        pluginItems_.clear();
        pluginsView_->clear();

        foreach(CrPlugin* plugin, CrCore::plugins()){
            QTreeWidgetItem* item = new QTreeWidgetItem();
            item->setData(0, Qt::UserRole, QVariant::fromValue(plugin));
            item->setText(0, plugin->name());
            item->setText(1, plugin->version().toString());

            pluginItems_.insert(plugin, item);

            if(plugins_.contains(plugin)){
                setItemBackground(item, Qt::green);
            }

            if(plugin->status() == CrPlugin::Error){
                setItemBackground(item, Qt::red);
            }
            pluginsView_->addTopLevelItem(item);
        }
    };

    resetPlugins();

    QObject::connect(CrCore::instance(), &CrCore::pluginsChanged, &receiver_, resetPlugins);

}

CrProjectSettingsDialogPrivate::~CrProjectSettingsDialogPrivate()
{

}

void CrProjectSettingsDialogPrivate::addMonitor(CrMonitor *monitor)
{
    if(monitors_.contains(monitor))
        return;

    QTreeWidgetItem* item = new QTreeWidgetItem();
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    item->setData(0, Qt::UserRole, QVariant::fromValue(monitor));

    item->setText(0, monitor->name());
    QObject::connect(monitor, &CrMonitor::nameChanged, &receiver_, [monitor, item]{
        item->setText(0, monitor->name());
    });

    auto updateNode = [this, item, monitor]{
        CrNode* oldNode = item->data(1, Qt::UserRole).value<CrNode*>();
        if(oldNode){
            QObject::disconnect(monitor->node(), &CrNode::pathChanged, &receiver_, nullptr);
            QObject::disconnect(monitor->node(), &CrNode::iconChanged, &receiver_, nullptr);
        }

        CrNode* newNode = monitor->node();

        item->setData(1, Qt::UserRole, QVariant::fromValue(newNode));
        if(newNode){
            item->setText(1, monitor->node()->path());
            QObject::connect(monitor->node(), &CrNode::pathChanged, &receiver_, [monitor, item]{
                item->setText(1, monitor->node()->path());
            });
            item->setIcon(1, monitor->node()->icon());
            QObject::connect(monitor->node(), &CrNode::iconChanged, &receiver_, [monitor, item]{
                item->setIcon(1, monitor->node()->icon());
            });
        }else {
            item->setText(1, QObject::tr("<None>"));
            item->setIcon(1, QIcon());
        }
    };

    updateNode();

    QObject::connect(monitor, &CrMonitor::nodeChanged, &receiver_, [updateNode]{
        updateNode();
    });

    item->setData(2, Qt::EditRole, monitor->screenNumber());
    //item->setFlags(item->flags() | Qt::ItemIsEditable);
    QObject::connect(monitor, &CrMonitor::screenNumberChanged, &receiver_, [monitor, item]{
        item->setData(2, Qt::EditRole, monitor->screenNumber());
    });

    monitors_.insert(monitor);
    monitorsView_->addTopLevelItem(item);
    monitorItems_.insert(monitor, item);
}

void CrProjectSettingsDialogPrivate::removeMonitor(CrMonitor *monitor)
{
    QObject::disconnect(monitor, &CrMonitor::nameChanged, &receiver_, nullptr);
    QObject::disconnect(monitor->node(), &CrNode::pathChanged, &receiver_, nullptr);
    QObject::disconnect(monitor->node(), &CrNode::iconChanged, &receiver_, nullptr);
    QObject::disconnect(monitor, &CrMonitor::screenNumberChanged, &receiver_, nullptr);
    monitors_.remove(monitor);
    delete monitorItems_.value(monitor);
    monitorItems_.remove(monitor);
}

void CrProjectSettingsDialogPrivate::addPlugin(CrPlugin *plugin)
{
    if(plugin->status() == CrPlugin::Error){
        return;
    }

    if(plugin->status() == CrPlugin::NotLoaded){
        if(!plugin->load()){
            project_->pushErrorMessage(QObject::tr("Load plugin error."),
                                       plugin->errorString(),
                                       nullptr);

            if(QTreeWidgetItem* item = pluginItems_.value(plugin, nullptr)){
                setItemBackground(item, Qt::red);
            }
            return;
        }
    }

    plugins_.insert(plugin);

    if(QTreeWidgetItem* item = pluginItems_.value(plugin, nullptr)){
        setItemBackground(item, Qt::green);
    }
}

void CrProjectSettingsDialogPrivate::removePlugin(CrPlugin *plugin)
{
    plugins_.remove(plugin);
    if(QTreeWidgetItem* item = pluginItems_.value(plugin, nullptr)){
        setItemBackground(item, Qt::white);
    }
}

void CrProjectSettingsDialogPrivate::setItemBackground(QTreeWidgetItem *item, const QBrush &brush)
{
    for (int i = 0; i < item->columnCount(); ++i) {
        item->setBackground(i, brush);
    }
}

CrProjectSettingsDialog::CrProjectSettingsDialog(QWidget *parent) :
    QDialog (parent), d_(new CrProjectSettingsDialogPrivate(this))
{
    auto mainLayout = new QVBoxLayout();
    setLayout(mainLayout);

    auto mainTabWidget = new QTabWidget();
    mainLayout->addWidget(mainTabWidget);

    // General tab
    //

    auto generalTab = new QWidget();
    mainTabWidget->addTab(generalTab, tr("General"));

    auto generalTabLayuout = new QVBoxLayout();
    generalTab->setLayout(generalTabLayuout);

    auto generalFormLayout = new QFormLayout();
    generalTabLayuout->addLayout(generalFormLayout);

    d_->nameEditor_ = new QLineEdit();
    generalFormLayout->addRow(tr("Name"), d_->nameEditor_);

    d_->autorEditor_ = new QLineEdit();
    generalFormLayout->addRow(tr("Autor"), d_->autorEditor_);

    d_->editorPasswordEditor_ = new QLineEdit();
    generalFormLayout->addRow(tr("Editor password"),  d_->editorPasswordEditor_);

    d_->monitorPasswordEditor_ = new QLineEdit();
    generalFormLayout->addRow(tr("Monitor password"), d_->monitorPasswordEditor_);

    auto commentsLabel = new QLabel(tr("Comments"));
    generalTabLayuout->addWidget(commentsLabel);

    d_->commentsEditor_ = new QTextEdit();
    generalTabLayuout->addWidget(d_->commentsEditor_);

    // Monitors tab
    //

    auto monitorsTab = new QWidget();
    mainTabWidget->addTab(monitorsTab, tr("Monitors"));

    auto monitorsTabLayuout = new QVBoxLayout();
    monitorsTab->setLayout(monitorsTabLayuout);

    auto monitorsButtonLayout = new QHBoxLayout();
    monitorsTabLayuout->addLayout(monitorsButtonLayout);

    monitorsButtonLayout->addStretch();

    auto addMonitorButton = new QPushButton(tr("Add"));
    monitorsButtonLayout->addWidget(addMonitorButton);
    connect(addMonitorButton, &QPushButton::clicked, this, [this](){
        if(d_->project_){
            d_->addMonitor(new CrMonitor(d_->project_));
        }

    });

    auto removeMonitorButton = new QPushButton(tr("Remove"));
    monitorsButtonLayout->addWidget(removeMonitorButton);
    connect(removeMonitorButton, &QPushButton::clicked, this, [this](){
        foreach(QTreeWidgetItem* selectItem, d_->monitorsView_->selectedItems()){
            CrMonitor* monitor = selectItem->data(0, Qt::UserRole).value<CrMonitor*>();
            d_->removeMonitor(monitor);
        }
    });

    d_->monitorsView_ = new QTreeWidget();
    d_->monitorsView_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    d_->monitorsView_->header()->setSectionResizeMode(QHeaderView::Interactive);
    d_->monitorsView_->setSelectionBehavior(QAbstractItemView::SelectRows);
    d_->monitorsView_->setHeaderLabels({tr("Name"), tr("Node"), tr("Screen number")});

    connect(d_->monitorsView_,  QOverload<QTreeWidgetItem*, int>::of(&QTreeWidget::itemDoubleClicked),
            this, [this](QTreeWidgetItem* item, int column){
        if(column == 1){
            CrMonitor* monitor = item->data(0, Qt::UserRole).value<CrMonitor*>();
            Q_ASSERT(monitor);
            CrProjectExplorerDialog* explorer = CrProjectExplorerDialog::commonDialog();
            explorer->setProject(d_->project_);
            explorer->setFilter(CrProjectExplorerDialog::NoFilter);

            if(explorer->exec()){
                if(!explorer->selectedNodes().isEmpty()){
                    monitor->setNode(explorer->selectedNodes().first());
                }else {
                    monitor->setNode(nullptr);
                }
            }
        } else {
            d_->monitorsView_->editItem(item, column);
        }
    });

    connect(d_->monitorsView_,  QOverload<QTreeWidgetItem*, int>::of(&QTreeWidget::itemChanged),
            this, [](QTreeWidgetItem* item, int column){
        CrMonitor* monitor = item->data(0, Qt::UserRole).value<CrMonitor*>();
        if(!monitor)
            return;
        Q_ASSERT(monitor);
        switch (column) {
        case 0:{
            monitor->setName(item->text(0).simplified());
            break;
        }
        case 2:{
            monitor->setScreenNumber(item->data(2, Qt::EditRole).toInt());
            break;
        }
        default:
            break;
        }
    });


    monitorsTabLayuout->addWidget(d_->monitorsView_);


    // Plugins tab
    //

    auto pluginsTab = new QWidget();
    mainTabWidget->addTab(pluginsTab, tr("Plugins"));

    auto pluginsTabLayuout = new QVBoxLayout();
    pluginsTab->setLayout(pluginsTabLayuout);

    auto pluginsButtonLayout = new QHBoxLayout();
    pluginsTabLayuout->addLayout(pluginsButtonLayout);

    auto infoPluginButton = new QPushButton(tr("Info"));
    pluginsButtonLayout->addWidget(infoPluginButton);
    connect(infoPluginButton, &QPushButton::clicked, this, [this](){
        QTreeWidgetItem* item = d_->pluginsView_->currentItem();
        if(!item)
            return;
        //
    });

    pluginsButtonLayout->addStretch();

    auto loadPluginButton = new QPushButton(tr("Load"));
    pluginsButtonLayout->addWidget(loadPluginButton);
    connect(loadPluginButton, &QPushButton::clicked, this, [this](){
        QTreeWidgetItem* item = d_->pluginsView_->currentItem();
        if(!item)
            return;

        CrPlugin* plugin = item->data(0, Qt::UserRole).value<CrPlugin*>();
        Q_ASSERT(plugin);

        d_->addPlugin(plugin);

    });

    auto removePluginButton = new QPushButton(tr("Unload"));
    pluginsButtonLayout->addWidget(removePluginButton);
    connect(removePluginButton, &QPushButton::clicked, this, [this](){
        QTreeWidgetItem* item = d_->pluginsView_->currentItem();
        if(!item)
            return;

        CrPlugin* plugin = item->data(0, Qt::UserRole).value<CrPlugin*>();
        Q_ASSERT(plugin);

        d_->removePlugin(plugin);

    });

    d_->pluginsView_->setSelectionMode(QAbstractItemView::SingleSelection);
    d_->pluginsView_->header()->setSectionResizeMode(QHeaderView::Interactive);
    d_->pluginsView_->setSelectionBehavior(QAbstractItemView::SelectRows);
    d_->pluginsView_->setHeaderLabels({tr("Name"), tr("Version")});
    pluginsTabLayuout->addWidget(d_->pluginsView_);

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    mainLayout->addWidget(buttonBox);

    connect(this, &QDialog::accepted, this, [this](){

        if(!d_->project_)
            return ;

        QString name = d_->nameEditor_->text().simplified();
        QString autor = d_->autorEditor_->text().simplified();
        QString editorPassword = d_->editorPasswordEditor_->text().simplified();
        QString monitorPassword = d_->monitorPasswordEditor_->text().simplified();

        if(name == d_->project_->name()
                && autor == d_->project_->autorName()
                && editorPassword == d_->project_->editorPassword()
                && monitorPassword == d_->project_->monitorPassword()
                && d_->monitors_ == d_->project_->monitors().toSet()
                && d_->plugins_ == d_->project_->plugins().toSet())
            return ;

        class ChangeProjectSettings : public QUndoCommand
        {
        public:
            ChangeProjectSettings(CrProject* project, const QString& name,
                                  const QString& autor, const QString& editorPassword,
                                  const QString& monitorPassword, QSet<CrMonitor*> monitors, QSet<CrPlugin*> plugins,
                                  QUndoCommand *parent = nullptr):
                QUndoCommand(parent),
                project_(project),
                newName_(name),
                newAutor_(autor),
                newEditorPassword_(editorPassword),
                newMonitorPassword_(monitorPassword),
                newMonitors_(monitors),
                newPlugins_(plugins)

            {

                oldName_ = project_->name();
                oldAutor_ = project_->autorName();
                oldEditorPassword_ = project_->editorPassword();
                oldMonitorPassword_ = project_->monitorPassword();
                oldMonitors_ = project_->monitors().toSet();
                oldPlugins_ = project_->plugins().toSet();

                setText(tr("Change settings project \"%1\"").arg(project_->name()));
            }
            virtual void undo(){
                project_->setName(oldName_);
                project_->setAutorName(oldAutor_);
                project_->setEditorPassword(oldEditorPassword_);
                project_->setMonitorPassword(oldMonitorPassword_);

                foreach(CrMonitor* addMonitor, oldMonitors_ - newMonitors_){
                    project_->addMonitor(addMonitor);
                }

                foreach(CrMonitor* delMonitor, newMonitors_ - oldMonitors_){
                    project_->removeMonitor(delMonitor);
                }

                foreach(CrPlugin* addPlugin, oldPlugins_ - newPlugins_){
                    project_->addPlugin(addPlugin);
                }

                foreach(CrPlugin* delPlugin,  newPlugins_ - oldPlugins_){
                    project_->removePlugin(delPlugin);
                }

            }
            virtual void redo(){
                project_->setName(newName_);
                project_->setAutorName(newAutor_);
                project_->setEditorPassword(newEditorPassword_);
                project_->setMonitorPassword(newMonitorPassword_);

                foreach(CrMonitor* addMonitor, newMonitors_ - oldMonitors_){
                    project_->addMonitor(addMonitor);
                }
                foreach(CrMonitor* delMonitor, oldMonitors_ - newMonitors_){
                    project_->removeMonitor(delMonitor);
                }

                foreach(CrPlugin* addPlugin, newPlugins_ - oldPlugins_){
                    project_->addPlugin(addPlugin);
                }

                foreach(CrPlugin* delPlugin, oldPlugins_ - newPlugins_){
                    project_->removePlugin(delPlugin);
                }

            }
        private:
            CrProject* project_;

            QString newName_;
            QString oldName_;

            QString newAutor_;
            QString oldAutor_;

            QString newEditorPassword_;
            QString oldEditorPassword_;

            QString newMonitorPassword_;
            QString oldMonitorPassword_;

            QSet<CrMonitor*> newMonitors_;
            QSet<CrMonitor*> oldMonitors_;

            QSet<CrPlugin*> newPlugins_;
            QSet<CrPlugin*> oldPlugins_;
        };

        d_->project_->pushUndoCommand(new ChangeProjectSettings(d_->project_, name, autor, editorPassword, monitorPassword, d_->monitors_, d_->plugins_));
    });

    connect(this, &QDialog::rejected, this, [this](){

        if(!d_->project_)
            return ;

        foreach(CrMonitor* delMonitor, d_->monitors_ - d_->project_->monitors().toSet()){
            d_->removeMonitor(delMonitor);
        }

        foreach(CrPlugin* delPlugin, d_->plugins_ - d_->project_->plugins().toSet()){
            d_->removePlugin(delPlugin);
        }

    });
}

CrProjectSettingsDialog::~CrProjectSettingsDialog()
{
    delete d_;
}

CrProject *CrProjectSettingsDialog::project() const
{
    return d_->project_;
}

void CrProjectSettingsDialog::setProject(CrProject *project)
{
    if(d_->project_ == project)
        return;

    foreach(CrMonitor* monitor, d_->monitors_){
        d_->removeMonitor(monitor);
    }

    foreach(CrPlugin* plugin, d_->plugins_){
        d_->removePlugin(plugin);
    }

    if(d_->project_){

        disconnect(d_->project_, &CrProject::nameChanged,            &d_->receiver_, nullptr);
        disconnect(d_->project_, &CrProject::autorNameChanged,       &d_->receiver_, nullptr);
        disconnect(d_->project_, &CrProject::editorPasswordChanged,  &d_->receiver_, nullptr);
        disconnect(d_->project_, &CrProject::monitorPasswordChanged, &d_->receiver_, nullptr);
        disconnect(d_->project_, &CrProject::commentsChanged,        &d_->receiver_, nullptr);
        disconnect(d_->project_, &CrProject::monitorAdded,           &d_->receiver_, nullptr);
        disconnect(d_->project_, &CrProject::monitorRemoved,         &d_->receiver_, nullptr);
        disconnect(d_->project_, &CrProject::pluginAdded,            &d_->receiver_, nullptr);
        disconnect(d_->project_, &CrProject::pluginRemoved,          &d_->receiver_, nullptr);
    }

    d_->project_ = project;

    if(d_->project_){
        d_->nameEditor_->setText(d_->project_->name());
        connect(d_->project_, &CrProject::nameChanged,
                &d_->receiver_, [this]{d_->nameEditor_->setText(d_->project_->name());});

        d_->autorEditor_->setText(d_->project_->autorName());
        connect(d_->project_, &CrProject::autorNameChanged,
                &d_->receiver_, [this]{d_->autorEditor_->setText(d_->project_->autorName());});

        d_->editorPasswordEditor_->setText(d_->project_->editorPassword());
        connect(d_->project_, &CrProject::editorPasswordChanged,
                &d_->receiver_, [this]{d_->editorPasswordEditor_->setText(d_->project_->editorPassword());});

        d_->monitorPasswordEditor_->setText(d_->project_->monitorPassword());
        connect(d_->project_, &CrProject::monitorPasswordChanged,
                &d_->receiver_, [this]{d_->monitorPasswordEditor_->setText(d_->project_->monitorPassword());});

        d_->commentsEditor_->setText(d_->project_->comments());
        connect(d_->project_, &CrProject::commentsChanged,
                &d_->receiver_, [this]{d_->commentsEditor_->setText(d_->project_->comments());});


        foreach(CrMonitor* monitor, d_->project_->monitors()){
            d_->addMonitor(monitor);
        }
        connect(d_->project_, QOverload<CrMonitor*>::of(&CrProject::monitorAdded),
                &d_->receiver_, [this](CrMonitor* monitor){d_->addMonitor(monitor);});

        connect(d_->project_, QOverload<CrMonitor*>::of(&CrProject::monitorRemoved),
                &d_->receiver_, [this](CrMonitor* monitor){d_->removeMonitor(monitor);});

        foreach(CrPlugin* plugin, d_->project_->plugins()){
            d_->addPlugin(plugin);
        }
        connect(d_->project_, QOverload<CrPlugin*>::of(&CrProject::pluginAdded),
                &d_->receiver_, [this](CrPlugin* plugin){d_->addPlugin(plugin);});
        connect(d_->project_, QOverload<CrPlugin*>::of(&CrProject::pluginRemoved),
                &d_->receiver_, [this](CrPlugin* plugin){d_->removePlugin(plugin);});


    }else {
        d_->nameEditor_->setText(d_->project_->name());
        d_->autorEditor_->setText(d_->project_->autorName());
        d_->editorPasswordEditor_->setText(d_->project_->editorPassword());
        d_->monitorPasswordEditor_->setText(d_->project_->monitorPassword());
        d_->commentsEditor_->setText(d_->project_->comments());
    }

    emit projectChanged();
}

bool CrProjectSettingsDialog::restoreState(const QByteArray &state)
{
    QDataStream stream(state);
    QByteArray monitorsViewState, pluginsViewState;
    stream >> monitorsViewState >> pluginsViewState;
    d_->monitorsView_->header()->restoreState(monitorsViewState);
    d_->pluginsView_->header()->restoreState(pluginsViewState);
    return true;
}

QByteArray CrProjectSettingsDialog::saveState() const
{
    QByteArray state;
    QDataStream stream(&state, QIODevice::WriteOnly);
    stream << d_->monitorsView_->header()->saveState()
           << d_->pluginsView_->header()->saveState();
    return state;
}
CrProjectSettingsDialog* CrProjectSettingsDialog::commonDialog(){
    static CrProjectSettingsDialog* common = nullptr;
    if(!common){
        common = new CrProjectSettingsDialog();
        auto settings = new QSettings("CrayonEnt", "Crayon");
        common->restoreGeometry(settings->value("commonProjectSettingsDialogGeom").toByteArray());
        common->restoreState(settings->value("commonProjectSettingsDialogState").toByteArray());

        connect(QApplication::instance(), &QApplication::aboutToQuit, common, [settings](){
            settings->setValue("commonProjectSettingsDialogGeom", CrProjectSettingsDialog::commonDialog()->saveGeometry());
            settings->setValue("commonProjectSettingsDialogState", CrProjectSettingsDialog::commonDialog()->saveState());
        });
    }

    return common;
}

