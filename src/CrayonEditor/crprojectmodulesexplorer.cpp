#include "crprojectmodulesexplorer.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTreeView>
#include <QSortFilterProxyModel>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QHeaderView>
#include <QContextMenuEvent>
#include <QMenu>
#include <QAction>
#include <QSettings>
#include <QApplication>
#include <QMimeData>

#include <CrModule>
#include <CrPlugin>
#include <CrProject>
#include <CrLog>

#include  <crprojectmodulesmodel.h>


CrProjectModulesExplorer::CrProjectModulesExplorer(CrProject *project, QWidget *parent) :
    QWidget(parent),
    project_(project)
{
    auto mainLayout = new QVBoxLayout();
    mainLayout->setMargin(0);
    setLayout(mainLayout);

    auto headerLayout = new QHBoxLayout();
    headerLayout->setContentsMargins(5,0,5,0);
    headerLayout->addWidget(new QLabel(tr("Find")));
    mainLayout->addLayout(headerLayout);

    auto findField = new QLineEdit();
    headerLayout->addWidget(findField);

    auto findLabel = new QLabel();
    findLabel->setPixmap(QPixmap(":/CrayonGraphicsEditorResources/search_icon.svg").scaled(14, 14, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    headerLayout->addWidget(findLabel);

    view_ = new QTreeView();
    auto model = new CrProjectModulesModel(this);
    model->setFolderIcon(QIcon(":/CrayonGraphicsEditorResources/folder_icon.svg"));
    model->setProject(project_);
    auto proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(model);
    proxyModel->setFilterKeyColumn(CrProjectModulesModel::Name);
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxyModel->setRecursiveFilteringEnabled(true);
    connect(findField, &QLineEdit::textChanged, this, [findField, proxyModel]{
        proxyModel->setFilterFixedString(findField->text());
    });

    view_->setModel(proxyModel);
    view_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    view_->setDragEnabled(true);
    view_->setAcceptDrops(true);
    view_->setDropIndicatorShown(true);
    view_->header()->setSectionResizeMode(QHeaderView::Interactive);
    view_->setAutoExpandDelay(600);
    view_->setUniformRowHeights(true);
    view_->setIconSize(QSize(20, 20));
    view_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    view_->setSelectionBehavior(QAbstractItemView::SelectRows);
    mainLayout->addWidget(view_);

   // auto popup_ = new QMenu(this);
}

CrProjectModulesExplorer::~CrProjectModulesExplorer()
{

}

bool CrProjectModulesExplorer::restoreState(const QByteArray &state)
{
    QDataStream stream(state);
    QByteArray headerState;
    stream >> headerState;
    view_->header()->restoreState(headerState);
    return true;
}

QByteArray CrProjectModulesExplorer::saveState() const
{
    QByteArray state;
    QDataStream stream(&state, QIODevice::WriteOnly);
    stream << view_->header()->saveState();
    return state;
}

