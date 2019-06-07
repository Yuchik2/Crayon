#include "crprojectmodulesexplorerdialog.h"

#include <QApplication>
#include <QSettings>
#include <QTreeView>
#include <QSpinBox>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QSortFilterProxyModel>

#include <QLabel>
#include <QHeaderView>

#include <QMimeData>

#include <crmodule.h>
#include <CrPlugin>
#include "crprojectmodulesmodel.h"

#include <QDebug>

class CrProjectModulesExplorerDialogPrivate {
public:
    CrProjectModulesModel* model_;
    QSortFilterProxyModel* proxyModel_;
    QTreeView* view_;
    QSpinBox* countSpinBox_;
    CrModule* currentModule_;

};

CrProjectModulesExplorerDialog::CrProjectModulesExplorerDialog(QWidget *parent) :
    QDialog (parent),
    d_(new CrProjectModulesExplorerDialogPrivate())
{
    setModal(true);
    setSizeGripEnabled(true);

    auto mainLayout = new QVBoxLayout();
    setLayout(mainLayout);

    d_->model_  = new CrProjectModulesModel(this);
    connect(d_->model_, &CrProjectModulesModel::projectChanged, this, &CrProjectModulesExplorerDialog::projectChanged);

    d_->proxyModel_ = new QSortFilterProxyModel(this);
    d_->proxyModel_->setSourceModel(d_->model_);
    d_->proxyModel_->setFilterKeyColumn(CrProjectModulesModel::Name);
    d_->proxyModel_->setFilterCaseSensitivity(Qt::CaseInsensitive);
    d_->proxyModel_->setRecursiveFilteringEnabled(true);

    auto headerLayout = new QHBoxLayout();
    mainLayout->addLayout(headerLayout);

    auto findField = new QLineEdit();
    findField->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    connect(findField, &QLineEdit::textChanged, this, [this, findField]{
        d_->proxyModel_->setFilterFixedString(findField->text());
    });
    headerLayout->addWidget(findField);

    auto findLabel = new QLabel();
    findLabel->setPixmap(QPixmap(":/CrayonGraphicsEditorResources/search_icon.svg").scaled(14, 14, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    headerLayout->addWidget(findLabel);

    d_->view_ = new QTreeView;
    d_->view_->setModel(d_->proxyModel_);
    d_->view_->setSelectionMode(QAbstractItemView::SingleSelection);
    d_->view_->setSelectionBehavior(QAbstractItemView::SelectRows);
    d_->view_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mainLayout->addWidget(d_->view_);

    connect(d_->view_, QOverload<const QModelIndex&>::of(&QTreeView::clicked),
            this, [this](const QModelIndex& index){
        if(d_->model_->rowCount(d_->proxyModel_->mapToSource(index)) == 0)
            return;
        setCurrentModule(d_->model_->getModule(d_->proxyModel_->mapToSource(index)));
    });

    // Footer
    auto footerLayout = new QHBoxLayout();
    mainLayout->addLayout(footerLayout);

    auto footerIconLabel = new QLabel();
    footerLayout->addWidget(footerIconLabel);

    auto footerInfoLayout = new QGridLayout();
    footerLayout->addLayout(footerInfoLayout);

    footerInfoLayout->addWidget(new QLabel(tr("Name:")), 0 ,0);
    auto footerNameField = new QLabel();
    footerNameField->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    footerInfoLayout->addWidget(footerNameField, 0,1);

    footerInfoLayout->addWidget(new QLabel(tr("Id:")), 1 ,0);
    auto footerIdField = new QLabel();
    footerIdField->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    footerInfoLayout->addWidget(footerIdField, 1,1);

    footerInfoLayout->addWidget(new QLabel(tr("Version:")), 0 ,2);
    auto footerVersionField = new QLabel(tr("None"));
    footerVersionField->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    footerInfoLayout->addWidget(footerVersionField, 0,3);

    footerInfoLayout->addWidget(new QLabel(tr("Plugin:")), 1 ,2);
    auto footerPluginField = new QLabel();
    footerPluginField->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    footerInfoLayout->addWidget(footerPluginField, 1,3);

    connect(d_->view_->selectionModel(), QOverload<const QModelIndex &, const QModelIndex &>::of(&QItemSelectionModel::currentChanged),
            this, [this, footerIconLabel, footerNameField, footerIdField, footerVersionField, footerPluginField]
            (const QModelIndex & current, const QModelIndex & previous){

        Q_UNUSED(previous);
        CrModule* module = d_->model_->getModule(d_->proxyModel_->mapToSource(current));
        setCurrentModule(module);
        if(module){
            setCurrentModule(module);
            footerIconLabel->setPixmap(module->icon().pixmap(24,24));
            footerNameField->setText(module->path());
            footerIdField->setText(module->id());
            footerVersionField->setText(module->version().toString());
            if(module->plugin()){
                footerPluginField->setText(module->plugin()->key().id());
            }else {
                footerPluginField->setText(tr("None"));
            }
        }else {
            footerIconLabel->clear();
            footerNameField->clear();
            footerIdField->clear();
            footerVersionField->clear();
            footerPluginField->clear();
        }
    });

    // Button box

    auto bottomLayout = new QHBoxLayout();
    mainLayout->addLayout(bottomLayout);
    bottomLayout->addWidget(new QLabel(tr("Count")));
    d_->countSpinBox_ = new QSpinBox();
    d_->countSpinBox_->setRange(1, 256);
    bottomLayout->addWidget(d_->countSpinBox_);

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Open | QDialogButtonBox::Cancel);
    bottomLayout->addWidget(buttonBox);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

CrProjectModulesExplorerDialog::~CrProjectModulesExplorerDialog()
{
    delete d_;
}

CrProject *CrProjectModulesExplorerDialog::project() const
{
    return d_->model_->project();
}

void CrProjectModulesExplorerDialog::setProject(CrProject *project)
{
    d_->model_->setProject(project);
}

CrProjectModulesExplorerDialog *CrProjectModulesExplorerDialog::commonDialog()
{
    static CrProjectModulesExplorerDialog* common = nullptr;
    if(!common){
        common = new CrProjectModulesExplorerDialog();
        auto settings = new QSettings("CrayonEnt", "Crayon");
        common->restoreGeometry(settings->value("commonModulesExplorerDialogGeom").toByteArray());
        common->restoreState(settings->value("commonModulesExplorerDialogState").toByteArray());

        connect(QApplication::instance(), &QApplication::aboutToQuit, common, [settings](){
            settings->setValue("commonModulesExplorerDialogGeom", CrProjectModulesExplorerDialog::commonDialog()->saveGeometry());
            settings->setValue("commonModulesExplorerDialogState", CrProjectModulesExplorerDialog::commonDialog()->saveState());
        });
    }

    return common;
}

QByteArray CrProjectModulesExplorerDialog::saveState() const
{
    QByteArray state;
    QDataStream stream(&state, QIODevice::WriteOnly);
    stream << d_->view_->header()->saveState();
    return state;
}

bool CrProjectModulesExplorerDialog::restoreState(const QByteArray &state)
{
    QDataStream stream(state);
    QByteArray viewState;
    stream >> viewState;
    d_->view_->header()->restoreState(viewState);

    return true;
}

int CrProjectModulesExplorerDialog::count() const
{
    return d_->countSpinBox_->value();
}

void CrProjectModulesExplorerDialog::setCount(int count)
{
    d_->countSpinBox_->setValue(count);
}

CrModule *CrProjectModulesExplorerDialog::currentModule() const
{
    return d_->currentModule_;
}

void CrProjectModulesExplorerDialog::setCurrentModule(CrModule *currentModule)
{
    if(d_->currentModule_ == currentModule)
        return;

    d_->currentModule_ = currentModule;
}
