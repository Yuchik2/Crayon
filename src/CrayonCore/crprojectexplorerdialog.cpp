#include "crprojectexplorerdialog.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QButtonGroup>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QSplitter>
#include <QTreeView>
#include <QHeaderView>
#include <QSettings>
#include <QApplication>
#include <QSortFilterProxyModel>
#include <QIdentityProxyModel>

#include <crproject.h>
#include <CrNode>
#include <CrModule>
#include <crprojectmodel.h>

#include <QDebug>

class CrProjectFilterProxyModel : public QIdentityProxyModel{

public:
    explicit CrProjectFilterProxyModel(QObject* parent = nullptr) :  QIdentityProxyModel(parent) {}
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
    void setFilter(const CrProjectExplorerDialog::Filters &filter, CrNode* target = nullptr);

private:
    CrProjectExplorerDialog::Filters filter_ = CrProjectExplorerDialog::NoFilter;
    CrNode* filterTarget_ = nullptr;
};

class  CrProjectExplorerDialogPrivate{

public:

    CrProject* project_ = nullptr;
    CrNode* currentNode_ = nullptr;
    CrProjectModel* model_;
    QSortFilterProxyModel* proxyModel_;
    CrProjectFilterProxyModel* filterProxyModel_;
    QTreeView* sideView_;
    QTreeView* view_;
    QLineEdit* pathField_;
    QSplitter* splitter_;
};

QVariant CrProjectFilterProxyModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if(filter_ == CrProjectExplorerDialog::NoFilter || role != Qt::BackgroundRole)
        return  QIdentityProxyModel::data(index, role);


    CrNode* node = data(index.siblingAtColumn(CrProjectModel::Editor)).value<CrNode*>();

    QColor backgroundColor("#82e0aa");
    switch (filter_) {
    case CrProjectExplorerDialog::ValidInputNode:{
        if(!node || !filterTarget_             ||
                !(node->flags() & CrNode::Out) ||
                !(filterTarget_->flags() & CrNode::In) ||
                !node->isValidOunputNode(filterTarget_) ||
                !filterTarget_->isValidInputNode(node)
                )
            backgroundColor =  "#e74c3c";
        break;
    }
    case CrProjectExplorerDialog::ValidOutputNode:{
        if(!node || !filterTarget_             ||
                !(node->flags() & CrNode::In) ||
                !(filterTarget_->flags() & CrNode::Out) ||
                !node->isValidInputNode(filterTarget_) ||
                !filterTarget_->isValidOunputNode(node)
                )
            backgroundColor =  "#e74c3c";
        break;
    }
    default:
        break;
    }
    return backgroundColor;
}

Qt::ItemFlags CrProjectFilterProxyModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flag = QIdentityProxyModel::flags(index);
    if(filter_ == CrProjectExplorerDialog::NoFilter || !index.isValid())
        return flag;

    CrNode* node = data(index.siblingAtColumn(CrProjectModel::Editor)).value<CrNode*>();
    switch (filter_) {
    case CrProjectExplorerDialog::ValidInputNode:{
        if(!node || !filterTarget_             ||
                !(node->flags() & CrNode::Out) ||
                !(filterTarget_->flags() & CrNode::In)  ||
                !node->isValidOunputNode(filterTarget_) ||
                !filterTarget_->isValidInputNode(node)
                )
            flag &= ~Qt::ItemIsSelectable;
        break;
    }
    case CrProjectExplorerDialog::ValidOutputNode:{
        if(!node || !filterTarget_             ||
                !(node->flags() & CrNode::In)  ||
                !(filterTarget_->flags() & CrNode::Out) ||
                !node->isValidInputNode(filterTarget_)  ||
                !filterTarget_->isValidOunputNode(node)
                )
            flag &= ~Qt::ItemIsSelectable;
        break;
    }
    default:
        break;
    }
    return flag;
}

void CrProjectFilterProxyModel::setFilter(const CrProjectExplorerDialog::Filters &filter, CrNode *target)
{
    if(filter_ == filter && filterTarget_ == target)
        return;
    filter_ = filter;
    filterTarget_ = target;
    switch (filter_) {
    case CrProjectExplorerDialog::NoFilter:{
        filterTarget_ = nullptr;
        break;
    }
    case CrProjectExplorerDialog::ValidInputNode:{

        break;
    }
    case CrProjectExplorerDialog::ValidOutputNode:{

        break;
    }
    default:
        break;
    }
}

CrProjectExplorerDialog::CrProjectExplorerDialog(QWidget *parent) :
    QDialog (parent), d_(new CrProjectExplorerDialogPrivate())
{
    setModal(true);
    setSizeGripEnabled(true);

    auto mainLayout = new QVBoxLayout();
    setLayout(mainLayout);

    d_->model_  = new CrProjectModel(this);
    d_->proxyModel_ = new QSortFilterProxyModel(this);
    d_->proxyModel_->setSourceModel(d_->model_);
    d_->proxyModel_->setFilterKeyColumn(CrProjectModel::Name);
    d_->proxyModel_->setFilterCaseSensitivity(Qt::CaseInsensitive);
    d_->proxyModel_->setRecursiveFilteringEnabled(true);
    d_->filterProxyModel_ = new CrProjectFilterProxyModel(this);
    d_->filterProxyModel_->setSourceModel(d_->proxyModel_);

    // Splitter
    d_->splitter_ = new QSplitter();
    d_->splitter_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainLayout->addWidget(d_->splitter_);

    d_->sideView_ = new QTreeView();
    d_->sideView_->setModel(d_->model_);
    d_->sideView_->setColumnHidden(CrProjectModel::Index, true);
    d_->sideView_->setColumnHidden(CrProjectModel::Comments, true);
    d_->sideView_->setColumnHidden(CrProjectModel::Editor, true);
    d_->sideView_->setColumnHidden(CrProjectModel::InputNode, true);
    d_->sideView_->setColumnHidden(CrProjectModel::OutputNodes, true);
    d_->sideView_->setColumnHidden(CrProjectModel::Module, true);
    d_->sideView_->setTreePosition(CrProjectModel::Name);
    d_->sideView_->setHeaderHidden(true);
    d_->sideView_->setSelectionMode(QAbstractItemView::SingleSelection);
    d_->sideView_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    d_->splitter_->addWidget(d_->sideView_);

    auto viewWidget = new QWidget();
    d_->splitter_->addWidget(viewWidget);

    auto viewLayout = new QVBoxLayout();
    viewLayout->setMargin(0);
    viewWidget->setLayout(viewLayout);

    auto pathLayout = new QHBoxLayout();
    viewLayout->addLayout(pathLayout);

    d_->pathField_ = new QLineEdit();
    pathLayout->addWidget(d_->pathField_);

    auto buttonUp = new QToolButton();
   // buttonUp->setIcon(QIcon(":/resources/upload-button.png"));
    connect(buttonUp, &QToolButton::clicked, this, [this]{
        if(d_->currentNode_)
            setCurrentNode(d_->currentNode_->parentNode());
    });
    pathLayout->addWidget(buttonUp);

    auto findField = new QLineEdit();
    findField->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    connect(findField, &QLineEdit::textChanged, this, [this, findField]{
        d_->proxyModel_->setFilterFixedString(findField->text());
        if(findField->text().isEmpty()){
            if(d_->currentNode_){
                QModelIndex index = d_->model_->getIndex(d_->currentNode_);
                d_->view_->setRootIndex(d_->filterProxyModel_->mapFromSource(d_->proxyModel_->mapFromSource(index)));
                d_->view_->update(d_->filterProxyModel_->mapFromSource(d_->proxyModel_->mapFromSource(index)));
                d_->pathField_->setText(d_->currentNode_->path());
            }else {
                QModelIndex index = QModelIndex();
                d_->view_->setRootIndex(index);
                d_->view_->update(index);
                d_->pathField_->clear();
            }
        }
    });
    pathLayout->addWidget(findField);

    auto findLabel = new QLabel();
    findLabel->setPixmap(QPixmap(":/CrayonGraphicsEditorResources/search_icon.svg").scaled(16, 16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    pathLayout->addWidget(findLabel);

    d_->view_ = new QTreeView;
    d_->view_->setModel(d_->filterProxyModel_);
    d_->view_->setSelectionMode(QAbstractItemView::SingleSelection);
    d_->view_->setSelectionBehavior(QAbstractItemView::SelectRows);
    d_->view_->setColumnHidden(CrProjectModel::Index, true);
    d_->view_->setColumnHidden(CrProjectModel::Editor, true);
    d_->view_->setColumnHidden(CrProjectModel::InputNode, true);
    d_->view_->setColumnHidden(CrProjectModel::OutputNodes, true);
    d_->view_->setColumnHidden(CrProjectModel::Module, true);
    d_->view_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    d_->view_->setItemsExpandable(false);
    viewLayout->addWidget(d_->view_);

    connect(d_->sideView_, QOverload<const QModelIndex&>::of(&QTreeView::clicked),
            this, [this](const QModelIndex& index){
        if(d_->model_->rowCount(index) == 0)
            return;
        setCurrentNode(d_->model_->getNode(index));
    });

    connect(d_->view_, QOverload<const QModelIndex&>::of(&QTreeView::doubleClicked),
            this, [this](const QModelIndex& index){
        if(d_->model_->rowCount(d_->proxyModel_->mapToSource(d_->filterProxyModel_->mapToSource(index))) == 0)
            return;
        setCurrentNode(d_->model_->getNode(d_->proxyModel_->mapToSource(d_->filterProxyModel_->mapToSource(index))));
    });

    // Footer
    auto footerLayout = new QHBoxLayout();
    mainLayout->addLayout(footerLayout);

    auto nodeIconLabel = new QLabel();
    footerLayout->addWidget(nodeIconLabel);

    auto footerInfoLayout = new QGridLayout();
    footerLayout->addLayout(footerInfoLayout);

    footerInfoLayout->addWidget(new QLabel(tr("Name:")), 0 ,0);
    auto footerNameField = new QLabel();
    footerInfoLayout->addWidget(footerNameField, 0,1);

    footerInfoLayout->addWidget(new QLabel(tr("Uuid:")), 1 ,0);
    auto footerUuidField = new QLabel();
    footerInfoLayout->addWidget(footerUuidField, 1,1);

    footerInfoLayout->addWidget(new QLabel(tr("Module:")), 0 ,2);
    auto footerModuleField = new QLabel(tr("None"));
    footerInfoLayout->addWidget(footerModuleField, 0,3);

    footerInfoLayout->addWidget(new QLabel(tr("Version:")), 1 ,2);
    auto footerVersionField = new QLabel();
    footerInfoLayout->addWidget(footerVersionField, 1,3);

    auto commentsLayout = new QVBoxLayout();
    footerLayout->addLayout(commentsLayout);

    commentsLayout->addWidget(new QLabel(tr("Comments")));
    auto commentsField = new QLabel();
    commentsField->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    commentsLayout->addWidget(commentsField);

    connect(d_->view_->selectionModel(), QOverload<const QModelIndex &, const QModelIndex &>::of(&QItemSelectionModel::currentChanged),
            this, [this, nodeIconLabel, footerNameField, footerUuidField, footerModuleField, footerVersionField, commentsField]
            (const QModelIndex & current, const QModelIndex & previous){


        Q_UNUSED(previous);
        CrNode* node = current.isValid() ? d_->model_->getNode(d_->proxyModel_->mapToSource(d_->filterProxyModel_->mapToSource(current))) : nullptr;
        if(node){
            nodeIconLabel->setPixmap(node->icon().pixmap(24, 24));
            footerNameField->setText(node->name());
            footerUuidField->setText(node->uuid().toString());
            commentsField->setText(node->comments());
            if(node->isModule()){
                footerModuleField->setText(node->module()->id());
                footerVersionField->setText(node->module()->version().toString());
            }else {
                footerModuleField->setText(tr("None"));
                footerVersionField->clear();
            }
        }else {
            nodeIconLabel->clear();
            footerNameField->clear();
            footerUuidField->clear();
            commentsField->clear();
            footerModuleField->clear();
            footerVersionField->clear();
        }
    });

    // Button box

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Open | QDialogButtonBox::Cancel);
    mainLayout->addWidget(buttonBox);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

CrProjectExplorerDialog::~CrProjectExplorerDialog()
{
    delete d_;
}

CrProjectExplorerDialog *CrProjectExplorerDialog::commonDialog()
{
    static CrProjectExplorerDialog* common = nullptr;
    if(!common){
        common = new CrProjectExplorerDialog();
        auto settings = new QSettings("CrayonEnt", "Crayon");
        common->restoreGeometry(settings->value("commonProjectExplorerDialogGeom").toByteArray());
        common->restoreState(settings->value("commonProjectExplorerDialogState").toByteArray());

        connect(QApplication::instance(), &QApplication::aboutToQuit, common, [settings](){
            settings->setValue("commonProjectExplorerDialogGeom", CrProjectExplorerDialog::commonDialog()->saveGeometry());
            settings->setValue("commonProjectExplorerDialogState", CrProjectExplorerDialog::commonDialog()->saveState());
        });
    }

    return common;
}

CrProject *CrProjectExplorerDialog::project() const
{
    return d_->project_;
}

void CrProjectExplorerDialog::setProject(CrProject *project)
{
    if(d_->project_ == project)
        return;

    d_->filterProxyModel_->setFilter(CrProjectExplorerDialog::NoFilter);

    if(d_->project_){
        d_->model_->setProject(nullptr);
        disconnect(d_->project_, &QObject::destroyed, this, nullptr);
    }

    d_->project_ = project;

    if(d_->project_){
        d_->model_->setProject(d_->project_);
        setCurrentNode(d_->project_->rootNode());
        connect(d_->project_, &QObject::destroyed, this, [this]{
            setCurrentNode(nullptr);
        });
    }else {
        d_->model_->setProject(nullptr);
        setCurrentNode(nullptr);
    }
}

void CrProjectExplorerDialog::setCurrentNode(CrNode *node)
{
    if(d_->currentNode_ == node)
        return;

    d_->currentNode_ = node;

    if(d_->currentNode_){
        QModelIndex index = d_->model_->getIndex(d_->currentNode_);
        d_->view_->setRootIndex(d_->filterProxyModel_->mapFromSource(d_->proxyModel_->mapFromSource(index)));
        d_->view_->update(d_->filterProxyModel_->mapFromSource(d_->proxyModel_->mapFromSource(index)));
        d_->pathField_->setText(d_->currentNode_->path());
    }else {
        QModelIndex index = QModelIndex();
        d_->view_->setRootIndex(index);
        d_->view_->update(index);
        d_->pathField_->clear();
    }
}

CrNode *CrProjectExplorerDialog::currentNode()
{
    return d_->currentNode_;
}

QList<CrNode *> CrProjectExplorerDialog::selectedNodes() const
{
    QList<CrNode *> list;
    foreach(const QModelIndex& index, d_->view_->selectionModel()->selectedRows()){
        CrNode*  node = d_->model_->getNode(d_->proxyModel_->mapToSource(d_->filterProxyModel_->mapToSource(index)));
        if(node)
            list << node;
    }
    return list;
}

QByteArray CrProjectExplorerDialog::saveState() const
{
    QByteArray state;
    QDataStream stream(&state, QIODevice::WriteOnly);
    stream << d_->splitter_->saveState();
    stream << d_->view_->header()->saveState();
    return state;
}

bool CrProjectExplorerDialog::restoreState(const QByteArray &state)
{
    QDataStream stream(state);
    QByteArray splitterState, viewState;
    stream >> splitterState >> viewState;
    d_->splitter_->restoreState(splitterState);
    d_->view_->header()->restoreState(viewState);
    return true;
}

void CrProjectExplorerDialog::setFilter(const Filters &filter, CrNode *target)
{
    d_->filterProxyModel_->setFilter(filter, target);
    d_->view_->update(QModelIndex());
}





