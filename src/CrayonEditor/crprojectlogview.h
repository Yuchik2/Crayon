#ifndef CRPROJECTLOGVIEW_H
#define CRPROJECTLOGVIEW_H

#include <QTreeWidget>

class CrProject;
class CrProjectLogView : public QTreeWidget
{
    Q_OBJECT
public:
    explicit CrProjectLogView(QWidget *parent = nullptr);

    CrProject *project() const;
    void setProject(CrProject *project);

signals:
    void projectChanged();
private:
    CrProject *project_ = nullptr;

};

#endif // CRPROJECTLOGVIEW_H
