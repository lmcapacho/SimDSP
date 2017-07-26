#ifndef SDNEWPROJECT_H
#define SDNEWPROJECT_H

#include <QDialog>
#include <QFileDialog>
#include <QDir>

namespace Ui {
class SDNewProject;
}

class SDNewProject : public QDialog
{
    Q_OBJECT

public:
    explicit SDNewProject(QWidget *parent = 0);
    ~SDNewProject();
    void getProjectInfo(QString &name, QString &path);

public slots:
    void browsePath();
    void accepted();

private:
    Ui::SDNewProject *ui;
};

#endif // SDNEWPROJECT_H
